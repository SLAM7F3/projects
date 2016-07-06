// ==========================================================================
// Header file for CYLINDERPICKHANDLER class
// ==========================================================================
// Last modfied on 4/30/08; 5/29/08; 2/9/11
// ==========================================================================

#ifndef CYLINDER_PICK_HANDLER_H
#define CYLINDER_PICK_HANDLER_H

#include "color/colorfuncs.h"
#include "osg/osgGeometry/GeometricalPickHandler.h"

class Cylinder;
class CylindersGroup;
class ModeController;
class PickHandlerCallbacks;
class WindowManager;

class CylinderPickHandler : public GeometricalPickHandler
{

  public: 

   CylinderPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,CylindersGroup* CG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr);

// Set and get methods:

   void set_text_screen_axis_alignment_flag(bool flag);
   void set_text_size(double size);
   void set_text_displacement(double d);
   double get_text_displacement() const;
   void set_pnt_on_Zplane_flag(bool flag);
   void set_permanent_color(colorfunc::Color c);
//    void set_selected_color(colorfunc::Color c);
   void set_PickHandlerCallbacks_ptr(PickHandlerCallbacks* PHCB_ptr);

// Cylinder generation, manipulation and annihiilation methods:

   bool instantiate_Cylinder(double X,double Y);
   bool select_Cylinder();
   bool select_Cylinder(int selected_Cylinder_ID);

  protected:

   bool pnt_on_Zplane_flag;

   virtual ~CylinderPickHandler();

// Mouse event handling member functions:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool rotate(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);
   virtual bool release();
   virtual bool toggle_rotate_mode();
   virtual bool toggle_scaling_mode();

// Bluegrass application member functions:

  private:

   bool text_screen_axis_alignment_flag;
   double text_size,text_displacement;
   colorfunc::Color permanent_color;
   CylindersGroup* CylindersGroup_ptr;
   PickHandlerCallbacks* PickHandlerCallbacks_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void CylinderPickHandler::set_text_screen_axis_alignment_flag(
   bool flag)
{
   text_screen_axis_alignment_flag=flag;
}

inline void CylinderPickHandler::set_text_size(double size)
{
   text_size=size;
}

inline void CylinderPickHandler::set_text_displacement(double d)
{
   text_displacement=d;
}

inline double CylinderPickHandler::get_text_displacement() const
{
   return text_displacement;
}

inline void CylinderPickHandler::set_pnt_on_Zplane_flag(bool flag)
{
   pnt_on_Zplane_flag=flag;
}

inline void CylinderPickHandler::set_permanent_color(colorfunc::Color c)
{
   permanent_color=c;
}

#endif // CylinderPickHandler.h



