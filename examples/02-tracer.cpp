#include "00-hello_world.hpp"

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/trace.hpp>

namespace pegtl = tao::pegtl;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return 1;
  }

  pegtl::argv_input argv_in(argv, 1);
  pegtl::standard_trace<grammar>(argv_in);

  return 0;
}
