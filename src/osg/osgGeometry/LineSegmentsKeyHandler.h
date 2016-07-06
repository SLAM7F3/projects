// ==========================================================================
// LineSegmentsKeyHandler header file 
// ==========================================================================
// Last modified on 9/20/05; 12/18/05; 1/3/07
// ==========================================================================

#ifndef LINESEGMENTSKEYHANDLER_H
#define LINESEGMENTSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class LineSegmentsGroup;
class ModeController;

class LineSegmentsKeyHandler : public GraphicalsKeyHandler
{
  public:

   LineSegmentsKeyHandler(LineSegmentsGroup* LSG_ptr,ModeController* MC_ptr);

   LineSegmentsGroup* const get_LineSegmentsGroup_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~LineSegmentsKeyHandler();

  private:

   LineSegmentsGroup* LineSegmentsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif 
