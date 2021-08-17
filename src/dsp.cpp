#include "../hdr/dsp.hpp"

#include <opus/opus.h>
#include <opus/opusenc.h>
#include <opus/opusfile.h>
#include <runtime/include/net/network.hpp>
#include <simdjson.h>
#include <vector>
#include <vk/include/methods/basic.hpp>
#include <vk/include/methods/utility/constructor.hpp>
#include <vk/include/events/message_new.hpp>

#define MAX_FRAME_SIZE (960 * 6)
namespace bot {

static int write_callback(void* user_data, const unsigned char* ptr, opus_int32 len)
{
    std::vector<char>* data = reinterpret_cast<std::vector<char>*>(user_data);
    data->insert(data->end(), ptr, ptr + len);
    return false;
}

static int close_callback(void* user_data)
{
    return false;
}

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
    std::transform(args.begin() + 1, args.end(), std::back_inserter(args_float),
                   [](const std::string& str) {
                       return std::stof(str);
                   });

    for (auto&& attachment : event.reply()->attachments())
    {
        if (attachment->type() != "audio_message")
        {
            continue;
        }

        auto audio_message = vk::attachment::cast<vk::attachment::audio_message>(attachment);

        std::vector<char> audio;
        if (runtime::network::download(&audio, audio_message->raw_ogg()) == 0)
        {
            int error = 0;

            std::vector<float> audio_decoded;
            int frame_size = 0;

            OggOpusFile* stream = op_open_memory(reinterpret_cast<const unsigned char*>(audio.data()), audio.size(), &error);
            std::unique_ptr<float[]> audio_frame_dec(new float[MAX_FRAME_SIZE]);

            if (error < 0)
                throw std::runtime_error("[dsp_voice_processor] failed to create decoder");

            while (true)
            {
                frame_size = op_read_float(stream, audio_frame_dec.get(), MAX_FRAME_SIZE, NULL);
                if (frame_size == 0)
                    break;
                if (frame_size < 0)
                {
                    op_free(stream);
                    throw std::runtime_error("[dsp_voice_processor] decoder failed: " + std::to_string(frame_size));
                }
                audio_decoded.insert(audio_decoded.end(), audio_frame_dec.get(), audio_frame_dec.get() + frame_size);
            }

            op_free(stream);
            audio.clear();

            cmd->execute(audio_decoded, args_float);

            OpusEncCallbacks callbacks = {write_callback, close_callback};
            OggOpusComments* comments = ope_comments_create();
            OggOpusEnc* enc = ope_encoder_create_callbacks(&callbacks, &audio, comments, 48000, 1, 0, &error);

            ope_encoder_ctl(enc, OPUS_SET_VBR(true));
            ope_encoder_ctl(enc, OPUS_SET_BITRATE(16 * 1000));

            frame_size = ope_encoder_write_float(enc, audio_decoded.data(), audio_decoded.size());
            if (frame_size < 0)
            {
                ope_encoder_destroy(enc);
                ope_comments_destroy(comments);
                throw std::runtime_error("[dsp_voice_processor] encoder failed: " + std::to_string(frame_size));
            }

            ope_encoder_drain(enc);
            ope_encoder_destroy(enc);
            ope_comments_destroy(comments);

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