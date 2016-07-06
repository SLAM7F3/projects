// ==========================================================================
// Program SEGMENT_VIDEO_FRAMES imports video frames from a specified
// subdirectory.  After downsizing each input frame, it iteratively
// picks previously unselected seed pixel
// locations. SEGMENT_VIDEO_FRAMES performs "color oozing" at each
// seed location and generates a pixel flood region.  Different flood
// regions whose median RGB colors are sufficiently similar are
// assigned to the same equivalence class via a union_find
// datastructure.  Distinct flood regions are colored and exported to
// an segment image file.

// SEGMENT_VIDEO_FRAMES also generates cumulative distribution
// functions for the zeroth ("mass") and second ("moment of
// inertia") moments of all nontrivially-sized parent regions.  The
// cumulative distribution functions are exported metafile
// subdirectories of the segmentation subdir.


// 			 ./segment_video_frames

// ==========================================================================
// Last updated on 11/17/13; 11/18/13; 11/19/13; 6/7/14
// ==========================================================================

#include  <iostream>
#include  <map>
#include  <set>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "color/colorfuncs.h"
#include "math/ltduple.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "datastructures/Triple.h"
#include "image/TwoDarray.h"
#include "datastructures/union_find.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   timefunc::initialize_timeofday_clock();      

   cout.precision(12);
   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   string liberalized_color="";
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table(liberalized_color);

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* flooded_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* composite_flooded_texture_rectangle_ptr=
      new texture_rectangle();
//   texture_rectangle* quantized_texture_rectangle_ptr=new texture_rectangle();
//   texture_rectangle* averaged_quantized_texture_rectangle_ptr=
//      new texture_rectangle();
//   texture_rectangle* median_quantized_texture_rectangle_ptr=
//      new texture_rectangle();

   string ImageEngine_subdir="/data/ImageEngine/";
//   string root_subdir=ImageEngine_subdir+"NewsWrap/";
//   string root_subdir=ImageEngine_subdir+
//      "BostonBombing/Nightline_YouTube2/transcripted/";
//   string root_subdir=
//      "/data/ImageEngine/BostonBombing/clips_1_thru_133/clip34/";

//   string images_subdir="/data/ImageEngine/BostonBombing/clips_1_thru_133/";
//   string root_subdir=ImageEngine_subdir+"NewsWrap/";
//   string root_subdir=ImageEngine_subdir+"BostonBombing/clip3/";
//   string root_subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/korea/NK/ground_videos/NorthKorea/";
//   string images_subdir=root_subdir;
//   vector<string> image_filenames=filefunc::image_files_in_subdir(
//      images_subdir);

//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";
   string root_subdir=JAV_subdir;
   string images_subdir=root_subdir+"jpg_frames/";

   string segmentation_subdir=root_subdir+"segmentation/";
   string segmentation_metafiles_subdir=segmentation_subdir+"metafiles/";
   string segmentation_metafiles_frac_area_subdir=
      segmentation_metafiles_subdir+"frac_area/";
   string segmentation_metafiles_Iuu_subdir=
      segmentation_metafiles_subdir+"Iuu/";
   string segmentation_metafiles_Iuv_subdir=
      segmentation_metafiles_subdir+"Iuv/";
   string segmentation_metafiles_Ivv_subdir=
      segmentation_metafiles_subdir+"Ivv/";

   filefunc::dircreate(segmentation_subdir);
   filefunc::dircreate(segmentation_metafiles_subdir);
   filefunc::dircreate(segmentation_metafiles_frac_area_subdir);
   filefunc::dircreate(segmentation_metafiles_Iuu_subdir);
   filefunc::dircreate(segmentation_metafiles_Iuv_subdir);
   filefunc::dircreate(segmentation_metafiles_Ivv_subdir);
   
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      images_subdir);

/*
   string images_subdir=ImageEngine_subdir+"/tidmarsh/";
   string root_subdir=images_subdir;
   string thumbnails_subdir=images_subdir+"thumbnails/";
   vector<string> image_filenames=filefunc::image_files_in_subdir(
      thumbnails_subdir);
*/

