// ==========================================================================
// Header file for POINTPICKHANDLER class
// ==========================================================================
// Last modfied on 1/21/07; 6/21/07; 8/26/07; 9/2/08
// ==========================================================================

#ifndef POINT_PICK_HANDLER_H
#define POINT_PICK_HANDLER_H

#include <iostream>
#include "osg/osgGeometry/GeometricalPickHandler.h"

// class osgGeometry::Point;
// class osgGeometry::PointsGroup;
class ModeController;
class WindowManager;

namespace osgGeometry
{

   class PointPickHandler : public GeometricalPickHandler
      {

        public: 

         PointPickHandler(
            const int p_ndims,Pass* PI_ptr,
            osgGA::CustomManipulator* CM_ptr,PointsGroup* PG_ptr,
            ModeController* MC_ptr,WindowManager* WCC_ptr,
            threevector* GO_ptr);
         virtual ~PointPickHandler();

         virtual bool release();

// Point generation, manipulation and annihilation methods:

      
        protected:

         PointsGroup* const get_PointsGroup_ptr();

// Mouse event handling methods:

         virtual bool pick(const osgGA::GUIEventAdapter& ea);
         virtual bool drag(const osgGA::GUIEventAdapter& ea);

        private:

         PointsGroup* PointsGroup_ptr;

         void allocate_member_objects();
         void initialize_member_objects();

         bool instantiate_point();
         bool select_point();
      };

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

   inline PointsGroup* const PointPickHandler::get_PointsGroup_ptr()
      {
         return PointsGroup_ptr;
      }

} // osgGeometry namespace

#endif // PointPickHandler.h



