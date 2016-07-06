// ========================================================================
// Program FUNDAMENTAL is a testing grounds for fundamental matrix
// relationships between 2 images.  It pops open 2 OSG windows to hold
// overlapping images of some scene.  SIFT features from both images
// are matched and used to compute the fundamental matrix relating the
// two views.  Epipoles ( = images of camera centers) are calculated
// and displayed within the image planes.  When a feature is selected
// in one image, the corresponding epipolar line on which its tiepoint
// must lie is displayed within the counterpart image.
// ========================================================================
// Last updated on 11/7/11; 11/8/11; 4/3/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <lmcurve.h>

#include "color/colorfuncs.h"
#include "osg/osgOrganization/Decorations.h"
#include "general/filefuncs.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "video/sift_detector.h"
#include "osg/osgWindow/ViewerManager.h"



// n_par = dimension of parameter vector param

// par = parameter vector.  On input, it must contain reasonable guess
// entries. On output, it contains soln found to minimize |fvec| .

// m_dat = dimension of residue vector fvec
// m_dat >= n_par

// data  = pointer forwarded to evaluate and printout

// evaluate = routine that calculates residue vector fvec for given
// parameter vector par

// *info = setting *info to negative value causes lm_minimize to terminate

void evaluate_cost_function(
   const double* par, int m_dat, const void* const_data_ptr, 
   double* fvec, int* info)
{
   cout << "inside evaluate_cost_function()" << endl;
   
   void* data_ptr=const_cast<void*>(const_data_ptr);
   fundamental* fundamental_ptr=static_cast<fundamental*>(data_ptr);

   threevector epipole_UV(par[0],par[1],par[2]);
   genvector* epsilon_ptr=fundamental_ptr->solve_for_fundamental(epipole_UV);

   for (int i=0; i<epsilon_ptr->get_mdim(); i++)
   {
      fvec[i]=sqr(epsilon_ptr->get(i));
//      fvec[i]=fabs(epsilon_ptr->get(i));
//      cout << "i = " << i << " fvec[i] = " << fvec[i] << endl;
   }
   delete epsilon_ptr;
}

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input data files:
   
   const int ndims=2;
   PassesGroup passes_group(&arguments);

   int videopass_ID=passes_group.get_videopass_ID();
   cout << "videopass_ID = " << videopass_ID << endl;
   Pass* videopass_ptr=passes_group.get_pass_ptr(videopass_ID);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;
   
   sift_detector SIFT(photogroup_ptr);

// FAKE FAKE:  Tues Apr 3, 2012 at 12:47 pm
// Hardwire hand-selected kermit features

   string features_subdir="./features/";
   string features1_filename=features_subdir+"features_2D_redbldg1.txt";
   string features2_filename=features_subdir+"features_2D_redbldg2.txt";

//   string features_subdir=
//      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/kermit/images/";
//   string features1_filename=features_subdir+"features_2D_kermit000.txt";
//   string features2_filename=features_subdir+"features_2D_kermit002.txt";

   vector<twovector> UV,XY;

   filefunc::ReadInfile(features1_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      UV.push_back(twovector(column_values[3],column_values[4]));
      cout << "i = " << i << " UV = " << UV.back() << endl;
   }

   filefunc::ReadInfile(features2_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      XY.push_back(twovector(column_values[3],column_values[4]));
//      cout << "i = " << i << " XY = " << XY.back() << endl;
   }


/*
   string sift_keys_subdir="./";
   SIFT.extract_SIFT_features(sift_keys_subdir);
//   SIFT.print_features(5);

// SIFT feature matching becomes LESS stringent as sqrd_max_ratio
// increases.

// SIFT tiepoint inlier identification becomes LESS stringent as
// max_scalar_product increases.

   const int n_min_quadrant_features=1;		
   double max_ratio=0.7;
//   cout << "Enter Lowe ratio threshold:" << endl;
//   cout << "(Default value = 0.7)" << endl;
//   cin >> max_ratio;
   double sqrd_max_ratio=sqr(max_ratio);
   const double worst_frac_to_reject=0.10;

   double max_scalar_product;
   cout << "Enter max value for fundamental matrix scalar product:" << endl;
   cout << "(Default value = 0.01)" << endl;
   cin >> max_scalar_product;

   SIFT.identify_candidate_feature_matches_via_fundamental_matrix(
      n_min_quadrant_features,sqrd_max_ratio,worst_frac_to_reject,
      max_scalar_product);

   string banner="SIFT computation of F";
   outputfunc::write_big_banner(banner);
*/

   fundamental* fundamental_ptr=SIFT.get_fundamental_ptr();   
   genmatrix* F_ptr=fundamental_ptr->get_F_ptr();

