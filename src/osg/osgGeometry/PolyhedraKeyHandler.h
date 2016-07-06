// ==========================================================================
// PolyhedraKeyHandler header file 
// ==========================================================================
// Last modified on 4/25/08; 5/1/08
// ==========================================================================

#ifndef POLYHEDRAKEYHANDLER_H
#define POLYHEDRAKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class ModeController;
class PolyhedraGroup;

class PolyhedraKeyHandler : public GraphicalsKeyHandler
{

  public:

   PolyhedraKeyHandler(PolyhedraGroup* PHG_ptr,ModeController* MC_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

   void set_OSGsubPAT_number_to_toggle(int n);
   
  protected:

   int OSGsubPAT_number_to_toggle;
   virtual ~PolyhedraKeyHandler();

  private:

   PolyhedraGroup* PolyhedraGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PolyhedraKeyHandler::set_OSGsubPAT_number_to_toggle(int n)
{
   OSGsubPAT_number_to_toggle=n;
}


#endif 
