// Note added on 9/5/04: Oversight found within
// determine_subregion_rooftype().. If rooftop is declared to NOT be
// flat and if no spine is found, it should probably be classified as
// pyramidal.  As of 9/5/04, this method declares an oriented box in
// this category to have NO rooftop...

// ==========================================================================
// URBANFUNCS stand-alone methods
// ==========================================================================
// Last modified on 8/6/08; 10/3/09; 12/4/10; 12/20/11; 4/5/14
// ==========================================================================

#include <iostream>
#include <new>
#include "math/basic_math.h"
#include "urban/building.h"
#include "math/constants.h"
#include "math/constant_vectors.h"
#include "color/colorfuncs.h"
#include "datastructures/containerfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h" 
#include "geometry/linesegment.h"
#include "datastructures/Linkedlist.h"
#include "urban/oriented_box.h"
#include "geometry/parallelogram.h"
#include "plot/plotfuncs.h"
#include "geometry/polygon.h"
#include "math/prob_distribution.h"
#include "image/recursivefuncs.h"
#include "urban/roadpoint.h"
#include "urban/rooftop.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "threeDgraphics/threeDstring.h"
#include "urban/tree_cluster.h"
#include "urban/treefuncs.h"
#include "urban/urbanfuncs.h"
#include "urban/urbanimage.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

namespace urbanfunc
{

// ==========================================================================
// Rooftop abstraction methods
// ==========================================================================

// Method clean_rooftop_pixels takes in a linked list of rooftop
// pixels.  It searches for height outliers among these points.  This
// method subsequently destroys the original linked list and returns a
// new one in which height outliers have been excised.

   void clean_rooftop_pixels(linkedlist*& pixel_list_ptr)
      {
         unsigned int npixels=pixel_list_ptr->size();      
         double *x,*y,*z;
         new_clear_array(x,npixels);
         new_clear_array(y,npixels);
         new_clear_array(z,npixels);
   
         unsigned int pixel_number=0;
         for (Mynode<datapoint>* curr_pixel_ptr=pixel_list_ptr->
                 get_start_ptr(); curr_pixel_ptr != NULL; curr_pixel_ptr=
                 curr_pixel_ptr->get_nextptr())
         {
            x[pixel_number]=curr_pixel_ptr->get_data().get_var(2);
            y[pixel_number]=curr_pixel_ptr->get_data().get_var(3);
            z[pixel_number]=curr_pixel_ptr->get_data().get_func(0);
            pixel_number++;
         }

// Compute bounding heights for rooftop voxel distribution:

         double zlo,zhi;
         rooftop_height_limits(npixels,z,zlo,zhi);

// Delete original pixel list and regenerate it with height outlier
// points excised:

         delete pixel_list_ptr;
         pixel_list_ptr=new linkedlist;

         const int n_node_indep_vars=4;
         const int n_node_depend_vars=2;
         double var[n_node_indep_vars];
         double func_value[n_node_depend_vars];
         
         var[0]=var[1]=-1; // pixel coordinates are undetermined at this stage
         func_value[1]=featurefunc::building_sentinel_value;
         for (unsigned int i=0; i<pixel_number; i++)
         {
            if (z[i] > zlo && z[i] < zhi)
            {
               var[2]=x[i];
               var[3]=y[i];
               func_value[0]=z[i];
               pixel_list_ptr->append_node(
                  datapoint(n_node_indep_vars,n_node_depend_vars,
                            var,func_value));
            }
         } // loop over index i labeling all rooftop pixels
         delete [] x;
         delete [] y;
         delete [] z;
      }

// ---------------------------------------------------------------------
// Method isolate_and_clean_rooftop_pixels takes in a linked list
// containing rooftop voxels after their center-of-mass has been
// translated to (x,y)=(0,0).  This method establishes reasonable
// minimum and maximum limits (zhi and zlo) for z which bracket the
// bulk of the rooftop voxels, and it discards height outlier voxels
// from the rooftop set.  The cleaned voxels are returned within the
// dynamically generated arrays xnew[], ynew[], znew[] and pnew[].

   int isolate_and_clean_rooftop_pixels(
      linkedlist *translated_pixel_list_ptr,string xyzp_filename,
      double& zlo,double& zhi,
      double*& xnew,double*& ynew,double*& znew,double*& pnew)
      {
         unsigned int npixels=translated_pixel_list_ptr->size();
      
         double *x,*y,*z,*p;
         new_clear_array(x,npixels);
         new_clear_array(y,npixels);
         new_clear_array(z,npixels);
         new_clear_array(p,npixels);
   
         unsigned int pixel_number=0;
         for (Mynode<datapoint>* curr_pixel_ptr=translated_pixel_list_ptr->
                 get_start_ptr(); curr_pixel_ptr != NULL; curr_pixel_ptr=
                 curr_pixel_ptr->get_nextptr())
         {
            x[pixel_number]=curr_pixel_ptr->get_data().get_var(2);
            y[pixel_number]=curr_pixel_ptr->get_data().get_var(3);
            z[pixel_number]=curr_pixel_ptr->get_data().get_func(0);
            p[pixel_number]=featurefunc::building_sentinel_value;
            pixel_number++;
         }

// Compute bounding heights for rooftop voxel distribution:

         rooftop_height_limits(npixels,z,zlo,zhi);
//   cout << "zlo = " << zlo << " zhi = " << zhi << endl;

// Remove height outliers and form new set of x,y,z,p arrays:

         new_clear_array(xnew,npixels);
         new_clear_array(ynew,npixels);
         new_clear_array(znew,npixels);
         new_clear_array(pnew,npixels);
         int j=0;
         for (unsigned int i=0; i<pixel_number; i++)
         {
            if (z[i] > zlo && z[i] < zhi)
            {
               xnew[j]=x[i];
               ynew[j]=y[i];
               znew[j]=z[i];
               pnew[j]=p[i];
               j++;
            }
         }
         npixels=j;

         delete [] x;
         delete [] y;
         delete [] z;
         delete [] p;

//   ladarfunc::write_xyzp_data(npixels,xnew,ynew,znew,pnew,xyzp_filename,true);
//   filefunc::gunzip_file_if_gzipped(xyzp_filename);
//   const threevector fake_posn(Zero_vector);
//   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
//      xyzp_filename,fake_posn);
//   filefunc::gzip_file(xyzp_filename);         

// Add lo and hi z-planes into rooftop points xyzp file:

//   filefunc::gunzip_file_if_gzipped(xyzp_filename);
//   double x_extent=z2Darray_ptr->get_xhi()-z2Darray_ptr->get_xlo();
//   double y_extent=z2Darray_ptr->get_yhi()-z2Darray_ptr->get_ylo();
//   draw3Dfunc::draw_zplane(
//      zlo,0.25*x_extent,-0.25*x_extent,0.15*y_extent,-0.15*y_extent,
//      xyzp_filename,0.2);
//   draw3Dfunc::draw_zplane(
//      zhi,0.25*x_extent,-0.25*x_extent,0.15*y_extent,-0.15*y_extent,
//      xyzp_filename,0.3);
//   const threevector fake_posn(Zero_vector);
//   draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
//      xyzp_filename,fake_posn);
//   filefunc::gzip_file(xyzp_filename);         
         return npixels;
      }

// ---------------------------------------------------------------------
// Method rooftop_height_limits takes in an array of z values
// extracted from an individual building's rooftop voxels.  It
// computes the 2% and 98% points within the cumulative distribution
// for these height values.  These extremal values are returned in
// output parameters zlo and zhi.

   void rooftop_height_limits(
      unsigned int npixels,double z[],double& zlo,double& zhi)
      {
         prob_distribution prob_z(npixels,z,100);
         const double prob_lo=0.02;
         const double prob_hi=0.99;
         zlo=prob_z.find_x_corresponding_to_pcum(prob_lo);
         zhi=prob_z.find_x_corresponding_to_pcum(prob_hi);

//  double zdiff=zhi-zlo;
//  prob_z.set_xmin(basic_math::max(0,zlo-zdiff));
//  prob_z.set_xmax(zhi+zdiff);
//  prob_z.set_xtic(1);
//  prob_z.set_xsubtic(0.5);
//         prob_z.set_densityfilenamestr("./zdist.meta");
//         prob_z.set_densityfilenamestr(imagedir+"zdist.meta");
//         prob_z.set_xlabel("Z (meters)");
//         prob_z.write_density_dist();
      }

// ---------------------------------------------------------------------
// Method rooftop_symmetry_directions attempts to find the basic
// symmetry directions for a building's set of rooftop voxels assuming
// that the projection of the voxels in the xy plane (= ground plane)
// involves only right angles.  It takes in arrays xnew[], ynew[],
// znew[] and pnew[] containing rooftop voxel positions after their
// center-of-mass has been translated to (x,y)=(0,0).  This method
// rotates the rooftop voxels about the z-axis and computes their
// cross-sectional area in the x-z plane as a function of the
// azimuthal rotation angle.  Rooftops whose footprints involve only
// right angles exhibit a periodicity in this cross sectional area
// function, even if the rooftop's z-extent is quite minimal (i.e. if
// the rooftop is flat as a pancake)!  The machine identifies the
// angle for which the cross sectional area is minimal as a symmetry
// direction for the rooftop voxels, and it assumes that the
// orthongonal direction is also a symmetry axis.  This minimal angle
// is returned by this method.

   double rooftop_symmetry_directions(
      int n,unsigned int npixels,
      double xnew[],double ynew[],double znew[],double pnew[],
      twoDarray* binary_mask_twoDarray_ptr,double zlo,double zhi,
      string xyzp_filename)
      {
// Rotate rooftop points from (x,y,z) to (u,v,z) coordinates:

         const double theta_lo=0;
         const double theta_hi=2*PI;
         const double dtheta=1*PI/180;
         const unsigned int n_angular_bins=basic_math::round(
            (theta_hi-theta_lo)/dtheta);

         double theta[n_angular_bins];
         double raw_proj_area[n_angular_bins],
            filtered_proj_area[n_angular_bins];
         for (unsigned int i=0; i<n_angular_bins; i++)
         {
            theta[i]=theta_lo+i*dtheta;
            double cos_theta=cos(theta[i]);
            double sin_theta=sin(theta[i]);

            binary_mask_twoDarray_ptr->clear_values();
            for (unsigned int p=0; p<npixels; p++)
            {
               double u=cos_theta*xnew[p]+sin_theta*ynew[p];
//                double v=-sin_theta*xnew[p]+cos_theta*ynew[p];
               unsigned int px,py;
               if (binary_mask_twoDarray_ptr->point_to_pixel(u,znew[p],px,py))
               {
                  binary_mask_twoDarray_ptr->put(px,py,znew[p]);
               }
            }
//         writeimage("rotated_rooftop_"+stringfunc::number_to_string(n),
//                    binary_mask_twoDarray_ptr,false);
            const double dA=binary_mask_twoDarray_ptr->get_deltax()*
               binary_mask_twoDarray_ptr->get_deltay();
            raw_proj_area[i]=imagefunc::count_pixels_in_z_interval(
               zlo,zhi,binary_mask_twoDarray_ptr)*dA;
         } // loop over index i labeling rotation angle theta

//   delete [] xnew;
//   delete [] ynew;
//   delete [] znew;
//   delete [] pnew;

// Convolve raw projected area measurements with gaussian filter:
   
         filter_projected_rooftop_areas(
            n_angular_bins,dtheta,raw_proj_area,filtered_proj_area);

// Plot filtered projected rooftop area measurements vs azimuthal
// rotation angle.  Find angle min_theta for which projected area is
// minimal:

         string proj_area_filename="junk";
//            imagedir+"proj_area_"+stringfunc::number_to_string(n);
         double min_theta=minimal_projected_rooftop_area_orientation(
            n_angular_bins,theta,filtered_proj_area,proj_area_filename);

// Add rays emanating form (x,y,z)=(0,0,zhi) in directions of minimal
// and maximal projected areas to xyzp rooftop points file:

//   draw3Dfunc::draw_symmetry_directions(
//      zhi,min_theta,xyzp_filename,draw3Dfunc::annotation_value1);

         return min_theta;
      }

// ---------------------------------------------------------------------
// Method rooftop_spine_pixel_height_interval computes the
// distribution of rooftop heights contained within input array
// z_roof[].  It returns the 85% and 98% values of this distribution
// as the rooftop spine's minimum and maximum heights.  It also
// returns the number of rooftop spine pixels which exist within this
// height range.

