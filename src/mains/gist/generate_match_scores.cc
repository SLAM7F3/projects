// ==========================================================================
// Program GENERATE_MATCH_SCORES imports relatively small numbers of
// pairs of matching image filenames generated via program
// TRAIN_IMAGE_COMPARISONS and GENERATE_NONMATCHING_FILENAMES.  It
// first instantiates an STL map to hold quadruples formed from the
// image pairs' clip and frame IDs.  GENERATE_MATCH_SCORES next imports
// scores indicating overlap within color, GIST, texture, etc channels
// for large numbers of image pairs.  Dependent score vectors are
// filled for independent image pair quadruples within the STL map.
// Repeated image pair filenames are ignored.

// Iterating over all entries in the STL map, GENERATE_MATCH_SCORES
// exports matching_images.dat and nonmatching_images.dat text files
// which list n-tuples of feature matching scores for matching and
// nonmatching image categories.  These text files become input for
// program SVM_GLOBAL_DESCRIPS.  It also outputs matching_files.txt
// and nonmatching_files.txt which contain the names of paired files
// whose scores are actually exported in matching_images.dat and
// nonmatching_images.dat.


//		          ./generate_match_scores

// ==========================================================================
// Last updated on 11/22/13; 12/12/13; 12/13/13
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

//   int n_descriptors=1;
//   int n_descriptors=3;
//   int n_descriptors=4;
//   int n_descriptors=5;
//   int n_descriptors=6;
//   int n_descriptors=7;
//   int n_descriptors=11;
//   int n_descriptors=11+20;
//   int n_descriptors=11+100;	// OK
//   int n_descriptors=11+200;  // OK
//   int n_descriptors=11+250;	// OK
//   int n_descriptors=11+256; // FAILS
//   int n_descriptors=11+262; // FAILS
//   int n_descriptors=11+275; // FAILS
//   int n_descriptors=11+300; // FAILS
//   int n_descriptors=11+400;

   int n_HOG_bins=4;
//   int n_HOG_bins=1024;
//   int n_descriptors=11+n_HOG_bins;
   int n_descriptors=n_HOG_bins;

   typedef std::map<quadruple,genvector*,ltquadruple> 
      GLOBAL_DESCRIPTOR_SCORES_MAP;

// independent quadruple holds first clip ID, first frame ID, second
// clip ID and second frame ID

// dependent genvector holds color, GIST, texture, global color, LBP, etc
// scores:

   GLOBAL_DESCRIPTOR_SCORES_MAP global_descriptor_scores_map;
   GLOBAL_DESCRIPTOR_SCORES_MAP::iterator iter;

//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
   bool video_clip_data_flag=true;

