#ifndef SPEF_STRUCTS_HPP
#define SPEF_STRUCTS_HPP

#include <tao/pegtl.hpp>

// GRAMMAR STRUCTS
// clang-format off
namespace pegtl = tao::pegtl;

// helpers
struct sep : pegtl::plus<pegtl::sor<pegtl::blank, pegtl::eol>> {};

// numbers
struct spef_sign : pegtl::one<'+', '-'> {};
struct spef_integer : pegtl::seq<pegtl::opt<spef_sign>, pegtl::plus<pegtl::digit>> {};
struct spef_decimal : pegtl::seq<pegtl::opt<spef_sign>, pegtl::plus<pegtl::digit>, pegtl::one<'.'>, pegtl::star<pegtl::digit>> {};
struct spef_fraction : pegtl::seq<pegtl::opt<spef_sign>, pegtl::one<'.'>, pegtl::star<pegtl::digit>> {};
struct spef_radix : pegtl::sor<spef_integer, spef_decimal, spef_fraction> {};
struct spef_exp_char : pegtl::one<'E', 'e'> {};
struct spef_exp : pegtl::seq<spef_radix, spef_exp_char, spef_integer> {};
struct spef_float : pegtl::sor<spef_decimal, spef_fraction, spef_exp> {};
struct spef_number : pegtl::sor<spef_float, spef_integer> {};
struct spef_pos_integer : pegtl::plus<pegtl::digit> {};  // must not consume trailing whitespace
struct spef_pos_decimal : pegtl::seq<pegtl::plus<pegtl::digit>, pegtl::one<'.'>, pegtl::opt<pegtl::plus<pegtl::digit>>> {};
struct spef_pos_fraction : pegtl::seq<pegtl::one<'.'>, pegtl::plus<pegtl::digit>> {};
struct spef_pos_radix : pegtl::sor<spef_pos_integer, spef_pos_decimal, spef_pos_fraction> {};
struct spef_pos_exp : pegtl::seq<spef_pos_radix, spef_exp_char, spef_integer>{};
struct spef_pos_float : pegtl::sor<spef_pos_decimal, spef_pos_fraction, spef_pos_exp> {};
struct spef_pos_number : pegtl::sor<spef_pos_integer, spef_pos_float> {};

// characters
struct spef_hchar : pegtl::one<'.', '/', ':', '|'> {};
struct spef_hier_delim : spef_hchar {};
struct spef_pin_delim : spef_hchar {};  // must not consume trailing whitespace
struct spef_special_char : pegtl::one<'!', '#', '$', '%', '\'', '(', ')', '*', '+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[', '\\', ']', '^', '`', '{', '|', '}', '~'> {};
struct spef_escaped_char_set : pegtl::sor<spef_special_char, pegtl::one<'"'>> {};
struct spef_escaped_char : pegtl::seq<pegtl::one<'\\'>, spef_escaped_char_set> {};
struct spef_prefix_bus_delim : pegtl::one<'[', '{', '(', '<', ':', '.'> {};
struct spef_suffix_bus_delim : pegtl::one<']', '}', ')', '>'> {};
struct spef_identifier_char : pegtl::sor<spef_escaped_char, pegtl::alpha, pegtl::digit, pegtl::one<'_'>, spef_pin_delim, spef_hier_delim, spef_prefix_bus_delim, spef_suffix_bus_delim> {};
struct spef_identifier : pegtl::plus<spef_identifier_char> {};
struct spef_bit_identifier : pegtl::sor<spef_identifier, pegtl::seq<spef_identifier, spef_prefix_bus_delim, spef_pos_integer, pegtl::opt<spef_suffix_bus_delim>>> {};
struct spef_partial_path : pegtl::seq<spef_identifier, spef_hier_delim> {};
struct spef_path : pegtl::sor<pegtl::seq<pegtl::opt<spef_hier_delim>, spef_bit_identifier>, pegtl::seq<pegtl::opt<spef_hier_delim>, pegtl::plus<spef_partial_path>, spef_bit_identifier>> {};
struct spef_qstring_char : pegtl::sor<spef_special_char, pegtl::alpha, pegtl::digit, pegtl::blank, pegtl::one<'_'>> {};
struct spef_qstring : pegtl::seq<pegtl::one<'"'>, pegtl::star<spef_qstring_char>, pegtl::one<'"'>> {};
struct spef_name : pegtl::sor<spef_qstring, spef_identifier> {};
struct spef_partial_physical_ref : pegtl::seq<spef_hier_delim, spef_name> {};
struct spef_physical_ref : pegtl::seq<spef_name, pegtl::plus<spef_partial_physical_ref>> {};

