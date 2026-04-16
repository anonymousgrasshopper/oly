#pragma once

#include <string>

namespace Color {
// clang-format off
constexpr const std::string RESET  = "\x1b[0m";
constexpr const std::string BOLD   = "\x1b[1m";
constexpr const std::string ITALIC = "\x1b[3m";

constexpr const std::string RED    = "\x1b[38;5;196m";
constexpr const std::string YELLOW = "\x1b[38;5;226m";
constexpr const std::string BLUE   = "\x1b[38;5;39m";
constexpr const std::string GREEN  = "\x1b[38;5;46m";
constexpr const std::string GRAY   = "\x1b[38;5;245m";
constexpr const std::string FAINT  = "\x1b[38;5;250m";
// clang-format on
} // namespace Color
