// ==========================================================================
// Program EXTRACT_HORIZON_SEGMENTS loops over input DIME WISP
// panoramas.  For each image, it runs the line segment detector
// algorithm/code of von Gioi (version 1.6 Nov 2011) on input DIME WISP
// panoramas.  This variant of program LSD bins detected line
// segments into (segment azimuthal orientation angle,impact
// parameter) cells.  Within each cell, EXTRACT_HORIZON_SEGMENTS tries
// to concatenate line segments into longer "super" segments.  The
// long segments are exported to an output text file.

//			./extract_horizon_segments

// ==========================================================================
// Last updated on 7/11/13; 7/14/13; 8/8/13; 8/12/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "geometry/linesegment.h"
#include "math/lttwovector.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;

// On Dunmeyer's Ubuntu 12.4 box:

   string date_string="05202013";
   cout << "Enter date string (e.g. 05202013 or 05222013):" << endl;
   cin >> date_string;
   filefunc::add_trailing_dir_slash(date_string);

   string MayFieldtest_subdir=
      "/data/DIME/panoramas/May2013_Fieldtest/";
//   string FSFdate_subdir=MayFieldtest_subdir+"05202013/";
   string FSFdate_subdir=MayFieldtest_subdir+date_string;
   cout << "FSFdate_subdir = " << FSFdate_subdir << endl;

   int scene_ID;
   cout << "Enter scene ID:" << endl;
   cin >> scene_ID;
   string scene_ID_str=stringfunc::integer_to_string(scene_ID,2);
   string panos_subdir=FSFdate_subdir+"Scene"+scene_ID_str+"/";
   cout << "panos_subdir = " << panos_subdir << endl;

   string raw_images_subdir=panos_subdir+"raw/";
   string horizons_subdir=panos_subdir+"horizons/";
   filefunc::dircreate(horizons_subdir);
   string linesegments_subdir=horizons_subdir+"linesegments_files/";
   filefunc::dircreate(linesegments_subdir);

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      raw_images_subdir);
   timefunc::initialize_timeofday_clock();

   cout << "image_filenames.size() = " << image_filenames.size() << endl;

   int iter_start=0;
   cout << "Enter starting image number:" << endl;
   cin >> iter_start;
   
   int iter_step=1;
//   int iter_step=25;
   int iter_stop=image_filenames.size()-1;
   for (int iter=iter_start; iter<iter_stop; iter += iter_step)
   {
      double progress_frac=double(iter-iter_start)/
         double(iter_stop-iter_start);
      outputfunc::print_elapsed_and_remaining_time(progress_frac);

      string image_filename=image_filenames[iter];
//      cout << "image_filename = " << image_filename << endl;

      string basename=filefunc::getbasename(image_filename);
//      cout << "basename = " << basename << endl;
      string separator_chars="_.";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         basename,separator_chars);
   
      string framenumber_str=substrings[2];
//      cout << "framenumber_str = " << framenumber_str << endl;
      string banner="Importing WISP panorama image from "+basename;
      outputfunc::write_banner(banner);

// Initialize output line segments text file:

      string linesegments_filename=
         linesegments_subdir+"linesegments_"+framenumber_str+".dat";
      cout << "linesegments_filename = " << linesegments_filename << endl;
      ofstream segments_stream;
      filefunc::openfile(linesegments_filename,segments_stream);
      segments_stream << "# Segment ID  U0  V0    U1     V1   phi (degs)" 
                      << endl;
      segments_stream << endl;

