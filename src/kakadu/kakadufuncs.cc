// ==========================================================================
// Kakadu functions
// ==========================================================================
// Last updated on 4/16/11; 4/17/11
// ==========================================================================

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include "kdu_stripe_decompressor.h"
#include "general/filefuncs.h"
#include "kakadu/kakadufuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::ofstream;
using std::string;
using std::vector;

namespace kakadufunc
{

   int numCPUs = 1;
   int doubleBufferSize = 16;

// ==========================================================================
// Methods below are derived from Group 99 codes received from Chris
// Bowen in April 2011
// ==========================================================================

/*
 * function KDUReadJP2Mono16
 *
 * Read a "Smart" JPEG2000 file that has been written with KDUWriteJP2Mono16.
 * We only properly read the GeoJP2 data if it was written by our routine.
 *
 * The output image returned is a signed 16-bit buffer.  If an output buffer
 * is passed in through 'outBuf', this is returned on success.  If outBuf is
 * NULL, the returned buffer is allocated in this routine and the calling
 * routine must deallocate the image when done with it.  On error, the
 * returned pointer is always NULL.
 *
 * Inputs:
 * 'filename' is the input file name.
 * 'ROI' is the region of interest.  If all values are 0, the whole image.
 *
 * When ROI is all zeroes, the following outputs are set, otherwise, they
 * are meaningless:
 * 'sizeX' and 'sizeY' are the size of the image in pixels.
 * 'utm' is a RECT containing the upper-left and lower-right corners of the
 *      image to the nearest meter. 
 * 'zone' is the output UTM zone (1 or 2-digit ASCII number and one letter).
 * 'time64' is the output UTC time nugget which indicates the time of the
 *         image, which could be 0 if no time was encoded.
 */

