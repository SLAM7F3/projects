// ==========================================================================
// Program TRAIN_IMAGE_COMPARISONS imports comparison scores for
// pairs of images based upon their color histogram dotproducts, gist
// descriptor difference magnitudes and texture histogram dotproducts.  
// It first stores these measures of image similarity within an STL
// map which depends upon first and second clip IDS and frame IDs.
// Iterating over a subset of the entries within the STL map,
// TRAIN_IMAGE_COMPARISONS then displays a pair of images and asks
// the user to label them as either matching or nonmatching.  The
// triple of global image descriptors is then exported to either a
// matching or non-matching text file.  The labeled triple descriptors
// can later be used to generate binary classified and/or rank SVM
// functions.

//		          ./train_image_comparisons

// ==========================================================================
// Last updated on 11/12/13; 11/20/13; 12/11/13
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "video/descriptorfuncs.h"
#include "image/imagefuncs.h"
#include "math/ltquadruple.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "datastructures/Quadruple.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   timefunc::initialize_timeofday_clock();      
   nrfunc::init_time_based_seed();

//   int n_descriptors=1;
//   int n_descriptors=3;
//   int n_descriptors=4;
//   int n_descriptors=7;
//   int n_descriptors=11;
   int n_descriptors=12;

   typedef std::map<quadruple,genvector*,ltquadruple> 
      GLOBAL_DESCRIPTOR_SCORES_MAP;

// independent quadruple holds first clip ID, first frame ID, second
// clip ID and second frame ID
// dependent fourvector holds color, GIST, texture, correlation and
// LBP scores

   GLOBAL_DESCRIPTOR_SCORES_MAP global_descriptor_scores_map;
   GLOBAL_DESCRIPTOR_SCORES_MAP::iterator iter;


//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
   bool video_clip_data_flag=true;
   bool keyframes_flag=true;
//   bool keyframes_flag=false;


/*
   string ImageEngine_subdir="/data/ImageEngine/";
   string tidmarsh_subdir=ImageEngine_subdir+"tidmarsh/";
   string images_subdir=tidmarsh_subdir;
   string root_subdir=tidmarsh_subdir;
   string ellipse_histograms_subdir=root_subdir+"ellipses/";
   bool video_clip_data_flag=false;
*/

   string global_color_histograms_subdir=
      root_subdir+"global_color_histograms/";
   string gist_histograms_subdir=root_subdir+"gist_files/";
   string texture_histograms_subdir=root_subdir+"texture_histograms/";
   string SURF_histograms_subdir=root_subdir+"SURF_keys/";
   string LBP_histograms_subdir=root_subdir+"LBP_histograms/";
   string segmentation_histograms_subdir=root_subdir+"segmentation/metafiles/";
   string faces_subdir=root_subdir+"faces/";

   string SURF_scale_histograms_subdir=SURF_histograms_subdir+
      "metafiles/scale/cumulative/";
   string SURF_angle_histograms_subdir=SURF_histograms_subdir+
      "metafiles/angle/cumulative/";
   string SURF_separation_histograms_subdir=SURF_histograms_subdir+
      "metafiles/separation/cumulative/";

   string segmentation_histograms_frac_area_subdir=
      segmentation_histograms_subdir+"frac_area/";
   string segmentation_histograms_Iuu_subdir=
      segmentation_histograms_subdir+"Iuu/";
   string segmentation_histograms_Iuv_subdir=
      segmentation_histograms_subdir+"Iuv/";
   string segmentation_histograms_Ivv_subdir=
      segmentation_histograms_subdir+"Ivv/";

   string global_color_comparison_filename=global_color_histograms_subdir+
      "image_global_colors.comparison";
   string gist_comparison_filename=gist_histograms_subdir+
      "image_gists.comparison";
   string texture_comparison_filename=texture_histograms_subdir+
      "image_textures.comparison";
   string LBP_comparison_filename=LBP_histograms_subdir+
      "image_LBPs.comparison";
   string faces_comparison_filename=faces_subdir+
      "image_faces.comparison";

   string SURF_scale_comparison_filename=SURF_scale_histograms_subdir+
      "image_SURFs.comparison";
   string SURF_angle_comparison_filename=
      SURF_angle_histograms_subdir+"image_SURFs.comparison";
   string SURF_separation_comparison_filename=
      SURF_separation_histograms_subdir+"image_SURFs.comparison";
   string segment_area_comparison_filename=
      segmentation_histograms_frac_area_subdir+"image_segments.comparison";
   string segment_Iuu_comparison_filename=
      segmentation_histograms_Iuu_subdir+"image_segments.comparison";
   string segment_Iuv_comparison_filename=
      segmentation_histograms_Iuv_subdir+"image_segments.comparison";
   string segment_Ivv_comparison_filename=
      segmentation_histograms_Ivv_subdir+"image_segments.comparison";

   if (keyframes_flag)
   {
      global_color_comparison_filename += ".keyframes";
      gist_comparison_filename += ".keyframes";
      texture_comparison_filename += ".keyframes";
      LBP_comparison_filename += ".keyframes";
      faces_comparison_filename += ".keyframes";
      SURF_scale_comparison_filename += ".keyframes";
      SURF_angle_comparison_filename += ".keyframes";
      SURF_separation_comparison_filename += ".keyframes";
      segment_area_comparison_filename += ".keyframes";
      segment_Iuu_comparison_filename += ".keyframes";
      segment_Iuv_comparison_filename += ".keyframes";
      segment_Ivv_comparison_filename += ".keyframes";
   }

   string output_filename;
   vector<vector<string> > substrings;

   cout << endl;
   cout << "Importing global image descriptors:" << endl;
   cout << endl;

   int descriptor_start=0;
