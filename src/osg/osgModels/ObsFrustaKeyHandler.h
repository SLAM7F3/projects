// ==========================================================================
// ObsFrustaKeyHandler header file 
// ==========================================================================
// Last modified on 2/27/07; 3/5/07; 8/19/07
// ==========================================================================

#ifndef OBSFRUSTAKEYHANDLER_H
#define OBSFRUSTAKEYHANDLER_H

#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class ModeController;
class ObsFrustaGroup;

class ObsFrustaKeyHandler : public GraphicalsKeyHandler
{
  public:

   ObsFrustaKeyHandler(ObsFrustaGroup* OFG_ptr,ModeController* MC_ptr);
   ObsFrustaKeyHandler(ObsFrustaGroup* OFG_ptr,ModeController* MC_ptr,
                       osgGA::Terrain_Manipulator* AM_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~ObsFrustaKeyHandler();

  private:

   int n_still_images,n_frustum,toggle_counter;
   osgGA::Terrain_Manipulator* Terrain_Manipulator_ptr;
   ObsFrustaGroup* ObsFrustaGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   Movie* get_Movie_ptr();
}; 

#endif 
