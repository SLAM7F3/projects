// =======================================================================
// Program AFFINE_ELLIPSES
// =======================================================================
// Last updated on 10/18/13
// =======================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "geometry/contour.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "geometry/geometry_funcs.h"
#include "math/lttriple.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

   string images_subdir="./jpg_files/";
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");

   string ellipses_subdir=images_subdir+"ellipses/";
   filefunc::dircreate(ellipses_subdir);
   
   vector<string> jpg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,images_subdir);

   for (int j=0; j<jpg_filenames.size(); j++)
   {
      string jpg_filename=jpg_filenames[j];
      
      string basename=filefunc::getbasename(jpg_filename);
      string prefix=stringfunc::prefix(jpg_filename);
      string png_filename=prefix+".png";
      string unix_cmd="convert "+jpg_filename+" "+png_filename;
      sysfunc::unix_command(unix_cmd);

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         png_filename,NULL);
      double width=texture_rectangle_ptr->getWidth();
      double height=texture_rectangle_ptr->getHeight();
      double image_area=width*height;

      twoDarray* ptwoDarray_ptr=
         texture_rectangle_ptr->refresh_ptwoDarray_ptr();
      ptwoDarray_ptr->init_coord_system(0,width/height,0,1);
      ptwoDarray_ptr->clear_values();

      string affine_filename="affine.dat";
//   string unix_cmd="h_affine.ln -harlap -i "
//   string unix_cmd="h_affine.ln -heslap -i "
//   string unix_cmd="h_affine.ln -haraff -i "
//   string unix_cmd="h_affine.ln -hessaff -i "
//      +image_filename+" -o "+affine_filename;

//   string unix_cmd="ibr.ln  "
//   string unix_cmd="ebr.ln  "
//      +image_filename+" "+affine_filename;

      unix_cmd="extract_features_64bit.ln -harlap -koen -i "+
//      unix_cmd="extract_features_64bit.ln -harlap -mom -i "+
//   string unix_cmd="extract_features.ln -heslap -mom -i "+
//   string unix_cmd="extract_features.ln -haraff -mom -i "+
//   string unix_cmd="extract_features.ln -hesaff -mom -i "+
//   string unix_cmd="extract_features.ln -harhes -mom -i "+
//   string unix_cmd="extract_features.ln -sedgelap -mom -i "+
         png_filename+" -o1 "+affine_filename;
      unix_cmd += " -harThres 500";
//      unix_cmd += " -harThres 100";
      
      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   
// Import extracted affine region ellipse parameters:

      vector<vector<double> > row_numbers=filefunc::ReadInRowNumbers(
         affine_filename);
   
      cout << "n_ellipses = " << row_numbers.size()-2 << endl;

      int n_nontrivial_ellipses=0;
      double major_axis,minor_axis,major_axis_phi;
      const double min_frac_area=0.01;
      vector<double> frac_areas;

      for (int i=2; i<row_numbers.size(); i++)
      {
         double px_center=row_numbers[i].at(0);
         double py_center=row_numbers[i].at(1);
         double a=row_numbers[i].at(2);
         double b=row_numbers[i].at(3);
         double c=row_numbers[i].at(4);

         geometry_func::quadratic_form_to_ellipse_params(
            a,b,c,major_axis,minor_axis,major_axis_phi);
         double ellipse_area=PI*major_axis*minor_axis;   // pixel**2
         double curr_frac_area=ellipse_area/image_area;

         if (curr_frac_area < min_frac_area) continue;
         frac_areas.push_back(curr_frac_area);

         n_nontrivial_ellipses++;
   
         polygon ellipse=videofunc::generate_ellipse_polygon(
            a,b,c,px_center,py_center,texture_rectangle_ptr);
         contour ellipse_contour(&ellipse);
         triangles_group* triangles_group_ptr=
            ellipse_contour.generate_interior_triangles();

         for (int t=0; t<triangles_group_ptr->get_n_triangles(); t++)
         {
            triangle* triangle_ptr=triangles_group_ptr->get_triangle_ptr(t);
            polygon* triangle_poly_ptr=triangle_ptr->generate_polygon();
            
            double value=1;
            bool accumulate_flag=true;
            drawfunc::color_triangle_interior(
               *triangle_poly_ptr,value,ptwoDarray_ptr,accumulate_flag);

            triangle_ptr->delete_triangle_poly_ptr();
         } // loop over index t labeling triangles
         
      } // loop over index i labeling lines within affine parameter file

// Recolor pixels within *texture_rectangle_ptr which correspond to
// nonzero entries in *ptwoDarray_ptr:

      for (int py=0; py<height; py++)
      {
         for (int px=0; px<width; px++)
         {
            int n_fills=ptwoDarray_ptr->get(px,py);
            if (n_fills==0) continue;

//            cout << "px = " << px << " py = " << py 
//                 << " n_fills = " << n_fills << endl;

            colorfunc::Color curr_color=colorfunc::get_color(n_fills%15);

//            cout << colorfunc::getcolor(n_fills%5) << endl;
            
            colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(curr_color);

            int R=255*curr_RGB.first;
            int G=255*curr_RGB.second;
            int B=255*curr_RGB.third;
            texture_rectangle_ptr->set_pixel_RGB_values(px,py,R,G,B);
         } // loop over px index
      } // loop over py index
      
      unix_cmd="/bin/rm "+png_filename;
      sysfunc::unix_command(unix_cmd);

      cout << "n_nontrivial_ellipses = " << n_nontrivial_ellipses << endl;

      prob_distribution ellipse_area_prob(frac_areas,100,0);
      ellipse_area_prob.writeprobdists(false);

      string ellipse_image_filename=ellipses_subdir+"ellipse_"+basename;
      texture_rectangle_ptr->write_curr_frame(ellipse_image_filename);

      string banner="Exported ellipses to "+ellipse_image_filename;
      outputfunc::write_big_banner(banner);

      delete texture_rectangle_ptr;
   } // loop over index j labeling ellipses
   


}


