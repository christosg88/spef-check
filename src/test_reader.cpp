#include "file_reader.hpp"
#include <BS_thread_pool.hpp>
#include <array>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fmt/core.h>
#include <iostream>
#include <spdlog/fmt/std.h>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <thread>
#include <vector>

static constexpr std::array<std::size_t, 12> NUM_PRODUCERS{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 40};
static constexpr std::array<std::size_t, 12> NUM_CONSUMERS{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 40};
static constexpr std::array<std::size_t, 16> BUFFER_SIZES{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1'024, 2'000, 5'000, 10'000, 100'000, 1'000'000};

//namespace pegtl = tao::pegtl;
namespace fs = std::filesystem;
static const std::error_code no_error;

void consume(
    std::size_t id,
    std::atomic<std::size_t> &idx,
    MTFileReader &reader) {
  while (true) {
    auto const c_idx = idx.fetch_add(1);
    auto const &[chunk, stop] = reader.get_chunk(c_idx);
    if (chunk.empty()) {
      break;
    }
    std::FILE *of = std::fopen(
        fmt::format("/media/night/Files/tests/c{:05}_w{:02}.txt", c_idx, id).c_str(),
        "wb");
    auto bytes_written =
        std::fwrite(chunk.data(), sizeof(char), chunk.size(), of);
    if (bytes_written != chunk.size()) {
      if (std::ferror(of) != 0) {
        spdlog::error("An error occured while writing chunk {}", c_idx);
      } else {
        spdlog::error(
            "No error occured but fewer bytes than available were written "
            "while writing chunk {}",
            c_idx);
      }
      return;
    }
    auto ec = std::fclose(of);
    if (ec != 0) {
      spdlog::error(
          "An error occured while closing the file for chunk {}",
          c_idx);
      return;
    }
    reader.mark_chunk(c_idx);

    if (stop) {
      break;
    }
  }
}

void validate(fs::path const &target, std::vector<fs::path> const &gen_paths) {
  auto const target_size = fs::file_size(target);
  std::vector<char> target_output(target_size);
  {
    std::ifstream target_stream(target, std::ios::binary);
    target_stream.read(target_output.data(), target_size);
  }

  std::vector<char> gen_output(target_size);
  auto *data = gen_output.data();
  // load all gen_paths to memory
  for (auto const &gen_path : gen_paths) {
    auto const gen_size = fs::file_size(gen_path);
    std::ifstream gen_stream(gen_path, std::ios::binary);
    gen_stream.read(data, gen_size);
    std::advance(data, gen_size);
  }

  for (std::size_t i = 0; i < target_size; ++i) {
    if (target_output[i] != gen_output[i]) {
      throw std::runtime_error(fmt::format(
          "Inconsistency found at byte {} with values {} != {}",
          i,
          target_output[i],
          gen_output[i]));
    }
  }
}

void cleanup(std::vector<fs::path> const &gen_paths) {
  for (auto const &gen_path : gen_paths) {
    fs::remove(gen_path);
  }
}

void validate_and_cleanup(fs::path const &target) {
  // collect all generated files
  std::vector<fs::path> gen_paths;
  auto const dir_it_end = fs::directory_iterator();
  for (auto dir_it = fs::directory_iterator("/media/night/Files/tests");
       dir_it != dir_it_end;
       ++dir_it) {
    gen_paths.emplace_back(dir_it->path());
  }

  // sort the paths in lexicographic order
  std::sort(gen_paths.begin(), gen_paths.end());

  validate(target, gen_paths);
  cleanup(gen_paths);
}

void run_test1(
    fs::path const &spef_file,
    std::size_t buffer_size,
    std::size_t num_producers,
    std::size_t num_consumers) {
  spdlog::stopwatch sw;

  // the first producer reads the whole file
  auto wait_and_produce = [](MTFileReader &reader) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);
    reader.produce_uncomp();
  };

  auto const filesize = fs::file_size(spef_file);
  MTFileReader reader(spef_file.c_str(), num_producers, buffer_size);
  std::atomic<std::size_t> idx{0};

  BS::thread_pool pool(num_consumers + num_producers - 1);
  for (std::size_t i = 0; i < num_consumers; ++i) {
    pool.push_task(consume, i, std::ref(idx), std::ref(reader));
  }
  for (std::size_t i = 0; i < num_producers - 1; ++i) {
    pool.push_task(wait_and_produce, std::ref(reader));
  }
  reader.produce_uncomp();
  pool.wait_for_tasks();

  spdlog::info(
      "== TEST 1 -- {} FILE_SIZE:{} BUFFER_SIZE:{} NUM_PRODUCERS:{} NUM_CONSUMERS:{} RUNTIME:{}",
      spef_file.c_str(),
      filesize,
      buffer_size,
      num_producers,
      num_consumers,
      sw);
  validate_and_cleanup(spef_file);
}