// unit_def
struct spef_time_unit : pegtl::sor<TAO_PEGTL_STRING("NS"), TAO_PEGTL_STRING("PS")> {};
struct spef_cap_unit : pegtl::sor<TAO_PEGTL_STRING("PF"), TAO_PEGTL_STRING("FF")> {};
struct spef_res_unit : pegtl::sor<TAO_PEGTL_STRING("OHM"), TAO_PEGTL_STRING("KOHM")> {};
struct spef_induct_unit : pegtl::sor<TAO_PEGTL_STRING("HENRY"), TAO_PEGTL_STRING("MH"), TAO_PEGTL_STRING("UH")> {};
struct spef_time_scale : pegtl::seq<TAO_PEGTL_STRING("*T_UNIT"), sep, pegtl::must<spef_pos_number, sep, spef_time_unit, sep>> {};
struct spef_cap_scale : pegtl::seq<TAO_PEGTL_STRING("*C_UNIT"), sep, pegtl::must<spef_pos_number, sep, spef_cap_unit, sep>> {};
struct spef_res_scale : pegtl::seq<TAO_PEGTL_STRING("*R_UNIT"), sep, pegtl::must<spef_pos_number, sep, spef_res_unit, sep>> {};
struct spef_induct_scale : pegtl::seq<TAO_PEGTL_STRING("*L_UNIT"), sep, pegtl::must<spef_pos_number, sep, spef_induct_unit, sep>> {};
struct spef_unit_def : pegtl::plus<pegtl::sor<spef_time_scale, spef_cap_scale, spef_res_scale, spef_induct_scale>> {};

// header_def
struct spef_version : pegtl::seq<TAO_PEGTL_STRING("*SPEF"), sep, pegtl::must<spef_qstring, sep>> {};
struct spef_design_name : pegtl::seq<TAO_PEGTL_STRING("*DESIGN"), sep, pegtl::must<spef_qstring, sep>> {};
struct spef_date : pegtl::seq<TAO_PEGTL_STRING("*DATE"), sep, pegtl::must<spef_qstring, sep>> {};
struct spef_vendor : pegtl::seq<TAO_PEGTL_STRING("*VENDOR"), sep, pegtl::must<spef_qstring, sep>> {};
struct spef_program_name : pegtl::seq<TAO_PEGTL_STRING("*PROGRAM"), sep, pegtl::must<spef_qstring, sep>> {};
struct spef_program_version : pegtl::seq<TAO_PEGTL_STRING("*VERSION"), sep, pegtl::must<spef_qstring, sep>> {};
struct spef_design_flow : pegtl::seq<TAO_PEGTL_STRING("*DESIGN_FLOW"), sep, pegtl::must<pegtl::plus<spef_qstring, sep>>> {};
struct spef_hierarchy_div_def : pegtl::seq<TAO_PEGTL_STRING("*DIVIDER"), sep, pegtl::must<spef_hier_delim, sep>> {};
struct spef_pin_delim_def : pegtl::seq<TAO_PEGTL_STRING("*DELIMITER"), sep, pegtl::must<spef_pin_delim, sep>> {};
struct spef_bus_delim_def : pegtl::seq<TAO_PEGTL_STRING("*BUS_DELIMITER"), sep, pegtl::must<spef_prefix_bus_delim, sep, pegtl::opt<spef_suffix_bus_delim, sep>>> {};
struct spef_header_def : pegtl::plus<pegtl::sor<spef_version, spef_design_name, spef_date, spef_vendor, spef_program_name, spef_program_version, spef_design_flow, spef_hierarchy_div_def, spef_pin_delim_def, spef_bus_delim_def, spef_unit_def>> {};

