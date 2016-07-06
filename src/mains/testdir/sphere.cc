// ==========================================================================
// Program sphere uses the GTS library to generate a triangulated unit
// sphere by recursive subdivision.  First approximation is an
// isocahedron; each level of refinement increases the number of
// triangles by a factor of 4.

// Usage: sphere 

// ==========================================================================
// Last updated on 3/2/07
// ==========================================================================

#include <iostream>
#include <locale.h>
#include <stdlib.h>
#include <string>
#include "config.h"
#include "gts.h"
#include "math/constants.h"

int main (int argc, char * argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;

   int LOD_level=4;
//   cout << "Enter LOD setting:" << endl;
//   cin >> LOD_level;
   guint level=LOD_level;

// Generate triangulated sphere:

   GtsSurface* s = gts_surface_new (gts_surface_class (),
                                    gts_face_class (),
                                    gts_edge_class (),
                                    gts_vertex_class ());
   gts_surface_generate_sphere (s, level);
   gts_surface_print_stats (s, stdout);

   gboolean closed_surface_flag=gts_surface_is_closed(s);
   cout << "closed_surface_flag = " << closed_surface_flag << endl;

   gdouble surface_area=gts_surface_area(s);
   gdouble volume=gts_surface_volume(s);
   cout << "surface_area/4*PI = " << surface_area/(4*PI) << endl;
   cout << "volume/ (4/3 PI) = " << volume/(4.0/3.0 * PI) << endl;

// write generated surface to standard output 

   string output_filename="sphere.gts";
   FILE* fp= fopen(output_filename.c_str(), "w");
   gts_surface_write (s, fp);
   fclose(fp);

   return 1;
}
