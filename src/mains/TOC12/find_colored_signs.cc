// ==========================================================================
// Program FIND_COLORED_SIGNS is a playpen for experimenting with
// nominating TOC12 ppt sign regions based primarily upon their color
// content.
// ==========================================================================
// Last updated on 9/13/12; 9/18/12; 9/21/12
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <set>
#include  <string>
#include  <vector>

#include "video/connected_components.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/RGB_analyzer.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   string ppt_signs_subdir="./images/ppt_signs/";
   string output_subdir=ppt_signs_subdir+"quantized_colors/";
   filefunc::dircreate(output_subdir);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* edges_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* black_grads_texture_rectangle_ptr=
      new texture_rectangle();
   texture_rectangle* white_grads_texture_rectangle_ptr=
      new texture_rectangle();
   texture_rectangle* quantized_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* filtered_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* binary_texture_rectangle_ptr=new texture_rectangle();

   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   vector<string> colors_to_find;

   double min_gradient_mag=0.5; 
//   cout << "Enter minimium gradient magnitude:" << endl;
//   cin >> min_gradient_mag;


/*
   vector<int> sign_IDs;
   vector<vector<string> > sign_colors_to_find;
   vector<string> sign_hues;

   sign_IDs.push_back(0);	// Yellow radiation
   sign_IDs.push_back(1);	// Water

   sign_hues.push_back("yellow");	// Yellow radiation
   sign_hues.push_back("blue");		// Water
*/

// Yellow radiation TOC12 ppt sign:

   RGB_analyzer_ptr->import_quantized_RGB_lookup_table("yellow");

   colors_to_find.push_back("yellow");
   colors_to_find.push_back("darkyellow");
   colors_to_find.push_back("lightyellow");
   string sign_hue="yellow";

   double min_aspect_ratio=0.4;
   double max_aspect_ratio=2.2;
   double min_compactness=0.055;
   double max_compactness=0.115;
   int min_n_holes=3;
   int max_n_holes=6;
   int min_n_crossings=2;
   int max_n_crossings=6;
   int min_n_significant_holes=2;
   int max_n_significant_holes=5;

   bool black_interior_flag=true;
//   bool black_interior_flag=false;
   bool white_interior_flag=false;

/*
// Blue water TOC12 ppt sign:

   RGB_analyzer_ptr->import_quantized_RGB_lookup_table("blue"); 
   
   colors_to_find.push_back("blue");
   colors_to_find.push_back("darkblue");
   colors_to_find.push_back("greyblue");
   string sign_hue="blue";

   double min_aspect_ratio=0.4;
   double max_aspect_ratio=2.2;
   double min_compactness=0.055;
   double max_compactness=0.16;
   int min_n_holes=1;
   int max_n_holes=6;
   int min_n_crossings=2;
   int max_n_crossings=6;

   int min_n_significant_holes=1;
   int max_n_significant_holes=4;

   bool black_interior_flag=false;
   bool white_interior_flag=true;
*/

/*
// Blue-purple radiation sign:

   RGB_analyzer_ptr->import_quantized_RGB_lookup_table("blue"); 

   colors_to_find.push_back("blue");
   colors_to_find.push_back("darkblue");
   colors_to_find.push_back("greyblue");
   string sign_hue="blue";

   double min_aspect_ratio=0.4;
   double max_aspect_ratio=2.2;
   double min_compactness=0.055;
   double max_compactness=0.16;
   int min_n_holes=1;
   int max_n_holes=7;
   int min_n_crossings=2;
   int max_n_crossings=6;

   int min_n_significant_holes=1;
   int max_n_significant_holes=5;

   bool black_interior_flag=false;
   bool white_interior_flag=false;
*/


/*
// Blue gasoline sign:

   RGB_analyzer_ptr->import_quantized_RGB_lookup_table("blue2");

   colors_to_find.push_back("blue");
   colors_to_find.push_back("darkblue");
   colors_to_find.push_back("greyblue");
   string sign_hue="blue2";

   double min_aspect_ratio=0.4;
   double max_aspect_ratio=2.2;
   double min_compactness=0.045;
   double max_compactness=0.16;
   int min_n_holes=1;
   int max_n_holes=5;
   int min_n_crossings=2;
   int max_n_crossings=8;

   int min_n_significant_holes=1;
   int max_n_significant_holes=3;

   bool black_interior_flag=false;
   bool white_interior_flag=true;
*/

