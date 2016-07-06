// ========================================================================
// Program IPHONE_METADATA is a specialized utility which converts
// iPhone GPS coordinates into Noah's raw BUNDLER coordinates.  We
// hardwire the transformation between world and BUNDLER coordinates
// within this program.

// 		     iphone_metadata ./IMG_0142.JPG

// ========================================================================
// Last updated on 2/5/10; 2/24/10
// ========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image.hpp"
#include "exif.hpp"

#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "video/photograph.h"
#include "math/rotation.h"

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
//   cout << "photo = " << photo << endl;

/*

Peter's raw coordinate conventions:

XY plane corresponds to "ground"
+Z_hat represents unit normal pointing up away from ground

X_peter=-X_noah
Y_peter=Z_noah
Z_peter=Y_noah


2.  Converting from raw Peter coordinates to georegistered UTM coordinates


XYZ_peter *= scale_factor
XYZ_peter += translation

rotation global_R=rotation_from_az_el_roll(
         global_az,global_el,global_roll);

rel_XYZ=XYZ_peter-rotation_origin;
rel_XYZ = global_R * rel_XYZ;
georegistered_XYZ=rel_XYZ+rotation_origin;


For MIT 2.3K reconstructed set:


scale_factor = 11.2648221644

translation_X =  328141.302781 meters
translation_Y =  4692067.27943 meters
translation_Z = 18.7822026982 meters

global_az  = -159.785505829 degs
global_el  = 1.14926469438 degs
global_roll = -16.5751038749 degs

global_R =

-0.938217    0.325808    0.116611    
-0.345466    -0.901389    -0.261058    
0.0200571    -0.285215    0.958254    


rotation_origin_X = 328212.210605 meters
rotation_origin_Y = 4692025.66432 meters
rotation_origin_Z = 36.1552629968 meters
*/

   threevector georegistered_camera_posn=photo.get_geolocation().
      get_UTM_posn();

   const bool northern_hemisphere_flag=true;
   int UTM_zone=19;	// Boston
   geopoint camera_geoposn(
      northern_hemisphere_flag,UTM_zone,
      georegistered_camera_posn.get(0),
      georegistered_camera_posn.get(1),
      georegistered_camera_posn.get(2));

   const threevector rotation_origin(
      328212.210605 , 4692025.66432 , 36.1552629968 );
   threevector rel_camera_posn=georegistered_camera_posn-rotation_origin;


   double global_az  = -159.785505829 * PI/180;
   double global_el  = 1.14926469438 * PI/180;
   double global_roll = -16.5751038749 * PI/180;
   rotation R,Rtrans;
   R=R.rotation_from_az_el_roll(global_az,global_el,global_roll);
   Rtrans=R.transpose();
   
   rel_camera_posn=Rtrans*rel_camera_posn;

   threevector camera_posn_peter=rel_camera_posn+rotation_origin;

   const threevector translation(328141.302781, 4692067.27943, 18.7822026982);
   camera_posn_peter -= translation;

   const double scale_factor = 11.2648221644;
   camera_posn_peter /= scale_factor;
//   cout << "camera_posn_peter = " << camera_posn_peter << endl;

   threevector camera_posn_noah;
   camera_posn_noah.put(0,-camera_posn_peter.get(0));
   camera_posn_noah.put(1,camera_posn_peter.get(2));
   camera_posn_noah.put(2,camera_posn_peter.get(1));
//   cout << "camera_posn_noah = " << camera_posn_noah << endl;

   cout.precision(12);
   cout << filename << " "
        << camera_geoposn.get_UTM_easting() << " "
        << camera_geoposn.get_UTM_northing() << " "
        << camera_geoposn.get_longitude() << " "
        << camera_geoposn.get_latitude() 
        << endl;

/*
   cout << filename << "\t\t"  
        << georegistered_camera_posn.get(0) << " "
        << georegistered_camera_posn.get(1) << " "
        << georegistered_camera_posn.get(2) << " "
        << camera_posn_noah.get(0) << "\t\t"
        << camera_posn_noah.get(1) << "\t\t"
        << camera_posn_noah.get(2) << "\t\t"
        << endl;
*/
}

