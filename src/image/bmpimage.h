// ==========================================================================
// Header file for BMPImg class
// ==========================================================================
// Last modified on 9/3/11
// ==========================================================================

#ifndef BMPIMG_H
#define BMPIMG_H

#include <iostream>
#include <fstream>
#include <memory.h>

class BMPImg
{

#define IMG_OK 0x1
#define IMG_ERR_NO_FILE 0x2
#define IMG_ERR_MEM_FAIL 0x4
#define IMG_ERR_BAD_FORMAT 0x8
#define IMG_ERR_UNSUPPORTED 0x40

  public:

   BMPImg();
   ~BMPImg();
   int Load(char* szFilename);
   int GetBPP();
   int GetWidth();
   int GetHeight();
   unsigned char* GetImg();       // Return a pointer to image data
   unsigned char* GetPalette();   // Return a pointer to VGA palette

  private:

   unsigned int iWidth,iHeight,iEnc;
   short int iBPP,iPlanes;
   int iImgOffset,iDataSize;
   unsigned char *pImage, *pPalette, *pData;
  
   // Internal workers

   int GetFile(char* szFilename);
   int ReadBmpHeader();
   int LoadBmpRaw();
   int LoadBmpRLE8();
   int LoadBmpPalette();
   void FlipImg(); // Inverts image data, BMP is stored in reverse scanline order
};

#endif  // loadBMP

