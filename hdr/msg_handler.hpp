#ifndef VOVAN_MSG_HANDLER_H
#define VOVAN_MSG_HANDLER_H

#include <spdlog/spdlog.h>
#include <vk/include/events/message_new.hpp>
#include "base_command.hpp"

#include <string_view>
#include <memory>
#include <unordered_map>


namespace bot {

class message_handler
{
public:
    void process(vk::event::message_new&& event) const
    {
        spdlog::info("Message event: {} from {}", event.text(), event.peer_id());

        auto get_first = [](std::string_view str) { return (str.find(' ') == std::string::npos) ? str.data() : std::string(str.substr(0, str.find(' '))); };
        m_commands.at(get_first(event.text()))->execute(event);
    }

    template <typename Handler>
    message_handler& on_command(std::string_view trigger)
    {
        m_commands.emplace(trigger, std::make_unique<Handler>());
        return *this;
    }

    void dump_commands()
    {
        for (const auto& command : m_commands) {
            spdlog::info("bot command: {}", command.first);
        }
    }

private:
    std::unordered_map<std::string, std::shared_ptr<command::base>> m_commands{};
    // Event wrappers.
};

}// namespace bot


#endif