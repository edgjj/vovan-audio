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
    base() { ; }
    base(int arg_cnt) : expected_args (arg_cnt) { ; }
    virtual void execute(const vk::event::message_new& event, const std::vector<std::string>& args) const = 0; 
    size_t get_expected_args() { return expected_args; }
    bool is_variable_args() { return expected_args == 0;}
    virtual ~base() = default;
protected:
    size_t expected_args = 0;
};

class base_dsp : public base
{
public:
    base_dsp(int arg_cnt) : base (arg_cnt) { ; }

    void execute(const vk::event::message_new& event, const std::vector<std::string>& args) const override { ; }
    virtual void execute(std::vector<float>& buf, const std::vector<float>& params) const = 0;
};

}// namespace command
}// namespace bot

#endif// BOT_COMMANDS_BASE_HPP