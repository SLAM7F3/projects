// ==========================================================================
// Specialized utility program BAGDHAD_EO_TILES generates a script
// file which runs gdal_translate, eotif2tdp and lodtree for a
// sequence of Baghdad subtiles.  We wrote this program in late Aug
// 2008 as a desperate attempt to get some Baghdad background to work
// on the classified touchy2 disks (which we believe are fundamentally
// corrupted).

// 				baghdad_eo_tiles

// ==========================================================================
// Last updated on 8/31/08
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "gdal_priv.h"
#include "osg/osgGIS/raster_parser.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ==========================================================================
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

//   string input_filename;
//   cout << "Enter input super EO geotiff file name:" << endl;
//   cin >> input_filename;

   for (int r=1; r<=3; r++)
   {
      for (int c=4; c<=4; c++)
//      for (int c=1; c<=3; c++)
      {
         string input_filename="Baghdad_2006_";
         input_filename += "R"+stringfunc::number_to_string(r);
         input_filename += "C"+stringfunc::number_to_string(c);
         input_filename += ".tif";
//         cout << "input_filename = " << input_filename << endl;

//         int m_tile=3584;
         int m_tile=2799;
         int n_tile=3584;

//         for (int m=0; m<4; m++)
         for (int m=0; m<1; m++)
         {
            int xoff=m*m_tile;
            for (int n=0; n<4; n++)
            {
               int yoff=n*n_tile;

               string output_tif_filename=
                  stringfunc::prefix(input_filename)+"_"
                  +stringfunc::number_to_string(m)+"_"
                  +stringfunc::number_to_string(n)+".tif";

               string translate_str=
                  "gdal_translate -srcwin "
                  +stringfunc::number_to_string(xoff)+" "
                  +stringfunc::number_to_string(yoff)+" "
                  +stringfunc::number_to_string(m_tile)+" "
                  +stringfunc::number_to_string(n_tile)+" "
                  +input_filename+" "+output_tif_filename;
               string eotif2tdp_str=
                  "eotif2tdp "+output_tif_filename;

               string output_tdp_filename=
                  stringfunc::prefix(output_tif_filename)+".tdp";
               string lodtree_str=
                  "lodtree "+output_tdp_filename;
         
               cout << translate_str << endl;
               cout << eotif2tdp_str << endl;
               cout << lodtree_str << endl << endl;
         
            } // loop over n index
         } // loop over m index

      } // loop over c index labeling columns
   } // loop over r index labeling rows
   

} 

