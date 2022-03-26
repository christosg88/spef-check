#include "00-hello_world.hpp"

#include <iostream>
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>

namespace pegtl = tao::pegtl;

int main() {
  if (pegtl::analyze<grammar>() != 0) {
    std::cerr << "cycles without progress detecteed!\n";
    return 1;
  }
  return 0;
}