   int rooftop_spine_pixel_height_interval(
      unsigned int npixels,double z_roof[],double& min_height,double& max_height)
      {
         prob_distribution prob(npixels,z_roof,30);
         min_height=prob.find_x_corresponding_to_pcum(0.85);
         max_height=prob.find_x_corresponding_to_pcum(0.98);
         int n_spine_pixels=0;
         for (unsigned int i=0; i<npixels; i++)
         {
            if (z_roof[i] > min_height && z_roof[i] < max_height)
               n_spine_pixels++;
         }
         return n_spine_pixels;
      }

   int rooftop_spine_pixel_height_interval(
      const vector<double>& subregion_height,
      double& min_height,double& max_height)
      {
         prob_distribution prob(subregion_height,30);
         min_height=prob.find_x_corresponding_to_pcum(0.85);
         max_height=prob.find_x_corresponding_to_pcum(0.98);
         int n_spine_pixels=0;
         for (unsigned int i=0; i<subregion_height.size(); i++)
         {
            if (subregion_height[i] > min_height && 
                subregion_height[i] < max_height) n_spine_pixels++;
         }
         return n_spine_pixels;
      }
   
// ---------------------------------------------------------------------
// Method rooftop_spine_COM takes in a set of rooftop pixels within
// input arrays x_roof[], y_roof[] and z_roof[].  It also takes in the
// height range for rooftop spine pixels within input parameters
// min_height and max_height.  This method returns the COM of these
// rooftop spine pixels.

   threevector rooftop_spine_COM(
      unsigned int npixels,double min_height,double max_height,
      double x_roof[],double y_roof[],
      double z_roof[],double p_roof[],twoDarray* ftwoDarray_ptr)
      {
         threevector rooftop_COM,Imax_hat;
         return rooftop_spine_COM(
            npixels,min_height,max_height,POSITIVEINFINITY,rooftop_COM,
            Imax_hat,x_roof,y_roof,z_roof,p_roof,ftwoDarray_ptr);
      }
   
   threevector rooftop_spine_COM(
      unsigned int npixels,double min_height,double max_height,
      double max_transverse_dist,const threevector& old_rooftop_COM,
      const threevector& Imax_hat,double x_roof[],double y_roof[],
      double z_roof[],double p_roof[],twoDarray* ftwoDarray_ptr)
      {
         generate_rooftop_spine_binary_image(
            npixels,min_height,x_roof,y_roof,z_roof,p_roof,
            max_transverse_dist,old_rooftop_COM,Imax_hat,ftwoDarray_ptr);
         threevector rooftop_COM;
         imagefunc::center_of_mass(ftwoDarray_ptr,rooftop_COM);
         return threevector(rooftop_COM.get(0),rooftop_COM.get(1),max_height);
      }

   threevector rooftop_spine_COM(
      double min_height,double max_height,
      const vector<threevector>& xy_translated_subregion_point,
      twoDarray* ftwoDarray_ptr)
      {
         threevector rooftop_COM,Imax_hat;
         return rooftop_spine_COM(
            min_height,max_height,POSITIVEINFINITY,rooftop_COM,Imax_hat,
            xy_translated_subregion_point,ftwoDarray_ptr);
      }

   threevector rooftop_spine_COM(
      double min_height,double max_height,double max_transverse_dist,
      const threevector& old_rooftop_COM,const threevector& Imax_hat,
      const vector<threevector>& xy_translated_subregion_point,
      twoDarray* ftwoDarray_ptr)
      {
         generate_rooftop_spine_binary_image(
            min_height,max_transverse_dist,
            xy_translated_subregion_point,old_rooftop_COM,Imax_hat,
            ftwoDarray_ptr);
         threevector rooftop_COM;
         imagefunc::center_of_mass(ftwoDarray_ptr,rooftop_COM);
         return threevector(rooftop_COM.get(0),rooftop_COM.get(1),max_height);
      }

// ---------------------------------------------------------------------
// Method compute_initial_rooftop_moi_ratio diagonalizes the
// moment-of-inertia matrix for the top 15% of the rooftop voxels
// contained within input arrays x_roof[], y_roof[] and z_roof[].  It
// returns the ratio of the maximum to minimum eigenvalues of the
// moment-of-inertia matrix.
   
   double compute_initial_rooftop_moi_ratio(
      unsigned int npixels,double x_roof[],double y_roof[],double z_roof[],
      double p_roof[],twoDarray* ftwoDarray_ptr,threevector& rooftop_COM)
      {
         double min_height,max_height,Imin,Imax;
         rooftop_spine_pixel_height_interval(
            npixels,z_roof,min_height,max_height);

         rooftop_COM=rooftop_spine_COM(
            npixels,min_height,max_height,x_roof,y_roof,z_roof,p_roof,
            ftwoDarray_ptr);
         imagefunc::moment_of_inertia(rooftop_COM,Imin,Imax,ftwoDarray_ptr);
         return Imax/Imin;
      }
   
   double compute_initial_rooftop_moi_ratio(
      vector<threevector>& xy_translated_subregion_point,
      vector<double>& subregion_height,twoDarray* ftwoDarray_ptr,
      threevector& rooftop_COM)
      {
         double min_height,max_height,Imin,Imax;
         rooftop_spine_pixel_height_interval(
            subregion_height,min_height,max_height);
         rooftop_COM=rooftop_spine_COM(
            min_height,max_height,xy_translated_subregion_point,
            ftwoDarray_ptr);
         imagefunc::moment_of_inertia(rooftop_COM,Imin,Imax,ftwoDarray_ptr);
         return Imax/Imin;
      }

// ---------------------------------------------------------------------
// Method pyramidal_rooftop_structure tests whether a non-flat rooftop
// posses a significant spine or not.  If the input ratio moi_ratio of
// a putative spine's maximum to minimum moment-of-inertia matrix
// eigenvalues is small, this method checks whether most of the top
// 15% rooftop pixels are spatially concentrated near its maximum
// point.  If so, this boolean method returns true and it returns the
// apex of the rooftop's pyramid within rooftop_COM.
   
   bool pyramidal_rooftop_structure(
      unsigned int npixels,double x_roof[],double y_roof[],double z_roof[],
      double p_roof[],string xyzp_filename,
      double moi_ratio,double rooftop_pitch_angle,
      const threevector& rooftop_COM)
      {
         bool rooftop_is_pyramidal=false;
         
         double min_height,max_height;
         int n_spine_pixels=rooftop_spine_pixel_height_interval(
            npixels,z_roof,min_height,max_height);

         const double max_moi_ratio=2.0;
         if (moi_ratio < max_moi_ratio)
         {
            double radius=3*(max_height-min_height)/tan(rooftop_pitch_angle);
            regular_polygon octagon(8,radius);
            octagon.absolute_position(
               threevector(rooftop_COM.get(0),rooftop_COM.get(1),0));

            unsigned int npixels_within_octagon=0;
            for (unsigned int i=0; i<npixels; i++)
            {
               if (z_roof[i] > min_height)
               {
                  if (octagon.point_inside_polygon(
                     threevector(x_roof[i],y_roof[i],0)))
                  {
                     npixels_within_octagon++;
//                     p_roof[i]=featurefunc::grass_sentinel_value;
                  }
               } // z_roof conditional
               z_roof[i]=0;
            } // loop over index i labeling all rooftop pixels
            
            double octagon_fill_frac=double(npixels_within_octagon)/
               double(n_spine_pixels);
//            cout << "npixels_within_octagon = " << npixels_within_octagon 
//                 << endl;
//            cout << "n_spine_pixels = " << n_spine_pixels << endl;
//            cout << "octagon fill fraction = " << octagon_fill_frac << endl;

            const double octagon_fill_frac_threshold=0.8;
            rooftop_is_pyramidal=
               (octagon_fill_frac > octagon_fill_frac_threshold);

//            ladarfunc::write_xyzp_data(
//               npixels,x_roof,y_roof,z_roof,p_roof,xyzp_filename,true);
//            filefunc::gunzip_file_if_gzipped(xyzp_filename);
//            const threevector fake_posn(Zero_vector);
//            draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
//               xyzp_filename,fake_posn);
//            draw3Dfunc::draw_polygon(
//               octagon,xyzp_filename,draw3Dfunc::annotaton_value2);
//            filefunc::gzip_file(xyzp_filename);         

         }
         return rooftop_is_pyramidal;
      }

   bool pyramidal_rooftop_structure(
      double moi_ratio,double rooftop_pitch_angle,
      const vector<threevector>& xy_translated_subregion_point,
      const vector<double>& subregion_height,const threevector& rooftop_COM)
      {
         bool rooftop_is_pyramidal=false;
         
         double min_height,max_height;
         int n_spine_pixels=rooftop_spine_pixel_height_interval(
            subregion_height,min_height,max_height);

         const double max_moi_ratio=2.0;
         if (moi_ratio < max_moi_ratio)
         {
            double radius=3*(max_height-min_height)/
               tan(rooftop_pitch_angle);
            regular_polygon octagon(8,radius);
            octagon.absolute_position(
               threevector(rooftop_COM.get(0),rooftop_COM.get(1),0));

            unsigned int npixels_within_octagon=0;
            for (unsigned int i=0; i<xy_translated_subregion_point.size(); 
                 i++)
            {
               if (subregion_height[i] > min_height)
               {
                  if (octagon.point_inside_polygon(
                     threevector(xy_translated_subregion_point[i].get(0),
                              xy_translated_subregion_point[i].get(1),0)))
                     npixels_within_octagon++;
               } 
//               z_roof[i]=0;
            } // loop over index i labeling all rooftop pixels
            
            double octagon_fill_frac=double(npixels_within_octagon)/
               double(n_spine_pixels);

            const double octagon_fill_frac_threshold=0.8;
            rooftop_is_pyramidal=
               (octagon_fill_frac > octagon_fill_frac_threshold);
         }
         return rooftop_is_pyramidal;
      }

// ---------------------------------------------------------------------
// Method rooftop_spine_extraction takes in rooftop pixel information
// within input arrays x_roof[], y_roof[], z_roof[] and p_roof[].  It
// first distills the tallest 15% of these rooftop pixels and forms
// their projected x-y image.  It subsequently computes the x-y COM of
// these tall pixels as well as their moments-of-inertia with respect
// to this COM.  If the ratio of the maximum to minimum moments
// exceeds some critical value, the x-y distribution of the tallest
// rooftop pixels is close to linear.  In this case, we recompute the
// COM and moments of inertia using just those points which are
// located relatively close the initial estimate for the spine
// symmetry axis.  A distribution for rooftop spine points along the
// refined minimum moment of inertia direction is then computed, and
// the 2% and 98% points of this distribution are used to fix the
// length of the rooftop spine.

   linesegment* rooftop_spine_extraction(
      unsigned int npixels,double x_roof[],double y_roof[],double z_roof[],
      double p_roof[],threevector& rooftop_COM,
      string xyzp_filename,twoDarray* ftwoDarray_ptr)
      {
         double min_height,max_height;
         rooftop_spine_pixel_height_interval(
            npixels,z_roof,min_height,max_height);

         double Imin,Imax;
         threevector Imin_hat,Imax_hat;
         imagefunc::moment_of_inertia(
            rooftop_COM,Imin,Imax,Imin_hat,Imax_hat,ftwoDarray_ptr);

//         cout << "Initial estimates:" << endl;
//         cout << "COM = " << COM << endl;
//         cout << "Imax/Imin = " << Imax/Imin << endl;
//         cout << "Imin_hat = " << Imin_hat << " Imax_hat = " << Imax_hat
//              << endl;

         const double min_moi_ratio1=3.0;
         const double min_moi_ratio2=6.0;
         threevector Vmin,Vmax;
         linesegment* spine_ptr=NULL;

         if (Imax/Imin > min_moi_ratio1)
         {

// Recompute spine direction excluding those rooftop voxels which lie
// more than 0.5 meter away from our initial estimate for Imin_hat:

            generate_rooftop_spine_binary_image(
               npixels,min_height,x_roof,y_roof,z_roof,p_roof,
               0.5,rooftop_COM,Imax_hat,ftwoDarray_ptr);
            imagefunc::center_of_mass(ftwoDarray_ptr,rooftop_COM);
            rooftop_COM=threevector(rooftop_COM.get(0),rooftop_COM.get(1),
                                 max_height);

            imagefunc::moment_of_inertia(
               rooftop_COM,Imin,Imax,Imin_hat,Imax_hat,ftwoDarray_ptr);

            if (Imax/Imin > min_moi_ratio2)
            {
               
//            cout << "Refined estimates:" << endl;
//            cout << "COM = " << COM << endl;
//            cout << "Imax/Imin = " << Imax/Imin << endl;
//            cout << "Imin_hat = " << Imin_hat << " Imax_hat = " << Imax_hat
//                 << endl;

               int j=0;
               double* r=new double[npixels];
               for (unsigned int i=0; i<npixels; i++)
               {
                  if (z_roof[i] > min_height)
                  {
                     double curr_dx=x_roof[i]-rooftop_COM.get(0);
                     double curr_dy=y_roof[i]-rooftop_COM.get(1);
                     threevector curr_dr(curr_dx,curr_dy);
                     r[j++]=curr_dr.dot(Imin_hat);
                  } 
               } // loop over index i labeling rooftop pixels
               prob_distribution prob_r(j,r,30);
               delete [] r;
               double r_min=prob_r.find_x_corresponding_to_pcum(0.02);
               double r_max=prob_r.find_x_corresponding_to_pcum(0.98);
               Vmin=rooftop_COM+Imin_hat*r_min;
               Vmax=rooftop_COM+Imin_hat*r_max;
               spine_ptr=new linesegment(Vmin,Vmax);
            } // Imax/Imin > min_moi_ratio2 conditional
         } // Imax/Imin > min_moi_ratio1 conditional

//  ladarfunc::write_xyzp_data(
//  npixels,x_roof,y_roof,z_roof,p_roof,xyzp_filename,true);
//  filefunc::gunzip_file_if_gzipped(xyzp_filename);
//  const threevector fake_posn(Zero_vector);
//  draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
//  xyzp_filename,fake_posn);
//  draw3Dfunc::draw_tiny_cube(
//  COM,xyzp_filename,draw3Dfunc::annotaton_value2);
//  draw3Dfunc::draw_line(
//  Vmin,Vmax,xyzp_filename,draw3Dfunc::annotaton_value2);
//  filefunc::gzip_file(xyzp_filename);         
         return spine_ptr;
      }

