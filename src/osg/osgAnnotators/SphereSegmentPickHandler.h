// ==========================================================================
// Header file for SPHERESEGMENTPICKHANDLER class
// ==========================================================================
// Last modfied on 1/21/07; 2/6/07; 2/9/11
// ==========================================================================

#ifndef SPHERESEGMENT_PICK_HANDLER_H
#define SPHERESEGMENT_PICK_HANDLER_H

#include "osg/osgGeometry/GeometricalPickHandler.h"

// class osg::Group;
class ModeController;
class SphereSegment;
class SphereSegmentsGroup;
class WindowManager;

class SphereSegmentPickHandler : public GeometricalPickHandler
{

  public: 

   SphereSegmentPickHandler(
      Pass* PI_ptr,
      osgGA::CustomManipulator* CM_ptr,SphereSegmentsGroup* SSG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr,
      double radius,double az_min,double az_max,double el_min,double el_max,
      bool display_spokes_flag,bool include_blast_flag);

// SphereSegment generation, manipulation and annihilation methods:

   bool instantiate_SphereSegment(double X,double Y);
   bool select_SphereSegment();

  protected:

   virtual ~SphereSegmentPickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool toggle_rotate_mode();
   virtual bool rotate(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);
   virtual bool release();
      
  private:

   bool display_spokes_flag,include_blast_flag;
   double SphereSegmentRadius,az_min,az_max,el_min,el_max;
   SphereSegmentsGroup* SphereSegmentsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // SphereSegmentPickHandler.h



