// ========================================================================
// Program TILE_NEIGHBORS reads in a text file generated in MATLAB by
// Chris Sataline which contains 2D coordinates for each of the
// Merrick ladar tiles for Lubbock, TX.  For each ladar tile, this
// program prints out a list of neighboring tiles.  The tiles are
// labeled according to their original Merrick 100-896 indices.  This
// program's output should be saved into text file
// "tile_neighbors.dat" which becomes an input into program TILE_FUSE.
// ========================================================================
// Last updated on 1/30/08; 1/31/08; 2/1/08; 2/7/08; 12/4/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "math/lttwovector.h"
#include "templates/mytemplates.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "datastructures/Triple.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   string tile_info_filename="Tiles.txt";

   if (!filefunc::ReadInfile(tile_info_filename))
   {
      cout << "Could not read in tile_info_filename = "
           << tile_info_filename << endl;
   }
   
   cout.precision(12);
   int max_x_renorm_index=44;
   int max_y_renorm_index=31;
   int max_index=basic_math::max(max_x_renorm_index,max_y_renorm_index)+1;
   genmatrix tiles(max_index,max_index);
   tiles.initialize_values(-1);

//   typedef Triple<int,int,int> tile_type;
//   tile_type curr_tile;
//   vector<tile_type> tiles;

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << i << " " << filefunc::text_line[i] << endl;
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      int tile_number=stringfunc::string_to_number(substrings[0]);
      int x_index=stringfunc::string_to_number(substrings[1]);
      int y_index=stringfunc::string_to_number(substrings[2]);
      int x_renorm_index=x_index-2;
      int y_renorm_index=y_index-1;

//      cout << tile_number << "  "
//           << x_renorm_index << "  "
//           << y_renorm_index << endl;
      tiles.put(x_renorm_index,y_renorm_index,tile_number);

//      curr_tile=tile_type(tile_number,x_renorm_index,y_renorm_index);
//      tiles.push_back(curr_tile);
//      cout << "i = " << i 
//           << " curr_tile = " << curr_tile << endl;

   } // tile_number for loop   

//   cout << "max_y_renorm_index = " << max_y_renorm_index << endl;


//   int center_tile=400;
//   cout << "Enter center tile number:" << endl;
//   cin >> center_tile;

   int start_tile=100;
   int stop_tile=896;
   for (int center_tile=start_tile; center_tile <= stop_tile; 
        center_tile++)
   {
      int i_tile=0,j_tile=0;
      for (int i=0; i<max_index; i++)
      {
         for (int j=0; j<max_index; j++)
         {
            int curr_tile_number=basic_math::round(tiles.get(i,j));
            if (curr_tile_number==center_tile)
            {
               i_tile=i;
               j_tile=j;
               break;
            }
         } // loop over j index
      } // loop over i index

//   cout << "i_tile = " << i_tile << " j_tile = " << j_tile
//        << " center_tile = " << center_tile << endl;

      int i_min=basic_math::max(0,i_tile-1);
      int i_max=basic_math::min(max_x_renorm_index,i_tile+1);
      int j_min=basic_math::max(0,j_tile-1);
      int j_max=basic_math::min(max_y_renorm_index,j_tile+1);

      vector<int> tiles_to_load;
      for (int i=i_min; i<=i_max; i++)
      {
         for (int j=j_min; j<=j_max; j++)
         {
            int curr_tile_number=basic_math::round(tiles.get(i,j));
            if (curr_tile_number >= 0)
            {
//            cout << " i = " << i << " j = " << j 
//                 << " curr_tile = " << curr_tile_number << endl;
               tiles_to_load.push_back(curr_tile_number);
            }
         } // loop over index i 
      } // loop over index j

      cout << center_tile << " ";
      for (unsigned int t=0; t<tiles_to_load.size(); t++)
      {
         string filename=stringfunc::number_to_string(tiles_to_load[t])+
            ".tif";
         cout << filename << " ";
//      cout << tiles_to_load[t] << "  ";
      }
      cout << endl;
   
   } // loop over center_tile index


}