   short* KDUReadJP2Mono16(string filename,int& sizeX,int& sizeY, 
   bool isSigned, int precision, short* outBuf)
   {

      int blockSize;
      int numComponents = 1;
      short* buf;
      jp2_family_src jp2_ultimate_src;
      jp2_source jp2_in;
      kdu_codestream codestream;
      siz_params *siz;
      kdu_thread_env threadEnv;
      kdu_thread_env *threadEnvRef = NULL;

// Check inputs:

      if (precision < 10 || precision > 16) return NULL;

// Redirect the kdu error routine so that kdu errors don't terminate the app.

//      kdu_customize_errors( &err_formatter );

/*
// Set up multi-threaded environment.

      if (numCPUs > 1) 
      {
         int nt = numCPUs;
         threadEnv.create();
         for (int ct = 1; ct < nt; ct++)
            if (!threadEnv.add_thread()) {
               nt = ct; // Unable to create all the threads requested
               PutLog( "KDUReadJP2Mono16 - Tried to create %d threads, but created %d instead\n",
               numCPUs, nt );
            }
         threadEnvRef = &threadEnv;
      }
*/

// Open up through KDU and get codestream/size info:

      try {
         jp2_ultimate_src.open( filename.c_str(), true );
         if (jp2_in.open( &jp2_ultimate_src ) == false)
            throw 0;
         else {
            if (!jp2_in.read_header()) {
               jp2_in.close();
               return NULL;
            }
         }
      }
      catch (...) {
// File open failed.
         sizeX = 0;
         sizeY = 0;
         return NULL;
      }

      codestream.create( &jp2_in, threadEnvRef );

      kdu_dims* reg_ptr = NULL;

// If all elements of the ROI are 0, get the whole image,
// otherwise, pull out the requested region.

//  if (ROI.pos.x == 0  && ROI.pos.y == 0 &&
//      ROI.size.x == 0 && ROI.size.y == 0) 
      {
         siz = codestream.access_siz();
         siz->get( Ssize, 0, 0, sizeY );
         siz->get( Ssize, 0, 1, sizeX );
         reg_ptr = NULL;
      } 
//  else
//  {
//  reg_ptr = &ROI;
//  }

      codestream.apply_input_restrictions( 0, numComponents, 0, 0, reg_ptr );
//codestream.apply_input_restrictions( 0, numComponents, 2, 0, reg_ptr );

      blockSize = sizeX * sizeY;
      if (outBuf == NULL) 
      {
         buf = (short *)malloc( blockSize*sizeof(short) );
         if (buf == NULL) {
            codestream.destroy();
            jp2_in.close();
            return NULL;
         }
      } 
      else
      {
         buf = outBuf;
      }
      
// Try to read out the "smart" GeoJP2 information.

      bool found = false;
      jp2_input_box smartBox;
//      char datetime[DATESIZE];
//      double geodoubles[GEODOUBLESIZE];
//      unsigned char geoBoxAndHeader[TIFFBOXSIZE + HEADERCHARSIZE];
//      unsigned short geokeys[GEOKEYSIZE];
//      TIFFTagA tifftags[NUMTIFFTAGS];


      if (smartBox.open( &jp2_ultimate_src )) 
      {
//PutLog( "KDUReadJP2Mono16 - opened first box\n" );
         if (smartBox.get_box_type() == jp2_uuid_4cc) 
         {
//PutLog( "KDUReadJP2Mono16 - first box is UUID\n" );
            found = true;
         } else {
            bool openok;
            do {
               smartBox.close();
               openok = smartBox.open_next();
               if (openok) {
//PutLog( "KDUReadJP2Mono16 - opened another box\n" );
                  if (smartBox.get_box_type() == jp2_uuid_4cc) {
//                     PutLog( "KDUReadJP2Mono16 - found UUID box\n" );
                     found = true;
                  }
               }
            } while (openok == true && found == false);
         }
         if (found == true) 
         {
            double res;
//            smartBox.read( geoBoxAndHeader, TIFFBOXSIZE + HEADERCHARSIZE );
//            smartBox.read( (unsigned char *)tifftags, 
//            sizeof(TIFFTagA) * NUMTIFFTAGS );
//            smartBox.read( (unsigned char *)geokeys, 
//            sizeof(unsigned short) * GEOKEYSIZE);
//            smartBox.read( (unsigned char *)geodoubles, 
//            sizeof(double) * GEODOUBLESIZE );
//            smartBox.read( (unsigned char *)datetime, 
//            sizeof(unsigned char) * DATESIZE);
            smartBox.close();
//            res = geodoubles[0];
         } 
         else 
         {
//            PutLog( "KDUReadJP2Mono16 - never found UUID box\n" );
//            res    = 0;
         }
      }

// Extract region directly from the image using kdu_stripe_decompressor

      int preferred_min_stripe_height = 8;
      int absolute_max_stripe_height = 1024;
      int precisions[1] = { precision };
      int stripe_heights[1];
      int max_stripe_heights[1];
      bool signs[1] = { isSigned };
      kdu_stripe_decompressor decompressor;

      decompressor.start( 
         codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
      decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
      absolute_max_stripe_height,
      stripe_heights, max_stripe_heights );
      bool continues = true;
      short* bufptr = buf;
      while (continues) 
      {
         decompressor.get_recommended_stripe_heights( 
            preferred_min_stripe_height,
            absolute_max_stripe_height,
            stripe_heights, NULL );
         continues = decompressor.pull_stripe( 
            bufptr, stripe_heights, NULL, NULL,
            NULL, precisions, signs );
         bufptr += stripe_heights[0] * sizeX;
      }
      decompressor.finish();
      codestream.destroy();
      jp2_in.close();
      if (jp2_ultimate_src.exists())
         jp2_ultimate_src.close();
      if (threadEnv.exists())
         threadEnv.destroy();

// Convert to unsigned representation

      if (!isSigned) 
      {
         int offset = (1 << precision) >> 1;
         for (int i = 0 ; i < sizeX * sizeY; i++)
            buf[i] = buf[i] + offset;
      }

      return buf;

   } // end KDUReadJP2Mono16 

// --------------------------------------------------------------------------
/* function KDUReadJP2Mono16Buf
 *
 * Read a "Smart" JPEG2000 file that has been written with KDUWriteJP2Mono16.
 * We only properly read the GeoJP2 data if it was written by our routine.
 *
 * The output image returned is a signed 16-bit buffer, and the calling
 * routine must deallocate the image when done with it.  On error, the
 * returned pointer is NULL.
 *
 * Inputs:
 * 'filename' is the input file name.
 * 'ROI' is the region of interest.  If all values are 0, the whole image.
 *
 * When ROI is all zeroes, the following outputs are set, otherwise, they
 * are meaningless:
 * 'sizeX' and 'sizeY' are the size of the image in pixels.
 * 'utm' is a RECT containing the upper-left and lower-right corners of the
 *      image to the nearest meter. 
 * 'zone' is the output UTM zone (1 or 2-digit ASCII number and one letter).
 * 'time64' is the output UTC time nugget which indicates the time of the
 *         image, which could be 0 if no time was encoded.
 ***************************************************************************/

