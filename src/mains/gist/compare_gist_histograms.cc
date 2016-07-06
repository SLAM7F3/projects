// ==========================================================================
// Program COMPARE_GIST_HISTOGRAMS computes the magnitude of the
// difference between GIST descriptors for pairs of images.  It
// outputs a probability distribution for the difference magnitudes.
// COMPARE_GIST_HISTOGRAMS also exports a text file containing pairs
// of similar images whose difference magnitude is less than some
// user-specified threshold.

//		      	./compare_gist_histograms

// ==========================================================================
// Last updated on 10/24/13; 10/28/13; 11/8/13
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "video/descriptorfuncs.h"
#include "image/imagefuncs.h"
#include "math/ltduple.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
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

   bool keyframes_flag=true;
   bool different_clips_flag=false;

   string input_char;
   cout << "Enter 'k' to process just video keyframes:" << endl;
   cin >> input_char;
   if (input_char != "k")
   {
      keyframes_flag=false;
   }
   cout << "Enter 'w' to compare histograms within and not between clips:"
        << endl;
   cin >> input_char;
   if (input_char != "w")
   {
      different_clips_flag=true;
   }

// Import filenames for images and their GIST descriptors.  Recall
// that some images do NOT have GIST descriptors!

//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";
   string keyframes_subdir=images_subdir+"keyframes/";
//   const double min_dotproduct=0.865919;	// NewsWrap
   double min_dotproduct=0.6;	// NewsWrap

/*
   string ImageEngine_subdir="/data/ImageEngine/";
   string tidmarsh_subdir=ImageEngine_subdir+"tidmarsh/";
   string root_subdir=tidmarsh_subdir;
   string images_subdir=root_subdir;
   const double min_dotproduct=0.89;	// Tidmarsh
*/

// Report all gist matching scores when we're comparing keyframes
// within same video clips:

   if (keyframes_flag && !different_clips_flag) min_dotproduct=0;

// Store keyframes' clip and frame IDs within STL map keyframes_map:

   typedef map<DUPLE,string> KEYFRAMES_MAP;
   KEYFRAMES_MAP keyframes_map;

// independent DUPLE holds video image clip and frame ID
// dependent string holds video image's filename

   if (keyframes_flag)
   {
      vector<string> keyframe_filenames=filefunc::image_files_in_subdir(
         keyframes_subdir);

      for (int i=0; i<keyframe_filenames.size(); i++)
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               filefunc::getbasename(keyframe_filenames[i])," _-");
//         for (int s=0; s<substrings.size(); s++)
//         {
//            cout << "i = " << i << " s = " << s << " substring[s] = " 
//                 << substrings[s] << endl;
//         }

// If input filename has form clip_0000_frame-00001.jpg, record clip's
// ID within STL vector:

         int clip_ID=-1;
         int frame_ID=-1;
         if (substrings.size()==4)
         {
            if (substrings[0]=="clip")
            {
               clip_ID=stringfunc::string_to_number(substrings[1]);
               frame_ID=stringfunc::string_to_number(substrings[3]);
               keyframes_map[DUPLE(clip_ID,frame_ID)]=keyframe_filenames[i];
            }
         }
      } // loop over index i labeling keyframe filenames
      
   } // keyframes_flag conditional
   cout << "keyframes_map.size() = " << keyframes_map.size() << endl;


   string gist_histograms_subdir=root_subdir+"gist_files/";

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,images_subdir);
   int n_images=image_filenames.size();
   cout << "n_images = " << n_images << endl;
   
   allowed_suffixes.clear();
   allowed_suffixes.push_back("gist");
   vector<string> histogram_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,gist_histograms_subdir);

// If keyframes_flag==true, cull non-keyframe histograms from
// STL vector histogram_filenames.  Fill new STL vector hist_filenames
// with histogram files to actually be processed:

   vector<string> hist_filenames;
   for (int i=0; i<histogram_filenames.size(); i++)
   {
//      cout << "i = " << i << " histogram_filename = " 
//           << histogram_filenames[i] << endl;

      if (keyframes_flag)
      {
         string basename=filefunc::getbasename(histogram_filenames[i]);
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               stringfunc::prefix(basename)," _-");
//         for (int s=0; s<substrings.size(); s++)
//         {
//            cout << " s = " << s << " substring[s] = " 
//                 << substrings[s] << endl;
//         }

// If input filename has form clip_0000_frame-00001, record clip's
// ID within STL vector:

         int clip_ID=-1;
         int frame_ID=-1;
         if (substrings.size()==4)
         {
            if (substrings[0]=="clip")
            {
               clip_ID=stringfunc::string_to_number(substrings[1]);
               frame_ID=stringfunc::string_to_number(substrings[3]);
               KEYFRAMES_MAP::iterator keyframe_iter=keyframes_map.find(
                  DUPLE(clip_ID,frame_ID));
               if (keyframe_iter==keyframes_map.end())
               {
                  continue;
               }
//               else
//               {
//                  cout << "clip_ID = " << clip_ID << " frame_ID = " << frame_//ID
//                       << " is a keyframe" << endl;
//               }
            }
         } // substrings.size() conditional
      } // keyframes_flag conditional

      hist_filenames.push_back(histogram_filenames[i]);
   } // loop over index i labeling input histogram files
   int n_histograms=hist_filenames.size();
   cout << "n_histograms = " << n_histograms << endl;

// Import GIST descriptors for images and store them within a
// genmatrix:

   string banner="Importing GIST descriptors from text files:";
   outputfunc::write_banner(banner);

   int i_max=n_histograms;

