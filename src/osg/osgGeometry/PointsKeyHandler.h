// ==========================================================================
// PointsKeyHandler header file 
// ==========================================================================
// Last modified on 7/10/06; 1/3/07; 6/21/07
// ==========================================================================

#ifndef POINTSKEYHANDLER_H
#define POINTSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

// class osgGeometry::PointsGroup;
class ModeController;

namespace osgGeometry
{

   class PointsKeyHandler : public GraphicalsKeyHandler
      {
        public:

         PointsKeyHandler(const int p_ndims,PointsGroup* PG_ptr,
                          ModeController* MC_ptr);


         PointsGroup* const get_PointsGroup_ptr() const;

         virtual bool handle( 
            const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

        protected:

         virtual ~PointsKeyHandler();

        private:

         int ndims;
         PointsGroup* PointsGroup_ptr;

         void allocate_member_objects();
         void initialize_member_objects();
      }; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

   inline PointsGroup* const PointsKeyHandler::get_PointsGroup_ptr() const
      {
         return PointsGroup_ptr;
      }

}

#endif 