/*
   fundamental* fundamental_ptr=SIFT.get_fundamental_ptr();   
   fundamental_ptr->parse_fundamental_inputs(XY,UV);
   fundamental_ptr->compute_fundamental_matrix();
   bool print_flag=true;
   fundamental_ptr->check_fundamental_matrix(XY,UV,print_flag);

   genmatrix* F_ptr=fundamental_ptr->get_F_ptr();
   *F_ptr /= F_ptr->get(2,2);
*/

// redbldg1.jpg vs redbldg2.jpg

   F_ptr->put(0,0,-8.81671515323);
   F_ptr->put(0,1,94.4398301838);
   F_ptr->put(0,2,-25.664636541);
   F_ptr->put(1,0,-94.4188203866);
   F_ptr->put(1,1,-8.85152616708);
   F_ptr->put(1,2,85.4221974448);
   F_ptr->put(2,0,36.0558200104);
   F_ptr->put(2,1,-91.8267973973);
   F_ptr->put(2,2,1);

//   F_ptr->put(0,0,-8.64927907494);
//   F_ptr->put(0,1,92.6711069918);
//   F_ptr->put(0,2,-25.1583279694);
//   F_ptr->put(1,0,-92.5892329312);
//   F_ptr->put(1,1,-8.63895149431);
//   F_ptr->put(1,2,83.7210091207);
//   F_ptr->put(2,0,35.3321708483);
//   F_ptr->put(2,1,-90.0932442134);
//   F_ptr->put(2,2,1);

/*
// tower_pushcart.jpg vs ts.jpg 

   F_ptr->put(0,0,-6.68169374906);
   F_ptr->put(0,1,41.2857930454);
   F_ptr->put(0,2,19.9000281214);
   F_ptr->put(1,0,-34.1087861301);
   F_ptr->put(1,1,-2.35876143846);
   F_ptr->put(1,2,12.7173987648);
   F_ptr->put(2,0,-28.1883911439);
   F_ptr->put(2,1,-24.7551875263);
   F_ptr->put(2,2,1);
*/


/*
// FAKE FAKE: Hardwire fundamental matrix recovered from P0 and P2 for
//    kermit0 and kermit2:

   F_ptr->put(0,0,0.1706760849);
   F_ptr->put(0,1,0.5270025638);
   F_ptr->put(0,2,0.1920874157);
   F_ptr->put(1,0,-0.6325376775);
   F_ptr->put(1,1,-1.231633994);
   F_ptr->put(1,2,-0.5763289281);
   F_ptr->put(2,0,1.000565002);
   F_ptr->put(2,1,2.418430684);
   F_ptr->put(2,2,1);

   cout.precision(12);
   cout << "*fundamental_ptr = " << *fundamental_ptr << endl;
   cout << "fundamental rank = " << F_ptr->rank() << endl;
   cout << "Det(F) = " << F_ptr->determinant() << endl;
*/

/*
   fundamental_ptr->renormalize_F_entries();

   int n_params = 3; // number of parameters entering into score function
   double param[3];
   for (int n=0; n<n_params; n++)
   {
      param[n]=fundamental_ptr->get_epipole_UV().get(n);
   }
   
   int m_dat = fundamental_ptr->get_A_ptr()->get_mdim();  
	// dimension of residue vector (m_dat >= n_params)

   lm_status_struct status;
   lm_control_struct control = lm_control_double;
//   control.printflags = 0;
   control.printflags = 3; // monitor status (+1) and parameters (+2)
   control.maxcall=5000;
*/

/*
   double fraction;
   cout << "Enter fraction value:" << endl;
   cin >> fraction;

   control.ftol *= fraction;
   control.xtol *= fraction;
   control.gtol *= fraction;

   cout << "control.ftol = " << control.ftol << endl;
   cout << "control.xtol = " << control.xtol << endl;
   cout << "control.gtol = " << control.gtol << endl;

   outputfunc::enter_continue_char();
*/

/*
   void* data_ptr=static_cast<void*>(fundamental_ptr);

   cout << "Performing LM fit:" << endl;
   lmmin( 
      n_params, param, m_dat, data_ptr,
      evaluate_cost_function, &control, &status, lm_printout_std );

   cout << "Fit results:" << endl;
   cout << "Status after " << status.nfev << " function evaluations = "
        << lm_infmsg[status.info] << endl;
    
   cout << "Fitted parameter values:" << endl;
   for ( int i=0; i<n_params; i++ )
   {
      cout << "param[" << i << "] = " << param[i] << endl;
   }
    
   cout << "Residual norm = " << status.fnorm << endl;

   fundamental_ptr->renormalize_F_entries();
   genmatrix* F_ptr=fundamental_ptr->get_F_ptr();
   cout << "Refined F = " << *F_ptr << endl;
   cout << "F.rank = " << F_ptr->rank() << endl;

   banner="Projection matrix computation of F";
   outputfunc::write_big_banner(banner);

   camera* camera_XY_ptr=photogroup_ptr->get_photograph_ptr(0)->
      get_camera_ptr();
   camera* camera_UV_ptr=photogroup_ptr->get_photograph_ptr(1)->
      get_camera_ptr();
   const genmatrix* P_XY_ptr=camera_XY_ptr->get_P_ptr();
   const genmatrix* P_UV_ptr=camera_UV_ptr->get_P_ptr();
   
   fundamental_ptr->compute_from_projection_matrices(*P_UV_ptr,*P_XY_ptr);

   exit(-1);
*/

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_XY_ptr=new ViewerManager();
   WindowManager* window_mgr_UV_ptr=new ViewerManager();
   string window_XY_title="XY image";
   string window_UV_title="UV image";

