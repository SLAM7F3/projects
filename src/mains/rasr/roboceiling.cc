// ==========================================================================
// Program ROBOCEILING reads in raw Hokoyu ladar points for individual
// scans collected within a room from an input text file.  It extracts
// gross shape information from ladar points located near the
// room's ceiling.  We assume the room's ceiling is well-represented
// by a flat plane (i.e. no domes, cathedral vaulted ceilings, etc).
// After RANSAC fitting a plane to the ceiling points, all ladar
// points are rotated and translated so that the floor is well
// represented by the z=0 plane.  ROBOCEILING then projects all
// z-leveled points into a z-plane.  The largest connected component
// in a binary, recursively filled twoDarray is found (in order to
// eliminate ladar points which lie outside the room from open
// doorways).  The convex hull for the largest connected component is
// calculated, and its dominant edge direction angle (modulo 90
// degrees) is determined.  Wall direction vectors what, lhat are
// found from the averaged edge direction angle, and extremal W & L
// coordinates are calculated.  A 2D bounding box is thus determined
// which fits snugly around the large connected component's convex hull.
// ROBOCEILING writes to output text files the bounding box parameters
// as well as raw ladar points translated so that the bbox center =
// (0,0).

// 		    roboceiling peter_no_wedge_out1.dat

// ==========================================================================
// Last updated on 12/2/10; 12/4/10; 2/10/11; 2/28/13
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "image/myimage.h"
#include "general/outputfuncs.h"
#include "image/recursivefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

#include "osg/osgGraphicals/AnimationController.h"
#include "image/binaryimagefuncs.h"
#include "image/connectfuncs.h"
#include "geometry/contour.h"
#include "image/drawfuncs.h"
#include "image/graphicsfuncs.h"
#include "datastructures/Hashtable.h"
#include "templates/mytemplates.h"
#include "numrec/nrfuncs.h"
#include "geometry/plane.h"
#include "math/prob_distribution.h"
#include "math/rotation.h"
#include "video/texture_rectangle.h"
#include "threeDgraphics/xyzpfuncs.h"

#include "kht/buffer_2d.h"
#include "kht/eigen.h"
#include "kht/kht.h"
#include "kht/linking.h"
#include "kht/peak_detection.h"
#include "kht/subdivision.h"
#include "kht/types.h"
#include "kht/voting.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   nrfunc::init_time_based_seed();

   string hokoyu_filename=argv[1];
   cout << "hokoyu_filename = " << hokoyu_filename << endl;

   threevector robo_coords;
   vector<threevector>* xyz_pnt_ptr=
      xyzpfunc::parse_Hokoyu_binary_datafile(hokoyu_filename,robo_coords);
   cout << "xyz_pnt_ptr->size() = " << xyz_pnt_ptr->size() << endl;

// Read in raw Hokoyu ladar data from individual scans conducted at
// fixed locations within a room from text files with one X,Y,Z
// triple written per input line:

   double xmin=POSITIVEINFINITY;
   double xmax=NEGATIVEINFINITY;
   double ymin=POSITIVEINFINITY;
   double ymax=NEGATIVEINFINITY;
   double zmin=POSITIVEINFINITY;
   double zmax=NEGATIVEINFINITY;

   vector<double> X,Y,Z;
   for (int i=0; i<xyz_pnt_ptr->size(); i++)
   {
      if (i%1000==0) cout << i/1000 << " ";
      threevector curr_xyz(xyz_pnt_ptr->at(i));
      X.push_back(curr_xyz.get(0));
      Y.push_back(curr_xyz.get(1));
      Z.push_back(curr_xyz.get(2));

      xmin=basic_math::min(xmin,X.back());
      xmax=basic_math::max(xmax,X.back());
      ymin=basic_math::min(ymin,Y.back());
      ymax=basic_math::max(ymax,Y.back());
      zmin=basic_math::min(zmin,Z.back());
      zmax=basic_math::max(ymax,Z.back());
   } // loop over index i labeling text lines within input robot XYZ file
   cout << endl;

