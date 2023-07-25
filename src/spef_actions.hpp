#ifndef SPEF_ACTIONS_HPP
#define SPEF_ACTIONS_HPP

#include "spef_structs.hpp"
#include <array>
#include <cctype>
#include <charconv>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <iostream>
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

void split(
    std::string_view str,
    std::vector<std::string_view> &tokens,
    std::size_t max_splits = 0,
    std::string_view sep = " \t\n\r\f\v");

void handle_from_chars(std::errc ec, std::string_view match);

DirType convert_direction(std::string_view direction_sv);

Capacitances get_caps(std::vector<std::string_view> const &tokens);

Thresholds get_thresholds(std::vector<std::string_view> const &tokens);

template<typename Rule>
struct spef_action : tao::pegtl::nothing<Rule> {};

template<>
struct spef_action<spef_version> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens, 1);
    spef.m_version = spef_h.m_tokens[1];
  }
};

template<>
struct spef_action<spef_design_name> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens, 1);
    spef.m_design_name = spef_h.m_tokens[1];
  }
};

template<>
struct spef_action<spef_date> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens, 1);
    spef.m_date = spef_h.m_tokens[1];
  }
};

template<>
struct spef_action<spef_vendor> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens, 1);
    spef.m_vendor = spef_h.m_tokens[1];
  }
};

template<>
struct spef_action<spef_program_name> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens, 1);
    spef.m_program_name = spef_h.m_tokens[1];
  }
};

template<>
struct spef_action<spef_program_version> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens, 1);
    spef.m_program_version = spef_h.m_tokens[1];
  }
};

template<>
struct spef_action<spef_design_flow> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens, 1);
    spef.m_design_flow = spef_h.m_tokens[1];
  }
};

template<>
struct spef_action<spef_hierarchy_div_def> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    spef.m_hierarchy_div_def = spef_h.m_tokens[1][0];
  }
};

template<>
struct spef_action<spef_pin_delim_def> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    spef.m_pin_delim_def = spef_h.m_tokens[1][0];
  }
};

template<>
struct spef_action<spef_bus_delim_def> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    if (spef_h.m_tokens.size() == 3) {
      spef.m_prefix_bus_delim = spef_h.m_tokens[1][0];
      spef.m_suffix_bus_delim = spef_h.m_tokens[2][0];
    } else {
      spef.m_prefix_bus_delim = spef_h.m_tokens[1][0];
    }
  }
};

template<>
struct spef_action<spef_time_scale> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    auto number = spef_h.m_tokens[1];
    auto unit = spef_h.m_tokens[2];

    double num{};
    auto const [_, ec] = std::from_chars(number.begin(), number.end(), num);
    handle_from_chars(ec, number);

    spef.m_time_scale = scaled_value{num, std::string(unit)};
  }
};

template<>
struct spef_action<spef_cap_scale> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    auto number = spef_h.m_tokens[1];
    auto unit = spef_h.m_tokens[2];

    double num{};
    auto const [_, ec] = std::from_chars(number.begin(), number.end(), num);
    handle_from_chars(ec, number);

    spef.m_cap_scale = scaled_value{num, std::string(unit)};
  }
};

template<>
struct spef_action<spef_res_scale> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    auto number = spef_h.m_tokens[1];
    auto unit = spef_h.m_tokens[2];

    double num{};
    auto const [_, ec] = std::from_chars(number.begin(), number.end(), num);
    handle_from_chars(ec, number);

    spef.m_res_scale = scaled_value{num, std::string(unit)};
  }
};

template<>
struct spef_action<spef_induct_scale> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    auto number = spef_h.m_tokens[1];
    auto unit = spef_h.m_tokens[2];

    double num{};
    auto const [_, ec] = std::from_chars(number.begin(), number.end(), num);
    handle_from_chars(ec, number);

    spef.m_induct_scale = scaled_value{num, std::string(unit)};
  }
};

template<>
struct spef_action<spef_power_net_def> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);

    for (auto it = std::next(spef_h.m_tokens.begin());
         it != spef_h.m_tokens.end();
         ++it) {
      spef.m_power_nets.emplace_back(*it);
    }
  }
};

template<>
struct spef_action<spef_ground_net_def> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);

    for (auto it = std::next(spef_h.m_tokens.begin());
         it != spef_h.m_tokens.end();
         ++it) {
      spef.m_ground_nets.emplace_back(*it);
    }
  }
};

