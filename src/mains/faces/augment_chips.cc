// ========================================================================
// Program AUGMENT_CHIPS

//			./augment_chips

// ========================================================================
// Last updated on 7/22/16; 7/23/16
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

   string faces_rootdir = "/data/TrainingImagery/faces/";
   string labeled_faces_subdir = faces_rootdir + "images/";
//   string bbox_labels_filename = labeled_faces_subdir+"labeled_face_bboxes.txt";
   string bbox_labels_filename = labeled_faces_subdir+
      "labeled_face_bboxes_sans_corrupted_imgs.txt";
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
   string output_chips_subdir = "./augmented_face_chips/";
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

         texture_rectangle* tr2_ptr = new texture_rectangle(
            xdim, ydim, 1, 3, NULL);

         int n_augmentations_per_chip = 4;
         for(int a = 0; a < n_augmentations_per_chip; a++)
         {
            tr2_ptr->copy_RGB_values(tr_ptr);
            
            int delta_x = 2 * (nrfunc::ran1()-0.5) * 0.25 * px_extent;

// Intentionally bias vertical translation generally in the +gravity
// direction:
            int delta_y = (2 * nrfunc::ran1()-1.25) * 0.25 * py_extent;

            int qx_start = px_start + delta_x;
            int qx_stop = px_stop + delta_x;
            int qy_start = py_start + delta_y;
            int qy_stop = py_stop + delta_y;

            qx_start = basic_math::max(qx_start, 0);
            qx_stop = basic_math::min(qx_stop, xdim - 1);
            qy_start = basic_math::max(qy_start, 0);
            qy_stop = basic_math::min(qy_stop, ydim - 1);

            bool horiz_flipped_flag = false;
            if(a%2==1)
            {
               horiz_flipped_flag = true;
            }

            if(a > 0)
            {
               double delta_h = -25 + nrfunc::ran1() * 50;
               double delta_s = -0.2 + nrfunc::ran1() * 0.4;

// If rgb2grey_flag == true, effectively reset saturation to zero for
// some relatively small percentage of output tiles:

               if(rgb2grey_flag && nrfunc::ran1() < rgb2grey_threshold)
               {
                  delta_s = -1.5;
               }
               double delta_v = -0.2 + nrfunc::ran1() * 0.4;
               
               tr2_ptr->globally_perturb_hsv(
                  qx_start, qx_stop, 0, ydim - 1,
//                   qx_start, qx_stop, qy_start, qy_stop,
                  delta_h, delta_s, delta_v);
            }

            if(nrfunc::ran1() >= noise_threshold)
            {
               double noise_frac= 0.045 * nrfunc::ran1(); 
               double sigma = noise_frac * 255;
               tr2_ptr->add_gaussian_noise(
                  qx_start, qx_stop, qy_start, qy_stop, sigma);
            }

            string output_filename=output_chips_subdir + 
               attr_value+"_face_" 
               +stringfunc::integer_to_string(face_ID++,5)
               +".png";

            tr2_ptr->write_curr_subframe(
               qx_start, qx_stop, qy_start, qy_stop, output_filename,
               horiz_flipped_flag);

            int max_xdim = 224;
            int max_ydim = 224;
            videofunc::downsize_image(output_filename, max_xdim, max_ydim);

         } // loop over index a labeling augmentations
         delete tr2_ptr;

      } // loop over index b labeling bounding boxes for current image

      delete tr_ptr;
      image_counter++;
   } // loop over annotated_bboxes_iter

}
