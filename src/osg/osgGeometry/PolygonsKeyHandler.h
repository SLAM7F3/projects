// ==========================================================================
// PolygonsKeyHandler header file 
// ==========================================================================
// Last modified on 2/18/08
// ==========================================================================

#ifndef POLYGONSKEYHANDLER_H
#define POLYGONSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class ModeController;


namespace osgGeometry
{

// Forward declare PolygonsGroup class which sits inside osgGeometry
// namespace:

   class PolygonsGroup;

   class PolygonsKeyHandler : public GraphicalsKeyHandler
      {

        public:

         PolygonsKeyHandler(PolygonsGroup* PG_ptr,ModeController* MC_ptr);

         virtual bool handle( 
            const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

        protected:

         virtual ~PolygonsKeyHandler();

        private:

         PolygonsGroup* PolygonsGroup_ptr;

         void allocate_member_objects();
         void initialize_member_objects();
      }; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

} // osgGeometry namespace

#endif // PolygonsKeyhandler.h