//   int i_start=0;
   int i_start=1199;
   int i_stop=image_filenames.size();
   int i_skip=1;
//   int i_skip=3;
//   int i_skip=100;

   int max_xdim=250;
   int max_ydim=250;
//   int max_xdim=300;
//   int max_ydim=300;
//   int max_xdim=400;
//   int max_ydim=400;

   ofstream outstream;
   for (int i=i_start; i<i_stop; i += i_skip)
   {
      if (i%10==0)
      {
         double progress_frac=double(i-i_start)/double(i_stop-i_start);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }
      
      string image_filename=image_filenames[i];
      string image_prefix=filefunc::getprefix(image_filename);

// Subsample input image:

      string downsized_image_filename="downsized.jpg";
      unsigned int new_xdim,new_ydim;
      videofunc::downsize_image(
         image_filename,max_xdim,max_ydim,
         downsized_image_filename,new_xdim,new_ydim);
//      cout << "new_xdim = " << new_xdim
//           << " new_ydim = " << new_ydim << endl;

      texture_rectangle_ptr->reset_texture_content(
         downsized_image_filename);
      flooded_texture_rectangle_ptr->reset_texture_content(
         downsized_image_filename);
      composite_flooded_texture_rectangle_ptr->reset_texture_content(
         downsized_image_filename);
      composite_flooded_texture_rectangle_ptr->clear_all_RGB_values();

//      quantized_texture_rectangle_ptr->reset_texture_content(
//         downsized_image_filename);
//      averaged_quantized_texture_rectangle_ptr->reset_texture_content(
//         downsized_image_filename);
//      median_quantized_texture_rectangle_ptr->reset_texture_content(
//         downsized_image_filename);

// ------------------------------------------------------------------------
// STL MAP declarations

      typedef map< DUPLE, vector<int> > PIXEL_FLOOD_REGIONS_MAP;

// independent DUPLE holds pixel (px,py) coordinates      
// dependent STL vector holds flood region IDs

      PIXEL_FLOOD_REGIONS_MAP pixel_flood_regions_map;
      PIXEL_FLOOD_REGIONS_MAP::iterator pixel_flood_regions_iter;

      typedef map< int, Triple<int,threevector,threevector> > 
         FLOOD_REGIONS_MAP;
      
// independent int = flood region ID
// dependent Triple holds region number of pixels, median RGB
// threevector and quartile width RGB threevector

      FLOOD_REGIONS_MAP flood_regions_map;
      FLOOD_REGIONS_MAP::iterator flood_regions_iter;

      typedef map<int, threevector> PARENT_REGIONS_RGB_MAP;

// independent int = parent region ID
// dependent threevector = parent region RGB      

      PARENT_REGIONS_RGB_MAP parent_regions_RGB_map;
      PARENT_REGIONS_RGB_MAP::iterator parent_regions_iter;

      typedef map<int, Triple<double,double,double> > 
         PARENT_REGION_PROPERTIES_MAP;
      
// independent int = parent region ID
// dependent Triple holds parent region fractional area and two
// eigenvalues of moment-of-inertia matrix

      PARENT_REGION_PROPERTIES_MAP parent_region_properties_map;
      PARENT_REGION_PROPERTIES_MAP::iterator parent_region_properties_iter;

      typedef map<int, vector<DUPLE> > PARENT_REGION_PIXELS_MAP;
      
// independent int = parent region ID
// dependent STL vector holds pixel coordinates for parent region

      PARENT_REGION_PIXELS_MAP parent_region_pixels_map;
      PARENT_REGION_PIXELS_MAP::iterator parent_region_pixels_iter;

// ------------------------------------------------------------------------
// Generate flood regions from starting lattice points superposed over
// downsized image:

      int flood_R=255;
      int flood_G=0;
      int flood_B=255;

      int flood_region_ID=0;
      string flooded_images_subdir="./flooded_images/";
      filefunc::dircreate(flooded_images_subdir);

      int delta_p=1;
//      int delta_p=2;
      for (unsigned int flood_pu=0; flood_pu<new_xdim-1; flood_pu += delta_p)
      {
         for (unsigned int flood_pv=0; flood_pv<new_ydim; flood_pv += delta_p)
         {
            int composite_R,composite_G,composite_B;
            composite_flooded_texture_rectangle_ptr->get_pixel_RGB_values(
               flood_pu,flood_pv,composite_R,composite_G,composite_B);
            if (composite_R==flood_R && composite_G==flood_G &&
                composite_B==flood_B) continue;

            flooded_texture_rectangle_ptr->reset_texture_content(
               downsized_image_filename);

            int init_R,init_G,init_B;
            flooded_texture_rectangle_ptr->get_pixel_RGB_values(
               flood_pu,flood_pv,init_R,init_G,init_B);

//            double local_threshold=10;
//            double global_threshold=15;

//            double local_threshold=10;
//            double global_threshold=50;

            double local_threshold=12;
            double global_threshold=50;

            vector<pair<int,int> > filled_pixels;
            vector<threevector> encountered_RGBs;

            flooded_texture_rectangle_ptr->floodFill(
               flood_pu,flood_pv,flood_R,flood_G,flood_B,
               init_R,init_G,init_B,
               local_threshold,global_threshold,
               filled_pixels,encountered_RGBs);

// Append region ID for all pixels within current flood region in
// pixel_flood_regions_map:

            int n_filled_pixels=filled_pixels.size();
            for (int p=0; p<n_filled_pixels; p++)
            {
               pixel_flood_regions_iter=pixel_flood_regions_map.find(
                  filled_pixels[p]);
               if (pixel_flood_regions_iter==pixel_flood_regions_map.end())
               {
                  vector<int> V;
                  V.push_back(flood_region_ID);
                  pixel_flood_regions_map[filled_pixels[p]]=V;
               }
               else
               {
                  pixel_flood_regions_iter->second.push_back(flood_region_ID);
               }

               int px=filled_pixels[p].first;
               int py=filled_pixels[p].second;
               composite_flooded_texture_rectangle_ptr->set_pixel_RGB_values(
                  px,py,flood_R,flood_G,flood_B);

            } // loop over index p labeling flooded pixels

// Store median and quartile width RGB threevectors for current flood
// region within STL map:

            threevector median_RGB,quartile_width_RGB;
            mathfunc::median_value_and_quartile_width(
               encountered_RGBs,median_RGB,quartile_width_RGB);

            Triple<int,threevector,threevector> T(
               n_filled_pixels,median_RGB,quartile_width_RGB);
            flood_regions_map[flood_region_ID]=T;

// Increment flood region ID:

            flood_region_ID++;

/*
            string flooded_image_filename=flooded_images_subdir+"flooded_"+
               stringfunc::integer_to_string(flood_pu,2)+"_"+
               stringfunc::integer_to_string(flood_pv,2)+".jpg";
            composite_flooded_texture_rectangle_ptr->write_curr_frame(
               flooded_image_filename);
*/

         } // loop over index flood_pv
      } // loop over index flood_pu

//      cout << "pixel_flood_regions_map.size() = "
//           << pixel_flood_regions_map.size() << endl;
//      cout << "new_xdim*new_ydim = " << new_xdim*new_ydim << endl;

/*
      string composite_flooded_image_filename="composite_flooded.jpg";
      composite_flooded_texture_rectangle_ptr->write_curr_frame(
         composite_flooded_image_filename);
*/

// Instantiate union_find object to hold flood regions which belong to
// the same equivalence class:

      union_find* union_find_ptr=new union_find();
      for (pixel_flood_regions_iter=pixel_flood_regions_map.begin();
           pixel_flood_regions_iter != pixel_flood_regions_map.end(); 
           pixel_flood_regions_iter++)
      {
         vector<int> flood_region_IDs=pixel_flood_regions_iter->second;
         for (unsigned int f=0; f<flood_region_IDs.size(); f++)
         {
            int curr_region_ID=flood_region_IDs[f];

            flood_regions_iter=flood_regions_map.find(curr_region_ID);
            threevector median_RGB=flood_regions_iter->second.second;
            threevector quartile_width_RGB=flood_regions_iter->second.third;

            int parent_ID=union_find_ptr->Find(curr_region_ID);
            if (parent_ID==-1)
            {
               union_find_ptr->MakeSet(curr_region_ID);
            }

            flood_regions_iter=flood_regions_map.find(curr_region_ID);
            threevector curr_median_RGB=flood_regions_iter->second.second;
            threevector curr_quartile_width_RGB=flood_regions_iter->
               second.third;

            const double max_child_region_separation=80;
//            const double max_child_region_separation=100;
            for (unsigned int k=0; k<f; k++)
            {
               int prev_region_ID=flood_region_IDs[k];
               flood_regions_iter=flood_regions_map.find(prev_region_ID);
               threevector prev_median_RGB=flood_regions_iter->second.second;
               threevector prev_quartile_width_RGB=flood_regions_iter->
                  second.third;
               
               threevector delta_RGB=curr_median_RGB-prev_median_RGB;
               if (delta_RGB.magnitude() > max_child_region_separation)
                  continue;

               union_find_ptr->Link(curr_region_ID,prev_region_ID);
            } // loop over index k labeling "previous" flooded regions

//            cout << "f = " << f 
//		   << " curr_region_ID = " << curr_region_ID
//                 << " parent_ID = " << parent_ID << endl;
         } // loop over index f labeling flooded regions

      } // loop over pixel_flood_regions_iter
//      cout << "union_find_ptr->get_n_nodes() = "
//           << union_find_ptr->get_n_nodes() << endl;

// Determine unique "children node" IDs corresponding to parent region
// IDs.  Then set parent region color equal to the weighted average of
// its children region RGB colors.

      union_find_ptr->fill_parent_nodes_map();
      int parent_ID=union_find_ptr->reset_parent_node_iterator();
      while (parent_ID >= 0)
      {
//         cout << "Parent ID = " << parent_ID << endl;
         vector<int> children_node_IDs=union_find_ptr->
            get_children_nodes_corresponding_to_parent(parent_ID);
//         cout << "   Children IDs: " << flush;
         
         double n_parent_region_pixels=0;
         threevector parent_region_RGB(0,0,0);
         for (unsigned int c=0; c<children_node_IDs.size(); c++)
         {
//            cout << children_node_IDs[c] << " " << flush;

            flood_regions_iter=flood_regions_map.find(children_node_IDs[c]);
            if (flood_regions_iter==flood_regions_map.end()) continue;
            int n_pixels=flood_regions_iter->second.first;
            threevector median_RGB=flood_regions_iter->second.second;
            threevector quartile_width_RGB=flood_regions_iter->second.third;
            parent_region_RGB += n_pixels*median_RGB;
            n_parent_region_pixels += n_pixels;
         }
//         cout << endl;

         parent_region_RGB /= n_parent_region_pixels;
         parent_regions_RGB_map[parent_ID]=parent_region_RGB;
         
         parent_ID=union_find_ptr->increment_parent_node_iterator();
      } //  while parent_ID >= 0 loop
      
      flooded_texture_rectangle_ptr->clear_all_RGB_values();

      for (unsigned int py=0; py<new_ydim; py++)
      {
         for (unsigned int px=0; px<new_xdim; px++)
         {
            DUPLE curr_duple(px,py);

            pixel_flood_regions_iter=pixel_flood_regions_map.find(curr_duple);
            if (pixel_flood_regions_iter==pixel_flood_regions_map.end()) 
               continue;
            int region_ID=pixel_flood_regions_iter->second.back();
            int parent_ID=union_find_ptr->Find(region_ID);            

// Add curr_duple to parent_region_pixels_map:

            parent_region_pixels_iter=parent_region_pixels_map.find(parent_ID);
            if (parent_region_pixels_iter==parent_region_pixels_map.end())
            {
               vector<DUPLE> V;
               V.push_back(curr_duple);
               parent_region_pixels_map[parent_ID]=V;
            }
            else
            {
               parent_region_pixels_iter->second.push_back(curr_duple);
            }

// Store number of pixels, u and v integrals associated with parent_ID
// in parent_region_properties_map:

            double u,v;
            texture_rectangle_ptr->get_uv_coords(px,py,u,v);

            parent_region_properties_iter=parent_region_properties_map.find(
               parent_ID);
            if (parent_region_properties_iter==
                parent_region_properties_map.end())
            {
               Triple<double,double,double> T;
               T.first=1;
               T.second=u;
               T.third=v;
               parent_region_properties_map[parent_ID]=T;
            }
            else
            {
               parent_region_properties_iter->second.first=
                  parent_region_properties_iter->second.first+1;
               parent_region_properties_iter->second.second=
                  parent_region_properties_iter->second.second+u;
               parent_region_properties_iter->second.third=
                  parent_region_properties_iter->second.third+v;
            }
            
            parent_regions_iter=parent_regions_RGB_map.find(parent_ID);
            threevector parent_region_RGB=parent_regions_iter->second;
            flooded_texture_rectangle_ptr->set_pixel_RGB_values(
               px,py,parent_region_RGB.get(0),
               parent_region_RGB.get(1),parent_region_RGB.get(2));
         } // loop over px index
      } // loop over py index

//      cout << "parent_region_properties_map.size() = "
//           << parent_region_properties_map.size() << endl;

// Compute fractional areas and moment-of-inertial eigenvalues for 
// nontrivially-sized parent regions:

//      const double min_region_frac_area=1E-6;
//      const double min_region_frac_area=0.001;
      const double min_region_frac_area=0.01;
//      const double min_region_frac_area=0.25;

      vector<double> parent_region_frac_areas,
         parent_region_Iuu,parent_region_Iuv,parent_region_Ivv,
         parent_region_MoI_evalue_plus,parent_region_MoI_evalue_minus,
         parent_region_thetas;
      for (parent_region_properties_iter=parent_region_properties_map.begin();
           parent_region_properties_iter != parent_region_properties_map.end();
           parent_region_properties_iter++)
      {
         int parent_ID=parent_region_properties_iter->first;
         double curr_region_area=
            parent_region_properties_iter->second.first;
         double curr_region_frac_area=curr_region_area/(new_xdim*new_ydim);

// Ignore any parent region whose fractional area is too small:

         if (curr_region_frac_area < min_region_frac_area) 
         {
            parent_region_properties_iter->second.first=-1;
            continue;
         }

         parent_region_frac_areas.push_back(curr_region_frac_area);
         
         parent_regions_iter=parent_regions_RGB_map.find(parent_ID);
         threevector parent_region_RGB=parent_regions_iter->second;

//         cout << "region_frac_area = " << curr_region_frac_area << endl;
//         cout << "parent_region_RGB = " << parent_region_RGB << endl;

// Compute dimensionless 2D COM for current parent region:

         double U_COM=parent_region_properties_iter->second.second/
            curr_region_area;
         double V_COM=parent_region_properties_iter->second.third/
            curr_region_area;
         
         parent_region_pixels_iter=parent_region_pixels_map.find(
            parent_ID);
         vector<DUPLE> pixel_duples=parent_region_pixels_iter->second;

// Compute 2D moment-of-inertia matrix:

         int n_pixels=pixel_duples.size();
         double Iuu,Iuv,Ivv;
         Iuu=Iuv=Ivv=0;
         for (int p=0; p<n_pixels; p++)
         {
            double u,v;
            int px=pixel_duples[p].first;
            int py=pixel_duples[p].second;
            texture_rectangle_ptr->get_uv_coords(px,py,u,v);
            
            Iuu += sqr(u-U_COM);
            Iuv += (u-U_COM)*(v-V_COM);
            Ivv += sqr(v-V_COM);
         } // loop over index p labeling pixels associated with parent region

         Iuu /= n_pixels;
         Iuv /= n_pixels;
         Ivv /= n_pixels;

         parent_region_Iuu.push_back(Iuu);
         parent_region_Iuv.push_back(Iuv);
         parent_region_Ivv.push_back(Ivv);
         
/*
         double Trace=Iuu+Ivv;
         double Det=Iuu*Ivv-sqr(Iuv);
         double discrim=sqr(Trace)-4*Det;
         
         double lambda_plus=0.5*(Trace+sqrt(discrim));
         double lambda_minus=0.5*(Trace-sqrt(discrim));
         parent_region_MoI_evalue_plus.push_back(lambda_plus);
         parent_region_MoI_evalue_minus.push_back(lambda_minus);

         double argument=2*Iuv/(lambda_plus-lambda_minus);
         if (fabs(argument) > 1)
         {
            cout << "lambda_plus = " << lambda_plus 
                 << " lambda_minus = " << lambda_minus 
                 << " argument = " << argument << endl;
         }
         else
         {
            double theta=0.5*asin(2*Iuv/(lambda_plus-lambda_minus));
            theta *= 180/PI;
            theta=basic_math::phase_to_canonical_interval(theta,0,360);
            parent_region_thetas.push_back(theta);
         }
*/

      } // parent_region_properties_iter loop

      delete union_find_ptr;

// Generate montage illustrating original and segmented video frames
// side-by-side.  Then move montage into segmented_subdir:

      string flooded_image_filename="./flooded_regions.jpg";
      flooded_texture_rectangle_ptr->write_curr_frame(
         flooded_image_filename);

      string unix_cmd="montageview "+downsized_image_filename+" "+
         flooded_image_filename+" NO_DISPLAY";
      sysfunc::unix_command(unix_cmd);

      string substring="montage_";
      vector<string> montage_filenames=
         filefunc::files_in_subdir_matching_substring("./",substring);
      string montage_jpg_filename=montage_filenames.back();
   
      string segmented_filename=segmentation_subdir+"segmented_"+
         image_prefix+".jpg";
      unix_cmd="mv "+montage_jpg_filename+" "+segmented_filename;
      sysfunc::unix_command(unix_cmd);
      string banner="Exported "+segmented_filename;
      outputfunc::write_banner(banner);
      
// Compute distribution for parent region fractional areas and MoI
// eigenvalues:

      cout << "parent_region_frac_areas.size() = "
           << parent_region_frac_areas.size() << endl;
      if (parent_region_frac_areas.size()==0) continue;

      prob_distribution prob_parent_region_frac_area(
         600, 0, 0.6, parent_region_frac_areas);
//      prob_parent_region_frac_area.set_densityfilenamestr(
//         "parent_region_frac_areas_dens.meta");
      prob_parent_region_frac_area.set_cumulativefilenamestr(
         segmentation_metafiles_frac_area_subdir+
         image_prefix+"_cum.meta");
      prob_parent_region_frac_area.write_cumulative_dist(false,false);

      prob_distribution prob_parent_region_Iuu(
         500, 0, 0.5, parent_region_Iuu);
//      prob_parent_region_Iuu.set_densityfilenamestr(
//         "parent_region_Iuu_dens.meta");
      prob_parent_region_Iuu.set_cumulativefilenamestr(
         segmentation_metafiles_Iuu_subdir+
         image_prefix+"_cum.meta");
      prob_parent_region_Iuu.write_cumulative_dist(false,false);

      prob_distribution prob_parent_region_Ivv(
         500, 0, 0.5, parent_region_Ivv);
//      prob_parent_region_Ivv.set_densityfilenamestr(
//         "parent_region_Ivv_dens.meta");
      prob_parent_region_Ivv.set_cumulativefilenamestr(
         segmentation_metafiles_Ivv_subdir+
         image_prefix+"_cum.meta");
      prob_parent_region_Ivv.write_cumulative_dist(false,false);

      prob_distribution prob_parent_region_Iuv(
         500, -0.25, 0.25, parent_region_Iuv);
//      prob_parent_region_Iuv.set_densityfilenamestr(
//         "parent_region_Iuv_dens.meta");
      prob_parent_region_Iuv.set_cumulativefilenamestr(
         segmentation_metafiles_Iuv_subdir+
         image_prefix+"_cum.meta");
      prob_parent_region_Iuv.write_cumulative_dist(false,false);


/*
      prob_distribution prob_parent_region_lambda_plus(
         parent_region_MoI_evalue_plus,1000,0);
      prob_parent_region_lambda_plus.set_densityfilenamestr(
         "parent_region_lambda_plus_dens.meta");
      prob_parent_region_lambda_plus.set_cumulativefilenamestr(
         "parent_region_lambda_plus_cum.meta");
      prob_parent_region_lambda_plus.writeprobdists(false);

      prob_distribution prob_parent_region_lambda_minus(
         parent_region_MoI_evalue_minus,1000,0);
      prob_parent_region_lambda_minus.set_densityfilenamestr(
         "parent_region_lambda_minus_dens.meta");
      prob_parent_region_lambda_minus.set_cumulativefilenamestr(
         "parent_region_lambda_minus_cum.meta");
      prob_parent_region_lambda_minus.writeprobdists(false);

      prob_distribution prob_parent_region_theta(parent_region_thetas,360,0);
      prob_parent_region_theta.set_densityfilenamestr(
         "parent_region_theta_dens.meta");
      prob_parent_region_theta.set_cumulativefilenamestr(
         "parent_region_theta_cum.meta");
      prob_parent_region_theta.writeprobdists(false);
*/

/*
      RGB_analyzer_ptr->quantize_texture_rectangle_colors(
         quantized_texture_rectangle_ptr);

      string quantized_image_filename="quantized_colors.jpg";
      quantized_texture_rectangle_ptr->write_curr_frame(
         quantized_image_filename);

//      int n_iters=2;
      int n_iters=6;
//      int n_iters=10;
      int nsize=3;       
      for (int iter=0; iter<n_iters; iter++)
      {
//         RGB_analyzer_ptr->average_texture_rectangle_colors(
//            nsize,quantized_texture_rectangle_ptr,
//            averaged_quantized_texture_rectangle_ptr);
//         quantized_texture_rectangle_ptr->copy_RGB_values(
//            averaged_quantized_texture_rectangle_ptr);
         RGB_analyzer_ptr->median_texture_rectangle_colors(
            nsize,quantized_texture_rectangle_ptr,
            median_quantized_texture_rectangle_ptr);
         quantized_texture_rectangle_ptr->copy_RGB_values(
            median_quantized_texture_rectangle_ptr);
      } // loop over iteration index
      
//      string averaged_quantized_image_filename="averaged_quantized_colors.jpg//";
//      averaged_quantized_texture_rectangle_ptr->write_curr_frame(
//         averaged_quantized_image_filename);

      string median_quantized_image_filename="median_quantized_colors.jpg";
      median_quantized_texture_rectangle_ptr->write_curr_frame(
         median_quantized_image_filename);
*/

   } // loop over index i labeling image filenames

   delete RGB_analyzer_ptr;
   delete texture_rectangle_ptr;
   delete flooded_texture_rectangle_ptr;
   delete composite_flooded_texture_rectangle_ptr;

//   delete quantized_texture_rectangle_ptr;
//   delete averaged_quantized_texture_rectangle_ptr;
//   delete median_quantized_texture_rectangle_ptr;

   cout << "At end of program SEGMENT_VIDEO_FRAMES" << endl;
   outputfunc::print_elapsed_time();
}
