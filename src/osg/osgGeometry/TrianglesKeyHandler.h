// ==========================================================================
// TrianglesKeyHandler header file 
// ==========================================================================
// Last modified on 12/18/05; 1/3/07
// ==========================================================================

#ifndef TRIANGLESKEYHANDLER_H
#define TRIANGLESKEYHANDLER_H

#include <osgGA/GUIEventHandler>
#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class TrianglesGroup;
class ModeController;

class TrianglesKeyHandler : public GraphicalsKeyHandler
{
  public:

   TrianglesKeyHandler(TrianglesGroup* TG_ptr,ModeController* MC_ptr);


   TrianglesGroup* const get_TrianglesGroup_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~TrianglesKeyHandler();

  private:

   TrianglesGroup* TrianglesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif 
