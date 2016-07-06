// ==========================================================================
// MODELSGROUP class member function definitions
// ==========================================================================
// Last modified on 10/12/11; 2/1/12; 6/27/12; 4/5/14; 4/6/14
// ==========================================================================

#include <iomanip>
#include <set>
#include <string>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "image/compositefuncs.h"
#include "math/constant_vectors.h"
#include "osg/osgEarth/EarthRegion.h"
#include "geometry/ellipse.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "math/mathfuncs.h"
#include "messenger/message.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "osg/osg2D/Movie.h"
#include "osg/osgModels/OBSFRUSTUMfuncs.h"
#include "osg/osgOperations/Operations.h"
#include "filter/piecewise_linear_vector.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGeometry/PolyLinePickHandler.h"
#include "video/texture_rectangle.h"
#include "osg/osgEarth/TextureSectorsGroup.h"
#include "time/timefuncs.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MODELSGROUP::allocate_member_objects()
{
   tracks_group_ptr=new tracks_group();
   movers_group_ptr=new movers_group();
}		       

void MODELSGROUP::initialize_member_objects()
{
   GraphicalsGroup_name="MODELSGROUP";

   aircraft_model_type=predator;
   generate_Predator_model_on_next_cycle_flag=false;
   alter_UAV_path_flag=false;
   continuously_plan_UAV_paths_flag=false;
   fade_UAV_track_color_flag=false;
   long_initial_UAV_track_flag=false;
   compute_passed_ground_targets_flag=true;
   compute_zface_height_flag=false;
   compute_skymap_flag=false;
   ground_bbox_ptr=NULL;
   instantiate_OBSFRUSTAGROUP_flag=true;
   instantiate_OBSFRUSTUMPickHandler_flag=false;
//   double_LiMIT_lobe_pattern_flag=true;
   double_LiMIT_lobe_pattern_flag=false;
   LiMIT_FOV_OBSFRUSTUM_flag=false;
   loiter_at_track_end_flag=true;

   model_filename="";
   model_counter=0;
   n_future_repeats=1;

// Note: 36 meters/sec = 70 knots = Predator loiter speed

   Predator_speed=36;	// meters/sec

// On 6/1/09, Melissa Meyers suggested that we adopt 100 m/s as a
// reasonable guestimate for the G105 sensor of interest:

   LiMIT_speed=100;	// meters/sec	

   aircraft_speed=36;	// meters/sec

// Note: Max Predator ceiling = 25,000 ft = 7.6 km

   aircraft_altitude=2500;	// meters

   flightpath_fraction_offset=0;
   prev_UAV_path_planning_time=0;

   OBSFRUSTUM_az_extent=OBSFRUSTUM_el_extent=0;
   OBSFRUSTUM_roll=OBSFRUSTUM_pitch=0;

   ArrowsGroup_ptr=NULL;
   Operations_ptr=NULL;
   Path_PolyLinesGroup_ptr=NULL;
   Path_PolyLinePickHandler_ptr=NULL;
   ROI_PolyhedraGroup_ptr=NULL;
   EarthRegionsGroup_ptr=NULL;

   TilesGroup_ptr=NULL;
   ColorGeodeVisitor_ptr=NULL;
   ModeController_ptr=NULL;
   WindowManager_ptr=NULL;
   viewer_Messenger_ptr=NULL;

   get_OSGgroup_ptr()->setUpdateCallback(new AbstractOSGCallback<MODELSGROUP>(
      this,&MODELSGROUP::update_display));
}		       

void MODELSGROUP::assign_MODELSGROUP_Messenger_ptrs()
{
//   cout << "inside MODELSGROUP::assign_MODELSGROUP_Messenger_ptrs()" << endl;
   for (unsigned int i=0; i<get_n_Messenger_ptrs(); i++)
   {
      Messenger* curr_Messenger_ptr=get_Messenger_ptr(i);
      if (curr_Messenger_ptr->get_topicName()=="viewer_update")
      {
         viewer_Messenger_ptr=curr_Messenger_ptr;
      }
   }
//   cout << "viewer_Messenger_ptr = " << viewer_Messenger_ptr << endl;
}

MODELSGROUP::MODELSGROUP(
   Pass* PI_ptr,threevector* GO_ptr,
   AnimationController* AC_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr,GO_ptr),AnnotatorsGroup(3,PI_ptr)
{	
//   cout << "inside MODELSGROUP constructor #1" << endl;
   initialize_member_objects();
   allocate_member_objects();
}		       

MODELSGROUP::MODELSGROUP(Pass* PI_ptr,PolyLinesGroup* PLG_ptr,
                         AnimationController* AC_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
//   cout << "inside MODELSGROUP constructor #2" << endl;
   initialize_member_objects();
   allocate_member_objects();
   Path_PolyLinesGroup_ptr=PLG_ptr;
   initialize_Path_PolyLinesGroup();
}		       

MODELSGROUP::MODELSGROUP(
   Pass* PI_ptr,PolyLinesGroup* PLG_ptr,
   threevector* GO_ptr,AnimationController* AC_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr,GO_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
//   cout << "inside MODELSGROUP constructor #3" << endl;
   initialize_member_objects();
   allocate_member_objects();
   Path_PolyLinesGroup_ptr=PLG_ptr;
   initialize_Path_PolyLinesGroup();
}		       

MODELSGROUP::MODELSGROUP(
   Pass* PI_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
   threevector* GO_ptr,osgGA::Terrain_Manipulator* CM_3D_ptr,
   AnimationController* AC_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr,GO_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
//   cout << "inside MODELSGROUP constructor #4" << endl;
   initialize_member_objects();
   allocate_member_objects();
   Path_PolyLinesGroup_ptr=PLG_ptr;
   initialize_Path_PolyLinesGroup();

   Path_PolyLinePickHandler_ptr=PLPH_ptr;
   initialize_Path_PolyLinePickHandler();

   set_CM_3D_ptr(CM_3D_ptr);
}		       

MODELSGROUP::MODELSGROUP(
   Pass* PI_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
   threevector* GO_ptr,osgGA::Terrain_Manipulator* CM_3D_ptr,
   Operations* O_ptr):
   GeometricalsGroup(3,PI_ptr,O_ptr->get_AnimationController_ptr(),GO_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
//   cout << "inside MODELSGROUP constructor #5" << endl;
   initialize_member_objects();
   allocate_member_objects();
   Path_PolyLinesGroup_ptr=PLG_ptr;
   initialize_Path_PolyLinesGroup();

   Path_PolyLinePickHandler_ptr=PLPH_ptr;
   initialize_Path_PolyLinePickHandler();

   set_CM_3D_ptr(CM_3D_ptr);
   Operations_ptr=O_ptr;
}		       

// ---------------------------------------------------------------------
// Member function initialize_Path_PolyLinePickHandler customizes
// member Path_PolyLinePickHandler so that it can be easily used for
// instantiating flight paths for UAV models.

void MODELSGROUP::initialize_Path_PolyLinesGroup()
{
//   cout << "inside MODELSGROUP::initialize_Path_PolyLinesGroup()" << endl;
//   cout << "Path_PolyLinesGroup_ptr = " << Path_PolyLinesGroup_ptr << endl;
   if (Path_PolyLinesGroup_ptr==NULL) return;
//   Path_PolyLinesGroup_ptr->set_width(3);
   Path_PolyLinesGroup_ptr->set_constant_vertices_altitude(
      get_grid_world_origin().get(2)+aircraft_altitude);

//   cout << "const vertices alt = "
//        << Path_PolyLinesGroup_ptr->get_constant_vertices_altitude()
//        << endl;
}

void MODELSGROUP::initialize_Path_PolyLinePickHandler()
{
   if (Path_PolyLinePickHandler_ptr==NULL) return;
   Path_PolyLinePickHandler_ptr->set_min_doubleclick_time_spread(0.05);
   Path_PolyLinePickHandler_ptr->set_max_doubleclick_time_spread(0.2);
   Path_PolyLinePickHandler_ptr->set_approx_range_to_polyline(100); // meters
   Path_PolyLinePickHandler_ptr->set_fix_PolyLine_altitudes_flag(true);
   Path_PolyLinePickHandler_ptr->set_z_offset(5);
   Path_PolyLinePickHandler_ptr->set_PolyLine_rather_than_Line_mode(true);
}

