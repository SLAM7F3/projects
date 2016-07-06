// ========================================================================
// ParseThread member function definitions
// ========================================================================
// Last updated on 1/17/08; 1/23/08; 1/24/08
// ========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "general/outputfuncs.h"
#include "ffmpeg/ParseThread.h"

using std::cout;
using std::endl;
using namespace FFWrapperOT;

// Forward declarations

#define AVFMT_FLAG_GENPTS       0x0001 ///< generate pts if missing even if it requires parsing future frames

struct ByteIOContext;

enum CodecType {
    CODEC_TYPE_UNKNOWN = -1,
    CODEC_TYPE_VIDEO
};

extern "C"
{
   int av_find_stream_info(AVFormatContext* input_context_ptr);
   int av_read_frame(AVFormatContext* s, AVPacket* pkt);
   int av_read_pause(AVFormatContext* s);
   int av_seek_frame(
      AVFormatContext* s, int stream_index, int64_t timestamp, int flags);

   int64_t av_gettime(void);

   void dump_format(
      AVFormatContext* input_context_ptr,int index,const char* url,
      int is_output);

   int get_flags(const AVFormatContext* input_context_ptr);
   void set_flags(AVFormatContext* input_context_ptr,int flags);
   int get_eof_reached(ByteIOContext* BIOC_ptr);
   void set_error(ByteIOContext* BIOC_ptr,int* m_error_ptr);

   int64_t get_start_time(const AVFormatContext* input_context_ptr);
   int64_t get_duration(const AVFormatContext* input_context_ptr);
   int64_t get_n_frames(const AVFormatContext* input_context_ptr);
   int64_t get_file_size(const AVFormatContext* context_ptr);
   int get_bit_rate(const AVFormatContext* context_ptr);
   double get_frame_rate(AVStream* stream_ptr);

   enum CodecType get_codec_type(AVStream* stream_ptr);
}

// ========================================================================
void ParseThread::allocate_member_objects()
{
}		       

void ParseThread::initialize_member_objects()
{
   m_Error=0;
   m_GeneratePts=false;
   m_ShowStatus=true;

   m_SeekRequested=false;
//   m_SeekStream=0;

   stream_index=-1;
}

ParseThread::ParseThread()
{
//   cout << "inside ParseThread constructor" << endl;

   allocate_member_objects();
   initialize_member_objects();

//   m_SeekFlags=1;	// Seek backward
//   m_SeekFlags=2;	// Seek based on position in bytes
//   m_SeekFlags=4;	// Seek to any frame, even non keyframes
}

// ------------------------------------------------------------------------
// Member function open

int ParseThread::open(
   const char *p_Filename,
   AVInputFormat *p_ForceFormat,int p_BufSize,
   AVFormatParameters *p_AdditionalParameters)
{
//   cout << "inside ParseThread::open()" << endl;

   ScopedLock input_lock(m_InputMutex);
   typedef std::vector<PacketQueue *>::size_type Size;

   int err = 0;
   err = m_InputFile.open_nothrow(
      p_Filename, p_ForceFormat, p_BufSize, p_AdditionalParameters);
   if (err < 0) return err;

   input_context_ptr = m_InputFile.get_context();
   if (generate_pts())
   {
      set_flags(input_context_ptr,get_flags(
         input_context_ptr) | AVFMT_FLAG_GENPTS);
   }
   err = av_find_stream_info(input_context_ptr);

   if (err < 0)
   {
      return err;
   }
    
   {
      ScopedLock dest_lock(m_DestinationsMutex);
      m_Destinations.assign(
         static_cast<Size>(get_nb_streams(input_context_ptr)), 0);
   }

   if (show_status())
   {
      dump_format(input_context_ptr, 0, p_Filename, 0);
//       std::cerr<<FFUtil::dump_stream_info(input_context_ptr);
   }

   return 0;
}

// ------------------------------------------------------------------------
// Member function compute_start_and_stop_times retrieves the movie's
// start & stop times in microseconds from an AVFormatContext object.
// It converts these values into seconds and stores the results within
// members start[stop]_time_in_secs.

