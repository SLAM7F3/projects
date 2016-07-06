// ==========================================================================
// Program VOLUME uses the GTS library to compute the volume of a
// given surface if it is a closed and orientable manifold

// Usage: volume < file.gts

// ==========================================================================
// Last updated on 3/2/07
// ==========================================================================

#include <iostream>
#include <stdlib.h>
#include <locale.h>
#include "config.h"
#include "gts.h"
#include "math/threevector.h"

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   
// Read in surface

   GtsSurface* s = gts_surface_new (gts_surface_class (),
                                    gts_face_class (),
                                    gts_edge_class (),
                                    gts_vertex_class ());
   GtsFile* fp = gts_file_new (stdin);
   if (gts_surface_read (s, fp)) 
   {
      cout << "The input file is not a valid GTS file" << endl;
      cout << "line = " << fp->line << " posn = " << fp->pos
           << " error = " << fp->error << endl;
      return 1; // failure 
   }

   gts_surface_print_stats (s, stdout);

// Check that surface is a closed and orientable manifold:

   cout << "closed surface = " << gts_surface_is_closed(s) << endl;
   cout << "orientable surface = " << gts_surface_is_orientable(s) << endl;
      
   if (!gts_surface_is_closed(s) || !gts_surface_is_orientable(s))
   {
      return 1; /* failure */
   }
  
   cout << "Volume = " << gts_surface_volume(s) << endl;

   GtsVector COM;
   gts_surface_center_of_mass (s, COM);
   cout << "COM = " << threevector(COM[0],COM[1],COM[2]) << endl;

   return 0; /* success */
}
