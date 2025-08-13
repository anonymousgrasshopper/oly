#include <filesystem>
namespace fs = std::filesystem;

fs::path parse_pb_name(const std::string& pb_name);

fs::path get_path(const std::string& pb_name);
