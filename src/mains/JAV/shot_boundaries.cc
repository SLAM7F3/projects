// ==========================================================================
// Program SHOT_BOUNDARIES takes in a temporal series of video frames.
// GIST descriptors for each video frame are imported.  Each image is also 
// subdivided into 4x4 sectors within which color histograms are
// calculated.  Dotproducts between the sectors' current and previous
// color histograms are formed, and the median of the 16 dotproducts
// is returned at each time step.  A shot boundary between two
// adjacent video frames is declared when some combination of the
// median dotproduct and change in GIST descriptor magnitude exceeds
// an empirically determined threshold function value.

// ==========================================================================
// Last updated on 9/2/13; 9/3/13; 10/8/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/genvector.h"
#include "video/descriptorfuncs.h"
#include "math/mathfuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   timefunc::initialize_timeofday_clock();      

   string ImageEngine_subdir="/data/ImageEngine/";
//   string video_subdir=ImageEngine_subdir+"NewsWrap/";
//   string video_subdir=ImageEngine_subdir+"BostonBombing/Nightline_YouTube2/";
   string video_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/korea/NK/ground_videos/NorthKorea/";
   string gist_subdir=video_subdir+"gist_files/";

// Import video clip frames:

   string videoframe_substring="frame";	// NewsWrap, NorthKorea
//   string videoframe_substring="transcripted_clip"; // Boston Bombing
   vector<string> video_filenames=filefunc::files_in_subdir_matching_substring(
      video_subdir,videoframe_substring);
   int n_images=video_filenames.size();

// Import GIST descriptors into genmatrix *GIST_descriptor_matrix_ptr:

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("gist");
   vector<string> gist_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,gist_subdir);
   genmatrix* GIST_matrix_ptr=descriptorfunc::GIST_descriptor_matrix(gist_filenames);
   int n_fields=GIST_matrix_ptr->get_ndim();
   genvector* curr_gist_descriptor_ptr=new genvector(n_fields);
   genvector* prev_gist_descriptor_ptr=new genvector(n_fields);

   if (n_images != GIST_matrix_ptr->get_mdim())
   {
      cout << "Error!" << endl;
      cout << "n_images = " << n_images << endl;
      cout << "n_GIST_descriptors = " << GIST_matrix_ptr->get_mdim()
           << endl;
      exit(-1);
   }   

// Initialize RGB_analyzer and texture_rectangle objects:

   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   string liberalized_color="";
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table(liberalized_color);
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

// Instantiate STL map to hold 4x4 sector color histograms:

   typedef map<int,vector<double>* > SECTOR_COLOR_HISTOGRAMS;
   SECTOR_COLOR_HISTOGRAMS prev_color_histograms;
   SECTOR_COLOR_HISTOGRAMS::iterator iter;

// Open output text file to hold shot boundary information:

   string output_filename="boundaries.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

// Loop over input video files starts here:

   for (int i=0; i<video_filenames.size(); i++)
   {
      string image_filename=video_filenames[i];
      string basename=filefunc::getbasename(image_filename);
      string prefix=stringfunc::prefix(basename);
//      cout << "i = " << i 
//           << " image_filename = " << image_filename << endl;

// Compute GIST descriptor difference between current and previous
// video frames:

      GIST_matrix_ptr->get_row(i,*curr_gist_descriptor_ptr);
      if (i > 0)
      {
         GIST_matrix_ptr->get_row(i-1,*prev_gist_descriptor_ptr);
      }
      else
      {
         GIST_matrix_ptr->get_row(i,*prev_gist_descriptor_ptr);
      }
      double GIST_dotproduct=curr_gist_descriptor_ptr->unitvector().dot(
         prev_gist_descriptor_ptr->unitvector());
      double Delta_GIST=
         (*curr_gist_descriptor_ptr - *prev_gist_descriptor_ptr).
         magnitude()/3.0;

      int n_changed_sectors=0;
      vector<double> sector_dotproducts;
      for (int index=0; index<16; index++)
      {
         int row=index/4;
         int column=index%4;
//         cout << "index = " << index << " row = " << row
//              << " column = " << column << endl;
         
         vector<double> sector_color_histogram=
            descriptorfunc::compute_sector_color_histogram(
               4,4,row,column,image_filename,texture_rectangle_ptr,
               RGB_analyzer_ptr);

         if (i==0)
         {
            vector<double>* color_histogram_ptr=new vector<double>;
            for (int c=0; c<sector_color_histogram.size(); c++)
            {
               color_histogram_ptr->push_back(sector_color_histogram[c]);
//               cout << "c = " << c << " color_histogram[c] = " 
//                    << color_histogram_ptr->at(c) << endl;
            }
            prev_color_histograms[index]=color_histogram_ptr;
            sector_dotproducts.push_back(1);
         }
         else
         {
            iter=prev_color_histograms.find(index);
            vector<double>* prev_color_histogram_ptr=iter->second;
            
            double dotproduct=0;
            double curr_normsq=0;
            double prev_normsq=0;
            for (int c=0; c<sector_color_histogram.size(); c++)
            {
               double prev_coeff=prev_color_histogram_ptr->at(c);
               double curr_coeff=sector_color_histogram[c];
               (*prev_color_histogram_ptr)[c]=curr_coeff;
               
               dotproduct += curr_coeff*prev_coeff;
               curr_normsq += sqr(curr_coeff);
               prev_normsq += sqr(prev_coeff);
            }
            
            dotproduct /= (sqrt(curr_normsq)*sqrt(prev_normsq));
            sector_dotproducts.push_back(dotproduct);
         } // i==0 conditional

//         cout << "index = " << index << " sector dotproduct = " 
//              << sector_dotproducts.back() << endl;

      } // loop over index labeling 16 sectors within current video frame
      
      double median_sector_dotproduct=
         mathfunc::median_value(sector_dotproducts);
      
      if (Delta_GIST > -0.02512+0.4148*median_sector_dotproduct)
      {
         cout << "------------------------------------------------"
              << endl;
         outstream << "---------------------------------------------------------------" << endl;
      }

      cout << "i = " << i 
           << " median dotproduct = " << median_sector_dotproduct
           << " Delta_GIST = " << Delta_GIST
           << " " << basename
           << endl;

      outstream << "i = " << i 
                << " median dotproduct = " << median_sector_dotproduct
                << " Delta_GIST = " << Delta_GIST
                << " " << basename
                << endl;

   } // loop over index i labeling input video frames

   filefunc::closefile(output_filename,outstream);
   string banner="Exported shot boundaries to "+output_filename;
   outputfunc::write_big_banner(banner);
      
   delete RGB_analyzer_ptr;
   delete texture_rectangle_ptr;
   delete curr_gist_descriptor_ptr;
   delete prev_gist_descriptor_ptr;
   delete GIST_matrix_ptr;

   cout << "At end of program SHOT_BOUNDARIES" << endl;
   outputfunc::print_elapsed_time();
}