MODELSGROUP::~MODELSGROUP()
{
//   cout << "inside MODELSGROUP destructor" << endl;
   delete tracks_group_ptr;
   delete movers_group_ptr;
   destroy_all_MODELS();
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const MODELSGROUP& M)
{
   int node_counter=0;
   for (unsigned int n=0; n<M.get_n_Graphicals(); n++)
   {
      MODEL* MODEL_ptr=M.get_MODEL_ptr(n);
      outstream << "Model node # " << node_counter++ << endl;
      outstream << "Model = " << *MODEL_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Set & get methods
// ==========================================================================

vector<MODEL*> MODELSGROUP::get_all_MODEL_ptrs() const
{
//   cout << "inside MODELSGROUP::get_all_MODEL_ptrs()" << endl;
//   cout << "get_n_Graphicals() = " << get_n_Graphicals() << endl;
   vector<Graphical*> Graphical_ptrs=get_all_Graphical_ptrs();
   vector<MODEL*> MODEL_ptrs;
   for (unsigned int g=0; g<Graphical_ptrs.size(); g++)
   {
      MODEL_ptrs.push_back(static_cast<MODEL*>(Graphical_ptrs[g]));
   }
   return MODEL_ptrs;
}

// As of 6/27/12, we explicitly check whether webapps_outputs_subdir
// exists.  If not, set_tomcat_subdir() creates it.  Note that this
// may be dangerous.  For example, user cho might create
// webapps/LOST/outputs/ .  But then user los may not be able to write
// to it...

void MODELSGROUP::set_tomcat_subdir(std::string subdir)
{
//   std::cout << "inside MODELSGROUP::set_tomcat_subdir()" << std::endl;
//   std::cout << "subdir = " << subdir << std::endl;
   tomcat_subdir=subdir;
   webapps_outputs_subdir=tomcat_subdir+"outputs/";
   filefunc::dircreate(webapps_outputs_subdir);
}

// ==========================================================================
// Model creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Model from all other graphical insertion
// and manipulation methods...

MODEL* MODELSGROUP::generate_new_Model(
   string filename,int OSGsubPAT_number,int ID)
{
   model_filename=filename;
   return generate_new_Model(OSGsubPAT_number,ID);
}

MODEL* MODELSGROUP::generate_new_Model(int OSGsubPAT_number,int ID)
{
//   cout << "inside MODELSGROUP::generate_new_model()" << endl;
   if (ID==-1) ID=get_next_unused_ID();

   if (model_filename=="")
   {
      cout << "Error in MODELSGROUP::generate_new_Model()" << endl;
      cout << "No model filename specified!" << endl;
      return NULL;
   }
   osgGA::Terrain_Manipulator* TM_ptr=
      dynamic_cast<osgGA::Terrain_Manipulator*>(get_CM_3D_ptr());

   MODEL* curr_MODEL_ptr=new MODEL(
      get_pass_ptr(),get_grid_world_origin_ptr(),AnimationController_ptr,
      TM_ptr,ModeController_ptr,WindowManager_ptr,
      model_filename,instantiate_OBSFRUSTAGROUP_flag,
      instantiate_OBSFRUSTUMPickHandler_flag,ID);
   curr_MODEL_ptr->set_compute_zface_height_flag(
      compute_zface_height_flag);

   initialize_Graphical(curr_MODEL_ptr,curr_MODEL_ptr->get_model_node_ptr());
   GraphicalsGroup::insert_Graphical_into_list(curr_MODEL_ptr);

   insert_graphical_PAT_into_OSGsubPAT(curr_MODEL_ptr,OSGsubPAT_number);

   return curr_MODEL_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_Predator_Model_for_flight_PolyLine
// dynamically instantiates a Predator model to fly along the last
// PolyLine member of *Path_PolyLinesGroup_ptr.  It generates and
// fills a track for the Model based upon its flight Polyline.  This
// method also computes the UAV's variable orientation so that it
// points forward everywhere along the flight path.

MODEL* MODELSGROUP::generate_Predator_Model_for_flight_PolyLine(
   PolyLine* flight_PolyLine_ptr,bool broadcast_UAV_track_flag)
{
//   cout << "inside MODELSGROUP::generate_Predator_Model_for_flight_PolyLine()"        << endl;
//   cout << "flight_PolyLine_ptr->get_ID() = "
//        << flight_PolyLine_ptr->get_ID() << endl;

   string Predator_model_filename="dark_predator.osg";
//   string Predator_model_filename="blue_predator.osg";
//   string Predator_model_filename="mypredator.osg";

   int predator_OSGsubPAT_number;
   MODEL* predator_MODEL_ptr=generate_predator_MODEL(
      predator_OSGsubPAT_number,Predator_model_filename,Predator_speed);
//   cout << "predator_MODEL_ptr->get_ID() = " 
//        << predator_MODEL_ptr->get_ID() << endl;
   
   set_constant_scale(predator_MODEL_ptr,10);
//   set_constant_scale(predator_MODEL_ptr,30);	// for viewgraph generation

// Predator OBSFRUSTUM parameters:

   predator_MODEL_ptr->set_dynamically_compute_OBSFRUSTUM_flag(true);
   double predator_az_extent=10*PI/180;
   double predator_el_extent=10*PI/180;
   predator_MODEL_ptr->push_back_OBSFRUSTUM_roll(0*PI/180);
   predator_MODEL_ptr->push_back_OBSFRUSTUM_pitch(0*PI/180);

//   predator_MODEL_ptr->push_back_OBSFRUSTUM_z_base_face(
//      get_grid_world_origin().get(2)+35);		// AR #1
   predator_MODEL_ptr->push_back_OBSFRUSTUM_z_base_face(
      get_grid_world_origin().get(2)+71);		// Full Lubbock 3D map

// Assign pointer to Constant Hawk surface texture video to predator
// MODEL's movie ptr:

   Movie* Movie_ptr=NULL;

//   cout << "EarthRegionsGroup_ptr = " << EarthRegionsGroup_ptr << endl;
   if (EarthRegionsGroup_ptr != NULL)
   {
      int r=1;	// EarthRegion #1 = video surface texture
      Movie_ptr=EarthRegionsGroup_ptr->generate_EarthRegion_video_chip(r);
      cout << "Movie_ptr = " << Movie_ptr << endl;   

// On 9/16/08, we learned the painful and hard way that *Movie_ptr
// must have its alpha value initially set equal to zero so that the
// full CH video doesn't momentarily pop-up before its geometry is
// resized and its alpha value is set back to zero for the Baghdad
// demo!

      if (Movie_ptr != NULL)
      {
         Movie_ptr->set_alpha(0.0);
//         Movie_ptr->set_alpha(0.5);
//         Movie_ptr->set_alpha(1.0); 
      }
   }

   OBSFRUSTUM* OBSFRUSTUM_ptr=instantiate_predator_OBSFRUSTUM(
      predator_OSGsubPAT_number,predator_MODEL_ptr,
      predator_az_extent,predator_el_extent,Movie_ptr);
   
   OBSFRUSTUM_ptr->set_rectangular_movie_flag(false);

   generate_Aircraft_MODEL_track_and_mover(predator_MODEL_ptr);
   
   add_flight_path_arrows(flight_PolyLine_ptr);

   track* track_ptr=update_UAV_track(flight_PolyLine_ptr,predator_MODEL_ptr);
   recolor_UAV_track(track_ptr,flight_PolyLine_ptr);
   
   if (broadcast_UAV_track_flag)
   {
      broadcast_add_track_to_GoogleEarth_channel(track_ptr);
   }

   return predator_MODEL_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_aircraft_MODEL_for_flight_PolyLine()
// dynamically instantiates a Cessna model to fly along the last
// PolyLine member of *Path_PolyLinesGroup_ptr.  

MODEL* MODELSGROUP::generate_aircraft_MODEL_for_flight_PolyLine(
   PolyLine* flight_PolyLine_ptr,bool broadcast_UAV_track_flag)
{
   cout << "inside MODELSGROUP::generate_aircraft_MODEL_for_flight_PolyLine()"
        << endl;
//   cout << "flight_PolyLine_ptr->get_ID() = "
//        << flight_PolyLine_ptr->get_ID() << endl;

   int aircraft_OSGsubPAT_number;
   double model_scalefactor=20;
   MODEL* aircraft_MODEL_ptr=generate_Cessna_MODEL(
      aircraft_OSGsubPAT_number,model_scalefactor);
//   cout << "aircraft_MODEL_ptr->get_ID() = " 
//        << aircraft_MODEL_ptr->get_ID() << endl;

   aircraft_MODEL_ptr->set_dynamically_compute_OBSFRUSTUM_flag(true);

// On 26 Aug 2009, we learned from Peter Boettcher in Group 99 that
// the total the field-of-regard for the 12 Massivs imagers = 72
// degrees in both the U and V directions:

// WFOV OBSFRUSTUM parameters:

   double FOV_az_extent=72*PI/180;  	// radians
   double FOV_el_extent=72*PI/180;	// radians

// Recall dynamic OBSFRUSTA are initially instantiated so that they're
// nadir oriented.  They are then rolled away from nadir about the
// velocity vector and subsequently pitched forwards in the velocity
// direction:

   aircraft_MODEL_ptr->push_back_OBSFRUSTUM_roll(25*PI/180);
   aircraft_MODEL_ptr->push_back_OBSFRUSTUM_pitch(0*PI/180);

   OBSFRUSTUM* WFOV_OBSFRUSTUM_ptr=instantiate_MODEL_FOV_OBSFRUSTUM(
      aircraft_OSGsubPAT_number,aircraft_MODEL_ptr,
      FOV_az_extent,FOV_el_extent);
   WFOV_OBSFRUSTUM_ptr->set_rectangular_movie_flag(false);

// NFOV OBSFRUSTUM parameters:

   double NFOV_az_extent=5*PI/180;	// radians
   double NFOV_el_extent=5*PI/180;	// radians
   aircraft_MODEL_ptr->push_back_OBSFRUSTUM_roll(30*PI/180); 
   aircraft_MODEL_ptr->push_back_OBSFRUSTUM_pitch(10*PI/180);

   OBSFRUSTUM* NFOV_OBSFRUSTUM_ptr=instantiate_MODEL_FOV_OBSFRUSTUM(
      aircraft_OSGsubPAT_number,aircraft_MODEL_ptr,
      NFOV_az_extent,NFOV_el_extent);
   NFOV_OBSFRUSTUM_ptr->set_rectangular_movie_flag(false);

   double base_altitude_above_grid=50;		// meters
   double z_base_face=get_grid_world_origin().get(2)+base_altitude_above_grid;
   aircraft_MODEL_ptr->push_back_OBSFRUSTUM_z_base_face(z_base_face);
   aircraft_MODEL_ptr->push_back_OBSFRUSTUM_z_base_face(z_base_face);

   return aircraft_MODEL_ptr;
}

// ---------------------------------------------------------------------
// Member function instantiate_OBSFRUSTUM appends a new OBSFRUSTUM to
// *curr_MODEL_ptr's OBSFRUSTAGROUP.  It then incorporates the
// OBSFRUSTAGROUP's OSGgroup_ptr to the current MODELSGROUP's
// OSGsubPAT whose number is passed as an input parameter.

OBSFRUSTUM* MODELSGROUP::instantiate_OBSFRUSTUM(
   MODEL* MODEL_ptr,double az_extent,double el_extent,string movie_filename,
   int OSGsubPAT_number)
{
//   cout << "inside MODELSGROUP::instantiate_OBSFRUSTUM(Movie_ptr) #1" << endl;
//   cout << "movie_filename = " << movie_filename << endl;
   
   const double alpha=1.0;
   bool force_empty_movie_construction_flag=true;
   Movie* Movie_ptr=MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->
      get_MoviesGroup_ptr()->generate_new_Movie(
         movie_filename,alpha,-1,force_empty_movie_construction_flag);
   return instantiate_OBSFRUSTUM(MODEL_ptr,az_extent,el_extent,Movie_ptr,
                                 OSGsubPAT_number);
}

OBSFRUSTUM* MODELSGROUP::instantiate_OBSFRUSTUM(
   MODEL* MODEL_ptr,double az_extent,double el_extent,Movie* Movie_ptr,
   int OSGsubPAT_number)
{
//   cout << "inside MODELSGROUP::instantiate_OBSFRUSTUM(Movie_ptr) #2" << endl;
//   cout << "Movie_ptr = " << Movie_ptr << endl;

//   cout << "az_extent = " << az_extent
//        << " el_extent = " << el_extent << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=MODEL_ptr->
      get_OBSFRUSTAGROUP_ptr()->generate_new_OBSFRUSTUM(
         az_extent,el_extent,Movie_ptr);

   OBSFRUSTUM_ptr->instantiate_OSG_Pyramids();
   OBSFRUSTUM_ptr->generate_Pyramid_geodes();

   insert_OSGgroup_into_OSGsubPAT(
      MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->get_OSGgroup_ptr(),
      OSGsubPAT_number);

   return OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
// Member function move_z vertically displaces the model in proportion
// to its current Z value.  This allows the cessna model's altitude to
// be rapidly but grossly reset when the aircraft is high above the
// Z=0 plane.  On the other hand, vertical displacement of the
// airplane becomes more fine as its height approaches Z=0.

MODEL* MODELSGROUP::move_z(int sgn)
{
   MODEL* curr_MODEL_ptr=get_ID_labeled_MODEL_ptr(
      get_selected_Graphical_ID());
   if (curr_MODEL_ptr==NULL) return NULL;

   threevector curr_posn;
   if (curr_MODEL_ptr->get_UVW_coords(
      get_curr_t(),get_passnumber(),curr_posn))
   {
      double delta_z=0.05*curr_posn.get(2);
      return dynamic_cast<MODEL*>(GraphicalsGroup::move_z(sgn*delta_z));
   }
   return NULL;
}

// ---------------------------------------------------------------------
// Member function change_scale

void MODELSGROUP::change_scale(double scalefactor)
{
//   cout << "inside MODELSGROUP::change_scale()" << endl;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      MODEL* curr_MODEL_ptr=get_MODEL_ptr(n);

      threevector scale;
      if (curr_MODEL_ptr != NULL &&
          curr_MODEL_ptr->get_scale(get_curr_t(),get_passnumber(),scale))
      {
         for (unsigned int i=get_first_framenumber(); i<=get_last_framenumber(); i++)
         {
            double curr_t=double(i);
            curr_MODEL_ptr->set_scale(
               curr_t,get_passnumber(),scalefactor*scale);
         }
      }
   } // loop over index n labeling models
}

// ---------------------------------------------------------------------
// Member function unmask_next_model was written for demonstration
// purposes.  For clarity's sake, it's useful to start the San
// Clemente tour with both the LiMIT and predator models masked off.
// Then we can call this method once to turn on the LiMIT model and
// once again to turn on the predator model.  The order in which the
// models are instantiated within the main program determines the
// order in which they are unmasked by this method.

void MODELSGROUP::unmask_next_model()
{
   cout << "inside MODELSGROUP::unmask_next_model()" << endl;
   if (model_counter >= get_n_OSGsubPATs()) model_counter=0;
   set_OSGsubPAT_nodemask(model_counter++,1);
   update_display();
}		       

void MODELSGROUP::toggle_model_mask(int OSGsubPAT_number)
{
   cout << "inside MODELSGROUP::toggle_model_mask()" << endl;
   toggle_OSGsubPAT_nodemask(OSGsubPAT_number);
   update_display();
}		       

// --------------------------------------------------------------------------
// Member function update_display is repeatedly executed by a callback
// in a main program.

void MODELSGROUP::update_display()
{   
//   cout << "inside MODELSGROUP::update_display()" << endl;
   
   parse_latest_messages();

   double min_altitude=get_grid_world_origin().get(2)+1000;	// meters
   follow_selected_Geometrical(min_altitude);

   AnimationController* AnimationController_ptr=get_AnimationController_ptr();
   if (AnimationController_ptr != NULL)
   {
      double curr_t=AnimationController_ptr->
         get_time_corresponding_to_curr_frame();

// If continuously_plan_UAV_paths_flag==true and world clock is not
// paused, periodically broadcast sensor and target statevectors so
// that Luca Bertucelli's algorithms will update the multi-UAV flight
// paths:

      if (continuously_plan_UAV_paths_flag &&
          AnimationController_ptr->getState()==AnimationController::PLAY &&
          get_n_Graphicals() > 0 &&
          get_Path_PolyLinesGroup_ptr() != NULL &&
          get_Path_PolyLinePickHandler_ptr() != NULL)
      {
         double delta_t=curr_t-prev_UAV_path_planning_time;
         const double delta_t_between_updates=10;	// secs in world time
         if (delta_t > delta_t_between_updates)
         {
//            cout << "delta_t = " << delta_t << endl;
            prev_UAV_path_planning_time=curr_t;
            broadcast_sensor_and_target_statevectors();
         }
      }
    
// Check for encounters between UAV MODELS and fixed ROI or dynamic
// vehicle ground movers:

      if (EarthRegionsGroup_ptr != NULL)
      {
         EarthRegion* EarthRegion_ptr=EarthRegionsGroup_ptr->
            get_ID_labeled_EarthRegion_ptr(0);
         if (EarthRegion_ptr != NULL)
         {
            movers_group* ground_movers_group_ptr=EarthRegion_ptr->
               get_movers_group_ptr();
            if (ground_movers_group_ptr != NULL)
            {
               ground_movers_group_ptr->
                  check_for_UAV_ground_target_encounters(
                     curr_t,movers_group_ptr,mover::ROI);
               ground_movers_group_ptr->
                  check_for_UAV_ground_target_encounters(
                     curr_t,movers_group_ptr,mover::VEHICLE);
            }
         } // EarthRegion_ptr != NULL conditional
      } // EarthRegionsGroup_ptr != NULL conditional
   } // AnimationController_ptr != NULL conditional
   
// Compute & display dynamic OBSFRUSTA only if they're not currently
// masked:

   vector<MODEL*> MODEL_ptrs=get_all_MODEL_ptrs();
//   cout << "MODEL_ptrs.size() = " << MODEL_ptrs.size() << endl;
   
   for (unsigned int n=0; n<MODEL_ptrs.size(); n++)
   {
      MODEL* MODEL_ptr=MODEL_ptrs[n];

      if (MODEL_ptr->get_dynamically_compute_OBSFRUSTUM_flag())
      {
         int OSGsubPAT_number=OSGsubPAT_parent_of_Graphical(MODEL_ptr);
         int nodemask=get_OSGsubPAT_nodemask(OSGsubPAT_number);
         if (nodemask==1 && !compute_skymap_flag)
         {
            bool OBSFRUSTA_previously_built_flag=false;
            vector<OBSFRUSTUM*> OBSFRUSTA_ptrs=
               MODEL_ptr->compute_dynamic_OBSFRUSTA(
                  get_curr_t(),get_passnumber(),
                  EarthRegionsGroup_ptr,n_future_repeats,
                  OBSFRUSTA_previously_built_flag);

         } // nodemask==1 conditional
      } // dynamically_compute_OBSFRUSTUM_flag conditional
   } // loop over index n labeling MODELs

// If a new Path_PolyLine has been fully entered, instantiate a UAV
// model to fly along it.  Assign most recently added PolyLine's ID to
// the new UAV:

   if (Path_PolyLinesGroup_ptr != NULL)
   {
      unsigned int n_Path_PolyLines=
         Path_PolyLinesGroup_ptr->get_n_Graphicals();
      if (n_Path_PolyLines > 0 && n_Path_PolyLines > get_n_Graphicals())
      {
//         cout << "n_Path_PolyLines = " << n_Path_PolyLines << endl;
//         cout << "most recently added PolyLine ID = "
//              << Path_PolyLinesGroup_ptr->get_most_recently_added_ID()
//              << endl;
         
         PolyLine* Path_PolyLine_ptr=static_cast<PolyLine*>(
            Path_PolyLinesGroup_ptr->get_most_recently_added_Graphical_ptr());
         
// FAKE FAKE:  Tuesday, February 3 at 7 am

// As of Tuesday, Feb 3 at 7 am, we can't get alpha blending to work
// for PolyLines.  But at least cutting the value V by a factor of 2
// may be sufficient for the dynamic ground mover multi-UAV problem...


//         double alpha=0.05;
         Path_PolyLine_ptr->set_permanent_color(
            UAV_flight_path_color(Path_PolyLine_ptr->get_ID()));
//            UAV_flight_path_color(Path_PolyLine_ptr->get_ID()),alpha);
//         Path_PolyLine_ptr->set_curr_color(
//            UAV_flight_path_color(Path_PolyLine_ptr->get_ID()),alpha);
//         Path_PolyLine_ptr->dirtyDisplay();

// On 8/20/08, we added the following contorted logic so that Predator
// MODELS and their OBSFRUSTA are not generated on the current OSG
// update cycle but rather on the following one.  We have empirically
// found that instantiating a Predator from an entered flight path is
// time-consuming.  So the user has to wait several seconds from the
// time he double-clicks(taps) to the time when the Predator's future
// history is instantiated.  We prefer that the user quickly see the
// elevated version of his entered ground path followed later by the
// Predator with its OBSFRUSTUM so that he receives prompt visual
// feedback as to when his entry of the flight path is terminated:

// This ugly hack should someday be replaced by spinning off a thread
// which handled instantiation of the time-consuming Predator and its
// OBSFRUSTUM while the rest of the OSG event loop can proceed...

         if (Path_PolyLine_ptr->get_entry_finished_flag())
         {
            if (generate_Predator_model_on_next_cycle_flag)
            {
               generate_Predator_model_on_next_cycle_flag=false;
               if (aircraft_model_type==predator)
               {
                  generate_Predator_Model_for_flight_PolyLine(
                     Path_PolyLine_ptr);
               }
               else if (aircraft_model_type==cessna)
               {
                  generate_aircraft_MODEL_for_flight_PolyLine(
                     Path_PolyLine_ptr);
               }
            }
            else if (!generate_Predator_model_on_next_cycle_flag &&
                     n_Path_PolyLines > get_n_Graphicals() &&
                     !alter_UAV_path_flag)
            {
               generate_Predator_model_on_next_cycle_flag=true;               
            }
            else if (alter_UAV_path_flag)
            {
               alter_UAV_Path_PolyLine();
            }
         } // Path_Polyline entry finished flag conditional
      }
   } // Path_PolyLinesGroup_ptr != NULL conditional
    
   GraphicalsGroup::update_display();
}

// ==========================================================================
// Model destruction methods
// ==========================================================================
 
void MODELSGROUP::destroy_all_MODELS()
{
//   cout << "inside MODELSGROUP::destroy_all_MODELS()" << endl;
   unsigned int n_MODELs=get_n_Graphicals();

   vector<MODEL*> MODELs_to_destroy;
   for (unsigned int p=0; p<n_MODELs; p++)
   {
      MODEL* MODEL_ptr=get_MODEL_ptr(p);
      MODELs_to_destroy.push_back(MODEL_ptr);
   }

   for (unsigned int p=0; p<n_MODELs; p++)
   {
      destroy_MODEL(MODELs_to_destroy[p]);
   }
   set_selected_Graphical_ID(-1);
}

// --------------------------------------------------------------------------
bool MODELSGROUP::destroy_MODEL(int ID)
{   
//   cout << "inside MODELsGroup::destroy_MODEL(int ID)" << endl;
//   cout << "int ID = " << ID << endl;
   if (ID >= 0)
   {
      return destroy_MODEL(get_ID_labeled_MODEL_ptr(ID));
   }
   else
   {
      return false;
   }
}

// --------------------------------------------------------------------------
bool MODELSGROUP::destroy_last_created_MODEL()
{   
//   cout << "inside MODELSGROUP::destroy_last_created_MODEL()" << endl;

   int ID=get_n_Graphicals()-1;
   return destroy_MODEL(ID);
}

// ---------------------------------------------------------------------
bool MODELSGROUP::destroy_MODEL(MODEL* curr_MODEL_ptr)
{
//   cout << "inside MODELSGROUP::destroy_MODEL(curr_MODEL_ptr)" << endl;
//   cout << "curr_MODEL_ptr = " << curr_MODEL_ptr << endl;
//   cout << "MODELSGROUP this = " << this << endl;

   if (curr_MODEL_ptr==NULL) return false;

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=curr_MODEL_ptr->
      get_OBSFRUSTAGROUP_ptr();

// Destroy any video chips specifically associated with MODEL
// OBSFRUSTA:

   if (EarthRegionsGroup_ptr != NULL)
   {
      int r=1;	// EarthRegion #1 = video surface texture

      for (unsigned int n=0; n<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); n++)
      {
         OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(n);
         Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();

         if (Movie_ptr != NULL)
         {
            EarthRegionsGroup_ptr->destroy_EarthRegion_video_chip(
               r,Movie_ptr);
         }
      }
   }

// Recall that OBSFRUSTAGROUP is added as a child to OSGsubPAT.  So we
// must explicitly remove it from the scenegraph before the MODEL is
// destroyed:

   if (OBSFRUSTAGROUP_ptr != NULL)
   {
      remove_OSGgroup_from_OSGsubPAT(OBSFRUSTAGROUP_ptr->get_OSGgroup_ptr());
   }

   bool flag=destroy_Graphical(curr_MODEL_ptr);

//    for (unsigned int n=0; n<get_n_Graphicals(); n++)
//    {
//       Graphical* Graphical_ptr=get_Graphical_ptr(n);
//       cout << "n = " << n 
//           << " Graphical_ptr = " << Graphical_ptr 
//           << " Graphical_ptr->get_ID() = " << Graphical_ptr->get_ID()
//           << endl;
//    }

   return flag;
}

// ==========================================================================
// Model path creation methods
// ==========================================================================

void MODELSGROUP::record_waypoint()
{
//   cout << "inside MODELSGROUP::record_waypoint()" << endl;
   
   MODEL* curr_MODEL_ptr=get_ID_labeled_MODEL_ptr(
      get_selected_Graphical_ID());

   threevector curr_posn;
   if (curr_MODEL_ptr != NULL && curr_MODEL_ptr->get_UVW_coords(
      get_curr_t(),get_passnumber(),curr_posn))
   {
      candidate_waypoint.push_back(curr_posn);
      cout << "i = " << candidate_waypoint.size()
           << " waypoint-grid_origin = " 
           << candidate_waypoint.back()-get_grid_world_origin() << endl;
   }
}

// ---------------------------------------------------------------------
void MODELSGROUP::finish_waypoint_entry()
{
//   cout << "inside MODELSGROUP::finish_waypoint_entry()" << endl;

   waypoint.clear();
   for (unsigned int i=0; i<candidate_waypoint.size(); i++)
   {
      waypoint.push_back(candidate_waypoint[i]);
   }
   candidate_waypoint.clear();

   MODEL* curr_MODEL_ptr=get_ID_labeled_MODEL_ptr(
      get_selected_Graphical_ID());

   PolyLine* curr_PolyLine_ptr=Path_PolyLinesGroup_ptr->generate_new_PolyLine(
      waypoint,colorfunc::get_OSG_color(colorfunc::white));
   polyline* curr_polyline_ptr=curr_PolyLine_ptr->construct_polyline();
   curr_polyline_ptr->compute_regularly_spaced_edge_points(
      curr_MODEL_ptr->get_path_update_distance(),even_path_point);

   double roll=0;
   double pitch=0;
   const threevector north_hat(y_hat);
   vector<threevector> RPY;
   for (unsigned int i=0; i<even_path_point.size()-1; i++)
   {
      threevector ds(even_path_point[i+1]-even_path_point[i]);

      pair<double,threevector> p=mathfunc::angle_and_axis_between_unitvectors(
         north_hat,ds.unitvector());
      double yaw=p.first;
      double dotproduct=z_hat.dot(p.second);
      yaw *= sgn(dotproduct);

      RPY.push_back(threevector(roll,pitch,yaw));
   }

// Assume aircraft's attitude at last sample point is the same as at
// its next-to-last sample point:

   RPY.push_back(RPY.back());

// On 11/3/06, we learned the painful and hard way that model scale
// information must be explicitly expressed for each temporal sample
// in addition to model attitude and position :

   threevector initial_scale;
   curr_MODEL_ptr->get_scale(
      get_initial_t(),get_passnumber(),initial_scale);
   curr_MODEL_ptr->set_scale_attitude_posn(
      get_passnumber(),initial_scale,RPY,even_path_point);

// Set number of frames within Animation Controller equal to number of
// evenly sampled path points:

   AnimationController_ptr->set_nframes(even_path_point.size());
}

// ---------------------------------------------------------------------
// Member function update_UAV_track() fills the input UAV model's
// track with time, position and velocity information.  It also
// reorients the UAV model at each sample point so that it's pointed
// along the flight path. 

track* MODELSGROUP::update_UAV_track(
   PolyLine* flight_PolyLine_ptr,MODEL* UAV_MODEL_ptr)
{
   int initial_framenumber=
      get_AnimationController_ptr()->get_curr_framenumber();
   return update_UAV_track(initial_framenumber,flight_PolyLine_ptr,
                           UAV_MODEL_ptr);
}

track* MODELSGROUP::update_UAV_track(
   int initial_framenumber,PolyLine* flight_PolyLine_ptr,MODEL* UAV_MODEL_ptr)
{
//   cout << "inside MODELSGROUP::update_UAV_track()" << endl;
//   cout << "initial_framenumber = " << initial_framenumber << endl;
//   cout << "flight_PolyLine_ptr = " << flight_PolyLine_ptr << endl;
//   cout << "*flight_PolyLine_ptr = " << *flight_PolyLine_ptr << endl;
//   cout << "UAV_MODEL_ptr = " << UAV_MODEL_ptr << endl;
//   cout << "UAV_MODEL_ptr->get_ID() = " << UAV_MODEL_ptr->get_ID() << endl;

// Compute flight path sampling distance based upon Predator speed and
// world game clock time step:

   double delta_world_time=
      AnimationController_ptr->get_delta_world_time_per_frame();
   if (nearly_equal(delta_world_time,0)) delta_world_time=1.0;	// sec
   double delta_distance=UAV_MODEL_ptr->get_speed()*delta_world_time;
//   cout << "delta_world_time = " << delta_world_time << endl;
//   cout << "AC.firstframenumber = " << AnimationController_ptr->
//      get_first_framenumber() << endl;
//   cout << "AC.lastframenumber = " << AnimationController_ptr->
//      get_last_framenumber() << endl;
//   cout << "AC.get_nframes() = " << AnimationController_ptr->
//      get_nframes() << endl;

//   cout << "UAV speed = " << UAV_MODEL_ptr->get_speed() << endl;
//   cout << "delta_distance = " << delta_distance << endl;

   polyline* flight_polyline_ptr=flight_PolyLine_ptr->
      get_or_set_polyline_ptr();
//   cout << "flight_polyline_ptr = " << flight_polyline_ptr << endl;
//   cout << "initial *flight_polyline_ptr = "
//        << *flight_polyline_ptr << endl;
   
   flight_polyline_ptr->compute_regularly_spaced_edge_points(
      delta_distance,even_path_point);

// Check whether number of evenly spaced path points is less than
// remaining number of frames.  If not, jettison track points which
// will not be reached by
// AnimationController_ptr()->get_last_framenumber():

   vector<threevector> constrained_path_point;
   for (unsigned int i=0; i<even_path_point.size(); i++)
   {
      int future_framenumber=initial_framenumber+i;
      if (future_framenumber <= get_AnimationController_ptr()->
          get_last_framenumber())
      {
         constrained_path_point.push_back(even_path_point[i]);
      }
      else
      {
         cout << "Warning in MODELSGROUP::update_UAV_track()" << endl;
         cout << "i = " << i << " future_framenumber = " << future_framenumber
              << " exceeds last framenumber = "
              << get_AnimationController_ptr()->get_last_framenumber()
              << endl;
//         outputfunc::enter_continue_char();
      }
   } // loop over index i labeling evenly sampled path points
   
// Recopy contents of constrained_path_point back into even_path_point:

// For the TOC11 red actor path planning problem, we want to be able
// to vary the starting phase offset of the airborne sensor.  So add
// fractional offset i_offset to index i and then cyclically permute
// its value so that it lies within [0,n_even_points-1]:

   unsigned int n_even_points=constrained_path_point.size();
   unsigned int i_offset=flightpath_fraction_offset*n_even_points;
         
   even_path_point.clear();
   for (unsigned int i=0; i<n_even_points; i++)
   {
      unsigned int j=(i+i_offset)%n_even_points;
      even_path_point.push_back(constrained_path_point[j]);
   }
//   cout << "even_path_point.size() = " << even_path_point.size() << endl;

   double roll=0;
   double pitch=0;
   const threevector north_hat(y_hat);

   track* UAV_track_ptr=UAV_MODEL_ptr->get_track_ptr();
//   cout << "UAV_track_ptr = " << UAV_track_ptr << endl;
   UAV_track_ptr->purge_all_values();

   vector<threevector> RPY;
   for (unsigned int i=0; i<even_path_point.size(); i++)
   {
//      cout << "i = " << i << " evenX = " << even_path_point[i].get(0)
//           << " evenY = " << even_path_point[i].get(1) << endl;

      int future_framenumber=initial_framenumber+i;
      if (future_framenumber > get_AnimationController_ptr()->
          get_last_framenumber())
      {
         cout << "Error in update_UAV_track() !!!" << endl;
         cout << "future_framenumber = " << future_framenumber
              << " exceeds last framenumber = "
              << get_AnimationController_ptr()->get_last_framenumber()
              << endl;
         continue;
      }

      double future_t=get_AnimationController_ptr()->
         get_time_corresponding_to_frame(future_framenumber);

      double curr_frac=flight_polyline_ptr->frac_distance_along_polyline(
         even_path_point[i]);
      curr_frac=basic_math::max(0.001,curr_frac);
      curr_frac=basic_math::min(0.999,curr_frac);

      threevector position=flight_polyline_ptr->edge_point(curr_frac);
      threevector velocity=Predator_speed*
         flight_polyline_ptr->edge_direction(curr_frac);

// Perform expensive temporally sorting of UAV track positions and
// velocities only on final iteration over loop index i!

      bool temporally_sort_flag=false;
      if (i==even_path_point.size()-1)
      {
         temporally_sort_flag=true;
      }
      UAV_track_ptr->set_posn_velocity(
         future_t,position,velocity,temporally_sort_flag);
//         future_t,even_path_point[i],velocity,temporally_sort_flag);
       
      pair<double,threevector> p=
         mathfunc::angle_and_axis_between_unitvectors(
            north_hat,velocity.unitvector());

      double yaw=p.first;
      double dotproduct=z_hat.dot(p.second);
      yaw *= sgn(dotproduct);

      RPY.push_back(threevector(roll,pitch,yaw));
   } // loop over index i labeling regularly samples of flight polyline

//   cout << "*UAV_track_ptr = " << *UAV_track_ptr << endl;
//   cout << "UAV_track_ptr = " << UAV_track_ptr << endl;
//   cout << "UAV_track_ptr->size() = "
//        << UAV_track_ptr->size() << endl;

   if (RPY.size() != even_path_point.size())
   {
      cout << "Error in MODELSGROUP::update_UAV_track()" << endl;
      cout << "RPY.size() = " << RPY.size() << endl;
      cout << "even_path_point.size() = " << even_path_point.size() << endl;
      outputfunc::enter_continue_char();
   }

// As of 8/16/08, we implement a poor-man's loitering of the UAV at
// its last specified position with its last computed velocity:

   unsigned int i_start=even_path_point.size();

   cout << "i_start = " << i_start
        << " last_framenumber = " << get_AnimationController_ptr()->
      get_last_framenumber() << endl;
   
   for (unsigned int i=i_start; 
        i<(unsigned int) get_AnimationController_ptr()->
           get_last_framenumber(); i++)
   {

// If !loiter_at_track_end_flag, eliminate loitering when UAV reaches
// end of flight path.  Instead, start UAV over at its beginning
// location:

      if (loiter_at_track_end_flag)
      {
         even_path_point.push_back(even_path_point.back());
         RPY.push_back(RPY.back());
      }
      else
      {
         int i_new=i%i_start;
//         cout << "i = " << i << " i_start = " << i_start
//              << " i_new = " << i_new << endl;
         even_path_point.push_back(even_path_point[i_new]);
         RPY.push_back(RPY[i_new]);
      }
   } // loop over index i labeling anim frames for which no UAV path 
     //  was specified

   if (RPY.size() != even_path_point.size())
   {
      cout << "Error in MODELSGROUP::update_UAV_track()" << endl;
      cout << "RPY.size() = " << RPY.size() << endl;
      cout << "even_path_point.size() = " << even_path_point.size() << endl;
      outputfunc::enter_continue_char();
   }
   
   UAV_MODEL_ptr->Graphical::set_attitude_posn(
      get_passnumber(),RPY,even_path_point,initial_framenumber);

   return UAV_track_ptr;
}

// ---------------------------------------------------------------------
// Member function recolor_UAV_track assigns the flight path's RGB
// color to input track *UAV_track_ptr.

void MODELSGROUP::recolor_UAV_track(
   track* UAV_track_ptr,PolyLine* flight_PolyLine_ptr)
{
//   cout << "inside MODELSGROUP::recolor_UAV_track()" << endl;
//   cout << "flight_PolyLine_ptr = " << flight_PolyLine_ptr << endl;
//   cout << "UAV_track_ptr = " << UAV_track_ptr << endl;
//   cout << "*flight_PolyLine_ptr = " << *flight_PolyLine_ptr << endl;

// Assign flight path's RGB color to UAV_track:

   colorfunc::Color flight_path_color=
      UAV_flight_path_color(flight_PolyLine_ptr->get_ID());
   colorfunc::RGB flight_path_RGB=
      colorfunc::get_RGB_values(flight_path_color);
   UAV_track_ptr->set_RGB_color(flight_path_RGB);

   if (fade_UAV_track_color_flag)
   {
      flight_PolyLine_ptr->compute_color_fading(flight_path_RGB);
      flight_PolyLine_ptr->set_multicolor_flag(true);
   }
}

// ---------------------------------------------------------------------
// Member function generate_racetrack_orbit takes in parameters which
// uniquely define a circular racetrack orbit.  It computes the
// model's XYZ position and roll, pitch, yaw orientation (in radians)
// as functions of orbit position.  This method returns the number of
// frames which the model takes to complete the orbit.

void MODELSGROUP::generate_racetrack_orbit(
   MODEL* curr_MODEL_ptr,double center_longitude,double center_latitude,
   double radius,double height_above_center,
   double orbit_period,double phase_offset)
{
   geopoint racetrack_center(center_longitude,center_latitude);

//   cout << "racetrack_center = " << racetrack_center << endl;
   threevector center_origin(
      racetrack_center.get_UTM_easting(),
      racetrack_center.get_UTM_northing(),0);
   generate_racetrack_orbit(
      curr_MODEL_ptr,center_origin,radius,height_above_center,
      orbit_period,phase_offset);
}

void MODELSGROUP::generate_racetrack_orbit(
   MODEL* curr_MODEL_ptr,const threevector& center_origin,
   double radius,double height_above_center,double orbit_period,
   double phase_offset)
{
   double t_start=0;
   double dt=1;
   double omega=2*PI/orbit_period;

   for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); n++)
   {
      double t=t_start+n*dt;
      double theta=omega*t+phase_offset;
      double x=center_origin.get(0)+radius*cos(theta);
      double y=center_origin.get(1)+radius*sin(theta);
      double z=center_origin.get(2)+height_above_center;
      threevector posn(x,y,z);

      double roll=0;	      	// radians
      double pitch=0;		// radians
      double yaw=theta;		// radians
      threevector RPY(roll,pitch,yaw);

      curr_MODEL_ptr->set_attitude_posn(n,get_passnumber(),RPY,posn);
   } // loop over index n labeling angular posn along racetrack orbit
}

// ---------------------------------------------------------------------
// Member function generate_racetrack_orbit takes in an STL vector
// containing regularized aircraft positions which are assumed to be
// evenly matched to the master game clock.  It uses the regularized
// values to adjust input MODEL's position and attitude as a function
// of game time.

void MODELSGROUP::generate_racetrack_orbit(
   const vector<threevector>& posn_reg,MODEL* MODEL_ptr)
{
   string banner="Generating racetrack orbit";
   outputfunc::write_banner(banner);

   unsigned int first_frame=
      get_AnimationController_ptr()->get_first_framenumber();
   unsigned int last_frame=
      get_AnimationController_ptr()->get_last_framenumber();

   threevector RPY_init,RPY;
   for (unsigned int n=first_frame+1;n<=last_frame-1; n++)
   {
      threevector v_prop(posn_reg[n+1]-posn_reg[n]);
      double curr_phi=atan2(v_prop.get(1),v_prop.get(0));
      double yaw=curr_phi-PI/2;	// radians
      double roll=0;	      	// radians
      double pitch=0;		// radians
      RPY=threevector(roll,pitch,yaw);
      if (n==first_frame+1)
      {
         RPY_init=RPY;
      }
      
      MODEL_ptr->set_attitude_posn(n,get_passnumber(),RPY,posn_reg[n]);
   } // loop over index n labeling angular posn along racetrack orbit

   MODEL_ptr->set_attitude_posn(
      first_frame,get_passnumber(),RPY_init,posn_reg.front());
   MODEL_ptr->set_attitude_posn(
      last_frame,get_passnumber(),RPY,posn_reg.back());
}

// ---------------------------------------------------------------------
// Member function generate_circular_PolyLine_Path() takes in center
// longitude,latitude geocoords as well as an orbit radius.  It
// generates a circular PolyLine member of *Path_PolyLinesGroup_ptr.

PolyLine* MODELSGROUP::generate_circular_PolyLine_Path(
   double center_longitude,double center_latitude,double orbit_radius,
   int flightpath_sgn)
{
//   cout << "inside MODELSGROUP::generate_circular_PolyLine_Path()" << endl;

   if (Path_PolyLinesGroup_ptr==NULL)
   {
      cout << "Error in MODELSGROUP::generate_circular_PolyLine_Path()"
           << endl;
      cout << "Path_PolyLinesGroup_ptr=NULL!" << endl;
      return NULL;
   }

   double center_altitude=aircraft_altitude;
//   cout << "center_altitude = " << center_altitude << endl;
   int UTM_zone=get_EarthRegionsGroup_ptr()->get_specified_UTM_zonenumber();
   geopoint orbit_center(center_longitude,center_latitude,center_altitude,
                         UTM_zone);

   double orbit_length=2*PI*orbit_radius;
   double delta_t=5*60;	 // secs
   double delta_s=LiMIT_speed*delta_t;

// Note added on Sunday, 4/6/2014: Do NOT change n_samples from int to
// unsigned int.  Doing so messes up circular flight paths within LOST!

   int n_samples=orbit_length/delta_s;

// Perform sanity check on number of circular path samples:

   if (n_samples < 20)
   {
      n_samples=20;
      delta_s=orbit_length/n_samples;
      delta_t=delta_s/LiMIT_speed;
   }
   double d_theta=2*PI/n_samples;
   
//   cout << "n_samples = " << n_samples << " delta_s = " << delta_s << endl;
//   cout << "delta_t = " << delta_t << " secs" << endl;
//   cout << "d_theta = " << d_theta*180/PI << endl;

   vector<threevector> V;
   for (int n=0; n<=n_samples; n++)
   {
      double theta=flightpath_sgn*n*d_theta;
      threevector curr_posn=orbit_center.get_UTM_posn()+
         orbit_radius*threevector(cos(theta),sin(theta));
      V.push_back(curr_posn);
//      cout << "n = " << n << " theta = " << theta*180/PI << endl;
   }

   PolyLine* circular_PolyLine_ptr=Path_PolyLinesGroup_ptr->
      generate_new_PolyLine(V);
   unsigned int n_Path_PolyLines=Path_PolyLinesGroup_ptr->get_n_Graphicals();
   cout << "n_Path_PolyLines = " << n_Path_PolyLines << endl;

   circular_PolyLine_ptr->set_entry_finished_flag(true);
   return circular_PolyLine_ptr;
}

// ---------------------------------------------------------------------
// Member function regularize_aircraft_posns_and_orientations takes in
// an STL vector containing airborne sensor position and orientation
// information as functions of movie frame number.  It instantiates
// piecewise_linear_vector objects in order to interpolate the
// irregularly sampled aircraft positions and orientations onto evenly
// spaced temporal bins.

void MODELSGROUP::regularize_aircraft_posns_and_orientations(
   const vector<Triple<threevector,rpy,int> >& sensor_posn_orientation_frames,
   AnimationController* movie_anim_controller_ptr,
   vector<threevector>& posn_reg,vector<threevector>& orientation_reg)
{
   vector<double> T;
   vector<threevector> sensor_posn;
   vector<rpy> sensor_orientation;

   for (unsigned int i=0; i<sensor_posn_orientation_frames.size(); i++)
   {
      sensor_posn.push_back(sensor_posn_orientation_frames[i].first);
      sensor_orientation.push_back(sensor_posn_orientation_frames[i].second);
      int curr_framenumber=sensor_posn_orientation_frames[i].third;
      double curr_time=movie_anim_controller_ptr->
         get_time_corresponding_to_frame(curr_framenumber);
      T.push_back(curr_time);
   }

   piecewise_linear_vector PLV_posn(T,sensor_posn);
   piecewise_linear_vector PLV_orientation(T,sensor_orientation);

   unsigned int first_frame=
      get_AnimationController_ptr()->get_first_framenumber();
   unsigned int last_frame=
      get_AnimationController_ptr()->get_last_framenumber();
//   int n_animation_frames=last_frame-first_frame+1;
//   cout << "n_anim_frames = " << n_animation_frames << endl;

   double curr_T;
   posn_reg.clear();
   orientation_reg.clear();
   for (unsigned int n=first_frame; n<=last_frame; n++)
   {
      curr_T=get_AnimationController_ptr()->
         get_time_corresponding_to_frame(n);
      posn_reg.push_back(PLV_posn.value(curr_T));
      orientation_reg.push_back(PLV_orientation.value(curr_T));
//      cout << "n = " << n 
//           << " " << get_AnimationController_ptr()->get_world_time_string()
//           << endl;
//      cout << "curr_posn = " << posn_reg.back() << endl;
//      cout << "curr_orientation = " << orientation_reg.back() << endl;
   }
}

// ==========================================================================
// Aircraft MODEL generation member functions
// ==========================================================================

// Member function generate_predator_and_LiMIT_Models is a specialized
// method developed specifically for the ISDS integrated demo in
// February 2007.  It puts predator and AWACS aircraft models into
// notional racetrack orbits.

MODEL* MODELSGROUP::generate_Cessna_MODEL(
   int& OSGsubPAT_number,double model_scalefactor,
   colorfunc::Color cessna_color)
{
   string banner="Generating Cessna MODEL";
   outputfunc::write_banner(banner);

   string cessna_dir(getenv("OSG_FILE_PATH"));
   cessna_dir += "/";
   string model_filename=cessna_dir+"my_cessna.osg";
   if (cessna_color==colorfunc::yellow)
   {
      model_filename=cessna_dir+"yellow_cessna.osg";
   }

   OSGsubPAT_number=get_n_OSGsubPATs();
   MODEL* cessna_ptr=generate_new_Model(model_filename,OSGsubPAT_number);

//   cout << "model_scalefactor = " << model_scalefactor << endl;
   set_constant_scale(cessna_ptr,model_scalefactor);
   return cessna_ptr;
}

MODEL* MODELSGROUP::generate_predator_MODEL(
   string model_filename,double predator_speed,double model_scalefactor)
{
   int OSGsubPAT_number;
   return generate_predator_MODEL(
      OSGsubPAT_number,model_filename,predator_speed,model_scalefactor);
}

MODEL* MODELSGROUP::generate_predator_MODEL(
   int& OSGsubPAT_number,string model_filename,
   double predator_speed,double model_scalefactor)
{
//   cout << "inside MODELSGROUP::generate_predator_MODEL()" << endl;
   string banner="Generating predator MODEL";
//   cout << "model_scalefactor = " << model_scalefactor << endl;
   outputfunc::write_banner(banner);

   string UAV_dir(getenv("OSG_FILE_PATH"));
   UAV_dir += "/Predator/";
   if (model_filename.size()==0)
   {
      model_filename=UAV_dir+"mypredator.osg";
   }
   else
   {
      model_filename=UAV_dir+model_filename;
   }

   OSGsubPAT_number=get_n_OSGsubPATs();
   MODEL* predator_MODEL_ptr=generate_new_Model(
      model_filename,OSGsubPAT_number);
   predator_MODEL_ptr->set_speed(predator_speed);

   return predator_MODEL_ptr;
}

MODEL* MODELSGROUP::generate_LiMIT_MODEL(
   int& OSGsubPAT_number,bool use_AWACS_model_flag)
{
//   cout << "inside MODELSGROUP::generate_LiMIT_MODEL()" << endl;
   string banner="Generating LiMIT MODEL";
   outputfunc::write_banner(banner);

   string LiMIT_dir(getenv("OSG_FILE_PATH"));

   if (use_AWACS_model_flag)
   {
      LiMIT_dir += "/AWACS/MESHES/";
      model_filename=LiMIT_dir+"AWACS.osg";
   }
   else
   {
      LiMIT_dir += "/";
//   model_filename=LiMIT_dir+"my_cessna.osg";
      model_filename=LiMIT_dir+"yellow_cessna.osg";
   }

   OSGsubPAT_number=get_n_OSGsubPATs();

// As of 4/19/09, we assume that a user should be able to pick
// OBSFRUSTA attached to LiMIT MODELS for Afghanistan line-of-site
// analysis purposes:

//   cout << "instantiate_OBSFRUSTUMPickHandler_flag = "
//        << instantiate_OBSFRUSTUMPickHandler_flag << endl;

   MODEL* LiMIT_ptr=generate_new_Model(model_filename,OSGsubPAT_number);
   return LiMIT_ptr;
}

// --------------------------------------------------------------------------
// Member function generate_LiMIT_MODEL() dynamically instantiates a
// LiMIT model and sets various aircraft parameters such as speed,
// extremal raytracing ranges, OBSFRUSTUM roll & pitch.

MODEL* MODELSGROUP::generate_LiMIT_MODEL()
{
//   cout << "inside MODELSGROUP::generate_LiMIT_Model()" << endl;

   int LiMIT_OSGsubPAT_number;
   bool use_AWACS_model_flag=false;
   MODEL* LiMIT_MODEL_ptr=MODELSGROUP::generate_LiMIT_MODEL(
      LiMIT_OSGsubPAT_number,use_AWACS_model_flag);
//   cout << "LiMIT_OSGsubPAT_number = " << LiMIT_OSGsubPAT_number << endl;
//   cout << "LiMIT_MODEL_ptr->get_ID() = " 
//        << LiMIT_MODEL_ptr->get_ID() << endl;

   LiMIT_MODEL_ptr->set_speed(LiMIT_speed);
//   cout << "LiMIT_speed = " << LiMIT_speed << endl;

//   cout << "use_AWACS_model_flag = " << use_AWACS_model_flag << endl;
   if (use_AWACS_model_flag)
   {
      set_constant_scale(LiMIT_MODEL_ptr,100);	// AWACS model
   }
   else
   {
//      set_constant_scale(LiMIT_MODEL_ptr,5);	// Cessna model over HAFB 
//      set_constant_scale(LiMIT_MODEL_ptr,50);	// Cessna model for LOST ?
      set_constant_scale(LiMIT_MODEL_ptr,250);	// Cessna model for FLIRSIM
   }

   if (instantiate_OBSFRUSTAGROUP_flag)
   {

// LiMIT OBSFRUSTUM parameters:

      LiMIT_MODEL_ptr->set_dynamically_compute_OBSFRUSTUM_flag(true);
//   LiMIT_MODEL_ptr->set_min_raytrace_range(min_raytrace_range);
//   LiMIT_MODEL_ptr->set_max_raytrace_range(max_raytrace_range);
//   cout << "min_raytrace_range = " << min_raytrace_range
//        << " max_raytrace_range = " << max_raytrace_range << endl;
      LiMIT_MODEL_ptr->push_back_OBSFRUSTUM_roll(OBSFRUSTUM_roll);
      LiMIT_MODEL_ptr->push_back_OBSFRUSTUM_pitch(OBSFRUSTUM_pitch);

//   cout << "OBSFRUSTUM_az_extent = " << OBSFRUSTUM_az_extent*180/PI << endl;
//   cout << "OBSFRUSTUM_el_extent = " << OBSFRUSTUM_el_extent*180/PI << endl;
//   cout << "OBSFRUSTUM_roll = " << OBSFRUSTUM_roll*180/PI << endl;

      double base_altitude_above_grid=60;		// meters
      LiMIT_MODEL_ptr->push_back_OBSFRUSTUM_z_base_face(
         get_grid_world_origin().get(2)+base_altitude_above_grid);	  

      OBSFRUSTUM* OBSFRUSTUM_ptr=instantiate_LiMIT_FOV_OBSFRUSTUM(
         LiMIT_OSGsubPAT_number,LiMIT_MODEL_ptr,
         OBSFRUSTUM_az_extent,OBSFRUSTUM_el_extent);
      OBSFRUSTUM_ptr->set_TilesGroup_ptr(TilesGroup_ptr);

// Generally instantiate double-lobed pattern for LOST project:

      if (double_LiMIT_lobe_pattern_flag)
      {
         LiMIT_MODEL_ptr->push_back_OBSFRUSTUM_roll(-OBSFRUSTUM_roll);
         LiMIT_MODEL_ptr->push_back_OBSFRUSTUM_pitch(OBSFRUSTUM_pitch);

         LiMIT_MODEL_ptr->push_back_OBSFRUSTUM_z_base_face(
            get_grid_world_origin().get(2)+base_altitude_above_grid);	
         OBSFRUSTUM* second_OBSFRUSTUM_ptr=instantiate_LiMIT_FOV_OBSFRUSTUM(
            LiMIT_OSGsubPAT_number,LiMIT_MODEL_ptr,
            OBSFRUSTUM_az_extent,OBSFRUSTUM_el_extent);
         second_OBSFRUSTUM_ptr->set_TilesGroup_ptr(TilesGroup_ptr);
      } // double_LiMIT_lobe_pattern_flag conditional

   } // instantiate_OBSFRUSTAGROUP_flag==true conditional

   return LiMIT_MODEL_ptr;
}

// --------------------------------------------------------------------------
// Member function generate_LiMIT_MODEL_for_flight_PolyLine()
// dynamically instantiates a LiMIT model to fly along the last
// PolyLine member of *Path_PolyLinesGroup_ptr.  It generates and
// fills a track for the Model based upon its flight Polyline.  

MODEL* MODELSGROUP::generate_LiMIT_MODEL_for_flight_PolyLine(
   PolyLine* flight_PolyLine_ptr)
{
   cout << "inside MODELSGROUP::generate_LiMIT_Model_for_flight_PolyLine()"
        << endl;

   MODEL* LiMIT_MODEL_ptr=generate_LiMIT_MODEL();

//   cout << "flight_PolyLine_ptr->get_ID() = "
//        << flight_PolyLine_ptr->get_ID() << endl;

//    double polyline_length=
   flight_PolyLine_ptr->get_or_set_polyline_ptr()->compute_total_length();
//   cout << "Flight path length = " << polyline_length << " meters" << endl;
//   cout << "flight polyline = " 
//        << *(flight_PolyLine_ptr->get_polyline_ptr()) << endl;
   double total_length=flight_PolyLine_ptr->get_polyline_ptr()->
      get_total_length();
   double flight_duration=total_length/LiMIT_MODEL_ptr->get_speed(); // secs
//   cout << "Calculated flight duration = " << flight_duration/3600.0
//        << endl;

   if (Operations_ptr != NULL)
   {
      Operations_ptr->set_current_master_clock_time_duration(
         flight_duration/3600.0);
   }

// Generate track for LiMIT model.  For ActiveMQ messaging with Luca's
// MATLAB UAV task assignment code purposes, set track's label_ID
// equal to LiMIT MODEL's ID and NOT equal to track's ID:

   track* curr_track_ptr=tracks_group_ptr->generate_new_track();
   curr_track_ptr->set_description("Airborne Sensor");
   curr_track_ptr->set_label_ID(LiMIT_MODEL_ptr->get_ID());
   LiMIT_MODEL_ptr->set_track_ptr(curr_track_ptr);

   double scale_factor=15;
   add_flight_path_arrows(flight_PolyLine_ptr,scale_factor);

   track* track_ptr=update_UAV_track(flight_PolyLine_ptr,LiMIT_MODEL_ptr);
   bool compute_posns_with_distinct_dirs_flag=false;
   broadcast_add_track_to_GoogleEarth_channel(
      track_ptr,compute_posns_with_distinct_dirs_flag);

   return LiMIT_MODEL_ptr;
}

// ---------------------------------------------------------------------
void MODELSGROUP::generate_predator_racetrack_orbit(
   int n_total_frames,MODEL* predator_MODEL_ptr)
{
   string banner="Generating predator racetrack orbit";
   outputfunc::write_banner(banner);

   AnimationController_ptr->set_nframes(n_total_frames);

   double prefactor=1.0;
   double radius=prefactor * 6 * 1000;			// meters
   double height_above_center=prefactor * 5 * 1000;	// meters
   double phase_offset=0*PI/180;	// rads
   double orbit_period=0.5*n_total_frames;
   threevector racetrack_center(0,0,0);
   generate_racetrack_orbit(
      predator_MODEL_ptr,racetrack_center,radius,height_above_center,
      orbit_period,phase_offset);
   const double predator_scale_factor=30;
   set_constant_scale(predator_MODEL_ptr,predator_scale_factor);
}

// ---------------------------------------------------------------------
void MODELSGROUP::generate_predator_racetrack_orbit(
   int n_total_frames,double racetrack_center_longitude,
   double racetrack_center_latitude,MODEL* predator_MODEL_ptr)
{
   string banner="Generating predator racetrack orbit";
   outputfunc::write_banner(banner);

   AnimationController_ptr->set_nframes(n_total_frames);

   double prefactor=1.0;
   double radius=prefactor * 6 * 1000;			// meters
//   double radius=prefactor * 3 * 1000;		// meters  RCO slides
   double height_above_center=prefactor * 5 * 1000;	// meters
//   double height_above_center=prefactor * 1.2 * 1000;	// meters  RCO slides
   double phase_offset=0*PI/180;	// rads
   double orbit_period=0.5*n_total_frames;
   generate_racetrack_orbit(
      predator_MODEL_ptr,racetrack_center_longitude,racetrack_center_latitude,
      radius,height_above_center,orbit_period,phase_offset);
   const double predator_scale_factor=30;
   set_constant_scale(predator_MODEL_ptr,predator_scale_factor);
}

// ---------------------------------------------------------------------
// Member function generate_elliptical_LiMIT_racetrack_orbit is a
// specialized method which produces a continuous racetrack orbit for
// LiMIT that closely matches the discontinuous polygonal orbit
// implemented by Virtual Hammer in the Feb 2007 integrated ISDS demo.
// The parameters for the ellipse were calculated using main program
// isds/LiMIT_orbit and hardwired into this method.

void MODELSGROUP::generate_elliptical_LiMIT_racetrack_orbit(
   int n_total_frames,const geopoint& racetrack_center,MODEL* LiMIT_ptr)
{
   string banner="Generating LiMIT racetrack orbit";
   outputfunc::write_banner(banner);

   threevector center(
      racetrack_center.get_UTM_easting(),racetrack_center.get_UTM_northing(),
      racetrack_center.get_altitude());
   generate_elliptical_LiMIT_racetrack_orbit(n_total_frames,center,LiMIT_ptr);
}

void MODELSGROUP::generate_elliptical_LiMIT_racetrack_orbit(
   int n_total_frames,const threevector& racetrack_center,MODEL* LiMIT_ptr)
{
   string banner="Generating LiMIT racetrack orbit";
   outputfunc::write_banner(banner);

//   cout << "inside MODELSGROUP::generate_elliptical_LiMIT_racetrack()" 
//        << endl;
   AnimationController_ptr->set_nframes(n_total_frames);

   const double a=35851.7620423408;
   const double b=28737.783019816;
   const double theta = 113.919717495022*PI/180;
   ellipse racetrack_orbit(racetrack_center,a,b,theta);

   const double phase_offset=180*PI/180;
   vector<threevector> vertex=racetrack_orbit.generate_vertices(
      n_total_frames,phase_offset);

   const double altitude_above_center=11000;	// meters
   const double omega=2*PI/n_total_frames;
   for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); n++)
   {
      double t=n;
      double curr_phi=omega*t+phase_offset;
      threevector posn=vertex[n]+altitude_above_center*z_hat;

      double roll=0;	      	// radians
      double pitch=0;		// radians
      double yaw=atan2(b*cos(curr_phi),-a*sin(curr_phi))-PI/2+theta;
      threevector RPY(roll,pitch,yaw);

      LiMIT_ptr->set_attitude_posn(t,get_passnumber(),RPY,posn);
   } // loop over index n labeling angular posn along racetrack orbit

   const double LiMIT_scale_factor=30;
   set_constant_scale(LiMIT_ptr,LiMIT_scale_factor);
}