void run_test2(
    fs::path const &spef_file,
    std::size_t buffer_size,
    std::size_t num_producers,
    std::size_t num_consumers) {
  spdlog::stopwatch sw;

  auto const filesize = fs::file_size(spef_file);
  MTFileReader reader(spef_file.c_str(), num_producers, buffer_size);
  std::atomic<std::size_t> idx{0};

  BS::thread_pool pool(num_consumers + num_producers - 1);
  for (std::size_t i = 0; i < num_consumers; ++i) {
    pool.push_task(consume, i, std::ref(idx), std::ref(reader));
  }
  for (std::size_t i = 0; i < num_producers - 1; ++i) {
    pool.push_task(&MTFileReader::produce_uncomp, std::ref(reader));
  }
  reader.produce_uncomp();
  pool.wait_for_tasks();

  spdlog::info(
      "== TEST 2 -- {} FILE_SIZE:{} BUFFER_SIZE:{} NUM_PRODUCERS:{} NUM_CONSUMERS:{}, RUNTIME: {}",
      spef_file.c_str(),
      filesize,
      buffer_size,
      num_producers,
      num_consumers,
      sw);
  validate_and_cleanup(spef_file);
}

void run_test3(
    fs::path const &spef_file,
    std::size_t buffer_size,
    std::size_t num_producers,
    std::size_t num_consumers) {
  spdlog::stopwatch sw;

  auto const filesize = fs::file_size(spef_file);
  MTFileReader reader(spef_file.c_str(), num_producers, buffer_size);
  std::atomic<std::size_t> idx{0};

  BS::thread_pool pool(num_consumers + num_producers - 1);
  for (std::size_t i = 0; i < num_consumers; ++i) {
    pool.push_task(consume, i, std::ref(idx), std::ref(reader));
  }
  for (std::size_t i = 0; i < num_producers - 1; ++i) {
    pool.push_task(&MTFileReader::produce_uncomp, std::ref(reader));
  }
  reader.produce_uncomp();
  pool.wait_for_tasks();

  spdlog::info(
      "== TEST 3 -- {} FILE_SIZE:{} BUFFER_SIZE:{} NUM_PRODUCERS:{} NUM_CONSUMERS:{} RUNTIME:{}",
      spef_file.c_str(),
      filesize,
      buffer_size,
      num_producers,
      num_consumers,
      sw);
  validate_and_cleanup(spef_file);
}

void test1(fs::path const &spef_file) {
  auto filesize = fs::file_size(spef_file);
  auto buffer_size = filesize;
  for (std::size_t num_producers : NUM_PRODUCERS) {
    for (std::size_t num_consumers : NUM_CONSUMERS) {
      run_test1(spef_file, buffer_size, num_producers, num_consumers);
    }
  }
}

void test2(fs::path const &spef_file) {
  // the first producer reads exactly the size of the file +-1
  auto const filesize = fs::file_size(spef_file);
  for (std::size_t buffer_size = filesize - 1; buffer_size <= filesize + 1; ++buffer_size) {
    for (std::size_t num_producers : NUM_PRODUCERS) {
      for (std::size_t num_consumers : NUM_CONSUMERS) {
        run_test2(spef_file, buffer_size, num_producers, num_consumers);
      }
    }
  }
}

void test3(fs::path const &spef_file) {
  auto const filesize = fs::file_size(spef_file);
  for (std::size_t buffer_size : BUFFER_SIZES) {
    // if more than 1'000 files would be produced, skip this step
    if (filesize / buffer_size > 1'000) {
      continue;
    }
    for (std::size_t num_producers : NUM_PRODUCERS) {
      for (std::size_t num_consumers : NUM_CONSUMERS) {
        run_test3(spef_file, buffer_size, num_producers, num_consumers);
      }
    }
  }
}

int main(int argc, char const *const *argv) {
  spdlog::set_level(spdlog::level::info);

  if (argc != 2) {
    spdlog::warn("usage: {} <file>", argv[0]);
    return EXIT_FAILURE;
  }

  fs::path const spef_file{argv[1]};

  static constexpr std::size_t NUM_REPS{1};
  for (std::size_t i = 0; i < NUM_REPS; ++i) {
    test1(spef_file);
  }
  for (std::size_t i = 0; i < NUM_REPS; ++i) {
    test2(spef_file);
  }
  for (std::size_t i = 0; i < NUM_REPS; ++i) {
    test3(spef_file);
  }

  return EXIT_SUCCESS;
}
