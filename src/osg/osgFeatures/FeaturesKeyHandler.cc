// ==========================================================================
// FeaturesKeyHandler class member function definitions
// ==========================================================================
// Last modified on 2/16/09; 9/29/09; 5/7/13; 6/21/14
// ==========================================================================

#include "osg/osgFeatures/FeaturesGroup.h"
#include "osg/osgFeatures/FeaturesKeyHandler.h"
#include "osg/ModeController.h"

using std::cin;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void FeaturesKeyHandler::allocate_member_objects()
{
}

void FeaturesKeyHandler::initialize_member_objects()
{
   FeaturesGroup_ptr=NULL;
}

FeaturesKeyHandler::FeaturesKeyHandler(
   const int p_ndims,FeaturesGroup* FG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(FG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ndims=p_ndims;
   FeaturesGroup_ptr=FG_ptr;
}

FeaturesKeyHandler::~FeaturesKeyHandler()
{
}

// ------------------------------------------------------
bool FeaturesKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
//   cout << "inside FKH::handle()" << endl;
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==ModeController::SET_CENTER)
      {

// Press "m" to toggle on/off centering of imagery upon some selected
// feature:

         if (ea.getKey()=='m')
         {
            get_FeaturesGroup_ptr()->center_imagery_on_selected_feature();
            return true;
         }
      } // mode = SET_CENTER conditional

// ......................................................................

      else if (get_ModeController_ptr()->getState()==
          ModeController::INSERT_FEATURE)
      {

// Press "r" to restore feature information from ascii text file:
      
         if (ea.getKey()=='r')
         {
            get_FeaturesGroup_ptr()->read_feature_info_from_file();
            get_FeaturesGroup_ptr()->write_feature_html_file();
            return true;
         }
      
// Press "s" to save feature information to ascii text file:

         else if (ea.getKey()=='s')
         {
//            get_FeaturesGroup_ptr()->new_save_feature_info_to_file();
            get_FeaturesGroup_ptr()->save_feature_info_to_file();

            return true;
         }

      }
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_FEATURE)
      {
      
// Press "e" to edit a feature:

         if (ea.getKey()=='e')
         {
            get_FeaturesGroup_ptr()->edit_feature_label();
            return true;
         }

// Press "d" to renormalize feature coords from multi-image (U,V) to 
// single image (u,v) coordinates:

         if (ea.getKey()=='d')
         {
            get_FeaturesGroup_ptr()->renormalize_feature_coords();
            return true;
         }

// Press "j" to toggle on/off score display for features:
      
         else if (ea.getKey()=='j')
         {
            bool flag=get_FeaturesGroup_ptr()->
               get_display_feature_scores_flag();
            get_FeaturesGroup_ptr()->set_display_feature_scores_flag(!flag);
            return true;
         }

// Press "m" to compute multi-line intersection point from within 2D
// video window:
      
         else if (ea.getKey()=='m')
         {
            get_FeaturesGroup_ptr()->compute_multi_line_intersection();
            return true;
         }

// Press "r" to restore feature information from ascii text file:
      
         else if (ea.getKey()=='r')
         {
            get_FeaturesGroup_ptr()->read_feature_info_from_file();
            get_FeaturesGroup_ptr()->write_feature_html_file();
            return true;
         }
      
// Press "s" to save feature information to ascii text file:

         else if (ea.getKey()=='s')
         {
//            get_FeaturesGroup_ptr()->new_save_feature_info_to_file();
            get_FeaturesGroup_ptr()->save_feature_info_to_file();

            return true;
         }

         else if (ea.getKey()=='t')
         {
            string features_subdir=
               "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/devens/Aug26_2011/images/shovel/";
            string consolidated_features_filename=features_subdir+
               "features.shovels";
            get_FeaturesGroup_ptr()->draw_backprojected_2D_features(
               consolidated_features_filename);
            return true;
         }

// Press "Backspace" key to erase (U,V,W) coordinates for a particular
// image:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_BackSpace)
         {
            get_FeaturesGroup_ptr()->erase_feature();
            return true;
         }

// Press "Insert" key to un-erase (U,V,W) coordinates for a
// particular image:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Insert)
         {
            get_FeaturesGroup_ptr()->unerase_feature();
            return true;
         }

// Press "Delete" key to completely destroy a feature:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            get_FeaturesGroup_ptr()->destroy_feature();
            return true;
         }

// Press ">" ["<"] key to increase [decrease] a feature's size:

         else if (ea.getKey()=='>')
         {
            get_FeaturesGroup_ptr()->change_size(2.0);
            return true;
         }
         else if (ea.getKey()=='<')
         {
            get_FeaturesGroup_ptr()->change_size(0.5);
            return true;
         }

// Press "Up arrow" or "Down arrow" to move a selected feature up or
// down in the world-z direction:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            get_FeaturesGroup_ptr()->move_z(1);
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            get_FeaturesGroup_ptr()->move_z(-1);
            return true;
         }
         else if (ea.getKey()=='p')
         {
            get_FeaturesGroup_ptr()->fit_features_to_parallelogram();
            return true;
         }
