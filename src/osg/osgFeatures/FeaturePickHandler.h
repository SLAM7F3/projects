// ==========================================================================
// Header file for FEATUREPICKHANDLER class
// ==========================================================================
// Last modfied on 8/27/07; 9/2/08; 2/9/11; 10/16/13
// ==========================================================================

#ifndef FEATURE_PICK_HANDLER_H
#define FEATURE_PICK_HANDLER_H

#include <iostream>
#include "osg/osgGeometry/PointPickHandler.h"

class Feature;
class FeaturesGroup;
class ModeController;
class OBSFRUSTAGROUP;
class WindowManager;

class FeaturePickHandler : public osgGeometry::PointPickHandler
{

  public: 

   FeaturePickHandler(
      const int p_ndims,Pass* PI_ptr,
      osgGA::CustomManipulator* CM_ptr,FeaturesGroup* FG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr);
   FeaturePickHandler(
      const int p_ndims,Pass* PI_ptr,
      osgGA::CustomManipulator* CM_ptr,FeaturesGroup* FG_ptr,
      OBSFRUSTAGROUP* OFG_ptr,ModeController* MC_ptr,
      WindowManager* WCC_ptr,threevector* GO_ptr);

// Set & get member functions:

   void set_convert_3D_to_2D_flag(bool flag);
   void set_insert_single_feature_flag(bool flag);

// 2D photo member functions:

   bool recover_photo_feature_world_coords(
      int OBSFRUSTUM_ID,const twovector& video_UV,threevector& XYZ);
      
  protected:

   virtual ~FeaturePickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool toggle_rotate_mode();
   virtual bool rotate(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);
   virtual bool release();

  private:

   bool convert_3D_to_2D_flag,insert_single_feature_flag;
   FeaturesGroup* FeaturesGroup_ptr;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

// Feature generation, manipulation and annihilation methods:

   bool instantiate_feature(double X,double Y);
   bool propagate_feature();
   bool select_feature();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void FeaturePickHandler::set_convert_3D_to_2D_flag(bool flag)
{
   convert_3D_to_2D_flag=flag;
}

inline void FeaturePickHandler::set_insert_single_feature_flag(bool flag)
{
   insert_single_feature_flag=flag;
}


#endif // FeaturePickHandler.h