// Compute Z distribution for input ladar points.  Define room
// height in terms of 2 and 98 percentiles of the Z distribution:

   prob_distribution zprob(Z,100,zmin);
   double z2=zprob.find_x_corresponding_to_pcum(0.02);
   double z5=zprob.find_x_corresponding_to_pcum(0.05);
   double z25=zprob.find_x_corresponding_to_pcum(0.25);
   double z50=zprob.find_x_corresponding_to_pcum(0.50);
   double z75=zprob.find_x_corresponding_to_pcum(0.75);
   double z95=zprob.find_x_corresponding_to_pcum(0.95);
   double z98=zprob.find_x_corresponding_to_pcum(0.98);
   double room_height=z98-z2;

   cout << "z2 = " << z5 << endl;
   cout << "z5 = " << z5 << endl;
   cout << "z25 = " << z25 << endl;
   cout << "z50 = " << z50 << endl;
   cout << "z75 = " << z75 << endl;
   cout << "z95 = " << z95 << endl << endl;
   cout << "z98 = " << z98 << endl << endl;
   cout << "room height = " << room_height << endl;

// Grid continuous XYZ points onto raster images:

   double dx=0.03;	// meters
   double dy=0.03;	// meters
   unsigned int mdim=(xmax-xmin)/dx;
   unsigned int ndim=(ymax-ymin)/dy;
   
   twoDarray* ztwoDarray_ptr=new twoDarray(mdim,ndim);
   ztwoDarray_ptr->init_coord_system(xmin,xmax,ymin,ymax);
   ztwoDarray_ptr->clear_values();

   twoDarray* binary_ztwoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   binary_ztwoDarray_ptr->clear_values();

// We focus upon extracting gross room shape information from ceiling
// points.  So fill binary image *ztwoDarray_ptr with unit values
// for just room voxels lying within 1/2 meter of the ceiling:

   unsigned int px,py;
   int n_tall_points=0;
   for (int i=0; i<X.size(); i++)
   {
      double curr_z=Z[i];
      if (curr_z > room_height-0.5)
      {
         ztwoDarray_ptr->point_to_pixel(X[i],Y[i],px,py);
         ztwoDarray_ptr->put(px,py,curr_z);
         binary_ztwoDarray_ptr->put(px,py,1);
         n_tall_points++;
      }
   }

   int binary_integral=0;
   for (int px=0; px<mdim; px++)
   {
      for (int py=0; py<ndim; py++)
      {
         binary_integral += binary_ztwoDarray_ptr->get(px,py);
      }
   }

//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;
//   cout << "mdim*ndim = " << mdim*ndim << endl;
//   cout << "X.size() = " << X.size() << endl;
//   cout << "n_tall_points = " << n_tall_points << endl;
//   cout << "binary integral = " << binary_integral << endl;
//   outputfunc::enter_continue_char();

// Write binary image to output PNG file via intermediate
// texture_rectangle:
   
   AnimationController* AnimationController_ptr=new AnimationController;

   int n_images=1;
   int n_channels=3;
   texture_rectangle* texture_rectangle_ptr=
      new texture_rectangle(mdim,ndim,n_images,n_channels,
      AnimationController_ptr);
   texture_rectangle_ptr->initialize_twoDarray_image(
      binary_ztwoDarray_ptr,n_channels);   
   string output_filename="raw_zimage.png";
   texture_rectangle_ptr->write_curr_frame(output_filename);

// Hokoyu ladar's vertical axis is generally not perfectly aligned
// with z_hat.  Room's ceiling may also not be a perfect z-plane.  So
// we first fit a plane to ladar's ceiling content.  We subsequently
// rotate the raw ladar cloud so that its ceiling plane does become
// parallel to z_hat.

