// ==========================================================================
// Program FRAME2FRAME implements a simple model comparison of
// frame-to-frame vs frame-to-map video imagery registration.  We
// start with a temporal sequence of simulated "checkerboard" images
// which we assume were collected by a stationary camera.  We next
// simulate optic flow tracking by starting multiple KLT features at
// the corners and intersections of the checkerboard.  We model
// intensity feature wandering over time relative to genuine strong
// gradient locations by letting the KLT features undergo a diffusive
// random walk.  We then compute the homography that maps the KLT
// features in each image to their predecessor image counterparts and
// apply its inverse to propagate entire image corrections forward in
// time.  

// In order to quantify the increasing distortion which KLT feature
// random walks have upon video imagery registration, we compute the
// RMS displacement of every pixel in the static, input checkerboard
// image to its transformed location following inverse homography.
// application.  This program generates metafile output of this RMS
// displacement as a function of time averaged over multiple
// simulations.  Not surprisingly, the average RMS pixel-level
// registration error exhibits a square-root divergence over time.

// Finally, this same program can be used to model the benefit that a
// high-fidelity orthorectified map obtained via ladar imagery fusion
// with a single video frame.  We assume that future algorithms can be
// developed which would correct KLT feature locations in the video
// stream by comparing their locations with their counterparts in the
// fixed orthorectified map.  Such positive feedback would hopefully
// limit KLT feature fluctuations to within a fixed radius about true
// intensity gradient locations and prevent diffusive wander.  In this
// case, we vary our model tiepoints about their true locations but do
// not let them walk away from the mean position.  Again not
// surprisingly, the average RMS pixel-level registration error then
// becomes constant over time.

// ==========================================================================
// Last updated on 1/2/06; 12/7/08
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "image/binaryimagefuncs.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "geometry/homography.h"
#include "image/imagefuncs.h"
#include "datastructures/Linkedlist.h"
#include "image/myimage.h"
#include "numrec/nrfuncs.h"
#include "geometry/polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"
#include "math/twovector.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ios;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================
   
   bool input_param_file;
   int ninputlines,currlinenumber=0;
   string inputline[200];
//   clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

//   const int nxbins=501;
//   const int nybins=501;
//   const int nxbins=201;
//   const int nybins=201;
   const int nxbins=300;
   const int nybins=300;
   const double max_x=7;  // meters
   const double max_y=7;
	
   myimage xyzimage(nxbins,nybins);   
   xyzimage.z2Darray_ptr=new twoDarray(nxbins,nybins);

// Initialize image parameters:

   xyzimage.imagedir=filefunc::get_pwd()+"images/fitimage/";
   filefunc::dircreate(xyzimage.imagedir);
   xyzimage.classified=false;
   xyzimage.title="Simulated Video Image";
   string dirname="/home/cho/programs/c++/svn/projects/src/mains/alirt_acc/colortables/";
   xyzimage.colortablefilename=dirname+"colortable.image";
   xyzimage.z2Darray_ptr->init_coord_system(max_x,max_y);
   twoDarray* ztwoDarray_ptr=xyzimage.z2Darray_ptr;

// Create canonical "checkerboard" image:

   twoDarray* zprev_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);

   vector<twovector> vertex;
   const double s=100.0;
   vertex.push_back(twovector(-s,-s));
   vertex.push_back(twovector(s,-s));
   vertex.push_back(twovector(s,s));
   vertex.push_back(twovector(-s,s));
   polygon poly(vertex);
   const double poly_color_value=40;

   polygon upper_right(poly);
   polygon upper_left(poly);
   polygon lower_left(poly);
   polygon lower_right(poly);
   upper_right.translate(twovector(s,s));
   upper_left.translate(twovector(-s,s));
   lower_left.translate(twovector(-s,-s));
   lower_right.translate(twovector(s,-s));

   zprev_twoDarray_ptr->initialize_values(20);
   drawfunc::color_polygon_interior(
      upper_right,poly_color_value,zprev_twoDarray_ptr);
   drawfunc::color_polygon_interior(
      lower_left,poly_color_value,zprev_twoDarray_ptr);

// Create permanent record of initial checkboard image within
// *zorig_twoDarray_ptr:

   twoDarray* zorig_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   zprev_twoDarray_ptr->copy(zorig_twoDarray_ptr);

