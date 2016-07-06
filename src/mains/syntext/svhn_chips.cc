// ==========================================================================
// Program SVHN_CHIPS imports JSON files containing Google Street View
// House Number image chip metadata.  For each PNG image specified
// within an input JSON file, SVHN_CHIPS excises a padded version of
// the digit image chip.  Image chips below a minimal reasonable pixel
// size are ignored.

// 	                    ./svhn_chips

// ==========================================================================
// Last updated on 1/18/16; 1/26/16
// ==========================================================================

#include <iostream>
#include <Magick++.h>
#include <string>
#include <vector>
#include "geometry/bounding_box.h"
#include "general/filefuncs.h"
#include "graphs/cppJSON.h"
#include "general/outputfuncs.h"
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

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   timefunc::initialize_timeofday_clock();

   //   string MoreData_subdir = "/media/MoreData/";
   string MoreData_subdir = "/media/pcho/MoreData/";
   string training_imagery_subdir = MoreData_subdir+"TrainingImagery/";
   string svhn_subdir = training_imagery_subdir+"svhn/";
//   string category = "test/";
//   string category = "train/";
   string category = "extra/";
   string image_subdir = svhn_subdir+category;
   
   string json_filename=image_subdir+"digitStruct.json";
   string svhn_chips_subdir="./svhn_image_chips/";
   filefunc::dircreate(svhn_chips_subdir);

   cppJSON* cppJSON_ptr=new cppJSON();
   string json_string=cppJSON_ptr->get_JSON_string_from_JSON_file(
      json_filename);
   cJSON* root_ptr=cppJSON_ptr->parse_json(json_string);
   cppJSON_ptr->generate_JSON_tree();
   cppJSON_ptr->extract_key_value_pairs(root_ptr);

   unsigned int n_JSON_objects=cppJSON_ptr->get_n_objects();
   cout << "n_JSON_objects = " << n_JSON_objects << endl;

   vector<bounding_box> curr_img_bboxes;
   vector<string> curr_img_labels;

   int svhn_chip_counter = 0;
   unsigned int n_start = 0;
   unsigned int n_stop = n_JSON_objects;
   for(unsigned int n = n_start; n < n_stop; n++)
   {
      if ((n-n_start)%1000 == 0)
      {
         cout << "Processing n = " << n << " n_start = " << n_start 
              << " n_stop = " << n_stop << endl;
         double progress_frac = double(n - n_start)/(n_stop-n_start);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      vector<cppJSON::KEY_VALUE_PAIR> key_value_pairs = cppJSON_ptr->
         get_object_key_value_pairs(n);

//      cout << "JSON node n = " << n << endl;      
      double bbox_width, bbox_height, bbox_top, bbox_left;
      string bbox_label;
      bbox_width = bbox_height = bbox_top = bbox_left = -1;

      for(unsigned int k = 0; k < key_value_pairs.size(); k++)
      {
         cppJSON::KEY_VALUE_PAIR curr_key_value = key_value_pairs[k];
         string key = curr_key_value.first;
         string value = curr_key_value.second;
         
//         cout << "key = " << key << " value = " << value << endl;

         if(key == "filename")
         {
            string img_basename=value.substr(1,value.size() - 2);
            string curr_img_filename = image_subdir+img_basename;
            texture_rectangle curr_tr(curr_img_filename, NULL);
//            cout << "curr_img_filename = " << curr_img_filename << endl;
//            cout << "============================================= " << endl;

            unsigned int padding = 3;	 // pixels

            for (unsigned int b = 0; b < curr_img_bboxes.size(); b++)
            {
               unsigned int px_start = curr_img_bboxes[b].get_xmin() - padding;
               unsigned int px_stop = curr_img_bboxes[b].get_xmax() + padding;
               unsigned int py_start = curr_img_bboxes[b].get_ymin() - padding;
               unsigned int py_stop = curr_img_bboxes[b].get_ymax() + padding;

               unsigned int min_width = 20 + 2 * padding;
               unsigned int min_height = 30 + 2 * padding;
               if(px_stop - px_start < min_width || 
                  py_stop - py_start < min_height) continue;
               if(px_start < 0) continue;
               if(px_stop >= curr_tr.getWidth()) continue;
               if(py_start < 0) continue;
               if(py_stop >= curr_tr.getHeight()) continue;
               
               int subdir_counter = svhn_chip_counter / 1000;
               string output_subdir=svhn_chips_subdir+category
                  +stringfunc::integer_to_string(subdir_counter,5)+"/";
               
               filefunc::dircreate(output_subdir);
               string output_filename=output_subdir+curr_img_labels[b]+"_"+
                  stringfunc::integer_to_string(svhn_chip_counter, 6)+".jpg";

               curr_tr.write_curr_frame(
                  px_start, px_stop, py_start, py_stop, output_filename);
               svhn_chip_counter++;
            } // loop over index b labeling bboxes for current image

            curr_img_bboxes.clear();
            curr_img_labels.clear();
         }
         
         if(key == "width") bbox_width = stringfunc::string_to_number(value);
         if(key == "top") bbox_top = stringfunc::string_to_number(value);
         if(key == "label") bbox_label = value;
         if(key == "left") bbox_left = stringfunc::string_to_number(value);
         if(key == "height")
         {
            bbox_height = stringfunc::string_to_number(value);

            bounding_box curr_bbox(bbox_left, bbox_left + bbox_width,
                                   bbox_top, bbox_top + bbox_height);
            curr_img_bboxes.push_back(curr_bbox);
            if(bbox_label == "10") bbox_label = "0";
            curr_img_labels.push_back(bbox_label);
//            cout << "bbox = " << curr_img_bboxes.back()
//                 << " label = " << curr_img_labels.back() << endl;
         }
      } // loop over index k labeling key-value pairs
   }
   
   delete cppJSON_ptr;
} 