   linesegment* rooftop_spine_extraction(
      threevector& rooftop_COM,
      const vector<threevector>& xy_translated_subregion_point,
      const vector<double>& subregion_height,twoDarray* ftwoDarray_ptr)
      {
         double min_height,max_height;
         rooftop_spine_pixel_height_interval(
            subregion_height,min_height,max_height);
//         cout << "inside  urbanfunc::rooftop_spine_extraction()" << endl;
//         cout << "min_height = " << min_height << " max_height = " 
//              << max_height << endl;

         double Imin,Imax;
         threevector Imin_hat,Imax_hat;
         imagefunc::moment_of_inertia(
            rooftop_COM,Imin,Imax,Imin_hat,Imax_hat,ftwoDarray_ptr);

//         cout << "Initial estimates:" << endl;
//         cout << "COM = " << COM << endl;
//         cout << "Imax/Imin = " << Imax/Imin << endl;
//         cout << "Imin_hat = " << Imin_hat << " Imax_hat = " << Imax_hat
//              << endl;

         const double min_moi_ratio1=3.0;
         const double min_moi_ratio2=6.0;
         threevector Vmin,Vmax;
         linesegment* spine_ptr=NULL;

         if (Imax/Imin > min_moi_ratio1)
         {

// Recompute spine direction excluding those rooftop voxels which lie
// more than 0.5 meter away from our initial estimate for Imin_hat:

            generate_rooftop_spine_binary_image(
               min_height,0.5,xy_translated_subregion_point,
               rooftop_COM,Imax_hat,ftwoDarray_ptr);
            imagefunc::center_of_mass(ftwoDarray_ptr,rooftop_COM);
            rooftop_COM=threevector(
               rooftop_COM.get(0),rooftop_COM.get(1),max_height);
            imagefunc::moment_of_inertia(
               rooftop_COM,Imin,Imax,Imin_hat,Imax_hat,ftwoDarray_ptr);

            if (Imax/Imin > min_moi_ratio2)
            {
               
//            cout << "Refined estimates:" << endl;
//            cout << "COM = " << COM << endl;
//            cout << "Imax/Imin = " << Imax/Imin << endl;
//            cout << "Imin_hat = " << Imin_hat << " Imax_hat = " << Imax_hat
//                 << endl;

               int j=0;
               double* r=new double[subregion_height.size()];
               for (unsigned int i=0; i<subregion_height.size(); i++)
               {
                  if (subregion_height[i] > min_height)
                  {
                     double curr_dx=xy_translated_subregion_point[i].get(0)
                        -rooftop_COM.get(0);
                     double curr_dy=xy_translated_subregion_point[i].get(1)
                        -rooftop_COM.get(1);
                     threevector curr_dr(curr_dx,curr_dy);
                     r[j++]=curr_dr.dot(Imin_hat);
                  } 
               } // loop over index i labeling rooftop pixels
               prob_distribution prob_r(j,r,30);
               delete [] r;
               double r_min=prob_r.find_x_corresponding_to_pcum(0.02);
               double r_max=prob_r.find_x_corresponding_to_pcum(0.98);
               Vmin=rooftop_COM+Imin_hat*r_min;
               Vmax=rooftop_COM+Imin_hat*r_max;
               spine_ptr=new linesegment(Vmin,Vmax);
            } // Imax/Imin > min_moi_ratio2 conditional
         } // Imax/Imin > min_moi_ratio1 conditional
         return spine_ptr;
      }

// ---------------------------------------------------------------------
// Method generate_rooftop_spine_image takes in rooftop pixel
// information within input arrays x_roof[], y_roof[], z_roof[] and
// p_roof[].  It also takes in minimum height and maximum transverse
// distance threshold parameters.  It generates a binary twoDarray
// containing those rooftop pixels whose z values exceed the minimum
// height and whose transverse distances in the Imax_hat direction are
// less than the max transverse threshold.  It also returns the number
// of such tall rooftop pixels.

   void generate_rooftop_spine_binary_image(
      unsigned int npixels,double min_height,
      double x_roof[],double y_roof[],double z_roof[],double p_roof[],
      double max_transverse_dist,const threevector& COM,
      const threevector& Imax_hat,twoDarray *ftwoDarray_ptr)
      {
         ftwoDarray_ptr->clear_values();
         for (unsigned int i=0; i<npixels; i++)
         {
            if (z_roof[i] > min_height)
            {
               double curr_dx=x_roof[i]-COM.get(0);
               double curr_dy=y_roof[i]-COM.get(1);
               threevector curr_dr(curr_dx,curr_dy);
               if (curr_dr.dot(Imax_hat) < max_transverse_dist)
               {
                  p_roof[i]=draw3Dfunc::annotation_value1;
                  unsigned int px,py;
                  ftwoDarray_ptr->point_to_pixel(x_roof[i],y_roof[i],px,py);
                  ftwoDarray_ptr->put(px,py,1);
               }
            }
         } // loop over index i over rooftop pixels
      }

   void generate_rooftop_spine_binary_image(
      double min_height,double max_transverse_dist,
      const vector<threevector>& xy_translated_subregion_point,
      const threevector& COM,const threevector& Imax_hat,twoDarray* ftwoDarray_ptr)
      {
         ftwoDarray_ptr->clear_values();
         for (unsigned int i=0; i<xy_translated_subregion_point.size(); i++)
         {
            if (xy_translated_subregion_point[i].get(2) > min_height)
            {
               double x=xy_translated_subregion_point[i].get(0);
               double y=xy_translated_subregion_point[i].get(1);
               double curr_dx=x-COM.get(0);
               double curr_dy=y-COM.get(0);
               threevector curr_dr(curr_dx,curr_dy);
               if (curr_dr.dot(Imax_hat) < max_transverse_dist)
               {
                  unsigned int px,py;
                  ftwoDarray_ptr->point_to_pixel(x,y,px,py);
                  ftwoDarray_ptr->put(px,py,1);
               }
            }
         } // loop over index i over rooftop pixels
      }

// ---------------------------------------------------------------------
// Method plot_projected_rooftop_area generates metafile output
// displaying projected rooftop point cloud area in the u-z plane as a
// function of the plane's azimuthal rotation angle about the z axis.
// It also indicates the angle for which the projected area is minimal
// within the metafile.

   void plot_projected_rooftop_area(
      string filename,double min_theta,linkedlist& proj_area_list)
      {

// 7/8/05: Need to completely redo this method, for we have eliminated
// metafile pointer within linked list class...

/*
// Plot projected area versus rotation angle:

         containerfunc::find_max_min_func_values(&proj_area_list);
         proj_area_list.get_metafile_ptr()->set_title(
            "Projected rooftop point cloud area in uz plane");
         proj_area_list.get_metafile_ptr()->set_labels(
            "Azimuthal rotation angle about z axis (degs)",
            "Projected rooftop area (meters^+2^n)");
         proj_area_list.get_metafile_ptr()->set_filename(filename);
         proj_area_list.get_metafile_ptr()->set_xbounds(0,360);
         proj_area_list.get_metafile_ptr()->set_xtics(30,15);
         proj_area_list.get_metafile_ptr()->set_ybounds(
            0,proj_area_list.get_fmax()+1);
         proj_area_list.get_metafile_ptr()->set_ytic(
            trunclog(proj_area_list.get_metafile_ptr()->get_ymax()-
                     proj_area_list.get_metafile_ptr()->get_ymin()));
         proj_area_list.get_metafile_ptr()->set_ysubtic(
            0.5*proj_area_list.get_metafile_ptr()->get_ytic());
         proj_area_list.get_metafile_ptr()->add_extraline(
            "curve color red thick 3 "
            +stringfunc::number_to_string(min_theta*180/PI)+" 0 "
            +stringfunc::number_to_string(min_theta*180/PI)+" 10000");
         plotfunc::writelist(proj_area_list);
*/

      }

// ---------------------------------------------------------------------
// Method minimal_projected_rooftop_area_orientation takes in an array
// of filtered projected rooftop areas.  It conducts a brute force
// search for the azimuthal angle for which the projected area is
// minimal.  It returns this minimal angle.

   double minimal_projected_rooftop_area_orientation(
      unsigned int n_angular_bins,double theta[],double filtered_proj_area[],
      string proj_area_filename)
      {
         double min_theta=0;
         double min_proj_area=POSITIVEINFINITY;
         linkedlist proj_area_list;
//         linkedlist proj_area_list(true);

         for (unsigned int i=0; i<n_angular_bins; i++)
         {
            double curr_proj_area=filtered_proj_area[i]+
               filtered_proj_area[modulo(i+n_angular_bins/2,n_angular_bins)];
            if (curr_proj_area < min_proj_area)
            {
               min_proj_area=curr_proj_area;
               min_theta=theta[i];
            }
            min_proj_area=basic_math::min(min_proj_area,curr_proj_area);
            proj_area_list.append_node(
               datapoint(theta[i]*180/PI,curr_proj_area));
         }
//         cout << "min_theta = " << min_theta*180/PI << endl;

// Plot projected area versus rotation angle:

//         plot_projected_rooftop_area(
//            proj_area_filename,min_theta,proj_area_list);

         return min_theta;
      }

// ---------------------------------------------------------------------
// Method filter_projected_rooftop_areas convolves raw projected area
// measurements with a gaussian filter.

   void filter_projected_rooftop_areas(
      unsigned int n_angular_bins,double dtheta,
      double raw_proj_area[],double filtered_proj_area[])
      {
         double sigma=2*PI/180;		// rads
         double h[n_angular_bins];
         filterfunc::gaussian_filter(n_angular_bins,dtheta,sigma,h);
         filterfunc::brute_force_filter(
            n_angular_bins,n_angular_bins,raw_proj_area,h,filtered_proj_area,
            true);
      }

// ---------------------------------------------------------------------
// Method align_rooftop_and_building_parallelepiped takes in a rooftop
// spine linesegment as well as a building's oriented box.  It first
// determines which box face's normal points most closely in the
// direction of the rooftop spine.  It next computes the azimuthal
// angular discrepancy delta_theta between the building's and spine's
// orientations.  This method forms a weighted linear combination of
// the two orientations and resets the box and linesegment alignments
// to match this combination.

   void align_rooftop_and_building_parallelepiped(
      linesegment* spine_ptr,oriented_box* box_3D_ptr)
      {
         double max_dotproduct=NEGATIVEINFINITY;
         threevector closest_normal;
         for (unsigned int i=0; i<4; i++)
         {
            polygon curr_side_face=box_3D_ptr->get_sideface(i);
            threevector curr_normal=curr_side_face.get_normal();
            double dotproduct=curr_normal.dot(spine_ptr->get_ehat());
            if (dotproduct > max_dotproduct)
            {
               max_dotproduct=dotproduct;
               closest_normal=curr_normal;
            }
         }

         double delta_theta_mag=acos(max_dotproduct);
         double delta_theta_sgn=sgn(z_hat.dot(spine_ptr->get_ehat().cross(
            closest_normal)));
         double delta_theta=delta_theta_sgn*delta_theta_mag;

// Weight factor favoring building's orientation over rooftop spine's:

         double w=0.333; 
         spine_ptr->rotate(spine_ptr->get_midpoint(0),0,0,(1-w)*delta_theta);
         box_3D_ptr->rotate(
            box_3D_ptr->get_center().get_pnt(),0,0,-w*delta_theta);
      }

