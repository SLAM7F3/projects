// ========================================================================
// Anye Li's FfmpegStream class method definitions
// ========================================================================
// Last updated on 1/17/08; 1/21/08; 1/22/08; 2/23/08; 12/4/10
// ========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "ffmpeg/FFmpegStream.h"
#include "ffmpeg/Frame.h"
#include "general/outputfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using namespace osgFFmpeg;

// Forward declarations

#define SWS_BICUBIC           4

enum PixelFormat {
    PIX_FMT_NONE= -1,
    PIX_FMT_YUV420P,   ///< Planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    PIX_FMT_YUYV422,   ///< Packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    PIX_FMT_RGB24,     ///< Packed RGB 8:8:8, 24bpp, RGBRGB...
    PIX_FMT_BGR24,     ///< Packed RGB 8:8:8, 24bpp, BGRBGR...
    PIX_FMT_YUV422P,   ///< Planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    PIX_FMT_YUV444P,   ///< Planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    PIX_FMT_RGB32,     ///< Packed RGB 8:8:8, 32bpp, (msb)8A 8R 8G 8B(lsb), in cpu endianness
    PIX_FMT_YUV410P,   ///< Planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    PIX_FMT_YUV411P,   ///< Planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    PIX_FMT_RGB565,    ///< Packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), in cpu endianness
    PIX_FMT_RGB555,    ///< Packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), in cpu endianness most significant bit to 0
    PIX_FMT_GRAY8,     ///<        Y        ,  8bpp
    PIX_FMT_MONOWHITE, ///<        Y        ,  1bpp, 0 is white, 1 is black
    PIX_FMT_MONOBLACK, ///<        Y        ,  1bpp, 0 is black, 1 is white
    PIX_FMT_PAL8,      ///< 8 bit with PIX_FMT_RGB32 palette
    PIX_FMT_YUVJ420P,  ///< Planar YUV 4:2:0, 12bpp, full scale (jpeg)
    PIX_FMT_YUVJ422P,  ///< Planar YUV 4:2:2, 16bpp, full scale (jpeg)
    PIX_FMT_YUVJ444P,  ///< Planar YUV 4:4:4, 24bpp, full scale (jpeg)
    PIX_FMT_XVMC_MPEG2_MC,///< XVideo Motion Acceleration via common packet passing(xvmc_render.h)
    PIX_FMT_XVMC_MPEG2_IDCT,
    PIX_FMT_UYVY422,   ///< Packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    PIX_FMT_UYYVYY411, ///< Packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    PIX_FMT_BGR32,     ///< Packed RGB 8:8:8, 32bpp, (msb)8A 8B 8G 8R(lsb), in cpu endianness
    PIX_FMT_BGR565,    ///< Packed RGB 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), in cpu endianness
    PIX_FMT_BGR555,    ///< Packed RGB 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), in cpu endianness most significant bit to 1
    PIX_FMT_BGR8,      ///< Packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    PIX_FMT_BGR4,      ///< Packed RGB 1:2:1,  4bpp, (msb)1B 2G 1R(lsb)
    PIX_FMT_BGR4_BYTE, ///< Packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    PIX_FMT_RGB8,      ///< Packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    PIX_FMT_RGB4,      ///< Packed RGB 1:2:1,  4bpp, (msb)2R 3G 3B(lsb)
    PIX_FMT_RGB4_BYTE, ///< Packed RGB 1:2:1,  8bpp, (msb)2R 3G 3B(lsb)
    PIX_FMT_NV12,      ///< Planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 for UV
    PIX_FMT_NV21,      ///< as above, but U and V bytes are swapped

    PIX_FMT_RGB32_1,   ///< Packed RGB 8:8:8, 32bpp, (msb)8R 8G 8B 8A(lsb), in cpu endianness
    PIX_FMT_BGR32_1,   ///< Packed RGB 8:8:8, 32bpp, (msb)8B 8G 8R 8A(lsb), in cpu endianness

    PIX_FMT_GRAY16BE,  ///<        Y        , 16bpp, big-endian
    PIX_FMT_GRAY16LE,  ///<        Y        , 16bpp, little-endian
    PIX_FMT_YUV440P,   ///< Planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    PIX_FMT_YUVJ440P,  ///< Planar YUV 4:4:0 full scale (jpeg)
    PIX_FMT_YUVA420P,  ///< Planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
    PIX_FMT_NB,        ///< number of pixel formats, DO NOT USE THIS if you want to link with shared libav* because the number of formats might differ between versions
};