// name_map
struct spef_mapped_item : pegtl::sor<spef_identifier, spef_bit_identifier, spef_path, spef_name, spef_physical_ref> {};
struct spef_index : pegtl::seq<pegtl::one<'*'>, spef_pos_integer> {};  // must not consume trailing whitespace
struct spef_name_map_entry : pegtl::seq<spef_index, sep, spef_mapped_item> {};
struct spef_name_map : pegtl::seq<TAO_PEGTL_STRING("*NAME_MAP"), sep, pegtl::must<pegtl::plus<spef_name_map_entry, sep>>> {};

// power_def
struct spef_net_ref : pegtl::sor<spef_index, spef_path> {};  // must not consume trailing whitespace
struct spef_pnet_ref : pegtl::sor<spef_index, spef_physical_ref> {};  // must not consume trailing whitespace
struct spef_net_name : pegtl::sor<spef_net_ref, spef_pnet_ref> {};
struct spef_power_net_def : pegtl::seq<TAO_PEGTL_STRING("*POWER_NETS"), sep, pegtl::must<pegtl::plus<spef_net_name, sep>>> {};
struct spef_ground_net_def : pegtl::seq<TAO_PEGTL_STRING("*GROUND_NETS"), sep, pegtl::must<pegtl::plus<spef_net_name, sep>>> {};
struct spef_power_def : pegtl::plus<pegtl::sor<spef_power_net_def, spef_ground_net_def>> {};

// conn_attr
#ifdef STRICT
struct spef_par_value : pegtl::sor<pegtl::seq<spef_float, sep>, pegtl::seq<spef_float, pegtl::one<':'>, spef_float, pegtl::one<':'>, spef_float, sep>> {};
#else
struct spef_par_value : pegtl::sor<pegtl::seq<spef_number, sep>, pegtl::seq<spef_number, pegtl::one<':'>, spef_number, pegtl::one<':'>, spef_number, sep>> {};
#endif
struct spef_coordinates : pegtl::seq<TAO_PEGTL_STRING("*C"), sep, pegtl::must<spef_number, sep, spef_number, sep>> {};
struct spef_cap_load : pegtl::seq<TAO_PEGTL_STRING("*L"), sep, pegtl::must<spef_par_value>> {};
struct spef_threshold : pegtl::sor<spef_pos_fraction, pegtl::seq<spef_pos_fraction>, pegtl::one<':'>, spef_pos_fraction, pegtl::one<':'>, spef_pos_fraction> {};
struct spef_slews : pegtl::seq<TAO_PEGTL_STRING("*S"), sep, pegtl::must<spef_par_value, spef_par_value, pegtl::opt<spef_threshold, sep, spef_threshold, sep>>> {};
struct spef_cell_type : pegtl::sor<spef_index, spef_name> {};
struct spef_driving_cell : pegtl::seq<TAO_PEGTL_STRING("*D"), sep, pegtl::must<spef_cell_type, sep>> {};
struct spef_conn_attr : pegtl::sor<spef_coordinates, spef_cap_load, spef_slews, spef_driving_cell> {}; // consumes trailing space

// external_def
struct spef_direction : pegtl::one<'I', 'B', 'O'> {};
struct spef_inst_name : pegtl::sor<spef_index, spef_path> {};
struct spef_physical_inst : pegtl::sor<spef_index, spef_physical_ref> {};
struct spef_port : pegtl::sor<spef_index, spef_bit_identifier> {};
struct spef_pport : pegtl::sor<spef_index, spef_name> {};
struct spef_port_name : pegtl::seq<pegtl::opt<pegtl::seq<spef_inst_name, spef_pin_delim>>, spef_port> {};
struct spef_pport_name : pegtl::seq<pegtl::opt<pegtl::seq<spef_physical_inst, spef_pin_delim>>, spef_pport> {};
struct spef_port_entry : pegtl::seq<spef_port_name, sep, spef_direction, sep, pegtl::star<spef_conn_attr>> {};
struct spef_pport_entry : pegtl::seq<spef_pport_name, sep, spef_direction, sep, pegtl::star<spef_conn_attr>> {};
struct spef_port_def : pegtl::seq<TAO_PEGTL_STRING("*PORTS"), sep, pegtl::must<pegtl::plus<spef_port_entry>>> {};
struct spef_physical_port_def : pegtl::seq<TAO_PEGTL_STRING("*PHYSICAL_PORTS"), sep, pegtl::must<pegtl::plus<spef_pport_entry>>> {};
struct spef_external_def : pegtl::plus<pegtl::sor<spef_port_def, spef_physical_port_def>> {};

