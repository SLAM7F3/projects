// On 6/12/13, we remembered that a line around 452 in
// FFMPEGVideo::decodeNextFrame() must be enabled in order for the
// SIFT main lobby and Bluegrass demos to run:

// ========================================================================
// FFMPEGVideo member function definitions
// ========================================================================
// Last updated on 3/21/12; 5/29/13; 1/4/14
// ========================================================================

#include <cassert>
#include <iostream>
#include <math.h>
#include "ffmpeg/FFMPEGVideo.h"
#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

// static
FFMPEGVideo* FFMPEGVideo::fromFile(string filename)
{
   // one time registration
   if( !( FFMPEGVideo::s_bInitialized ) )
   {
      av_register_all();
      FFMPEGVideo::s_bInitialized = true;
   }

   AVFormatContext* pFormatContext;
   AVCodecContext* pCodecContext;
   AVCodec* pCodec;
   AVFrame* pFrameRaw;
   AVFrame* pFrameRGB;	

   // Open the file and examine the header
   // populates pFormatContext
   int retVal = av_open_input_file
      (
         &pFormatContext, // output context
         filename.c_str(), // filename
         NULL, // format, NULL --> auto-detect, otherwise, forces file format
         0, // buffer size, 0 --> auto-select
         NULL // format options, NULL --> auto-detect, otherwise force decode options
         );

   if( retVal == 0 ) // if succeeded
   {
      // Retrieve stream information
      // populates pFormatContext->streams with data

      retVal = av_find_stream_info( pFormatContext );
      if( retVal >= 0 ) // if succeeded
      {
         // TODO: let the user select which stream
         // Find the first video stream
         uint i = 0;
         int videoStreamIndex = -1;
         while( ( videoStreamIndex == -1 ) && ( i < pFormatContext->nb_streams ) )
         {
            if( pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO )
            {
               videoStreamIndex = i;
            }
            ++i;
         }

         // if we found a video stream
         // load its codec context
         if( videoStreamIndex > -1 )
         {
            // get a pointer to the codec context for the video stream
            pCodecContext = pFormatContext->streams[ videoStreamIndex ]->codec;

            // find a codec for the codec context
            pCodec = avcodec_find_decoder( pCodecContext->codec_id );
            if( pCodec != NULL )
            {
               // ok we found a codec, try opening it
               retVal = avcodec_open( pCodecContext, pCodec );
               if( retVal >= 0 )
               {
                  // Allocate a frame for the incoming data						
                  pFrameRaw = avcodec_alloc_frame();
                  if( pFrameRaw != NULL )
                  {
                     // Allocate another for RGB
                     pFrameRGB = avcodec_alloc_frame();
                     if( pFrameRGB != NULL )
                     {
                        // Note: PixelFormats are in avutil.h
                        // Note: flags are in swscale.h
                        SwsContext* pSWSContext = sws_getContext
                           (
                              pCodecContext->width, pCodecContext->height, // source width and height
                              pCodecContext->pix_fmt, // source format
                              pCodecContext->width, pCodecContext->height, // destination width and height
                              PIX_FMT_RGB24, // destination format
                              SWS_POINT, // flags
                              NULL, // source filter, NULL --> default
                              NULL, // destination filter, NULL --> default
                              NULL // filter parameters, NULL --> default
                              );

                        if( pSWSContext != NULL )
                        {
                           FFMPEGVideo* video = new FFMPEGVideo
                              (
                                 pFormatContext,
                                 videoStreamIndex,
                                 pCodecContext,
                                 pFrameRaw,
                                 pFrameRGB,
                                 pSWSContext
                                 );

                           if( video != NULL )
                           {
                              return video;										
                           }
                           else
                           {
                              fprintf( stderr, "Out of memory allocating video object!\n" );
                           }

                           sws_freeContext( pSWSContext );
                        }
                        else
                        {
                           fprintf( stderr, "Error creating RGB conversion context!\n" );
                        }

                        av_free( pFrameRGB );
                     }
                     else
                     {
                        fprintf( stderr, "Error allocating RGB frame!\n" );
                     }
							
                     av_free( pFrameRaw );
                  }
               }
               else
               {
                  fprintf( stderr, "Error opening codec!\n" );
               }
            }
            else
            {
               fprintf( stderr, "Unsupported codec!\n" );
            }
         }			
         else
         {
            fprintf( stderr, "File contains no video streams!\n" );
         }
      }
      else
      {
         fprintf( stderr, "Error parsing stream information!\n" );
      }

      // close the video file in case of failure
      av_close_input_file( pFormatContext );
   }
   else
   {
      fprintf( stderr, "Error opening %s!\n", filename.c_str() );
   }
	
   assert( false );
   return NULL;	
}

