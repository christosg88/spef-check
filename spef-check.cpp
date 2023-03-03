#include <iostream>
#include <filesystem>

#include <tao/pegtl.hpp>

namespace pegtl = tao::pegtl;

// numbers
struct spef_sign : pegtl::one<'+', '-'> {};
struct spef_integer : pegtl::seq<pegtl::opt<spef_sign>, pegtl::plus<pegtl::digit>> {};
struct spef_decimal : pegtl::seq<pegtl::opt<spef_sign>, pegtl::plus<pegtl::digit>, pegtl::one<'.'>, pegtl::star<pegtl::digit>> {};
struct spef_fraction : pegtl::seq<pegtl::opt<spef_sign>, pegtl::one<'.'>, pegtl::star<pegtl::digit>> {};
struct spef_radix : pegtl::sor<spef_integer, spef_decimal, spef_fraction> {};
struct spef_exp_char : pegtl::one<'E', 'e'> {};
struct spef_exp : pegtl::seq<spef_radix, spef_exp_char, spef_integer> {};
struct spef_float : pegtl::sor<spef_decimal, spef_fraction, spef_exp> {};
struct spef_number : pegtl::sor<spef_integer, float> {};
struct spef_pos_integer : pegtl::plus<pegtl::digit> {};
struct spef_pos_decimal : pegtl::seq<pegtl::plus<pegtl::digit>, pegtl::one<'.'>, pegtl::opt<pegtl::plus<pegtl::digit>>> {};
struct spef_pos_fraction : pegtl::seq<pegtl::one<'.'>, pegtl::plus<pegtl::digit>> {};
struct spef_pos_radix : pegtl::sor<spef_pos_integer, spef_pos_decimal, spef_pos_fraction> {};
struct spef_pos_exp : pegtl::seq<spef_pos_radix, spef_exp_char, spef_integer>{};
struct spef_pos_float : pegtl::sor<spef_pos_decimal, spef_pos_fraction, spef_pos_exp> {};
struct spef_pos_number : pegtl::sor<spef_pos_integer, spef_pos_float> {};

// characters
struct special_char : pegtl::one<'!', '#', '$', '%', '\'', '(', ')', '*', '+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[', '\\', ']', '^', '`', '{', '|', '}', '~'> {};
struct hier_delim : pegtl::one<'.', '/', ':', '|'> {};
struct prefix_bus_delim : pegtl::one<'[', '{', '(', '<', ':', '.'> {};
struct suffix_bus_delim : pegtl::one<']', '}', ')', '>'> {};
struct qstring_char : pegtl::sor<special_char, pegtl::alpha, pegtl::digit, pegtl::blank, pegtl::one<'_'>> {};
struct qstring : pegtl::seq<pegtl::one<'"'>, pegtl::star<qstring_char>, pegtl::one<'"'>> {};

// unit_def
struct time_unit : pegtl::sor<TAO_PEGTL_STRING("NS"), TAO_PEGTL_STRING("PS")> {};
struct cap_unit : pegtl::sor<TAO_PEGTL_STRING("PF"), TAO_PEGTL_STRING("FF")> {};
struct res_unit : pegtl::sor<TAO_PEGTL_STRING("OHM"), TAO_PEGTL_STRING("KOHM")> {};
struct induct_unit : pegtl::sor<TAO_PEGTL_STRING("HENRY"), TAO_PEGTL_STRING("MH"), TAO_PEGTL_STRING("UH")> {};
struct time_scale : pegtl::seq<TAO_PEGTL_STRING("*T_UNIT"), pegtl::plus<pegtl::blank>, spef_pos_number, pegtl::plus<pegtl::blank>, time_unit, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct cap_scale : pegtl::seq<TAO_PEGTL_STRING("*C_UNIT"), pegtl::plus<pegtl::blank>, spef_pos_number, pegtl::plus<pegtl::blank>, cap_unit, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct res_scale : pegtl::seq<TAO_PEGTL_STRING("*R_UNIT"), pegtl::plus<pegtl::blank>, spef_pos_number, pegtl::plus<pegtl::blank>, res_unit, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct induct_scale : pegtl::seq<TAO_PEGTL_STRING("*L_UNIT"), pegtl::plus<pegtl::blank>, spef_pos_number, pegtl::plus<pegtl::blank>, induct_unit, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct unit_def : pegtl::plus<pegtl::sor<time_scale, cap_scale, res_scale, induct_scale>> {};

// header_def
struct SPEF_version : pegtl::seq<TAO_PEGTL_STRING("*SPEF"), pegtl::plus<pegtl::blank>, qstring, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct design_name : pegtl::seq<TAO_PEGTL_STRING("*DESIGN"), pegtl::plus<pegtl::blank>, qstring, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct date : pegtl::seq<TAO_PEGTL_STRING("DATE"), pegtl::plus<pegtl::blank>, qstring, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct vendor : pegtl::seq<TAO_PEGTL_STRING("*VENDOR"), pegtl::plus<pegtl::blank>, qstring, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct program_name : pegtl::seq<TAO_PEGTL_STRING("*PROGRAM"), pegtl::plus<pegtl::blank>, qstring, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct program_version : pegtl::seq<TAO_PEGTL_STRING("*VERSION"), pegtl::plus<pegtl::blank>, qstring, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct design_flow : pegtl::seq<TAO_PEGTL_STRING("*DESIGN_FLOW"), pegtl::plus< pegtl::seq< pegtl::plus<pegtl::blank>, qstring>>, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct hierarchy_div_def : pegtl::seq<TAO_PEGTL_STRING("*DIVIDER"), pegtl::plus<pegtl::blank>, hier_delim, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct pin_delim_def : pegtl::seq<TAO_PEGTL_STRING("*DELIMITER"), pegtl::plus<pegtl::blank>, hier_delim, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct bus_delim_def : pegtl::seq<TAO_PEGTL_STRING("*BUS_DELIMITER"), pegtl::plus<pegtl::blank>, prefix_bus_delim, pegtl::opt<pegtl::plus<pegtl::blank>, suffix_bus_delim>, pegtl::star<pegtl::blank>, pegtl::eolf> {};
struct header_def : pegtl::plus<pegtl::sor<SPEF_version, design_name, date, vendor, program_name, program_version, design_flow, hierarchy_div_def, pin_delim_def, bus_delim_def, unit_def>> {};

template <typename Rule> struct print_action : pegtl::nothing<Rule> {};

template <> struct print_action<header_def> {
  template <typename ActionInput> static void apply(ActionInput const &input) {
    std::cout << "header_def\n" << input.string() << "\n";
  }
};

int main(int argc, char const * const * argv) {
  if (argc != 2
      || std::strcmp(argv[1], "-h") == 0
      || std::strcmp(argv[1], "--help") == 0) {
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
      success = pegtl::parse<header_def, print_action>(input);
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
