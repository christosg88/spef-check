#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include "file_reader.hpp"
#include <BS_thread_pool.hpp>
#include <dbg_out.h>

std::mutex dbg_out_mtx;

//namespace pegtl = tao::pegtl;
namespace fs = std::filesystem;


void consume(std::size_t id, std::atomic<std::size_t> &idx, MTFileReader &reader) {
  while (true) {
    auto const c_idx = idx.fetch_add(1);
    auto const &chunk = reader.get_chunk(c_idx);
    if (chunk.empty()) {
      break;
    }
    dbg_out(fmt::format("Writing chunk {} from consumer {}", c_idx, id));
    std::ofstream ofs(fmt::format("/SCRATCH/gkan/out/c{:05}_w{:02}.txt", c_idx, id));
    ofs.write(chunk.data(), chunk.size());
    ofs.close();
    reader.mark_chunk(c_idx);
  }
}

void run_test(
    fs::path const &spef_file,
    std::size_t BUFFER_SIZE,
    std::size_t NUM_PRODUCERS,
    std::size_t NUM_CONSUMERS) {
  fmt::print(
      "== BUFFER_SIZE:{} NUM_PRODUCERS:{} NUM_CONSUMERS:{}\n",
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

int main(int argc, char const *const *argv) {
  if (argc != 2) {
    fmt::print(std::cerr, "usage: {} <file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  fs::path const spef_file{argv[1]};
  //run_test(spef_file, 1'000'000, 2, 2);
  //run_test(spef_file, 2000, 2, 7);
  //return EXIT_SUCCESS;
  run_test(spef_file, 4'000'000, 2, 2);
  return EXIT_SUCCESS;

  for (std::size_t NUM_PRODUCERS : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 40}) {
    for (std::size_t NUM_CONSUMERS : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 40}) {
      for (std::size_t BUFFER_SIZE : {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1'024, 2'000, 5'000, 10'000, 100'000, 1'000'000}) {
        run_test(spef_file, BUFFER_SIZE, NUM_PRODUCERS, NUM_CONSUMERS);
      }
    }
  }

  return EXIT_SUCCESS;
}
