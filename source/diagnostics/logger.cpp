#include "logger.hpp"
#include <filesystem>

namespace nyxpiri::ovrpenguin
{
Logger::Logger()
{
    log_out_stream.lock().data().open("ovrpenguin_log.txt", std::ofstream::app);
    print("\n"
        "--------================================--------\n"
        "--------======== NEW LOG START! ========--------\n"
        "--------================================--------\n\n"
        , false);
    log("logger", "Logger object constructed!", false);
}

Logger::~Logger()
{
    log("logger", "Logger object being destroyed...", false);
    print("\n"
        "--------==========================--------\n"
        "--------======== LOG END! ========--------\n"
        "--------==========================--------\n\n"
        , false);
}

void Logger::log(const std::string& log_source_name, const std::string& message, bool terminal_output)
{
    const std::string output_string = "[info] ~ " + log_source_name + ": " + message + "\n";
    
    print(output_string, terminal_output);
}
void Logger::log_error(const std::string &log_source_name, const std::string &message, bool terminal_output)
{
    const std::string output_string = "[error] ~ " + log_source_name + ": " + message + "\n";

    print(output_string, terminal_output);
}
void Logger::print(const std::string &message, bool terminal_output)
{
    auto guard = log_out_stream.lock();
    guard.data().write(message.c_str(), message.size());

    if (terminal_output_function and terminal_output)
    {
        terminal_output_function(message);
    }
}
} // namespace nyxpiri::ovrpenguin
