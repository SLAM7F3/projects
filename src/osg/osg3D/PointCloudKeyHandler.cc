// ==========================================================================
// PointCloudKeyHandler class member function definitions
// ==========================================================================
// Last modified on 11/15/09; 5/16/11; 5/22/11
// ==========================================================================

#include <iostream>
#include <string>
#include <osgDB/WriteFile>
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "osg/ModeController.h"
#include "osg/osg3D/PointCloud.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PointCloudKeyHandler::allocate_member_objects()
{
}		       

void PointCloudKeyHandler::initialize_member_objects()
{
   set_origin_to_zeroth_xyz_flag=true;
   PointCloudsGroup_ptr=NULL;
   Terrain_Manipulator_ptr=NULL;
}		       

PointCloudKeyHandler::PointCloudKeyHandler(
   PointCloudsGroup* PCG_ptr,ModeController* MC_ptr,
   bool set_origin_to_zeroth_xyz_flag)
{
   allocate_member_objects();
   initialize_member_objects();
   PointCloudsGroup_ptr=PCG_ptr;
   ModeController_ptr=MC_ptr;
   this->set_origin_to_zeroth_xyz_flag=set_origin_to_zeroth_xyz_flag;
}

PointCloudKeyHandler::PointCloudKeyHandler(
   PointCloudsGroup* PCG_ptr,ModeController* MC_ptr,
   osgGA::Terrain_Manipulator* TM_ptr,
   bool set_origin_to_zeroth_xyz_flag)
{
   allocate_member_objects();
   initialize_member_objects();
   PointCloudsGroup_ptr=PCG_ptr;
   ModeController_ptr=MC_ptr;
   Terrain_Manipulator_ptr=TM_ptr;
   this->set_origin_to_zeroth_xyz_flag=set_origin_to_zeroth_xyz_flag;
}

// ---------------------------------------------------------------------
// Perform following operations on point cloud only if current mode ==
// VIEW_DATA

bool PointCloudKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (get_ModeController_ptr()->getState()==ModeController::VIEW_DATA)
   {
      if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
      {
         switch ( ea.getKey() )
         {
            case ',' :
               PointCloudsGroup_ptr->toggle_colormap();
               return true;
            case '=' :
               PointCloudsGroup_ptr->set_pt_size(
                  PointCloudsGroup_ptr->get_pt_size()+1);
               return true;
            case '-' :
               PointCloudsGroup_ptr->set_pt_size(
                  PointCloudsGroup_ptr->get_pt_size()-1);
               return true;

            case '/' :
               PointCloudsGroup_ptr->adjust_cyclic_colormap_offset();
               return true;

            case 'c' :
               PointCloudsGroup_ptr->next_dependent_var();
               PointCloudsGroup_ptr->update_dynamic_Grid_color();
               PointCloudsGroup_ptr->reload_all_colors();
               return true;

/*
            case 'd' :
            {
               double rx,ry,rz;
               cout << "Enter x component of illumination direction vector:" 
                    << endl;
               cin >> rx;
               cout << "Enter y component of illumination direction vector:" 
                    << endl;
               cin >> ry;
               cout << "Enter z component of illumination direction vector:" 
                    << endl;
               cin >> rz;
               threevector r_hat(rx,ry,rz);
               r_hat=r_hat.unitvector();

               get_cloud_ptr()->generate_shadow_map(r_hat);
               get_cloud_ptr()->determine_shaded_points(r_hat);
               return true;
            }
*/
            case 'e':
               get_cloud_ptr()->equalize_intensity_histogram();
               return true;

            case 'f':
               get_cloud_ptr()->retrieve_hires_XYZs_in_bbox();
               return true;

            case 'h':
               PointCloudsGroup_ptr->
                  reset_PolyLine_heights_using_TilesGroup();
               return true;

            case 'i':
//               get_cloud_ptr()->pure_hues();
               get_cloud_ptr()->pure_intensities();
               return true;

            case 'n' :
               PointCloudsGroup_ptr->next_color_map();
               return true;

            case 'r':
               get_cloud_ptr()->read_normals_from_xyzp_file();
               return true;

            case 's' :
               get_cloud_ptr()->orthorectify();
               return true;

            case 't' :
               PointCloudsGroup_ptr->generate_tracks_from_PolyLines();
               return true;

            case 'x' :
               PointCloudsGroup_ptr->prev_dependent_var();
               return true;

            case 'y':
               get_cloud_ptr()->write_output_file(
                  "output",set_origin_to_zeroth_xyz_flag);

/*
               cout << "Trying to write out entire scenegraph" << endl;
               osg::Node* root=Terrain_Manipulator_ptr->get_root_ptr();
               HiresDataVisitor* HiresDataVisitor_ptr=
                  get_cloud_ptr()->get_HiresDataVisitor_ptr();
               HiresDataVisitor_ptr->set_application_type(
                  HiresDataVisitor::reset_pageLOD_child_filenames);
               root->accept(*HiresDataVisitor_ptr);

               ofstream binary_outstream;
//               string output_filename="root.ive";
               string output_filename="root.osg";
               filefunc::deletefile(output_filename);
            
               if ( osgDB::writeNodeFile( *root, output_filename) )
                  osg::notify(osg::NOTICE) << "Wrote file: " 
                                           << output_filename << "\n";
               else
                  osg::notify(osg::WARN) << "Could not write output file.\n";
*/

               return true;

            case osgGA::GUIEventAdapter::KEY_Home:
               PointCloudsGroup_ptr->increase_min_threshold();
               return true;

            case osgGA::GUIEventAdapter::KEY_End:
               PointCloudsGroup_ptr->decrease_min_threshold();
               return true;

            case osgGA::GUIEventAdapter::KEY_Page_Up:
               PointCloudsGroup_ptr->increase_max_threshold();
               return true;

            case osgGA::GUIEventAdapter::KEY_Page_Down:
               PointCloudsGroup_ptr->decrease_max_threshold();
               return true;

         } // switch ( ea.getKey() ) 
      } // ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN conditional
   } // Mode = VIEW_DATA conditional

// For touchtable demo purposes, we want to minimize the number of
// mode switches which must be performed.  So we allow the user to
// toggle colormaps in MANIPULATE_ANNOTATION and MANIPULATE_FUSED_DATA
// modes as well as VIEW_DATA mode:

   else if (get_ModeController_ptr()->getState()==
            ModeController::MANIPULATE_ANNOTATION ||
            get_ModeController_ptr()->getState()==
            ModeController::MANIPULATE_FUSED_DATA)
   {
      if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
      {
         switch ( ea.getKey() )
         {
            case ',' :
               PointCloudsGroup_ptr->toggle_colormap();
               return true;

         } // switch ( ea.getKey() ) 
      } // ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN conditional
   }
   
   return false;
}
