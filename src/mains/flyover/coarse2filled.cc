// ==========================================================================
// Program COARSE2FILLED imports a set of aerial JPG images and 3D
// geometry-based coarse water contours supplied by Sam Friedman.  It
// first computes RGB distributions for pixels residing inside the
// coarse water contours.  Pixels lying inside the water contours
// whose RGB values lie close to the RGB medians are nominated as
// likely water candidates.  If the coarse contours contain more water
// than non-water pixels and if there is not too much RGB variation
// among genuine water pixels, this statistical approach can overcome
// errors in the geometry-based water contours.

// COARSE2FILLED next randomly selects a handful of nominated pixels
// as starting seed locations for flood-filling.  After flood-filling
// is performed on a median-filtered aerial image, the resulting water
// mask is hopefully more accurate than the original coarse water
// region contour.

//			       coarse2filled

// ==========================================================================
// Last updated on 3/5/14; 3/6/14; 3/12/14; 4/14/14
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "image/binaryimagefuncs.h"
#include "color/colorfuncs.h"
#include "geometry/contour.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "math/ltduple.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/polygon.h"
#include "math/prob_distribution.h"
#include "image/recursivefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);

   timefunc::initialize_timeofday_clock();      

   string imagery_subdir="./images/Sam/";
   string training_imagery_subdir=imagery_subdir+"Training_images/";
   string resized_imagery_subdir=training_imagery_subdir+"resized_images/";
   filefunc::dircreate(resized_imagery_subdir);
   string RGB_dists_subdir=training_imagery_subdir+"RGB/";
   filefunc::dircreate(RGB_dists_subdir);

   int min_training_image_ID=-1;

   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_substring(
         training_imagery_subdir,"training");
   vector<string> outline_filenames=
      filefunc::files_in_subdir_matching_substring(
         training_imagery_subdir,"outline");

   texture_rectangle* image_texture_rectangle_ptr=new texture_rectangle();

   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      outputfunc::print_elapsed_time();
      string flyover_jpg_filename=image_filenames[i];
      cout << "flyover_jpg_filename = " << flyover_jpg_filename << endl;
      
      string suffix=stringfunc::suffix(flyover_jpg_filename);
      if (suffix=="png") continue;

// Convert flyover jpg image into PNG format in order to circumvent
// YV12 JPG format:

      string image_prefix=stringfunc::prefix(flyover_jpg_filename);
      string png_image_filename=image_prefix+".png";
      cout << "png_image_filename = " << png_image_filename << endl;
      string unix_cmd="convert "+flyover_jpg_filename+" "+png_image_filename;
//      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      image_texture_rectangle_ptr->import_photo_from_file(png_image_filename);
      unsigned int width=image_texture_rectangle_ptr->getWidth();
      unsigned int height=image_texture_rectangle_ptr->getHeight();
      cout << "width = " << width << " height = " << height << endl;

// Instantiate twoDarray to hold coarse water mask:

      twoDarray* coarse_water_mask_twoDarray_ptr=new twoDarray(
         width,height);
      coarse_water_mask_twoDarray_ptr->clear_values();

// Import (resized version of) Sam's water contour file:

      image_prefix=filefunc::getprefix(flyover_jpg_filename);
      cout << "image_prefix = " << image_prefix << endl;
      int prefix_size=image_prefix.size();
      string image_prefix_wo_training=image_prefix.substr(8,prefix_size-8);
      int training_image_ID=stringfunc::string_to_number(
         image_prefix_wo_training);
      
      if (training_image_ID < min_training_image_ID) continue;

      string outline_filename=training_imagery_subdir+"outline"+
         image_prefix_wo_training+".txt";
      cout << "outline_filename = " << outline_filename << endl;

      if (!filefunc::fileexist(outline_filename)) continue;
      vector< vector<double> > row_numbers=
         filefunc::ReadInRowNumbers(outline_filename);

