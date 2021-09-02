#include <cmath>
#include <iostream>
#include <cpp_vk_lib/runtime/net/network.hpp>
#include <cpp_vk_lib/vk/config/config.hpp>
#include <cpp_vk_lib/vk/long_poll/long_poll.hpp>
#include <cpp_vk_lib/vk/methods/basic.hpp>
#include <cpp_vk_lib/runtime/setup_logger.hpp>

#include "../hdr/msg_handler.hpp"
#include "../hdr/dsp.hpp"
#include "../hdr/premade_text.hpp"
#include "../hdr/long_poller.hpp"

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

    runtime::setup_logger(spdlog::level::level_enum::trace);
    spdlog::info("workers: {}", vk::config::num_workers());
    asio::io_context io_ctx;
    bot::long_poller lp_handler (io_ctx);

    lp_handler.get_message_handler()
        .set_prefix("вован", "старый");

    bot::dsp_voice_processor dsp_processor{};
    dsp_processor.on_command_dsp<tanh_processor>("tanh");

    std::unique_ptr <bot::command::base> dsp_base;
    dsp_base.reset ( &dsp_processor );

    lp_handler.get_message_handler()
        .on_command("dsp", dsp_base)
        .on_command<bot::premade_binary_data::help_string>("помощь")
        .on_command<bot::premade_binary_data::legend_1>("легенда")
        .on_command<bot::premade_binary_data::legend_2>("легенда2")
        .on_command<bot::premade_binary_data::snus.size(), bot::premade_binary_data::snus>("фильм")
        .on_command<bot::premade_binary_data::rubrics.size(), bot::premade_binary_data::rubrics>("рубрика")
        .on_command<bot::premade_binary_data::quotes.size(), bot::premade_binary_data::quotes>("цитата");

    lp_handler.run();

    return 0;
}