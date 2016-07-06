// ==========================================================================
// Program BBOX_PREC_RECALL imports all ground-truth and detected
// face bounding boxes for a testing image set.  For each ground-truth
// bbox within a test image, we search for a detection bbox
// counterpart with maximal intersection-over-union value.  A
// frequency histogram for ground-truth face bbox IoU values is
// exported. Similarly, for each detected face bbox within a test
// image, we search for a ground-truth bbox counterpart with maximal
// IoU. Another frequency histogram fro detected face bbox IoU values
// is exported.
// ==========================================================================
// Last updated on 6/26/16; 7/3/16
// ==========================================================================

#include <iostream>
#include <string>
#include <map>
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
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

// Import and parse ground truth testing images:

   string xml_filename="testing_images_faces_hands.xml";
   dlib::image_dataset_metadata::dataset testing_data;
   dlib::image_dataset_metadata::load_image_dataset_metadata(
      testing_data, xml_filename);
   int n_testing_images = testing_data.images.size();
   cout << "Number of ground truth testing images = " << n_testing_images 
        << endl;   

// Import and parse detected bboxes:

   if(argc != 2)
   {
      cout << "Must pass full path for text file containing extracted bboxes"
           << endl;
      exit(-1);
   }
   string extracted_bboxes_filename(argv[1]);
   filefunc::ReadInfile(extracted_bboxes_filename);

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
         vector<double> values = stringfunc::string_to_numbers(curr_line);
//          int CC_ID = values[0];
         int CC_class = values[1];
         int px_min = values[2];
         int px_max = values[3];
         int py_min = values[4];
         int py_max = values[5];
