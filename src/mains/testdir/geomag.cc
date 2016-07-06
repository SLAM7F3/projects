// =======================================================================
// PROGRAM MAGPOINT (GEOMAG DRIVER) 
// =======================================================================
// Last updated on 7/30/07; 7/31/07
// =======================================================================

#include <iostream>
#include <math.h>               
#include <string>
#include "astro_geo/geomagnet.h"
#include "astro_geo/geopoint.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// =======================================================================

int main()
{
   geomagnet curr_geomagnet;

   curr_geomagnet.input_geolocation_and_time();
   curr_geomagnet.compute_magnetic_field_components();
   curr_geomagnet.display_magnetic_field_components();
}
