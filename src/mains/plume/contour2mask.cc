// ========================================================================
// Program CONTOUR2MASK

// 	contour2mask --region_filename ./bundler/devens/Oct24_2011/recon15/packages/photo_0002.pkg 

// ========================================================================
// Last updated on 11/26/11
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "geometry/contour.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "image/TwoDarray.h"
#include "math/twovector.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;
   
   for (int n=0; n<n_photos; n++)
   {
//      int photo_number=17+n;
//      int photo_number=19+n;
      int photo_number=23+n;      

      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      string photo_filename=photo_ptr->get_filename();

      cout << "n = " << n 
           << " photo_filename = " << photo_filename << endl;

// Read in PolyLine contour from text file:

      string contours_subdir="./contours/";
      string contour_filename=contours_subdir+"polylines_2D_"+
         stringfunc::number_to_string(photo_number)+".txt";
      filefunc::ReadInfile(contour_filename);

      vector<threevector> vertices;
      for (int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<double> column_values=stringfunc::string_to_numbers(
            filefunc::text_line[i]);
         vertices.push_back(threevector(column_values[3],column_values[4]));
//      cout << "i = " << i << " u = " << vertices.back().get(0)
//           << " v = " << vertices.back().get(1) << endl;
      }

      contour* contour_ptr=new contour(vertices);
//      cout << "*contour_ptr = " << *contour_ptr << endl;

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         photo_filename,NULL);
      texture_rectangle_ptr->clear_all_RGB_values();

      int width=texture_rectangle_ptr->getWidth();
      int height=texture_rectangle_ptr->getHeight();
      
      twoDarray* ftwoDarray_ptr=new twoDarray(width,height);
      ftwoDarray_ptr->init_coord_system(
         texture_rectangle_ptr->get_minU(),
         texture_rectangle_ptr->get_maxU(),
         texture_rectangle_ptr->get_minV(),
         texture_rectangle_ptr->get_maxV());

      const double znull=0;
      const double zboundary=1.0;
      const double zfill=1.0;

      polygon poly(contour_ptr);
      drawfunc::draw_polygon(poly,zboundary,ftwoDarray_ptr);

      int npixels_filled;
      threevector origin=contour_ptr->robust_locate_origin();
      twoDarray* mask_twoDarray_ptr=graphicsfunc::mask_boundaryFill(
         zfill,znull,origin,ftwoDarray_ptr,npixels_filled);

      int p_step=100;
      double u,v;
      for (int pu=0; pu<width; pu += p_step)
      {
         outputfunc::update_progress_fraction(pu,p_step,width);
         for (int pv=0; pv<height; pv += p_step)
         {
            ftwoDarray_ptr->pixel_to_point(pu,pv,u,v);
            threevector origin(u,v);
            if (!contour_ptr->point_within_contour(origin)) continue;

            graphicsfunc::mask_boundaryFill(
               zfill,znull,origin,ftwoDarray_ptr,mask_twoDarray_ptr,
               npixels_filled);
         }
      }
      
      int n_channels=3;
      texture_rectangle_ptr->initialize_twoDarray_image(
         mask_twoDarray_ptr,n_channels);
    
      string masks_subdir="./masks/";
      string mask_filename=masks_subdir+"mask_"+
         stringfunc::number_to_string(photo_number)+".jpg";
      cout << "mask_filename = " << mask_filename << endl;
      texture_rectangle_ptr->write_curr_frame(mask_filename);
   
      delete ftwoDarray_ptr;
      delete mask_twoDarray_ptr;
      delete contour_ptr;
      delete texture_rectangle_ptr;

   } // loop over index n labeling input photos
   
}
