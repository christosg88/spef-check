#include "spef_structs.hpp"
#include <ctime>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <random>

/// This class can be used to generate a random, standard-compliant SPEF file,
/// with optional fields potentially missing
class RandomSPEFGenerator {
private:
  SPEF m_spef;
  std::random_device m_rd;  // a seed source for the random number engine
  std::mt19937 m_gen;       // mersene_twister_engine

  std::string const m_design_name;
  std::size_t const m_num_power_nets;
  std::size_t const m_num_ground_nets;
  std::size_t const m_num_d_nets;

public:
  RandomSPEFGenerator(
      std::string design_name,
      std::size_t num_power_nets,
      std::size_t num_ground_nets,
      std::size_t num_d_nets)
      : m_gen(m_rd()),
        m_design_name(std::move(design_name)),
        m_num_power_nets(num_power_nets),
        m_num_ground_nets(num_ground_nets),
        m_num_d_nets(num_d_nets) {}

  SPEF generate() {
    gen_header_def();
    gen_name_map();
    gen_power_def();
    gen_external_def();
    gen_entities_def();
    gen_variation_def();
    gen_internal_def();

    return std::move(m_spef);
  }

private:
  char r_choose(std::string_view choices) {
    std::uniform_int_distribution<std::size_t> dist(0, choices.size() - 1);
    return choices[dist(m_gen)];
  }
  template<typename T>
  T &r_choose(std::initializer_list<T> &choices) {
    std::uniform_int_distribution<std::size_t> dist(0, choices.size() - 1);
    return *std::next(choices.begin(), dist(m_gen));
  }
  template<typename T>
  T const &r_choose(std::initializer_list<T> const &choices) {
    std::uniform_int_distribution<std::size_t> dist(0, choices.size() - 1);
    return *std::next(choices.begin(), dist(m_gen));
  }
  template<typename T>
  T &r_choose(std::vector<T> &choices) {
    std::uniform_int_distribution<std::size_t> dist(0, choices.size() - 1);
    return choices[dist(m_gen)];
  }
  template<typename T>
  T const &r_choose(std::vector<T> const &choices) {
    std::uniform_int_distribution<std::size_t> dist(0, choices.size() - 1);
    return choices[dist(m_gen)];
  }
  bool r_coin_flip(double bias = 0.5) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return (dist(m_gen) < bias);
  }
  std::size_t r_rand(std::size_t min, std::size_t max) {
    return std::uniform_int_distribution<std::size_t>(min, max)(m_gen);
  }
  double r_rand(double min, double max) {
    return std::uniform_real_distribution<double>(min, max)(m_gen);
  }
  std::string r_name() {
    //std::size_t num_chars = r_rand(1UL, 30UL);
    std::size_t num_chars = 5;
    std::string name(num_chars, '\0');
    std::generate(name.begin(), name.end(), [this]() {
      static constexpr std::string_view alnum =
          "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
      return alnum[r_rand(0, alnum.size() - 1)];
    });
    return name;
  }

  void gen_header_def() {
    m_spef.m_version = "\"IEEE 1481-2019\"";
    m_spef.m_design_name = "\"Random Design\"";
    auto now = std::time(nullptr);
    m_spef.m_date = fmt::format("\"{:%c}\"", fmt::localtime(now));
    m_spef.m_vendor = "\"RandomSPEFGenerator\"";
    m_spef.m_program_name = "\"RandomSPEFGenerator\"";
    m_spef.m_program_version = "\"0.0.1\"";
    m_spef.m_design_flow = "\"FULL_CONNECTIVITY\"";
    m_spef.m_hierarchy_div_def = r_choose("./:|");  // default '/'
    do {
      m_spef.m_pin_delim_def = r_choose("./:|");  // default ':'

    } while (m_spef.m_pin_delim_def == m_spef.m_hierarchy_div_def);
    do {
      switch (r_rand(0UL, 5UL)) {
      case 0UL:
        // default
        m_spef.m_prefix_bus_delim = '[';
        m_spef.m_suffix_bus_delim = ']';
        break;
      case 1UL:
        m_spef.m_prefix_bus_delim = '{';
        m_spef.m_suffix_bus_delim = '}';
        break;
      case 2UL:
        m_spef.m_prefix_bus_delim = '(';
        m_spef.m_suffix_bus_delim = ')';
        break;
      case 3UL:
        m_spef.m_prefix_bus_delim = '<';
        m_spef.m_suffix_bus_delim = '>';
        break;
      case 4UL:
        m_spef.m_prefix_bus_delim = ':';
        break;
      case 5UL:
        m_spef.m_prefix_bus_delim = '.';
        break;
      }
    } while (m_spef.m_prefix_bus_delim == m_spef.m_hierarchy_div_def ||
             m_spef.m_prefix_bus_delim == m_spef.m_pin_delim_def);
    m_spef.m_time_scale = scaled_value{1, r_choose({"NS", "PS"})};
    m_spef.m_cap_scale = scaled_value{1, r_choose({"FF", "PF"})};
    m_spef.m_res_scale = scaled_value{1, r_choose({"OHM", "KOHM"})};
    m_spef.m_induct_scale = scaled_value{1, r_choose({"HENRY", "MH", "UH"})};
  }

  void gen_name_map() {}

  void gen_power_def() {
    if (r_coin_flip()) {
      gen_power_nets();
    }
    if (r_coin_flip()) {
      gen_ground_nets();
    }
  }

  void gen_external_def() {}
  void gen_entities_def() {}
  void gen_variation_def() {}
  void gen_internal_def() { gen_d_nets(); }

  void gen_power_nets() {
    m_spef.m_power_nets.reserve(m_num_power_nets);
    for (std::size_t i = 0; i < m_num_power_nets; ++i) {
      m_spef.m_power_nets.emplace_back(r_name());
    }
  }

  void gen_ground_nets() {
    m_spef.m_ground_nets.reserve(m_num_ground_nets);
    for (std::size_t i = 0; i < m_num_ground_nets; ++i) {
      m_spef.m_ground_nets.emplace_back(r_name());
    }
  }

  void gen_d_nets_names() {
    // TODO: make sure the names are unique
    for (DNet &d_net : m_spef.m_d_nets) {
      d_net.m_name = r_name();
    }
  }

  void gen_d_nets_routing_confidence() {
    for (DNet &d_net : m_spef.m_d_nets) {
      if (r_coin_flip()) {
        continue;
      }

      static constexpr std::initializer_list<unsigned int> routing_confidences{
          0 /*no routing confidence*/,
          10 /*Statistical wire load model*/,
          20 /*Physical wire load model*/,
          30 /*Physical partitions with locations, no cell placement*/,
          40 /*Estimated cell placement with steiner tree based route*/,
          50 /*Estimated cell placement with global route*/,
          60 /*Final cell placement with Steiner route*/,
          70 /*Final cell placement with global route*/,
          80 /*Final cell placement, final route, 2d extraction*/,
          90 /*Final cell placement, final route, 2.5d extraction*/,
          100 /*Final cell placement, final route, 3d extraction*/};
      d_net.m_routing_conf = r_choose(routing_confidences);
    }
  }

  void gen_d_nets_connections() {
    for (DNet &d_net : m_spef.m_d_nets) {
      // generate external/internal connections (ports/pins)
      // genetate internal connections (pins)
      d_net.m_conns.resize(r_rand(1UL, 5UL));
      for (DNet::Connection &conn : d_net.m_conns) {
        conn.m_type = r_choose(
            {ConnType::ExternalConnection, ConnType::InternalConnection});
        conn.m_name = r_name();
        conn.m_direction =
            r_choose({DirType::Bidirectional, DirType::Input, DirType::Output});
        // TODO: generate connection attributes
      }

      // generate net nodes
      d_net.m_nodes.resize(r_rand(1UL, 5UL));
      for (DNet::InternalNode &node : d_net.m_nodes) {
        // TODO: make sure the name is in the correct format
        node.m_name = r_name();
        node.m_coords = std::make_unique<CoordinatesAttr>(
            Coordinates{r_rand(0.0, 1000.0), r_rand(0.0, 1000.0)});
      }
    }
  }

  void gen_d_nets_ground_capacitances() {
    for (DNet &d_net : m_spef.m_d_nets) {
      for (DNet::Connection &conn : d_net.m_conns) {
        d_net.m_ground_caps.push_back({conn.m_name, r_rand(1.0, 10.0)});
      }
      for (DNet::InternalNode &node : d_net.m_nodes) {
        d_net.m_ground_caps.push_back({node.m_name, r_rand(1.0, 10.0)});
      }
    }
  }

  void gen_d_nets_coupling_capacitances() {
    for (DNet &d_net : m_spef.m_d_nets) {
      for (DNet::Connection &conn : d_net.m_conns) {
        DNet::CouplingCapacitance ccap;
        ccap.m_cap = r_rand(1.0, 10.0);
        ccap.m_node1 = conn.m_name;

        // get a random victim net
        DNet &other_d_net = r_choose(m_spef.m_d_nets);
        if (r_coin_flip()) {
          ccap.m_node2 = other_d_net.m_name + m_spef.m_pin_delim_def +
                         r_choose(other_d_net.m_conns).m_name;
        } else {
          ccap.m_node2 = other_d_net.m_name + m_spef.m_pin_delim_def +
                         r_choose(other_d_net.m_nodes).m_name;
        }

        d_net.m_coupling_caps.emplace_back(ccap);
        std::swap(ccap.m_node1, ccap.m_node2);
        other_d_net.m_coupling_caps.emplace_back(ccap);
      }
      for (DNet::InternalNode &node : d_net.m_nodes) {
        DNet::CouplingCapacitance ccap;
        ccap.m_cap = r_rand(1.0, 10.0);
        ccap.m_node1 = node.m_name;
        // get a random victim net
        DNet &other_d_net = r_choose(m_spef.m_d_nets);
        if (r_coin_flip()) {
          ccap.m_node2 = other_d_net.m_name + m_spef.m_pin_delim_def +
                         r_choose(other_d_net.m_conns).m_name;
        } else {
          ccap.m_node2 = other_d_net.m_name + m_spef.m_pin_delim_def +
                         r_choose(other_d_net.m_nodes).m_name;
        }

        d_net.m_coupling_caps.emplace_back(ccap);
        std::swap(ccap.m_node1, ccap.m_node2);
        other_d_net.m_coupling_caps.emplace_back(ccap);
      }
    }
  }

  void gen_d_nets_capacitances() {
    gen_d_nets_ground_capacitances();
    gen_d_nets_coupling_capacitances();
  }

  void gen_d_nets() {
    m_spef.m_d_nets.resize(m_num_d_nets);

    // first we need to generate names for all nets, so that we can refer to
    // them from other nets
    gen_d_nets_names();
    gen_d_nets_routing_confidence();
    gen_d_nets_connections();
    gen_d_nets_capacitances();

    // TODO: gen_res_sec
    // TODO: gen_induc_sec
  }
};
