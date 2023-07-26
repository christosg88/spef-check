#include <algorithm>
#include <array>
#include <atomic>
#include <condition_variable>
#include <fmt/core.h>
#include <fmt/std.h>
#include <fstream>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>
#include <zlib.h>

static constexpr std::size_t Kilo = 1'000ULL;
static constexpr std::size_t Mega = 1'000'000ULL;
static constexpr std::size_t Giga = 1'000'000'000ULL;

class MTFileReader {
private:
  std::size_t m_num_producers{};
  std::size_t m_buffer_size{};
  std::vector<char> const m_empty_buffer;
  std::vector<std::vector<char>> m_buffers;

  mutable std::vector<std::mutex> m_mtx;
  mutable std::vector<std::condition_variable> m_cond;
  // unsigned char instead of bool due to std::vector<bool> specialization
  std::unique_ptr<bool[]> m_consumed;

  std::vector<std::ifstream> m_files;
  std::atomic<std::size_t> m_idx{};
  std::mutex m_consumed_mtx;
  std::condition_variable m_consumed_cond;
  std::shared_mutex m_produced_mtx;
  std::condition_variable_any m_produced_cond;
  std::size_t m_chunks_consumed{};
  // when we have more consumers than producers, we must make sure that no two consumers get the same produced chunk
  std::size_t m_chunks_produced{};
  std::atomic<std::size_t> m_chunks_returned{};
  std::atomic<std::size_t> m_empty_returned{};

  // we need the m_finished member, in case the user requests chunk 1000 and only 1 chunk can be produced
  // in this case, the get_chunk function must be able to stop waiting and return the empty buffer
  std::atomic<std::size_t> m_finished{};
  bool m_all_done{false};

  std::mutex m_gz_mutex;
  gzFile m_gz_file{};
  std::string m_error{};

public:
  explicit MTFileReader(
      char const *filename,
      std::size_t num_producers,
      std::size_t buffer_size)
      : m_num_producers(num_producers),
        m_buffer_size(buffer_size),
        m_buffers(m_num_producers),
        m_mtx(m_num_producers),
        m_cond(m_num_producers),
        m_consumed(std::make_unique<bool[]>(m_num_producers)),
        m_files(m_num_producers) {
    std::generate(begin(m_buffers), end(m_buffers), [buffer_size] {
      return std::vector<char>(buffer_size);
    });

    std::fill_n(m_consumed.get(), m_num_producers, true);

    if (is_gzip_file(filename)) {
      m_gz_file = gzopen(filename, "rb");
      if (m_gz_file == Z_NULL) {
        m_error =
            fmt::format("MTFileReader(): Could not open file {}", filename);
      }
    } else {
      // open NUM_PRODUCERS file streams to the same file
      std::generate(begin(m_files), end(m_files), [filename] {
        return std::ifstream(filename, std::ios::binary);
      });
      // advance all file streams, except the first one, to the needed position
      for (std::size_t idx = 1; idx < m_num_producers; ++idx) {
        m_files[idx].seekg((long)(idx * m_buffer_size));
      }
    }
  }
  MTFileReader(MTFileReader const &) = delete;
  MTFileReader &operator=(MTFileReader const &) = delete;
  MTFileReader(MTFileReader &&) = delete;
  MTFileReader &operator=(MTFileReader &&) = delete;
  ~MTFileReader() { gzclose(m_gz_file); }

  [[nodiscard]] std::string const &get_error() const { return m_error; }

  void produce_chunks() {
    //if (m_gz_file != nullptr) {
    //  produce_comp();
    //} else {
    produce_uncomp();
    //}
  }

  void produce_uncomp() {
    while (true) {
      // get the index and post-increment it atomically
      auto idx = m_idx.fetch_add(1);
      auto c_idx = idx % m_num_producers;

      auto &file = m_files[c_idx];
      auto &buffer = m_buffers[c_idx];

      // wait for a buffer to become available
      std::unique_lock<std::mutex> lck(m_mtx[c_idx]);
      m_cond[c_idx].wait(lck, [&] { return m_consumed[c_idx]; });

      if (file.peek() == std::char_traits<char>::eof()) {
        lck.unlock();

        auto finished = m_finished.fetch_add(1);
        if (finished == m_num_producers - 1) {
          // this is the last producer which tried to read past the EOF, so lock
          // all m_mtx mutexes, and notify all waiting threads that all
          // producers are done
          for (std::size_t i = 0; i < m_num_producers; ++i) {
            m_mtx[i].lock();
          }
          m_all_done = true;
          for (std::size_t i = 0; i < m_num_producers; ++i) {
            m_cond[i].notify_all();
            m_mtx[i].unlock();
          }
        }
        return;
      }

      file.read(buffer.data(), (long)m_buffer_size);

      // we read fewer bytes than the size of the buffer, so shrink the buffer
      auto const bytes_read = (std::size_t)file.gcount();
      if (bytes_read != m_buffer_size) {
        buffer.resize(bytes_read);
      }

      file.seekg((long)((m_num_producers - 1) * m_buffer_size), std::ios::cur);

      lck.unlock();
      std::scoped_lock sl(m_mtx[c_idx], m_produced_mtx);
      m_consumed[c_idx] = false;
      ++m_chunks_produced;
      m_produced_cond.notify_all();
      m_cond[c_idx].notify_all();
    }
  }

