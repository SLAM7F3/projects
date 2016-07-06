// ==========================================================================
// LOSMODELSGROUP class member function definitions
// ==========================================================================
// Last modified on 6/27/12; 7/3/12; 5/19/13; 4/5/14
// ==========================================================================

#include "image/binaryimagefuncs.h"
#include "video/camerafuncs.h"
#include "image/compositefuncs.h"
#include "filter/filterfuncs.h"
#include "postgres/gis_database.h"
#include "image/imagefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/osgModels/LOSMODELSGROUP.h"
#include "osg/osg2D/MoviesGroup.h"
#include "numerical/param_range.h"
#include "numrec/nrfuncs.h"
#include "osg/osgModels/OBSFRUSTUM.h"
#include "osg/osgModels/OBSFRUSTUMfuncs.h"
#include "osg/osgOperations/Operations.h"
#include "geometry/polyline.h"
#include "image/raster_parser.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void LOSMODELSGROUP::allocate_member_objects()
{
   target_visibility_map_ptr=new TARGET_VISIBILITY_MAP;
   target_skymap_map_ptr=new TARGET_SKYMAP_MAP;
   LineSegmentsGroup_ptr=new LineSegmentsGroup(
      3,get_pass_ptr(),get_AnimationController_ptr());
   insert_OSGgroup_into_OSGsubPAT(
      LineSegmentsGroup_ptr->get_OSGgroup_ptr());

   string broker_URL="tcp://127.0.0.1:61616";
   string message_queue_channel_name="viewer_update";
   cancel_messenger_ptr=new Messenger(
      broker_URL,message_queue_channel_name);
}		       

void LOSMODELSGROUP::initialize_member_objects()
{

   GraphicalsGroup_name="LOSMODELSGROUP";

   altitude_dependent_MODEL_scale_flag=true;
   ladar_height_data_flag=false;
   raytrace_ground_targets_flag=false;
   score_broadcasted_flag=false;
   update_dynamic_aircraft_MODEL_flag=false;
   fixed_aircraft_MODEL_orientation_flag=false;
   north_up_orientation_flag=false;
   PYXIS_server_flag=false;

//   min_raytrace_range=1 * 1000;	// meters // for alg development only
   set_min_raytrace_range(30 * 1000);	// meters
   set_max_raytrace_range(200 * 1000);	// meters

   raytrace_progress=0;
   ray_type=single_air_to_multi_ground;
   reload_DTED_tiles_flag=true;
   endpoint_size_prefactor=1;
   horiz_frustum_FOV=vert_frustum_FOV=-1;

   images_database_ptr=NULL;
   GroundTarget_SignPostsGroup_ptr=NULL;
   MoviesGroup_ptr=NULL;
   PointCloudsGroup_ptr=NULL;
   ReferenceFrameHUD_ptr=NULL;
   threat_texture_rectangle_ptr=NULL;
   threatmap_twoDarray_ptr=NULL;

   get_OSGgroup_ptr()->setUpdateCallback(
      new AbstractOSGCallback<LOSMODELSGROUP>(
         this,&LOSMODELSGROUP::update_display));
}		       

