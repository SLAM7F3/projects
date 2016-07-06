// ==========================================================================
// Program PREC_RECALL imports all testing image masks exported by
// program CONSOLIDATED_PYRAMID_SEGMENTATIONS along with their ground
// truth bboxes.  It generates a new set of predominantly greyscale
// masks containing truth bbox annotations.  PREC_RECALL also computes
// pixel-level precision and bbox recall per truth image.  Recall
// results are exported for tiny, medium, medium and large bboxes
// corresponding to the 1st, 2nd, 3rd and 4th quartiles of bbox pixel
// widths. Precision and recall probability distributions are written
// into the same subdirectory as where the bbox-annotated masks are
// written.
// ==========================================================================
// Last updated on 6/5/16; 6/15/16; 6/21/16; 7/2/16
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

   bool ignore_hands_flag = true;

   string banner="Starting PREC_RECALL program";
   outputfunc::write_big_banner(banner);

   if(argc != 2)
   {
      cout << "Must pass input_images_subdir (e.g. '/home/pcho/programs/c++/svn/projects/src/mains/machine_learning/deeplab/images/faces/testing_images_12/') as command-line argument" << endl;
      exit(-1);
   }
   string input_images_subdir(argv[1]);
   filefunc::add_trailing_dir_slash(input_images_subdir);
   cout << "input_images_subdir = " << input_images_subdir << endl;

//   string consolidated_subdir = input_images_subdir+"consolidated/";
   string consolidated_subdir = input_images_subdir+"ccs/consolidated_images/";
   string validation_xml_filename=input_images_subdir+"validation_images.xml";
   string testing_xml_filename=input_images_subdir+"testing_images.xml";

   bool testing_flag = false;
   bool validation_flag = false;
   if(filefunc::fileexist(validation_xml_filename))
   {
      validation_flag = true;
   }
   else if(filefunc::fileexist(testing_xml_filename))
   {
      testing_flag = true;
   }
   
   if(!testing_flag && !validation_flag)
   {
      cout << "Could not find validation_xml_filename = "
           << validation_xml_filename << endl;
      cout << "nor testing_xml_filename = "
           << testing_xml_filename << endl;
      exit(-1);
   }

   dlib::image_dataset_metadata::dataset input_data;
   if(testing_flag)
   {
      dlib::image_dataset_metadata::load_image_dataset_metadata(
         input_data, testing_xml_filename);
   }
   else if (validation_flag)
   {
      dlib::image_dataset_metadata::load_image_dataset_metadata(
         input_data, validation_xml_filename);

   }
   int n_images = input_data.images.size();
   cout << "Number of images = " << n_images << endl;   

   string bboxes_subdir=consolidated_subdir+"bboxes/";
   filefunc::dircreate(bboxes_subdir);
   string evaluation_subdir=bboxes_subdir+"evaluation/";
   filefunc::dircreate(evaluation_subdir);

   ofstream outstream;
   string output_filename=evaluation_subdir+"performance.dat";
   filefunc::openfile(output_filename, outstream);
   outstream << "Evaluation subdir = " << evaluation_subdir << endl;
   outstream << timefunc::getcurrdate() << endl << endl;

   vector<double> bbox_recalls;
//    vector<double> pixel_widths;
   vector<double> tiny_bbox_recalls, small_bbox_recalls, medium_bbox_recalls,
      large_bbox_recalls;
   vector<double> segmented_pixel_precisions;

   vector<double> decade_widths;
   decade_widths.push_back(0);	 	 // width_0
   decade_widths.push_back(7.74278);	 // width_10
   decade_widths.push_back(12.8915);	 // width_20
   decade_widths.push_back(18.7037);	 // width_30
   decade_widths.push_back(24.9123);	 // width_40
   decade_widths.push_back(31.2091);	 // width_50
   decade_widths.push_back(39.1713);	 // width_60
   decade_widths.push_back(50.8385);	 // width_70
   decade_widths.push_back(67.9516);	 // width_80
   decade_widths.push_back(100.315);	 // width_90
   decade_widths.push_back(10000.0);	 // width_100

   vector< vector<double>* > decade_bbox_recalls;
   for(unsigned int d = 0; d < decade_widths.size(); d++)
   {
      vector<double>* curr_bbox_recalls = new vector<double>;
      decade_bbox_recalls.push_back(curr_bbox_recalls);
   }

   texture_rectangle *tr_ptr = NULL;
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
      string segmented_image_filename=consolidated_subdir+"segmented_image_"+
         image_ID_str+".png";
