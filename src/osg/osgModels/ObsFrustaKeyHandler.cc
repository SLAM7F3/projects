// ==========================================================================
// ObsFrustaKeyHandler class member function definitions
// ==========================================================================
// Last modified on 2/17/08; 6/15/08; 12/4/10
// ==========================================================================

#include "video/camera.h"
#include "color/colorfuncs.h"
#include "osg/ModeController.h"
#include "osg/osg2D/Movie.h"
#include "osg/osgModels/ObsFrustaGroup.h"
#include "osg/osgModels/ObsFrustaKeyHandler.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ObsFrustaKeyHandler::allocate_member_objects()
{
}

void ObsFrustaKeyHandler::initialize_member_objects()
{
   n_still_images=n_frustum=toggle_counter=0;
   Terrain_Manipulator_ptr=NULL;
   ObsFrustaGroup_ptr=NULL;
}

ObsFrustaKeyHandler::ObsFrustaKeyHandler(
   ObsFrustaGroup* OFG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(OFG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ObsFrustaGroup_ptr=OFG_ptr;
}

ObsFrustaKeyHandler::ObsFrustaKeyHandler(
   ObsFrustaGroup* OFG_ptr,ModeController* MC_ptr,
   osgGA::Terrain_Manipulator* TM_ptr):
   GraphicalsKeyHandler(OFG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   Terrain_Manipulator_ptr=TM_ptr;
   ObsFrustaGroup_ptr=OFG_ptr;
}

ObsFrustaKeyHandler::~ObsFrustaKeyHandler()
{
}

// ------------------------------------------------------
bool ObsFrustaKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_FUSED_DATA)
      {
         if (ea.getKey()=='c')
         {

// Recall that the OSGgroup for the Pyramids within each static movie
// ObsFrusta is initially masked.  Whenever the user presses 'c', we
// unmask the n_frustum ObsFrustum and then increment n_frustum:

            ObsFrustaGroup_ptr->unerase_Graphical(n_frustum++);
         }
         else if (ea.getKey()=='t')
         {
            for (unsigned int i=0; i<ObsFrustaGroup_ptr->get_n_Graphicals(); 
                 i++)
            {
               if (is_even(toggle_counter))
               {
                  ObsFrustaGroup_ptr->unerase_Graphical(i);
               }
               else
               {
                  ObsFrustaGroup_ptr->erase_Graphical(i);
               }
            }
            ObsFrustaGroup_ptr->reset_frustum_colors_based_on_Zcolormap();
            toggle_counter++;
            return true;
         }
//         else if (ea.getKey()=='i')
//         {
//            ObsFrustaGroup_ptr->compute_polyhedra_intersections();
//            return true;
//         }
//         else if (ea.getKey()=='d')
//         {
//            ObsFrustaGroup_ptr->display_intersection_footprint();
//            return true;
//         }
         else if (ea.getKey()=='f')
         {
            int selected_ID=ObsFrustaGroup_ptr->get_selected_Graphical_ID();
            if (selected_ID >= 0)
            {
               ObsFrustaGroup_ptr->flyto_camera_location(selected_ID);
            } 
            return true;
         }
         else if (ea.getKey()==',')
         {
            for (unsigned int n=0; n<ObsFrustaGroup_ptr->get_n_Graphicals(); 
                 n++)
            {
               ObsFrustaGroup_ptr->erase_Graphical(n);
            }
            return true;
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
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            Movie* Movie_ptr=get_Movie_ptr();
            if (Movie_ptr != NULL)
            {
               double alpha=Movie_ptr->get_alpha();
               alpha=basic_math::max(0.0,alpha-0.05);
               Movie_ptr->set_alpha(alpha);
               return true;
            }
         } // if statement
      } // mode = MANIPULATE_FUSED_DATA conditional
   } // key down conditional
   
   return false;
}

// ------------------------------------------------------
Movie* ObsFrustaKeyHandler::get_Movie_ptr()
{
//   cout << "inside ObsFrustaKeyHandler::get_Movie_ptr()" << endl;

   MoviesGroup* MoviesGroup_ptr=ObsFrustaGroup_ptr->get_MoviesGroup_ptr();
   if (MoviesGroup_ptr != NULL)
   {
      int selected_ObsFrustum_number=ObsFrustaGroup_ptr->
         get_selected_Graphical_ID();
      if (selected_ObsFrustum_number >= 0)
      {
         return MoviesGroup_ptr->get_Movie_ptr(selected_ObsFrustum_number);
      }
      return NULL;
   }
   else
   {
      cout << "Error in ObsFrustaKeyHandler:get_Movie_ptr()" << endl;
      cout << " MoviesGroup_ptr = NULL" << endl;
      return NULL;
   }
}


