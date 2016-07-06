// ========================================================================
// ========================================================================
// Last updated on 1/7/08; 1/8/08; 6/5/12
// ========================================================================

#ifndef INCLUDE_FFWRAPPEROT_PACKETQUEUE
#define INCLUDE_FFWRAPPEROT_PACKETQUEUE

#include <list>
#include <stdint.h>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>
#include <OpenThreads/Condition>

typedef struct AVPacket {
    int64_t pts;                            ///< presentation time stamp in time_base units
    int64_t dts;                            ///< decompression time stamp in time_base units
    uint8_t* data;
    int   size;
    int   stream_index;
    int   flags;
    int   duration;                         ///< presentation duration in time_base units (0 if not available)
    void  (*destruct)(struct AVPacket *);
    void  *priv;
    int64_t pos;                            ///< byte position in stream, -1 if unknown
} AVPacket;


namespace FFWrapperOT
{
    /// Note: This is not based on AVPacketList.  If it turns out
    /// that there is a significant performance boost to using the
    /// singly linked AVPacketList and av_malloc() for memory
    /// allocation, then I will rewrite it as such later.  For
    /// clarity, I think it's better to use the STL lists.

    class PacketQueue
    {
    public:
        PacketQueue();
        ~PacketQueue();

        /// Put packet *p_Packet onto the queue.  If p_Block is
        /// set to false, then this call will return immediately;
        /// otherwise it will block until the packet is actually
        /// put in the queue.
        ///
        /// Returns 0 if succeeded, < 0 if error, 1 if aborted, 2
        /// if failed without error.

        int put(AVPacket *p_Packet, bool p_Block = true);

        /// Remove the first packet from the queue and place it in
        /// *p_Packet.  If p_Block is set to false, then this call
        /// will return immediately; otherwise it will block until
        /// there is actually a packet to receive.
        /// 
        /// Returns 0 if succeeded, 1 if aborted, 2 if failed
        /// without error.
        int get(AVPacket *p_Packet, bool p_Block = true);

        /// Clear the list, freeing all packet data.
        void flush();

        std::size_t num_bytes() const throw();
        std::size_t max_num_bytes() const throw();
        void set_max_num_bytes(std::size_t p_MaxNumBytes);

        bool empty() const throw();
        bool full() const throw();

        /// Request that all pending transactions be aborted.

        void request_abort(bool p_Requested = true);
        bool abort_requested() const throw();
        
        /// Request that the decoder flush its buffers.
        /// 
        /// Technically, this has nothing to do with the packet
        /// queue.  The reason this is here is to keep all the
        /// communication between the parse thread and decoder
        /// threads within the same structure.
        void codec_request_flush(bool p_Requested = true);
        bool codec_flush_requested() const throw();

        /// Convenience function to flush the queue and unset the
        /// abort_requested() state.
        void reset();

    private:
        typedef std::list<AVPacket> List;
        typedef List::iterator Iterator;
        
        List m_Packets;

        std::size_t m_NumBytes;
        std::size_t m_MaxNumBytes;

        bool m_AbortRequested;
        
        bool m_FlushRequested;

        // provide exception safety and make life easier when
        // jumping out of loops and conditions

        typedef OpenThreads::ScopedLock<OpenThreads::Mutex>
        ScopedLock;
        OpenThreads::Mutex m_Mutex;
        OpenThreads::Condition m_CanPut;
        OpenThreads::Condition m_CanGet;
    };
}

inline std::size_t FFWrapperOT::PacketQueue::num_bytes() const throw()
{
    return m_NumBytes;
}

inline std::size_t FFWrapperOT::PacketQueue::max_num_bytes() const throw()
{
    return m_MaxNumBytes;
}

inline void FFWrapperOT::PacketQueue::set_max_num_bytes(
   std::size_t p_MaxNumBytes)
{
    ScopedLock lock(m_Mutex);
    m_MaxNumBytes = p_MaxNumBytes;
}

inline bool FFWrapperOT::PacketQueue::empty() const throw()
{
    return m_NumBytes == 0;
}

inline bool FFWrapperOT::PacketQueue::full() const throw()
{
    return m_NumBytes > m_MaxNumBytes;
}

inline void FFWrapperOT::PacketQueue::request_abort(bool p_Requested)
{
    ScopedLock lock(m_Mutex);
    m_AbortRequested = p_Requested;
    if(p_Requested)
    {
        m_CanPut.broadcast();
        m_CanGet.broadcast();
    }
}

inline bool FFWrapperOT::PacketQueue::abort_requested() const throw()
{
    return m_AbortRequested;
}

inline void FFWrapperOT::PacketQueue::codec_request_flush(bool p_Requested)
{
    ScopedLock lock(m_Mutex);
    m_FlushRequested = p_Requested;
}

inline bool FFWrapperOT::PacketQueue::codec_flush_requested() const throw()
{
    return m_FlushRequested;
}

inline void FFWrapperOT::PacketQueue::reset()
{
    flush();
    request_abort(false);
}

inline void av_free_packet(AVPacket* pkt)
{
   if (pkt && pkt->destruct) 
   {
      pkt->destruct(pkt);
   }
}

#endif