// Convert coarse water mask contours from pixel to UV image plane
// coordinates:

      int prev_contour_ID=0;
      double min_delta_u=0.02;
      double min_delta_v=0.02;
      double prev_u=NEGATIVEINFINITY, prev_v=NEGATIVEINFINITY;      
      vector<twovector> curr_contour_vertices;
      vector<vector<twovector> > all_contour_vertices;

//      cout << "row_numbers.size() = " << row_numbers.size() << endl;
      for (unsigned int r=0; r<row_numbers.size(); r++)
      {
         int px=row_numbers[r].at(0);
         int py=row_numbers[r].at(1);
	 int contour_ID=row_numbers[r].at(2);

//	 cout << "r = " << r << " px = " << px << " py = " << py << " contour_ID = "
//	      << contour_ID << endl;

	 if (contour_ID != prev_contour_ID) 
         {
//            cout << "curr_contour_vertices.size() = " 
//                 << curr_contour_vertices.size() << endl;
            all_contour_vertices.push_back(curr_contour_vertices);
            curr_contour_vertices.clear();
            prev_contour_ID=contour_ID;
         }

         double u=double(px)/height;
         double v=1-double(py)/height;
         v=basic_math::max(0.0,v);
         v=basic_math::min(1.0,v);
//	 cout << "height = " << height 
//	      << " u = " << u << " v = " << v 
// 	      << " contour_ID = " << contour_ID 
//	      << endl;

	 if (fabs(u-prev_u) > min_delta_u || fabs(v-prev_v) > min_delta_v)
         {
            curr_contour_vertices.push_back(twovector(u,v));
            prev_u=u;
            prev_v=v;
//            cout << "r = " << r << " u = " << u << " v = " << v << " contour_ID = "
//                 << contour_ID << endl;
	 }
      } // loop over index r labeling input rows

//      cout << "curr_contour_vertices.size() = " 
//           << curr_contour_vertices.size() << endl;

      all_contour_vertices.push_back(curr_contour_vertices);
      unsigned int n_contours=all_contour_vertices.size();
      cout << "n_contours = " << n_contours << endl;

// Label image pixels within coarse water mask that lie inside water
// contours with polygon IDs.  Also append pixels' RGB values to STL
// vectors:

      double coarse_contour_area_sum=0;
      vector<double> reds,greens,blues;
      vector<polygon> water_contours;
      for (unsigned int c=0; c<n_contours; c++)
      {
         polygon curr_water_contour(all_contour_vertices[c]);
//	 cout << "curr_water_contour polygon = " << curr_water_contour 
//	      << endl;
         cout << "c = " << c
              << " curr_water_contour.area = "
              << curr_water_contour.compute_area() << endl;

         water_contours.push_back(curr_water_contour);
	 coarse_contour_area_sum += curr_water_contour.get_area();

         int line_thickness=3;
         int contour_color_index=colorfunc::purple;
	 videofunc::display_polygon(
	   curr_water_contour,image_texture_rectangle_ptr,
	   contour_color_index,line_thickness);

	 int R,G,B;
         double u,v;
         for (unsigned int py=0; py<height; py++)
         {
            for (unsigned int px=0; px<width; px++)
            {
               image_texture_rectangle_ptr->get_uv_coords(px,py,u,v);
               if (!curr_water_contour.point_inside_polygon(twovector(u,v)))
                  continue;

               coarse_water_mask_twoDarray_ptr->put(px,py,c+1);
               image_texture_rectangle_ptr->get_pixel_RGB_values(
                  px,py,R,G,B);

//               cout << "px = " << px << " py = " << py
//                    << " R = " << R << " G = " << G << " B = " << B
//                    << endl;
//               outputfunc::enter_continue_char();
               
               reds.push_back(R);
               greens.push_back(G);
               blues.push_back(B);
            } // loop over px index
         } // loop over py index
      
      } // loop over index c labeling different water contours for curr image

      cout << "coarse_contour_area_sum = " << coarse_contour_area_sum << endl;
      cout << "reds.size() = " << reds.size() << endl;
      cout << "greens.size() = " << greens.size() << endl;
      cout << "blues.size() = " << blues.size() << endl;

