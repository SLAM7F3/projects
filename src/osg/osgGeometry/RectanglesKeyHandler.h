// ==========================================================================
// RectanglesKeyHandler header file 
// ==========================================================================
// Last modified on 12/6/05; 11/2/06; 12/25/06; 1/3/07; 7/9/11
// ==========================================================================

#ifndef RECTANGLESKEYHANDLER_H
#define RECTANGLESKEYHANDLER_H

#include "osg/osgGeometry/GeometricalsKeyHandler.h"

class ModeController;
class RectanglesGroup;

class RectanglesKeyHandler : public GeometricalsKeyHandler
{

  public:

   RectanglesKeyHandler(RectanglesGroup* RG_ptr,ModeController* MC_ptr);


   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~RectanglesKeyHandler();

  private:

   RectanglesGroup* RectanglesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif 
