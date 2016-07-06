// ==========================================================================
// Program TILE_TRANSFORM reads in a set of manually selected
// ladar/satellite EO tiepoints from two feature text files.  It also
// reads in a text file containing the easting,northing coordinates
// for the Lubbock tile centers.  For each individual tile center,
// this program performs a KDtree search to find the closest 8
// tiepoints.  It then solves a least-squares fit for the 6 linear
// transformation parameters (2x2 matrix plus 2-vector translation)
// which match the ladar and EO tiepoints.  The tiepoint with the
// worst residual is discarded, and the least-squares fit is performed
// a second time.  The revised matrix and translation parameters for
// each tile are written to an output text file.  This information may
// subsequently be read in by program TILE_FUSE.
// ==========================================================================
// Last updated on 2/6/08; 2/7/08
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "kdtree/kdtreefuncs.h"
#include "general/outputfuncs.h"
#include "math/rubbersheet.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   const int PRECISION=15;
   cout.precision(PRECISION);

   string ladar_features_filename="fladar.txt";
   filefunc::ReadInfile(ladar_features_filename);

   vector<threevector> XYID;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      int feature_ID=stringfunc::string_to_number(substring[1]);
      double X=stringfunc::string_to_number(substring[3]);
      double Y=stringfunc::string_to_number(substring[4]);
      XYID.push_back(threevector(X,Y,feature_ID));
//      cout << "i = " << i 
//           << " ID = " << XYID.back().get(2)
//           << " X = " << XYID.back().get(0)
//           << " Y = " << XYID.back().get(1)
//           << endl;
   }

// Form KDtree from 3D ladar feature information:

   KDTree::KDTree<2, threevector>* kdtree_ptr=
      kdtreefunc::generate_2D_kdtree(XYID);
//   cout << "kdtree = " << *kdtree_ptr << endl;

   string EO_features_filename="fEO.txt";
   filefunc::ReadInfile(EO_features_filename);
   
// Form UV STL vector containing EO feature information:

   vector<twovector> UV;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
      vector<string> substring=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      
      double U=stringfunc::string_to_number(substring[3]);
      double V=stringfunc::string_to_number(substring[4]);
      UV.push_back(twovector(U,V));
   }

// Read in easting and northing locations of Lubbock tile centers:

   string tile_centers_filename="tile_centers.dat";
   vector<threevector> tile_centers;
   if (filefunc::ReadInfile(tile_centers_filename))
   {
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<double> input_numbers=stringfunc::string_to_numbers(
            filefunc::text_line[i]);
         tile_centers.push_back(threevector(
            input_numbers[0],input_numbers[1],input_numbers[2]));
      } // loop over index i labeling lines within tile centers file
   }
   else
   {
      cout << "Could not read in tile centers from input file!" << endl;
      exit(-1);
   }
   
// Instantiate rubber sheet object which locally transforms ladar
// easting and northing values into EO eastings and northings:

   rubbersheet RubberSheet;

// Open output file to write linear transformation parameters for each
// Lubbock tile:

   string output_filename="linear_warp.txt";
   ofstream outstream;
   outstream.precision(PRECISION);
   filefunc::openfile(output_filename,outstream);

//   while (true)   
   for (unsigned int t=0; t<tile_centers.size(); t++)
   {
      double rho=2000;	// Initial search radius in meters
      int n_closest_nodes=8;

      double easting,northing;
      easting=tile_centers[t].get(1);
      northing=tile_centers[t].get(2);

      cout << "t = " << t 
           << " easting = " << easting << " northing = " << northing
           << endl;

//      cout << "Enter easting X value:" << endl;
//      cin >> easting;
//      cout << "Enter northing Y value:" << endl;
//      cin >> northing;
      threevector curr_XYZ(easting,northing,0);

      vector<threevector> closest_curr_node;
      kdtreefunc::find_closest_nodes(
         kdtree_ptr,curr_XYZ,rho,n_closest_nodes,closest_curr_node);

      vector<threevector> nearby_XYID;
      vector<twovector> nearby_UV;
//      cout << "closest_curr_node.size() = " << closest_curr_node.size()
//           << endl;

      for (unsigned int i=0; i<closest_curr_node.size(); i++)
      {
         double X=closest_curr_node[i].get(0);
         double Y=closest_curr_node[i].get(1);
         int ID=closest_curr_node[i].get(2);
//         cout << "ID = " << ID << endl;
         double U=UV[ID].get(0);
         double V=UV[ID].get(1);

         cout << "3D feature ID = " << ID
              << " X = " << X
              << " U = " << U
              << " Y = " << Y
              << " V = " << V << endl;
         cout << endl;

         nearby_XYID.push_back(threevector(X,Y,ID));
         nearby_UV.push_back(twovector(U,V));
      }

      RubberSheet.copy_XYID(nearby_XYID);
      RubberSheet.copy_UV(nearby_UV);

// Perform first round of rubber sheeting using all features returned
// from KDtree search:

      vector<bool> retained_feature_flag;
      for (unsigned int i=0; i<RubberSheet.get_n_features(); i++)
      {
         retained_feature_flag.push_back(true);
      }
      RubberSheet.fit_linear_warp(retained_feature_flag);

// Use residuals to identify worst feature match in least squares fit:

      int npoints_to_discard=1;
      RubberSheet.compare_measured_and_linearly_transformed_UVs(
         npoints_to_discard,retained_feature_flag);

// Perform second round of rubber sheeting with worst feature not
// included into least squares fit:

      RubberSheet.fit_linear_warp(retained_feature_flag);

      npoints_to_discard=0;
      RubberSheet.compare_measured_and_linearly_transformed_UVs(
         npoints_to_discard,retained_feature_flag);

      genmatrix* M_ptr=RubberSheet.get_M_ptr();
      twovector trans=RubberSheet.get_trans();
      
      outstream << basic_math::round(tile_centers[t].get(0)) << "  "
                << M_ptr->get(0,0) << "  "
                << M_ptr->get(0,1) << "  "
                << M_ptr->get(1,0) << "  "
                << M_ptr->get(1,1) << "  "
                << trans.get(0) << "  "
                << trans.get(1) << endl;

   } // loop over index t labeling Lubbock tiles
//   } // infinite input while loop
   
   filefunc::closefile(output_filename,outstream);   

}
