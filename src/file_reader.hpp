#include <algorithm>
#include <array>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>
#include <zlib.h>
#include <limits>
#include <spdlog/spdlog.h>

static constexpr std::size_t Kilo = 1'000ULL;
static constexpr std::size_t Mega = 1'000'000ULL;
static constexpr std::size_t Giga = 1'000'000'000ULL;

// TODO: support different number of producers and buffer, in case consumption takes longer

class MTFileReader {
private:
  std::size_t m_num_producers{};
  std::size_t m_buffer_size{};
  std::atomic<std::size_t> m_idx{};

  std::vector<std::FILE *> m_files;
  std::vector<std::vector<char>> m_buffers;
  std::vector<std::size_t> m_prod_idx;
  std::vector<std::size_t> m_cons_idx;
  std::vector<std::mutex> m_mtx;
  std::vector<std::condition_variable> m_cond;

  bool m_all_done{};
  std::mutex m_all_done_mtx;
  std::condition_variable m_all_done_cond;

public:
  MTFileReader(
      char const *filename,
      std::size_t num_producers,
      std::size_t buffer_size)
      : m_num_producers(num_producers),
        m_buffer_size(buffer_size),
        m_files(m_num_producers),
        m_buffers(m_num_producers),
        m_prod_idx(m_num_producers, std::numeric_limits<std::size_t>::max()),
        m_cons_idx(m_num_producers, std::numeric_limits<std::size_t>::max()),
        m_mtx(num_producers),
        m_cond(num_producers) {
    // open num_producers file streams to the same file
    std::generate(std::begin(m_files), std::end(m_files), [filename] {
      return std::fopen(filename, "rb");
    });

    // advance all file streams, except the first one, to the needed position
    for (std::size_t idx = 1; idx < m_num_producers; ++idx) {
      auto ec = std::fseek(m_files[idx], (long)(idx * m_buffer_size), SEEK_SET);
      if (ec != 0) {
        spdlog::error("fseek failed for file {}", idx);
      }
    }

    // allocate memory for the buffer
    std::generate(std::begin(m_buffers), std::end(m_buffers), [buffer_size] {
      return std::vector<char>(buffer_size);
    });
  }

  MTFileReader(MTFileReader const &) = delete;
  MTFileReader &operator=(MTFileReader const &) = delete;
  MTFileReader(MTFileReader &&) = delete;
  MTFileReader &operator=(MTFileReader &&) = delete;

  ~MTFileReader() {
    for (std::FILE *fp : m_files) {
      auto ec = std::fclose(fp);
      if (ec != 0) {
        spdlog::error("An error occured while closing a file");
      }
    }
  }

  void produce_chunks() {
    produce_uncomp();
  }

  void mark_all_done() {
    std::unique_lock<std::mutex> lck(m_all_done_mtx);
    m_all_done = true;
    m_all_done_cond.notify_all();
  }

  void produce_uncomp() {
    while (true) {
      // get the index and post-increment it atomically
      std::size_t idx{m_idx.fetch_add(1)};
      spdlog::debug("Producing chunk {}", idx);

      auto const c_bucket = idx % m_num_producers;
      auto *file = m_files[c_bucket];
      auto &c_mtx = m_mtx[c_bucket];
      auto &c_cond = m_cond[c_bucket];
      auto &c_buffer = m_buffers[c_bucket];
      auto &c_prod_idx = m_prod_idx[c_bucket];

      // wait for the previous chunk in this bucket to be consumed
      if (idx >= m_num_producers) {
        auto const p_idx = idx - m_num_producers;
        auto const &c_cons_idx = m_cons_idx[c_bucket];
        spdlog::debug("Waiting for chunk {} to be consumed to produce chunk {}", p_idx, idx);
        std::unique_lock<std::mutex> lck(c_mtx);
        c_cond.wait(lck, [&c_cons_idx, &p_idx]() { return c_cons_idx == p_idx; });
        spdlog::debug("Done waiting for chunk {} to be consumed to produce chunk {}", p_idx, idx);
      }

      std::unique_lock<std::mutex> lck(c_mtx);
      auto const bytes_read = std::fread(c_buffer.data(), sizeof(char), m_buffer_size, file);
      spdlog::debug("Read {} bytes for chunk {}", bytes_read, idx);

      // we read fewer bytes than the size of the buffer, so shrink the buffer
      if (bytes_read != m_buffer_size) {
        c_buffer.resize(bytes_read);
      }

      if (bytes_read == 0) {
        if (std::ferror(m_files[c_bucket]) != 0) {
          spdlog::error("An error occured while reading file {}", c_bucket);
        }
        spdlog::debug("Produced empty chunk {}", idx);
        c_prod_idx = idx;
        c_cond.notify_all();
        return;
      }

      auto ec = std::fseek(
          m_files[c_bucket],
          (long)((m_num_producers - 1) * m_buffer_size),
          SEEK_CUR);
      if (ec != 0) {
        spdlog::error("fseek failed for file {}", idx);
        return;
      }

      // mark the buffer as not consumed
      spdlog::debug("Produced chunk {}", idx);
      c_prod_idx = idx;
      c_cond.notify_all();
    }
  }

