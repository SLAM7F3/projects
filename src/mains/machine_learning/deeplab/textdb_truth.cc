// ==========================================================================
// Program TEXTDB_TRUTH parses the text file accompanying O(300)
// images.  It displays the manually-extracted axis-aligned bboxes for
// text labels within the images.  TEXTDB_TRUTH also retrieves the
// semantically segmented images and searches for colored pixels.  It
// forms precision and recall metrics which quantify gross text
// localization performance.
// ==========================================================================
// Last updated on 4/15/16; 4/22/16; 4/24/16; 4/25/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

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

   string text_database_subdir="./images/wildtext/text_database/";
   string halfsized_text_database_subdir=text_database_subdir+"halfsized/";
   string doublesized_text_database_subdir=text_database_subdir+"doublesized/";
   string recombined_basedir;
   cout << "Enter recombined segmentations subdir relative to "
        << text_database_subdir << endl;
   cin >> recombined_basedir;
   filefunc::add_trailing_dir_slash(recombined_basedir);

   string recombined_subdir=text_database_subdir+recombined_basedir;
   string halfsized_recombined_subdir=halfsized_text_database_subdir+
      recombined_basedir;
   string doublesized_recombined_subdir=doublesized_text_database_subdir+
      recombined_basedir;

   string output_subdir=text_database_subdir+"bboxes/";
   filefunc::dircreate(output_subdir);
   string bboxes_filename=text_database_subdir + "bboxes.txt";
   filefunc::ReadInfile(bboxes_filename);

   int item_ID = -1;
   int bbox_pixels_integral = 0;
   int segmented_pixels_integral = 0;
   int bbox_segmented_pixels_integral = 0;
   int bbox_unsegmented_pixels_integral = 0;

   string image_filename;
   texture_rectangle *tr_ptr = NULL;
   texture_rectangle *halfsized_tr_ptr = NULL;
   texture_rectangle *doublesized_tr_ptr = NULL;

   vector<string> text_labels;
   vector<bounding_box> bboxes;
   vector<double> IoUs, bbox_recalls;
   vector<double> segmented_pixel_precisions;

   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      outputfunc::update_progress_and_remaining_time(
         i, 1000, filefunc::text_line.size());
      string curr_line = filefunc::text_line[i];
      int curr_line_size = curr_line.size();
