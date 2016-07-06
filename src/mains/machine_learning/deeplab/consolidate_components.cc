// ==========================================================================
// Program CONSOLIDATE_COMPONENTS imports segmentation masks along
// with score images generated at half-size, full-size and double-size
// scales.  At each image size scale, connected components are formed
// from semantic segmentation pixels corresponding to distinct object
// classes.  Each CC is assigned a "hue" score which corresponds to
// the "hottest" 20th-percentile among its overlapped score pixels.
// CCs which are either too large or too small (for a particular image
// scale) are ignored.  CCs which have hue scores that are too low are
// also ignored.  (Score thresholds are more stringent for hands than
// for faces, and these thresholds are most stringent for double-sized
// images and least stringent for half-sized images.)

// Pixel connected components are next "flattened" across image
// pyramid scales.  If two CCs at different scales overlap in area by
// more than 25%, the CC with the better score relative to its
// threshold is retained while the other is rejected.

// Among all non-rejected pixel CCs, we search for face centers within
// quad segmentation masks.  A small bbox is dragged across each pixel
// CC bbox.  At each candidate face center location, we count the
// numbers of green [blue] (red) {yellow} pixels in the upper left
// [upper right] (lower left) {lower right) of the small bbox.  The
// location of the small bbox is not allowed to overlap any
// previously-found small bbox location.  Moreover, ratios of quadrant
// pixel contents are required to be reasonably close to unity for
// 2nd-best, 3rd-best small bboxes.  All pixels inside each
// non-rejected pixel CC is reassigned to the closest small bbox
// center (in a similar fashion to K-means clustering).  

// Once the final set of pixel CCs are set, we compute probability
// densities for their horizontal and vertical pixel coordinates.  CC
// bbox borders are set equal to 5% and 95% cumulative value within
// these probability distributions.  Each CC's pixels are tinted
// according to object class, and similarly-colored bounding boxes are
// drawn around each pixel CC.

// ==========================================================================
// Last updated on 6/25/16; 6/27/16; 6/28/16; 7/5/16
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "video/pixel_cc.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
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
// As of 6/22/16, we impose a strong score threshold
// for hands and a more lenient threshold for faces.  We also impose
// stronger scores thresholds as image size decreases:

double get_score_threshold(int imagesize, int class_ID)
{
   int score_threshold = -1;
   if(class_ID == 1)           // face
   {
      if(imagesize == 0)
      {
         score_threshold = 180;   // cyan
      }
      else if (imagesize == 1)
      {
         score_threshold = 160;
//         score_threshold = 150;
      }
      else if (imagesize == 2)
      {
         score_threshold = 130;   
//         score_threshold = 100;   
      }
   }
   else if (class_ID == 2)     // hand
   {
      if(imagesize == 0)
      {
         score_threshold = 125;   // green
      }
      else if(imagesize == 1)
      {
         score_threshold = 115;   
//         score_threshold = 110;   
      }
      else if(imagesize == 2)
      {
         score_threshold = 100;   
//         score_threshold = 90;   
      }
   }
   return score_threshold;
}

// ---------------------------------------------------------------------
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   timefunc::initialize_timeofday_clock();

   string banner="Starting CONNECT_COMPONENTS program";
   outputfunc::write_big_banner(banner);

   int n_classes = 2; // faces, hands

   if(argc < 2)
   {
      cout << "Must pass input_images_subdir (e.g. '/home/pcho/programs/c++/svn/projects/src/mains/machine_learning/deeplab/images/faces/testing_images_05/') as command-line argument" << endl;
      exit(-1);
   }
   string input_images_subdir(argv[1]);
   filefunc::add_trailing_dir_slash(input_images_subdir);
//    cout << "input_images_subdir = " << input_images_subdir << endl;

   string halfsized_subdir = input_images_subdir+"halfsized/";
   string fullsized_subdir = input_images_subdir+"fullsized/";
   string doublesized_subdir = input_images_subdir+"doublesized/";

   vector<string> fullsized_image_filenames=
      filefunc::image_files_in_subdir(fullsized_subdir);
   int n_images = fullsized_image_filenames.size();
   cout << "n_images = " << n_images << endl;

   vector<string> image_ID_strings;
   for(int i = 0; i < n_images; i++)
   {
      string basename=filefunc::getbasename(fullsized_image_filenames[i]);
      string image_ID_str = basename.substr(6,5);
      image_ID_strings.push_back(image_ID_str);
   }

   string connected_components_subdir = input_images_subdir+"ccs/";
   filefunc::dircreate(connected_components_subdir);
   string recombined_basedir="recombined_segmentations";
   filefunc::add_trailing_dir_slash(recombined_basedir);
   string consolidated_images_subdir=connected_components_subdir+
      "consolidated_images/";
   filefunc::dircreate(consolidated_images_subdir);

   string ccs_images_subdir = connected_components_subdir+"ccs_images/";
   filefunc::dircreate(ccs_images_subdir);
   string score_images_subdir=connected_components_subdir+"score_images/";
   filefunc::dircreate(score_images_subdir);
   string quad_images_subdir=connected_components_subdir+"quad_images/";
   filefunc::dircreate(quad_images_subdir);

   vector<string> sized_recombined_subdirs;
   sized_recombined_subdirs.push_back(doublesized_subdir+recombined_basedir);
   sized_recombined_subdirs.push_back(fullsized_subdir+recombined_basedir);
   sized_recombined_subdirs.push_back(halfsized_subdir+recombined_basedir);
   int s_stop = sized_recombined_subdirs.size();