// ------------------------------------------------------------------------
// virtual
FFMPEGVideo::~FFMPEGVideo()
{
   sws_freeContext( m_pSWSContext );
   av_free( m_pFrameRGB );
   av_free( m_pFrameRaw );
   avcodec_close( m_pCodecContext );
   av_close_input_file( m_pFormatContext );
}

// ------------------------------------------------------------------------
// virtual
long long FFMPEGVideo::numFrames()
{
   return m_nFrames;	
}

// ------------------------------------------------------------------------
// virtual
float FFMPEGVideo::framePeriodMilliseconds()
{
   return( 1000.f * framePeriodSeconds() );
}

// ------------------------------------------------------------------------
// virtual
float FFMPEGVideo::framePeriodSeconds()
{
   return m_framePeriodSeconds;
}

// ------------------------------------------------------------------------
// virtual
int FFMPEGVideo::width()
{
   return m_width;
}

// ------------------------------------------------------------------------
// virtual
int FFMPEGVideo::height()
{
   return m_height;
}

// ------------------------------------------------------------------------
// virtual
int FFMPEGVideo::bytesPerFrame()
{
   return m_nBytesPerFrame;	
}

// ------------------------------------------------------------------------
void FFMPEGVideo::setNextFrameIndex( long long frameIndex )
{
   m_nextFrameIndex=frameIndex;
}

// ------------------------------------------------------------------------
// virtual
long long FFMPEGVideo::getNextFrameIndex()
{
   return m_nextFrameIndex;
}

// ------------------------------------------------------------------------
// virtual
bool FFMPEGVideo::setAndDecodeNextFrame( long long frameIndex )
{
   // if frameIndex is out of range, then return false
   if( frameIndex < 0 || frameIndex >= m_nFrames )
   {
#if _WIN32		
      fprintf( stderr, "Cannot seek to frame %I64d, frameIndex must be between 0 and %I64d\n", frameIndex, m_nFrames );
#else
      fprintf( stderr, "Cannot seek to frame %lld, frameIndex must be between 0 and %lld\n", frameIndex, m_nFrames );
#endif
      return false;
   }

   // else if it's going to be the next frame anyway
   // then do nothing
   if ( frameIndex == m_nextFrameIndex )
   {
      return true;
   }	

   // always seek to the keyframe right before t
   int seekFlags = AVSEEK_FLAG_BACKWARD;

   // tell ffmpeg to seek
   int retVal = av_seek_frame( 
      m_pFormatContext, m_videoStreamIndex, frameIndex, seekFlags );
   if( retVal < 0 )
   {
#if _WIN32		
      fprintf( stderr, "ffmpeg error seeking to frame: %I64d\n", frameIndex );
#else
      fprintf( stderr, "ffmpeg error seeking to frame: %lld\n", frameIndex );
#endif
      return false;
   }

   // seek was successful, flush codec internal buffers
   avcodec_flush_buffers( m_pCodecContext );
   m_nextFrameIndex = frameIndex;
   return true;
}

// ------------------------------------------------------------------------
// virtual

