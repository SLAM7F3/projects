// ====================================================================
// Program PARSE_BBOX_LABELS
// ====================================================================
// Last updated on 7/5/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"
#include "time/timefuncs.h"

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
   timefunc::initialize_timeofday_clock(); 

   string faces_rootdir = "/data/TrainingImagery/faces/";
   string labeled_faces_subdir = faces_rootdir + "images/";
   string bbox_labels_filename = labeled_faces_subdir+"labeled_bboxes.txt";
   filefunc::ReadInfile(bbox_labels_filename);

   typedef std::map<string, std::vector<bounding_box> > EXTRACTED_BBOXES_MAP;
// independent string: image_ID_str
// dependent STL vector: extracted bboxes

   EXTRACTED_BBOXES_MAP extracted_bboxes_map;
   EXTRACTED_BBOXES_MAP::iterator bboxes_iter;

   bool first_image_flag = true;
   string image_ID_str = "";
   vector<bounding_box> extracted_bboxes;
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
            extracted_bboxes_map[image_ID_str] = extracted_bboxes;
            extracted_bboxes.clear();
         }
         extracted_bboxes.clear();
         image_ID_str = substrings[6];
      }
      else
      {
         vector<string> substrings = 
            stringfunc::decompose_string_into_substrings(curr_line);
         int bbox_ID = stringfunc::string_to_number(substrings[0]);
         string bbox_label = substrings[1];
         int px_min = stringfunc::string_to_number(substrings[2]);
         int px_max = stringfunc::string_to_number(substrings[3]);
         int py_min = stringfunc::string_to_number(substrings[4]);
         int py_max = stringfunc::string_to_number(substrings[5]);
         bounding_box curr_bbox(px_min, px_max, py_min, py_max);
         curr_bbox.set_ID(bbox_ID);

         colorfunc::Color bbox_color = colorfunc::black;
         if(bbox_label == "face")
         {
            bbox_color = colorfunc::orange;
         }
         else if(bbox_label == "hand")
         {
            bbox_color = colorfunc::yegr;
         }

         curr_bbox.set_label(bbox_label);
         curr_bbox.set_color(bbox_color);

// Search for any bbox attribute key-value pairs:

         int n_attribute_pairs = (substrings.size() - 6)/2;
         for(int ap = 0; ap < n_attribute_pairs; ap++)
         {
            string attr_key = substrings[6 + ap * 2];
            string attr_value = substrings[6 + ap * 2 + 1];
            curr_bbox.get_attributes_map()[attr_key] = attr_value;
         }
         extracted_bboxes.push_back(curr_bbox);

//         cout << extracted_bboxes.back() << endl;

      } // substrings[0] == "Image:" conditional
   } // loop over index i labeling lines in detections text file

   
}