//      cout << "segmented_image_filename = " << segmented_image_filename 
//           << endl;

      if(!filefunc::fileexist(segmented_image_filename)) continue;
      tr_ptr = new texture_rectangle(segmented_image_filename, NULL);

// Loop over all pixels within segmented texture rectangle.  Count
// total number whose saturation != 0:

      int segmented_pixels_integral = 0;
      for(unsigned int qy = 0; qy < tr_ptr->getHeight(); qy++)
      {
         for(unsigned int qx = 0; qx < tr_ptr->getWidth(); qx++)
         {
            double h, s, v;
            tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
//            if(s > 0)
            if(fabs(h < 5) && s > 0)
            {
//               cout << "qx = " << qx << " qy = " << qy
//                    << " h = " << h << " s = " << s << " v = " << v << endl;
               segmented_pixels_integral++;
            }
         } // loop over qx
      } // loop over qy

      int bbox_segmented_pixels_integral = 0;
      vector<dlib::image_dataset_metadata::box> boxes = 
         input_data.images[i].boxes;
      for(unsigned int b = 0; b < boxes.size(); b++)
      {
         dlib::image_dataset_metadata::box curr_box = boxes[b];
         string curr_box_label = curr_box.label;

// FAKE FAKE:  Hack as of Tues Jun 14

         if(ignore_hands_flag && curr_box_label == "hand") continue;

         dlib::rectangle curr_rect = curr_box.rect;
         int px_min = curr_rect.left();
         int px_max = curr_rect.right();
         int py_min = curr_rect.top();
         int py_max = curr_rect.bottom();
         bounding_box curr_bbox(px_min, px_max, py_min, py_max);
         unsigned int bbox_width = px_max - px_min;
         unsigned int bbox_height = abs(py_max - py_min);
         int n_bbox_pixels = bbox_width * bbox_height;

// Expt as of Jul 2, 2016 with subtracting off hand pixels that
// overlap current face bbox BEFORE computing recall:

         if(curr_box_label == "face")
         {
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
                  int intersection_bbox_pixels = 
                     intersection_bbox.get_xextent() * 
                     intersection_bbox.get_yextent();
                 // n_bbox_pixels -= intersection_bbox_pixels;
               }
            } // loop over index b2 labeling hand bboxes in current image
	    if(bbox_width * bbox_height - n_bbox_pixels > 0){
            cout << "Orig bbox_width * bbox_height = "
		 << bbox_width * bbox_height << endl;
	    cout << "n_bbox_pixels = " << n_bbox_pixels << endl;
		}

         } // curr_bbox_label == face conditional

// Recall to good approximation, face bbox aspect ratio = 0.731
// --> w = sqrt(bbox_aspect_ratio * n_bbox_pixels);

       //  const double bbox_aspect_ratio = 0.731; // Updated on 6/5/16
       //  double pixel_width = sqrt(bbox_aspect_ratio * n_bbox_pixels);
         double pixel_width = bbox_width;
//         pixel_widths.push_back(pixel_width);

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

         for(int d = 0; d < 10; d++)
         {
            if(pixel_width >= decade_widths[d] && 
               pixel_width < decade_widths[d+1])
            {
//               cout << "d = " << d << " pixel_width = " << pixel_width << endl;
               decade_bbox_recalls[d]->push_back(curr_bbox_recall);
               break;
            }
         } // loop over index d labeling decades
       
         if(pixel_width <= 15)  // 0 - 15 pixels = 1st quartile
         {
            tiny_bbox_recalls.push_back(curr_bbox_recall);
         }
         else if (pixel_width >=  16 && pixel_width <= 30)
            // 16 - 30 pixels = 2nd quartile
         {
            small_bbox_recalls.push_back(curr_bbox_recall);
         }
         else if (pixel_width >= 31 && pixel_width <= 57)
            // 31 - 57 pixels = 3rd quartile
         {
            medium_bbox_recalls.push_back(curr_bbox_recall);
         }
         else
         {
            large_bbox_recalls.push_back(curr_bbox_recall);
         }
         bbox_recalls.push_back(curr_bbox_recall);