// Subdirectories for optional face quads segmentations:

   string input_quad_images_subdir="";
   vector<string> sized_quad_recombined_subdirs;
   if(argc >= 3)
   {
      input_quad_images_subdir=argv[2];
      filefunc::add_trailing_dir_slash(input_quad_images_subdir);
      string doublesized_quads_subdir=input_quad_images_subdir+"doublesized/";
      string fullsized_quads_subdir = input_quad_images_subdir + "fullsized/";
      string halfsized_quads_subdir = input_quad_images_subdir + "halfsized/";
      sized_quad_recombined_subdirs.push_back(
         doublesized_quads_subdir+recombined_basedir);
      sized_quad_recombined_subdirs.push_back(
         fullsized_quads_subdir+recombined_basedir);
      sized_quad_recombined_subdirs.push_back(
         halfsized_quads_subdir+recombined_basedir);
   }

// On 5/22/16, we learned the hard and painful way that the total
// number of fullsized, halfsized and doublesized segmented image
// files may not be equal!  

   string image_filename;
   texture_rectangle *tr_ptr = NULL;
   texture_rectangle *halfsized_tr_ptr = NULL;
   texture_rectangle *doublesized_tr_ptr = NULL;
   texture_rectangle *cc_tr_ptr = NULL;
   texture_rectangle *score_tr_ptr = NULL;
   texture_rectangle *halfsized_score_tr_ptr = NULL;
   texture_rectangle *doublesized_score_tr_ptr = NULL;