   void align_box_and_rooftop_spine_orientations(
      oriented_box* oriented_box_ptr)
      {
         linesegment* spine_ptr=oriented_box_ptr->get_roof_ptr()->
            get_spine_ptr();
         if (spine_ptr != NULL)
         {
            double max_dotproduct=NEGATIVEINFINITY;
            threevector closest_normal;
            
            for (unsigned int i=0; i<4; i++)
            {
               polygon curr_side_face(oriented_box_ptr->get_sideface(i));
               threevector curr_normal(curr_side_face.get_normal());
               double dotproduct=curr_normal.dot(spine_ptr->get_ehat());
               if (dotproduct > max_dotproduct)
               {
                  max_dotproduct=dotproduct;
                  closest_normal=curr_normal;
               }
            }

            double delta_theta_mag=acos(max_dotproduct);
            double delta_theta_sgn=sgn(z_hat.dot(spine_ptr->get_ehat().cross(
               closest_normal)));
            double delta_theta=delta_theta_sgn*delta_theta_mag;

// Weight factor favoring building's orientation over rooftop spine's:

            double w=0.333; 

// FAKE FAKE: As of 6:47 am on Sat July 24, we only rotate the spine
// and not the building at all...

            w=0;
            spine_ptr->rotate(
               spine_ptr->get_midpoint(0),0,0,(1-w)*delta_theta);
            oriented_box_ptr->rotate(
               oriented_box_ptr->get_center().get_pnt(),0,0,-w*delta_theta);
         } // spine_ptr != NULL conditional
      }

   void align_bldg_and_all_rooftop_spine_orientations(
      Linkedlist<oriented_box*>* box_list_ptr)
      {
         for (Mynode<oriented_box*>* box_node_ptr=box_list_ptr->
                 get_start_ptr(); box_node_ptr != NULL; box_node_ptr=
                 box_node_ptr->get_nextptr())
         {
            oriented_box* oriented_box_ptr=box_node_ptr->get_data();
            align_box_and_rooftop_spine_orientations(oriented_box_ptr);
         } // loop over all nodes within *box_list_ptr
      }

// ==========================================================================
// Building wireframe model methods
// ==========================================================================

// Method construct_building_wireframe_model takes in building *b_ptr.
// It first attempts to bracket its rooftop subregions' voxels within
// distinct height ranges.  It next tries to determine the shape
// (flat, pyramidal or spined) for each rooftop subregion if the
// subregion's area is appreciable compared to the overall rooftop
// footprint's area.  This method next instantiates oriented 3D boxes
// for each subregion based upon subregion rooftop height information.
// Finally, it adjusts each oriented 3D box so that its bottom face
// coincides with the average ground height in its immediate vicinity.

   void construct_building_wireframe_model(
      building* b_ptr,twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr,
      twoDarray* binary_mask_twoDarray_ptr)
      {
         linkedlist* pixel_list_ptr=b_ptr->get_pixel_list_ptr();
         Linkedlist<contour*>* subcontour_list_ptr=b_ptr->
            get_subcontour_list_ptr();
         Linkedlist<oriented_box*>* box_list_ptr=
            new Linkedlist<oriented_box*>;

// First remove height outliers from rooftop pixel list:

         clean_rooftop_pixels(pixel_list_ptr);
         threevector cleaned_COM(connectfunc::pixel_list_COM(
            pixel_list_ptr,ztwoDarray_ptr,true,false));
         
// Decompose rooftop pixels according to the subregion to which they
// belong:
         
         unsigned int n_subregions=subcontour_list_ptr->size();
         vector<parallelogram> p;
         vector<threevector> xy_translated_subregion_point[n_subregions];
         vector<double> subregion_height[n_subregions];
         decompose_rooftop_pixels(
            n_subregions,cleaned_COM.xy_projection(),
            subcontour_list_ptr,pixel_list_ptr,p,
            xy_translated_subregion_point,subregion_height);

// From pixel height distributions for each parallelogram region,
// compute lower and upper bounds zlo & zhi on rooftop height.
// Subsequently generate an oriented box based upon each parallelogram
// and extrude upwards by zlo:
         
         const double prob_lo=0.02;
         const double prob_hi=0.99;

         int n_grnd_measurements=0;
         double ground_height_sum=0;

         for (unsigned int n=0; n<n_subregions; n++)
         {
            prob_distribution prob_z(subregion_height[n],100);
            double zlo=prob_z.find_x_corresponding_to_pcum(prob_lo);
            double zhi=prob_z.find_x_corresponding_to_pcum(prob_hi);
//            cout << "Subregion n = " << n << " zlo = " << zlo
//                 << " zhi = " << zhi << endl;
            oriented_box* oriented_box_ptr=new oriented_box(p[n],z_hat,zlo);

// Don't bother to assign any nontrivial rooftype to subregions which
// are small compared to the entire building's footprint:

            double region_area=oriented_box_ptr->get_bottomface().compute_area();
            double area_ratio=region_area/b_ptr->get_footprint_area();
//            cout << "Subregion n = " << n << " area = " << region_area
//                 << " area_ratio = " << area_ratio << endl;
            const double min_region_area=100;	// meter**2
            const double min_area_ratio=0.40;
            if (region_area > min_region_area || area_ratio > min_area_ratio)
               determine_subregion_rooftype(
                  zhi,xy_translated_subregion_point[n],subregion_height[n],
                  binary_mask_twoDarray_ptr,oriented_box_ptr);
//            cout << "*oriented_box_ptr = " << *oriented_box_ptr << endl;

            double avg_ground_height_in_subcontour_vicinity=
               refine_ground_height_estimate(
                  oriented_box_ptr,ztwoDarray_ptr,features_twoDarray_ptr);
            if (!nearly_equal(avg_ground_height_in_subcontour_vicinity,0))
            {
               n_grnd_measurements++;
               ground_height_sum += 
                  avg_ground_height_in_subcontour_vicinity;
            }
            box_list_ptr->append_node(oriented_box_ptr);
         } // loop over index n labeling subregions

// Translate and rescale 3D oriented boxes so that their bottom face
// heights equal the average surrounding ground height while their top
// face heights remain unchanged:

          adjust_oriented_boxes_relative_to_ground(
             n_grnd_measurements,ground_height_sum,box_list_ptr);

          align_bldg_and_all_rooftop_spine_orientations(box_list_ptr);

         b_ptr->set_box_list_ptr(box_list_ptr);         
      }
      
// ---------------------------------------------------------------------
// Method decompose_rooftop_pixels takes in the subcontour linked list
// along with an STL vector for parallelograms and pre-allocated
// arrays of STL vectors for pixel points and pixel heights.  This
// method fills the STL vectors with information corresponding to each
// of the subregions defined by the subcontours.

   void decompose_rooftop_pixels(
      int n_subregions,const threevector& COM_xy,
      Linkedlist<contour*> const *subcontour_list_ptr,
      linkedlist const *pixel_list_ptr,vector<parallelogram>& p,
      vector<threevector> xy_translated_subregion_point[],
      vector<double> subregion_height[])
      {

// Construct parallelograms corresponding to rectangles contained
// within linked list of building footprint subcontours:

         int n=0;
         for (Mynode<contour*> const *subcontour_node_ptr=
                 subcontour_list_ptr->
                 get_start_ptr(); subcontour_node_ptr != NULL; 
              subcontour_node_ptr=subcontour_node_ptr->get_nextptr())
         {
            p.push_back(parallelogram(subcontour_node_ptr->get_data()));
//            cout << "subregion n = " << n << " parallelogram = " 
//                 << p[p.size()-1] << endl;
            n++;
         } // loop over subcontour linked list

// Loop over all pixels within cleaned version of *pixel_list_ptr and
// determine in which subregion they reside.  Save subregion points
// and heights separately into STL vectors for later processing needs:

         for (Mynode<datapoint> const *pixelnode_ptr=pixel_list_ptr->
                 get_start_ptr(); pixelnode_ptr != NULL; 
              pixelnode_ptr=pixelnode_ptr->get_nextptr())
         {
            threevector currpoint(pixelnode_ptr->get_data().get_var(2),
                               pixelnode_ptr->get_data().get_var(3),
                               pixelnode_ptr->get_data().get_func(0));
            for (n=0; n<n_subregions; n++)
            {
               if (p[n].point_inside(currpoint))
               {
                  xy_translated_subregion_point[n].push_back(
                     currpoint-COM_xy);
                  subregion_height[n].push_back(currpoint.get(2));
               }
            } // loop over parallelograms array
         } // loop over pixel linked list
      }

// ---------------------------------------------------------------------
// Method determine_subregion_rooftype takes in an oriented box
// corresponding to some subregion of a particular building.  It first
// computes the rooftop pitch angle within this subregion.  If the
// pitch is sufficiently small, the region is declared to have a flat
// rooftop.  Otherwise, this method subsequently checks whether the
// rooftop region has a spine or a pyramid shape.

   void determine_subregion_rooftype(
      double zroof_hi,vector<threevector>& xy_translated_subregion_point,
      vector<double>& subregion_height,twoDarray* binary_mask_twoDarray_ptr,
      oriented_box* oriented_box_ptr)
      {
         double delta_z=zroof_hi-oriented_box_ptr->get_height();
         double s=basic_math::min(oriented_box_ptr->get_width(),
                      oriented_box_ptr->get_length());
         double rooftop_pitch_angle=atan2(delta_z,0.5*s);

//         const double flat_roof_height_variation=1.5;	// meter

// On 7/22/04, we empirically found that the roof pitch angle for the
// apartment complex is slightly less than 15 degrees....

         const double min_roof_pitch=15*PI/180;
         if (rooftop_pitch_angle < min_roof_pitch)
         {
            oriented_box_ptr->get_roof_ptr()->set_none_flag(false);
            oriented_box_ptr->get_roof_ptr()->set_flat_flag(true);
         }
         else
         {
            threevector rooftop_COM;
            double moi_ratio=compute_initial_rooftop_moi_ratio(
               xy_translated_subregion_point,subregion_height,
               binary_mask_twoDarray_ptr,rooftop_COM);
            if (pyramidal_rooftop_structure(
               moi_ratio,rooftop_pitch_angle,
               xy_translated_subregion_point,subregion_height,rooftop_COM))
            {
               oriented_box_ptr->get_roof_ptr()->set_none_flag(false);
               oriented_box_ptr->get_roof_ptr()->set_pyramid_flag(true);
               oriented_box_ptr->get_roof_ptr()->set_COM(
                  oriented_box_ptr->get_bottomface().vertex_average()+
                  threevector(0,0,rooftop_COM.get(2)));
//               cout << "pyramidal roof found at " 
//                    << oriented_box_ptr->get_roof_ptr()->get_COM()
//                    << endl;
            }
            else
            {
               linesegment* spine_ptr=rooftop_spine_extraction(
                  rooftop_COM,xy_translated_subregion_point,
                  subregion_height,binary_mask_twoDarray_ptr);
               if (spine_ptr != NULL)
               {
                  oriented_box_ptr->get_roof_ptr()->set_none_flag(false);
//                  cout << "*spine_ptr = " << *spine_ptr << endl;
               
                  oriented_box_ptr->get_roof_ptr()->set_COM(
                     oriented_box_ptr->get_bottomface().vertex_average()+
                     threevector(0,0,rooftop_COM.get(2)));
//                  cout << "rooftop_COM = " << rooftop_COM << endl;
//                  cout << "Roof spine found at " 
//                       << oriented_box_ptr->get_roof_ptr()->get_COM()
//                       << endl;
                  spine_ptr->absolute_position(
                     oriented_box_ptr->get_roof_ptr()->get_COM());
//                  xy_projection());
                  oriented_box_ptr->get_roof_ptr()->set_spine_ptr(spine_ptr);
//                  cout << "roof spine = " 
//                       << *(oriented_box_ptr->get_roof_ptr()->get_spine_ptr())
//                       << endl;
               }
            }
         } // rooftop_pitch_angle < min_roof_pitch conditional
         
//         cout << "*oriented_box_ptr = " << *oriented_box_ptr << endl;
      }
      
// ---------------------------------------------------------------------
// Method refine_ground_height_estimate takes in a 3D box which is
// supposed to have already been basically fitted to some building's
// rooftop pixels.  It first creates an enlarged copy of the box's
// bottom face.  It subsequently looks at the feature classification
// values for points along the bottom face's edges.  Height values for
// those points corresponding to road and grass are averaged together
// and returned as a refined height estimate for the ground in the
// immediate vicinity of the building.  