/*
   string ImageEngine_subdir="/data/ImageEngine/";
   string tidmarsh_subdir=ImageEngine_subdir+"tidmarsh/";
   string images_subdir=tidmarsh_subdir;
   string root_subdir=tidmarsh_subdir;
   string ellipse_histograms_subdir=root_subdir+"ellipses/";
   bool video_clip_data_flag=false;
*/
   
   string matching_images_filename=root_subdir+"matching_image_filenames.dat";
   string nonmatching_images_filename=root_subdir+
      "nonmatching_image_filenames.dat";
   vector<string> input_image_filenames;
   input_image_filenames.push_back(matching_images_filename);
   input_image_filenames.push_back(nonmatching_images_filename);

   int n_repeat_filename_pairs=0;
   for (unsigned int f=0; f<input_image_filenames.size(); f++)
   {
      if (f==0)
      {
         cout << "Importing names of matching image files:" << endl;
      }
      else
      {
         cout << "Importing names of non-matching image files:" << endl;
      }

      global_descriptor_scores_map.clear();

// Import pairs of matching and non-matching image filenames.  Store
// their clip & frame IDs within STL map global_descriptor_scores_map:

      string image_pair_filenames=input_image_filenames[f];
      cout << "image_pair_filenames = " << image_pair_filenames << endl;
      filefunc::ReadInfile(image_pair_filenames);

      string separator_chars=" _-";
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         string curr_line=filefunc::text_line[i];
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(curr_line);
         string basename1=filefunc::getbasename(substrings[0]);
         string basename2=filefunc::getbasename(substrings[1]);
//      cout << "basename1 = " << basename1 
//           << " basename2 = " << basename2 << endl;

         int first_clip_ID=0;
         int second_clip_ID=0;
         int first_frame_ID,second_frame_ID;
         if (video_clip_data_flag)
         {
            vector<string> subsubstrings=
               stringfunc::decompose_string_into_substrings(
                  basename1,separator_chars);
            first_clip_ID=stringfunc::string_to_number(subsubstrings[1]);
            first_frame_ID=stringfunc::string_to_number(subsubstrings[3]);
         }
         else
         {
            string curr_substr=basename1;
            first_frame_ID=stringfunc::string_to_number(
               curr_substr.substr(3,curr_substr.size()-3));
         }

         if (video_clip_data_flag)
         {
            vector<string> subsubstrings=
               stringfunc::decompose_string_into_substrings(
                  basename2,separator_chars);
            second_clip_ID=stringfunc::string_to_number(subsubstrings[1]);
            second_frame_ID=stringfunc::string_to_number(subsubstrings[3]);
         }
         else
         {
            string curr_substr=basename2;
            second_frame_ID=stringfunc::string_to_number(
               curr_substr.substr(3,curr_substr.size()-3));
         }
    
         quadruple curr_q(
            first_clip_ID,first_frame_ID,second_clip_ID,second_frame_ID);
//         cout << "curr_q = " << curr_q << endl;
//         cout << first_clip_ID << "  " << first_frame_ID << "  "
//              << second_clip_ID << "  " << second_frame_ID << endl;

         iter=global_descriptor_scores_map.find(curr_q);
         if (iter==global_descriptor_scores_map.end())
         {
            genvector* descriptor_score_ptr=new genvector(n_descriptors);
            for (int d=0; d<n_descriptors; d++)
            {
               descriptor_score_ptr->put(d,0);
//               descriptor_score_ptr->put(d,-1);
            }
            global_descriptor_scores_map[curr_q]=descriptor_score_ptr;

            if (first_clip_ID==20 && first_frame_ID==168)
            {
               cout << "curr_q = " << curr_q << endl;
            }
         }
         else
         {
            n_repeat_filename_pairs++;
         }
      } // loop over index i labeling matching image filenames





      cout << "n_repeat_filename_pairs = " << n_repeat_filename_pairs << endl;
      cout << "global_descriptor_scores_map.size() = "
           << global_descriptor_scores_map.size() << endl;

      string global_color_histograms_subdir=root_subdir+
         "global_color_histograms/";
      string gist_histograms_subdir=root_subdir+"gist_files/";
      string texture_histograms_subdir=root_subdir+"texture_histograms/";
      string SURF_histograms_subdir=root_subdir+"SURF_keys/";
      string LBP_histograms_subdir=root_subdir+"LBP_histograms/";
      string segmentation_histograms_subdir=root_subdir+
         "segmentation/metafiles/";
      string BoW_subdir=root_subdir+"BoWs/";
      string BoW_comparisons_subdir=BoW_subdir+"comparisons/";

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

      string input_filename;
      vector<vector<string> > substrings;

      cout << endl;
      cout << "Importing global image pair matching scores:" << endl;
      cout << endl;




