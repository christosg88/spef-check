#include <algorithm>
#include <array>
#include <atomic>
#include <condition_variable>
#include <dbg_out.h>
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
  std::vector<unsigned char>
      m_consumed;  // unsigned char instead of bool due to std::vector<bool> specialization

  std::vector<std::ifstream> m_files;
  std::atomic<std::size_t> m_idx{};
  std::atomic<std::size_t> m_chunks_consumed{};
  std::atomic<std::size_t>
      m_chunks_produced{};  // when we have more consumers than producers, we must make sure that
                            // no two consumers get the same produced chunk

  std::atomic<bool> m_finished{false};
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
        m_consumed(m_num_producers, true),
        m_files(m_num_producers) {
    std::generate(begin(m_buffers), end(m_buffers), [buffer_size] {
      return std::vector<char>(buffer_size);
    });
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
  ~MTFileReader() {
    fmt::print(
        "Produced:{} Consumed:{}\n",
        m_chunks_produced.load(),
        m_chunks_consumed.load());
    gzclose(m_gz_file);
  }

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
      // wrap-around the write index
      auto c_idx = idx % m_num_producers;

      auto &file = m_files[c_idx];
      auto &buffer = m_buffers[c_idx];

      std::unique_lock<std::mutex> lck(m_mtx[c_idx]);
      dbg_out(fmt::format("P: Waiting to produce buffer {}({})", c_idx, idx));
      // wait for a buffer to become available
      m_cond[c_idx].wait(lck, [&] { return m_consumed[c_idx]; });

      dbg_out(fmt::format(
          "P: Producing buffer {}({}) -- {} -- {}",
          c_idx,
          idx,
          (void *)buffer.data(),
          (void *)file.rdbuf()));

      file.read(buffer.data(), (long)m_buffer_size);

      auto const bytes_read = (std::size_t)file.gcount();
      if (file.fail() && bytes_read == 0) {
        dbg_out(fmt::format(
            "P: Didn't produce buffer because I tried to read past the EOF "
            "{}({})",
            c_idx,
            idx));
        if (idx != m_chunks_produced) {
          // only the producer who produced the last chunk should set m_finished
          return;
        }
        // we're at the EOF
        m_finished = true;
        // notify all waiting producers and consumers
        lck.unlock();
        std::for_each(
            begin(m_cond),
            end(m_cond),
            std::mem_fn(&std::condition_variable::notify_all));
        return;
      }

      ++m_chunks_produced;
      m_consumed[c_idx] = false;

      // we read fewer bytes than the size of the buffer, so shrink the buffer
      if (bytes_read != m_buffer_size) {
        buffer.resize(bytes_read);
      }

      file.seekg((long)((m_num_producers - 1) * m_buffer_size), std::ios::cur);

      dbg_out(fmt::format("P: Produced buffer {}({})", c_idx, idx));
      lck.unlock();
      m_cond[c_idx].notify_all();
    }
  }

  std::vector<char> const &get_chunk(std::size_t idx) const {
    // wrap-around the read index
    auto c_idx = idx % m_num_producers;
    // lock the mutex of the r_idx buffer
    std::unique_lock lck{m_mtx[c_idx]};
    dbg_out(fmt::format("C: Waiting to get chunk {}({})", c_idx, idx));
    // wait for the buffer to not be consumed, and make sure no two consumers get the same unconsumed chunk
    m_cond[c_idx].wait(lck, [&]() {
      dbg_out(
          fmt::format("C: idx:{} produced:{}", idx, m_chunks_produced.load()));
      return m_finished || (!m_consumed[c_idx] && idx < m_chunks_produced);
    });

    if (!m_consumed[c_idx] && idx < m_chunks_produced) {
      dbg_out(fmt::format(
          "C: Getting chunk buffer {}({}) with size {} and data {}",
          c_idx,
          idx,
          m_buffers[c_idx].size(),
          (void *)m_buffers[c_idx].data()));
      return m_buffers[c_idx];
    }

    dbg_out(fmt::format("C: Returning empty buffer"));
    return m_empty_buffer;
  }

  void mark_chunk(std::size_t idx) {
    // wrap-around the read index
    auto c_idx = idx % m_num_producers;
    std::unique_lock lck(m_mtx[c_idx]);
    dbg_out(fmt::format("C: Waiting to mark chunk ({})", idx));
    if (idx != 0) {
      // we need to wait for the previous chunk to be consumed, to mark this
      // chunk as consumed
      m_cond[c_idx].wait(lck, [&] { return idx <= m_chunks_consumed; });
    }

    // mark the current chunk as consumed, and notify the current chunk's
    // condition_variable in case a next chunk is waiting for it to be consumed
    m_consumed[c_idx] = true;
    ++m_chunks_consumed;
    dbg_out(fmt::format("C: Marked chunk ({})", idx));
    // we notify_all, because there might be more than one threads waiting on
    // this condition_variable: a produce_* function waiting to refill this
    // buffer and the mark_chunk waiting on the current condition_variable to
    // mark the next chunk consumed
    lck.unlock();
    auto n_idx = (idx + 1) % m_num_producers;
    m_cond[c_idx].notify_all();
    m_cond[n_idx].notify_all();
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
