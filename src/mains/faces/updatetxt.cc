// ====================================================================
// Program UPDATETXT combines together one old and one new labeled
// bboxes text files.  The former is assumed to have contain gender
// attributes assigned via program LABELIMAGES.  The latter is assumed
// to have been generated via a call to XML2TXT and to correspond to
// an enlarged imagery set whose attributes all equal
// "unset". UPDATETXT copies any nontrivial attribute metadata from
// the old labeled bboxes text file into the new one for bboxes whose
// labels and pixel coordinates precisely match.

//                           ./updatetxt

// ====================================================================
// Last updated on 7/21/16; 7/24/16
// ====================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <dlib/image_processing.h>
#include <dlib/data_io.h>

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


typedef map<string, vector<bounding_box> > ANNOTATED_BBOXES_MAP;
// independent string: image_ID_str
// dependent STL vector: annotated bboxes

// ====================================================================
void populate_annotated_bboxes_map(
   string bbox_labels_filename,
   ANNOTATED_BBOXES_MAP& annotated_bboxes_map)
{
   filefunc::ReadInfile(bbox_labels_filename);

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
         curr_bbox.set_label(bbox_label);

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
//         cout << annotated_bboxes.back() << endl;

      } // substrings[0] == "Image:" conditional
   } // loop over index i labeling lines in detections text file

   // Save final image info into data structures
   annotated_bboxes_map[image_ID_str] = annotated_bboxes;
}

// ====================================================================

int main(int argc, char** argv)
{  
   timefunc::initialize_timeofday_clock(); 

//   bool ignore_hands_flag = false;
   bool ignore_hands_flag = true;

   string faces_rootdir = "/data/TrainingImagery/faces/";
   string labeled_faces_subdir = faces_rootdir + "images/";
   string old_bbox_labels_filename = labeled_faces_subdir+
      "labeled_bboxes_522481344.txt";
   string new_bbox_labels_filename = labeled_faces_subdir+
      "initial_labeled_face_072416_bboxes.txt";

   ANNOTATED_BBOXES_MAP old_annotated_bboxes_map, new_annotated_bboxes_map;
   ANNOTATED_BBOXES_MAP::iterator old_annotated_bboxes_iter, 
      new_annotated_bboxes_iter;

// Import metadata for old set of annotated bboxes into
// old_annotated_bboxes_map:

   populate_annotated_bboxes_map(
      old_bbox_labels_filename, old_annotated_bboxes_map);

// Import metadata for new, unset annotated bboxes into
// new_annotated_bboxes_map:

   populate_annotated_bboxes_map(
      new_bbox_labels_filename, new_annotated_bboxes_map);

// Export new attributed bounding boxes to output text file:

   string output_filename=labeled_faces_subdir+"new_labeled_bboxes.txt";
   ofstream outstream;
//   cout << "output_filename = " << output_filename << endl;
   filefunc::openfile(output_filename, outstream);
   outstream << "# " << timefunc::getcurrdate() << endl;
   outstream << "# Image: index  ID_str " << endl;
   outstream << "# Bbox_ID  label  xmin  xmax  ymin ymax (attr_key attr_val)"
             << endl << endl;

   int n_face_bboxes = 0;
   int n_hand_bboxes = 0;
   int image_index = 0;
   int bbox_ID = 0;
   for(new_annotated_bboxes_iter = new_annotated_bboxes_map.begin();
       new_annotated_bboxes_iter != new_annotated_bboxes_map.end();
       new_annotated_bboxes_iter++)
   {
      string image_ID_str = new_annotated_bboxes_iter->first;
      outstream << "Image: index = " << image_index++
                << " ID_str = " << image_ID_str << endl;

// Retrieve attributed bboxes corresponding to image_ID_str from
// old_annotated_bboxes_map:

      vector<bounding_box> old_bboxes;
      old_annotated_bboxes_iter = old_annotated_bboxes_map.find(image_ID_str);
      if (old_annotated_bboxes_iter != old_annotated_bboxes_map.end())
      {
         old_bboxes = old_annotated_bboxes_iter->second;
      }

      bool unset_gender_bbox_flag = false;
      vector<bounding_box> new_bboxes = new_annotated_bboxes_iter->second;
      for(unsigned int b = 0; b < new_bboxes.size(); b++)
      {
         bounding_box new_bbox = new_bboxes[b];
         if(new_bbox.get_label() == "face")
         {
            n_face_bboxes++;
         }
         else if(new_bbox.get_label() == "hand")
         {
            n_hand_bboxes++;
            if (ignore_hands_flag) continue;
         }

         outstream << bbox_ID++ << "  "
                   << new_bbox.get_label() << "   "
                   << new_bbox.get_xmin() << "  "
                   << new_bbox.get_xmax() << "  "
                   << new_bbox.get_ymin() << "  "
                   << new_bbox.get_ymax() << "  ";
         
// Perform brute-force search over old bboxes to find one which has
// precisely the same pixel coordinates as new_bbox.  

         bounding_box curr_bbox = new_bbox;
         for(unsigned int b_old = 0; b_old < old_bboxes.size(); b_old++)
         {
            bounding_box old_bbox = old_bboxes[b_old];
            if(old_bbox.get_label() == new_bbox.get_label() &&
               nearly_equal(old_bbox.get_xmin(), new_bbox.get_xmin()) &&
               nearly_equal(old_bbox.get_xmax(), new_bbox.get_xmax()) &&
               nearly_equal(old_bbox.get_ymin(), new_bbox.get_ymin()) &&
               nearly_equal(old_bbox.get_ymax(), new_bbox.get_ymax()))
            {
               curr_bbox = old_bbox;
               break;
            }
         }


         for(curr_bbox.get_attributes_map_iter() = 
                curr_bbox.get_attributes_map().begin(); 
             curr_bbox.get_attributes_map_iter() != 
                curr_bbox.get_attributes_map().end();
             curr_bbox.get_attributes_map_iter()++)
         {
            string attr_key = curr_bbox.get_attributes_map_iter()->first;
            string attr_value = curr_bbox.get_attributes_map_iter()->second;
            outstream << attr_key << "  "  << attr_value << "  ";
            if(attr_key == "gender" && attr_value == "unset")
            {
               unset_gender_bbox_flag = true;
            }
         }
         outstream << endl;
      } // loop over index b labeling new bboxes

      if(unset_gender_bbox_flag)
      {
         cout << "Unset gender bbox found in frame = " << image_index-1
              << " ID_str = " << image_ID_str << endl;
      }

   } // loop over index i labeling all input images
   
   cout << "n_face_bboxes = " << n_face_bboxes << endl;
   cout << "n_hand_bboxes = " << n_hand_bboxes << endl;
   cout << "Exported bbox metadata to = " << output_filename << endl;
}

