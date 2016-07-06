// ==========================================================================
// Header file for RECTANGLEPICKHANDLER class
// ==========================================================================
// Last modfied on 6/15/08; 9/2/08; 2/9/11
// ==========================================================================

#ifndef RECTANGLE_PICK_HANDLER_H
#define RECTANGLE_PICK_HANDLER_H

#include "osg/osgGeometry/GeometricalPickHandler.h"
#include "datastructures/Linkedlist.h"

class ModeController;
class Rectangle;
class RectanglesGroup;
class WindowManager;

class RectanglePickHandler : public GeometricalPickHandler
{

  public: 

   RectanglePickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,RectanglesGroup* RG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr);

// Rectangle generation, manipulation and annihiilation methods:

  protected:

   virtual ~RectanglePickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool rotate(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);
   virtual bool release();
   virtual bool toggle_rotate_mode();

  private:

   RectanglesGroup* RectanglesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   RectanglesGroup* get_RectanglesGroup_ptr();
   Linkedlist<Rectangle*>* get_Rectanglelist_ptr();
   Rectangle* get_rectangle_ptr();

   bool instantiate_Rectangle();
   bool select_Rectangle();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline Rectangle* RectanglePickHandler::get_rectangle_ptr() 
{
//   std::cout << "inside RPH::get_rectangle_ptr()" << std::endl;
   int rect_number=get_RectanglesGroup_ptr()->
      get_selected_Graphical_ID();
   if (rect_number < 0)
   {
      return NULL;
   }
   else
   {
      return dynamic_cast<Rectangle*>(
         get_RectanglesGroup_ptr()->
         get_ID_labeled_Graphical_ptr(rect_number));
   }
}


#endif // RectanglePickHandler.h



