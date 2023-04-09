#ifndef SPEF_ACTIONS_HPP
#define SPEF_ACTIONS_HPP

#include <charconv>

#include <tao/pegtl.hpp>

#include "spef_structs.hpp"

struct scaled_double {
  double base = 0.0;
  double scale = 1.0;
};

struct SPEF {
  std::string m_version;
  std::string m_design_name;
  std::string m_date;
  std::string m_vendor;
  std::string m_program_name;
  std::string m_program_version;
  std::string m_design_flow;
  char m_hierarchy_div_def = '\0';
  char m_pin_delim_def = '\0';
  char m_prefix_bus_delim = '\0';
  char m_suffix_bus_delim = '\0';
  scaled_double m_time_scale;
  scaled_double m_cap_scale;
  scaled_double m_res_scale;
  scaled_double m_induct_scale;
};

std::ostream &operator<<(std::ostream &os, SPEF const &spef);

template <typename Rule>
struct spef_action : tao::pegtl::nothing<Rule> {};

constexpr std::string_view whitespace = " \t";

constexpr std::string_view skip_label_and_space(std::string_view match, size_t skip) {
  match.remove_prefix(match.find_first_not_of(whitespace, skip));
  return match;
}

constexpr std::string_view rtrim(std::string_view match) {
  return std::string_view(match.begin(), match.find_last_not_of(whitespace));
}

constexpr std::string_view trim(std::string_view match) {
  match.remove_prefix(match.find_first_not_of(whitespace));
  return match;
}

template <>
struct spef_action<spef_version> {
  template <typename Action>
  static void apply(Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*SPEF");
    spef.m_version = rtrim(skip_label_and_space(input.string_view(), skip));
  }
};

template <>
struct spef_action<spef_design_name> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*DESIGN");
    spef.m_design_name = rtrim(skip_label_and_space(input.string_view(), skip));
  }
};

template <>
struct spef_action<spef_date> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*DATE");
    spef.m_date = rtrim(skip_label_and_space(input.string_view(), skip));
  }
};

template <>
struct spef_action<spef_vendor> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*VENDOR");
    spef.m_vendor = rtrim(skip_label_and_space(input.string_view(), skip));
  }
};

template <>
struct spef_action<spef_program_name> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*PROGRAM");
    spef.m_program_name = rtrim(skip_label_and_space(input.string_view(), skip));
  }
};

template <>
struct spef_action<spef_program_version> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*VERSION");
    spef.m_program_version = rtrim(skip_label_and_space(input.string_view(), skip));
  }
};

template <>
struct spef_action<spef_design_flow> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*DESIGN_FLOW");
    spef.m_design_flow = rtrim(skip_label_and_space(input.string_view(), skip));
  }
};

template <>
struct spef_action<spef_hierarchy_div_def> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*DIVIDER");
    spef.m_hierarchy_div_def = skip_label_and_space(input.string_view(), skip).front();
  }
};

template <>
struct spef_action<spef_pin_delim_def> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*DELIMITER");
    spef.m_pin_delim_def = skip_label_and_space(input.string_view(), skip).front();
  }
};

template <>
struct spef_action<spef_bus_delim_def> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*BUS_DELIMITER");
    std::string_view bus_delim_pair = rtrim(skip_label_and_space(input.string_view(), skip));
    spef.m_prefix_bus_delim = bus_delim_pair.front();
    spef.m_suffix_bus_delim = bus_delim_pair.back();
  }
};

template <>
struct spef_action<spef_time_scale> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*T_UNIT");
    std::string_view t_unit_pair = rtrim(skip_label_and_space(input.string_view(), skip));
    auto [ptr, ec] {std::from_chars(t_unit_pair.begin(), t_unit_pair.end(), spef.m_time_scale.base)};
    t_unit_pair.remove_prefix(std::distance(t_unit_pair.begin(), ptr));
    std::string_view scale = trim(t_unit_pair);
    if (scale == "NS") {
      spef.m_time_scale.scale = 1e-9;
    } else if (scale == "PS") {
      spef.m_time_scale.scale = 1e-12;
    }
  }
};

template <>
struct spef_action<spef_cap_scale> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*C_UNIT");
    std::string_view c_unit_pair = rtrim(skip_label_and_space(input.string_view(), skip));
    auto [ptr, ec] {std::from_chars(c_unit_pair.begin(), c_unit_pair.end(), spef.m_cap_scale.base)};
    c_unit_pair.remove_prefix(std::distance(c_unit_pair.begin(), ptr));
    std::string_view scale = trim(c_unit_pair);
    if (scale == "PF") {
      spef.m_cap_scale.scale = 1e-12;
    } else if (scale == "FF") {
      spef.m_cap_scale.scale = 1e-15;
    }
  }
};

template <>
struct spef_action<spef_res_scale> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*R_UNIT");
    std::string_view r_unit_pair = rtrim(skip_label_and_space(input.string_view(), skip));
    auto [ptr, ec] {std::from_chars(r_unit_pair.begin(), r_unit_pair.end(), spef.m_res_scale.base)};
    r_unit_pair.remove_prefix(std::distance(r_unit_pair.begin(), ptr));
    std::string_view scale = trim(r_unit_pair);
    if (scale == "KOHM") {
      spef.m_res_scale.scale = 1e3;
    }
  }
};

template <>
struct spef_action<spef_induct_scale> {
  template <typename Action>
  static void apply([[maybe_unused]] Action const &input, SPEF &spef) {
    static constexpr size_t skip = sizeof("*L_UNIT");
    std::string_view l_unit_pair = rtrim(skip_label_and_space(input.string_view(), skip));
    auto [ptr, ec] {std::from_chars(l_unit_pair.begin(), l_unit_pair.end(), spef.m_induct_scale.base)};
    l_unit_pair.remove_prefix(std::distance(l_unit_pair.begin(), ptr));
    std::string_view scale = trim(l_unit_pair);
    if (scale == "MH") {
      spef.m_induct_scale.scale = 1e-3;
    } else if (scale == "UH") {
      spef.m_induct_scale.scale = 1e-6;
    }
  }
};

#endif  // SPEF_ACTIONS_HPP