  std::pair<std::vector<char> const &, bool> get_chunk(std::size_t idx) {
    spdlog::debug("Getting chunk {}", idx);
    auto const c_bucket = idx % m_num_producers;
    auto &mtx = m_mtx[c_bucket];
    auto &cond = m_cond[c_bucket];
    auto const &buffer = m_buffers[c_bucket];
    auto const &c_prod_idx = m_prod_idx[c_bucket];

    {
      std::unique_lock<std::mutex> lck(mtx);
      cond.wait(lck, [&c_prod_idx, &idx, &buffer]() {
        return c_prod_idx == idx || buffer.empty();
      });
    }

    spdlog::debug("Got chunk {}", idx);
    return {buffer, buffer.size() != m_buffer_size};
  }

  void mark_chunk(std::size_t idx) {
    spdlog::debug("Marking chunk {}", idx);
    // we need to wait for the previous chunk to be consumed, to mark this
    // chunk as consumed
    if (idx >= m_num_producers) {
      auto const p_idx = idx - 1;
      auto const p_bucket = p_idx % m_num_producers;
      auto const &p_cons_idx = m_cons_idx[p_bucket];
      auto &p_mtx = m_mtx[p_bucket];
      auto &p_cond = m_cond[p_bucket];

      spdlog::debug("Waiting for chunk {} to be marked to mark chunk {}", p_idx, idx);
      std::unique_lock<std::mutex> lck(p_mtx);
      p_cond.wait(lck, [&p_cons_idx, p_idx]() { return p_cons_idx == p_idx; });
      spdlog::debug("Done waiting for chunk {} to be marked to mark chunk {}", p_idx, idx);
    }

    // mark the chunk as consumed and notify the threads waiting on it
    {
      auto const c_bucket = idx % m_num_producers;
      auto &c_mtx = m_mtx[c_bucket];
      auto &c_cond = m_cond[c_bucket];
      auto &c_cons_idx = m_cons_idx[c_bucket];

      std::unique_lock<std::mutex> lck(c_mtx);
      c_cons_idx = idx;
      c_cond.notify_all();
    }
    spdlog::debug("Marked chunk {}", idx);
  }
};

