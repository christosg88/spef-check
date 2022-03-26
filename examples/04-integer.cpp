#include <tao/pegtl.hpp>

namespace pegtl = tao::pegtl;

struct integer : pegtl::seq<pegtl::opt<pegtl::one<'+', '-'>>, // ('+'/'-')?
                            pegtl::plus<pegtl::digit>         // digit+
                            > {};
