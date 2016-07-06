// ==========================================================================
// Program ALARM
// ==========================================================================
// Last updated on 12/8/05
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "datastructures/Linkedlist.h"
#include "image/myimage.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "geometry/triangulate_funcs.h"
#include "image/twoDarray.h"

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

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int nxbins=1001;
   const int nybins=1001;
   const double max_x=200;  // meters
   const double max_y=160;  // meters
	
   myimage xyzimage(nxbins,nybins);   
   xyzimage.z2Darray_orig_ptr=new twoDarray(nxbins,nybins);
   xyzimage.z2Darray_ptr=new twoDarray(nxbins,nybins);
   xyzimage.title="Simulated neighborhood"; 
   xyzimage.xaxislabel="X (meters)";
   xyzimage.yaxislabel="Y (meters)";
   xyzimage.xtic=50; // meters
   xyzimage.ytic=50; // meters

   bool input_param_file;
   int ninputlines,currlinenumber=0;
   string inputline[200];
//   clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Initialize image parameters:

   xyzimage.imagedir=filefunc::get_pwd()+"images/fitimage/";
   filefunc::dircreate(xyzimage.imagedir);
   xyzimage.classified=false;
   string dirname="/home/cho/programs/c++/svn/projects/src/mains/alirt_acc/colortables/";
   xyzimage.colortablefilename=dirname+"colortable.image";
   xyzimage.z2Darray_orig_ptr->init_coord_system(max_x,max_y);

   xyzimage.z2Darray_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->initialize_values(xyzpfunc::null_value);
   twoDarray* ztwoDarray_ptr=xyzimage.z2Darray_ptr;

   const double house_length=10;	// meters
   const double house_width=7;		// meters
   vector<myvector> vertex;
   vertex.push_back(myvector(-0.5*house_width,-0.5*house_length));
   vertex.push_back(myvector(0.5*house_width,-0.5*house_length));
   vertex.push_back(myvector(0.5*house_width,0.5*house_length));
   vertex.push_back(myvector(-0.5*house_width,0.5*house_length));
   polygon house(vertex);

   double xhi=ztwoDarray_ptr->get_xhi();
   double xlo=ztwoDarray_ptr->get_xlo();
   double yhi=ztwoDarray_ptr->get_yhi();
   double ylo=ztwoDarray_ptr->get_ylo();

   const double next_door_neighbor_separation=15;	// meters
   const double across_street_separation=35; // meters
   int mx=2*xhi/next_door_neighbor_separation;
   int ny=2*yhi/across_street_separation;
   cout << "mx = " << mx << " ny = " << ny << endl;
   
   colorfunc::Color house_color=colorfunc::cyan;

   for (int m=-mx/2; m<=mx/2; m++)
   {
      double x=m*next_door_neighbor_separation;
      for (int n=-ny/2; n<=ny/2; n++)
      {
         double y=n*across_street_separation;  
         polygon curr_house(house);
         curr_house.absolute_position(threevector(x,y));
         drawfunc::color_polygon_interior(
            curr_house,house_color,ztwoDarray_ptr);         
      } // loop over index n labeling vertical house location
   } // loop over index m labeling horizontal house location
   
// Randomly generate tracks which lie completely inside simulated
// neighborhood:

   int ntracks=500;
   double init_track_length=0;
   double final_track_length=400;
   int n_iters=100;
   double dtrack_length=(final_track_length-init_track_length)/(n_iters-1);
   const double ds=min(
      ztwoDarray_ptr->get_deltax(),ztwoDarray_ptr->get_deltay());
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
   obstruct_stream << "x axis min 0 max 400" << endl;
   obstruct_stream << "label 'Spurious track length (meters)'" << endl;
   obstruct_stream << "tics 50 50" << endl;
   obstruct_stream << "y axis min 0 max 100" << endl;
   obstruct_stream << "label 'Percentage track intersections with buildings'"
                   << endl;
   obstruct_stream << "tics 20 10" << endl;
   obstruct_stream << "curve color red thick 2" << endl;

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
         if (xstop > xlo && xstop < xhi && ystop > ylo && ystop < yhi)
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
               }
            } // loop over index n labeling pixels along track path
            if (obstructed_path_flag) n_obstructed_paths++;

//            linesegment curr_track(threevector(xstart,ystart),
//                                   threevector(xstop,ystop));
//            drawfunc::draw_line(curr_track,colorfunc::red,ztwoDarray_ptr);
            track_number++;
         }
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
   }

   filefunc::closefile(metafilename,obstruct_stream);

   
   xyzimage.adjust_x_scale=false;
   xyzimage.writeimage("houses",0,"Simulated neighborhood",xhi,yhi,
                       ztwoDarray_ptr);


}
