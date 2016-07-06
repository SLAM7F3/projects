// ==========================================================================
// BMPImg class member function definitions
// ==========================================================================
// Last modified on 9/3/11
// =========================================================================

#include "image/bmpimage.h"

BMPImg::BMPImg()

{ 
   pImage=pPalette=pData=NULL;
   iWidth=iHeight=iBPP=iPlanes=iEnc=0;
}

BMPImg::~BMPImg()

{
   if(pImage)
   {
      delete [] pImage;
      pImage=NULL;
   }

   if(pPalette)
   {
      delete [] pPalette;
      pPalette=NULL;
   }

   if(pData)
   {
      delete [] pData;
      pData=NULL;
   }
}


int BMPImg::Load(char* szFilename)

{
   int iRet;

   // Clear out any existing image and palette
   if(pImage)
   {
      delete [] pImage;
      pImage=NULL;
   }

   if(pPalette)
   {
      delete [] pPalette;
      pPalette=NULL;
   }

   // Get the file into memory
   iRet=GetFile(szFilename);

   if(iRet!=IMG_OK)
      return iRet;

   // Process the header
   iRet=ReadBmpHeader();

   if(iRet!=IMG_OK)
      return iRet;

   if(iBPP<8) // We'll only bother with 8 bit and above
      return IMG_ERR_UNSUPPORTED;

   // Get the image data
   switch(iEnc)
   {
      case 0: // Uncompressed
         iRet=LoadBmpRaw(); // 8 / 24 Bit. (24 bit is in BGR order)
         break;

      case 1: // RLE 8 (Indexed 256 colour only)
         iRet=LoadBmpRLE8();
         break;

      case 2: // RLE 4 (16 Colour indexed, Outdated, not covered here)
         return IMG_ERR_UNSUPPORTED;

      case 3: // Bitfields (16/32 bit only, Rare, not covered here)
         return IMG_ERR_UNSUPPORTED;

      default:
         return IMG_ERR_UNSUPPORTED;
   }

   if(iRet!=IMG_OK)
      return iRet;

   // Flip image to correct scanline reversal
   FlipImg();

   // Load palette if present
   iRet=LoadBmpPalette();

   if(iRet!=IMG_OK)
      return iRet;

   // Free the file data
   delete [] pData;
   pData=NULL;

   return IMG_OK;
}


int BMPImg::GetFile(char* szFilename)

{
   using namespace std;
   ifstream fIn;
   unsigned long ulSize;

   // Open the specified file
   fIn.open(szFilename,ios::binary);
   
   if(fIn==NULL)
      return IMG_ERR_NO_FILE;

   // Get file size
   fIn.seekg(0,ios_base::end);
   ulSize=fIn.tellg();
   fIn.seekg(0,ios_base::beg);

   // Allocate some space
   // Check and clear pDat, just in case
   if(pData)
   {
      delete [] pData; 
      pData=NULL;
   }

   pData=new unsigned char[ulSize];

   if(pData==NULL)
   {
      fIn.close();
      return IMG_ERR_MEM_FAIL;
   }

   // Read the file into memory
   fIn.read((char*)pData,ulSize);

   fIn.close();

   return IMG_OK;
}


int BMPImg::ReadBmpHeader()

{
   int iInfo;

   if(pData==NULL)
      return IMG_ERR_NO_FILE;

   if(pData[0x0]!='B' || pData[0x1]!='M') // BMP ID Bytes, should be 'BM'
      return IMG_ERR_BAD_FORMAT;

   memcpy(&iImgOffset,&pData[0xA],4);   // Offset to image data
  
   memcpy(&iInfo,&pData[0xE],4);        // Info header size, should be 0x28
   if(iInfo!=0x28)
      return IMG_ERR_BAD_FORMAT;

   memcpy(&iWidth,&pData[0x12],4);   // Image width
   memcpy(&iHeight,&pData[0x16],4);  // Image height
   memcpy(&iPlanes,&pData[0x1A],2);  // Colour planes
   memcpy(&iBPP,&pData[0x1C],2);     // BPP
   memcpy(&iEnc,&pData[0x1E],4);     // Encoding

   iDataSize=(iWidth*iHeight*(iBPP/8)); // Calculate Image Data size

   return IMG_OK;
}


