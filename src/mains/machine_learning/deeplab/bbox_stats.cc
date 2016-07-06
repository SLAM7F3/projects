// ==========================================================================
// Program BBOX_STATS
// ==========================================================================
// Last updated on 6/28/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <dlib/image_processing.h>
#include <dlib/data_io.h>

#include "math/basic_math.h"
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

   string banner="Starting BBOX_STATS program";
   outputfunc::write_big_banner(banner);

/*
   if(argc != 2)
   {
      cout << "Must pass XML filename (e.g. 'all_labeled_face_hand_images.xml') as command-line argument" << endl;
      exit(-1);
   }
*/
   string xml_filename = "./all_labeled_face_hand_images.xml";
   cout << "xml_filename = " << xml_filename << endl;

   dlib::image_dataset_metadata::dataset input_data;
   dlib::image_dataset_metadata::load_image_dataset_metadata(
      input_data, xml_filename);

   int n_images = input_data.images.size();
   cout << "Number of images = " << n_images << endl;   

   int n_images_with_no_faces_nor_hands = 0;
   int n_face_bboxes = 0;
   int n_hand_bboxes = 0;
   int n_images_hand_intersects_face = 0;
   int n_face_bboxes_intersected_by_hand_bboxes = 0;
   
   for(int i = 0; i < n_images; i++)
   {
      if(i%50 == 0)
      {
         double progress_frac = double(i)/double(n_images);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string image_filename=input_data.images[i].filename;
//      cout << "image_filename = " << image_filename << endl;
      string image_basename=filefunc::getbasename(image_filename);
//      cout << "image_basename = " << image_basename << endl;
      string image_ID_str = image_basename.substr(6,5);
//      cout << "image_ID_str = " << image_ID_str << endl;
      int image_ID = stringfunc::string_to_number(image_ID_str);
//      cout << "image_ID = " << image_ID << endl;

      vector<dlib::image_dataset_metadata::box> boxes = 
         input_data.images[i].boxes;

      if(boxes.size() == 0)
      {
         n_images_with_no_faces_nor_hands++;
      }

      bool some_hand_intersects_some_face_flag = false;
      for(unsigned int b = 0; b < boxes.size(); b++)
      {
         dlib::image_dataset_metadata::box curr_box = boxes[b];
         string curr_box_label = curr_box.label;
         dlib::rectangle curr_rect = curr_box.rect;
         int px_min = curr_rect.left();
         int px_max = curr_rect.right();
         int py_min = curr_rect.top();
         int py_max = curr_rect.bottom();
         bounding_box curr_bbox(px_min, px_max, py_min, py_max);

         if(curr_box_label == "face")
         {
            n_face_bboxes++;

            bool hand_bbox_intersects_face_bbox = false;
            for(unsigned int b2 = 0; b2 < boxes.size(); b2++)
            {
               dlib::image_dataset_metadata::box next_box = boxes[b2];
               string next_box_label = next_box.label;
               if(next_box_label == "face") continue;

               dlib::rectangle next_rect = next_box.rect;
               int qx_min = next_rect.left();
               int qx_max = next_rect.right();
               int qy_min = next_rect.top();
               int qy_max = next_rect.bottom();
               bounding_box next_bbox(qx_min, qx_max, qy_min, qy_max);

               bounding_box intersection_bbox;
               if(curr_bbox.bbox_intersection(next_bbox, intersection_bbox))
               {
                  some_hand_intersects_some_face_flag = true;
                  hand_bbox_intersects_face_bbox = true;
                  break;
               }
            } // loop over index b2
            if(hand_bbox_intersects_face_bbox)
            {
               n_face_bboxes_intersected_by_hand_bboxes++;
            }
         } // curr_bbox_label == face conditional
         

         if(curr_box_label == "hand")
         {
            n_hand_bboxes++;
         }

/*
// FAKE FAKE:  Hack as of Tues Jun 14

         if(ignore_hands_flag && curr_box_label == "hand") continue;

         dlib::rectangle curr_rect = curr_box.rect;
         unsigned int px_min = curr_rect.left();
         unsigned int px_max = curr_rect.right();
         unsigned int py_min = curr_rect.top();
         unsigned int py_max = curr_rect.bottom();
         unsigned int bbox_width = px_max - px_min;
         unsigned int bbox_height = abs(py_max - py_min);
         int n_bbox_pixels = bbox_width * bbox_height;

// Recall to good approximation, face bbox aspect ratio = 0.731
// --> w = sqrt(bbox_aspect_ratio * n_bbox_pixels);

         const double bbox_aspect_ratio = 0.731; // Updated on 6/5/16
         double pixel_width = sqrt(bbox_aspect_ratio * n_bbox_pixels);
         pixel_widths.push_back(pixel_width);

// Count number of segmented pixels located inside current bbox:

         int n_bbox_segmented_pixels = 0;
         for(unsigned int qy = py_min; qy < py_min + bbox_height; qy++)
         {
            for(unsigned int qx = px_min; qx < px_min + bbox_width; qx++)
            {
               double h, s, v;
               tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
               if(s > 0)
               {
                  n_bbox_segmented_pixels++;
               }
            }
         }
//         cout << "b = " << b << " n_bbox_pixels = " << n_bbox_pixels
//              << " n_bbox_segmented_pixels = " << n_bbox_segmented_pixels
//              << endl;

         double curr_bbox_recall = double(n_bbox_segmented_pixels)/
            n_bbox_pixels;

// Draw truth bbox into texture rectangle:

         bounding_box bbox(px_min, px_max, py_min, py_max);
         colorfunc::Color bbox_color = colorfunc::yellow;
         tr_ptr->draw_pixel_bbox(bbox, bbox_color);

         bounding_box offset_bbox(
            px_min + 1, px_max + 1, py_min + 1, py_max + 1);
         tr_ptr->draw_pixel_bbox(offset_bbox, bbox_color);
         offset_bbox = bounding_box(
            px_min - 1, px_max - 1, py_min - 1, py_max - 1);
         tr_ptr->draw_pixel_bbox(offset_bbox, bbox_color);
*/

      } // loop over index b labeling bboxes for current image

      if(some_hand_intersects_some_face_flag)
      {
         n_images_hand_intersects_face++;
      }
      

   } // loop over index i labeling images

   cout << "n_images = " << n_images << endl;
   cout << "n_images_with_no_faces_nor_hands = " 
        << n_images_with_no_faces_nor_hands << endl;
   cout << "n_face_bboxes = " << n_face_bboxes << endl;
   cout << "n_hand_bboxes = " << n_hand_bboxes << endl;
   cout << "n_images_hand_intersects_face = " 
        << n_images_hand_intersects_face << endl;
   cout << "n_face_bboxes_intersected_by_hand_bboxes = "
        << n_face_bboxes_intersected_by_hand_bboxes << endl;
   double hand_on_face_frac = double(n_face_bboxes_intersected_by_hand_bboxes)/
      n_face_bboxes;
   cout << "hand_on_face_frac = " << hand_on_face_frac << endl;
}
