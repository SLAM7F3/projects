// ========================================================================
// Program STABILIZED_CHIP imports ERSA feature information for a
// single, specified target track.  It excises a 2D bounding box
// around the target's image-plane location for each imported panel
// frame.  In theory, the resulting set of cropped images should have
// the specified ERSA target stabilized near their centers.  
// ========================================================================
// Last updated on 4/10/13; 4/6/14
// ========================================================================

#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "astro_geo/Clock.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "general/filefuncs.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "image/imagefuncs.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "osg/osg3D/Terrain_Manipulator.h"

// ========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("2D imagery");

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=true;
//   bool display_movie_state=false;			// viewgraphs
//   bool display_movie_number=false;			// viewgraphs
//   bool display_movie_world_time=false;		// viewgraphs
   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::RUN_MOVIE);
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();

// Specify start, stop and step times for master game clock:

   double start_time=1360867646;   
			// 2013-02-14 18:47:26.00 UTC = 1:47:26 EST
   double stop_time=start_time+2*429;	
			// 2013-02-14 19:01:43.100 UTC = 2:01:43 EST
   double delta_t=2;	// secs

   bool start_time_flag=true;
   operations.set_master_world_UTC(start_time,start_time_flag);
   start_time_flag=false;
   operations.set_master_world_UTC(stop_time,start_time_flag);
   operations.set_delta_master_world_time_step_per_master_frame(delta_t);
   operations.reset_AnimationController_world_time_params();

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);

   string movie_filename=
      passes_group.get_videopass_ptr()->get_first_filename();
   cout << "movie_filename = " << movie_filename << endl;
   string image_subdir=filefunc::getdirname(movie_filename);
   if (image_subdir.size()==0) image_subdir="./";
   cout << "image_subdir = " << image_subdir << endl;

   AnimationController_ptr->store_ordered_image_filenames(image_subdir);
   int number_of_images=AnimationController_ptr->
      get_n_ordered_image_filenames();
   cout << "number_of_images = " << number_of_images << endl;

   texture_rectangle* texture_rectangle_ptr=
      movies_group.generate_new_texture_rectangle(movie_filename);
   Movie* movie_ptr=movies_group.generate_new_Movie(texture_rectangle_ptr);

// Scan through subdirectory containing video frames.  Set minimum and
// maximum photo numbers based upon the files' name:

   int min_photo_number=-1;
   int max_photo_number=-1;
   videofunc::find_min_max_photo_numbers(
      image_subdir,min_photo_number,max_photo_number);
   int Nimages=max_photo_number-min_photo_number+1;
   if (number_of_images > Nimages)
   {
      Nimages=number_of_images;
      min_photo_number=0;
      max_photo_number=Nimages-1;
   }

   cout << "min_photo_number = " << min_photo_number
        << " max_photo_number = " << max_photo_number
        << " Nimages = " << Nimages << endl;

   AnimationController_ptr->set_nframes(Nimages);
   AnimationController_ptr->set_frame_counter_offset(min_photo_number);

   movie_ptr->get_texture_rectangle_ptr()->set_first_frame_to_display(
      min_photo_number);
   movie_ptr->get_texture_rectangle_ptr()->set_last_frame_to_display(
      max_photo_number);

   int track_ID=21698;

// Import ERSA tracks:

   vector<int> frame_ID;
   vector<double> U,V,range;

//   string features_filename="features7.txt";
   string features_filename="better_stable_features.txt";
   filefunc::ReadInfile(features_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      int feature_ID=column_values[1];
      if (feature_ID != track_ID) continue;

      frame_ID.push_back(column_values[0]);
      U.push_back(column_values[3]);
      V.push_back(column_values[4]);
      range.push_back(column_values[5]);
   }

   string stabilized_features_filename="stable_features.txt";
   ofstream features_stream;

   filefunc::openfile(stabilized_features_filename,features_stream);
   features_stream << "# Frame   Feature_ID   Passnumber   U 			 V  	   Range (kms)" << endl;
   features_stream << endl;

   unsigned int ydim=texture_rectangle_ptr->getHeight();
   for (unsigned int n=0; n<frame_ID.size(); n++)
   {
      int curr_framenumber=frame_ID[n];
      AnimationController_ptr->set_curr_framenumber(curr_framenumber);
      string image_filename=AnimationController_ptr->get_curr_image_filename();
      texture_rectangle_ptr->import_photo_from_file(
         image_filename);

      string input_subdir=filefunc::getdirname(image_filename);
//      string output_subdir=input_subdir+"track"+stringfunc::number_to_string(
//         track_ID)+"/";
      string output_subdir=input_subdir+"recropped/";
      filefunc::dircreate(output_subdir);

      string basename=filefunc::getbasename(image_filename);
      string prefix=stringfunc::prefix(basename);

      double Ulo=U[n]-0.2;
      double Uhi=U[n]+0.2;

      double Vlo=V[n]-0.2;
      double Vhi=V[n]+0.2;

      unsigned int px_Ulo,px_Uhi,py_Vlo,py_Vhi;
      texture_rectangle_ptr->get_pixel_coords(Ulo,Vlo,px_Ulo,py_Vlo);
      texture_rectangle_ptr->get_pixel_coords(Uhi,Vhi,px_Uhi,py_Vhi);
      py_Vlo=ydim-py_Vlo;
      py_Vhi=ydim-py_Vhi;
      
      string cropped_filename=output_subdir+basename;
      texture_rectangle_ptr->write_curr_frame(
         px_Ulo,px_Uhi,py_Vlo,py_Vhi,cropped_filename);
      cout << "Exported cropped_filename = " << cropped_filename << endl;

      double Ustable=1.0;
      double Vstable=0.0;

      features_stream << curr_framenumber << " \t "
                      << track_ID << " \t "
                      << " 0 " << " \t "
                      << Ustable << " \t " 
                      << Vstable << " \t " 
                      << range[n] 
                      << endl;
   }
   
   delete texture_rectangle_ptr;

   filefunc::closefile(stabilized_features_filename,features_stream);

}

