// ========================================================================
// Program FINDFACES processes all images within the subdirectory of
// /data/ImageEngine/ specified by the user.  It utilizes the face
// detection binary of Kalal, Matas and Mikolajczyk to place
// circles around human faces within each image. The center and radius
// of each human face measured in pixels are exported to an output
// text file along with image filenames.  Output from this program
// becomes input to program INSERTFACES.
// ========================================================================
// Last updated on 5/9/12; 10/5/13; 11/1/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "classification/classification_funcs.h"
#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

/*
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;

   string images_subdir="NewsWrap";
   cout << "Enter subdirectory of /data/ImageEngine/ containing images to process:" << endl;
   cin >> images_subdir;
   images_subdir="/data/ImageEngine/"+images_subdir;
   filefunc::add_trailing_dir_slash(images_subdir);
//   cout << "images_subdir = " << images_subdir << endl;

// As of 4/14/12, we only insert metadata for images within the
// image_list_filename and NOT for all images within images_subdir.
// This ensures consistency between SIFT graph calculated via
// parallelized bundler on TXX and the images table within the imagery
// database!

   vector<string> image_filenames;
   filefunc::ReadInfile(image_list_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      string curr_image_filename=substrings[0];
      string basename=filefunc::getbasename(curr_image_filename);
      string image_filename=images_subdir+basename;
//      cout << "i = " << i 
//           << " " << image_filename << endl;
      image_filenames.push_back(image_filename);
   }
*/

//   string images_subdir="/data/ImageEngine/BostonBombing/clips_1_thru_133/";
//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
   string root_subdir=JAV_subdir;

   string images_subdir=root_subdir+"jpg_frames/";

   string faces_subdir=root_subdir+"faces/";
   filefunc::dircreate(faces_subdir);

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);

   timefunc::initialize_timeofday_clock();

   string output_filename=faces_subdir+"face_detections.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# Image filename			 Center.U Center.V Radius" << endl;
   outstream << endl;

   int i_start=0;
   cout << "Total number of input images = " << image_filenames.size() 
        << endl;
   cout << "Enter starting image number:" << endl;
   cin >> i_start;
   int image_skip=1;
//   int image_skip=50;
//   int image_skip=100;
   int n_images_processed=0;
   int n_images_w_faces=0;
   for (unsigned int i=i_start; i<image_filenames.size(); i+= image_skip)
   {
      double progress_frac=
         outputfunc::update_progress_fraction(i,100,image_filenames.size());
      cout << "i = " << i << 
         " Processing filename = " << image_filenames[i] << endl;
      outputfunc::print_elapsed_and_remaining_time(progress_frac);      
      n_images_processed++;

      vector<threevector> face_circles=classification_func::detect_faces(
         image_filenames[i]);
//      outputfunc::enter_continue_char();
      
      int n_faces=face_circles.size();
      cout << "Number faces detected = " << n_faces << endl;
      if (n_faces==0) continue;
      n_images_w_faces++;

      for (unsigned int f=0; f<face_circles.size(); f++)
      {
         threevector curr_face_circle=face_circles[f];
         twovector circle_center(
            curr_face_circle.get(0),curr_face_circle.get(1));
         double radius=curr_face_circle.get(2);

         cout << "f = " << f 
              << " center.U = " << circle_center.get(0)
              << " center.V = " << circle_center.get(1)
              << " radius = " << radius
              << endl;
         outstream << image_filenames[i] << "    "
                   << circle_center.get(0) << "     "
                   << circle_center.get(1) << "      "
                   << radius << endl;
      }
      cout << endl;

   } // loop over index i labeling image filenames
   filefunc::closefile(output_filename,outstream);

   cout << "*******************************************************" << endl;
   cout << "Total number of images processed = " << n_images_processed
        << endl;
   cout << "Number of images containing faces = " << n_images_w_faces << endl;
   cout << "*******************************************************" << endl;
   cout << endl;
   cout << "At end of program FINDFACES" << endl;
   outputfunc::print_elapsed_time();

}
