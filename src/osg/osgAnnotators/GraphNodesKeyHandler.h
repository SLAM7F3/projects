// ==========================================================================
// GraphNodesKeyHandler header file 
// ==========================================================================
// Last modified on 11/10/06; 12/11/06; 1/3/07
// ==========================================================================

#ifndef GRAPHNODESKEYHANDLER_H
#define GRAPHNODESKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class GraphNodesGroup;
class ModeController;

class GraphNodesKeyHandler : public GraphicalsKeyHandler
{
  public:

   GraphNodesKeyHandler(GraphNodesGroup* GNG_ptr,ModeController* MC_ptr);


   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~GraphNodesKeyHandler();

  private:

   GraphNodesGroup* GraphNodesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif 
