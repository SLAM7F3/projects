// ==========================================================================
// PointCloudKeyHandler header file 
// ==========================================================================
// Last modified on 10/12/07; 11/5/07; 11/22/07; 12/24/07
// ==========================================================================

#ifndef POINTCLOUDKEYHANDLER_H
#define POINTCLOUDKEYHANDLER_H

#include <osgGA/GUIEventHandler>
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osg3D/PointCloudsGroup.h"

class ModeController;
class PointCloud;

class PointCloudKeyHandler : public osgGA::GUIEventHandler
{
  public:

   PointCloudKeyHandler(PointCloudsGroup* PCG_ptr,ModeController* MC_ptr,
                        bool set_origin_to_zeroth_xyz_flag=true);
   PointCloudKeyHandler(PointCloudsGroup* PCG_ptr,ModeController* MC_ptr,
                        osgGA::Terrain_Manipulator* TM_ptr,
                        bool set_origin_to_zeroth_xyz_flag=true);


   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );
  
  protected:

  private:

   bool set_origin_to_zeroth_xyz_flag;
   PointCloudsGroup* PointCloudsGroup_ptr;
   ModeController* ModeController_ptr;
   osgGA::Terrain_Manipulator* Terrain_Manipulator_ptr;
   
   void allocate_member_objects();
   void initialize_member_objects();
   PointCloud* const get_cloud_ptr();
   ModeController* const get_ModeController_ptr();
}; 

// ---------------------------------------------------------------------
inline PointCloud* const PointCloudKeyHandler::get_cloud_ptr()
{
   return PointCloudsGroup_ptr->get_Cloud_ptr(0);
}

inline ModeController* const PointCloudKeyHandler::get_ModeController_ptr()
{
   return ModeController_ptr;
}

#endif // PointCloudKeyHandler.h