// ==========================================================================
// OBSFRUSTA generation member functions
// ==========================================================================

// Member function generate_predator_[LiMIT]_OBSFRUSTA are specialized
// methods developed specifically for the ISDS integrated demo in
// February 2007.  They attach OBSFRUSTA to models squinted off from
// aircrafts' velocity vectors by some fixed direction.

OBSFRUSTUM* MODELSGROUP::instantiate_predator_OBSFRUSTUM(
   int predator_OSGsubPAT_number,MODEL* predator_MODEL_ptr,
   double predator_az_extent,double predator_el_extent,
   string movie_filename,double zface_offset)
{
//   cout << "inside MODELSGROUP::instantiate_predator_OBSFRUSTUM() #1" << endl;
   return generate_predator_OBSFRUSTUM(
      get_first_framenumber(),get_first_framenumber(),
      predator_OSGsubPAT_number,predator_MODEL_ptr,
      predator_az_extent,predator_el_extent,movie_filename,zface_offset);
}

OBSFRUSTUM* MODELSGROUP::instantiate_predator_OBSFRUSTUM(
   int predator_OSGsubPAT_number,MODEL* predator_MODEL_ptr,
   double predator_az_extent,double predator_el_extent,
   Movie* Movie_ptr,double zface_offset)
{
//   cout << "inside MODELSGROUP::instantiate_predator_OBSFRUSTUM() #2" << endl;
   return generate_predator_OBSFRUSTUM(
      get_first_framenumber(),get_first_framenumber(),
      predator_OSGsubPAT_number,predator_MODEL_ptr,
      predator_az_extent,predator_el_extent,Movie_ptr,zface_offset);
}