//         cout << CC_ID << "  "
//              << CC_class << "  "
//              << px_min << "  "
//              << px_max << "  "
//              << py_min << "  "
//              << py_max << endl;
         bounding_box curr_bbox(px_min, px_max, py_min, py_max);

         string label;
         colorfunc::Color bbox_color = colorfunc::black;
         if(CC_class == 1)
         {
            label = "face";
            bbox_color = colorfunc::orange;
         }
         else if(CC_class == 2)
         {
            label = "hand";
            bbox_color = colorfunc::yegr;
         }
         curr_bbox.set_label(label);
         curr_bbox.set_color(bbox_color);
         extracted_bboxes.push_back(curr_bbox);

      } // substrings[0] == "Image:" conditional
   } // loop over index i labeling lines in detections text file

   int n_gt_face_bboxes = 0;
   int n_detected_face_bboxes = 0;
   vector<double> gt_bbox_IoUs, detected_bbox_IoUs;
   for(int i = 0; i < n_testing_images; i++)
   {
      string image_filename=testing_data.images[i].filename;
      string image_basename=filefunc::getbasename(image_filename);
      string image_ID_str = image_basename.substr(6,5);

// Recover ground-truth face bounding boxes:

      vector<bounding_box> ground_truth_face_bboxes;
      vector<dlib::image_dataset_metadata::box> boxes = 
         testing_data.images[i].boxes;
      for(unsigned int b = 0; b < boxes.size(); b++)
      {
         dlib::image_dataset_metadata::box curr_box = boxes[b];
         string curr_box_label = curr_box.label;

         if(curr_box_label == "hand") continue;

         dlib::rectangle curr_rect = curr_box.rect;
         int px_min = curr_rect.left();
         int px_max = curr_rect.right();
         int py_min = curr_rect.top();
         int py_max = curr_rect.bottom();
         bounding_box ground_truth_face_bbox(px_min, px_max, py_min, py_max);
         ground_truth_face_bboxes.push_back(ground_truth_face_bbox);
         n_gt_face_bboxes++;
      } // loop over index b labeling ground truth bboxes for curr image

// Recover detected face bounding boxes:

      vector<bounding_box> extracted_face_bboxes;
      bboxes_iter = extracted_bboxes_map.find(image_ID_str);
      if(bboxes_iter != extracted_bboxes_map.end())
      {
         vector<bounding_box> extracted_bboxes = bboxes_iter->second;
         for(unsigned int b = 0 ; b < extracted_bboxes.size(); b++)
         {
            bounding_box curr_bbox(extracted_bboxes[b]);
            if(curr_bbox.get_label() == "face")
            {
               extracted_face_bboxes.push_back(curr_bbox);
               n_detected_face_bboxes++;
            }
         }
      }

// Loop over each ground truth face bbox.  Search for detected
// face bbox counterpart with maximal IoU:

      double max_IoU = 0;
      for(unsigned int b = 0; b < ground_truth_face_bboxes.size(); b++)
      {
         bounding_box gt_bbox(ground_truth_face_bboxes[b]);
         for(unsigned int b2 = 0; b2 < extracted_face_bboxes.size(); b2++)
         {
            bounding_box detected_bbox(extracted_face_bboxes[b2]);
            double IoU = gt_bbox.intersection_over_union(detected_bbox);
            max_IoU = basic_math::max(max_IoU, IoU);
         } // loop over index b2 labeling extracted face bboxes
         gt_bbox_IoUs.push_back(max_IoU);
      } // loop over index b labeling ground truth face bboxes

// Loop over each detected face bbox.  Search for some ground truth
// face bbox counterpart:

      max_IoU = 0;
      for(unsigned int b = 0; b < extracted_face_bboxes.size(); b++)
      {
         bounding_box detected_bbox(extracted_face_bboxes[b]);
         for(unsigned int b2 = 0; b2 < ground_truth_face_bboxes.size(); b2++)
         {
            bounding_box gt_bbox(ground_truth_face_bboxes[b2]);
            double IoU = detected_bbox.intersection_over_union(gt_bbox);
            max_IoU = basic_math::max(max_IoU, IoU);
         } // loop over index b2 labeling extracted face bboxes
         detected_bbox_IoUs.push_back(max_IoU);
      } // loop over index b labeling ground truth face bboxes
   } // loop over i labeling testing images

   cout << "n_gt_face_bboxes = " << n_gt_face_bboxes << endl;
   cout << "n_detected_face_bboxes = " << n_detected_face_bboxes << endl;

   prob_distribution prob_gt_IoU(50, 0, 1, gt_bbox_IoUs);
   prob_gt_IoU.set_densityfilenamestr("gt_IoU_dens.meta");
   prob_gt_IoU.set_cumulativefilenamestr("gt_IoU_cum.meta");
   prob_gt_IoU.set_freq_histogram(true);
   string title = "";
   prob_gt_IoU.set_title(title);
   prob_gt_IoU.set_xlabel("Ground truth face bounding box IoU");
   prob_gt_IoU.set_xtic(0.2);
   prob_gt_IoU.set_xsubtic(0.1);
   prob_gt_IoU.writeprobdists(false,true);
   cout << "gt_bbox_IoUs.size() = " << gt_bbox_IoUs.size() << endl;
   cout << "Median ground truth IoU = " << prob_gt_IoU.median()
        << endl;
   cout << "Median value for maximal ground truth face bbox IoU = " 
        << mathfunc::median_value(gt_bbox_IoUs) << endl;

   prob_distribution prob_detected_IoU(50, 0, 1, detected_bbox_IoUs);
   prob_detected_IoU.set_densityfilenamestr("detected_IoU_dens.meta");
   prob_detected_IoU.set_cumulativefilenamestr("detected_IoU_cum.meta");
   prob_detected_IoU.set_freq_histogram(true);
   title = "";
   prob_detected_IoU.set_title(title);
   prob_detected_IoU.set_xlabel("Detected face bounding box IoU");
   prob_detected_IoU.set_xtic(0.2);
   prob_detected_IoU.set_xsubtic(0.1);
   prob_detected_IoU.writeprobdists(false,true);
   cout << "detected_bbox_IoUs.size() = " << detected_bbox_IoUs.size() << endl;
   cout << "Median detected bbox IoU = " << prob_detected_IoU.median()
        << endl;
   cout << "Median value for maximal detected face bbox IoU = " 
        << mathfunc::median_value(detected_bbox_IoUs) << endl;
}
