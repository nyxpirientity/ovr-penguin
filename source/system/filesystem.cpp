#include "filesystem.hpp"

#include <unistd.h>
#include <pwd.h>

namespace nyxpiri::ovrpenguin::filesystem
{
static std::filesystem::path home_dir;
static std::filesystem::path save_dir;

std::optional<std::filesystem::path> get_home_dir()
{
    if (home_dir.empty())
    {
        char* home_str = getenv("HOME");
		
        if (home_str == NULL)
		{
            passwd* pwuid = getpwuid(getuid());
            if (pwuid == NULL)
            {
                return std::nullopt;
            }

			home_str = pwuid->pw_dir;
        }

        home_dir = std::filesystem::path(std::string(home_str));
    }

    if (!std::filesystem::exists(home_dir) or home_dir.empty() or !std::filesystem::is_directory(home_dir))
    {
        return std::nullopt;
    }

    return home_dir;
}

std::optional<std::filesystem::path> get_save_dir()
{
    if (save_dir.empty())
    {
        std::optional<std::filesystem::path> home = get_home_dir();

        if (!home)
        {
            return std::nullopt;
        }

        save_dir = *home / ".config" / "ovrpenguin";
    }

    if (save_dir.empty())
    {
        return std::nullopt;
    }

    std::filesystem::create_directories(save_dir);

    if (!std::filesystem::exists(save_dir) or !std::filesystem::is_directory(save_dir))
    {
        return std::nullopt;
    }

    return save_dir;
}
} // namespace nyxpiri::ovrpenguin::filesystem