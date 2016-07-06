// ========================================================================
// ========================================================================
// Last updated on 1/7/08; 1/8/08; 9/3/09
// ========================================================================

#ifndef INCLUDE_FFWRAPPEROT_PACKETDUMPER
#define INCLUDE_FFWRAPPEROT_PACKETDUMPER

#ifndef INCLUDE_FFWRAPPEROT_PACKETQUEUE
#include "ffmpeg/PacketQueue.h"
#endif

#include <iostream>
#include <OpenThreads/Thread>

namespace FFWrapperOT
{
   /// A really simple, dumb "decoder", for debugging and
   /// learning purposes
   class PacketDumper : public OpenThreads::Thread
      {
        public:
         PacketDumper();
        
         void run();

         void quit(bool p_Quit = true);        

         void set_source(PacketQueue *p_PacketQueue);
        private:
         PacketQueue* m_PacketQueue;
         OpenThreads::Condition m_HavePacketQueue;

         bool m_Quit;
        
         typedef OpenThreads::ScopedLock<OpenThreads::Mutex>
            ScopedLock;
         OpenThreads::Mutex m_Mutex;
      };
}

inline void FFWrapperOT::PacketDumper::quit(bool p_Quit)
{
   ScopedLock lock(m_Mutex);
   m_Quit = p_Quit;
   if(p_Quit && m_PacketQueue)
   {
      m_PacketQueue->request_abort();
   }
   m_HavePacketQueue.signal();
}

inline void FFWrapperOT::PacketDumper::set_source(
   FFWrapperOT::PacketQueue *p_PacketQueue)
{
   ScopedLock lock(m_Mutex);
   if(m_PacketQueue = p_PacketQueue)
   {
      m_HavePacketQueue.signal();
   }
}

#endif
