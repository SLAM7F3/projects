// ========================================================================
// Program EXTRACT_CHIPS

//			./extract_chips

// ========================================================================
// Last updated on 7/22/16
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
   using std::pair;
   using std::string;
   using std::vector;

   timefunc::initialize_timeofday_clock(); 
   std::set_new_handler(sysfunc::out_of_memory);

   bool ignore_hands_flag = true;

   string faces_rootdir = "/data/TrainingImagery/faces/";
   string labeled_faces_subdir = faces_rootdir + "images/";
   string bbox_labels_filename = labeled_faces_subdir+
      "labeled_face_hand_bboxes.txt";
   if(ignore_hands_flag)
   {
      bbox_labels_filename = labeled_faces_subdir+"labeled_face_bboxes.txt";
   }
   filefunc::ReadInfile(bbox_labels_filename);

   typedef map<string, vector<bounding_box> > ANNOTATED_BBOXES_MAP;
// independent string: image_ID_str
// dependent STL vector: annotated bboxes

   ANNOTATED_BBOXES_MAP annotated_bboxes_map;
   ANNOTATED_BBOXES_MAP::iterator annotated_bboxes_iter;

   bool first_image_flag = true;
   string image_ID_str = "";
   vector<bounding_box> annotated_bboxes;
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
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

   int face_ID = 0;
   string output_chips_subdir = "./face_chips/";
   filefunc::dircreate(output_chips_subdir);

   int image_counter = 0;
   int n_images = annotated_bboxes_map.size();
   for(annotated_bboxes_iter = annotated_bboxes_map.begin();
       annotated_bboxes_iter != annotated_bboxes_map.end();
       annotated_bboxes_iter++)
   {
      if(image_counter%100 == 0)
      {
         double progress_frac = double(image_counter)/double(n_images);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string image_ID_str = annotated_bboxes_iter->first;
      vector<bounding_box> bboxes = annotated_bboxes_iter->second;
      string image_filename=labeled_faces_subdir+"image_"+
         image_ID_str+".jpg";
      texture_rectangle* tr_ptr = new texture_rectangle(image_filename, NULL);
      int xdim = tr_ptr->getWidth();
      int ydim = tr_ptr->getHeight();

      for(unsigned int b = 0; b < bboxes.size(); b++)
      {
         bounding_box curr_bbox = bboxes[b];

         double mag_factor = 2;
         int px_center = curr_bbox.get_xcenter();
         int px_extent = curr_bbox.get_xextent();
         int px_start = px_center - mag_factor * 0.5 * px_extent;
         int px_stop = px_center + mag_factor * 0.5 * px_extent;
         px_start = basic_math::max(0, px_start);
         px_stop = basic_math::min(xdim-1, px_stop);

         int py_center = ydim - curr_bbox.get_ycenter();
         int py_extent = curr_bbox.get_yextent();
         int py_start = py_center - mag_factor * 0.5 * py_extent;
         int py_stop = py_center + mag_factor * 0.5 * py_extent;
         py_start = basic_math::max(0, py_start);
         py_stop = basic_math::min(ydim-1, py_stop);

         string output_filename=output_chips_subdir+
            "face_"+stringfunc::integer_to_string(face_ID++,5)+".png";
//         cout << "image_counter = " << image_counter
//              << " " << image_filename << endl;
//         cout << " b = " << b << endl;
//         cout << " px_start = " << px_start << " px_stop = " << px_stop
//              << " xdim = " << xdim << endl;
//         cout << " py_start = " << py_start << " py_stop = " << py_stop
//              << " ydim = " << ydim << endl;
         
         tr_ptr->write_curr_frame(
            px_start, px_stop, py_start, py_stop, output_filename);
      } // loop over index b labeling bounding boxes for current image
      image_counter++;
   } // loop over annotated_bboxes_iter

}

