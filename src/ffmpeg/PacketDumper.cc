// ========================================================================
// ========================================================================
// Last updated on 1/7/08; 1/8/08
// ========================================================================

#include "ffmpeg/PacketDumper.h"
#include <cstdio>

using namespace FFWrapperOT;

// Forward declarations:

struct AVPacket;

extern "C"
{
   void av_pkt_dump(FILE* f,AVPacket* pkt, int dump_payload);
}


// ========================================================================
PacketDumper::PacketDumper()
   : m_PacketQueue(0)
{
}

void PacketDumper::run()
{
   AVPacket pkt;

   quit(false);

   for(;;)
   {
      PacketQueue *q = NULL;

      {
         ScopedLock lock(m_Mutex);
         for(;;)
         {
            if(m_Quit)
            {
               break;
            }
            if(m_PacketQueue)
            {
               q = m_PacketQueue;
               break;
            }
            else
            {
               m_HavePacketQueue.wait(&m_Mutex);
            }
         }
      }
        
      if(m_Quit)
      {
         break;
      }
      int err = q->get(&pkt);
        
      if(!err)
      {
         av_pkt_dump(stdout, &pkt, 0);
      }
   }
}