   short* KDUReadJP2Mono16Buf( 
      unsigned char* buffer,int numBytes,int& sizeX,int& sizeY, 
      bool isSigned,int precision,int stride,int res_level,short* outBuf)
   {
      std::auto_ptr<hsdMemorySource> memTgt;
      int blockSize;
      int numComponents = 1;
      short *buf;
      jp2_family_src jp2_ultimate_src;
      jp2_source jp2_in;
      kdu_codestream codestream;
      siz_params *siz;
      kdu_thread_env threadEnv;
      kdu_thread_env* threadEnvRef = NULL;

// Check inputs

      if (precision < 10 || precision > 16) return NULL;

// Redirect the kdu error routine so that kdu errors don't terminate the app.
//      kdu_customize_errors( &err_formatter );

/*
// Set up multi-threaded environment.
      if (numCPUs > 1) {
         int nt = numCPUs;
         threadEnv.create();
         for (int ct = 1; ct < nt; ct++)
            if (!threadEnv.add_thread()) {
               nt = ct; // Unable to create all the threads requested
               PutLog( "KDUReadJP2Mono16 - Tried to create %d threads, but created %d instead\n",
               numCPUs, nt );
            }
         threadEnvRef = &threadEnv;
      }
*/

// Open up through KDU and get codestream/size info.

      int jp2Open = 1;
      kdu_simple_buffer_source input( buffer, numBytes );

      if((buffer[0] == 0xff) && (buffer[1] == 0x4f)) 
      { // raw codestream
         jp2Open = 0;
      } 
      else 
      {
         try {
            memTgt.reset(new hsdMemorySource( buffer, numBytes ));
            jp2_ultimate_src.open( memTgt.get() );

            //jp2_ultimate_src.open( filename, true );
            if (jp2_in.open( &jp2_ultimate_src ) == false){
               throw 0;
            } else {
               if (!jp2_in.read_header()) {
                  jp2_in.close();
                  return NULL;
               }else{
                  codestream.create( &jp2_in, threadEnvRef );
               }
            }
         }
         catch (...) {
            // File open failed.  Maybe it's a tile.
            jp2Open = 0;
            //sizeX = 0;
            //sizeY = 0;
            //return NULL;
         }
      }

      try 
      {
         if(!jp2Open)
            codestream.create( &input, threadEnvRef );
      } 
      catch (...) 
      {
         return NULL;
      }

      //codestream.create( &jp2_in, threadEnvRef );

      kdu_dims* reg_ptr = NULL;

      /* If all elements of the ROI are 0, get the whole image,
       * otherwise, pull out the requested region.
       */
      siz = codestream.access_siz();
      siz->get( Ssize, 0, 0, sizeY );
      siz->get( Ssize, 0, 1, sizeX );
      reg_ptr = NULL;

      codestream.apply_input_restrictions( 
         0, numComponents, res_level, 0, reg_ptr );

      //codestream.set_block_truncation(2048);  this looks like crap but makes it go fast

      blockSize = sizeX * sizeY;
      if (outBuf == NULL) {
         buf = (short *)malloc( blockSize*sizeof(short) );
         if (buf == NULL) {
            codestream.destroy();
            if(jp2Open)
               jp2_in.close();
            return NULL;
         }
      } 
      else
      {
         buf = outBuf;
      }
      
/*
//       Try to read out the "smart" GeoJP2 information.

      bool found = false;
      jp2_input_box smartBox;
      char datetime[DATESIZE];
      double geodoubles[GEODOUBLESIZE];
      unsigned char geoBoxAndHeader[TIFFBOXSIZE + HEADERCHARSIZE];
      unsigned short geokeys[GEOKEYSIZE];
      TIFFTagA tifftags[NUMTIFFTAGS];

      if (jp2Open && smartBox.open( &jp2_ultimate_src )) {
         //PutLog( "KDUReadJP2Mono16 - opened first box\n" );
         if (smartBox.get_box_type() == jp2_uuid_4cc) {
            //PutLog( "KDUReadJP2Mono16 - first box is UUID\n" );
            found = true;
         } else {
            bool openok;
            do {
               smartBox.close();
               openok = smartBox.open_next();
               if (openok) {
                  //PutLog( "KDUReadJP2Mono16 - opened another box\n" );
                  if (smartBox.get_box_type() == jp2_uuid_4cc) {
                     PutLog( "KDUReadJP2Mono16 - found UUID box\n" );
                     found = true;
                  }
               }
            } while (openok == true && found == false);
         }
         if (found == true) {
            double res;
            smartBox.read( geoBoxAndHeader, TIFFBOXSIZE + HEADERCHARSIZE );
            smartBox.read( (unsigned char *)tifftags, sizeof(TIFFTagA) * NUMTIFFTAGS );
            smartBox.read( (unsigned char *)geokeys, sizeof(unsigned short) * GEOKEYSIZE);
            smartBox.read( (unsigned char *)geodoubles, sizeof(double) * GEODOUBLESIZE );
            smartBox.read( (unsigned char *)datetime, sizeof(unsigned char) * DATESIZE);
            smartBox.close();
            utm.left = (long)geodoubles[6];
            utm.top  = (long)geodoubles[7];
            res = geodoubles[0];
            utm.right  = (long)(utm.left + (res * sizeX));
            utm.bottom = (long)(utm.top - (res * sizeY));
            UTCTime( datetime, time64 );
            GetUTMCode( geokeys[15], utm.top, zone );
         } else {
            PutLog( "KDUReadJP2Mono16 - never found UUID box\n" );
            res    = 0;
            time64 = 0;
            utm.left   = 0;
            utm.right  = 0;
            utm.top    = 0;
            utm.bottom = 0;
            if (zone != NULL)
               zone[0] = '\0';
         }
      }
*/

      if (stride==-1) 
      {
         stride = sizeX;
      }
    
      // Extract region directly from the image using kdu_stripe_decompressor

      int preferred_min_stripe_height = 8;
      int absolute_max_stripe_height = 1024;
      int precisions[1] = { precision };
      int stripe_heights[1];
      int max_stripe_heights[1];
      bool signs[1] = { isSigned };
      int row_gaps[1] = {stride};
      kdu_stripe_decompressor decompressor;

      decompressor.start( 
         codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
      decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
      absolute_max_stripe_height,
      stripe_heights, max_stripe_heights );
      bool continues = true;
      short *bufptr = buf;
      while (continues) {
         decompressor.get_recommended_stripe_heights( 
            preferred_min_stripe_height,
            absolute_max_stripe_height,
            stripe_heights, NULL );
         continues = decompressor.pull_stripe( 
            bufptr, stripe_heights, NULL, NULL,
            row_gaps, precisions, signs );
         bufptr += stripe_heights[0] * stride;
      }
      decompressor.finish();
      codestream.destroy();
      if(jp2Open)
         jp2_in.close();
      if (jp2Open && jp2_ultimate_src.exists())
         jp2_ultimate_src.close();
      if (threadEnv.exists())
         threadEnv.destroy();

      // Convert to unsigned representation
#if 0
      if (!isSigned) {
         int offset = (1 << precision) >> 1;
         for (int i = 0 ; i < sizeX * sizeY; i++)
            buf[i] = buf[i] + offset;
      }
#endif
      return buf;

   } 

// --------------------------------------------------------------------------
// Method ParseWISPHeaderFile() opens the header (or "index") file,
// reads the size of the compressed frames, determines the size of the
// file, and calculates the number of JP2 frames.

