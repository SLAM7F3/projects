// ==========================================================================
// Program EXTRACT_WALLS is a playground for extruding walls from G76
// robot ladar data of S-building.

//				extract_walls

// ==========================================================================
// Last updated on 1/15/10; 1/16/10; 1/17/10
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "image/binaryimagefuncs.h"
#include "image/connectfuncs.h"
#include "geometry/contour.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "datastructures/Hashtable.h"
#include "datastructures/Linkedlist.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"


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

   string subdir="./south_lab/";
   string tdp_filename=subdir+"filtered_floorplan.tdp";
   string components_subdir=subdir+"components/";

   const double delta_x=0.05;	// meter
   const double delta_y=0.05;	// meter
   twoDarray* ztwoDarray_ptr=tdpfunc::generate_ztwoDarray_from_tdpfile(
      tdp_filename,delta_x,delta_y);

   twoDarray* ptwoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   ptwoDarray_ptr->clear_values();
   delete ztwoDarray_ptr;

   int n_iters=2;
//   cout << "Enter n_iters:" << endl;
//   cin >> n_iters;
      
   int c_start=0;
//   int n_connected_components=10;
//   int n_connected_components=50;
   int n_connected_components=198;
   double numer=0;
   double denom=0;

   for (int c=c_start; c<n_connected_components; c++)
   {
//      c=1;
//      c=10;
      cout << "Abstracting component c = " << c << endl;

      string component_tdp_filename=components_subdir+
         "component_"+stringfunc::integer_to_string(c,3)+".tdp";

      ptwoDarray_ptr->clear_values();
      tdpfunc::fill_ztwoDarray_from_tdpfile(
         component_tdp_filename,ptwoDarray_ptr);

      int n_size=1;
      double znull=0;
      twoDarray* ptwoDarray_dilated_ptr=binaryimagefunc::
         binary_dilate(n_size,znull,ptwoDarray_ptr);

      Linkedlist<threevector>* turtle_boundary_ptr=
         graphicsfunc::turtle_boundary(ptwoDarray_dilated_ptr);
//      cout << "turtle_boundary_ptr->size() = "
//           << turtle_boundary_ptr->size() << endl;

      double ds=0.1;	// meter
      contour* curr_contour_ptr=graphicsfunc::
         convert_turtle_boundary_to_contour(ds,turtle_boundary_ptr);
      delete turtle_boundary_ptr;
      curr_contour_ptr->regularize_vertices(ds);
//      cout << "regularized contour size = " 
//           << curr_contour_ptr->get_nvertices() << endl;

// Note added on Sun, Jan 17, 2010 at 9 am: 

// We have empirically found that calling
// contour::generate_consolidated_contour() does more harm than good
// for the 2009 G76 S-building ladar data set.  It leads to several
// contour edge crossings and infidelities.  So for contour fitting,
// we choose not to call the next section.  On the other hand, we
// would need to use the following section in order to determine the
// average canonical edge orientation...

/*      
// ------------------------------------------------------------------------
      double edge_angle_deviation=10*PI/180;
      contour consolidated_contour=
         curr_contour_ptr->generate_consolidated_contour(
            edge_angle_deviation);

      ds=0.2;	// meter
      consolidated_contour.regularize_vertices(ds);
      cout << "First consolidated contour size = " 
           << consolidated_contour.get_nvertices() << endl;

// Compute all contour edge orientations wrt canonical angle interval
// [0, 90 degs].  Form contour edge orientation angle average weighted
// by contour perimeters:

      double perimeter=consolidated_contour.get_perimeter();
      cout << "Perimeter = " << perimeter << endl;

      double canonical_edge_theta=
         consolidated_contour.average_edge_orientation();
      cout << "Canonical_edge_theta = " 
           << canonical_edge_theta*180/PI << endl;

      numer += perimeter * canonical_edge_theta;
      denom += perimeter;

      cout << "Weighted average of canonical edge orientation angles = " 
           << numer/denom*180/PI << endl;

// ------------------------------------------------------------------------
*/

// For the 2009 S-lab ladar data collected by G76 robot, we calculated
// the following average edge orientation angle:

      const double avg_canonical_edge_orientation = 56.33 * PI/180; 

      contour consolidated_contour=*curr_contour_ptr;

//      double delta_theta=10*PI/180;
      double delta_theta=15*PI/180;
//      double delta_theta=22.5*PI/180;
//      double delta_theta=45*PI/180;
//      edge_angle_deviation=5*PI/180;
      double edge_angle_deviation=10*PI/180;
      double ds_min=0.05;

      for (int iter=0; iter<n_iters && 
              consolidated_contour.get_nvertices() > 4; iter++)
      {
         cout << "iter = " << iter << endl;
         consolidated_contour.align_edges_with_sym_dirs(
            avg_canonical_edge_orientation,delta_theta,
            edge_angle_deviation,ds_min);
         ds_min += 0.05;
      }

      string contour_filename=components_subdir+
         "contour_"+stringfunc::integer_to_string(c,3)+".contour";
      cout << "contour_filename = " << contour_filename << endl;
      ofstream outstream;
      filefunc::openfile(contour_filename,outstream);
      consolidated_contour.write_to_textstream(outstream);
      filefunc::closefile(contour_filename,outstream);

      cout << "**********************************************" << endl;

      delete ptwoDarray_dilated_ptr;

   } // loop over index c labeling connected components
   
   delete ptwoDarray_ptr;
}
