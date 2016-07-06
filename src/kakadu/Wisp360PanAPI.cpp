/*=========================================================================

  Program:   PS_OrthoRect

  Copyright © 2007-2010
  Massachusetts Institute of Technology
  All rights reserved.

  =========================================================================*/

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>

#include "CMatrix.h"
#include <iostream>
#include <fstream>

#include <math.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "Wisp360PanAPI.h"

#include "constants.h"
#include "LatLongUTMconversion.h"
#include "FileCache.hpp"
#include "int_types.h"
#include "llimage/LLImage.hpp"

#include <errno.h>
#include "fatal_error.h"
#include "LeastSquares.hpp"
#include "GroundPlane.hpp"
#include "PutLog.h"

#include "ipp.h"

#include "stopwatch.h"

#include "llimage/GlobalCuda.hpp"
#include "KakaduSupport.h"

extern CondorUserMessenging *usrMsg;

//#include <cairomm/cairomm.h>

#include "CondorDriver.h"
CONDOR_REGISTER_DRIVER_NOLIVE_WITH_LITE(Wisp360Pan, ".w360", "wisp360Pan", Wisp360PanAPI);

static inline int fseek64(FILE *fid, int64_t pos)
{
#ifdef _MSC_VER
   return(fsetpos(fid, &pos));
#else
   return(fseek(fid, pos, SEEK_SET));
#endif
}

#define DEBUG 0
#define DEBUG_TIMER 1


class W360PanReader
{
  public:

   int live;
   int tivo;
   string panPath;

   int sensorType;

   int Read(const char *path)
   {
      static const char *w360PanTags[] = {"LIVE", "TIVO", "PANPATH"};

      static string W360PanReader::*stringPtrs[4] = {NULL, NULL, &W360PanReader::panPath};

      bfs::path w360PanPath(path);
      w360PanPath.remove_filename();

      ifstream ifs(path);
      if(ifs.fail()) return 0;
      string s;
      std::getline(ifs, s);

      boost::algorithm::trim(s);

      if(s=="W360PAN1.0") {
         sensorType = Condor::WISP360PAN;
      } else {
         return 0;
      }

      while(std::getline(ifs, s)) {
         size_t colon = s.find_first_of(':');
         string tag = s.substr(0, colon);
         string filename = s.substr(colon+1);

         boost::algorithm::trim(filename);

         for(int i=0; i<3; i++)
            if(tag == w360PanTags[i]) {

               if(i==0)
                  sscanf(filename.c_str(), "%d", &live);
               else if(i==1)
                  sscanf(filename.c_str(), "%d", &tivo);

               else{

                  bfs::path p(filename);
                  if(!exists(p) && !p.empty()) {
                     p = w360PanPath / *--p.end();
                     if(!exists(p)) {
                        printf("Warning could not open %s Please check file path\n", s.c_str());
                        return 0;
                     }
                  }
                  this->*stringPtrs[i] = p.file_string();
               }
            }
      }

      return 1;
   }
};


// Thread to update the number of frames during live mode
int Wisp360PanAPI::UpdateWisp360DatasetThreadProc(void)
{
   std::vector<Wisp360PanFrameHeader> frameHeader;
   int ii;
   int numFrames;

   while(m_liveDatasetOpen){

      // Determine how many frames there are based on the size of the file
      if(m_netMode)
         numFrames = m_wisp360PanDataSetUpdateInfo->m_numFrames;
      else{
         bfs::path dataFilePath(m_indexFilename);
         int64_t fsize = file_size(dataFilePath);
         numFrames = (int)((fsize-sizeof(Wisp360PanFileHeader)) / m_fileHeader.frameHeaderSize);
      }

      // If there is a new frame, read the header and call the call back function
      if(m_numFrames < numFrames){

         // Read in the index file for all new frames
         if(m_netMode){

            // Send requests for each frame's info
            m_frameTime.resize(numFrames);
            for(ii=m_numFrames; ii<numFrames; ii++){
               m_frameInfoReady.Reset();
               m_rawNetClient->SendRequest(FRAME_INFO_REQUEST, sizeof(int), (char *)(&ii));
               m_frameInfoReady.Wait();
               m_frameTime[ii] = m_wisp360PanFrameInfo->m_frameTime;
            }

         }else{

            frameHeader.resize(numFrames-m_numFrames);
            if(1){
               boost::mutex::scoped_lock lock(m_wispPanMutex);
               int64_t fpos = (int64_t) m_numFrames*m_fileHeader.frameHeaderSize+sizeof(Wisp360PanFileHeader);
               fseek64(m_fid, fpos);
               fread(&(frameHeader[0]), sizeof(Wisp360PanFrameHeader), numFrames-m_numFrames, m_fid);
            }

            // Add to the time vector from the time information in the frame headers
            m_frameTime.resize(numFrames);
            for(ii=m_numFrames; ii<numFrames; ii++)
               m_frameTime[ii] = frameHeader[ii-m_numFrames].time;
         }

         // Update the number of frames
         m_numFrames = numFrames;
         m_wisp360PanDataSetUpdateInfo->m_numFrames = m_numFrames;

         // call back to user program
         if(m_frameAvailCB) {
            m_frameAvailCB();
         }
      }

      // Sleep for 100 ms
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
   }

   return 0;
}


