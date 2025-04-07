#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <string>
#include <fstream>
#include <functional>
#include "types/mutex.hpp"

namespace nyxpiri::ovrpenguin
{
class Logger
{
public:
    Logger();
    ~Logger();

    void log(const std::string& log_source_name, const std::string& message, bool terminal_output);
    void log_error(const std::string& log_source_name, const std::string& message, bool terminal_output);

    std::function<void(std::string)> terminal_output_function;
protected:
    void print(const std::string& message, bool terminal_output);

private:
    Mutex<std::ofstream> log_out_stream;
};
} // namespace nyxpiri::ovrpenguin
#endif