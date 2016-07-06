// ========================================================================
// Header file for Anye Li's FFmegStream class
// ========================================================================
// Last updated on 1/13/08; 1/21/08; 1/22/08
// ========================================================================

#ifndef INCLUDE_OSGFFMPEG_FFMPEGSTREAM
#define INCLUDE_OSGFFMPEG_FFMPEGSTREAM

#include <osg/ImageStream>
#include <OpenThreads/Thread>
#include "ffmpeg/CodecContext.h"
#include "ffmpeg/ParseThread.h"
#include "datastructures/Hashtable.h"

struct AVCodecContext;
struct AVFormatParameters;
struct AVInputFormat;
struct AVStream;

namespace osgFFmpeg
{

   class FFmpegStream : public osg::ImageStream,
      private OpenThreads::Thread
      {
        public:

         FFmpegStream();
         ~FFmpegStream();

// Set & get methods:

         int get_n_frames() const;
         void set_n_channels(int n);
         void set_display_next_frame_flag(bool flag);
         bool get_display_next_frame_flag() const;
         void set_frame_actually_displayed_flag(bool flag);
         bool get_frame_actually_displayed_flag() const;
         void set_loop_to_beginning(bool flag);
         bool get_loop_to_beginning() const;
         void set_loop_to_end(bool flag);
         bool get_loop_to_end() const;
         int get_width() const;
         int get_height() const;
         void set_curr_framenumber(int frame_number);

         int open(
            const char *p_Filename,
            AVInputFormat* p_ForceFormat,int p_BufSize,
            AVFormatParameters* p_AdditionalParameters);
         int compute_video_parameters();
         void set_image_data_space(unsigned char* data);

         virtual void play();
//          virtual void pause();
         virtual void rewind();
         virtual void quit();

        private:

         bool display_next_frame_flag,frame_actually_displayed_flag;
         bool loop_to_beginning,loop_to_end;
         int n_frames,n_rewound_frames,n_channels,width,height;
         int curr_framenumber;
         double time_per_frame_in_secs,movie_duration_in_secs;
         FFWrapperOT::ParseThread m_Parser;
         FFWrapperOT::PacketQueue m_PacketQueue;
         FFWrapper::CodecContext m_CodecContext;
         AVStream* m_VideoStream;
         AVCodecContext* AVCodecContext_ptr;

         Hashtable<double>* keyframe_table_ptr;

         void allocate_member_objects();
         void initialize_member_objects();
         void run();
      };

// ==========================================================================
// Inlined methods:
// ==========================================================================

   inline int FFmpegStream::get_n_frames() const
      {
         return n_frames;
      }

   inline void FFmpegStream::set_n_channels(int n)
      {
         n_channels=n;
      }

   inline void FFmpegStream::set_display_next_frame_flag(bool flag)
      {
         display_next_frame_flag=flag;
      }

   inline bool FFmpegStream::get_display_next_frame_flag() const
      {
         return display_next_frame_flag;
      }
   
   inline void FFmpegStream::set_frame_actually_displayed_flag(bool flag)
      {
         frame_actually_displayed_flag=flag;
      }

   inline bool FFmpegStream::get_frame_actually_displayed_flag() const
      {
         return frame_actually_displayed_flag;
      }

   inline void FFmpegStream::set_loop_to_beginning(bool flag)
      {
         loop_to_beginning=flag;
      }

   inline bool FFmpegStream::get_loop_to_beginning() const
      {
         return loop_to_beginning;
      }

   inline void FFmpegStream::set_loop_to_end(bool flag)
      {
         loop_to_end=flag;
      }

   inline bool FFmpegStream::get_loop_to_end() const
      {
         return loop_to_end;
      }

   inline int FFmpegStream::get_width() const
      {
         return width;
      }
   
   inline int FFmpegStream::get_height() const
      {
         return height;
      }
   
   inline void FFmpegStream::set_curr_framenumber(int frame_number)
      {
         curr_framenumber=frame_number;
      }
   

} // osgFFmpeg namespace


#endif