int Wisp360PanAPI::OpenDataSet(const char *w360PanFileName)
{
   std::vector<Wisp360PanFrameHeader> frameHeader;
   int ii;

   // Try to read the supplied w360 file.
   m_liveDatasetOpen = 0;

   usrMsg->ProgressInit("Opening Data Set", w360PanFileName);

   // First check if it is a url
   std::string input = w360PanFileName;
   CondorFactoryDescriptor d;
   d.ParseInputString(input);

   // Determine whether this is a local file or a file to grab from the server
   if(d.m_protocol == "wisp360Pan") {

      m_netMode = true;
      input = d.m_file;
      printf("connecting on socket...\n");
      m_rawNetClient.reset(new RawNetworkClient());
      m_rawNetClient->Connect(d.m_host.c_str(), 5774);
      m_rawNetClient->SetCallback(boost::bind(&Wisp360PanAPI::netCallback, this, _1, _2, _3));

      m_dataSetOpenReady.Reset();
      m_rawNetClient->SendRequest(OPEN_DATA_SET, (int) input.size(), input.c_str());
      m_dataSetOpenReady.Wait();
        
      if(m_netError) {
         FATAL_ERROR("Couldn't open network data set from %s [%s]: %s", 
         m_rawNetClient->GetRemoteHostname().c_str(),
         m_rawNetClient->GetRemoteAddress().c_str(),
         m_netErrorString.c_str());
      }

      m_panSizeX = m_wisp360PanDataSetInfo->m_imgWidth;
      m_panSizeY = m_wisp360PanDataSetInfo->m_imgHeight;
      m_numFrames = m_wisp360PanDataSetInfo->m_numFrames;

      m_panFovX = m_wisp360PanDataSetInfo->m_panFovX;
      m_panFovY = m_wisp360PanDataSetInfo->m_panFovY;
      m_panCenterX = m_wisp360PanDataSetInfo->m_panCenterX;
      m_panCenterY = m_wisp360PanDataSetInfo->m_panCenterY;
      m_liveDatasetOpen = m_wisp360PanDataSetInfo->m_liveDataSetOpen;

      m_panUpLeftX = m_panCenterX - m_panFovX/2;
      m_panUpLeftY = m_panCenterY + m_panFovY/2;
      m_srcPixPerDegX = m_panSizeX / m_panFovX;
      m_srcPixPerDegY = m_panSizeY / m_panFovY;

      // Send requests for each frame's info
      m_frameTime.resize(m_numFrames);
      for(ii=0; ii<m_numFrames; ii++){
         m_frameInfoReady.Reset();
         m_rawNetClient->SendRequest(FRAME_INFO_REQUEST, sizeof(int), (char *)(&ii));
         m_frameInfoReady.Wait();
         m_frameTime[ii] = m_wisp360PanFrameInfo->m_frameTime;
      }

   }else{

      m_netMode = false;

      // Try to read the supplied w360 file.
      W360PanReader w360;
      if(! w360.Read(w360PanFileName) ) {
         //m_internalErrorCode = ErrNoW360;
         return -1;
      }

      // Open the input file, read the size of the compressed frames, determine the size of the file, and then calculate the number of frames
      m_fid = fopen(w360.panPath.c_str(), "rb");
      if(!m_fid){

         // Throw an exception
      }

      // Read the file header
      fread(&m_fileHeader, sizeof(Wisp360PanFileHeader), 1, m_fid);

      // Make sure this is a proper wisp360 file
      if(m_fileHeader.fileID != 0x31593601){

         // Throw an exception
      }

      strcpy(m_indexFilename, w360.panPath.c_str());

      // Copy some variables
      m_panSizeX = m_fileHeader.width;
      m_panSizeY = m_fileHeader.length;
      m_panFovX = m_fileHeader.fovX;
      m_panFovY = m_fileHeader.fovY;
      m_panCenterX = m_fileHeader.yaw;
      m_panCenterY = m_fileHeader.pitch;

      m_panUpLeftX = m_panCenterX - m_panFovX/2;
      m_panUpLeftY = m_panCenterY + m_panFovY/2;
      m_srcPixPerDegX = m_panSizeX / m_panFovX;
      m_srcPixPerDegY = m_panSizeY / m_panFovY;

      // Determine how many frames there are based on the size of the file
      bfs::path dataFilePath(w360.panPath.c_str());
      int64_t fsize = file_size(dataFilePath);
      m_numFrames = (int)((fsize-sizeof(Wisp360PanFileHeader)) / m_fileHeader.frameHeaderSize);

      m_rootPath = dataFilePath.parent_path();

      // Read in the entire index file
      frameHeader.resize(m_numFrames);
      fread(&(frameHeader[0]), m_fileHeader.frameHeaderSize, m_numFrames, m_fid);

      // Build a time vector from the time information in the frame headers
      m_frameTime.resize(m_numFrames);
      for(ii=0; ii<m_numFrames; ii++)
         m_frameTime[ii] = frameHeader[ii].time;

      // Open the first data file
      m_fidData = NULL;
#if 0
      if(m_numFrames > 0){

         // Read the frame header for frame 0
         fread(&frameHeader, sizeof(Wisp360PanFrameHeader), 1, m_fid);

         // Construct the filename from the index file location and the file name of the data file
         sprintf(m_dataFilename, "%s/%s", m_rootPath.file_string().c_str(), frameHeader.dataFile);
         m_fidData = fopen(m_dataFilename, "rb");
      }
#endif

      if(w360.tivo)
         m_liveDatasetOpen = 1;

      m_wisp360PanDataSetInfo->m_band = 0;
      m_wisp360PanDataSetInfo->m_imgWidth = m_panSizeX;
      m_wisp360PanDataSetInfo->m_imgHeight = m_panSizeY;
      m_wisp360PanDataSetInfo->m_numFrames = m_numFrames;
      m_wisp360PanDataSetInfo->m_j2kTileSize = 512;

      m_wisp360PanDataSetInfo->m_panFovX = m_panFovX;
      m_wisp360PanDataSetInfo->m_panFovY = m_panFovY;
      m_wisp360PanDataSetInfo->m_panCenterX = m_panCenterX;
      m_wisp360PanDataSetInfo->m_panCenterY = m_panCenterY;
      m_wisp360PanDataSetInfo->m_liveDataSetOpen = m_liveDatasetOpen;
   }

   // This doesn't need to be here in a "lite" mode
   //m_src = new LLImage_u16(m_panSizeX, m_panSizeY, 1);

   if(m_liveDatasetOpen){
      m_w360DatasetThread = boost::thread(boost::bind(&Wisp360PanAPI::UpdateWisp360DatasetThreadProc, this));
   }

   return 1;
}