//   cout << "window_mgr_XY_ptr = " << window_mgr_XY_ptr << endl;
//   cout << "window_mgr_UV_ptr = " << window_mgr_UV_ptr << endl;
   
   window_mgr_XY_ptr->initialize_dual_windows(
      window_XY_title,window_UV_title,window_mgr_UV_ptr,false);

   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      window_mgr_XY_ptr);

// Create OSG root_UV node:

   osg::Group* root_XY = new osg::Group;	
   osg::Group* root_UV = new osg::Group;	

// Instantiate Operations object to handle mode, animation and image
// number control:

//   bool display_movie_state=true;
//   bool display_movie_number=true;
//   bool display_movie_world_time=true;
//   bool hide_Mode_HUD_flag=true;
   bool display_movie_state=false;
   bool display_movie_number=false;
   bool display_movie_world_time=false;
   bool hide_Mode_HUD_flag=false;

   Operations operations_XY(
      ndims,window_mgr_XY_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);
   root_XY->addChild(operations_XY.get_OSGgroup_ptr());

   Operations operations_UV(
      ndims,window_mgr_UV_ptr,passes_group,
      display_movie_state,display_movie_number,display_movie_world_time,
      hide_Mode_HUD_flag);
   root_UV->addChild(operations_UV.get_OSGgroup_ptr());

   AnimationController* AnimationController_XY_ptr=
      operations_XY.get_AnimationController_ptr();
   AnimationController* AnimationController_UV_ptr=
      operations_UV.get_AnimationController_ptr();

   ModeController* ModeController_XY_ptr=operations_XY.
      get_ModeController_ptr();
   ModeController_XY_ptr->setState(ModeController::MANIPULATE_FEATURE);

   ModeController* ModeController_UV_ptr=operations_UV.
      get_ModeController_ptr();
   ModeController_UV_ptr->setState(ModeController::MANIPULATE_FEATURE);

   window_mgr_UV_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_UV_ptr) );
   root_UV->addChild(osgfunc::create_Mode_HUD(2,ModeController_UV_ptr));

