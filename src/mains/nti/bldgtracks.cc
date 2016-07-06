// ==========================================================================
// Program BLDGTRACKS reads in one of our favorite ALIRT-A Lowell
// chunks as well as building and road network information previously
// calculated as part of the 2003-2004 ACC project.  It generates a 2D
// mask of the buildings footprints on the ground.  The program then
// randomly generates spruious tracks of varying lengths which are
// forced to reside within the bounding box surrounding the Lowell
// data.  We check whether a spurious track ever intersects any
// building footprint in the mask.  If so, that track could be
// eliminated as bogus if such high-fidelity building information were
// available.

// We cobbled together this quick-and-dirty program to quantitatively
// address (using real Lowell data) the question of how ladar data
// could be used to cut down on PSS false alarm tracks.

// ==========================================================================
// Last updated on 1/1/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <vector>
#include "urban/bldgstrandfuncs.h"
#include "urban/cityblock.h"
#include "urban/cityblockfuncs.h"
#include "image/connectfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "ladar/groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "math/mathfuncs.h"
#include "templates/mytemplates.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "image/recursivefuncs.h"
#include "urban/roadfuncs.h"
#include "urban/roadpoint.h"
#include "urban/roadsegment.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "urban/tree_cluster.h"
#include "urban/treefuncs.h"
#include "image/TwoDarray.h"
#include "urban/urbanfuncs.h"
#include "urban/urbanimage.h"
#include "geometry/voronoifuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ios;
   using std::istream;
   using std::ofstream;
   using std::ostream;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   int ninputlines,currlinenumber;
   string inputline[200];

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Read in contents of partially processed binary xyzp file:

   cout << "Enter refined feature image:" << endl;
   urbanimage cityimage;

// Chimney footprint dimensions:

   const double delta_x=0.3;	// meters
   const double delta_y=0.3;	// meters
   cityimage.initialize_image(input_param_file,inputline,currlinenumber);
   cityimage.parse_and_store_input_data(delta_x,delta_y,true,false,false);
   cityimage.compute_data_bbox(cityimage.z2Darray_ptr,false);

// Eliminate junk nearby edges of data bounding box:

   parallelogram* data_bbox_ptr=cityimage.get_data_bbox_ptr();
   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.z2Darray_ptr);
   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.get_p2Darray_ptr());

   twoDarray* ztwoDarray_ptr=cityimage.z2Darray_ptr;
   twoDarray const *features_twoDarray_ptr=cityimage.get_p2Darray_ptr();
   twoDarray const *features_and_heights_twoDarray_ptr=
      urbanfunc::color_feature_heights(
         ztwoDarray_ptr,features_twoDarray_ptr);

//   string features_filename=cityimage.imagedir+"features.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,features_twoDarray_ptr,features_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      features_twoDarray_ptr,features_filename);

   double xhi=ztwoDarray_ptr->get_xhi();
   double xlo=ztwoDarray_ptr->get_xlo();
   double yhi=ztwoDarray_ptr->get_yhi();
   double ylo=ztwoDarray_ptr->get_ylo();

   cout << "xhi = " << xhi << " xlo = " << xlo << endl;
   cout << "yhi = " << yhi << " ylo = " << ylo << endl;

// ==========================================================================
// Buildings network restoration
// ==========================================================================

// Read in previously saved buildings network information from ascii
// text file:

   string bldgs_text_filename=cityimage.imagedir+"buildings_network.txt";
   cityimage.set_buildings_network_ptr(
      urbanfunc::readin_buildings_network_from_textfile(bldgs_text_filename));
//   cout << "total number of buildings = " 
//        << cityimage.get_buildings_network_ptr()->size()
//        << endl;

// ==========================================================================
// Draw buildings network results
// ==========================================================================

   double delta_z=3;	// meters
//   draw3Dfunc::draw_thick_lines=true;		// default
   draw3Dfunc::draw_thick_lines=false;	// for poster
   draw3Dfunc::ds=0.02;			// for poster
   draw3Dfunc::delta_phi=10*PI/180;		// for poster

   string networks_filename=cityimage.imagedir+"networks.xyzp";   
   filefunc::deletefile(networks_filename);