void Wisp360PanAPI::netCallback(uint32_t msgid, uint32_t msglen, const char* body)
{
   membuf mb(body, msglen);
   std::istream is(&mb);

   if(0){ //msgid == DATA_SET_INFO) {

   } else {
      CondorProcessing::netCallback(msgid, msglen, body);
   }
}


wisp360PanCompressedTile::ptr_t Wisp360PanAPI::ReadCompressedTile(
   const TileSpecifier &ts)
{
   Wisp360PanFrameHeader frameHeader;

   char m_dataFilenameLocal[256];

   int frameNum;
   int srcRes;

   kdu_dims ROI;

   frameNum = ts.m_frameNumber;
   srcRes = ts.m_resLevel;
   ROI.pos.x = ts.m_requestedROI.getLeft();
   ROI.pos.y = ts.m_requestedROI.getTop();
   ROI.size.x = ts.m_requestedROI.getSizeX();
   ROI.size.y = ts.m_requestedROI.getSizeY();

   wisp360PanCompressedTile::ptr_t t(new wisp360PanCompressedTile(ts));

   // Read the frame header
   if(1){
      boost::mutex::scoped_lock lock(m_wispPanMutex);
      int64_t fpos = (int64_t) frameNum*m_fileHeader.frameHeaderSize+sizeof(Wisp360PanFileHeader);
      fseek64(m_fid, fpos);
      fread(&frameHeader, m_fileHeader.frameHeaderSize, 1, m_fid);
   }

// A mutex is around this portion of the function to prevent a
// different file from being opened
// while the compressed data is still being read.  Also, it's to
// prevent compressed data from
// being read at the same time from two different locations in the
// file

   if(1){
      boost::mutex::scoped_lock lock(m_wispPanDataMutex);

      // Construct the filename from the index file location and the file name of the data file
      sprintf(m_dataFilenameLocal, "%s/%s", m_rootPath.file_string().c_str(), frameHeader.dataFile);

      // Open the file if it's different from the previous
      if(strcmp(m_dataFilename, m_dataFilenameLocal)){

         if(m_fidData)
            fclose(m_fidData);

         strcpy(m_dataFilename, m_dataFilenameLocal);
         m_fidData = fopen(m_dataFilename, "rb");
      }

      // Backwards compatibility for previous datasets that only had a 32 bit file position pointer.  Eventually this will go away
      if(frameHeader.filePosLong)
         KDUGetRegionCondorServer( m_fidData, frameHeader.filePosLong, ROI, t->compBuffer, srcRes );
      else
         KDUGetRegionCondorServer( m_fidData, frameHeader.filePos, ROI, t->compBuffer, srcRes );
   }

   return t;
}