OBSFRUSTUM* MODELSGROUP::generate_predator_OBSFRUSTUM(
   int predator_OSGsubPAT_number,MODEL* predator_MODEL_ptr,
   double az_extent,double el_extent,string movie_filename,
   double zface_offset)
{
   return generate_predator_OBSFRUSTUM(
      get_first_framenumber(),get_last_framenumber(),
      predator_OSGsubPAT_number,predator_MODEL_ptr,az_extent,el_extent,
      movie_filename,zface_offset);
}

OBSFRUSTUM* MODELSGROUP::generate_predator_OBSFRUSTUM(
   int first_framenumber,int last_framenumber,
   int predator_OSGsubPAT_number,MODEL* predator_MODEL_ptr,
   double az_extent,double el_extent,string movie_filename,
   double zface_offset)
{
//   cout << "inside MODELSGROUP::generate_predator_OBSFRUSTUM()" << endl;
   const double alpha=1.0;

   Movie* Movie_ptr=predator_MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->
      get_MoviesGroup_ptr()->generate_new_Movie(movie_filename,alpha);
   return generate_predator_OBSFRUSTUM(
      first_framenumber,last_framenumber,
      predator_OSGsubPAT_number,predator_MODEL_ptr,az_extent,el_extent,
      Movie_ptr,zface_offset);
}