// Overlay coarse water region polygons on to raw images:
      
      string annotated_image_filename=
         training_imagery_subdir+"coarse_contours_"+image_prefix_wo_training
         +".jpg";
      image_texture_rectangle_ptr->write_curr_frame(annotated_image_filename);

      string banner="Exported "+annotated_image_filename;
      outputfunc::write_big_banner(banner);

// Export R, G and B probability distributions for pixels enclosed by
// coarse water contours:

      vector<string> prob_jpg_filenames;

      string density_filename_prefix=RGB_dists_subdir+"reds_dens_"
         +image_prefix_wo_training;
      prob_distribution prob_reds(reds,500,0);
      prob_reds.set_xmin(0);
      prob_reds.set_xmax(256);
      prob_reds.set_xtic(16);
      prob_reds.set_xsubtic(8);
      prob_reds.set_densityfilenamestr(density_filename_prefix+".meta");
      prob_reds.write_density_dist(false,true);
      prob_jpg_filenames.push_back(density_filename_prefix+".jpg");

      density_filename_prefix=RGB_dists_subdir+"greens_dens_"
         +image_prefix_wo_training;
      prob_distribution prob_greens(greens,500,0);
      prob_greens.set_xmin(0);
      prob_greens.set_xmax(256);
      prob_greens.set_xtic(16);
      prob_greens.set_xsubtic(8);
      prob_greens.set_densityfilenamestr(density_filename_prefix+".meta");
      prob_greens.write_density_dist(false,true);
      prob_jpg_filenames.push_back(density_filename_prefix+".jpg");

      density_filename_prefix=RGB_dists_subdir+"blues_dens_"
         +image_prefix_wo_training;
      prob_distribution prob_blues(blues,500,0);
      prob_blues.set_xmin(0);
      prob_blues.set_xmax(256);
      prob_blues.set_xtic(16);
      prob_blues.set_xsubtic(8);
      prob_blues.set_densityfilenamestr(density_filename_prefix+".meta");
      prob_blues.write_density_dist(false,true);
      prob_jpg_filenames.push_back(density_filename_prefix+".jpg");

// Generate script to display 3 RGB dists together as a montage:

      string script_filename=RGB_dists_subdir+"view_dists_"+
         image_prefix_wo_training;
      ofstream scriptstream;
      filefunc::openfile(script_filename,scriptstream);
      unix_cmd="montageview "+
         filefunc::getbasename(prob_jpg_filenames[0])+" "+
         filefunc::getbasename(prob_jpg_filenames[1])+" "+
         filefunc::getbasename(prob_jpg_filenames[2]);
      scriptstream << unix_cmd << endl;
      filefunc::closefile(script_filename,scriptstream);
      filefunc::make_executable(script_filename);

// Compute median and tight width values for RGB dists generated from 
// coarse water contour regions:

      double Umax=double(width)/double(height);
      double Vmax=1;
      double image_area=Umax*Vmax;
      double UV_area_per_pixel=image_area/(width*height);

      int n_seeds=0;
      double candidate_water_pixel_area=0;
      double candidate_water_pixel_area_frac=0;
      double min_candidate_water_pixel_area_frac=0.03;
      
      double frac_from_median=0;

      typedef map<int,vector<DUPLE>* > WATER_REGION_SEED_CANDIDATES_MAP;
      WATER_REGION_SEED_CANDIDATES_MAP water_region_seed_candidates_map;
      WATER_REGION_SEED_CANDIDATES_MAP::iterator iter;

