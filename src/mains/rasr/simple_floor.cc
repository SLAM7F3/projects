// ==========================================================================
// Program SIMPLE_FLOOR is a playground for cleaning up Group 76 robot
// ladar scans of S-building.  We first bin continuous XY position
// coordinates into a lattice with 5 cm x 5 cm cells.  We next
// perform recursive filling followed by recursive emptying to reduce
// small noise artificats in the ladar map.  After outputing a
// filtered TDP file, this program calls LODTREE to generate a
// corresponding OSGA file.

//				simple_floor			

// ==========================================================================
// Last updated on 12/30/09
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "image/recursivefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"
#include "osg/osg3D/tdpfuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

   string subdir="./south_lab/";
   const double delta_x=0.05;	// meter
   const double delta_y=0.05;	// meter

/*
   string floorplan_xy_filename=subdir+"floorplan_xy.dat";

   string banner="Reading in floorplan_xy file:";
   outputfunc::write_banner(banner);
   
   filefunc::ReadInfile(floorplan_xy_filename);
   cout << "filefunc::text_line.size() = " << filefunc::text_line.size()
        << endl;

   vector<double> X,Y;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      if (i%10000==0) cout << i/10000 << " " << flush;
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      if (!stringfunc::is_number(substrings[0]) ||
          !stringfunc::is_number(substrings[1])) continue;

      X.push_back(stringfunc::string_to_number(substrings[0]));
      Y.push_back(stringfunc::string_to_number(substrings[1]));
//      cout << "i = " << i << " X = " << X.back() << " Y = " << Y.back()
//           << endl;
   }
   cout << endl;
   cout << "X.size() = " << X.size() << endl;

   double x_max=mathfunc::maximal_value(X);
   double x_min=mathfunc::minimal_value(X);
   double y_max=mathfunc::maximal_value(Y);
   double y_min=mathfunc::minimal_value(Y);

   cout << "x_max = " << x_max << " x_min = " << x_min << endl;
   cout << "y_max = " << y_max << " y_min = " << y_min << endl;


   int mdim=(x_max-x_min)/delta_x;
   int ndim=(y_max-y_min)/delta_y;
   twoDarray* ptwoDarray_ptr=new twoDarray(mdim,ndim);
   ptwoDarray_ptr->init_coord_system(x_min,x_max,y_min,y_max);
   ptwoDarray_ptr->clear_values();

   int px,py;
   const double Z_height=1.0;
   for (int i=0; i<X.size(); i++)
   {
      if (ptwoDarray_ptr->point_to_pixel(X[i],Y[i],px,py))
      {
         ptwoDarray_ptr->put(px,py,Z_height);
//         ptwoDarray_ptr->increment(px,py,1);
      }
   } // loop over index i labeling XY points

   string UTMzone="19";
   string tdp_filename="floorplan_twoDarray.tdp";
   tdpfunc::write_xyz_data(UTMzone,tdp_filename,ptwoDarray_ptr);


   exit(-1);
*/


   string tdp_filename=subdir+"floorplan_twoDarray.tdp";
   twoDarray* ptwoDarray_ptr=tdpfunc::generate_ztwoDarray_from_tdpfile(
      tdp_filename,delta_x,delta_y);

//   bool find_pixel_borders_before_filling=true;
   bool find_pixel_borders_before_filling=false;
   
   unsigned int px_min=0;
   unsigned int py_min=0;
   unsigned int px_max=ptwoDarray_ptr->get_mdim()-1;
   unsigned int py_max=ptwoDarray_ptr->get_ndim()-1;

   double zmin=0;
   double znull=0;

   string banner="Recursive filling:";
   outputfunc::write_banner(banner);

   int nrecursion_emptying_max=5;
//   cout << "Enter nrecursion_emptying_max:" << endl;
//   cin >> nrecursion_emptying_max;

   int nrecursion_filling_max=basic_math::max(2,nrecursion_emptying_max/3);
   double zempty_value=0;
   double zfill_value=1;
   recursivefunc::binary_fill(
      nrecursion_filling_max,px_min,px_max,py_min,py_max,
      zempty_value,zfill_value,ptwoDarray_ptr);

   banner="Recursive emptying:";
   outputfunc::write_banner(banner);

   recursivefunc::recursive_empty(
      nrecursion_emptying_max,zmin,ptwoDarray_ptr,
      find_pixel_borders_before_filling,znull);

/*
   int n_ones=0;
   for (int px=0; px<ptwoDarray_ptr->get_mdim(); px++)
   {
      for (int py=0; py<ptwoDarray_ptr->get_ndim(); py++)
      {
         if (ptwoDarray_ptr->get(px,py) < 0.5)
         {
            ptwoDarray_ptr->put(px,py,xyzpfunc::null_value);
         }
         else
         {
            n_ones++;
         }
      } // loop over py
   } // loop over px
   cout << "mdim*ndim = " << ptwoDarray_ptr->get_mdim()*
      ptwoDarray_ptr->get_ndim() << endl;
   cout << "n_ones = " << n_ones << endl;
*/

 
   string UTMzone="19";
   string filtered_tdp_filename=subdir+"filtered_floorplan.tdp";
   tdpfunc::write_xyz_data(UTMzone,filtered_tdp_filename,ptwoDarray_ptr);
   delete ptwoDarray_ptr;

   string unix_command="lodtree "+filtered_tdp_filename;
   sysfunc::unix_command(unix_command);


}
