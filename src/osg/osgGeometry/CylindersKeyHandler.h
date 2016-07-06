// ==========================================================================
// CylindersKeyHandler header file 
// ==========================================================================
// Last modified on 1/24/07; 5/29/08
// ==========================================================================

#ifndef CYLINDERSKEYHANDLER_H
#define CYLINDERSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"
#include "osg/osgGeometry/CylinderPickHandler.h"

class CylindersGroup;
class ModeController;

class CylindersKeyHandler : public GraphicalsKeyHandler
{
  public:

   CylindersKeyHandler(CylindersGroup* CG_ptr,ModeController* MC_ptr);

   CylindersGroup* const get_CylindersGroup_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

// Set & get member functions

   void set_CylinderPickHandler_ptr(CylinderPickHandler* CPH_ptr);

  protected:

   virtual ~CylindersKeyHandler();

  private:

   CylindersGroup* CylindersGroup_ptr;
   CylinderPickHandler* CylinderPickHandler_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void CylindersKeyHandler::set_CylinderPickHandler_ptr(
   CylinderPickHandler* CPH_ptr)
{
   CylinderPickHandler_ptr=CPH_ptr;
}


#endif 