   void ParseWISPHeaderFile(
      string header_filename,int& n_frames,int& JP2_size_in_bytes,
      int& frame_width,int& frame_height)
   {
      string banner="Reading in WISP header file:";
      outputfunc::write_big_banner(banner);

      FILE* m_fid = fopen(header_filename.c_str(), "rb");
      if( !m_fid)
      {
         // Throw an exception
      }

// Read and parse the contents of the header file.  It first contains
// a Wisp360PanFileHeader followed by n_frames *
// Wisp360PanFrameHeaders.

      Wisp360PanFileHeader m_fileHeader;
//      cout << "sizeof(Wisp360PanFileHeader) = "
//           << sizeof(Wisp360PanFileHeader) << endl;
   
      fread(&m_fileHeader, sizeof(Wisp360PanFileHeader), 1, m_fid);

      JP2_size_in_bytes=m_fileHeader.compressSize;
      frame_width=m_fileHeader.width;
      frame_height=m_fileHeader.length;
      
      cout << "fileID = " << m_fileHeader.fileID << endl;
      cout << "compressSize = " << m_fileHeader.compressSize << endl;
      cout << "width = " << m_fileHeader.width << endl;
      cout << "length = " << m_fileHeader.length << endl;
      cout << "fovX = " << m_fileHeader.fovX << endl;
      cout << "fovY = " << m_fileHeader.fovY << endl;
      cout << "yaw = " << m_fileHeader.yaw << endl;
      cout << "pitch = " << m_fileHeader.pitch << endl;
      cout << "frameHeaderSize = " << m_fileHeader.frameHeaderSize << endl;
      cout << "temp = " << m_fileHeader.temp << endl;

// Make sure this is a proper wisp360 header file

      if(m_fileHeader.fileID != 0x31593601)
      {
         cout << "Not a proper wisp360 file!" << endl;
         // Throw an exception
      }

// Copy some variables

      int m_panSizeX = m_fileHeader.width;
      int m_panSizeY = m_fileHeader.length;
      double m_panFovX = m_fileHeader.fovX;
      double m_panFovY = m_fileHeader.fovY;
      double m_panCenterX = m_fileHeader.yaw;
      double m_panCenterY = m_fileHeader.pitch;
   
      double m_panUpLeftX = m_panCenterX - m_panFovX/2;
      double m_panUpLeftY = m_panCenterY + m_panFovY/2;
      double m_srcPixPerDegX = m_panSizeX / m_panFovX;
      double m_srcPixPerDegY = m_panSizeY / m_panFovY;

      m_panUpLeftX = m_panCenterX - m_panFovX/2;
      m_panUpLeftY = m_panCenterY + m_panFovY/2;
      m_srcPixPerDegX = m_panSizeX / m_panFovX;
      m_srcPixPerDegY = m_panSizeY / m_panFovY;

// Header file contains Wisp360PanFileHeader at its top.  It then
// contains Wisp360PanFrameHeaders for each of the WISP frames.  So
// total number of JP2 frames =
// (headerfile_size-sizeof(Wisp360PanFileHeader)) / frameHeaderSize

// Note: Wisp panos file contains just concatenated compressed JP2s
// with no header information.  So total number JP2 frames =
// sizeof(pano_file)/compressed_jp2_size:

// Compute number of compressed JP2 frames that reside in panofile:

      int headerfile_size=filefunc::size_of_file_in_bytes(header_filename);
//   cout << "headerfile_size = " << headerfile_size << endl;
      n_frames = double(headerfile_size-sizeof(Wisp360PanFileHeader))/
         double(m_fileHeader.frameHeaderSize);

//      int panofile_size=filefunc::size_of_file_in_bytes(panos_filename);
//      cout << "panofile_size = " << panofile_size << endl;
//      int n_frames=panofile_size/compressed_JP2_size;

      cout << "n_frames = " << n_frames << endl;

// Read in entire header file:
   
      vector<Wisp360PanFrameHeader> frameHeader;
      frameHeader.resize(n_frames);
      fread(&(frameHeader[0]), m_fileHeader.frameHeaderSize, 
      n_frames, m_fid);

      for (int i=0; i<n_frames; i++)
      {
         Wisp360PanFrameHeader curr_frameHeader=frameHeader[i];
         string datafilename(curr_frameHeader.dataFile);
         int fileposn=curr_frameHeader.filePos;
         int framenumber=curr_frameHeader.frameNum;
         double frametime=curr_frameHeader.time;
         double integtime=curr_frameHeader.integTime;
         unsigned long long int filePosLong=curr_frameHeader.filePosLong;
      
//         cout << "datafilename = " << datafilename << endl;
//         cout << "fileposn = " << fileposn << endl;
//         cout << "framenumber = " << framenumber << endl;
//         cout << "frametime = " << frametime << endl;
//         cout << "integtime = " << integtime << endl;
//         cout << "fileposlong = " << filePosLong << endl;
      } // loop over index i labeling JP2 frames
   }
   
// --------------------------------------------------------------------------
// Method ParseWISPFramesFile() takes in the name for a WISP data file
// containing concatenated JP2s.  It also takes in the size in bytes
// of each JP2 along with the total number of frames in the data file.
// This method instantiates a 16-byte integer array and fills it with
// all of the WISP intensity values for each frame.

