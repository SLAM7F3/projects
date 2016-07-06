// ==========================================================================
// Header file for PYRAMIDPICKHANDLER class
// ==========================================================================
// Last modfied on 7/21/07; 9/2/08
// ==========================================================================

#ifndef PYRAMID_PICK_HANDLER_H
#define PYRAMID_PICK_HANDLER_H

#include "osg/osgGeometry/GeometricalPickHandler.h"

class Pyramid;
class PyramidsGroup;
class ModeController;
class WindowManager;

class PyramidPickHandler : public GeometricalPickHandler
{

  public: 

   PyramidPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,PyramidsGroup* CG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr);

// Set and get methods:

   void set_pnt_on_Zplane_flag(bool flag);

// Pyramid generation, manipulation and annihiilation methods:

   virtual bool rotate_Graphical();

  protected:

   bool pnt_on_Zplane_flag;

   virtual ~PyramidPickHandler();

   PyramidsGroup* get_PyramidsGroup_ptr();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool rotate(const osgGA::GUIEventAdapter& ea);
   virtual bool release();
   virtual bool toggle_rotate_mode();
   virtual bool toggle_scaling_mode();

  private:

   PyramidsGroup* PyramidsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   bool instantiate_Pyramid(double X,double Y);
   bool select_Pyramid(double X,double Y);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PyramidPickHandler::set_pnt_on_Zplane_flag(bool flag)
{
   pnt_on_Zplane_flag=flag;
}


#endif // PyramidPickHandler.h



