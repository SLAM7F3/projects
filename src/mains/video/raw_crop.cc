// ========================================================================
// Program RAW_CROP
// ========================================================================
// Last updated on 6/22/05
// ========================================================================

#include <algorithm>	// for std::_MIN() & std::_MAX() & auto_ptr<>
#include <iostream>

// #include <ipp>
//#include "D10GrabberFileHdr.h"
#include "grabberfilehdr.h"
// #include "stdafx.h"
#include "VidFile.h"

using std::cout;
using std::endl;

#define RAW_FORMAT 0
#define VID_FORMAT 1
#define IMG_FORMAT 2

// not sure what the next line means...6/22/05 at 7:18 am

// extern "C" int __cdecl _fseeki64( FILE *, __int64, int );

int main(int argc, char* argv[])
{
   FILE* fpOutImg = NULL;
   VidFile* outImg = NULL;
   FILE* fpImg = NULL;

   int in_numFrames, out_numFrames;
   int startFrame = -1, endFrame = 100, skip = 1;
   int crop_x0 = -1, crop_y0 = -1, crop_width = 0, crop_height = 0;
   int out_width, out_height, in_width, in_height;
   double scale = 1;
   bool wholeImage = true;
   int out_format = -1, in_format = -1;

   unsigned int totalBytesPerImgIn, totalBytesPerImgOut;

   int ii = 3;
   while (ii < argc)
   {
      if (strcmp(argv[ii],"-frames") == 0)
      {
         startFrame = atoi(argv[ii+1]);
         endFrame = atoi(argv[ii+2]);
         ii+=2;
      } 
      else if (strcmp(argv[ii],"-crop") == 0)
      {
         wholeImage = false;
         crop_x0 = atoi(argv[ii+1]);
         crop_y0 = atoi(argv[ii+2]);
         crop_width = atoi(argv[ii+3]);
         crop_height = atoi(argv[ii+4]);
         ii+=4;
      } 
      else if (strcmp(argv[ii],"-skip") == 0)
      {
         skip = atoi(argv[ii+1]);
         ii++;
      }
      else if (strcmp(argv[ii],"-scale") == 0)
      {
         scale = atof(argv[ii+1]);
         ii++;
      }
      else cout << "Illegal argument " << argv[ii] << endl;
      ii++;
   } // ii < argc while loop

   cout << "Reading file " << argv[1] << endl;

   char* outExtStr = argv[2] + strlen(argv[2]) - 4;
   char* inExtStr = argv[1] + strlen(argv[1]) - 4;

// OPEN INPUT IMAGE:

   in_format = RAW_FORMAT;
   fpImg = ::fopen( argv[1], "rb" );
   if ( !fpImg )
   {
      cout << "Bad RAW file !!" << endl;
      return 1;
   }

   tD10GrabberFileHdr	hdrD10in;
//      ::ZeroMemory( &hdrD10in, sizeof hdrD10in );
   memset(&hdrD10in,0,sizeof(hdrD10in));

	
   _fseeki64( fpImg, 0i64, SEEK_SET );
   //read input header
   ::fread( &hdrD10in, 1, sizeof hdrD10in, fpImg );
   tD10GrabberFileHdr hdrD10out = hdrD10in;
		
   totalBytesPerImgIn = hdrD10in.dwBytesPerImgDiv10 
      + hdrD10in.wBytesPerImgExtraDiv10;
   cout << "Bytes in header = " << sizeof(hdrD10in) << endl;
   cout << "Bytes in image = " << hdrD10in.dwBytesPerImgDiv10
        << " footer = " << hdrD10in.wBytesPerImgExtraDiv10
        << " total = " << totalBytesPerImgIn << endl;
      
// GET SIZE:

   __int64	i64;
		
   _fseeki64( fpImg, 0i64, SEEK_END );
   i64 = _telli64( _fileno(fpImg ) );
   i64 -= sizeof hdrD10in;
		
   i64 /= totalBytesPerImgIn;
   in_numFrames = (UINT)i64;

   in_width = hdrD10in.row_length;
   in_height = hdrD10in.col_length;
	
// DETERMINE OUTPUT SPECS:

   if (crop_x0 != -1)	 // crop
   {	       
      out_width = scale*crop_width;
      out_height = scale*crop_height;
   }
   else  // don't crop
   {				       
      crop_x0 = 0;
      crop_y0 = 0;
      crop_width = in_width;
      crop_height = in_height;
      out_width = scale*in_width;
      out_height = scale*in_height;
   }

   if (startFrame < 0) 
   {
      startFrame = 0;
      endFrame = in_numFrames - 1;
   }
   out_numFrames = ((endFrame - startFrame)/skip) + 1;

   cout << "Writing file " << argv[2] << " extension" << outExtStr << endl;

// WRITE OUTPUT HEADER:

   if (strcmp(outExtStr, ".vid")== 0)
   {
      out_format = VID_FORMAT;
      outImg = new VidFile();
      outImg->New_8U(argv[2],out_width,out_height,out_numFrames, 1);
   } 
   else if (strcmp(outExtStr, ".raw")== 0)
   {
      out_format = RAW_FORMAT;

// write output header:

      hdrD10out.frames_good = out_numFrames;
      hdrD10out.col_length = out_height;
      hdrD10out.row_length = out_width;
      hdrD10out.dwBytesPerImgDiv10 = (out_width*out_height);
      totalBytesPerImgOut = hdrD10out.dwBytesPerImgDiv10 
         + hdrD10out.wBytesPerImgExtraDiv10;
      fpOutImg = ::fopen( argv[2], "wb" );
      ::fwrite(&hdrD10out, sizeof(hdrD10out), 1, fpOutImg);
   }

// ADVANCE INPUT POINTER:

   if (in_format == RAW_FORMAT)
   {
      _fseeki64( fpImg, sizeof(hdrD10in), SEEK_SET );
      for (ii = 0; ii < startFrame; ii++)
      {
         _fseeki64( fpImg, totalBytesPerImgIn, SEEK_CUR );
      }	
   }

/*
//PREPARE BUFFERS/IPP:

   const IppiSize ippiszSrc = 
   {	
      in_width,	// width
      in_height	// height
   },
                                               
      ippiszDst = 
      {	
         out_width,	// width
         out_height	// height
      };
                                             
   const IppiRect ippirectSrcROI = 
   { crop_x0, crop_y0, crop_width, crop_height };
*/

   BYTE* pbyImgIn = new BYTE[totalBytesPerImgIn];
   BYTE* pbyImgIn2 = NULL;
	
   if (in_format == RAW_FORMAT)
   {
      pbyImgIn2 = pbyImgIn + 4;
   }
      
   BYTE* pbyImgOut = new BYTE[totalBytesPerImgOut];

      // only use if input raw
   tDiv10ImgInfo* infoIn = 
      (tDiv10ImgInfo *)(pbyImgIn + hdrD10in.dwBytesPerImgDiv10);
   tDiv10ImgInfo* infoOut = 
      (tDiv10ImgInfo *)(pbyImgOut + hdrD10out.dwBytesPerImgDiv10);

      //printf("output file should be %d bytes", sizeof(hdrD10out) + totalBytesPerImgOut*(1+endFrame-startFrame));
      
   cout << "Processing frames " << startFrame << " to " << endFrame 
        << endl;
   cout << "skip = " << skip << " scale = " << scale
        << " out_numframes = " << out_numframes << endl;

   for (ii = startFrame; ii <= endFrame; ii+= skip)
   {
         // READ DATA IN
      if (in_format == RAW_FORMAT)
      {
         ::fread( pbyImgIn, 1, totalBytesPerImgIn, fpImg );
         _fseeki64( fpImg, totalBytesPerImgIn*(skip - 1), SEEK_CUR );
      } 

      if (scale != 1)
      {
         ::ippiResize_8u_C1R(
            (const Ipp8u *)pbyImgIn2,
            ippiszSrc,
            in_width,
            ippirectSrcROI,
            (Ipp8u *)pbyImgOut,
            out_width,
            ippiszDst,
            scale,	// x-factor
            scale,	// y-factor
                                //IPPI_INTER_NN		// nearest neighbor interpolation
            IPPI_INTER_LINEAR	// linear interpolation
                                //IPPI_INTER_CUBIC	// cubic interpolation
                                //IPPI_INTER_SUPER	// supersampling interpolation,
                                // can not be applied for image enlarging.
            );

      } 
      else if (wholeImage) 
      { //just copy the whole image
         memcpy(pbyImgOut, pbyImgIn2, out_width*out_height);
      } 
      else 
      {		// copy cropped region
         BYTE* pby_tmp = pbyImgIn2 + (crop_y0*in_width + crop_x0);
         BYTE* pby_tmp2 = pbyImgOut;
         for (int jj = 0; jj < crop_height; jj++)
         {
            memcpy(pby_tmp2, pby_tmp, out_width);
            pby_tmp2 += out_width;
            pby_tmp += in_width;
         }
      }

// WRITE DATA:

      if (out_format == RAW_FORMAT)
      {
         memcpy(infoOut, infoIn, sizeof(tDiv10ImgInfo));
         ::fwrite(pbyImgOut, totalBytesPerImgOut, 1, fpOutImg);
      }
      else if (out_format == VID_FORMAT)
      {
         outImg->WriteFrame(pbyImgOut, out_width);
      }

      printf("%9d\r",ii);
   }

   if (outImg)
   {
      outImg->Close();
      delete outImg;
   }
   if (fpOutImg) fclose(fpOutImg);
   if (fpImg) fclose(fpImg);

   cout << "Done!!" << endl;
   
   return 0;
}

