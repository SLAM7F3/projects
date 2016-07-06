// ==========================================================================
// Definitions for C set/get wrapper methods
// ==========================================================================
// Last updated on 1/11/08; 1/13/08; 1/17/08; 1/23/08
// ==========================================================================

#include <stdio.h>
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/avio.h>
#include "ffmpeg/setget.h"

// ==========================================================================
// AVFormatContext methods
// ==========================================================================

int get_flags(const AVFormatContext* ic)
{
   return ic->flags;
}

void set_flags(AVFormatContext* ic,int flags)
{
   ic->flags=flags;
}

/*
AVStream* get_curr_stream(AVFormatContext* context_ptr)
{
   return context_ptr->cur_st;
}
*/

AVStream* get_curr_stream(const AVFormatContext* context_ptr)
{
   return context_ptr->cur_st;
}

AVStream* get_indexed_stream(const AVFormatContext* ic,int p_Index)
{
   return ic->streams[p_Index];
}

unsigned int get_nb_streams(const AVFormatContext* ic)
{
   return ic->nb_streams;
}

int get_eof_reached(ByteIOContext* BIOC_ptr)
{
   return BIOC_ptr->eof_reached;
}

/*
int get_error(ByteIOContext* BIOC_ptr)
{
   return BIOC_ptr->error;
}
*/

void set_error(ByteIOContext* BIOC_ptr,int* m_error_ptr)
{
   *m_error_ptr=BIOC_ptr->error;
}

int64_t get_start_time(const AVFormatContext* ic)
{
   return ic->start_time;
}

int64_t get_duration(const AVFormatContext* ic)
{
   return ic->duration;
}

int64_t get_n_frames(const AVFormatContext* context_ptr)
{
   AVStream* curr_stream_ptr=get_curr_stream(context_ptr);
   if (curr_stream_ptr==NULL)
   {
      return -1;
   }
   else
   {
      return get_curr_stream(context_ptr)->nb_frames;
   }
}

int64_t get_file_size(const AVFormatContext* context_ptr)
{
   return context_ptr->file_size;
}

int get_bit_rate(const AVFormatContext* context_ptr)
{
   return context_ptr->bit_rate;
}

// ==========================================================================
// AVCodecContext methods
// ==========================================================================

int get_AVCC_flags(const AVCodecContext* ic)
{
   return ic->flags;
}

void set_AVCC_flags(AVCodecContext* ic,int flags)
{
   ic->flags=flags;
}

int get_AVCC_flags2(const AVCodecContext* ic)
{
   return ic->flags2;
}

void set_AVCC_flags2(AVCodecContext* ic,int flags2)
{
   ic->flags2=flags2;
}

enum CodecID get_codec_ID(AVCodecContext* avcc_ptr)
{
   return avcc_ptr->codec_id;
}

void set_debug_mv(AVCodecContext* context_ptr,int dmv)
{
   context_ptr->debug_mv = dmv;
}

void set_debug(AVCodecContext* context_ptr,int dbg)
{
   context_ptr->debug = dbg;
}

void set_workaround_bugs(AVCodecContext* context_ptr,int wab)
{
   context_ptr->workaround_bugs = wab;
}

int get_lowres(AVCodecContext* context_ptr)
{
   return context_ptr->lowres;
}

void set_idct_algo(AVCodecContext* context_ptr,int idct_algo)
{
   context_ptr->idct_algo=idct_algo;
}

void set_skip_frame(AVCodecContext* context_ptr,enum AVDiscard skip_frame)
{
   context_ptr->skip_frame=skip_frame;
}

void set_skip_idct(AVCodecContext* context_ptr,enum AVDiscard skip_idct)
{
   context_ptr->skip_idct=skip_idct;
}

void set_skip_loop_filter(
   AVCodecContext* context_ptr,enum AVDiscard skip_loop_filter)
{
   context_ptr->skip_loop_filter=skip_loop_filter;
}

/*
void set_error_resilience(
   AVCodecContext* context_ptr,int error_resilience)
{
   context_ptr->error_resilience = error_resilience;
}

void set_error_concealment(
   AVCodecContext* context_ptr,int error_concealment)
{
   context_ptr->error_resilience = error_concealment;
}
*/

void set_thread_count(AVCodecContext* context_ptr,int thread_count)
{
   context_ptr->thread_count = thread_count;
}

int get_AVCC_width(AVCodecContext* context_ptr)
{
   return context_ptr->width;
}

int get_AVCC_height(AVCodecContext* context_ptr)
{
   return context_ptr->height;
}

enum PixelFormat get_pix_fmt(AVCodecContext* context_ptr)
{
   return context_ptr->pix_fmt;
}

AVRational get_time_base(AVCodecContext* context_ptr)
{
   return context_ptr->time_base;
}

int get_frame_number(AVCodecContext* context_ptr)
{
   return context_ptr->frame_number;
}

int get_real_pict_number(AVCodecContext* context_ptr)
{
   return context_ptr->real_pict_num;
}

int get_AVCC_bit_rate(AVCodecContext* context_ptr)
{
   return context_ptr->bit_rate;
}

int get_frame_bits(AVCodecContext* context_ptr)
{
   return context_ptr->frame_bits;
}

// ==========================================================================
// AVStream methods
// ==========================================================================

AVCodecContext* get_codec_ptr(AVStream* stream_ptr)
{
   return stream_ptr->codec;
}

enum CodecType get_codec_type(AVStream* stream_ptr)
{
   return stream_ptr->codec->codec_type;
}

double get_frame_rate(AVStream* stream_ptr)
{
   double numer=stream_ptr->r_frame_rate.num;
   double denom=stream_ptr->r_frame_rate.den;
   double frame_rate=numer/denom;
//   printf("numer = %f denom = %f \n",numer,denom);
   return frame_rate;
}

// ==========================================================================
// AVFrame methods
// ==========================================================================

uint8_t** get_avframe_data(AVFrame* frame_ptr)
{
   return frame_ptr->data;
}

int* get_avframe_linesize(AVFrame* frame_ptr)
{
   return frame_ptr->linesize;
}

int get_avframe_repeat_pict(AVFrame* frame_ptr)
{
   return frame_ptr->repeat_pict;
}

int get_keyframe_flag(AVFrame* frame_ptr)
{
   return frame_ptr->key_frame;
}

int64_t get_presentation_timestamp(AVFrame* frame_ptr)
{
   return frame_ptr->pts;
}

int get_coded_picture_number(AVFrame* frame_ptr)
{
   return frame_ptr->coded_picture_number;
}

int get_display_picture_number(AVFrame* frame_ptr)
{
   return frame_ptr->display_picture_number;
}