enum CodecType {
    CODEC_TYPE_UNKNOWN = -1,
    CODEC_TYPE_VIDEO,
    CODEC_TYPE_AUDIO,
    CODEC_TYPE_DATA,
    CODEC_TYPE_SUBTITLE,
    CODEC_TYPE_NB
};

struct AVCodecContext;
struct AVFrame;
struct AVStream;
struct SwsContext;
struct SwsFilter;

typedef struct AVRational{
    int num; // numerator
    int den; // denominator
} AVRational;

extern "C"
{
   int avcodec_decode_video(
      AVCodecContext* avctx, AVFrame* picture,
      int* got_picture_ptr,uint8_t* buf, int buf_size);

   struct SwsContext* sws_getContext(
      int srcW, int srcH, int srcFormat, int dstW, int dstH, 
      int dstFormat, int flags,
      SwsFilter* srcFilter, SwsFilter* dstFilter, double* param);

   int sws_scale(
      struct SwsContext* context, uint8_t* src[], int srcStride[], 
      int srcSliceY,int srcSliceH, uint8_t* dst[], int dstStride[]);

   AVCodecContext* get_codec_ptr(AVStream* stream_ptr);
   enum CodecType get_codec_type(AVStream* stream_ptr);
   int get_AVCC_width(AVCodecContext* context_ptr);
   int get_AVCC_height(AVCodecContext* context_ptr);
   enum PixelFormat get_pix_fmt(AVCodecContext* context_ptr);

   uint8_t** get_avframe_data(AVFrame* frame_ptr);
   int* get_avframe_linesize(AVFrame* frame_ptr);
   int get_avframe_repeat_pict(AVFrame* frame_ptr);
   int get_keyframe_flag(AVFrame* frame_ptr);
   int64_t get_presentation_timestamp(AVFrame* frame_ptr);
   int get_coded_picture_number(AVFrame* frame_ptr);
   int get_display_picture_number(AVFrame* frame_ptr);

// Note: On 1/11/08, we empirically observed that get_time_base() can
// return frame rates which are off by 100.  So this method is
// deprecated:

//    AVRational get_time_base(AVCodecContext* context_ptr);

// Note: On 1/11/08, we empirically observed that get_bit_rate() can
// yield useless zero values.  So this method is deprecated:

//    int get_bit_rate(AVCodecContext* context_ptr);

   int get_frame_number(AVCodecContext* context_ptr);
   int get_real_pict_number(AVCodecContext* context_ptr);

   void avcodec_flush_buffers(AVCodecContext* context_ptr);
   int get_frame_bits(AVCodecContext* context_ptr);
}

// ========================================================================
void FFmpegStream::allocate_member_objects()
{
   keyframe_table_ptr=new Hashtable<double>(1000);
}		       

void FFmpegStream::initialize_member_objects()
{
   display_next_frame_flag=true;
   frame_actually_displayed_flag=false;
   loop_to_beginning=loop_to_end=false;
   n_channels=3;   //  Assume input movies are RGB rather than black/white
   curr_framenumber=-1;
   setLoopingMode(LOOPING);
   AVCodecContext_ptr=NULL;
}

FFmpegStream::FFmpegStream():
   m_VideoStream(0)
{
//   cout << "inside FFmpegStream constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();
}

// ------------------------------------------------------------------------
FFmpegStream::~FFmpegStream()
{
   quit();
   m_Parser.join();
   join();

   delete keyframe_table_ptr;
}

// ------------------------------------------------------------------------
int FFmpegStream::open(
   const char* p_Filename,AVInputFormat* p_ForceFormat,
   int p_BufSize,AVFormatParameters* p_AdditionalParameters)
{
//   cout << "inside FFmpegStream::open()" << endl;

   m_Parser.open(p_Filename, p_ForceFormat, p_BufSize,
                 p_AdditionalParameters);
   int stream_index = -1;
   for (int i = 0; i < m_Parser.num_streams(); ++i)
   {
      AVStream* stream = m_Parser.get_stream(i);
      if (get_codec_type(stream) == CODEC_TYPE_VIDEO)
      {
         stream_index = i;
         m_VideoStream = stream;
         break;
      }
   }
   if (stream_index < 0) return -1;
    
   m_Parser.connect(stream_index, &m_PacketQueue);
   m_Parser.compute_start_and_stop_times();
    
   return 0;
}

