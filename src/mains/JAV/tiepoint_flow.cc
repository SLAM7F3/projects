// ==========================================================================
// Program TIEPOINT_FLOW is a playground for identifying sharp
// temporal discontinuities, camera movements and stable segments
// within video clips.  It imports SIFT/SURF tiepoints calculated
// between pairs of adjacent video frames.  If very few tiepoint
// exists, a temporal discontinuity is declared.  If nontrivial flow
// among all tiepoints between two frames is detected, camera movement
// is declared.  Otherwise, a video frame is declared stable.

// TIEPOINT_FLOW also superposes line segments onto video frames which
// display SURF tiepoint flows with following frames.  When the
// annotated frames are assembled into a movie, it's easy for a human
// eye to see gross camera and foreground object movements.

//				./tiepoint_flow

// ==========================================================================
// Last updated on 11/19/13; 11/20/13; 11/21/13
// ==========================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "geometry/affine_transform.h"
#include "general/filefuncs.h"
#include "geometry/linesegment.h"
#include "math/mathfuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "plot/metafile.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "math/twovector.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   timefunc::initialize_timeofday_clock();      

   string ImageEngine_subdir="/data/ImageEngine/";

//   string root_subdir=ImageEngine_subdir+"NewsWrap/";
//   string root_subdir=ImageEngine_subdir+
//      "BostonBombing/Nightline_YouTube2/transcripted/";
//   string root_subdir=
//      "/data/ImageEngine/BostonBombing/clips_1_thru_133/clip34/";
//   string root_subdir=ImageEngine_subdir+"BostonBombing/clip3/";
//   string root_subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/korea/NK/ground_videos/NorthKorea/";
//   string images_subdir=root_subdir;
//   string images_subdir="/data/ImageEngine/BostonBombing/clips_1_thru_133/";


//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
   string tiepoints_subdir=root_subdir+"tiepoints/";
   string tiepoint_flows_subdir=tiepoints_subdir+"flows/";
   filefunc::dircreate(tiepoint_flows_subdir);

   string global_colors_subdir=root_subdir+"global_color_histograms/";
   string LBPs_subdir=root_subdir+"LBP_histograms/";

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* text_texture_rectangle_ptr=new texture_rectangle();

// Import global color histogram matching scores for successive video
// frames:

   typedef map<string,double> GLOBAL_COLORS_MAP;
   GLOBAL_COLORS_MAP global_colors_map;
   GLOBAL_COLORS_MAP::iterator global_colors_iter;
   
// independent string = video frame prefix
// dependent double = global color matching score for temporally adjacent
//    video frames

   string global_colors_comparison_filename=global_colors_subdir+
      "image_global_colors_adjacent_frames.comparison";
   vector<vector<string> > all_substrings=filefunc::ReadInSubstrings(
      global_colors_comparison_filename);
   for (int a=0; a<all_substrings.size(); a++)
   {
      double matching_score=stringfunc::string_to_number(
         all_substrings[a].at(0));
      string prefix1=all_substrings[a].at(1);
      string prefix2=all_substrings[a].at(2);

      string separator_chars="_-";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         prefix1,separator_chars);
      int prefix1_clip_ID=stringfunc::string_to_number(substrings[1]);
      int prefix1_frame_ID=stringfunc::string_to_number(substrings[3]);

      substrings=stringfunc::decompose_string_into_substrings(
         prefix2,separator_chars);
      int prefix2_clip_ID=stringfunc::string_to_number(substrings[1]);
      int prefix2_frame_ID=stringfunc::string_to_number(substrings[3]);

      if (prefix2_frame_ID-prefix1_frame_ID != 1) continue;

      global_colors_map[prefix1]=matching_score;
//      global_colors_map[prefix2]=matching_score;

   } // loop over index a labeling all_substrings
   cout << "global_colors_map.size() = " << global_colors_map.size()
        << endl;

// Import LBP matching scores for successive video frames:

   typedef map<string,double> LBPs_MAP;
   LBPs_MAP LBPs_map;
   LBPs_MAP::iterator LBPs_iter;
   
// independent string = video frame prefix
// dependent double = LBP matching score for temporally adjacent
//    video frames

   string separator_chars="_-";
   string LBPs_comparison_filename=LBPs_subdir+
      "image_LBPs_adjacent_frames.comparison";
   all_substrings.clear();
   all_substrings=filefunc::ReadInSubstrings(LBPs_comparison_filename);
   for (int a=0; a<all_substrings.size(); a++)
   {
      double matching_score=stringfunc::string_to_number(
         all_substrings[a].at(0));
      string prefix1=all_substrings[a].at(1);
      string prefix2=all_substrings[a].at(2);

      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         prefix1,separator_chars);
      int prefix1_clip_ID=stringfunc::string_to_number(substrings[1]);
      int prefix1_frame_ID=stringfunc::string_to_number(substrings[3]);

      substrings=stringfunc::decompose_string_into_substrings(
         prefix2,separator_chars);
      int prefix2_clip_ID=stringfunc::string_to_number(substrings[1]);
      int prefix2_frame_ID=stringfunc::string_to_number(substrings[3]);

      if (prefix2_frame_ID-prefix1_frame_ID != 1) continue;

      LBPs_map[prefix1]=matching_score;