template<>
struct spef_action<spef_coordinates> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    auto x_sv = spef_h.m_tokens[1];
    auto y_sv = spef_h.m_tokens[2];

    Coordinates coord{};
    {
      auto const [_, ec] = std::from_chars(x_sv.begin(), x_sv.end(), coord.x);
      handle_from_chars(ec, x_sv);
    }
    {
      auto const [_, ec] = std::from_chars(y_sv.begin(), y_sv.end(), coord.y);
      handle_from_chars(ec, y_sv);
    }
    spef_h.attributes.emplace_back(std::make_unique<CoordinatesAttr>(coord));
  }
};

template<>
struct spef_action<spef_cap_load> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    split(spef_h.m_tokens[1], spef_h.m_tokens2, 0, ":");

    Capacitances caps = get_caps(spef_h.m_tokens2);
    spef_h.attributes.emplace_back(std::make_unique<CapLoadAttr>(caps));
  }
};

template<>
struct spef_action<spef_slews> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    split(spef_h.m_tokens[1], spef_h.m_tokens2, 0, ":");
    Capacitances caps1 = get_caps(spef_h.m_tokens2);
    split(spef_h.m_tokens[2], spef_h.m_tokens2, 0, ":");
    Capacitances caps2 = get_caps(spef_h.m_tokens2);

    if (spef_h.m_tokens.size() == 3) {
      // only caps given
      spef_h.attributes.emplace_back(
          std::make_unique<SlewsAttr>(std::move(caps1), std::move(caps2)));
      return;
    }

    // both caps and thresholds given
    split(spef_h.m_tokens[3], spef_h.m_tokens2, 0, ":");
    Thresholds thresh1 = get_thresholds(spef_h.m_tokens2);
    split(spef_h.m_tokens[4], spef_h.m_tokens2, 0, ":");
    Thresholds thresh2 = get_thresholds(spef_h.m_tokens2);
    spef_h.attributes.emplace_back(std::make_unique<SlewsAttr>(
        std::move(caps1),
        std::move(caps2),
        std::move(thresh1),
        std::move(thresh2)));
  }
};

template<>
struct spef_action<spef_driving_cell> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    spef_h.attributes.emplace_back(
        std::make_unique<DrivingCellAttr>(std::string(spef_h.m_tokens[1])));
  }
};

template<>
struct spef_action<spef_port_entry> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);

    auto port_name = spef_h.m_tokens[0];
    auto direction = spef_h.m_tokens[1];

    spef.m_ports.push_back(
        {std::string(port_name),
         convert_direction(direction),
         std::move(spef_h.attributes)});
  }
};

template<>
struct spef_action<spef_pport_entry> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);

    auto port_name = spef_h.m_tokens[0];
    auto direction = spef_h.m_tokens[1];

    spef.m_physcial_ports.push_back(
        {std::string(port_name),
         convert_direction(direction),
         std::move(spef_h.attributes)});
  }
};

template<>
struct spef_action<spef_name_map_entry> {
  template<typename Action>
  static void apply(Action const &input, SPEF &spef, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens, 1);
    auto index = spef_h.m_tokens[0];
    auto name = spef_h.m_tokens[1];
    spef.m_name_map.emplace(std::string(index), std::string(name));
  }
};

template<>
struct spef_action<spef_d_net_begin> {
  template<typename Action>
  static void apply(Action const &, SPEF &, SPEFHelper &spef_h) {
    spef_h.reading_d_net = true;
  }
};

template<>
struct spef_action<spef_r_net_begin> {
  template<typename Action>
  static void apply(Action const &, SPEF &, SPEFHelper &spef_h) {
    spef_h.reading_r_net = true;
  }
};

template<>
struct spef_action<spef_net_ref> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    if (spef_h.reading_d_net) {
      spef_h.m_current_d_net.m_name = std::move(input.string());
    } else if (spef_h.reading_r_net) {
      spef_h.m_current_r_net.m_name = std::move(input.string());
    }
  }
};

template<>
struct spef_action<spef_total_cap> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    cap_t cap{};
    auto number = input.string_view();
    auto const [_, ec] = std::from_chars(number.begin(), number.end(), cap);
    handle_from_chars(ec, number);

    if (spef_h.reading_d_net) {
      spef_h.m_current_d_net.m_total_cap = cap;
    } else if (spef_h.reading_r_net) {
      spef_h.m_current_r_net.m_total_cap = cap;
    }
  }
};

