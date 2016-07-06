// ========================================================================
// Program NYCVID is a variant of VIDEO developed for powerpoint chart
// generation purposes.  We hardwire below the projected UV
// coordinates of bounding boxes surrounding two skyscrapers nearby
// Rockefeller center.  The projections correspond to two particular
// photos shot from RCA and Empire State building.  This program
// superposes alpha-blended versions of these projected bboxes on the
// photos themselves.  This demonstrates nontrivial transfer of
// knowledge from one photo to another via 3D geometry.


// 			    nycvid east_crop.jpg

// 			    nycvid empire4_crop.jpg

// ========================================================================
// Last updated on 8/29/07
// ========================================================================

#include <vector>
#include <osgUtil/SceneView>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/AnimationKeyHandler.h"
#include "osg/AnimationPathCreator.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/Moviefuncs.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGeometry/Polygon.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "general/stringfuncs.h"
#include "passes/TextDialogBox.h"
#include "osg/ViewerManager.h"

// ========================================================================
int main( int argc, char** argv )
{

   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();

// Construct the viewer and set it up with sensible event handlers:

   osgProducer::Viewer viewer(arguments);
   viewer.setClearColor(osg::Vec4(0,0,0,0));
   viewer.setUpViewer( osgProducer::Viewer::ESCAPE_SETS_DONE );

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate a mode controller and mode key event handler:

   ModeController* ModeController_ptr=new ModeController();
   viewer.getEventHandlerList().push_back( 
      new ModeKeyHandler(ModeController_ptr) );
   root->addChild(osgfunc::create_Mode_HUD(ndims,ModeController_ptr));

// Instantiate animation controller & key handler:

   AnimationController* AnimationController_ptr=new AnimationController();
   root->addChild(AnimationController_ptr->get_OSGgroup_ptr());
   
   AnimationKeyHandler* AnimationKeyHandler_ptr=
      new AnimationKeyHandler(ModeController_ptr,AnimationController_ptr);
   viewer.getEventHandlerList().push_back( AnimationKeyHandler_ptr);
//   bool display_movie_state=false;
   bool display_movie_state=true;
//   bool display_movie_number=false;
   bool display_movie_number=true;
   root->addChild(Moviefunc::create_Imagenumber_HUD(
      AnimationController_ptr,display_movie_state,display_movie_number));

// Instantiate WindowCoordConverter:

   ViewerManager window_mgr;
   window_mgr.set_Viewer_ptr(&viewer);
   window_mgr.initialize_window("2D imagery");

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr);
   window_mgr.set_CameraManipulator(CM_2D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(
      &window_mgr,ModeController_ptr,CM_2D_ptr);

// Instantiate points, polygons, linesegments, triangles, rectangles
// and features decorations group:

   decorations.add_Points(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   PolyLinesGroup* PolyLinesGroup_ptr=
      decorations.add_PolyLines(ndims,passes_group.get_pass_ptr(
         videopass_ID));
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr=decorations.add_Polygons(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   LineSegmentsGroup* LineSegmentsGroup_ptr=
      decorations.add_LineSegments(
         ndims,passes_group.get_pass_ptr(videopass_ID),
         AnimationController_ptr);
   decorations.add_Triangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   decorations.add_Rectangles(
      ndims,passes_group.get_pass_ptr(videopass_ID));

   vector<threevector> proj_vertices;

   for (int iter=0; iter<2; iter++)
   {
      proj_vertices.clear();
      colorfunc::Color bbox_color;
      
      if (iter==0)
      {

// Vertices for "zig-zag rooftop" building bbox projected into
// Rockefeller_east_crop photo:
   
         proj_vertices.push_back(threevector(0.999516842349,-0.371849158872));
         proj_vertices.push_back(threevector(0.679753851033,-0.425388284408));
         proj_vertices.push_back(threevector(0.742553621764,-0.693936132797));
         proj_vertices.push_back(threevector(1.12164361411,-0.616863643542));

         proj_vertices.push_back(threevector(1.17122077053,0.484167570522));
         proj_vertices.push_back(threevector(0.72515761808,0.459687553423));
         proj_vertices.push_back(threevector(0.656436064704,0.542419518945));
         proj_vertices.push_back(threevector(1.02255559996,0.558485325341));

         proj_vertices.push_back(threevector(1.12164361411,-0.616863643542));
         proj_vertices.push_back(threevector(1.17122077053,0.484167570522));
         proj_vertices.push_back(threevector(1.02255559996,0.558485325341));
         proj_vertices.push_back(threevector(0.999516842349,-0.371849158872));

         proj_vertices.push_back(threevector(0.679753851033,-0.425388284408));
         proj_vertices.push_back(threevector(0.656436064704,0.542419518945));
         proj_vertices.push_back(threevector(0.72515761808,0.459687553423));
         proj_vertices.push_back(threevector(0.742553621764,-0.693936132797));

         proj_vertices.push_back(threevector(0.999516842349,-0.371849158872));
         proj_vertices.push_back(threevector(1.02255559996,0.558485325341));
         proj_vertices.push_back(threevector(0.656436064704,0.542419518945));
         proj_vertices.push_back(threevector(0.679753851033,-0.425388284408));

         proj_vertices.push_back(threevector(0.742553621764,-0.693936132797));
         proj_vertices.push_back(threevector(0.72515761808,0.459687553423));
         proj_vertices.push_back(threevector(1.17122077053,0.484167570522));
         proj_vertices.push_back(threevector(1.12164361411,-0.616863643542));

/*
// Vertices for "zig-zag rooftop" building bbox projected into
// empire4_crop_crop photo:

         proj_vertices.push_back(threevector(1.09020536609,0.516404440342));
         proj_vertices.push_back(threevector(1.07892605591,0.536349328635));
         proj_vertices.push_back(threevector(1.01099701298,0.536847722111));
         proj_vertices.push_back(threevector(1.01874475338,0.516995014668));

         proj_vertices.push_back(threevector(1.02888270187,0.754441989479));
         proj_vertices.push_back(threevector(1.02033463462,0.762983187055));
         proj_vertices.push_back(threevector(1.09080877218,0.763170484233));
         proj_vertices.push_back(threevector(1.10316542354,0.754609680961));

         proj_vertices.push_back(threevector(1.01874475338,0.516995014668));
         proj_vertices.push_back(threevector(1.02888270187,0.754441989479));
         proj_vertices.push_back(threevector(1.10316542354,0.754609680961));
         proj_vertices.push_back(threevector(1.09020536609,0.516404440342));

         proj_vertices.push_back(threevector(1.07892605591,0.536349328635));
         proj_vertices.push_back(threevector(1.09080877218,0.763170484233));
         proj_vertices.push_back(threevector(1.02033463462,0.762983187055));
         proj_vertices.push_back(threevector(1.01099701298,0.536847722111));

         proj_vertices.push_back(threevector(1.09020536609,0.516404440342));
         proj_vertices.push_back(threevector(1.10316542354,0.754609680961));
         proj_vertices.push_back(threevector(1.09080877218,0.763170484233));
         proj_vertices.push_back(threevector(1.07892605591,0.536349328635));

         proj_vertices.push_back(threevector(1.01099701298,0.536847722111));
         proj_vertices.push_back(threevector(1.02033463462,0.762983187055));
         proj_vertices.push_back(threevector(1.02888270187,0.754441989479));
         proj_vertices.push_back(threevector(1.01874475338,0.516995014668));
*/

         bbox_color=colorfunc::green;
      }
      else
      {


// Vertices for "cylinder" building bbox projected into
// Rockefeller_east_crop photo:

         proj_vertices.push_back(threevector(1.36180488003,0.0079087738613));
         proj_vertices.push_back(threevector(1.16906691245,-0.0186368930028));
         proj_vertices.push_back(threevector(1.29224350586,-0.157527594357));
         proj_vertices.push_back(threevector(1.52148970423,-0.13266891738));

         proj_vertices.push_back(threevector(1.61701269041,0.817452842068));
         proj_vertices.push_back(threevector(1.35755201383,0.815462058648));
         proj_vertices.push_back(threevector(1.20957483102,0.825515650435));
         proj_vertices.push_back(threevector(1.42371216839,0.827532286576));

         proj_vertices.push_back(threevector(1.52148970423,-0.13266891738));
         proj_vertices.push_back(threevector(1.61701269041,0.817452842068));
         proj_vertices.push_back(threevector(1.42371216839,0.827532286576));
         proj_vertices.push_back(threevector(1.36180488003,0.0079087738613));

         proj_vertices.push_back(threevector(1.16906691245,-0.0186368930028));
         proj_vertices.push_back(threevector(1.20957483102,0.825515650435));
         proj_vertices.push_back(threevector(1.35755201383,0.815462058648));
         proj_vertices.push_back(threevector(1.29224350586,-0.157527594357));

         proj_vertices.push_back(threevector(1.36180488003,0.0079087738613));
         proj_vertices.push_back(threevector(1.42371216839,0.827532286576));
         proj_vertices.push_back(threevector(1.20957483102,0.825515650435));
         proj_vertices.push_back(threevector(1.16906691245,-0.0186368930028));

         proj_vertices.push_back(threevector(1.29224350586,-0.157527594357));
         proj_vertices.push_back(threevector(1.35755201383,0.815462058648));
         proj_vertices.push_back(threevector(1.61701269041,0.817452842068));
         proj_vertices.push_back(threevector(1.52148970423,-0.13266891738));

/*
// Vertices for "cylinder" building bbox projected into empire4_crop
// photo:

         proj_vertices.push_back(threevector(1.31909453315,0.438510583665));
         proj_vertices.push_back(threevector(1.29189566246,0.465113282521));
         proj_vertices.push_back(threevector(1.19845280016,0.466022706296));
         proj_vertices.push_back(threevector(1.21340230962,0.43962902865));

         proj_vertices.push_back(threevector(1.23991854983,0.790649084254));
         proj_vertices.push_back(threevector(1.2226230035,0.798297470725));
         proj_vertices.push_back(threevector(1.32131745142,0.798727873766));
         proj_vertices.push_back(threevector(1.35191928785,0.791109091799));

         proj_vertices.push_back(threevector(1.21340230962,0.43962902865));
         proj_vertices.push_back(threevector(1.23991854983,0.790649084254));
         proj_vertices.push_back(threevector(1.35191928785,0.791109091799));
         proj_vertices.push_back(threevector(1.31909453315,0.438510583665));

         proj_vertices.push_back(threevector(1.29189566246,0.465113282521));
         proj_vertices.push_back(threevector(1.32131745142,0.798727873766));
         proj_vertices.push_back(threevector(1.2226230035,0.798297470725));
         proj_vertices.push_back(threevector(1.19845280016,0.466022706296));

         proj_vertices.push_back(threevector(1.31909453315,0.438510583665));
         proj_vertices.push_back(threevector(1.35191928785,0.791109091799));
         proj_vertices.push_back(threevector(1.32131745142,0.798727873766));
         proj_vertices.push_back(threevector(1.29189566246,0.465113282521));

         proj_vertices.push_back(threevector(1.19845280016,0.466022706296));
         proj_vertices.push_back(threevector(1.2226230035,0.798297470725));
         proj_vertices.push_back(threevector(1.23991854983,0.790649084254));
         proj_vertices.push_back(threevector(1.21340230962,0.43962902865));
*/

         bbox_color=colorfunc::red;
         
      } // iter conditional

      for (int f=0; f<6; f++)
      {
         vector<threevector> V;
         for (int v=f*4; v<(f+1)*4; v++)
         {
            V.push_back(proj_vertices[v]);
         } // loop over index v labeling current face projected vertices
         V.push_back(V.front());
         
         threevector origin(0,0,0);
         PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
            V,colorfunc::get_OSG_color(bbox_color),origin);
         PolyLine_ptr->set_linewidth(2);

         polygon poly(V);
         osgGeometry::Polygon* Polygon_ptr=
            PolygonsGroup_ptr->generate_new_Polygon(poly);
      
         Polygon_ptr->set_permanent_color(colorfunc::get_OSG_color(
            bbox_color,0.3));
         PolygonsGroup_ptr->reset_colors();
      } // loop over index f labeling box faces

   } // loop over iter index labeling skyscrapers

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      decorations.get_PointsGroup_ptr(),
      decorations.get_PolygonsGroup_ptr(),AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);
   AnimationController_ptr->set_nframes(movie_ptr->get_Nimages());
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   viewer.getEventHandlerList().push_back(MoviesKeyHandler_ptr);

// Instantiate CentersGroup and center pick handler to handle mid
// point selection:

   CentersGroup centers_group(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   CenterPickHandler* CenterPickHandler_ptr=new CenterPickHandler(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      CM_2D_ptr,&centers_group,ModeController_ptr,&window_mgr);
   viewer.getEventHandlerList().push_back(CenterPickHandler_ptr);

   FeaturesGroup* FeaturesGroup_ptr=decorations.add_Features(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      &centers_group,movie_ptr,decorations.get_TrianglesGroup_ptr(),
      decorations.get_LineSegmentsGroup_ptr(),AnimationController_ptr);

// Attach the scene graphs to the viewer:

   root->addChild(decorations.get_OSGgroup_ptr());

   viewer.setSceneData( root );

// Open text dialog box to display feature information:

//   FeaturesGroup_ptr->get_TextDialogBox_ptr()->open("Feature Information");
//   FeaturesGroup_ptr->update_feature_text();

// Create the windows and run the threads:

   viewer.realize();
   osgUtil::SceneView* SceneView_ptr=viewer.getSceneHandlerList().front()->
      getSceneView();
   CM_2D_ptr->set_SceneView_ptr(SceneView_ptr);
   decorations.set_DataNode_ptr(movie_ptr->getGeode());

// Add an animation path creator to the event handler list AFTER the
// viewer has been realized:

   AnimationPathCreator* animation_path_handler = 
      new AnimationPathCreator(&viewer);
   viewer.getEventHandlerList().push_back(animation_path_handler);

   viewer.getUsage(*arguments.getApplicationUsage());

   while (!viewer.done())
   {
      viewer.sync();
      viewer.update();
      viewer.frame();
   }

   viewer.sync();

//   FeaturesGroup_ptr->get_TextDialogBox_ptr()->close();

   return 0;


}