// Export detected face and hand bounding boxes to output text file:

   string output_filename=
      input_images_subdir.substr(0,input_images_subdir.size()-1)+"_extracted_bboxes.txt";
   ofstream outstream;
   cout << "output_filename = " << output_filename << endl;
   filefunc::openfile(output_filename, outstream);
   outstream << "# " << timefunc::getcurrdate() << endl;
   outstream << "# Image: index  ID_str " << endl;
   outstream << "# CC_ID  Class  xmin  xmax  ymin ymax " << endl << endl;

   unsigned int istart = 0;
   unsigned int istop = image_ID_strings.size();
   for(unsigned int i = istart; i < istop; i++)
   {
      double progress_frac = double(i - istart)/(istop - istart);
      if(i%10 == 0)
      {
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string curr_ID_str = image_ID_strings[i];
      string fullsized_segmented_filename = 
         sized_recombined_subdirs[1]+"segmented_image_"+curr_ID_str+".png";
      string fullsized_scores_filename = 
         sized_recombined_subdirs[1]+"segmentation_scores/scores_image_"
         +curr_ID_str+".png";

      cout << "i = " << i << " Processing image_"+curr_ID_str+".png"
           << endl;

      if(!filefunc::fileexist(fullsized_segmented_filename)) continue;
      if(!filefunc::fileexist(fullsized_scores_filename)) continue;
      
      int full_xdim, full_ydim;
      imagefunc::get_image_width_height(
         fullsized_segmented_filename, full_xdim, full_ydim);
      int full_maxdim = basic_math::max(full_xdim, full_ydim);

      vector<pixel_cc> all_pixel_ccs;
      for(int imagesize = 0; imagesize < s_stop; imagesize++)
      {
         cout << "  imagesize = " << imagesize << endl;
         string segmented_filename = 
            sized_recombined_subdirs[imagesize]+"segmented_image_"
            +curr_ID_str+".png";
         if(!filefunc::fileexist(segmented_filename)) continue;
//         cout << "segmented_filename = " << segmented_filename << endl;

         string recombined_scores_subdir=sized_recombined_subdirs[imagesize]+
            "segmentation_scores/";
         string scores_filename = 
            recombined_scores_subdir+"scores_image_"+curr_ID_str+".png";
         if(!filefunc::fileexist(scores_filename)) continue;
//         cout << "scores_filename = " << scores_filename << endl;

         delete tr_ptr;
         delete score_tr_ptr;
         tr_ptr = new texture_rectangle(fullsized_segmented_filename, NULL);
         score_tr_ptr = new texture_rectangle(fullsized_scores_filename, NULL);

         int min_bbox_dim=0, max_bbox_dim=0;

         if(imagesize == 1)
         {
//          min_bbox_dim = 50;
//          max_bbox_dim = 200;
            min_bbox_dim = 45;
            max_bbox_dim = 300;
         }

// Transfer text pixel labels from double-sized segmented images to
// full-sized segmented image:

         if(imagesize == 0)
         {
            min_bbox_dim = 0.01 * (2 * full_maxdim);
//            max_bbox_dim = 200;
            max_bbox_dim = 300;

            delete doublesized_tr_ptr;
            delete doublesized_score_tr_ptr;
//            cout << "segmented_filename = " << segmented_filename << endl;

            doublesized_tr_ptr = new texture_rectangle(
               segmented_filename, NULL);
            doublesized_score_tr_ptr = new texture_rectangle(
               scores_filename, NULL);

            for(unsigned int qy = 0; qy < doublesized_tr_ptr->getHeight(); 
                qy++)
            {
               for(unsigned int qx = 0; qx < doublesized_tr_ptr->getWidth(); 
                   qx++)
               {
                  double h, s, v;
                  doublesized_tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
                  tr_ptr->set_pixel_hsv_values(qx/2,qy/2,h,s,v);
                  doublesized_score_tr_ptr->get_pixel_hsv_values(
                     qx, qy, h, s, v);
                  score_tr_ptr->set_pixel_hsv_values(qx/2,qy/2,h,s,v);
               } // loop over qx
            } // loop over qy
         } // imagesize == 0 conditional

// Transfer text pixel labels from half-sized segmented images to
// full-sized segmented image:

         else if(imagesize == 2)
         {
//            min_bbox_dim = 50;
            min_bbox_dim = 75;
            max_bbox_dim = full_maxdim;
            
            delete halfsized_tr_ptr;
            delete halfsized_score_tr_ptr;
            halfsized_tr_ptr = new texture_rectangle(segmented_filename, NULL);
            halfsized_score_tr_ptr = new texture_rectangle(
               scores_filename, NULL);

            for(unsigned int qy = 0; qy < halfsized_tr_ptr->getHeight(); qy++)
            {
               for(unsigned int qx = 0; qx < halfsized_tr_ptr->getWidth(); 
                   qx++)
               {
                  double h, s, v;
                  halfsized_tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
                  tr_ptr->set_pixel_hsv_values(2*qx,2*qy, h,s,v);
                  tr_ptr->set_pixel_hsv_values(2*qx+1,2*qy, h,s,v);
                  tr_ptr->set_pixel_hsv_values(2*qx,2*qy+1, h,s,v);
                  tr_ptr->set_pixel_hsv_values(2*qx+1,2*qy+1, h,s,v);

                  halfsized_score_tr_ptr->get_pixel_hsv_values(qx,qy,h,s,v);
                  score_tr_ptr->set_pixel_hsv_values(2*qx,2*qy,h,s,v);
                  score_tr_ptr->set_pixel_hsv_values(2*qx+1,2*qy,h,s,v);
                  score_tr_ptr->set_pixel_hsv_values(2*qx,2*qy+1,h,s,v);
                  score_tr_ptr->set_pixel_hsv_values(2*qx+1,2*qy+1,h,s,v);

               } // loop over qx
            } // loop over qy
         } // imagesize == 2 conditional

// Loop over all pixels within segmented texture rectangle.  Count
// total number whose saturation != 0:

         twoDarray* cc_labels_twoDarray_ptr = new twoDarray(
            full_xdim, full_ydim);
         twoDarray* ptwoDarray_ptr = new twoDarray(full_xdim, full_ydim);
         cc_labels_twoDarray_ptr->clear_values();

         graphicsfunc::LABELS_MAP labels_map;
         graphicsfunc::LABELS_MAP::iterator labels_map_iter;

         for(int curr_class = 0; curr_class < n_classes; curr_class++)
         {
            double curr_class_hue = 77 * curr_class;
            ptwoDarray_ptr->clear_values();

            for(unsigned int qy = 0; qy < tr_ptr->getHeight(); qy++)
            {
               for(unsigned int qx = 0; qx < tr_ptr->getWidth(); qx++)
               {
                  double h, s, v;
                  tr_ptr->get_pixel_hsv_values(qx, qy, h, s, v);
                  if(s > 0)
                  {
//                     cout << "qx = " << qx << " qy = " << qy
//                          << " h = " << h << " s = " << s << " v = " << v 
//                          << endl;
                     if(nearly_equal(h,curr_class_hue,2)) 
                     {
                        ptwoDarray_ptr->put(qx,qy,1);
                        tr_ptr->set_pixel_hsv_values(qx,qy,curr_class_hue,s,v);
                     }
                  }
               } // loop over qx
            } // loop over qy

            int n_neighbors = 8;
            int label_offset = labels_map.size();
            int curr_class_ID = curr_class + 1;
            graphicsfunc::label_connected_components(
               n_neighbors, label_offset, ptwoDarray_ptr, 
               curr_class_ID, labels_map, cc_labels_twoDarray_ptr);
         } // loop over curr_class labeling object classes

// Store info about all connected components within STL vector of
// pixel_cc objects:

         vector<pixel_cc> pixel_ccs;
         for(labels_map_iter = labels_map.begin(); labels_map_iter !=
                labels_map.end(); labels_map_iter++)
         {
            pixel_cc curr_pixel_cc(
               labels_map_iter->first, labels_map_iter->second.first);
            curr_pixel_cc.set_imagesize(imagesize);

            int px_min = full_xdim;
            int px_max = 0;
            int py_min = full_ydim;
            int py_max = 0;

            vector<double> hue_scores;
            for(int py = 0; py < full_ydim; py++)
            {
               for(int px = 0; px < full_xdim; px++)
               {
                  if(cc_labels_twoDarray_ptr->get(px,py) != 
                     curr_pixel_cc.get_ID()) continue;

                  curr_pixel_cc.set_pixel_coords(px,py);

                  px_min = basic_math::min(px_min, px);
                  px_max = basic_math::max(px_max, px);
                  py_min = basic_math::min(py_min, py);
                  py_max = basic_math::max(py_max, py);

                  double h,s,v;
                  score_tr_ptr->get_pixel_hsv_values(px, py, h, s, v);
                  if(s > 0.5)
                  {
                     hue_scores.push_back(h);
                  }

               } // loop over px index
            } // loop over py index

// Ignore any bbox which is either too large or too small:

            int bbox_width = px_max - px_min;
            int bbox_height = py_max - py_min;
            if(bbox_width < min_bbox_dim || bbox_width > max_bbox_dim ||
               bbox_height < min_bbox_dim || bbox_height > max_bbox_dim)
            {
//               cout << "bbox_width = " << bbox_width 
//                    << " bbox_height = " << bbox_height << endl;
//               cout << "min_bbox_dim = " << min_bbox_dim
//                    << " max_bbox_dim = " << max_bbox_dim << endl;
               curr_pixel_cc.set_rejected_flag(true);
            }

            curr_pixel_cc.set_bbox(px_min, px_max, py_min, py_max);
            prob_distribution prob_hues(hue_scores,100);
            double hue_20 = prob_hues.find_x_corresponding_to_pcum(0.20);
            curr_pixel_cc.set_score(hue_20);

// As of 6/22/16, we impose a strong score threshold
// for hands and a more lenient threshold for faces.  We also impose
// stronger scores thresholds as image size decreases:
  
            double score_threshold = get_score_threshold(
               imagesize, curr_pixel_cc.get_class_ID());
            curr_pixel_cc.set_score_threshold(score_threshold);

            if(curr_pixel_cc.get_score() > score_threshold) 
            {
//               cout << "Rejecting curr_pixel_cc which has score "
//                    << curr_pixel_cc.get_score() << endl;
//               cout << "Score_threshold = " << score_threshold << endl;
               curr_pixel_cc.set_rejected_flag(true);
            }
            
            pixel_ccs.push_back(curr_pixel_cc);
            all_pixel_ccs.push_back(curr_pixel_cc);
//            cout << pixel_ccs.back() << endl;
         } // loop over labels_map iterator

         delete cc_tr_ptr;
         cc_tr_ptr = new texture_rectangle(fullsized_segmented_filename, NULL);

         for(int py = 0; py < full_ydim; py++)
         {
            for(int px = 0; px < full_xdim; px++)
            {
               int curr_label = cc_labels_twoDarray_ptr->get(px,py);
               double h = 77 * cc_labels_twoDarray_ptr->get(px,py);
               double s = 1;
               double v = 1;
               if(curr_label == 0)
               {
                  v = 0;
               }
               cc_tr_ptr->set_pixel_hsv_values(px, py, h, s, v);
            }
         }

         delete ptwoDarray_ptr;
         delete cc_labels_twoDarray_ptr;

// Draw bboxes around connected components.  Bbox coloring indicates
// if CC has been rejected or not:

         for(unsigned int c = 0; c < pixel_ccs.size(); c++)
         {
            pixel_cc curr_pixel_cc = pixel_ccs[c];
            if(curr_pixel_cc.get_rejected_flag())
            {
               tr_ptr->draw_pixel_bbox(
                  curr_pixel_cc.get_bbox(), colorfunc::black);
//               cout << "c = " << c << " is rejected" << endl;
            }
            else
            {
               double h = 77 * (curr_pixel_cc.get_class_ID() - 1);
               double s = 1;
               double v = 1;
               double r,g,b;
               colorfunc::hsv_to_RGB(h,s,v,r,g,b);
               int R = 255 * r;
               int G = 255 * g;
               int B = 255 * b;
               tr_ptr->draw_pixel_bbox(curr_pixel_cc.get_bbox(), R,G,B);
            }
         }

         string ccs_image = ccs_images_subdir+"ccs_image_"+
            curr_ID_str+"_"+
            stringfunc::number_to_string(imagesize)+".png";
         tr_ptr->write_curr_frame(ccs_image);
//         cout << "ccs_image = " << ccs_image << endl;

         string scores_image = score_images_subdir+"score_image_"+
            curr_ID_str+"_"+
            stringfunc::number_to_string(imagesize)+".png";
         score_tr_ptr->write_curr_frame(scores_image);
//         cout << "scores_image = " << scores_image << endl;
      } // loop over imagesize index

// Consolidate all pixel CCs across different image pyramid scales:

      for(unsigned int b = 0; b < all_pixel_ccs.size(); b++)
      {
         if(all_pixel_ccs[b].get_rejected_flag()) continue;
         bounding_box curr_bbox = all_pixel_ccs[b].get_bbox();
         double curr_rel_hue_score = 
            fabs(all_pixel_ccs[b].get_score() -
                 all_pixel_ccs[b].get_score_threshold());
         for(unsigned int b2 = 0; b2 < all_pixel_ccs.size(); b2++)
         {
            if(b == b2) continue;
            if(all_pixel_ccs[b].get_rejected_flag()) continue;
            if(all_pixel_ccs[b2].get_rejected_flag()) continue;
            if(all_pixel_ccs[b].get_class_ID() != 
               all_pixel_ccs[b2].get_class_ID()) continue;
            
            bounding_box next_bbox = all_pixel_ccs[b2].get_bbox();

// If current and next bounding boxes have some nontrivial
// intersection, keep only the pixel_cc with the better score relative
// to its score threshold:

            bounding_box intersection_bbox;
            if(curr_bbox.bbox_intersection(next_bbox, intersection_bbox))
            {
               double area_ratio = intersection_bbox.get_area() / 
                  curr_bbox.get_area();
               const double min_area_ratio = 0.25;
               if(area_ratio > min_area_ratio)
               {
                  double next_rel_hue_score = 
                     fabs(all_pixel_ccs[b2].get_score() - 
                          all_pixel_ccs[b2].get_score_threshold());
                  if(curr_rel_hue_score > next_rel_hue_score)
                  {
                     all_pixel_ccs[b2].set_rejected_flag(true);
                  }
                  else
                  {
                     all_pixel_ccs[b].set_rejected_flag(true);
                  }
               }
            }
         } // loop over index b2 labeling other pixel CCs for current image
      } // loop over index b labeling all pixel CCs for current image

// Search for face centers via quad segmentations for all non-rejected
// pixel CCs:

      if(sized_quad_recombined_subdirs.size() > 0)
      {
         vector<string> segmented_quads_filename;
         segmented_quads_filename.push_back(
            sized_quad_recombined_subdirs[0]+
            "segmented_image_"+curr_ID_str+".png");
         segmented_quads_filename.push_back(
            sized_quad_recombined_subdirs[1]+
            "segmented_image_"+curr_ID_str+".png");
         segmented_quads_filename.push_back(
            sized_quad_recombined_subdirs[2]+
            "segmented_image_"+curr_ID_str+".png");
 
         vector<texture_rectangle*> quads_tr_ptrs;
         for(unsigned int s = 0; s < segmented_quads_filename.size(); s++)
         {
            if(filefunc::fileexist(segmented_quads_filename[s]))
            {
               quads_tr_ptrs.push_back(
                  new texture_rectangle(segmented_quads_filename[s], NULL));
            }
            else
            {
               quads_tr_ptrs.push_back(NULL);
            }
         }
      
         vector<bool> export_flags;
         export_flags.push_back(false);
         export_flags.push_back(false);
         export_flags.push_back(false);

         vector<pixel_cc> new_pixel_ccs;
         for(unsigned int c = 0; c < all_pixel_ccs.size(); c++)
         {
            pixel_cc curr_pixel_cc = all_pixel_ccs[c];
            if(curr_pixel_cc.get_rejected_flag()) continue;

            if(curr_pixel_cc.get_class_ID() != 1)
            {
               new_pixel_ccs.push_back(curr_pixel_cc);
               continue;
            }
            
            int curr_s = curr_pixel_cc.get_imagesize();
            if(quads_tr_ptrs[curr_s] == NULL) continue;

// Tentatively reset rejected_flag --> true:

            curr_pixel_cc.set_rejected_flag(true);

            bounding_box curr_bbox = curr_pixel_cc.get_bbox();
            if(curr_s == 0)
            {
               curr_bbox.dilate(2);
            }
            else if (curr_s == 2)
            {
               curr_bbox.dilate(0.5);
            }

//            quads_tr_ptrs[curr_s]->draw_pixel_bbox(
//            curr_bbox,255,0,255);
            quads_tr_ptrs[curr_s]->draw_pixel_bbox(
               curr_bbox,colorfunc::orange);

// IDEA: Count total number of tl, tr, bl, br pixels within curr_bbox.
// If they roughly equal 0.5 of curr_bbox's volume, do NOT split
// curr_pixel_cc into any smaller pixel CCs:

            int n_red, n_yellow, n_green, n_blue;
            n_red = n_yellow = n_green = n_blue = 0;
            int n_bbox_pixels = curr_bbox.get_xextent() * 
               curr_bbox.get_yextent();
            for(int py = curr_bbox.get_ymin(); py < curr_bbox.get_ymax();
                py++)
            {
               for(int px = curr_bbox.get_xmin(); px < curr_bbox.get_xmax();
                   px++)
               {
                  double h,s,v;
                  quads_tr_ptrs[curr_s]->get_pixel_hsv_values(
                     px,py,h,s,v);

// Score bbox overlap with quadrant segmentations by requiring green
// pixel to be located in upper left quadrant of small_bbox, blue
// pixels in upper right quadrant, red pixels in lower left quadrant
// and yellow pixels in lower right quadrant:

                  if(s > 0.1)
                  {
                     if(nearly_equal(h,0,3)) 
                     {
                        n_red++;
                     }
                     else if (nearly_equal(h,77,3)) 
                     {
                        n_yellow++;
                     }
                     else if (nearly_equal(h,154,3))
                     {
                        n_green++;
                     }
                     else if (nearly_equal(h,231,3))
                     {
                        n_blue++;
                     }
                  }
               } // loop over px indexing pixels inside curr_bbox
            } // loop over py indexing pixels inside curr_bbox

            double red_frac = double(n_red)/n_bbox_pixels;
            double yellow_frac = double(n_yellow)/n_bbox_pixels;
            double green_frac = double(n_green)/n_bbox_pixels;
            double blue_frac = double(n_blue)/n_bbox_pixels;
            double tot_colored_frac = red_frac + yellow_frac + green_frac 
               + blue_frac;

//            cout << "curr_s = " << curr_s << endl;
//            cout << "n_red = " << n_red << " n_yellow = " << n_yellow
//                 << " n_green = " << n_green << " n_blue = " << n_blue
//                 << endl;
//            cout << "n_bbox_pixels = " << n_bbox_pixels << endl;
//            cout << "red_frac = " << red_frac << endl;
//            cout << "yellow_frac = " << yellow_frac << endl;
//            cout << "green_frac = " << green_frac << endl;
//            cout << "blue_frac = " << blue_frac << endl;
//            cout << "tot_colored_frac = "
//                 << red_frac+yellow_frac+green_frac+blue_frac
//                 << endl;

            vector<bounding_box> maximal_small_bboxes;
            bool all_centers_found = false;
            int max_iters = 8;

// Experiment with disallowing potential separation of large pixel_CC
// bboxes if their relative quadrant contents are nearly equal and
// fill up most of the bbox:

            if(n_bbox_pixels >= 100000 &&
               red_frac >= 0.15 && red_frac < 0.30 &&
               yellow_frac >= 0.15 && yellow_frac < 0.30 &&
               green_frac >= 0.15 && green_frac < 0.30 &&
               blue_frac >= 0.15 && blue_frac < 0.30 &&
               tot_colored_frac > 0.85)
            {
               max_iters = 1;
            }
//            cout << "max_iters = " << max_iters << endl;

            for(int iter = 0; iter < max_iters; iter++)
            {
//               cout << "iter = " << iter 
//                    << " curr_s = " << curr_s << endl;

// Take small_bbox to be a fractional size of curr_bbox.  Force
// small_bbox to have aspect_ratio <= 1:
               
               bounding_box small_bbox(curr_bbox);
               if(small_bbox.get_aspect_ratio() > 1)
               {
                  double small_height = small_bbox.get_yextent();
                  double xmin = small_bbox.get_xcenter() - 0.5 * small_height;
                  double xmax = small_bbox.get_xcenter() + 0.5 * small_height;
                  small_bbox.set_x_bounds(xmin, xmax);
               }
               
               double dilation_factor = 0.50 - iter * 0.05;
               small_bbox.dilate(dilation_factor);

// Drag small bbox over curr_bbox.  Search for small bbox location
// where it contains pixel content from all quadrants:

               int n_gbry = -1;
               int max_n_gbry = -1;
               int best_n_red, best_n_yellow, best_n_green, best_n_blue;
               int py_skip = 2, px_skip = 2;
//               int py_skip = 1, px_skip = 1;

               for(int py = curr_bbox.get_ymin(); py < curr_bbox.get_ymax(); 
                   py += py_skip)
               {
                  for(int px = curr_bbox.get_xmin(); 
                      px < curr_bbox.get_xmax(); px += px_skip)
                  {
                     int n_red, n_yellow, n_green, n_blue;
                     n_red = n_yellow = n_green = n_blue = 0;
                     small_bbox.recenter(px,py);

// Ignore any current small_bbox which overlaps any previously-found
// maximal small bboxes:

                     bool overlap_previous_small_bbox = false;
                     for(unsigned int b = 0; b < maximal_small_bboxes.size(); 
                         b++)
                     {
                        if(small_bbox.overlap(maximal_small_bboxes[b]))
                        {
                           overlap_previous_small_bbox = true;
                           break;
                        }
                     }
                     if(overlap_previous_small_bbox) continue;

                     int qy_skip = 0.05 * small_bbox.get_yextent();
                     qy_skip = basic_math::max(1, qy_skip);

//                     cout << "px = " << px << " py = " << py 
//                          << " qy_skip = " << qy_skip << endl;
                  
                     for(int qy = small_bbox.get_ymin(); 
                         qy < small_bbox.get_ymax(); qy += qy_skip)
                     {

                        if(qy < curr_bbox.get_ymin() ||
                           qy > curr_bbox.get_ymax()) continue;

                        int qx_skip = 0.05 * small_bbox.get_xextent();
                        if(n_gbry == 0)
                        {
                           qx_skip = 0.10 * small_bbox.get_xextent();
                        }
                        qx_skip = basic_math::max(1, qx_skip);

                        for(int qx = small_bbox.get_xmin();
                            qx < small_bbox.get_xmax(); qx += qx_skip)
                        {
                           if(qx < curr_bbox.get_xmin() ||
                              qx > curr_bbox.get_xmax()) continue;

                           double h,s,v;
                           quads_tr_ptrs[curr_s]->get_pixel_hsv_values(
                              qx,qy,h,s,v);

// Score bbox overlap with quadrant segmentations by requiring green
// pixel to be located in upper left quadrant of small_bbox, blue
// pixels in upper right quadrant, red pixels in lower left quadrant
// and yellow pixels in lower right quadrant:

                           if(s > 0.1)
                           {
                              double xc=small_bbox.get_xcenter();
                              double yc=small_bbox.get_ycenter();

                              if(nearly_equal(h,0,3) &&
                                 qx < xc && qy >= yc) // lower left 
                              {
                                 n_red++;
                              }
                              else if (nearly_equal(h,77,3) &&
                                       qx >= xc && qy >= yc) // lower right 
                              {
                                 n_yellow++;
                              }
                              else if (nearly_equal(h,154,3) && 
                                       qx < xc && qy < yc) // upper left 
                              {
                                 n_green++;
                              }
                              else if (nearly_equal(h,231,3) &&
                                       qx >= xc && qy < yc) // upper right 
                              {
                                 n_blue++;
                              }
                           }
                        } // loop over qx
                     } // loop over qy

                     if(n_green == 0 || n_blue == 0 || n_red == 0 ||
                        n_yellow == 0) continue;
                  
                     n_gbry = n_green + n_blue + n_red + n_yellow;
                     if (n_gbry > max_n_gbry)
                     {
                        curr_pixel_cc.set_center(px,py);
                        max_n_gbry = n_gbry;
                        best_n_green = n_green;
                        best_n_blue = n_blue;
                        best_n_red = n_red;
                        best_n_yellow = n_yellow;
                     }
                  } // loop over px
               } // loop over py

               int px_center, py_center;
               curr_pixel_cc.get_center(px_center, py_center);

//               cout << "max_n_gbry = " << max_n_gbry << endl;
               if(max_n_gbry > 1)
               {
                  double gb_ratio = double(best_n_green) / best_n_blue;
                  double gr_ratio = double(best_n_green) / best_n_red;
                  double gy_ratio = double(best_n_green) / best_n_yellow;
                  double br_ratio = double(best_n_blue) / best_n_red;
                  double by_ratio = double(best_n_blue) / best_n_yellow;
                  double ry_ratio = double(best_n_red) / best_n_yellow;

//                  cout << "best_n_green = " << best_n_green 
//                       << " best_n_blue = " << best_n_blue << endl;
//                  cout << "best_n_red = " << best_n_red
//                       << " best_n_yellow = " << best_n_yellow << endl;

//                  cout << "gb_ratio = " << gb_ratio << endl;
//                  cout << "gr_ratio = " << gr_ratio << endl;
//                  cout << "gy_ratio = " << gy_ratio << endl;
//                  cout << "br_ratio = " << br_ratio << endl;
//                  cout << "by_ratio = " << by_ratio << endl;
//                  cout << "ry_ratio = " << ry_ratio << endl;
//                  cout << "maximal_small_bboxes.size() = "
//                       << maximal_small_bboxes.size() << endl;

// Ignore any 2nd-best, 3rd-best, etc candidate quadrant center whose
// quadrant contents differ too significantly in magnitude:

//                  const double quad_thr = 2;  // too low
//                  const double quad_thr = 3;
                  const double quad_thr = 4;
//                  const double quad_thr = 5;  // pretty good
//                  const double quad_thr = 7;  // too large

                  if( (maximal_small_bboxes.size() == 0) ||
                      (maximal_small_bboxes.size() >= 1 &&
                       gb_ratio > 1/quad_thr && gb_ratio < quad_thr &&
                       gr_ratio > 1/quad_thr && gr_ratio < quad_thr &&
                       gy_ratio > 1/quad_thr && gy_ratio < quad_thr &&
                       br_ratio > 1/quad_thr && br_ratio < quad_thr &&
                       by_ratio > 1/quad_thr && by_ratio < quad_thr &&
                       ry_ratio > 1/quad_thr && ry_ratio < quad_thr) )
                  {
                     small_bbox.recenter(px_center, py_center);
                     quads_tr_ptrs[curr_s]->draw_pixel_bbox(
                        small_bbox,255,255,255);
                     curr_pixel_cc.set_rejected_flag(false);
                     maximal_small_bboxes.push_back(small_bbox);
//                     cout << "max_small_bboxes.size = "
//                          << maximal_small_bboxes.size() << endl;
                  }
               }
               else
               {
                  all_centers_found = true;
               }
               
               for(int s = 0 ; s < 3; s++)
               {
                  if(curr_s == s)
                  {
                     export_flags[s] = true;
                  }
               }
            } // loop over iter labeling attempts to split current pixel_CC
              // into more than one face

            if(maximal_small_bboxes.size() == 0) continue;

// Create as many new separated pixel ccs as there are maximal small
// bboxes:

            vector<pixel_cc> separated_pixel_ccs;
            for(unsigned int m = 0; m < maximal_small_bboxes.size(); m++)
            {
               bounding_box curr_bbox(maximal_small_bboxes[m]);
               int curr_ID = new_pixel_ccs.size() + m;
               int class_ID = curr_pixel_cc.get_class_ID();
               pixel_cc separated_pixel_cc(curr_ID, class_ID);
               separated_pixel_cc.set_imagesize(curr_pixel_cc.get_imagesize());
               separated_pixel_cc.set_score(curr_pixel_cc.get_score());
               separated_pixel_cc.set_score_threshold(
                  curr_pixel_cc.get_score_threshold());

               if(separated_pixel_cc.get_imagesize() == 0)
               {
                  curr_bbox.dilate(0.5);
               }
               else if (separated_pixel_cc.get_imagesize() == 2)
               {
                  curr_bbox.dilate(2);
               }
               separated_pixel_cc.set_bbox(curr_bbox);
               separated_pixel_cc.set_center(curr_bbox.get_xcenter(),
                                             curr_bbox.get_ycenter());
               separated_pixel_ccs.push_back(separated_pixel_cc);
            } // loop over index m labeling maximal small bboxes

// Reassign all pixels within curr_pixel_cc to the closest
// maximal_pixel_cc center:

            vector<pixel_cc::PIXEL_COORDINATES>* pixel_coords_ptr = 
               curr_pixel_cc.get_pixel_coords_ptr();
            for(unsigned int j = 0; j < pixel_coords_ptr->size(); j++)
            {
               pixel_cc::PIXEL_COORDINATES curr_pixel_coords = 
                  pixel_coords_ptr->at(j);

               int closest_m = -1;
               double min_sqr_dist = POSITIVEINFINITY;
               for(unsigned int m = 0; m < separated_pixel_ccs.size(); m++)
               {
                  int px_m, py_m;
                  separated_pixel_ccs[m].get_center(px_m, py_m);
                  double curr_sqr_dist = sqr(px_m - curr_pixel_coords.first) +
                     sqr(py_m - curr_pixel_coords.second);
                  if(curr_sqr_dist < min_sqr_dist)
                  {
                     min_sqr_dist = curr_sqr_dist;
                     closest_m = m;
                  }
               } // loop over index m labeling separated pixel ccs
               separated_pixel_ccs[closest_m].get_pixel_coords_ptr()->
                  push_back(curr_pixel_coords);
               separated_pixel_ccs[closest_m].get_bbox().update_bounds(
                  curr_pixel_coords.first, curr_pixel_coords.second);
            } // loop over index j labeling pixel coords

// Empirical hack: Check if all separated pixel CCs have bboxes with
// aspect ratios > 1.  If so, reject them and keep original pixel CC:

            bool all_aspect_ratios_greater_than_one = true;
            for(unsigned int m = 0; m < separated_pixel_ccs.size(); m++)
            {
               if(separated_pixel_ccs[m].get_bbox().get_aspect_ratio() < 1)
               {
                  all_aspect_ratios_greater_than_one = false;
                  break;
               }
            }

            if(all_aspect_ratios_greater_than_one)
            {
               new_pixel_ccs.push_back(all_pixel_ccs[c]);
            }
            else
            {
               for(unsigned int m = 0; m < separated_pixel_ccs.size(); m++)
               {
                  new_pixel_ccs.push_back(separated_pixel_ccs[m]);
               }
            }
         } // loop over index c labeling pixel CCs

// Refill all_pixel_ccs with pixel_ccs sitting inside
// new_pixel_ccs:

         all_pixel_ccs.clear();
         for(unsigned int cc = 0; cc < new_pixel_ccs.size(); cc++)
         {
            all_pixel_ccs.push_back(new_pixel_ccs[cc]);
         }

         for(int s = 0; s < 3; s++)
         {
            if(export_flags[s])
            {
               string quads_filename=quad_images_subdir+
                  "quads_"+curr_ID_str+"_"+
                  stringfunc::number_to_string(s)+".png";
               quads_tr_ptrs[s]->write_curr_frame(quads_filename);
            }
         }
         
      } // sized_quad_recombined_subdirs.size() > 0 conditional      

// Compute prob distributions for each pixel CCs horizontal and
// vertical pixel coordinates.  Reset CC bbox borders to correspond to
// 5% and 95% cumulative values of these prob densities:

      for(unsigned int cc = 0; cc < all_pixel_ccs.size(); cc++)
      {
         vector<pixel_cc::PIXEL_COORDINATES>* pixel_coords_ptr = 
            all_pixel_ccs[cc].get_pixel_coords_ptr();
         int n_pixels = pixel_coords_ptr->size();
         if(n_pixels < 100) continue;
         
         vector<double> px_values, py_values;
         for(int p = 0; p < n_pixels; p++)
         {
            px_values.push_back(pixel_coords_ptr->at(p).first);
            py_values.push_back(pixel_coords_ptr->at(p).second);
         }
         prob_distribution prob_px(
            px_values, 100,mathfunc::minimal_value(px_values));
         prob_distribution prob_py(
            py_values, 100,mathfunc::minimal_value(py_values));
         int px_03 = prob_px.find_x_corresponding_to_pcum(0.03);
         int px_97 = prob_px.find_x_corresponding_to_pcum(0.97);
         int py_03 = prob_py.find_x_corresponding_to_pcum(0.03);
         int py_97 = prob_py.find_x_corresponding_to_pcum(0.97);
         all_pixel_ccs[cc].set_bbox(px_03, px_97, py_03, py_97);         
      } // loop over index cc labeling pixel CCs

// Eliminate any face bbox which is completely surrounded by some hand
// bbox!

      for(unsigned int cc = 0; cc < all_pixel_ccs.size(); cc++)
      {
         if (all_pixel_ccs[cc].get_rejected_flag()) continue;
         if (all_pixel_ccs[cc].get_class_ID() != 2) continue; // not hand

         for(unsigned int next_cc = 0; next_cc < all_pixel_ccs.size(); 
             next_cc++)
         {
            if(all_pixel_ccs[next_cc].get_rejected_flag()) continue;
            if(all_pixel_ccs[next_cc].get_class_ID() != 1) continue;// not face

            if(all_pixel_ccs[cc].get_bbox().encloses(
                  all_pixel_ccs[next_cc].get_bbox()))
            {
               all_pixel_ccs[next_cc].set_rejected_flag(true);
            }
         } // loop over index next_cc
      } // loop over index cc

// Export final set of pixel CCs and their bounding boxes:

      delete tr_ptr;
      string fullsized_image_filename = 
         fullsized_subdir+"image_"+curr_ID_str+".jpg";
      tr_ptr = new texture_rectangle(fullsized_image_filename, NULL);
      tr_ptr->convert_color_image_to_greyscale();

// Tint non-rejected CC's pixels:

      for(unsigned int c = 0; c < all_pixel_ccs.size(); c++)
      {
         pixel_cc curr_pixel_cc = all_pixel_ccs[c];
         if(curr_pixel_cc.get_rejected_flag()) continue;
         for(unsigned int p = 0; 
             p < curr_pixel_cc.get_pixel_coords_ptr()->size(); p++)
         {
            int px = curr_pixel_cc.get_pixel_coords_ptr()->at(p).first;
            int py = curr_pixel_cc.get_pixel_coords_ptr()->at(p).second;
            double h,s,v;
            tr_ptr->get_pixel_hsv_values(px,py,h,s,v);

            h = 77 * (curr_pixel_cc.get_class_ID() - 1);
            double smin = 0.66;
            double smax = 1.0;
            double vmin = 0.5;
            double vmax = 1.0;
            s = smin + s * (smax - smin);
            v = vmin + v * (vmax - vmin);
            tr_ptr->set_pixel_hsv_values(px,py,h,s,v);
         }
      } // loop over index c labeling pixel CCs

// Draw bboxes around non-rejected connected component:

      outstream << "Image: index = " << i 
                << " ID_str = " << curr_ID_str << endl;

      int exported_bbox_counter = 0;
      for(unsigned int c = 0; c < all_pixel_ccs.size(); c++)
      {
         pixel_cc curr_pixel_cc = all_pixel_ccs[c];
         if(curr_pixel_cc.get_rejected_flag()) continue;

         double h = 77 * (curr_pixel_cc.get_class_ID() - 1) + 30;
         double s = 1;
         double v = 1;
         double r,g,b;
         colorfunc::hsv_to_RGB(h,s,v,r,g,b);
         int R = 255 * r;
         int G = 255 * g;
         int B = 255 * b;
         bounding_box curr_bbox(curr_pixel_cc.get_bbox());
         tr_ptr->draw_pixel_bbox(curr_bbox, R,G,B);

         outstream << exported_bbox_counter++ << "  "
                   << curr_pixel_cc.get_class_ID() << "   "
                   << curr_bbox.get_xmin() << "  "
                   << curr_bbox.get_xmax() << "  "
                   << curr_bbox.get_ymin() << "  "
                   << curr_bbox.get_ymax() 
                   << endl;

      } // loop over index c labeling pixel CCs
      cout << endl;
      outstream << endl;

      string segmented_image = consolidated_images_subdir+
         "segmented_image_"+curr_ID_str+".png";
      tr_ptr->write_curr_frame(segmented_image);
      cout << "Exported " << segmented_image << endl << endl;

   } // loop over index i labeling segmented images

   filefunc::closefile(output_filename, outstream);

   banner="Exported extracted bboxes to "+output_filename;
   outputfunc::write_big_banner(banner);
}