template<>
struct spef_action<spef_d_net_end> {
  template<typename Action>
  static void apply(Action const &, SPEF &spef, SPEFHelper &spef_h) {
    spef.m_d_nets.emplace_back(std::move(spef_h.m_current_d_net));
    spef_h.reading_d_net = false;
  }
};

template<>
struct spef_action<spef_r_net_end> {
  template<typename Action>
  static void apply(Action const &, SPEF &spef, SPEFHelper &spef_h) {
    spef.m_r_nets.emplace_back(std::move(spef_h.m_current_r_net));
    spef_h.reading_r_net = false;
  }
};

template<>
struct spef_action<spef_routing_conf> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    auto const &number = spef_h.m_tokens[2];
    unsigned int routing_conf{};
    auto const [_, ec] =
        std::from_chars(number.begin(), number.end(), routing_conf);
    handle_from_chars(ec, number);

    if (spef_h.reading_d_net) {
      spef_h.m_current_d_net.m_routing_conf = routing_conf;
    } else if (spef_h.reading_r_net) {
      spef_h.m_current_r_net.m_routing_conf = routing_conf;
    }
  }
};

template<>
struct spef_action<spef_external_connection_def> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    auto const name = spef_h.m_tokens[1];
    auto const direction_sv = spef_h.m_tokens[2];

    auto direction = convert_direction(direction_sv);

    spef_h.m_current_d_net.m_conns.push_back(
        {ConnType::ExternalConnection,
         std::string(name),
         direction,
         std::move(spef_h.attributes)});
  }
};

template<>
struct spef_action<spef_internal_connection_def> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    auto const name = spef_h.m_tokens[1];
    auto const direction_sv = spef_h.m_tokens[2];

    auto direction = convert_direction(direction_sv);

    spef_h.m_current_d_net.m_conns.push_back(
        {ConnType::InternalConnection,
         std::string(name),
         direction,
         std::move(spef_h.attributes)});
  }
};

template<>
struct spef_action<spef_internal_node_coord> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);
    auto const name = spef_h.m_tokens[1];
    spef_h.m_current_d_net.m_nodes.push_back(
        {std::string(name),
         std::unique_ptr<CoordinatesAttr>(
             static_cast<CoordinatesAttr *>(spef_h.attributes[0].get()))});
    spef_h.attributes[0].release();
    spef_h.attributes.clear();
  }
};

template<>
struct spef_action<spef_cap_elem_ground> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);

    auto const node = spef_h.m_tokens[1];
    cap_t cap{};
    auto const cap_value = spef_h.m_tokens[2];
    {
      auto const [_, ec] =
          std::from_chars(cap_value.begin(), cap_value.end(), cap);
      handle_from_chars(ec, cap_value);
    }

    // TODO: add sensitivity

    spef_h.m_current_d_net.m_ground_caps.push_back({std::string{node}, cap});
  }
};

template<>
struct spef_action<spef_cap_elem_coupling> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);

    auto const node1 = spef_h.m_tokens[1];
    auto const node2 = spef_h.m_tokens[2];
    cap_t cap{};
    auto const cap_value = spef_h.m_tokens[3];
    {
      auto const [_, ec] =
          std::from_chars(cap_value.begin(), cap_value.end(), cap);
      handle_from_chars(ec, cap_value);
    }

    // TODO: add sensitivity

    spef_h.m_current_d_net.m_coupling_caps.push_back(
        {std::string{node1}, std::string{node2}, cap});
  }
};

template<>
struct spef_action<spef_res_elem> {
  template<typename Action>
  static void apply(Action const &input, SPEF &, SPEFHelper &spef_h) {
    split(input.string_view(), spef_h.m_tokens);

    auto const id = spef_h.m_tokens[0];
    auto const node1 = spef_h.m_tokens[1];
    auto const node2 = spef_h.m_tokens[2];
    res_t res{};
    auto const res_value = spef_h.m_tokens[3];
    auto const [_, ec] =
        std::from_chars(res_value.begin(), res_value.end(), res);
    handle_from_chars(ec, res_value);

    // TODO: add sensitivity

    spef_h.m_current_d_net.m_resistances.push_back(
        {std::string{id}, std::string{node1}, std::string{node2}, res});
  }
};

#endif  // SPEF_ACTIONS_HPP