//   int descriptor_start=11;
   for (int descriptor=descriptor_start; descriptor<n_descriptors; 
        descriptor++)
   {
      outputfunc::print_elapsed_time();
      cout << "Descriptor = " << descriptor << endl;
      substrings.clear();
      if (descriptor==0)
      {
         output_filename=global_color_histograms_subdir+
            "image_global_colors.dat";
         substrings=filefunc::ReadInSubstrings(
            global_color_comparison_filename);
      }
      else if (descriptor==1)
      {
         output_filename=gist_histograms_subdir+"image_gists.dat";
         substrings=filefunc::ReadInSubstrings(gist_comparison_filename);
      }
      else if (descriptor==2)
      {
         output_filename=texture_histograms_subdir+"image_textures.dat";
         substrings=filefunc::ReadInSubstrings(texture_comparison_filename);
      }
      else if (descriptor==3)
      {
         output_filename=SURF_scale_histograms_subdir+"image_SURF_scales.dat";
         substrings=filefunc::ReadInSubstrings(SURF_scale_comparison_filename);
      }
      else if (descriptor==4)
      {
         output_filename=SURF_angle_histograms_subdir+"image_SURF_angles.dat";
         substrings=filefunc::ReadInSubstrings(SURF_angle_comparison_filename);
      }
      else if (descriptor==5)
      {
         output_filename=SURF_separation_histograms_subdir+
            "image_SURF_separations.dat";
         substrings=filefunc::ReadInSubstrings(
            SURF_separation_comparison_filename);
      }
      else if (descriptor==6)
      {
         output_filename=LBP_histograms_subdir+"image_LBPs.dat";
         substrings=filefunc::ReadInSubstrings(LBP_comparison_filename);
      }
      else if (descriptor==7)
      {
         output_filename=segmentation_histograms_frac_area_subdir+
            "image_segments.dat";
         substrings=filefunc::ReadInSubstrings(
            segment_area_comparison_filename);
      }
      else if (descriptor==8)
      {
         output_filename=segmentation_histograms_Iuu_subdir+
            "image_segments.dat";
         substrings=filefunc::ReadInSubstrings(
            segment_Iuu_comparison_filename);
      }
      else if (descriptor==9)
      {
         output_filename=segmentation_histograms_Iuv_subdir+
            "image_segments.dat";
         substrings=filefunc::ReadInSubstrings(
            segment_Iuv_comparison_filename);
      }
      else if (descriptor==10)
      {
         output_filename=segmentation_histograms_Ivv_subdir+
            "image_segments.dat";
         substrings=filefunc::ReadInSubstrings(
            segment_Ivv_comparison_filename);
      }
      else if (descriptor==11)
      {
         output_filename=faces_subdir+"image_faces.dat";
         substrings=filefunc::ReadInSubstrings(faces_comparison_filename);
      }

      cout << "Finished importing descriptors for type " << descriptor << endl;

      ofstream outstream;
      filefunc::openfile(output_filename,outstream);
      outstream << "# Score  First_clip_ID  First_frame_ID  Second_clip_ID  Second_frame_ID" << endl << endl;

      string separator_chars=" _-";
      for (unsigned int i=0; i<substrings.size(); i++)
      {
         double score=stringfunc::string_to_number(substrings[i].at(0));
//         if (i%100000==0) cout << score << " " << flush;

// Score thresholds for Tidmarsh imagery:

         if (!video_clip_data_flag)
         {
            if (descriptor==0)
            {
               if (score < 0.5) break;
            }
            else if (descriptor==1)
            {
               if (score < 0.85) break;
            }
//            else if (descriptor==2)
//            {
//               if (score < 0.74) break;
//            }
         } 
         else
         {
         } // video_clip_data_flag conditional

         int first_clip_ID=0;
         int second_clip_ID=0;
         int first_frame_ID,second_frame_ID;
         if (video_clip_data_flag)
         {
            vector<string> subsubstrings=
               stringfunc::decompose_string_into_substrings(
                  substrings[i].at(1),separator_chars);
            first_clip_ID=stringfunc::string_to_number(subsubstrings[1]);
            first_frame_ID=stringfunc::string_to_number(subsubstrings[3]);
         }
         else
         {
            string curr_substr=substrings[i].at(1);
            first_frame_ID=stringfunc::string_to_number(
               curr_substr.substr(3,curr_substr.size()-3));
         }

         if (video_clip_data_flag)
         {
            vector<string> subsubstrings=
               stringfunc::decompose_string_into_substrings(
                  substrings[i].at(2),separator_chars);
            second_clip_ID=stringfunc::string_to_number(subsubstrings[1]);
            second_frame_ID=stringfunc::string_to_number(subsubstrings[3]);
         }
         else
         {
            string curr_substr=substrings[i].at(2);
            second_frame_ID=stringfunc::string_to_number(
               curr_substr.substr(3,curr_substr.size()-3));
         }

         outstream << score << " "
                   << first_clip_ID << " "
                   << first_frame_ID << " "
                   << second_clip_ID << " "
                   << second_frame_ID << endl;

         quadruple curr_q(
            first_clip_ID,first_frame_ID,second_clip_ID,second_frame_ID);
//         cout << "curr_q = " << curr_q << endl;

         iter=global_descriptor_scores_map.find(curr_q);
         if (iter==global_descriptor_scores_map.end())
         {
            genvector* descriptor_score_ptr=new genvector(n_descriptors);
            for (int d=0; d<n_descriptors; d++)
            {
               descriptor_score_ptr->put(d,-1);
            }
            descriptor_score_ptr->put(descriptor,score);
//            cout << "Descriptor_score = " << descriptor_score << endl;
            global_descriptor_scores_map[curr_q]=descriptor_score_ptr;
         }
         else
         {
            iter->second->put(descriptor,score);
         }
      } // loop over index i labeling substrings
      cout << endl;

      filefunc::closefile(output_filename,outstream);   
      string banner="Exported "+output_filename;
      outputfunc::write_big_banner(banner);
   } // loop over descriptor index

