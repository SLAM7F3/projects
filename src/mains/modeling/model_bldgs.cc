// ==========================================================================
// Program MODEL_BLDGS reads in 2D polylines generated via VIDEO from
// orthorectified aerial EO imagery.  It also takes in an affine 3x4
// projection matrix for the aerial image.  MODEL_BLDGS converts from
// UV image plane coordinates to easting,northing geocoordinates.
// After combining with ladar-derived height information, MODEL_BLDGS
// generates simple polyhedra models for urban buildings.

//        model_bldgs --region_filename ./packages/Cambridge_aerial.pkg

// ==========================================================================
// Last updated on 1/9/12; 1/10/12; 1/12/12; 1/22/12
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "geometry/contour.h"
#include "general/filefuncs.h"
#include "math/genvector.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "geometry/polyline.h"
#include "geometry/polyhedron.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "osg/osgTiles/TilesGroup.h"

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

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   cout.precision(12);

// Import MIT TEC2004 ladar data from geotif file:

   TilesGroup* TilesGroup_ptr=new TilesGroup();
   string map_countries_name="Boston";
   string geotif_subdir="/data/DTED/"+map_countries_name+"/geotif/";
   TilesGroup_ptr->set_geotif_subdir(geotif_subdir);
   TilesGroup_ptr->set_ladar_height_data_flag(true);
   TilesGroup_ptr->purge_tile_files();

   string ladar_tile_filename=geotif_subdir+"Ztiles/"+"MIT_TEC04.tif";
   TilesGroup_ptr->load_ladar_height_data_into_ztwoDarray(ladar_tile_filename);
   twoDarray* DTED_ztwoDarray_ptr=TilesGroup_ptr->get_DTED_ztwoDarray_ptr();
   cout << "DTED_ztwoDarray = " << *DTED_ztwoDarray_ptr << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   genmatrix* P_ptr=NULL;
   vector<camera*> camera_ptrs;
   for (int n=0; n<n_photos; n++)
   {
      PassInfo* PassInfo_ptr=passes_group.get_passinfo_ptr(n);
      P_ptr=PassInfo_ptr->get_projection_matrix_ptr();


      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
//      cout << "n = " << n << " *photo_ptr = " << *photo_ptr << endl;
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      camera_ptrs.push_back(camera_ptr);

      camera_ptr->set_projection_matrix(*P_ptr);
//      cout << "n = " << n
//           << " camera = " << *camera_ptr << endl;
   }

   double Xlo=-P_ptr->get(0,3);
   double Ylo=-P_ptr->get(1,3);
   double deltaY=P_ptr->get(2,3);

// Read in polylines manually extracted from orthorectified aerial
// photo:

   string polyline_filename="./bldg_2D_footprints.txt";
   cout << "polyline_filename = " << polyline_filename << endl;
   filefunc::ReadInfile(polyline_filename);

   int n_polylines=0;
   int prev_polyline_ID=-1;
   vector<int> polyline_IDs;
   vector<fourvector> footprint_XYZID;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);

      int polyline_ID=column_values[1];
      if (polyline_ID != prev_polyline_ID)
      {
         polyline_IDs.push_back(polyline_ID);
         n_polylines++;
         prev_polyline_ID=polyline_ID;
      }
      
      double time=column_values[0];
      int passnumber=column_values[2];
      double U=column_values[3];
      double V=column_values[4];

      double X=Xlo+deltaY*U;
      double Y=Ylo+deltaY*V;

      footprint_XYZID.push_back(fourvector(X,Y,0,polyline_ID));

      cout << "U = " << U << " V = " << V
           << " X = " << X << " Y = " << Y << endl;
   } // loop over index i 

// Note added on Weds, Jan 11, 2012:

// Need to renumber integer IDs within footprint_XYZID STL vector so
// that they sequentially range from 0 to number of input polylines:

   cout << "n_polylines = " << n_polylines << endl;
   cout << "polyline_IDs.size() = " << polyline_IDs.size() << endl;

// Purge contents of OFF subdirectory:

   string OFF_subdir="./OFF/";
   string off_suffix="off";
   filefunc::purge_files_with_suffix_in_subdir(OFF_subdir,off_suffix);

   int max_n_peaks=4;
   cout << "Enter max_n_peaks:" << endl;
   cin >> max_n_peaks;