OBSFRUSTUM* MODELSGROUP::generate_predator_OBSFRUSTUM(
   int first_framenumber,int last_framenumber,
   int predator_OSGsubPAT_number,MODEL* predator_MODEL_ptr,
   double az_extent,double el_extent,Movie* Movie_ptr,double zface_offset)
{ 
//   cout << "inside MODELSGROUP::generate_predator_OBSFRUSTUM(Movie_ptr)" 
//        << endl;
//   cout << "first_framenumber = " << first_framenumber
//        << " last_framenumber = " << last_framenumber << endl;

   string banner="Generating predator OBSFRUSTUM";
   outputfunc::write_banner(banner);

//   double az_extent=25*PI/180;  // rads		 RCO slides
//   double el_extent=25*PI/180;  // rads		 RCO slides
   OBSFRUSTUM* OBSFRUSTUM_ptr=instantiate_OBSFRUSTUM(
      predator_MODEL_ptr,az_extent,el_extent,Movie_ptr,
      predator_OSGsubPAT_number);

/*
   double delta_phi=90*PI/180;
   double delta_theta = -45*PI/180;		// genuine value
//   double delta_theta = -60*PI/180;		// RCO chart generation
   bool sinusoidal_variation_flag=true;		// genuine value
//   bool sinusoidal_variation_flag=false;	// RCO chart generation
   
   predator_MODEL_ptr->orient_and_position_OBSFRUSTUM(
      first_framenumber,last_framenumber,get_passnumber(),
      delta_phi,delta_theta,zface_offset,sinusoidal_variation_flag);
*/

   OBSFRUSTUM_ptr->set_UAV_OBSFRUSTUM_colorings(predator_MODEL_ptr->get_ID());

   return OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
OBSFRUSTUM* MODELSGROUP::instantiate_LiMIT_FOV_OBSFRUSTUM(
   int LiMIT_OSGsubPAT_number,MODEL* LiMIT_ptr,
   double FOV_az_extent,double FOV_el_extent)
{
//   cout << "inside MODELSGROUP::instantiate_LiMIT_FOV_OBSFRUSTUM()" << endl;
   return generate_LiMIT_FOV_OBSFRUSTUM(
      get_first_framenumber(),get_first_framenumber(),LiMIT_OSGsubPAT_number,
      LiMIT_ptr,FOV_az_extent,FOV_el_extent);
}

OBSFRUSTUM* MODELSGROUP::generate_LiMIT_FOV_OBSFRUSTUM(
   int LiMIT_OSGsubPAT_number,MODEL* LiMIT_ptr,
   double FOV_az_extent,double FOV_el_extent)
{
   return generate_LiMIT_FOV_OBSFRUSTUM(
      get_first_framenumber(),get_last_framenumber(),LiMIT_OSGsubPAT_number,
      LiMIT_ptr,FOV_az_extent,FOV_el_extent);
}

OBSFRUSTUM* MODELSGROUP::generate_LiMIT_FOV_OBSFRUSTUM(
   int first_framenumber,int last_framenumber,
   int LiMIT_OSGsubPAT_number,MODEL* LiMIT_ptr,
   double FOV_az_extent,double FOV_el_extent)
{
   string banner="Generating LiMIT FOV OBSFRUSTUM";
   outputfunc::write_banner(banner);

//   cout << "inside MODELSGROUP::generate_LiMIT_FOV_OBSFRUSTUM()" << endl;
//   cout << "first_framenumber = " << first_framenumber
//        << " last_framenumber = " << last_framenumber << endl;

// On 2/16/07, Peter Jones told us that the GMTI viewing bounds for
// the electronically steerable array onboard the LiMIT platform
// within the ISDS integrated demo (as of Feb 2007) are +/- 30 degs
// from broadside in azimuth and 16-45 degrees in declination.  

//   FOV_az_extent=60*PI/180;  // rads
//   FOV_el_extent=29*PI/180;  // rads
//   cout << "FOV_az_extent = " << FOV_az_extent*180/PI << endl;
//   cout << "FOV_el_extent = " << FOV_el_extent*180/PI << endl;
   
   OBSFRUSTUM* FOV_OBSFRUSTUM_ptr=
      instantiate_OBSFRUSTUM(LiMIT_ptr,FOV_az_extent,FOV_el_extent,
                             "",LiMIT_OSGsubPAT_number);

   double volume_alpha=0.2;
   set_LiMIT_FOV_OBSFRUSTUM_colors(FOV_OBSFRUSTUM_ptr,volume_alpha);
   return FOV_OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
void MODELSGROUP::set_LiMIT_FOV_OBSFRUSTUM_colors(
   OBSFRUSTUM* FOV_OBSFRUSTUM_ptr,double volume_alpha)
{
//   cout << "inside MODELSGROUP::set_LiMIT_FOV_OBSFRUSTUM_colors()" << endl;

   colorfunc::Color SideEdgeColor=colorfunc::white;
   colorfunc::Color ZplaneEdgeColor=colorfunc::white;
   colorfunc::Color VolumeColor=colorfunc::white;

   FOV_OBSFRUSTUM_ptr->set_color(
      SideEdgeColor,ZplaneEdgeColor,VolumeColor,volume_alpha);

   LiMIT_FOV_OBSFRUSTUM_flag=true;
}

// ---------------------------------------------------------------------
OBSFRUSTUM* MODELSGROUP::generate_LiMIT_instantaneous_dwell_OBSFRUSTUM(
   int LiMIT_OSGsubPAT_number,MODEL* LiMIT_ptr,
   OBSFRUSTUM* FOV_OBSFRUSTUM_ptr,EarthRegion* GMTI_region_ptr)
{
   string banner="Generating LiMIT instantaneous dwell OBSFRUSTUM";
   outputfunc::write_banner(banner);

//   cout << "inside MODELSGROUP::generate_LiMIT_instantaneous_dwell_ObsFrustrum()"
//        << endl;

   if (GMTI_region_ptr->get_GMTI_targets().size() > 0)
   {
      OBSFRUSTUM* instantaneous_dwell_OBSFRUSTUM_ptr=
         instantiate_OBSFRUSTUM(LiMIT_ptr,0,0,"",LiMIT_OSGsubPAT_number);

      double volume_alpha=0.2;
      instantaneous_dwell_OBSFRUSTUM_ptr->set_color(
         colorfunc::get_OSG_color(colorfunc::red),volume_alpha);
      LiMIT_ptr->get_OBSFRUSTAGROUP_ptr()->reset_colors();

      double z_offset=50;   // meters

      LiMIT_ptr->orient_and_position_instantaneous_dwell_OBSFRUSTUM(
         get_first_framenumber(),get_last_framenumber(),
         get_passnumber(),z_offset,
         instantaneous_dwell_OBSFRUSTUM_ptr,FOV_OBSFRUSTUM_ptr,
         GMTI_region_ptr->get_GMTI_targets());

      update_display();
      return instantaneous_dwell_OBSFRUSTUM_ptr;
   }
   else
   {
      return NULL;
   }
}

// ==========================================================================
// HAFB video3D methods
// ==========================================================================

// Member function generate_HAFB_video_pass_model is a specialized,
// high-level method which returns a Cessna model with an OBSFRUSTUM
// that displays the Group 99 HAFB video at its base.  Both the Model
// and the OBSFRUSTUM are placed within a new OSGsubPAT of this
// MODELSGROUP object.  The flight path is rotated away from its true
// direction over HAFB by input angle z_rot_angle.  (The true flight
// direction made an angle of approximately -60 degs relative to
// east).  The desired, absolute XY location of the aircraft for the
// first frame within the HAFB video pass is contained within input
// threevector first_frame_aircraft_xy_posn.

MODEL* MODELSGROUP::generate_HAFB_video_pass_model(
   double z_rot_angle,const threevector& first_frame_aircraft_posn,
   AnimationController* HAFB_AnimationController_ptr)
{
//   cout << "inside MODELSGROUP::generate_HAFB_video_pass_model()" << endl;
   
   string subdir="/home/cho/programs/c++/svn/projects/src/mains/fusion/";
   string model_filename=subdir+"my_cessna.osg";
//   string model_filename=subdir+"yellow_cessna.osg";

   int OSGsubPAT_number=get_n_OSGsubPATs();
   MODEL* curr_MODEL_ptr=generate_new_Model(model_filename,OSGsubPAT_number);

// Initialize aircraft position and attitude as functions of HAFB
// video frame number:

   vector<threevector> plane_posn,plane_attitude;
   read_HAFB_plane_info(plane_posn,plane_attitude);

   const double cessna_scale_factor=5;
//   const double cessna_scale_factor=10;

   curr_MODEL_ptr->set_scale_attitude_posn(
      get_passnumber(),cessna_scale_factor,plane_attitude,plane_posn);

// Generate an OBSFRUSTUM for the aircraft model.  Then insert HAFB
// video at its base:

   double z_offset=get_grid_world_origin().get(2);
   instantiate_HAFB_video_OBSFRUSTUM(curr_MODEL_ptr,plane_posn,z_offset,
                                     OSGsubPAT_number);

// Translate model from its relative XY posn within the first frame of
// original HAFB video pass to (0,0):

   osg::Vec3d init_trans(-47172.19,1751.168,0); 
   get_OSGsubPAT_ptr(OSGsubPAT_number)->setPivotPoint(init_trans);

// Rotate aircraft model (along with its OBSFRUSTUM and associated
// movie) about (0,0) by angle z_rot_angle:

   const osg::Vec3f Z_hat(0,0,1);
   osg::Quat quat;
   quat.makeRotate(z_rot_angle,Z_hat);
   get_OSGsubPAT_ptr(OSGsubPAT_number)->setAttitude(quat);

// Translate model from (0,0) to first_frame_aircraft_posn:

   osg::Vec3d final_trans(
      first_frame_aircraft_posn.get(0),
      first_frame_aircraft_posn.get(1),
      first_frame_aircraft_posn.get(2));
   get_OSGsubPAT_ptr(OSGsubPAT_number)->setPosition(final_trans);

   return curr_MODEL_ptr;
}

/*
// ---------------------------------------------------------------------
// This next version of member function generate_cessna_model() is a
// hacked version of generate_HAFB_video_pass_model().  We wrote this
// little method in order to produce a blue/yellow cessna against a
// black background in program BASEMENT.  We then sent PNG screen
// captures of the cessna to Jennifer Drexler in Oct 2009 for ISR
// poster generation purposes.

MODEL* MODELSGROUP::generate_cessna_model(
   double z_rot_angle,const threevector& first_frame_aircraft_posn,
   AnimationController* HAFB_AnimationController_ptr)
{
   string cessna_dir(getenv("OSG_FILE_PATH"));
   cessna_dir += "/";
   string model_filename=cessna_dir+"my_cessna.osg";


   int OSGsubPAT_number=get_n_OSGsubPATs();
   MODEL* curr_MODEL_ptr=generate_new_Model(model_filename,OSGsubPAT_number);

// Translate model from its relative XY posn within the first frame of
// original HAFB video pass to (0,0):

   osg::Vec3d init_trans(0,0,0);
   get_OSGsubPAT_ptr(OSGsubPAT_number)->setPivotPoint(init_trans);

// Rotate aircraft model (along with its OBSFRUSTUM and associated
// movie) about (0,0) by angle z_rot_angle:

   const osg::Vec3f Z_hat(0,0,1);
   osg::Quat quat;
   quat.makeRotate(z_rot_angle,Z_hat);
   get_OSGsubPAT_ptr(OSGsubPAT_number)->setAttitude(quat);

// Translate model from (0,0) to first_frame_aircraft_posn:

   osg::Vec3d final_trans(
      first_frame_aircraft_posn.get(0),
      first_frame_aircraft_posn.get(1),
      first_frame_aircraft_posn.get(2));
   get_OSGsubPAT_ptr(OSGsubPAT_number)->setPosition(final_trans);

   return curr_MODEL_ptr;
}
*/

// ---------------------------------------------------------------------
// Member function instantiate_HAFB_video_OBSFRUSTUM

OBSFRUSTUM* MODELSGROUP::instantiate_HAFB_video_OBSFRUSTUM(
   MODEL* MODEL_ptr,const vector<threevector>& aircraft_posn,double z_offset,
   int OSGsubPAT_number)
{
//   cout << "inside MODELSGROUP::instantiate_HAFB_video_OBSFRUSTUM()" << endl;
   
   OBSFRUSTUM* OBSFRUSTUM_ptr=MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->
      generate_HAFB_movie_OBSFRUSTUM(aircraft_posn,z_offset);
   get_OSGsubPAT_ptr(OSGsubPAT_number)->addChild(
      MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->get_OSGgroup_ptr());

   get_OSGsubPAT_ptr(OSGsubPAT_number)->addChild(
      MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->get_MoviesGroup_ptr()->
      get_OSGgroup_ptr());

   return OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
void MODELSGROUP::read_HAFB_plane_info(
   vector<threevector>& plane_posn,vector<threevector>& plane_attitude)
{

// Read in HAFB aircraft filtered position and attitude as functions
// of time:

   string subdir="/home/cho/programs/c++/svn/projects/src/mains/fusion/";
   string TPA_filename=subdir+"TPA_filtered.txt";
   filefunc::ReadInfile(TPA_filename);

// Recall first image in HAFB_overlap_corrected_grey.vid actually
// corresponds to tiepoint_imagenumber 300 in uncut HAFB.vid file:

   const int imagenumber_offset=300;	

   const int n_fields=8;
   double X[n_fields];
   for (unsigned int i=imagenumber_offset; i<filefunc::text_line.size(); i++)
   {
      stringfunc::string_to_n_numbers(n_fields,filefunc::text_line[i],X);
      plane_posn.push_back(threevector(X[2],X[3],X[4]));
      plane_attitude.push_back(
         threevector(X[5]*PI/180,X[6]*PI/180,X[7]*PI/180));
   }
}

// ---------------------------------------------------------------------
// Member function generate_MODEL_FOV_OBSFRUSTUM 

OBSFRUSTUM* MODELSGROUP::instantiate_MODEL_FOV_OBSFRUSTUM(
   int MODEL_OSGsubPAT_number,MODEL* MODEL_ptr,
   double FOV_az_extent,double FOV_el_extent)
{
   return generate_MODEL_FOV_OBSFRUSTUM(
      MODEL_OSGsubPAT_number,MODEL_ptr,
      get_AnimationController_ptr()->get_first_framenumber(),
      get_AnimationController_ptr()->get_first_framenumber(),
      FOV_az_extent,FOV_el_extent);
}

// ---------------------------------------------------------------------
OBSFRUSTUM* MODELSGROUP::generate_MODEL_FOV_OBSFRUSTUM(
   int MODEL_OSGsubPAT_number,MODEL* MODEL_ptr,
   double FOV_az_extent,double FOV_el_extent)
{
   return generate_MODEL_FOV_OBSFRUSTUM(
      MODEL_OSGsubPAT_number,MODEL_ptr,
      get_AnimationController_ptr()->get_first_framenumber(),
      get_AnimationController_ptr()->get_last_framenumber(),
      FOV_az_extent,FOV_el_extent);
}

// ---------------------------------------------------------------------
OBSFRUSTUM* MODELSGROUP::generate_MODEL_FOV_OBSFRUSTUM(
   int MODEL_OSGsubPAT_number,MODEL* MODEL_ptr,
   int first_framenumber,int last_framenumber,
   double FOV_az_extent,double FOV_el_extent)
{
//   cout << "inside MODELSGROUP::generate_MODEL_FOV_OBSFRUSTUM()" << endl;

   string banner="Generating FOV OBSFRUSTUM";
   outputfunc::write_banner(banner);

   OBSFRUSTUM* FOV_OBSFRUSTUM_ptr=
      instantiate_OBSFRUSTUM(MODEL_ptr,FOV_az_extent,FOV_el_extent,
                             "",MODEL_OSGsubPAT_number);

   insert_OSGgroup_into_OSGsubPAT(
      MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->get_OSGgroup_ptr(),
      MODEL_OSGsubPAT_number);

   double volume_alpha=0.05;  
   colorfunc::Color SideEdgeColor=colorfunc::yellow;
//   colorfunc::Color ZplaneEdgeColor=colorfunc::orange;
   colorfunc::Color ZplaneEdgeColor=colorfunc::brick;
   colorfunc::Color VolumeColor=colorfunc::yellow;   

   if (FOV_OBSFRUSTUM_ptr->get_ID() > 0)
   {
      volume_alpha=0.1;
      SideEdgeColor=colorfunc::cyan;
      ZplaneEdgeColor=colorfunc::blue;
      VolumeColor=colorfunc::cyan;   
   }

   FOV_OBSFRUSTUM_ptr->set_color(
      SideEdgeColor,ZplaneEdgeColor,VolumeColor,volume_alpha);

   return FOV_OBSFRUSTUM_ptr;
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

bool MODELSGROUP::parse_next_message_in_queue(message& curr_message)
{
//   cout << "inside MODELSGROUP::parse_next_message_in_queue()" << endl;
//   cout << "curr_message.get_text_message() = "
//        << curr_message.get_text_message() << endl;

   bool message_handled_flag=false;
   
   if (curr_message.get_text_message()=="ASSIGN_TASK")
   {
//      double t_after_path_planning_algs=timefunc::elapsed_timeofday_time();
//      cout << "After calling path planning algorithms, t = "
//           << t_after_path_planning_algs << endl;

//      cout << "Received ASSIGN_TASK message from ActiveMQ" << endl;
      curr_message.extract_and_store_property_keys_and_values();

// On 8/7/08, Luca told us that MatlabMQ does not transmit multiple
// messages with the same key.  So we now listen for key-value pairs
// coming from Luca's MATLAB codes which are of the form "key =
// UAVID_2" and "value = 6,2,4,8":

      vector<int> UAV_IDs;
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         MODEL* curr_Predator_MODEL_ptr=get_MODEL_ptr(n);
         UAV_IDs.push_back(curr_Predator_MODEL_ptr->
                           get_track_ptr()->get_label_ID());
//         cout << "n = " << n << " UAV_IDs.back() = "
//              << UAV_IDs.back() << endl;
      }

      vector<track*> reduced_UAV_track_ptrs;
      for (unsigned int n=0; n<UAV_IDs.size(); n++)
      {
         int UAV_ID=UAV_IDs[n];
//         cout << "UAV_ID = " << UAV_ID << endl;
         string key="UAVID_"+stringfunc::number_to_string(UAV_ID);
//         cout << "key = " << key << endl;
         string value=curr_message.get_property_value(key);
//         cout << "value = " << value << endl;

         if (value.size() > 0)
         {
            vector<string> substrings=
               stringfunc::decompose_string_into_substrings(value,",");

            vector<int> ground_target_IDs;
            for (unsigned int s=0; s<substrings.size(); s++)
            {
               ground_target_IDs.push_back(stringfunc::string_to_integer(
                  substrings[s]));
            }
            track* reduced_track_ptr=generate_task_assignments(
               UAV_ID,ground_target_IDs);
            if (reduced_track_ptr != NULL) reduced_UAV_track_ptrs.push_back(
               reduced_track_ptr);
         } // value.size() > 0 conditional
      } // loop over index n labeling UAV_IDs

      broadcast_add_tracks_to_GoogleEarth_channel(reduced_UAV_track_ptrs);

// Recall that reduced track pointers were dynamically generated in
// generate_task_assignments() member function.  After their contents
// have been broadcast, we need to destroy them:

      for (unsigned int n=0; n<reduced_UAV_track_ptrs.size(); n++)
      {
         delete reduced_UAV_track_ptrs[n];
      }

      message_handled_flag=true;
   }
   else if (curr_message.get_text_message()=="INSTANTIATE_UAV")
   {
      curr_message.extract_and_store_property_keys_and_values();

      for (unsigned int n=0; n<curr_message.get_n_properties(); n++)
      {
         message::Property curr_property=curr_message.get_property(n);
         string key=curr_property.first;

         string value=curr_property.second;
         vector<string> key_substrings=
            stringfunc::decompose_string_into_substrings(key);
         vector<string> value_substrings=
            stringfunc::decompose_string_into_substrings(value);
         
         threevector UAV_init_posn;
         if (key_substrings[0] == "LONGITUDE" &&
             key_substrings[1] == "LATITUDE")
         {
            geopoint UAV_pt(
               value_substrings,
               EarthRegionsGroup_ptr->get_specified_UTM_zonenumber());
            UAV_init_posn=UAV_pt.get_UTM_posn();

         }
         else if (key_substrings[0] == "X" &&
                  key_substrings[1] == "Y")
         {
            UAV_init_posn=threevector(
               stringfunc::string_to_number(value_substrings[0]),
               stringfunc::string_to_number(value_substrings[1]),
               stringfunc::string_to_number(value_substrings[2]));
         }
         else
         {
            continue;
         }

// Minimize fictitious UAV path so that it's not visible within RTPS
// viewer output:

         threevector delta_posn(1,1,0);
         if (long_initial_UAV_track_flag)
         {
            delta_posn=threevector(100,100,0);
         }
         
         threevector UAV_next_posn=UAV_init_posn+delta_posn;
         vector<threevector> V;
         V.push_back(UAV_init_posn);
         V.push_back(UAV_next_posn);

         PolyLine* new_UAV_PolyLine_ptr=
            Path_PolyLinesGroup_ptr->generate_new_PolyLine(V);
         new_UAV_PolyLine_ptr->set_entry_finished_flag(true);

         new_UAV_PolyLine_ptr->set_permanent_color(
            UAV_flight_path_color(new_UAV_PolyLine_ptr->get_ID()));

// As of 2/18/09, INSTANTIATE_UAV messages should only be transmitted
// by ISAT demo programs where the initial UAV's flight path is
// precomputed.  So we do not want to redbroadcast our fictitious
// flight path consisting of only two UAV waypoints:

         bool broadcast_UAV_track_flag=false;
         if (aircraft_model_type==predator)
         {
            generate_Predator_Model_for_flight_PolyLine(
               new_UAV_PolyLine_ptr,broadcast_UAV_track_flag);
         }
         else if (aircraft_model_type==cessna)
         {
            generate_aircraft_MODEL_for_flight_PolyLine(
               new_UAV_PolyLine_ptr,broadcast_UAV_track_flag);
         }
      } // loop over index n labeling messages
      
      message_handled_flag=true;
   }
   else if (curr_message.get_text_message()=="SET_INIT_WAYPOINTS")
   {
      curr_message.extract_and_store_property_keys_and_values();

// FAKE FAKE: As of 2/13/09, we force added ROIs to be inserted into
// EarthRegion #0:

      EarthRegion* EarthRegion_ptr=get_EarthRegionsGroup_ptr()->
         get_ID_labeled_EarthRegion_ptr(0);

      int UAV_ID=-1;
      vector<threevector> waypoints;
      for (unsigned int n=0; n<curr_message.get_n_properties(); n++)
      {
         message::Property curr_property=curr_message.get_property(n);
         string key=curr_property.first;
         string value=curr_property.second;
//         cout << "n = " << n << " key = " << key << endl;
//         cout << "value = " << value << endl;
         vector<string> key_substrings=
            stringfunc::decompose_string_into_substrings(key);
         vector<string> value_substrings=
            stringfunc::decompose_string_into_substrings(value);

         if (key_substrings[0]=="UAV_ID")
         {
            UAV_ID=stringfunc::string_to_number(value_substrings[0]);
            continue;
         }
         else if (key_substrings[0] != "LONGITUDE" || 
             key_substrings[1] != "LATITUDE")
         {
            continue;
         }

         double waypoint_longitude=stringfunc::string_to_number(
            value_substrings[0]);
         double waypoint_latitude=stringfunc::string_to_number(
            value_substrings[1]);
         double waypoint_altitude=stringfunc::string_to_number(
            value_substrings[2]);

         geopoint waypoint_pt(
            value_substrings,
            EarthRegionsGroup_ptr->get_specified_UTM_zonenumber());
         threevector curr_waypoint=waypoint_pt.get_UTM_posn();

         curr_waypoint.put(2,EarthRegion_ptr->get_LatLongGrid_ptr()->
                           get_world_origin().get(2));
         waypoints.push_back(curr_waypoint);

         const double waypoint_radius=50;	// meters         
         vector<threevector> vertices=
            EarthRegion_ptr->generate_ROI_circle_vertices(
               waypoint_longitude,waypoint_latitude,waypoint_altitude,
               waypoint_radius,EarthRegionsGroup_ptr->
               get_specified_UTM_zonenumber());
         EarthRegion_ptr->generate_nominated_ROI(
            vertices,EarthRegionsGroup_ptr->get_ROI_color());

      } // loop over index n labeling properties

//      cout << "UAV_ID = " << UAV_ID << endl;
      mover* mover_ptr=movers_group_ptr->get_mover_ptr(
         mover::UAV,UAV_ID);
//      cout << "mover_ptr = " << mover_ptr << endl;
      track* orig_track_ptr=mover_ptr->get_orig_track_ptr();
      if (orig_track_ptr==NULL)
      {
         orig_track_ptr=new track(0);
         mover_ptr->set_orig_track_ptr(orig_track_ptr);
      }
      for (unsigned int n=0; n<waypoints.size(); n++)
      {
         orig_track_ptr->set_XYZ_coords(n,waypoints[n]);
      }
//      cout << "*orig_track_ptr = " << *orig_track_ptr << endl;

      message_handled_flag=true;
   } 
   else if (curr_message.get_text_message()=="UPDATE_UAV_STATEVECTOR")
   {
      int UAV_ID=-1;
      threevector curr_UAV_posn;
      threevector curr_UAV_velocity(y_hat);
      for (unsigned int n=0; n<curr_message.get_n_properties(); n++)
      {
         message::Property curr_property=curr_message.get_property(n);
         string key=curr_property.first;

         string value=curr_property.second;
         vector<string> key_substrings=
            stringfunc::decompose_string_into_substrings(key);
         vector<string> value_substrings=
            stringfunc::decompose_string_into_substrings(value);
         
         if (key_substrings[0]=="UAV_ID")
         {
            UAV_ID=stringfunc::string_to_number(value_substrings[0]);
            continue;
         }
         else if (key_substrings[0] == "LONGITUDE" &&
             key_substrings[1] == "LATITUDE")
         {
            geopoint UAV_pt(
               value_substrings,
               EarthRegionsGroup_ptr->get_specified_UTM_zonenumber());
            curr_UAV_posn=UAV_pt.get_UTM_posn();
         }
         else if (key_substrings[0] == "X" &&
                  key_substrings[1] == "Y" )
         {
            double UAV_x=stringfunc::string_to_number(value_substrings[0]);
            double UAV_y=stringfunc::string_to_number(value_substrings[1]);
            double UAV_z=stringfunc::string_to_number(value_substrings[2]);
            curr_UAV_posn=threevector(UAV_x,UAV_y,UAV_z);
         }
         else if (key_substrings[0] == "Vx" &&
                  key_substrings[1] == "Vy" )
         {
            double UAV_Vx=stringfunc::string_to_number(value_substrings[0]);
            double UAV_Vy=stringfunc::string_to_number(value_substrings[1]);
            double UAV_Vz=stringfunc::string_to_number(value_substrings[2]);
            curr_UAV_velocity=threevector(UAV_Vx,UAV_Vy,UAV_Vz);
         }
      } // loop over index n labeling message properties

//      cout << "UAV_ID = " << UAV_ID << endl;
//      cout << "curr_UAV_posn = " << curr_UAV_posn << endl;

      MODEL* predator_MODEL_ptr=get_ID_labeled_MODEL_ptr(UAV_ID);
//      cout << "predator_MODEL_ptr = " << predator_MODEL_ptr << endl;
      if (predator_MODEL_ptr==NULL) return message_handled_flag;

      threevector prev_UAV_posn;
      predator_MODEL_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),prev_UAV_posn);

      if (!prev_UAV_posn.nearly_equal(curr_UAV_posn))
      {

// Set Predator MODEL's current attitude and position:

         const threevector north_hat(y_hat);
         pair<double,threevector> p=
            mathfunc::angle_and_axis_between_unitvectors(
               north_hat,curr_UAV_velocity.unitvector());
         double yaw=p.first;
         double dotproduct=z_hat.dot(p.second);
         yaw *= sgn(dotproduct);

         double roll=0;
         double pitch=0;
         threevector RPY(roll,pitch,yaw);

// To avoid MODEL flickering in the real-time persistent surveillance
// demo, repeat MODEL's current position and attitude at several
// timesteps into the future:

         for (unsigned int r=0; r<n_future_repeats; r++)
         {
            predator_MODEL_ptr->set_attitude_posn(
               get_curr_t()+r,get_passnumber(),RPY,curr_UAV_posn);
            predator_MODEL_ptr->set_UVW_velocity(
               get_curr_t()+r,get_passnumber(),curr_UAV_velocity);
         }

// If UAV's current position differs from its previous position, check
// whether aircraft has passed any waypoint targets along the ground:

         if (compute_passed_ground_targets_flag)
         {
         
// FAKE FAKE: As of 2/13/09, we force added ROIs to be inserted into
// EarthRegion #0:

            EarthRegion* EarthRegion_ptr=get_EarthRegionsGroup_ptr()->
               get_ID_labeled_EarthRegion_ptr(0);
            movers_group* ground_movers_group_ptr=EarthRegion_ptr->
               get_movers_group_ptr();
            ground_movers_group_ptr->compute_passed_ground_targets(
               get_curr_t(),UAV_ID,curr_UAV_posn,movers_group_ptr,mover::ROI);

         } // compute_passed_ground_targets_flag conditional
      } // !prev_UAV_posn conditional
      
      message_handled_flag=true;
   }
   else if (curr_message.get_text_message()=="UPDATE_UAV_PATHS")
   {
//      cout << "Path_PolyLinesGroup_ptr = " << Path_PolyLinesGroup_ptr
//           << endl;
      if (Path_PolyLinesGroup_ptr != NULL &&
          Path_PolyLinesGroup_ptr->get_n_Graphicals() > 0)
      {
         broadcast_sensor_and_target_statevectors();
      }
      message_handled_flag=true;
   } 
   else if (curr_message.get_text_message()=="PURGE_UAVS")
   {
      purge_UAV_MODELS_and_tracks();
      message_handled_flag=true;
   } // curr_message.get_text_message() conditional
   else if (curr_message.get_text_message()=="UPDATE_NFOV_LOOKPOINT")
   {
      MODEL* aircraft_MODEL_ptr=NULL;
      geopoint NFOV_lookpoint;
      for (unsigned int n=0; n<curr_message.get_n_properties(); n++)
      {
         message::Property curr_property=curr_message.get_property(n);
         string key=curr_property.first;

         string value=curr_property.second;
         vector<string> key_substrings=
            stringfunc::decompose_string_into_substrings(key);
         vector<string> value_substrings=
            stringfunc::decompose_string_into_substrings(value);
         
         if (key_substrings[0]=="UAV_ID")
         {
            int UAV_ID=stringfunc::string_to_number(value_substrings[0]);
            aircraft_MODEL_ptr=get_ID_labeled_MODEL_ptr(UAV_ID);
//            cout << "aircraft_MODEL_ptr = " << aircraft_MODEL_ptr << endl;
            if (aircraft_MODEL_ptr==NULL) return message_handled_flag;

            continue;
         }
         else if (key_substrings[0] == "X" && key_substrings[1] == "Y" )
         {
            double NFOV_lookpoint_x=
               stringfunc::string_to_number(value_substrings[0]);
            double NFOV_lookpoint_y=
               stringfunc::string_to_number(value_substrings[1]);
            bool northern_hemisphere_flag=true;
            int UTM_zone=16;	// Milwaukee
            NFOV_lookpoint=geopoint(northern_hemisphere_flag,UTM_zone,
                                    NFOV_lookpoint_x,NFOV_lookpoint_y);
//            cout << "NFOV_lookpoint = " << NFOV_lookpoint << endl;
         }
      } // loop over index n labeling message properties
      if (aircraft_MODEL_ptr != NULL)
      {
         aircraft_MODEL_ptr->update_NFOV_OBSFRUSTUM_roll_and_pitch(
            get_curr_t(),get_passnumber(),NFOV_lookpoint);
      }
      
      message_handled_flag=true;
   } // curr_message.get_text_message() conditional

//   cout << "message_handled_flag = " << message_handled_flag << endl;
   return message_handled_flag;
}

// ==========================================================================
// Multi-UAV C2 member functions
// ==========================================================================

// Member function broadcast_sensor_and_target_statevectors()

bool MODELSGROUP::broadcast_sensor_and_target_statevectors()
{
//   cout << "inside MODELSGROUP::broadcast_sensor_and_target_statevectors()" 
//        << endl;

   if (EarthRegionsGroup_ptr==NULL) return false;
   EarthRegion* EarthRegion_ptr=EarthRegionsGroup_ptr->
      get_ID_labeled_EarthRegion_ptr(0);

   return broadcast_sensor_and_target_statevectors(EarthRegion_ptr);
}

// ---------------------------------------------------------------------
// Member function broadcast_sensor_and_target_statevectors first
// sends out a START_PACKET message.  It then loops over each MODEL
// within the current MODELSGROUP object and broadcasts its track's
// current statevector.  Finally, this method sends out a STOP_PACKET
// message.

bool MODELSGROUP::broadcast_sensor_and_target_statevectors(
   EarthRegion* curr_EarthRegion_ptr)
{
//   cout << "inside MODELSGROUP::broadcast_sensor_and_target_statevectors()" 
//        << endl;
//   cout << "this = " << this << endl;

   if (EarthRegionsGroup_ptr == NULL) 
   {
      cout << "Error in MODELSGROUP::broadcast_sensor_and_target_statevectors!"
           << endl;
      cout << "EarthRegionsGroup_ptr = NULL!" << endl;
      outputfunc::enter_continue_char();
      return false;
   }

// As of 9/15/08, we check whether any dynamic and spatially fixed
// ground tracks exist.  If not, we do not broadcast any sensor and
// target statevector information, for Michael Yee's traveling
// salesman module is expecting to receive at least one ground target.
// In the future, we may need to relax this constraint if/when other
// modules interested only in air vehicle tracks are listening...

   int n_total_ground_tracks=
      curr_EarthRegion_ptr->get_dynamic_tracks_group_ptr()->get_n_tracks()+
      curr_EarthRegion_ptr->get_spatially_fixed_tracks_group_ptr()->
      get_n_tracks();
//   cout << "n_total_ground_tracks = " << n_total_ground_tracks << endl;
   if (n_total_ground_tracks <= 0) 
   {
      return false;
   }

// Broadcast dynamic and spatially fixed ground targets' statevectors:

   EarthRegionsGroup_ptr->get_robots_Messenger_ptr()->broadcast_subpacket(
      "START_PACKET");
   curr_EarthRegion_ptr->broadcast_dynamic_tracks();
   curr_EarthRegion_ptr->broadcast_spatially_fixed_tracks();
   curr_EarthRegion_ptr->broadcast_KOZ_tracks();

// Next broadcast statevectors for (airborne platform) sensors:

   broadcast_sensor_statevectors();

   EarthRegionsGroup_ptr->get_robots_Messenger_ptr()->broadcast_subpacket(
      "STOP_PACKET");

   return true;
}

// ---------------------------------------------------------------------
// Member function broadcast_sensor_statevectors() loops over each
// MODEL within the current MODELSGROUP object and broadcasts its
// track's current statevector.  

void MODELSGROUP::broadcast_sensor_statevectors()
{
//   cout << "inside MODELSGROUP::broadcast_sensor_statevectors()" << endl;

//   cout << "n_MODELs = " << get_n_Graphicals() << endl;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      MODEL* MODEL_ptr=get_MODEL_ptr(n);
//      cout << "n = " << n << " MODEL_ptr = " << MODEL_ptr << endl;

// If a track has been instantiated for *MODEL_ptr, broadcast its
// current statevector information to ActiveMQ:
      
      if (MODEL_ptr->get_track_ptr() != NULL)
      {
         double secs_elapsed_since_epoch=
            AnimationController_ptr->get_time_corresponding_to_curr_frame();

// As of 1/24/09, we assume that the zeroth Messenger for
// Predator_MODELSGROUP communicates on the robots channel:

         MODEL_ptr->get_track_ptr()->broadcast_statevector(
            secs_elapsed_since_epoch,&(get_grid_world_origin()),
            get_Messenger_ptr(0));
      }
      else
      {
         cout << "MODEL_ptr = NULL !!!" << endl;
         outputfunc::enter_continue_char();
      }
   } // loop over index n labeling models
}

