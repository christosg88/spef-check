#include "00-hello_world.hpp"

#include <iostream>
#include <tao/pegtl.hpp>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return 1;
  }

  std::string name;
  pegtl::argv_input argv_in(argv, 1);
  pegtl::parse<grammar, action>(argv_in, name);

  std::cout << "Goodbye, " << name << "!\n";
  return 0;
}