//         else if (ea.getKey()=='o')
//         {
//            get_FeaturesGroup_ptr()->change_origin_point();
//            return true;
//         }

      } // mode = MANIPULATE_FEATURE conditional         

// ......................................................................

      if (get_ModeController_ptr()->getState()==ModeController::TRACK_FEATURE)
      {

// Press "r" to restore feature information from ascii text file:
         
//         if (ea.getKey()=='r')
//         {
//            bool propagate_features_flag=true;
//            get_FeaturesGroup_ptr()->read_feature_info_from_file(
//               propagate_features_flag);
//            return true;
//         }

// Press "m" to compute avg offsets from mid-pass location needed to
// stabilize imagery 
      
         if (ea.getKey()=='m')
         {
            get_FeaturesGroup_ptr()->
               compute_avg_feature_offsets_from_midpass_location();
         }

// On 10/26/05, we discovered using the lowell3_corr_grey_280-550.vid
// data setthat the following set of operations has a bug.  In order
// to give Jessica Sandland some poor-man's version of video
// stabilization, we temporarily assign the 'm' key to just computing
// average feature offsets from midpass location using raw KLT
// results.  And we separated off the following (apparently buggy!)
// commands which attempt to improve the KLT results under the 'i' key
// that only we are ever likely to use...

         else if (ea.getKey()=='i')
         {
            get_FeaturesGroup_ptr()->
               compute_avg_feature_offsets_from_midpass_location();
            get_FeaturesGroup_ptr()->temporally_filter_stabilized_features();
            get_FeaturesGroup_ptr()->
               compute_avg_feature_offsets_from_midpass_location();
            get_FeaturesGroup_ptr()->
               compute_features_scores_and_adjust_their_locations();
            get_FeaturesGroup_ptr()->temporally_filter_stabilized_features();
            get_FeaturesGroup_ptr()->
               compute_avg_feature_offsets_from_midpass_location();
         }

/*
// Press "Backspace" key to erase (U,V,W) coordinates for a particular
// image:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_BackSpace)
         {
            get_FeaturesGroup_ptr()->erase_feature();
            return true;
         }

// Press "Delete" key to completely destroy a feature:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            get_FeaturesGroup_ptr()->destroy_feature();
            return true;
         }
*/

// Press ">" ["<"] key to increase [decrease] a feature's size:

         else if (ea.getKey()=='>')
         {
            get_FeaturesGroup_ptr()->change_size(2.0);
            return true;
         }
         else if (ea.getKey()=='<')
         {
            get_FeaturesGroup_ptr()->change_size(0.5);
            return true;
         }

// Press "P" to count feature frequency in video images:

         else if (ea.getKey()=='P')
         {
            get_FeaturesGroup_ptr()->prune_short_track_features();            
            return true;
         }
         else if (ea.getKey()=='a')
         {
            get_FeaturesGroup_ptr()->update_image_appearances();            
            return true;
         }
         else if (ea.getKey()=='s')
         {
            get_FeaturesGroup_ptr()->update_feature_scores();            
            return true;
         }
         else if (ea.getKey()=='e')
         {
            get_FeaturesGroup_ptr()->erase_unmarked_features();            
            return true;
         }
         else if (ea.getKey()=='n')
         {
            get_FeaturesGroup_ptr()->renumber_all_features();            
            return true;
         }
         else if (ea.getKey()=='f')
         {
            int delta_ID;
            cout << "Enter shift for all feature IDs:" << endl;
            cin >> delta_ID;
            get_FeaturesGroup_ptr()->shift_all_feature_IDs(delta_ID); 
            return true;
         }
         else if (ea.getKey()=='v')
         {
            get_FeaturesGroup_ptr()->stabilize_video_imagery();
            return true;
         }
         else if (ea.getKey()=='d')
         {
            get_FeaturesGroup_ptr()->Delaunay_triangulate();
            return true;
         }
//         else if (ea.getKey()=='D')
//         {
//            get_FeaturesGroup_ptr()->destroy_features();
//            return true;
//         }
         else if (ea.getKey()=='c')
         {
            double t1,t2;
            cout << "Enter first image number:" << endl;
            cin >> t1;
            cout << "Enter second image number:" << endl;
            cin >> t2;
            get_FeaturesGroup_ptr()->common_unerased_feature_IDs(t1,t2);
//            get_FeaturesGroup_ptr()->generate_feature_kdtree(t1,t2);
//            get_FeaturesGroup_ptr()->propagate_feature(t1,t2);
            return true;
         }
//         else if (ea.getKey()=='k')
//         {
//            get_FeaturesGroup_ptr()->KLT_wander_test();
//            return true;
//         }

      } // mode = TRACK_FEATURE conditional
      
   } // key down conditional
   
   return false;
}