// Initialize 4 tiepoint locations at centers of checkerboard
// intersecting edges:

   vector<twovector> true_tiepoint,tiepoint;

   const double reduction_factor=0.975;
   double x_max=reduction_factor*ztwoDarray_ptr->get_xhi();
   double x_min=reduction_factor*ztwoDarray_ptr->get_xlo();
   double y_max=reduction_factor*ztwoDarray_ptr->get_yhi();
   double y_min=reduction_factor*ztwoDarray_ptr->get_ylo();
   true_tiepoint.push_back(twovector(x_max,y_max));
   true_tiepoint.push_back(twovector(x_max,0));
   true_tiepoint.push_back(twovector(x_max,y_min));

   true_tiepoint.push_back(twovector(0,y_max));
   true_tiepoint.push_back(twovector(0,0));
   true_tiepoint.push_back(twovector(0,y_min));

   true_tiepoint.push_back(twovector(x_min,y_max));
   true_tiepoint.push_back(twovector(x_min,0));
   true_tiepoint.push_back(twovector(x_min,y_min));
   
/*
  true_tiepoint.push_back(twovector(0.5*x_max,0));
  true_tiepoint.push_back(twovector(0,0.5*y_max));
  true_tiepoint.push_back(twovector(-0.5*x_max,0));
  true_tiepoint.push_back(twovector(0,-0.5*y_max));
*/

// Draw initial locations of tiepoints in initial image:

//   for (int i=0; i<tiepoint.size(); i++)
//   {
//      drawfunc::draw_hugepoint(
//         tiepoint[i],0.2,30,zprev_twoDarray_ptr);
//   }
//   xyzimage.writeimage("initial",zprev_twoDarray_ptr);

      char wander_char;
      cout << "Enter 'y' to let tiepoints wander in random walk:" << endl;
      cin >> wander_char;
      bool wander_flag=false;
      if (wander_char=='y')
      {
         wander_flag=true;
      }

//      nrfunc::init_time_based_seed();

      const int n_iters=1;
//      const int n_iters=10;

//   int n_images=1;
//   int n_images=40;
//   int n_images=100;
//   int n_images=200;
//      int n_images=500;
      int n_images=1000;
//   int skip_display_image=1;
//   int skip_display_image=5;
      int skip_display_image=10;
//      int skip_display_image=20;
//   double random_walk_scalefactor=0.01;
//   double random_walk_scalefactor=0.033;
      double random_walk_scalefactor=0.1;
