#ifndef VOVAN_MSG_HANDLER_H
#define VOVAN_MSG_HANDLER_H

#include <spdlog/spdlog.h>
#include <vk/include/methods/basic.hpp>
#include <vk/include/events/message_new.hpp>
#include "base_command.hpp"

#include <string_view>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace bot {

class message_handler
{
public:
    void process(vk::event::message_new&& event) const
    {
        spdlog::info("Message event: {} from {}", event.text(), event.peer_id());

        auto tokens = tokenize (event.text(), std::string (" "));
        bool has_pfx = !m_prefixes.empty();

        spdlog::trace ("Tokens count: {}", tokens.size());

        if (tokens.size() < 1 + has_pfx)
            return;

        if (has_pfx && m_prefixes.find(tokens[0]) == m_prefixes.end())
            return;

        if (m_commands.find(tokens[has_pfx]) == m_commands.end())
            return;

        std::vector<std::string> args (tokens.begin() + 1 + has_pfx, tokens.end());
        
        auto cmd = m_commands.at(tokens[has_pfx]);

        if (!cmd->is_variable_args() && args.size() < cmd->get_expected_args())
            vk::method::messages::send(event.peer_id(), "Not enough arguments.");
        else
            cmd->execute(event, args);    
    }

    template <typename Handler>
    message_handler& on_command(std::string_view trigger)
    {
        m_commands.emplace(trigger, std::make_unique<Handler>());
        return *this;
    }

    template <typename Handler>
    message_handler& on_command (std::string_view trigger, Handler& handler)
    {
        m_commands[trigger.data()] = std::move (handler);
        return *this;
    }

    template <typename... Prefixes>
    void set_prefix(Prefixes... pfx)
    {
        std::string pfxs[] = { std::string(pfx)... };
        m_prefixes.insert ( std::make_move_iterator(std::begin(pfxs)), std::make_move_iterator(std::end(pfxs)) );
    }

    void dump_commands()
    {
        for (const auto& command : m_commands) {
            spdlog::info("bot command: {}", command.first);
        }
    }

    void dump_prefixes()
    {
        for (const auto& pfx : m_prefixes) {
            spdlog::info("bot prefix: {}", pfx);
        }
    }
private:
    std::vector<std::string> tokenize(std::string&& str, std::string_view delim) const
    {
        size_t pos = 0;
        std::vector<std::string> out;
        while ((pos = str.find(delim)) != std::string::npos) {
            out.push_back( str.substr(0, pos) );
            str.erase(0, pos + delim.length());
        }
        out.push_back(str);
        return out;
    }

    std::unordered_map<std::string, std::shared_ptr<command::base>> m_commands{};
    // Event wrappers.

    std::unordered_set<std::string> m_prefixes{};
    // Prefixes.
};

}// namespace bot


#endif