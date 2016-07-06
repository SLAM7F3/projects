// ==========================================================================
// Program ALIRT_TILES_IN_BBOX has hardwired lon-lat [or
// easting,northing UTM] geocoordinates for lower-left and upper-right
// corners of a bounding box.  It generates an STL vector of filenames
// corresponding to ALIRT tiles (in Ross' base-32 naming convention)
// which lie inside the bbox.  ALIRT_TILES_IN_BBOX writes out an
// executable script which copies the ALIRT tiles into a specified
// subdirectory.

//			alirt_tiles_in_bbox

// ==========================================================================
// Last updated on 2/24/11; 2/25/11; 4/3/11; 7/16/11
// ==========================================================================

#include <iomanip>
#include <map>
#include <string>
#include <vector>

#include "video/camera.h"
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "ladar/ladarfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   bool northern_hemisphere_flag=true;

   double lower_left_lon=-112.8;	// Grand Canyon
   double lower_left_lat=35.94;	// Grand Canyon
//   double lower_left_lon=-71.269;
//   double lower_left_lat=42.4575;
//   double lower_left_lon=-71.29;
//   double lower_left_lat=42.44;
   geopoint lower_left(lower_left_lon,lower_left_lat);

   double upper_right_lon=-112.13;
   double upper_right_lat=36.26;
//   double upper_right_lon=-71.245;
//   double upper_right_lat=42.48;
   geopoint upper_right(upper_right_lon,upper_right_lat);

/*
   int UTM_zone=19;
   double easting=311600;
   double northing=4701833;
//   double easting=311976;	// D7 near flight facility
//   double northing=4702920;	// D7 near flight facility
   geopoint lower_left(northern_hemisphere_flag,UTM_zone,
		       easting,northing);

   easting=315461;
   northing=4705114;
//   easting=313217;		// D7 near flight facility
//   northing=4704571;		// D7 near flight facility
   geopoint upper_right(northern_hemisphere_flag,UTM_zone,
		        easting,northing);

*/

   cout << "Lower left corner = " << lower_left << endl;
   cout << "Upper right corner = " << upper_right << endl;

   vector<string> tile_labels=ladarfunc::AlirtTilesInBbox(
      lower_left,upper_right);

   ofstream outstream;
//   string script_filename="cp_tiles";
   string script_filename="ln_tiles";
   filefunc::openfile(script_filename,outstream);

   string osga_subdir="/media/66368D22368CF3F9/GrandCanyon/ALIRT/osga/";
   string subdir="./bbox_tiles";
   outstream << "mkdir "+subdir << endl;

   for (int i=0; i<tile_labels.size(); i++)
   {
//      string curr_filename=tile_labels[i]+".tif";
      string curr_filename=tile_labels[i]+".osga";

      string pathname=osga_subdir+curr_filename;
      if (!filefunc::fileexist(pathname)) continue;

//      string cp_cmd="cp "+curr_filename+" "+subdir;
      string ln_cmd="ln -s ../"+curr_filename+"  . ";
//      outstream << cp_cmd << endl;
      outstream << ln_cmd << endl;
   }
   filefunc::closefile(script_filename,outstream);

   string unix_cmd="chmod a+x "+script_filename;
   sysfunc::unix_command(unix_cmd);

/*

   string tile_label="0SGA-0Z1Q";
   cout << "tile_label = " << tile_label << endl;
   

   double longitude,latitude;
   ladarfunc::AlirtTileLabelToLonLat(tile_label,longitude,latitude);

   cout.precision(12);
   cout << "lon = " << longitude << " lat = " << latitude << endl;

   string check_label=ladarfunc::LonLatToAlirtTileLabel(longitude,latitude);
   cout << "check_label = " << check_label << endl;
*/

}
