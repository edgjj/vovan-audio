
#include <vk/include/config/loader.hpp>
#include <vk/include/log_level.hpp>

#include <vk/include/long_poll/api.hpp>
#include <vk/include/events/message_new.hpp>
#include <vk/include/methods/basic.hpp>
#include <vk/include/methods/utility/message_constructor.hpp>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


#include "../hdr/msg_handler.hpp"
#include <iostream>
#include <random>
#include <ctime>

class pull_random final : public bot::command::base
{
public:
	pull_random() : bot::command::base (1) { ; }
	void execute (const vk::event::message_new& event, const std::vector<std::string>& args) const override
	{
		std::mt19937 gener(std::time(0));
		std::uniform_int_distribution<int> distr(0, std::stoi(args[0]));
		int r = distr(gener);
		vk::method::messages::send(event.peer_id(), std::to_string (r));
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
    vk::log_level::trace();

	asio::io_context ctx;
    vk::long_poll api (ctx);
    vk::long_poll_data data = api.server();

    spdlog::info("workers: {}", vk::config::num_workers());

	bot::message_handler msg_handler{};
	msg_handler.on_command<pull_random> ("рандом");
	msg_handler.set_prefix ("вован", "старый");
    while (true) {
        auto events = api.listen(data);

        for (auto& event : events) {
            api.on_event("message_new", event, [&event, &msg_handler] {
                vk::event::message_new message_event = event.get_message_new();
                msg_handler.process(std::forward<vk::event::message_new> (message_event));
            });
        }
        api.run();
    }
	
	return EXIT_SUCCESS;
}