int BMPImg::LoadBmpRaw()

{
   if(pImage)
   {
      delete [] pImage;
      pImage=NULL;
   }

   // Allocate space for the image data
   pImage=new unsigned char[iDataSize];

   if(pImage==NULL)
      return IMG_ERR_MEM_FAIL;

   memcpy(pImage,&pData[iImgOffset],iDataSize);

   return IMG_OK;
}


int BMPImg::LoadBmpRLE8()

{
   unsigned char bOpCode,bVal;
   unsigned char *pSrc;
   int iDcode=1,iCount,iPos,iIndex;

   // Allocate space for the image  
   if(pImage)
      delete [] pImage;

   pImage=new unsigned char[iDataSize];

   if(pImage==NULL)
      return IMG_ERR_MEM_FAIL;

   // Get the start of the RLE data
   pSrc=&pData[iImgOffset];

   iPos=0;
   iIndex=0;

   while(iDcode)
   {
      // Stay on even bytes
      while(iPos%2)
      {
         iPos++;
      }

      bOpCode=pSrc[iPos];
      bVal=pSrc[iPos+1];
      iPos+=2;

      if(bOpCode>0) // Run mode, Repeat 'bVal' 'OpCode' times
      {
         for(iCount=0;iCount!=bOpCode;iCount++)
         {
            pImage[iIndex]=bVal;
            ++iIndex;
         }
      }
      else // Absolute Mode (Opcode=0), various options
      {
         switch(bVal)
         {
            case 0:  // EOL, no action
               break;

            case 1:  // EOF, STOP!
               iDcode=0;
               break;

            case 2:  // Reposition, Never used
               break;

            default: // Copy the next 'bVal' bytes directly to the image
               for(iCount=bVal;iCount!=0;iCount--)
               {
                  pImage[iIndex]=pSrc[iPos];
                  ++iIndex;
                  ++iPos;
               }
               break;
         }

      }

      if(iIndex>iDataSize) // Stop if image size exceeded.
         iDcode=0;
   }
 
   return IMG_OK;
}


int BMPImg::LoadBmpPalette()

{
   int iIndex;
   unsigned char *pPalPos, *pDatPos;
 
   if(pPalette)
   {
      delete [] pPalette;
      pPalette=NULL;
   }

   if(iBPP>8) // NULL Palette for RGB images
      return IMG_OK;

   // Create space for palette
   pPalette=new unsigned char[768];

   if(pPalette==NULL)
      return IMG_ERR_MEM_FAIL;

   // Set starting position for pointers
   pPalPos=pPalette;
   pDatPos=&pData[0x36];

   // Get colour values, skip redundant 4th value
   for(iIndex=0;iIndex!=256;++iIndex)
   {
      pPalPos[0]=pDatPos[2]; // Red
      pPalPos[1]=pDatPos[1]; // Green
      pPalPos[2]=pDatPos[0]; // Blue

      pPalPos+=3;
      pDatPos+=4;
   }

   return IMG_OK;
}


void BMPImg::FlipImg(void)

{
   unsigned char bTemp;
   unsigned char *pLine1, *pLine2;
   int iLineLen,iIndex;

   iLineLen=iWidth*(iBPP/8);
   pLine1=pImage;
   pLine2=&pImage[iLineLen * (iHeight - 1)];

   for( ;pLine1<pLine2;pLine2-=(iLineLen*2))
   {
      for(iIndex=0;iIndex!=iLineLen;pLine1++,pLine2++,iIndex++)
      {
         bTemp=*pLine1;
         *pLine1=*pLine2;
         *pLine2=bTemp;       
      }
   } 

}


int BMPImg::GetBPP()

{
   return iBPP;
}


int BMPImg::GetWidth()

{
   return iWidth;
}


int BMPImg::GetHeight()

{
   return iHeight;
}


unsigned char* BMPImg::GetImg()

{
   return pImage;
}


unsigned char* BMPImg::GetPalette()

{
   return pPalette;
}
