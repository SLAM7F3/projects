/*=========================================================================

  Program:   PS_OrthoRect

  Copyright © 2007-2010
  Massachusetts Institute of Technology
  All rights reserved.

  =========================================================================*/

#ifndef WISP_DATA_H
#define WISP_DATA_H

#include "CBox.h"
#include "llimage/LLImage.hpp"
#include <string>

#include "CondorProcessing.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

namespace bfs = boost::filesystem;

class Wisp360PanDataSetInfo : public DataSetInfo {
  public:
  Wisp360PanDataSetInfo(void) : DataSetInfo() {};
   typedef boost::shared_ptr< Wisp360PanDataSetInfo > ptr_t;

   double m_panFovX;
   double m_panFovY;
   double m_panCenterX;
   double m_panCenterY;
   int m_liveDataSetOpen;

   virtual void Serialize(std::ostream &os) const
   {
      DataSetInfo::Serialize(os);
      SerializeHelper ser(os);

      ser << m_panFovX << m_panFovY << m_panCenterX << m_panCenterY << m_liveDataSetOpen;
   }

   virtual void Deserialize(std::istream &is)
   {
      DeserializeHelper des(is);
      DataSetInfo::Deserialize(is);

      des >> m_panFovX >> m_panFovY >> m_panCenterX >> m_panCenterY >> m_liveDataSetOpen;
   }
};


class Wisp360PanDataSetUpdateInfo : public DataSetUpdateInfo {
  public:
  Wisp360PanDataSetUpdateInfo(void) : DataSetUpdateInfo() {};
   typedef boost::shared_ptr< Wisp360PanDataSetUpdateInfo > ptr_t;

   double m_temp;

   virtual void Serialize(std::ostream &os) const
   {
      DataSetUpdateInfo::Serialize(os);
      SerializeHelper ser(os);

      ser << m_temp;
   }

   virtual void Deserialize(std::istream &is)
   {
      DeserializeHelper des(is);
      DataSetUpdateInfo::Deserialize(is);

      des >> m_temp;
   }
};


class Wisp360PanFrameInfo : public FrameInfo {
  public:
  Wisp360PanFrameInfo(void) : FrameInfo() {};
   typedef boost::shared_ptr< Wisp360PanFrameInfo > ptr_t;

   double m_temp;

   virtual void Serialize(std::ostream &os) const
   {
      FrameInfo::Serialize(os);
      SerializeHelper ser(os);

      ser << m_temp;
   }

   virtual void Deserialize(std::istream &is)
   {
      DeserializeHelper des(is);
      FrameInfo::Deserialize(is);

      des >> m_temp;
   }
};


#if 1
class wisp360PanCompressedTile : public RawTile
{
  public:
   wisp360PanCompressedTile(void) {}
  wisp360PanCompressedTile(const TileSpecifier &ts): RawTile(ts) {}

   typedef boost::shared_ptr< wisp360PanCompressedTile > ptr_t;

   static ptr_t CastFromRawTilePointer(const RawTile::ptr_t &t)
   {
      return(boost::static_pointer_cast< wisp360PanCompressedTile, RawTile >(t));
   }

   // data members
   std::vector <unsigned char> compBuffer;

   void Serialize(ostream &os) const
   {
      RawTile::Serialize(os);

      SerializeHelper s(os);

      s.SerializeAs<uint32_t>((uint32_t)compBuffer.size());
      os.write((char*) &compBuffer[0], (std::streamsize) compBuffer.size());
   }

   virtual void Deserialize(std::istream &is)
   {
      DeserializeHelper des(is);

      RawTile::Deserialize(is);  // call back to superclass method

      compBuffer.resize(des.Deserialize<uint32_t>());
      is.read((char*) &compBuffer[0], (std::streamsize) compBuffer.size());
   }
};
#endif

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

      uint64_t filePosLong;

      char extra[224];
};


struct Wisp360PanoramicFileStruct
{
      char wisp360PanFile[256];
};


