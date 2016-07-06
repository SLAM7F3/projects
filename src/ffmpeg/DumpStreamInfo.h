// ========================================================================
// ========================================================================
// Last updated on 1/7/08
// ========================================================================

#ifndef INCLUDE_FFUTIL_DUMPSTREAMINFO
#define INCLUDE_FFUTIL_DUMPSTREAMINFO

#include <sstream>
#include <string>
// #include <ffmpeg/avformat.h>

struct AVFormatContext;

namespace FFUtil
{
   inline std::string dump_stream_info(const AVFormatContext* ic)
      {
         std::ostringstream s;
         if(ic->track)
            s<<"Track: "<<ic->track<<std::endl;
         if(ic->title[0])
            s<<"Title: "<<ic->title<<std::endl;
         if(ic->author[0])
            s<<"Author: "<<ic->author<<std::endl;
         if(ic->copyright[0])
            s<<"Copyright: "<<ic->copyright<<std::endl;
         if(ic->comment[0])
            s<<"Comment: "<<ic->comment<<std::endl;
         if(ic->album[0])
            s<<"Album: "<<ic->album<<std::endl;
         if(ic->year)
            s<<"Year: "<<ic->year<<std::endl;
         if(ic->genre[0])
            s<<"Genre: "<<ic->genre<<std::endl;
         return s.str();
      }
}

#endif