// Draw 3D buildings network:

   outputfunc::write_banner("Drawing 3D buildings network:");
   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),
      networks_filename,0.64,false);

// Initialize all ztwoDarray entries with some dummy value.  Then
// create binary mask of house footprints in ztwoDarray.  We will
// subsequently perform a brute force search along each track's path
// for masked footprint pixel values.  If any are encountered during
// the track's lifetime, the track must have encountered an
// impenetrable building.  It is then marked as a collision track...

   ztwoDarray_ptr->initialize_values(5);
   ladarfunc::null_data_outside_bbox(data_bbox_ptr,ztwoDarray_ptr);
   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),ztwoDarray_ptr);
  
// Randomly generate tracks which lie completely inside Lowell
// neighborhood:

   int ntracks=25;
//   int ntracks=50;
//   int ntracks=100;
//   int ntracks=500;
//   int ntracks=1000;

// Next set of parameters for viewgraph generation purposes:

   double init_track_length=50;
   double final_track_length=50;
   int n_iters=2;

   double dtrack_length=(final_track_length-init_track_length)/(n_iters-1);
   const double ds=min(
      ztwoDarray_ptr->get_deltax(),ztwoDarray_ptr->get_deltay());
   const colorfunc::Color house_color=colorfunc::cyan;
   const double house_color_value=colorfunc::color_to_value(house_color);

//   nrfunc::init_time_based_seed();
   for (int iter=0; iter<n_iters; iter++)
   {
      double track_length=init_track_length+iter*dtrack_length;
      int n_pixels=track_length/ds;

      cout << "iter = " << iter << " track length = " << track_length << endl;

      int n_obstructed_paths=0;
      int track_number=0;
   
      while (track_number < ntracks)
      {
         double xstart=xlo+nrfunc::ran1()*(xhi-xlo);
         double ystart=ylo+nrfunc::ran1()*(yhi-ylo);
         double theta=nrfunc::ran1()*2*PI;
         double xstop=xstart+cos(theta)*track_length;
         double ystop=ystart+sin(theta)*track_length;

         threevector start_point(xstart,ystart);
         threevector stop_point(xstop,ystop);
         if (data_bbox_ptr->point_inside(start_point) &&
             data_bbox_ptr->point_inside(stop_point))
         {
//            cout << "Track number = " << track_number << endl;
//            cout << "xstart,ystart = " << xstart << "," << ystart << endl;
//            cout << "xstop,ystop = " << xstop << "," << ystop << endl;

// Check every pixel along track path.  If any corresponds to a house
// color, increment n_obstructed_paths:

            twovector e_hat(cos(theta),sin(theta));
            bool obstructed_path_flag=false;
            for (int n=0; n<n_pixels; n++)
            {
               twovector XY=twovector(xstart,ystart)+n*ds*e_hat;
               int px,py;
               ztwoDarray_ptr->point_to_pixel(XY.get(0),XY.get(1),px,py);
               double pixel_color=ztwoDarray_ptr->get(px,py);

               if (nearly_equal(pixel_color,house_color_value))
               {
                  obstructed_path_flag=true;
                  break;
               }
            } // loop over index n labeling pixels along track path
            if (obstructed_path_flag) n_obstructed_paths++;

            const double z_nom=0.5;	// meter
            linesegment curr_track(threevector(xstart,ystart,z_nom),
                                   threevector(xstop,ystop,z_nom));
//            drawfunc::draw_line(curr_track,45,ztwoDarray_ptr,true,false);

            double annotation_value=0.4;
            if (obstructed_path_flag) annotation_value=0.02;
            draw3Dfunc::draw_line(curr_track,networks_filename,
                                  annotation_value);
            track_number++;
         } // starting & stopping points inside data bounding box conditional
      } // while loop over tracks

      double obstruction_percentage=double(n_obstructed_paths)/
         double(ntracks)*100;
//      cout << "Track length = " << track_length << endl;
//      cout << "Number of tracks = " << ntracks << endl;
      cout << "Number obstructed paths = " << n_obstructed_paths << endl;
      cout << "Obstructed path percentage = " << obstruction_percentage
           << endl << endl;

   } // loop over iter index labeling track length

   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,networks_filename,false);

}


