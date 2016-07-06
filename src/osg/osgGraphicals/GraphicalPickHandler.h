// Note added on 1/1/11: CM_ptr already contains ModeController and
// WindowManager pointers.  So there is no need to explicitly pass
// these pointers in addition to CM_ptr !!!

// ==========================================================================
// Header file for pure virtual GRAPHICALPICKHANDLER class
// ==========================================================================
// Last modfied on 5/26/10; 2/9/11; 3/20/11; 6/7/14
// ==========================================================================

#ifndef GRAPHICAL_PICK_HANDLER_H
#define GRAPHICAL_PICK_HANDLER_H

#include <iostream>
#include <string>
#include <osgGA/GUIEventAdapter>
#include <osgUtil/IntersectVisitor>
#include <osg/Node>

#include "osg/Custom3DManipulator.h"
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "osg/ModeController.h"
#include "osg/osgSceneGraph/ParentVisitor.h"
#include "passes/PassesGroup.h"
#include "osg/PickHandler.h"
#include "math/threevector.h"

// class osgUtil::Hit;
class Graphical;
class Messenger;
class twovector;
class WindowManager;

class GraphicalPickHandler : public PickHandler
{

  public: 

   GraphicalPickHandler(
      const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
      GraphicalsGroup* GG_ptr,ModeController* MC_ptr,
      WindowManager* WCC_ptr,
      threevector* GO_ptr=NULL,osg::Node* DN_ptr=NULL);

// Set & get methods:

   void set_disable_input_flag(bool flag);
   bool get_disable_input_flag() const;
   void set_selected_Graphical_ID(int n);
   int get_selected_Graphical_ID() const;
   void set_DataNode_ptr(osg::Node* node_ptr);
   void set_Messenger_ptr(Messenger* M_ptr);
   Messenger* get_Messenger_ptr();
   const Messenger* get_Messenger_ptr() const;
   void set_surface_picking_flag(bool flag);
   void set_cloud_picking_flag(bool flag);
   void set_Zplane_picking_flag(bool flag);

// Graphical generation & manipulation methods:

   threevector instantiate_Graphical(Graphical* curr_Graphical_ptr);

   float sqrd_screen_dist(const threevector& V1,const threevector& V2);
   virtual float get_max_distance_to_Graphical()=0;
   int select_Graphical();

  protected:

   bool two_mouse_posns_detected;
   Pass* pass_ptr;
   threevector prev_voxel_screenspace_posn,curr_voxel_screenspace_posn;
   threevector curr_voxel_worldspace_posn;
   threevector curr_screenspace_rotation,curr_screenspace_scale;
   osg::ref_ptr<ParentVisitor> ParentVisitor_refptr;
   Messenger* Messenger_ptr;

   virtual ~GraphicalPickHandler()=0;
   double get_curr_t() const;
   int get_passnumber() const;
   threevector get_grid_origin();
   osgGA::CustomManipulator* get_CustomManipulator_ptr();
   osgGA::Custom3DManipulator* get_CM_3D_ptr();
   GraphicalsGroup* get_GraphicalsGroup_ptr();
   const GraphicalsGroup* get_GraphicalsGroup_ptr() const;
   ModeController* get_ModeController_ptr();
   const ModeController* get_ModeController_ptr() const;

// Mouse pick event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool pick_3D_point(float X,float Y);
   virtual bool pick_point_on_Zplane(float X,float Y,double Zplane);

   bool pick_3D_box(float oldX, float oldY, float X,float Y);
   virtual bool pick_box(
      float oldX, float oldY, const osgGA::GUIEventAdapter& ea);

   void set_pick_handler_voxel_coords();

// Mouse dragging event handling methods:

   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool drag_box(
      float oldX, float oldY,const osgGA::GUIEventAdapter& ea);

// Mouse double click event handling methods:

   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);

// Mouse scaling event handling methods:

   virtual bool toggle_scaling_mode();
   virtual bool scale(const osgGA::GUIEventAdapter& ea);
   virtual bool scale(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);
   bool scale(float X,float Y,float oldX,float oldY);
   bool scale_Graphical(float deltaX,float deltaY);
   
// Mouse rotating event handling methods:

   virtual bool toggle_rotate_mode();
   virtual bool rotate(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);
   virtual bool rotate_Graphical();

