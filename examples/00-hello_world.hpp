#ifndef HELLO_WORLD_HPP
#define HELLO_WORLD_HPP

#include <tao/pegtl.hpp>

namespace pegtl = tao::pegtl;

// parsing rule that matches the literal "Hello, "
struct prefix : pegtl::string<'H', 'e', 'l', 'l', 'o', ',', ' '> {};

// parsing rule that matches a non-empty sequence of alpabetic characters with
// greedy-matching
struct name : pegtl::plus<pegtl::alpha> {};

// the whole grammar to read "Hello, name!"
struct grammar : pegtl::must<prefix, name, pegtl::one<'!'>, pegtl::eolf> {};

// class template for user-defined actions, that does nothing by default
template <typename Rule> struct action {};

// specialiazation of the user-defined action class, to do something when the
// "name" rule succeeds; it's called with the portion of the input that matched
// the rule
template <> struct action<name> {
  template <typename ParseInput>
  static void apply(ParseInput const &in, std::string &v) {
    v = in.string();
  }
};

#endif // HELLO_WORLD_HPP
