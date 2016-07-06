// ========================================================================
// Program TEMPORAL_COMPOSITES loops over all tripod cameras.  For
// each pixel in a tripod camera's mask, it computes the maximum
// intensity value over all time-slices.  The resulting temporally composited
// mask illustrates where plume activity actually occurred within a
// particular camera's image plane.  We intend to use the temporal
// composite masks to ignore image plane regions where no smoke ever
// passed for 3D volume reconstruction.
// ========================================================================
// Last updated on 1/15/13; 1/16/13; 1/27/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osgDB/FileUtils>

#include "osg/osgGraphicals/AnimationController.h"
#include "geometry/bounding_box.h"
#include "video/camera.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "postgres/plumedatabasefuncs.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "osg/osg3D/tdpfuncs.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"
#include "osg/osgWindow/ViewerManager.h"
#include "coincidence_processing/VolumetricCoincidenceProcessor.h"

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

   timefunc::initialize_timeofday_clock();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

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

   string start_timestamp;
   plumedatabasefunc::retrieve_fieldtest_metadata_from_database(
      postgis_db_ptr,fieldtest_ID,start_timestamp);
   cout << "start_timestamp = " << start_timestamp << endl;
   Clock clock;
   cout.precision(13);

   bool UTC_flag=true;
   double epoch=
      clock.timestamp_string_to_elapsed_secs(start_timestamp,UTC_flag);
   int year=clock.get_year();
   string month_name=clock.get_month_name();

   int day_number;
   string experiment_label;
   plumedatabasefunc::retrieve_mission_metadata_from_database(
      postgis_db_ptr,fieldtest_ID,mission_ID,
      day_number,experiment_label);

   string experiment_subdir="/data/ImageEngine/plume/";
   experiment_subdir += month_name+stringfunc::number_to_string(year)+"/Day";
   experiment_subdir += stringfunc::number_to_string(day_number)+"/"+
      experiment_label+"/";
//   cout << "experiment_subdir = " << experiment_subdir << endl;

   string packages_subdir=experiment_subdir+"packages/";
   filefunc::dircreate(packages_subdir);

   string results_subdir=bundler_IO_subdir+"plume_results/";
   results_subdir += stringfunc::number_to_string(day_number)+
      experiment_label+"/";
   cout << "results_subdir = " << results_subdir << endl;
   filefunc::dircreate(results_subdir);
   string temporal_composites_subdir=results_subdir+"temporal_composites/";
   filefunc::dircreate(temporal_composites_subdir);

   int start_slice_number=1;
   int stop_slice_number=499;
//   cout << endl;
//   cout << "Enter starting time slice number:" << endl;
//   cin >> start_slice_number;
   
//   cout << "Enter stopping time slice number:" << endl;
//   cin >> stop_slice_number;

   int min_slice_number,max_slice_number;
   plumedatabasefunc::retrieve_extremal_slice_IDs_from_database(
      postgis_db_ptr,mission_ID,min_slice_number,max_slice_number);
//   cout << "min_slice_number = " << min_slice_number
//        << " max_slice_number = " << max_slice_number << endl;
   start_slice_number=basic_math::max(start_slice_number,min_slice_number);
   stop_slice_number=basic_math::min(stop_slice_number,max_slice_number);
   cout << "start_slice_number = " << start_slice_number
        << " stop_slice_number = " << stop_slice_number << endl;

   int slice_number_step=1;
//   cout << "Enter time slice number step:" << endl;
//   cin >> slice_number_step;

   int n_cameras=10;
   vector<texture_rectangle*> temporal_composite_texture_rectangle_ptrs;

   int camera_ID_offset=-1;
   if (fieldtest_ID==1)
   {
      camera_ID_offset=17;
   }
   else if (fieldtest_ID==2)
   {
      camera_ID_offset=1;
   }