/*
// Orange biohazard sign:

   RGB_analyzer_ptr->import_quantized_RGB_lookup_table("orange");
   
   colors_to_find.push_back("orange");
   colors_to_find.push_back("red");
   string sign_hue="orange";

   double min_aspect_ratio=0.4;
   double max_aspect_ratio=3.9;
   double min_compactness=0.045;
   double max_compactness=0.19;
   int min_n_holes=1;
   int max_n_holes=10;
   int min_n_crossings=2;
   int max_n_crossings=8;

   int min_n_significant_holes=1;
   int max_n_significant_holes=9;

   bool black_interior_flag=true;
*/

/*
// Red stop sign:

   RGB_analyzer_ptr->import_quantized_RGB_lookup_table("red");

   colors_to_find.push_back("red");
   colors_to_find.push_back("lightred");
   string sign_hue="red";

   double min_aspect_ratio=0.4;
//   double max_aspect_ratio=3.9;
   double max_aspect_ratio=2;
   double min_compactness=0.045;
   double max_compactness=0.23;
   int min_n_holes=0;
   int max_n_holes=5;
   int min_n_crossings=2;
   int max_n_crossings=8;

//   int min_n_significant_holes=0;
   int min_n_significant_holes=1;
   int max_n_significant_holes=4;

   min_gradient_mag=0.3;
   bool black_interior_flag=false;
   bool white_interior_flag=true;
*/

/*
// Green start sign:

   RGB_analyzer_ptr->import_quantized_RGB_lookup_table("green");

   colors_to_find.push_back("green");
   colors_to_find.push_back("lightgreen");
   colors_to_find.push_back("darkgreen");
   colors_to_find.push_back("greygreen");
   colors_to_find.push_back("yellow");
   colors_to_find.push_back("lightyellow");
   string sign_hue="green";

   double min_aspect_ratio=0.27;
   double max_aspect_ratio=2;
   double min_compactness=0.045;
   double max_compactness=0.23;
   int min_n_holes=1;
//   int max_n_holes=9;
   int max_n_holes=17;
   int min_n_crossings=2;
   int max_n_crossings=8;

   int min_n_significant_holes=1;
   int max_n_significant_holes=14;

   min_gradient_mag=0.3;
   bool black_interior_flag=false;
   bool white_interior_flag=true;
*/

// -----------------------------------------------

   twoDarray* xderiv_twoDarray_ptr=NULL;
   twoDarray* yderiv_twoDarray_ptr=NULL;
   twoDarray* gradient_mag_twoDarray_ptr=NULL;
   twoDarray* gradient_phase_twoDarray_ptr=NULL;

   vector<pair<int,int> > black_pixels,white_pixels;

   int video_start=5;
   int video_stop=19;
//   int video_stop=26;
   int delta_video=1;
