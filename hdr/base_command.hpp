#ifndef VOVAN_COMMANDS_BASE_HPP
#define VOVAN_COMMANDS_BASE_HPP

namespace vk {
namespace event {
class message_new;
}// namespace event
}// namespace vk

namespace bot {
namespace command {

class base
{
public:
    virtual void execute(const vk::event::message_new& event) const = 0;
    virtual ~base() = default;
};

}// namespace command
}// namespace bot

#endif// BOT_COMMANDS_BASE_HPP