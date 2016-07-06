// ========================================================================
// Kevin Chen's FFMPEGVideo class 
// ========================================================================
// Last updated on 1/15/09; 1/27/09; 3/21/12
// ========================================================================

#ifndef FFMPEG_VIDEO_H
#define FFMPEG_VIDEO_H

#include <string>
#include "ffmpeg/BasicTypes.h"
#include "ffmpeg/ReferenceCountedArray.h"

// stdint.h requires __STDC_CONSTANT_MACROS to be defined
// in order to define INT64_C()
// which is needed by ffmpeg

// #if _WIN32
#define __STDC_CONSTANT_MACROS
// #endif

extern "C" {
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/swscale.h>
           }

#include <ffmpeg/swscale.h>

// #if _WIN32
#undef __STDC_CONSTANT_MACROS
// #endif

// A video loaded using ffmpeg 

class FFMPEGVideo
{
  public:

   static FFMPEGVideo* fromFile(std::string filename );
   ~FFMPEGVideo();

   long long numFrames();
   float framePeriodMilliseconds();
   float framePeriodSeconds();

   int width();
   int height();
   int bytesPerFrame();

   void set_frame_skip(long long skip);
   long long get_frame_skip() const;

   // returns the internal frame counter

   void setNextFrameIndex( long long frameIndex );
   long long getNextFrameIndex();
   bool setAndDecodeNextFrame( long long frameIndex );

   // Populates dataOut with the contents of the next frame
   // and increments the internal frame counter
   // returns true if succeeded
   // and false on failure (i.e. at the end of the video stream).

   bool getNextFrame( unsigned char* dataOut );
   void getCurrFrame( unsigned char* dataOut );

  private:

   FFMPEGVideo( AVFormatContext* pFormatContext, int iVideoStreamIndex,
		AVCodecContext* pCodecContext,
		AVFrame* pFrameRaw,
		AVFrame* pFrameRGB,
		SwsContext* pSWSContext );
   
   void recompute_nframes();
   
   // reads the next frame in its internal format and stores it in m_pFrameRaw
   // on success, returns the index of the frame that was decoded
   // returns -1 on failure
   bool decodeNextFrame( long long* decodedFrameIndex );

   // converts the next frame into RGB
   void convertDecodedFrameToRGB( unsigned char* rgbOut );

   bool isDecodedFrameKey();

   // initially false
   // set to true once global ffmpeg initialization is complete
   // (initialized the first time an FFMPEGVideo is created)
   static bool s_bInitialized;

   AVFormatContext* m_pFormatContext;
   int m_videoStreamIndex;
   AVCodecContext* m_pCodecContext;
   AVFrame* m_pFrameRaw;
   AVFrame* m_pFrameRGB;
   SwsContext* m_pSWSContext; // for YUV --> RGB conversion

   // dimensions
   int m_width;
   int m_height;
   long long m_nFrames;

   // derived units
   int m_nBytesPerFrame;
   float m_framePeriodSeconds;

   long long m_nextFrameIndex,frame_skip;

   long long denom_factor;
};


// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void FFMPEGVideo::set_frame_skip(long long skip)
{
   frame_skip=skip;
}

inline long long FFMPEGVideo::get_frame_skip() const
{
   return frame_skip;
}

#endif // FFMPEG_VIDEO_H