// Perform RANSAC search for best-fit plane to ceiling points:

   const int max_iters=100;
   int iter=0;
   int max_n_ceiling_points=0;
   threevector currpoint;
   plane ceiling_plane;
   while (iter < max_iters)
   {
      vector<threevector> candidate_points;
      for (int p=0; p<3; p++)
      {
         bool genuine_ladar_point_flag=false;

         int px,py;
         while (!genuine_ladar_point_flag)
         {
            px=mdim*nrfunc::ran1();
            py=ndim*nrfunc::ran1();
            if (nearly_equal(binary_ztwoDarray_ptr->get(px,py),1))
            {
               genuine_ladar_point_flag=true;
            }
         }
         ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
         currpoint.put(2,ztwoDarray_ptr->get(px,py));
         candidate_points.push_back(currpoint);
//         cout << "p = " << p << " candidate_point = " << currpoint << endl;
      } // loop over index p labeling candidate plane points

// Make sure 3 candidate points lie are separated from each
// other in X & Y by some minimal distance:

      const double min_XY_separation=1;	// meters
      bool well_separated_flag=true;
      for (int p=0; p<3; p++)
      {
         int q=modulo(p+1,3);
         double curr_separation=(candidate_points[p]-candidate_points[q]).
            magnitude();
         if (curr_separation < min_XY_separation)
         {
            well_separated_flag=false;
            break;
         }
      } // loop over index p labeling candidate plane points
      
      if (!well_separated_flag) continue;
      
      plane curr_plane(candidate_points[0],candidate_points[1],
		       candidate_points[2]);

      int n_ceiling_points=0;
      const double max_distance_from_plane=0.02;	// meters
      for (int px=0; px<mdim; px++)
      {
         for (int py=0; py<ndim; py++)
         {
            if (nearly_equal(binary_ztwoDarray_ptr->get(px,py),0)) continue;
            ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
            currpoint.put(2,ztwoDarray_ptr->get(px,py));
            double curr_distance=fabs(curr_plane.signed_distance_from_plane(
               currpoint));
            if (curr_distance < max_distance_from_plane) n_ceiling_points++;
         } // loop over py
      } // loop over px
      
//      cout << "iter = " << iter
//           << " n_ceiling_points = " << n_ceiling_points
//           << " binary_integral = " << binary_integral
//           << " max_n_ceiling_points = " << max_n_ceiling_points << endl;

      if (n_ceiling_points > max_n_ceiling_points)
      {
         max_n_ceiling_points=n_ceiling_points;
         ceiling_plane=curr_plane;

//         cout << "max_n_ceiling_points = " << max_n_ceiling_points << endl;
//         cout << "binary_integral = " << binary_integral << endl;
//         cout << "ceiling_plane = " << ceiling_plane << endl;
      }
      iter++;
   } // loop over iter index

// Form rotation R which transforms ceiling plane's normal direction
// vector to z_hat.  Then rotate all points within raw cloud about
// ceiling plane's origin by R.  Translate rotated points in z_hat
// direction so that floor basically corresponds to z=0 plane:

   threevector n_hat(ceiling_plane.get_nhat());
   if (n_hat.get(2) < 0) n_hat=-n_hat;
   cout << "n_hat = " << n_hat << endl;
   rotation R;
   R=R.rotation_taking_u_to_v(n_hat,z_hat);
   cout << "R = " <<  R << endl;

   threevector ceiling_origin(ceiling_plane.get_origin());

   zmin=POSITIVEINFINITY;
   zmax=NEGATIVEINFINITY;
   for (unsigned int i=0; i<Z.size(); i++)
   {
      currpoint=ceiling_origin+R*(threevector(X[i],Y[i],Z[i])-ceiling_origin);
      X[i]=currpoint.get(0);
      Y[i]=currpoint.get(1);
      Z[i]=currpoint.get(2);
      zmin=basic_math::min(zmin,Z[i]);
      zmax=basic_math::max(zmax,Z[i]);
   }

   zprob=prob_distribution(Z,100,zmin);

   bool gzip_flag=false;
   zprob.write_density_dist(gzip_flag);

   int n_max_bin;
   double p_peak=zprob.peak_density_value(n_max_bin);
   cout << "n_max_bin = " << n_max_bin 
        << " p_peak = " << p_peak << endl;
   double z_peak=zprob.get_x(n_max_bin);
   cout << "z_peak = " << z_peak << endl;

   z2=zprob.find_x_corresponding_to_pcum(0.02);
   z5=zprob.find_x_corresponding_to_pcum(0.05);
   z25=zprob.find_x_corresponding_to_pcum(0.25);
   z50=zprob.find_x_corresponding_to_pcum(0.50);
   z75=zprob.find_x_corresponding_to_pcum(0.75);
   z95=zprob.find_x_corresponding_to_pcum(0.95);
   z98=zprob.find_x_corresponding_to_pcum(0.98);

   cout << "zmin = " << zmin << " zmax = " << zmax << endl;
   cout << "z2 = " << z5 << endl;
   cout << "z5 = " << z5 << endl;
   cout << "z25 = " << z25 << endl;
   cout << "z50 = " << z50 << endl;
   cout << "z75 = " << z75 << endl;
   cout << "z95 = " << z95 << endl << endl;
   cout << "z98 = " << z98 << endl << endl;

   double zfloor=z2;
   z_peak -= zfloor;
   cout << "Renormalized z_peak = " << z_peak << endl;

   room_height=z98-z2;
   for (unsigned int i=0; i<Z.size(); i++)
   {
      Z[i]=Z[i]-zfloor;
   }