   short* ParseWISPFramesFile(
      string panos_filename,int JP2_size_in_bytes,int n_frames)
   {
      string banner="Reading in WISP frames:";
      outputfunc::write_big_banner(banner);

      FILE* m_fidData = fopen(panos_filename.c_str(), "rb");
      
      int n_bytes=n_frames*JP2_size_in_bytes;
      unsigned char* jp2data_ptr=new unsigned char[n_bytes];

      fread(jp2data_ptr, n_bytes, 1, m_fidData);

      int frame_width,frame_height;
      short* buffer_ptr=KDUReadJP2Mono16Buf( 
         jp2data_ptr,n_bytes,frame_width,frame_height);
      fclose(m_fidData);

      return buffer_ptr;
   }

// --------------------------------------------------------------------------
// Method ExtractWISPFrameData() takes in the filename for the
// concatenated WISP JP2 data for all WISP frames.  It seeks to the
// location in the data file of the frame specified by input
// framenumber.  Raw 16-byte intensity values are then read out from
// the data file into a dynamically instantiated array of short
// integers.  The intensity value array is returned by this method.

   short* ExtractWISPFrameData(
      string panos_filename,int framenumber,int JP2_size_in_bytes)
   {
      cout << "Extracting WISP data for frame " << framenumber << endl;

      FILE* m_fidData = fopen(panos_filename.c_str(), "rb");

      long byte_offset=framenumber*JP2_size_in_bytes;
      fseek(m_fidData,byte_offset,SEEK_SET);
      
      unsigned char* jp2data_ptr=new unsigned char[JP2_size_in_bytes];

      fread(jp2data_ptr, JP2_size_in_bytes, 1, m_fidData);
      fclose(m_fidData);

      int frame_width,frame_height;
      short* buffer_ptr=KDUReadJP2Mono16Buf( 
         jp2data_ptr,JP2_size_in_bytes,frame_width,frame_height);

      return buffer_ptr;
   }


} // kakadufunc namespace


