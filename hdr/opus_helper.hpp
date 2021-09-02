#ifndef VOVAN_AUDIO_OPUS_HELPER_HPP
#define VOVAN_AUDIO_OPUS_HELPER_HPP

#include <opus/opusfile.h>
#include <opus/opusenc.h>

#include <vector>
#include <memory>
#include <cstdint>

namespace helpers
{
class cpp_ogg_opus_decoder
{
public:
    cpp_ogg_opus_decoder(const std::vector<uint8_t>& data);

    void perform_decode(std::vector<float>& buffer);
private:
    std::string decode_error (const int& err_code);
    struct ogg_opus_file_deleter
    {
        void operator()(OggOpusFile* st)
        {
            op_free(st);
        }
    };
    std::unique_ptr<OggOpusFile, ogg_opus_file_deleter> opus_file_impl;
    int error = 0;
};

class cpp_ogg_opus_encoder
{
public:
    cpp_ogg_opus_encoder(std::vector<uint8_t>& buffer);

    void set_option (const int& request, const int& value);
    void perform_encode(const std::vector<float>& buffer);
private:
    std::string decode_error(const int& err_code);

    struct _cpp_ogg_opus_encoder
    {
        OggOpusComments* comments;
        OggOpusEnc* enc;
    };

    struct ogg_opus_enc_deleter
    {
        void operator()(_cpp_ogg_opus_encoder* st)
        {
            ope_encoder_destroy(st->enc);
            ope_comments_destroy(st->comments);
            delete st;
        }
    };
    std::unique_ptr<_cpp_ogg_opus_encoder, ogg_opus_enc_deleter> opus_enc_impl;
    int error = 0;
};

}

#endif// VOVAN_AUDIO_OPUS_HELPER_HPP