// Refill *ztwoDarray_ptr and *binary_ztwoDarray_ptr with new,
// leveled points:

   ztwoDarray_ptr->clear_values();
   binary_ztwoDarray_ptr->clear_values();

// Fill binary image *ztwoDarray_ptr with unit values for those room
// voxels lying within 1/2 meter of the ceiling:

   for (unsigned int i=0; i<Z.size(); i++)
   {
      double curr_z=Z[i];

// Experiment with intentionally ignoring most ceiling points when
// trying to find single largest connected component

//      if (curr_z > room_height-0.5)
      if (curr_z < z_peak - 0.25 && curr_z > z_peak - 0.75)
      {
         ztwoDarray_ptr->point_to_pixel(X[i],Y[i],px,py);
         ztwoDarray_ptr->put(px,py,curr_z);
         binary_ztwoDarray_ptr->put(px,py,1);
      }
   }

//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;
//   cout << "mdim*ndim = " << mdim*ndim << endl;
//   cout << "X.size() = " << X.size() << endl;
//   cout << "n_tall_points = " << n_tall_points << endl;
//   cout << "binary integral = " << binary_integral << endl;
//   outputfunc::enter_continue_char();

// Write binary image to output PNG file via intermediate
// texture_rectangle:
   
   texture_rectangle_ptr->initialize_twoDarray_image(
      binary_ztwoDarray_ptr,n_channels);   
   output_filename="leveled_zimage.png";
   texture_rectangle_ptr->write_curr_frame(output_filename);

/*
   string leveled_filename="leveled_points.dat";
   ofstream outstream;
   filefunc::openfile(leveled_filename,outstream);

   for (int i=0; i<Z.size(); i++)
   {
      outstream << X[i] << "   "
                << Y[i] << "   "
                << Z[i] << endl;
   }
   filefunc::closefile(leveled_filename,outstream);
*/
   string UTMzone="";
   string tdp_filename="leveled.tdp";
   tdpfunc::write_relative_xyz_data(tdp_filename,X,Y,Z);

// Compute connected components of unit-valued pixels in
// *binary_ztwoDarray_ptr.  Then discard very small connected
// components:

   double zthreshold=0.5;
   int min_component_pixels=10;
   bool threshold_below_cutoff=true;
   Hashtable<linkedlist*>* connected_components_hashtable_ptr=
      connectfunc::generate_connected_hashtable(
         zthreshold,min_component_pixels,binary_ztwoDarray_ptr,
         threshold_below_cutoff);

   twoDarray* zconnected_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   bool set_component_intensities_less_than_unity=false;
   connectfunc::decode_connected_hashtable(
      connected_components_hashtable_ptr,zconnected_twoDarray_ptr,
      set_component_intensities_less_than_unity,0);
   delete connected_components_hashtable_ptr;

   binaryimagefunc::binary_threshold(0.5,zconnected_twoDarray_ptr,0,1);

// Perform recursive filling on *zconnected_twoDarray_ptr.  Eliminate
// small islands of black surrounded by oceans of white:

   int n_iters=5;
