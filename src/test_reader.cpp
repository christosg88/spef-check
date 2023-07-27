#include <filesystem>
#include <cstdio>
#include <iostream>
#include <thread>
#include <vector>
#include "file_reader.hpp"
#include <BS_thread_pool.hpp>
#include <chrono>

//namespace pegtl = tao::pegtl;
namespace fs = std::filesystem;


void consume(std::size_t id, std::atomic<std::size_t> &idx, MTFileReader &reader) {
  while (true) {
    auto const c_idx = idx.fetch_add(1);
    auto const &chunk = reader.get_chunk(c_idx);
    if (chunk.empty()) {
      break;
    }
    std::FILE *of = std::fopen(fmt::format("/SCRATCH/gkan/out/c{:05}_w{:02}.txt", c_idx, id).c_str(), "wb");
    auto bytes_written = std::fwrite(chunk.data(), sizeof(char), chunk.size(), of);
    if (bytes_written != chunk.size()) {
      if (std::ferror(of) != 0) {
        fmt::print("An error occured while writing chunk {}\n", c_idx);
      } else {
        fmt::print("No error occured but fewer bytes than available were written while writing chunk {}\n", c_idx);
      }
      return;
    }
    auto ec = std::fclose(of);
    if (ec != 0) {
      fmt::print("An error occured while closing the file for chunk {}\n", c_idx);
      return;
    }
    reader.mark_chunk(c_idx);
  }
}

void test1(fs::path const &spef_file) {
  // the first producer reads the whole file
  auto wait_and_produce = [](MTFileReader &reader) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    reader.produce_uncomp();
  };

  auto BUFFER_SIZE = fs::file_size(spef_file);
  for (std::size_t NUM_PRODUCERS = 1; NUM_PRODUCERS < 64; NUM_PRODUCERS *= 2) {
    for (std::size_t NUM_CONSUMERS = 1; NUM_CONSUMERS < 64;
         NUM_CONSUMERS *= 2) {
      fmt::print(
          "== {} BUFFER_SIZE:{} NUM_PRODUCERS:{} NUM_CONSUMERS:{}\n",
          spef_file,
          BUFFER_SIZE,
          NUM_PRODUCERS,
          NUM_CONSUMERS);

      MTFileReader reader(
          spef_file.c_str(),
          NUM_PRODUCERS,
          BUFFER_SIZE);

      std::atomic<std::size_t> idx{0};
      BS::thread_pool pool(NUM_CONSUMERS + NUM_PRODUCERS - 1);
      for (std::size_t i = 0; i < NUM_CONSUMERS; ++i) {
        pool.push_task(consume, i, std::ref(idx), std::ref(reader));
      }
      for (std::size_t i = 0; i < NUM_PRODUCERS - 1; ++i) {
        pool.push_task(wait_and_produce, std::ref(reader));
      }
      reader.produce_uncomp();

      pool.wait_for_tasks();
    }
  }
}

void test2(fs::path const &spef_file) {
  // the first producer reads exactly the size of the file +-1
  auto filesize = fs::file_size(spef_file);
  for (std::size_t BUFFER_SIZE = filesize - 1; BUFFER_SIZE < filesize + 1; ++BUFFER_SIZE) {
    for (std::size_t NUM_PRODUCERS = 1; NUM_PRODUCERS < 64;
         NUM_PRODUCERS *= 2) {
      for (std::size_t NUM_CONSUMERS = 1; NUM_CONSUMERS < 64;
           NUM_CONSUMERS *= 2) {
        fmt::print(
            "== {} BUFFER_SIZE:{} NUM_PRODUCERS:{} NUM_CONSUMERS:{}\n",
            spef_file,
            BUFFER_SIZE,
            NUM_PRODUCERS,
            NUM_CONSUMERS);

        MTFileReader reader(
            spef_file.c_str(),
            NUM_PRODUCERS,
            BUFFER_SIZE);

        std::atomic<std::size_t> idx{0};
        BS::thread_pool pool(NUM_CONSUMERS + NUM_PRODUCERS - 1);
        for (std::size_t i = 0; i < NUM_CONSUMERS; ++i) {
          pool.push_task(consume, i, std::ref(idx), std::ref(reader));
        }
        for (std::size_t i = 0; i < NUM_PRODUCERS - 1; ++i) {
          pool.push_task(&MTFileReader::produce_uncomp, std::ref(reader));
        }
        reader.produce_uncomp();

        pool.wait_for_tasks();
      }
    }
  }
}

void run_test(
    fs::path const &spef_file,
    std::size_t BUFFER_SIZE,
    std::size_t NUM_PRODUCERS,
    std::size_t NUM_CONSUMERS) {
  fmt::print(
      "== {} BUFFER_SIZE:{} NUM_PRODUCERS:{} NUM_CONSUMERS:{}\n",
      spef_file,
      BUFFER_SIZE,
      NUM_PRODUCERS,
      NUM_CONSUMERS);
  MTFileReader reader(spef_file.c_str(), NUM_PRODUCERS, BUFFER_SIZE);
  std::atomic<std::size_t> idx{0};

  BS::thread_pool pool(NUM_CONSUMERS + NUM_PRODUCERS - 1);
  for (std::size_t i = 0; i < NUM_CONSUMERS; ++i) {
    pool.push_task(consume, i, std::ref(idx), std::ref(reader));
  }
  for (std::size_t i = 0; i < NUM_PRODUCERS - 1; ++i) {
    pool.push_task(&MTFileReader::produce_uncomp, std::ref(reader));
  }
  reader.produce_uncomp();

  pool.wait_for_tasks();
}

void test3(fs::path const &spef_file) {
  for (std::size_t NUM_PRODUCERS : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 40}) {
    for (std::size_t NUM_CONSUMERS : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 40}) {
      for (std::size_t BUFFER_SIZE : {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1'024, 2'000, 5'000, 10'000, 100'000, 1'000'000}) {
        run_test(spef_file, BUFFER_SIZE, NUM_PRODUCERS, NUM_CONSUMERS);
      }
    }
  }
}

void test4(fs::path const &spef_file) {
  run_test(spef_file, 2, 2, 3);
}

int main(int argc, char const *const *argv) {
  if (argc != 2) {
    fmt::print(std::cerr, "usage: {} <file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  fs::path const spef_file{argv[1]};

  for (int i = 0; i < 20; ++i) {
    test1(spef_file);
    test2(spef_file);
    test3(spef_file);
  }

  return EXIT_SUCCESS;
}

