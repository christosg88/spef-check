#include <iostream>
#include <cctype>

#include "spef_actions.hpp"

std::ostream &operator<<(std::ostream &os, SPEF const &spef) {
  if (!spef.m_version.empty()) {
    os << "*SPEF " << spef.m_version << '\n';
  }
  if (!spef.m_design_name.empty()) {
    os << "*DESIGN " << spef.m_design_name << '\n';
  }
  if (!spef.m_date.empty()) {
    os << "*DATE " << spef.m_date << '\n';
  }
  if (!spef.m_vendor.empty()) {
    os << "*VENDOR " << spef.m_vendor << '\n';
  }
  if (!spef.m_program_name.empty()) {
    os << "*PROGRAM " << spef.m_program_name << '\n';
  }
  if (!spef.m_program_version.empty()) {
    os << "*VERSION " << spef.m_program_version << '\n';
  }
  if (!spef.m_design_flow.empty()) {
    os << "*DESIGN_FLOW " << spef.m_design_flow << '\n';
  }
  if (spef.m_hierarchy_div_def != '\0') {
    os << "*DIVIDER " << spef.m_hierarchy_div_def << '\n';
  }
  if (spef.m_pin_delim_def != '\0') {
    os << "*DELIMITER " << spef.m_pin_delim_def << '\n';
  }
  if (spef.m_prefix_bus_delim != '\0') {
    os << "*BUS_DELIMITER " << spef.m_prefix_bus_delim;
    if (spef.m_suffix_bus_delim != '\0') {
      os << ' ' << spef.m_suffix_bus_delim;
    }
    os << '\n';
  }

  os << "*T_UNIT " << spef.m_time_scale.base;
  if (spef.m_time_scale.scale == 1e-9) {
    os << " NS\n";
  } else if (spef.m_time_scale.scale == 1e-12) {
    os << " PS\n";
  }
  os << "*C_UNIT " << spef.m_cap_scale.base;
  if (spef.m_cap_scale.scale == 1e-12) {
    os << " PF\n";
  } else if (spef.m_cap_scale.scale == 1e-15) {
    os << " FF\n";
  }
  os << "*R_UNIT " << spef.m_res_scale.base;
  if (spef.m_res_scale.scale == 1.0) {
    os << " OHM\n";
  } else if (spef.m_res_scale.scale == 1e3) {
    os << " KOHM\n";
  }
  os << "*L_UNIT " << spef.m_induct_scale.base;
  if (spef.m_induct_scale.scale == 1.0) {
    os << " HENRY\n";
  } else if (spef.m_induct_scale.scale == 1e-3) {
    os << " MH\n";
  } else if (spef.m_induct_scale.scale == 1e-6) {
    os << " UH\n";
  }
  os << '\n';

  return os;
}