//   cout << "Enter niters for recursive filling:" << endl;
//   cin >> n_iters;
   recursivefunc::binary_fill(
      n_iters,0,mdim,0,ndim,0,1,zconnected_twoDarray_ptr);

// Keep single-largest connected component within binary image:

   connected_components_hashtable_ptr=
      connectfunc::generate_connected_hashtable(
         zthreshold,min_component_pixels,zconnected_twoDarray_ptr,
         threshold_below_cutoff);
   linkedlist* pixel_list_ptr=connectfunc::largest_connected_component(
      connected_components_hashtable_ptr);

   zconnected_twoDarray_ptr->clear_values();
   connectfunc::convert_pixel_list_to_binary_image(
      pixel_list_ptr,zconnected_twoDarray_ptr);

   texture_rectangle_ptr->initialize_twoDarray_image(
      zconnected_twoDarray_ptr,n_channels);   
   output_filename="zconnected_init.png";
   texture_rectangle_ptr->write_curr_frame(output_filename);

// Restore some unit-valued pixels from leveled *binary_ztwoDarray_ptr
// into *zconnected_twoDarray_ptr:

   connectfunc::fill_connected_component_from_original_image(
      binary_ztwoDarray_ptr,zconnected_twoDarray_ptr);

   texture_rectangle_ptr->initialize_twoDarray_image(
      zconnected_twoDarray_ptr,n_channels);   
   output_filename="zconnected_filled.png";
   texture_rectangle_ptr->write_curr_frame(output_filename);

/*
// Compute binary image gradient:

   double spatial_resolution=0.010; // meters
//   double spatial_resolution=0.015; // meters
   twoDarray* xderiv_twoDarray_ptr=new twoDarray(zconnected_twoDarray_ptr);
   twoDarray* yderiv_twoDarray_ptr=new twoDarray(zconnected_twoDarray_ptr);
   twoDarray* gradient_mag_twoDarray_ptr=new twoDarray(
	zconnected_twoDarray_ptr);
   imagefunc::compute_x_y_deriv_fields(
      spatial_resolution,zconnected_twoDarray_ptr,
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr);
   imagefunc::compute_gradient_magnitude_field(
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      gradient_mag_twoDarray_ptr);

   texture_rectangle_ptr->initialize_twoDarray_image(
      gradient_mag_twoDarray_ptr);   
   output_filename="gradient_mag.png";
   texture_rectangle_ptr->write_curr_frame(output_filename);
*/

// Compute convex hull of partially restored *zconnected_twoDarray_ptr:

   polygon* convex_hull_ptr=
      graphicsfunc::connected_region_convex_hull(zconnected_twoDarray_ptr);
//   cout << "convex hull = " << *convex_hull_ptr << endl;
   
   twoDarray* zhull_twoDarray_ptr=new twoDarray(zconnected_twoDarray_ptr);
   double intensity=0.5;
   drawfunc::draw_polygon(*convex_hull_ptr,intensity,zhull_twoDarray_ptr);
   texture_rectangle_ptr->initialize_twoDarray_image(
      zhull_twoDarray_ptr,n_channels);   
   output_filename="convex_hull.png";
   texture_rectangle_ptr->write_curr_frame(output_filename);

// Form convex hull contour.  Then consolidate any of its edges which
// are nearly parallel:

   contour convex_hull(convex_hull_ptr);
   convex_hull.initialize_edge_segments();

   double edge_angle_deviation=1.5*PI/180;
   contour consolidated_convex_hull=convex_hull.generate_consolidated_contour(
      edge_angle_deviation);
   
// After edge consolidation, compute edge angles modulo 90 degrees.
// Here we assume walls surrounding room are well-represented by an
// orthogonal countour:

   vector<double> edge_length,edge_angle;
   for (unsigned int e=0; e<consolidated_convex_hull.get_nedges(); e++)
   {
      linesegment curr_edge=consolidated_convex_hull.get_edge(e);
      edge_length.push_back(curr_edge.get_length());
      threevector ehat=curr_edge.get_ehat();
      double theta=atan2(ehat.get(1),ehat.get(0));
      edge_angle.push_back(basic_math::phase_to_canonical_interval(
         theta,0,PI/2));
//      cout << "e = " << e << " length = " << edge_length.back()
//           << " theta = " << theta*180/PI << endl;
   }
