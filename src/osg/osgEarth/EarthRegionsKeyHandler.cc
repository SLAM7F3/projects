// ==========================================================================
// EarthRegionsKeyHandler class member function definitions
// ==========================================================================
// Last modified on 5/20/08
// ==========================================================================

#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgEarth/EarthRegionsKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"

using std::cin;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void EarthRegionsKeyHandler::allocate_member_objects()
{
}

void EarthRegionsKeyHandler::initialize_member_objects()
{
   MoviesGroup_ptr=NULL;
}

EarthRegionsKeyHandler::EarthRegionsKeyHandler(
   EarthRegionsGroup* ERG_ptr,ModeController* MC_ptr,MoviesGroup* MG_ptr):
   GraphicalsKeyHandler(ERG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   EarthRegionsGroup_ptr=ERG_ptr;
   MoviesGroup_ptr=MG_ptr;
}

EarthRegionsKeyHandler::~EarthRegionsKeyHandler()
{
}

// ---------------------------------------------------------------------
EarthRegionsGroup* const EarthRegionsKeyHandler::get_EarthRegionsGroup_ptr()
{
   return EarthRegionsGroup_ptr;
}

// ------------------------------------------------------
bool EarthRegionsKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{

   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {

      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_EARTH)
      {
         if (ea.getKey()=='r')
         {
            cout << "Toggling road network" << endl;
            PolyLinesGroup* roadlines_group_ptr=
               EarthRegionsGroup_ptr->get_ID_labeled_EarthRegion_ptr(0)->
               get_roadlines_group_ptr();
            roadlines_group_ptr->toggle_OSGgroup_nodemask();

// As of 5/20/08, we asssume that the last Movie within
// *MoviesGroup_ptr corresponds to a surface texture which
// significantly overlaps the roads network contained in
// *roadlines_group_ptr.  We adjust the road network's altitude so
// that it lies slightly above the current surface texture's altitude:

            if (MoviesGroup_ptr != NULL)
            {
               Movie* movie_texture_ptr=dynamic_cast<Movie*>(
                  MoviesGroup_ptr->get_last_Graphical_ptr());

               osg::PositionAttitudeTransform* OSGsubPAT_ptr=
                  roadlines_group_ptr->get_OSGsubPAT_ptr(0);
               threevector posn(OSGsubPAT_ptr->getPosition());

               double curr_roadlines_height=
                  movie_texture_ptr->get_absolute_altitude(
                     MoviesGroup_ptr->get_curr_t(),
                     MoviesGroup_ptr->get_passnumber())+5;
               posn.put(2,curr_roadlines_height);

               OSGsubPAT_ptr->setPosition(osg::Vec3d(
                  posn.get(0),posn.get(1),posn.get(2)));
            }
          
            return true;
         }

      } // getState conditional
   } // key down conditional
   
   return false;
}