RawTile::ptr_t Wisp360PanAPI::ReadSingleTile(TileSpecifier::ptr_t tsr)
{
   wisp360PanCompressedTile::ptr_t t = ReadCompressedTile(*tsr);

   return t;
}


int Wisp360PanAPI::ReadCompressedTiles(TileRequestList &ls)
{
   for(TileRequestList::iterator it = ls.begin(); it!=ls.end(); ++it) {
      TileSpecifier::ptr_t ts = (*it);

      wisp360PanCompressedTile::ptr_t t = ReadCompressedTile(*ts);

      TileReadComplete(t);
   }

   return 1;
}


void Wisp360PanAPI::SetFrameInfo(int frameNumber)
{
   m_wisp360PanFrameInfo->m_frameTime = m_frameTime[frameNumber];
}


double Wisp360PanAPI::GetDoubleTimeAtFrame( int frameNumber )
{
   if(frameNumber >= 0 && frameNumber < m_numFrames)
      return m_frameTime[frameNumber];
   else
      return -1;
}


int Wisp360PanAPI::GetFrameNum( double utc )
{
   std::vector<double>::iterator low;
   low = std::lower_bound(m_frameTime.begin(), m_frameTime.end(), utc);
    
   return ((int)(low-m_frameTime.begin()));
}