// independent int = water contour ID
// dependent STL vector holds candidate seed pixel coords

      while (candidate_water_pixel_area_frac < 
             min_candidate_water_pixel_area_frac)
      {
       
// Increment frac_from_median:

         frac_from_median += 0.03;

         double mu_R,mu_G,mu_B;
         double sigma_R,sigma_G,sigma_B;
         mathfunc::median_value_and_percentile_width(
            reds,frac_from_median,mu_R,sigma_R);
         mathfunc::median_value_and_percentile_width(
            greens,frac_from_median,mu_G,sigma_G);
         mathfunc::median_value_and_percentile_width(
            blues,frac_from_median,mu_B,sigma_B);

         cout << "frac_from_median = " << frac_from_median << endl;
         cout << "'water' R = " << mu_R << " +/- " << sigma_R << endl;
         cout << "'water' G = " << mu_G << " +/- " << sigma_G << endl;
         cout << "'water' B = " << mu_B << " +/- " << sigma_B << endl;

// Mark all pixels within coarse water contours which lie within
// current percentile width of RGB median values:

         for (unsigned int py=0; py<height; py++)
         {
            for (unsigned int px=0; px<width; px++)
            {
               int contour_ID=coarse_water_mask_twoDarray_ptr->get(px,py);
               if (contour_ID==0) continue;

	       vector<DUPLE>* V_ptr=NULL;
               iter=water_region_seed_candidates_map.find(contour_ID);
               if (iter==water_region_seed_candidates_map.end())
               {
                  V_ptr=new vector<DUPLE>;
                  water_region_seed_candidates_map[contour_ID]=V_ptr;
               }
               else
               {
                  V_ptr=iter->second;
               }
	    
               int R,G,B;
               image_texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);

               bool water_red_flag = (fabs(R-mu_R) < sigma_R);
               bool water_green_flag = (fabs(G-mu_G) < sigma_G);
               bool water_blue_flag = (fabs(B-mu_B) < sigma_B);
               if (water_red_flag && water_green_flag && water_blue_flag)
               {
		  n_seeds++;
                  colorfunc::RGB flag_RGB=colorfunc::get_RGB_values(
                     colorfunc::get_color(2+contour_ID));
                  image_texture_rectangle_ptr->set_pixel_RGB_values(
                     px,py,255*flag_RGB.first,255*flag_RGB.second, 
                     255*flag_RGB.third);
                  V_ptr->push_back(DUPLE(px,py));

// Null current pixel's entry in *coarse_water_mask_twoDarray_ptr so that 
// we don't consider it again in future while loop iterations:

                  coarse_water_mask_twoDarray_ptr->put(px,py,0);

               }
            } // loop over px 
         } // loop over py

         candidate_water_pixel_area=n_seeds * UV_area_per_pixel;
         candidate_water_pixel_area_frac=candidate_water_pixel_area/
            coarse_contour_area_sum;
         cout << "n_seeds = " << n_seeds 
              << " candidate_water_pixel_area_frac = "
              << candidate_water_pixel_area_frac
              << endl;

      } // candidate_water_pixel_area_frac < 
        // min_candidate_water_pixel_area_frac while loop

      string candidate_seeds_filename=
         training_imagery_subdir+"candidate_seeds_"+
         image_prefix_wo_training+".jpg";
      image_texture_rectangle_ptr->write_curr_frame(candidate_seeds_filename);

      delete coarse_water_mask_twoDarray_ptr;

