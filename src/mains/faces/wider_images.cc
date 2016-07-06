// ====================================================================
// Program WIDER_IMGS imports the XML files for face bboxes within 
// training and validation WIDER images.  

// 			 ./wider_images
// ====================================================================
// Last updated on 5/22/16; 6/6/16; 6/16/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <dlib/array.h>
#include <dlib/array2d.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/cmd_line_parser.h>
#include <dlib/data_io.h>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char** argv)
{  
   string wider_subdir="/data/peter_stuff/imagery/faces/WIDER/";
   string images_subdir=wider_subdir+"images/";
   string homogenized_images_subdir=wider_subdir+"homogenized_images/";
   filefunc::dircreate(homogenized_images_subdir);

   string training_xml_filename=wider_subdir+"train_annotations.xml";
   string validation_xml_filename=wider_subdir+"val_annotations.xml";
   string homogenized_xml_filename=homogenized_images_subdir+"homogenized.xml";
   
   dlib::command_line_parser parser;
   dlib::array<dlib::array2d<unsigned char> > images;
   vector<vector<dlib::rectangle> > object_locations;

   dlib::image_dataset_file train_img_dataset_file(training_xml_filename);
   dlib::image_dataset_file val_img_dataset_file(validation_xml_filename);

   dlib::image_dataset_metadata::dataset training_data, validation_data,
      homogenized_data;
   dlib::image_dataset_metadata::load_image_dataset_metadata(
      training_data, training_xml_filename);
   dlib::image_dataset_metadata::load_image_dataset_metadata(
      validation_data, validation_xml_filename);

   int n_wider_training_images = training_data.images.size();
   int n_wider_validation_images = validation_data.images.size();
   int n_wider_total_images = 
      n_wider_training_images + n_wider_validation_images;

   const int min_bbox_width = 18;

/*
// WIDER validation images subset:  
//   (2356 images have bboxes with widths exceeding min_bbox_width)

   int counter_start = 8686;
   int delta_counter = 0;

   for(int i = 0; i < n_wider_validation_images; i++)
   {
      int curr_image_ID = i;
      string currimage_filename = wider_subdir + 
         validation_data.images[curr_image_ID].filename;
      vector<dlib::image_dataset_metadata::box> boxes = 
         validation_data.images[curr_image_ID].boxes;

      bool skip_wider_image = false;
      for(unsigned int b = 0; b < boxes.size(); b++)
      {
         dlib::rectangle curr_rect = boxes[b].rect;
         int px_min = curr_rect.left();
         int px_max = curr_rect.right();
         int py_min = curr_rect.top();
         int py_max = curr_rect.bottom();
         int width = px_max - px_min;
         int height = py_max - py_min;
         if(width < min_bbox_width)
         {
            skip_wider_image = true;
            break;
         }
      }
      if(skip_wider_image) continue;

      string homogenized_image_basename=
         "image_"+stringfunc::integer_to_string(counter_start+delta_counter,5)
         +".jpg";
      string homogenized_image_filename=homogenized_images_subdir+
         homogenized_image_basename;

// Reset image filename to homogenized form:

      validation_data.images[curr_image_ID].filename = 
         homogenized_image_basename;
      homogenized_data.images.push_back(validation_data.images[curr_image_ID]);
      string unix_cmd="cp "+currimage_filename+" "+homogenized_image_filename;
      sysfunc::unix_command(unix_cmd);
      cout << unix_cmd << endl;
      delta_counter++;
   }
*/


// WIDER training images subset:

   int counter_start = 11042;
   int delta_counter = 0;

// Raw WIDER training images are ordered by categories (e.g. parade
// images, celebrity images, etc).  So we'll work with a randomzied
// ordering of WIDER training imagery:

   vector<int> image_indices = 
      mathfunc::random_sequence(n_wider_training_images);

   for(int i = 0; i < n_wider_training_images; i++)
   {
//       int curr_image_ID = i;
      int curr_image_ID = image_indices[i];
      string currimage_filename = wider_subdir + 
         training_data.images[curr_image_ID].filename;
      vector<dlib::image_dataset_metadata::box> boxes = 
         training_data.images[curr_image_ID].boxes;

      bool skip_wider_image = false;
      for(unsigned int b = 0; b < boxes.size(); b++)
      {
         dlib::rectangle curr_rect = boxes[b].rect;
         int px_min = curr_rect.left();
         int px_max = curr_rect.right();
         int py_min = curr_rect.top();
         int py_max = curr_rect.bottom();
         int width = px_max - px_min;
         int height = py_max - py_min;
         if(width < min_bbox_width)
         {
            skip_wider_image = true;
            break;
         }
      }
      if(skip_wider_image) continue;

      string homogenized_image_basename=
         "image_"+stringfunc::integer_to_string(counter_start+delta_counter,5)
         +".jpg";
      string homogenized_image_filename=homogenized_images_subdir+
         homogenized_image_basename;

// Reset image filename to homogenized form:

      training_data.images[curr_image_ID].filename = 
         homogenized_image_basename;
      homogenized_data.images.push_back(training_data.images[curr_image_ID]);
      string unix_cmd="cp "+currimage_filename+" "+homogenized_image_filename;
      sysfunc::unix_command(unix_cmd);
      cout << unix_cmd << endl;
      delta_counter++;
   }

   dlib::image_dataset_metadata::save_image_dataset_metadata(
      homogenized_data, homogenized_xml_filename);
}

