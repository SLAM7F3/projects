// ========================================================================
// Program TEMPORALLY_FILTER_MASKS reads in raw mask filenames from
// the photos table in the plume database.  Each raw mask is assumed
// to contain the substring "_bksub" in its file name. For each time
// slice, TEMPORALLY_FILTER_MASKS finds n_timesteps=0 (for montage AVI
// movie generation) and n_timesteps=2 masks earlier and later in
// time.  This program uses gaussian weights to generate a temporally
// averaged mask.  Such temporal smoothing reduces some smoke
// classification errors.

// Both greyscale and colored versions of the n_timesteps=2 temporally
// averaged masks are exported as JPG files to the same subdirectories
// containing the original raw masks.  Only colored versions of the
// n_timesteps=0 (i.e. not-temporally averaged) versions of the raw
// masks are exported as JPG files for AVI movie generation purposes
// in program GENERATE_MONTAGE_AVIS.

//    ./temporally_filter_masks --GIS_layer ./packages/plume_metadata.pkg 

// ========================================================================
// Last updated on 1/11/13; 1/25/13; 1/26/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osgDB/FileUtils>

#include "osg/osgGraphicals/AnimationController.h"
#include "image/arrayfuncs.h"
#include "video/camera.h"
#include "osg/osgOrganization/Decorations.h"
#include "image/imagefuncs.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "passes/PassesGroup.h"
#include "postgres/plumedatabasefuncs.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "image/raster_parser.h"
#include "osg/osg3D/tdpfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

#include "general/outputfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int mission_ID=22;
//   cout << "Enter mission ID:" << endl;
//   cin >> mission_ID;

   int fieldtest_ID=plumedatabasefunc::retrieve_fieldtest_ID_given_mission_ID(
      postgis_db_ptr,mission_ID);
//   cout << "fieldtest_ID = " << fieldtest_ID << endl;

   int day_number;
   string experiment_label;
   plumedatabasefunc::retrieve_mission_metadata_from_database(
      postgis_db_ptr,fieldtest_ID,mission_ID,
      day_number,experiment_label);

   string start_timestamp;
   plumedatabasefunc::retrieve_fieldtest_metadata_from_database(
      postgis_db_ptr,fieldtest_ID,start_timestamp);
//   cout << "start_timestamp = " << start_timestamp << endl;
   Clock clock;
   cout.precision(13);

   bool UTC_flag=true;
   double epoch=
      clock.timestamp_string_to_elapsed_secs(start_timestamp,UTC_flag);
   int year=clock.get_year();
   string month_name=clock.get_month_name();


   string experiment_subdir="/data/ImageEngine/plume/";
   experiment_subdir += month_name+stringfunc::number_to_string(year)+"/Day";
   experiment_subdir += stringfunc::number_to_string(day_number)+"/"+
      experiment_label+"/";
//   cout << "experiment_subdir = " << experiment_subdir << endl;

   string bksub_subdir=experiment_subdir+"bksub/time_slices/";
   cout << "bksub_subdir = " << bksub_subdir << endl;

   bool search_all_children_dirs_flag=true;
   vector<string> bksub_filenames=filefunc::files_in_subdir_matching_substring(
      bksub_subdir,"_bksub",search_all_children_dirs_flag);
   cout << "bksub_filenames.size() = " << bksub_filenames.size() << endl;

   unsigned int mask_width,mask_height;
   if (fieldtest_ID==1)
   {
      imagefunc::get_image_width_height(
         bksub_filenames.front(),mask_width,mask_height);
   }
   else if (fieldtest_ID==2)
   {
      mask_width=1920;
      mask_height=1080;
   }

   string packages_subdir=experiment_subdir+"packages/";
   filefunc::dircreate(packages_subdir);
   string results_subdir="./plume_results/";
   results_subdir += stringfunc::number_to_string(day_number)+
      experiment_label+"/";
   filefunc::dircreate(results_subdir);