//      LBPs_map[prefix2]=matching_score;

   } // loop over index a labeling all_substrings
   cout << "LBPs_map.size() = " << LBPs_map.size() << endl;

// Classify video frames' temporal states based upon SIFT feature
// global color and LBP matches:

   string output_filename=root_subdir+"video_camera.state";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# video_frame_prefix temporal_state n_tiepoints global_color_matching LBP_matching delta_rho" << endl << endl;

// Import SIFT/SURF tiepoints between successive video frames:

   vector<string> videoframe_filenames=filefunc::image_files_in_subdir(
      images_subdir);
   vector<linesegment> SURF_flow_segments;
   vector<twovector> curr_SURF_centers,next_SURF_centers;

   int t_start=0;
   int t_stop=videoframe_filenames.size();
//   int t_start=2241;	// clip 27 frame 1
//   int t_stop=t_start+200;	// clip 27 frame 200

   for (int t=t_start; t<t_stop; t++)
   {
      string curr_prefix=filefunc::getprefix(videoframe_filenames[t]);
      string SURF_flow_segments_filename=
         tiepoint_flows_subdir+curr_prefix+".jpg";

      cout << "=====================================================" << endl;
      cout << "Analyzing " << curr_prefix << endl << endl;


      if (t%100==0)
      {
         double progress_frac=double(t)/videoframe_filenames.size();
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         curr_prefix,separator_chars);
      int curr_framenumber=stringfunc::string_to_number(substrings[3]);
      int next_framenumber=curr_framenumber+1;

// Check if a global color matching score exists for temporally
// adjacent image pair labeled by curr_prefix:

      global_colors_iter=global_colors_map.find(curr_prefix);
      double global_color_matching_score=-1;
      if (global_colors_iter != global_colors_map.end())
      {
         global_color_matching_score=global_colors_iter->second;
      }

// Check if a LBP color matching score exists for temporally
// adjacent image pair labeled by curr_prefix:

      LBPs_iter=LBPs_map.find(curr_prefix);
      double LBP_matching_score=-1;
      if (LBPs_iter != LBPs_map.end())
      {
         LBP_matching_score=LBPs_iter->second;
      }

// Check whether tiepoints subdirectory corresponding to curr and next
// video frames exists.  If not, very few or no tiepoint matches
// between adjacent frames were found:

      string tiepoint_subdir_name=tiepoints_subdir+"tiepoints_"+
         stringfunc::integer_to_string(t,4)+"_"+
         stringfunc::integer_to_string(t+1,4)+"/";

/*
      cout << "t = " << t << " curr_prefix = " << curr_prefix << endl;
      cout << "curr_framenumber = " << curr_framenumber 
           << " next_framenumber = " << next_framenumber << endl;
      cout << "tiepoint_subdir = " << tiepoint_subdir_name << endl;
      cout << "global color matching score = "
           << global_color_matching_score 
           << " LBP matching score = " 
           << LBP_matching_score
           << endl;
*/

      int n_tiepoints=0;
      string curr_state="";
      if (!filefunc::direxist(tiepoint_subdir_name))
      {
         if (global_color_matching_score < 0.51 ||
         (global_color_matching_score < 0.6 && LBP_matching_score < 0.65) )
         {
            curr_state="temporal discontinuity";
         }
         else
         {
            curr_state="dynamic scene/abrupt change";
         }
         outstream << curr_prefix << "  "
                   << curr_state << "  "
                   << n_tiepoints << "  "
                   << global_color_matching_score << "  "
                   << LBP_matching_score 
                   << endl;
//         string unix_cmd="ln -s "+videoframe_filenames[t]+" "
//            +SURF_flow_segments_filename;
//         sysfunc::unix_command(unix_cmd);
      }

      vector<string> feature_filenames;
      vector< vector<double> > curr_row_numbers,next_row_numbers;
      if (curr_state.size()==0)
      {
         feature_filenames=filefunc::files_in_subdir_matching_substring(
            tiepoint_subdir_name,"features_2D_");
         curr_row_numbers=
            filefunc::ReadInRowNumbers(feature_filenames[0]);
         next_row_numbers=
            filefunc::ReadInRowNumbers(feature_filenames[1]);

      
// Even if some tiepoints exist between the current and next frames,
// declare a temporal discontinuity or dynamic scene content if their
// number is small:

         n_tiepoints=curr_row_numbers.size();
         if (n_tiepoints < 13 ||
         (n_tiepoints < 35 && global_color_matching_score < 0.4) ||
         (n_tiepoints < 20 && global_color_matching_score < 0.45 &&
         LBP_matching_score < 0.55) )
         {
            if (global_color_matching_score < 0.51 ||
            (global_color_matching_score < 0.6 && LBP_matching_score < 0.65) )
            {
               curr_state="temporal discontinuity";
            }
            else
            {
               curr_state="dynamic scene/abrupt change";
            }
            outstream << curr_prefix << "  "
                      << curr_state << "  "
                      << n_tiepoints << "  "
                      << global_color_matching_score << "  "
                      << LBP_matching_score 
                      << endl;
//            string unix_cmd="ln -s "+videoframe_filenames[t]+" "
//               +SURF_flow_segments_filename;
//            sysfunc::unix_command(unix_cmd);
         }
      } // curr_state.size()==0 conditional

      if (curr_state.size()==0)
      {

         const double moved_feature_delta_rho=0.005;

         vector<double> delta_rhos,thetas;
         curr_SURF_centers.clear();
         next_SURF_centers.clear();

         int n_moved_tiepoints=0;
         SURF_flow_segments.clear();
         for (int r=0; r<n_tiepoints; r++)
         {
            double curr_U=curr_row_numbers[r].at(3);
            double curr_V=curr_row_numbers[r].at(4);
            double next_U=next_row_numbers[r].at(3);
            double next_V=next_row_numbers[r].at(4);

            twovector curr_center(curr_U,curr_V);
            curr_SURF_centers.push_back(curr_center);
            twovector next_center(next_U,next_V);
            next_SURF_centers.push_back(next_center);

//         cout << curr_U << " " << curr_V << " " 
//              << next_U << " " << next_V << endl;

            double dU=next_U-curr_U;
            double dV=next_V-curr_V;
            double d_rho=sqrt(dU*dU+dV*dV);
            if (d_rho > moved_feature_delta_rho)
            {
               n_moved_tiepoints++;
               linesegment flow_segment(
                  threevector(curr_U,curr_V),threevector(next_U,next_V));
               SURF_flow_segments.push_back(flow_segment);
            }
         
            double theta=atan2(dV,dU)*180/PI;
            delta_rhos.push_back(d_rho);
            thetas.push_back(theta);
         } // loop over index r labeling SURF tiepoints
         double moved_tiepoints_frac=double(n_moved_tiepoints)/n_tiepoints;

// Fit affine transformation between current and next SURF tiepoints:

         affine_transform* affine_transform_ptr=new affine_transform();
         affine_transform_ptr->parse_affine_transform_inputs(
            curr_SURF_centers,next_SURF_centers);
         affine_transform_ptr->fit_affine_transformation();
         double RMS_residual=
            affine_transform_ptr->check_affine_transformation(
               curr_SURF_centers,next_SURF_centers);
//      cout << "RMS_residual = " << RMS_residual << endl;

         genmatrix* A_ptr=affine_transform_ptr->get_A_ptr();
         twovector trans=affine_transform_ptr->get_trans();
//      cout << "trans = " << trans << endl;
//      cout << "A = " << *A_ptr << endl;

         genmatrix U(2,2),W(2,2),V(2,2);
         A_ptr->sorted_singular_value_decomposition(U,W,V);

         if (U.determinant() < 0 && V.determinant() < 0)
         {
            genmatrix Unew(2,2),Wnew(2,2),Vnew(2,2);

            Unew.put(0,0,U.get(0,1));
            Unew.put(1,0,U.get(1,1));
            Unew.put(0,1,U.get(0,0));
            Unew.put(1,1,U.get(1,0));

            Vnew.put(0,0,V.get(0,1));
            Vnew.put(1,0,V.get(1,1));
            Vnew.put(0,1,V.get(0,0));
            Vnew.put(1,1,V.get(1,0));

            Wnew.put(0,0,W.get(1,1));
            Wnew.put(1,1,W.get(0,0));
            Wnew.put(0,1,0);
            Wnew.put(1,0,0);

//         cout << "Unew = " << Unew << endl;
//         cout << "Wnew = " << Wnew << endl;
//         cout << "Vnew = " << Vnew << endl;
//         cout << "A - Unew * Wnew * Vnew.trans = "
//              << *A_ptr - Unew * Wnew * Vnew.transpose() << endl;

            U=Unew;
            V=Vnew;
            W=Wnew;
         }

/*
  cout << "U = " << U << endl;
  cout << "W = " << W << endl;
  cout << "V = " << V << endl;

  cout << "U*Utrans = " << U*U.transpose() << endl;
  cout << "V*Vtrans = " << V*V.transpose() << endl;

  cout << "detU = " << U.determinant() << endl;
  cout << "detV = " << V.determinant() << endl;

  cout << "A - U*W*Vtrans = " 
  << *A_ptr - U*W*V.transpose() << endl;
*/

         double cos_Utheta=U.get(0,0);
         double sin_Utheta=U.get(1,0);
         double Utheta=atan2(sin_Utheta,cos_Utheta);

         double cos_Vtheta=V.get(0,0);
         double sin_Vtheta=V.get(1,0);
         double Vtheta=atan2(sin_Vtheta,cos_Vtheta);

//      cout << "Utheta = " << Utheta*180/PI << endl;
//      cout << "Vtheta = " << Vtheta*180/PI << endl;
         double delta_theta=(Utheta-Vtheta)*180/PI;
         double scale1=W.get(0,0);
         double scale2=W.get(1,1);
         double trans_mag=trans.magnitude();

         cout << "trans_mag = " << trans_mag << endl;
         cout << "delta_theta = " << delta_theta << " degs" << endl;
         cout << "scale1 = " << scale1 << endl;
         cout << "scale2 = " << scale2 << endl;

         if (moved_tiepoints_frac < 0.5)
         {
            curr_state="stable";
         }
         else 
         {
            if (scale1 > 1.05 || scale2 > 1.05)
            {
               curr_state="zoom in/forward movement";
            }
            else if (scale1 < 0.95 || scale2 < 0.95)
            {
               curr_state="zoom out/backward movememnt";
            }
            else
            {
               curr_state="rotation/translation";
            }
         } // moved_tiepoints_frac conditional
         delete affine_transform_ptr;
      } // curr_state.size()==0 conditional
      
// Superpose line segments onto current video frame which displays
// SURF tiepoint flows with next video frame:

      bool export_annotated_frames_flag=true;
//      bool export_annotated_frames_flag=false;
      if (export_annotated_frames_flag)
      {
         texture_rectangle_ptr->reset_texture_content(videoframe_filenames[t]);

         if (SURF_flow_segments.size() > 0)
         {
            videofunc::draw_line_segments(
               SURF_flow_segments,texture_rectangle_ptr,colorfunc::red);
         }
         
         if (curr_SURF_centers.size() > 0)
         {
            int square_length_in_pixels=3;
            videofunc::draw_solid_squares(
               curr_SURF_centers,square_length_in_pixels,texture_rectangle_ptr,
               colorfunc::brightyellow);
         }
         
         vector<string> text_lines;
         text_lines.push_back(curr_state);
         cout << "curr_state = " << curr_state << endl;

         vector<twovector> xy_start;
         xy_start.push_back(twovector(0,0));
         vector<colorfunc::Color> text_colors;
         text_colors.push_back(colorfunc::brightpurple);

         int fontsize=30;
         videofunc::annotate_image_with_text(
            texture_rectangle_ptr,
            text_texture_rectangle_ptr,fontsize,text_lines,
            xy_start,text_colors);

         text_texture_rectangle_ptr->write_curr_frame(
            SURF_flow_segments_filename);
      }
/*
      double delta_rho_25,delta_rho_75;
      mathfunc::lo_hi_values(
         delta_rhos,delta_rho_25,delta_rho_75);

      double median_theta,quartile_width_theta;
      mathfunc::median_value_and_quartile_width(
         thetas,median_theta,quartile_width_theta);

      double median_delta_rho,quartile_width_delta_rho;
      mathfunc::median_value_and_quartile_width(
         delta_rhos,median_delta_rho,quartile_width_delta_rho);
//      cout << "median_delta_rho = " << median_delta_rho
//           << " quartile_width_delta_rho = "
//           << quartile_width_delta_rho << endl;
*/    

/*
      const double stable_camera_delta_rho=0.020;      
      if (delta_rho_25 > stable_camera_delta_rho)
      {
         curr_state="camera movement";
         if (quartile_width_theta < 10) 
         {
            curr_state="camera rotation";
         }
         else if (quartile_width_theta > 20 && quartile_width_theta < 160)
         {
            curr_state="camera zoom";
         }
         
         outstream << curr_prefix << "  "
                   << curr_state << "  "
                   << n_tiepoints << "  "
                   << global_color_matching_score << "  "
                   << LBP_matching_score << "  "
                   << "drho_25=" << delta_rho_25 << "  "
                   << "drho_75=" << delta_rho_75 << "  "
                   << " theta=" << median_theta << " +/- " 
                   << quartile_width_theta 
                   << endl;
      }
      else
      {
         curr_state="camera stable";
      }
*/

   } // loop over index t labeling input video frames

   filefunc::closefile(output_filename,outstream);
   string banner="Exported "+output_filename;
   outputfunc::write_big_banner(banner);

   delete texture_rectangle_ptr;
   delete text_texture_rectangle_ptr;

   cout << "At end of program TIEPOINT_FLOW" << endl;
   outputfunc::print_elapsed_time();
}