// Randomly select seeds from among candidate pixels that lie within
// each coarse water region contour:

      vector<twovector> seed_UVs;

      cout << "n_contours = " << n_contours << endl;
      image_texture_rectangle_ptr->import_photo_from_file(png_image_filename);
      for (unsigned int c=1; c<=n_contours; c++)
      {
         iter=water_region_seed_candidates_map.find(c);

         vector<DUPLE>* V_ptr=iter->second;
         cout << "V_ptr->size() = " << V_ptr->size() << endl;
         if (V_ptr->size()==0)
         {
            cout << "No candidate seeds found within coarse contour " 
                 << c << endl;
            continue;
         }

         unsigned int n_seeds_per_contour=3;
         for (unsigned int s=0; s<n_seeds_per_contour; s++)
         {
            int sector_size=1.0/n_seeds_per_contour*V_ptr->size();
            int candidate_seed_index=
               (s+nrfunc::ran1())*sector_size;
            DUPLE seed_duple=V_ptr->at(candidate_seed_index);

            double seed_U,seed_V;
            image_texture_rectangle_ptr->get_uv_coords(
               seed_duple.first,seed_duple.second,seed_U,seed_V);
            twovector curr_seed_UV(seed_U,seed_V);
            seed_UVs.push_back(curr_seed_UV);

// Mark seed locations with colored circles:

            int pixel_radius=10;
            image_texture_rectangle_ptr->fill_circle(
               seed_duple.first,seed_duple.second,
               pixel_radius,colorfunc::darkpurple);

         } // loop over s index labeling seeds for current contour
         
      } // loop over index c labeling coarse water contours

      string seeds_filename=
         training_imagery_subdir+"seeds_"+
         image_prefix_wo_training+".jpg";
      image_texture_rectangle_ptr->write_curr_frame(seeds_filename);

// Generate script to display candidate_seeds and actual seeds outputs
// together as a montage:

      int blank_width=40;
      string separator_filename="separator.jpg";
      image_texture_rectangle_ptr->generate_blank_image_file(
         blank_width,height,separator_filename,1.0);
      unix_cmd="montageview "+candidate_seeds_filename+" "+
         separator_filename+" "+seeds_filename+" NO_DISPLAY";
      sysfunc::unix_command(unix_cmd);

      unix_cmd="mv montage*.jpg "+training_imagery_subdir;
      sysfunc::unix_command(unix_cmd);

      banner="Exported montage of candidate seed & seed images to "
         +training_imagery_subdir;
      outputfunc::write_big_banner(banner);


   
      double global_mu_threshold=45;
      double local_mu_threshold=15;
//      double global_sigma_threshold=10;
//      double global_sigma_threshold=6;
      double global_sigma_threshold=5;
//      cout << "Enter global_sigma_threshold:" << endl;
//      cin >> global_sigma_threshold;
//      double local_sigma_threshold=5;
//      double local_sigma_threshold=3;
      double local_sigma_threshold=2;
//      cout << "Enter local_sigma_threshold:" << endl;
//      cin >> local_sigma_threshold;

// Downsize original imported image to smaller size to speed up flood
// filling:

      int max_xdim=640;
      int max_ydim=480;

      string downsized_image_filename=
         resized_imagery_subdir+filefunc::getbasename(png_image_filename);
      videofunc::downsize_image(
         png_image_filename,max_xdim,max_ydim,downsized_image_filename);

      filefunc::deletefile(png_image_filename);

      unsigned int downsized_width,downsized_height;
      imagefunc::get_image_width_height(
         downsized_image_filename, downsized_width, downsized_height);

      texture_rectangle* texture_rectangle_ptr=
         new texture_rectangle();
      texture_rectangle* filtered_texture_rectangle_ptr=
         new texture_rectangle();
      texture_rectangle_ptr->import_photo_from_file(downsized_image_filename);
      filtered_texture_rectangle_ptr->import_photo_from_file(
         downsized_image_filename);


      bool include_alpha_channel_flag=false;
      RGBA_array curr_RGBA_array=texture_rectangle_ptr->get_RGBA_twoDarrays(
         include_alpha_channel_flag);

// Experiment with median filtering to reduce high-spatial frequency
// fluctuations in water regions due to solar glints:

      int nsize=3;