// Import GIST descriptors calculated via program
// COMPUTE_GIST_HISTOGRAMS:

   genmatrix* GIST_matrix_ptr=descriptorfunc::GIST_descriptor_matrix(
      hist_filenames,i_max);
   int n_bins=GIST_matrix_ptr->get_ndim();
   genvector* gist_descriptor1_ptr=new genvector(n_bins);
   genvector* gist_descriptor2_ptr=new genvector(n_bins);

   vector<string> image_pair_names;
   vector<int> basename_i_indices,basename_j_indices;
   vector<float> GIST_dotproducts;

   cout << endl;
   cout << "Computing GIST descriptor dotproducts:" << endl;
   cout << endl;
   
   int j_max=i_max;
   vector<double> dotproducts;
   for (int i=0; i<i_max; i++)
   {
      outputfunc::update_progress_fraction(i,100,i_max);

      GIST_matrix_ptr->get_row(i,*gist_descriptor1_ptr);

//      if (!different_clips_flag) j_max=basic_math::min(i+2,i_max);

      for (int j=i+1; j<j_max; j++)
      {
         GIST_matrix_ptr->get_row(j,*gist_descriptor2_ptr);

         double dotproduct=0;
//         bool conventional_dotproduct_flag=true;
         bool conventional_dotproduct_flag=false;
         if (conventional_dotproduct_flag)
         {
            for (int b=0; b<n_bins; b++)
            {
               dotproduct += 
                  gist_descriptor1_ptr->get(b)*
                  gist_descriptor2_ptr->get(b);
            }
            dotproduct /= (gist_descriptor1_ptr->magnitude()*
                           gist_descriptor2_ptr->magnitude());
         }
         else
         {

// Nonparametric dissimilarity measure:

            for (int b=0; b<n_bins; b++)
            {
               double numer=fabs(
                  gist_descriptor2_ptr->get(b)-gist_descriptor1_ptr->get(b));
               double denom=fabs(
                  gist_descriptor2_ptr->get(b)+gist_descriptor1_ptr->get(b));

               if (nearly_equal(numer,0) || nearly_equal(denom,0)) continue;
               dotproduct += numer/denom;
            }
            dotproduct /= n_bins;

// Transform "dotproduct" so that unity [zero] value indicates
// similarity [dissimilarity]"

            dotproduct=1-dotproduct;

//            cout << "Nonparameteric dissimilarity measure = " 
// 		   << dotproduct << endl;
         }
         dotproducts.push_back(dotproduct);

         if (dotproduct < min_dotproduct) continue;

         GIST_dotproducts.push_back(dotproduct);

         string basename_i=filefunc::getbasename(hist_filenames[i]);
         string basename_j=filefunc::getbasename(hist_filenames[j]);

         image_pair_names.push_back(
            stringfunc::prefix(basename_i)+"  "+
            stringfunc::prefix(basename_j) );

         basename_i_indices.push_back(i);
         basename_j_indices.push_back(j);

      } // loop over index j labeling gist filenames

//      cout << "----------------------------------------------------------" 
//           << endl;
   } // loop over index i labeling gist filenames
   cout << endl;

   cout << "dotproducts.size() = " << dotproducts.size() << endl;
   cout << "n_dotproducts exceeding min threshold = " 
        << GIST_dotproducts.size() << endl;

   prob_distribution prob(dotproducts,100,0);
   prob.writeprobdists(false);

   delete GIST_matrix_ptr;
   delete gist_descriptor1_ptr;
   delete gist_descriptor2_ptr;

   templatefunc::Quicksort_descending(GIST_dotproducts,image_pair_names);

   string output_filename=gist_histograms_subdir;
   if (different_clips_flag)
   {
      output_filename += "image_gists.comparison";
   }
   else
   {
      output_filename += "image_gists_sameclips.comparison";
   }

   if (keyframes_flag)
   {
      output_filename += ".keyframes";
   }

   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# GIST_delta   Image_prefix   Image'_prefix   "
             << endl << endl;
   outstream << "# GIST descriptor dotproduct magnitude threshold = " 
             << min_dotproduct << endl;
   outstream << endl;

   for (int g=0; g<GIST_dotproducts.size(); g++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         image_pair_names[g]," _-");

      int clip_ID_i=-1;
      int clip_ID_j=-2;
      int frame_ID_i,frame_ID_j;
      if (substrings.size()==8)
      {
         if (substrings[0]=="clip")
         {
            clip_ID_i=stringfunc::string_to_number(substrings[1]);
            frame_ID_i=stringfunc::string_to_number(substrings[3]);
            clip_ID_j=stringfunc::string_to_number(substrings[5]);
            frame_ID_j=stringfunc::string_to_number(substrings[7]);
//            cout << "clip_ID_i = " << clip_ID_i
//                 << " clip_ID_j = " << clip_ID_j
//                 << endl;
         }
      } // substrings.size()==8 conditional

      if ( ((clip_ID_i != clip_ID_j) && different_clips_flag) ||
      ((clip_ID_i==clip_ID_j) && 
//       (frame_ID_j==frame_ID_i+1) && 
      !different_clips_flag) )
      {
         outstream << GIST_dotproducts[g] << "   "
                   << image_pair_names[g] << endl;
      }

   } // loop over index g labeling gist descriptor dotproducts
   filefunc::closefile(output_filename,outstream);

   banner="Exported GIST scores for image pairs to "+output_filename;
   outputfunc::write_big_banner(banner);

   cout << "At end of program COMPARE_GIST" << endl;
   outputfunc::print_elapsed_time();
}