// ------------------------------------------------------------------------
int FFmpegStream::compute_video_parameters()
{
//   cout << "inside FFmpegStream::compute_video_parameters()" << endl;

   AVCodecContext_ptr = get_codec_ptr(m_VideoStream);
   int err = m_CodecContext.open_nothrow(AVCodecContext_ptr);
   if (err < 0) return err;

   width=get_AVCC_width(AVCodecContext_ptr);
   height=get_AVCC_height(AVCodecContext_ptr);

   movie_duration_in_secs=m_Parser.get_stop_time_in_secs()-
      m_Parser.get_start_time_in_secs();
   time_per_frame_in_secs=1.0/m_Parser.get_Framerate();
   n_frames=m_Parser.get_n_frames();

//   cout << "N_frames = " << n_frames << endl;

   return 0;
}

// ------------------------------------------------------------------------
void FFmpegStream::set_image_data_space(unsigned char* data)
{
//   cout << "inside FFmpegStream::set_image_data_space()" << endl;

   int GLimageDepth=1;
   GLenum internalTextureFormat=GL_RGB;
   GLenum pixelFormat = GL_RGB;
   GLenum GLtype=GL_UNSIGNED_BYTE;
   osg::Image::AllocationMode allocation_mode=osg::Image::NO_DELETE;
   int GLpacking=1;

//   setImage( 1200,1200, GLimageDepth,		// 1700 px plays in UL corner

//   setImage( 1300,1300, GLimageDepth,		// 1700 px plays in UL corner

//   setImage( 1350,1350, GLimageDepth,		// 1700 px plays in UL corner

//   setImage( 1375,1375, GLimageDepth,		// X crashes

//   setImage( 1400,1400, GLimageDepth,		// X crashes

//   cout << "1300 * 1300 * 3 = " << 1300 * 1300 * 3 << endl;
//   cout << "1350 * 1350 * 3 = " << 1350 * 1350 * 3 << endl;
//   cout << "1375 * 1375 * 3 = " << 1375 * 1375 * 3 << endl;
//   cout << "1400 * 1400 * 3 = " << 1400 * 1400 * 3 << endl;

//   setImage( 1362,1362, GLimageDepth,

//   setImage( 1365,1365, GLimageDepth,

//   setImage( 1368,1368, GLimageDepth,

// As of 1/25/08, we have empiricially determined that 1365x1365 is
// the maximum image size which does not cause OpenGL to
// catastrophically fail...

   setImage( width, height, GLimageDepth,
             internalTextureFormat,pixelFormat,GLtype,
             data, allocation_mode, GLpacking );
}

// ------------------------------------------------------------------------
void FFmpegStream::play()
{
//   cout << "inside FFmpegStream::play()" << endl;
//   cout << "isRunning() = " << isRunning() << endl;

   if (isRunning()) return;

   m_PacketQueue.reset();
   m_Parser.start();

   start();
}

// ------------------------------------------------------------------------
void FFmpegStream::rewind()
{
//   cout << "inside FFmpegStream::rewind()" << endl;
   m_PacketQueue.reset();
    
   m_Parser.start();
   start();
   cout << "at end of FFmegStream::rewind()" << endl;
}

// ------------------------------------------------------------------------
void FFmpegStream::quit()
{
   cout << "inside FFmpegStream::quit()" << endl;
   m_PacketQueue.request_abort();
}