void ParseThread::compute_start_and_stop_times()
{
//   cout << "inside ParseThread::compute_start_and_stop_times()" << endl;

// Extract movie's starting and stopping times measured in seconds:

   starting_time=get_start_time(input_context_ptr);
   int64_t time_duration=get_duration(input_context_ptr);
   stopping_time=starting_time+time_duration;

   const int AV_TIME_BASE=1000000;
   start_time_in_secs=double(starting_time)/double(AV_TIME_BASE);
   stop_time_in_secs=double(stopping_time)/double(AV_TIME_BASE);
   double total_duration_in_secs=stop_time_in_secs-start_time_in_secs;

   int64_t file_size=get_file_size(input_context_ptr);
   int bit_rate=get_bit_rate(input_context_ptr);
   byte_rate=bit_rate/8;

   cout << "start time = " << start_time_in_secs << " secs    " 
        << " stop time = " << stop_time_in_secs << " secs" << endl;
   cout << "file_size = " << file_size << " bit_rate = " << bit_rate
        << endl;
   cout << "byte_rate = " << byte_rate << endl;

// As of Jan 2008, we assume that ParseThread is handling only a
// single video which may or may not contain audio.  So we let
// stream_index range from 0 to 1 and assume that one of these two
// corresponds to a genuine video stream:

   cout << "stream_index = " << stream_index << endl;
   AVStream* stream_ptr=get_indexed_stream(input_context_ptr,stream_index);
   if (get_codec_type(stream_ptr)==CODEC_TYPE_VIDEO)
   {
      frame_rate=get_frame_rate(stream_ptr);
      cout << "frame rate = " << frame_rate << endl;
      n_frames= basic_math::mytruncate(frame_rate*total_duration_in_secs);
      cout << "n_frames = " << n_frames << endl;
   }


//   int secs  = time_duration / AV_TIME_BASE;
//   double dsecs = double(time_duration % AV_TIME_BASE)/AV_TIME_BASE;
//   int mins  = secs / 60;
//   secs %= 60;
//   int hours = mins / 60;
//   mins %= 60;

//   cout << "hours = " << hours << " mins = " << mins
//        << " secs = " << secs << " dsecs = " << dsecs << endl;
}

// ------------------------------------------------------------------------
void ParseThread::close()
{
//   cout << "inside ParseThread::close()" << endl;
   
   ScopedLock input_lock(m_InputMutex);
   m_InputFile.close();
   {
      ScopedLock dest_lock(m_DestinationsMutex);
      m_Destinations.clear();
   }
}

// ------------------------------------------------------------------------
// Member function run executes the thread which places packets from a
// coded video file into a FIFO queue.  A second thread within
// FFmpegStream retrieves and decodes the packets.

void ParseThread::run()
{
//   cout << "inside ParseThread::run()" << endl;

// Make life easier--you are not allowed to change the input while
// running:

   ScopedLock input_lock(m_InputMutex);

   AVPacket pkt;
   m_Error = 0;
   quit(false);
    
   while (!m_Quit)
   {

      // Some play/pause crap???

      if (m_SeekRequested)
      {
//         cout << "inside ParseThread::run()" << endl;
//         cout << "m_SeekStream = " << m_SeekStream << endl;
//         cout << "m_SeekTime = " << m_SeekTime << endl;
//         cout << "m_SeekFlags = " << m_SeekFlags << endl;

         m_Error = av_seek_frame(
            input_context_ptr, m_SeekStream, m_SeekTime, m_SeekFlags);

         if (m_Error < 0)
         {
            // non-fatal
            // DELETE ME
            cout << "Problem seeking in ParseThread::run()" << endl;
         }
         else
         {
            flush_all();
         }
         m_SeekRequested = false;
      }

      m_Error = av_read_frame(input_context_ptr, &pkt);

      if (m_Error)
      {
         cout << "m_Error = " << m_Error << endl;
         break;
      }

      PacketQueue* dest;
      if (dest = m_Destinations.at(pkt.stream_index))
      {
         dest->put(&pkt);
      }
      else
      {
         av_free_packet(&pkt);
      }
   } // while !m_Quit

//   cout << "At end of ParseThread::run()" << endl;
}

// ------------------------------------------------------------------------
// Member function read_next_packet

void ParseThread::read_next_packet()
{
//   cout << "inside ParseThread::read_next_packet()" << endl;

   m_Error = 0;
   if (m_SeekRequested)
   {
//         cout << "m_SeekStream = " << m_SeekStream << endl;
//         cout << "m_SeekTime = " << m_SeekTime << endl;
//         cout << "m_SeekFlags = " << m_SeekFlags << endl;

      m_Error = av_seek_frame(
         input_context_ptr, m_SeekStream, m_SeekTime, m_SeekFlags);

      if (m_Error < 0)
      {
         // non-fatal
         // DELETE ME
         cout << "Problem seeking in ParseThread::run()" << endl;
      }
      else
      {
         flush_all();
      }
      m_SeekRequested = false;
   }

   AVPacket pkt;
   m_Error = av_read_frame(input_context_ptr, &pkt);
//      cout << "m_Error = " << m_Error << endl;
   if (m_Error) return;

   PacketQueue* destination_ptr;
   if (destination_ptr = m_Destinations.at(pkt.stream_index))
   {
      destination_ptr->put(&pkt);
   }
   else
   {
      av_free_packet(&pkt);
   }

}