   double refine_ground_height_estimate(      
      oriented_box const *oriented_box_ptr,twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr)
      {
         polygon enlarged_bottom_face(oriented_box_ptr->get_bottomface());
         const double min_footprint_area=10;	// meters**2
         if (enlarged_bottom_face.compute_area() < min_footprint_area) 
            return 0;
   
         enlarged_bottom_face.scale(1.25);
         enlarged_bottom_face.initialize_edge_segments();

//   drawfunc::draw_polygon(
//      enlarged_bottom_face,colorfunc::red,features_twoDarray_ptr);
//   writeimage("enlarged_bface",features_twoDarray_ptr,false,
//              ladarimage::p_data);   
//   writeimage("ztwoDarray",ztwoDarray_ptr);

         const double ds=0.3;	// meter
         const unsigned int n_edges=enlarged_bottom_face.get_nvertices();
         int n_integrated_edges=0;
         double ground_height_sum=0;
         for (unsigned int n=0; n<n_edges; n++)
         {
            linesegment curr_edge=enlarged_bottom_face.get_edge(n);
            unsigned int nsteps=basic_math::round(curr_edge.get_length()/ds);

            int n_low_points=0;
            double zsum=0;
            for (unsigned int i=0; i<nsteps; i++)
            {
               threevector curr_pnt(curr_edge.get_v1()+
                                 i*ds*curr_edge.get_ehat());
               unsigned int px,py;
               if (features_twoDarray_ptr->point_to_pixel(
                  curr_pnt,px,py))
               {
                  double curr_p=features_twoDarray_ptr->get(px,py);
                  if (nearly_equal(curr_p,featurefunc::road_sentinel_value) 
                      ||
                      nearly_equal(curr_p,featurefunc::grass_sentinel_value))
                  {
                     zsum += ztwoDarray_ptr->get(px,py);
                     n_low_points++;
                  }
               }
            } // loop over index i labeling step along current edge
            if (n_low_points > 0)
            {
               ground_height_sum += zsum/n_low_points;
               n_integrated_edges++;
            }
         } // loop over index n labeling enlarged bottom face edges

         double avg_ground_height=0;
         if (n_integrated_edges > 0)
         {
            avg_ground_height=ground_height_sum/n_integrated_edges;
         }
         return avg_ground_height;
      }

// ---------------------------------------------------------------------
// Method adjust_oriented_boxes_relative_to_ground translates and
// rescales the 3D oriented boxes within the input linked list
// *box_list_ptr so that their bottom face heights equal the average
// surrounding ground height while their top face heights remain
// unchanged.

   void adjust_oriented_boxes_relative_to_ground(
      int n_grnd_measurements,double ground_height_sum,
      Linkedlist<oriented_box*>* box_list_ptr)
      {
         if (n_grnd_measurements >= 1)
         {
            double avg_ground_height=
               ground_height_sum/double(n_grnd_measurements);

            for (Mynode<oriented_box*>* currnode_ptr=box_list_ptr->
                    get_start_ptr(); currnode_ptr != NULL; currnode_ptr=
                    currnode_ptr->get_nextptr())
            {
               oriented_box* oriented_box_ptr=currnode_ptr->get_data();
               double scale_factor=
                  1-avg_ground_height/oriented_box_ptr->get_height();
               oriented_box_ptr->scale(
                  oriented_box_ptr->get_center().get_pnt(),
                  threevector(1,1,scale_factor));
               oriented_box_ptr->translate(
                  threevector(0,0,0.5*avg_ground_height));
            } // loop over nodes in *box_list_ptr 
         } // n_grnd_measurements >= 1 conditional
      }

// ---------------------------------------------------------------------
// Method construct_3D_bldg_bbox() takes in a binary mask for an NYC
// skyscraper rooftop.  After rotating the mask so that it's aligned
// with the XY axes, this method computes an XY parallelogram for the
// building's footprint.  It then forms and returns a polyhedron bbox
// from the footprint plus the input max_roof_z value.

   polyhedron construct_3D_bldg_bbox(
      double theta,double max_roof_z,const threevector& COM,
      twoDarray* p_roof_binary_twoDarray_ptr)
      {
//         cout << "inside urbanfunc::construct_3D_bldg_bbox()" << endl;

         urbanimage p_roof_urbanimage;
         double edge_fraction_of_median=0.02;
         parallelogram* bbox_ptr=p_roof_urbanimage.compute_building_bbox(
            COM,p_roof_binary_twoDarray_ptr,theta,edge_fraction_of_median);
//         cout << "bbox_ptr = " << bbox_ptr << endl;
//         cout << "parallelogram = " << *bbox_ptr << endl;
         delete p_roof_binary_twoDarray_ptr;

         vector<threevector> vertices;
         for (unsigned int v=0; v<bbox_ptr->get_nvertices(); v++)
         {
            vertices.push_back(bbox_ptr->get_vertex(v));
         }
         for (unsigned int v=0; v<bbox_ptr->get_nvertices(); v++)
         {
            threevector top_vertex=bbox_ptr->get_vertex(v);
            top_vertex.put(2,max_roof_z);
            vertices.push_back(top_vertex);
         }
         delete bbox_ptr;

         polyhedron bbox_3D;
         bbox_3D.generate_box(vertices);
         bbox_3D.ensure_faces_handedness(face::right_handed);
//         cout << "bbox_3D = " << bbox_3D << endl;
         return bbox_3D;
      }

// ==========================================================================
// Network methods
// ==========================================================================

// Method link_passes_through_building takes in two roadpoints as well
// as a twoDarray which is presumed to contain contain building
// information with all other feature values set to zero.  It
// instantiates a line segment between the roadpoints and then
// computes the line integral of the building twoDarray.  If the
// distance over which the segment is on top of building valued pixels
// exceeds max_length, this boolean method returns true.

   bool link_passes_through_building(
      roadpoint const *curr_roadpoint_ptr,
      roadpoint const *neighbor_roadpoint_ptr,
      twoDarray const *just_bldgs_twoDarray_ptr)
      {
         linesegment roadlink(curr_roadpoint_ptr->get_posn(),
                              neighbor_roadpoint_ptr->get_posn());
         double line_integral=imagefunc::fast_line_integral_along_segment(
            just_bldgs_twoDarray_ptr,roadlink);
         const double max_length=6;	// meters
         return (line_integral/featurefunc::building_sentinel_value > 
                 max_length);
      }

// ---------------------------------------------------------------------
// Method frac_link_length_over_asphalt takes in linesegment roadlink
// as well as a twoDarray which is presumed to contain contain asphalt
// information with all other feature values set to zero.  It computes
// the line integral of the asphalt twoDarray.  It returns the
// fraction of the linesegment's length which lies on top of asphalt.

   double frac_link_length_over_asphalt(
      const linesegment& roadlink,twoDarray const *just_asphalt_twoDarray_ptr)
      {
         double line_integral=imagefunc::fast_line_integral_along_segment(
            just_asphalt_twoDarray_ptr,roadlink);
         double asphalt_frac=(line_integral/featurefunc::road_sentinel_value)/
            roadlink.get_length();
         return asphalt_frac;
      }

// ---------------------------------------------------------------------
// Method compute_building_height_distribution scans over the input
// buildings network.  It computes and stores the maximum heights for
// all the buildings within the network.  This method generates a
// metafile plot of the max building height distribution.

   void accumulate_building_heights(
      Network<building*> const *buildings_network_ptr,
      vector<double>& bldg_heights)
      {
         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            building* b_ptr=buildings_network_ptr->get_site_data_ptr(n);
            double max_height=b_ptr->find_max_height();
            cout << "n = " << n << " max height = " << max_height << endl;
            bldg_heights.push_back(b_ptr->find_max_height());
         }
      }

   void plot_building_height_distribution(
      const vector<double>& building_heights,string imagedir)
      {
         prob_distribution prob_z(building_heights,25);
         const double prob_lo=0.02;
         const double prob_hi=0.99;
         double zlo=prob_z.find_x_corresponding_to_pcum(prob_lo);
         double zhi=prob_z.find_x_corresponding_to_pcum(prob_hi);
         cout << "zlo = " << zlo << " zhi = " << zhi << endl;

         prob_z.set_xtic(1);
         prob_z.set_xsubtic(0.5);
         prob_z.set_densityfilenamestr(imagedir+"bldg_height_dist.meta");
         prob_z.set_xlabel("Building height (meters)");
         prob_z.write_density_dist();
      }

   void compute_building_height_distribution(
      string imagedir,Network<building*> const *buildings_network_ptr)
      {
         vector<double> bldg_heights;

         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            building* b_ptr=buildings_network_ptr->get_site_data_ptr(n);
            double max_height=b_ptr->find_max_height();
            cout << "n = " << n << " max height = " << max_height << endl;
            bldg_heights.push_back(b_ptr->find_max_height());
         }

         prob_distribution prob_z(bldg_heights,25);
         const double prob_lo=0.02;
         const double prob_hi=0.99;
         double zlo=prob_z.find_x_corresponding_to_pcum(prob_lo);
         double zhi=prob_z.find_x_corresponding_to_pcum(prob_hi);
         cout << "zlo = " << zlo << " zhi = " << zhi << endl;

         prob_z.set_xtic(1);
         prob_z.set_xsubtic(0.5);
         prob_z.set_densityfilenamestr(imagedir+"bldg_height_dist.meta");
         prob_z.set_xlabel("Building height (meters)");
         prob_z.write_density_dist();
      }
   
// ==========================================================================
// Object annotation methods
// ==========================================================================

// Method annotate_building_labels loops over each entry within
// *buildings_network_ptr.  It generates a threeDstring for each
// building corresponding to its integer label.  This method adds the
// threeDstring label above the building within the output xyzp file.

   void annotate_building_labels(
      Network<building*> const *buildings_network_ptr,
      string xyzp_filename,double annotation_value,
      bool display_cityblock_IDs)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);

         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            building* building_ptr=buildings_network_ptr->
               get_site_data_ptr(n);
            int cityblock_ID=building_ptr->get_cityblock_ID();
            threeDstring building_label;
            if (display_cityblock_IDs)
            {
               building_label=stringfunc::number_to_string(cityblock_ID);
            }
            else
            {
               building_label=stringfunc::number_to_string(n);
            }
//            building_label.scale(building_label.get_origin().get_pnt(),
//                                 building_label_size);
            threevector annotation_point(building_ptr->get_posn()+15*z_hat);
            building_label.center_upon_location(annotation_point);
            draw3Dfunc::draw_threeDstring(
               building_label,xyzp_filename,annotation_value);

         } // loop over sites in buildings network 
         filefunc::gzip_file(xyzp_filename);         
      }

// ---------------------------------------------------------------------
   void annotate_particular_building(
      int n,Network<building*> const *buildings_network_ptr,
      string xyzp_filename,double annotation_value)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);

         threeDstring building_label(stringfunc::number_to_string(n));
         threevector annotation_point(
            buildings_network_ptr->get_site_data_ptr(n)->get_posn()+15*z_hat);
         building_label.center_upon_location(annotation_point);
         draw3Dfunc::draw_threeDstring(
            building_label,xyzp_filename,annotation_value);
         filefunc::gzip_file_if_gunzipped(xyzp_filename);         
      }

// ---------------------------------------------------------------------
// Method annotate_street_islands_and_peninsulas takes in an xyzp file
// which we assume contains a feature map.  This method scans over all
// entries within the buildings network.  It draws the integer IDs for
// those buildings which have been classified as street islands and
// street peninsulas onto the xyzp output file.

   void annotate_street_islands_and_peninsulas(
      Network<building*> const *buildings_network_ptr,string xyzp_filename,
      double annotation_value)
      {
         outputfunc::write_banner("Displaying street islands & peninsulas:");
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            building* curr_bldg_ptr=buildings_network_ptr->
               get_site_data_ptr(n);
            if (curr_bldg_ptr->get_is_street_island())
            {
               annotate_particular_building(
                  n,buildings_network_ptr,xyzp_filename,annotation_value);
            }
            else if (curr_bldg_ptr->get_is_street_peninsula())
            {
               annotate_particular_building(
                  n,buildings_network_ptr,xyzp_filename,annotation_value);
            }
         } // loop over sites in buildings network 
         filefunc::gzip_file(xyzp_filename);         
      }

// ---------------------------------------------------------------------
// Method annotate_tree_cluster_labels loops over each entry within
// *trees_network_ptr.  It generates a threeDstring for each
// building corresponding to its integer label.  This method adds the
// threeDstring label above the building within the output xyzp file.

   void annotate_treecluster_labels(
      Network<tree_cluster*> const *trees_network_ptr,
      string xyzp_filename,double annotation_value)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);

         for (const Mynode<int>* currnode_ptr=trees_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            threeDstring treecluster_label(stringfunc::number_to_string(n));
//            const double treecluster_label_size=0.25;
//            treecluster_label.scale(treecluster_label.get_origin().get_pnt(),
//                                    treecluster_label_size);
            tree_cluster* treecluster_ptr=trees_network_ptr->
               get_site_data_ptr(n);
            threevector annotation_point(treecluster_ptr->get_posn()+15*z_hat);
            treecluster_label.center_upon_location(annotation_point);
            draw3Dfunc::draw_threeDstring(
               treecluster_label,xyzp_filename,annotation_value);
         } // loop over sites in buildings network 
