#include <cmath>
#include <iostream>
#include <runtime/include/net/network.hpp>
#include <vk/include/config/loader.hpp>
#include <vk/include/long_poll/long_poll.hpp>
#include <vk/include/methods/basic.hpp>
#include <vk/include/setup_logger.hpp>

#include "../hdr/msg_handler.hpp"
#include "../hdr/dsp.hpp"

class tanh_processor : public bot::command::base_dsp
{
public:
    tanh_processor() : bot::command::base_dsp (1) { ; }
    void execute(std::vector<float>& buf, const std::vector<float>& params) const override
    {
        for (auto& i : buf)
            i = std::tanh(params[0] * i);
    }
};

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Missing config path as argument." << std::endl;
        return -1;
    }

    vk::config::load(argv[1]);
    vk::setup_logger("trace");

    asio::io_context ctx;
    vk::long_poll api(ctx);

    spdlog::info("workers: {}", vk::config::num_workers());

    bot::message_handler msg_handler{};

    bot::dsp_voice_processor dsp_processor{};
    dsp_processor.on_command_dsp<tanh_processor>("tanh");

    std::unique_ptr <bot::command::base> dsp_base;
    dsp_base.reset ( &dsp_processor );

    msg_handler.on_command("dsp", dsp_base );
    msg_handler.set_prefix("вован", "старый");

    while (true)
    {
        auto events = api.listen();

        for (auto& event : events)
        {
            api.on_event("message_new", event, [&event, &msg_handler] {
                vk::event::message_new message_event = event.get_message_new();
                try
                {
                    msg_handler.process(std::move(message_event));
                } catch (std::exception& e)
                {
                    spdlog::error("Exception: {}", e.what());
                }
            });
        }
        api.run();
    }

    return EXIT_SUCCESS;
}