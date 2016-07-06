/*---------------------------------------------------------------------------
 *                        Copyright © 2005-2009
 *                   Massachusetts Institute of Technology
 *                         All rights reserved
 *---------------------------------------------------------------------------
 *
 * Description: Kakadu support routines.
 *
 * Author: Herb DaSilva - June 9, 2006
 *---------------------------------------------------------------------------*/

#ifndef KAKADUSUPPORT_HEADER
#define KAKADUSUPPORT_HEADER

#ifdef LINUX
#include "linux_defs.h"
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "jp2.h"
#include "jpx.h"
#include "kdu_stripe_decompressor.h"
#include "kdu_stripe_compressor.h"
#include "PutLog.h"
#include <vector>
#include "CBox.h"

#include <string>
#include <stdexcept>

#ifndef ZONE_LEN
#define ZONE_LEN 4
#endif

#ifndef MIN
#define MIN(x, y) ((x)<(y)?(x):(y))
#endif


/******************************************************************************
 * This class is used so that the file->open in the jp2 library doesn't
 * exit the freakin' app if the file can't be opened (which is the default
 * behavior).  Basically, it redirects error messages to PutLog, then throws
 * an exception we can catch.
 ******************************************************************************/

class kd_core_message_collector : public kdu_message {
public:
  kd_core_message_collector(bool for_errors) { end_exception = for_errors; }
  void put_text(const char *string) {
      //std::string err_msg = string;
      PutLog( "%s", string );
      //message += string;
  }
  void flush(bool end_of_message) {
      if (end_of_message) {
          if (end_exception)
              //throw (int)0;
              throw std::runtime_error("kdu error");
      }
  }
private:
  bool end_exception;
};



/******************************************************************************
 * CLASS                     kdu_simple_buffer_source
 *
 * A class for accessing a memory buffer as a kdu_compressed_source object.
 ******************************************************************************/

class kdu_simple_buffer_source : public kdu_compressed_source {
public: // Member functions
  kdu_simple_buffer_source(const kdu_byte *buf_s, int max_bytes_s)
  { 
    buf = buf_s;
    max_bytes = max_bytes_s;
    bytes_read = 0;
  }
  ~kdu_simple_buffer_source() { }
  virtual int read(kdu_byte *buf_d, int num_bytes)
  { /* [SYNOPSIS] See `kdu_compressed_source::read' for an explanation. */
    int N = MIN(num_bytes, max_bytes - bytes_read);
    memcpy(buf_d, buf+bytes_read, N);
    bytes_read += N;
    return N;
  }
  int bytes_read; //# of bytes read so far
private: // Data
  int max_bytes; //maximum # of bytes allowed to read
  const kdu_byte *buf; //read from this buffer
};


bool KDUGetJP2Size( const char filename[], int &sizeX, int &sizeY);
bool KDUGetJP2SizeBuffer( const unsigned char *buffer, int numBytes, int &sizeX, int &sizeY);

void KakaduInit( int limit = 0);
void KDUInit( int limit = 0 );
int  KDUGetCPUCount( void );
int  KDUWriteJP2RGB24( char filename[], unsigned char *rgb, int sizeX, int sizeY,
			  RECT utm, char zone[], double res, __int64 time64 );
int  KDUWriteJP2Mono8( char filename[], unsigned char *data, int sizeX, int sizeY, RECT utm,
			  char zone[], double res, __int64 time64, long targetSizeBytes );
int  KDUWriteJP2Mono8NoGeo( char filename[], unsigned char *data, int sizeX, int sizeY,
			       long targetSizeBytes );
int  KDUWriteJP2Mono16( char filename[], short *gray, int sizeX, int sizeY,
			   RECT utm, char zone[], double res, __int64 time64,
			   long targetSizeBytes, bool isSigned = true, int precision = 16 );
int  KDUWriteJP2Mono16NoGeo( char filename[], short *gray, int sizeX, int sizeY,
				long targetSizeBytes, bool isSigned = true, int precision = 16 );
