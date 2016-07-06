// ==========================================================================
// Header file for POLYGONPICKHANDLER class
// ==========================================================================
// Last modfied on 2/18/08
// ==========================================================================

#ifndef POLYGON_PICK_HANDLER_H
#define POLYGON_PICK_HANDLER_H

#include "osg/osgGeometry/GeometricalPickHandler.h"

class Polygon;
class PolygonsGroup;
class ModeController;
class WindowManager;

namespace osgGeometry
{
   
   class PolygonPickHandler : public GeometricalPickHandler
      {

        public: 

         PolygonPickHandler(
            Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
            PolygonsGroup* PG_ptr,ModeController* MC_ptr,
            WindowManager* WCC_ptr,threevector* GO_ptr);

// Set and get methods:

         void set_text_size(double size);
         void set_pnt_on_Zplane_flag(bool flag);

// Polygon generation, manipulation and annihiilation methods:

         bool select_Polygon(double X,double Y);

        protected:

//         bool Polygon_continuing_flag;
         bool pnt_on_Zplane_flag;

         virtual ~PolygonPickHandler();

// Mouse event handling methods:

         virtual bool pick(const osgGA::GUIEventAdapter& ea);
         virtual bool drag(const osgGA::GUIEventAdapter& ea);
         virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
         virtual bool release();

        private:

         int curr_Polygon_ID;
         double text_size;
         PolygonsGroup* PolygonsGroup_ptr;
         Polygon* curr_Polygon_ptr;
         osg::Vec4 permanent_color,selected_color;
         std::vector<threevector> V;
         std::vector<osg::Vec4> colors;

         void allocate_member_objects();
         void initialize_member_objects();
      };

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

   inline void PolygonPickHandler::set_text_size(double size)
      {
         text_size=size;
      }

   inline void PolygonPickHandler::set_pnt_on_Zplane_flag(bool flag)
      {
         pnt_on_Zplane_flag=flag;
      }

} // osgGeometry namespace

#endif // PolygonPickHandler.h