// Add custom manipulators to the event handler list:

   osgGA::Custom2DManipulator* CM_XY_ptr = new 
      osgGA::Custom2DManipulator(ModeController_XY_ptr,window_mgr_XY_ptr);
   window_mgr_XY_ptr->set_CameraManipulator(CM_XY_ptr);

   osgGA::Custom2DManipulator* CM_UV_ptr = new 
      osgGA::Custom2DManipulator(ModeController_UV_ptr,window_mgr_UV_ptr);
   window_mgr_UV_ptr->set_CameraManipulator(CM_UV_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations_XY(
      window_mgr_XY_ptr,ModeController_XY_ptr,CM_XY_ptr);
   Decorations decorations_UV(
      window_mgr_UV_ptr,ModeController_UV_ptr,CM_UV_ptr);

// Instantiate group to hold XY movie:

   MoviesGroup* MoviesGroup_XY_ptr=new MoviesGroup(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_XY_ptr);
   MoviesGroup_XY_ptr->set_OSGgroup_nodemask(1);
   root_XY->addChild( MoviesGroup_XY_ptr->get_OSGgroup_ptr() );

   string XY_movie_filename=
      passes_group.get_videopass_ptr()->get_first_filename();
   texture_rectangle* texture_rectangle_XY_ptr=
      MoviesGroup_XY_ptr->generate_new_texture_rectangle(XY_movie_filename);
   Movie* Movie_XY_ptr=MoviesGroup_XY_ptr->generate_new_Movie(
      texture_rectangle_XY_ptr);
   root_XY->addChild( MoviesGroup_XY_ptr->get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesXYKeyHandler_ptr=
      new MovieKeyHandler(ModeController_XY_ptr,MoviesGroup_XY_ptr);
   window_mgr_XY_ptr->get_EventHandlers_ptr()->push_back(
      MoviesXYKeyHandler_ptr);

// Instantiate group to hold UV movie:

   MoviesGroup* MoviesGroup_UV_ptr=new MoviesGroup(
      ndims,passes_group.get_pass_ptr(videopass_ID+1),
      AnimationController_UV_ptr);
   MoviesGroup_UV_ptr->set_OSGgroup_nodemask(1);
   root_UV->addChild( MoviesGroup_UV_ptr->get_OSGgroup_ptr() );

   string UV_movie_filename=
      passes_group.get_pass_ptr(videopass_ID+1)->get_first_filename();
   texture_rectangle* texture_rectangle_UV_ptr=
      MoviesGroup_UV_ptr->generate_new_texture_rectangle(UV_movie_filename);
   Movie* Movie_UV_ptr=MoviesGroup_UV_ptr->generate_new_Movie(
      texture_rectangle_UV_ptr);
   root_UV->addChild( MoviesGroup_UV_ptr->get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesUVKeyHandler_ptr=
      new MovieKeyHandler(ModeController_UV_ptr,MoviesGroup_UV_ptr);
   window_mgr_UV_ptr->get_EventHandlers_ptr()->push_back(
      MoviesUVKeyHandler_ptr);

// Instantiate PointsGroups to hold epipoles:

   osgGeometry::PointsGroup* PointsGroup_XY_ptr=decorations_XY.add_Points(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_XY_ptr);
   osgGeometry::PointsGroup* PointsGroup_UV_ptr=decorations_UV.add_Points(
      ndims,passes_group.get_pass_ptr(videopass_ID+1),
      AnimationController_UV_ptr);

   threevector epipole_XY=fundamental_ptr->get_epipole_XY();
   threevector epipole_UV=fundamental_ptr->get_epipole_UV();
   cout << "epipole_XY = " << epipole_XY << endl;
   cout << "epipole_UV = " << epipole_UV << endl;

   osgGeometry::Point* epipole_UV_Point_ptr=
      PointsGroup_XY_ptr->generate_new_Point(epipole_UV);
   osgGeometry::Point* epipole_XY_Point_ptr=
      PointsGroup_UV_ptr->generate_new_Point(epipole_XY);
   epipole_XY_Point_ptr->reset_text_label("XY epipole");
   epipole_UV_Point_ptr->reset_text_label("UV epipole");

// Instantiate PolyLinesGroups to hold epipolar lines:

   PolyLinesGroup* PolyLinesGroup_XY_ptr=decorations_XY.add_PolyLines(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   PolyLinesGroup_XY_ptr->set_width(1);
   PolyLinesGroup* PolyLinesGroup_UV_ptr=decorations_UV.add_PolyLines(
      ndims,passes_group.get_pass_ptr(videopass_ID+1));
   PolyLinesGroup_UV_ptr->set_width(1);

// Instantiate groups to hold XY and UV features:

   FeaturesGroup* FeaturesGroup_XY_ptr=decorations_XY.add_Features(
      ndims,passes_group.get_pass_ptr(videopass_ID));
   FeaturesGroup* FeaturesGroup_UV_ptr=decorations_UV.add_Features(
      ndims,passes_group.get_pass_ptr(videopass_ID+1));
   FeaturesGroup_XY_ptr->set_fundamental_matrix_ptr(F_ptr);
   FeaturesGroup_UV_ptr->set_fundamental_matrix_ptr(F_ptr);
   FeaturesGroup_XY_ptr->set_PolyLinesGroup_ptr(PolyLinesGroup_XY_ptr);
   FeaturesGroup_UV_ptr->set_PolyLinesGroup_ptr(PolyLinesGroup_UV_ptr);
   FeaturesGroup_XY_ptr->set_UV_image_flag(false);
   FeaturesGroup_UV_ptr->set_UV_image_flag(true);
   FeaturesGroup_XY_ptr->set_counterpart_FeaturesGroup_2D_ptr(
      FeaturesGroup_UV_ptr);
   FeaturesGroup_UV_ptr->set_counterpart_FeaturesGroup_2D_ptr(
      FeaturesGroup_XY_ptr);

   decorations_XY.set_DataNode_ptr(Movie_XY_ptr->getGeode());
   decorations_UV.set_DataNode_ptr(Movie_UV_ptr->getGeode());

   root_XY->addChild(decorations_XY.get_OSGgroup_ptr());
   root_UV->addChild(decorations_UV.get_OSGgroup_ptr());

// ========================================================================

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_XY_ptr->setSceneData(root_XY);
   window_mgr_UV_ptr->setSceneData(root_UV);

   window_mgr_XY_ptr->realize();
   window_mgr_UV_ptr->realize();

   int counter=0;
   while( !window_mgr_XY_ptr->done()  && !window_mgr_UV_ptr->done())
   {
      window_mgr_XY_ptr->process();
      window_mgr_UV_ptr->process();
   }
   delete window_mgr_XY_ptr;
   delete window_mgr_UV_ptr;
}