// Intersection methods:

   bool compute_intersections(float X,float Y);

  private:

   bool disable_input_flag;
   bool surface_picking_flag,cloud_picking_flag,Zplane_picking_flag;
   int *passnumber_ptr,*Nimages_ptr,*imagenumber_ptr;
   GraphicalsGroup* GraphicalsGroup_ptr;
   ModeController* ModeController_ptr;
   WindowManager* WindowManager_ptr;
   threevector* grid_origin_ptr;
   osg::Vec3 world_coords;
   osg::ref_ptr<osgGA::CustomManipulator> CustomManipulator_refptr;
   osg::ref_ptr<osg::Node> DataNode_refptr;

   void allocate_member_objects();
   void initialize_member_objects();

   virtual bool move_Graphical();
   bool find_closest_Graphical_nearby_mouse_posn(int& closest_Graphical_ID);
   bool pick_2D_box(float oldX, float oldY, float X,float Y);
   bool drag_2D_box(float oldX,float oldY,float X,float Y);
   bool drag_3D_box(float oldX,float oldY,float X,float Y);

// Intersection methods:


   bool closest_intersection_hit(float X,float Y,osgUtil::Hit& hit);
   bool ComputeIntersectionsFromViewerCode(
      const twovector& pixel_mouse_coords,
      osgUtil::IntersectVisitor::HitList& hitlist,
      osg::Node::NodeMask traversalMask = 0xffffffff);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void GraphicalPickHandler::set_disable_input_flag(bool flag)
{
   disable_input_flag=flag;
}

inline bool GraphicalPickHandler::get_disable_input_flag() const
{
   return disable_input_flag;
}

inline void GraphicalPickHandler::set_selected_Graphical_ID(int n)
{
   get_GraphicalsGroup_ptr()->set_selected_Graphical_ID(n);
}

inline int GraphicalPickHandler::get_selected_Graphical_ID() const
{
   return get_GraphicalsGroup_ptr()->get_selected_Graphical_ID();
}

// Method sqrd_screen_dist returns the (X1-X2)**2+(Y1-Y2)**2 where Xn
// and Yn represent screen coordinates:

inline float GraphicalPickHandler::sqrd_screen_dist(
   const threevector& V1,const threevector& V2)
{
   return sqr(V1.get(0)-V2.get(0))+sqr(V1.get(1)-V2.get(1));
}

// ---------------------------------------------------------------------
inline ModeController* GraphicalPickHandler::get_ModeController_ptr() 
{
   return ModeController_ptr;
}

inline const ModeController* GraphicalPickHandler::get_ModeController_ptr() 
   const
{
   return ModeController_ptr;
}

// --------------------------------------------------------------------------
inline double GraphicalPickHandler::get_curr_t() const
{
   return get_GraphicalsGroup_ptr()->get_curr_t();
}

// --------------------------------------------------------------------------
inline int GraphicalPickHandler::get_passnumber() const
{
   return get_GraphicalsGroup_ptr()->get_passnumber();
}

// ---------------------------------------------------------------------
inline osgGA::CustomManipulator* 
GraphicalPickHandler::get_CustomManipulator_ptr()
{
   return CustomManipulator_refptr.get();
}

// ---------------------------------------------------------------------
inline osgGA::Custom3DManipulator* GraphicalPickHandler::get_CM_3D_ptr()
{
   return dynamic_cast<osgGA::Custom3DManipulator*>(
      get_CustomManipulator_ptr());

}

// ---------------------------------------------------------------------
inline GraphicalsGroup* GraphicalPickHandler::get_GraphicalsGroup_ptr() 
{
   return GraphicalsGroup_ptr;
}

inline const GraphicalsGroup* GraphicalPickHandler::get_GraphicalsGroup_ptr() 
   const
{
   return GraphicalsGroup_ptr;
}

// ---------------------------------------------------------------------
inline void GraphicalPickHandler::set_DataNode_ptr(osg::Node* node_ptr)
{
   DataNode_refptr=node_ptr;
}

// ---------------------------------------------------------------------
inline void GraphicalPickHandler::set_surface_picking_flag(bool flag)
{
   surface_picking_flag=flag;
}

inline void GraphicalPickHandler::set_cloud_picking_flag(bool flag)
{
   cloud_picking_flag=flag;
}

inline void GraphicalPickHandler::set_Zplane_picking_flag(bool flag)
{
   Zplane_picking_flag=flag;
}

#endif // GraphicalPickHandler.h