//         filefunc::gzip_file(xyzp_filename);         
      }

// ==========================================================================
// Coloring methods
// ==========================================================================

// Method color_feature_heights takes in flattened height image in
// twoDarray *ztwoDarray as well as feature map *ftwoDarray_ptr.  It
// dynamically generates a new twoDarray in which height and feature
// classification information are fused together.  The output is
// specifically intended to be displayed with our "hue+value" colormap
// addition to the Group 94 dataviewer.  In mid-April 2004, we
// explicitly charted out reasonable color ranges for buildings
// (various shades of blue), roads (various shades of red, orange and
// rust), trees (shades of dark olive green) and grass (shades of
// bright lime green).  This method cycles over these color ranges,
// and it chooses a particular color value for a particular pixel
// depending upon its height.  The resulting fused height and feature
// classification images look quite striking!

   twoDarray* color_feature_heights(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ftwoDarray_ptr)
      {
         twoDarray* gtwoDarray_ptr=new twoDarray(ftwoDarray_ptr);
         gtwoDarray_ptr->initialize_values(xyzpfunc::null_value);

// Cars show up very well when delta_z_road=0.2, delta_f_road=0.005
// f_road_min=0.12 and f_road_max=0.18

         const double delta_z_road=0.2;	 // meter
//         const double delta_z_road=0.3;	 // meter
//         const double delta_z_road=0.5;	 // meter
//         const double delta_z_road=0.75;	 // meter
//         const double delta_z_road=1.0;	 // meter
         const double delta_z_tree=1.0;	 // meter
         const double delta_z_grass=0.3; // meter
//         const double delta_z_grass=0.2; // meter
         const double delta_z_bldg=0.3;	 // meter
         const double delta_z_shadow=0.1;  // meter

//         const double delta_f_road=0.0025;
          const double delta_f_road=0.005;			// default
//         const double delta_f_road=0.0075;
//         const double delta_f_road=0.01;
         const double delta_f_tree=0.01;
         const double delta_f_grass=0.01;
         const double delta_f_bldg=0.005;
         const double delta_f_shadow=0.005;

//         const double f_road_min=0.05;	// red-orange
         const double f_road_min=0.12;	// red-orange	      default
//         const double f_road_min=0.13;	// red-orange
//         const double f_road_max=0.17;	// yellow-orange
         const double f_road_max=0.18;	// yellow-orange      default

// For reasons which we don't understand as of 1/20/05, the asphalt
// coloring for Lowell chunk 45-51 is noticeably lighter than in the
// past.  In order to better match the asphalt coloring in older JPEG
// images, we have found that using the following specialized road
// parameters for this one particular Lowell pass helps: (f_road
// params updated on 7/15/05 for poster generation purposes...)

//         const double delta_f_road=0.0026;
//         const double f_road_min=0.11;		
//         const double f_road_max=0.23;		

         const double f_grass_min=0.44;  
         const double f_grass_max=0.46;  

         const double f_tree_min=0.545;  // olive
         const double f_tree_max=0.555;  // dark olive

// In June 04, we realized that our previous 0.61 value for f_bldg_min
// was too low.  Various parts of building rooftops looked too close
// in color to tall trees.  So we increased 0.61 to 0.649.

         const double f_bldg_min=0.649;  // cyan  
         const double f_bldg_max=0.75;   // dark blue
        
//         const double f_shadow_min=0.935;  // purple
//         const double f_shadow_max=0.98;  // purple
         const double f_shadow_min=0.95;  // purple
         const double f_shadow_max=0.95;  // purple
//         const double f_shadow_min=0.19;  // dark yellow
//         const double f_shadow_max=0.22;  // bright yellow
         
         const int n_roadbins=basic_math::round(
            (f_road_max-f_road_min)/delta_f_road)+1;
         const int n_grassbins=basic_math::round(
            (f_grass_max-f_grass_min)/delta_f_grass)+1;
         const int n_treebins=basic_math::round(
            (f_tree_max-f_tree_min)/delta_f_tree)+1;
         const int n_bldgbins=basic_math::round(
            (f_bldg_max-f_bldg_min)/delta_f_bldg)+1;
         const int n_shadowbins=basic_math::round(
            (f_shadow_max-f_shadow_min)/delta_f_shadow)+1;
         
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double curr_f=ftwoDarray_ptr->get(px,py);
               double curr_z=ztwoDarray_ptr->get(px,py);
               
               double new_f=xyzpfunc::null_value;
               if (curr_f > 0.99*xyzpfunc::null_value)
               {
                  if (nearly_equal(curr_f,featurefunc::road_sentinel_value))
                  {
                     int z_bin=basic_math::round(curr_z/delta_z_road);
                     int n=modulo(z_bin,n_roadbins);
                     new_f=f_road_min+n*delta_f_road;
//                     cout << " n = " << n << " z = " << curr_z
//                          << " z_bin = " << z_bin 
//                          << " new_f = " << new_f << endl;
                  }
                  else if (nearly_equal(
                     curr_f,featurefunc::tree_sentinel_value))
                  {
                     int z_bin=basic_math::round(curr_z/delta_z_tree);
                     int n=modulo(z_bin,n_treebins);
                     new_f=f_tree_min+n*delta_f_tree;
                  }
                  else if (nearly_equal(
                     curr_f,featurefunc::grass_sentinel_value))
                  {
                     int z_bin=basic_math::round(curr_z/delta_z_grass);
                     int n=modulo(z_bin,n_grassbins);
                     new_f=f_grass_min+n*delta_f_grass;
                  }
                  else if (nearly_equal(
                     curr_f,featurefunc::building_sentinel_value))
                  {
                     int z_bin=basic_math::round(curr_z/delta_z_bldg);
                     int n=modulo(z_bin,n_bldgbins);
                     new_f=f_bldg_min+n*delta_f_bldg;
                  }
                  else if (nearly_equal(
                     curr_f,featurefunc::shadow_sentinel_value))
                  {
                     int z_bin=basic_math::round(curr_z/delta_z_shadow);
                     int n=modulo(z_bin,n_shadowbins);
                     new_f=f_shadow_min+n*delta_f_shadow;
                  }
                  gtwoDarray_ptr->put(px,py,new_f);
               } // curr_f > null_value conditional
            } // loop over py index
         } // loop over px index

         return gtwoDarray_ptr;
      }
   
// ---------------------------------------------------------------------
// Method fuse_height_feature_and_prob_images takes in height image
// *ztwoDarray_ptr, refined features & height image
// *features_twoDarray_ptr and danger probability image
// *ptwoDarray_ptr (whose values are assumed to range between 0.0 and
// 1.0).  This method returns a fused image which is meant to be
// viewed with our (small) hue and value colormap.  P information is
// indicated by coarse hue variations ranging from reds (p_lo) to dark
// purple (p_hi).  Fine hue variations are used to cyclically convey
// height information.

// We specifically implemented this method in July 04 in order to
// construct IED "danger maps".  

   twoDarray* fuse_height_feature_and_prob_images(
      double p_hi,double p_lo,
      twoDarray const *ztwoDarray_ptr,
      twoDarray const *features_twoDarray_ptr,twoDarray const *ptwoDarray_ptr)
      {
         outputfunc::write_banner(
            "Fusing height, features and probability images:");
         twoDarray* ftwoDarray_ptr=new twoDarray(ztwoDarray_ptr);   

         const double delta_z=0.2;	// meter
         const double delta_p=0.25;	// Danger probability bin size

         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double z=ztwoDarray_ptr->get(px,py);
               double p=p_lo+ptwoDarray_ptr->get(px,py)*(p_hi-p_lo);
               double feature=features_twoDarray_ptr->get(px,py);

               double f=xyzpfunc::null_value;
               if (z > 0.99*xyzpfunc::null_value  &&
                   p > 0.99*xyzpfunc::null_value)
               {
                  int p_bin=basic_math::mytruncate(p/delta_p);  
				// pbin=0,1,..,7

                  if (p_bin >= 3) 	// 0 < p_danger < 0.25
                  {
                     f=feature;
                  }
                  else
                  {
                     double fmin,fmax;
                     if (p_bin==0)	// 0.75 < p_danger < 1.0
                     {
                        fmin=0.0;	// red
                        fmax=0.07;
                     }
                     else if (p_bin==1)	// 0.50 < p_danger < 0.75
                     {
                        fmin=0.94;	// pink/magenta
                        fmax=0.98;
                     }
                     else if (p_bin==2)	// 0.25 < p_danger < 0.50
                     {
                        fmin=0.89;	// dark purple
                        fmax=0.93;
                     }
                     double delta_f=0.0035;
                     int n_fbins=basic_math::round((fmax-fmin)/delta_f)+1;
                     int z_bin=basic_math::round(z/delta_z);
                     int n=modulo(z_bin,n_fbins);
                     f=fmin+n*delta_f;
                     f=basic_math::min(0.98,f);
                  }
                  
//                  cout << "px = " << px << " py = " << py << " p = " << p
//                       << endl;
//                  cout << "p_bin = " << p_bin 
//                       << " z_bin = " << z_bin << " n = " 
//                       << n << " f = " << f << endl << endl;
               }
               ftwoDarray_ptr->put(px,py,f);
            } // loop over index py
         } // loop over index px
         return ftwoDarray_ptr;
      }

// ==========================================================================
// 3D drawing methods
// ==========================================================================

// Method draw_rooftop_structure takes in a building pointer as well
// as the filename for an output xyzp file.  This method draws a
// rooftop's spine (if it exists) as well as the line segments which
// connect the spine to the closest points on the building's oriented
// box.  If the rooftop is flat, this method draws a rectangular grid
// on top of the building's oriented box.  If the rooftop's shape is
// pyramidal, no spine is drawn but the connecting segments to the top
// face of the building's oriented box are displayed.

   void draw_rooftop_structure(
      oriented_box const *oriented_box_ptr,string xyzp_filename,
      double annotation_value)
      {
         rooftop const *roof_ptr=oriented_box_ptr->get_roof_ptr();
//         cout << "roof = " << *roof_ptr << endl;
         
         if (!roof_ptr->get_none_flag())
         {
            if (roof_ptr->get_flat_flag())
            {
               polygon top_face=oriented_box_ptr->get_topface();
               draw3Dfunc::draw_rectangle_grid(
                  top_face,xyzp_filename,annotation_value);
            }
            else if (roof_ptr->get_pyramid_flag())
            {
               threevector pyramid_apex(roof_ptr->get_COM());
               for (unsigned int i=0; 
                    i<oriented_box_ptr->get_topface().get_nvertices(); 
                    i++)
               {
                  threevector curr_vertex=oriented_box_ptr->get_topface().
                     get_vertex(i);
                  draw3Dfunc::draw_line(
                     linesegment(curr_vertex,pyramid_apex),xyzp_filename,
                     annotation_value);
               } // loop over index i labeling top face vertex
            }
            else if (roof_ptr->get_spine_ptr() != NULL)
            {
//               cout << "roof_ptr has non null spine ptr" << endl;
               
               draw3Dfunc::draw_line(
                  *(roof_ptr->get_spine_ptr()),
                  xyzp_filename,annotation_value);
            
               for (unsigned int i=0; i<oriented_box_ptr->get_topface().
                       get_nvertices(); i++)
               {
                  threevector curr_vertex(oriented_box_ptr->get_topface().
                                       get_vertex(i));
                  threevector v1(roof_ptr->get_spine_ptr()->get_v1());
                  threevector v2(roof_ptr->get_spine_ptr()->get_v2());
                  double dist1=(curr_vertex-v1).magnitude();
                  double dist2=(curr_vertex-v2).magnitude();
                  if (dist1 < dist2)
                  {
                     draw3Dfunc::draw_line(
                        linesegment(curr_vertex,v1),xyzp_filename,
                        annotation_value);
                  }
                  else
                  {
                     draw3Dfunc::draw_line(
                        linesegment(curr_vertex,v2),xyzp_filename,
                        annotation_value);
                  }
               } // loop over index i labeling top face vertex
            } // rooftop_spine_ptr != NULL conditional
         } // !none_flag conditional
      }
   
