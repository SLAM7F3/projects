// ========================================================================
// Program EXTRACT_CHIPS 

//			./extract_chips

// ========================================================================
// Last updated on 7/22/16; 7/23/16; 7/29/16; 8/3/16
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

//   bool force_single_face_per_bbox_flag = true;
   bool force_single_face_per_bbox_flag = false;

   string faces_rootdir = "/data/TrainingImagery/faces/";
//   string face_images_subdir = faces_rootdir + "images/";
   string face_images_subdir = faces_rootdir + 
      "labeled_data/faces_14_testing_images/fullsized/";
//   string bbox_labels_filename = face_images_subdir+
//      "labeled_face_bboxes_sans_corrupted_imgs.txt";

// FAKE FAKE:  Weds Aug 3 at 1:37 pm
// Hardwire filename for input bbox labels:

   string bbox_labels_filename = faces_rootdir+"labeled_data/faces_14/"+
      "Aug3_faces_hands_testing_images_extracted_bboxes.txt";

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
   string output_chips_subdir = "./faces_14_chips/";
//   string output_chips_subdir = "./face_blur_chips/";
//   string output_chips_subdir = "./face_chips/";
   filefunc::dircreate(output_chips_subdir);

   string female_chips_subdir = output_chips_subdir+"female/";
   filefunc::dircreate(female_chips_subdir);
   string male_chips_subdir = output_chips_subdir+"male/";
   filefunc::dircreate(male_chips_subdir);
   string unknown_chips_subdir = output_chips_subdir+"unknown/";
   filefunc::dircreate(unknown_chips_subdir);

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
      string image_filename=face_images_subdir+"image_"+
         image_ID_str+".jpg";

      for(unsigned int b = 0; b < bboxes.size(); b++)
      {
         texture_rectangle* tr_ptr = new texture_rectangle(
            image_filename, NULL);
         int xdim = tr_ptr->getWidth();
         int ydim = tr_ptr->getHeight();

// Black out all face bboxes other than current one within current
// image:

         if(force_single_face_per_bbox_flag)
         {
            for(unsigned int b2 = 0; b2 < bboxes.size(); b2++)
            {
               if(b2 == b) continue;
               tr_ptr->fill_pixel_bbox(bboxes[b2], 0, 0, 0);
            }
         }

         bounding_box curr_bbox = bboxes[b];
         string attr_key = "gender";
         string attr_value = curr_bbox.get_attribute_value(attr_key);

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

//         double focus_measure = videofunc::avg_modified_laplacian(
//            px_center - 0.5 * px_extent, px_center + 0.5 * px_extent,
//            py_center - 0.5 * py_extent, py_center + 0.5 * py_extent,
//            tr_ptr);

//         bool filter_intensities_flag = true;
//         int color_channel_ID = 0;
//         double entropy = tr_ptr->compute_image_entropy(
//            px_center - 0.5 * px_extent, px_center + 0.5 * px_extent,
//            py_center - 0.5 * py_extent, py_center + 0.5 * py_extent,
//            filter_intensities_flag, color_channel_ID);
//         double focus_measure = entropy;


         string output_filename;
         if(attr_value.size() == 0)
         {
            output_filename = output_chips_subdir +"/" +
               +"face_"+image_ID_str+"_"
               +stringfunc::integer_to_string(face_ID++,5)
               +".png";

         }
         else
         {
            output_filename = output_chips_subdir + attr_value+"/" +
               attr_value+"_face_" 
//            +stringfunc::number_to_string(focus_measure)+"_"
               +stringfunc::integer_to_string(face_ID++,5)
               +".png";
         }
         

         bool horiz_flipped_flag = false;
         tr_ptr->write_curr_subframe(
            px_start, px_stop, py_start, py_stop, output_filename,
            horiz_flipped_flag);

         delete tr_ptr;

//         int max_xdim = 224;
//         int max_ydim = 224;
         int max_xdim = 96;
         int max_ydim = 96;
         videofunc::downsize_image(output_filename, max_xdim, max_ydim);

      } // loop over index b labeling bounding boxes for current image


      image_counter++;
   } // loop over annotated_bboxes_iter
}

