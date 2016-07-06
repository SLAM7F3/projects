// ==========================================================================
// Program PARSE_RAW_HEADER

// 				parse_raw_header

// ==========================================================================
// Last updated on 2/27/13
// ==========================================================================

#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;

// ==========================================================================
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

   string data_subdir="/data_third_disk/DIME/Wisp_data/";
   string data_filename=data_subdir+"wisp4-raw-5min-20121116.img";
    
   // Open actual data file
   FILE* m_fid = fopen(data_filename.c_str(), "rb");
   if (!m_fid)
   {
      cout << "Couldn't open raw wisp image file" << endl;
   }

// Read the file header

   unsigned short fileheader[2048];
   fread(fileheader, 2048, 2, m_fid);

   int m_fileHeaderSize = 2048*2;

// Read the length and width from the file header

   int m_len = fileheader[54];
   int m_wid = fileheader[56];
   cout << "m_len = " << m_len << " m_wid = " << m_wid << endl;

   unsigned short hdr[256*256];

   // Read swath 0's header information

   int swathNum;
   cout << "Enter swathNum:" << endl;
   cin >> swathNum;
   
   int64_t fpos = (int64_t) m_fileHeaderSize + 
      (int64_t)swathNum*m_len*m_wid*sizeof(unsigned short) + 
      (int64_t)(m_len*(m_wid-256))*sizeof(unsigned short);

   cout << "fpos = " << fpos << endl;

   fseek(m_fid, fpos,SEEK_SET);
   int numread = fread(hdr, 2, 256*256, m_fid);
   cout << "numread = " << numread << endl;

   int swathCount    = hdr[m_len*192 + 2];
   int frameCount    = hdr[m_len*192 + 3];
   cout << "swathCount = " << swathCount << endl;
   cout << "frameCount = " << frameCount << endl;

   double utc=-1;
   memcpy(&utc, &(hdr[m_len*64]), sizeof(double));
   cout << "utc = " << utc << endl;

// Get the start of swath time from the first polygon packet to set
// the dataset base time

   int64_t m_baseDatasetTime = 
       (((int64_t)hdr[m_len*192 + 8])<<48) | 
       (((int64_t)hdr[m_len*192 + 9])<<32) | 
       (((int64_t)hdr[m_len*192 + 10])<<16) |   
      ((int64_t)hdr[m_len*192 + 11]);

   cout << "m_baseDatasetTime = " << m_baseDatasetTime << endl;

   double freeCount = 
      ((double)((((((int64_t)hdr[m_len*192 + 8])<<48) | 
      (((int64_t)hdr[m_len*192 + 9])<<32) | 
      (((int64_t)hdr[m_len*192 + 10])<<16) |   
      ((int64_t)hdr[m_len*192 + 11])))-m_baseDatasetTime)) / 100000000.0;
   
   cout << "freeCount = " << freeCount << endl;


} 

