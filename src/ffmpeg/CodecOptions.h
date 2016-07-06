// ========================================================================
// ========================================================================
// Last updated on 1/7/08
// ========================================================================

#ifndef INCLUDE_FFWRAPPER_CODECOPTIONS
#define INCLUDE_FFWRAPPER_CODECOPTIONS

// extern "C" {
// #include <ffmpeg/avcodec.h>
// }

enum AVDiscard{
   /* We leave some space between them for extensions (drop some
    * keyframes for intra-only or drop just some bidir frames). */
   AVDISCARD_NONE   =-16, ///< discard nothing
   AVDISCARD_DEFAULT=  0, ///< discard useless packets like 0 size packets in avi
   AVDISCARD_NONREF =  8, ///< discard all non reference
   AVDISCARD_BIDIR  = 16, ///< discard all bidirectional frames
   AVDISCARD_NONKEY = 32, ///< discard all frames except keyframes
   AVDISCARD_ALL    = 48, ///< discard all
};

namespace FFWrapper
{

   /**
    * Options that get passed to the AVCodecContext.  Since none
    * of these options are actually documented, I can't really
    * tell you what they mean. */
    
   class CodecOptions
      {
        public:
         CodecOptions();

         void set_debug(int p_Debug) throw();
         int get_debug() const throw();

         void set_debug_mv(int p_DebugMv) throw();
         int get_debug_mv() const throw();
        
         void set_workaround_bugs(int p_WorkaroundBugs) throw();
         int get_workaround_bugs() const throw();
        
         void set_lowres(int p_Lowres) throw();
         int get_lowres() const throw();
        
         void set_idct_algo(int p_IdctAlgo) throw();
         int get_idct_algo() const throw();

         void set_fast(bool p_Fast) throw();
         bool get_fast() const throw();
        
         void set_skip_frame(enum AVDiscard p_SkipFrame) throw();
         enum AVDiscard get_skip_frame() const throw();
        
         void set_skip_idct(enum AVDiscard p_SkipIdct) throw();
         enum AVDiscard get_skip_idct() const throw();
        
         void set_skip_loop_filter(
            enum AVDiscard p_SkipLoopFilter) throw();
         enum AVDiscard get_skip_loop_filter() const throw();

         void set_error_resilience(int p_ErrorResilience) throw();
         int get_error_resilience() const throw();
        
         void set_error_concealment(int p_ErrorConcealment) throw();
         int get_error_concealment() const throw();

         void set_thread_count(int p_ThreadCount) throw();
         int get_thread_count() const throw();
        
        private:
         int m_Debug;
         int m_DebugMv;
         int m_WorkaroundBugs;
         int m_Lowres;
         int m_IdctAlgo;
         bool m_Fast;
         enum AVDiscard m_SkipFrame;
         enum AVDiscard m_SkipIdct;
         enum AVDiscard  m_SkipLoopFilter;
         int m_ErrorResilience;
         int m_ErrorConcealment;
         int m_ThreadCount;
      };
}

#define FF_IDCT_AUTO          0
#define FF_ER_CAREFUL         1

// Defaults stolen from ffplay

inline FFWrapper::CodecOptions::CodecOptions()
   : m_Debug(0),
     m_DebugMv(0),
     m_WorkaroundBugs(1),
     m_Lowres(0),
     m_Fast(false),
     m_IdctAlgo(FF_IDCT_AUTO),
     m_SkipFrame(AVDISCARD_DEFAULT),
     m_SkipIdct(AVDISCARD_DEFAULT),
     m_SkipLoopFilter(AVDISCARD_DEFAULT),
     m_ErrorResilience(FF_ER_CAREFUL),
     m_ErrorConcealment(3),
     m_ThreadCount(1)
{
}

inline void FFWrapper::CodecOptions::set_debug(int p_Debug) throw()
{
   m_Debug = p_Debug;
}

inline int FFWrapper::CodecOptions::get_debug() const throw()
{
   return m_Debug;
}

inline void FFWrapper::CodecOptions::set_debug_mv(int p_DebugMv) throw()
{
   m_DebugMv = p_DebugMv;
}

inline int FFWrapper::CodecOptions::get_debug_mv() const throw()
{
   return m_DebugMv;
}
    
inline void FFWrapper::CodecOptions::set_workaround_bugs(
   int p_WorkaroundBugs) throw()
{
   m_WorkaroundBugs = p_WorkaroundBugs;
}

inline int FFWrapper::CodecOptions::get_workaround_bugs() const throw()
{
   return m_WorkaroundBugs;
}
    
inline void FFWrapper::CodecOptions::set_lowres(int p_Lowres) throw()
{
   m_Lowres = p_Lowres;
}

inline int FFWrapper::CodecOptions::get_lowres() const throw()
{
   return m_Lowres;
}
        
inline void FFWrapper::CodecOptions::set_idct_algo(int p_IdctAlgo) throw()
{
   m_IdctAlgo = p_IdctAlgo;
}

inline int FFWrapper::CodecOptions::get_idct_algo() const throw()
{
   return m_IdctAlgo;
}

inline void FFWrapper::CodecOptions::set_fast(bool p_Fast) throw()
{
   m_Fast = p_Fast;
}

inline bool FFWrapper::CodecOptions::get_fast() const throw()
{
   return m_Fast;
}
        
inline void FFWrapper::CodecOptions::set_skip_frame(
   enum AVDiscard p_SkipFrame) throw()
{
   m_SkipFrame = p_SkipFrame;
}

inline enum AVDiscard FFWrapper::CodecOptions::get_skip_frame() const throw()
{
   return m_SkipFrame;
}
        
inline void FFWrapper::CodecOptions::set_skip_idct(
   enum AVDiscard p_SkipIdct) throw()
{
   m_SkipIdct = p_SkipIdct;
}

inline enum AVDiscard FFWrapper::CodecOptions::get_skip_idct() const throw()
{
   return m_SkipIdct;
}
        
inline void FFWrapper::CodecOptions::set_skip_loop_filter(
   enum AVDiscard p_SkipLoopFilter) throw()
{
   m_SkipLoopFilter = p_SkipLoopFilter;
}

inline enum AVDiscard FFWrapper::CodecOptions::get_skip_loop_filter() 
   const throw()
{
   return m_SkipLoopFilter;
}

inline void FFWrapper::CodecOptions::set_error_resilience(
   int p_ErrorResilience) throw()
{
   m_ErrorResilience = p_ErrorResilience;
}

inline int FFWrapper::CodecOptions::get_error_resilience() const throw()
{
   return m_ErrorResilience;
}

inline void FFWrapper::CodecOptions::set_error_concealment(
   int p_ErrorConcealment) throw()
{
   m_ErrorConcealment = p_ErrorConcealment;
}

inline int FFWrapper::CodecOptions::get_error_concealment() const throw()
{
   return m_ErrorConcealment;
}

inline void FFWrapper::CodecOptions::set_thread_count(
   int p_ThreadCount) throw()
{
   m_ThreadCount = p_ThreadCount;
}

inline int FFWrapper::CodecOptions::get_thread_count() const throw()
{
   return m_ThreadCount;
}

#endif