// define_def
struct spef_entity : spef_qstring {};
struct spef_define_entry : pegtl::sor<pegtl::seq<TAO_PEGTL_STRING("*DEFINE"), sep, pegtl::must<pegtl::plus<spef_inst_name, sep>, spef_entity>>, pegtl::seq<TAO_PEGTL_STRING("*PDEFINE"), sep, pegtl::must<spef_physical_inst, sep, spef_entity>>> {};
struct spef_define_def : pegtl::plus<spef_define_entry, sep> {};

// variation_def
struct spef_param_id : spef_integer {};  // must not consume trailing whitespace
struct spef_param_name : spef_qstring {};
struct spef_param_type_for_cap : pegtl::one<'N', 'D', 'X'> {};
struct spef_param_type_for_res : pegtl::one<'N', 'D', 'X'> {};
struct spef_param_type_for_induct : pegtl::one<'N', 'D', 'X'> {};
struct spef_var_coeff : spef_float {};
struct spef_normalization_factor : spef_float {};
struct spef_crt_entry1 : pegtl::seq<spef_param_id, sep, TAO_PEGTL_STRING("CRT1")> {};
struct spef_crt_entry2 : pegtl::seq<spef_param_id, sep, TAO_PEGTL_STRING("CRT2")> {};
struct spef_nominal_temperature : spef_float {};
struct spef_temperature_coeff_def : pegtl::seq<spef_crt_entry1, sep, spef_crt_entry2, sep, spef_nominal_temperature> {};
struct spef_process_param_def : pegtl::seq<spef_param_id, sep, spef_param_name, sep, spef_param_type_for_cap, sep, spef_param_type_for_res, sep, spef_param_type_for_induct, sep, spef_var_coeff, sep, spef_normalization_factor> {};
struct spef_variation_def : pegtl::seq<TAO_PEGTL_STRING("*VARIATION_PARAMETERS"), sep, pegtl::must<pegtl::sor<pegtl::seq<pegtl::star<spef_process_param_def, sep>, spef_temperature_coeff_def>, pegtl::plus<spef_process_param_def, sep>>>> {};

// conn_sec
struct spef_pnode : pegtl::sor<spef_index, spef_bit_identifier> {};
struct spef_pnode_ref : pegtl::seq<spef_physical_inst, spef_pin_delim, spef_pnode, sep> {};  // consumes whitespace
struct spef_pin : pegtl::sor<spef_index, spef_bit_identifier> {};
#ifdef STRICT
struct spef_pin_name : pegtl::seq<spef_inst_name, spef_pin_delim, spef_pin, sep> {};  // consumes whitespace
#else
struct spef_pin_name : pegtl::seq<pegtl::opt<spef_inst_name, spef_pin_delim>, spef_pin, sep> {};  // consumes whitespace
#endif
struct spef_external_connection : pegtl::seq<pegtl::sor<spef_port_name, spef_pport_name>, sep> {};  // consumes whitespace
struct spef_internal_connection : pegtl::sor<spef_pin_name, spef_pnode_ref> {};  // consumes whitespace
struct spef_internal_node_name : pegtl::seq<spef_net_ref, spef_pin_delim, spef_pos_integer, sep> {}; // consumes whitespace
struct spef_internal_node_coord : pegtl::seq<TAO_PEGTL_STRING("*N"), sep, pegtl::must<spef_internal_node_name, spef_coordinates, sep>> {};
struct spef_internal_connection_def : pegtl::seq<TAO_PEGTL_STRING("*I"), sep, pegtl::must<spef_internal_connection, spef_direction, sep, pegtl::star<spef_conn_attr>>> {};
struct spef_external_connection_def : pegtl::seq<TAO_PEGTL_STRING("*P"), sep, pegtl::must<spef_external_connection, spef_direction, sep, pegtl::star<spef_conn_attr>>> {};
struct spef_conn_def : pegtl::sor<spef_external_connection_def, spef_internal_connection_def> {};
struct spef_conn_sec : pegtl::seq<TAO_PEGTL_STRING("*CONN"), sep, pegtl::must<pegtl::plus<spef_conn_def>, pegtl::star<spef_internal_node_coord>>> {};

