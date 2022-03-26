#include "00-hello_world.hpp"

#include <iostream>
#include <tao/pegtl.hpp>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return 1;
  }

  // outer try/catch for normal exceptions that might occur for example if the
  // file is not found
  try {
    pegtl::file_input infile(argv[1]);

    // inner try/catch for the parser exceptions
    try {
      pegtl::parse<grammar>(infile);
    } catch (pegtl::parse_error const &e) {
      // this catch block needs access to the input
      auto const &p = e.positions().front();
      std::cerr << e.what() << '\n'
                << infile.line_at(p) << '\n'
                << std::setw((int)p.column) << '^' << std::endl;
    }

  } catch (std::exception const &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