// ---------------------------------------------------------------------
// Member function broadcast_add_track_to_GoogleEarth_channel takes in
// *track_ptr.  It first broadcasts a START_PACKET message on the GE
// channel.  This method then generates a message containing the
// track's longitude,latitude coordinates.  Finally, it broadcasts a
// STOP_PACKET message.

bool MODELSGROUP::broadcast_add_track_to_GoogleEarth_channel(
   track* track_ptr,bool compute_posns_with_distinct_dirs_flag)
{
//   cout << "inside MODELSGROUP::broadcast_add_track_to_GoogleEarth_channel()" 
//        << endl;
//   cout << "*track_ptr = " << *track_ptr << endl;
//   cout << "EarthRegionsGroup_ptr = " << EarthRegionsGroup_ptr << endl;
//   cout << "movers_group_ptr = " << movers_group_ptr << endl;

   if (EarthRegionsGroup_ptr != NULL && movers_group_ptr != NULL)
   {
      EarthRegion* EarthRegion_ptr=EarthRegionsGroup_ptr->
         get_EarthRegion_ptr(0);

      EarthRegionsGroup_ptr->get_GoogleEarth_Messenger_ptr()->
         broadcast_subpacket("START_PACKET");
      movers_group_ptr->issue_add_track_message(
         EarthRegion_ptr->get_northern_hemisphere_flag(),
         EarthRegion_ptr->get_UTM_zonenumber(),track_ptr,
         compute_posns_with_distinct_dirs_flag);
      EarthRegionsGroup_ptr->get_GoogleEarth_Messenger_ptr()->
         broadcast_subpacket("STOP_PACKET");
      return true;
   }
   return false;
}

