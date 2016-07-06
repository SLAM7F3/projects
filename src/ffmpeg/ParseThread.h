// ========================================================================
// Header file for Anye Li's ParseThread class
// ========================================================================
// Last updated on 1/11/08; 1/17/08; 1/24/08
// ========================================================================

#ifndef INCLUDE_FFWRAPPEROT_PARSETHREAD
#define INCLDUE_FFWRAPPEROT_PARSETHREAD

#include <iostream>
#include <vector>
#include <OpenThreads/Thread>
#include "ffmpeg/InputFile.h"
#include "ffmpeg/PacketQueue.h"

struct AVFormatContext;
struct AVStream;

extern "C"
{
   unsigned int get_nb_streams(const AVFormatContext* ic);
   AVStream* get_indexed_stream(const AVFormatContext* ic,int p_Index);
}

namespace FFWrapperOT
{
   class ParseThread : public OpenThreads::Thread
      {
        public:

         ParseThread();

         void quit(bool p_Quit = true);
         int open(const char *p_Filename,AVInputFormat *p_ForceFormat,
                  int p_BufSize,AVFormatParameters *p_AdditionalParameters);
         void close();
         void compute_start_and_stop_times();
        
         void seek(int64_t p_Timestamp,int p_Flags);
         void seek(int p_StreamIndex,int64_t p_Timestamp,int p_Flags);

         void run();
         void read_next_packet();
        
         AVStream* get_stream(int p_Index) const;
         int num_streams() const throw();
        
         int get_last_error() const throw();

         void set_generate_pts(bool p_Genpts) throw();
         bool generate_pts() const throw();
         void set_show_status(bool p_ShowStatus) throw();
         bool show_status() const throw();
        
         void connect(int p_StreamIndex, PacketQueue *p_Dest);
         void disconnect(int p_StreamIndex);
         /// should be faster than individually disconnecting
         void disconnect_all();

         int get_error();

         int64_t get_starting_time() const;
         int64_t get_stopping_time() const;
         double get_start_time_in_secs() const;
         double get_stop_time_in_secs() const;
         double get_Framerate() const;
         int get_byte_rate() const;
         int get_n_frames() const;

        private:

// Scoped locks for exception safety:

         typedef OpenThreads::ScopedLock<OpenThreads::Mutex> ScopedLock;

         bool m_GeneratePts, m_ShowStatus, m_SeekRequested, m_Quit;
         int m_SeekStream,m_Error,m_SeekFlags;
         int byte_rate,stream_index,n_frames;
         int64_t m_SeekTime,starting_time,stopping_time;
         double start_time_in_secs,stop_time_in_secs,frame_rate;

         AVFormatContext* input_context_ptr;
         OpenThreads::Mutex m_QuitMutex;
         OpenThreads::Mutex m_InputMutex;
         OpenThreads::Mutex m_DestinationsMutex;
         std::vector<PacketQueue *> m_Destinations;
         FFWrapper::InputFile m_InputFile;

         void allocate_member_objects();
         void initialize_member_objects();
         void flush_all();
      };
};

// ========================================================================

inline void FFWrapperOT::ParseThread::connect(
   int p_StreamIndex, PacketQueue *p_Dest)
{

//   std::cout << "inside ParseThread::connect()" << std::endl;
   
   ScopedLock lock(m_DestinationsMutex);
   stream_index=p_StreamIndex;
   m_Destinations.at(stream_index) = p_Dest;
}

inline void FFWrapperOT::ParseThread::disconnect(int p_StreamIndex)
{
   ScopedLock lock(m_DestinationsMutex);
   m_Destinations.at(p_StreamIndex) = 0;
}

inline void FFWrapperOT::ParseThread::disconnect_all()
{
   typedef std::vector<PacketQueue *>::iterator Iter;
   ScopedLock lock(m_DestinationsMutex);
   for(Iter p = m_Destinations.begin();
       p != m_Destinations.end(); ++p)
   {
      *p = 0;
   }
}

inline int FFWrapperOT::ParseThread::get_error()
{
   return m_Error;
}

inline void FFWrapperOT::ParseThread::quit(bool p_Quit)
{
   ScopedLock lock(m_QuitMutex);
   m_Quit = p_Quit;
}

inline void FFWrapperOT::ParseThread::seek(int64_t p_Timestamp,int p_Flags)
{
   seek(m_SeekStream,p_Timestamp,p_Flags);
}

inline void FFWrapperOT::ParseThread::seek(
   int p_StreamIndex,int64_t p_Timestamp,int p_Flags)
{
   m_SeekRequested = true;
   m_SeekStream = p_StreamIndex;
   m_SeekTime = p_Timestamp;
   m_SeekFlags = p_Flags;
}

inline AVStream* FFWrapperOT::ParseThread::get_stream(int p_Index) const
{
   const AVFormatContext* ic = m_InputFile.get_context();
   if (ic)
   {
      if(p_Index >= 0 && p_Index < int(get_nb_streams(ic)))
      {
         return get_indexed_stream(ic,p_Index);
      }
   }
   return 0;
}

inline void FFWrapperOT::ParseThread::flush_all()
{
   ScopedLock lock(m_DestinationsMutex);
   std::vector<PacketQueue *>::iterator p,
      end = m_Destinations.end();
   for(p = m_Destinations.begin(); p != end; ++p)
   {
      if(*p)
      {
         (*p)->flush();
         (*p)->codec_request_flush();
      }
   }
}

inline int FFWrapperOT::ParseThread::get_last_error() const throw()
{
   return m_Error;
}

inline void FFWrapperOT::ParseThread::set_generate_pts(bool p_Genpts) throw()
{
   m_GeneratePts = p_Genpts;
}

inline bool FFWrapperOT::ParseThread::generate_pts() const throw()
{
   return m_GeneratePts;
}

inline void FFWrapperOT::ParseThread::set_show_status(bool p_ShowStatus) 
   throw()
{
   m_ShowStatus = p_ShowStatus;
}

inline bool FFWrapperOT::ParseThread::show_status() const throw()
{
   return m_ShowStatus;
}

inline int FFWrapperOT::ParseThread::num_streams() const throw()
{
   return m_Destinations.size();
}

inline int64_t FFWrapperOT::ParseThread::get_starting_time() const
{
   return starting_time;
}

inline int64_t FFWrapperOT::ParseThread::get_stopping_time() const
{
   return stopping_time;
}

inline double FFWrapperOT::ParseThread::get_start_time_in_secs() const
{
   return start_time_in_secs;
}

inline double FFWrapperOT::ParseThread::get_stop_time_in_secs() const
{
   return stop_time_in_secs;
}

inline double FFWrapperOT::ParseThread::get_Framerate() const
{
   return frame_rate;
}

inline int FFWrapperOT::ParseThread::get_byte_rate() const
{
   return byte_rate;
}

inline int FFWrapperOT::ParseThread::get_n_frames() const
{
   return n_frames;
}

#endif