//   int delta_video=2;

   cout << "Enter starting video pass number ( >= 5):" << endl;
   cin >> video_start;
   cout << "Enter stopping video pass number ( >= 19):" << endl;
   cin >> video_stop;


   for (int video_number=video_start; video_number <=video_stop; 
        video_number += delta_video)
   {
//      int video_number=1;
//      cout << "Enter video number:" << endl;
//      cin >> video_number;

      string video_subdir=output_subdir+
         "vid_"+stringfunc::integer_to_string(video_number,2)+"/";
      filefunc::dircreate(video_subdir);

      string image_subdir=ppt_signs_subdir+"videos/vid";
      image_subdir += stringfunc::integer_to_string(video_number,2)+"_frames/";

      vector<string> image_filenames=filefunc::image_files_in_subdir(
         image_subdir);

      int i_start=0;
      int i_stop=image_filenames.size()-1;
//      cout << "Last image number = " << image_filenames.size() << endl;
      cout << "Enter starting image number:" << endl;
      cin >> i_start;
      cout << "Enter stopping image number:" << endl;
      cin >> i_stop;
      i_start--;
      i_stop--;

      for (int i=i_start; i<=i_stop; i++)
      {
         string image_filename=image_filenames[i];
         cout << "image_filename = " << image_filename << endl;
         int frame_number=i+1;
     
         cout << "image_filename = " << image_filename << endl;
         texture_rectangle_ptr->reset_texture_content(image_filename);
         edges_texture_rectangle_ptr->reset_texture_content(image_filename);
         black_grads_texture_rectangle_ptr->reset_texture_content(
            image_filename);
         white_grads_texture_rectangle_ptr->reset_texture_content(
            image_filename);
         quantized_texture_rectangle_ptr->reset_texture_content(
            image_filename);
         filtered_texture_rectangle_ptr->reset_texture_content(image_filename);
         binary_texture_rectangle_ptr->reset_texture_content(image_filename);

         string lookup_map_name=sign_hue;
         RGB_analyzer_ptr->quantize_texture_rectangle_colors(
            lookup_map_name,quantized_texture_rectangle_ptr);

//         int n_filter_iters=1;
//         int n_filter_iters=2;
//         int n_filter_iters=3;
         int n_filter_iters=4;
         for (int filter_iter=0; filter_iter < n_filter_iters; filter_iter++)
         {
            RGB_analyzer_ptr->smooth_quantized_image(
               texture_rectangle_ptr,quantized_texture_rectangle_ptr);
         }
         string quantized_filename=video_subdir+
            "quantized_"+stringfunc::number_to_string(frame_number)+".jpg";
         quantized_texture_rectangle_ptr->write_curr_frame(
            quantized_filename);

         string banner3="Exported "+quantized_filename;
         outputfunc::write_big_banner(banner3);

         RGB_analyzer_ptr->isolate_quantized_colors(
            quantized_texture_rectangle_ptr,colors_to_find,
            filtered_texture_rectangle_ptr,binary_texture_rectangle_ptr);
         
         string selected_colors_filename=video_subdir+
            "selected_colors_"+stringfunc::number_to_string(frame_number)+
            ".jpg";
         filtered_texture_rectangle_ptr->write_curr_frame(
            selected_colors_filename);

         string binary_quantized_filename=video_subdir+
            "binary_colors_"+stringfunc::number_to_string(frame_number)+".jpg";
         binary_texture_rectangle_ptr->write_curr_frame(
            binary_quantized_filename);

// Compute edge map.  Require strong edge content within genuine TOC12
// signs:

         if (black_interior_flag)
         {
            edges_texture_rectangle_ptr->convert_color_image_to_greyscale(); 
         }
         else
         {
            edges_texture_rectangle_ptr->convert_color_image_to_luminosity();
         }

         string greyscale_filename=video_subdir+
            "greyscale_"+stringfunc::number_to_string(frame_number)+".jpg";
         edges_texture_rectangle_ptr->write_curr_frame(greyscale_filename);

         twoDarray* ptwoDarray_ptr=edges_texture_rectangle_ptr->
            get_ptwoDarray_ptr();
         ptwoDarray_ptr->set_deltax(1);
         ptwoDarray_ptr->set_deltay(1);
         
         int xdim=ptwoDarray_ptr->get_xdim();
         int ydim=ptwoDarray_ptr->get_ydim();

         if (xderiv_twoDarray_ptr==NULL || 
         xderiv_twoDarray_ptr->get_xdim() != xdim ||
         xderiv_twoDarray_ptr->get_ydim() != ydim)
         {
            xderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
            yderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
            gradient_mag_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
         }
            
         double spatial_resolution=0.25;
         imagefunc::compute_x_y_deriv_fields(
            spatial_resolution,ptwoDarray_ptr,
            xderiv_twoDarray_ptr,yderiv_twoDarray_ptr);
         imagefunc::compute_gradient_magnitude_field(
            xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
            gradient_mag_twoDarray_ptr);

         imagefunc::threshold_intensities_below_cutoff(
            gradient_mag_twoDarray_ptr,min_gradient_mag,0);
         edges_texture_rectangle_ptr->initialize_twoDarray_image(
            gradient_mag_twoDarray_ptr,3,false);

         string edges_filename=video_subdir+
            "edges_"+stringfunc::number_to_string(frame_number)+".jpg";
         edges_texture_rectangle_ptr->write_curr_frame(edges_filename);


         double step_distance=-2;
         if (white_interior_flag) step_distance=2;

         typedef map<int,int> GRADSTEP_MAP;
         GRADSTEP_MAP::iterator gradstep_iter;

         GRADSTEP_MAP black_gradstep_map=
            imagefunc::compute_gradient_steps(
               min_gradient_mag,step_distance,
               xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
               gradient_mag_twoDarray_ptr);
         GRADSTEP_MAP white_gradstep_map=
            imagefunc::compute_gradient_steps(
               min_gradient_mag,-step_distance,
               xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
               gradient_mag_twoDarray_ptr);

//         cout << "black_gradstep_map.size() = " << black_gradstep_map.size()
//              << endl;
//         cout << "white_gradstep_map.size() = " << white_gradstep_map.size()
//              << endl;

         int px,py,qx,qy;
         double stepped_h,stepped_s,stepped_v;

         if (black_interior_flag)
         {

// Search for hot edges which have "black"-colored pixels on their
// cool sides:

            black_pixels.clear();
            for (gradstep_iter=black_gradstep_map.begin(); 
                 gradstep_iter != black_gradstep_map.end(); gradstep_iter++)
            {
               int pixel_ID=gradstep_iter->first;
               int stepped_pixel_ID=gradstep_iter->second;
               graphicsfunc::get_pixel_px_py(pixel_ID,xdim,px,py);
               graphicsfunc::get_pixel_px_py(stepped_pixel_ID,xdim,qx,qy);
               quantized_texture_rectangle_ptr->get_pixel_hsv_values(
                  qx,qy,stepped_h,stepped_s,stepped_v);
            
               if (stepped_v < 0.5)
               {
                  int gradstep_R=255;
                  int gradstep_G=0;
                  int gradstep_B=255;
                  black_grads_texture_rectangle_ptr->set_pixel_RGB_values(
                     px,py,gradstep_R,gradstep_G,gradstep_B);
                  black_pixels.push_back(pair<int,int>(px,py));
               }
            } // loop over black_gradstep_map iterator

            string black_grads_filename=video_subdir+"black_grads_"
               +stringfunc::number_to_string(frame_number)+".jpg";
            cout << "black_grads_filename = " << black_grads_filename << endl;
            black_grads_texture_rectangle_ptr->write_curr_frame(
               black_grads_filename);
         }
         else if (white_interior_flag)
         {

// Search for hot edges which have "white"-colored pixels on their
// warm sides:

            white_pixels.clear();
            for (gradstep_iter=white_gradstep_map.begin(); 
                 gradstep_iter != white_gradstep_map.end(); gradstep_iter++)
            {
               int pixel_ID=gradstep_iter->first;
               int stepped_pixel_ID=gradstep_iter->second;
               graphicsfunc::get_pixel_px_py(pixel_ID,xdim,px,py);
               graphicsfunc::get_pixel_px_py(stepped_pixel_ID,xdim,qx,qy);
               quantized_texture_rectangle_ptr->get_pixel_hsv_values(
                  qx,qy,stepped_h,stepped_s,stepped_v);
            
               if (stepped_v > 0.5 && stepped_s < 0.3)
               {
                  int gradstep_R=255;
                  int gradstep_G=0;
                  int gradstep_B=255;
                  white_grads_texture_rectangle_ptr->set_pixel_RGB_values(
                     px,py,gradstep_R,gradstep_G,gradstep_B);
                  white_pixels.push_back(pair<int,int>(px,py));
               }
            } // loop over gradstep iterator

            string white_grads_filename=video_subdir+"white_grads_"
               +stringfunc::number_to_string(frame_number)+".jpg";
            cout << "white_grads_filename = " << white_grads_filename << endl;
            white_grads_texture_rectangle_ptr->write_curr_frame(
               white_grads_filename);

         } // black_interior_flag and white_interior_flag conditionals

         connected_components* connected_components_ptr=
            new connected_components();
         connected_components* inverse_connected_components_ptr=
            new connected_components();

         int color_channel_ID=-1;
         int image_counter=0;
         connected_components_ptr->reset_image(
            binary_quantized_filename,color_channel_ID,image_counter);
         inverse_connected_components_ptr->reset_image(
            binary_quantized_filename,color_channel_ID,image_counter);
         filefunc::deletefile(binary_quantized_filename);

         int index=0;
//         int threshold=64;
         int threshold=128;
         int level=threshold;
         bool RLE_flag=true;
         bool invert_binary_values_flag=false;
//         bool export_connected_regions_flag=false;
         bool export_connected_regions_flag=true;
         int n_components=
            connected_components_ptr->compute_connected_components(
               index,threshold,level,RLE_flag,invert_binary_values_flag,
               export_connected_regions_flag);
         cout << "n_components = " << n_components << endl;

         int n_inverse_components=
            inverse_connected_components_ptr->compute_connected_components(
               index,threshold,level,RLE_flag,!invert_binary_values_flag,
               export_connected_regions_flag);
         cout << "n_inverse_components = " << n_inverse_components << endl;

         outputfunc::enter_continue_char();

         vector<extremal_region*> extremal_region_ptrs=
            connected_components_ptr->select_extremal_regions(
               level,min_aspect_ratio,max_aspect_ratio,
               min_compactness,max_compactness,min_n_holes,max_n_holes,
               min_n_crossings,max_n_crossings);
         cout << "extremal_region_ptrs.size() = " 
              << extremal_region_ptrs.size() << endl;

         vector<extremal_region*> inverse_extremal_region_ptrs=
            inverse_connected_components_ptr->select_extremal_regions(
               level,0.1,10,0.045,10,0,10,0,10);
         cout << "inverse_extremal_region_ptrs.size() = "
              << inverse_extremal_region_ptrs.size() << endl;
         vector<polygon> bbox_polygons;
         for (int r=0; r<extremal_region_ptrs.size(); r++)
         {
            extremal_region* extremal_region_ptr=extremal_region_ptrs[r];
//            cout << "r = " << r 
//                 << " extremal region = " << *extremal_region_ptr 
//                 << endl;

            if (sign_hue != "grey")
            {
               int dominant_hue_index=
                  RGB_analyzer_ptr->dominant_extremal_region_hue_content(
                     lookup_map_name,extremal_region_ptr,
                     quantized_texture_rectangle_ptr);
               string dominant_hue_name=RGB_analyzer_ptr->
                  get_hue_given_index(dominant_hue_index);
               cout << "Dominant hue = " << dominant_hue_name 
                    << " sign_hue = " << sign_hue << endl;

// Ignore candidate extremal region if its dominant hue does not agree
// with TOC12 ppt sign:

               if (dominant_hue_name=="blue" && sign_hue=="blue2")
               {
               }
               else if (dominant_hue_name != sign_hue) continue;
            }
            
            int left_pu,bottom_pv,right_pu,top_pv;
            extremal_region_ptr->get_bbox(left_pu,bottom_pv,right_pu,top_pv);
            
// Reject bbox if it is very small:

//               cout << "right_pu-left_pu = " << right_pu-left_pu 
//                    << " bottom_pv-top_pv = " << bottom_pv-top_pv
//                    << endl;
               if ((right_pu-left_pu <= 10) && (bottom_pv-top_pv <=10)) 
               continue;


// Next count number of significant holes within extremal region
// based upon inverse regions' bounding box coordinates:

//            const int min_inverse_pixel_area=10;
            const int min_inverse_pixel_area=12;
            int n_significant_holes=0;
            for (int ir=0; ir<inverse_extremal_region_ptrs.size(); ir++)
            {
               extremal_region* inverse_region_ptr=
                  inverse_extremal_region_ptrs[ir];

               int inverse_pixel_area=inverse_region_ptr->get_pixel_area();
//               cout << "inverse_pixel_area = " << inverse_pixel_area
//                    << endl;
               if (inverse_pixel_area <= min_inverse_pixel_area) continue;
               
               int ir_left_pu,ir_bottom_pv,ir_right_pu,ir_top_pv;
               inverse_region_ptr->get_bbox(
                  ir_left_pu,ir_bottom_pv,ir_right_pu,ir_top_pv);

//               cout << "ir_left_pu = " << ir_left_pu 
//                    << " left_pu = " << left_pu << endl;
//               cout << "ir_right_pu = " << ir_right_pu 
//                    << " right_pu = " << right_pu << endl;
//               cout << "ir_bottom_pv = " << ir_bottom_pv 
//                    << " bottom_pv = " << bottom_pv << endl;
//               cout << "ir_top_pv = " << ir_top_pv 
//                    << " top_pv = " << top_pv << endl;

               if (ir_left_pu < left_pu) continue;
               if (ir_right_pu > right_pu) continue;
               if (ir_bottom_pv < bottom_pv) continue;
               if (ir_top_pv > top_pv) continue;

               n_significant_holes++;
            } // loop over index ir labeling inverse regions

            cout << "n_significant_holes = " << n_significant_holes
                 << endl;
            if (n_significant_holes < min_n_significant_holes ||
                n_significant_holes > max_n_significant_holes) continue;

// Require TOC12 sign bbox to contain some minimal number of
// black-colored or white-colored pixels:

            int n_hole_pixels=0;
            bounding_box curr_bbox(left_pu,right_pu,bottom_pv,top_pv);
            if (black_interior_flag)
            {
               for (int p=0; p<black_pixels.size(); p++)
               {
                  int px=black_pixels[p].first;
                  int py=black_pixels[p].second;
                  if (curr_bbox.point_inside(px,py)) n_hole_pixels++;
               }
            } 
            else if (white_interior_flag)
            {
               for (int p=0; p<white_pixels.size(); p++)
               {
                  int px=white_pixels[p].first;
                  int py=white_pixels[p].second;
                  if (curr_bbox.point_inside(px,py)) n_hole_pixels++;
               }
            } // black_interior_flag & white_interior_flag conditionals

            cout << "n_hole_pixels = " << n_hole_pixels << endl;
            if (n_hole_pixels < 5) continue;

            double left_u,top_v,right_u,bottom_v;
            binary_texture_rectangle_ptr->get_uv_coords(
               left_pu,top_pv,left_u,top_v);
            binary_texture_rectangle_ptr->get_uv_coords(
               right_pu,bottom_pv,right_u,bottom_v);

            vector<threevector> vertices;
            vertices.push_back(threevector(left_u,top_v));
            vertices.push_back(threevector(left_u,bottom_v));
            vertices.push_back(threevector(right_u,bottom_v));
            vertices.push_back(threevector(right_u,top_v));
            polygon bbox(vertices);
            bbox_polygons.push_back(bbox);
         } // loop over index r labeling selected extremal regions

         delete connected_components_ptr;
         delete inverse_connected_components_ptr;

         if (bbox_polygons.size() > 0)
         {
            string bboxes_filename=video_subdir+
               "bboxes_"+stringfunc::number_to_string(frame_number)+".jpg";
            int thickness=1;
            videofunc::display_polygons(
               bbox_polygons,texture_rectangle_ptr,
               bboxes_filename,-1,thickness);
         }
         
      } // loop over index i labeling image filenames
   } // loop over video_number 

   delete RGB_analyzer_ptr;
   delete texture_rectangle_ptr;
   delete edges_texture_rectangle_ptr;
   delete quantized_texture_rectangle_ptr;
   delete filtered_texture_rectangle_ptr;
   delete binary_texture_rectangle_ptr;
   delete black_grads_texture_rectangle_ptr;

   delete xderiv_twoDarray_ptr;
   delete yderiv_twoDarray_ptr;
   delete gradient_mag_twoDarray_ptr;
   delete gradient_phase_twoDarray_ptr;
}

