// ========================================================================
// ========================================================================
// Last updated on 1/7/08; 1/8/08
// ========================================================================

#include "ffmpeg/PacketQueue.h"

using namespace FFWrapperOT;

// Forward declarations:

struct AVPacket;

extern "C"
{
   int av_dup_packet(AVPacket* pkt);
}

// ========================================================================
PacketQueue::PacketQueue(): 
   m_NumBytes(0),
     m_MaxNumBytes(5 * 16 * 1024),
     m_AbortRequested(false)
{
}

PacketQueue::~PacketQueue()
{
   flush();
}

int PacketQueue::put(AVPacket *p_Packet, bool p_Block)
{
   int err;
   // Trust me, you do not want to know what this is.
   err = av_dup_packet(p_Packet);
   if(err < 0)
   {
      return err;
   }
   ScopedLock lock(m_Mutex);
   for(;;)
   {
      if(abort_requested())
      {
         return 1;
      }
      if(full())
      {
         if(p_Block)
         {
            m_CanPut.wait(&m_Mutex);
         }
         else
         {
            return 2;
         }
      }
      else
      {
         m_Packets.push_back(*p_Packet);
         m_NumBytes += p_Packet->size;
         m_CanGet.signal();
         return 0;
      }
   }
   // shouldn't happen
   return -1;
}

int PacketQueue::get(AVPacket *p_Packet, bool p_Block)
{
//   int err;
   ScopedLock lock(m_Mutex);
   for(;;)
   {
      if(abort_requested())
      {
         return 1;
      }
      if(empty())
      {
         if(p_Block)
         {
            m_CanGet.wait(&m_Mutex);
         }
         else
         {
            return 2;
         }
      }
      else
      {
         *p_Packet = m_Packets.front();
         m_Packets.pop_front();
         m_NumBytes -= p_Packet->size;
         m_CanPut.signal();
         return 0;
      }
   }
   // shouldn't happen
   return -1;
}

void PacketQueue::flush()
{
   ScopedLock lock(m_Mutex);
   for(Iterator p = m_Packets.begin(); p != m_Packets.end(); ++p)
   {
      av_free_packet(&*p);
   }
   m_Packets.clear();
   m_NumBytes = 0;
   m_CanPut.signal();
}
