// ==========================================================================
// PanoramasKeyHandler header file 
// ==========================================================================
// Last modified on 8/12/09; 1/10/10
// ==========================================================================

#ifndef PANORAMAS_KEYHANDLER_H
#define PANORAMAS_KEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"
#include "osg/osgAnnotators/SignPostsGroup.h"

class ModeController;
class PanoramasGroup;

class PanoramasKeyHandler : public GraphicalsKeyHandler
{
  public:

   PanoramasKeyHandler(PanoramasGroup* PG_ptr,ModeController* MC_ptr);
   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

// Set & get member functions:
   
   void set_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr);

  protected:

   virtual ~PanoramasKeyHandler();

  private:

   PanoramasGroup* PanoramasGroup_ptr;
   SignPostsGroup* SignPostsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PanoramasKeyHandler::set_SignPostsGroup_ptr(
   SignPostsGroup* SPG_ptr)
{
   SignPostsGroup_ptr=SPG_ptr;
}


#endif // PANORAMASKEYHANDLER.h