//   cout << endl;

// Sort in descending order edge angles by edge lengths.  Keep angles
// whose corresponding edge length exceeds a minimal distance threshold:

   templatefunc::Quicksort_descending(edge_length,edge_angle);

   const double min_edge_length=2;	// meters
   vector<double> long_edge_angle,long_edge_length;
   for (unsigned int e=0; e<edge_length.size(); e++)
   {
//      cout << "e = " << e << " length = " << edge_length[e]
//           << " modified theta = " << edge_angle[e]*180/PI << endl;
      if (edge_length[e] > min_edge_length)
      {
         long_edge_angle.push_back(edge_angle[e]);
         long_edge_length.push_back(edge_length[e]);
      }
   }
   
// Perform "RANSAC" type check to eliminate long edges of
// consolidated hull which may not correspond to wall directions:

   const double max_edge_angle_difference=3*PI/180;
   vector<int> n_similar_edge_angles;
   for (unsigned int l=0; l<long_edge_angle.size(); l++)
   {
      double curr_edge_angle=long_edge_angle[l];
      
      int n_votes=0;
      for (unsigned int m=0; m<long_edge_angle.size(); m++)
      {
         if (m==l) continue;
         double other_edge_angle=basic_math::phase_to_canonical_interval(
            long_edge_angle[m],curr_edge_angle-PI/4,curr_edge_angle+PI/4);
//         cout << "m = " << m
//              << " other_edge_angle = " << other_edge_angle*180/PI << endl;
         
         if (fabs(curr_edge_angle-other_edge_angle) < 
                  max_edge_angle_difference)
         {
            n_votes++;
         }
      } // loop over index m labeling long consolidated convex hull edges
      n_similar_edge_angles.push_back(n_votes);

      cout << "l = " << l 
           << " edge angle = " << curr_edge_angle*180/PI
           << " n_votes = " << n_similar_edge_angles.back()
           << endl;

   } // loop over index l labeling long consolidate convex hull edges
   
// Sort in descending order n_similar_edge_angles.  Take edge
// angle corresponding to largest number of votes as estimate for
// dominant wall direction angle:

   templatefunc::Quicksort_descending(
      n_similar_edge_angles,long_edge_angle,long_edge_length);

   double init_wall_angle=long_edge_angle[0];
   cout << "Initial wall direction angle estimate = " 
        << init_wall_angle*180/PI << endl;

   double numer,denom;
   numer=denom=0;
   const double max_delta_difference=4*PI/180;
   for (unsigned int l=0; l<long_edge_angle.size(); l++)
   {
      double curr_edge_angle=long_edge_angle[l];
      curr_edge_angle=basic_math::phase_to_canonical_interval(
         curr_edge_angle,init_wall_angle-PI/4,init_wall_angle+PI/4);

      double curr_edge_length=long_edge_length[l];
      double delta_angle=fabs(curr_edge_angle-init_wall_angle);
      cout << "l = " << l 
           << " curr_edge_angle = " << curr_edge_angle*180/PI
           << " delta_angle = " << delta_angle*180/PI << endl;
      
      if (delta_angle < max_delta_difference)
      {
         numer += curr_edge_angle*curr_edge_length;
         denom += curr_edge_length;
      }
   }
   double wall_angle=numer/denom;
//   wall_angle=basic_math::phase_to_canonical_interval(wall_angle,0,PI/2);
   cout << "Average wall angle weighted by long edge lengths = "
        << wall_angle*180/PI << endl;

