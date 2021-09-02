#ifndef VOVAN_AUDIO_LONG_POLLER_HPP
#define VOVAN_AUDIO_LONG_POLLER_HPP

#include <cpp_vk_lib/vk/long_poll/long_poll.hpp>
#include "../hdr/msg_handler.hpp"

namespace bot {

class long_poller
{
public:
    long_poller(asio::io_context& io_context);

    message_handler& get_message_handler() noexcept;
    void run();

private:
    vk::long_poll lp_;
    message_handler message_handler_{};
};

}// namespace bot


#endif// VOVAN_AUDIO_LONG_POLLER_HPP