//      int px=-1, py=-1, width=-1, height=-1;
      int px, py, width, height;
      bool text_line_flag = false;

      if(curr_line_size > 5)
      {
         if(curr_line.substr(0,5) == "TEXT:")
         {
            text_line_flag = true;
         }
      }
      
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         curr_line);
      if(substrings[0] == "ITEM:")
      {
         if(item_ID >= 0)
         {
            // Save bboxes for current image

            cout << "item_ID = " << item_ID << endl;
//            cout << "image = " << image_filename << endl;
            colorfunc::Color bbox_color = colorfunc::yellow;
            for(unsigned int b = 0; b < bboxes.size(); b++)
            {
//               cout << "b = " << b << " bbox = " << bboxes[b] << endl;
               tr_ptr->draw_pixel_bbox(bboxes[b], bbox_color);
               bounding_box offset_bbox(
                  bboxes[b].get_xmin()+1,bboxes[b].get_xmax()+1,
                  bboxes[b].get_ymin()+1,bboxes[b].get_ymax()+1);
               tr_ptr->draw_pixel_bbox(offset_bbox, bbox_color);
               offset_bbox = bounding_box(
                  bboxes[b].get_xmin()-1,bboxes[b].get_xmax()-1,
                  bboxes[b].get_ymin()-1,bboxes[b].get_ymax()-1);
               tr_ptr->draw_pixel_bbox(offset_bbox, bbox_color);

            }
            string output_filename=output_subdir+
               "bboxes_"+stringfunc::integer_to_string(
                  item_ID, 4)+".png";
            tr_ptr->write_curr_frame(output_filename);
            delete tr_ptr;
            tr_ptr = NULL;

//            cout << "bbox_pixels_integral = " << bbox_pixels_integral
//                 << endl;
//            cout << "bbox_segmented_pixels_integral = " 
//                 << bbox_segmented_pixels_integral << endl;
//            cout << "segmented_pixels_integral = " << segmented_pixels_integr//al
//                 << endl;

            double curr_segmented_pixel_prec = 
               double(bbox_segmented_pixels_integral) / 
               segmented_pixels_integral;
            segmented_pixel_precisions.push_back(curr_segmented_pixel_prec);

            cout << "Segmented pixels precision = " 
                 << curr_segmented_pixel_prec << endl;

/*
  double pixels_intersection = bbox_segmented_pixels_integral;
  double pixels_union = bbox_pixels_integral + 
  segmented_pixels_integral - pixels_intersection;
  cout << "pixels_intersection = " << pixels_intersection
  << " pixels_union = " << pixels_union << endl;
  double IoU = double(pixels_intersection)/pixels_union;
  IoUs.push_back(IoU);
  cout << "IoU = " << IoU << endl;
*/
//             outputfunc::enter_continue_char();
         }

         bbox_pixels_integral = 0;
         segmented_pixels_integral = 0;
         bbox_segmented_pixels_integral = 0;
         bbox_unsegmented_pixels_integral = 0;
         bboxes.clear();
         text_labels.clear();

         item_ID = stringfunc::string_to_number(substrings[1]);

         cout << "==================================================" << endl;
         cout << "item_ID = " << item_ID << endl;
         string png_basename="text_img"+stringfunc::integer_to_string(
            item_ID,4)+".png";
//         cout << "png_basename = " << png_basename << endl;
         image_filename=text_database_subdir+png_basename;
         string segmented_filename=recombined_subdir+"segmented_image_"+
            stringfunc::integer_to_string(item_ID,5)+".png";
         string halfsized_segmented_filename=halfsized_recombined_subdir
            +"segmented_image_"+
            stringfunc::integer_to_string(item_ID,5)+".png";
         string doublesized_segmented_filename=doublesized_recombined_subdir
            +"segmented_image_"+
            stringfunc::integer_to_string(item_ID,5)+".png";

         tr_ptr = new texture_rectangle(segmented_filename, NULL);

//          double blue_cyan_h = 210;
         double purple_h = 300;

// Transfer text pixel labels from half-sized segmented images to
// full-sized segmented image:

         if(filefunc::fileexist(halfsized_segmented_filename))
         {
            delete halfsized_tr_ptr;
            halfsized_tr_ptr = new texture_rectangle(
               halfsized_segmented_filename, NULL);

            for(unsigned int qy = 0; qy < halfsized_tr_ptr->getHeight(); qy++)
            {
               for(unsigned int qx = 0; qx < halfsized_tr_ptr->getWidth(); 
                   qx++)
               {
                  double h, s, v;
                  halfsized_tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);

                  if(h > 0 && s > 0)
                  {
//                  cout << "qx = " << qx << " qy = " << qy
//                       << " h = " << h << " s = " << s << " v = " << v << endl;
                     double halfsized_h = purple_h;
                     double smin = 0.66;
                     double smax = 1.0;
                     double vmin = 0.5;
                     double vmax = 1.0;
                  
                     tr_ptr->get_pixel_hsv_values(2*qx, 2*qy, h, s, v);
                     s = smin + s * (smax - smin);
                     v = vmin + v * (vmax - vmin);
                     tr_ptr->set_pixel_hsv_values(2*qx,2*qy,halfsized_h, s, v);
                     
                     tr_ptr->get_pixel_hsv_values(2*qx+1, 2*qy, h, s, v);
                     s = smin + s * (smax - smin);
                     v = vmin + v * (vmax - vmin);
                     tr_ptr->set_pixel_hsv_values(2*qx+1,2*qy,halfsized_h,s,v);

                     tr_ptr->get_pixel_hsv_values(2*qx, 2*qy+1, h, s, v);
                     s = smin + s * (smax - smin);
                     v = vmin + v * (vmax - vmin);
                     tr_ptr->set_pixel_hsv_values(2*qx,2*qy+1,halfsized_h,s,v);

                     tr_ptr->get_pixel_hsv_values(2*qx+1, 2*qy+1, h, s, v);
                     s = smin + s * (smax - smin);
                     v = vmin + v * (vmax - vmin);
                     tr_ptr->set_pixel_hsv_values(2*qx+1,2*qy+1,halfsized_h,s,v);
                  }
               } // loop over qx
            } // loop over qy
         } // half_sized_segmented_filename exists conditional

         if(filefunc::fileexist(doublesized_segmented_filename))
         {
            delete doublesized_tr_ptr;
            doublesized_tr_ptr = new texture_rectangle(
               doublesized_segmented_filename, NULL);

            for(unsigned int qy = 0; qy < doublesized_tr_ptr->getHeight(); 
                qy++)
            {
               for(unsigned int qx = 0; qx < doublesized_tr_ptr->getWidth(); 
                   qx++)
               {
                  double h, s, v;
                  doublesized_tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
                  if(h > 0 && s > 0)
                  {
//                  cout << "qx = " << qx << " qy = " << qy
//                       << " h = " << h << " s = " << s << " v = " << v << endl;
                     double doublesized_h = purple_h;
                     double smin = 0.66;
                     double smax = 1.0;
                     double vmin = 0.5;
                     double vmax = 1.0;

                     tr_ptr->get_pixel_hsv_values(qx/2, qy/2, h, s, v);
                     s = smin + s * (smax - smin);
                     v = vmin + v * (vmax - vmin);

// Don't color pixel again if it's already colored

                     if(fabs(h-purple_h) > 20){
                       tr_ptr->set_pixel_hsv_values(qx/2,qy/2,doublesized_h,s,v);
                     }
                     

                  }
               } // loop over qx
            } // loop over qy
         } // doublesized_segmented_filename exists conditional