unsigned char *KDUCompressJP2Mono8( unsigned char *inBuf, unsigned char *outBuf, int sizeX, int sizeY,
				       RECT utm, char zone[ZONE_LEN], double res, __int64 time64,
				       long targetSizeBytes, int *actualSizeBytes );
unsigned char *KDUCompressJP2Mono16( short *gray, unsigned char *output, int sizeX, int sizeY, long targetSizeBytes,
					int *actualSizeBytes, bool isSigned = true, int precision = 16, int stride=-1 );
unsigned char *KDUCompressJP2RGB16( short *rgb, unsigned char *output, int sizeX, int sizeY,
                    long targetSizeBytes, int *actualSizeBytes,bool isSigned = true, int precision=16, int stride=-1);
unsigned char *KDUReadJP2Mono8( char filename[], unsigned char *outBuf, kdu_dims ROI, int &sizeX,
				   int &sizeY, double &res, RECT &utm, char zone[], __int64 &time64, int res_level=0 );
unsigned char *KDUReadJP2Mono8NoGeo( char filename[], unsigned char *outBuf, kdu_dims ROI,
					int &sizeX, int &sizeY );
short *KDUReadJP2Mono16( char filename[], kdu_dims ROI, int &sizeX, int &sizeY, double &res, RECT &utm,
			    char zone[], __int64 &time64, bool isSigned = true, int precision = 16,
			    short *outBuf = NULL );
unsigned char *KDUReadJP2Mono8Buf( unsigned char *buffer, unsigned char *outBuf, int numBytes, int numCPUs, kdu_dims ROI,
				   int &sizeX, int &sizeY, double &res,
				   RECT &utm, char zone[], __int64 &time64 );
short *KDUReadJP2Mono16Buf( unsigned char *buffer, short *outBuf, int numBytes, kdu_dims ROI,
								int &sizeX, int &sizeY, double &res, 
								RECT &utm, char zone[], __int64 &time64,
                            bool isSigned, int precision, int stride=-1, int res_level=0 );
short *KDUReadJP2RGB16Buf( unsigned char *buffer, short *outBuf, int numBytes, kdu_dims ROI,
								int &sizeX, int &sizeY, double &res, 
								RECT &utm, char zone[], __int64 &time64,
								bool isSigned, int precision, int stride=-1 );
short *KDUReadJP2Mono16Buf( FILE *fid, short *outBuf, kdu_long fileOffset, kdu_dims ROI,
								int &sizeX, int &sizeY, double &res, 
								RECT &utm, char zone[], __int64 &time64,
                            bool isSigned, int precision, int stride=-1, int resolution=0 );
int KDUGetGeoJP2NoData( char filename[], int &sizeX, int &sizeY, RECT &utm, char zone[], __int64 &time64 );
//void KDUDecompressROI( kdu_byte *buffer_out, kdu_byte *buffer_com, int bytes_compressed, kdu_dims ROI );
void KDUDecompressROI( kdu_byte *buffer_out, kdu_byte *buffer_com,
			  int bytes_compressed, kdu_dims ROI, int &sizeX, int &sizeY, int res_level=0 );

void KDUSimpleDecompress( kdu_byte *buffer_out, kdu_byte *buffer_com, int bytes_compressed );
kdu_byte *KDUDecompress( kdu_byte *buffer_out, kdu_byte *buffer_com, int bytes_compressed,
			    int &width, int &height, int &prec, int &channels );
kdu_long KDUSimpleCompress( kdu_byte *buffer_in, int width, int height, 
			       kdu_byte *buffer_com, int bytes_compressed );
unsigned char *KDUGetRegionJPGServer( char *fullPath, kdu_dims ROI, int &bufSize, short &compress );
unsigned char *KDUGetRegionCondorServer( FILE *fid, kdu_long fileOffset, kdu_dims ROI, std::vector<unsigned char> &outbuf, int res_level=0 );

#endif