  std::vector<char> const &get_chunk(std::size_t idx) {
    // wrap-around the read index
    auto c_idx = idx % m_num_producers;
    // lock the mutex of the r_idx buffer
    // we need to wait for two things:
    // 1. for the buffer to not be consumed
    // 2. for the produced chunks to be greater than idx
    while (true) {
      {
        // wait for the buffer to not be consumed, or all producers to have
        // finished
        std::unique_lock lck{m_mtx[c_idx]};
        m_cond[c_idx].wait(lck, [&]() {
          return !m_consumed[c_idx] || m_all_done;
        });

        if (m_all_done && m_consumed[c_idx]) {
          ++m_empty_returned;
          return m_empty_buffer;
        }
      }
      {
        // if the produced chunks are greater than idx, return the buffer, else
        // continue waiting until all producers have finished or the buffer has
        // been consumed by another consumer
        std::shared_lock lck(m_produced_mtx);
        if (idx < m_chunks_produced) {
          ++m_chunks_returned;
          return m_buffers[c_idx];
        }
      }
    }
  }

  void mark_chunk(std::size_t idx) {
    // we need to wait for the previous chunk to be consumed, to mark this
    // chunk as consumed
    if (idx != 0) {
      std::unique_lock lck(m_consumed_mtx);
      m_consumed_cond.wait(lck, [&] { return idx <= m_chunks_consumed; });
    }

    auto c_idx = idx % m_num_producers;
    // we need to lock both mutexes, since the first controls changes to the
    // m_consumed and the second changes to the m_chunks consumed
    std::scoped_lock c_lck(m_mtx[c_idx], m_consumed_mtx);
    // mark the current chunk as consumed, and increment the number of consumed
    // chunks
    m_consumed[c_idx] = true;
    ++m_chunks_consumed;
    // we notify_all, because there might be more than one threads waiting on
    // this condition_variable: a produce_* function waiting to refill this
    // buffer and the mark_chunk waiting on the current condition_variable to
    // mark the next chunk consumed
    m_cond[c_idx].notify_all();
    // we notify_all, because many threads will be waiting for the change in
    // m_chunks_consumed
    m_consumed_cond.notify_all();
  }

  //void produce_comp() {
  //  while (true) {
  //    // only one thread can read the compressed file stream at a time
  //    std::scoped_lock sl{m_gz_mutex};

  //    // get the index and post-increment it atomically
  //    auto c_idx = m_idx.fetch_add(1);
  //    // wrap-around the write index
  //    c_idx = c_idx % m_num_producers;

  //    auto &buffer = m_buffers[c_idx];
  //    auto &cond = m_conds[c_idx];
  //    auto &mtx = m_mutexes[c_idx];

  //    // lock the mutex of the w_idx buffer
  //    std::unique_lock lck{mtx};
  //    // wait for the buffer to be consumed in case it's not empty
  //    cond.wait(lck, [&]() { return m_consumed[c_idx]; });

  //    auto bytes_read = gzread(m_gz_file, buffer.data(), m_buffer_size);
  //    if (bytes_read < 0) {
  //      // gzread returns a negative value in case of error
  //      int errnum{};
  //      auto const *error_msg = gzerror(m_gz_file, &errnum);
  //      m_error = fmt::format(
  //          "produce_comp(): An error happened while reading from the "
  //          "compressed stream\nreturn_code: {}\ngzerror(): {}\nerrnum: {}",
  //          bytes_read,
  //          error_msg,
  //          errnum);
  //      return;
  //    }
  //    if (bytes_read == 0) {
  //      // gzread return 0 in case we are at the EOF
  //      lck.unlock();
  //      std::for_each(
  //          begin(m_conds),
  //          end(m_conds),
  //          std::mem_fn(&std::condition_variable::notify_all));
  //      return;
  //    }

  //    // we need to resize the buffer (reducing its size) if the last block we
  //    // read didn't have enough bytes
  //    if ((std::size_t)bytes_read != m_buffer_size) {
  //      buffer.resize((std::size_t)bytes_read);
  //    }

  //    m_consumed[c_idx] = false;
  //    lck.unlock();
  //    cond.notify_one();
  //  }
  //}

private:
  bool is_gzip_file(std::string_view filename) {
    static constexpr std::array<unsigned char, 2> magic_bytes{0x1f, 0x8b};
    std::array<char, magic_bytes.size()> buffer{};

    std::ifstream infile(filename.data(), std::ios::in | std::ios::binary);
    if (!infile.is_open()) {
      m_error = fmt::format("is_gzip_file(): Could not open file {}", filename);
    }

    if (!infile.read(buffer.data(), buffer.size())) {
      return false;
    }

    return std::memcmp(begin(magic_bytes), begin(buffer), buffer.size()) == 0;
  }
};