// Loop over all pixels within segmented texture rectangle.  Count
// total number whose saturation != 0:

         for(unsigned int qy = 0; qy < tr_ptr->getHeight(); qy++)
         {
            for(unsigned int qx = 0; qx < tr_ptr->getWidth(); qx++)
            {
               double h, s, v;
               tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
               if(h > 0 && s > 0)
               {
//                  cout << "qx = " << qx << " qy = " << qy
//                       << " h = " << h << " s = " << s << " v = " << v << endl;
                  segmented_pixels_integral++;

                  double fullsized_h = purple_h;
                  tr_ptr->set_pixel_hsv_values(qx,qy,fullsized_h,s,v);

               }
            } // loop over qx
         } // loop over qy



      }

      else if (substrings[0] == "RECT:")
      {
         px = stringfunc::string_to_number(substrings[2]);
         py = stringfunc::string_to_number(substrings[4]);
         width = stringfunc::string_to_number(substrings[6]);
         height = stringfunc::string_to_number(substrings[8]);
//         cout << "px = " << px << " py = " << py
//              << " width = " << width << " height = " << height << endl;
         int n_bbox_pixels = width * height;
         bbox_pixels_integral += n_bbox_pixels;

// Count number of segmented pixels located inside current bbox:

         int n_bbox_segmented_pixels = 0;
         for(int qy = py; qy < py+height; qy++)
         {
            for(int qx = px; qx < px+width; qx++)
            {
               double h, s, v;
               tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
               if(h > 0 && s > 0)
               {
                  n_bbox_segmented_pixels++;
               }
            }
         }
         int n_bbox_unsegmented_pixels = n_bbox_pixels - 
            n_bbox_segmented_pixels;

         double curr_bbox_recall = double(n_bbox_segmented_pixels)/
            n_bbox_pixels;
         cout << "n_bbox_pixels = " << n_bbox_pixels
              << " n_bbox_segmented_pixels = " << n_bbox_segmented_pixels
              << " bbox_recall = " << curr_bbox_recall 
              << endl;

         bbox_recalls.push_back(curr_bbox_recall);

         bbox_segmented_pixels_integral += n_bbox_segmented_pixels;
         bbox_unsegmented_pixels_integral += n_bbox_unsegmented_pixels;

      }
      else if (text_line_flag)
      {
         string curr_text = filefunc::text_line[i].substr(
            5,curr_line_size - 5);
         text_labels.push_back(curr_text);
         cout << "text label = " << curr_text << endl;
         bounding_box curr_bbox(px, px+width, py, py+height);
         curr_bbox.set_label(curr_text);
         bboxes.push_back(curr_bbox);
      }
   } // loop over index i labeling input file lines

   
   prob_distribution prob_recall(50, 0, 1, bbox_recalls);
   prob_recall.set_densityfilenamestr("./results/recall_dens.meta");
   prob_recall.set_cumulativefilenamestr("./results/recall_cum.meta");
   prob_recall.set_freq_histogram(true);
   prob_recall.set_xlabel("Bbox recall");
   prob_recall.set_xtic(0.2);
   prob_recall.set_xsubtic(0.1);
   prob_recall.writeprobdists(false,true);

   prob_distribution prob_precision(50, 0, 1, segmented_pixel_precisions);
   prob_precision.set_densityfilenamestr("./results/precision_dens.meta");   
   prob_precision.set_cumulativefilenamestr("./results/precision_cum.meta");
   prob_precision.set_freq_histogram(true);
   prob_precision.set_xlabel("Pixel precision");
   prob_precision.set_xtic(0.2);
   prob_precision.set_xsubtic(0.1);
   prob_precision.writeprobdists(false,true);

   string banner="Exported recall and precision distributions to ./results/";
   outputfunc::write_banner(banner);

   delete tr_ptr;
}

