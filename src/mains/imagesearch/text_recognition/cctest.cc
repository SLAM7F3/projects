// ==========================================================================
// Program CCTEST
// ==========================================================================
// Last updated on 7/17/12
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "image/extremal_region.h"
#include "general/filefuncs.h"
#include "datastructures/Forest.h"
#include "image/graphicsfuncs.h"
#include "general/sysfuncs.h"
#include "datastructures/tree.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
 
//   string input_filename="junk.dat";
   string input_filename="c.dat";

   filefunc::ReadInfile(input_filename);
   int ydim=filefunc::text_line.size();
   cout << "ydim = " << ydim << endl;

//   int xdim=5;
   int xdim=34;

   twoDarray* pbinary_twoDarray_ptr=new twoDarray(xdim,ydim);
   pbinary_twoDarray_ptr->clear_values();
   
   for (int r=0; r<ydim; r++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[r]);
      cout << "r = " << r << " column_values.size() = "
           << column_values.size() << endl;
      for (int c=0; c<column_values.size(); c++)
      {
         pbinary_twoDarray_ptr->put(c,r,basic_math::round(column_values[c]));
      } // loop over index c
   } // loop over index r
   
   int n_neighbors=4;
   int label_offset=10;
   double z_null=0;
   
//   int xdim=4;
//   int ydim=7;

//   int xdim=18;
//   int ydim=4;


   twoDarray* cc_twoDarray_ptr=new twoDarray(pbinary_twoDarray_ptr);

/*
   pbinary_twoDarray_ptr->put(5,1,1);
   pbinary_twoDarray_ptr->put(6,1,1);
   pbinary_twoDarray_ptr->put(10,1,1);
   pbinary_twoDarray_ptr->put(11,1,1);
   pbinary_twoDarray_ptr->put(12,1,1);

   pbinary_twoDarray_ptr->put(3,2,1);
   pbinary_twoDarray_ptr->put(13,2,1);
   pbinary_twoDarray_ptr->put(14,2,1);

   pbinary_twoDarray_ptr->put(1,3,1);
   pbinary_twoDarray_ptr->put(2,3,1);
*/
 
/*
//   pbinary_twoDarray_ptr->put(1,1,1);
//   pbinary_twoDarray_ptr->put(2,1,1);
   pbinary_twoDarray_ptr->put(2,2,1);
   pbinary_twoDarray_ptr->put(2,3,1);
   pbinary_twoDarray_ptr->put(2,4,1);


   pbinary_twoDarray_ptr->put(2,5,1);
   pbinary_twoDarray_ptr->put(1,5,1);
   pbinary_twoDarray_ptr->put(0,5,1);

//   pbinary_twoDarray_ptr->put(0,6,1);
*/

   
   for (int py=0; py<pbinary_twoDarray_ptr->get_ydim(); py++)
   {
      for (int px=0; px<pbinary_twoDarray_ptr->get_xdim(); px++)
      {
         cout << pbinary_twoDarray_ptr->get(px,py) << " ";
      }
      cout << endl;
   }
   cout << endl;


//   cout << "*pbinary_twoDarray_ptr = " 
//        << *pbinary_twoDarray_ptr << endl;

   int n_components=graphicsfunc::Label_Connected_Components(
      n_neighbors,label_offset,z_null,
      pbinary_twoDarray_ptr,cc_twoDarray_ptr);
   
   cout << "inside main cctest, n_components = " << n_components << endl;

   for (int py=0; py<cc_twoDarray_ptr->get_ydim(); py++)
   {
      for (int px=0; px<cc_twoDarray_ptr->get_xdim(); px++)
      {
         cout << cc_twoDarray_ptr->get(px,py) << " ";
      } // loop over py index
      cout << endl;
   } // loop over px index


}
