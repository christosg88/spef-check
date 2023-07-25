#include "spef_actions.hpp"

void split(
    std::string_view str,
    std::vector<std::string_view> &tokens,
    std::size_t max_splits,
    std::string_view sep) {
  tokens.clear();

  auto const is_sep = [sep](char const chr) -> bool {
    return std::find(std::begin(sep), std::end(sep), chr) != std::end(sep);
  };

  char const *begin = nullptr;
  std::size_t len = 0;
  bool consume_remaining = false;

  for (char const &chr : str) {
    if (consume_remaining) {
      ++len;
    } else if (is_sep(chr)) {
      if (len != 0) {
        tokens.emplace_back(begin, len);
        len = 0;
      }
    } else {
      if (len == 0) {
        begin = &chr;
        if (max_splits != 0 && tokens.size() == max_splits) {
          consume_remaining = true;
        }
      }
      ++len;
    }
  }

  if (len != 0) {
    tokens.emplace_back(begin, len);
  }

  if (!tokens.empty()) {
    // trim the trailing sep from the end of the last token
    std::size_t to_trim{};
    auto &last_token = tokens.back();
    for (auto rit = last_token.rbegin(); rit != last_token.rend(); ++rit) {
      if (!is_sep(*rit)) {
        break;
      }
      ++to_trim;
    }
    last_token.remove_suffix(to_trim);
  }
}

void handle_from_chars(std::errc ec, std::string_view match) {
  if (ec == std::errc::invalid_argument) {
    throw std::runtime_error(
        fmt::format("Couldn't parse the value: {}", match));
  }
  if (ec == std::errc::result_out_of_range) {
    throw std::runtime_error(fmt::format(
        "The value does not fit in the underlying type: {}",
        match));
  }
}

DirType convert_direction(std::string_view direction_sv) {
  if (direction_sv == "I") {
    return DirType::Input;
  }
  if (direction_sv == "O") {
    return DirType::Output;
  }
  if (direction_sv == "B") {
    return DirType::Bidirectional;
  }
  throw std::runtime_error("Unknown direction type");
};

Capacitances get_caps(std::vector<std::string_view> const &tokens) {
  Capacitances caps;
  for (auto const &token : tokens) {
    cap_t &cap = caps.m_caps.emplace_back();
    auto const [_, ec] = std::from_chars(token.begin(), token.end(), cap);
    handle_from_chars(ec, token);
  }
  return caps;
}

Thresholds get_thresholds(std::vector<std::string_view> const &tokens) {
  Thresholds threshs;
  for (auto const &token : tokens) {
    thresh_t &thresh = threshs.m_thresh.emplace_back();
    auto const [_, ec] = std::from_chars(token.begin(), token.end(), thresh);
    handle_from_chars(ec, token);
  }
  return threshs;
}
