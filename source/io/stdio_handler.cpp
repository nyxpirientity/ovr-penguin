#include "stdio_handler.hpp"
#include <cstdio>
#include <iostream>


namespace nyxpiri::ovrpenguin
{

void async_print_string(const std::string& string);

StdIoHandler::StdIoHandler()
{
    auto input_buf_guard = input_buffer.write_lock();
    input_buf_guard.data().reserve(8192);
    input_buf_guard.unlock();

    auto output_buf_guard = output_buffer.write_lock();
    output_buf_guard.data().reserve(8192);
    output_buf_guard.unlock();
}

void StdIoHandler::on_stop()
{
    io_thread_enabled = false;
    io_thread.join();

    std::cout << "[info] ~ StdIoHandler: Stopping. My final message, if any, is the result of flushing the output buffer.\n";

    auto output_buf_guard = output_buffer.write_lock();

    if (not output_buf_guard.data().empty())
    {
        std::cout << output_buf_guard.data();
        output_buf_guard.data().clear();
    }
    
    std::cout.flush();

    output_buf_guard.unlock();
}

Event<std::string>& StdIoHandler::async_await_input()
{
    input_events.push_back({});
    num_input_awaits += 1;
    return *(input_events.end() - 1);
}

void StdIoHandler::async_print_string(const std::string& string)
{
    if (not is_started())
    {
        std::cout << string;
        std::cout.flush();
        return;
    }
    auto output_buf_guard = output_buffer.write_lock();
    output_buf_guard.data().append(string);
    output_buf_guard.unlock();
}

void StdIoHandler::on_start()
{
    io_thread = std::thread([this]
    {
        while(true)
        {
            auto output_buf_guard = output_buffer.write_lock();

            if (not output_buf_guard.data().empty())
            {
                std::cout << output_buf_guard.data();
                output_buf_guard.data().clear();
                std::cout.flush();
            }
        
            output_buf_guard.unlock();
            
            // we'll want to flush the output buffer before exiting 
            if (not io_thread_enabled)
            {
                break;
            }

            if (num_input_awaits > 0)
            {
                std::string input;
                std::getline(std::cin, input);
                --num_input_awaits;
        
                auto input_buf_guard = input_buffer.write_lock();
                input_buf_guard.data().append(input + "\n");
                
                input_buf_guard.unlock();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });
}

void StdIoHandler::on_tick(real delta_seconds)
{
    auto input_buf_guard = input_buffer.write_lock();

    if (not input_buf_guard.data().empty())
    {   
        usize i = -1;
        while ((i = input_buf_guard.data().find_first_of("\n")) != std::string::npos)
        {
            assert(input_events.size() > 0);
            std::string input = input_buf_guard.data().substr(0, i);
            input_events[0].broadcast(input);
            input_buf_guard.data().erase(0, i + 1);
            input_events.erase(input_events.begin());
        }
    }

    input_buf_guard.unlock();
}

} // namespace nyxpiri::ovrpenguin
