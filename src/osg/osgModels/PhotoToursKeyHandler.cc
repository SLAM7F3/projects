// ==========================================================================
// PhotoToursKeyHandler class member function definitions
// ==========================================================================
// Last modified on 2/28/10; 3/1/10
// ==========================================================================

#include <iostream>
#include <vector>
#include "osg/ModeController.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgModels/PhotoToursGroup.h"
#include "osg/osgModels/PhotoToursKeyHandler.h"
#include "osg/osg3D/PointCloudsGroup.h"

using std::cin;
using std::cout;
using std::endl;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PhotoToursKeyHandler::allocate_member_objects()
{
}

void PhotoToursKeyHandler::initialize_member_objects()
{
}

PhotoToursKeyHandler::PhotoToursKeyHandler(
   PhotoToursGroup* PTG_ptr,OBSFRUSTAGROUP* OG_ptr,
   PointCloudsGroup* PCG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(PTG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   OBSFRUSTAGROUP_ptr=OG_ptr;
   PhotoToursGroup_ptr=PTG_ptr;
   PointCloudsGroup_ptr=PCG_ptr;
}

PhotoToursKeyHandler::~PhotoToursKeyHandler()
{
}
 
// ------------------------------------------------------
bool PhotoToursKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::RUN_MOVIE)
      {
         if (ea.getKey()=='a')
         {

            vector<int> tour_photo_IDs;

// Marriott tour:

            tour_photo_IDs.push_back(200);
            tour_photo_IDs.push_back(1615);
            tour_photo_IDs.push_back(816);
            tour_photo_IDs.push_back(836);
            tour_photo_IDs.push_back(830);
            tour_photo_IDs.push_back(191);
            tour_photo_IDs.push_back(190);
            tour_photo_IDs.push_back(215);
            tour_photo_IDs.push_back(1640);

            PhotoToursGroup_ptr->generate_specified_tour(tour_photo_IDs);
            return true;
         }
         else if (ea.getKey()=='c')
         {
            cout << "Before call to toggle OBSFRUSTAGROUP nodemask" << endl;
            OBSFRUSTAGROUP_ptr->toggle_OSGgroup_nodemask();
            return true;
         }
         else if (ea.getKey()=='d')
         {
            cout << "Before erasing ladar points" << endl;
            PointCloudsGroup_ptr->erase_all_Graphicals();
            PointCloudsGroup_ptr->unerase_Graphical(0);
            return true;
         }
         else if (ea.getKey()=='e')
         {
            cout << "Before unerasing ladar points" << endl;
            PointCloudsGroup_ptr->unerase_all_Graphicals();
            return true;
         }
      } // mode = RUN_MOVIE conditional
   } // key down conditional
   
   return false;
}

