#include "../hdr/dsp.hpp"
#include "../hdr/opus_helper.hpp"

#include <runtime/include/net/network.hpp>
#include <simdjson.h>
#include <vector>
#include <vk/include/methods/basic.hpp>
#include <vk/include/methods/utility/constructor.hpp>
#include <vk/include/events/message_new.hpp>


namespace bot {

void dsp_voice_processor::execute(const vk::event::message_new& event, const std::vector<std::string>& args) const
{
    if (m_commands.find(args[0]) == m_commands.end())
        return;

    auto cmd = m_commands.at(args[0]);
    if (args.size() - 1 < cmd->get_expected_args())
    {
        vk::method::messages::send(event.peer_id(), "[dsp_voice_processor] Not enough parameters.");
        return;
    }

    std::vector<float> args_float;

    try
    {
        std::transform(args.begin() + 1, args.end(), std::back_inserter(args_float), [](const std::string& str) {
            return std::stof(str);
        });
    } catch (const std::exception& e)
    {
        vk::method::messages::send(event.peer_id(), "Invalid parameters.");
        return;
    }


    for (auto&& attachment : event.reply()->attachments())
    {
        if (attachment->type() != "audio_message")
        {
            continue;
        }

        auto audio_message = vk::attachment::cast<vk::attachment::audio_message>(attachment);

        std::vector<uint8_t> audio;
        if (runtime::network::download(audio, audio_message->raw_ogg()) == 0)
        {
            int error = 0;

            std::vector<float> audio_decoded;
            helpers::cpp_ogg_opus_decoder decoder(audio);
            decoder.perform_decode(audio_decoded);

            audio.clear();

            cmd->execute(audio_decoded, args_float);
            helpers::cpp_ogg_opus_encoder encoder(audio);

            encoder.set_option(OPUS_SET_VBR(true));
            encoder.set_option(OPUS_SET_BITRATE(16 * 1000));

            encoder.perform_encode(audio_decoded);

            simdjson::dom::parser p;
            std::string res = vk::method::group_constructor()
                                  .method("docs.getMessagesUploadServer")
                                  .param("peer_id", std::to_string(event.peer_id()))
                                  .param("type", "audio_message")
                                  .perform_request();

            std::string res2 = runtime::network::upload("file", audio, p.parse(res)["response"]["upload_url"]);

            std::string res3 = vk::method::group_constructor().method("docs.save").param("file", p.parse(res2)["file"]).perform_request();

            simdjson::dom::element parsed3 = p.parse(res3)["response"]["audio_message"];

            std::vector<vk::attachment::attachment_ptr_t> vec;
            vec.push_back(vk::attachment::attachment_ptr_t(
                new vk::attachment::document(parsed3["owner_id"].get_int64().value(), parsed3["id"].get_int64().value(), "")));

            vk::method::messages::send(event.peer_id(), "", vec);
        }
    }
}
}