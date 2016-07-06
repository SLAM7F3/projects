// ==========================================================================
// Program ERODE
// ==========================================================================
// Last updated on 2/18/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "general/sysfuncs.h"
#include "general/stringfuncs.h"
#include "image/TwoDarray.h"
#include "regular_polygon.h"
#include "drawfuncs.h"
#include "general/filefuncs.h"
#include "imagefuncs.h"
#include "groundfuncs.h"
#include "ladarfuncs.h"
#include "ladarimage.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::ostream;
   using std::cout;
   using std::ios;
   using std::endl;
   using std::string;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================
// Constant definitions
// ==========================================================================

   const int nxbins=255;
   const int nybins=255;
   const double max_x=30;  // meters
   const double max_y=30;

	
   ladarimage xyzimage(nxbins,nybins);   

   xyzimage.z2Darray_orig_ptr=new twoDarray(nxbins,nybins);
   xyzimage.z2Darray_ptr=new twoDarray(nxbins,nybins);
   
   bool input_param_file;
   int ninputlines,currlinenumber=0;
   string inputline[200];
//   clearscreen();
   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Initialize image parameters:

   xyzimage.imagedir=sysfunc::get_cplusplusrootdir()+"alirt/images/erode/";
   filefunc::dircreate(xyzimage.imagedir);
   xyzimage.classified=false;
   xyzimage.title="Simulated Ladar Image";
   xyzimage.colortablefilename=sysfunc::get_cplusplusrootdir()
      +"alirt/colortables/colortable.image";
   xyzimage.z2Darray_orig_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->init_coord_system(max_x,max_y);
   xyzimage.z2Darray_ptr->initialize_values(NEGATIVEINFINITY);

   double spatial_resolution=2*xyzimage.z2Darray_ptr->get_deltax();

   regular_polygon circle(20,10);
   circle.translate(threevector(5,0,0));

   threevector vertex[4];
   vertex[0]=threevector(0,0,0);
   vertex[1]=threevector(1,0,0);
   vertex[2]=threevector(1,1,0);
   vertex[3]=threevector(0,1,0);
   polygon square(4,vertex);
   square.scale(15);
   square.translate(threevector(-5,-5,0));

   double x,y;
   double min_x=-max_x;
   double min_y=-max_y;
   double zlo=0;
   double zhi=10;
   for (int px=0; px<xyzimage.z2Darray_ptr->get_mdim(); px++)
   {
      for (int py=0; py<xyzimage.z2Darray_ptr->get_ndim(); py++)
      {
         xyzimage.z2Darray_ptr->pixel_to_point(px,py,x,y);
         double z=zlo+(zhi-zlo)*(x-min_x)/(max_x-min_x);
         xyzimage.z2Darray_ptr->put(px,py,z);
      }
   }

   drawfunc::color_polygon_interior(circle,20,xyzimage.z2Darray_ptr);
   drawfunc::color_polygon_interior(square,30,xyzimage.z2Darray_ptr);
   xyzimage.writeimage("polygons",xyzimage.z2Darray_ptr);
   twoDarray* z2Darray_orig_ptr=new twoDarray(xyzimage.z2Darray_ptr);
   xyzimage.z2Darray_ptr->copy(z2Darray_orig_ptr);
   
   ladarfunc::fake_compute_data_bbox(xyzimage.z2Darray_ptr,
      xyzimage.get_data_bbox_ptr());

   twoDarray* xderiv_twoDarray_ptr=new twoDarray(xyzimage.z2Darray_ptr);
   twoDarray* yderiv_twoDarray_ptr=new twoDarray(xyzimage.z2Darray_ptr);

   int max_iters=25;
   double min_distance_to_border=3;
   for (int iter=0; iter<max_iters; iter++)
   {
      xderiv_twoDarray_ptr->initialize_values(ladarimage::null_value);
      yderiv_twoDarray_ptr->initialize_values(ladarimage::null_value);
      
      ladarfunc::compute_x_y_deriv_fields(
         spatial_resolution,xyzimage.get_data_bbox_ptr(),
         xyzimage.z2Darray_ptr,
         xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,min_distance_to_border,
         ladarimage::null_value);

      xyzimage.compute_gradient_magnitude_field(
         xderiv_twoDarray_ptr,yderiv_twoDarray_ptr);
      
      double min_gradmag_threshold=-0.1;
      double max_gradmag_threshold=100;
      xyzimage.compute_gradient_direction_field(
         min_gradmag_threshold,max_gradmag_threshold,
         xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
         xyzimage.get_gradient_mag_twoDarray_ptr());

      groundfunc::erode_strong_gradient_regions(
         10,xyzimage.z2Darray_ptr,xyzimage.get_gradient_mag_twoDarray_ptr(),
         xyzimage.get_gradient_phase_twoDarray_ptr());

      imagefunc::median_filter(3,xyzimage.z2Darray_ptr);

//      if (iter%5==0)
      {
         xyzimage.writeimage(
            "erode"+stringfunc::number_to_string(iter),xyzimage.z2Darray_ptr);
      }
      
      xyzimage.dynamic_colortable=true;
      xyzimage.dynamic_colortable_minz=0;
      xyzimage.dynamic_colortable_maxz=10;
      xyzimage.writeimage(
         "grad_erode"+stringfunc::number_to_string(iter),
         xyzimage.get_gradient_mag_twoDarray_ptr());
      xyzimage.dynamic_colortable=false;
      xyzimage.writeimage(
         "grad_phase"+stringfunc::number_to_string(iter),
         xyzimage.get_gradient_phase_twoDarray_ptr(),false,
         ladarimage::phase_data);
   } // loop over iter index

   delete xderiv_twoDarray_ptr;
   delete yderiv_twoDarray_ptr;

// Subtract original height image from flattened one.  Set to null any
// points in flattened image which differ by more than some small
// height (e.g. 2.5 meters) from original image.  Result should be
// silhouettes in place of buildings.

   const double height_threshold=2.5;
   for (int px=0; px<xyzimage.z2Darray_ptr->get_mdim(); px++)
   {
      for (int py=0; py<xyzimage.z2Darray_ptr->get_ndim(); py++)
      {
         double curr_diff=z2Darray_orig_ptr->get(px,py)-
            xyzimage.z2Darray_ptr->get(px,py);
         if (curr_diff > height_threshold) 
         {
            xyzimage.z2Darray_ptr->put(px,py,ladarimage::null_value);
         }
      }
   }
   xyzimage.writeimage("silhouette",xyzimage.z2Darray_ptr);
   
}
