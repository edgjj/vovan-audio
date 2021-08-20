#include "../hdr/opus_helper.hpp"
#include <cstring>
#include <cassert>
#define MAX_FRAME_SIZE (960 * 6)

namespace helpers
{
struct cpp_opus_mem_stream
{
    const std::vector<uint8_t>& buf;
    std::ptrdiff_t pos;
};

static int dec_read_callback(void* _stream, unsigned char *_ptr, opus_int32 _nbytes)
{
    if (_nbytes <= 0)
        return 0;

    cpp_opus_mem_stream* data = reinterpret_cast<cpp_opus_mem_stream*>(_stream);
    if (data->pos >= data->buf.size())
        return 0;

    int _buf_size = std::min<int>(data->buf.size() - data->pos, _nbytes);

    std::memcpy(_ptr, data->buf.data() + data->pos, _buf_size);
    data->pos += _nbytes;
    return _buf_size;
}

static int dec_seek_callback(void* _stream, opus_int64 _offset, int _whence)
{
    cpp_opus_mem_stream* stream = reinterpret_cast<cpp_opus_mem_stream*>(_stream);
    std::ptrdiff_t pos = stream->pos;
    assert (pos >= 0);
    switch (_whence)
    {
        case SEEK_SET:
        {
            /*Check for overflow:*/
            if (_offset < 0 || _offset > PTRDIFF_MAX)
                return -1;
            pos = _offset;
            break;
        }
        case SEEK_CUR:
        {
            /*Check for overflow:*/
            if (_offset < -pos || _offset > PTRDIFF_MAX - pos)
                return -1;
            pos = pos + _offset;
            break;
        }
        case SEEK_END:
        {
            std::ptrdiff_t size = stream->buf.size();
            assert (size >= 0);
            /*Check for overflow:*/
            if (_offset < -size || _offset > PTRDIFF_MAX - size)
                return -1;
            pos = size + _offset;
            break;
        }
        default:
            return -1;
    }
    stream->pos = pos;
    return 0;
}

static opus_int64 dec_tell_callback(void *_stream)
{
    cpp_opus_mem_stream* data = reinterpret_cast<cpp_opus_mem_stream*>(_stream);
    return (opus_int64)data->pos;
}

static int enc_write_callback(void* _stream, const unsigned char* ptr, opus_int32 len)
{
    std::vector<uint8_t>* data = reinterpret_cast<std::vector<uint8_t>*>(_stream);
    data->insert(data->end(), ptr, ptr + len);
    return false;
}

static int enc_read_callback(void* user_data)
{
    return false;
}

using opus_exception = std::runtime_error;

cpp_ogg_opus_decoder::cpp_ogg_opus_decoder(const std::vector<uint8_t>& data)
{
    OpusFileCallbacks cb = { dec_read_callback, dec_seek_callback, dec_tell_callback, NULL};
    cpp_opus_mem_stream mem_stream = { data, 0 };

    opus_file_impl.reset( op_open_callbacks( reinterpret_cast<void*>(&mem_stream), &cb, NULL, NULL, &error ) );

    if (error != OPUS_OK)
        throw opus_exception ("Failed to init decoder: " + decode_error(error));
}


void cpp_ogg_opus_decoder::perform_decode(std::vector<float>& buffer)
{
    std::unique_ptr<float[]> audio_frame_dec(new float[MAX_FRAME_SIZE]);
    int frame_size = 0;
    while (true)
    {
        frame_size = op_read_float(opus_file_impl.get(), audio_frame_dec.get(), MAX_FRAME_SIZE, NULL);
        if (frame_size == 0)
            break;
        if (frame_size < 0)
            throw opus_exception("decoder failed: " + decode_error(frame_size));

        buffer.insert(buffer.end(), audio_frame_dec.get(), audio_frame_dec.get() + frame_size);
    }
}

std::string cpp_ogg_opus_decoder::decode_error(const int& err_code)
{
    switch (err_code)
    {
        case OP_FALSE:
            return "A request did not succeed.";
        case OP_HOLE:
            return "There was a hole in the page sequence numbers (e.g., a page was corrupt or missing).";
        case OP_EREAD:
            return "An underlying read, seek, or tell operation failed when it should have succeeded, or we failed to find data in the stream we had seen before.";
        case OP_EFAULT:
            return "There was a memory allocation failure, or an internal library error.";
        case OP_EIMPL:
            return "The stream used a feature that is not implemented, such as an unsupported channel family.";
        case OP_EINVAL:
            return "seek() was implemented and succeeded on this source, but tell() did not, or the starting position indicator was not equal to _initial_bytes.";
        case OP_ENOTFORMAT:
            return "The stream contained a link that did not have any logical Opus streams in it.";
        case OP_EBADHEADER:
            return "A required header packet was not properly formatted, contained illegal values, or was missing altogether.";
        case OP_EVERSION:
            return "An ID header contained an unrecognized version number.";
        case OP_EBADLINK:
            return "We failed to find data we had seen before after seeking.";
        case OP_EBADTIMESTAMP:
            return "The first or last timestamp in a link failed basic validity checks.z";
        default:
            return "Unknown error: " + std::to_string(err_code);
    }
}

cpp_ogg_opus_encoder::cpp_ogg_opus_encoder(std::vector<uint8_t>& buffer)
{
    opus_enc_impl.reset ( new _cpp_ogg_opus_encoder() );

    OpusEncCallbacks cb = { enc_write_callback, enc_read_callback};

    opus_enc_impl->comments = ope_comments_create();
    opus_enc_impl->enc = ope_encoder_create_callbacks(&cb, &buffer, opus_enc_impl->comments, 48000, 1, 0, &error);

    if (error != OPUS_OK)
        throw opus_exception ("Failed to init encoder: " + decode_error(error));
}

void cpp_ogg_opus_encoder::set_option(const int& request, const int& value)
{
    ope_encoder_ctl (opus_enc_impl->enc, request, value);
}

void cpp_ogg_opus_encoder::perform_encode(const std::vector<float>& buffer)
{
    int frame_size = ope_encoder_write_float(opus_enc_impl->enc, buffer.data(), buffer.size());

    if (frame_size < 0)
        throw opus_exception ("Encoder failed: " + decode_error(frame_size));

    ope_encoder_drain(opus_enc_impl->enc);
}

std::string cpp_ogg_opus_encoder::decode_error(const int& err_code)
{
    switch (err_code)
    {
        case OPE_BAD_ARG:
            return "One or more invalid/out of range arguments.";
        case OPE_INTERNAL_ERROR:
            return "An internal error was detected.";
        case OPE_UNIMPLEMENTED:
            return "Invalid/unsupported request number.";
        case OPE_ALLOC_FAIL:
            return "Memory allocation has failed.";
        case OPE_CANNOT_OPEN:
            return "Cannot open stream.";
        case OPE_TOO_LATE:
            return "Too late.";
        case OPE_INVALID_PICTURE:
            return "Invalid picture.";
        case OPE_INVALID_ICON:
            return "Invalid picture";
        case OPE_WRITE_FAIL:
            return "Stream write fail.";
        case OPE_CLOSE_FAIL:
            return "Stream close fail.";
        default:
            return "Unknown error: " + std::to_string(err_code);
    }
}

}