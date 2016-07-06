// ==========================================================================
// Program REJECT reads in one of our favorite ALIRT-A Lowell chunks
// as well as building and road network information previously
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
// Last updated on 1/1/06; 4/24/06; 7/29/06; 12/4/10
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
   unsigned int ninputlines,currlinenumber;
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
   cityimage.compute_data_bbox(cityimage.get_z2Darray_ptr(),false);

// Eliminate junk nearby edges of data bounding box:

   parallelogram* data_bbox_ptr=cityimage.get_data_bbox_ptr();

   cout << "data_bbox = " << *data_bbox_ptr << endl;
   exit(-1);
   
   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.get_z2Darray_ptr());
   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.get_p2Darray_ptr());

   twoDarray* ztwoDarray_ptr=cityimage.get_z2Darray_ptr();
   twoDarray const *features_twoDarray_ptr=cityimage.get_p2Darray_ptr();
   twoDarray const *features_and_heights_twoDarray_ptr=
      urbanfunc::color_feature_heights(
         ztwoDarray_ptr,features_twoDarray_ptr);

//   string features_filename=cityimage.get_imagedir()+"features.xyzp";   
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
// Trees, road and buildings networks restoration
// ==========================================================================

// Read in previously saved trees network information from ascii text
// file:

   string trees_network_filename=cityimage.get_imagedir()+"trees_network.txt";
   bool vertices_lie_in_plane=false;
   treefunc::trees_network_ptr=
      treefunc::readin_trees_network_from_textfile(
         trees_network_filename,vertices_lie_in_plane);
   KDTree::KDTree<3, threevector>* kdtree_ptr=
      treefunc::generate_tree_network_kdtree(treefunc::trees_network_ptr);

// Draw tree cluster COMs and cylinders:

   twoDarray* COMs_twoDarray_ptr=new twoDarray(
      features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(COMs_twoDarray_ptr);
//   cityimage.draw_network_posns(
//      treefunc::trees_network_ptr,COMs_twoDarray_ptr);

//   string COMs_filename=cityimage.get_imagedir()+"tree_cylinders.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,COMs_twoDarray_ptr,COMs_filename);
//   cityimage.draw_network_contour_cylinders(
//      COMs_filename,treefunc::trees_network_ptr);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      COMs_twoDarray_ptr,COMs_filename);

// Read in previously saved road intersections network information
// from ascii text file:

   string intersections_network_filename=cityimage.get_imagedir()+
      "intersections_network.txt";
   roadfunc::intersections_network_ptr=
      roadfunc::readin_road_network_from_textfile(
         intersections_network_filename);
   roadfunc::intersections_network_ptr->sort_all_site_neighbors();

// Read in previously saved buildings network information from ascii
// text file:

   string bldgs_text_filename=cityimage.get_imagedir()+"buildings_network.txt";
   cityimage.set_buildings_network_ptr(
      urbanfunc::readin_buildings_network_from_textfile(bldgs_text_filename));
//   cityimage.get_buildings_network_ptr()->sort_all_site_neighbors();

//   cout << "total number of buildings = " 
//        << cityimage.get_buildings_network_ptr()->size()

//        << endl;

   string dirname=
      "/home/cho/programs/c++/svn/projects/src/mains/alirt_acc/colortables/";
   cityimage.set_colortable_filename(dirname+"colortable.image");

// ==========================================================================
// Draw final 3D network results
// ==========================================================================

   double delta_z=3;	// meters
//   draw3Dfunc::draw_thick_lines=true;		// default
   draw3Dfunc::draw_thick_lines=false;	// for poster
//   draw3Dfunc::ds=0.2;			// default
//   draw3Dfunc::ds=0.1;
//   draw3Dfunc::ds=0.05;
   draw3Dfunc::ds=0.02;			// for poster
//   draw3Dfunc::delta_phi=60*PI/180;
   draw3Dfunc::delta_phi=10*PI/180;		// for poster

   twoDarray* intersections_twoDarray_ptr=new twoDarray(
      features_and_heights_twoDarray_ptr);
   features_and_heights_twoDarray_ptr->copy(intersections_twoDarray_ptr);

   string networks_filename=cityimage.get_imagedir()+"networks.xyzp";   
   filefunc::deletefile(networks_filename);
//   roadfunc::draw_roadpoints(
//      roadfunc::intersections_network_ptr,intersections_twoDarray_ptr,false);
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,intersections_twoDarray_ptr,networks_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      intersections_twoDarray_ptr,networks_filename);