// ==========================================================================
// Iteration over building footprint polylines starts here

   int prev_building_ID=-1;
   int building_part_counter=0;
   for (int polyline_counter=0; polyline_counter<n_polylines; 
        polyline_counter++)
   {
      int polyline_ID=polyline_IDs[polyline_counter];
//      cout << "polyline_counter = " << polyline_counter
//           << " polyline_ID = " << polyline_ID << endl;

// As of 1/12/12, we assume IDs for polylines all corresponding to the
// same building are offset by 1000: e.g. 7, 1007, 2007, etc...

      int building_ID=polyline_ID%1000;
      if (building_ID != prev_building_ID)
      {
         prev_building_ID=building_ID;
         building_part_counter=0;
      }

      vector<threevector> bottom_footprint_XYZs;

      for (int i=0; i<footprint_XYZID.size(); i++)
      {
         int curr_footprint_ID=footprint_XYZID[i].get(3);
         if (curr_footprint_ID == polyline_ID) 
         {
            bottom_footprint_XYZs.push_back(threevector(
               footprint_XYZID[i]));
         }
      } // loop over index i 

//      cout << "bottom_footprint_XYZs.size() = "
//           << bottom_footprint_XYZs.size() << endl;

// Generate contour corresponding to building's footprint:

      polyline bottom_footprint(bottom_footprint_XYZs);
//      cout << "Bottom footprint = " << bottom_footprint << endl;

      polygon bottom_poly(bottom_footprint);
//   cout << "bottom_poly = " << bottom_poly << endl;

      contour bottom_contour(&bottom_poly);

      double ds=1;	// meters
      bottom_contour.regularize_vertices(ds);

      bottom_contour.compute_edges();
//   cout << "bottom_contour = " << bottom_contour << endl;
//   cout << "contour perimeter = " << bottom_contour.get_perimeter() << endl;

      vector<double> interior_Zs,exterior_Zs;
      vector<threevector> edge_posns;
      for (int e=0; e<bottom_contour.get_nedges(); e++)
      {
         linesegment curr_edge=bottom_contour.get_edge(e);
         threevector r_hat=bottom_contour.radial_direction_vector(e);
         threevector curr_edge_posn=curr_edge.get_v1();
         edge_posns.push_back(curr_edge_posn);

         threevector interior_posn=curr_edge_posn-2*ds*r_hat;
         double curr_z=DTED_ztwoDarray_ptr->fast_XY_to_Z(
            interior_posn.get(0),interior_posn.get(1));
         interior_Zs.push_back(curr_z);

// Compute Z value lying outside of contour edge.  If it's not too
// large, append onto exterior_Zs for subsequent ground plane finding
// purposes:

         threevector exterior_posn=curr_edge_posn+2*ds*r_hat;
         curr_z=DTED_ztwoDarray_ptr->fast_XY_to_Z(
            exterior_posn.get(0),exterior_posn.get(1));
         const double max_Z_ground=50;	// meters
         if (curr_z < max_Z_ground)
            exterior_Zs.push_back(curr_z);
      } // loop over index e labeling contour edges

// Calculate distribution for exterior Zs.  Then set building's
// constant bottom footprint altitude equal 10% cumulative distribution
// value:
      
      int n_output_bins=100;
      prob_distribution prob_groundZ(exterior_Zs,n_output_bins);
      prob_groundZ.writeprobdists(false);
      double Z_bottom=prob_groundZ.find_x_corresponding_to_pcum(0.10);
      cout << "Z_bottom = " << Z_bottom << endl;

// Calculate up to max_n_peaks values along top footprint
// contour corresponding to various rooftop heights:

      double peak_width=2;
      vector<double> peak_Zs=mathfunc::find_local_peaks(
         interior_Zs,peak_width,max_n_peaks);

//   for (int j=0; j<peak_Zs.size(); j++)
//   {
//      cout << "j = " << j << " peak_Z = " << peak_Zs[j] << endl;
//   }
   
      for (int i=0; i<interior_Zs.size(); i++)
      {
         double curr_Z=interior_Zs[i];
         vector<int> peakZ_index;
         vector<double> dZ;
         for (int j=0; j<peak_Zs.size(); j++)
         {
            double curr_dZ=fabs(curr_Z-peak_Zs[j]);
            dZ.push_back(curr_dZ);
            peakZ_index.push_back(j);
         }
         templatefunc::Quicksort(dZ,peakZ_index);
         double fitted_interior_Z=peak_Zs[peakZ_index.front()];
//      cout << "i = " << i 
//           << " Zorig = " << interior_Zs[i]
//           << " Zfitted = " << fitted_interior_Z << endl;
         interior_Zs[i]=fitted_interior_Z;
      } // loop over index i 

// Circularly median filter interior_Zs in order to eliminate
// transient fluctations in rooftop heights:

//      int window_size=11;
      int window_size=15;
      vector<double> filtered_interior_Zs=
         mathfunc::circular_median_filter(interior_Zs,window_size);

// Search for height discontinuities within median filtered interior Z
// values:

      int n_filtered_Zs=filtered_interior_Zs.size();
      vector<double> discontinuity_fracs;
      vector<twovector> height_discontinuities;
      for (int i=0; i<n_filtered_Zs; i++)
      {
//      cout << "i = " << i
//           << " filtered Z = " << filtered_interior_Zs[i] << endl;
         int i_next=modulo(i+1,n_filtered_Zs);
         double Z_curr=filtered_interior_Zs[i];
         double Z_next=filtered_interior_Zs[i_next];
         if (!nearly_equal(Z_curr,Z_next))
         {
            double curr_frac=double(i)/double(n_filtered_Zs);
            discontinuity_fracs.push_back(curr_frac);
            twovector curr_height_discontinuity(Z_curr,Z_next);
            height_discontinuities.push_back(curr_height_discontinuity);
//         cout << "frac = " << discontinuity_fracs.back() << endl;
//         cout << "height discontinuity: " << height_discontinuities.back() 
//              << endl;
         }
      } // loop over index i 

      bottom_contour=contour(&bottom_poly);
      for (int e=0; e<bottom_contour.get_nedges(); e++)
      {
         linesegment curr_edge=bottom_contour.get_edge(e);
         threevector curr_XY_corner=curr_edge.get_v1();
      
         double curr_frac=bottom_contour.frac_distance_along_contour(
            curr_XY_corner);
         discontinuity_fracs.push_back(curr_frac);
         int i=curr_frac*n_filtered_Zs;
         double Z_curr=filtered_interior_Zs[i];
         twovector curr_height_discontinuity(Z_curr,Z_curr);
         height_discontinuities.push_back(curr_height_discontinuity);

//      cout << "frac = " << discontinuity_fracs.back() << endl;
//      cout << "height discontinuity: " << height_discontinuities.back() 
//           << endl;
      } // loop over index b labeling manually selected XY polyline corners
   
// Sort STL vector frac while making corresponding changes to
// height_discontinuities:

      templatefunc::Quicksort(discontinuity_fracs,height_discontinuities);
   
      vector<threevector> rooftop_corners;
      for (int i=0; i<discontinuity_fracs.size(); i++)
      {
//      cout << "frac = " << discontinuity_fracs[i] << endl;
//      cout << "height discontinuity: " << height_discontinuities[i]
//           << endl;

         double curr_frac=discontinuity_fracs[i];
         threevector curr_corner=bottom_contour.edge_point(curr_frac);
         twovector curr_height_discontinuity(height_discontinuities[i]);
         double Z0=curr_height_discontinuity.get(0);
         double Z1=curr_height_discontinuity.get(1);
         if (nearly_equal(Z0,Z1))
         {
            curr_corner.put(2,Z0);
            rooftop_corners.push_back(curr_corner);
         }
         else
         {
            curr_corner.put(2,Z0);
            rooftop_corners.push_back(curr_corner);
            curr_corner.put(2,Z1);
            rooftop_corners.push_back(curr_corner);
         }
      } // loop over index i 

//      for (int i=0; i<rooftop_corners.size(); i++)
//      {
//         cout << "i = " << i
//              << " corner = " << rooftop_corners[i] << endl;
//      }

// Progressively cleave off sub-contours from rooftop_corners which
// have constant height values:

      vector<polyline> rooftop_subcontours=
         geometry_func::decompose_contour_into_subcontours(
            rooftop_corners);
      cout << "rooftop_subcontours.size() = " << rooftop_subcontours.size() 
           << endl;

      int n_polyhedra=rooftop_subcontours.size();
      for (int p=0; p<n_polyhedra; p++)
      {
         polyline top_footprint=rooftop_subcontours[p];

         bottom_footprint_XYZs.clear();
         double height;
         for (int v=0; v<top_footprint.get_n_vertices(); v++)
         {
            threevector curr_V=top_footprint.get_vertex(v);
            height=curr_V.get(2)-Z_bottom;
            curr_V.put(2,Z_bottom);
            bottom_footprint_XYZs.push_back(curr_V);
         } // loop over index v
      
         bottom_footprint=polyline(bottom_footprint_XYZs);

         polyhedron building_polyhedron;
         building_polyhedron.generate_prism_with_rectangular_faces(
            bottom_footprint,height);

         cout << "building_polyhedron = " << building_polyhedron << endl;

         filefunc::dircreate(OFF_subdir);

         string OFF_filename=OFF_subdir
            +"building_"+stringfunc::number_to_string(building_ID)
            +"_part_"+stringfunc::number_to_string(building_part_counter++)
            +".off";
         building_polyhedron.write_OFF_file(OFF_filename);

//      polyhedron reconstructed_polyhedron;
//      reconstructed_polyhedron.read_OFF_file(OFF_filename);
//      cout << "reconstructed polyhedron = " << reconstructed_polyhedron 
//           << endl;
      } // loop over index p labeling polyhedra building parts
      

   } // loop over index polyline_ID labeling manually derived bldg footprints

// ========================================================================== 

}