// ---------------------------------------------------------------------
// Method draw_3D_buildings loops over all entries within the input
// buildings network.  It draws the parallelepiped associated with
// each building's rooftop set of voxels onto the specified output
// XYZP file.  It also draws the rooftop structure for each building
// onto the XYZP file.

   void draw_3D_buildings(
      Network<building*> const *buildings_network_ptr,
      string xyzp_filename,double annotation_value,bool gzip_output_file)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            draw_particular_3D_building(
               n,buildings_network_ptr,xyzp_filename,annotation_value);
         } // loop over nodes in buildings network entries list

//         draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
//            xyzp_filename,Zero_vector);
         if (gzip_output_file) 
            filefunc::gzip_file_if_gunzipped(xyzp_filename);         
      }

// ---------------------------------------------------------------------
// This overloaded version of draw_3D_buildings highlights buildings
// which have some particular spatial relationship with nearby trees
// (e.g. have tall trees located in their backyards).  It takes in a
// linked list containing the integer IDs of those buildings within
// input *buildings_network_ptr which have the particular spatial
// relationship with nearby trees.  Those special buildings are
// colored according to annotation_value2, while all the other
// buildings are colored according to annotation_value1.

   void draw_3D_buildings(
      Linkedlist<int> const *bldgs_near_trees_list_ptr,
      Network<building*> const *buildings_network_ptr,
      string xyzp_filename,double annotation_value1,double annotation_value2,
      bool gzip_output_file)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);

         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            double annotation_value=annotation_value1;
            if (bldgs_near_trees_list_ptr->data_in_list(n) != NULL)
            {
               annotation_value=annotation_value2;
            }
            draw_particular_3D_building(
               n,buildings_network_ptr,xyzp_filename,annotation_value);
         } // loop over nodes in buildings network entries list
         delete bldgs_near_trees_list_ptr;

//         draw3Dfunc::append_fake_xyzp_points_for_dataviewer_coloring(
//            xyzp_filename,Zero_vector);
         if (gzip_output_file) 
            filefunc::gzip_file_if_gunzipped(xyzp_filename);         
      }

// ---------------------------------------------------------------------
// This specialized version of draw_3D_buildings was quickly concocted
// in Dec 2005 to generate a twoDarray containing building footprint
// information for simulated spurious PSS track rejection purposes.

   void draw_3D_buildings(
      Network<building*> const *buildings_network_ptr,
      twoDarray* ztwoDarray_ptr)
      {
         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            draw_particular_3D_building(
               n,buildings_network_ptr,ztwoDarray_ptr);
         } // loop over nodes in buildings network entries list
      }

// ---------------------------------------------------------------------
   void draw_particular_3D_building(
      int n,Network<building*> const *buildings_network_ptr,
      string xyzp_filename,double annotation_value)
      {
         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         Linkedlist<oriented_box*>* box_list_ptr=buildings_network_ptr->
            get_site_data_ptr(n)->get_box_list_ptr();
         if (box_list_ptr != NULL)
         {
            for (Mynode<oriented_box*>* box_node_ptr=
                    box_list_ptr->get_start_ptr(); box_node_ptr != NULL;
                 box_node_ptr=box_node_ptr->get_nextptr())
            {
               oriented_box* oriented_box_ptr=box_node_ptr->get_data();
               building* building_ptr=buildings_network_ptr->
                  get_site_data_ptr(n);

//               if (!building_ptr->get_on_street())
//               {
//                  annotation_value=0.84;
//               }
               if (building_ptr->get_on_street_corner())
               {
                  annotation_value=0.96;
               }
//               if (building_ptr->get_is_street_island())
//               {
//                  annotation_value=0.52;
//               }

               draw3Dfunc::draw_parallelepiped(
                  *oriented_box_ptr,xyzp_filename,annotation_value);

// For poster generation purposes only, we do not draw the ugly
// rooftop for ugly house #55 in chunk 45-51:

//               if (n != 55)
               {
                  urbanfunc::draw_rooftop_structure(
                     oriented_box_ptr,xyzp_filename,annotation_value);
               }
               
            } // loop over oriented box subcomponents of nth building
         } // box_list_ptr != NULL conditional
      }

// ---------------------------------------------------------------------
// This specialized version of draw_particular_3D_building was
// concocted in Dec 2005 to output building footprint information to
// output twoDarray *ztwoDarray_ptr for spurious PSS track rejection
// study purposes.

   void draw_particular_3D_building(
      int n,Network<building*> const *buildings_network_ptr,
      twoDarray* ztwoDarray_ptr)
      {
         const colorfunc::Color bldg_color=colorfunc::cyan;
         
         Linkedlist<oriented_box*>* box_list_ptr=buildings_network_ptr->
            get_site_data_ptr(n)->get_box_list_ptr();
         if (box_list_ptr != NULL)
         {
            for (Mynode<oriented_box*>* box_node_ptr=
                    box_list_ptr->get_start_ptr(); box_node_ptr != NULL;
                 box_node_ptr=box_node_ptr->get_nextptr())
            {
               oriented_box* oriented_box_ptr=box_node_ptr->get_data();
               drawfunc::color_polygon_interior(
                  oriented_box_ptr->get_bottomface(),bldg_color,
                  ztwoDarray_ptr);
            } // loop over oriented box subcomponents of nth building
         } // box_list_ptr != NULL conditional
      }

// ---------------------------------------------------------------------
// Method draw_3D_building_front_dirs loops over each entry within the
// input buildings network.  For those buildings whose front direction
// is "known", this method constructs a vector from the center of the
// building's parallelepiped which emanates outward in the "front
// direction".  This vector information is drawn onto the specified
// output xyzp binary file.

   void draw_3D_building_front_dirs(
      Network<building*> const *buildings_network_ptr,
      string xyzp_filename,double annotation_value,bool gzip_output_file)
      {
         outputfunc::write_banner("Drawing 3D building front directions:");

         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            building* b_ptr=buildings_network_ptr->get_site_data_ptr(n);

// Draw direction vectors pointing from Voronoi region center towards
// significant asphalt:

            const double arrow_length=15;	// meters

            threevector front_direction=b_ptr->get_front_dir(0);
            if (front_direction != Zero_vector)
            {
               threevector v(arrow_length*front_direction);
               threevector basepoint(b_ptr->get_center());
               draw3Dfunc::draw_vector(
                  v,basepoint,xyzp_filename,annotation_value);
            }

            if (b_ptr->get_on_street_corner())
            {
               threevector front_direction=b_ptr->get_front_dir(1);
               if (front_direction != Zero_vector)
               {
                  threevector v(arrow_length*front_direction);
                  threevector basepoint(b_ptr->get_center());
                  draw3Dfunc::draw_vector(
                     v,basepoint,xyzp_filename,annotation_value);
               }
            } // building is on street corner conditional
         } // loop over sites in buildings network 
         if (gzip_output_file) filefunc::gzip_file_if_gunzipped(
            xyzp_filename);
      }

// ==========================================================================
// High-level feature information input/output methods
// ==========================================================================

   void print_out_particular_building(
      int n,Network<building*> const *buildings_network_ptr,
      string output_filename)
      {
         ofstream outstream;
         filefunc::openfile(output_filename,outstream);
         contour* contour_ptr=buildings_network_ptr->
            get_site_data_ptr(n)->get_contour_ptr();
         cout << "contour = " << *contour_ptr << endl;

         Linkedlist<oriented_box*>* box_list_ptr=buildings_network_ptr->
            get_site_data_ptr(n)->get_box_list_ptr();
         if (box_list_ptr != NULL)
         {
            int oriented_box_counter=0;
            for (Mynode<oriented_box*>* box_node_ptr=
                    box_list_ptr->get_start_ptr(); box_node_ptr != NULL;
                 box_node_ptr=box_node_ptr->get_nextptr())
            {
               oriented_box* oriented_box_ptr=box_node_ptr->get_data();
               contour c_top(&(oriented_box_ptr->get_topface()));
               contour c_bottom(&(oriented_box_ptr->get_bottomface()));

               outstream << "Oriented box counter = " 
                         << oriented_box_counter << endl;
               outstream << "Top face:" << endl;
               outstream << c_top << endl;
//               outstream << "Bottom face:" << endl;
//               outstream << c_bottom << endl;
               oriented_box_counter++;
            } // loop over oriented box subcomponents of nth building
         } // box_list_ptr != NULL conditional

         filefunc::closefile(output_filename,outstream);
      }

// ---------------------------------------------------------------------
// Method writeout_building_wall_polys takes in integer n which labels
// some building within the input buildings network
// *buildings_network_ptr.  It first retrieves the building
// footprint's orthogonal contour.  It next computes the midpoint for
// each edge along the footprint contour.  This method finds the
// height along the top face for the point corresponding to the
// midpoint along the bottom face.  It generates a polygon using the
// xy information from the orthogonal contour edge and the height
// information from the top face midpoint.  This poly info is written
// to the output text file specified by the input parameter
// output_filename.

   void writeout_building_wall_polys(
      int n,Network<building*> const *buildings_network_ptr,
      string output_filename)
      {
         contour* contour_ptr=buildings_network_ptr->
            get_site_data_ptr(n)->get_contour_ptr();
         cout << "contour = " << *contour_ptr << endl;

         vector<polygon>* walls_poly_ptr=new vector<polygon>;
         for (unsigned int i=0; i<contour_ptr->get_nvertices(); i++)
         {
            linesegment curr_edge(contour_ptr->get_edge(i));
            threevector bottom_midpoint(curr_edge.get_midpoint());
            
// Scan through oriented boxes and find which one overlaps current
// edge's midpoint.  Then compute fractional distance of midpoint
// along the oriented box's bottom face.  Retrieve height value
// corresponding to that fractional distance from oriented box's top
// face:

            Linkedlist<oriented_box*>* box_list_ptr=buildings_network_ptr->
               get_site_data_ptr(n)->get_box_list_ptr();
            if (box_list_ptr != NULL)
            {
               int oriented_box_counter=0;
               for (Mynode<oriented_box*>* box_node_ptr=
                       box_list_ptr->get_start_ptr(); box_node_ptr != NULL;
                    box_node_ptr=box_node_ptr->get_nextptr())
               {
                  oriented_box* oriented_box_ptr=box_node_ptr->get_data();

                  polygon bottom_face(oriented_box_ptr->get_bottomface());
                  unsigned int nvertices=bottom_face.get_nvertices();

                  vector<threevector> vertex;
                  for (unsigned int j=0; j<nvertices; j++)
                  {
                     vertex.push_back(bottom_face.get_vertex(j));
                     threevector v1=curr_edge.get_v1();
                     threevector v2=curr_edge.get_v2();
                     v1.put(2,vertex[j].get(2));
                     v2.put(2,vertex[j].get(2));
                     curr_edge.set_v1(v1);
                     curr_edge.set_v2(v2);
//                     curr_edge.set_v1_component(2,vertex[j].get(2));
//                     curr_edge.set_v2_component(2,vertex[j].get(2));
                     vertex.back().put(2,0);
                  }
                  polygon zeroed_bottom_face(vertex);

                  zeroed_bottom_face.initialize_edge_segments();
                  double frac=zeroed_bottom_face.frac_distance_along_polygon(
                     bottom_midpoint);
                  
                  if (frac > 0)
                  {
                     threevector top_midpoint;
                     oriented_box_ptr->get_topface().edge_point(
                        frac,top_midpoint);

                     vector<threevector> wall_vertex;
                     wall_vertex.push_back(curr_edge.get_v1());
                     wall_vertex.push_back(curr_edge.get_v2());
                     wall_vertex.push_back(curr_edge.get_v2());
                     wall_vertex.push_back(curr_edge.get_v1());

                     wall_vertex[2].put(2,top_midpoint.get(2));
                     wall_vertex[3].put(2,top_midpoint.get(2));
                     polygon curr_wall_poly(wall_vertex);
                     walls_poly_ptr->push_back(curr_wall_poly);
                     break;
                  } // midpoint lies along bottom poly conditional
                  oriented_box_counter++;
               } // loop over oriented box subcomponents of nth building
            } // box_list_ptr != NULL conditional
         
         } // loop over index i labeling contour vertex/edge

         cout << "Wall polys:" << endl;
         templatefunc::printVector(*walls_poly_ptr);
         
// Write wall polygon information to output text file:

         ofstream outstream;
         filefunc::openfile(output_filename,outstream);
         for (unsigned int i=0; i<walls_poly_ptr->size(); i++)
         {
            polygon curr_wall((*walls_poly_ptr)[i]);
            outstream << "# Wall " << i << endl;
            for (unsigned int v=0; v<4; v++)
            {
               threevector curr_vertex(curr_wall.get_vertex(v));
               outstream << curr_vertex.get(0) << "  "
                         << curr_vertex.get(1) << "  "
                         << curr_vertex.get(2) << "  " << endl;
            } // loop over index v labeling current wall vertices
            outstream << endl;
         } // loop over index i labeling wall polygons
         filefunc::closefile(output_filename,outstream);

         delete walls_poly_ptr;
      }

