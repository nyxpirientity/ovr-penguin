#ifndef STRING_COMMAND_HPP
#define STRING_COMMAND_HPP
#include "math/num_types.hpp"
#include <string>
#include <vector>
#include <optional>

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

    const std::string& get_option(usize index)
    {
        return options[index];
    }

    std::string copy_parameter(usize index)
    {
        std::string copy;
        copy.resize(parameters[index].size());
        parameters[index].copy(copy.data(), copy.size());
        return copy;
    }

    usize parameter_count() const
    {
        return parameters.size();
    }

    usize option_count() const
    {
        return options.size();
    }

    bool has_parameter(usize index) const
    {
        return parameters.size() > index;
    }

    usize find_parameter(const std::string& parameter) const
    {
        for (usize i = 0; i < parameters.size(); i++)
        {
            if (parameters[i] == parameter)
            {
                return i;
            }
        }

        return -1;
    }

        
    bool has_parameter(const std::string& parameter) const
    {
        return (find_parameter(parameter) != -1);
    }


    void set_options(const std::vector<std::string>& in_options)
    {
        options = in_options;
        options.push_back("");
        option_parameters.resize(options.size());

        for (usize i = 0; i < option_parameters.size(); i++)
        {
            option_parameters[i].clear();
        }

        usize current_option = options.size() - 1;

        for (usize i = 0; i < parameter_count(); i++)
        {
            bool is_option_param = false;

            for (usize j = 0; j < option_count(); j++)
            {
                if (get_parameter(i) == get_option(j))
                {
                    current_option = j;
                    is_option_param = true;
                    break;
                }
            }

            if (is_option_param)
            {
                continue;
            }

            option_parameters[current_option].push_back(get_parameter(i));
        }
    }

    const std::vector<std::string_view>& get_option_parameters(usize index)
    {
        return option_parameters[index];
    }

    usize get_option_parameters_index(const std::string& option)
    {
        for (usize i = 0; i < options.size(); i++)
        {
            if (options[i] == option)
            {
                return i;
            }
        }

        return -1;
    }

    const std::vector<std::string_view>* get_option_parameters(const std::string& option)
    {
        usize index = get_option_parameters_index(option);

        if (index == -1)
        {
            return nullptr;
        }

        return &get_option_parameters(index);
    }

    std::string_view get_option_parameter(const std::string& option, usize index)
    {
        const std::vector<std::string_view>* option_params = get_option_parameters(option);

        if (!option_params)
        {
            return {};
        }

        if (index >= option_params->size())
        {
            return {};
        }

        return (*option_params)[index];
    }

    std::string get_option_parameter_copy(const std::string& option, usize index)
    {
        std::string_view param = get_option_parameter(option, index);
        std::string copy;
        copy.resize(param.size());
        param.copy(copy.data(), copy.size());
        return copy;
    }

    const std::string& get_raw_command() const
    {
        return raw_command;
    }

private:
    std::string raw_command;
    std::vector<std::string_view> parameters;
    std::vector<std::string> options;
    std::vector<std::vector<std::string_view>> option_parameters;
};
} // namespace nyxpiri::ovrpenguin

#endif // STR_CMD_HPP