//   int n_median_filtering_iters=1;
      unsigned int n_median_filtering_iters=3;

      for (unsigned int mfi=0; mfi<n_median_filtering_iters; mfi++)
      {
         cout << "Median filtering iteration " << mfi << endl;
         imagefunc::median_filter(nsize,curr_RGBA_array.first);
         imagefunc::median_filter(nsize,curr_RGBA_array.second);
         imagefunc::median_filter(nsize,curr_RGBA_array.third);
      }
      texture_rectangle_ptr->set_from_RGB_twoDarrays(curr_RGBA_array);

      string filtered_image_filename="filtered_image.jpg";
      texture_rectangle_ptr->write_curr_frame(filtered_image_filename);
      banner="Exported filtered image to "+filtered_image_filename;
      outputfunc::write_big_banner(banner);

// Store median filtered RGB values within *filtered_texture_rectangle_ptr:

      filtered_texture_rectangle_ptr->set_from_RGB_twoDarrays(
         curr_RGBA_array);

      twoDarray* segmentation_mask_twoDarray_ptr=new twoDarray(
         downsized_width,downsized_height);
      twoDarray* mu_R_twoDarray_ptr=
         new twoDarray(downsized_width,downsized_height);
      twoDarray* mu_G_twoDarray_ptr=
         new twoDarray(downsized_width,downsized_height);
      twoDarray* mu_B_twoDarray_ptr=
         new twoDarray(downsized_width,downsized_height);
      twoDarray* sigma_R_twoDarray_ptr=
         new twoDarray(downsized_width,downsized_height);
      twoDarray* sigma_G_twoDarray_ptr=
         new twoDarray(downsized_width,downsized_height);
      twoDarray* sigma_B_twoDarray_ptr=
         new twoDarray(downsized_width,downsized_height);

      segmentation_mask_twoDarray_ptr->clear_values();
      mu_R_twoDarray_ptr->initialize_values(NEGATIVEINFINITY);
      mu_G_twoDarray_ptr->initialize_values(NEGATIVEINFINITY);
      mu_B_twoDarray_ptr->initialize_values(NEGATIVEINFINITY);
      sigma_R_twoDarray_ptr->initialize_values(NEGATIVEINFINITY);
      sigma_G_twoDarray_ptr->initialize_values(NEGATIVEINFINITY);
      sigma_B_twoDarray_ptr->initialize_values(NEGATIVEINFINITY);

      int flood_R=255;
      int flood_G=0;
      int flood_B=255;
      for (unsigned int seed=0; seed < seed_UVs.size(); seed++)
      {
    	 unsigned int pu,pv;
         texture_rectangle_ptr->get_pixel_coords(
            seed_UVs[seed].get(0),seed_UVs[seed].get(1),pu,pv);
         cout << "seed: pu = " << pu << " pv = " << pv << endl;

// For each new seed, reset *texture_rectangle_ptr to median filtered
// RGB values:

         texture_rectangle_ptr->set_from_RGB_twoDarrays(curr_RGBA_array);

// Verify seed pixel RGB doesn't equal flood RGB to avoid entering
// into infinite loop:

         int init_R,init_G,init_B;
         texture_rectangle_ptr->get_pixel_RGB_values(
            pu,pv,init_R,init_G,init_B);
         if (init_R == flood_R && init_G == flood_G && init_B == flood_B) 
            continue;

         int bbox_size=7;
         texture_rectangle_ptr->floodFill(
            filtered_texture_rectangle_ptr,
            segmentation_mask_twoDarray_ptr,
            mu_R_twoDarray_ptr,mu_G_twoDarray_ptr,mu_B_twoDarray_ptr,
            sigma_R_twoDarray_ptr,sigma_G_twoDarray_ptr,sigma_B_twoDarray_ptr,
            pu,pv,flood_R,flood_G,flood_B,
            bbox_size,local_mu_threshold,global_mu_threshold,
            local_sigma_threshold,global_sigma_threshold);

         int n_segmented_pixels=imagefunc::count_pixels_above_zmin(
            0.5,segmentation_mask_twoDarray_ptr);
         double cum_filled_frac=double(n_segmented_pixels)/
            (downsized_width*downsized_height);
         cout << "Cum filled pixels = " << n_segmented_pixels 
              << " Cum filled fraction = " << cum_filled_frac << endl;

      } // loop over seed index

