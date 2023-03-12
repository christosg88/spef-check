#ifndef PRINT_ACTION_HPP
#define PRINT_ACTION_HPP

#include <iostream>

#include <tao/pegtl.hpp>

#include "spef_structs.hpp"

namespace pegtl = tao::pegtl;

template <typename Rule> struct print_action : pegtl::nothing<Rule> {};
template <> struct print_action<spef_cap_scale> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_cap_scale\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_res_scale> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_res_scale\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_induct_scale> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_induct_scale\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_unit_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_unit_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_version> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_version\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_design_name> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_design_name\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_date> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_date\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_vendor> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_vendor\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_program_version> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_program_version\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_design_flow> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_design_flow\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_hierarchy_div_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_hierarchy_div_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_pin_delim_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_pin_delim_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_bus_delim_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_bus_delim_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_header_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_header_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_pnet_ref> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_pnet_ref\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_net_name> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_net_name\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_power_net_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_power_net_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_ground_net_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_ground_net_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_power_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_power_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_name_map> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_name_map\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_grammar> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_grammar\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_port_entry> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_port_entry\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_pport_entry> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_pport_entry\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_port_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_port_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_physical_port_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_physical_port_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_external_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_external_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_define_entry> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_define_entry\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_conn_sec> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_conn_sec\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_d_net> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_d_net\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_internal_def> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_internal_def\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_net_ref> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_net_ref\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_index> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_index\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_path> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_path\n" << "\n" << input.string() << "\n"; } };
template <> struct print_action<spef_physical_ref> { template <typename ActionInput> static void apply(ActionInput const &input) { std::cout << "[+] spef_physical_ref\n" << "\n" << input.string() << "\n"; } };
#endif
