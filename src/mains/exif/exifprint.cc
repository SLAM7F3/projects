// ========================================================================
// Program EXIFPRINT
// ========================================================================
// Last updated on 7/11/07; 7/12/07
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
 
   string filename=argv[1];
   photograph photo(filename);
   cout << "photo = " << photo << endl;

   exit(-1);

   Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[1]);
//   assert(image.get() != 0);
   image->readMetadata();
 
   Exiv2::ExifData& exifData = image->exifData();
   if (exifData.empty()) 
   {
      string error(argv[1]);
      error += ": No Exif data found in the file";
      throw Exiv2::Error(1, error);
   }

   Exiv2::ExifData::const_iterator end = exifData.end();

   for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) 
   {
      cout << setw(44) << std::setfill(' ') << std::left
           << i->key() << " "
           << "0x" << setw(4) << std::setfill('0') << std::right
           << std::hex << i->tag() << " "
           << setw(9) << std::setfill(' ') << std::left
           << i->typeName() << " "
           << std::dec << setw(3)
           << std::setfill(' ') << std::right
           << i->count() << "  "
           << std::dec << i->value() << endl;
   }

   Exiv2::Exifdatum curr_Datum=exifData["Exif.GPSInfo.GPSLongitude"];
   
/*

   long count=curr_Datum.count();
   cout << "count = " << count << endl;
   cout << "size = " << curr_Datum.size() << endl;
   cout << "key = " << curr_Datum.key() << endl;
   
   int degs=curr_Datum.toFloat(0);
   int mins=curr_Datum.toFloat(1);
   double secs=curr_Datum.toFloat(2);
   cout << "Degs = " << degs << " mins = " << mins << " secs = " << secs
        << endl;
   double longitude=latlongfunc::dms_to_decimal_degs(degs,mins,secs);
   cout.precision(12);
   cout << "longitude = " << longitude << endl;

//   Exiv2::Rational r=exifData["Exif.GPSInfo.GPSLongitude"];
//   double longitude=r.first/r.second;
//   cout << "longitude = " << longitude << endl;

   cout << "exifData[Exif.GPSInfo.GPSLongitude] = "
        << exifData["Exif.GPSInfo.GPSLongitude"] << endl;
   cout << "exifData[Exif.GPSInfo.GPSLatitude] = "
        << exifData["Exif.GPSInfo.GPSLatitude"] << endl;
   cout << "exifData[Exif.GPSInfo.GPSAltitude] = "
        << exifData["Exif.GPSInfo.GPSAltitude"] << endl;

   cout << "exifData[Exif.GPSInfo.GPSDestBearingRef] = "
        << exifData["Exif.GPSInfo.GPSDestBearingRef"] << endl;
   cout << "exifData[Exif.GPSInfo.GPSDestBearing] = "
        << exifData["Exif.GPSInfo.GPSDestBearing"] << endl;
*/

   photo.parse_Exif_metadata();
   cout << "photo = " << photo << endl;

   return 0;
}

