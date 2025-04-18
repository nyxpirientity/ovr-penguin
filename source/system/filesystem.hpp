#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP
#include <filesystem>
#include <optional>

namespace nyxpiri::ovrpenguin::filesystem
{
    std::optional<std::filesystem::path> get_home_dir();
    std::optional<std::filesystem::path> get_save_dir();
} // namespace nyxpiri::ovrpenguin::filesystem
#endif