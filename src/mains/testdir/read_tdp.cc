// =======================================================================
// Test program to read TDP files
// =======================================================================
// Last updated on 4/26/06
// =======================================================================

// examples/read.cpp

// This example demonstrates how to:
//  1. Open a TDP File
//  2. Read Data all at once
//  3. Read Data in chunks

// To compile this file, certain defines must be used to assist
// in handling types such as uint64_t .  These types are defined
// in src/types/basic_type.h .
//
//  for linux, use:         -DHAS_STDINT
//  for windows, use:       /DWIN32
//  for other unix, try:    -DHAS_STDINT, -DHAS_INTTYPES, or -DUSE_TYPEDEFS
//
// see src/types/basic_type.h for more information

#include <iostream>
#include <string>
#include <libtdp/tdp.h>
#include "threeDgraphics/tdpfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

int main()
{
   /***
    * Open a TDP File (read-only)
    **/

   string tdp_filedir=
      "/home/cho/programs/c++/svn/projects/src/mains/testdir/";
   string tdp_filename=tdp_filedir+"local.tdp";
   cout << "tdp_filename = " << tdp_filename << endl;

   Tdp_file tdp_file;
   tdp_file.file_open(tdp_filename,"r+b");
   if ( tdp_file.klv_exists( TdpKeyXYZ_POINT_DATA, 0 ) )
   {
      cout << "XYZ_point_data key exists in tdp file:" << endl;

      uint64_t length;
      length = tdp_file.klv_length( TdpKeyXYZ_POINT_DATA, 0 );
      cout << "length = " << length << endl;
   }

   exit(-1);


   Tdp_base* handler = Tdp_base::stream_open("file:"+tdp_filename);
    


   if ( handler == NULL ) return 1;


   

   double pnull_threshold=0;
   vector<fourvector>* xyzp_ptr=tdpfunc::read_xyzp_data(
      handler,pnull_threshold);

// Read point data from file 4 floats at a time:

   int klv_index=0;
   int counter=0;

/*

   const int bytes_per_real32=sizeof(real32_t);
   const int nfloats_per_field=3;
   real32_t point_data[bytes_per_real32]; // real32 == float32
   uint64_t offset = 0;    // file position offset




   bool xyz_data_remaining=true;
   while (xyz_data_remaining)
   {
      xyz_data_remaining=handler->klv_read(
         TdpKeyXYZ_POINT_DATA,	// klv key
         klv_index,         // klv index, for multiple occurrences
         point_data,             // destination for data
         nfloats_per_field*bytes_per_real32,        // length of read in bytes
         tdp_data,               // this is a data klv
         offset                  // offset within klv
         ) == nfloats_per_field*bytes_per_real32;
	 // when the data runs out, read( ... ) will no loner be able to read
	 //  nfloats_per_field*bytes_per_real32 bytes

      offset += nfloats_per_field*bytes_per_real32;

      if (counter%10000==0) cout << counter << " " << flush;
      // do something with point_data
      counter++;
   } // xyz_data_remaining while conditional
   cout << "counter = " << counter << endl;
*/

   const int bytes_per_unit16=sizeof(uint16_t);
   const int nuints_per_field=1;
   uint16_t coverage_data[bytes_per_unit16]; 
   uint64_t coverage_offset = 0;   

   bool coverage_data_remaining=true;
   while (coverage_data_remaining)
   {
      coverage_data_remaining=handler->klv_read(
         TdpKeyMETADATA_COVERAGE_DENSITY, // klv_key
         klv_index,         // klv index, for multiple occurrences
         coverage_data,             // destination for data
         nuints_per_field*bytes_per_unit16,        // length of read in bytes
         tdp_data,               // this is a data klv
         coverage_offset                  // offset within klv
         ) == nuints_per_field*bytes_per_unit16;
	 // when the data runs out, read( ... ) will no loner be able to read
	 //  nfloats_per_field*bytes_per_real32 bytes
      if (counter%10000==0) cout << counter << " " << flush;
      // do something with point_data
      coverage_offset += nuints_per_field*bytes_per_unit16;
      counter++;
   } // coverage_data_remaining while conditional

   cout << "counter = " << counter << endl;

   delete( handler );  // deleting the file object calls the destructor
   // which will properly close the stream
   // this is necessary!!

   return 0;
}
