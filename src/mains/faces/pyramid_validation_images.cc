// ====================================================================
// Program PYRAMID_VALIDATION_IMAGES imports all validation images and
// object bounding boxes specified within an input XML file created by
// Davis King's IMGLAB tool.  If user explicitly requests image
// resizing be performed, this program downsizes by a factor of 2 each
// image within the input folder.  It also upsamples by a factor of 2
// each input image.  PYRAMID_VALIDATION_IMAGES exports new XML files
// for the downsized and upsized images.

//    		        ./pyramid_validation_images

// ====================================================================
// Last updated on 5/13/16; 5/23/16; 6/6/16; 6/18/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <dlib/array.h>
#include <dlib/array2d.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/cmd_line_parser.h>
#include <dlib/data_io.h>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char** argv)
{  
   timefunc::initialize_timeofday_clock(); 

   string faces_rootdir = "/data/TrainingImagery/faces/";
   string labeled_data_subdir=faces_rootdir+"labeled_data/";   
   
   int faces_ID = -1;
   cout << "Enter faces ID (-1 for default):" << endl;
   cin >> faces_ID;
   string faces_subdir=labeled_data_subdir+"faces";
   if(faces_ID >= 0)
   {
      faces_subdir += "_"+stringfunc::integer_to_string(faces_ID,2);
   }
   filefunc::add_trailing_dir_slash(faces_subdir);
   string validation_images_subdir=faces_subdir+"validation_images/";
   string validation_xml_filename=validation_images_subdir+
      "validation_images.xml";

   string halfsized_validation_subdir = validation_images_subdir+"halfsized/";
   string fullsized_validation_subdir = validation_images_subdir+"fullsized/";
   string doublesized_validation_subdir = validation_images_subdir+"doublesized/";
   filefunc::dircreate(halfsized_validation_subdir);
   filefunc::dircreate(fullsized_validation_subdir);
   filefunc::dircreate(doublesized_validation_subdir);

   string halfsized_validation_xml_filename=
      halfsized_validation_subdir+"halfsized_validation_images.xml";
   string fullsized_validation_xml_filename=
      fullsized_validation_subdir+"fullsized_validation_images.xml";
   string doublesized_validation_xml_filename=
      doublesized_validation_subdir+"doublesized_validation_images.xml";

   bool just_xml_output_flag = false;
/*
   string just_xml_input;
   cout << "Enter 'x' to generate just XML files without exporting resized images" << endl;
   cin >> just_xml_input;
   if(just_xml_input == "x" || just_xml_input == "X")
   {
      just_xml_output_flag = true;
   }
*/

   dlib::image_dataset_metadata::dataset validation_dataset;
   dlib::image_dataset_metadata::load_image_dataset_metadata(
      validation_dataset, validation_xml_filename);
   cout << "Validation_dataset.images.size = " 
        << validation_dataset.images.size() << endl;

   int n_validation_images = validation_dataset.images.size();
   vector<string> image_basenames;
   vector<vector<dlib::image_dataset_metadata::box> > object_bboxes;

   for(int n = 0; n < n_validation_images; n++)
   {
      image_basenames.push_back(validation_dataset.images[n].filename);
      vector<dlib::image_dataset_metadata::box> bboxes;
      for(unsigned int b = 0; b < validation_dataset.images[n].boxes.size(); 
          b++)
      {
         bboxes.push_back(validation_dataset.images[n].boxes[b]);
      }
      object_bboxes.push_back(bboxes);
   }

   dlib::image_dataset_metadata::dataset doublesized_validation_data;
   dlib::image_dataset_metadata::dataset halfsized_validation_data;
   doublesized_validation_data.name="doublesized_validation_data";
   halfsized_validation_data.name="halfsized_validation_data";

   int istart = 0;
//   int istop = 5;
   int istop = n_validation_images;
   for(int i = istart; i < istop; i++)
   {
      string image_basename=image_basenames[i];
      string image_filename=validation_images_subdir + image_basename;
      cout << "i = " << i << " image_basename = " << image_basename << endl;
      string basename_substr=image_basename.substr(0,6);
      if(basename_substr != "image_") continue;

      if(i%50 == 0)
      {
         double progress_frac = double(i)/double(n_validation_images);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      unsigned int width, height;
      imagefunc::get_image_width_height(image_filename,width,height);

      string halfsized_image_filename=halfsized_validation_subdir+"halfsized_"+
         stringfunc::prefix(image_basename)+".jpg";
      if(!just_xml_output_flag)
      {
         videofunc::resize_image(
            image_filename,width,height,width/2,height/2,
            halfsized_image_filename);
      }
      halfsized_validation_data.images.push_back(
         filefunc::getbasename(halfsized_image_filename));

      string doublesized_image_filename=
         doublesized_validation_subdir+"doublesized_"+
         stringfunc::prefix(image_basename)+".jpg";
      if(!just_xml_output_flag)
      {
         videofunc::resize_image(
            image_filename,width,height,2*width,2*height,
            doublesized_image_filename);
      }
      doublesized_validation_data.images.push_back(
         filefunc::getbasename(doublesized_image_filename));

// Set bboxes for halfsized and doublesized validation images:

      vector<vector<dlib::image_dataset_metadata::box> > 
         halfsized_object_bboxes, doublesized_object_bboxes;

      for(unsigned int b = 0; b < object_bboxes[i].size(); b++)
      {
         dlib::image_dataset_metadata::box curr_bbox = object_bboxes[i].at(b);
         dlib::rectangle curr_rect = curr_bbox.rect;
         unsigned int px_min = curr_rect.left();
         unsigned int px_max = curr_rect.right();
         unsigned int py_min = curr_rect.top();
         unsigned int py_max = curr_rect.bottom();

         dlib::rectangle half_rect;
         half_rect.set_left(0.5 * px_min);
         half_rect.set_right(0.5 * px_max);
         half_rect.set_top(0.5 * py_min);
         half_rect.set_bottom(0.5 * py_max);
         dlib::image_dataset_metadata::box half_bbox(half_rect);
         half_bbox.label = curr_bbox.label;
         halfsized_validation_data.images.back().boxes.push_back(half_bbox);

         dlib::rectangle double_rect;
         double_rect.set_left(2 * px_min);
         double_rect.set_right(2 * px_max);
         double_rect.set_top(2 * py_min);
         double_rect.set_bottom(2 * py_max);
         dlib::image_dataset_metadata::box double_bbox(double_rect);
         double_bbox.label = curr_bbox.label;
         doublesized_validation_data.images.back().boxes.push_back(
            double_bbox);
      } // loop over index b labeling bboxes for current validation image

      string unix_cmd="mv "+image_filename+" "+fullsized_validation_subdir;
      sysfunc::unix_command(unix_cmd);

   } // loop over index i labeling validation images

   string unix_cmd = "cp "+validation_xml_filename+" "+
      fullsized_validation_subdir+"fullsized_validation_images.xml";
   sysfunc::unix_command(unix_cmd);
   
   cout << "halfsized_validation_data.images.size() = "
        << halfsized_validation_data.images.size() << endl;
   cout << "doublesized_validation_data.images.size() = "
        << doublesized_validation_data.images.size() << endl;

   dlib::image_dataset_metadata::save_image_dataset_metadata(
      halfsized_validation_data, halfsized_validation_xml_filename);
   dlib::image_dataset_metadata::save_image_dataset_metadata(
      doublesized_validation_data, doublesized_validation_xml_filename);

   cout << "Exported half-sized images to " << halfsized_validation_subdir 
        << endl;
   cout << "Exported double-sized images to " << doublesized_validation_subdir 
        << endl;
   cout << "Moved full-sized images to " << fullsized_validation_subdir << endl;
}