class Wisp360PanAPI : public CondorProcessing {
  public:
   Wisp360PanAPI(void) {
      // Base class members
      m_numChannels = 1;
      m_sensorType = Condor::WISP360PAN;
      m_src = NULL;
      m_dataFilename[0] = 0;

      m_wisp360PanDataSetInfo.reset(new Wisp360PanDataSetInfo);
      m_dataSetInfo = boost::static_pointer_cast<DataSetInfo, Wisp360PanDataSetInfo>(m_wisp360PanDataSetInfo); // pointer for CondorProcessing

      m_wisp360PanDataSetUpdateInfo.reset(new Wisp360PanDataSetUpdateInfo);
      m_dataSetUpdateInfo = boost::static_pointer_cast<DataSetUpdateInfo, Wisp360PanDataSetUpdateInfo>(m_wisp360PanDataSetUpdateInfo); // pointer for CondorProcessing

      m_wisp360PanFrameInfo.reset(new Wisp360PanFrameInfo);
      m_frameInfo = boost::static_pointer_cast<FrameInfo, Wisp360PanFrameInfo>(m_wisp360PanFrameInfo); // pointer for CondorProcessing
   };

   ~Wisp360PanAPI() {
      if(m_liveDatasetOpen){
         m_liveDatasetOpen = 0;
         m_w360DatasetThread.join();
      }
      if(m_src)
         delete m_src;
   };

   static Condor * CreateAndInitializeDataSet(const char *input_string) {
      Wisp360PanAPI *wisp360 = new Wisp360PanAPI();
      if(wisp360->OpenDataSet(input_string) == -1){
         delete wisp360;
         return NULL;
      }
      return (Condor *)wisp360;
   }

   static Condor * CreateAndInitializeDataSetLite(const char *input_string) {
      Wisp360PanAPI *wisp360 = new Wisp360PanAPI();
      if(wisp360->OpenDataSet(input_string) == -1){
         delete wisp360;
         return NULL;
      }
      return (Condor *)wisp360;
   }

   int OpenDataSet(const char *fname);

   virtual int GetBitDepth(void) { return 16; }

   int GetMaxPixelVal(void){return ((1<<14) - 1); }

   int GetPanoramic(unsigned int frameNum, double yaw, double pitch, double fovX, double fovY, LLImage_u16 *dst);

   int GetNumFrames() { return m_numFrames; };

   int GetFrameNum( double utc );
   double GetDoubleTimeAtFrame( int frameNumber );

   virtual int RenderImage16( CondorRenderRequest::ptr_t req )
   { WARN_UNIMPLEMENTED_FUNCTION; return 0; }

   int GetUTMExtent( unsigned int frameNumber, double bBox[4], char *utmZone ) {return 0;};
   int GetUTMExtent( const Pose &pose, double bBox[4], char *utmZone ) {return 0;};
   int GetUTMExtent( double bBox[4], char *utmZone ) {return 0;};

   int UpdateWisp360DatasetThreadProc(void);

   void netCallback(uint32_t msgid, uint32_t msglen, const char* body);

   RawTile::ptr_t ReadSingleTile(TileSpecifier::ptr_t tsr);
   wisp360PanCompressedTile::ptr_t ReadCompressedTile(const TileSpecifier &ts);
   int ReadCompressedTiles(TileRequestList &ls);

   void SetFrameInfo(int frameNumber);

  private:

   virtual RawTile::ptr_t CreateNewRawTile(void) const
   {
      return (RawTile::ptr_t) new wisp360PanCompressedTile();
   }

   LLImage_u16 *m_src;

   Wisp360PanFileHeader m_fileHeader;
   std::vector<double> m_frameTime;

   FILE *m_fid;
   FILE *m_fidData;

   bfs::path m_rootPath;
   char m_indexFilename[256];
   char m_dataFilename[256];

   int m_headerSize;

   int m_panSizeX;
   int m_panSizeY;
   double m_panFovX;
   double m_panFovY;
   double m_panCenterX;
   double m_panCenterY;
   double m_panUpLeftX;
   double m_panUpLeftY;
   double m_srcPixPerDegX;
   double m_srcPixPerDegY;

   int m_compressSize;
   int m_numFrames;

   int m_liveDatasetOpen;
   boost::thread m_w360DatasetThread;

   std::vector<Wisp360PanoramicFileStruct> m_w360PanFiles;

   Wisp360PanDataSetInfo::ptr_t m_wisp360PanDataSetInfo;
   Wisp360PanDataSetUpdateInfo::ptr_t m_wisp360PanDataSetUpdateInfo;
   Wisp360PanFrameInfo::ptr_t m_wisp360PanFrameInfo;

   boost::mutex m_wispPanMutex;
   boost::mutex m_wispPanDataMutex;

};


#endif
