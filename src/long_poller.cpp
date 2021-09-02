#include "../hdr/long_poller.hpp"

bot::long_poller::long_poller(asio::io_context& io_context)
  : lp_(io_context)
{}

bot::message_handler& bot::long_poller::get_message_handler() noexcept
{
    return message_handler_;
}

void bot::long_poller::run()
{
    auto msg_new_cb = [this] (const vk::event::common& event) {
        try
        {
            message_handler_.process(event.get_message_new());
        } catch (std::exception& e)
        {
            spdlog::error("Exception [message_handler]: {}", e.what());
        }
    };
    lp_.on_event (vk::event::type::message_new, msg_new_cb);

    try
    {
        lp_.run();
    }
    catch (std::exception& e)
    {
        spdlog::trace ("Exception [long_poller]: {}", e.what());
    }

}