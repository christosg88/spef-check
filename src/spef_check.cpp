#include "spef_actions.hpp"
#include "spef_random.hpp"
#include "spef_structs.hpp"
#include "spef_write.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <tao/pegtl/contrib/analyze.hpp>
//#include <tao/pegtl/contrib/trace.hpp>

namespace pegtl = tao::pegtl;
namespace fs = std::filesystem;

int main(int argc, char const *const *argv) {
  if (pegtl::analyze<spef_grammar>() != 0) {
    std::cerr << "cycles without progress detected!\n";
    return 1;
  }

  if (argc == 1 || std::strcmp(argv[1], "-h") == 0 ||
      std::strcmp(argv[1], "--help") == 0) {
    std::cerr << "Usage: " << argv[0] << " "
              << " <filename>.spef\n";
    return 1;
  }

  if (argc == 3 && std::strcmp(argv[1], "--random") == 0) {
    std::size_t num_random{};
    std::string_view num_random_sv{argv[2]};
    auto const [_, ec] =
        std::from_chars(num_random_sv.begin(), num_random_sv.end(), num_random);
    handle_from_chars(ec, num_random_sv);
    for (std::size_t i = 0; i < num_random; ++i) {
      RandomSPEFGenerator rsg(fmt::format("Random {}", i), 10, 10, 100);
      SPEF spef = rsg.generate();

      fs::path const outfile_p{fmt::format("random/random_{:02d}.spef", i)};
      fs::create_directories(outfile_p.parent_path());
      {
        std::ofstream outfile(outfile_p);
        outfile << spef;
      }
      //{
      //  SPEF spef;
      //  SPEFHelper spef_h;
      //  pegtl::read_input input{outfile_p};
      //  bool success = pegtl::parse<pegtl::must<spef_grammar>, spef_action>(
      //      input,
      //      spef,
      //      spef_h);

      //  if (!success) {
      //    throw std::runtime_error(
      //        fmt::format("Failed to parse {}", outfile_p.c_str()));
      //  }

      //  fs::path const outfile2_p{fmt::format("gen/random_{:02d}.spef", i)};
      //  fs::create_directories(outfile2_p.parent_path());
      //  std::ofstream outfile(outfile2_p);
      //  outfile << spef;
      //}
    }

    return 0;
  }

  std::filesystem::path const spef_file{argv[1]};

  bool success = false;

  // outer try/catch for normal exceptions that might occur for example if the
  // file is not found
  try {
    pegtl::read_input input{spef_file};

    // inner try/catch for the parser exceptions
    try {
      //pegtl::tracer<pegtl::tracer_traits<>> tracer(input);
      //tracer.parse<spef_grammar>(input);
      SPEF spef;
      SPEFHelper spef_h;
      success = pegtl::parse<pegtl::must<spef_grammar>, spef_action>(
          input,
          spef,
          spef_h);

      std::cout << spef;
    } catch (pegtl::parse_error &err) {
      std::cerr << "ERROR: An exception occurred during parsing:\n";
      // this catch block needs access to the input
      auto const &pos = err.positions().front();
      std::cerr << err.what() << '\n'
                << input.line_at(pos) << '\n'
                << std::setw((int)pos.column) << '^' << std::endl;
      std::cerr << err.what() << '\n';
    }
  } catch (std::exception const &e) {
    std::cerr << e.what() << std::endl;
  }

  if (!success) {
    std::cerr << "Parsing failed\n";
    return 2;
  }

  return 0;
}