// As of Mon Jun 17, 2013, we first subsample input FSF WISP images:

      string unix_cmd="convert "+image_filename+" -resize 50% wisp_small.jpg";
      sysfunc::unix_command(unix_cmd);

      for (int edge_weight=3; edge_weight <=4; edge_weight++)
      {

// Run ImageMagick's edge filter on subsampled FSF images:

         image_filename="edges.jpg";
//      unix_cmd="convert -edge 3 wisp_small.jpg "+image_filename;
//         unix_cmd="convert -edge 4 wisp_small.jpg "+image_filename;
         unix_cmd="convert -edge "+stringfunc::number_to_string(
            edge_weight)+" wisp_small.jpg "+image_filename;
         sysfunc::unix_command(unix_cmd);

         texture_rectangle* grey_texture_rectangle_ptr=
            new texture_rectangle(image_filename,NULL);
         int width=grey_texture_rectangle_ptr->getWidth();
         int height=grey_texture_rectangle_ptr->getHeight();
         double Umax=double(width)/double(height);
         double Vmax=1;
         cout << "Umax = " << Umax << " Vmax = " << Vmax << endl;

         int n_channels=grey_texture_rectangle_ptr->getNchannels();
         cout << "n_channels = " << n_channels << endl;

// Run von Gioi's line segment detector algorithm and generate initial
// raw set of line segment candidates for input image:

         vector<linesegment> line_segments=
            videofunc::detect_line_segments(grey_texture_rectangle_ptr);
         cout << "n_segments = " << line_segments.size() << endl;

// Instantiate STL map *phi_r_segments_map_ptr and
// *parallel_segments_map_ptr which hold vectors of linesegments
// functions of quantized integer indices for impact parameter r from
// origin (U,V)=(0,0) and azimuthal linesegment orientation angle phi:

         double phi_start=0;
         double phi_stop=PI;
         double d_phi=1.5*PI/180;
//   double d_phi=3*PI/180;
         int n_phi=(phi_stop-phi_start)/d_phi;

         double r_start=0;
         double r_stop=basic_math::max(Umax,Vmax);
         double d_r=5.0/basic_math::min(width,height);
         int n_r=(r_stop-r_start)/d_r;

         twoDarray* r_phi_twoDarray_ptr=new twoDarray(n_phi,n_r);
         r_phi_twoDarray_ptr->init_coord_system(
            phi_start,phi_stop,r_start,r_stop);

         typedef std::map<twovector,vector<linesegment>,lttwovector > 
            PHI_R_SEGMENTS_MAP;
         PHI_R_SEGMENTS_MAP* phi_r_segments_map_ptr=new PHI_R_SEGMENTS_MAP;
         // *phi_r_segment_map_ptr independent var = (px_phi,py_r)
         // dependent var = vector<linesegment>

         vector<linesegment> orig_line_segments,long_orig_segments;
         for (unsigned int l=0; l<line_segments.size(); l++)
         {
            linesegment curr_l(line_segments[l]);
            threevector e_hat=curr_l.get_ehat();
            vector<double> abc=curr_l.compute_2D_line_coeffs();
            double a=abc[0];
            double b=abc[1];
            double c=abc[2];
      
            if (sgn(c) < 0) 
            {
               a *= -1;
               b *= -1;
               c *= -1;
            }

            double phi=atan2(e_hat.get(1),e_hat.get(0));
            double r=1/sqrt(a*a+b*b);

//      cout << "e_hat = " << e_hat << endl;
//      cout << "phi = " << phi*180/PI << endl;

            if (phi < 0) phi += PI;
//      cout << " restricted phi = " << phi*180/PI << endl;
//      cout << "r = " << r << endl;where
            
            unsigned int px,py;
//      cout << "r_phi_twoDarray = " << *r_phi_twoDarray_ptr << endl;
      
            if (r_phi_twoDarray_ptr->point_to_pixel(phi,r,px,py))
            {
//         cout << "px = " << px << " py = " << py << endl;
               twovector phi_r_indices(px,py);
               PHI_R_SEGMENTS_MAP::iterator iter=phi_r_segments_map_ptr->find(
                  phi_r_indices);
               if (iter==phi_r_segments_map_ptr->end()) 
               {
                  vector<linesegment> V;
                  V.push_back(curr_l);
                  (*phi_r_segments_map_ptr)[phi_r_indices]=V;
               }
               else
               {
                  iter->second.push_back(curr_l);
               }
            }
            else
            {
//         cout << "Point lies outside twoDarray bounds!" << endl;
//         cout << "phi = " << phi*180/PI << " r = " << r << endl;
            }

//         cout << "l = " << l << " phi_r_segments_map_ptr->size() = "
//              << phi_r_segments_map_ptr->size() << endl;
//         outputfunc::enter_continue_char();

//      const double min_length=0.01;	// UV space
            const double min_length=0.05;	// UV space
            if (curr_l.get_length() > min_length)
            {
               long_orig_segments.push_back(curr_l);
            }
            orig_line_segments.push_back(curr_l);
         }
         delete r_phi_twoDarray_ptr;

         cout << "Number original line segments = " 
              << orig_line_segments.size() << endl;
         cout << "Number long segments = " << long_orig_segments.size()
              << endl;
   
         delete grey_texture_rectangle_ptr;

         for (unsigned int i=0; i<long_orig_segments.size(); i++)
         {
            linesegment curr_segment=long_orig_segments[i];
            threevector V1=curr_segment.get_v1();
            threevector V2=curr_segment.get_v2();
            double phi=atan2(V2.get(1)-V1.get(1) , V2.get(0)-V1.get(0) );

            segments_stream << i << "  " 
                            << V1.get(0) << "  "
                            << V1.get(1) << "  "
                            << V2.get(0) << "  "
                            << V2.get(1) << "  "
                            << phi*180/PI << endl;
         } // loop over index i labeling long orig segments

      } // loop over edge_weight index
   
      filefunc::closefile(linesegments_filename,segments_stream);

      banner="Exported long segments to "+linesegments_filename;
      outputfunc::write_big_banner(banner);

   } // loop over iter index

   string banner="Finished running program EXTRACT_HORIZON_SEGMENTS";
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();   
}