bool FFMPEGVideo::getNextFrame( unsigned char* dataOut )
{
//   cout << "inside FFMPEGVideo::getNextFrame()" << endl;

   if( m_nextFrameIndex >= m_nFrames )
   {
      return false;
   }

   // TODO: can potentially accelerate this by using
   // m_pCodexContext->hurry_up = 1

   long long t;
   bool decodeSucceeded = decodeNextFrame( &t );
   while( decodeSucceeded && ( t/denom_factor < m_nextFrameIndex ) )
   {
      decodeSucceeded = decodeNextFrame( &t );
   }

   // if the loop was successful
   // then t = m_nextFrameIndex

//   cout << "t/denom_factor = " << t/denom_factor 
//        << " m_nextFrameIndex = " << m_nextFrameIndex 
//        << endl;

   if ( decodeSucceeded )
   {
      // convert the decoded frame to RGB
      convertDecodedFrameToRGB( dataOut );

      ++m_nextFrameIndex;			
      return true;
   }
   else
   {
      return false;
   }	
}

// ------------------------------------------------------------------------
// Virtual member function getCurrFrame populates unsigned char* array
// dataOut with the contents of the frame number set within
// setNextFrameIndex.

void FFMPEGVideo::getCurrFrame( unsigned char* dataOut )
{
//   cout << "inside FFMPEGVide::getCurrFrame()" << endl;
//   cout << "numFrames() = " << numFrames() << endl;
//   cout << "m_nextFrameIndex = " << m_nextFrameIndex << endl;
   convertDecodedFrameToRGB( dataOut );
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

FFMPEGVideo::FFMPEGVideo( 
   AVFormatContext* pFormatContext, int videoStreamIndex,
   AVCodecContext* pCodecContext,
   AVFrame* pFrameRaw,
   AVFrame* pFrameRGB,
   SwsContext* pSWSContext ) :
   
   m_pFormatContext( pFormatContext ),
   m_videoStreamIndex( videoStreamIndex ),
   m_pCodecContext( pCodecContext ),
   m_pFrameRaw( pFrameRaw ),
   m_pFrameRGB( pFrameRGB ),
   m_pSWSContext( pSWSContext ),
   
   m_width( pCodecContext->width ),
   m_height( pCodecContext->height ),
   m_nFrames(m_pFormatContext->streams[ m_videoStreamIndex ]->duration),
   m_nextFrameIndex( 0 )
{
//   cout << "Inside FFMPEGVIDEO constructor " << endl;

   recompute_nframes();

   m_nBytesPerFrame = avpicture_get_size( 
      PIX_FMT_RGB24, width(), height() );

   AVRational framePeriod = m_pFormatContext->streams[ 
      m_videoStreamIndex ]->time_base;
   m_framePeriodSeconds = static_cast< float >( av_q2d( framePeriod ) );
}

// ------------------------------------------------------------------------
// On 1/15/09, we empirically observed that m_nFrames for the Jan 2009
// Lobby 7 Quicktime movie was 100 times too large.  So we wrote this
// little hack member function which tries to compute a denominator
// factor by which the naive m_nFrames should be divided in order to
// yield a genuine number of frames within movies parsed by FFMPEG.

void FFMPEGVideo::recompute_nframes()
{

//   cout << "m_nFrames = " << m_nFrames << endl;

   long long imagenumber_0,imagenumber_1,frameIndex;
   m_nextFrameIndex=0;
   decodeNextFrame(&frameIndex);   
   decodeNextFrame(&frameIndex);   
   decodeNextFrame(&frameIndex);   

   imagenumber_0=frameIndex;
   decodeNextFrame(&frameIndex);   
   imagenumber_1=frameIndex;

//   cout << "imagenumber_0 = " << imagenumber_0
//        << " imagenumber_1 = " << imagenumber_1 << endl;
   denom_factor=fabs(imagenumber_1-imagenumber_0);
   m_nFrames /= denom_factor;

   cout << "denom_factor = " << denom_factor 
        << " m_nFrames = " << m_nFrames << endl;

   m_nextFrameIndex=0;

// On 1/27/09, we learned the hard and painful way that we need to
// include the next two lines in order to avoid mysterious delays
// between the playback of the Boston skyline video sequence and the
// rotation of its OBSFRUSTUM:

   setAndDecodeNextFrame(0);
   setNextFrameIndex(1);
}

// ------------------------------------------------------------------------
bool FFMPEGVideo::decodeNextFrame( long long* decodedFrameIndex )
{
   AVPacket packet;
   bool readFrameSucceeded;	
   bool decodeSucceeded = true;

   int frameFinished = 0; // frameFinished > 0 means the frame is finished
   readFrameSucceeded = ( av_read_frame( m_pFormatContext, &packet ) >= 0 );
   // loop while it can still read (bSucceeded) and
   // the frame is NOT done (frameFinished == 0)
   while( readFrameSucceeded && ( frameFinished == 0 ) )
   {
      // printf( "decodeNextFrame: packet.dts = %I64d, packet.pts = %I64d\n", packet.dts, packet.pts );

      // is this a packet from the video stream we selected?
      if( packet.stream_index == m_videoStreamIndex )
      {
         // if so, then decode it

// On 5/29/13, we discovered that the following line doesn't link
// correctly under gcc/g++ version 4.6.3 in Ubuntu 12.4.  So for now,
// we comment it out in order to build
// mains/photosynth/Qt/qtphotoserver:

// On 6/12/13, we remembered that the following line must be enabled
// in order for the SIFT main lobby and Bluegrass demos to run:

// On 1/4/14, we discovered that the following line does link
// correctly under gcc/g++ versio 4.6.3 in Ubuntu 12.4.  Moreover we
// can build the Bluegrass demo provided we explicitly add
// /usr/local/libavcodec.a, /usr/local/lib/avutil.a, etc to
// projects/config/common_all.pro:

         int decodeReturnVal = avcodec_decode_video( 
            m_pCodecContext, m_pFrameRaw, &frameFinished, 
            packet.data, packet.size );

         decodeSucceeded = ( decodeReturnVal > 0 );

         // we failed in decoding the video
         if( !decodeSucceeded )
         {
#if _WIN32
            fprintf( stderr, "ffmpeg error decoding video frame: %I64d\n", m_nextFrameIndex );
#else
            fprintf( stderr, "ffmpeg error decoding video frame: %lld\n", m_nextFrameIndex );
#endif
            // always free the packet that was allocated by av_read_frame
            av_free_packet( &packet );
            return false;
         }

         if( decodedFrameIndex != NULL )
         {
            *decodedFrameIndex = packet.pts; // ffmpeg uses 0-based frame indices
         }
      }

      // always free the packet that was allocated by av_read_frame
      av_free_packet( &packet );

      // if the frame isn't finished, then read another packet
      if( frameFinished == 0 )
      {
         readFrameSucceeded = ( av_read_frame( m_pFormatContext, &packet ) >= 0 );
      }
   };

   if( !readFrameSucceeded )
   {
#if _WIN32
      fprintf( stderr, "ffmpeg error reading next packet on frame: %I64d\n", m_nextFrameIndex );
#else
      fprintf( stderr, "ffmpeg error reading next packet on frame: %lld\n", m_nextFrameIndex );
#endif
      return false;
   }	
   return true;
}

// ------------------------------------------------------------------------
void FFMPEGVideo::convertDecodedFrameToRGB( unsigned char* rgbOut )
{
   // associate buffer with pFrameRGB
   avpicture_fill( ( AVPicture* )m_pFrameRGB,
                   rgbOut,
                   PIX_FMT_RGB24,
                   width(), height() );

   sws_scale
      (
         m_pSWSContext, // converter
         m_pFrameRaw->data, m_pFrameRaw->linesize, // source data and stride
         0, height(), // starting y and height
         m_pFrameRGB->data, m_pFrameRGB->linesize
         );
}

// ------------------------------------------------------------------------
bool FFMPEGVideo::isDecodedFrameKey()
{
   return( m_pFrameRaw->key_frame == 1 );
}

// ------------------------------------------------------------------------
// static
bool FFMPEGVideo::s_bInitialized = false;