//   double random_walk_scalefactor=0.2;

      vector<double> registration_error;
      registration_error.reserve(n_images);
      registration_error.clear();

      for (int iter=0; iter<n_iters; iter++)
      {

// Initialize tiepoints to their true locations at the start of each
// simulation:

         tiepoint.clear();
         for (int i=0; i<true_tiepoint.size(); i++)
         {
            tiepoint.push_back(true_tiepoint[i]);
         }

         for (int n=0; n<n_images; n++)
         {

// Assume KLT video tiepoints execute random walk relative to their
// starting image locations:

            vector<twovector> new_tiepoint;
            for (int i=0; i<tiepoint.size(); i++)
            {
               double dx=random_walk_scalefactor*2*(nrfunc::ran1()-0.5);
               double dy=random_walk_scalefactor*2*(nrfunc::ran1()-0.5);
               if (wander_flag)
               {
                  new_tiepoint.push_back(tiepoint[i]+twovector(dx,dy));
               }
               else
               {
                  new_tiepoint.push_back(true_tiepoint[i]+twovector(dx,dy));
               }
         
            } // loop over index i labeling polygon vertex

// Compute homography relating new tiepoints to previous tiepoints:

            homography H;

// We should compute the homography that relates new_tiepoint to
// tiepoint and then apply its inverse to the *corrected* previous
// image in *zprev_twoDarray_ptr.  Or equivalently, we can compute the
// homography that relates new_tiepoint to true_tiepoint and then
// apply its inverse to the original image in *zorig_twoDarray_ptr:

//            H.parse_homography_inputs(new_tiepoint,tiepoint);         
            H.parse_homography_inputs(new_tiepoint,true_tiepoint);         
            H.compute_homography_matrix();
//            H.check_homography_matrix(new_tiepoint,tiepoint);

// Loop over all pixels in current *ztwoDarray_ptr.  Retrieve their
// color values from *zorig_twoDarray_ptr:

            ztwoDarray_ptr->initialize_values(0);

            int px_prev,py_prev,n_pixels=0;
            double x,y,x_prev,y_prev;
            double sqrd_pixel_discrepancy=0;
            for (int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
            {
               for (int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
               {
                  ztwoDarray_ptr->pixel_to_point(px,py,x,y);

                  H.project_world_plane_to_image_plane(x,y,x_prev,y_prev);

//                  if (zprev_twoDarray_ptr->point_to_pixel(
//                     x_prev,y_prev,px_prev,py_prev))
                  if (zorig_twoDarray_ptr->point_to_pixel(
                     x_prev,y_prev,px_prev,py_prev))
                  {
                     ztwoDarray_ptr->put(px,py,zprev_twoDarray_ptr->get(
                        px_prev,py_prev));
//                     ztwoDarray_ptr->put(px,py,zorig_twoDarray_ptr->get(
//                        px_prev,py_prev));
                     sqrd_pixel_discrepancy += 
                        sqr(px-px_prev)+sqr(py-py_prev);
                     n_pixels++;
                  }
               } // loop over py index
            } // loop over px index
            registration_error[n] += 
               sqrt(sqrd_pixel_discrepancy/double(n_pixels));

// Take number of non-matching pixels in *zprev_twoDarray_ptr and
// *ztwoDarray_ptr as the "imagery registration error":

/*
  const int nsize=21;
  int n_alarms=0;
  for (int px=0; px<ztwoDarray_ptr->get_mdim(); px += nsize)
  {
  for (int py=0; py<ztwoDarray_ptr->get_ndim(); py += nsize)
  {
  int n_unequal_pixels=0;
  for (int qx=-nsize/2; qx <= nsize/2; qx++)
  {
  for (int qy=-nsize/2; qy <= nsize/2; qy++)
  {
  if (!nearly_equal(ztwoDarray_ptr->get(px+qx,py+qy),
  zprev_twoDarray_ptr->get(px+qx,py+qy)))
  {
  n_unequal_pixels++;
  } // individual pixel not equal conditional
  } // loop over qy index
  } // loop over qx index

  if (n_unequal_pixels > nsize/2)
  {
  n_alarms++;
  }
  } // loop over py index
  } // loop over px index
  registration_error.push_back(n_alarms);
*/


/*
  int n_mismatched_pixels=0;
  for (int p=0; p<ztwoDarray_ptr->get_mdim()*
  ztwoDarray_ptr->get_ndim(); p++)
  {
  if (!nearly_equal(
  ztwoDarray_ptr->get(p),zprev_twoDarray_ptr->get(p)))
  {
  n_mismatched_pixels++;
  }
  }
  registration_error.push_back(n_mismatched_pixels);
*/

// Before advancing to next image, copy current image to
// *zprev_twoDarray_ptr and new_tiepoint to tiepoint:

            ztwoDarray_ptr->copy(zprev_twoDarray_ptr);
            tiepoint.clear();
            for (int i=0; i<new_tiepoint.size(); i++)
            {
               tiepoint.push_back(new_tiepoint[i]);
            } // loop over index i labeling polygon vertex

// Write out and display results:

            if (n%skip_display_image==0)
            {
               cout << "iter = " << iter << " image = " << n
                    << " registration_error = " << registration_error[n]
                    << endl;

// Draw current locations of tiepoints in canonical image:

               for (int i=0; i<tiepoint.size(); i++)
               {
                  drawfunc::draw_hugepoint(
                     tiepoint[i],0.3,60,ztwoDarray_ptr);
               }
               string filename="current_"+stringfunc::integer_to_string(n,4);

               xyzimage.writeimage(filename,ztwoDarray_ptr);
               string imagedir="./images/fitimage/";
               string jpeg_filename=imagedir+filename+".jpg";
               
               const int width=448;
               const int height=445;
               const int xoffset=166;
               const int yoffset=81;
               imagefunc::crop_image(
                  jpeg_filename,width,height,xoffset,yoffset);
            } // display image conditional
            
         } // loop over index n labeling image
         
         //xyzimage.writeimage("final",ztwoDarray_ptr);
         
      } // loop over iter index labeling simulation number


// Write registration error results averaged over all simulation
// iterations to metafile output:

      ofstream outstream;
      string outfilename="registration_error.meta";
      filefunc::openfile(outfilename,outstream);
      for (int n=0; n<n_images; n++)
      {
         outstream << n << "  " << registration_error[n]/double(n_iters) 
                   << endl;
      }
      filefunc::closefile(outfilename,outstream);

      string imagedir="./images/fitimage/";
      outputfunc::generate_animation_script(
         0,n_images,"current_",imagedir,"view_movie",10,"jpg",
         skip_display_image);

      delete zprev_twoDarray_ptr;
}