// ------------------------------------------------------------------------
void FFmpegStream::run()
{
//   cout << "inside FFmpegStream::run()" << endl;
   
   if (!m_VideoStream) return;

// We should be able to customize this

   int sws_flags = SWS_BICUBIC; 
   struct SwsContext* sws_context_ptr;
   if (n_channels==3)
   {
      sws_context_ptr = sws_getContext(
         width,height,get_pix_fmt(AVCodecContext_ptr),s(),t(), 
         PIX_FMT_RGB24,
         sws_flags, // no flags, since we are not really scaling
         0, // no source filter
         0, // no destination filter
         0); // no params
   }
   else if (n_channels==1)
   {
      sws_context_ptr = sws_getContext(
         width,height,get_pix_fmt(AVCodecContext_ptr),s(),t(), 
         PIX_FMT_GRAY8,     
         sws_flags, // no flags, since we are not really scaling
         0, // no source filter
         0, // no destination filter
         0); // no params
   }
    
   if (!sws_context_ptr)
   {
      cout << "Couldn't initialize conversion context!" << endl;
      return;
   }
    
   int linesize[4];
   linesize[0] = s() * n_channels;
   linesize[1]=linesize[2]=linesize[3]=0;
   uint8_t* data2[4];
   data2[0] = static_cast<uint8_t *>(data());
   data2[1] = data2[2] = data2[3] = 0;

   FFWrapper::Frame frame;
   AVFrame* avframe_ptr = frame.get();

   int n_loops=0;

   double prev_timeofday=timefunc::elapsed_timeofday_time();
   while (true)
   {
      if (loop_to_beginning)
      {
         m_PacketQueue.reset();
         m_Parser.start();

         double frac=0.0;
         int64_t seek_time=
            (1-frac)*m_Parser.get_starting_time()+
            frac*m_Parser.get_stopping_time();
//            int seek_flag=1;
         int seek_flag=4;

         avcodec_flush_buffers(AVCodecContext_ptr);
         m_Parser.seek(seek_time,seek_flag);        
         loop_to_beginning=false;

         display_next_frame_flag=true;
         cout << "n_loops = " << n_loops++ << endl;
//         cout << "keyframe hashtable = " << *keyframe_table_ptr << endl;
      }

//      cout << "Inside FFmpegStream::run()" << endl;
//      cout << "display_next_frame_flag = " 
//           << display_next_frame_flag
//           << " frame_actually_displayed_flag = "
//           << frame_actually_displayed_flag << endl;
//      cout << " n_frames=" << n_frames 
//           << " display_next_frame_flag = " << display_next_frame_flag 
//           << endl;
//      cout << "loop_to_beginning = " << loop_to_beginning << endl;
//      cout << "movie_duration = " << movie_duration_in_secs
//           << " time_per_frame = " << time_per_frame_in_secs << endl;

      // what should we do if the image gets resized???

// In order to avoid race condition for seeking to beginning or end,
// simply continue within this while loop of loop_to_beginning or
// loop_to_end equals true:

      if (!display_next_frame_flag || loop_to_beginning ||
          loop_to_end)
      {
         continue;
      }
      else
      {
         display_next_frame_flag=false;
      }

      AVPacket pkt;
      int err = m_PacketQueue.get(&pkt);
      if (err)
      {
         cout << "err = " << err << endl;
         break;
      }

      int got_picture;
      int n_bytes = avcodec_decode_video(
         AVCodecContext_ptr, avframe_ptr, &got_picture, pkt.data, pkt.size);

// Store keyframe information within member Hashtable
// *keyframe_table_ptr.  Use keyframe number as independent ID and
// keyframe time as dependent variable.

      int keyframe_flag=get_keyframe_flag(avframe_ptr);
      if (keyframe_flag > 0 && curr_framenumber >= 0)
      {
         double curr_frametime=curr_framenumber*time_per_frame_in_secs;
         keyframe_table_ptr->update_key(curr_framenumber,curr_frametime);
//         cout << "keyframe_flag = " << keyframe_flag
//              << " curr frame = " << curr_framenumber
//              << " time = " << curr_frametime << endl;
      }
      
//      int64_t pts=get_presentation_timestamp(avframe_ptr);
//      int coded_picture_number=get_coded_picture_number(avframe_ptr);
//      int display_picture_number=get_display_picture_number(avframe_ptr);

//      cout << "keyframe_flag = " << keyframe_flag
//           << " curr_framenumber = " << curr_framenumber 
//           << " pkt.pts = " << pkt.pts
//           << " pkt.dts = " << pkt.dts 
//           << " pts = " << pts 
//           << " coded pic # = " << coded_picture_number
//           << " display pic # = " << display_picture_number 
//           << " curr framenumber = " << curr_framenumber 
//           << " frame time = " << curr_frametime
//           << endl;

      if (&got_picture==NULL)
      {
         cout << "****************************************************"
              << endl;
         cout << "Did NOT successfully retrieve next frame !!!" << endl;
         cout << "****************************************************"
              << endl;
         continue;
      }
      else
      {
         frame_actually_displayed_flag=true;
      }

      sws_scale(
         sws_context_ptr,
         get_avframe_data(avframe_ptr),get_avframe_linesize(avframe_ptr),
         0,height,data2,linesize);
      dirty();

      double curr_timeofday=timefunc::elapsed_timeofday_time();
      double elapsed_realtime=curr_timeofday-prev_timeofday;
      double frame_delay = basic_math::max(
         0.0,time_per_frame_in_secs-elapsed_realtime);

//      cout << "elapsed_realtime = " << elapsed_realtime
//           << " frame_delay = " << frame_delay 
//           << " time_per_frame = " << time_per_frame_in_secs << endl;
      
      microSleep(static_cast<int>(1000000 * frame_delay));
      prev_timeofday=curr_timeofday;

   } // infinite while loop

//   cout << "at end of FFmpegStream::run()" << endl;
}
