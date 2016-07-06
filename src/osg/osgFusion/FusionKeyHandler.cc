// ==========================================================================
// FusionKeyHandler class member function definitions
// ==========================================================================
// Last modified on 7/18/11; 11/19/11; 12/26/11
// ==========================================================================

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgFusion/FusionKeyHandler.h"
#include "osg/osg2D/Movie.h"
#include "osg/osg2D/MoviesGroup.h"

using std::cin;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void FusionKeyHandler::allocate_member_objects()
{
}

void FusionKeyHandler::initialize_member_objects()
{
   ModeController_ptr=NULL;
   FusionGroup_ptr=NULL;
   m_display=NULL;
   m_controller=NULL;
   MoviesGroup_ptr=NULL;
}

FusionKeyHandler::FusionKeyHandler(ModeController* MC_ptr,FusionGroup* FG_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ModeController_ptr=MC_ptr;
   FusionGroup_ptr=FG_ptr;
}

FusionKeyHandler::FusionKeyHandler(
 ModeController* MC_ptr,FusionGroup* FG_ptr,
 Movie* p_display,AnimationController* p_controller,MoviesGroup* MG_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ModeController_ptr=MC_ptr;
   FusionGroup_ptr=FG_ptr;

   m_display=p_display;
   m_controller=p_controller;
   MoviesGroup_ptr=MG_ptr;
}

FusionKeyHandler::~FusionKeyHandler()
{
}

// ------------------------------------------------------
bool FusionKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==ModeController::FUSE_DATA)
      {
         if (ea.getKey()=='a')
         {
            get_FusionGroup_ptr()->compute_backprojected_color_arrays();
            return true;
         }
         else if (ea.getKey()=='b')
         {
            get_FusionGroup_ptr()->video_backprojection();
            return true;
         }
         else if (ea.getKey()=='c')
         {
            get_FusionGroup_ptr()->consolidate_XYZ_and_UV_feature_info();
//            get_FusionGroup_ptr()->output_all_tiepoint_data();
            return true;
         }
         else if (ea.getKey()=='d')
         {
            get_FusionGroup_ptr()->tiepoint_backprojection();
            return true;
         }
         else if (ea.getKey()=='i')
         {
            get_FusionGroup_ptr()->insert_2D_image_into_3D_worldspace();
            return true;
         }
         else if (ea.getKey()=='j')
         {
            get_FusionGroup_ptr()->constrained_image_insertion();
            return true;
         }
         else if (ea.getKey()=='l')
         {
            get_FusionGroup_ptr()->test_score();
            return true;
         }
         else if (ea.getKey()=='t')
         {
//            get_FusionGroup_ptr()->generate_terrain_masks();
            get_FusionGroup_ptr()->generate_terrain_masks(6);
            get_FusionGroup_ptr()->generate_terrain_masks(7);
            get_FusionGroup_ptr()->generate_terrain_masks(8);
            get_FusionGroup_ptr()->generate_terrain_masks(9);
            get_FusionGroup_ptr()->generate_terrain_masks(0);
            get_FusionGroup_ptr()->generate_terrain_masks(1);
            get_FusionGroup_ptr()->generate_terrain_masks(2);
            get_FusionGroup_ptr()->generate_terrain_masks(3);
            get_FusionGroup_ptr()->generate_terrain_masks(4);
            get_FusionGroup_ptr()->generate_terrain_masks(5);
            return true;
         }

      }
      else if (get_ModeController_ptr()->getState()==
               ModeController::MANIPULATE_FUSED_DATA)
      {
         if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            Movie* Movie_ptr=get_Movie_ptr();
            if (Movie_ptr != NULL)
            {
               double alpha=Movie_ptr->get_alpha();
               alpha=basic_math::max(0.0,alpha-0.05);
               Movie_ptr->set_alpha(alpha);
               return true;
            }
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            Movie* Movie_ptr=get_Movie_ptr();
            if (Movie_ptr != NULL)
            {
               double alpha=Movie_ptr->get_alpha();
               alpha=basic_math::min(1.0,alpha+0.05);
               Movie_ptr->set_alpha(alpha);
               return true;
            }
         }
         else if (ea.getKey()=='v')
         {
            MoviesGroup* MoviesGroup_ptr=get_MoviesGroup_ptr();
            if (MoviesGroup_ptr != NULL)
            {
               get_Movie_ptr()->set_CM3D_viewpoint(
                  MoviesGroup_ptr->get_curr_t(),
                  MoviesGroup_ptr->get_passnumber());
               return true;
            }
         }
      }
      else if (get_ModeController_ptr()->getState()==
               ModeController::MANIPULATE_RECTANGLE)
      {
         if (ea.getKey()=='a')
         {
            get_FusionGroup_ptr()->
               project_ground_polyhedron_into_aerial_OBSFRUSTA();
            return true;
         }
         else if (ea.getKey()=='b')
         {
            get_FusionGroup_ptr()->fly_to_best_aerial_OBSFRUSTUM();
            return true;
         }
         else if (ea.getKey()=='d')
         {
            get_FusionGroup_ptr()->destroy_all_sub_aerial_OBSFRUSTA();
            return true;
         }
         else if (ea.getKey()=='p')
         {
//            polyhedron* polyhedron_ptr=
               get_FusionGroup_ptr()->backproject_2D_rectangle_into_3D();
            return true;
         }

      } // Mode conditional
   } // key down conditional

   return false;
}

// ------------------------------------------------------
MoviesGroup* FusionKeyHandler::get_MoviesGroup_ptr()
{
   ObsFrustaGroup* ObsFrustaGroup_ptr=get_FusionGroup_ptr()->
      get_ObsFrustaGroup_ptr();
   if (ObsFrustaGroup_ptr != NULL)
   {
      return ObsFrustaGroup_ptr->get_MoviesGroup_ptr();
   }
   else
   {
      cout << "ObsFrustaGroup_ptr = NULL" << endl;
      return NULL;
   }
}

Movie* FusionKeyHandler::get_Movie_ptr()
{
   MoviesGroup* MoviesGroup_ptr=get_MoviesGroup_ptr();
   if (MoviesGroup_ptr != NULL)
   {
      return MoviesGroup_ptr->get_Movie_ptr(0);
   }
   else
   {
      cout << "MoviesGroup_ptr = NULL" << endl;
      return NULL;
   }
}

         
         