/*
   exit(-1);
   
   cout << endl;
   cout << "Labeling images as functions of their global descriptors:" << endl;
   cout << endl;

   string matching_images_filename=root_subdir+"matching_images.dat";
   string nonmatching_images_filename=root_subdir+"nonmatching_images.dat";
   string matching_imagenames_filename=
      root_subdir+"matching_image_filenames.dat";
   string nonmatching_imagenames_filename=
      root_subdir+"nonmatching_image_filenames.dat";
   ofstream match_stream,nonmatch_stream,matching_imagenames_stream,
      nonmatching_imagenames_stream;

   filefunc::openfile(matching_images_filename,match_stream);
   filefunc::openfile(nonmatching_images_filename,nonmatch_stream);
   filefunc::openfile(matching_imagenames_filename,matching_imagenames_stream);
   filefunc::openfile(nonmatching_imagenames_filename,
                      nonmatching_imagenames_stream);

   match_stream << "# global_color GIST texture ellipse_area ellipse_density ellipse_separation LBP image1_filename image2_filename" << endl << endl;
   nonmatch_stream << "# global_color GIST texture ellipse_area ellipse_density ellipse_separation LBP image1_filename image2_filename" << endl << endl;

   int prev_counter=-1000;
   int counter=0;
   int n_descriptor_scores=global_descriptor_scores_map.size();
   cout << "n_descriptor_scores = " << n_descriptor_scores << endl;
   
   for (iter=global_descriptor_scores_map.begin(); iter != 
           global_descriptor_scores_map.end(); iter++)
   {
      outputfunc::update_progress_fraction(
         counter++,n_descriptor_scores/100,n_descriptor_scores);

      double global_color_score=iter->second->get(0);
      double gist_score=iter->second->get(1);
      double texture_score=iter->second->get(2);
      double ellipse_area_score=iter->second->get(3);
      double ellipse_density_score=iter->second->get(4);
      double ellipse_separation_score=iter->second->get(5);
      double LBP_score=iter->second->get(6);

      if (global_color_score < 0 || gist_score < 0 || texture_score < 0 ||
      ellipse_area_score < 0 || ellipse_density_score < 0 ||
      ellipse_separation_score < 0 || LBP_score < 0)
         continue;

//      if (global_color_score < 0.1) continue;
//		      if (color_score < 0.4) continue;
//      if (gist_score < 0.8) continue;
//      if (texture_score < 0.4) continue;
//		      if (LBP_score < 0.25) continue;


// FAKE FAKE:  Sat Oct 12, 2013 at 7:29 am

// To force nonmatching Tidmarsh images to appear, set UPPER bounds on
// color, GIST and texture scores:

//      if (color_score > 0.83) continue;
//      if (gist_score > 0.88) continue;
//      if (texture_score > 0.83) continue;


// Skip over several nearby image pairs 

//      if (counter-prev_counter < 1.0/200.0*n_descriptor_scores) continue;
//      if (counter-prev_counter < 1.0/3000.0*n_descriptor_scores) continue;
//      if (nrfunc::ran1() > 0.3) continue;

      double counter_frac=double(counter)/n_descriptor_scores;
      cout << endl;
      cout << "counter = " << counter << " prev_counter = " << prev_counter
           << " counter_frac = " << counter_frac << endl;

      int first_clip_ID=iter->first.first;
      int first_frame_ID=iter->first.second;
      int second_clip_ID=iter->first.third;
      int second_frame_ID=iter->first.fourth;

      string first_image_filename,second_image_filename;
      if (video_clip_data_flag)
      {
         first_image_filename=images_subdir+
            "clip_"+stringfunc::integer_to_string(first_clip_ID,4)+"_frame-"+
            stringfunc::integer_to_string(first_frame_ID,5)+".jpg";
         second_image_filename=images_subdir+
            "clip_"+stringfunc::integer_to_string(second_clip_ID,4)+"_frame-"+
            stringfunc::integer_to_string(second_frame_ID,5)+".jpg";
      }
      else
      {
         first_image_filename=images_subdir+
            "pic"+stringfunc::integer_to_string(first_frame_ID,5)+".jpg";
         second_image_filename=images_subdir+
            "pic"+stringfunc::integer_to_string(second_frame_ID,5)+".jpg";
      }

      string unix_cmd="montageview "+first_image_filename+" "+
         second_image_filename;
      sysfunc::unix_command(unix_cmd);

      cout << "first_image_filename = " 
           << filefunc::getbasename(first_image_filename) << endl;
      cout << "second_image_filename = " 
           << filefunc::getbasename(second_image_filename) << endl;

      cout << iter->first << "  "
           << global_color_score << " "
           << gist_score << " "
           << texture_score << " "
           << ellipse_area_score << " "

           << LBP_score 
           << endl;

      string match_char;
      cout << "Good image match? (y/n/s/q)" << endl;
      cin >> match_char;

      if (match_char=="y")
      {
         match_stream 
            << global_color_score << " "
//            << color_score << " "
            << gist_score << " "
            << texture_score << " "
//            << LBP_score 
            << " # "
            << stringfunc::prefix(
               filefunc::getbasename(first_image_filename)) << " "
            << stringfunc::prefix(
               filefunc::getbasename(second_image_filename)) 
            << endl;
         matching_imagenames_stream 
            << first_image_filename << " "
            << second_image_filename << " "
            << endl;
      }
      else if (match_char=="n")
      {
         nonmatch_stream 
            << global_color_score << " "
//            << color_score << " "
            << gist_score << " "
            << texture_score << " "
//            << LBP_score 
            << " # "
            << stringfunc::prefix(
               filefunc::getbasename(first_image_filename)) << " "
            << stringfunc::prefix(
               filefunc::getbasename(second_image_filename)) 
            << endl;
         nonmatching_imagenames_stream
            << first_image_filename << " "
            << second_image_filename 
            << endl;
      }
      else if (match_char=="s")	// skip
      {

// On 10/9/13, Davis King strongly recommended that we NOT label any
// image pair as matching or not-matching if we're unsure.  Instead,
// he said it's better to simply skip such ambiguous examples:

      }
      else if (match_char=="q")
      {
         break;
      }

      string substring="montage_";
      vector<string> montage_filenames=
         filefunc::files_in_subdir_matching_substring(
            "./",substring);
      for (int m=0; m<montage_filenames.size(); m++)
      {
         filefunc::deletefile(montage_filenames[m]);
      }

      vector<int> process_IDs=sysfunc::my_get_pid("display");
      for (int p=0; p<process_IDs.size(); p++)
      {
//         cout << "p = " << p << " process_ID = " << process_IDs[p] << endl;
         string unix_cmd="kill -9 "+
            stringfunc::number_to_string(process_IDs[p]);
         sysfunc::unix_command(unix_cmd);
      }

      prev_counter=counter;
      
   } // loop over global_descriptor_scores_map iterator
   cout << endl;

   filefunc::closefile(matching_images_filename,match_stream);
   filefunc::closefile(nonmatching_images_filename,nonmatch_stream);
   filefunc::closefile(
      matching_imagenames_filename,matching_imagenames_stream);
   filefunc::closefile(
      nonmatching_imagenames_filename,nonmatching_imagenames_stream);
*/

   cout << "At end of program TRAIN_IMAGE_COMPARISONS" << endl;
   outputfunc::print_elapsed_time();
}