// ---------------------------------------------------------------------
bool MODELSGROUP::broadcast_add_tracks_to_GoogleEarth_channel(
   const vector<track*> track_ptrs)
{
//   cout << "inside MODELSGROUP::broadcast_add_tracks_to_GoogleEarth_channel()"
//        << endl;

   if (EarthRegionsGroup_ptr != NULL && movers_group_ptr != NULL)
   {
      EarthRegion* EarthRegion_ptr=EarthRegionsGroup_ptr->
         get_EarthRegion_ptr(0);

      EarthRegionsGroup_ptr->get_GoogleEarth_Messenger_ptr()->
         broadcast_subpacket("START_PACKET");
      for (unsigned int t=0; t<track_ptrs.size(); t++)
      {
         bool compute_posns_with_distinct_dirs_flag=false;
         movers_group_ptr->issue_add_track_message(
            EarthRegion_ptr->get_northern_hemisphere_flag(),
            EarthRegion_ptr->get_UTM_zonenumber(),track_ptrs[t],
            compute_posns_with_distinct_dirs_flag);
      }
      EarthRegionsGroup_ptr->get_GoogleEarth_Messenger_ptr()->
         broadcast_subpacket("STOP_PACKET");

      return true;
   }
   return false;
}

// ---------------------------------------------------------------------
bool MODELSGROUP::broadcast_delete_tracks_to_GoogleEarth_channel(
   const vector<int> track_IDs)
{
//   cout << "inside MODELSGROUP::broadcast_delete_tracks_to_GoogleEarth_channel()" 
// 	  << endl;

   if (EarthRegionsGroup_ptr != NULL && movers_group_ptr != NULL &&
       track_IDs.size() > 0)
   {
      EarthRegionsGroup_ptr->get_GoogleEarth_Messenger_ptr()->
         broadcast_subpacket("START_PACKET");
      for (unsigned int t=0; t<track_IDs.size(); t++)
      {
         movers_group_ptr->issue_delete_track_message(track_IDs[t]);
      }
      EarthRegionsGroup_ptr->get_GoogleEarth_Messenger_ptr()->
         broadcast_subpacket("STOP_PACKET");
      return true;
   }
   return false;
}

// ---------------------------------------------------------------------
// Member function generate_task_assignments takes in the ID for some
// UAV and the ground targets which it is supposed to visit in the
// immediate future.  It first converts the ground target IDs into
// ground target positions. then resets the Path_PolyLine_ptr
// corresponding to the input UAV so that the aircraft flies over the
// input ground targets.  Finally, this method updates the UAV MODEL's
// track with the new flight path information.  This method
// dynamically generates and returns a reduced track containing only
// waypoint locations for the UAV corresponding to input UAV_ID.

