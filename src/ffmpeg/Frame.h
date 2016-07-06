// ========================================================================
// ========================================================================
// Last updated on 1/7/08; 1/8/08
// ========================================================================

#ifndef INCLUDE_FFWRAPPER_FRAME
#define INCLUDE_FFWRAPPER_FRAME

// Forward declarations

struct AVFrame;

extern "C"
{
   void av_free(void* ptr);
   AVFrame* avcodec_alloc_frame(void);
}

namespace FFWrapper
{
   class Frame
      {
         AVFrame* m_Frame;
        public:
         Frame() { m_Frame = avcodec_alloc_frame(); }
         ~Frame() { av_free(m_Frame); }

         AVFrame* get() throw() { return m_Frame; }
         const AVFrame* get() const throw() { return m_Frame; }
      };
}

#endif