// Draw 3D road network:

   outputfunc::write_banner("Drawing 3D road network:");

//   draw3Dfunc::draw_3D_nearest_neighbor_links(
//      networks_filename,roadfunc::intersections_network_ptr,
//      featurefunc::road_sentinel_value,delta_z);

//   roadfunc::annotate_roadpoint_labels(
//      roadfunc::intersections_network_ptr,networks_filename,
//      draw3Dfunc::annotation_value1);

// Draw 3D buildings network:

   outputfunc::write_banner("Drawing 3D buildings network:");

   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),
      networks_filename,0.64,false);

   ztwoDarray_ptr->initialize_values(5);
   ladarfunc::null_data_outside_bbox(data_bbox_ptr,ztwoDarray_ptr);
      
   urbanfunc::draw_3D_buildings(
      cityimage.get_buildings_network_ptr(),ztwoDarray_ptr);
//   cityimage.adjust_x_scale=false;
  
// Randomly generate tracks which lie completely inside Lowell
// neighborhood:

   int ntracks=25;
//   int ntracks=50;
//   int ntracks=100;
//   int ntracks=500;
//   int ntracks=1000;

//   double init_track_length=0;
//   double final_track_length=100;
//   int n_iters=100;
//   int n_iters=300;
//   int n_iters=500;

// Next set of parameters for viewgraph generation purposes:

   double init_track_length=50;
   double final_track_length=50;
   int n_iters=2;

   double dtrack_length=(final_track_length-init_track_length)/(n_iters-1);
   const double ds=basic_math::min(
      ztwoDarray_ptr->get_deltax(),ztwoDarray_ptr->get_deltay());
   const colorfunc::Color house_color=colorfunc::cyan;
   const double house_color_value=colorfunc::color_to_value(house_color);

//   double track_length=200;	// meters
//   cout << "Enter track length in meters" << endl;
//   cin >> track_length;

// Initialize metafile to display percentages of spurious tracks that
// intersect impenetrable buildings in simulated neighborhood:

   string metafilename="obstruction.meta";
   ofstream obstruct_stream;
   filefunc::openfile(metafilename,obstruct_stream);
   obstruct_stream << 
      "title 'Spurious track collisions with impenetrable buildings'" << endl;
   obstruct_stream << "x axis min 0 max "+stringfunc::number_to_string(
      final_track_length) << endl;
   obstruct_stream << "label 'Spurious track length (meters)'" << endl;
   obstruct_stream << "tics 50 50" << endl;
   obstruct_stream << "y axis min 0 max 100" << endl;
   obstruct_stream << "label 'Percentage track intersections with buildings'"
                   << endl;
   obstruct_stream << "tics 20 10" << endl;
   obstruct_stream << "curve color red thick 2" << endl;

   vector<double> w,percentage_spurious_tracks_eliminated;

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
               unsigned int px,py;
               ztwoDarray_ptr->point_to_pixel(XY.get(0),XY.get(1),px,py);
               double pixel_color=ztwoDarray_ptr->get(px,py);

               if (nearly_equal(pixel_color,house_color_value))
               {
                  obstructed_path_flag=true;
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
      obstruct_stream << track_length << "\t\t" 
                      << obstruction_percentage << endl;
      const double mu=23.58;
      const double sigma=5;
      double prefactor=1/(sqrt(2*PI)*sigma);
      w.push_back(prefactor*exp(-0.5*sqr( (track_length-mu)/sigma ) ) );
      percentage_spurious_tracks_eliminated.push_back(
         w.back()*obstruction_percentage);
   } // loop over iter index labeling track length

   filefunc::closefile(metafilename,obstruct_stream);

   double gaussian_integral=mathfunc::simpsonsum(w)*dtrack_length;
   double integrated_spurious_track_eliminated_percentage=
      mathfunc::simpsonsum(percentage_spurious_tracks_eliminated)*
      dtrack_length;

   cout << "gaussian_integral = " << gaussian_integral << endl;
   cout << " integrated spurious track elimianted percentage = "
        << integrated_spurious_track_eliminated_percentage << endl;

//   cityimage.myimage::writeimage(
//      "house_footprints",0,"Lowell neighborhood",xhi,yhi,
//      ztwoDarray_ptr);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      ztwoDarray_ptr,networks_filename,false);

}


