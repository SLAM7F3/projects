// =======================================================================
// Program PLOT_AFFINE_ELLIPSES imports a set of JPG files from a
// specified subdirectory.  Looping over each image, this program
// extracts interest points via the Affine Covariant Region Detector
// linux binary (see
// http://www.robots.ox.ac.uk/~vgg/research/affine/detectors.html).
// Parameters associated with the elliptical affine covariant regions
// are written to an output text file.  Ellipses corresponding to each
// affine region are also superposed on the input image and exported
// to ellipses_subdir.
// =======================================================================
// Last updated on 10/18/13; 10/19/13; 10/21/13; 10/22/13
// =======================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "geometry/geometry_funcs.h"
#include "math/lttriple.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
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

   timefunc::initialize_timeofday_clock();      

//   string root_subdir="./jpg_files/";
//   string images_subdir=root_subdir;

//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";

/*
   string ImageEngine_subdir="/data/ImageEngine/";
   string tidmarsh_subdir=ImageEngine_subdir+"tidmarsh/";
   string root_subdir=tidmarsh_subdir;
   string images_subdir=root_subdir;
   bool video_clip_data_flag=false;

*/

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");

   string ellipses_subdir=root_subdir+"ellipses/";
//   string ellipses_subdir=images_subdir+"ellipses/";
   filefunc::dircreate(ellipses_subdir);
   
   vector<string> jpg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,images_subdir);

   int j_start=0;
   int j_stop=jpg_filenames.size()-1;
   for (int j=j_start; j<=j_stop; j++)
   {
      double progress_frac=double(j-j_start)/double(j_stop-j_start);

      outputfunc::print_elapsed_time();
      outputfunc::print_remaining_time(progress_frac);

      string jpg_filename=jpg_filenames[j];
      string basename=filefunc::getbasename(jpg_filename);
      string prefix=stringfunc::prefix(jpg_filename);
      string png_filename=prefix+".png";
//      string unix_cmd="convert "+jpg_filename+" "+png_filename;

// Resize image so that its maximum pixel dimension does not exceed
// 1000:

      string unix_cmd="convert "+jpg_filename+" -resize 1000x1000\\> "
         +png_filename;
//      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         png_filename,NULL);
   
      double width=texture_rectangle_ptr->getWidth();
      double height=texture_rectangle_ptr->getHeight();
      double image_area=width*height;

      string affine_filename="affine.dat";
//   string unix_cmd="h_affine.ln -harlap -i "
//   string unix_cmd="h_affine.ln -heslap -i "
//   string unix_cmd="h_affine.ln -haraff -i "
//   string unix_cmd="h_affine.ln -hessaff -i "
//      +image_filename+" -o "+affine_filename;

//   string unix_cmd="ibr.ln  "
//   string unix_cmd="ebr.ln  "
//      +image_filename+" "+affine_filename;

      unix_cmd="extract_features.ln -harlap -mom -i "+
//   string unix_cmd="extract_features.ln -heslap -mom -i "+
//   string unix_cmd="extract_features.ln -haraff -mom -i "+
//   string unix_cmd="extract_features.ln -hesaff -mom -i "+
//   string unix_cmd="extract_features.ln -harhes -mom -i "+
//   string unix_cmd="extract_features.ln -sedgelap -mom -i "+
         png_filename+" -o1 "+affine_filename;
      cout << "unix_cmd = " << unix_cmd << endl;

/*
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
*/

      sysfunc::unix_command(unix_cmd);
   
// Import extract affine region ellipse parameters:

      filefunc::ReadInfile(affine_filename);
      vector<vector<double> > row_numbers=filefunc::ReadInRowNumbers(
         affine_filename);
   
      cout << "n_ellipses = " << row_numbers.size()-2 << endl;

// Save ellipse parameters into text file output:

      string ellipse_filename=ellipses_subdir+stringfunc::prefix(basename)+
         ".ellipse_params";
      ofstream ellipse_stream;
      filefunc::openfile(ellipse_filename,ellipse_stream);
      ellipse_stream << "# px_center py_center a b c major_axis minor_axis frac_area" << endl << endl;

      int n_nontrivial_ellipses=0;
      double major_axis,minor_axis,major_axis_phi;
      const double min_frac_area=0.0033;
//      const double min_frac_area=0.01;
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

         ellipse_stream 
            << px_center << " "
            << py_center << " "
            << a << " "
            << b << " "
            << c << " "
            << major_axis << " "
            << minor_axis << " "
            << curr_frac_area << " "
            << endl;
         
         int color_index=i%15;
         int line_thickness=1;
         videofunc::draw_ellipse(
            a,b,c,px_center,py_center,texture_rectangle_ptr,
            color_index,line_thickness);
         n_nontrivial_ellipses++;
   
      } // loop over index i labeling lines within ellipse parameter file

      filefunc::closefile(ellipse_filename,ellipse_stream);

      unix_cmd="/bin/rm "+png_filename;
      sysfunc::unix_command(unix_cmd);

      cout << "n_nontrivial_ellipses = " << n_nontrivial_ellipses << endl;

//      prob_distribution ellipse_area_prob(frac_areas,100,0);
//      ellipse_area_prob.writeprobdists(false);

      string ellipse_image_filename=ellipses_subdir+"ellipse_"+basename;
      texture_rectangle_ptr->write_curr_frame(ellipse_image_filename);

      string banner="Exported ellipses to "+ellipse_image_filename;
      outputfunc::write_big_banner(banner);

      delete texture_rectangle_ptr;
   } // loop over index j labeling input jpg images

   cout << "At end of program PLOT_AFFINE_ELLIPSES" << endl;
   outputfunc::print_elapsed_time();
}