//      int start_descriptor=0;
      int start_descriptor=11;
      int stop_descriptor=11+n_HOG_bins;
      for (int descriptor=start_descriptor; 
           descriptor<stop_descriptor; descriptor++)
      {
         double descriptor_frac=double(descriptor)/n_descriptors;
         outputfunc::print_elapsed_and_remaining_time(descriptor_frac);
         cout << "Descriptor = " << descriptor << endl;

// FAKE FAKE:  Thurs Dec 12, 2013 at 9 am
// Eliminate SURF separation descriptor by replacing it with dummy
// copy of global color descriptor:

         if (descriptor==0 || descriptor==5)
         {
            input_filename=global_color_histograms_subdir+
              "image_global_colors.dat";
         }
         else if (descriptor==1)
         {
            input_filename=gist_histograms_subdir+"image_gists.dat";
         }
         else if (descriptor==2)
         {
            input_filename=texture_histograms_subdir+"image_textures.dat";
         }
         else if (descriptor==3)
         {
            input_filename=SURF_scale_histograms_subdir+
               "image_SURF_scales.dat";
         }
         else if (descriptor==4)
         {
            input_filename=SURF_angle_histograms_subdir+
               "image_SURF_angles.dat";
         }
//         else if (descriptor==5)
//         {
//            input_filename=SURF_separation_histograms_subdir+
//               "image_SURF_separations.dat";
//         }
         else if (descriptor==6)
         {
            input_filename=LBP_histograms_subdir+"image_LBPs.dat";
         }
         else if (descriptor==7)
         {
            input_filename=segmentation_histograms_frac_area_subdir+
               "image_segments.dat";
         }
         else if (descriptor==8)
         {
            input_filename=segmentation_histograms_Iuu_subdir+
               "image_segments.dat";
         }
         else if (descriptor==9)
         {
            input_filename=segmentation_histograms_Iuv_subdir+
               "image_segments.dat";
         }
         else if (descriptor==10)
         {
            input_filename=segmentation_histograms_Ivv_subdir+
               "image_segments.dat";
         }
         else if (descriptor >=11 && descriptor < 11+n_HOG_bins)
         {
            input_filename=BoW_comparisons_subdir+"image_BoWs_"+
               stringfunc::integer_to_string(descriptor-11,4)+
               ".comparison.keyframes";
         }
         cout << "input_filename = " << input_filename << endl;



      cout << "---------------------------------------------" << endl;
      for (iter=global_descriptor_scores_map.begin(); 
           iter != global_descriptor_scores_map.end(); iter++)
      {
         quadruple curr_q=iter->first;
         int first_clip_ID=curr_q.first;
         int first_frame_ID=curr_q.second;
//         int second_clip_ID=curr_q.third;
//         int second_frame_ID=curr_q.fourth;
//         genvector* descriptor_score_ptr=iter->second;

         if (first_clip_ID==20 && first_frame_ID==168)
         {
            cout << "verify: curr_q = " << curr_q << endl;
         }
      }


         int line_start=2;
         int n_scores_found=0;
         int n_numbers_per_row=5;
         vector< vector<double> > row_numbers=
            filefunc::ReadInRowNumbers(
               line_start,n_numbers_per_row,input_filename);
         for (unsigned int r=0; r<row_numbers.size(); r++)
         {
            double curr_score=row_numbers[r].at(0);
//            cout << "curr_score = " << curr_score << endl;
            int first_clip_ID=row_numbers[r].at(1);
//            cout << "first_clip_ID = " << first_clip_ID << endl;
            int first_frame_ID=row_numbers[r].at(2);
//            cout << "first_frame_ID = " << first_frame_ID << endl;
            int second_clip_ID=row_numbers[r].at(3);
//            cout << "second_clip_ID = " << second_clip_ID << endl;
            int second_frame_ID=row_numbers[r].at(4);
//            cout << "second_frame_ID = " << second_frame_ID << endl;
            quadruple curr_q(
               first_clip_ID,first_frame_ID,second_clip_ID,second_frame_ID);

            iter=global_descriptor_scores_map.find(curr_q);
            if (iter != global_descriptor_scores_map.end())
            {
               genvector* descriptor_score_ptr=iter->second;
               descriptor_score_ptr->put(descriptor,curr_score);
               n_scores_found++;

               if (first_clip_ID==20 && first_frame_ID==168)
               {
                  cout << "from input_filename, curr_q = " << curr_q << endl;
               }

            }
         } // loop over index r labeling rows within input_filename

         cout << "n_scores_found = " << n_scores_found << endl;
      } // loop over descriptor index
      cout << "global_descriptor_scores_map.size() = "
           << global_descriptor_scores_map.size() << endl;
      outputfunc::enter_continue_char();

      string output_filename=root_subdir+"matching_images.dat";
      string files_filename=root_subdir+"matching_files.txt";
      if (f==1)
      {
         output_filename=root_subdir+"nonmatching_images.dat";
         files_filename=root_subdir+"nonmatching_files.txt";
      }

      ofstream outstream,filesstream;
      filefunc::openfile(output_filename,outstream);
      filefunc::openfile(files_filename,filesstream);
      outstream << "# color GIST texture SURF_scale SURF_angle SURF_separation LBP segment_area segment_Iuu segment_Iuv segment_Ivv BoWs image1_filename image2_filename" << endl << endl;
      filesstream << "# image1_filename image2_filename" << endl << endl;

      int image_pair_counter=0;
      for (iter=global_descriptor_scores_map.begin(); 
           iter != global_descriptor_scores_map.end(); iter++)
      {
         quadruple curr_q=iter->first;
         int first_clip_ID=curr_q.first;
         int first_frame_ID=curr_q.second;
         int second_clip_ID=curr_q.third;
         int second_frame_ID=curr_q.fourth;
         genvector* descriptor_score_ptr=iter->second;
//         cout << "*descriptor_score_ptr = " << *descriptor_score_ptr << endl;
         

// FAKE FAKE:

         continue;
         

         double descriptors_sum=0;
         vector<int> negative_descriptor_coeffs;
         bool all_scores_positive_flag=true;
         for (int d=0; d<n_descriptors; d++)
         {
            descriptors_sum += descriptor_score_ptr->get(d);
            if  (descriptor_score_ptr->get(d) < 0) 
            {
               negative_descriptor_coeffs.push_back(d);
               all_scores_positive_flag=false;
            }
         }

/*
         if (!all_scores_positive_flag) 
         {
            for (int n=0; n<negative_descriptor_coeffs.size(); n++)
            {
               cout << n << " " << flush;
            }
            cout << endl;
            continue;
         }
*/
         if (nearly_equal(0,descriptors_sum)) continue;

         for (int d=0; d<n_descriptors; d++)
         {
            outstream << descriptor_score_ptr->get(d) << " "
                      << flush;
         }
//         outstream << " # ";

         if (video_clip_data_flag)
         {
            outstream << "clip_"+stringfunc::integer_to_string(first_clip_ID,4)
               +"_frame-"+stringfunc::integer_to_string(first_frame_ID,5) 
                      << " ";
            outstream << "clip_"+stringfunc::integer_to_string(
               second_clip_ID,4)
               +"_frame-"+stringfunc::integer_to_string(second_frame_ID,5) 
                      << endl;

            filesstream << image_pair_counter 
                        << "  clip_"+stringfunc::integer_to_string(
                           first_clip_ID,4)
               +"_frame-"+stringfunc::integer_to_string(first_frame_ID,5) 
                      << " ";
            filesstream << "  clip_"+stringfunc::integer_to_string(
                           second_clip_ID,4)
               +"_frame-"+stringfunc::integer_to_string(second_frame_ID,5) 
                      << endl;
         }
         else
         {
            outstream << "pic"+stringfunc::integer_to_string(first_frame_ID,5)
                      << " ";
            outstream << "pic"+stringfunc::integer_to_string(
               second_frame_ID,5) << endl;
            filesstream << image_pair_counter 
                        << "  pic"+stringfunc::integer_to_string(
                           first_frame_ID,5) << " ";
            filesstream << "  pic"+stringfunc::integer_to_string(
               second_frame_ID,5) << endl;
         }
         image_pair_counter++;
         
      } // loop over iter
   
      filefunc::closefile(output_filename,outstream);
      filefunc::closefile(files_filename,outstream);

      string banner="Exported "+output_filename;
      outputfunc::write_big_banner(banner);

      banner="Exported "+files_filename;
      outputfunc::write_big_banner(banner);

   } // loop over index f labeling input image filenames

   cout << "At end of program GENERATE_MATCH_SCORES" << endl;
   outputfunc::print_elapsed_time();
}

