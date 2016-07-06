// ==========================================================================
// Header file for BOXPICKHANDLER class
// ==========================================================================
// Last modfied on 6/16/07; 9/2/08; 2/9/11
// ==========================================================================

#ifndef BOX_PICK_HANDLER_H
#define BOX_PICK_HANDLER_H

#include "datastructures/Linkedlist.h"
#include "osg/osgGeometry/GeometricalPickHandler.h"

class Box;
class BoxesGroup;
class ModeController;
class WindowManager;

class BoxPickHandler : public GeometricalPickHandler
{

  public: 

   BoxPickHandler(
      Pass* PI_ptr,
      osgGA::CustomManipulator* CM_ptr,BoxesGroup* BG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr);

// Set and get methods:

   void set_pnt_on_Zplane_flag(bool flag);

// Box generation, manipulation and annihiilation methods:

   bool scale_Box(float X,float Y,float oldX,float oldY);

  protected:

   bool pnt_on_Zplane_flag;

   virtual ~BoxPickHandler();

   BoxesGroup* get_BoxesGroup_ptr();
   Linkedlist<Box*>* get_Boxlist_ptr();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
//    bool pick_3D_point(float X,float Y);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool rotate(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);
   virtual bool scale(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);
   virtual bool release();
   virtual bool toggle_rotate_mode();
   virtual bool toggle_scaling_mode();

  private:

   BoxesGroup* BoxesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   bool instantiate_Box(double X,double Y);
   bool select_Box(double X,double Y);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void BoxPickHandler::set_pnt_on_Zplane_flag(bool flag)
{
   pnt_on_Zplane_flag=flag;
}


#endif // BoxPickHandler.h