int Wisp360PanAPI::GetPanoramic(unsigned int frameNum, double yaw, double pitch, double fovX, double fovY, LLImage_u16 *dst)
{
   double dstPixPerDegX;
   double dstPixPerDegY;
   double scaleFactorX;
   double scaleFactorY;
   double shiftX;
   double shiftY;

   int srcRes;

   kdu_dims ROI;

   int sizeX;
   int sizeY;
   double res;
   RECT utm;
   char zone[4];
   __int64 time64;
   int sz;

   // Read and decompress the necessary region into the src buffer
   // Start by determining the ROI
   ROI.pos.x = (int)max(0.0, floor((yaw - fovX/2 - m_panUpLeftX) * m_srcPixPerDegX));
   ROI.pos.y = (int)max(0.0, floor((-pitch - fovY/2 + m_panUpLeftY) * m_srcPixPerDegY));
   ROI.size.x = (int)min((double) m_panSizeX-ROI.pos.x, ceil(fovX * m_panSizeX / m_panFovX));
   ROI.size.y = (int)min((double) m_panSizeY-ROI.pos.y, ceil(fovY * m_panSizeY / m_panFovY));

   // Now determine the appropriate number of resolution levels from the source imagery (0 means all, 1 means leave out the highest res, etc)
   dstPixPerDegX = dst->m_sizeX / fovX;
   dstPixPerDegY = dst->m_sizeY / fovY;
   scaleFactorX = m_srcPixPerDegX / dstPixPerDegX;
   scaleFactorY = m_srcPixPerDegY / dstPixPerDegY;

   // Shift values represent the upper left corner of the source image that is part of the dst roi
   shiftX = -(yaw - fovX/2 - m_panUpLeftX) * m_srcPixPerDegX + ROI.pos.x;
   shiftY = -(-pitch - fovY/2 + m_panUpLeftY) * m_srcPixPerDegY + ROI.pos.y;

   shiftX /= scaleFactorX;
   shiftY /= scaleFactorY;

   srcRes = 0;
   while(scaleFactorX > 1.6 && scaleFactorY > 1.6){
      scaleFactorX /= 2;
      scaleFactorY /= 2;
      //shiftX /= 2;
      //shiftY /= 2;
      srcRes++;
   }

   // Create a tile list to send to the reader
   TileRequestList tileList;
   TileSpecifier::ptr_t tspec(new TileSpecifier);
   tspec->m_frameNumber = frameNum;
   tspec->m_imagerId = 0;
   tspec->m_tileId = 0;
   tspec->m_resLevel = srcRes;
   tspec->m_requestedROI.assignLRTB(ROI.pos.x, ROI.pos.x+ROI.size.x-1, ROI.pos.y, ROI.pos.y+ROI.size.y-1);
   tileList.push_back(tspec);

   if(m_netMode)
      ReadTilesNet(tileList);
   else
      ReadCompressedTiles(tileList);

   wisp360PanCompressedTile::ptr_t t = wisp360PanCompressedTile::CastFromRawTilePointer(tileQueue.pop_blocking());

   // This doesn't need to be here in a "lite" mode
   LLImage_u16 *src = new LLImage_u16(m_panSizeX, m_panSizeY, 1);

   KDUReadJP2Mono16Buf( &(t->compBuffer[0]), (short *)(src->GetImagePointer(HOST, WRITE_ONLY)), t->compBuffer.size(), ROI,
   sizeX, sizeY, res, utm, zone, time64, 0, 16, src->m_stride, srcRes );

   //LLImage_u8 src8(m_src->m_sizeX, m_src->m_sizeY, 1);
   //LLImageHistogram hst(4096*16, 4096*16);

   //m_src->CalcHistogram(hst);
   //HistogramLimitType limits = hst.FindLimits(0.02F);
   //m_src->ConvertToLLImage_U8(limits.first, limits.second, &src8);

   //src8.WritePGMFile("decomp.pgm");

   ippiResizeSqrPixelGetBufSize(dst->GetIppiSize(), dst->m_numComponents, IPPI_INTER_LINEAR, &sz);
   std::vector < unsigned char > ippTempBuffer(sz); //use a vector for auto cleanup

   dst->Clear();

   int minX=0;
   int maxX=dst->m_sizeX-1;
   int minY=0;
   int maxY=dst->m_sizeY-1;

   minX = max(0, (int) ceil((m_panUpLeftX-yaw+fovX/2) * dstPixPerDegX - 0.1));
   minY = max(0, (int) ceil(-(m_panUpLeftY-pitch-fovY/2) * dstPixPerDegY - 0.1));
   maxX = min(dst->m_sizeX-1, (int) floor((m_panUpLeftX+m_panFovX-yaw+fovX/2) * dstPixPerDegX + 0.1));
   maxY = min(dst->m_sizeY-1, (int) floor(-(m_panUpLeftY-m_panFovY-pitch-fovY/2) * dstPixPerDegY + 0.1));

   IppiRect r = {minX, minY, (maxX-minX+1), (maxY-minY+1)};

   //r.x = {0, 0, sizeX, sizeY};
   ippiResizeSqrPixel_16u_C1R( src->GetImagePointer(HOST, READ_ONLY), src->GetIppiSize(), src->m_stride*2, src->GetIppiFullRect(),
   dst->GetImagePointer(HOST, WRITE_ONLY), dst->m_stride*2, 
   r, 1/scaleFactorX, 1/scaleFactorY,
   shiftX, shiftY, IPPI_INTER_LINEAR, &ippTempBuffer[0]);

   delete src;

   //LLImage_u8 dst8(dst->m_sizeX, dst->m_sizeY, 1);
   //LLImageHistogram hst2(4096*16, 4096*16);

   //dst->CalcHistogram(hst2);
   //limits = hst2.FindLimits(0.02F);
   //dst->ConvertToLLImage_U8(limits.first, limits.second, &dst8);

   //dst8.WritePGMFile("resize.pgm");

   return(1);
}