// ---------------------------------------------------------------------
// Method readin_building_wall_polys reads in the wall polygon
// information written out by method writeout_building_wall_polys()
// from the text file specified by input_filename.  It dynamically
// generates and returns an STL vector which holds this building wall
// polygon information.

   vector<polygon>* readin_building_wall_polys(string input_filename)
      {
         cout << "inside urbanfunc::readin_building_wall_polys()" << endl;
         cout << "Make sure this method works OK after converting" << endl;
         cout << "from vertex[4] to vector<vertex>" << endl;

         vector<polygon>* walls_poly_ptr=new vector<polygon>;
         vector<string> line;
         filefunc::ReadInfile(input_filename,line);
         stringfunc::comment_strip(line);

         vector<threevector> vertex(4);
         for (unsigned int i=0; i<line.size(); i++)
         {
            vector<double> R=stringfunc::string_to_numbers(line[i]);
            vertex[i%4]=threevector(R[0],R[1],R[2]);
            if (i >= 3 && (i+1)%4==0)
            {
               polygon curr_wall(vertex);
               walls_poly_ptr->push_back(curr_wall);
            }
         }
         return walls_poly_ptr;
      }

// ==========================================================================
// Buildings network text input/output methods:
// ==========================================================================

// Method output_buildings_network_to_textfile loops over all
// buildings within the input *buildings_network_ptr and saves their
// information to the text file specified by output_filename.

   void output_buildings_network_to_textfile(
      Network<building*> const *buildings_network_ptr,string output_filename)
      {
         outputfunc::write_banner(
            "Writing buildings network to output textfile:");
         
         ofstream outstream;
         filefunc::deletefile(output_filename);
         filefunc::openfile(output_filename,outstream);

         for (const Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            output_building_and_its_links(buildings_network_ptr,n,outstream);
         } // loop over nodes in buildings network entries list
         filefunc::closefile(output_filename,outstream);
      }

// ---------------------------------------------------------------------
   void output_building_and_its_links(
      Network<building*> const *buildings_network_ptr,int n,
      ofstream& outstream)
      {
         building* building_ptr=buildings_network_ptr->
            get_site_data_ptr(n);
         Linkedlist<oriented_box*>* box_list_ptr=building_ptr->
            get_box_list_ptr();

         outstream << n << " # building number" << endl;
         threevector bldg_posn(building_ptr->get_posn());
         outstream << bldg_posn.get(0) << " "
                   << bldg_posn.get(1) << " "
                   << bldg_posn.get(2) << " " << endl;
//         building_ptr->get_posn().write_to_textstream(outstream);
         threevector bldg_ctr(building_ptr->get_center());
         outstream << bldg_ctr.get(0) << " "
                   << bldg_ctr.get(1) << " "
                   << bldg_ctr.get(2) << " " << endl;
//         building_ptr->get_center().write_to_textstream(outstream);
         outstream << building_ptr->get_on_street_corner() 
                   << " # on street corner boolean" << endl;
         outstream << building_ptr->get_is_street_island() 
                   << " # on street island boolean" << endl;
         outstream << building_ptr->get_is_street_peninsula() 
                   << " # on street peninsula boolean" << endl;
         
         if (box_list_ptr != NULL)
         {
            outstream << box_list_ptr->size() 
                      << " # number of oriented boxes" << endl;
            for (Mynode<oriented_box*>* box_node_ptr=
                    box_list_ptr->get_start_ptr(); box_node_ptr != NULL;
                 box_node_ptr=box_node_ptr->get_nextptr())
            {
               oriented_box* oriented_box_ptr=box_node_ptr->get_data();
               oriented_box_ptr->write_to_textstream(outstream);
            } // loop over oriented box subcomponents of nth building
         } // box_list_ptr != NULL conditional

// Output building neighbor information:

         const Site<building*>* curr_building_site_ptr=
            buildings_network_ptr->get_site_ptr(n);
         const Linkedlist<netlink>* curr_bldglink_list_ptr
            =curr_building_site_ptr->get_netlink_list_ptr();
         if (curr_bldglink_list_ptr != NULL)
         {
            outstream << curr_bldglink_list_ptr->size() 
                      << " # Number of bldg neighbors" << endl;
            for (const Mynode<netlink>* curr_bldglink_node_ptr=
                    curr_bldglink_list_ptr->get_start_ptr();
                 curr_bldglink_node_ptr != NULL; 
                 curr_bldglink_node_ptr=curr_bldglink_node_ptr->get_nextptr())
            {
               int q=curr_bldglink_node_ptr->get_data().get_ID();
               outstream << q << " # Bldg neighbor ID" << endl;
            }
         } // curr_bldglink_list_ptr != NULL conditional
         else
         {
            outstream << "0 # No neighbors" << endl;	 
					// no neighboring buildings
         }
         outstream << endl;

      }

// ---------------------------------------------------------------------
// Method readin_buildings_network_from_textfile performs the inverse
// operation of method output_buildings_network_to_textfile.  It
// dynamically generates a buildings network based upon the
// information read in from a text file generated by
// output_buildings_network_to_textfile.

   Network<building*>* readin_buildings_network_from_textfile(
      string input_filename)
      {
         outputfunc::write_banner(
            "Reading in buildings network from textfile:");

         vector<string> line;
         filefunc::ReadInfile(input_filename,line);
         stringfunc::comment_strip(line);

         Network<building*>* buildings_network_ptr=
            new Network<building*>(10*line.size());

         unsigned int i=0;
         while (i<line.size())
         {
            int n=stringfunc::string_to_number(line[i++]);
            building* curr_bldg_ptr=new building(n);

            vector<double> V=stringfunc::string_to_numbers(line[i++]);
            threevector curr_posn(V[0],V[1],V[2]);
            curr_bldg_ptr->set_posn(curr_posn);

            V.clear();
            V=stringfunc::string_to_numbers(line[i++]);
            threevector curr_center(V[0],V[1],V[2]);
//            curr_bldg_ptr->set_center(curr_center);

            curr_bldg_ptr->set_on_street_corner(
               nearly_equal(stringfunc::string_to_number(line[i++]),1));
            curr_bldg_ptr->set_is_street_island(
               nearly_equal(stringfunc::string_to_number(line[i++]),1));
            curr_bldg_ptr->set_is_street_peninsula(
               nearly_equal(stringfunc::string_to_number(line[i++]),1));

            unsigned int n_oriented_boxes=stringfunc::string_to_number(
               line[i++]);
            Linkedlist<oriented_box*>* box_list_ptr=
               new Linkedlist<oriented_box*>;

            for (unsigned int nbox=0; nbox<n_oriented_boxes; nbox++)
            {
               oriented_box* oriented_box_ptr=new oriented_box;
               oriented_box_ptr->read_from_text_lines(i,line);
               box_list_ptr->append_node(oriented_box_ptr);
            } // loop over nth building's oriented boxes
            curr_bldg_ptr->set_box_list_ptr(box_list_ptr);         

// For final 3D display purposes, we set the center of each building
// to equal its footprint's COM rather than the center position
// determined from its rooftop pixels:

            curr_bldg_ptr->set_center(curr_bldg_ptr->footprint_COM());

            buildings_network_ptr->insert_site(
               n,Site<building*>(curr_bldg_ptr));

// Read in current building's neighbor information:

            unsigned int n_neighbors=stringfunc::string_to_number(line[i++]);
            for (unsigned int j=0; j<n_neighbors; j++)
            {
               int q=stringfunc::string_to_number(line[i++]);
               buildings_network_ptr->add_to_neighbor_list(n,q);
            }

         } // i < line.size() conditional

         cout << "# buildings within network = " << buildings_network_ptr->
            size() << endl;

         return buildings_network_ptr;
      }

// ==========================================================================
// Building-tree spatial relationship methods
// ==========================================================================

// Method search_for_trees_near_bldgs loops over all buildings within
// input network *buildings_network_ptr.  It searches for tree
// clusters within input KDtree *kdtree_ptr lying at least partially
// within max_bldg_to_tree_dist of each building in the buildings
// network.  This method also requires that portions of the tree
// cluster contour make an angle relative to the buildings' front
// directions which lies within the interval
// [min_angle_relative_to_front,max_angle_relative_to_front] (where
// both angles range from 0 to 2*PI in radians).  Finally, the tree
// cluster portions must also range in height over the interval
// [min_tree_height,max_tree_height].  ID numbers for those buildings
// within the network satisfying all these constraints are returned
// within a dynamically generated linked list.

   Linkedlist<int>* search_for_trees_near_bldgs(
      Network<building*>* buildings_network_ptr,
      KDTree::KDTree<3, threevector> const *kdtree_ptr,
      double max_bldg_to_tree_dist,
      double min_angle_rel_to_front,double max_angle_rel_to_front,
      double min_tree_height,double max_tree_height)
      {
         outputfunc::write_banner("Searching for trees near buildings:");

         Linkedlist<int>* bldgs_with_nearby_trees_list_ptr=
            new Linkedlist<int>;
         for (Mynode<int>* currnode_ptr=buildings_network_ptr->
                 get_entries_list_ptr()->get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->
                 get_nextptr())
         {
            int n=currnode_ptr->get_data();
            cout << n << " " << flush;
            building* curr_bldg_ptr=buildings_network_ptr->
               get_site_data_ptr(n);
            if (search_for_trees_near_bldg(
               curr_bldg_ptr,max_bldg_to_tree_dist,
               min_angle_rel_to_front,max_angle_rel_to_front,
               min_tree_height,max_tree_height,kdtree_ptr))
            {
               bldgs_with_nearby_trees_list_ptr->append_node(n);
            }
         }
         return bldgs_with_nearby_trees_list_ptr;
      }

// ---------------------------------------------------------------------
// Method search_for_trees_near_bldg performs the search described in
// the preceding method's comment for the particular building *b_ptr.
// If the radial distance, angular and height conditions are all
// satisfied, this boolean method returns true.

   bool search_for_trees_near_bldg(
      building* b_ptr,double max_bldg_to_tree_dist,
      double min_angle_rel_to_front,double max_angle_rel_to_front,
      double min_tree_height,double max_tree_height,
      KDTree::KDTree<3, threevector> const *kdtree_ptr)  
      {
         b_ptr->find_max_height();

         threevector f_hat(b_ptr->get_front_dir(0));
         threevector f1_hat(b_ptr->get_front_dir(1));
//         cout << "b_ptr->get_max_height() = " 
//              << b_ptr->get_max_height() << endl;
//         cout << "f_hat = " << f_hat << endl;
//         cout << "f1_hat = " << f1_hat << endl;

         vector<threevector> nearby_tree_points;
         treefunc::tree_points_near_input_location(
            b_ptr->get_posn(),max_bldg_to_tree_dist,kdtree_ptr,
            nearby_tree_points);

         bool tree_cluster_found=false;
         for (unsigned int i=0; i<nearby_tree_points.size(); i++)
         {
            threevector XY_proj(nearby_tree_points[i].xy_projection());
            threevector g_hat( (XY_proj-b_ptr->get_posn()).unitvector());

            double theta=basic_math::phase_to_canonical_interval(
               mathfunc::angle_between_unitvectors(f_hat,g_hat),0,2*PI);
            double theta1=0;
            if (b_ptr->get_on_street_corner())
            {
               theta1=basic_math::phase_to_canonical_interval(
                  mathfunc::angle_between_unitvectors(f1_hat,g_hat),0,2*PI);
            }
      
            if ( (theta > min_angle_rel_to_front && 
                  theta < max_angle_rel_to_front) ||
                 (theta1 > min_angle_rel_to_front && 
                  theta1 < max_angle_rel_to_front))
            {
               double tree_height=nearby_tree_points[i].get(2);
//               double height_ratio=nearby_tree_points[i].get(2)/
//                  b_ptr->get_max_height();
//               cout << "min ratio = " << min_tree_to_bldg_height_ratio
//                    << " height ratio = " << height_ratio
//                    << " max ratio = " << max_tree_to_bldg_height_ratio
//                    << endl;
               
               if (tree_height > min_tree_height &&
                   tree_height < max_tree_height)
//               if (height_ratio > min_tree_to_bldg_height_ratio &&
//                   height_ratio < max_tree_to_bldg_height_ratio)
               {
                  tree_cluster_found=true;
//                  cout << "i = " << i 
//                       << " theta = " << theta*180/PI 
//                       << " theta1 = " << theta1*180/PI 
//                       << " height = " << tree_height << endl;
               }
            } // theta and theta1 conditionals
         } // loop over nearby tree points
         return tree_cluster_found;
      }

} // urbanfunc namespace