// cap_sec
struct spef_cap_id : pegtl::seq<spef_pos_integer, sep> {};  // consumes
struct spef_net_ref2 : spef_net_ref {};  // must not consume trailing whitespace
struct spef_node_name : pegtl::sor<spef_external_connection, spef_internal_connection, spef_internal_node_name, spef_pnode_ref> {};  // consumes whitespace
struct spef_node_name2 : pegtl::sor<spef_node_name, pegtl::seq<spef_pnet_ref, spef_pin_delim, spef_pos_integer, sep>, pegtl::seq<spef_net_ref2, spef_pin_delim, spef_pos_integer, sep>> {};  // consumes whitespace
struct spef_sensitivity_coeff : spef_float {};  // must not consume trailing whitespace
struct spef_sensitivity : pegtl::seq<TAO_PEGTL_STRING("*SC"), sep, pegtl::must<pegtl::plus<spef_param_id, pegtl::one<':'>, spef_sensitivity_coeff, sep>>> {};  // consumes whitespace
struct spef_cap_elem_ground : pegtl::seq<spef_cap_id, spef_node_name, spef_par_value, pegtl::opt<spef_sensitivity>> {};
struct spef_cap_elem_coupling : pegtl::seq<spef_cap_id, spef_node_name, spef_node_name2, spef_par_value, pegtl::opt<spef_sensitivity>> {};
struct spef_cap_elem : pegtl::sor<spef_cap_elem_ground, spef_cap_elem_coupling> {};
struct spef_cap_sec : pegtl::seq<TAO_PEGTL_STRING("*CAP"), sep, pegtl::must<pegtl::plus<spef_cap_elem>>> {};

// res_sec
struct spef_res_id : pegtl::seq<spef_pos_integer, sep> {};
struct spef_res_elem : pegtl::seq<spef_res_id, spef_node_name, spef_node_name, spef_par_value, pegtl::opt<spef_sensitivity>> {};
struct spef_res_sec : pegtl::seq<TAO_PEGTL_STRING("*RES"), sep, pegtl::must<pegtl::plus<spef_res_elem>>> {};

// induc_sec
struct spef_induc_id : pegtl::seq<spef_pos_integer, sep> {};
struct spef_induc_elem : pegtl::seq<spef_induc_id, spef_node_name, spef_node_name, spef_par_value, pegtl::opt<spef_sensitivity>> {};
struct spef_induc_sec : pegtl::seq<TAO_PEGTL_STRING("*INDUC"), sep, pegtl::must<pegtl::plus<spef_induc_elem>>> {};

// load_desc
struct spef_real_component : spef_number {};
struct spef_imaginary_component : spef_number {};
struct spef_cnumber : pegtl::seq<pegtl::one<'('>, pegtl::opt<sep>, spef_real_component, sep, spef_imaginary_component, pegtl::opt<sep>, pegtl::one<')'>> {};
struct spef_complex_par_value : pegtl::seq<pegtl::sor<spef_cnumber, spef_number, pegtl::seq<spef_cnumber, pegtl::one<':'>, spef_cnumber, pegtl::one<':'>, spef_cnumber>, pegtl::seq<spef_number, pegtl::one<':'>, spef_number, pegtl::one<':'>, spef_number>>, sep> {};
struct spef_pole : spef_complex_par_value {};
struct spef_pole_desc : pegtl::seq<TAO_PEGTL_STRING("*Q"), sep, pegtl::must<spef_pos_integer, sep, pegtl::plus<spef_pole>>> {};
struct spef_residue : pegtl::seq<spef_complex_par_value, sep> {};
struct spef_residue_desc : pegtl::seq<TAO_PEGTL_STRING("*K"), sep, pegtl::must<spef_pos_integer, sep, pegtl::plus<spef_residue>>> {};
struct spef_pole_residue_desc : pegtl::seq<spef_pole_desc, spef_residue_desc> {};
struct spef_rc_desc : pegtl::seq<TAO_PEGTL_STRING("*RC"), sep, pegtl::must<spef_pin_name, spef_par_value, pegtl::opt<spef_pole_residue_desc>>> {};
struct spef_load_desc : pegtl::seq<TAO_PEGTL_STRING("*LOADS"), sep, pegtl::must<pegtl::plus<spef_rc_desc>>> {};