// Compute what & lhat wall direction vectors from averaged wall
// angle.  Project all pixels in filled *zconnected_twoDarray_ptr onto
// W and L axes.  Then compute extremal W and L coordinates:

   threevector w_hat(cos(wall_angle),sin(wall_angle));
   threevector l_hat(-sin(wall_angle),cos(wall_angle));
   threevector center_point=zconnected_twoDarray_ptr->center_point();

   cout << "what = " << w_hat << endl;
   cout << "lhat = " << l_hat << endl;

   double min_w=POSITIVEINFINITY;
   double max_w=NEGATIVEINFINITY;
   double min_l=POSITIVEINFINITY;
   double max_l=NEGATIVEINFINITY;
   threevector curr_point;
   for (unsigned int px=0; px<mdim; px++)
   {
      for (unsigned int py=0; py<ndim; py++)
      {
         double curr_z=zconnected_twoDarray_ptr->get(px,py);
         if (curr_z < 0.9) continue;
         zconnected_twoDarray_ptr->pixel_to_threevector(px,py,curr_point);
         threevector rel_point=curr_point-center_point;
         double w=rel_point.dot(w_hat);
         double l=rel_point.dot(l_hat);
         min_w=basic_math::min(w,min_w);
         max_w=basic_math::max(w,max_w);
         min_l=basic_math::min(l,min_l);
         max_l=basic_math::max(l,max_l);
      } // loop over py index
   } // loop over px index
   
   cout << "min_w = " << min_w << " max_w = " << max_w << endl;
   cout << "min_l = " << min_l << " max_l = " << max_l << endl;
   double width=max_w-min_w;
   double length=max_l-min_l;
   cout << "width = " << width << " length = " << length << endl;

// Compute oriented bounding box which snugly encloses convex hull:

   vector<threevector> bbox_corners;
   bbox_corners.push_back(center_point+min_w*w_hat+min_l*l_hat);
   bbox_corners.push_back(center_point+max_w*w_hat+min_l*l_hat);
   bbox_corners.push_back(center_point+max_w*w_hat+max_l*l_hat);
   bbox_corners.push_back(center_point+min_w*w_hat+max_l*l_hat);
   polygon bbox(bbox_corners);
   threevector bbox_center=0.25*(
      bbox_corners[0]+bbox_corners[1]+bbox_corners[2]+bbox_corners[3]);

   drawfunc::draw_polygon(bbox,intensity,zconnected_twoDarray_ptr);

   texture_rectangle_ptr->initialize_twoDarray_image(
      zconnected_twoDarray_ptr,n_channels);   
   output_filename="bbox.png";
   texture_rectangle_ptr->write_curr_frame(output_filename);

// Export bounding box parameters to output text file:

   string bbox_filename="bbox.dat";
   ofstream outstream;
   filefunc::openfile(bbox_filename,outstream);
   outstream << "WallAngle   " << wall_angle*180/PI << endl;
   outstream << "Width   " << width << endl;
   outstream << "Length   " << length << endl;
   outstream << "BBoxCenter " << bbox_center.get(0) << "   "
             << bbox_center.get(1) << endl;
   outstream << "Corner0   " << bbox_corners[0].get(0)-bbox_center.get(0) 
             << "   " << bbox_corners[0].get(1)-bbox_center.get(1) << endl;
   outstream << "Corner1   " << bbox_corners[1].get(0)-bbox_center.get(0) 
             << "   " << bbox_corners[1].get(1)-bbox_center.get(1) << endl;
   outstream << "Corner2   " << bbox_corners[2].get(0)-bbox_center.get(0) 
             << "   " << bbox_corners[2].get(1)-bbox_center.get(1) << endl;
   outstream << "Corner3   " << bbox_corners[3].get(0)-bbox_center.get(0) 
             << "   " << bbox_corners[3].get(1)-bbox_center.get(1) << endl;
   filefunc::closefile(bbox_filename,outstream);

// Translate all points so that bbox center moves to (0,0).  Export
// translated points to output text file:

   string translated_filename="translated_points.dat";
   filefunc::openfile(translated_filename,outstream);
   for (unsigned int i=0; i<Z.size(); i++)
   {
      outstream << X[i]-bbox_center.get(0) << "   "
                << Y[i]-bbox_center.get(1) << "   "
                << Z[i] << endl;
   }
   filefunc::closefile(translated_filename,outstream);

