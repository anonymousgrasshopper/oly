#include <filesystem>
#include <string>

namespace fs = std::filesystem;

[[nodiscard]]
fs::path get_problem_path(const std::string& source);

[[nodiscard]]
std::string get_problem_relative_path(const std::string& source);
