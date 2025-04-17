#ifndef NYXPIRI_OVRPENGUIN_RESULT_HPP
#define NYXPIRI_OVRPENGUIN_RESULT_HPP
#include <string>
#include "math/num_types.hpp"

namespace nyxpiri::ovrpenguin
{
template<typename CodeType = i32>
class Result
{
public:
    Result() = default;
    Result(const std::string& in_error_message, CodeType in_error_code)
        : error(true), error_message(in_error_message), error_code(in_error_code) {}

    [[nodiscard]] bool is_error() const
    {
        return error;
    }

    [[nodiscard]] bool failed() const
    {
        return error;
    }

    [[nodiscard]] const std::string& get_error_message() const
    {
        return error_message;
    }

    [[nodiscard]] CodeType get_error_code() const
    {
        return error_code;
    }

private:
    bool error = false;
    std::string error_message;
    CodeType error_code = CodeType();
};
} // namespace nyxpiri::ovrpenguin
#endif