LOSMODELSGROUP::LOSMODELSGROUP(Pass* PI_ptr,threevector* GO_ptr,
                         AnimationController* AC_ptr):
   MODELSGROUP(PI_ptr,GO_ptr,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

LOSMODELSGROUP::LOSMODELSGROUP(
   Pass* PI_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
   threevector* GO_ptr,osgGA::Terrain_Manipulator* CM_3D_ptr,
   Operations* O_ptr):
   MODELSGROUP(PI_ptr,PLG_ptr,PLPH_ptr,GO_ptr,CM_3D_ptr,O_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

LOSMODELSGROUP::LOSMODELSGROUP(
   Pass* PI_ptr,PolyLinesGroup* PLG_ptr,PolyLinePickHandler* PLPH_ptr,
   threevector* GO_ptr,osgGA::Terrain_Manipulator* CM_3D_ptr,
   Operations* O_ptr,MoviesGroup* MG_ptr):
   MODELSGROUP(PI_ptr,PLG_ptr,PLPH_ptr,GO_ptr,CM_3D_ptr,O_ptr)
{	
//   cout << "inside LOSMODELSGROUP constructor #3" << endl;
   
   initialize_member_objects();
   allocate_member_objects();
   MoviesGroup_ptr=MG_ptr;
}		       

LOSMODELSGROUP::~LOSMODELSGROUP()
{
//   cout << "inside LOSMODELSGROUP destructor" << endl;
   delete target_visibility_map_ptr;
   delete target_skymap_map_ptr;
   delete LineSegmentsGroup_ptr;
   delete cancel_messenger_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const LOSMODELSGROUP& M)
{
   int node_counter=0;
   for (unsigned int n=0; n<M.get_n_Graphicals(); n++)
   {
      LOSMODEL* LOSMODEL_ptr=M.get_LOSMODEL_ptr(n);
      outstream << "Model node # " << node_counter++ << endl;
      outstream << "Model = " << *LOSMODEL_ptr << endl;
   }
   return outstream;
}

// ==========================================================================
// Set & get methods
// ==========================================================================

double LOSMODELSGROUP::get_raytrace_cellsize() const
{
   double raytrace_cellsize;

   if (ladar_height_data_flag)
   {
//      raytrace_cellsize=0.5;		// meters  	HAFB mini-map
      raytrace_cellsize=1;		// meters  	FOB Blessing
//      raytrace_cellsize=2;		// meters
//      raytrace_cellsize=3;		// meters
   }
   else
   {
      raytrace_cellsize=250;		// meters
   }
   return raytrace_cellsize;
}

// ==========================================================================
// Model creation and manipulation methods
// ==========================================================================

// Member function generate_empty_MODEL()

MODEL* LOSMODELSGROUP::generate_empty_MODEL(
   double min_raytrace_range,double max_raytrace_range)
{
   cout << "inside LOSMODELSGROUP::generate_empty_Model()" << endl;

   int OSGsubPAT_number=get_n_OSGsubPATs();
   double model_scalefactor=1;
   set_instantiate_OBSFRUSTAGROUP_flag(false);
   MODEL* MODEL_ptr=MODELSGROUP::generate_Cessna_MODEL(
      OSGsubPAT_number,model_scalefactor);
   
   MODEL_ptr->set_min_raytrace_range(min_raytrace_range);
   MODEL_ptr->set_max_raytrace_range(max_raytrace_range);
   return MODEL_ptr;
}

// --------------------------------------------------------------------------
// Member function generate_LiMIT_MODEL() dynamically instantiates a
// LiMIT model and sets various aircraft parameters such as speed,
// extremal raytracing ranges, OBSFRUSTUM roll & pitch.

MODEL* LOSMODELSGROUP::generate_LiMIT_MODEL()
{
//   cout << "inside LOSMODELSGROUP::generate_LiMIT_MODEL()" << endl;

   MODEL* LiMIT_MODEL_ptr=MODELSGROUP::generate_LiMIT_MODEL();
   LiMIT_MODEL_ptr->set_min_raytrace_range(min_raytrace_range);
   LiMIT_MODEL_ptr->set_max_raytrace_range(max_raytrace_range);
   return LiMIT_MODEL_ptr;
}

// --------------------------------------------------------------------------
MODEL* LOSMODELSGROUP::generate_LiMIT_MODEL_for_flight_PolyLine(
   PolyLine* flight_PolyLine_ptr)
{
//   cout << "inside LOSMODELSGROUP::generate_LiMIT_MODEL_for_flight_PolyLine()"
//        << endl;

   MODEL* LiMIT_MODEL_ptr=
      MODELSGROUP::generate_LiMIT_MODEL_for_flight_PolyLine(
         flight_PolyLine_ptr);
   LiMIT_MODEL_ptr->set_min_raytrace_range(min_raytrace_range);
   LiMIT_MODEL_ptr->set_max_raytrace_range(max_raytrace_range);

// When working with ALIRT ladar imagery rather than DTED-2 height
// data, suppress display of aircraft OBSFRUSTA:

   if (ladar_height_data_flag)
   {
      LiMIT_MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->erase_all_OBSFRUSTA();
   }

   return LiMIT_MODEL_ptr;
}

// ---------------------------------------------------------------------
// Member function set_altitude_dependent_MODEL_scale()
// retrieves the current altitude of the virtual camera.  It resets
// the scale for all MODEL members of *this as a linear function of
// the altitude.  We wrote this method in order to make the Cessna
// model findable at all reasonable virtual camera altitudes.

void LOSMODELSGROUP::set_altitude_dependent_MODEL_scale()
{
//   cout << "inside LOSMODELSGROUP::set_altitude_dependent_MODEL_scale()" 
//        << endl;

   const double zmin=25*1000;	// meters
   const double zmax=2500*1000;	// meters

   double size_min=10;
   double size_max=1000;
   double curr_size=compute_altitude_dependent_size(
      zmin,zmax,size_min,size_max);
//   cout << "curr_size = " << curr_size << endl;

   vector<MODEL*> MODEL_ptrs=get_all_MODEL_ptrs();
   for (unsigned int n=0; n<MODEL_ptrs.size(); n++)
   {
      MODEL* MODEL_ptr=MODEL_ptrs[n];
      set_constant_scale(MODEL_ptr,curr_size);
   }
}

// --------------------------------------------------------------------------
// Member function update_display is repeatedly executed by a callback
// in a main program.

void LOSMODELSGROUP::update_display()
{   
//   cout << "inside LOSMODELSGROUP::update_display()" << endl;
//   cout << "curr_t = " << get_curr_t() << endl;

   parse_latest_messages();

// FAKE FAKE:  Sun Oct 9, 2011 at 12:19 pm

// Experiment with changing size of Cessna as a function of virtual
// camera altitude.  Definitely need to do this for program
// FLIRSIM/SAURON.  Not sure if this change is good for LOST or not...

   if (altitude_dependent_MODEL_scale_flag)
      set_altitude_dependent_MODEL_scale();

   if (update_dynamic_aircraft_MODEL_flag)
      update_dynamic_aircraft_model();

   double min_altitude=get_grid_world_origin().get(2)+1000;	// meters
   follow_selected_Geometrical(min_altitude);

// Compute & display dynamic OBSFRUSTA only if they're not currently
// masked:

   vector<MODEL*> MODEL_ptrs=get_all_MODEL_ptrs();
//    cout << "MODEL_ptrs.size() = " << MODEL_ptrs.size() << endl;
   for (unsigned int n=0; n<MODEL_ptrs.size(); n++)
   {
      MODEL* MODEL_ptr=MODEL_ptrs[n];

      int OSGsubPAT_number=OSGsubPAT_parent_of_Graphical(MODEL_ptr);
      int nodemask=get_OSGsubPAT_nodemask(OSGsubPAT_number);
//      cout << "OSGsubPAT_number = " << OSGsubPAT_number
//           << " nodemask = " << nodemask << endl;
      if (nodemask==1 && !compute_skymap_flag)
      {
         bool OBSFRUSTA_previously_built_flag=false;
//         cout << "dyn_compute_OBSFRUSTUM_flag = "
//              << MODEL_ptr->get_dynamically_compute_OBSFRUSTUM_flag()
//              << endl;
         if (MODEL_ptr->get_dynamically_compute_OBSFRUSTUM_flag())
         {
            MODEL_ptr->compute_dynamic_OBSFRUSTA(
               get_curr_t(),get_passnumber(),EarthRegionsGroup_ptr,
               n_future_repeats,OBSFRUSTA_previously_built_flag);
         }
         
// Check if cancel raytracing message has been received:

         string cancel_msg=
            cancel_messenger_ptr->check_for_cancel_operation_message();
//            cout << " cancel_msg = " << cancel_msg << endl;

         if (cancel_msg=="raytracing" &&
             MODEL_ptr->get_raytrace_occluded_ground_regions_flag())
         {
            MODEL_ptr->set_raytrace_occluded_ground_regions_flag(false);
            clear_raytracing_results();
            set_display_ImageNumberHUD(true);
            cancel_messenger_ptr->clear_cancelled_operation();
         }

// Compute visibility of ground point targets to aerial sensor:

         if (MODEL_ptr->get_OBSFRUSTAGROUP_ptr() != NULL &&
             MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->get_n_Graphicals() > 0 &&
             MODEL_ptr->get_raytrace_occluded_ground_regions_flag())
         {
            bool ROI_previously_raytraced_flag=true;

            if (MODEL_ptr->get_raytrace_ROI_flag())
            {
               ROI_previously_raytraced_flag=
                  MODEL_ptr->recover_or_compute_OBSFRUSTA_occlusion(
                     OBSFRUSTA_previously_built_flag,
                     get_curr_t(),get_passnumber(),
                     EarthRegionsGroup_ptr,ColorGeodeVisitor_ptr,
                     ground_bbox_ptr,viewer_Messenger_ptr,
                     get_raytrace_cellsize());
            }

// If LOST is being run as a "black box" server, output time-averaged
// line-of-sight results once ROI visibility computation has finished:

            if (PYXIS_server_flag && ROI_previously_raytraced_flag)
            {
               export_PYXIS_ROI_visibility_files();
            }

            bool tgts_previously_raytraced_flag=
               compute_target_visibilities_along_flightpath(
                  MODEL_ptr,reload_DTED_tiles_flag);

// As of 11/4/09, we don't want to have to perform expensive DTED tile
// reloading every time we compute visibility to ground point targets
// at finely sampled aerial orbit locations.  We assume that we only
// need to periodically reload DTED tiles.  

            if (!MODEL_ptr->get_raytrace_ROI_flag())
            {
               int n_frames_per_period=5;
//                  int n_frames_per_period=10;	// fast debugging value
               if (modulo(int(get_curr_t()),n_frames_per_period)==0)
               {
                  reload_DTED_tiles_flag = true;
               }
               else
               {
                  reload_DTED_tiles_flag = false;
               }
            }
               
            draw_colored_single_air_to_multi_ground_rays(
               tgts_previously_raytraced_flag,MODEL_ptr);

            if (viewer_Messenger_ptr != NULL &&
                (!ROI_previously_raytraced_flag ||
                 !tgts_previously_raytraced_flag))
            {
               raytrace_progress += 
                  1.0/double(get_AnimationController_ptr()->get_nframes());
               raytrace_progress=basic_math::min(raytrace_progress,1.0);
               double rounded_raytrace_progress=
                  0.01*basic_math::round(100*raytrace_progress);
               string progress_type="raytracing";
               viewer_Messenger_ptr->broadcast_progress(
                  rounded_raytrace_progress,progress_type);
               cout << "raytrace_progress = " << raytrace_progress 
                    << endl;
               viewer_Messenger_ptr->broadcast_current_framenumber(
                  get_curr_t());
            }
            else
            {

// If movie's play button has been pressed while AVI file output is
// being generated, do NOT pause AnimationController's clock:

//                  if (!get_AnimationController_ptr()->
//                      get_AVI_movie_generation_flag())
               {
                  get_AnimationController_ptr()->setState(
                     AnimationController::PAUSE);
               }
            }

         } // OBSFRUSTA.size > 0 && 
         //   raytrace_occluded_ground_regions_flag==true conditional

// For line-of-sight project, reset OBSFRUSTUM's volume alpha as a
// function of virtual camera's altitude:

         if (LiMIT_FOV_OBSFRUSTUM_flag)
         {
            OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=MODEL_ptr->
               get_OBSFRUSTAGROUP_ptr();
            OBSFRUSTAGROUP_ptr->
               set_altitude_dependent_OBSFRUSTA_volume_alpha();
         }
      } // nodemask==1 conditional

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
// which handles instantiation of the time-consuming Predator and its
// OBSFRUSTUM while the rest of the OSG event loop can proceed...

         if (Path_PolyLine_ptr->get_entry_finished_flag())
         {
            if (generate_Predator_model_on_next_cycle_flag)
            {
               generate_Predator_model_on_next_cycle_flag=false;
               if (aircraft_model_type==LiMIT)
               {
//                  MODEL* LiMIT_MODEL_ptr=
                     generate_LiMIT_MODEL_for_flight_PolyLine(
                        Path_PolyLine_ptr);
                  if (PYXIS_server_flag)
                  {
                     start_ROI_visibility_computation();
                  }
               }
            }
            else if (!generate_Predator_model_on_next_cycle_flag &&
                     n_Path_PolyLines > get_n_Graphicals() &&
                     !alter_UAV_path_flag)
            {
               generate_Predator_model_on_next_cycle_flag=true;               
            }
         } // Path_Polyline entry finished flag conditional
      }
   } // Path_PolyLinesGroup_ptr != NULL conditional

   LineSegmentsGroup_ptr->update_display();
    
   GraphicalsGroup::update_display();
}

// ==========================================================================
// OBSFRUSTA generation member functions
// ==========================================================================

// Member function compute_OBSFRUSTUM_parameters() extracts min and
// max ground heights from member *PointCloudsGroup_ptr.  It then
// converts the input height-independent extremal ground sensor range
// parameters into a height-independent OBSFRUSTUM elevation opening
// angle and maximum sensor range (based upon a z=0 flat-earth
// approximation).  min_sensor_range takes zground_max into
// account so that nadir-oriented sensors could see every point within
// the input terrain map.

void LOSMODELSGROUP::compute_OBSFRUSTUM_parameters(
   double min_ground_sensor_range,double max_ground_sensor_range,
   double horiz_FOV,int roll_sgn)
{
//   cout << "inside LOSMODELSGROUP::compute_OBSFRUSTUM_parameters()" << endl;

   PointCloud* PointCloud_ptr=PointCloudsGroup_ptr->get_Cloud_ptr(0);
//   double zground_min=PointCloud_ptr->get_min_value(2);
   double zground_max=PointCloud_ptr->get_max_value(2);

   double alt=get_aircraft_altitude();	// meters

   double min_sensor_range=
      sqrt(sqr(alt)+sqr(min_ground_sensor_range));
   double max_sensor_range=
      sqrt(sqr(alt)+sqr(max_ground_sensor_range));
   double theta_min=atan2(min_ground_sensor_range,alt);
   double theta_max=atan2(max_ground_sensor_range,alt);

// Nadir-oriented sensors should be able to see every point within
// the input 3D map when min_ground_sensor_range=0.  So we revise
// min_sensor_range (but NOT theta_min) by taking zground_max into
// account:

   min_sensor_range=
      sqrt(sqr(alt-zground_max)+sqr(min_ground_sensor_range));

   double depression_angle=PI/2-0.5*(theta_min+theta_max);
   double vert_FOV=theta_max-theta_min;
   horiz_FOV *= PI/180;
   double roll=roll_sgn*(PI/2-depression_angle);
   double pitch=0;

//   cout << "zground_min = " << zground_min
//        << " zground_max = " << zground_max << endl;
//  cout << "altitude = " << alt << endl;
//   cout << "min_sensor_range = " << min_sensor_range << " meters"
//        << " max_sensor_range = " << max_sensor_range  << " meters" << endl;
//   cout << "theta_min = " << theta_min*180/PI
//        << " theta_max = " << theta_max*180/PI << endl;
//   cout << "horiz_FOV = " << horiz_FOV*180/PI
//        << " vert_FOV = " << vert_FOV*180/PI << endl;
//   cout << "depression_angle = " << depression_angle*180/PI << endl;
//   cout << "roll angle = " << roll*180/PI << endl;

   set_min_raytrace_range(min_sensor_range);
   set_max_raytrace_range(max_sensor_range);
   set_OBSFRUSTUM_az_extent(horiz_FOV);
   set_OBSFRUSTUM_el_extent(vert_FOV);
   set_OBSFRUSTUM_roll(roll);
   set_OBSFRUSTUM_pitch(pitch);
}

/*

// FAKE FAKE:  12:20 pm on Thurs July 7, 2011
// Hacked version of compute_OBSFRUSTUM_params() for 
// temporary T-storm analysis purposes


void LOSMODELSGROUP::compute_OBSFRUSTUM_parameters(
   double min_ground_sensor_range,double max_ground_sensor_range,
   double horiz_FOV,int roll_sgn)
{
   cout << "inside LOSMODELSGROUP::compute_OBSFRUSTUM_parameters()" << endl;

   PointCloud* PointCloud_ptr=PointCloudsGroup_ptr->get_Cloud_ptr(0);
   double zground_min=PointCloud_ptr->get_min_value(2);
   double zground_max=PointCloud_ptr->get_max_value(2);

   double alt=get_aircraft_altitude();	// meters

   double min_sensor_range=
      sqrt(sqr(alt)+sqr(min_ground_sensor_range));
   double max_sensor_range=
      sqrt(sqr(alt)+sqr(max_ground_sensor_range));
   double theta_min=atan2(min_ground_sensor_range,alt);
   double theta_max=atan2(max_ground_sensor_range,alt);

// Nadir-oriented sensors should be able to see every point within
// the input 3D map when min_ground_sensor_range=0.  So we revise
// min_sensor_range (but NOT theta_min) by taking zground_max into
// account:

   min_sensor_range=
      sqrt(sqr(alt-zground_max)+sqr(min_ground_sensor_range));

   double depression_angle=PI/2-0.5*(theta_min+theta_max);
   double vert_FOV=theta_max-theta_min;
   horiz_FOV *= PI/180;
   double roll=roll_sgn*(PI/2-depression_angle);
   double pitch=0;

   cout << "zground_min = " << zground_min
        << " zground_max = " << zground_max << endl;
   cout << "altitude = " << alt << endl;
   cout << "min_sensor_range = " << min_sensor_range << " meters"
        << " max_sensor_range = " << max_sensor_range  << " meters" << endl;
   cout << "theta_min = " << theta_min*180/PI
        << " theta_max = " << theta_max*180/PI << endl;
   cout << "horiz_FOV = " << horiz_FOV*180/PI
        << " vert_FOV = " << vert_FOV*180/PI << endl;
   cout << "depression_angle = " << depression_angle*180/PI << endl;
   cout << "roll angle = " << roll*180/PI << endl;

   cout << "Enter 5 or 30 for horiz_FOV:" << endl;
   cin >> horiz_FOV;

   if (nearly_equal(horiz_FOV,5))
   {
      horiz_FOV=4.57;
      vert_FOV=2.57;
   }
   else
   {
      horiz_FOV=28.4;
      vert_FOV=16.2;
   }
   cout << "horiz_FOV = " << horiz_FOV 
        << " vert_FOV = " << vert_FOV << endl;

   horiz_FOV *= PI/180;
   vert_FOV *= PI/180;

   min_sensor_range=0;
   max_sensor_range=300*1000;
   
   cout << "Enter depression_angle:" << endl;
   cin >> depression_angle;
   depression_angle *= PI/180;
   roll=roll_sgn*(PI/2-depression_angle);
   cout << "roll = " << roll*180/PI << endl;

   set_min_raytrace_range(min_sensor_range);
   set_max_raytrace_range(max_sensor_range);
   set_OBSFRUSTUM_az_extent(horiz_FOV);
   set_OBSFRUSTUM_el_extent(vert_FOV);
   set_OBSFRUSTUM_roll(roll);
   set_OBSFRUSTUM_pitch(pitch);
}
*/

// ==========================================================================
// Line-of-sight analysis member functions
// ==========================================================================

// Member function display_average_LOS_results()

void LOSMODELSGROUP::display_average_LOS_results()
{   
   cout << "inside LOSMODELSGROUP::display_average_LOS_results()" << endl;
   vector<MODEL*> MODEL_ptrs=get_all_MODEL_ptrs();
   cout << "MODEL_ptrs.size() = " << MODEL_ptrs.size() << endl;
   string geotif_Ptiles_subdir=TilesGroup_ptr->get_geotif_Ptiles_subdir();
   cout << "geotif_Ptiles_subdir = " << geotif_Ptiles_subdir << endl;
   
   vector<string> geotif_filenames;
   if (ladar_height_data_flag)
   {
      geotif_filenames.push_back(
         compute_avg_LOS_ptwoDarray(geotif_Ptiles_subdir));
   }
   else
   {
      geotif_filenames=find_avg_LOS_geotif_files(
         geotif_Ptiles_subdir,ColorGeodeVisitor_ptr);
   }
   cout << "geotif_filenames.size() = " << geotif_filenames.size() << endl;

   for (unsigned int n=0; n<MODEL_ptrs.size(); n++)
   {
      MODEL* MODEL_ptr=MODEL_ptrs[n];
      cout << "n = " << n << " MODEL_ptr = " << MODEL_ptr << endl;
      MODEL_ptr->display_average_LOS_results(
         geotif_filenames,ColorGeodeVisitor_ptr,EarthRegionsGroup_ptr,
         viewer_Messenger_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function compute_avg_LOS_ptwoDarray() reads in all geotif
// files whose names start with "ptwoDarray_OBSFRUSTUM***.tif" from
// the input geotif_Ptiles_subdir.  We assume that these files were
// generated from raytracing of relatively small ALIRT ROIs.  This
// method averages together the contents of all the input geotif files
// and generates an output avg_ptwoDarray.tif file within
// geotif_Ptiles_subdir.

string LOSMODELSGROUP::compute_avg_LOS_ptwoDarray(string geotif_Ptiles_subdir)
{
//   cout << "inside LOSMODELSGROUP::compute_avg_LOS_ptwoDarray()" << endl;
//   cout << "geotif_Ptiles_subdir = " << geotif_Ptiles_subdir << endl;

   string avg_geotiff_filename="";
   
   string substring="ptwoDarray_OBSFRUSTUM";
   vector<string> geotif_filenames=
      filefunc::files_in_subdir_matching_substring(
         geotif_Ptiles_subdir,substring);

   if (geotif_filenames.size()==0) return avg_geotiff_filename;

   vector<twoDarray*> LOS_ptwoDarray_ptrs;
   for (unsigned int f=0; f<geotif_filenames.size(); f++)
   {
      raster_parser RasterParser;
      if (!RasterParser.open_image_file(geotif_filenames[f])) continue;

      int channel_ID=0; 
      RasterParser.fetch_raster_band(channel_ID);

      twoDarray* curr_LOS_ptwoDarray_ptr=
         new twoDarray(RasterParser.get_ztwoDarray_ptr());
      curr_LOS_ptwoDarray_ptr->initialize_values(-1);

      RasterParser.read_raster_data(curr_LOS_ptwoDarray_ptr);

      const double p_min=-1;
      const double p_max=1.01;
      RasterParser.convert_GUInts_to_doubles(
         p_min,p_max,curr_LOS_ptwoDarray_ptr);

      LOS_ptwoDarray_ptrs.push_back(curr_LOS_ptwoDarray_ptr);
      RasterParser.close_image_file();
   } // loop over index f labeling avg_LOS geotif files

   twoDarray* avg_LOS_ptwoDarray_ptr=
      new twoDarray(*LOS_ptwoDarray_ptrs.back());
//   cout << "*avg_LOS_ptwoDarray_ptr = " << *avg_LOS_ptwoDarray_ptr << endl;

   avg_LOS_ptwoDarray_ptr->initialize_values(-1);

   unsigned int n_layers=LOS_ptwoDarray_ptrs.size();
   for (unsigned int f=0; f<n_layers; f++)
   {
      for (unsigned int px=0; px<avg_LOS_ptwoDarray_ptr->get_mdim(); px++)
      {
         for (unsigned int py=0; py<avg_LOS_ptwoDarray_ptr->get_ndim(); py++)
         {
            double curr_p=LOS_ptwoDarray_ptrs[f]->get(px,py);

            if (curr_p < 0.9) continue;

            double avg_p=basic_math::max(0.0,avg_LOS_ptwoDarray_ptr->
            get(px,py));
            avg_LOS_ptwoDarray_ptr->put(px,py,avg_p+curr_p/n_layers);
         } // loop over py index
      } // loop over px index
   } // loop over index f labeling LOS ptwoDarrays   

// Export grey-scale version of time-averaged raytracing results to
// avg_ptwoDarray.tif:

   bool output_floats_flag=false;
   string avg_geotif_filename=geotif_Ptiles_subdir+"avg_ptwoDarray.tif";
//   cout << "avg_geotif_filename = " << avg_geotif_filename << endl;

   int UTM_zone=get_EarthRegionsGroup_ptr()->get_specified_UTM_zonenumber();
   bool northern_hemisphere_flag=get_EarthRegionsGroup_ptr()->
      get_northern_hemisphere_flag();
   
   raster_parser RasterParser;
   RasterParser.write_raster_data(
      output_floats_flag,avg_geotif_filename,
      UTM_zone,northern_hemisphere_flag,avg_LOS_ptwoDarray_ptr);

   vector<string> avg_LOS_tif_filenames;
   avg_LOS_tif_filenames.push_back(avg_geotif_filename);

// Generate colored version of time-averaged raytracing results geotif
// file for Google Earth import purposes:

   string colored_avg_geotif_filename=
      TilesGroup_ptr->convert_avg_geotifs_from_greyscale_to_color(
         avg_LOS_tif_filenames);

   return avg_geotif_filename;
}

// ---------------------------------------------------------------------
// Member function find_avg_LOS_geotif_files() recovers all
// geotif files whose names start with "avg_LOS" from the input
// geotif_Ptiles subdirectory.  It returns their filenames within the
// output STL vector.

vector<string> LOSMODELSGROUP::find_avg_LOS_geotif_files(
   string geotif_Ptiles_subdir,ColorGeodeVisitor* ColorGeodeVisitor_ptr)
{
//   cout << "inside LOSMODELSGROUP::find_avg_LOS_geotif_files()" << endl;
//   cout << "geotif_Ptiles_subdir = " << geotif_Ptiles_subdir << endl;
   
   string substring="avg_LOS";
   vector<string> candidate_geotif_filenames=
      filefunc::files_in_subdir_matching_substring(
         geotif_Ptiles_subdir,substring);

   int ID=0;
   vector<string> geotif_filenames;
   ColorGeodeVisitor_ptr->clear_long_lat_ID_map();
   for (unsigned int i=0; i<candidate_geotif_filenames.size(); i++)
   {
      if (stringfunc::suffix(candidate_geotif_filenames[i])=="tif")
      {
         if (stringfunc::first_substring_location(
            candidate_geotif_filenames[i],"_longlat") < 0)
         {
            string curr_filename=candidate_geotif_filenames[i];
            geotif_filenames.push_back(curr_filename);

// Extract longitude and latitude from geotif filename:

            bool western_hemisphere_flag=false;
            string substring=stringfunc::erase_chars_before_first_substring(
               filefunc::getbasename(curr_filename),"e");
//            cout << "substring = " << substring << endl;
            if (substring.size()==0)
            {
               substring=stringfunc::erase_chars_before_first_substring(
                  filefunc::getbasename(curr_filename),"w");
               western_hemisphere_flag=true;
            }
//            cout << "substring = " << substring << endl;
            string longlatstr=stringfunc::prefix(substring);
//            cout << "longlatstr = " << longlatstr << endl;
            string longitude_str;
            if (western_hemisphere_flag)
            {
               longitude_str=stringfunc::substring_between_substrings(
                  longlatstr,"w","n");
               longitude_str=stringfunc::find_and_replace_char(
                  longitude_str,"w"," ");
            }
            else
            {
               longitude_str=stringfunc::substring_between_substrings(
                  longlatstr,"e","n");
               longitude_str=stringfunc::find_and_replace_char(
                  longitude_str,"e"," ");
            }
            longitude_str=stringfunc::find_and_replace_char(
               longitude_str,"n"," ");
//            cout << "longitude_str = " << longitude_str << endl;
            double longitude=stringfunc::string_to_number(longitude_str);
            if (western_hemisphere_flag) longitude=-longitude;
//            cout << "longitude = " << longitude << endl;
    
            string latitude_str=
               stringfunc::erase_chars_before_first_substring(
                  longlatstr,"n");
            latitude_str=stringfunc::find_and_replace_char(
               latitude_str,"n"," ");
//            cout << "latitude_str = " << latitude_str << endl;
            double latitude=stringfunc::string_to_number(latitude_str);
//            cout << "latitude = " << latitude << endl;
            twovector longlat(longitude,latitude);
//            cout << "longlat = " << longlat << endl;

            ColorGeodeVisitor_ptr->insert_long_lat_ID_map_entry(longlat,ID);
            ID++;
         }
      }
   } // loop over index i labeling candidate geotif filenames
   
//   cout << "long_lat_ID_map.size() = "
//        << ColorGeodeVisitor_ptr->get_long_lat_ID_map_size() << endl;
//   ColorGeodeVisitor_ptr->print_long_lat_ID_map_contents();
   
   return geotif_filenames;
}

// ---------------------------------------------------------------------
// Member function mark_current_FOV_midpoint()

void LOSMODELSGROUP::mark_current_FOV_midpoint()
{   
   cout << "inside LOSMODELSGROUP::mark_current_FOV_midpoint()" << endl;

   MODEL* Aircraft_MODEL_ptr=get_MODEL_ptr(0);
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=Aircraft_MODEL_ptr->
      get_OBSFRUSTAGROUP_ptr();
//   OBSFRUSTUM* virtual_OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
//      get_virtual_OBSFRUSTUM_ptr();
//   cout << "virtual_OBSFRUSTUM_ptr = " << virtual_OBSFRUSTUM_ptr << endl;

   camera* virtual_camera_ptr=OBSFRUSTAGROUP_ptr->get_virtual_camera_ptr();
//   cout << "virtual_camera_ptr = " << virtual_camera_ptr << endl;
   vector<threevector> UV_corner_ray=virtual_camera_ptr-> 
      get_UV_corner_world_ray();
   threevector center_rhat(Zero_vector);
   for (unsigned int c=0; c<UV_corner_ray.size(); c++)
   {
      center_rhat += UV_corner_ray[c];
   }
   center_rhat=center_rhat.unitvector();
   cout << "center_rhat = " << center_rhat << endl;

   mark_ground_intercept(
      virtual_camera_ptr->get_world_posn(),center_rhat);
}

// ---------------------------------------------------------------------
// Member function mark_ground_intercept() takes in an aerial camera
// position as well as camera pointing vector r_hat.  It computes the
// SRTM ground intercept and marks the location with a new
// GroundTarget SignPost.  

void LOSMODELSGROUP::mark_ground_intercept(
   const threevector& aerial_camera_posn,const threevector& r_hat,
   string label)
{   
   cout << "inside LOSMODELSGROUP::mark_ground_intercept()" << endl;

   geopoint aerial_point(
      get_pass_ptr()->get_northern_hemisphere_flag(),
      get_pass_ptr()->get_UTM_zonenumber(),aerial_camera_posn);
//   cout << "aerial_point = " << aerial_point << endl;

   geopoint ground_intercept;
   if (TilesGroup_ptr->estimate_SRTM_ground_intercept_given_aerial_pt_and_ray(
      aerial_point,r_hat,ground_intercept))
   {
//      cout << "ground_intercept = " << ground_intercept << endl;

      double common_SignPost_size=GroundTarget_SignPostsGroup_ptr->
         get_common_geometrical_size();
      cout << "common_SignPost_size = " << common_SignPost_size << endl;
      
      SignPost* SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
         generate_new_SignPost(20,1);
//         generate_new_SignPost(common_SignPost_size,1);

      if (label.size()==0)
      {
         int SignPost_ID=SignPost_ptr->get_ID();
         label=stringfunc::number_to_string(SignPost_ID);
      }
      SignPost_ptr->set_label(label);

      SignPost_ptr->set_UVW_coords(
         GroundTarget_SignPostsGroup_ptr->get_curr_t(),
         GroundTarget_SignPostsGroup_ptr->get_passnumber(),
         ground_intercept.get_UTM_posn());

      GroundTarget_SignPostsGroup_ptr->set_selected_Graphical_ID(
         SignPost_ptr->get_ID());
   }
}

// ---------------------------------------------------------------------
// Member function export_average_occlusion_files() generates geotif &
// nitf files containing time-averaged ROI visibility results.

void LOSMODELSGROUP::export_average_occlusion_files(
   string dirname,string basename,
   string output_geotif_filename,string output_nitf_filename)
{
   
// Recall *ground_bbox_ptr has lower left and upper right coordinates
// specified in UTM rather than lon-lat geocoords:

   bounding_box* ground_bbox_ptr=get_ground_bbox_ptr();
   double min_easting=ground_bbox_ptr->get_xmin();
   double min_northing=ground_bbox_ptr->get_ymin();
   double max_easting=ground_bbox_ptr->get_xmax();
   double max_northing=ground_bbox_ptr->get_ymax();

   int UTM_zonenumber=TilesGroup_ptr->get_specified_UTM_zonenumber();
   bool northern_hemisphere_flag=TilesGroup_ptr->
      get_northern_hemisphere_flag();

   geopoint lower_left_corner(
      northern_hemisphere_flag,UTM_zonenumber,min_easting,min_northing);
   geopoint upper_right_corner(
      northern_hemisphere_flag,UTM_zonenumber,max_easting,max_northing);
   double min_longitude=lower_left_corner.get_longitude();
   double max_longitude=upper_right_corner.get_longitude();
   double min_latitude=lower_left_corner.get_latitude();
   double max_latitude=upper_right_corner.get_latitude();

   min_longitude=basic_math::mytruncate(min_longitude);
   max_longitude=basic_math::mytruncate(max_longitude);
   min_latitude=basic_math::mytruncate(min_latitude);
   max_latitude=basic_math::mytruncate(max_latitude);

   cout << "min_lon = " << min_longitude 
        << " max_lon+1 = " << max_longitude+1 << endl;
   cout << "min_lat = " << min_latitude
        << " max_lat+1 = " << max_latitude+1 << endl;

   string avg_LOS_lonlat_tif_filename=
      TilesGroup_ptr->export_avg_ground_bbox_LOS(
         min_longitude,max_longitude,min_latitude,max_latitude);

// Write averaged LOS raster image to Geotif and NITF output files in
// webapps subdir:

   string output_dirname="/usr/local/apache-tomcat/webapps/LOST/data/"+
      dirname;
   cout << "output_dirname = " << output_dirname << endl;
   filefunc::dircreate(output_dirname);
   string geotif_imagePath=output_dirname+basename+".tif";
   string nitf_imagePath=output_dirname+basename+".ntif";

   bool tif_exists_flag=filefunc::fileexist(avg_LOS_lonlat_tif_filename);
   cout << "tif_exists_flag = " << tif_exists_flag << endl;

   string unix_cmd="cp "+avg_LOS_lonlat_tif_filename+" "+geotif_imagePath;
   sysfunc::unix_command(unix_cmd);

   bool geotif_exists_flag=false;
   while (!geotif_exists_flag)
   {
      cout << "Waiting for geotif_imagePath = " << geotif_imagePath << endl;
      geotif_exists_flag=filefunc::fileexist(geotif_imagePath);
   }

   unix_cmd="gdal_translate -of NITF "+geotif_imagePath+" "+
      nitf_imagePath;
   sysfunc::unix_command(unix_cmd);

   bool nitf_exists_flag=false;
   while (!nitf_exists_flag)
   {
      cout << "Waiting for nitf_imagePath = " << nitf_imagePath << endl;
      nitf_exists_flag=filefunc::fileexist(nitf_imagePath);
   }

// Copy Geotif & NITF  output files to Desktop subdir:

   unix_cmd="cp "+geotif_imagePath+" "+output_geotif_filename;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="cp "+nitf_imagePath+" "+output_nitf_filename;
   sysfunc::unix_command(unix_cmd);
}

// ==========================================================================
// Visibility skymap member functions
// ==========================================================================

// Member function generate_target_visibility_skymaps() first
// instantiates *skymap_twoDarray_ptr to hold ground target visibility
// probabilities for cells within a 2D region of the sky.  It loads in
// terrain height data from geotif tiles surrounding the ground
// targets by at least two degs in latitude and longitude.  This
// method then loops over aircraft headings and computes separate
// skymaps for each angular heading.  Skymaps are written to output
// PNG files.  If no ground targets exist, this boolean method returns
// false.

bool LOSMODELSGROUP::generate_target_visibility_skymaps(
      double lower_left_longitude,double lower_left_latitude,
      double upper_right_longitude,double upper_right_latitude)
{   
//   cout << "inside LOSMODELSGROUP::generate_target_visibility_skymaps()" 
//        << endl;

   double flowfield_progress=0.02;
   string progress_type="flowfield computation";
   viewer_Messenger_ptr->broadcast_progress(flowfield_progress,progress_type);

// First compute skymap bounding box corners in UTM coordinates:

   int UTM_zone=get_EarthRegionsGroup_ptr()->get_specified_UTM_zonenumber();
   geopoint lower_left_corner( 
      lower_left_longitude,lower_left_latitude,0,UTM_zone);
   geopoint upper_right_corner(
      upper_right_longitude,upper_right_latitude,0,UTM_zone);
//   cout << "lower_left corner = " << lower_left_corner << endl;
//   cout << "upper_right_corner = " << upper_right_corner << endl;

   vector<twovector> target_posns;
   GroundTarget_SignPostsGroup_ptr->get_ground_target_posns(target_posns);
   unsigned int n_ground_targets=target_posns.size();
//   cout << "n_ground_targets = " << n_ground_targets << endl;
   if (n_ground_targets==0) return false;

   twoDarray *skymap_twoDarray_ptr,*skymap_Xsum_twoDarray_ptr,
      *skymap_Ysum_twoDarray_ptr;
   initialize_skymaps(
      n_ground_targets,lower_left_corner,upper_right_corner,
      skymap_twoDarray_ptr,skymap_Xsum_twoDarray_ptr,
      skymap_Ysum_twoDarray_ptr);
   twoDarray* skymap_phase_twoDarray_ptr=NULL;

   MODEL* MODEL_ptr=generate_LiMIT_MODEL();

   int OBSFRUSTUM_ID=0;
   OBSFRUSTUM* OBSFRUSTUM_ptr=MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->
      get_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);
   double alpha,beta;
   OBSFRUSTUMfunc::convert_FOVs_to_alpha_beta_angles(
      OBSFRUSTUM_ptr->get_az_extent(),OBSFRUSTUM_ptr->get_el_extent(),
      alpha,beta);

// Load all height data needed for entire skymap computation:

   compute_skymap_flag=true;
   twoDarray* DTED_ztwoDarray_ptr=NULL;
   twoDarray* reduced_DTED_ztwoDarray_ptr=NULL;
   twoDarray* DTED_ptwoDarray_ptr=NULL; 
   
   load_heightfields(
      target_posns,DTED_ztwoDarray_ptr,
      reduced_DTED_ztwoDarray_ptr,DTED_ptwoDarray_ptr);
   max_ground_Z=DTED_ztwoDarray_ptr->maximum_value();
//   cout << "max_ground_Z = " << max_ground_Z << endl;

// Loop over multiple azimuthal headings for aircraft:
   
   double theta_start=0;
   double theta_stop=360;
   unsigned int n_theta_bins=8;
   double d_theta=(theta_stop-theta_start)/n_theta_bins;

   double ds;
   if (ladar_height_data_flag)
   {
      ds=0.2;	   // meter
   }
   else
   {
      ds=0.25*get_raytrace_cellsize();
   }

   bool cancel_computation_flag=false;
   unsigned int n_OBSFRUSTA=MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->
      get_n_Graphicals();
   for (unsigned int t=0; t<n_theta_bins && !cancel_computation_flag; t++)
   {

      flowfield_progress += 1.0/double(n_theta_bins+1);
      flowfield_progress=basic_math::min(flowfield_progress,1.0);
      double rounded_flowfield_progress=
         0.01*basic_math::round(100*flowfield_progress);
      cout << "rounded_flowfield_progress = " << rounded_flowfield_progress
           << endl;
      viewer_Messenger_ptr->broadcast_progress(
         rounded_flowfield_progress,progress_type);

      double theta_deg=t*d_theta;
      cout << "theta = " << theta_deg << endl;
//      cin >> theta_deg;
//      cout << "theta_deg = " << theta_deg << endl;
      double theta=theta_deg*PI/180;
      threevector v_hat(cos(theta),sin(theta),0);

      for (unsigned int px=0; px<skymap_twoDarray_ptr->get_mdim(); px++)
      {
         cout << px << " " << flush;

// Check if cancel skymap generation message has been received via
// ActiveMQ:
      
         string cancel_msg=
            cancel_messenger_ptr->check_for_cancel_operation_message();
//            cout << " cancel_msg = " << cancel_msg << endl;
         if (cancel_msg=="flowfield computation")
         {
            clear_visibility_skymaps();
            cancel_messenger_ptr->clear_cancelled_operation();
            cancel_computation_flag=true;
            break;
         }

         double x=skymap_twoDarray_ptr->fast_px_to_x(px)+
            get_grid_world_origin().get(0);
         for (unsigned int py=0; py<skymap_twoDarray_ptr->get_ndim(); py++)
         {
            double y=skymap_twoDarray_ptr->fast_py_to_y(py)
               +get_grid_world_origin().get(1);
            threevector posn(x,y,aircraft_altitude);

            int n_visible_targets=0;
            vector<pair<int,threevector> > target_tracing_result;
            for (unsigned int id=0; id<n_OBSFRUSTA; id++)
            {
               OBSFRUSTUM* OBSFRUSTUM_ptr=MODEL_ptr->
                  compute_dynamic_OBSFRUSTUM(
                     get_curr_t(),get_passnumber(),posn,v_hat,alpha,beta,
                     MODEL_ptr->get_OBSFRUSTUM_z_base_face(0),id);
               n_visible_targets += OBSFRUSTUM_ptr->
                  raytrace_ground_targets(
                     target_posns,max_ground_Z,
                     max_raytrace_range,min_raytrace_range,ds,
                     DTED_ztwoDarray_ptr,DTED_ptwoDarray_ptr,
                     reduced_DTED_ztwoDarray_ptr,target_tracing_result);
            } // loop over index id labeling OBSFRUSTA

//            double visibility_frac=1.0;
//            cout << "visibility_frac = " << visibility_frac << endl;
            double visibility_frac=double(n_visible_targets)/
               target_posns.size();
            skymap_twoDarray_ptr->put(px,py,visibility_frac);

// Save tracing results for individual ground targets within STL map 
// target_skymap_map_ptr:

            for (unsigned int g=0; g<n_ground_targets; g++)
            {
               twovector az_tgt_ID(t,g);
               (*target_skymap_map_ptr)[az_tgt_ID]->put(
                  px,py,target_tracing_result[g].first);
            } // loop over index g labeling ground targets

         } // loop over skymap's py index
      } // loop over skymap's px index
      cout << endl;

//   cout << "*skymap_twoDarray_ptr = " << *skymap_twoDarray_ptr << endl;
//   cout << "skymap_twoDarray_ptr->minimum_value() = "
//        << skymap_twoDarray_ptr->minimum_value() << endl;
//   cout << "skymap_twoDarray_ptr->maximum_value() = "
//        << skymap_twoDarray_ptr->maximum_value() << endl;
   
      write_out_skymap_text_files(theta_deg,skymap_twoDarray_ptr);

      write_out_skymap_PNG_files(
         MODEL_ptr,theta_deg,skymap_twoDarray_ptr,skymap_phase_twoDarray_ptr);

//      export_skymap(target_posns,theta,skymap_twoDarray_ptr);
      accumulate_skymap_flowfield(
         theta,skymap_twoDarray_ptr,
         skymap_Xsum_twoDarray_ptr,skymap_Ysum_twoDarray_ptr);
   } // loop over index t labeling azimuthal angle theta

   if (!cancel_computation_flag)
   {
      write_out_individual_target_skymap_text_files();

//   delete DTED_ztwoDarray_ptr;
//   TilesGroup_ptr->set_DTED_ztwoDarray_ptr(NULL);
//   delete reduced_DTED_ztwoDarray_ptr;

      skymap_phase_twoDarray_ptr=new twoDarray(skymap_twoDarray_ptr);
      skymap_phase_twoDarray_ptr->clear_values();
      compute_average_skymap_flowfield(
         skymap_Xsum_twoDarray_ptr,skymap_Ysum_twoDarray_ptr,
         skymap_twoDarray_ptr,skymap_phase_twoDarray_ptr);

      write_out_ground_target_posns();
      write_out_skymap_PNG_files(
         MODEL_ptr,POSITIVEINFINITY,skymap_twoDarray_ptr,
         skymap_phase_twoDarray_ptr);

      export_flowfield_geocoords();
      viewer_Messenger_ptr->broadcast_finished_progress(progress_type);
   } // !cancel_computation_flag conditional

   delete DTED_ptwoDarray_ptr;
   destroy_MODEL(MODEL_ptr);

   delete skymap_twoDarray_ptr;
   delete skymap_Xsum_twoDarray_ptr;
   delete skymap_Ysum_twoDarray_ptr;
   delete skymap_phase_twoDarray_ptr;

   compute_skymap_flag=false;

   return true;
}

// ---------------------------------------------------------------------
// Member function initialize_skymaps() instantiates skymap twoDarrays
// whose geographic size is set by input lower_left_corner and
// upper_right_corner geopoints.

void LOSMODELSGROUP::initialize_skymaps(
   unsigned int n_ground_targets,
   const geopoint& lower_left_corner,const geopoint& upper_right_corner,
   twoDarray*& skymap_twoDarray_ptr,twoDarray*& skymap_Xsum_twoDarray_ptr,
   twoDarray*& skymap_Ysum_twoDarray_ptr)
{   
//   cout << "inside LOSMODELSGROUP::initialize_skymaps()" << endl;

// Instantiate skymap twoDarrays' borders relative to grid world
// origin:

   skymap_xlo=lower_left_corner.get_UTM_easting()
      -get_grid_world_origin().get(0);
   skymap_xhi=upper_right_corner.get_UTM_easting()
      -get_grid_world_origin().get(0);
   skymap_ylo=lower_left_corner.get_UTM_northing()
      -get_grid_world_origin().get(1);
   skymap_yhi=upper_right_corner.get_UTM_northing()
      -get_grid_world_origin().get(1);

//   int mdim=3;			// debugging only
//   int ndim=3;			// debugging only
//   int mdim=11;		
//   int ndim=11;		
//   int mdim=15;		// demo value
//   int ndim=15;		// demo value
//   int mdim=19;
//   int ndim=19;			// 2 mins on touchy2

   unsigned int mdim=21;		// true value
   unsigned int ndim=21;		// true value

//   int mdim=25;		// 3.5 mins on touchy2
//   int ndim=25;
//   int mdim=31;	// 5.5 mins on touchy2
//   int ndim=31;

   double delta_x=(skymap_xhi-skymap_xlo)/(mdim-1);
   double delta_y=(skymap_yhi-skymap_ylo)/(ndim-1);
//   cout << "delta_x = " << delta_x << " delta_y = " << delta_y << endl;
   skymap_ds=basic_math::min(delta_x,delta_y);
   cout << "skymap_ds = " << skymap_ds << endl;

   skymap_twoDarray_ptr=new twoDarray(mdim,ndim);
   skymap_twoDarray_ptr->init_coord_system(
      skymap_xlo,skymap_xhi,skymap_ylo,skymap_yhi);
//   cout << "*skymap_twoDarray_ptr = " << *skymap_twoDarray_ptr << endl;

   skymap_Xsum_twoDarray_ptr=new twoDarray(skymap_twoDarray_ptr);
   skymap_Ysum_twoDarray_ptr=new twoDarray(skymap_twoDarray_ptr);
   skymap_Xsum_twoDarray_ptr->clear_values();
   skymap_Ysum_twoDarray_ptr->clear_values();

// Clear any prior contents within member
// target_skymap_map_ptr before adding new skymaps into this STL map:

   for (TARGET_SKYMAP_MAP::iterator iter=target_skymap_map_ptr->begin();
        iter != target_skymap_map_ptr->end(); iter++)
   {
      twoDarray* curr_target_skymap_twoDarray_ptr=iter->second;
      if (curr_target_skymap_twoDarray_ptr != NULL)
         delete curr_target_skymap_twoDarray_ptr;
   }
   target_skymap_map_ptr->clear();

   skymap_azlo=0;
   skymap_azhi=360;
   skymap_daz=45;
   unsigned int n_az_bins=(skymap_azhi-skymap_azlo)/skymap_daz;

   for (unsigned int t=0; t<n_az_bins; t++)
   {
      for (unsigned int g=0; g<n_ground_targets; g++)
      {
         twoDarray* curr_twoDarray_ptr=new twoDarray(skymap_twoDarray_ptr);
         curr_twoDarray_ptr->clear_values();
         twovector az_tgt_ID(t,g);
         (*target_skymap_map_ptr)[az_tgt_ID]=curr_twoDarray_ptr;
      } // loop over index g labeling ground targets         
   } // loop over index t labeling azimuthal heading angles

}

// ---------------------------------------------------------------------
// Member function load_heightfields() first instantiates
// *DTED_ztwoDarray_ptr to be sufficiently large to hold all height
// data needed for skymap generation purposes.  It then loads DTED
// tile height data into *DTED_ztwoDarray_ptr.  This method also
// instantiates and fills *reduced_DTED_ztwoDarray_ptr in order to
// speed up raytracing computations.  Finally, this method
// instantiates *DTED_ptwoDarray_ptr to hold angularly visible zone
// information.

void LOSMODELSGROUP::load_heightfields(
   const vector<twovector>& target_posns,
   twoDarray*& DTED_ztwoDarray_ptr,twoDarray*& reduced_DTED_ztwoDarray_ptr,
   twoDarray*& DTED_ptwoDarray_ptr)
{   
//   cout << "inside LOSMODELSGROUP::load_heightfields()" << endl;

   int n_extra_degs=2;
//   delete DTED_ztwoDarray_ptr;

   if (ladar_height_data_flag)
   {
      DTED_ztwoDarray_ptr=TilesGroup_ptr->
         load_ladar_height_data_into_ztwoDarray();
   }
   else
   {
      DTED_ztwoDarray_ptr=TilesGroup_ptr->load_all_DTED_tiles(
         n_extra_degs,get_raytrace_cellsize(),target_posns);
   }

   reduced_DTED_ztwoDarray_ptr=TilesGroup_ptr->
      generate_reduced_DTED_ztwoDarray();

   delete DTED_ptwoDarray_ptr;
   DTED_ptwoDarray_ptr=new twoDarray(DTED_ztwoDarray_ptr);
   DTED_ptwoDarray_ptr->initialize_values(-1);
}

// ---------------------------------------------------------------------
void LOSMODELSGROUP::load_heightfields(
   const geopoint& lower_left_corner,const geopoint& upper_right_corner,
   twoDarray*& DTED_ztwoDarray_ptr,twoDarray*& reduced_DTED_ztwoDarray_ptr,
   twoDarray*& DTED_ptwoDarray_ptr)
{   
//   cout << "inside LOSMODELSGROUP::load_heightfields()" << endl;

   int n_extra_degs=2;
//   delete DTED_ztwoDarray_ptr;

   if (ladar_height_data_flag)
   {
      DTED_ztwoDarray_ptr=TilesGroup_ptr->
         load_ladar_height_data_into_ztwoDarray();
   }
   else
   {
      DTED_ztwoDarray_ptr=TilesGroup_ptr->load_all_DTED_tiles(
         n_extra_degs,get_raytrace_cellsize(),
         lower_left_corner,upper_right_corner);
   }

   reduced_DTED_ztwoDarray_ptr=TilesGroup_ptr->
      generate_reduced_DTED_ztwoDarray();

   delete DTED_ptwoDarray_ptr;
   DTED_ptwoDarray_ptr=new twoDarray(DTED_ztwoDarray_ptr);
   DTED_ptwoDarray_ptr->initialize_values(-1);
}

// ---------------------------------------------------------------------
// Member function write_out_ground_target_posns() exports ground
// target UTM coordinates to output text files.  We wrote this method
// in Sept 2009 in response to a request from Kenneth King Ho Lee.

void LOSMODELSGROUP::write_out_ground_target_posns()
{
//   cout << "inside LOSMODELSGROUP::write_out_ground_target_posns()" << endl;
   
   string ground_targets_filename=webapps_outputs_subdir+"ground_targets.txt";

   ofstream outstream;
   outstream.precision(12);
   filefunc::openfile(ground_targets_filename,outstream);
   for (unsigned int t=0; t<GroundTarget_SignPostsGroup_ptr->
           get_n_Graphicals(); t++)
   {
      SignPost* SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
         get_SignPost_ptr(t);
      
      threevector SignPost_posn;
      SignPost_ptr->get_UVW_coords(
         GroundTarget_SignPostsGroup_ptr->get_curr_t(),
         GroundTarget_SignPostsGroup_ptr->get_passnumber(),SignPost_posn);
      outstream << SignPost_ptr->get_ID() << "  "
                << SignPost_posn.get(0) << "  "
                << SignPost_posn.get(1) << endl;
   }
   filefunc::closefile(ground_targets_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function write_out_skymap_text_files() exports the current
// skymap within *skymap_twoDarray_ptr to an output text file.

void LOSMODELSGROUP::write_out_skymap_text_files(
   double theta_deg,twoDarray* skymap_twoDarray_ptr)
{
//   cout << "inside LOSMODELSGROUP::write_out_skymap_text_files()" << endl;
   
// Write out skymap visibility as a function of input theta_deg:

   string skymap_filename=webapps_outputs_subdir+
      "skymap_"+stringfunc::number_to_string(theta_deg)+".txt";
   ofstream outstream;
   outstream.precision(12);
   filefunc::openfile(skymap_filename,outstream);

   for (unsigned int px=0; px<skymap_twoDarray_ptr->get_mdim(); px++)
   {
      cout << px << " " << flush;
      double x=skymap_twoDarray_ptr->fast_px_to_x(px)+
         get_grid_world_origin().get(0);
      for (unsigned int py=0; py<skymap_twoDarray_ptr->get_ndim(); py++)
      {
         double y=skymap_twoDarray_ptr->fast_py_to_y(py)
            +get_grid_world_origin().get(1);
         double prob=skymap_twoDarray_ptr->get(px,py);
         outstream << x << "  " << y << "  " << prob << endl;
      } // loop over py index
      outstream << endl;
   } // loop over px index

   filefunc::closefile(skymap_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function write_out_individual_target_skymap_text_files()
// iterates over all skymaps within *target_skymap_map_ptr and exports
// each one to an output text file.

void LOSMODELSGROUP::write_out_individual_target_skymap_text_files()
{
//   cout << "inside LOSMODELSGROUP::write_out_individual_target_skymap_text_files()" << endl;
   
   for (TARGET_SKYMAP_MAP::iterator iter=target_skymap_map_ptr->begin();
        iter != target_skymap_map_ptr->end(); iter++)
   {
      twovector az_tgt_ID=iter->first;
      double theta_deg=skymap_daz*az_tgt_ID.get(0);
      int g=az_tgt_ID.get(1);

      string skymap_filename=webapps_outputs_subdir+
         "skymap_"+stringfunc::number_to_string(theta_deg)
         +"_tgt_"+stringfunc::number_to_string(g)+".txt";
      ofstream outstream;
      outstream.precision(12);
      filefunc::openfile(skymap_filename,outstream);

      twoDarray* curr_twoDarray_ptr=iter->second;

      for (unsigned int px=0; px<curr_twoDarray_ptr->get_mdim(); px++)
      {
         cout << px << " " << flush;
         double x=curr_twoDarray_ptr->fast_px_to_x(px)+
            get_grid_world_origin().get(0);
         for (unsigned int py=0; py<curr_twoDarray_ptr->get_ndim(); py++)
         {
            double y=curr_twoDarray_ptr->fast_py_to_y(py)
               +get_grid_world_origin().get(1);
            double visibility_flag=curr_twoDarray_ptr->get(px,py);

// Recall visibility_flag = -1 --> Target lies outside sensor's field-of-view
// 	  visibility_flag = 0  --> Target is occluded from sensor's view
//	  visibility_flag = 1  --> Target is visibile to sensor
            
            outstream << x << "  " << y << "  " << visibility_flag << endl;
         } // loop over py index
         outstream << endl;
      } // loop over px index

      filefunc::closefile(skymap_filename,outstream);

   } // loop over target_skymap_map iterator
}

// ---------------------------------------------------------------------
// Member function write_out_skymap_PNG_files() exports the current
// skymap within *skymap_twoDarray_ptr to an output PNG file.

void LOSMODELSGROUP::write_out_skymap_PNG_files(
   MODEL* MODEL_ptr,double theta_deg,twoDarray* skymap_twoDarray_ptr,
   twoDarray* skymap_phase_twoDarray_ptr)
{
//   cout << "inside LOSMODELSGROUP::write_out_skymap_PNG_files()" << endl;

   if (EarthRegionsGroup_ptr == NULL) return;

// Reset Movie's texture 3D coordinates to coincide with skymap's
// corners in UTM coordinates:

   Movie* Movie_ptr=MODEL_ptr->get_OBSFRUSTAGROUP_ptr()->
      get_MoviesGroup_ptr()->get_Movie_ptr(0);

   threevector bottom_left(
      skymap_twoDarray_ptr->get_xlo(),
      skymap_twoDarray_ptr->get_ylo(),aircraft_altitude);
   threevector bottom_right(
      skymap_twoDarray_ptr->get_xhi(),
      skymap_twoDarray_ptr->get_ylo(),aircraft_altitude);
   threevector top_right(
      skymap_twoDarray_ptr->get_xhi(),
      skymap_twoDarray_ptr->get_yhi(),aircraft_altitude);
   threevector top_left(
      skymap_twoDarray_ptr->get_xlo(),
      skymap_twoDarray_ptr->get_yhi(),aircraft_altitude);
   Movie_ptr->reset_geom_vertices(
      bottom_right,bottom_left,top_left,top_right);

/*
  bool northern_hemisphere_flag=true;
  int specified_UTM_zonenumber=42;
  geopoint lower_left_corner(
  northern_hemisphere_flag,specified_UTM_zonenumber,
  bottom_left.get(0),bottom_left.get(1));
  geopoint upper_right_corner(
  northern_hemisphere_flag,specified_UTM_zonenumber,
  top_right.get(0),top_right.get(1));
  cout << "lower_left_corner = " << lower_left_corner << endl;
  cout << "upper_right_corner = " << upper_right_corner << endl;
*/

// Recall twoDarray values written to output PNG files are assumed to
// range from 0 to 1.  So renormalize skymap magnitudes by their
// maximal value:

   double max_value=skymap_twoDarray_ptr->maximum_value();
//   cout << "max_value = " << max_value << endl;
   if (nearly_equal(max_value,0)) max_value=1;
   (*skymap_twoDarray_ptr) /= max_value;

// Copy contents from *skymap_twoDarray_ptr into
// *texture_rectangle_ptr's unsigned char image array:

   string movie_filename="twodarray";
   texture_rectangle* texture_rectangle_ptr=Movie_ptr->
      get_texture_rectangle_ptr();
   texture_rectangle_ptr->initialize_twoDarray_image(skymap_twoDarray_ptr);
   texture_rectangle_ptr->set_image();
//   const double output_alpha=1.0;
//   texture_rectangle_ptr->convert_greyscale_image_to_hue_colored(
//      output_alpha);

// Write out skymap magnitude field:

   string skymap_filename=webapps_outputs_subdir+
      "skymap_"+stringfunc::number_to_string(theta_deg)+".png";
   if (skymap_phase_twoDarray_ptr != NULL)
   {
      skymap_filename=webapps_outputs_subdir+"skymap_magnitude.png";
   }
//   cout << "skymap_filename = " << skymap_filename << endl;
   texture_rectangle_ptr->write_curr_frame(skymap_filename);
   
// Write out cumulative skymap phase field:

   if (skymap_phase_twoDarray_ptr != NULL)
   {
      if (ArrowsGroup_ptr != NULL)
      {
         ArrowsGroup_ptr->destroy_all_Arrows();

// Note added on 6/27/12: After changing arrowhead_size_prefactor from
// 1 to 0.001, LOST skymap flowfield arrows look OK again:

         int mdim=skymap_twoDarray_ptr->get_mdim();

//         double arrowhead_size_prefactor=1;
//         double arrowhead_size_prefactor=0.003;
         double arrowhead_size_prefactor=0.001*mdim/3.0;

         if (ladar_height_data_flag)
         {
            arrowhead_size_prefactor=0.0001;
         }

         ArrowsGroup_ptr->generate_flow_field(
            skymap_twoDarray_ptr,skymap_phase_twoDarray_ptr,
            aircraft_altitude,arrowhead_size_prefactor);
      }
      
// Recall twoDarray values written to output PNG files are assumed to
// range from 0 to 1.  So renormalize skymap phases by their maximal
// value:

      skymap_filename=webapps_outputs_subdir+"skymap_phase.png";
      double max_phase=skymap_phase_twoDarray_ptr->maximum_value();
      (*skymap_phase_twoDarray_ptr) /= max_phase;

      texture_rectangle_ptr->initialize_twoDarray_image(
         skymap_phase_twoDarray_ptr);
      texture_rectangle_ptr->set_image();
      texture_rectangle_ptr->write_curr_frame(skymap_filename);
   }
}

// ---------------------------------------------------------------------
// Member function export_skymap() is a specialized method requested
// by Michael Yee for his flight path planning purposes.  Michael
// requested that we generate a comma-separated-value text file
// containing skymap score value as a function of skymap location and
// aircraft heading.  

void LOSMODELSGROUP::export_skymap(
   const vector<twovector>& target_posns,double theta,
   twoDarray* skymap_twoDarray_ptr)
{
//   cout << "inside LOSMODELSGROUP::export_skymap()" << endl;
//   cout << "theta = " << theta*180/PI << endl;

   string hemisphere_string="south";
   if (get_EarthRegionsGroup_ptr()->get_northern_hemisphere_flag())
   {
      hemisphere_string="north";
   }
   int UTM_zone=get_EarthRegionsGroup_ptr()->get_specified_UTM_zonenumber();
   
   ofstream outstream;
   outstream.precision(12);
   string skymap_filename="skymap.txt";

   if (nearly_equal(theta,0))
   {
      filefunc::openfile(skymap_filename,outstream);
      for (unsigned int t=0; t<target_posns.size(); t++)
      {
         outstream << "# Ground target " << t << " : Easting = "
                   << target_posns[t].get(0) << " , Northing = "
                   << target_posns[t].get(1) << endl;
      } // loop over index t labeling ground targets
      outstream << endl;
      outstream << 
         "# Skymap Easting , Northing , hemisphere flag , UTM zonenumber" << endl;
      outstream << "#   Aircraft heading rel to east (degs) , Frac observable ground targets"
                << endl;
      outstream << endl;
   }
   else
   {
      filefunc::appendfile(skymap_filename,outstream);
   }

   for (unsigned int py=0; py<skymap_twoDarray_ptr->get_ndim(); py++)
   {
      double y=skymap_twoDarray_ptr->fast_py_to_y(py);
      for (unsigned int px=0; px<skymap_twoDarray_ptr->get_mdim(); px++)
      {
         double x=skymap_twoDarray_ptr->fast_px_to_x(px);

//         cout << "px = " << px << " py = " << py << endl;
         threevector posn(x,y,aircraft_altitude);
         double p=skymap_twoDarray_ptr->get(px,py);

         outstream << x << " , " 
                   << y << " , " 
                   << hemisphere_string << " , "
                   << UTM_zone << " , "
                   << theta*180/PI << " , "
                   << p << endl;
      } // loop over px index
   } // loop over py index

   outstream << endl;
      
   filefunc::closefile(skymap_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function accumulate_skymap_flowfield() takes in the
// aircraft's current heading angle theta along with the current
// ground target visibility field within *skymap_twoDarray_ptr.  For
// each cell within the skymap, this method computes the X and Y
// coordinates of the current visibility probability flow and adds
// them to *skymap_X[Y]sum_twoDarray_ptr.

void LOSMODELSGROUP::accumulate_skymap_flowfield(
   double theta,twoDarray* skymap_twoDarray_ptr,
   twoDarray* skymap_Xsum_twoDarray_ptr,twoDarray* skymap_Ysum_twoDarray_ptr)
{   
//   cout << "inside LOSMODELSGROUP::accumulate_skymap_flowfield()" << endl;
   
   for (unsigned int py=0; py<skymap_twoDarray_ptr->get_ndim(); py++)
   {
      for (unsigned int px=0; px<skymap_twoDarray_ptr->get_mdim(); px++)
      {
//         cout << "px = " << px << " py = " << py << endl;
         double curr_magnitude=skymap_twoDarray_ptr->get(px,py);
         if (curr_magnitude > 0.5*NEGATIVEINFINITY)
         {
            double curr_X=skymap_Xsum_twoDarray_ptr->get(px,py);
            double curr_Y=skymap_Ysum_twoDarray_ptr->get(px,py);
            curr_X += curr_magnitude*cos(theta);
            curr_Y += curr_magnitude*sin(theta);
            skymap_Xsum_twoDarray_ptr->put(px,py,curr_X);
            skymap_Ysum_twoDarray_ptr->put(px,py,curr_Y);
         }
      } // loop over px index
   } // loop over py index
}

// ---------------------------------------------------------------------
// Member function compute_average_skymap_flowfield() takes in
// aggregated skymap flowfield X and Y components.  It converts them
// to magnitude and phase in each sky cell and returns the results in
// *skymap_twoDarray_ptr and *skymap_phase_twoDarray_ptr.  Phase
// angles range from 0 to 360 degrees.

void LOSMODELSGROUP::compute_average_skymap_flowfield(
   twoDarray* skymap_Xsum_twoDarray_ptr,twoDarray* skymap_Ysum_twoDarray_ptr,
   twoDarray* skymap_twoDarray_ptr,twoDarray* skymap_phase_twoDarray_ptr)
{   
//   cout << "inside LOSMODELSGROUP::compute_average_skymap_flowfield()" << endl;
   
   for (unsigned int py=0; py<skymap_twoDarray_ptr->get_ndim(); py++)
   {
      for (unsigned int px=0; px<skymap_twoDarray_ptr->get_mdim(); px++)
      {
//         cout << "px = " << px << " py = " << py << endl;
         double curr_X=skymap_Xsum_twoDarray_ptr->get(px,py);
         double curr_Y=skymap_Ysum_twoDarray_ptr->get(px,py);
         double magnitude=sqrt(sqr(curr_X)+sqr(curr_Y));
         skymap_twoDarray_ptr->put(px,py,magnitude);

         if (!nearly_equal(magnitude,0))
         {
            double phase=atan2(curr_Y,curr_X)*180/PI;
            phase=basic_math::phase_to_canonical_interval(phase,0,360);
            skymap_phase_twoDarray_ptr->put(px,py,phase);
         }
         
      } // loop over px index
   } // loop over py index
}

// ---------------------------------------------------------------------
// Member function export_flowfield_geocoords() loops over all
// non-zero magnitude Arrow members of *ArrowsGroup_ptr.  It writes to
// output text file the longitude,latitude and UTM geocoordinates for
// arrow bases and tips.  We wrote this method in Sep 2009 for
// thin-client display and MIT path computation purposes in the LOST
// project.

void LOSMODELSGROUP::export_flowfield_geocoords()
{   
//   cout << "inside LOSMODELSGROUP::export_flowfield_geocoords()" << endl;

   string longlat_flowfield_filename=
      webapps_outputs_subdir+"flowfield_long_lat.txt";
   string UTM_flowfield_filename=
      webapps_outputs_subdir+"flowfield_UTM.txt";
   ofstream longlat_outstream,UTM_outstream;
   longlat_outstream.precision(12);
   UTM_outstream.precision(12);
   filefunc::openfile(longlat_flowfield_filename,longlat_outstream);
   filefunc::openfile(UTM_flowfield_filename,UTM_outstream);

   bool northern_hemisphere_flag=get_EarthRegionsGroup_ptr()->
      get_northern_hemisphere_flag();
   int UTM_zone=get_EarthRegionsGroup_ptr()->get_specified_UTM_zonenumber();

   unsigned int n_arrows=ArrowsGroup_ptr->get_n_Graphicals();
   for (unsigned int a=0; a<n_arrows; a++)
   {
      Arrow* Arrow_ptr=ArrowsGroup_ptr->get_Arrow_ptr(a);
      threevector V_tip(Arrow_ptr->get_V_tip());
      threevector V_base(Arrow_ptr->get_V_base());

      if (!V_tip.nearly_equal(V_base))
      {
         geopoint tip_geopoint(northern_hemisphere_flag,UTM_zone,
                               V_tip.get(0),V_tip.get(1));
         geopoint base_geopoint(northern_hemisphere_flag,UTM_zone,
                                V_base.get(0),V_base.get(1));
//         cout << "a = " << a
//              << " V_tip: long = " << tip_geopoint.get_longitude()
//              << " lat = " << tip_geopoint.get_latitude() 
//              << " V_base: long = " << base_geopoint.get_longitude()
//              << " lat = " << base_geopoint.get_latitude() << endl;
         longlat_outstream << base_geopoint.get_longitude() << "   "
                           << base_geopoint.get_latitude() << endl;
         longlat_outstream << tip_geopoint.get_longitude() << "   "
                           << tip_geopoint.get_latitude() << endl << endl;
         UTM_outstream << base_geopoint.get_UTM_easting() << "   "
                       << base_geopoint.get_UTM_northing() << endl;
         UTM_outstream << tip_geopoint.get_UTM_easting() << "   "
                       << tip_geopoint.get_UTM_northing() << endl << endl;
      }
   } // loop over index a labeling flowfield arrows

   filefunc::closefile(longlat_flowfield_filename,longlat_outstream);
   filefunc::closefile(UTM_flowfield_filename,longlat_outstream);
}

// ---------------------------------------------------------------------
// Member function clear_visibility_skymaps() 

void LOSMODELSGROUP::clear_visibility_skymaps()
{   
//   cout << "inside LOSMODELSGROUP::clear_visibility_skymaps()" 
//        << endl;

   get_ArrowsGroup_ptr()->destroy_all_Arrows();
   string progress_type="flowfield computation";
   get_viewer_Messenger_ptr()->broadcast_clear_progress(progress_type);
}

// ==========================================================================
// Ground target visibility member functions
// ==========================================================================

// Member function compute_target_visibilities_along_flightpath()
// first recovers positions for ground target SignPosts.  It then
// loops over OBSFRUSTA within MODEL_ptr->get_OBSFRUSTAGROUP_ptr() and
// loads DTED height data covering their trapezoidal projections onto
// ground zplanes.  This method raytraces each ground target and
// returns line-of-sight results within output STL vector
// target_tracing_result.

// This method should really become a member function of the MODEL
// class.  But we were unsuccessful in moving this method into MODEL
// on Weds, Oct 7 at 10 am...

bool LOSMODELSGROUP::compute_target_visibilities_along_flightpath(
   MODEL* MODEL_ptr,bool reload_DTED_tiles_flag)
{   
//   cout << "inside LOSMODELSGROUP::compute_target_visibilities_along_flightpath()" << endl;

   bool tgts_prev_raytraced_flag=true;
   if (GroundTarget_SignPostsGroup_ptr==NULL)
      return tgts_prev_raytraced_flag;

   vector<twovector> target_posns;
   GroundTarget_SignPostsGroup_ptr->get_ground_target_posns(target_posns);
//   cout << "n_ground_targets = " << target_posns.size() << endl;

   if (target_posns.size()==0) return tgts_prev_raytraced_flag;

// First check whether target visibilities to all ground targets have
// already been calculated:

   for (unsigned int t=0; t<GroundTarget_SignPostsGroup_ptr->
           get_n_Graphicals(); t++)
   {
      SignPost* SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
         get_SignPost_ptr(t);
      twovector t_ID(get_curr_t(),SignPost_ptr->get_ID());
//      cout << "t = " << t << " t_ID = " << t_ID << endl;

      TARGET_VISIBILITY_MAP::iterator tgt_vis_iter=
         target_visibility_map_ptr->find(t_ID);
      if (tgt_vis_iter==target_visibility_map_ptr->end())
      {
         tgts_prev_raytraced_flag=false;
      }
   } // loop over index t labeling ground targets

//   cout << "tgts_prev_raytraced_flag = " << tgts_prev_raytraced_flag << endl;
   if (tgts_prev_raytraced_flag) 
   {
//      for (unsigned int t=0; t<GroundTarget_SignPostsGroup_ptr->get_n_Graphicals(); 
//           t++)
//      {
//         SignPost* SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
//            get_SignPost_ptr(t);
//         cout << "curr_t = " << get_curr_t() 
//              << " SignPost ID = " << SignPost_ptr->get_ID()
//              << " visibility = " 
//              << get_ground_target_visibility(
//                 get_curr_t(),SignPost_ptr->get_ID()) << endl;
//      }

      if (!score_broadcasted_flag)
      {
         double avg_visibility_percentage=
            100*averaged_ground_target_visibility();
         cout << "Averaged ground targets' visibility percentage = " 
              << avg_visibility_percentage << endl;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

         vector<Messenger::Property> properties;

         string command="SEND_AVERAGED_GROUND_TARGET_VISIBILITY";	
         string key="Averaged visibility fraction";
         string value=stringfunc::number_to_string(
            avg_visibility_percentage,1);
         properties.push_back(Messenger::Property(key,value));

         viewer_Messenger_ptr->broadcast_subpacket(command,properties);
         score_broadcasted_flag=true;
      }
      return tgts_prev_raytraced_flag;
   }

   twoDarray* DTED_ztwoDarray_ptr=TilesGroup_ptr->get_DTED_ztwoDarray_ptr();
   twoDarray* reduced_DTED_ztwoDarray_ptr=TilesGroup_ptr->
      get_reduced_DTED_ztwoDarray_ptr();

// If reload_DTED_tiles_flag==true, recompute lon,lat coords for tiles
// which intersect OBSFRUSTA footprint and store results within STL
// vector latlong_points_inside_polygon:

//   cout << "reload_DTED_tiles_flag = " << reload_DTED_tiles_flag << endl;
   vector<threevector> latlong_points_inside_polygon;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=MODEL_ptr->get_OBSFRUSTAGROUP_ptr();
   for (unsigned int i=0; i<OBSFRUSTAGROUP_ptr->get_n_Graphicals() && 
           reload_DTED_tiles_flag && !ladar_height_data_flag; i++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(i);

      vector<threevector> curr_latlong_pnts_inside_polygon;
      OBSFRUSTUM_ptr->compute_latlong_pnts_inside_footprint(
         MODEL_ptr->get_max_raytrace_range(),get_raytrace_cellsize(),
         curr_latlong_pnts_inside_polygon);
      for (unsigned int l=0; l<curr_latlong_pnts_inside_polygon.size(); l++)
      {
         latlong_points_inside_polygon.push_back(
            curr_latlong_pnts_inside_polygon[l]);
      }
   } // loop over index i labeling OBSFRUSTA

   for (unsigned int i=0; i<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); i++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(i);

      if (reload_DTED_tiles_flag && !ladar_height_data_flag)
      {

// Load terrain height data into *DTED_ztwoDarray_ptr, generate a
// reduced height map for speed purposes, and update max_ground_Z
// value:

         DTED_ztwoDarray_ptr=OBSFRUSTUM_ptr->load_DTED_height_data(
            get_raytrace_cellsize(),latlong_points_inside_polygon);
//         cout << "DTED_ztwoDarray_ptr = " << DTED_ztwoDarray_ptr << endl;
         if (DTED_ztwoDarray_ptr != NULL)
            reduced_DTED_ztwoDarray_ptr=TilesGroup_ptr->
               generate_reduced_DTED_ztwoDarray();
      } // reload_DTED_tiles_flag conditional

      double ds;
      if (ladar_height_data_flag)
      { 
//         ds=0.1;	   // meter	// For 0.3 ALIRT data
//         ds=0.4;	   // meter	// For 1 meter ALIRT data
         ds=0.8;	   // meter	// For 1 meter ALIRT data
      }
      else
      {
         ds=0.25*get_raytrace_cellsize();
      }
      vector<pair<int,threevector> > target_tracing_result;

// For TOC11 red actor path discovery, we do NOT use
// DTED_ztwoDarray_ptr to determine ladar terrain heights.  Instead,
// OBSFRUSTUM::raytrace_ground_targets() calls
// TilesGroup_ptr->get_ladar_z_given_xy(curr_x,curr_y,curr_z);

      if (ladar_height_data_flag)
      {
         OBSFRUSTUM_ptr->raytrace_ground_targets(
            target_posns,max_ground_Z,max_raytrace_range,min_raytrace_range,ds,
            target_tracing_result);
      }
      else
      {

// DTED_ztwoDarray_ptr should NOT equal NULL in this part of the conditional!

         max_ground_Z=DTED_ztwoDarray_ptr->maximum_value();
         twoDarray* DTED_ptwoDarray_ptr=new twoDarray(DTED_ztwoDarray_ptr);
         OBSFRUSTUM_ptr->raytrace_ground_targets(
            target_posns,max_ground_Z,max_raytrace_range,min_raytrace_range,ds,
            DTED_ztwoDarray_ptr,DTED_ptwoDarray_ptr,
            reduced_DTED_ztwoDarray_ptr,target_tracing_result);
         delete DTED_ptwoDarray_ptr;
      }

      tgts_prev_raytraced_flag=true;
      for (unsigned int t=0; t<target_tracing_result.size(); t++)
      {
         SignPost* SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
            get_SignPost_ptr(t);
         int SignPost_ID=SignPost_ptr->get_ID();
         twovector t_ID(get_curr_t(),SignPost_ID);

         TARGET_VISIBILITY_MAP::iterator tgt_vis_iter=
            target_visibility_map_ptr->find(t_ID);
         if (tgt_vis_iter==target_visibility_map_ptr->end())
         {
            (*target_visibility_map_ptr)[t_ID]=target_tracing_result[t];
            tgts_prev_raytraced_flag=false;
         }
         else
         {

// If a target was not previously visible and now is visible or
// occluded, reset its visibility entry within
// *target_visibility_map_ptr.  If a target was previously occluded
// and is now visible, similarly reset its visibility entry:

            int prev_tgt_tracking_result=tgt_vis_iter->second.first;
            if (target_tracing_result[t].first > prev_tgt_tracking_result)
            {
               (*target_visibility_map_ptr)[t_ID]=target_tracing_result[t];
            }
         }
//         cout << "t = " << t << " target_vis[t_ID] = "
//              << (*target_visibility_map_ptr)[t_ID].first << endl;
      } // loop over index t labeling ground targets
   } // loop over index i labeling OBSFRUSTA

   return tgts_prev_raytraced_flag;
}

// ---------------------------------------------------------------------
void LOSMODELSGROUP::clear_raytracing_results()
{   
//   cout << "inside LOSMODELSGROUP::clear_raytracing_results()" << endl;

   get_LineSegmentsGroup_ptr()->destroy_all_LineSegments();
   target_visibility_map_ptr->clear();
   set_ray_type(LOSMODELSGROUP::single_air_to_multi_ground);
   set_reload_DTED_tiles_flag(true);
   score_broadcasted_flag=false;

   for (unsigned int i=0; i<get_n_OSGsubPATs(); i++)
   {
      set_OSGsubPAT_nodemask(i,1);
   }
   get_LineSegmentsGroup_ptr()->set_OSGgroup_nodemask(1);

   string progress_type="raytracing";
   get_viewer_Messenger_ptr()->broadcast_clear_progress(progress_type);

   for (unsigned int m=0; m<get_n_Graphicals(); m++)
   {
      MODEL* MODEL_ptr=get_MODEL_ptr(m);
      if (MODEL_ptr==NULL)
      {
         cout << "Error in LOSServer::clear_raytracing_results()!" << endl;
         cout << "MODEL_ptr = NULL!!!" << endl;
         outputfunc::enter_continue_char();
         continue;
      }
      
      MODEL_ptr->reset_all_scores(NEGATIVEINFINITY);
      MODEL_ptr->set_raytrace_occluded_ground_regions_flag(false);

      if (MODEL_ptr->get_raytrace_ROI_flag()==false)
      {
         MODEL_ptr->set_raytrace_ROI_flag(true);
      }

      set_raytrace_progress(0);

// Purge histogrammed average raytracing fractions from thin client
// output by broadcasting average occlusion fractions with
// clear_results_flag=true argument:

      vector<double> frac;
      frac.push_back(0);
      frac.push_back(0);
      frac.push_back(0);
      frac.push_back(0);
      bool clear_results_flag=true;
      MODEL_ptr->broadcast_average_occlusion_fractions(
         frac,get_viewer_Messenger_ptr(),clear_results_flag);
   } // loop over index m

   PointCloudsGroup_ptr->get_ColorGeodeVisitor_ptr()->
      clear_ptwoDarray_ptrs();
   PointCloudsGroup_ptr->reload_all_colors();
   TilesGroup_ptr->purge_tile_files();
   PointCloudsGroup_ptr->reset_cloud_coloring_to_zeroth_height_colormap();

// Reset clock to starting time and then pause it:

   AnimationController_ptr->set_curr_framenumber(
      AnimationController_ptr->get_first_framenumber());
   AnimationController_ptr->setState(AnimationController::PAUSE);
}

// ---------------------------------------------------------------------
// Member function set_display_ImageNumberHUD() turns on/off the movie
// number and world_time flags.

void LOSMODELSGROUP::set_display_ImageNumberHUD(bool flag)
{
   
// Turn on movie number and world time display:

   ImageNumberHUD* ImageNumberHUD_ptr=Operations_ptr->get_ImageNumberHUD_ptr();
   ImageNumberHUD_ptr->set_display_movie_number_flag(true);
   ImageNumberHUD_ptr->set_display_movie_world_time_flag(true);
}

// ---------------------------------------------------------------------
// Member function get_ground_target_visibility() returns -2 if no
// ground target entry within member *target_visibility_map_ptr is
// found, -1 if the ground target lies outside the aerial sensor's
// field-of-regard at time curr_time, 0 if the ground target is
// occluded from the sensor's view and 1 if the ground target is
// visible to the sensor.

int LOSMODELSGROUP::get_ground_target_visibility(
   double curr_time,int target_ID) const
{   
//   cout << "inside LOSMODELSGROUP::get_ground_target_visibility()" << endl;
//   cout << "curr_time = " << curr_time 
//        << " target ID = " << target_ID << endl;

// Store ground target visibility (-1 = target lies outside sensor's
// field of regard, 0 = target is occluded , 1 = target is visible) as
// a function of twovector (time, target ID) within
// *target_visibility_map_ptr:

   twovector t_ID(curr_time,target_ID);
   TARGET_VISIBILITY_MAP::iterator tgt_vis_iter=
      target_visibility_map_ptr->find(t_ID);
   if (tgt_vis_iter==target_visibility_map_ptr->end())
   {
      return -2;
   }
   else
   {
//      cout << "visibility = " << tgt_vis_iter->second << endl;
      return tgt_vis_iter->second.first;
   }
}

// ---------------------------------------------------------------------
// This overloaded version of get_ground_target_visibility() returns
// the point at which a ray is occluded by some mountain within output
// threevector occlusion_posn in addition to an integer visibility
// value.

int LOSMODELSGROUP::get_ground_target_visibility(
   double curr_time,int target_ID,threevector& occlusion_posn) const
{   
//   cout << "inside LOSMODELSGROUP::get_ground_target_visibility_and_occlusion_posn()" << endl;
//   cout << "curr_time = " << curr_time 
//        << " target ID = " << target_ID << endl;

// Store ground target visibility (-1 = target lies outside sensor's
// field of regard, 0 = target is occluded , 1 = target is visible) as
// a function of twovector (time, target ID) within
// *target_visibility_map_ptr:

   twovector t_ID(curr_time,target_ID);
   TARGET_VISIBILITY_MAP::iterator tgt_vis_iter=
      target_visibility_map_ptr->find(t_ID);
   if (tgt_vis_iter==target_visibility_map_ptr->end())
   {
      return -2;
   }
   else
   {
//      cout << "visibility = " << tgt_vis_iter->second << endl;
      occlusion_posn=tgt_vis_iter->second.second;
      return tgt_vis_iter->second.first;
   }
}

// ---------------------------------------------------------------------
// Member function draw_colored_single_air_to_multi_ground_rays()
// loops over all ground targets.  If a corresponding LineSegment ray
// connecting each ground target to the aerial MODEL does not already
// exist, this method instantiates a new member of
// *LineSegmentsGroup_ptr.  It then sets the color of the ray equal to
// black if the target lies outside the aerial sensor's
// field-of-regard, red if the target is occluded from the sensor's
// view and green if the target is visible to the sensor.

void LOSMODELSGROUP::draw_colored_single_air_to_multi_ground_rays(
   bool tgts_prev_raytraced_flag,MODEL* MODEL_ptr)
{   

//   cout << "inside LOSMODELSGROUP::draw_colored_single_air_to_multi_ground_rays()"
//        << endl;
        
   if (ray_type==multi_air_to_single_ground) return;
   if (GroundTarget_SignPostsGroup_ptr==NULL) return;

   double curr_t=get_curr_t();

   threevector aerial_posn,ground_posn;
   MODEL_ptr->get_UVW_coords(curr_t,get_passnumber(),aerial_posn);
//   cout << "aerial_posn = " << aerial_posn << endl;

   for (unsigned int t=0; t<GroundTarget_SignPostsGroup_ptr->
           get_n_Graphicals(); t++)
   {
      LineSegment* LineSegment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(t);
      if (LineSegment_ptr==NULL)
      {
         bool draw_endpoint1_flag=true;
         bool draw_endpoint2_flag=true;
         LineSegment_ptr=LineSegmentsGroup_ptr->
            generate_new_canonical_LineSegment(
               -1,draw_endpoint1_flag,draw_endpoint2_flag,
               endpoint_size_prefactor);
      }
      LineSegment_ptr->set_stationary_Graphical_flag(false);

      SignPost* SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
         get_SignPost_ptr(t);
      SignPost_ptr->get_UVW_coords(
         GroundTarget_SignPostsGroup_ptr->get_curr_t(),
         GroundTarget_SignPostsGroup_ptr->get_passnumber(),
         ground_posn);

      threevector occlusion_posn;
      int tgt_visibility=get_ground_target_visibility(
         curr_t,SignPost_ptr->get_ID(),occlusion_posn);

//      cout << "curr_t = " << curr_t 
//           << " ground_posn = " << ground_posn
//           << " tgt_visibility = " << tgt_visibility << endl;

      if (!tgts_prev_raytraced_flag)
      {
         MODEL_ptr->broadcast_ground_target_visibility(
            curr_t,SignPost_ptr->get_ID(),tgt_visibility,viewer_Messenger_ptr);
      }
    
      colorfunc::Color ray_color;
      if (tgt_visibility==-2)
      {
         continue;
      }
      else if (tgt_visibility==-1)
      {
//         ray_color=colorfunc::grey;
         ray_color=colorfunc::black;
      }
      else if (tgt_visibility==0)
      {
         ray_color=colorfunc::red;
         ground_posn=occlusion_posn;
      }
      else if (tgt_visibility==1)
      {
         ray_color=colorfunc::green;
      }

      LineSegment_ptr->set_scale_attitude_posn(
         LineSegmentsGroup_ptr->get_curr_t(),
         LineSegmentsGroup_ptr->get_passnumber(),
         aerial_posn,ground_posn);

//      cout << "ray_color = " << ray_color << endl;
      LineSegment_ptr->set_permanent_color(ray_color);
      LineSegment_ptr->set_curr_color(ray_color);
      LineSegment_ptr->get_LineWidth_ptr()->setWidth(10.0);

   } // loop over index t labeling ground targets

//   cout << "LineSegmentsGroup_ptr->get_n_Graphicals() = "
//        << LineSegmentsGroup_ptr->get_n_Graphicals() << endl;
}

// ---------------------------------------------------------------------
// Member function draw_colored_multi_air_to_single_ground_rays()
// loops over all sampled flight points.  If a corresponding
// LineSegment ray connecting each flight point to the specified
// ground target does not already exist, this method instantiates a
// new member of *LineSegmentsGroup_ptr.  It then sets the color of
// the ray equal to black if the target lies outside the aerial
// sensor's field-of-regard, red if the target is occluded from the
// sensor's view and green if the target is visible to the sensor.

void LOSMODELSGROUP::draw_colored_multi_air_to_single_ground_rays(
   int target_ID,MODEL* MODEL_ptr)
{   
//   cout << "inside LOSMODELSGROUP::draw_colored_multi_air_to_single_ground_rays()"
//        << endl;

//   cout << "ray_type = " << ray_type << endl;
//   cout << "single_air_to_multi_ground = " << single_air_to_multi_ground
//        << endl;

   if (ray_type==single_air_to_multi_ground) return;
   if (GroundTarget_SignPostsGroup_ptr==NULL) return;

   threevector aerial_posn,ground_posn;

   SignPost* SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
      get_SignPost_ptr(target_ID);
   if (SignPost_ptr==NULL) return;
   
   SignPost_ptr->get_UVW_coords(
      GroundTarget_SignPostsGroup_ptr->get_curr_t(),
      GroundTarget_SignPostsGroup_ptr->get_passnumber(),
      ground_posn);
//   cout << "ground_posn = " << ground_posn << endl;

   LineSegmentsGroup_ptr->destroy_all_LineSegments();
   for (unsigned int curr_t=get_first_framenumber(); curr_t<=get_last_framenumber(); 
        curr_t++)
   {
      MODEL_ptr->get_UVW_coords(curr_t,get_passnumber(),aerial_posn);

      threevector ray_stop_posn(ground_posn);
      threevector occlusion_posn;
      int tgt_visibility=get_ground_target_visibility(
         curr_t,SignPost_ptr->get_ID(),occlusion_posn);

//      cout << "curr_t = " << curr_t 
//           << " aerial_posn = " << aerial_posn
//           << " tgt_visibility = " << tgt_visibility << endl;

      LineSegment* LineSegment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(curr_t);
      if (LineSegment_ptr==NULL)
      {
         bool draw_endpoint1_flag=true;
         bool draw_endpoint2_flag=true;
         LineSegment_ptr=LineSegmentsGroup_ptr->
            generate_new_canonical_LineSegment(
               -1,draw_endpoint1_flag,draw_endpoint2_flag,
               endpoint_size_prefactor);
      }
      LineSegment_ptr->set_stationary_Graphical_flag(false);
      
      colorfunc::Color ray_color;
      if (tgt_visibility==-2)
      {
         continue;
      }
      else if (tgt_visibility==-1)
      {
//         ray_color=colorfunc::grey;
         ray_color=colorfunc::black;
      }
      else if (tgt_visibility==0)
      {
         ray_stop_posn=occlusion_posn;
         ray_color=colorfunc::red;
      }
      else if (tgt_visibility==1)
      {
         ray_color=colorfunc::green;
      }

      LineSegment_ptr->set_scale_attitude_posn(
         LineSegmentsGroup_ptr->get_curr_t(),
         LineSegmentsGroup_ptr->get_passnumber(),
         aerial_posn,ray_stop_posn);

//      cout << "ray_color = " << ray_color << endl;
      LineSegment_ptr->set_permanent_color(ray_color);
      LineSegment_ptr->set_curr_color(ray_color);
      LineSegment_ptr->get_LineWidth_ptr()->setWidth(10.0);
   } // loop over index curr_t labeling frame number

//   cout << "LineSegmentsGroup_ptr->get_n_Graphicals() = "
//        << LineSegmentsGroup_ptr->get_n_Graphicals() << endl;
}

// ---------------------------------------------------------------------
// Member function purge_multi_air_to_single_ground_linesegments()

void LOSMODELSGROUP::purge_multi_air_to_single_ground_linesegments()
{   
   LineSegmentsGroup_ptr->destroy_all_LineSegments();
}

// ---------------------------------------------------------------------
// Member function averaged_ground_target_visibility() retrieves
// previously computed target visibilities for all points along a
// flight path.  It forms and returns a normalized fraction
// corresponding to the average visibility of all the ground targets
// over the entire flight path.

double LOSMODELSGROUP::averaged_ground_target_visibility()
{   
//   cout << "inside LOSMODELSGROUP::averaged_ground_target_visibility()"
//        << endl;

   if (GroundTarget_SignPostsGroup_ptr==NULL) return 0;

   int n_frames=get_last_framenumber()-get_first_framenumber()+1;
   double tgt_visibility_integral=0;
   unsigned int n_ground_targets=GroundTarget_SignPostsGroup_ptr->
      get_n_Graphicals();
   for (unsigned int n=0; n<n_ground_targets; n++)
   {
      SignPost* SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
         get_SignPost_ptr(n);
      for (unsigned int curr_t=get_first_framenumber(); curr_t<=get_last_framenumber(); 
           curr_t++)
      {
         threevector occlusion_posn;
         int tgt_visibility=get_ground_target_visibility(
            curr_t,SignPost_ptr->get_ID(),occlusion_posn);
         if (tgt_visibility==1)
         {
            tgt_visibility_integral += 1;
         }
      } // loop over curr_t index labeling frame number
   } // loop over index n labeling ground targets
   double average_tgt_visibility = 
      tgt_visibility_integral/(n_frames*n_ground_targets);
//   cout << "average_tgt_visibility = " << average_tgt_visibility << endl;
   
   return average_tgt_visibility;
}

// ==========================================================================
// SAM threat map member functions
// ==========================================================================

// Member function visualize_SAM_threatmap() computes relative threat
// values within the aerial *threatmap_twoDarray_ptr.  It then
// instantiates a texture_rectangle with the same dimensions as the
// threatmap and copies the latter's data into the former.  This
// method assigns the texture rectangle to a Movie whose geometry
// vertices are stretched to cover the same lat-long extents in the
// sky as threatmap.

Movie* LOSMODELSGROUP::visualize_SAM_threatmap(
   double SAM_range,
   double threatmap_longitude_lo,double threatmap_longitude_hi,
   double threatmap_latitude_lo,double threatmap_latitude_hi)
{
   cout << "inside LOSMODELSGROUP::visualize_SAM_threatmap()" << endl;

   if (EarthRegionsGroup_ptr == NULL) return NULL;
   if (MoviesGroup_ptr==NULL) return NULL;

   double flowfield_progress=0.02;
   string progress_type="threat map";
   viewer_Messenger_ptr->broadcast_progress(flowfield_progress,progress_type);

   set_threatmap_corners(threatmap_longitude_lo,threatmap_longitude_hi,
                         threatmap_latitude_lo,threatmap_latitude_hi,
                         0.99*aircraft_altitude);
   generate_SAM_threatmap(SAM_range,flowfield_progress,progress_type);

// Purge existing *threat_texture_rectangle_ptr and instantiate new
// one:

   int n_images=1;
   int n_channels=3;
   delete threat_texture_rectangle_ptr;
   threat_texture_rectangle_ptr=new texture_rectangle(
      threatmap_twoDarray_ptr->get_mdim(),
      threatmap_twoDarray_ptr->get_ndim(),
      n_images,n_channels,get_AnimationController_ptr());
   cout << "threat_texture_rectangle_ptr = " 
        << threat_texture_rectangle_ptr << endl;

// Copy contents from *threatmap_twoDarray_ptr into
// *texture_rectangle_ptr's unsigned char image array:

   threat_texture_rectangle_ptr->initialize_twoDarray_image(
      threatmap_twoDarray_ptr);
   const double output_alpha=1.0;

// As of 10/30/09, we indicate danger with red rather than purple
// coloring:

   double hue=0;		// red
//   double hue=300;		// purple
   threat_texture_rectangle_ptr->convert_greyscale_image_to_hue_value_colored(
      hue,output_alpha);

// Reset Movie's texture 3D coordinates to coincide with threatmap's
// corners in UTM coordinates:

   MoviesGroup_ptr->destroy_all_Movies();
   Movie* Movie_ptr=MoviesGroup_ptr->generate_new_Movie(
      threat_texture_rectangle_ptr);
   cout << "Movie_ptr = " << Movie_ptr << endl;

// Reposition Movie_ptr so that it hovers above the threat area on the
// ground at the LOS aircraft's altitude:
   
   threevector bottom_left=lower_left_threatmap_corner.get_UTM_posn();
   threevector bottom_right(
      upper_right_threatmap_corner.get_UTM_easting(),
      lower_left_threatmap_corner.get_UTM_northing(),
      upper_right_threatmap_corner.get_altitude());
   threevector top_right=upper_right_threatmap_corner.get_UTM_posn();
   threevector top_left(
      lower_left_threatmap_corner.get_UTM_easting(),
      upper_right_threatmap_corner.get_UTM_northing(),
      upper_right_threatmap_corner.get_altitude());
   Movie_ptr->reset_geom_vertices(
      bottom_right,bottom_left,top_left,top_right);

   viewer_Messenger_ptr->broadcast_finished_progress(progress_type);

   return Movie_ptr;
}

// ---------------------------------------------------------------------
// Member function set_threatmap_corners() sets the lower left and
// upper right corners of the threatmap based upon the input lat-long
// extents.

void LOSMODELSGROUP::set_threatmap_corners(
   double threatmap_longitude_lo,double threatmap_longitude_hi,
   double threatmap_latitude_lo,double threatmap_latitude_hi,
   double threatmap_altitude)
{   
//   cout << "inside LOSMODELSGROUP::set_threatmap_extents()" << endl;

   int UTM_zone=get_EarthRegionsGroup_ptr()->
      get_specified_UTM_zonenumber();
   lower_left_threatmap_corner=geopoint(
      threatmap_longitude_lo,threatmap_latitude_lo,threatmap_altitude,
      UTM_zone);
   upper_right_threatmap_corner=geopoint(
      threatmap_longitude_hi,threatmap_latitude_hi,threatmap_altitude,
      UTM_zone);

//   cout << "lower_left_threatmap_corner = " 
//        << lower_left_threatmap_corner << endl;
//   cout << "upper_right_threatmap_corner = "
//        << upper_right_threatmap_corner << endl;
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function generate_SAM_threatmap() instantiates
// *threatmap_twoDarray_ptr with a size set by the threatmap's
// lat-long extents. Looping over 1deg x 1deg lat-long tiles, it
// subdivides each tile into 250 m x 250 m cells.  For each cell, this
// method tests whether the distance between the cell's ground height
// and the aircraft's altitude is less than the input SAM range.  If
// so, it computes the opening angle of a cone centered about the cell
// within which the SAM could hit an aircraft flying at the specified
// altitude.  It increments the threat count within all lattice sites
// of the aerial threat map covered by the cone.  

void LOSMODELSGROUP::generate_SAM_threatmap(
   double SAM_range,double threatmap_progress,string progress_type)
{
   cout << "inside LOSMODELSGROUP::generate_SAM_threatmap()" << endl;
   cout << "SAM_range = " << SAM_range << endl;

   double threatmap_longitude_lo=lower_left_threatmap_corner.get_longitude();
   double threatmap_longitude_hi=upper_right_threatmap_corner.get_longitude();
   double threatmap_latitude_lo=lower_left_threatmap_corner.get_latitude();
   double threatmap_latitude_hi=upper_right_threatmap_corner.get_latitude();
//   cout << "long_lo = " << threatmap_longitude_lo
//        << " long_hi = " << threatmap_longitude_hi << endl;
//   cout << "lat_lo = " << threatmap_latitude_lo
//        << " lat_hi = " << threatmap_latitude_hi << endl;
   unsigned int mdim=150*(threatmap_longitude_hi-threatmap_longitude_lo);
   unsigned int ndim=150*(threatmap_latitude_hi-threatmap_latitude_lo);
//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

   delete threatmap_twoDarray_ptr;
   threatmap_twoDarray_ptr=new twoDarray(mdim,ndim);
   threatmap_twoDarray_ptr->init_coord_system(
      lower_left_threatmap_corner.get_UTM_easting(),
      upper_right_threatmap_corner.get_UTM_easting(),
      lower_left_threatmap_corner.get_UTM_northing(),
      upper_right_threatmap_corner.get_UTM_northing());
   threatmap_twoDarray_ptr->initialize_values(0);
   
   double dx_threat=threatmap_twoDarray_ptr->get_deltax();
   int UTM_zone=get_EarthRegionsGroup_ptr()->get_specified_UTM_zonenumber();
   cout << "UTM_zone = " << UTM_zone << endl;

   const double dx_ground=250;	// meters
   const double dy_ground=250;	// meters

// Loop over 1 deg x 1 deg lat-long tiles:

   int longitude_int_lo=threatmap_longitude_lo;
   if (threatmap_longitude_lo < 0)
   {
      longitude_int_lo--;
   }
   int longitude_int_hi=basic_math::mytruncate(threatmap_longitude_hi)+1;

   cout << "longitude_int_lo = " << longitude_int_lo
        << " longitude_int_hi = " << longitude_int_hi << endl;

   int n_longitude_bins=longitude_int_hi-longitude_int_lo;

   for (int longitude=longitude_int_lo; longitude < longitude_int_hi; 
        longitude++)
   {
      for (int latitude=lower_left_threatmap_corner.get_latitude();
           latitude < upper_right_threatmap_corner.get_latitude(); latitude++)
      {
         cout << "long = " << longitude << " lat = " << latitude << endl;

         bool some_data_imported_flag;
         twoDarray* DTED_ztwoDarray_ptr=
            TilesGroup_ptr->load_single_DTED_tile_into_ztwoDarray(
               longitude,latitude,dx_ground,dy_ground,
               some_data_imported_flag);
         if (!some_data_imported_flag) continue;

         geopoint cell_lower_left(double(longitude),double(latitude),
                                  0.0,UTM_zone);
         geopoint cell_lower_right(double(longitude+1),double(latitude),
                                  0.0,UTM_zone);
         geopoint cell_upper_right(double(longitude+1),double(latitude+1),
                                   0.0,UTM_zone);
         geopoint cell_upper_left(double(longitude),double(latitude+1),
                                   0.0,UTM_zone);

         double easting_lo=basic_math::min(cell_lower_left.get_UTM_easting(),
                               cell_upper_left.get_UTM_easting());
         double easting_hi=basic_math::max(cell_lower_right.get_UTM_easting(),
                               cell_upper_right.get_UTM_easting());
         double northing_lo=basic_math::min(cell_lower_left.get_UTM_northing(),
                                cell_lower_right.get_UTM_northing());
         double northing_hi=basic_math::max(cell_upper_left.get_UTM_northing(),
                                cell_upper_right.get_UTM_northing());

// Loop over 250 m x 250 m cells within the current 1 deg x 1 deg
// lat-long tile:

         for (double easting=easting_lo; easting <= easting_hi; 
              easting += dx_ground)
         {
            for (double northing=northing_lo; northing <= northing_hi;
                 northing += dy_ground)
            {
               unsigned int px,py;
               if (DTED_ztwoDarray_ptr->point_to_pixel(
                      easting,northing,px,py))
               {
                  double z_ground=DTED_ztwoDarray_ptr->get(px,py);

// Recall zero values within *DTED_ztwoDarray_ptr indicate missing
// height data:

                  const double min_height_threshold=10;		 // meters
//                  cout << "east=" << easting
//                       << " north=" << northing
//                       << " zgrnd=" << z_ground << endl;
                  
                  if (z_ground < min_height_threshold) continue;

                  double delta_z=(aircraft_altitude-z_ground);
                  if (delta_z < SAM_range)
                  {
                     unsigned int qx_center,qy_center;
                     if (threatmap_twoDarray_ptr->point_to_pixel(
                            easting,northing,qx_center,qy_center))
                     {

                        double theta_danger=acos(delta_z/SAM_range);
                        double rho=SAM_range*sin(theta_danger);
                        double sqrd_rho=rho*rho;
                        int extra_q=rho/dx_threat;

                        for (unsigned int qx=qx_center-extra_q; 
                             qx<=qx_center+extra_q; qx++)
                        {
                           if (!threatmap_twoDarray_ptr->
                               px_inside_working_region(qx)) continue;

                           for (unsigned int qy=qy_center-extra_q; 
                                qy<=qy_center+extra_q; qy++)
                           {
                              if (!threatmap_twoDarray_ptr->
                                  py_inside_working_region(qy)) continue;

                              double curr_easting,curr_northing;
                              if (threatmap_twoDarray_ptr->pixel_to_point(
                                     qx,qy,curr_easting,curr_northing))
                              {
                                 double sqrd_dist=sqr(curr_easting-easting)+
                                    sqr(curr_northing-northing);
                                 if (sqrd_dist <= sqrd_rho) 
                                 {
                                    int curr_n_threats=
                                       threatmap_twoDarray_ptr->get(qx,qy);
                                    threatmap_twoDarray_ptr->put(
                                       qx,qy,curr_n_threats+1);
//                                    threatmap_twoDarray_ptr->put(qx,qy,1);
//                                    threatmap_twoDarray_ptr->put(
//                                       qx,qy,z_ground);
                                 } // sqrd_dist < sqrd_rho conditional
                              } // qx,qy in threatmap_twoDarray conditional
                           } // loop over qy index
                        } // loop over qx index

                     } // easting,northing in threatmap_twoDarray conditional
                  } // distance_to_ground < SAM_range conditional

               } // easting,northing inside DTED_ztwoDarray conditional
            } // loop over northing
         } // loop over easting
      } // loop over latitude index

      threatmap_progress += 1.0/double(n_longitude_bins+1);
      threatmap_progress=basic_math::min(threatmap_progress,1.0);
      double rounded_threatmap_progress=
         0.01*basic_math::round(100*threatmap_progress);
      cout << "rounded_threatmap_progress = " << rounded_threatmap_progress
           << endl;
      viewer_Messenger_ptr->broadcast_progress(
         rounded_threatmap_progress,progress_type);

   } // loop over longitude index

// Multiply threatmap_twoDarray entries to convert them into areas on
// ground within SAM range of aircraft measured in (km**2):

   double normalization=sqr(0.001)*dx_ground*dy_ground;		// km**2
   (*threatmap_twoDarray_ptr) *= normalization;

// Recall twoDarray inputs to texture rectangles values are assumed
// range from 0 to 1.  So renormalize threatmap magnitudes by ground
// area normalization factor:

   const double ground_area_normalization=100;	// km**2
   double max_ground_area_value=threatmap_twoDarray_ptr->maximum_value();
   cout << "Max threatmap ground area value = " 
        << max_ground_area_value << endl;
   (*threatmap_twoDarray_ptr) /= ground_area_normalization;

// Reset any renormalized threatmap value exceeding unity to 1.0:

   for (unsigned int px=0; px<mdim; px++)
   {
      for (unsigned int py=0; py<ndim; py++)
      {
         if (threatmap_twoDarray_ptr->get(px,py) > 1)
         {
            threatmap_twoDarray_ptr->put(px,py,1);
         }
      }
   }
}

// ---------------------------------------------------------------------
// Member function clear_SAM_threatmap() 

void LOSMODELSGROUP::clear_SAM_threatmap()
{
//   cout << "inside LOSMODELSGROUP::clear_SAM_threatmap()" << endl;

   delete threatmap_twoDarray_ptr;
   threatmap_twoDarray_ptr=NULL;

   delete threat_texture_rectangle_ptr;
   threat_texture_rectangle_ptr=NULL;

   MoviesGroup_ptr->destroy_all_Movies();
}

// ==========================================================================
// Automatic flight path planning member functions
// ==========================================================================

// Member function compute_optimal_clockwise_circular_flightpath()

double LOSMODELSGROUP::compute_optimal_clockwise_circular_flightpath(
   double& center_longitude,double& center_latitude,double& orbit_radius)
{
   cout << "inside LOSMODELSGROUP::compute_optimal_clockwise_circular_flightpath()"
        << endl;

   unsigned int n_iters=10;
   unsigned int n_samples=10000;
   double best_frac=0.05;
   double delta_frac=best_frac/n_iters;
   const double alpha=0.9;
//   const double alpha=1.0;

   bool northern_hemisphere_flag=get_EarthRegionsGroup_ptr()->
      get_northern_hemisphere_flag();
   int UTM_zone=get_EarthRegionsGroup_ptr()->get_specified_UTM_zonenumber();

   vector<double> xc_vars,yc_vars,r_vars;
   vector<double> xc_best,yc_best,r_best;
   
   bool terminate_flag=false;

   double avg_score=0;
   for (unsigned int iter=0; iter<n_iters && !terminate_flag; iter++)
   {
      cout << "iter = " << iter << endl;
      
      xc_vars.clear();
      yc_vars.clear();
      r_vars.clear();
      generate_circle_orbit_param_samples(
         iter,n_samples,xc_vars,yc_vars,r_vars);

      vector<double> candidate_circle_orbit_scores;
      for (unsigned int n=0; n<n_samples; n++)
      {
         double xc=xc_vars[n];
         double yc=yc_vars[n];
         double r=r_vars[n];
         double curr_score=candidate_circle_orbit_score(xc,yc,r);
         candidate_circle_orbit_scores.push_back(curr_score);
      } // loop over index n labeling samples

      templatefunc::Quicksort_descending(
         candidate_circle_orbit_scores,xc_vars,yc_vars,r_vars);
      
      xc_best.clear();
      yc_best.clear();
      r_best.clear();

      avg_score=0;
      unsigned int n_best_samples=best_frac*n_samples;
      cout << "best_frac = " << best_frac
           << " n_best_samples = " << n_best_samples << endl;
      best_frac -= delta_frac;
      for (unsigned int n=0; n<n_best_samples; n++)
      {
         if (candidate_circle_orbit_scores[n] <= 0) continue;

         xc_best.push_back(candidate_circle_orbit_scores[n]*xc_vars[n]);
         yc_best.push_back(candidate_circle_orbit_scores[n]*yc_vars[n]);
         r_best.push_back(candidate_circle_orbit_scores[n]*r_vars[n]);
         avg_score += candidate_circle_orbit_scores[n];

         double center_easting=xc_vars[n]+get_grid_world_origin().get(0);
         double center_northing=yc_vars[n]+get_grid_world_origin().get(1);
         geopoint flight_center(northern_hemisphere_flag,UTM_zone,
                                center_easting,center_northing);

         if (n < 5)
         {
            cout << "n=" << n
                 << " score=" << candidate_circle_orbit_scores[n]
                 << " r=" << r_vars[n]/1000 
                 << " lon=" << flight_center.get_longitude() 
                 << " lat=" << flight_center.get_latitude() << endl;
         }
      } // loop over index n labeling best candidate flight paths

      avg_score /= n_best_samples;
      cout << "avg_score = " << avg_score << endl;
      cout << "xc_best.size() = " << xc_best.size() << endl;
      
      double new_mu_xc=mathfunc::mean(xc_best)/avg_score;
      double new_sigma_xc=mathfunc::std_dev(xc_best)/avg_score;
      double new_mu_yc=mathfunc::mean(yc_best)/avg_score;
      double new_sigma_yc=mathfunc::std_dev(yc_best)/avg_score;
      double new_mu_r=mathfunc::mean(r_best)/avg_score;
      double new_sigma_r=mathfunc::std_dev(r_best)/avg_score;

      mu_xc=alpha*new_mu_xc+(1-alpha)*mu_xc;
      sigma_xc=alpha*new_sigma_xc+(1-alpha)*sigma_xc;
      mu_yc=alpha*new_mu_yc+(1-alpha)*mu_yc;
      sigma_yc=alpha*new_sigma_yc+(1-alpha)*sigma_yc;
      mu_r=alpha*new_mu_r+(1-alpha)*mu_r;
      sigma_r=alpha*new_sigma_r+(1-alpha)*sigma_r;

      cout << "xc = " << mu_xc << " +/- " << sigma_xc << endl;
      cout << "yc = " << mu_yc << " +/- " << sigma_yc << endl;
      cout << "r = " << mu_r/1000 << " +/- " << sigma_r/1000 << endl;
      cout << "xc_best.size() = " << xc_best.size() << endl;

      double center_easting=mu_xc+get_grid_world_origin().get(0);
      double center_northing=mu_yc+get_grid_world_origin().get(1);
      geopoint flight_center(northern_hemisphere_flag,UTM_zone,
      center_easting,center_northing);
      
      cout << "iter = " << iter
           << " best candidate score = " << candidate_circle_orbit_scores[0]
           << " avg score = " << avg_score << endl;
      cout << "center_easting = " << center_easting
           << " center_northing = " << center_northing << endl;
      cout << "center.lon = " << flight_center.get_longitude() 
           << " center.lat = " << flight_center.get_latitude() << endl;

      double automated_path_progress=double(iter+1)/n_iters;
      string progress_type="automated path";
      viewer_Messenger_ptr->broadcast_progress(
         automated_path_progress,progress_type);

      if (sigma_xc < 10*1000 && sigma_yc < 10*1000 && sigma_r < 10*1000)
         terminate_flag=true;
      
   } // loop over iter index

   double center_easting=mu_xc+get_grid_world_origin().get(0);
   double center_northing=mu_yc+get_grid_world_origin().get(1);

   geopoint flight_center(northern_hemisphere_flag,UTM_zone,
	   center_easting,center_northing);
   cout << "Final flight_center = " << flight_center << endl;
   cout << "r = " << mu_r/1000 << " +/- " << sigma_r/1000 << endl;

   center_longitude=flight_center.get_longitude();
   center_latitude=flight_center.get_latitude();
   orbit_radius=mu_r;	// meters

   return avg_score;
}

// --------------------------------------------------------------------
// Member function compute_optimal_clockwise_ellipse_params()

double LOSMODELSGROUP::compute_optimal_clockwise_ellipse_params(
   double& center_longitude,double& center_latitude,
   double& ellipse_a,double& ellipse_b,double& ellipse_phi)
{
   cout << "inside LOSMODELSGROUP::compute_optimal_clockwise_ellipse_params()"
        << endl;

   unsigned int n_iters=10;
   unsigned int n_samples=10000;
   double best_frac=0.05;
   double delta_frac=best_frac/n_iters;
   const double alpha=0.9;

   bool northern_hemisphere_flag=get_EarthRegionsGroup_ptr()->
      get_northern_hemisphere_flag();
   int UTM_zone=get_EarthRegionsGroup_ptr()->get_specified_UTM_zonenumber();

   vector<double> xc_vars,yc_vars,a_vars,b_vars,phi_vars;
   vector<double> xc_best,yc_best,a_best,b_best,phi_best;

   bool terminate_flag=false;
   double avg_score=0;
   for (unsigned int iter=0; iter<n_iters && !terminate_flag; iter++)
   {
      cout << "iter = " << iter << endl;
      
      xc_vars.clear();
      yc_vars.clear();
      a_vars.clear();
      b_vars.clear();
      phi_vars.clear();
      generate_ellipse_orbit_param_samples(
         iter,n_samples,xc_vars,yc_vars,a_vars,b_vars,phi_vars);

      vector<double> candidate_orbit_scores;
      for (unsigned int n=0; n<n_samples; n++)
      {
         double xc=xc_vars[n];
         double yc=yc_vars[n];
         double a=a_vars[n];
         double b=b_vars[n];
         double phi=phi_vars[n];
         double curr_score=candidate_ellipse_orbit_score(xc,yc,a,b,phi);
         candidate_orbit_scores.push_back(curr_score);
      } // loop over index n labeling samples

      templatefunc::Quicksort_descending(
         candidate_orbit_scores,xc_vars,yc_vars,a_vars,b_vars,phi_vars);
      
      xc_best.clear();
      yc_best.clear();
      a_best.clear();
      b_best.clear();
      phi_best.clear();
      avg_score=0;
      unsigned int n_best_samples=best_frac*n_samples;
      cout << "best_frac = " << best_frac
           << " n_best_samples = " << n_best_samples << endl;
      best_frac -= delta_frac;
      for (unsigned int n=0; n<n_best_samples; n++)
      {
         if (candidate_orbit_scores[n] <= 0) continue;

         xc_best.push_back(candidate_orbit_scores[n]*xc_vars[n]);
         yc_best.push_back(candidate_orbit_scores[n]*yc_vars[n]);
         a_best.push_back(candidate_orbit_scores[n]*a_vars[n]);
         b_best.push_back(candidate_orbit_scores[n]*b_vars[n]);
         phi_best.push_back(candidate_orbit_scores[n]*phi_vars[n]);
         avg_score += candidate_orbit_scores[n];

         double center_easting=xc_vars[n]+get_grid_world_origin().get(0);
         double center_northing=yc_vars[n]+get_grid_world_origin().get(1);
         geopoint flight_center(northern_hemisphere_flag,UTM_zone,
                                center_easting,center_northing);

         if (n < 5)
         {
            cout << "n=" << n
                 << " score=" << candidate_orbit_scores[n]
                 << " a=" << a_vars[n]/1000 
                 << " b=" << b_vars[n]/1000
                 << " lon=" << flight_center.get_longitude() 
                 << " lat=" << flight_center.get_latitude() << endl;
         }
         
      } // loop over index n labeling best candidate flight paths
      avg_score /= n_best_samples;
      cout << "avg_score = " << avg_score << endl;

      double new_mu_xc=mathfunc::mean(xc_best)/avg_score;
      double new_sigma_xc=mathfunc::std_dev(xc_best)/avg_score;
      double new_mu_yc=mathfunc::mean(yc_best)/avg_score;
      double new_sigma_yc=mathfunc::std_dev(yc_best)/avg_score;
      double new_mu_a=mathfunc::mean(a_best)/avg_score;
      double new_sigma_a=mathfunc::std_dev(a_best)/avg_score;
      double new_mu_b=mathfunc::mean(b_best)/avg_score;
      double new_sigma_b=mathfunc::std_dev(b_best)/avg_score;
      double new_mu_phi=mathfunc::mean(phi_best)/avg_score;
      double new_sigma_phi=mathfunc::std_dev(phi_best)/avg_score;

      mu_xc=alpha*new_mu_xc+(1-alpha)*mu_xc;
      sigma_xc=alpha*new_sigma_xc+(1-alpha)*sigma_xc;
      mu_yc=alpha*new_mu_yc+(1-alpha)*mu_yc;
      sigma_yc=alpha*new_sigma_yc+(1-alpha)*sigma_yc;
      mu_a=alpha*new_mu_a+(1-alpha)*mu_a;
      sigma_a=alpha*new_sigma_a+(1-alpha)*sigma_a;
      mu_b=alpha*new_mu_b+(1-alpha)*mu_b;
      sigma_b=alpha*new_sigma_b+(1-alpha)*sigma_b;
      mu_phi=alpha*new_mu_phi+(1-alpha)*mu_phi;
      sigma_phi=alpha*new_sigma_phi+(1-alpha)*sigma_phi;

      cout << "xc = " << mu_xc << " +/- " << sigma_xc << endl;
      cout << "yc = " << mu_yc << " +/- " << sigma_yc << endl;
      cout << "a = " << mu_a/1000 << " +/- " << sigma_a/1000 << endl;
      cout << "b = " << mu_b/1000 << " +/- " << sigma_b/1000 << endl;
      cout << "phi = " << mu_phi*180/PI << " +/- " << sigma_phi*180/PI << endl;
      cout << "xc_best.size() = " << xc_best.size() << endl;

      double center_easting=mu_xc+get_grid_world_origin().get(0);
      double center_northing=mu_yc+get_grid_world_origin().get(1);
      geopoint flight_center(northern_hemisphere_flag,UTM_zone,
      center_easting,center_northing);
      
      cout << "iter = " << iter
           << " candidate score = " << candidate_orbit_scores[0] << endl;
      cout << "center.lon = " << flight_center.get_longitude() 
           << " center.lat = " << flight_center.get_latitude() << endl;

      double automated_path_progress=double(iter+1)/n_iters;
      string progress_type="automated path";
      viewer_Messenger_ptr->broadcast_progress(
         automated_path_progress,progress_type);

      if (sigma_xc < 10*1000 && sigma_yc < 10*1000 && sigma_a < 2*1000 &&
          sigma_b < 2*1000)
         terminate_flag=true;
      
   } // loop over iter index

   double center_easting=mu_xc+get_grid_world_origin().get(0);
   double center_northing=mu_yc+get_grid_world_origin().get(1);

   geopoint flight_center(northern_hemisphere_flag,UTM_zone,
	   center_easting,center_northing);

   center_longitude=flight_center.get_longitude();
   center_latitude=flight_center.get_latitude();
   ellipse_a=mu_a;	
   ellipse_b=mu_b;	
   ellipse_phi=mu_phi;	

   return avg_score;
}

// --------------------------------------------------------------------
// Member function compute_optimal_linesegment_params()

double LOSMODELSGROUP::compute_optimal_linesegment_params(
   twovector& r1,twovector& r2)
{
   cout << "inside LOSMODELSGROUP::compute_optimal_linesegment_params()"
        << endl;

   unsigned int n_iters=20;
//   int n_samples=20000;
   unsigned int n_samples=25000;
   double best_frac=0.05;
   double delta_frac=best_frac/n_iters;
   const double alpha=0.9;

   vector<double> xc_vars,yc_vars,r_vars,theta_vars;
   vector<double> xc_best,yc_best,r_best,theta_best;
   
   bool terminate_flag=false;
   double avg_score=0;
   for (unsigned int iter=0; iter<n_iters && !terminate_flag; iter++)
   {
      cout << "iter = " << iter << endl;
      
      xc_vars.clear();
      yc_vars.clear();
      r_vars.clear();
      theta_vars.clear();

      generate_linesegment_param_samples(
         iter,n_samples,xc_vars,yc_vars,r_vars,theta_vars);

      vector<double> candidate_orbit_scores;
      for (unsigned int n=0; n<n_samples; n++)
      {
         double xc=xc_vars[n];
         double yc=yc_vars[n];
         double r=r_vars[n];
         double theta=theta_vars[n];

         double rcostheta=r*cos(theta);
         double rsintheta=r*sin(theta);
         double x1=xc+rcostheta;
         double y1=yc+rsintheta;
         double x2=xc-rcostheta;
         double y2=yc-rsintheta;
         
         double curr_score=candidate_linesegment_score(x1,y1,x2,y2);
         candidate_orbit_scores.push_back(curr_score);
      } // loop over index n labeling samples

      templatefunc::Quicksort_descending(
         candidate_orbit_scores,xc_vars,yc_vars,r_vars,theta_vars);
      
      xc_best.clear();
      yc_best.clear();
      r_best.clear();
      theta_best.clear();
      avg_score=0;
      unsigned int n_best_samples=best_frac*n_samples;
      cout << "best_frac = " << best_frac
           << " n_best_samples = " << n_best_samples << endl;
      best_frac -= delta_frac;
      for (unsigned int n=0; n<n_best_samples; n++)
      {
         if (candidate_orbit_scores[n] <= 0) continue;
         
         xc_best.push_back(xc_vars[n]);
         yc_best.push_back(yc_vars[n]);
         r_best.push_back(r_vars[n]);
         theta_best.push_back(theta_vars[n]);
         avg_score += candidate_orbit_scores[n];

         if (n < 5)
         {
            cout << "n=" << n
                 << " score=" << candidate_orbit_scores[n]
                 << " x=" << xc_vars[n]
                 << " y=" << yc_vars[n]
                 << " r=" << r_vars[n]
                 << " theta=" << theta_vars[n] << endl;
         }
      } // loop over index n labeling best candidate flight paths
      avg_score /= n_best_samples;
      cout << "avg_score = " << avg_score << endl;

      double new_mu_xc=mathfunc::mean(xc_best);
      double new_sigma_xc=mathfunc::std_dev(xc_best);
      double new_mu_yc=mathfunc::mean(yc_best);
      double new_sigma_yc=mathfunc::std_dev(yc_best);
      double new_mu_r=mathfunc::mean(r_best);
      double new_sigma_r=mathfunc::std_dev(r_best);
      double new_mu_theta=mathfunc::mean(theta_best);
      double new_sigma_theta=mathfunc::std_dev(theta_best);

      mu_xc=alpha*new_mu_xc+(1-alpha)*mu_xc;
      sigma_xc=alpha*new_sigma_xc+(1-alpha)*sigma_xc;
      mu_yc=alpha*new_mu_yc+(1-alpha)*mu_yc;
      sigma_yc=alpha*new_sigma_yc+(1-alpha)*sigma_yc;
      mu_r=alpha*new_mu_r+(1-alpha)*mu_r;
      sigma_r=alpha*new_sigma_r+(1-alpha)*sigma_r;
      mu_theta=alpha*new_mu_theta+(1-alpha)*mu_theta;
      sigma_theta=alpha*new_sigma_theta+(1-alpha)*sigma_theta;

      cout << "xc = " << mu_xc << " +/- " << sigma_xc << endl;
      cout << "yc = " << mu_yc << " +/- " << sigma_yc << endl;
      cout << "r = " << mu_r << " +/- " << sigma_r << endl;
      cout << "theta = " << mu_theta*180/PI << " +/- " 
           << sigma_theta*180/PI << endl;
      cout << "xc_best.size() = " << xc_best.size() << endl;

      cout << "iter = " << iter
           << " best candidate score = " << candidate_orbit_scores[0]
           << endl;

      twovector center(mu_xc,mu_yc);
      r1=center+mu_r*twovector(cos(mu_theta),sin(mu_theta));
      r2=center-mu_r*twovector(cos(mu_theta),sin(mu_theta));
//      cout << "r1 = " << r1 << " r2 = " << r2 << endl;
      cout << "pathlength = " << (r2-r1).magnitude() << endl;

      double automated_path_progress=double(iter+1)/n_iters;
      string progress_type="automated path";
      viewer_Messenger_ptr->broadcast_progress(
         automated_path_progress,progress_type);

      if (sigma_xc < 5*1000 && sigma_yc < 5*1000 && sigma_r < 5*1000 &&
          sigma_theta < 2*PI/180)
         terminate_flag=true;
      
   } // loop over iter index

   return avg_score;
}

/*
// --------------------------------------------------------------------
// Member function compute_optimal_linesegment_params()

double LOSMODELSGROUP::compute_optimal_linesegment_params(
   twovector& r1,twovector& r2)
{
   cout << "inside LOSMODELSGROUP::compute_optimal_linesegment_params()"
        << endl;

   int n_iters=25;
//   int n_samples=20000;
   int n_samples=25000;
   double best_frac=0.05;
   double delta_frac=best_frac/n_iters;
   const double alpha=0.9;

   vector<double> x1_vars,y1_vars,x2_vars,y2_vars;
   vector<double> x1_best,y1_best,x2_best,y2_best;
   
   bool terminate_flag=false;
   double avg_score=0;
   for (unsigned int iter=0; iter<n_iters && !terminate_flag; iter++)
   {
      cout << "iter = " << iter << endl;
      
      x1_vars.clear();
      y1_vars.clear();
      x2_vars.clear();
      y2_vars.clear();

      generate_linesegment_param_samples(
         iter,n_samples,x1_vars,y1_vars,x2_vars,y2_vars);

      vector<double> candidate_orbit_scores;
      for (unsigned int n=0; n<n_samples; n++)
      {
         double x1=x1_vars[n];
         double y1=y1_vars[n];
         double x2=x2_vars[n];
         double y2=y2_vars[n];
         double curr_score=candidate_linesegment_score(x1,y1,x2,y2);
         candidate_orbit_scores.push_back(curr_score);
      } // loop over index n labeling samples

      templatefunc::Quicksort_descending(
         candidate_orbit_scores,x1_vars,y1_vars,x2_vars,y2_vars);
      
      x1_best.clear();
      y1_best.clear();
      x2_best.clear();
      y2_best.clear();
      avg_score=0;
      int n_best_samples=best_frac*n_samples;
      cout << "best_frac = " << best_frac
           << " n_best_samples = " << n_best_samples << endl;
      best_frac -= delta_frac;
      for (unsigned int n=0; n<n_best_samples; n++)
      {
         if (candidate_orbit_scores[n] <= 0) continue;
         
//         x1_best.push_back(candidate_orbit_scores[n]*x1_vars[n]);
//         y1_best.push_back(candidate_orbit_scores[n]*y1_vars[n]);
//         x2_best.push_back(candidate_orbit_scores[n]*x2_vars[n]);
//         y2_best.push_back(candidate_orbit_scores[n]*y2_vars[n]);
//         avg_score += candidate_orbit_scores[n];

         x1_best.push_back(x1_vars[n]);
         y1_best.push_back(y1_vars[n]);
         x2_best.push_back(x2_vars[n]);
         y2_best.push_back(y2_vars[n]);
         avg_score += candidate_orbit_scores[n];

         if (n < 5)
         {
            cout << "n=" << n
                 << " score=" << candidate_orbit_scores[n]
                 << " x1=" << x1_vars[n]
                 << " y1=" << y1_vars[n]
                 << " x2=" << x2_vars[n]
                 << " y2=" << y2_vars[n] << endl;
         }
      } // loop over index n labeling best candidate flight paths
      avg_score /= n_best_samples;
      cout << "avg_score = " << avg_score << endl;

//      double new_mu_x1=mathfunc::mean(x1_best)/avg_score;
//      double new_sigma_x1=mathfunc::std_dev(x1_best)/avg_score;
//      double new_mu_y1=mathfunc::mean(y1_best)/avg_score;
//      double new_sigma_y1=mathfunc::std_dev(y1_best)/avg_score;
//      double new_mu_x2=mathfunc::mean(x2_best)/avg_score;
//      double new_sigma_x2=mathfunc::std_dev(x2_best)/avg_score;
//      double new_mu_y2=mathfunc::mean(y2_best)/avg_score;
//      double new_sigma_y2=mathfunc::std_dev(y2_best)/avg_score;

      double new_mu_x1=mathfunc::mean(x1_best);
      double new_sigma_x1=mathfunc::std_dev(x1_best);
      double new_mu_y1=mathfunc::mean(y1_best);
      double new_sigma_y1=mathfunc::std_dev(y1_best);
      double new_mu_x2=mathfunc::mean(x2_best);
      double new_sigma_x2=mathfunc::std_dev(x2_best);
      double new_mu_y2=mathfunc::mean(y2_best);
      double new_sigma_y2=mathfunc::std_dev(y2_best);

      mu_x1=alpha*new_mu_x1+(1-alpha)*mu_x1;
      sigma_x1=alpha*new_sigma_x1+(1-alpha)*sigma_x1;
      mu_y1=alpha*new_mu_y1+(1-alpha)*mu_y1;
      sigma_y1=alpha*new_sigma_y1+(1-alpha)*sigma_y1;
      mu_x2=alpha*new_mu_x2+(1-alpha)*mu_x2;
      sigma_x2=alpha*new_sigma_x2+(1-alpha)*sigma_x2;
      mu_y2=alpha*new_mu_y2+(1-alpha)*mu_y2;
      sigma_y2=alpha*new_sigma_y2+(1-alpha)*sigma_y2;

      cout << "x1 = " << mu_x1 << " +/- " << sigma_x1 << endl;
      cout << "y1 = " << mu_y1 << " +/- " << sigma_y1 << endl;
      cout << "x2 = " << mu_x2 << " +/- " << sigma_x2 << endl;
      cout << "y2 = " << mu_y2 << " +/- " << sigma_y2 << endl;
      cout << "x1_best.size() = " << x1_best.size() << endl;

      cout << "iter = " << iter
           << " best candidate score = " << candidate_orbit_scores[0]
           << endl;

      r1=twovector(mu_x1,mu_y1);
      r2=twovector(mu_x2,mu_y2);
//      cout << "r1 = " << r1 << " r2 = " << r2 << endl;
      cout << "pathlength = " << (r2-r1).magnitude() << endl;

      double automated_path_progress=double(iter+1)/n_iters;
      string progress_type="automated path";
      viewer_Messenger_ptr->broadcast_progress(
         automated_path_progress,progress_type);

      if (sigma_x1 < 10*1000 && sigma_y1 < 10*1000 && sigma_x2 < 10*1000 &&
          sigma_y2 < 10*1000)
         terminate_flag=true;
      
   } // loop over iter index

   return avg_score;
}
*/

// --------------------------------------------------------------------
// Member function generate_circle_orbit_param_samples()

void LOSMODELSGROUP::generate_circle_orbit_param_samples(
   int iter,int n_samples,vector<double>& xc_vars,vector<double>& yc_vars,
   vector<double>& r_vars)
{
//   cout << "inside LOSMODELSGROUP::generate_circle_orbit_param_samples()"
//        << endl;

   if (iter==0)
   {
      xc_min=skymap_xlo;
      xc_max=skymap_xhi;
      yc_min=skymap_ylo;
      yc_max=skymap_yhi;
      r_max=basic_math::max(
         0.75*(skymap_xhi-skymap_xlo),
         0.75*(skymap_yhi-skymap_ylo));
      r_min=basic_math::min(
         0.1*(skymap_xhi-skymap_xlo),
         0.1*(skymap_yhi-skymap_ylo));
         
      initialize_cross_entropy_distribution_params(
         xc_min,xc_max,mu_xc,sigma_xc);
      initialize_cross_entropy_distribution_params(
         yc_min,yc_max,mu_yc,sigma_yc);
      initialize_cross_entropy_distribution_params(
         r_min,r_max,mu_r,sigma_r);
//      cout << "xc_min = " << xc_min << " xc_max = " << xc_max
//           << " mu_xc = " << mu_xc << " sigma_xc = " << sigma_xc << endl;
//      cout << "yc_min = " << yc_min << " yc_max = " << yc_max
//           << " mu_yc = " << mu_yc << " sigma_yc = " << sigma_yc << endl;
//      cout << "r_min = " << r_min/1000 << " r_max = " << r_max/1000
//           << " mu_r = " << mu_r/1000 << " sigma_r = " << sigma_r/1000 
//           << endl;
   }

   xc_vars=generate_random_vars(n_samples,mu_xc,sigma_xc,xc_min,xc_max);
   yc_vars=generate_random_vars(n_samples,mu_yc,sigma_yc,yc_min,yc_max);
   r_vars=generate_random_vars(n_samples,mu_r,sigma_r,r_min,r_max);
}

// --------------------------------------------------------------------
// Member function generate_ellipse_orbit_param_samples()

void LOSMODELSGROUP::generate_ellipse_orbit_param_samples(
   int iter,int n_samples,vector<double>& xc_vars,vector<double>& yc_vars,
   vector<double>& a_vars,vector<double>& b_vars,vector<double>& phi_vars)
{
//   cout << "inside LOSMODELSGROUP::generate_ellipse_orbit_param_samples()"
//        << endl;

   if (iter==0)
   {
      xc_min=skymap_xlo;
      xc_max=skymap_xhi;
      yc_min=skymap_ylo;
      yc_max=skymap_yhi;
      a_max=basic_math::max(
         0.75*(skymap_xhi-skymap_xlo),
         0.75*(skymap_yhi-skymap_ylo));
      a_min=basic_math::min(
         0.1*(skymap_xhi-skymap_xlo),
         0.1*(skymap_yhi-skymap_ylo));
      b_max=a_max;
      b_min=a_min;
      phi_min=0;
      phi_max=2*PI;
         
      initialize_cross_entropy_distribution_params(
         xc_min,xc_max,mu_xc,sigma_xc);
      initialize_cross_entropy_distribution_params(
         yc_min,yc_max,mu_yc,sigma_yc);
      initialize_cross_entropy_distribution_params(
         a_min,a_max,mu_a,sigma_a);
      initialize_cross_entropy_distribution_params(
         b_min,b_max,mu_b,sigma_b);
      initialize_cross_entropy_distribution_params(
         phi_min,phi_max,mu_phi,sigma_phi);

//      cout << "xc_min = " << xc_min << " xc_max = " << xc_max
//           << " mu_xc = " << mu_xc << " sigma_xc = " << sigma_xc << endl;
//      cout << "yc_min = " << yc_min << " yc_max = " << yc_max
//           << " mu_yc = " << mu_yc << " sigma_yc = " << sigma_yc << endl;
//      cout << "a_min = " << a_min/1000 << " a_max = " << a_max/1000
//           << " mu_a = " << mu_a/1000 << " sigma_a = " << sigma_a/1000 
//           << endl;
//      cout << "b_min = " << b_min/1000 << " b_max = " << b_max/1000
//           << " mu_b = " << mu_b/1000 << " sigma_b = " << sigma_b/1000 
//           << endl;
//      cout << "phi_min = " << phi_min/1000 << " phi_max = " << phi_max/1000
//           << " mu_phi = " << mu_phi/1000 << " sigma_phi = " << sigma_phi/1000 
//           << endl;
   }

   xc_vars=generate_random_vars(n_samples,mu_xc,sigma_xc,xc_min,xc_max);
   yc_vars=generate_random_vars(n_samples,mu_yc,sigma_yc,yc_min,yc_max);
   a_vars=generate_random_vars(n_samples,mu_a,sigma_a,a_min,a_max);
   b_vars=generate_random_vars(n_samples,mu_b,sigma_b,b_min,b_max);
   phi_vars=generate_random_vars(n_samples,mu_phi,sigma_phi,phi_min,phi_max);
}

// --------------------------------------------------------------------
// Member function generate_linesegment_param_samples()

void LOSMODELSGROUP::generate_linesegment_param_samples(
   int iter,int n_samples,vector<double>& xc_vars,vector<double>& yc_vars,
   vector<double>& r_vars,vector<double>& theta_vars)
{
//   cout << "inside LOSMODELSGROUP::generate_linesegment_param_samples()"
//        << endl;

   if (iter==0)
   {
      xc_min=skymap_xlo;
      xc_max=skymap_xhi;
      yc_min=skymap_ylo;
      yc_max=skymap_yhi;

      r_min=0;
      r_max=0.5*sqrt(sqr(skymap_xhi-skymap_xlo)+sqr(skymap_yhi-skymap_ylo));
      theta_min=0;
      theta_max=2*PI;
         
      initialize_cross_entropy_distribution_params(
         xc_min,xc_max,mu_xc,sigma_xc);
      initialize_cross_entropy_distribution_params(
         yc_min,yc_max,mu_yc,sigma_yc);
      initialize_cross_entropy_distribution_params(
         r_min,r_max,mu_r,sigma_r);
      initialize_cross_entropy_distribution_params(
         theta_min,theta_max,mu_theta,sigma_theta);

      sigma_xc *= 2;
      sigma_yc *= 2;
      sigma_r *= 2;
      sigma_theta *= 2;
   }

   xc_vars=generate_random_vars(n_samples,mu_xc,sigma_xc,xc_min,xc_max);
   yc_vars=generate_random_vars(n_samples,mu_yc,sigma_yc,yc_min,yc_max);
   r_vars=generate_random_vars(n_samples,mu_r,sigma_r,r_min,r_max);
   theta_vars=generate_random_vars(n_samples,mu_theta,sigma_theta,
      theta_min,theta_max);
}

/*
// --------------------------------------------------------------------
// Member function generate_linesegment_param_samples()

void LOSMODELSGROUP::generate_linesegment_param_samples(
   int iter,int n_samples,vector<double>& x1_vars,vector<double>& y1_vars,
   vector<double>& x2_vars,vector<double>& y2_vars)
{
//   cout << "inside LOSMODELSGROUP::generate_linesegment_param_samples()"
//        << endl;

   if (iter==0)
   {
      x1_min=skymap_xlo;
      x1_max=skymap_xhi;
      y1_min=skymap_ylo;
      y1_max=skymap_yhi;

      x2_min=skymap_xlo;
      x2_max=skymap_xhi;
      y2_min=skymap_ylo;
      y2_max=skymap_yhi;
         
      initialize_cross_entropy_distribution_params(
         x1_min,x1_max,mu_x1,sigma_x1);
      initialize_cross_entropy_distribution_params(
         y1_min,y1_max,mu_y1,sigma_y1);
      initialize_cross_entropy_distribution_params(
         x2_min,x2_max,mu_x2,sigma_x2);
      initialize_cross_entropy_distribution_params(
         y2_min,y2_max,mu_y2,sigma_y2);

      sigma_x1 *= 2;
      sigma_y1 *= 2;
      sigma_x2 *= 2;
      sigma_y2 *= 2;
   }

   x1_vars=generate_random_vars(n_samples,mu_x1,sigma_x1,x1_min,x1_max);
   y1_vars=generate_random_vars(n_samples,mu_y1,sigma_y1,y1_min,y1_max);
   x2_vars=generate_random_vars(n_samples,mu_x2,sigma_x2,x2_min,x2_max);
   y2_vars=generate_random_vars(n_samples,mu_y2,sigma_y2,y2_min,y2_max);
}
*/

// --------------------------------------------------------------------
// Member function initialize_cross_entropy_distribution_params()

void LOSMODELSGROUP::initialize_cross_entropy_distribution_params(
   double xmin,double xmax,double& mu,double& sigma)
{
   mu=0.5*(xmin+xmax);
   sigma=(xmax-xmin)/6.0;
}

// ---------------------------------------------------------------------
// Member function generate_random_vars() generates a set of
// random variables drawn from a gaussian distribution with mean mu
// and standard deviation sigma.  Random variables are forced to lie
// within the interval [xmin,xmax].  The set of random variables is
// returned within an output STL vector.

vector<double> LOSMODELSGROUP::generate_random_vars(
   unsigned int n_samples,double mu,double sigma,double xmin,double xmax)
{
//   cout << "inside LOSMODELSGROUP::generate_random_vars()" << endl;

   vector<double> X;
   while (X.size() < n_samples)
   {
      double curr_x=mu+sigma*nrfunc::gasdev();
      if (curr_x > xmin && curr_x < xmax)
      {
         X.push_back(curr_x);
      }
   } // while loop
   return X;
}

// --------------------------------------------------------------------
// Member function candidate_circle_orbit_score() takes in 2D center
// and radius parameters for a circular flight path orbit.  It
// computes and returns the score function defined by Ken Lee in his
// 12 page "Generating dynamically feasible parameteric paths to
// maximize visibility of multiple targets in heavily occluded
// terrain" July 6, 2010 writeup.

double LOSMODELSGROUP::candidate_circle_orbit_score(
   double xc,double yc,double r)
{
//   cout << "inside LOSMODELSGROUP::candidate_circle_orbit_score()" << endl;
//   cout << "xc = " << xc << " yc = " << yc << " r = " << r << endl;

   const double dtheta=5*PI/180;
   unsigned int n_theta_bins=2*PI/dtheta;
//   cout << "n_theta_bins = " << n_theta_bins << endl;

   double pathlength=2*PI*r;
   double ds=r*dtheta;
   double score_integral=0;

   unsigned int n_ground_targets=GroundTarget_SignPostsGroup_ptr->
      get_n_Graphicals();
//   cout << "n_ground_targets = " << n_ground_targets << endl;
   
   for (unsigned int g=0; g<n_ground_targets; g++)
   {
      double target_weight=1.0;
      for (unsigned int n=0; n<n_theta_bins; n++)
      {
         double theta=n*dtheta;
         double x=xc+r*cos(theta);
         double y=yc+r*sin(theta);
         double az_heading=(theta-PI/2)*180/PI;
         double curr_visibility=get_interpolated_groundtarget_visibility_flag(
            x,y,az_heading,g);

         if (curr_visibility > 0)
         {
//            cout << "n = " << n << " theta = " << theta*180/PI 
//                 << " curr_vis = " << curr_visibility << endl;
         }
       
         score_integral += target_weight*curr_visibility*ds;
      } // loop over index n labeling theta bins
   } // loop over index g labeling ground targets
   double score=score_integral/pathlength;

//   cout << "score = " << score << endl;
   return score;
}

// --------------------------------------------------------------------
// Member function candidate_ellipse_orbit_score() takes in 2D center
// and parameters for an elliptical flight path orbit.  It
// computes and returns the score function defined by Ken Lee in his
// 12 page "Generating dynamically feasible parameteric paths to
// maximize visibility of multiple targets in heavily occluded
// terrain" July 6, 2010 writeup.

double LOSMODELSGROUP::candidate_ellipse_orbit_score(
   double xc,double yc,double a,double b,double phi)
{
//   cout << "inside LOSMODELSGROUP::candidate_ellipse_orbit_score()" << endl;
//   cout << "xc = " << xc << " yc = " << yc << " a = " << a 
//        << " b = " << b << " phi = " << phi*180/PI << endl;

   if (a < b) templatefunc::swap(a,b);

   const double dtheta=5*PI/180;
   unsigned int n_theta_bins=2*PI/dtheta;
//   cout << "n_theta_bins = " << n_theta_bins << endl;

   double score_integral=0;
   double pathlength=0;
   unsigned int n_ground_targets=GroundTarget_SignPostsGroup_ptr->
      get_n_Graphicals();
//   cout << "n_ground_targets = " << n_ground_targets << endl;
   for (unsigned int g=0; g<n_ground_targets; g++)
   {
      double target_weight=1.0;
      pathlength=0;
      for (unsigned int n=0; n<n_theta_bins; n++)
      {
         double theta=n*dtheta;
         double x=xc+a*cos(phi)*cos(theta)-b*sin(phi)*sin(theta);
         double y=yc+a*sin(phi)*cos(theta)+b*cos(phi)*sin(theta);

	 double xdot=-a*cos(phi)*sin(theta)-b*sin(phi)*cos(theta);
         double ydot=-a*sin(phi)*sin(theta)+b*cos(phi)*cos(theta);
         double az_heading=(atan2(ydot,xdot)-PI)*180/PI;

         double curr_visibility=get_interpolated_groundtarget_visibility_flag(
            x,y,az_heading,g);

         if (curr_visibility > 0)
         {
//            cout << "n = " << n << " theta = " << theta*180/PI 
//                 << " curr_vis = " << curr_visibility << endl;
         }
         
         double r=sqrt(sqr(x)+sqr(y));
         double rdot=(x*xdot+y*ydot)/r;
         double sdot=sqrt(sqr(r)+sqr(rdot));
         double ds=sdot*dtheta;
         
         score_integral += target_weight*curr_visibility*ds;
         pathlength += ds;
         
      } // loop over index n labeling theta bins
   } // loop over index g labeling ground targets

   double score=score_integral/pathlength;

//   cout << "score = " << score << endl;
//   cout << "pathlength = " << pathlength << endl;
   return score;
}

// --------------------------------------------------------------------
// Member function candidate_linesegment_score() takes in endpoint coordinates 
// for a linesegment flight path.  It computes and returns the score
// function defined by Ken Lee in his 12 page "Generating dynamically
// feasible parameteric paths to maximize visibility of multiple
// targets in heavily occluded terrain" July 6, 2010 writeup.

double LOSMODELSGROUP::candidate_linesegment_score(
   double x1,double y1,double x2,double y2)
{
//   cout << "inside LOSMODELSGROUP::candidate_linesegment_score()" << endl;
//   cout << "x1 = " << x1 << " y1 = " << y1 << endl;
//   cout << "x2 = " << x2 << " y2 = " << y2 << endl;

   twovector r1(x1,y1);
   twovector r2(x2,y2);

   double pathlength=(r2-r1).magnitude();
   twovector ehat=(r2-r1).unitvector();
   double az_heading=atan2(ehat.get(1),ehat.get(0))*180/PI;
   
//   double ds=3000;	// meters
   double ds=0.4*skymap_ds;
   unsigned int n_sbins=pathlength/ds;
   double score_integral=0;

   unsigned int n_ground_targets
      =GroundTarget_SignPostsGroup_ptr->get_n_Graphicals();
//   cout << "n_ground_targets = " << n_ground_targets << endl;
   
   for (unsigned int g=0; g<n_ground_targets; g++)
   {
      double target_weight=1.0;
      for (unsigned int n=0; n<n_sbins; n++)
      {
         twovector r=r1+n*ds*ehat;

         double x=r.get(0);
         double y=r.get(1);
         double curr_visibility=get_interpolated_groundtarget_visibility_flag(
            x,y,az_heading,g);

         if (curr_visibility > 0)
         {
//            cout << "n = " << n << " theta = " << theta*180/PI 
//                 << " curr_vis = " << curr_visibility << endl;
         }
       
         score_integral += target_weight*curr_visibility*ds;
      } // loop over index n labeling theta bins
   } // loop over index g labeling ground targets
//   double score=score_integral/pathlength;

//   double score=score_integral;
   double score=score_integral/sqrt(pathlength);

//   cout << "score = " << score << endl;
   return score;
}

// --------------------------------------------------------------------
// Member function get_interpolated_groundtarget_visibility_flag()
// takes in continuous (x,y) UTM coordinates for some point within a
// skymap as well as a continuous azimuthal heading angle for the
// sensor measured in degrees.  It also takes in discrete index g
// labeling a particular ground target.  If the skypoint lies inside
// of pre-calculated skymaps, this method returns the visibility flag
// averaged over the two discrete heading twoDarrays.  

// This method is a very crude approximation for the 3D triple
// bilinear interpolation implemented by Ken Lee for automatic
// elliptical flightpath determination.

double LOSMODELSGROUP::get_interpolated_groundtarget_visibility_flag(
   double x,double y,double az_heading,int g)
{
//   cout << "inside LOSMODELSGROUP::get_interpolated_groundtarget_visibility_flag()" 
//        << endl;
   
   double flag=0;

   const double d_az=skymap_daz;     	// degs
   az_heading=basic_math::phase_to_canonical_interval(az_heading,0,360);
   int a_lo=basic_math::mytruncate(az_heading/d_az);
   int a_hi=a_lo+1;
   if (a_hi==8) a_hi=0;
   double az_frac=(az_heading-a_lo*d_az)/d_az;
   az_frac=basic_math::max(0.0,az_frac);

//   cout << "az_heading = " << az_heading << endl;
//   cout << "a_lo = " << a_lo << " a_hi = " << a_hi << endl;
//   cout << "az_frac = " << az_frac << endl;

   TARGET_SKYMAP_MAP::iterator tgt_skymap_lo_iter=
      target_skymap_map_ptr->find(twovector(a_lo,g));
   twoDarray* visibility_twoDarray_lo_ptr=tgt_skymap_lo_iter->second;
//   cout << "*visibility_twoDarray_lo_ptr = "
//        << *visibility_twoDarray_lo_ptr << endl;

   TARGET_SKYMAP_MAP::iterator tgt_skymap_hi_iter=
      target_skymap_map_ptr->find(twovector(a_hi,g));
   twoDarray* visibility_twoDarray_hi_ptr=tgt_skymap_hi_iter->second;
//   cout << "*visibility_twoDarray_hi_ptr = "
//        << *visibility_twoDarray_hi_ptr << endl;

//   cout << "grid_world_origin = " << get_grid_world_origin() << endl;

   double vflag_azlo,vflag_azhi;
   if (!visibility_twoDarray_lo_ptr->point_to_interpolated_value(
      x,y,vflag_azlo))
   {
      vflag_azlo=0;
   }

   if (!visibility_twoDarray_hi_ptr->point_to_interpolated_value(
      x,y,vflag_azhi))
   {
      vflag_azhi=0;
   }

   vflag_azlo=basic_math::max(0.0,vflag_azlo);
   vflag_azhi=basic_math::max(0.0,vflag_azhi);
   flag=az_frac*vflag_azlo+(1-az_frac)*vflag_azhi;

   if (flag > 1.0001)
   {
      cout << "Error!: flag = " << flag << endl;
      cout << "vflag_azlo = " << vflag_azlo << endl;
      cout << "vflag_azhi = " << vflag_azhi << endl;
      cout << "az_frac = " << az_frac << endl;
      outputfunc::enter_continue_char();
   }

   return flag;
}

// ---------------------------------------------------------------------
// Member function test_skymap_interpolation() is an ancillary method
// written for debugging purposes.

void LOSMODELSGROUP::test_skymap_interpolation()
{
   cout << "inside LOSMODELSGROUP::test_skymap_interpolation()" << endl;

   int g;
   double lon,lat,az;
   while (true)
   {
      cout << "Enter longitude:" << endl;
      cin >> lon;
      cout << "Enter latitude:" << endl;
      cin >> lat;
      
      geopoint curr_posn(lon,lat);
      double x=curr_posn.get_UTM_easting()-get_grid_world_origin().get(0);
      double y=curr_posn.get_UTM_northing()-get_grid_world_origin().get(1);
      
//      cout << "Enter x:" << endl;
//      cin >> x;
//      cout << "Enter y:" << endl;
//      cin >> y;
      cout << "Enter az in degs:" << endl;
      cin >> az;
      cout << "Enter ground target ID:" << endl;
      cin >> g;
      
      double visibility_flag=get_interpolated_groundtarget_visibility_flag(
         x,y,az,g);
      cout << "interpolated visibility flag = "
           << visibility_flag << endl << endl;
   }
}

// --------------------------------------------------------------------
// Member function generate_ellipse_flightpath() takes in the
// parameters for an elliptical flightpath.  It generates a PolyLine
// within the UTM zone specified in EarthRegionsGroup.  This method
// returns the length of ellipse flightpath in meters.

double LOSMODELSGROUP::generate_ellipse_flightpath(
   double center_longitude,double center_latitude,
   double a,double b,double phi)
{
//   cout << "inside LOSMODELSGROUP::generate_ellipse_flightpath()" << endl;

   geopoint ellipse_center(center_longitude,center_latitude);
   ellipse_center.recompute_UTM_coords(
      get_EarthRegionsGroup_ptr()->get_specified_UTM_zonenumber());
//   cout << "Recomputed ellipse_center = " << ellipse_center << endl;

   double xc=ellipse_center.get_UTM_easting();
   double yc=ellipse_center.get_UTM_northing();

   double theta_start=2*PI;
   double theta_stop=0;
   double dtheta=5*PI/180;	
   unsigned int n_thetabins=(theta_start-theta_stop)/dtheta;
   double flightpath_sgn=-1;	// clockwise flight path

   vector<threevector> V;
   for (unsigned int n=0; n<n_thetabins; n++)
   {
      double theta=flightpath_sgn*n*dtheta;
      double x=xc+a*cos(phi)*cos(theta)-b*sin(phi)*sin(theta);
      double y=yc+b*sin(phi)*cos(theta)+b*cos(phi)*sin(theta);
      V.push_back(threevector(x,y));
   } // loop over index n labeling theta
   V.push_back(V[0]);
   
   PolyLine* curr_PolyLine_ptr=
      Path_PolyLinesGroup_ptr->generate_new_PolyLine(V.back(),V);
   Path_PolyLinesGroup_ptr->reset_PolyLine_altitudes(curr_PolyLine_ptr);

//   cout << "*curr_PolyLine_ptr = " << *curr_PolyLine_ptr << endl;
//   cout << "Path_PolyLinesGroup_ptr->get_n_Graphicals() = "
//        << Path_PolyLinesGroup_ptr->get_n_Graphicals() << endl;

   curr_PolyLine_ptr->add_vertex_points(
      V,curr_PolyLine_ptr->get_selected_color());
   curr_PolyLine_ptr->set_entry_finished_flag(true);

   polyline* polyline_ptr=curr_PolyLine_ptr->get_or_set_polyline_ptr();
   double flightpath_length=polyline_ptr->compute_total_length();

//   cout << "*polyline_ptr = " << *polyline_ptr << endl;
//   cout << "flightpath_length = " << flightpath_length << endl;
   
   return flightpath_length;
}

// --------------------------------------------------------------------
// Member function generate_linesegment_flightpath()

double LOSMODELSGROUP::generate_linesegment_flightpath(
   const twovector& r1,const twovector& r2)
{
//   cout << "inside LOSMODELSGROUP::generate_linesegment_flightpath()" << endl;

   double pathlength=(r2-r1).magnitude();
   threevector ehat=(r2-r1).unitvector();
   double ds=3000;	// meters
   unsigned int n_sbins=pathlength/ds;
   
   vector<threevector> V;
   for (unsigned int n=0; n<n_sbins; n++)
   {
      threevector r=threevector(r1)+get_grid_world_origin()+n*ds*ehat;
      V.push_back(r);
   } // loop over index n labeling theta
   
   PolyLine* curr_PolyLine_ptr=
      Path_PolyLinesGroup_ptr->generate_new_PolyLine(V.back(),V);
   Path_PolyLinesGroup_ptr->reset_PolyLine_altitudes(curr_PolyLine_ptr);

//   cout << "*curr_PolyLine_ptr = " << *curr_PolyLine_ptr << endl;
//   cout << "Path_PolyLinesGroup_ptr->get_n_Graphicals() = "
//        << Path_PolyLinesGroup_ptr->get_n_Graphicals() << endl;

   curr_PolyLine_ptr->add_vertex_points(
      V,curr_PolyLine_ptr->get_selected_color());
   curr_PolyLine_ptr->set_entry_finished_flag(true);

   polyline* polyline_ptr=curr_PolyLine_ptr->get_or_set_polyline_ptr();
   double flightpath_length=polyline_ptr->compute_total_length();

//   cout << "*polyline_ptr = " << *polyline_ptr << endl;
//   cout << "flightpath_length = " << flightpath_length << endl;
   
   return flightpath_length;
}

// ==========================================================================
// Ground target geolocation via raytraced visibility map comparisons
// ==========================================================================

// Member function generate_target_visibility_omnimap()

bool LOSMODELSGROUP::generate_target_visibility_omnimap()
{   
   cout << "inside LOSMODELSGROUP::generate_target_visibility_omnimaps()" 
        << endl;

   vector<twovector> target_posns;
   threevector apex=
      GroundTarget_SignPostsGroup_ptr->get_ground_target_posns(target_posns);

// Elevate apex 1.5 meters above local ground;

   apex += 1.5*z_hat;
//   cout << "apex = " << apex << endl;

   int n_ground_targets=target_posns.size();
   cout << "n_ground_targets = " << n_ground_targets << endl;
   if (n_ground_targets==0) return false;

   double ds;
   if (ladar_height_data_flag)
   {
      ds=0.33;	   // meter	Reasonable value for HAFB minimap
//      ds=0.5;	   // meter
//      ds=0.75;	   // meter
//      ds=2.0;	   // meter
   }
   else
   {
//      ds=0.25*get_raytrace_cellsize();
      ds=get_raytrace_cellsize();	// meters
   }
   cout << "ds = " << ds << endl;

// First compute omnimap's bounding box corners in UTM coordinates:

   double target_easting=target_posns[0].get(0);
   double target_northing=target_posns[0].get(1);
   double delta=1*1000;	// meters

   int UTM_zone=get_EarthRegionsGroup_ptr()->get_specified_UTM_zonenumber();
   bool northern_hemisphere_flag=true;
   geopoint lower_left_corner(
      northern_hemisphere_flag,UTM_zone,
      target_easting-0.5*delta,target_northing-0.5*delta);
   geopoint upper_right_corner(
      northern_hemisphere_flag,UTM_zone,
      target_easting+0.5*delta,target_northing+0.5*delta);
//   cout << "lower_left corner = " << lower_left_corner << endl;
//   cout << "upper_right_corner = " << upper_right_corner << endl;

   int mdim=101;
   int ndim=101;
//   int mdim=651;	// Reasonable upper bound for HAFB minimap
//   int ndim=651;	// Reasonable upper bound for HAFB minimap
   twoDarray* omni_twoDarray_ptr=initialize_groundmap(
      lower_left_corner,upper_right_corner,mdim,ndim);
   omni_twoDarray_ptr->initialize_values(-1);
   cout << "*omni_twoDarray_ptr = " << *omni_twoDarray_ptr << endl;
//   outputfunc::enter_continue_char();
   
   OBSFRUSTUM* OBSFRUSTUM_ptr=new OBSFRUSTUM();
   OBSFRUSTUM_ptr->set_TilesGroup_ptr(TilesGroup_ptr);

// Load all height data needed for entire omni map computation:

   twoDarray* DTED_ztwoDarray_ptr=
      TilesGroup_ptr->load_ladar_height_data_into_ztwoDarray();
   twoDarray* reduced_DTED_ztwoDarray_ptr=TilesGroup_ptr->
      generate_reduced_DTED_ztwoDarray();
   
   cout << "*DTED_ztwoDarray_ptr = " << *DTED_ztwoDarray_ptr << endl;
   cout << "*reduced_DTED_ztwoDarray_ptr = "
        << *reduced_DTED_ztwoDarray_ptr << endl;

   max_ground_Z=DTED_ztwoDarray_ptr->maximum_value();
//   cout << "max_ground_Z = " << max_ground_Z << endl;

   raytrace_visibility_omnimap(
      apex,max_ground_Z,max_raytrace_range,
      min_raytrace_range,ds,omni_twoDarray_ptr,
      DTED_ztwoDarray_ptr,reduced_DTED_ztwoDarray_ptr,OBSFRUSTUM_ptr);

   display_omni_occlusion(omni_twoDarray_ptr);
   string visibility_filename="simulated_visibility.omnimap";
   export_visibility_omnimap(visibility_filename,apex,omni_twoDarray_ptr);

   delete OBSFRUSTUM_ptr;

   return true;
}

// ---------------------------------------------------------------------
// Member function initialize_groundmap() instantiates a scalar groundmap
// twoDarray whose geographic size is set by input lower_left_corner
// and upper_right_corner geopoints.

twoDarray* LOSMODELSGROUP::initialize_groundmap(
   const geopoint& lower_left_corner,const geopoint& upper_right_corner,
   int mdim,int ndim)
{   
//   cout << "inside LOSMODELSGROUP::initialize_groundmap()" << endl;

// Instantiate groundmap twoDarrays' borders relative to grid world
// origin:

   double groundmap_xlo=lower_left_corner.get_UTM_easting();
   double groundmap_xhi=upper_right_corner.get_UTM_easting();
   double groundmap_ylo=lower_left_corner.get_UTM_northing();
   double groundmap_yhi=upper_right_corner.get_UTM_northing();

   return initialize_groundmap(
      groundmap_xlo,groundmap_xhi,groundmap_ylo,groundmap_yhi,mdim,ndim);
}

// ---------------------------------------------------------------------
twoDarray* LOSMODELSGROUP::initialize_groundmap(
   double groundmap_xlo,double groundmap_xhi,
   double groundmap_ylo,double groundmap_yhi,int mdim,int ndim)
{   
//   cout << "inside LOSMODELSGROUP::initialize_groundmap()" << endl;

   twoDarray* groundmap_twoDarray_ptr=new twoDarray(mdim,ndim);
   groundmap_twoDarray_ptr->init_coord_system(
      groundmap_xlo,groundmap_xhi,groundmap_ylo,groundmap_yhi);
//   cout << "*groundmap_twoDarray_ptr = " << *groundmap_twoDarray_ptr << endl;

   return groundmap_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function raytrace_visibility_omnimap() takes in the apex
// location for some omni-directional camera.  It also takes in
// pre-calculated terrain map *DTED_ztwoDarray_ptr.  It returns
// a binary visibility map within *omni_twoDarray_ptr.

void LOSMODELSGROUP::raytrace_visibility_omnimap(
   const threevector& apex,double max_ground_Z,double max_raytrace_range,
   double min_raytrace_range,double ds,twoDarray* omni_twoDarray_ptr,
   twoDarray* DTED_ztwoDarray_ptr,twoDarray* reduced_DTED_ztwoDarray_ptr,
   OBSFRUSTUM* OBSFRUSTUM_ptr)
{   
//   cout << "inside LOSMODELSGROUP::raytrace_visibility_omnimap()" << endl;

   omni_twoDarray_ptr->initialize_values(-1);
   double xlo=omni_twoDarray_ptr->get_xlo();
   double xhi=omni_twoDarray_ptr->get_xhi();
   double delta=xhi-xlo;

   int n_visible_cells=0;
   threevector occluded_ray_posn;
   for (unsigned int px=0; px<omni_twoDarray_ptr->get_mdim(); px++)
   {
//      cout << px << " " << flush;

      double x=omni_twoDarray_ptr->fast_px_to_x(px);
      for (unsigned int py=0; py<omni_twoDarray_ptr->get_ndim(); py++)
      {
         double y=omni_twoDarray_ptr->fast_py_to_y(py);

         double rhosqr=sqr(x-apex.get(0))+sqr(y-apex.get(1));

//         cout << "px = " << px << " py = " << py << " rho = " << rho
//              << " delta = " << delta << endl;
         
         if (rhosqr > sqr(0.5*delta)) continue;

         int n_visible_targets=OBSFRUSTUM_ptr->get_ray_tracer_ptr()->
            trace_individual_ray(
            apex,x,y,max_ground_Z,
            max_raytrace_range,min_raytrace_range,ds,
            DTED_ztwoDarray_ptr,reduced_DTED_ztwoDarray_ptr,occluded_ray_posn);
         n_visible_cells += n_visible_targets;
         omni_twoDarray_ptr->put(px,py,n_visible_targets);
      } // loop over omni's py index
   } // loop over omni's px index
   cout << endl;

//   int n_cells=omni_twoDarray_ptr->get_mdim()*
//      omni_twoDarray_ptr->get_ndim();
//   cout << "n_visible_cells = " << n_visible_cells << endl;
//   cout << "n_cells = " << n_cells << endl;
//   cout << "visibility frac = n_visible_cells/n_cells = "
//        << double(n_visible_cells)/double(n_cells) << endl;

//   cout << "*omni_twoDarray_ptr = " << *omni_twoDarray_ptr << endl;
//   cout << "omni_twoDarray_ptr->minimum_value() = "
//        << omni_twoDarray_ptr->minimum_value() << endl;
//   cout << "omni_twoDarray_ptr->maximum_value() = "
//        << omni_twoDarray_ptr->maximum_value() << endl;
}

// ---------------------------------------------------------------------
// Member function display_omni_occlusion()

void LOSMODELSGROUP::display_omni_occlusion(twoDarray* ptwoDarray_ptr)
{
   cout << "inside LOSMODELSGROUP::display_omni_occlusion()" << endl;

// Automatically reset ColorMap to display probabilities and force
// ColorGeodeVisitor to reload latest p values:

   if (ColorGeodeVisitor_ptr == NULL) return;
   
   ColorGeodeVisitor_ptr->clear_ptwoDarray_ptrs();
   ColorGeodeVisitor_ptr->push_back_ptwoDarray_ptr(ptwoDarray_ptr);
      
   if (PointCloudsGroup_ptr != NULL)
   {
      if (PointCloudsGroup_ptr->get_dependent_coloring_var() != 3)
      {
         PointCloudsGroup_ptr->set_dependent_coloring_var(3);
         PointCloudsGroup_ptr->update_dynamic_Grid_color();
      }
      PointCloudsGroup_ptr->reload_all_colors();
   }
}

// ---------------------------------------------------------------------
// Member function export_visibility_omnimap() takes in visibility
// *ptwoDarray_ptr which is assumed to contain binary integer values.
// The number of "visible" values is further assumed to be much less
// than the number of "occluded" values.  So this method writes out
// the (px,py) coordinates of all visible pixels to an output text file.
// It also exports metadata for *ptwoDarray_ptr needed to reconstruct
// the visibility map.

void LOSMODELSGROUP::export_visibility_omnimap(
   string visibility_filename,const threevector& apex,
   twoDarray* ptwoDarray_ptr)
{
//   cout << "inside LOSMODELSGROUP::export_visibility_omnimap()" << endl;
//   cout << "*ptwoDarray_ptr = " << *ptwoDarray_ptr << endl;

   ofstream outstream;
   outstream.precision(12);
   filefunc::openfile(visibility_filename,outstream);
   string comment1=
      "# Coordinates px1 py1 px2 py2 px3 py3 etc for pixels with clear";
   string comment2=
      "# line-of-sight to transmitter location";
   outstream << comment1 << endl;
   outstream << comment2 << endl << endl;

   outstream << apex.get(0) << " # Transmitter X" << endl;
   outstream << apex.get(1) << " # Transmitter Y" << endl;
   outstream << apex.get(2) << " # Transmitter Z" << endl << endl;

   outstream << ptwoDarray_ptr->get_mdim() << " # mdim" << endl;
   outstream << ptwoDarray_ptr->get_ndim() << " # ndim" << endl;
   outstream << ptwoDarray_ptr->get_xlo() << " # xlo" << endl;
   outstream << ptwoDarray_ptr->get_xhi() << " # xhi" << endl;
   outstream << ptwoDarray_ptr->get_deltax() << " # dx" << endl;
   outstream << ptwoDarray_ptr->get_ylo() << " # ylo" << endl;
   outstream << ptwoDarray_ptr->get_yhi() << " # yhi" << endl;
   outstream << ptwoDarray_ptr->get_deltay() << " # dy" << endl;
   outstream << endl;

   for (unsigned int px=0; px<ptwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ptwoDarray_ptr->get_ndim(); py++)
      {
         double curr_p=ptwoDarray_ptr->get(px,py);
         if (curr_p < 0.1) continue;
         outstream << px << " " << py << endl;
      } // loop over py index
   } // loop over px index
   
   filefunc::closefile(visibility_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function import_visibility_omnimap() reads in the text file
// generated by export_visibility_omnimap().  It instantiates,
// reconstructs and returns *omni_twoDarray_ptr from the parsed file.

twoDarray* LOSMODELSGROUP::import_visibility_omnimap(
   string visibility_filename,threevector& transmitter_posn)
{
//   cout << "inside LOSMODELSGROUP::import_visibility_omnimap()" << endl;
//   cout << "*ptwoDarray_ptr = " << *ptwoDarray_ptr << endl;

   filefunc::ReadInfile(visibility_filename);

   int line_counter=0;
   double transmitter_X=
      stringfunc::string_to_number(filefunc::text_line[line_counter++]);
   double transmitter_Y=
      stringfunc::string_to_number(filefunc::text_line[line_counter++]);
   double transmitter_Z=
      stringfunc::string_to_number(filefunc::text_line[line_counter++]);
   transmitter_posn=threevector(transmitter_X,transmitter_Y,transmitter_Z);

   cout << "Transmitter posn = " << transmitter_posn << endl;

   int mdim=stringfunc::string_to_number(filefunc::text_line[line_counter++]);
   int ndim=stringfunc::string_to_number(filefunc::text_line[line_counter++]);
   double xlo=stringfunc::string_to_number(filefunc::text_line[
      line_counter++]);
   double xhi=stringfunc::string_to_number(filefunc::text_line[
      line_counter++]);
   double dx=stringfunc::string_to_number(filefunc::text_line[line_counter++]);
   double ylo=stringfunc::string_to_number(filefunc::text_line[
      line_counter++]);
   double yhi=stringfunc::string_to_number(filefunc::text_line[
      line_counter++]);
   double dy=stringfunc::string_to_number(filefunc::text_line[line_counter++]);

   twoDarray* omni_twoDarray_ptr=new twoDarray(mdim,ndim);
   omni_twoDarray_ptr->set_xlo(xlo);
   omni_twoDarray_ptr->set_xhi(xhi);
   omni_twoDarray_ptr->set_deltax(dx);
   omni_twoDarray_ptr->set_ylo(ylo);
   omni_twoDarray_ptr->set_yhi(yhi);
   omni_twoDarray_ptr->set_deltay(dy);
   omni_twoDarray_ptr->clear_values();
   cout << "*omni_twoDarray_ptr = " << *omni_twoDarray_ptr << endl;

   for (unsigned int i=line_counter; i<filefunc::text_line.size(); i++)
   {
      vector<double> values=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
      int px=values[0];
      int py=values[1];
      omni_twoDarray_ptr->put(px,py,1);
   }

   display_omni_occlusion(omni_twoDarray_ptr);
   return omni_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function smear_visibility_omnimap() 

twoDarray* LOSMODELSGROUP::smear_visibility_omnimap(twoDarray* ptwoDarray_ptr)
{
//   cout << "inside LOSMODELSGROUP::smear_visibility_omnimap()" << endl;

   twoDarray* ptwoDarray_filtered_ptr=new twoDarray(ptwoDarray_ptr);

   double dx=ptwoDarray_ptr->get_deltax();
   double sigma=5*dx;
   double e_folding_distance=3;
   genmatrix* filter_ptr=filterfunc::gaussian_2D_filter(
      dx,sigma,e_folding_distance);
   cout << "filter_ptr->get_mdim() = " << filter_ptr->get_mdim() << endl;
   cout << "filter_ptr->get_ndim() = " << filter_ptr->get_ndim() << endl;
   
   imagefunc::brute_twoD_convolve(
      filter_ptr,ptwoDarray_ptr,ptwoDarray_filtered_ptr);
   delete filter_ptr;

//   return ptwoDarray_filtered_ptr;

// Downsample hires, convolved visibility map onto lower-res twoDarray:

   int nxbins_regrid=101;
   int nybins_regrid=101;
   twoDarray* ptwoDarray_filtered_downsampled_ptr=
      compositefunc::downsample(
         nxbins_regrid,nybins_regrid,ptwoDarray_filtered_ptr);
   delete ptwoDarray_filtered_ptr;

// Binary threshold filtered, downsampled visibility omnimap:

   double p_threshold=0.5;
   binaryimagefunc::binary_threshold(
      p_threshold,ptwoDarray_filtered_downsampled_ptr,0,1);

   return ptwoDarray_filtered_downsampled_ptr;
}

// ---------------------------------------------------------------------
// Member function fit_ground_target_position() takes in the true
// position for a ground transmitter along with a simulated 
// visibility map in *measured_twoDarray_ptr.  It iteratively loops
// over a 5x5 lattice whose easting and northing boundaries are
// intentionally randomized wrt xlo,xhi,ylo,hi of
// *measured_twoDarray_ptr.  At each lattice point, this method
// computes a candidate raytraced visibility map and compares it with
// *measured_twoDarray_ptr.  After 5 iterations, this method returns
// the best fit location for the ground transmitter deduced by
// comparing calculated vs measured visibility maps.  The best fit
// visibility map is displayed in the 3D viewer, and its non-zero
// entries are written out to fitted_visibility.omnimap.

void LOSMODELSGROUP::fit_ground_target_position(
   const threevector& transmitter_posn,twoDarray* measured_twoDarray_ptr)
{   
   cout << "inside LOSMODELSGROUP::fit_ground_target_position()" << endl;

   double ds;
   if (ladar_height_data_flag)
   {
      ds=0.33;	   // meter	Reasonable value for HAFB minimap
   }
   else
   {
      ds=get_raytrace_cellsize();	// meters
   }
//   cout << "ds = " << ds << endl;

// Load all height data needed for entire omni map computation:

   twoDarray* DTED_ztwoDarray_ptr=
      TilesGroup_ptr->load_ladar_height_data_into_ztwoDarray();
   twoDarray* reduced_DTED_ztwoDarray_ptr=TilesGroup_ptr->
      generate_reduced_DTED_ztwoDarray();
   
//   cout << "*DTED_ztwoDarray_ptr = " << *DTED_ztwoDarray_ptr << endl;
//   cout << "*reduced_DTED_ztwoDarray_ptr = "
//        << *reduced_DTED_ztwoDarray_ptr << endl;

   max_ground_Z=DTED_ztwoDarray_ptr->maximum_value();
//   cout << "max_ground_Z = " << max_ground_Z << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=new OBSFRUSTUM();
   OBSFRUSTUM_ptr->set_TilesGroup_ptr(TilesGroup_ptr);

   twoDarray* candidate_twoDarray_ptr=new twoDarray(measured_twoDarray_ptr);
   candidate_twoDarray_ptr->clear_values();
   twoDarray* bestfit_twoDarray_ptr=new twoDarray(measured_twoDarray_ptr);
   bestfit_twoDarray_ptr->initialize_values(-1);

   unsigned int mdim=measured_twoDarray_ptr->get_mdim();
   unsigned int ndim=measured_twoDarray_ptr->get_ndim();
   double xlo=measured_twoDarray_ptr->get_xlo();
   double xhi=measured_twoDarray_ptr->get_xhi();
   double ylo=measured_twoDarray_ptr->get_ylo();
   double yhi=measured_twoDarray_ptr->get_yhi();

   cout << "Initial xlo = " << xlo << " xhi = " << xhi << endl;
   cout << "Initial ylo = " << ylo << " yhi = " << yhi << endl;

// In order to avoid artificially good fit resulting, we intentionally
// force the measured and candidate twoDarrays to NOT perfectly overlap:

   int nbins=5;
   double delta_x=(xhi-xlo)/(nbins-1);
   xlo += 2*(nrfunc::ran1()-0.5)*delta_x;
   xhi += 2*(nrfunc::ran1()-0.5)*delta_x;
   double delta_y=(yhi-ylo)/(nbins-1);
   ylo += 2*(nrfunc::ran1()-0.5)*delta_y;
   yhi += 2*(nrfunc::ran1()-0.5)*delta_y;

   cout << "delta_x = " << delta_x << " delta_y = " << delta_y << endl;
   cout << "Randomized xlo = " << xlo << " xhi = " << xhi << endl;
   cout << "Randomized ylo = " << ylo << " yhi = " << yhi << endl;

   double min_score=POSITIVEINFINITY;
   threevector fitted_posn;

   param_range target_X(xlo,xhi,nbins);
   param_range target_Y(ylo,yhi,nbins);
//   int n_iters=1;
//   int n_iters=2;
   unsigned int n_iters=5;
   for (unsigned int iter=0; iter<n_iters; iter++)
   {
      cout << "----------------------------------------------------" << endl;
      cout << "Iteration " << iter+1 << " of " << n_iters << endl;
      cout << "X_start = " << target_X.get_start()
           << " X_stop = " << target_X.get_stop() << endl;
      cout << "X_step = " << target_X.get_delta() << endl;
      cout << "----------------------------------------------------" << endl;

// ========================================================================
// Begin while loop over target position parameters
// ========================================================================

      while (target_X.prepare_next_value())
      {
         cout << "target_X counter = " << target_X.get_counter() << endl;
         double tgt_x=target_X.get_value();
         while (target_Y.prepare_next_value())
         {
            double tgt_y=target_Y.get_value();

            double tgt_z;
            DTED_ztwoDarray_ptr->point_to_interpolated_value(
               tgt_x,tgt_y,tgt_z);

// Elevate apex 1.5 meters above local ground;

            threevector apex(tgt_x,tgt_y,tgt_z+1.5);

            raytrace_visibility_omnimap(
               apex,max_ground_Z,max_raytrace_range,min_raytrace_range,ds,
               candidate_twoDarray_ptr,DTED_ztwoDarray_ptr,
               reduced_DTED_ztwoDarray_ptr,OBSFRUSTUM_ptr);

            double score=0;
            for (unsigned int qx=0; qx<mdim; qx++)
            {
               for (unsigned int qy=0; qy<ndim; qy++)
               {
                  score += fabs(measured_twoDarray_ptr->get(qx,qy)-
                  candidate_twoDarray_ptr->get(qx,qy));
               }
            }
            if (score < min_score)
            {
               min_score=score;
               candidate_twoDarray_ptr->copy(bestfit_twoDarray_ptr);
               target_X.set_best_value();
               target_Y.set_best_value();
               
               fitted_posn=threevector(tgt_x,tgt_y,tgt_z);
               
               cout << "score = " << score << endl;
               cout << "Fitted posn = " << fitted_posn << endl;
               cout << "Transmitter posn = " << transmitter_posn << endl;
               cout << "Discrepancy = "
                    << (transmitter_posn-fitted_posn).magnitude() << endl;
            }

         } // target_Y while loop
      } // target_X while loop

// ========================================================================
// End while loop over target position parameters
// ========================================================================

      double frac=0.33;
      target_X.shrink_search_interval(target_X.get_best_value(),frac);
      target_Y.shrink_search_interval(target_Y.get_best_value(),frac);

   } // loop over iter index
   
   cout << endl;

//   cout << "*omni_twoDarray_ptr = " << *omni_twoDarray_ptr << endl;
//   cout << "omni_twoDarray_ptr->minimum_value() = "
//        << omni_twoDarray_ptr->minimum_value() << endl;
//   cout << "omni_twoDarray_ptr->maximum_value() = "
//        << omni_twoDarray_ptr->maximum_value() << endl;

   cout << "********************************************************" << endl;
   cout << "Minimum score = " << min_score << endl;
   cout << "Fitted_posn = " << fitted_posn << endl;
   cout << "Transmitter posn = " << transmitter_posn << endl;
   cout << "Discrepancy = "
        << (transmitter_posn-fitted_posn).magnitude() << endl;
   cout << "********************************************************" << endl;

   SignPost* fitted_SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
      generate_new_SignPost();
   fitted_SignPost_ptr->set_UVW_coords(
      GroundTarget_SignPostsGroup_ptr->get_curr_t(),
      GroundTarget_SignPostsGroup_ptr->get_passnumber(),fitted_posn);
   fitted_SignPost_ptr->set_label("Fitted position");
   GroundTarget_SignPostsGroup_ptr->update_colors();

   display_omni_occlusion(bestfit_twoDarray_ptr);
   string visibility_filename="fitted_visibility.omnimap";
   export_visibility_omnimap(
      visibility_filename,fitted_posn,bestfit_twoDarray_ptr);
}

// ==========================================================================
// Moving ground target member functions
// ==========================================================================

void LOSMODELSGROUP::raytrace_moving_ground_target()
{
   cout << "inside LOSMODELSGROUP::raytrace_moving_ground_target()" << endl;

   MODEL* MODEL_ptr=get_MODEL_ptr(0);
   MODEL_ptr->set_raytrace_occluded_ground_regions_flag(true);
   MODEL_ptr->set_raytrace_ROI_flag(false);
   get_AnimationController_ptr()->setState(AnimationController::PLAY);

// FAKE FAKE:  Monday Aug 15, 2011 at 1:17 pm
// Limit last frame number for AVI movie only...

   get_AnimationController_ptr()->set_last_framenumber(80);
}

// ---------------------------------------------------------------------
// Member function export_moving_ground_target_visibility_rays()
// loops over all sampled flight points.  If a corresponding
// LineSegment ray connecting each flight point to the specified
// ground target does not already exist, this method instantiates a
// new member of *LineSegmentsGroup_ptr.  It then sets the color of
// the ray equal to black if the target lies outside the aerial
// sensor's field-of-regard, red if the target is occluded from the
// sensor's view and green if the target is visible to the sensor.

void LOSMODELSGROUP::export_moving_ground_target_visibility_rays()
{   
   cout << "inside LOSMODELSGROUP::export_moving_ground_target_visibility_rays()"
        << endl;

   if (GroundTarget_SignPostsGroup_ptr==NULL) return;
   MODEL* MODEL_ptr=get_MODEL_ptr(0);

   int target_ID=0;
   SignPost* SignPost_ptr=GroundTarget_SignPostsGroup_ptr->
      get_SignPost_ptr(target_ID);
   if (SignPost_ptr==NULL) return;

   string output_filename="moving_ground_target.visibilities";
   ofstream outstream;
   outstream.precision(12);
   filefunc::openfile(output_filename,outstream);
   outstream << "# t  aerial_posn(x,y,z)  ray_stop_posn(x,y,z)  tgt_visibility"
             << endl << endl;

   threevector aerial_posn,ground_posn;   
   LineSegmentsGroup_ptr->destroy_all_LineSegments();

   for (unsigned int curr_t=get_first_framenumber(); curr_t<=get_last_framenumber(); 
        curr_t++)
   {
      SignPost_ptr->get_UVW_coords(
         curr_t,GroundTarget_SignPostsGroup_ptr->get_passnumber(),ground_posn);

      MODEL_ptr->get_UVW_coords(curr_t,get_passnumber(),aerial_posn);

      threevector ray_stop_posn(ground_posn);
      threevector occlusion_posn;
      int tgt_visibility=get_ground_target_visibility(
         curr_t,SignPost_ptr->get_ID(),occlusion_posn);

//      cout << "curr_t = " << curr_t 
//           << " aerial_posn = " << aerial_posn
//           << " tgt_visibility = " << tgt_visibility << endl;
//      cout << "ground_posn = " << ground_posn << endl;

      LineSegment* LineSegment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(curr_t);
      if (LineSegment_ptr==NULL)
      {
         bool draw_endpoint1_flag=true;
         bool draw_endpoint2_flag=true;
         LineSegment_ptr=LineSegmentsGroup_ptr->
            generate_new_canonical_LineSegment(
               -1,draw_endpoint1_flag,draw_endpoint2_flag,
               endpoint_size_prefactor);
      }
      LineSegment_ptr->set_stationary_Graphical_flag(false);
      
      colorfunc::Color ray_color;
      if (tgt_visibility==-2)
      {
         continue;
      }
      else if (tgt_visibility==-1)
      {
         ray_color=colorfunc::grey;
//         ray_color=colorfunc::black;
      }
      else if (tgt_visibility==0)
      {
         ray_stop_posn=occlusion_posn;
         ray_color=colorfunc::red;
      }
      else if (tgt_visibility==1)
      {
         ray_color=colorfunc::green;
      }

      LineSegment_ptr->set_scale_attitude_posn(
         LineSegmentsGroup_ptr->get_curr_t(),
         LineSegmentsGroup_ptr->get_passnumber(),
         aerial_posn,ray_stop_posn);

//      cout << "ray_color = " << ray_color << endl;
      LineSegment_ptr->set_permanent_color(ray_color);
      LineSegment_ptr->set_curr_color(ray_color);
      LineSegment_ptr->get_LineWidth_ptr()->setWidth(10.0);

      outstream << curr_t << " "
                << aerial_posn.get(0) << " "
                << aerial_posn.get(1) << " "
                << aerial_posn.get(2) << " "
                << ray_stop_posn.get(0) << " "
                << ray_stop_posn.get(1) << " "
                << ray_stop_posn.get(2) << " "
                << tgt_visibility << endl;

   } // loop over index curr_t labeling frame number

   filefunc::closefile(output_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function import_moving_ground_target_visibility_rays()
// loops over all sampled flight points.  If a corresponding
// LineSegment ray connecting each flight point to the specified
// ground target does not already exist, this method instantiates a
// new member of *LineSegmentsGroup_ptr.  It then sets the color of
// the ray equal to black if the target lies outside the aerial
// sensor's field-of-regard, red if the target is occluded from the
// sensor's view and green if the target is visible to the sensor.

void LOSMODELSGROUP::import_moving_ground_target_visibility_rays()
{   
   cout << "inside LOSMODELSGROUP::import_moving_ground_target_visibility_rays()"
        << endl;

   string input_filename="moving_ground_target.visibilities";
   filefunc::ReadInfile(input_filename);

   vector<double> t,tgt_visibility;
   vector<threevector> aerial_posn,ray_stop_posn;   
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      t.push_back(column_values[0]);
      aerial_posn.push_back(threevector(
         column_values[1],column_values[2],column_values[3]));
      ray_stop_posn.push_back(threevector(
         column_values[4],column_values[5],column_values[6]));
      tgt_visibility.push_back(column_values[7]);
   }

   LineSegmentsGroup_ptr->destroy_all_LineSegments();

   double frac_hits_to_keep=1;
   cout << "Enter fraction of simulated dynamic hits to actually keep:" 
        << endl;
   cin >> frac_hits_to_keep;

   for (unsigned int i=0; i<t.size(); i++)
   {
      if (nrfunc::ran1() > frac_hits_to_keep) continue;

      LineSegment* LineSegment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(t[i]);
      if (LineSegment_ptr==NULL)
      {
         bool draw_endpoint1_flag=true;
         bool draw_endpoint2_flag=false;
//         bool draw_endpoint2_flag=true;
         LineSegment_ptr=LineSegmentsGroup_ptr->
            generate_new_canonical_LineSegment(
               -1,draw_endpoint1_flag,draw_endpoint2_flag,
               endpoint_size_prefactor);
      }
      LineSegment_ptr->set_stationary_Graphical_flag(false);
      
      colorfunc::Color ray_color;
      if (tgt_visibility[i]==-2)
      {
         continue;
      }
      else if (tgt_visibility[i]==-1)
      {
         ray_color=colorfunc::grey;
//         ray_color=colorfunc::black;
      }
      else if (tgt_visibility[i]==0)
      {
         ray_color=colorfunc::red;
      }
      else if (tgt_visibility[i]==1)
      {
         ray_color=colorfunc::green;
      }

//      threevector ray_start_posn=aerial_posn[i];

//      const double pole_height=100;	// meters
      const double pole_height=400;	// meters
      threevector ray_start_posn=ray_stop_posn[i]+pole_height*z_hat;

      if (tgt_visibility[i]==1)
      {

         LineSegment_ptr->set_scale_attitude_posn(
            LineSegmentsGroup_ptr->get_curr_t(),
            LineSegmentsGroup_ptr->get_passnumber(),
            ray_start_posn,ray_stop_posn[i]);

//      cout << "ray_color = " << ray_color << endl;
         LineSegment_ptr->set_permanent_color(ray_color);
         LineSegment_ptr->set_curr_color(ray_color);
         LineSegment_ptr->get_LineWidth_ptr()->setWidth(10.0);
      
      }

   } // loop over index curr_t labeling frame number
}

// ==========================================================================
// Real-time manipulation of aircraft models
// ==========================================================================

// Member function update_dynamic_aircraft_model()

void LOSMODELSGROUP::update_dynamic_aircraft_model()
{   
//   cout << "inside LOSMODELSGROUP::update_dynamic_aircraft_model()" << endl;

   if (get_AnimationController_ptr()->
   curr_framenumber_equals_prev_framenumber()) return;
   get_AnimationController_ptr()->set_prev_framenumber(
      get_AnimationController_ptr()->get_curr_framenumber());

   MODEL* Aircraft_MODEL_ptr=get_MODEL_ptr(0);
//   cout << "Aircraft_MODEL_ptr = " << Aircraft_MODEL_ptr << endl;
   track* track_ptr=Aircraft_MODEL_ptr->get_track_ptr();
   double curr_t=
      get_AnimationController_ptr()->get_time_corresponding_to_curr_frame();

   threevector curr_posn,curr_sensor_aer;
   twovector curr_sensor_FOV;
   rpy curr_rpy;
   
   track_ptr->get_interpolated_posn_rpy_sensor_aer_FOV(
      curr_t,curr_posn,curr_rpy,curr_sensor_aer,curr_sensor_FOV);

   geopoint aerial_point(
      get_pass_ptr()->get_northern_hemisphere_flag(),
      get_pass_ptr()->get_UTM_zonenumber(),
      curr_posn.get(0),curr_posn.get(1),curr_posn.get(2));

   update_dynamic_aircraft_model(
      aerial_point,curr_rpy,curr_sensor_aer,curr_sensor_FOV);

//   cout << "curr_t = " << curr_t << endl;
//   cout << "aerial_point = " << aerial_point << endl;
//   cout << "curr_posn = " << curr_posn << endl;
//   cout << "curr_rpy = " << curr_rpy << endl;
//   cout << "curr_sensor_aer = " << curr_sensor_aer << endl;
//   cout << "curr_sensor_FOV = " << curr_sensor_FOV << endl;
}

// ---------------------------------------------------------------------
// Member function update_dynamic_aircraft_model()

void LOSMODELSGROUP::update_dynamic_aircraft_model(
   const geopoint& aerial_point,const rpy& aircraft_RPY,
   const threevector& frustum_AER,const twovector& frustum_FOV)
{   
//   cout << "inside LOSMODELSGROUP::update_dynamic_aircraft_model() #2" << endl;

// As of 10/4/11, we assume that *this contains only one MODEL:

   MODEL* Aircraft_MODEL_ptr=get_MODEL_ptr(0);
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr=Aircraft_MODEL_ptr->
      get_OBSFRUSTAGROUP_ptr();

   threevector aircraft_posn=aerial_point.get_UTM_posn();

// Note added on 10/3/11:

//   roll = Roll in orientation.txt
//   pitch = Pitch in orientation.txt
//   yaw = phi_h-90 degs from orientation.txt .  
//       Recall yaw = rotation of aircraft's forward direction about +z_hat

   double roll=aircraft_RPY.get_roll()*PI/180;
   double pitch=aircraft_RPY.get_pitch()*PI/180;
   double yaw=aircraft_RPY.get_yaw()*PI/180;

   update_Aircraft_MODEL(aircraft_posn,roll,pitch,yaw,Aircraft_MODEL_ptr);

// If fixed_aircraft_MODEL_orientation_flag==true, reset Terrain
// Manipulator virtual camera's Uhat and Vhat axes so that they are
// always aligned with the aircraft's right and forward directions.
// If north_up_orientation_flag==true, reset Terrain Manipulator's
// camera so that east points to the right and noints points up:

   if (fixed_aircraft_MODEL_orientation_flag || north_up_orientation_flag)
   {
      osg::Matrixd M=get_CM_3D_ptr()->getMatrix();

      if (north_up_orientation_flag) yaw=0;
//      cout << "yaw = " << yaw*180/PI << endl;

      osg::Matrixd new_M;
      new_M.set(
         cos(yaw) , sin(yaw), 0 , 0,		// TM Uhat
         -sin(yaw) , cos(yaw), 0 , 0,		// TM Vhat
         0, 0, 1, 0,				// TM What
         M(3,0) , M(3,1) , M(3,2) , M(3,3) );
      get_CM_3D_ptr()->setMatrices(new_M);
      osgGA::Terrain_Manipulator* Terrain_Manipulator_ptr=
         dynamic_cast<osgGA::Terrain_Manipulator*>(get_CM_3D_ptr());
      Terrain_Manipulator_ptr->update_compass_heading();
   }

/*
  threevector b_hat=-y_hat; // "behind" vector = -velocity direction
  threevector t_hat=z_hat;	// "top" vector
  threevector l_hat=b_hat.cross(t_hat);	// "left" vector

// Note: b_hat x t_hat = l_hat

// On 10/2/11, we empirically determined the following definitions and
// concatenation ordering for roll, pitch and yaw rotations:

rotation Ryaw(0,0,yaw);
rotation Rpitch(-pitch,0,0);
rotation Rroll(0,-roll,0);

// Firstly, right-hand rotate aircraft coordinate system about +t_hat
// by angle yaw.

// Secondly , right-hand rotate aircraft coordinate system about
// +l_hat = -r_hat by angle pitch

// Finally, right-hand rotate aircraft coordinate system by angle roll
// about +b_hat = -v_hat = -heading direction vector.  (LOST-like
// simulation does not model any non-zero "crab angle" between v_hat
// and heading_hat.)

rotation R=Rroll*Rpitch*Ryaw;

threevector bprime_hat=R*b_hat;
threevector tprime_hat=R*t_hat;
threevector lprime_hat=R*l_hat;
   
cout << "bprime_hat = " << bprime_hat << endl;
cout << "tprime_hat = " << tprime_hat << endl;
cout << "lprime_hat = " << lprime_hat << endl;
*/

   if (OBSFRUSTAGROUP_ptr==NULL) return;

//   cout << "frustum_FOV = " << frustum_FOV << endl;
//   cout << "horiz_frustum_FOV = " << horiz_frustum_FOV
//        << " vert_frustum_FOV = " << vert_frustum_FOV << endl;

   double frustum_sidelength=10;	// meters
   double blank_grey_level=0.5;
   if (OBSFRUSTAGROUP_ptr->get_virtual_OBSFRUSTUM_ptr()==NULL)
   {
      photograph* virtual_photo_ptr=
         OBSFRUSTAGROUP_ptr->instantiate_virtual_photo(
         frustum_FOV.get(0),frustum_FOV.get(1),
         frustum_sidelength,blank_grey_level);
      OBSFRUSTAGROUP_ptr->generate_virtual_OBSFRUSTUM(virtual_photo_ptr);
   }
   
// Don't bother to change virtual OBSFRUSTUM's UV corner rays unless
// horiz or vert FOV have changed by at least one degree from their
// preceding values:

   const double delta_frustum_FOV=1;	// degree

   if ( 
      (fabs(frustum_FOV.get(0)-horiz_frustum_FOV) > delta_frustum_FOV) ||
      (fabs(frustum_FOV.get(1)-vert_frustum_FOV) > delta_frustum_FOV) )
   {
      horiz_frustum_FOV=frustum_FOV.get(0);
      vert_frustum_FOV=frustum_FOV.get(1);

      double blank_grey_level=0.5;
//      photograph* virtual_photo_ptr=
         OBSFRUSTAGROUP_ptr->instantiate_virtual_photo(
         frustum_FOV.get(0),frustum_FOV.get(1),
         frustum_sidelength,blank_grey_level);
   }

   double az_frustum=frustum_AER.get(0)*PI/180;
   double el_frustum=frustum_AER.get(1)*PI/180;
   double roll_frustum=frustum_AER.get(2)*PI/180;

   rotation Rtotal;
   Rtotal=Rtotal.rotation_from_az_el_roll(az_frustum,el_frustum,roll_frustum);

// Extract camera's pointing direction -W_hat from first column of Rtotal:

   threevector neg_What;
   Rtotal.get_column(0,neg_What);
//   cout << "neg_What = " << neg_What << endl;

// Compute current frustum sidelength based upon aircraft's altitude
// above terrain and its pointing angles:

   double Z_ground;
   if (TilesGroup_ptr->estimate_SRTM_z_given_aerial_pt_and_ray(
      aerial_point,neg_What,Z_ground))
   {
      double range=fabs((Z_ground-aircraft_posn.get(2))/neg_What.get(2));
      double theta=0.5*(horiz_frustum_FOV+vert_frustum_FOV)*PI/180;
      frustum_sidelength=range*cos(0.5*theta);
   }
   else
   {
      frustum_sidelength=1000;
   }
//   cout << "Frustum sidelength = " << frustum_sidelength << endl;
      
   threevector camera_posn=aircraft_posn-25*z_hat;	
   OBSFRUSTAGROUP_ptr->get_virtual_camera_ptr()->set_world_posn(camera_posn);

//   OBSFRUSTUM* OBSFRUSTUM_ptr=
      OBSFRUSTAGROUP_ptr->update_virtual_OBSFRUSTUM(
      az_frustum,el_frustum,roll_frustum,frustum_sidelength);

//   cout << "az_frustum = " << az_frustum*180/PI << endl;
//   cout << "el_frustum = " << el_frustum*180/PI << endl;
//   cout << "roll_frustum = " << roll_frustum*180/PI << endl;
   
//   cout << "Rtotal = " << Rtotal << endl;
//   cout << "neg_What = " << neg_What << endl;
}

// ---------------------------------------------------------------------
// Member function toggle_between_earth_and_aircraft_reference_frames()
// switches the Terrain Manipulator from an aircraft-centric to an
// earth-centric view and vice-versa.

void LOSMODELSGROUP::toggle_between_earth_and_aircraft_reference_frames()
{   
   cout << "inside LOSMODELSGROUP::toggle_between_earth_and_aircraft_reference_frames()" << endl;

   MODEL* Aircraft_MODEL_ptr=get_MODEL_ptr(0);

   if (!fixed_aircraft_MODEL_orientation_flag && !north_up_orientation_flag)
   {
      set_selected_Graphical_ID(Aircraft_MODEL_ptr->get_ID());
      set_fixed_aircraft_MODEL_orientation_flag(true);
      set_north_up_orientation_flag(false);
      ReferenceFrameHUD_ptr->set_frame_type(ReferenceFrameHUD::AIRCRAFT_FRAME);
   }
   else if (fixed_aircraft_MODEL_orientation_flag && 
	    !north_up_orientation_flag)
   {
      set_selected_Graphical_ID(Aircraft_MODEL_ptr->get_ID());
      set_fixed_aircraft_MODEL_orientation_flag(false);
      set_north_up_orientation_flag(true);
      ReferenceFrameHUD_ptr->set_frame_type(ReferenceFrameHUD::NORTH_UP_FRAME);
   }
   else if (!fixed_aircraft_MODEL_orientation_flag && 
	    north_up_orientation_flag)
   {
      set_selected_Graphical_ID(-1);
      set_fixed_aircraft_MODEL_orientation_flag(false);
      set_north_up_orientation_flag(false);
      ReferenceFrameHUD_ptr->set_frame_type(ReferenceFrameHUD::FREE_FRAME);
   }
}

// ==========================================================================
// PYXIS server member functions
// ==========================================================================

// Member function start_ROI_visibility_computation() sets the MODEL's
// raytrace_occluded_ground_regions_flag to true, broadcasts an
// initial progress value, resets the AnimationController to its first
// frame and to PLAY mode.  This method should be called only by the
// PYXIS server version and not the interactive version of LOST.

void LOSMODELSGROUP::start_ROI_visibility_computation()
{
//   cout << "inside LOSMODELSGROUP::start_ROI_visibility_computation()" << endl;

   MODEL* MODEL_ptr=get_MODEL_ptr(0);
//   cout << "MODEL_ptr = " << MODEL_ptr << endl;
   
   MODEL_ptr->set_raytrace_occluded_ground_regions_flag(true);
   string progress_type="raytracing";
   get_viewer_Messenger_ptr()->broadcast_progress(0.02,progress_type);

   get_AnimationController_ptr()->set_curr_framenumber(
      get_AnimationController_ptr()->get_first_framenumber());
   get_AnimationController_ptr()->setState(AnimationController::PLAY);
}

// ---------------------------------------------------------------------
// Member function export_PYXIS_ROI_visibility_files() first calls
// display_average_LOS_results() which returns quartile visibility
// fractions via ActiveMQ.  It next generates time averaged visibility
// TIF, NITF & PNG files within a user-specified PYXIS output
// subdirectory and filename.  Finally, this method issues an ActiveMQ
// message so that the calling PYXIS client will know when the ROI
// visibility computation is finished.

void LOSMODELSGROUP::export_PYXIS_ROI_visibility_files()
{
   cout << "inside LOSMODELSGROUP::export_PYXIS_ROI_visibility_files()"
        << endl;
   
   PYXIS_server_flag=false;
   display_average_LOS_results();

   string PYXIS_output_geotif_filename=
      PYXIS_output_subdir+PYXIS_output_basename+".tif";
   string PYXIS_output_nitf_filename=
      PYXIS_output_subdir+PYXIS_output_basename+".nitf";
   string PYXIS_output_png_filename=
      PYXIS_output_subdir+PYXIS_output_basename+".png";

   cout << "PYXIS output geotif filename = "
        << PYXIS_output_geotif_filename << endl;
//   cout << "PYXIS output nitf filename = "
//        << PYXIS_output_nitf_filename << endl;

   string dirname="";
   string basename="TimeAveragedVisibility";
   export_average_occlusion_files(
      dirname,basename,
      PYXIS_output_geotif_filename,PYXIS_output_nitf_filename);
//   clear_raytracing_results();

// As of 1/19/2012, Tim Schreiner requests that time-averaged LOST
// results be returned in PNG image format:

   string unix_cmd="gdal_translate -of PNG "+PYXIS_output_geotif_filename+" "+
      PYXIS_output_png_filename;
   sysfunc::unix_command(unix_cmd);

/*
   bool png_exists_flag=false;
   while (!png_exists_flag)
   {
      cout << "Waiting for png_imagePath = " << PYXIS_output_png_filename 
           << endl;
      png_exists_flag=filefunc::fileexist(PYXIS_output_png_filename);
   }
*/

// As of 1/19/2012, Tim Schreiner requests that a text file also be
// exported containing lower-left/upper right corners of PNG file in
// lat-lon geocoords (dec degs).  

// Issue ActiveMQ message indicating ROI visiblity computation is
// finished and containing full paths for output imagery filenames:

   string command,key,value;
   vector<Messenger::Property> properties;
   command="ROI_VISIBILITY_CALCULATION_FINISHED";

   key="Output geotif filename";
   value=PYXIS_output_geotif_filename;
   properties.push_back(Messenger::Property(key,value));

   key="Output nitf filename";
   value=PYXIS_output_nitf_filename;
   properties.push_back(Messenger::Property(key,value));

   viewer_Messenger_ptr->broadcast_subpacket(command,properties);
   outputfunc::enter_continue_char();
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

bool LOSMODELSGROUP::parse_next_message_in_queue(message& curr_message)
{
//   cout << "inside LOSMODELSGROUP::parse_next_message_in_queue()" << endl;
//   cout << "curr_message.get_text_message() = "
//        << curr_message.get_text_message() << endl;

   bool message_handled_flag=false;

   if (curr_message.get_text_message()=="UPDATE_CURRENT_NODE")
   {
//      cout << "Received UPDATE_CURRENT_NODE message from ActiveMQ" << endl;
      curr_message.extract_and_store_property_keys_and_values();
      int hierarchy_ID=stringfunc::string_to_number(
         curr_message.get_property_value("hierarchy_ID"));
//      int graph_level=stringfunc::string_to_number(
//         curr_message.get_property_value("graph_level"));
      int node_ID=stringfunc::string_to_number(
         curr_message.get_property_value("node_ID"));
      
//      cout << "hierarchy_ID = " << hierarchy_ID
//           << " graph_level = " << graph_level
//           << " node_ID = " << node_ID << endl;

      int campaign_ID,mission_ID,image_ID;
      double epoch;
      imagesdatabasefunc::retrieve_particular_image_metadata(
         images_database_ptr,hierarchy_ID,node_ID,
         campaign_ID,mission_ID,image_ID,epoch);
//      cout << "campaign_ID = " << campaign_ID
//           << " mission_ID = " << mission_ID << endl;
//      cout << "image_ID = " << image_ID << " epoch = " << epoch << endl;

      string image_URL=imagesdatabasefunc::get_image_URL(
         images_database_ptr,campaign_ID,mission_ID,image_ID);
//      cout << "image_URL = " << image_URL << endl;

// Reset AnimationController's clock to epoch returned from imagery
// database:

      Clock* clock_ptr=get_AnimationController_ptr()->get_clock_ptr();
      clock_ptr->convert_elapsed_secs_to_date(epoch);

      threevector platform_lla,platform_rpy;
      imagesdatabasefunc::retrieve_particular_platform_metadata_from_database(
         images_database_ptr,campaign_ID,mission_ID,image_ID,
         platform_lla,platform_rpy);
//      cout << "platform_lla = " << platform_lla
//           << " platform_rpy = " << platform_rpy << endl;
      geopoint platform_pt(
         platform_lla.get(0),platform_lla.get(1),platform_lla.get(2));

      threevector camera_posn,camera_az_el_roll,f_u0_v0;
      imagesdatabasefunc::retrieve_particular_sensor_metadata_from_database(
         images_database_ptr,campaign_ID,mission_ID,image_ID,
         camera_posn,camera_az_el_roll,f_u0_v0);
//      cout << "camera_posn = " << camera_posn 
//           << " camera_az_el_roll = " << camera_az_el_roll
//           << " f_u0_v0 = " << f_u0_v0 << endl;

// On 5/4/12, we found cases where f, u0 and v0 came back as
// zero-valued.  So we need to explicitly check that the retrieved
// camera parameters are reasonable:
      
      double u0=f_u0_v0.get(1);
      double v0=f_u0_v0.get(2);
      if (!nearly_equal(u0,0) && !nearly_equal(v0,0))
      {
         double aspect_ratio=f_u0_v0.get(1)/f_u0_v0.get(2);
         double FOV_u,FOV_v;
         camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
            f_u0_v0.get(0),aspect_ratio,FOV_u,FOV_v);
         twovector camera_FOV(FOV_u*180/PI,FOV_v*180/PI);
      
         update_dynamic_aircraft_model(
            platform_pt,rpy(platform_rpy),camera_az_el_roll,camera_FOV);
      }
   }
   else if (curr_message.get_text_message()=="UPDATE_CURRENT_BACKPROJECTION")
   {
      curr_message.extract_and_store_property_keys_and_values();
      double camera_posn_x=stringfunc::string_to_number(
         curr_message.get_property_value("camera_posn_x"));
      double camera_posn_y=stringfunc::string_to_number(
         curr_message.get_property_value("camera_posn_y"));
      double camera_posn_z=stringfunc::string_to_number(
         curr_message.get_property_value("camera_posn_z"));
      double rhat_x=stringfunc::string_to_number(
         curr_message.get_property_value("rhat_x"));
      double rhat_y=stringfunc::string_to_number(
         curr_message.get_property_value("rhat_y"));
      double rhat_z=stringfunc::string_to_number(
         curr_message.get_property_value("rhat_z"));
      string label=curr_message.get_property_value("label");

      threevector camera_posn(camera_posn_x,camera_posn_y,camera_posn_z);
      threevector rhat(rhat_x,rhat_y,rhat_z);

//      cout << "camera_posn = " << camera_posn 
//           << " rhat = " << rhat << endl;
      
      mark_ground_intercept(camera_posn,rhat,label);

   } // text_message conditional

   return message_handled_flag;
}


