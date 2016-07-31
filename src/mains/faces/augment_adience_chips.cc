// ========================================================================
// Program AUGMENT_ADIENCE_CHIPS imports a set of 106x106 image chips
// extracted from the Adience facial data set via program
// ADIENCE_IMAGES.  Since Adience faces originate from flickr imagery
// (and contains lots of selfies!), we do not believe they are as
// stressing as our own facial image chips harvested from more random
// internet imagery.  So we reserve only 1.25% and 3.75% of the input
// Adience chips for validation and testing.  And we only take only 2
// augmented copies of each remaining Adience image for training
// rather than 4 as in AUGMENT_CHIPS.  All image chips exported by
// this program are intentionally rotated through some relatively
// small angle in order to counter the facial alignment of the input
// Adience image chips.

//			./augment_adience_chips

// ========================================================================
// Last updated on 7/31/16
// ========================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "geometry/bounding_box.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

// ========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::pair;
   using std::string;
   using std::vector;

   timefunc::initialize_timeofday_clock(); 
   std::set_new_handler(sysfunc::out_of_memory);

   bool rgb2grey_flag = true;		       // default as of Jun 14
   double rgb2grey_threshold = 0.2;            // default as of Jun 14
   double noise_threshold = 0.5;               // default as of Jun 14

   string faces_subdir = "/data/peter_stuff/imagery/faces/";
   string adience_chips_subdir = faces_subdir + "adiencefaces_106x106/";
   vector<string> image_filenames = filefunc::image_files_in_subdir(
      adience_chips_subdir);
   int n_images = image_filenames.size();
   cout << "n_images = " << n_images << endl;
   
   int face_ID_start = 0;
   int face_ID = face_ID_start;

   string output_chips_subdir = "./augmented_adience_face_chips/";
   filefunc::dircreate(output_chips_subdir);

   string training_chips_subdir = output_chips_subdir+"training/";
   filefunc::dircreate(training_chips_subdir);
   string validation_chips_subdir = output_chips_subdir+"validation/";
   filefunc::dircreate(validation_chips_subdir);
   string testing_chips_subdir = output_chips_subdir+"testing/";
   filefunc::dircreate(testing_chips_subdir);

   string female_training_chips_subdir = training_chips_subdir+"female/";
   filefunc::dircreate(female_training_chips_subdir);
   string male_training_chips_subdir = training_chips_subdir+"male/";
   filefunc::dircreate(male_training_chips_subdir);

   string female_validation_chips_subdir = validation_chips_subdir+"female/";
   filefunc::dircreate(female_validation_chips_subdir);
   string male_validation_chips_subdir = validation_chips_subdir+"male/";
   filefunc::dircreate(male_validation_chips_subdir);

   string female_testing_chips_subdir = testing_chips_subdir+"female/";
   filefunc::dircreate(female_testing_chips_subdir);
   string male_testing_chips_subdir = testing_chips_subdir+"male/";
   filefunc::dircreate(male_testing_chips_subdir);

   int image_counter = 0;
   for(int n = 0; n < n_images; n++)
   {
      if(image_counter%100 == 0)
      {
         double progress_frac = double(image_counter)/double(n_images);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string image_filename=image_filenames[n];
      string image_basename=filefunc::getbasename(image_filename);
      
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         image_basename,"_");
      string gender_value = substrings[0];

      texture_rectangle* tr_ptr = new texture_rectangle(
         image_filename, NULL);
      int xdim = tr_ptr->getWidth();
      int ydim = tr_ptr->getHeight();

      double tvt = nrfunc::ran1();
      string classification_value = "training";
      if(tvt < 0.0125)
      {
         classification_value = "validation";
      }
      else if (tvt >= 0.0125 && tvt < 0.05)
      {
         classification_value = "testing";
      }

      texture_rectangle* tr2_ptr = new texture_rectangle(
         xdim, ydim, 1, 3, NULL);

      int n_augmentations_per_chip = 1;
      if(classification_value == "training")
      {
         n_augmentations_per_chip = 2;
      }

      for(int a = 0; a < n_augmentations_per_chip; a++)
      {

         tr2_ptr->copy_RGB_values(tr_ptr);

         bool horiz_flipped_flag = false;
         if(a%2==1)
         {
            horiz_flipped_flag = true;
         }
         
         if(a > 0)
         {
            double delta_h = -30 + nrfunc::ran1() * 60;
            double delta_s = -0.25 + nrfunc::ran1() * 0.5;

// If rgb2grey_flag == true, effectively reset saturation to zero for
// some relatively small percentage of output tiles:

            if(rgb2grey_flag && nrfunc::ran1() < rgb2grey_threshold)
            {
               delta_s = -1.5;
            }
            double delta_v = -0.25 + nrfunc::ran1() * 0.5;

            tr2_ptr->globally_perturb_hsv(
               0, xdim - 1, 0, ydim - 1,
               delta_h, delta_s, delta_v);
         }

         if(a > 0 && nrfunc::ran1() >= noise_threshold)
         {
            double noise_frac= 0.05 * nrfunc::ran1(); 
            double sigma = noise_frac * 255;
            tr2_ptr->add_gaussian_noise(
               0, xdim - 1, 0, ydim - 1, sigma);
         }

         string output_subdir=output_chips_subdir;
         output_subdir += classification_value+"/"+gender_value+"/";
         string output_filename= output_subdir + 
            gender_value+"_adience_face_"
            +stringfunc::integer_to_string(a,2)+"_"
            +stringfunc::integer_to_string(face_ID++,5)+".jpg";
         
         tr2_ptr->write_curr_subframe(
            0, xdim - 1, 0, ydim - 1, output_filename, horiz_flipped_flag);

         Magick::Image IM_image;
         IM_image.backgroundColor(Magick::Color(0,0,0));
         if(!videofunc::import_IM_image(output_filename, IM_image))
         {
            cout << "Unable to import " << output_filename << endl;
            continue;
         }

         if(classification_value != "training")
         {
            int new_xdim = 96;
            int new_ydim = 96;
            Magick::Geometry newSize(new_xdim, new_ydim);
            IM_image.zoom(newSize);
         }

// Adience images appear to have all been rotated so that the face
// appears level relative to the horizontal axis.  So we intentionally
// introduce random rotations into training, validation and testing
// chips in order to remove this artificial regularity:

         double theta = 30 * 2 * (nrfunc::ran1() - 0.5);
         videofunc::crop_rotate_image(IM_image,theta);
         videofunc::export_IM_image(output_filename, IM_image);

      } // loop over index a labeling augmentations
         
      delete tr2_ptr;
      delete tr_ptr;
      image_counter++;
   } // loop over index n labeling input adience 106x106 image chips

   int face_ID_stop = face_ID;
   
   cout << "Starting face ID = " << face_ID_start << endl;
   cout << "Stopping face ID = " << face_ID_stop << endl;
}