//   cout << "results_subdir = " << results_subdir << endl;

// Import slice numbers from photo table of plume database:

   vector<int> slice_numbers=
      plumedatabasefunc::retrieve_photo_slice_numbers_from_database(
         postgis_db_ptr,fieldtest_ID,mission_ID);
   int first_slice_number=slice_numbers.front();
   int last_slice_number=slice_numbers.back();

   int start_slice_number=1;
   int stop_slice_number=499;
   
   cout << "Earliest time slice number = " << first_slice_number << endl;
   cout << "Latest time slice number = " << last_slice_number << endl << endl;
   cout << "Enter starting time slice number:" << endl;
//   cin >> start_slice_number;
   cout << "Enter stopping time slice number:" << endl;
//   cin >> stop_slice_number;

   int slice_number_step=1;
//   cout << "Enter time slice number step:" << endl;
//   cin >> slice_number_step;

// ************************************************************************
// Generate TWO sets of filtered masks.  One corresponds to
// n_timesteps=0 (i.e. no temporal averaging) and the other
// corresponds to n_timesteps=2.

   int n_timesteps=-1;

// Note added on 1/26/13: Temporal filtering is already performed for
// Nov 2012 Expt 2H via program SMOKE_MASKS.  So we should not need to
// perform more temporal filtering for Expt 2H here:

   int iter_stop=2;
   if (fieldtest_ID==2) iter_stop=1;

   for (int iter=0; iter<iter_stop; iter++)
   {
      if (iter==0)
      {
         n_timesteps=0;
      }
      else
      {
         n_timesteps=2;
      }

// Construct normalized Gaussian weights:

      vector<double> weight;
      const double mu=0;
//   double sigma=0.5;	// sec
      double sigma=0.66;	// sec
//   double sigma=1;	// sec
      double weight_integral=0;
      for (int t=-n_timesteps; t<=n_timesteps; t++)
      {
         double curr_weight=mathfunc::gaussian(t,mu,sigma);
         weight.push_back(curr_weight);
         weight_integral += curr_weight;
      }
      for (int w=0; w<weight.size(); w++)
      {
         double curr_weight=weight[w]/weight_integral;
         weight[w]=curr_weight;
         cout << "w = " << w 
              << " normalized Gaussian weight = " << weight[w] << endl;
      }
//      outputfunc::enter_continue_char();

      start_slice_number=basic_math::max(n_timesteps,start_slice_number);
      raster_parser RasterParser;
      vector<twoDarray*> twoDarray_ptrs;

      twoDarray* avg_ztwoDarray_ptr=new twoDarray(mask_width,mask_height);
      twoDarray* ptwoDarray_ptr=new twoDarray(avg_ztwoDarray_ptr);

// ========================================================================
// Loop over slice number starts here
// ========================================================================

      for (int slice_number=start_slice_number; slice_number 
              <= stop_slice_number; slice_number += slice_number_step)
      {

// Retrieve time-synced info from photos table of plume database:

         vector<int> photo_ID,sensor_ID;
         vector<string> URL,thumbnail_URL,mask_URL;
         plumedatabasefunc::retrieve_photo_metadata_from_database(
            postgis_db_ptr,fieldtest_ID,mission_ID,slice_number,
            photo_ID,sensor_ID,URL,thumbnail_URL,mask_URL);
      
         int n_sensors=sensor_ID.size();

// Read mask information from photos table of plume database.
// Convert contents of imported masks into texture rectangles and
// store within STL vector mask_texture_rectangle_ptrs.  Also record
// non-zero mask intensities within an STL vector:

         for (int s=0; s<n_sensors; s++)
         {
            int curr_camera_ID=sensor_ID[s];
            cout << "n_timesteps iter=" << iter 
                 << "  slice=" << slice_number 
                 << "  camera_ID=" << curr_camera_ID << endl;

            string avg_mask_URL,colored_avg_mask_URL;
            for (int t=-n_timesteps; t<=n_timesteps; t++)
            {
               string mask_URL=
                  plumedatabasefunc::retrieve_mask_URL_from_database(
                     postgis_db_ptr,fieldtest_ID,mission_ID,slice_number+t,
                     curr_camera_ID);
//            cout << "t = " << t << " mask_URL = " << mask_URL << endl;

               twoDarray* ztwoDarray_ptr=NULL;
               if (fieldtest_ID==1)
               {
                  ztwoDarray_ptr=
                     RasterParser.read_single_channel_image(mask_URL);
               }
               else if (fieldtest_ID==2)
               {
                  string mask_filename=mask_URL;

// Bunzip2 input mask file if necessary:

                  string bz2_mask_filename=mask_URL+".bz2";
                  if (filefunc::fileexist(bz2_mask_filename))
                  {
                     filefunc::bunzip2_file(bz2_mask_filename);
                  }

                  int xdim=1920;
                  int ydim=1080;
//                  double magnitude_factor=0.1;
                  double magnitude_factor=1.0;
                  ztwoDarray_ptr=arrayfunc::parse_binary_shorts_file(
                     xdim,ydim,mask_filename,magnitude_factor);
                  filefunc::bzip2_file(mask_filename);      

// As of Jan 2013, Joe's mask values for Nov 2012 Expt 2H should
// always yield entries in *ztwoDarray_ptr which range from 0 to 3276.

                  double min_z=0;
//                  double max_z=3276;
                  double max_z=65535;
//                  double min_z=ztwoDarray_ptr->min_value();
//                  double max_z=ztwoDarray_ptr->max_value();
//                cout << "min_z = " << min_z << " max_z = " << max_z << endl;

// Renormalize ztwoDarray entries so that they (loosely) correspond to
// probabilities ranging between 0 and 1:

                  double min_p=POSITIVEINFINITY;
                  double max_p=NEGATIVEINFINITY;

                  for (int py=0; py<ydim; py++)
                  {
                     for (int px=0; px<xdim; px++)
                     {
                        double z=ztwoDarray_ptr->get(px,py);
                        double p=(z-min_z)/(max_z-min_z);
                        ztwoDarray_ptr->put(px,py,p);

                        min_p=basic_math::min(min_p,p);
                        max_p=basic_math::max(max_p,p);
                     }
                  }

//                  cout << "min_p = " << min_p << " max_p = " << max_p
//                       << endl;
               } // fieldtest_ID conditional

/*
// TEST:  Export classification mask as colored JPG:

               string mask_filename="mask.jpg";
               texture_rectangle* classification_texture_rectangle_ptr=
                  new texture_rectangle(mask_width,mask_height,1,3,NULL);
               classification_texture_rectangle_ptr->
                  initialize_RGB_twoDarray_image(ztwoDarray_ptr);
               classification_texture_rectangle_ptr->fill_twoDarray_image(
                  ztwoDarray_ptr,3);
//               classification_texture_rectangle_ptr->
//                  convert_grey_values_to_hues();
               classification_texture_rectangle_ptr->write_curr_frame(
                  mask_filename);
               delete classification_texture_rectangle_ptr;

               cout << "Exported ./mask.jpg" << endl;
               outputfunc::enter_continue_char();
*/
               twoDarray_ptrs.push_back(ztwoDarray_ptr);

               if (t==0)
               {
                  string dirname=filefunc::getdirname(mask_URL);
                  string basename=filefunc::getbasename(mask_URL);
                  avg_mask_URL=dirname+"TimeAvgd_"+
                     stringfunc::number_to_string(n_timesteps)+"_"+basename;
                  colored_avg_mask_URL=dirname+"Colored_TimeAvgd_"+
                     stringfunc::number_to_string(n_timesteps)+"_"+basename;
               }
            } // loop over index t labeling relative slice number

            avg_ztwoDarray_ptr->clear_values();
            for (int t=-n_timesteps; t<=n_timesteps; t++)
            {
               twoDarray* ztwoDarray_ptr=twoDarray_ptrs[t+n_timesteps];
//            cout << "t = " << t << " ztwoDarray_ptr = " << ztwoDarray_ptr
//                 << endl;
            
               if (ztwoDarray_ptr==NULL) continue;

               for (int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
               {
                  for (int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
                  {
                     double curr_z=ztwoDarray_ptr->get(px,py);
                     double avg_z=avg_ztwoDarray_ptr->get(px,py);
                     avg_z += weight[t+n_timesteps]*curr_z;
                     avg_ztwoDarray_ptr->put(px,py,avg_z);

/*
  if (curr_z > 0.001)
  {
  cout << "t = " << t << " px = " << px << " py = " << py
  << " curr_z = " << curr_z << " avg_z = " << avg_z
  << " weight = " << weight[t+n_timesteps] 
  << endl;
  }
*/

                  } // loop over py index
               } // loop over px index
            } // loop over index t labeling relative slice number

// Renormalize avg_ztwoDarray_ptr values to range from 0 to 1:

            ptwoDarray_ptr->clear_values();
            for (int px=0; px<ptwoDarray_ptr->get_mdim(); px++)
            {
               for (int py=0; py<ptwoDarray_ptr->get_ndim(); py++)
               {
                  double curr_p=avg_ztwoDarray_ptr->get(px,py);
                  if (fieldtest_ID==1)
                  {
                     curr_p /= 255.0;
                  }
                  ptwoDarray_ptr->put(px,py,curr_p);
               }
            }

// For montage AVI movie generation purposes, export only
// colored-version of n_timesteps=0 "time-averaged" mask.  But for
// n_timesteps > 0, export both grey-scale and colored versions of
// time-averaged mask:

            int n_channels=3;
            texture_rectangle* texture_rectangle_ptr=
               new texture_rectangle(mask_width,mask_height,1,n_channels,NULL);
            texture_rectangle_ptr->initialize_RGB_twoDarray_image(
               ptwoDarray_ptr);
            texture_rectangle_ptr->fill_twoDarray_image(
               ptwoDarray_ptr,n_channels);

            string mask_URL_suffix=stringfunc::suffix(avg_mask_URL);

            if (n_timesteps > 0 || fieldtest_ID >= 2)
            {
               if (mask_URL_suffix != "jpg" && mask_URL_suffix != "JPG")
               {
                  avg_mask_URL=avg_mask_URL+".jpg";
               }
               texture_rectangle_ptr->write_curr_frame(avg_mask_URL);
            }
            texture_rectangle_ptr->convert_grey_values_to_hues();

            if (mask_URL_suffix != "jpg" && mask_URL_suffix != "JPG")
            {
               colored_avg_mask_URL=colored_avg_mask_URL+".jpg";
            }

// Note added on 1/26/13: For Nov 2012 Expt 2H, program SMOKE_MASKS
// should have already created jpg file corresponding to
// colored_avg_mask_URL for n_timesteps=0.  And for this experiment,
// we are not generating colored_avg_mask jpg file for n_timesteps=2.
// So we only execute following write_curr_frame() call if
// fieldtest_ID==1:

            if (fieldtest_ID==1)
            {
               texture_rectangle_ptr->write_curr_frame(colored_avg_mask_URL);
               string banner="Exported "+colored_avg_mask_URL;
               outputfunc::write_big_banner(banner);
            }
            
            delete texture_rectangle_ptr;

            for (int m=0; m<twoDarray_ptrs.size(); m++)
            {
               delete twoDarray_ptrs[m];
            }
            twoDarray_ptrs.clear();
         
         } // loop over index s labeling cameras for curr slice

      } // loop over slice_number index

      delete avg_ztwoDarray_ptr;
      delete ptwoDarray_ptr;

   } // loop over iter index 

// ************************************************************************
  
}
