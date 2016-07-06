// ==========================================================================
// ROI_PolyhedraKeyHandler class member function definitions.  This class
// does not perform any useful function as of 1/24/07.
// ==========================================================================
// Last modified on 12/21/09
// ==========================================================================

#include "osg/osgRTPS/ROI_PolyhedraGroup.h"
#include "osg/osgRTPS/ROI_PolyhedraKeyHandler.h"
#include "osg/ModeController.h"

using std::cout;
using std::cin;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ROI_PolyhedraKeyHandler::allocate_member_objects()
{
}

void ROI_PolyhedraKeyHandler::initialize_member_objects()
{
   OSGsubPAT_number_to_toggle=0;
   ROI_PolyhedraGroup_ptr=NULL;
}

ROI_PolyhedraKeyHandler::ROI_PolyhedraKeyHandler(
   ROI_PolyhedraGroup* RPHG_ptr,ModeController* MC_ptr):
   PolyhedraKeyHandler(RPHG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ROI_PolyhedraGroup_ptr=RPHG_ptr;
}

ROI_PolyhedraKeyHandler::~ROI_PolyhedraKeyHandler()
{
}

// ------------------------------------------------------
bool ROI_PolyhedraKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::RUN_MOVIE)
      {

// For Bluegrass demo, press 't' to toggle on/off Activity Region
// bounding box ROI_polyhedra:

         if (ea.getKey()=='t')
         {
            ROI_PolyhedraGroup_ptr->toggle_OSGsubPAT_nodemask(
               OSGsubPAT_number_to_toggle);

            cout << "ROI_PolyhedraGroup_ptr = " << ROI_PolyhedraGroup_ptr 
                 << endl;
            cout << "OSGsubPAT_number_to_toggle = "
                 << OSGsubPAT_number_to_toggle << endl;
            return true;
         }

// Press "Delete" key to completely destroy a PolyLine:

//         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
//         {
//            ROI_PolyhedraGroup_ptr->destroy_Polyhdron();
//            return true;
//         }

      } // mode = RUN_MOVIE conditional

   } // key down conditional
   
   return false;
}