//#include <algorithm>
//#include <array>
//#include <atomic>
//#include <condition_variable>
//#include <cstdio>
//#include <fmt/core.h>
//#include <fmt/std.h>
//#include <functional>
//#include <mutex>
//#include <shared_mutex>
//#include <string>
//#include <vector>
//#include <zlib.h>
//
//static constexpr std::size_t Kilo = 1'000ULL;
//static constexpr std::size_t Mega = 1'000'000ULL;
//static constexpr std::size_t Giga = 1'000'000'000ULL;
//
//class MTFileReader {
//private:
//  std::size_t m_num_producers{};
//  std::size_t m_buffer_size{};
//  std::vector<char> const m_empty_buffer;
//  std::vector<std::vector<char>> m_buffers;
//
//  mutable std::vector<std::mutex> m_mtx;
//  mutable std::vector<std::condition_variable> m_cond;
//  // we use an array of bools because the std::vector<bool> specialization uses single-bit representation which causes
//  // thread issues
//  std::unique_ptr<bool[]> m_consumed;
//
//  std::vector<std::FILE *> m_files;
//  std::atomic<std::size_t> m_idx{};
//  std::mutex m_consumed_mtx;
//  std::condition_variable m_consumed_cond;
//  std::shared_mutex m_produced_mtx;
//  std::condition_variable_any m_produced_cond;
//  std::size_t m_chunks_consumed{};
//  // when we have more consumers than producers, we must make sure that no two consumers get the same produced chunk
//  std::size_t m_chunks_produced{};
//  std::atomic<std::size_t> m_chunks_returned{};
//  std::atomic<std::size_t> m_empty_returned{};
//
//  // we need the m_finished member, in case the user requests chunk 1000 and only 1 chunk can be produced
//  // in this case, the get_chunk function must be able to stop waiting and return the empty buffer
//  std::atomic<std::size_t> m_finished{};
//  bool m_all_done{false};
//
//  std::mutex m_gz_mutex;
//  gzFile m_gz_file{};
//  std::string m_error{};
//
//public:
//  explicit MTFileReader(
//      char const *filename,
//      std::size_t num_producers,
//      std::size_t buffer_size)
//      : m_num_producers(num_producers),
//        m_buffer_size(buffer_size),
//        m_buffers(m_num_producers),
//        m_mtx(m_num_producers),
//        m_cond(m_num_producers),
//        m_consumed(std::make_unique<bool[]>(m_num_producers)),
//        m_files(m_num_producers) {
//    std::generate(begin(m_buffers), end(m_buffers), [buffer_size] {
//      return std::vector<char>(buffer_size);
//    });
//
//    std::fill_n(m_consumed.get(), m_num_producers, true);
//
//    if (is_gzip_file(filename)) {
//      m_gz_file = gzopen(filename, "rb");
//      if (m_gz_file == Z_NULL) {
//        m_error =
//            fmt::format("MTFileReader(): Could not open file {}", filename);
//      }
//    } else {
//      // open NUM_PRODUCERS file streams to the same file
//      std::generate(begin(m_files), end(m_files), [filename] {
//        return std::fopen(filename, "rb");
//      });
//      // advance all file streams, except the first one, to the needed position
//      for (std::size_t idx = 1; idx < m_num_producers; ++idx) {
//        auto ec =
//            std::fseek(m_files[idx], (long)(idx * m_buffer_size), SEEK_SET);
//        if (ec != 0) {
//          fmt::print("fseek failed for file {}\n", idx);
//        }
//      }
//    }
//  }
//  MTFileReader(MTFileReader const &) = delete;
//  MTFileReader &operator=(MTFileReader const &) = delete;
//  MTFileReader(MTFileReader &&) = delete;
//  MTFileReader &operator=(MTFileReader &&) = delete;
//  ~MTFileReader() {
//    for (std::size_t idx = 0; idx < m_num_producers; ++idx) {
//      auto ec = std::fclose(m_files[idx]);
//      if (ec != 0) {
//        fmt::print("An error occured while closing a file {}\n", idx);
//      }
//    }
//    gzclose(m_gz_file);
//  }
//
//  [[nodiscard]] std::string const &get_error() const { return m_error; }
//
//  void produce_chunks() {
//    //if (m_gz_file != nullptr) {
//    //  produce_comp();
//    //} else {
//    produce_uncomp();
//    //}
//  }
//
//  void set_all_done(std::size_t c_idx) {
//    auto finished = m_finished.fetch_add(1);
//    if (finished == m_num_producers - 1) {
//      // this is the last producer which tried to read past the EOF, so lock
//      // all m_mtx mutexes, and notify all waiting threads that all
//      // producers are done
//      for (std::size_t i = 0; i < m_num_producers; ++i) {
//        if (i != c_idx) {
//          m_mtx[i].lock();
//        }
//      }
//      m_all_done = true;
//      for (std::size_t i = 0; i < m_num_producers; ++i) {
//        m_cond[i].notify_all();
//        if (i != c_idx) {
//          m_mtx[i].unlock();
//        }
//      }
//    }
//  }
//
//  void produce_uncomp() {
//    while (true) {
//      // get the index and post-increment it atomically
//      auto idx = m_idx.fetch_add(1);
//      auto c_idx = idx % m_num_producers;
//
//      auto &buffer = m_buffers[c_idx];
//
//      // wait for a buffer to become available
//      std::unique_lock<std::mutex> lck(m_mtx[c_idx]);
//      m_cond[c_idx].wait(lck, [&] { return m_consumed[c_idx]; });
//
//      auto const bytes_read = std::fread(
//          buffer.data(),
//          sizeof(char),
//          m_buffer_size,
//          m_files[c_idx]);
//
//      // we read fewer bytes than the size of the buffer, so shrink the buffer
//      if (bytes_read != m_buffer_size) {
//        buffer.resize(bytes_read);
//      }
//
//      if (bytes_read == 0) {
//        if (std::ferror(m_files[c_idx]) != 0) {
//          fmt::print("An error occured while reading file {}\n", c_idx);
//        }
//        set_all_done(c_idx);
//        return;
//      }
//
//      auto ec = std::fseek(
//          m_files[c_idx],
//          (long)((m_num_producers - 1) * m_buffer_size),
//          SEEK_CUR);
//      if (ec != 0) {
//        fmt::print("fseek failed for file {}\n", idx);
//        set_all_done(c_idx);
//        return;
//      }
//
//      std::scoped_lock sl(m_produced_mtx);
//      m_consumed[c_idx] = false;
//      ++m_chunks_produced;
//      m_produced_cond.notify_all();
//      m_cond[c_idx].notify_all();
//    }
//  }
//
//  std::vector<char> const &get_chunk(std::size_t idx) {
//    // wrap-around the read index
//    auto c_idx = idx % m_num_producers;
//    // lock the mutex of the r_idx buffer
//    // we need to wait for two things:
//    // 1. for the buffer to not be consumed
//    // 2. for the produced chunks to be greater than idx
//    while (true) {
//      {
//        // wait for the buffer to not be consumed, or all producers to have
//        // finished
//        std::unique_lock lck{m_mtx[c_idx]};
//        m_cond[c_idx].wait(lck, [&]() {
//          return !m_consumed[c_idx] || m_all_done;
//        });
//
//        if (m_all_done && m_consumed[c_idx]) {
//          ++m_empty_returned;
//          return m_empty_buffer;
//        }
//      }
//      {
//        // if the produced chunks are greater than idx, return the buffer, else
//        // continue waiting until all producers have finished or the buffer has
//        // been consumed by another consumer
//        std::shared_lock lck(m_produced_mtx);
//        if (idx < m_chunks_produced) {
//          ++m_chunks_returned;
//          return m_buffers[c_idx];
//        }
//      }
//    }
//  }
//
//  void mark_chunk(std::size_t idx) {
//    // we need to wait for the previous chunk to be consumed, to mark this
//    // chunk as consumed
//    if (idx != 0) {
//      std::unique_lock lck(m_consumed_mtx);
//      m_consumed_cond.wait(lck, [&] { return idx <= m_chunks_consumed; });
//    }
//
//    auto c_idx = idx % m_num_producers;
//    // we need to lock both mutexes, since the first controls changes to the
//    // m_consumed and the second changes to the m_chunks consumed
//    std::scoped_lock c_lck(m_mtx[c_idx], m_consumed_mtx);
//    // mark the current chunk as consumed, and increment the number of consumed
//    // chunks
//    m_consumed[c_idx] = true;
//    ++m_chunks_consumed;
//    // we notify_all, because there might be more than one threads waiting on
//    // this condition_variable: a produce_* function waiting to refill this
//    // buffer and the mark_chunk waiting on the current condition_variable to
//    // mark the next chunk consumed
//    m_cond[c_idx].notify_all();
//    // we notify_all, because many threads will be waiting for the change in
//    // m_chunks_consumed
//    m_consumed_cond.notify_all();
//  }
//
//  //void produce_comp() {
//  //  while (true) {
//  //    // only one thread can read the compressed file stream at a time
//  //    std::scoped_lock sl{m_gz_mutex};
//
//  //    // get the index and post-increment it atomically
//  //    auto c_idx = m_idx.fetch_add(1);
//  //    // wrap-around the write index
//  //    c_idx = c_idx % m_num_producers;
//
//  //    auto &buffer = m_buffers[c_idx];
//  //    auto &cond = m_conds[c_idx];
//  //    auto &mtx = m_mutexes[c_idx];
//
//  //    // lock the mutex of the w_idx buffer
//  //    std::unique_lock lck{mtx};
//  //    // wait for the buffer to be consumed in case it's not empty
//  //    cond.wait(lck, [&]() { return m_consumed[c_idx]; });
//
//  //    auto bytes_read = gzread(m_gz_file, buffer.data(), m_buffer_size);
//  //    if (bytes_read < 0) {
//  //      // gzread returns a negative value in case of error
//  //      int errnum{};
//  //      auto const *error_msg = gzerror(m_gz_file, &errnum);
//  //      m_error = fmt::format(
//  //          "produce_comp(): An error happened while reading from the "
//  //          "compressed stream\nreturn_code: {}\ngzerror(): {}\nerrnum: {}",
//  //          bytes_read,
//  //          error_msg,
//  //          errnum);
//  //      return;
//  //    }
//  //    if (bytes_read == 0) {
//  //      // gzread return 0 in case we are at the EOF
//  //      lck.unlock();
//  //      std::for_each(
//  //          begin(m_conds),
//  //          end(m_conds),
//  //          std::mem_fn(&std::condition_variable::notify_all));
//  //      return;
//  //    }
//
//  //    // we need to resize the buffer (reducing its size) if the last block we
//  //    // read didn't have enough bytes
//  //    if ((std::size_t)bytes_read != m_buffer_size) {
//  //      buffer.resize((std::size_t)bytes_read);
//  //    }
//
//  //    m_consumed[c_idx] = false;
//  //    lck.unlock();
//  //    cond.notify_one();
//  //  }
//  //}
//
//private:
//  bool is_gzip_file(std::string_view filename) {
//    static constexpr std::array<unsigned char, 2> magic_bytes{0x1f, 0x8b};
//    std::array<char, magic_bytes.size()> buffer{};
//
//    std::ifstream infile(filename.data(), std::ios::in | std::ios::binary);
//    if (!infile.is_open()) {
//      m_error = fmt::format("is_gzip_file(): Could not open file {}", filename);
//    }
//
//    if (!infile.read(buffer.data(), buffer.size())) {
//      return false;
//    }
//
//    return std::memcmp(begin(magic_bytes), begin(buffer), buffer.size()) == 0;
//  }
//};
