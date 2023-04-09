#include <iostream>
#include <filesystem>

#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/trace.hpp>

#include "spef_structs.hpp"
#include "spef_actions.hpp"

namespace pegtl = tao::pegtl;

int main(int argc, char const * const * argv) {
  if (pegtl::analyze<spef_grammar>() != 0) {
    std::cerr << "cycles without progress detected!\n";
    return 1;
  }

  if (argc != 2 || std::strcmp(argv[1], "-h") == 0 ||
      std::strcmp(argv[1], "--help") == 0) {
    std::cerr << "Usage: " << argv[0] << " " << " <filename>.spef\n";
    return 1;
  }

  std::filesystem::path const spef_file{argv[1]};

  bool success = false;

  // outer try/catch for normal exceptions that might occur for example if the
  // file is not found
  try {
    pegtl::read_input input{spef_file};

    // inner try/catch for the parser exceptions
    try {
      //pegtl::tracer<pegtl::tracer_traits<true, true, true, 4>> tracer(input);
      //tracer.parse<spef_grammar>(input);
      SPEF spef;
      success = pegtl::parse<pegtl::must<spef_grammar>, spef_action>(input, spef);

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
