// ==========================================================================
// Header file for stand-alone Kakadu functions 
// ==========================================================================
// Last updated on 10/16/07; 4/16/11; 4/17/11
// ==========================================================================

#ifndef KAKADU_H
#define KAKADU_H

#include <string>

// Kakadu core includes
#include "kdu_elementary.h"
#include "kdu_messaging.h"
#include "kdu_params.h"
#include "kdu_compressed.h"
#include "kdu_sample_processing.h"
#include "kdu_arch.h"
// Application includes
#include "kdu_args.h"
#include "kdu_image.h"
#include "kdu_file_io.h"
#include "jpx.h"
#include "expand_local.h"

#include "math/basic_math.h"

namespace kakadufunc
{

// Methods derived from G99 codes received from Chris Bowen in April
// 2011:
   
   struct Wisp360PanFileHeader
   {
         int fileID;
         int compressSize;
         int width;
         int length;
         double fovX;
         double fovY;
         double yaw;
         double pitch;
         int frameHeaderSize;
         int temp;

         char extra[4040];
   };

   struct Wisp360PanFrameHeader
   {
         char dataFile[256];
         int filePos;
         int frameNum;

         double time;
         double integTime;

         unsigned long long int filePosLong;
         char extra[224];
   };

/****************************************************************************
 * CLASS                     kdu_simple_buffer_source
 *
 * A class for accessing a memory buffer as a kdu_compressed_source object.
 ****************************************************************************/

   class kdu_simple_buffer_source : public kdu_compressed_source 
   {
     public: // Member functions
      kdu_simple_buffer_source(const kdu_byte* buf_s, int max_bytes_s)
      { 
         buf = buf_s;
         max_bytes = max_bytes_s;
         bytes_read = 0;
      }
      ~kdu_simple_buffer_source() { }
      virtual int read(kdu_byte *buf_d, int num_bytes)
      { // [SYNOPSIS] See `kdu_compressed_source::read' for an explanation.
         int N = basic_math::min(num_bytes, max_bytes - bytes_read);
         memcpy(buf_d, buf+bytes_read, N);
         bytes_read += N;
         return N;
      }
      int bytes_read; //# of bytes read so far
     private: // Data
      int max_bytes; //maximum # of bytes allowed to read
      const kdu_byte* buf; //read from this buffer
   };

// --------------------------------------------------------------------------
   class hsdMemorySource : public kdu_compressed_source 
   {
     public:
      hsdMemorySource( const unsigned char *data, unsigned int size ) 
      {
         bufPtr = data; bufSize = size; offset = 0; dataSize = 0; }
      ~hsdMemorySource() { bufPtr = NULL; bufSize = 0; offset = 0; }
      int get_capabilities() { 
         return KDU_SOURCE_CAP_SEQUENTIAL | KDU_SOURCE_CAP_SEEKABLE; }
      int read( kdu_byte *buf, int numBytes ) 
      {
         if (offset + numBytes > bufSize) numBytes = (int)(bufSize - offset);
         if (numBytes <= 0) return 0;
         memcpy( buf, bufPtr + offset, numBytes );
         offset += numBytes;
         if (offset > dataSize) dataSize = offset;
         return numBytes;
      }
      bool seek( kdu_long offst ) { offset = offst; return true; }
      int  getSize() { return (int)dataSize; }
      kdu_long get_pos() { return offset; }

     private:
      const unsigned char* bufPtr;
      kdu_long bufSize;
      kdu_long dataSize;
      kdu_long offset;
   };

// ==========================================================================

   short* KDUReadJP2Mono16( 
      std::string filename, int& sizeX, int& sizeY, 
      bool isSigned = true, int precision = 16,short* outBuf = NULL);
   short* KDUReadJP2Mono16Buf( 
      unsigned char* buffer,int numBytes, int &sizeX, int &sizeY, 
      bool isSigned=true,int precision=16,int stride=-1,int res_level=0,
      short* outBuf=NULL);

   void ParseWISPHeaderFile(
      std::string header_filename,int& n_frames,int& JP2_size_in_bytes,
      int& frame_width,int& frame_height);
   short* ParseWISPFramesFile(
      std::string panos_filename,int JP2_size_in_bytes,int n_frames);
   short* ExtractWISPFrameData(
      std::string panos_filename,int framenumber,int JP2_size_in_bytes);

}

#endif  // kakadufunc namespace




