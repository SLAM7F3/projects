// ========================================================================
// Anye Li's FFMPEG Reader/Writer OSG plugin
// ========================================================================
// Last updated on 1/8/08
// ========================================================================

#include <iostream>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "ffmpeg/FFmpegStream.h"

using namespace osg;
using namespace osgDB;
using namespace osgFFmpeg;

namespace osgDB
{
   class ReaderWriterFFmpeg : public osgDB::ReaderWriter
      {
         virtual const char *className() const {
            return "ReaderWriterFFmpeg";
         }

         virtual bool acceptsExtension(
            const std::string& extension) const{
            return true;
         }
        
         virtual ReadResult readObject(
            const std::string& fileName,
            const Options *opt) const {
            return readImage(fileName, opt);
         }

         virtual ReadResult readImage(
            const std::string& file,
            const Options *opt) const {
            
            std::string fileName = osgDB::findDataFile(file, opt);
            if(fileName.empty())
            {
               return ReadResult::FILE_NOT_FOUND;
            }
     
            // code for setting up the database path so that
            // internally referenced file are searched for on
            // relative paths. 
            osg::ref_ptr<Options> local_opt
               = opt ? static_cast<Options*>(
                  opt->clone(osg::CopyOp::SHALLOW_COPY))
               : new Options;
            local_opt->setDatabasePath(
               osgDB::getFilePath(fileName));

            osg::ref_ptr<FFmpegStream> stream(new FFmpegStream);

            int err = stream->open(
               fileName.c_str(), 0, 0, 0);
            if(err < 0)
            {
               return ReadResult::ERROR_IN_READING_FILE;
            }
            return stream.get();
         }
      };
}

RegisterReaderWriterProxy<osgDB::ReaderWriterFFmpeg> g_FFmpegReaderWriterProxy;
