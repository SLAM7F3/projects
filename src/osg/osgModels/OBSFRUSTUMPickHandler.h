// ==========================================================================
// Header file for OBSFRUSTUMPICKHANDLER class
// ==========================================================================
// Last modfied on 9/2/08; 1/14/09; 3/19/09; 8/13/09
// ==========================================================================

#ifndef OBS_FRUSTUM_PICK_HANDLER_H
#define OBS_FRUSTUM_PICK_HANDLER_H

#include "osg/osgGrid/Grid.h"
#include "osg/osgGeometry/GeometricalPickHandler.h"

// class osg::Group;
class ModeController;
class OBSFRUSTUM;
class OBSFRUSTAGROUP;
class WindowManager;

class OBSFRUSTUMPickHandler : public GeometricalPickHandler
{

  public: 

   OBSFRUSTUMPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,OBSFRUSTAGROUP* OFG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr);

   virtual ~OBSFRUSTUMPickHandler();

// Set & get member functions:

   void set_mask_nonselected_OSGsubPATs_flag(bool flag);
   void set_Grid_ptr(Grid* Grid_ptr);
   void set_twoD_photo_picking_flag(bool flag);
   void set_disallow_OBSFRUSTUM_doubleclicking_flag(bool flag);

// OBSFRUSTUM generation, manipulation and annihilation methods:

  protected:

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool release();
      
  private:

   bool twoD_photo_picking_flag,mask_nonselected_OSGsubPATs_flag;
   bool disallow_OBSFRUSTUM_doubleclicking_flag;
   int rotate_about_curr_eyept_counter;
   Grid* Grid_ptr;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   double ray_angle_wrt_camera_posn(
      const threevector& V_world_posn,const threevector& camera_screen_posn);

   bool select_OBSFRUSTUM();
   bool select_OBSFRUSTUM(double U,double V);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void OBSFRUSTUMPickHandler::set_mask_nonselected_OSGsubPATs_flag(
   bool flag)
{
   mask_nonselected_OSGsubPATs_flag=flag;
}

inline void OBSFRUSTUMPickHandler::set_Grid_ptr(Grid* Grid_ptr)
{
   this->Grid_ptr=Grid_ptr;
}

inline void OBSFRUSTUMPickHandler::set_twoD_photo_picking_flag(bool flag)
{
   twoD_photo_picking_flag=flag;
}

inline void 
OBSFRUSTUMPickHandler::set_disallow_OBSFRUSTUM_doubleclicking_flag(bool flag)
{
   disallow_OBSFRUSTUM_doubleclicking_flag=flag;
}

#endif // OBSFRUSTUMPickHandler.h



