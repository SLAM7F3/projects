// ==========================================================================
// Program MAtif2UTMtif takes in the names for geotif files which were
// converted from MrSID files downloaded from the MASSGIS website.
// This program first write out a call to GDAL_TRANSLATE which
// rescales the RGB values from 0 - 255 to 1 - 255.  It next writes
// out a call to GDALWARP which reprojects from Massachusetts State
// Plane to UTM coordinates.  This program outputs a script file which
// can be run on multiple geotif files.

//		       	       MAtif2UTMtif

// ==========================================================================
// Last updated on 8/27/09
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ios;
   using std::istream;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   string tif_filenames_file;
   cout << "Enter name of file containing TIF filenames converted from MrSID files:" << endl;
   cin >> tif_filenames_file;

   filefunc::ReadInfile(tif_filenames_file);

   ofstream outstream;
   string output_filename="run_tif2UTM";
   filefunc::openfile(output_filename,outstream);

   for (int n=0; n<filefunc::text_line.size(); n++)
   {
      string curr_tif_filename=filefunc::text_line[n];
      string rescaled_tif_filename="rescaled_"+curr_tif_filename;
      string output_UTM_filename="UTM_"+curr_tif_filename;
      string unix_command=
         "/usr/local/bin/gdal_translate -scale 0 255 1 255 "+
         curr_tif_filename+" "+rescaled_tif_filename;
      string unix_command2=
         "/usr/local/bin/gdalwarp -t_srs '+proj=utm +zone=19 +dataum=WGS84' -dstnodata 0 "+
         rescaled_tif_filename+" "+output_UTM_filename;
      outstream << unix_command << endl;
      outstream << unix_command2 << endl << endl;
      
   } // loop over index n

   filefunc::closefile(output_filename,outstream);
}