// ========================================================================
// Loop over slice number starts here
// ========================================================================

   for (int slice_number=start_slice_number; slice_number <= stop_slice_number;
        slice_number += slice_number_step)
   {
      cout << "Processing slice_number = " << slice_number;
      double elapsed_secs=timefunc::elapsed_timeofday_time();
      double elapsed_mins=elapsed_secs/60;
      cout << " Computation time = " << elapsed_mins << " mins " << endl;

// Retrieve time-synced info from photos table of plume database:

      vector<int> photo_ID,sensor_ID;
      vector<string> URL,thumbnail_URL,mask_URL;
      plumedatabasefunc::retrieve_photo_metadata_from_database(
         postgis_db_ptr,fieldtest_ID,mission_ID,slice_number,
         photo_ID,sensor_ID,URL,thumbnail_URL,mask_URL);

      int n_photos=photo_ID.size();

// Ignore time slice if its number of synchronized photos is less than
// some reasonable minimal number:

      const int min_n_photos=8;
//      const int min_n_photos=10;
      if (n_photos < min_n_photos) continue;

// Read mask information from photos table of plume database.
// Convert contents of imported masks into texture rectangles and
// store within STL vector mask_texture_rectangle_ptrs.  Also record
// non-zero mask intensities within an STL vector:

      vector<double> mask_intensities;
      vector<texture_rectangle*> mask_texture_rectangle_ptrs;
      for (int m=0; m<n_photos; m++)
      {
         string dirname=filefunc::getdirname(mask_URL[m]);
         string basename=filefunc::getbasename(mask_URL[m]);
         string prefix=stringfunc::prefix(basename);
         string suffix=stringfunc::suffix(basename);
         
//         string time_avgd_mask_filename=dirname+"TimeAvgd_2_"+basename;
         string time_avgd_mask_filename=dirname+"TimeAvgd_0_"+basename;
//         string time_avgd_mask_filename=dirname+"TimeAvgd_0_"+prefix;
         if (suffix=="bin") 
         {
            time_avgd_mask_filename += ".jpg";
         }

         cout << "m = " << m 
              << " mask filename = " << mask_URL[m] 
              << " time avgd mask filename = " << time_avgd_mask_filename
              << endl;

         texture_rectangle* mask_texture_rectangle_ptr=new texture_rectangle(
            time_avgd_mask_filename,NULL);
         mask_texture_rectangle_ptrs.push_back(mask_texture_rectangle_ptr);

// Generate temporal composite texture rectangle for camera labeled by m
// iff it doesn't already exist:

         if (temporal_composite_texture_rectangle_ptrs.size() < m+1)
         {
            texture_rectangle* texture_rectangle_ptr=
               new texture_rectangle(time_avgd_mask_filename,NULL);
            texture_rectangle_ptr->clear_all_RGB_values();
            temporal_composite_texture_rectangle_ptrs.push_back(
               texture_rectangle_ptr);
         }

         int max_intensity_changes=0;
         int W=mask_texture_rectangle_ptr->getWidth(); 
         int H=mask_texture_rectangle_ptr->getHeight(); 
         for (int px=0; px<W; px++)
         {
            for (int py=0; py<H; py++)
            {
               int R,G,B;
               mask_texture_rectangle_ptr->get_pixel_RGB_values(
                  px,py,R,G,B);
               double r=double(R)/255.0;
               double g=double(G)/255.0;
               double b=double(B)/255.0;
               double h,s,v;
               colorfunc::RGB_to_hsv(r,g,b,h,s,v);

               const double TINY=0.0001;
               if (v > TINY) 
               {
                  mask_intensities.push_back(v);
                  int intensity=255*v;
                  temporal_composite_texture_rectangle_ptrs[m]->
                     get_pixel_RGB_values(px,py,R,G,B);
                  int max_intensity=R;
//                  cout << "px = " << px << " py = " << py
//                       << " intensity = " << intensity 
//                       << " max_intensity = " << max_intensity << endl;
                  if (max_intensity < intensity)
                  {
                     temporal_composite_texture_rectangle_ptrs[m]->
                        set_pixel_RGB_values(
                           px,py,intensity,intensity,intensity);
                     max_intensity_changes++;
                  }
               } // v > TINY conditional
               
            } // loop over py index
         } // loop over px index

         double frac_v_changes=double(max_intensity_changes)/(W*H);
         cout << "mask m = " << m 
              << " max_intensity_changes = " << max_intensity_changes 
              << " frac_v_changes = " << frac_v_changes << endl;

      } // loop over index m labeling masks

      for (int m=0; m<mask_texture_rectangle_ptrs.size(); m++)
      {
         delete mask_texture_rectangle_ptrs[m];
      }

   } // loop over slice_number index

// ========================================================================
// Loop over slice number stops here
// ========================================================================

   for (int m=0; m<n_cameras; m++)
   {
      int camera_ID=m+camera_ID_offset;
      temporal_composite_texture_rectangle_ptrs[m]->
         convert_grey_values_to_hues();
      string temporal_composite_filename=temporal_composites_subdir+
         "temporal_composite_"+stringfunc::integer_to_string(camera_ID,2)
         +".jpg";
      temporal_composite_texture_rectangle_ptrs[m]->
         write_curr_frame(temporal_composite_filename);
   }

   string banner="Exported temporal composites to "+temporal_composites_subdir;
   outputfunc::write_big_banner(banner);

}
