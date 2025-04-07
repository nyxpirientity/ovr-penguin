#ifndef STRING_COMMAND_HPP
#define STRING_COMMAND_HPP
#include "math/num_types.hpp"
#include <string>
#include <vector>
#include <tuple>

namespace nyxpiri::ovrpenguin
{
class StringCommand
{
public:
    StringCommand() = default;
    StringCommand(const std::string& in_command)
    {
        reset(in_command);
    }

    void reset(const std::string& in_command)
    {
        parameters.clear();
        raw_command = in_command;

        usize index = 0;

        while(index < raw_command.size())
        {
            usize seperator_index = std::min(std::min(raw_command.find_first_of("\t", index), raw_command.find_first_of(" ", index)), raw_command.find_first_of("\n", index));
            
            std::string_view param;

            if(seperator_index != std::string::npos)
            {
                param = {raw_command.c_str() + index, seperator_index - index};
                index = seperator_index + 1;
            }
            else
            {
                param = {raw_command.c_str() + index};
                index = seperator_index;
            }
            
            
            if (param.size() <= 0)
            {
                continue;
            }

            parameters.push_back(param);
        }
    }

    std::string_view get_parameter(usize index)
    {
        return parameters[index];
    }

    std::string copy_parameter(usize index)
    {
        std::string copy;
        copy.resize(parameters[index].size());
        parameters[index].copy(copy.data(), copy.size());
        return copy;
    }

    bool has_parameter(usize index)
    {
        return parameters.size() > index;
    }

private:
    std::string raw_command;
    std::vector<std::string_view> parameters;
};
} // namespace nyxpiri::ovrpenguin

#endif // STR_CMD_HPP
