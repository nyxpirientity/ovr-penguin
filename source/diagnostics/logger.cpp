#include "logger.hpp"
#include <filesystem>

namespace nyxpiri::ovrpenguin
{
Logger::Logger()
{
    log_out_stream.lock().data().open("ovrpenguin_log.txt", std::ofstream::app);
    log("logger", "Logger object constructed!", false);
}

Logger::~Logger()
{
    log("logger", "Logger object being destroyed...", false);
}

void Logger::log(const std::string& log_source_name, const std::string& message, bool terminal_output)
{
    const std::string output_string = "[info] ~ " + log_source_name + ": " + message + "\n";
    
    auto guard = log_out_stream.lock();
    guard.data().write(output_string.c_str(), output_string.size());

    if (terminal_output_function and terminal_output)
    {
        terminal_output_function(output_string);
    }
}
} // namespace nyxpiri::ovrpenguin
