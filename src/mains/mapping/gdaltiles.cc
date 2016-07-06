// ========================================================================
// Program GDALTILES generates an executable script which calls
// GDAL_TRANSLATE on a large, mosaiced TIF file.  Extremal easting &
// northing values bounding the tiles to be extracted from the TIF
// mosaic are hardwired in this main program.  The spatial extents of
// the individual tiles are also hardwired (5 km).  

// 				gdaltiles

// ========================================================================
// Last updated on 4/15/11
// ========================================================================

#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{

   string gdal_cmd_file="split_merged_file_into_tiles";
   ofstream outstream;
   filefunc::openfile(gdal_cmd_file,outstream);

   string merged_tiles_filename="merged_ptiles.tif";
   string merged_tiles_suffix=stringfunc::suffix(merged_tiles_filename);

   double min_easting=650300;
   double max_easting=687400;
   
   double min_northing=3857700;
   double max_northing=3891000;
   
   double d_easting=5*1000;	// meters
   double d_northing=5*1000;	// meters
   int n_eastings=(max_easting-min_easting)/d_easting;
   int n_northings=(max_northing-min_northing)/d_northing;
   
   for (int i=0; i<n_eastings; i++)
   {
      double curr_easting=min_easting+i*d_easting;
      for (int j=0; j<n_northings; j++)
      {
         double curr_northing=min_northing+j*d_northing;
         string gdal_cmd="gdal_translate -projwin ";
         gdal_cmd += stringfunc::number_to_string(curr_easting)+" ";
         gdal_cmd += stringfunc::number_to_string(curr_northing+d_northing)
            +" ";
         gdal_cmd += stringfunc::number_to_string(curr_easting+d_easting)+" ";
         gdal_cmd += stringfunc::number_to_string(curr_northing)+" ";
         gdal_cmd += merged_tiles_filename+" ";
         gdal_cmd += "tile_"+stringfunc::number_to_string(i)+"_"+
            stringfunc::number_to_string(j);
         gdal_cmd += "."+merged_tiles_suffix;
//         cout << gdal_cmd << endl;
         outstream << gdal_cmd << endl;
      } // loop over j index
   } // loop over i index

   filefunc::closefile(gdal_cmd_file,outstream);

   string unix_cmd="chmod a+x "+gdal_cmd_file;
   sysfunc::unix_command(unix_cmd);

   string banner="Gdal translate commands written to "+gdal_cmd_file;
   outputfunc::write_big_banner(banner);
   
}
