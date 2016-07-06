// ==========================================================================
// LOSMODELSKeyHandler header file 
// ==========================================================================
// Last modified on 9/19/09; 9/28/09
// ==========================================================================

#ifndef LOSMODELSKEYHANDLER_H
#define LOSMODELSKEYHANDLER_H

#include "osg/osgModels/MODELSKeyHandler.h"
#include "osg/osg3D/PointCloudsGroup.h"

class LOSMODELSGROUP;
class ModeController;

class LOSMODELSKeyHandler : public MODELSKeyHandler
{
  public:

   LOSMODELSKeyHandler(LOSMODELSGROUP* MG_ptr,ModeController* MC_ptr);

   void set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr);
   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& ga);

  protected:

   virtual ~LOSMODELSKeyHandler();

  private:

   LOSMODELSGROUP* LOSMODELSGROUP_ptr;
   PointCloudsGroup* PointCloudsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void LOSMODELSKeyHandler::set_PointCloudsGroup_ptr(
   PointCloudsGroup* PCG_ptr)
{
   PointCloudsGroup_ptr=PCG_ptr;
}

#endif 
