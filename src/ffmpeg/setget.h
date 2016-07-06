// ==========================================================================
// Header file for C set/get wrapper methods
// ==========================================================================
// Last updated on 1/11/08; 1/13/08; 1/17/08; 1/22/08
// ==========================================================================

#ifndef SETGET_C_H
#define SETGET_C_H

// #include <stdint.h>

#ifdef __cplusplus
extern "C" 
{
#endif
#include <ffmpeg/avcodec.h>
#ifdef __cplusplus
}
#endif
   
// Forward declarations

struct AVFrame;
struct AVFormatContext;
struct AVStream;
struct ByteIOContext;
struct CodecOptions;

// AVFormatContext methods

int get_flags(const AVFormatContext* ic);
void set_flags(AVFormatContext* ic,int flags);

unsigned int get_nb_streams(const AVFormatContext* ic);
//AVStream* get_curr_stream(AVFormatContext* context_ptr);
AVStream* get_curr_stream(const AVFormatContext* context_ptr);
AVStream* get_indexed_stream(const AVFormatContext* ic,int p_Index);

int get_eof_reached(ByteIOContext* BIOC_ptr);
void set_error(ByteIOContext* BIOC_ptr,int* m_error_ptr);

int64_t get_start_time(const AVFormatContext* ic);
int64_t get_duration(const AVFormatContext* ic);
int64_t get_n_frames(const AVFormatContext* context_ptr);
int64_t get_file_size(const AVFormatContext* context_ptr);
int get_bit_rate(const AVFormatContext* context_ptr);

// AVCodecContext methods

int get_AVCC_flags(const AVCodecContext* ic);
void set_AVCC_flags(AVCodecContext* ic,int flags);
int get_AVCC_flags2(const AVCodecContext* ic);
void set_AVCC_flags2(AVCodecContext* ic,int flags2);

enum CodecID get_codec_ID(AVCodecContext* avcc_ptr);
void set_debug_mv(AVCodecContext* context_ptr,int dmv);
void set_debug(AVCodecContext* context_ptr,int dbg);
void set_workaround_bugs(AVCodecContext* context_ptr,int wab);
int get_lowres(AVCodecContext* context_ptr);
void set_idct_algo(AVCodecContext* context_ptr,int idct_algo);
void set_skip_frame(AVCodecContext* context_ptr,enum AVDiscard skip_frame);
void set_skip_idct(AVCodecContext* context_ptr,enum AVDiscard skip_idct);
void set_skip_loop_filter(
   AVCodecContext* context_ptr,enum AVDiscard skip_loop_filter);
void set_error_resilience(
   AVCodecContext* context_ptr,int error_resilience);
void set_error_concealment(
   AVCodecContext* context_ptr,int error_concealment);
void set_thread_count(AVCodecContext* context_ptr,int thread_count);

int get_AVCC_width(AVCodecContext* context_ptr);
int get_AVCC_height(AVCodecContext* context_ptr);
int get_AVCC_channels(AVCodecContext* context_ptr);
enum PixelFormat get_pix_fmt(AVCodecContext* context_ptr);   

AVRational get_time_base(AVCodecContext* context_ptr);
int get_frame_number(AVCodecContext* context_ptr);
int get_real_pict_number(AVCodecContext* context_ptr);
int get_AVCC_bit_rate(AVCodecContext* context_ptr);
int get_frame_bits(AVCodecContext* context_ptr);

// AVStream methods

AVCodecContext* get_codec_ptr(AVStream* stream_ptr);
enum CodecType get_codec_type(AVStream* stream_ptr);
double get_frame_rate(AVStream* stream_ptr);

// AVFrame methods

uint8_t** get_avframe_data(AVFrame* frame_ptr);
int* get_avframe_linesize(AVFrame* frame_ptr);
int get_avframe_repeat_pict(AVFrame* frame_ptr);
int get_keyframe_flag(AVFrame* frame_ptr);
int64_t get_presentation_timestamp(AVFrame* frame_ptr);
int get_coded_picture_number(AVFrame* frame_ptr);
int get_display_picture_number(AVFrame* frame_ptr);


#endif  




