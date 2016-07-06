// ========================================================================
// Program TIMEPRINT
// ========================================================================
// Last updated on 7/11/07; 7/12/07; 4/20/10
// ========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image.hpp"
#include "exif.hpp"

#include "astro_geo/latlong2utmfuncs.h"
#include "video/photograph.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::setw;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   if (argc != 2) {
      cout << "Usage: " << argv[0] << " file" << endl;
      return 1;
   }
 
   int id=-1;
   string filename=argv[1];
   photograph photo(id,filename);
   photo.set_UTM_zonenumber(19);	// Boston

   photo.parse_Exif_metadata();
   int secs_since_epoch=photo.get_clock().secs_elapsed_since_reference_date();
   cout << "secs_since_epoch = " << secs_since_epoch << endl;
//   cout << "photo = " << photo << endl;

   return 0;
}

