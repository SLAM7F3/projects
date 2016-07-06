// ==========================================================================
// Header file for POLYHEDRONPICKHANDLER class
// ==========================================================================
// Last modfied on 4/26/09; 4/27/09; 5/4/09
// ==========================================================================

#ifndef POLYHEDRON_PICK_HANDLER_H
#define POLYHEDRON_PICK_HANDLER_H

#include "color/colorfuncs.h"
#include "osg/osgGeometry/GeometricalPickHandler.h"

class Polyhedron;
class PolyhedraGroup;
class ModeController;
class WindowManager;

class PolyhedronPickHandler : public GeometricalPickHandler
{

  public: 

   PolyhedronPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,PolyhedraGroup* CG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr);

// Set and get methods:

   void set_Polyhedron_color(colorfunc::Color color);
   void set_text_size(double size);
   void set_z_offset(double z);
   void set_pnt_on_Zplane_flag(bool flag);
   void set_permanent_color(const colorfunc::Color& c,double alpha=1.0);
   void set_permanent_color(const osg::Vec4& color);

// Polyhedron generation, manipulation and annihilation methods:

  protected:

   bool pnt_on_Zplane_flag;

   virtual ~PolyhedronPickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool release();

  private:

   int curr_Polyhedron_ID;

   double text_size,z_offset;
   PolyhedraGroup* PolyhedraGroup_ptr;
   Polyhedron* curr_Polyhedron_ptr;
   osg::Vec4 permanent_color,selected_color,Polyhedron_color;

   void allocate_member_objects();
   void initialize_member_objects();

   bool instantiate_Polyhedron(double X,double Y);
   bool select_Polyhedron();
   bool select_Polyhedron(int select_Polyhedron_ID);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:


inline void PolyhedronPickHandler::set_Polyhedron_color(
   colorfunc::Color color)
{
   Polyhedron_color=colorfunc::get_OSG_color(color);
}

inline void PolyhedronPickHandler::set_text_size(double size)
{
   text_size=size;
}

inline void PolyhedronPickHandler::set_z_offset(double z)
{
   z_offset=z;
}

inline void PolyhedronPickHandler::set_pnt_on_Zplane_flag(bool flag)
{
   pnt_on_Zplane_flag=flag;
}

inline void PolyhedronPickHandler::set_permanent_color(
   const colorfunc::Color& c,double alpha)
{
   set_permanent_color(colorfunc::get_OSG_color(c,alpha));
}

inline void PolyhedronPickHandler::set_permanent_color(const osg::Vec4& color)
{
   permanent_color=color;
}


#endif // PolyhedronPickHandler.h