struct spef_total_cap : spef_par_value {};  // consumes whitespace
struct spef_routing_conf : pegtl::seq<TAO_PEGTL_STRING("*V"), sep, pegtl::must<spef_pos_integer>> {};

struct spef_r_pnet : pegtl::one<'.'> {};
struct spef_d_pnet : pegtl::one<'.'> {};

// r_net (reduced net)
struct spef_driver_pin : pegtl::seq<TAO_PEGTL_STRING("*DRIVER"), sep, pegtl::must<spef_pin_name>> {};  // consumes whitespace
struct spef_driver_cell : pegtl::seq<TAO_PEGTL_STRING("*CELL"), sep, pegtl::must<spef_cell_type, sep>> {};
struct spef_pi_model : pegtl::seq<TAO_PEGTL_STRING("*C2_R1_C1"), sep, pegtl::must<spef_par_value, sep, spef_par_value, sep, spef_par_value, sep>> {};
struct spef_driver_reduc : pegtl::seq<spef_driver_pin, spef_driver_cell, spef_pi_model, spef_load_desc> {};
struct spef_r_net_begin : TAO_PEGTL_STRING("*R_NET") {};
struct spef_r_net_end : TAO_PEGTL_STRING("*END") {};
struct spef_r_net : pegtl::seq<spef_r_net_begin, sep, pegtl::must<spef_net_ref, sep, spef_total_cap, pegtl::opt<spef_routing_conf>, pegtl::opt<spef_driver_reduc>, spef_r_net_end, sep>> {};

// d_net (detailed net)
struct spef_d_net_begin : TAO_PEGTL_STRING("*D_NET") {};
struct spef_d_net_end : TAO_PEGTL_STRING("*END") {};
struct spef_d_net : pegtl::seq<spef_d_net_begin, sep, pegtl::must<spef_net_ref, sep, spef_total_cap, pegtl::opt<spef_routing_conf>, pegtl::opt<spef_conn_sec>, pegtl::opt<spef_cap_sec>, pegtl::opt<spef_res_sec>, pegtl::opt<spef_induc_sec>, spef_d_net_end, sep>> {};

// internal_def
struct spef_nets : pegtl::sor<spef_d_net, spef_r_net, spef_d_pnet, spef_r_pnet> {};
struct spef_internal_def : pegtl::plus<spef_nets> {};

struct spef_grammar : pegtl::seq<spef_header_def, pegtl::opt<spef_name_map>, pegtl::opt<spef_power_def>, pegtl::opt<spef_external_def>, pegtl::opt<spef_define_def>, pegtl::opt<spef_variation_def>, spef_internal_def> {};
// clang-format on

// ACTION STRUCTS

#include <unordered_map>

using cap_t = double;
using res_t = double;
using coord_t = double;
using thresh_t = double;

struct scaled_value {
  double value{0};
  std::string unit{};

  [[nodiscard]] explicit constexpr operator bool() const {
    return value != 0 || !unit.empty();
  }
};

enum struct NetType {
  Detailed,
  Reduced
};

enum struct ConnType {
  ExternalConnection,
  InternalConnection,
};

enum struct DirType {
  Input,
  Output,
  Bidirectional
};

enum struct ConnAttrType {
  Coordinates,
  CapLoad,
  Slews,
  DrivingCell
};

struct Coordinates {
  coord_t x;
  coord_t y;
};

struct Capacitances {
  std::vector<cap_t> m_caps;  // size must be either 1 or 3
};

struct Thresholds {
  std::vector<thresh_t> m_thresh;  // size must be either 1 or 3
};

struct ConnAttr {
  ConnAttr(ConnAttrType type) : m_type{type} {};
  ConnAttr() = delete;
  ConnAttr(ConnAttr const &) = default;
  ConnAttr(ConnAttr &&) = default;
  ConnAttr &operator=(ConnAttr const &) = default;
  ConnAttr &operator=(ConnAttr &&) = default;
  virtual ~ConnAttr() = default;  // virtual destructor so the destructor of
                                  // each subclass is called during destruction
                                  // through std::unique_ptr pointing to the
                                  // base class
  ConnAttrType m_type;
};

