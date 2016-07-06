// ========================================================================
// Header file for Anye Li's CodecContext class
// ========================================================================
// Last updated on 1/7/08; 1/8/08
// ========================================================================

#ifndef INCLUDE_FFWRAPPER_CODECCONTEXT
#define INCLUDE_FFWRAPPER_CODECCONTEXT

#ifndef INCLUDE_FFWRAPPER_CODECOPTIONS
#include "ffmpeg/CodecOptions.h"
#endif

struct AVCodecContext;

namespace FFWrapper
{
   class CodecContext
      {
        public:
         CodecContext();
         ~CodecContext();

         int open_nothrow(
            AVCodecContext* p_Context,
            const CodecOptions& p_Options
            = CodecOptions()
            ) throw();
         void close() throw();

         AVCodecContext* get() throw();
         const AVCodecContext* get() const throw();
        private:
         /// Copy constructor disabled
         CodecContext(const CodecContext&);
         /// Assignment operator disabled
         CodecContext& operator=(const CodecContext&);

         AVCodecContext* m_Context;
      };
}

inline AVCodecContext* FFWrapper::CodecContext::get() throw()
{
   return m_Context;
}

inline const AVCodecContext* FFWrapper::CodecContext::get() const throw()
{
   return m_Context;
}

#endif
