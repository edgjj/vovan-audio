#ifndef VOVAN_AUDIO_DSP_H

#include <vector>
#include <string_view>
#include <memory>
#include <unordered_map>
#include "base_command.hpp"

namespace bot
{

class dsp_voice_processor : public bot::command::base
{
public:
    dsp_voice_processor()
    {
        ;
    }
    void execute(const vk::event::message_new& event, const std::vector<std::string>& args) const override;

    template <typename Handler>
    dsp_voice_processor& on_command_dsp(std::string_view trigger)
    {
        m_commands.emplace(trigger, std::make_unique<Handler>());
        return *this;
    }
private:
    std::unordered_map<std::string, std::shared_ptr<command::base_dsp>> m_commands{};
};

}
#define VOVAN_AUDIO_DSP_H

#endif// VOVAN_AUDIO_DSP_H