track* MODELSGROUP::generate_task_assignments(
   int UAV_ID,const vector<int>& ground_target_IDs)
{
//   cout << "inside MODELSGROUP::generate_task_assignments()" << endl;
//   cout << "UAV_ID = " << UAV_ID << endl;

// Perform sanity check on ground_target_ID values:

   for (unsigned int i=0; i<ground_target_IDs.size(); i++)
   {
      if (ground_target_IDs[i] < 0) return NULL;
   }

// Perform sanity check on UAV_ID value:

   bool UAV_ID_OK_flag=false;
   for (unsigned int j=0; j<get_n_Graphicals(); j++)
   {
      int curr_ID=get_MODEL_ptr(j)->get_ID();
//      cout << "j = " << j << " curr_ID = " << curr_ID << endl;
      if (UAV_ID==curr_ID)
      {
         UAV_ID_OK_flag=true;
      }
   }
//   cout << "UAV_ID_OK_flag = " << UAV_ID_OK_flag << endl;
   if (!UAV_ID_OK_flag)
   {
      cout << "No UAV with ID = " << UAV_ID << " found in MODELSGROUP!"
           << endl;
      return NULL;
   }

   if (Path_PolyLinesGroup_ptr==NULL) return NULL;

   if (EarthRegionsGroup_ptr==NULL) return NULL;
   EarthRegion* EarthRegion_ptr=EarthRegionsGroup_ptr->
      get_ID_labeled_EarthRegion_ptr(0);
   movers_group* movers_group_ptr=EarthRegion_ptr->get_movers_group_ptr();

// As of Jan 22, 2009, we assume that the ground targets are either
// all static ROIs or else all (potentially) dynamic vehicles.  We do
// not allow for a mixture of ROIs and vehicles...

   vector<threevector> ground_target_posns=movers_group_ptr->
      generate_ground_target_posns(get_curr_t(),ground_target_IDs,mover::ROI);
//   cout << "ground_target_posns.size() = " 
//        << ground_target_posns.size() << endl;
   
// If no static ROI ground targets were found, search next for moving
// vehicle ground targets:

   if (ground_target_posns.size()==0)
   {
      double curr_world_time=
         AnimationController_ptr->get_time_corresponding_to_curr_frame();
      ground_target_posns=movers_group_ptr->
         generate_ground_target_posns(
            curr_world_time,ground_target_IDs,mover::VEHICLE);
   }

   MODEL* UAV_MODEL_ptr=get_ID_labeled_MODEL_ptr(UAV_ID);
//   cout << "UAV_MODEL_ptr = " << UAV_MODEL_ptr << endl;
   if (UAV_MODEL_ptr==NULL) return NULL;
   
   PolyLine* Path_PolyLine_ptr=Path_PolyLinesGroup_ptr->
      get_ID_labeled_PolyLine_ptr(UAV_ID);
   PolyLine* flight_path_PolyLine_ptr=generate_flight_PolyLine_among_ROIs(
      UAV_MODEL_ptr,ground_target_posns,Path_PolyLine_ptr);
   track* track_ptr=update_UAV_track(flight_path_PolyLine_ptr,UAV_MODEL_ptr);
   recolor_UAV_track(track_ptr,flight_path_PolyLine_ptr);

// For ActiveMQ broadcast purposes, dynamically generate a reduced
// track whose information content essentially matches that within
// *flight_path_PolyLine_ptr.  Time entries correspond waypoint
// indices.  After broadcasting the reduced track, it needs to be
// deleted.

   track* reduced_track_ptr=new track(UAV_ID);
   reduced_track_ptr->set_label_ID(UAV_ID);
   reduced_track_ptr->set_XYZ_coords(0,track_ptr->get_earliest_posn());
   for (unsigned int i=0; i<ground_target_posns.size(); i++)
   {
      reduced_track_ptr->set_XYZ_coords(i+1,ground_target_posns[i]);
   }
//   cout << "*reduced_track_ptr = " << *reduced_track_ptr << endl;

   return reduced_track_ptr;
//   return track_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_flight_PolyLine_among_ROIs takes in an
// ordered sequence of IDs for ROIs to be visited by a UAV.  It first
// recovers the UAV's track up till the current time.  It next reads
// in the fixed ground positions of the ROIs.  It overrides any
// existing flight path PolyLine and generates a new one that visits
// each ROI.  Finally,this method resets the altitude of the
// PolyLine's vertices to
// Path_PolyLinesGroup_ptr->constant_vertices_altitude.

PolyLine* MODELSGROUP::generate_flight_PolyLine_among_ROIs(
   MODEL* UAV_MODEL_ptr,const vector<threevector>& ROI_posns,
   PolyLine*& Path_PolyLine_ptr)
{
//   cout << "inside MODELSGROUP::generate_flight_PolyLine_among_ROIs()" 
//        << endl;

   threevector prev_posn(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   threevector curr_posn;
   vector<threevector> UAV_posns;

   if (UAV_MODEL_ptr->get_UVW_coords(
      get_curr_t(),get_passnumber(),curr_posn))
   {
      UAV_posns.push_back(curr_posn);
      prev_posn=curr_posn;
   }

//   cout << "Path_PolyLine_ptr = " << Path_PolyLine_ptr << endl;

   if (Path_PolyLine_ptr != NULL)
   {
      Path_PolyLinesGroup_ptr->destroy_PolyLine(Path_PolyLine_ptr);
      Path_PolyLine_ptr=NULL;
   }

//   cout << "ROI_posns.size() = " << ROI_posns.size() << endl;
   for (unsigned int i=0; i<ROI_posns.size(); i++)
   {
      UAV_posns.push_back(ROI_posns[i]);
   }

   osg::Vec4 path_color=colorfunc::get_OSG_color(
      UAV_flight_path_color(UAV_MODEL_ptr->get_ID()));
   bool force_display_flag=false;
   bool single_polyline_per_geode_flag=true;
   int n_text_messages=0;
   PolyLine* flight_Path_PolyLine_ptr=Path_PolyLinesGroup_ptr->
      generate_new_PolyLine(
         UAV_posns,path_color,
         force_display_flag,single_polyline_per_geode_flag,
         n_text_messages,UAV_MODEL_ptr->get_ID());
   Path_PolyLinesGroup_ptr->reset_PolyLine_altitudes(
      flight_Path_PolyLine_ptr);
   add_flight_path_arrows(flight_Path_PolyLine_ptr);
   flight_Path_PolyLine_ptr->set_entry_finished_flag(true);
   return flight_Path_PolyLine_ptr;
}

// ---------------------------------------------------------------------
// Member function UAV_flight_path_color sets distinct colors for up
// to four UAV flight path PolyLines.  The colorings are similar to
// the UAV OBSFRUSTUM's colorings established in
// OBSFRUSTUM::set_UAV_OBSFRUSTUM_colorings().

colorfunc::Color MODELSGROUP::UAV_flight_path_color(int UAV_ID)
{
//   cout << "inside MODELSGROUP::UAV_flight_path_color()" << endl;

   vector<colorfunc::Color> colors;
   colors.push_back(colorfunc::cyan);
   colors.push_back(colorfunc::green);
   colors.push_back(colorfunc::yellow);
   colors.push_back(colorfunc::blue);
   colors.push_back(colorfunc::orange);
   colors.push_back(colorfunc::purple);
   colors.push_back(colorfunc::brick);
   colors.push_back(colorfunc::pink);
   colors.push_back(colorfunc::blgr);
   colors.push_back(colorfunc::yegr);

   int color_scheme=UAV_ID%(int(colors.size()));
   colorfunc::Color FlightPathColor=colors[color_scheme];

   return FlightPathColor;
}

// ---------------------------------------------------------------------
// Member function add_flight_path_arrows()

void MODELSGROUP::add_flight_path_arrows(
   PolyLine* flight_PolyLine_ptr,double scale_factor)
{
//   double distance_between_arrows=1000;	// meters
   double distance_between_arrows=2000*scale_factor;	// meters
//   double linewidth=5;
//   double linewidth=10;
//   double linewidth=15*scale_factor;
   double linewidth=20*scale_factor;
   flight_PolyLine_ptr->add_flow_direction_arrows(
      distance_between_arrows,linewidth);
}

// ---------------------------------------------------------------------
// Member function alter_UAV_Path_PolyLine retrieves the UAV MODEL and
// flight PolyLine corresponding to member label ID_for_path_to_alter.
// It then merges this PolyLine with the most recently added PolyLine.
// After recomputing the modified Path PolyLine's sampled edge points
// kdtree, this method updates the UAV model's track from its initial
// starting time.

void MODELSGROUP::alter_UAV_Path_PolyLine()
{
//   cout << "inside MODELSGROUP::alter_UAV_Path_PolyLine()" << endl;
   
   PolyLinesGroup* Path_PolyLinesGroup_ptr=get_Path_PolyLinesGroup_ptr();
   if (Path_PolyLinesGroup_ptr == NULL)
   {
      cout << "Error in MODELSGROUP::alter_MODEL_Path_PolyLine()" << endl;
      cout << "Path_PolyLinesGroup_ptr=NULL!" << endl;
      return;
   }

//   cout << "ID_for_path_to_alter = " << ID_for_path_to_alter << endl;
   MODEL* Predator_MODEL_ptr=get_ID_labeled_MODEL_ptr(ID_for_path_to_alter);

   PolyLine* original_PolyLine_ptr=Path_PolyLinesGroup_ptr->
      get_ID_labeled_PolyLine_ptr(ID_for_path_to_alter);
   PolyLine* revised_Path_PolyLine_ptr=
      Path_PolyLinesGroup_ptr->merge_PolyLines(original_PolyLine_ptr);

   double regular_vertex_spacing=100;	// meters
   revised_Path_PolyLine_ptr->get_or_set_polyline_ptr()->
      generate_sampled_edge_points_kdtree(regular_vertex_spacing);

// After original and most recently selected PolyLines have been
// merged, reset all UAV PolyLine altitudes to several kilometers
// above world grid:

   initialize_Path_PolyLinesGroup();
   Path_PolyLinesGroup_ptr->reset_PolyLine_altitudes(
      revised_Path_PolyLine_ptr);

// Recompute Predator MODEL's track from its initial starting time and
// NOT from the current game clock's world time:

   track* track_ptr=Predator_MODEL_ptr->get_track_ptr();
   double initial_time=track_ptr->get_earliest_time();
   int initial_framenumber=get_AnimationController_ptr()->
      get_frame_corresponding_to_time(initial_time);
//   cout << "initial_time = " << initial_time
//        << " initial_frame = " << initial_framenumber << endl;

   update_UAV_track(initial_framenumber,revised_Path_PolyLine_ptr,
                    Predator_MODEL_ptr);
   set_alter_UAV_path_flag(false);
   set_ID_for_path_to_alter(-1);

   broadcast_add_track_to_GoogleEarth_channel(track_ptr);
}

// ---------------------------------------------------------------------
// Member function purge_UAV_MODELS_and_tracks() destroys all UAV
// tracks, PolyLines and movers.

vector<int> MODELSGROUP::purge_UAV_MODELS_and_tracks()
{
//   cout << "inside MODELSGROUP::purge_UAV_MODELS_and_tracks()" << endl;
   vector<int> destroyed_track_IDs;
   for (unsigned int m=0; m<get_n_Graphicals(); m++)
   {
      track* track_ptr=get_MODEL_ptr(m)->get_track_ptr();
//      cout << "m = " << m << " track_ptr = " << track_ptr << endl;
      destroyed_track_IDs.push_back(track_ptr->get_label_ID());
   }
   tracks_group_ptr->destroy_all_tracks();
   Path_PolyLinesGroup_ptr->destroy_all_PolyLines();
   destroy_all_MODELS();
   movers_group_ptr->purge_all_particular_movers(mover::UAV);

//   cout << "at end of MODELSGROUP::purge_UAV_MODELS_and_tracks()" << endl;
//   cout << "Path_PolyLinesGroup_ptr->get_n_Graphicals() = "
//        << Path_PolyLinesGroup_ptr->get_n_Graphicals() << endl;
   
   return destroyed_track_IDs;
}

// ==========================================================================
// RTPS ROI Polyhedra member functions
// ==========================================================================

// Member function destroy_ground_bbox() deletes member
// *ground_bbox_ptr and any Polyhedra within *ROI_PolyhedraGroup_ptr.

void MODELSGROUP::destroy_ground_bbox()
{
//   cout << "inside MODELSGROUP::destroy_ground_bbox()" << endl;

   delete ground_bbox_ptr;
   ground_bbox_ptr=NULL;

   if (ROI_PolyhedraGroup_ptr != NULL)
   {
//      cout << "ROI_PolyhedraGroup_ptr->get_n_Graphicals() = "
//           << ROI_PolyhedraGroup_ptr->get_n_Graphicals() << endl;
      ROI_PolyhedraGroup_ptr->destroy_all_Polyhedra();
   }
}

// ---------------------------------------------------------------------
// Member function set_ground_bbox() takes in lower left and upper
// right bbox corner coordinates.  It deletes and reinstantiates
// bounding box member *ground_bbox_ptr using these inputs.  This
// method also instantiates a translucent ROI_Polyhedron at the
// bounding box location so that the user can see where it resides
// within the 3D map.

void MODELSGROUP::set_ground_bbox(
   double lower_left_longitude,double lower_left_latitude,
   double upper_right_longitude,double upper_right_latitude)
{
//   cout << "inside MODELSGROUP::set_ground_bbox()" << endl;
//   cout << "lower_left_longitude = " << lower_left_longitude
//        << " lower_left_latitude = " << lower_left_latitude << endl;
//   cout << "upper_right_longitude = " << upper_right_longitude
//        << " upper_right_latitude = " << upper_right_latitude << endl;

   int UTM_zone=get_EarthRegionsGroup_ptr()->get_specified_UTM_zonenumber();
   geopoint lower_left_corner(lower_left_longitude,lower_left_latitude,0,
                              UTM_zone);
   geopoint upper_right_corner(upper_right_longitude,upper_right_latitude,0,
                               UTM_zone);

   ground_bbox_ptr=new bounding_box(
      lower_left_corner.get_UTM_easting(),
      upper_right_corner.get_UTM_easting(),
      lower_left_corner.get_UTM_northing(),
      upper_right_corner.get_UTM_northing());
//   cout << "*ground_bbox_ptr = " << *ground_bbox_ptr << endl;

   int Polyhedra_subgroup=0;
   colorfunc::Color bbox_color=colorfunc::pink;
//   colorfunc::Color bbox_color=colorfunc::magenta;
//   colorfunc::Color bbox_color=colorfunc::cyan;
//   colorfunc::Color bbox_color=colorfunc::purple;
//   colorfunc::Color bbox_color=colorfunc::blue;
//   colorfunc::Color bbox_color=colorfunc::blgr;

/*
   double h=390;
   double s=1;
   double v=0.75;
   colorfunc::HSV bbox_hsv(h,s,v);
   colorfunc::RGB bbox_rgb=colorfunc::hsv_to_RGB(bbox_hsv);

   osg::Vec4 bbox_color=colorfunc::get_OSG_color(bbox_rgb,bbox_alpha);
*/
   double bbox_alpha=0.40;

   if (ROI_PolyhedraGroup_ptr != NULL)
   {
//       Polyhedron* ROI_Polyhedron_ptr=
         ROI_PolyhedraGroup_ptr->generate_bbox(
            ground_bbox_ptr->get_xmin(),
            ground_bbox_ptr->get_xmax(),
            ground_bbox_ptr->get_ymin(),
            ground_bbox_ptr->get_ymax(),
            Polyhedra_subgroup,bbox_color,bbox_alpha);
   }
}

// ==========================================================================
// Human model member functions
// ==========================================================================

// Member function generate_man_MODEL()

MODEL* MODELSGROUP::generate_man_MODEL(int& OSGsubPAT_number)
{
   string banner="Generating Man MODEL";
   outputfunc::write_banner(banner);

   string humans_dir(getenv("OSG_FILE_PATH"));
   humans_dir += "/humans/";
   string model_filename=humans_dir+"male.ive";

   OSGsubPAT_number=get_n_OSGsubPATs();
   MODEL* human_ptr=generate_new_Model(model_filename,OSGsubPAT_number);

   const double model_scalefactor=0.00777224; // male with avg 1.75m height
   set_constant_scale(human_ptr,model_scalefactor);

// Note: Average head height = 1/7.25 average human height = 24.1 cm =
// 9.5 inches

   return human_ptr;
}

// ==========================================================================
// Real-time MODEL updating member functions
// ==========================================================================

void MODELSGROUP::update_Aircraft_MODEL(
   threevector posn,double roll,double pitch,double yaw,
   MODEL* Aircraft_MODEL_ptr)
{
//   cout << "inside MODELSGROUP::update_Aircraft_MODEL()" << endl;

   threevector RPY(roll,pitch,yaw);

   Aircraft_MODEL_ptr->set_attitude_posn(
      get_curr_t(),get_passnumber(),RPY,posn);
}

// ---------------------------------------------------------------------
// Member function generate_Aircraft_MODEL_track_and_mover() instatiates
// new track and mover objects corresponding to the input Aircraft MODEL.

void MODELSGROUP::generate_Aircraft_MODEL_track_and_mover(
   MODEL* Aircraft_MODEL_ptr)
{
//   cout << "inside MODELSGROUP::generate_Aircraft_MODEL_track_and_mover()" << endl;

// Generate track for Aircraft model.  For ActiveMQ messaging with
// Luca's MATLAB Aircraft task assignment code purposes, set track's
// label_ID equal to Aircraft MODEL's ID and NOT equal to track's ID:

   track* curr_track_ptr=tracks_group_ptr->generate_new_track();
   curr_track_ptr->set_description("Airborne Sensor");
   curr_track_ptr->set_label_ID(Aircraft_MODEL_ptr->get_ID());
   Aircraft_MODEL_ptr->set_track_ptr(curr_track_ptr);

// Instantiate new Aircraft mover and set its ID equal to the Aircraft
// MODEL's:

   mover* new_mover_ptr=movers_group_ptr->generate_new_mover(
      mover::UAV,Aircraft_MODEL_ptr->get_ID());
   
   new_mover_ptr->set_track_ptr(curr_track_ptr);
   movers_group_ptr->add_mover_to_outgoing_queue(new_mover_ptr);
}
