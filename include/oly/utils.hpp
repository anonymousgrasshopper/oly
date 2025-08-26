#include <filesystem>
namespace fs = std::filesystem;

// displays a help message
void print_help();

// expands tilde and environment variables in a string
std::string expand_env_vars(const std::string& str);

fs::path parse_pb_name(const std::string& pb_name);

fs::path get_path(const std::string& pb_name);
