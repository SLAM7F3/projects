// ==========================================================================
// PanoramasKeyHandler class member function definitions
// ==========================================================================
// Last modified on 8/12/09; 8/13/09; 1/10/10
// ==========================================================================

#include "osg/ModeController.h"
#include "osg/osgPanoramas/PanoramasGroup.h"
#include "osg/osgPanoramas/PanoramasKeyHandler.h"

using std::cout;
using std::endl;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PanoramasKeyHandler::allocate_member_objects()
{
}

void PanoramasKeyHandler::initialize_member_objects()
{
   SignPostsGroup_ptr=NULL;
}

PanoramasKeyHandler::PanoramasKeyHandler(
   PanoramasGroup* PG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(PG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   PanoramasGroup_ptr=PG_ptr;
}

PanoramasKeyHandler::~PanoramasKeyHandler()
{
}

// ------------------------------------------------------
bool PanoramasKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_FUSED_DATA)
      {

         if (ea.getKey()=='h')
         {
            cout << "H key pressed" << endl;
            if (PanoramasGroup_ptr->get_selected_Panorama_ID() >= 0)
            {
               int curr_OBSFRUSTUM_ID=PanoramasGroup_ptr->
                  get_curr_OBSFRUSTUM_ID();
               int jump_ID=PanoramasGroup_ptr->
                  get_jump_OBSFRUSTUM_ID(curr_OBSFRUSTUM_ID);
               cout << "curr_OBSFRUSTUM_ID = " << curr_OBSFRUSTUM_ID << endl;
               cout << "jump_ID = " << jump_ID << endl;

               if (jump_ID >= 0)
               {
//                  PanoramasGroup_ptr->fly_to_OBSFRUSTUM(jump_ID);
               } 
            }
            return true;
         }
         else if (ea.getKey()=='5')
         {
            cout << "5 key pressed" << endl;
//            threevector XYZ(0,0,0);
//            twovector closest_UV;
//            int closest_OBSFRUSTUM_ID=
//               PanoramasGroup_ptr->project_world_posn_into_imageplanes(
//                  XYZ,closest_UV);

            if (SignPostsGroup_ptr==NULL) return false;
            PanoramasGroup_ptr->project_SignPosts_into_imageplanes(
               SignPostsGroup_ptr);
            return true;
         }
         
      } // mode = MANIPULATE_FUSED_DATA conditional
   } // key down conditional
   
   return false;
}

