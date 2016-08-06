// ========================================================================
// Program DRAW_GENDER_BBOXES takes in a text file containing facial
// bounding box coordinates and gender attributes for some set of
// images.  It colors the bounding boxes according to gender
// classification (grey = nonface, blue = male, red = female, green =
// unknown gender). Images annotated with the colored bboxes are
// written to an output subdirectory.

//			./draw_gender_bboxes

// ========================================================================
// Last updated on 8/5/16; 8/6/16
// ========================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "geometry/bounding_box.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

// ========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::ofstream;
   using std::pair;
   using std::string;
   using std::vector;

   timefunc::initialize_timeofday_clock(); 
   std::set_new_handler(sysfunc::out_of_memory);

   string faces_rootdir = "/data/TrainingImagery/faces/";
   string bbox_labels_filename = faces_rootdir+"labeled_data/faces_14/"+
      "updated_Aug3_faces_hands_testing_images_extracted_bboxes.txt";
   filefunc::ReadInfile(bbox_labels_filename);

   string images_subdir = faces_rootdir + 
      "labeled_data/faces_14/testing_images/";
   string annotated_images_subdir = images_subdir+"annotated_images/";
   filefunc::dircreate(annotated_images_subdir);
   
   vector<string> image_filenames = filefunc::image_files_in_subdir(
      images_subdir);

// First import face bounding boxes from bbox_labels_filename:

   typedef map<string, vector<bounding_box> > ANNOTATED_BBOXES_MAP;
// independent string: image_ID_str
// dependent STL vector: annotated bboxes

   ANNOTATED_BBOXES_MAP annotated_bboxes_map;
   ANNOTATED_BBOXES_MAP::iterator annotated_bboxes_iter;

   bool first_image_flag = true;
   string image_ID_str = "";
   vector<bounding_box> annotated_bboxes;

   for(unsigned int i = 0 ; i < filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];

      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         curr_line);
      if(substrings[0] == "Image:")
      {
         if(first_image_flag)
         {
            first_image_flag = false;
         }
         else
         {
            // Save previous image info into data structures
            annotated_bboxes_map[image_ID_str] = annotated_bboxes;
            annotated_bboxes.clear();
         }
         annotated_bboxes.clear();
         image_ID_str = substrings[6];
      }
      else
      {
         vector<string> substrings = 
            stringfunc::decompose_string_into_substrings(curr_line);
         string bbox_label = substrings[1];
         int px_min = stringfunc::string_to_number(substrings[2]);
         int px_max = stringfunc::string_to_number(substrings[3]);
         int py_min = stringfunc::string_to_number(substrings[4]);
         int py_max = stringfunc::string_to_number(substrings[5]);
         bounding_box curr_bbox(px_min, px_max, py_min, py_max);

         colorfunc::Color bbox_color = colorfunc::black;
         if(bbox_label == "face")
         {
            bbox_color = colorfunc::red;
         }
         else if(bbox_label == "hand")
         {
            bbox_color = colorfunc::cyan;
         }

         curr_bbox.set_label(bbox_label);
         curr_bbox.set_color(bbox_color);

// Search for any bbox attribute key-value pairs:

         int n_attribute_pairs = (substrings.size() - 6)/2;
         for(int ap = 0; ap < n_attribute_pairs; ap++)
         {
            string attr_key = substrings[6 + ap * 2];
            string attr_value = substrings[6 + ap * 2 + 1];
            curr_bbox.set_attribute_value(attr_key, attr_value);
         }

         curr_bbox.set_ID(annotated_bboxes.size());
         annotated_bboxes.push_back(curr_bbox);

      } // substrings[0] == "Image:" conditional
   } // loop over index i labeling lines in detections text file

   // Save final image info into data structures
   annotated_bboxes_map[image_ID_str] = annotated_bboxes;

// Loop over test images starts here:

   int istart = 0;
   int istop = image_filenames.size();
   for(int i = istart; i < istop; i++)
   {
      if(i%50 == 0)
      {
         double progress_frac = double(i-istart)/double(istop-istart);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string image_filename = image_filenames[i];
      texture_rectangle* tr_ptr = new texture_rectangle(image_filename, NULL);
      string image_basename = filefunc::getbasename(image_filename);

      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         image_basename,"_.");
      string image_ID_str = substrings[1];
      annotated_bboxes_iter = annotated_bboxes_map.find(image_ID_str);
      if(annotated_bboxes_iter == annotated_bboxes_map.end())
      {
         cout << "cannot find image_ID_str = " << image_ID_str << endl;
         continue;
      }

      vector<bounding_box> annotated_bboxes = annotated_bboxes_iter->second;
      for(unsigned int b = 0; b < annotated_bboxes.size(); b++)
      {
         string gender = annotated_bboxes[b]. get_attribute_value("gender");
         string score = annotated_bboxes[b]. get_attribute_value("score");

         colorfunc::Color gender_color = colorfunc::black;
         if(gender == "non")
         {
//            gender_color = colorfunc::grey;
            gender_color = colorfunc::white;
         }
         else if(gender == "male")
         {
            gender_color = colorfunc::blue;
         }
         else if(gender == "female")
         {
            gender_color = colorfunc::red;
         }
         else if(gender == "unsure")
         {
            gender_color = colorfunc::green;
         }

         int thickness = 0;
         if(tr_ptr->getWidth() > 1000 || tr_ptr->getHeight() > 1000)
         {
            thickness = 1;
         }
         tr_ptr->draw_pixel_bbox(annotated_bboxes[b], gender_color, thickness);

      } // loop over index b labeling annotated bboxes

      string output_filename = annotated_images_subdir+image_basename;
      tr_ptr->write_curr_frame(output_filename);
      delete tr_ptr;
   } // loop over index i labeling image filenames

   string banner="Exported annotated images to "+annotated_images_subdir;
   outputfunc::write_banner(banner);
}