/*
// Rotate raw X & Y points about bbox center by -wall_angle so that
// wall axes become aligned with xhat and yhat.  Then translate all
// points so that bbox center moves to (0,0):

   R=rotation(0,0,-wall_angle);
   cout << "R = " << R << endl;

   string registered_filename="registered_points.dat";
   filefunc::openfile(registered_filename,outstream);

   for (int i=0; i<X.size(); i++)
   {
      threevector init_posn(X[i],Y[i],Z[i]);
      threevector new_posn=R*(init_posn-bbox_center);
      X[i]=new_posn.get(0);
      Y[i]=new_posn.get(1);
      outstream << X[i] << "   "
                << Y[i] << "   "
                << Z[i] << endl;
   }
   filefunc::closefile(registered_filename,outstream);
*/

/*
   contour* hull_contour_ptr=new contour(convex_hull_ptr);
   double delta_s=0.05;	// meter
   hull_contour_ptr->regularize_vertices(delta_s);

   double znull=0;
   double delta_r=1.0*delta_s;
   int n_max_iters=100;
//   cout << "Enter n_max_iters:" << endl;
//   cin >> n_max_iters;

//   graphicsfunc::shrink_wrap_regularized_contour(
//      znull,*hull_contour_ptr,zconnected_twoDarray_ptr,
//      delta_r,n_max_iters);

   cout << "After shrink wrapping:" << endl;
   cout << "hull contour nvertices = " << hull_contour_ptr->get_nvertices()
        << endl;
   cout << "hull contour origin = " << hull_contour_ptr->get_origin()
        << endl;
   cout << "hull contour normal = " << hull_contour_ptr->get_normal() << endl;

   double avg_edge_orientation=hull_contour_ptr->average_edge_orientation();
   cout << "average edge_orientation angle = " << avg_edge_orientation*180/PI
        << endl;
   double median_edge_orientation=hull_contour_ptr->median_edge_orientation();
   cout << "median edge_orientation angle = " << median_edge_orientation*180/PI
        << endl;
//   outputfunc::enter_continue_char();

   double edge_angle_deviation=1*PI/180;
   double ds_min=0.2;	// meters
   contour realigned_contour=hull_contour_ptr->align_edges_with_sym_dirs(
      median_edge_orientation,PI/2,
      edge_angle_deviation,ds_min);

   vector<threevector> vertices;
   for (int n=0; n<hull_contour_ptr->get_nvertices(); n++)
//   for (int n=0; n<realigned_contour.get_nvertices(); n++)
   {
      pair<threevector,bool> p=hull_contour_ptr->get_vertex(n);
//      pair<threevector,bool> p=realigned_contour.get_vertex(n);
      vertices.push_back(p.first);
   }
   polygon hull_poly(vertices);

   double intensity=0.5;
//   drawfunc::draw_polygon(hull_poly,intensity,zconnected_twoDarray_ptr);

   delete ztwoDarray_ptr;

   output_filename="zimage.png";
   texture_rectangle_ptr->write_curr_frame(output_filename);
*/


/*
// Generate single-channel version of binary image:


   twoDarray* gradient_copy_twoDarray_ptr=new twoDarray(
      gradient_mag_twoDarray_ptr);

   texture_rectangle_ptr->fill_twoDarray_image(gradient_copy_twoDarray_ptr,1);

   lines_list_t lines;
   size_t cluster_min_size=100;
   double cluster_min_deviation=2;
   double delta_theta=0.1;
   double kernel_min_height=0.002;
   double n_sigmas=2;

   kht(lines,texture_rectangle_ptr->get_m_image_ptr(),
       gradient_copy_twoDarray_ptr->get_mdim(),
       gradient_copy_twoDarray_ptr->get_ndim(),
       cluster_min_size,cluster_min_deviation,delta_theta,
       kernel_min_height,n_sigmas);

   int line_counter=0;
   for (int i=0; i<lines.size(); i++)
   {
      line_t curr_line=lines[i];
      double rho=curr_line.rho;
      double theta=curr_line.theta;
      cout << "line counter = " << line_counter++
           << " rho = " << rho << " theta = " << theta << endl;
   }
*/

/*
   double intensity=0.5;
   drawfunc::draw_line(
      const linesegment& l,colorfunc::red,zconnected_twoDarray_ptr);
*/

   
//   delete texture_rectangle_ptr;
//   delete AnimationController_ptr;
}