struct CoordinatesAttr : ConnAttr {
  CoordinatesAttr(Coordinates coord)
      : ConnAttr(ConnAttrType::Coordinates),
        m_coord(coord) {}
  Coordinates m_coord;
};

struct CapLoadAttr : ConnAttr {
  CapLoadAttr(Capacitances cap)
      : ConnAttr(ConnAttrType::CapLoad),
        m_cap(std::move(cap)) {}
  Capacitances m_cap;
};

struct SlewsAttr : ConnAttr {
  SlewsAttr(Capacitances cap1, Capacitances cap2)
      : ConnAttr(ConnAttrType::Slews),
        m_cap1(std::move(cap1)),
        m_cap2(std::move(cap2)) {}
  SlewsAttr(
      Capacitances cap1,
      Capacitances cap2,
      Thresholds thresh1,
      Thresholds thresh2)
      : ConnAttr(ConnAttrType::Slews),
        m_cap1(std::move(cap1)),
        m_cap2(std::move(cap2)),
        m_thresh1(std::move(thresh1)),
        m_thresh2(std::move(thresh2)) {}
  Capacitances m_cap1;
  Capacitances m_cap2;
  Thresholds m_thresh1;
  Thresholds m_thresh2;
};

struct DrivingCellAttr : ConnAttr {
  DrivingCellAttr(std::string cell)
      : ConnAttr(ConnAttrType::DrivingCell),
        m_cell(std::move(cell)) {}
  std::string m_cell;
};

struct Port {
  std::string m_name;
  DirType m_direction;
  std::vector<std::unique_ptr<ConnAttr>> m_conn_attrs;
};

struct PhysicalPort {
  std::string m_name;
  DirType m_direction;
  std::vector<std::unique_ptr<ConnAttr>> m_conn_attrs;
};

struct DNet {
  struct Connection;
  struct InternalNode;
  struct GroundCapacitance;
  struct CouplingCapacitance;
  struct Resistance;

  std::string m_name;
  cap_t m_total_cap;
  std::vector<Connection> m_conns;
  std::vector<InternalNode> m_nodes;
  std::vector<GroundCapacitance> m_ground_caps;
  std::vector<CouplingCapacitance> m_coupling_caps;
  std::vector<Resistance> m_resistances;
};

struct DNet::Connection {
  ConnType m_type;
  std::string m_name;
  DirType m_direction;
  std::vector<std::unique_ptr<ConnAttr>> m_conn_attrs;
};

struct DNet::InternalNode {
  std::string m_name;
  std::unique_ptr<CoordinatesAttr> m_coords;
};

struct DNet::GroundCapacitance {
  std::string m_id;
  std::string m_node;
  cap_t m_cap;
};

struct DNet::CouplingCapacitance {
  std::string m_id;
  std::string m_node1;
  std::string m_node2;
  cap_t m_cap;
};

struct DNet::Resistance {
  std::string m_id;
  std::string m_node1;
  std::string m_node2;
  res_t m_res;
};

struct RNet {
  std::string m_name;
  cap_t m_total_cap;
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
  scaled_value m_time_scale;    // in seconds
  scaled_value m_cap_scale;     // in Farad
  scaled_value m_res_scale;     // in Ohm
  scaled_value m_induct_scale;  // in Henry
  std::vector<std::string> m_power_nets;
  std::vector<std::string> m_ground_nets;
  std::vector<Port> m_ports;
  std::vector<PhysicalPort> m_physcial_ports;
  std::unordered_map<std::string, std::string> m_name_map;
  std::vector<DNet> m_d_nets;
  std::vector<RNet> m_r_nets;
};

// temporary data structure to be filled during parsing, and parts of it can be
// then moved to the SPEF structure
struct SPEFHelper {
  std::vector<std::string_view> m_tokens;
  std::vector<std::string_view> m_tokens2;
  bool reading_d_net;
  bool reading_r_net;
  DNet m_current_d_net;
  RNet m_current_r_net;
  std::vector<std::unique_ptr<ConnAttr>> attributes;
};
#endif  // SPEF_STRUCTS_HPP
