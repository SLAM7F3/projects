// ========================================================================
// Program MOSAIC is a laboratory for generating composite mosaics
// from multiple photos.

// 		mosaic Shanghai_04.png --newpass Shanghai_05.png

// ========================================================================
// Last updated on 5/29/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgProducer/Viewer>
#include <osgDB/ReadFile>
#include "osg/Custom2DManipulator.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osg2D/TextureQuadsGroup.h"
#include "math/twovector.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

// ========================================================================
int main (int argc, char** argv)
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int n_photos=passes_group.get_n_passes();
   vector<string> photo_filename;
   for (int n=0; n<n_photos; n++)
   {
      photo_filename.push_back(passes_group.get_pass_ptr(n)->
                               get_first_filename());
      cout << "n = " << n << " photo filename = " << photo_filename.back()
           << endl;
   }
   
   int texturepass_ID=passes_group.get_curr_texturepass_ID();
   cout << "texturepass_ID = " << texturepass_ID << endl;

   osgProducer::Viewer viewer(arguments);
   viewer.setClearColor(osg::Vec4(0,0,0,0));
   viewer.setUpViewer( osgProducer::Viewer::ESCAPE_SETS_DONE );

// Initialize viewer window:

   Producer::RenderSurface* rs_ptr =
      viewer.getCameraConfig()->getCamera(0)->getRenderSurface();
   string window_title="2D video imagery";
   osgfunc::initialize_window(rs_ptr,window_title);

// Create OSG root node:

   osg::Group* root = new osg::Group;
   osg::Group* group_ptr=new osg::Group;
   root->addChild(group_ptr);

// Instantiate a mode controller and mode key event handler:

   ModeController* ModeController_ptr=new ModeController();
   viewer.getEventHandlerList().push_back( 
      new ModeKeyHandler(ModeController_ptr) );
   root->addChild(osgfunc::create_Mode_HUD(ndims,ModeController_ptr));

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr);
   window_mgr.set_CameraManipulator(CM_2D_ptr);

   string image_filename;
   vector<twovector> UV_corners;
   osg::Geode* geode_ptr=NULL;
   
   TextureQuadsGroup* TextureQuadsGroup_ptr=new TextureQuadsGroup(
      passes_group.get_pass_ptr(0));

   UV_corners.clear();
//   UV_corners.push_back(twovector(-0.885381548036862,-0.0156026931332867));
//   UV_corners.push_back(twovector(0.613945164428027,-0.0505828945838871));
//   UV_corners.push_back(twovector(0.731665527983946,0.881800370448249));
//   UV_corners.push_back(twovector(-0.754495085664019,1.18392267651993));

// RMS residual between measured and calculated UV points = 0.000118

//   UV_corners.push_back(twovector(-1.09559666090817,-0.100627798697513));
//   UV_corners.push_back(twovector(0.61809365980911,-0.055050418360053));
//   UV_corners.push_back(twovector(0.705896126564344,0.856527414838622));
//   UV_corners.push_back(twovector(-0.815988430941159,1.22164000153095));

// RMS residual between measured and calculated UV points = 0.000439

   UV_corners.push_back(twovector(-1.03068511972542,-0.0667931194251147));
   UV_corners.push_back(twovector(0.618140465135577,-0.0530806778161155));
   UV_corners.push_back(twovector(0.71795994982724,0.861658505568587));
   UV_corners.push_back(twovector(-0.801291662378798,1.21463143144769));

   TextureQuad* TextureQuad_ptr=TextureQuadsGroup_ptr->
      generate_new_TextureQuad();
   geode_ptr=TextureQuad_ptr->generate_drawable_geode(
      photo_filename[0],UV_corners);
   group_ptr->addChild(geode_ptr);

   UV_corners.clear();
   UV_corners.push_back(twovector(0,0));
   UV_corners.push_back(twovector(1.33333,0));
   UV_corners.push_back(twovector(1.33333,1));
   UV_corners.push_back(twovector(0,1));

   TextureQuad* TextureQuad2_ptr=TextureQuadsGroup_ptr->
      generate_new_TextureQuad();
   geode_ptr=TextureQuad2_ptr->generate_drawable_geode(
      photo_filename[1],UV_corners);
   group_ptr->addChild(geode_ptr);

// Store help info about keyboard and mouse bindings within the
// viewer:

   viewer.setSceneData (root);

// Create the windows and run the threads:

   viewer.realize();

   while (!viewer.done())
   {
      viewer.sync();
      viewer.update();
      viewer.frame();
   }

   // Wait for all cull and draw threads to complete before exit.
   viewer.sync();
}