//         cout << "curr_bbox_recall = " << curr_bbox_recall << endl;
         bbox_segmented_pixels_integral += n_bbox_segmented_pixels;

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
      } // loop over index b labeling bboxes for current image

      double curr_segmented_pixel_prec = 1;
      if(segmented_pixels_integral > 0)
      {
         curr_segmented_pixel_prec = 
            double(bbox_segmented_pixels_integral) / 
            segmented_pixels_integral;
      }
//      cout << "bbox_segmented_pixels_integral = " 
//           << bbox_segmented_pixels_integral
//           << " segmented_pixels_integral = "
//           << segmented_pixels_integral << endl;
//      cout << "curr_segmented_pixel_prec = "
//           << curr_segmented_pixel_prec << endl;
      segmented_pixel_precisions.push_back(curr_segmented_pixel_prec);

      string output_filename=bboxes_subdir+
         "bboxes_"+stringfunc::integer_to_string(image_ID, 5)+".png";
      tr_ptr->write_curr_frame(output_filename);
      delete tr_ptr;
      tr_ptr = NULL;
   } // loop over index i labeling images

   for(int d = 0; d < 10; d++)
   {
      prob_distribution prob_recall(50, 0, 1, *(decade_bbox_recalls[d]));
      prob_recall.set_densityfilenamestr(
         evaluation_subdir+"recall_"+stringfunc::integer_to_string(d,2)
         +"_dens.meta");
      prob_recall.set_cumulativefilenamestr(
         evaluation_subdir+"recall_"+stringfunc::integer_to_string(d,2)
         +"_cum.meta");
      prob_recall.set_freq_histogram(true);
      string title = stringfunc::number_to_string(
         decade_bbox_recalls[d]->size())+" bounding boxes";
      prob_recall.set_title(title);
      prob_recall.set_xlabel("Bounding box recall");
      prob_recall.set_xtic(0.2);
      prob_recall.set_xsubtic(0.1);
      prob_recall.writeprobdists(false,true);
      
      outstream << "Decade " << d 
                << " Bbox width: min=" << int(decade_widths[d])
                << " max=" << int(decade_widths[d+1])
                << "\t Recall=" << prob_recall.median()
                << "\t " << decade_bbox_recalls[d]->size() << " bboxes"
                << endl;
   } // loop over index d labeling bbox decades

   prob_distribution tiny_prob_recall(50, 0, 1, tiny_bbox_recalls);
   tiny_prob_recall.set_densityfilenamestr(
      evaluation_subdir+"tiny_recall_dens.meta");
   tiny_prob_recall.set_cumulativefilenamestr(
      evaluation_subdir+"tiny_recall_cum.meta");
   tiny_prob_recall.set_freq_histogram(true);
   string title = stringfunc::number_to_string(tiny_bbox_recalls.size())+
      " tiny bounding boxes";
   tiny_prob_recall.set_title(title);
   tiny_prob_recall.set_xlabel("Tiny bbox recall");
   tiny_prob_recall.set_xtic(0.2);
   tiny_prob_recall.set_xsubtic(0.1);
   tiny_prob_recall.writeprobdists(false,true);

   outstream << "Tiny bbox recall = " << tiny_prob_recall.median()
             << " \t\t " << tiny_bbox_recalls.size() << " tiny bboxes"
             << endl;

   prob_distribution small_prob_recall(50, 0, 1, small_bbox_recalls);
   small_prob_recall.set_densityfilenamestr(
      evaluation_subdir+"small_recall_dens.meta");
   small_prob_recall.set_cumulativefilenamestr(
      evaluation_subdir+"small_recall_cum.meta");
   small_prob_recall.set_freq_histogram(true);
   title = stringfunc::number_to_string(small_bbox_recalls.size())+
      " small bounding boxes";
   small_prob_recall.set_title(title);
   small_prob_recall.set_xlabel("Small bbox recall");
   small_prob_recall.set_xtic(0.2);
   small_prob_recall.set_xsubtic(0.1);
   small_prob_recall.writeprobdists(false,true);

   outstream << "Small bbox recall = " << small_prob_recall.median()
             << " \t\t " << small_bbox_recalls.size() << " small bboxes"
             << endl;

   prob_distribution medium_prob_recall(50, 0, 1, medium_bbox_recalls);
   medium_prob_recall.set_densityfilenamestr(
      evaluation_subdir+"medium_recall_dens.meta");
   medium_prob_recall.set_cumulativefilenamestr(
      evaluation_subdir+"medium_recall_cum.meta");
   medium_prob_recall.set_freq_histogram(true);
   title = stringfunc::number_to_string(medium_bbox_recalls.size())+
      " medium bounding boxes";
   medium_prob_recall.set_title(title);
   medium_prob_recall.set_xlabel("Medium bbox recall");
   medium_prob_recall.set_xtic(0.2);
   medium_prob_recall.set_xsubtic(0.1);
   medium_prob_recall.writeprobdists(false,true);

   outstream << "Medium bbox recall = " << medium_prob_recall.median()
             << " \t\t " << medium_bbox_recalls.size() << " medium bboxes"
             << endl;

   prob_distribution large_prob_recall(50, 0, 1, large_bbox_recalls);
   large_prob_recall.set_densityfilenamestr(
      evaluation_subdir+"large_recall_dens.meta");
   large_prob_recall.set_cumulativefilenamestr(
      evaluation_subdir+"large_recall_cum.meta");
   large_prob_recall.set_freq_histogram(true);
   title = stringfunc::number_to_string(large_bbox_recalls.size())+
      " large bounding boxes";
   large_prob_recall.set_title(title);
   large_prob_recall.set_xlabel("Large bbox recall");
   large_prob_recall.set_xtic(0.2);
   large_prob_recall.set_xsubtic(0.1);
   large_prob_recall.writeprobdists(false,true);

   outstream << "Large bbox recall = " << large_prob_recall.median()
             << " \t\t " << large_bbox_recalls.size() << " large bboxes"
             << endl;

   prob_distribution total_prob_recall(50, 0, 1, bbox_recalls);
   total_prob_recall.set_densityfilenamestr(
      evaluation_subdir+"total_recall_dens.meta");
   total_prob_recall.set_cumulativefilenamestr(
      evaluation_subdir+"total_recall_cum.meta");
   total_prob_recall.set_freq_histogram(true);
   title = stringfunc::number_to_string(bbox_recalls.size())+
      " total bounding boxes";
   total_prob_recall.set_title(title);
   total_prob_recall.set_xlabel("Total bbox recall");
   total_prob_recall.set_xtic(0.2);
   total_prob_recall.set_xsubtic(0.1);
   total_prob_recall.writeprobdists(false,true);

   double median_recall = total_prob_recall.median();
   outstream << "Total bbox recall = " << median_recall
             << " \t\t " << bbox_recalls.size() << " total bboxes"
             << endl;

   prob_distribution prob_precision(50, 0, 1, segmented_pixel_precisions);
   prob_precision.set_densityfilenamestr(
      evaluation_subdir+"precision_dens.meta");
   prob_precision.set_cumulativefilenamestr(
      evaluation_subdir+"precision_cum.meta");
   prob_precision.set_freq_histogram(true);
   prob_precision.set_xlabel("Pixel precision");
   prob_precision.set_xtic(0.2);
   prob_precision.set_xsubtic(0.1);
   prob_precision.writeprobdists(false,true);

   double median_precision = prob_precision.median();
   outstream << "Pixel precision = " << median_precision << endl;

   double f_value = 2 * (median_precision * median_recall) / 
      (median_precision + median_recall);
   outstream << "f_value = 2 * prec * recall / (prec + recall) = "
             << f_value << endl;
   filefunc::closefile(output_filename, outstream);

   banner="Exported recall and precision distributions to "+evaluation_subdir;
   outputfunc::write_banner(banner);
   banner="Exported recall and precision performance to "+output_filename;
   outputfunc::write_banner(banner);

   string unix_cmd = "cat "+output_filename;
   sysfunc::unix_command(unix_cmd);
}
