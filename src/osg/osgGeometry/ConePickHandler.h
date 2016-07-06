// ==========================================================================
// Header file for CONEPICKHANDLER class
// ==========================================================================
// Last modfied on 6/16/07; 9/2/08; 2/9/11
// ==========================================================================

#ifndef CONE_PICK_HANDLER_H
#define CONE_PICK_HANDLER_H

#include "osg/osgGeometry/GeometricalPickHandler.h"

class Cone;
class ConesGroup;
class ModeController;
class WindowManager;

class ConePickHandler : public GeometricalPickHandler
{

  public: 

   ConePickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,ConesGroup* CG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr);

// Set and get methods:

   void set_pnt_on_Zplane_flag(bool flag);

// Cone generation, manipulation and annihiilation methods:

   virtual bool rotate_Graphical();

  protected:

   bool pnt_on_Zplane_flag;

   virtual ~ConePickHandler();

   ConesGroup* get_ConesGroup_ptr();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool rotate(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);
   virtual bool release();
   virtual bool toggle_rotate_mode();
   virtual bool toggle_scaling_mode();

  private:

   ConesGroup* ConesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   bool instantiate_Cone(double X,double Y);
   bool select_Cone(double X,double Y);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ConePickHandler::set_pnt_on_Zplane_flag(bool flag)
{
   pnt_on_Zplane_flag=flag;
}


#endif // ConePickHandler.h



