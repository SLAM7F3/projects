// ==========================================================================
// PhotoToursKeyHandler header file 
// ==========================================================================
// Last modified on 2/28/10
// ==========================================================================

#ifndef PHOTOTOURSKEYHANDLER_H
#define PHOTOTOURSKEYHANDLER_H

#include <vector>
#include "osg/osgGraphicals/GraphicalsKeyHandler.h"


class ModeController;
class OBSFRUSTAGROUP;
class PhotoToursGroup;
class PointCloudsGroup;

class PhotoToursKeyHandler : public GraphicalsKeyHandler
{
  public:

   PhotoToursKeyHandler(PhotoToursGroup* PTG_ptr,OBSFRUSTAGROUP* OG_ptr,
                        PointCloudsGroup* PCG_ptr,ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

// Set & get member functions:


  protected:

   virtual ~PhotoToursKeyHandler();

  private:

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;
   PhotoToursGroup* PhotoToursGroup_ptr;
   PointCloudsGroup* PointCloudsGroup_ptr;
   
   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // PHOTOTOURSKEYHANDLER.h
