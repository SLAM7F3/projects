// ========================================================================
// Program ADD_GENDER_ATTRIBUTES imports the text file output by
// program CONSOLIDATE_COMPONENTS which contains bounding boxes for
// detected faces in some set of images.  It also inputs the text file
// containing bbox gender classifications exported by
// CLASSIFY_GENDERS.  ADD_GENDER_ATTRIBUTES appends the gender
// classifications and scores as attributes to bounding boxes and
// outputs the updated bboxes to a new text file.

//			./add_gender_attributes

// ========================================================================
// Last updated on 8/5/16
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
      "Aug3_faces_hands_testing_images_extracted_bboxes.txt";
   string gender_classifications_filename = 
      faces_rootdir +"labeled_data/faces_14/gender.classifications";
   string updated_bbox_labels_filename = faces_rootdir+
      "labeled_data/faces_14/"+
      "updated_Aug3_faces_hands_testing_images_extracted_bboxes.txt";
   filefunc::ReadInfile(bbox_labels_filename);

// First import face bounding boxes from bbox_labels_filename:

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

// Next import gender classifications assigned to each bounding box
// from secondary text file:

   filefunc::ReadInfile(gender_classifications_filename);
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      string file_basename = substrings[1];
      vector<string> subsubstrings = 
         stringfunc::decompose_string_into_substrings(file_basename, "_");
      string image_ID_str = subsubstrings[1];
      int bbox_ID = stringfunc::string_to_number(subsubstrings[2]);

      annotated_bboxes_iter = annotated_bboxes_map.find(image_ID_str);
      vector<bounding_box> *bboxes = &annotated_bboxes_iter->second;
      bounding_box *curr_bbox = &bboxes->at(bbox_ID);

      string curr_gender = substrings[2];
      string curr_score = substrings[3];
      curr_bbox->set_attribute_value("gender", curr_gender);
      curr_bbox->set_attribute_value("score", curr_score);
   } // loop over index i labeling lines in gender classifications file
   
// Export updated bounding boxes to output text file:

   ofstream outstream;
   filefunc::openfile(updated_bbox_labels_filename, outstream);

   outstream << "# " << timefunc::getcurrdate() << endl;
   outstream << "# Image: index  ID_str " << endl;
   outstream << "# Bbox_ID  label  xmin  xmax  ymin ymax (attr_key attr_val)"
             << endl << endl;

   int image_ID = 0;
   for(annotated_bboxes_iter = annotated_bboxes_map.begin();
       annotated_bboxes_iter != annotated_bboxes_map.end();
       annotated_bboxes_iter++)
   {
      outstream << "Image: index = " << image_ID++
                << " ID_str = " << annotated_bboxes_iter->first << endl;
      vector<bounding_box> curr_image_bboxes = annotated_bboxes_iter->second;
      geometry_func::write_bboxes_to_file(outstream, curr_image_bboxes);
      outstream << endl;
   } // loop over annotated_bboxes_iter

   filefunc::closefile(updated_bbox_labels_filename, outstream);

   string banner="Exported labeled bboxes to "+ updated_bbox_labels_filename;
   outputfunc::write_banner(banner);
}