// Perform 2D recursive emptying and filling in order to eliminate
// small islands within binary mask:

      cout << "Recursively emptying black islands surrounded by white oceans:" 
           << endl;

      int n_recursion_iters=25;
      binaryimagefunc::binary_reverse(segmentation_mask_twoDarray_ptr);
      recursivefunc::recursive_empty(
         n_recursion_iters,segmentation_mask_twoDarray_ptr,false);
      binaryimagefunc::binary_reverse(segmentation_mask_twoDarray_ptr);

// Recolor flooded pixels union within *texture_rectangle_ptr:

      texture_rectangle_ptr->import_photo_from_file(downsized_image_filename);

      int R,G,B;
      for (unsigned int px=0; px<downsized_width; px++)
      {
         for (unsigned int py=0; py<downsized_height; py++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
            if (segmentation_mask_twoDarray_ptr->get(px,py) > 0.5)
            {
               texture_rectangle_ptr->set_pixel_RGB_values(
                  px,py,flood_R,flood_G,flood_B);
            }
         }
      }

// Mark seed locations with colored circles:

      int pixel_radius=4;
      for (unsigned int seed=0; seed < seed_UVs.size(); seed++)
      {
 	 unsigned int pu,pv;
         texture_rectangle_ptr->get_pixel_coords(
            seed_UVs[seed].get(0),seed_UVs[seed].get(1),pu,pv);
         texture_rectangle_ptr->fill_circle(
            pu,pv,pixel_radius,colorfunc::darkpurple);
      }
   
      string flooded_image_filename="flooded_image.jpg";
      texture_rectangle_ptr->write_curr_frame(flooded_image_filename);
      banner="Exported flood-filled image to "+flooded_image_filename;
      outputfunc::write_big_banner(banner);

// Export montage displaying raw vs segmented images side-by-side:

/*
      texture_rectangle_ptr->import_photo_from_file(downsized_image_filename);
      for (unsigned int c=0; c<water_contours.size(); c++)
      {
	videofunc::display_polygon(
	  water_contours[c],texture_rectangle_ptr,
	  colorfunc::purple,line_thickness);
      }
      texture_rectangle_ptr->write_curr_frame(
*/


      blank_width=15;
      texture_rectangle_ptr->generate_blank_image_file(
         blank_width,downsized_height,separator_filename,1.0);
//      unix_cmd="montageview "+downsized_image_filename+" "+
      unix_cmd="montageview "+annotated_image_filename+" "+
         separator_filename+" "+flooded_image_filename+" NO_DISPLAY";
      sysfunc::unix_command(unix_cmd);
      unix_cmd="mv montage*.jpg "+training_imagery_subdir;
      sysfunc::unix_command(unix_cmd);

      filefunc::deletefile(downsized_image_filename);
      banner="Exported montage of raw and flooded images to "+
         training_imagery_subdir;
      outputfunc::write_big_banner(banner);

      delete texture_rectangle_ptr;
      delete filtered_texture_rectangle_ptr;
      delete segmentation_mask_twoDarray_ptr;
      delete mu_R_twoDarray_ptr;
      delete mu_G_twoDarray_ptr;
      delete mu_B_twoDarray_ptr;
      delete sigma_R_twoDarray_ptr;
      delete sigma_G_twoDarray_ptr;
      delete sigma_B_twoDarray_ptr;

   } // loop over index i labeling input images & masks

   delete image_texture_rectangle_ptr;
//    delete coarse_mask_texture_rectangle_ptr;

   cout << "At end of program COARSE2FILLED" << endl;
   outputfunc::print_elapsed_time();
} 

