// ==========================================================================
// OSGBUTTONSERVER class file
// ==========================================================================
// Last updated on 5/10/10; 12/4/10; 1/10/11; 3/20/11
// ==========================================================================

#include <iostream>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include "color/colorfuncs.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "general/filefuncs.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "track/movers_group.h"
#include "templates/mytemplates.h"
#include "Qt/web/OSGButtonServer.h"
#include "general/outputfuncs.h"
#include "geometry/polyline.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"
#include "track/tracks_group.h"

#include "astro_geo/geopoint.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void OSGButtonServer::allocate_member_objects()
{
}		       

void OSGButtonServer::initialize_member_objects()
{
   ModeController_ptr=NULL;
   WindowManager_ptr=NULL;
   AnimationController_ptr=NULL;
   CylindersGroup_ptr=NULL;
   TrackLinesGroup_ptr=NULL;
   ROILinesGroup_ptr=NULL;
   KOZLinesGroup_ptr=NULL;
   ROILinePickHandler_ptr=NULL;
   KOZLinePickHandler_ptr=NULL;
   EarthRegionsGroup_ptr=NULL;
   EarthRegion_ptr=NULL;
   BluegrassClient_ptr=NULL;
   SKSClient_ptr=NULL;
   pointfinder_ptr=NULL;
   MoviesGroup_ptr=NULL;
   OBSFRUSTAGROUP_ptr=NULL;
   SignPostsGroup_ptr=NULL;
   MODELSGROUP_ptr=NULL;
   Predator_MODELSGROUP_ptr=NULL;
   Decorations_ptr=NULL;
   Operations_ptr=NULL;
   movers_group_ptr=NULL;
   ARs_PolyhedraGroup_ptr=NULL;
   KOZs_CylindersGroup_ptr=NULL;

   video_state=0;
   ROI_state=LOI_state=0;
   n_video_states=n_ROI_states=n_LOI_states=2;
   n_cumulative_tracks=0;
   ARs_PolyhedraGroup_OSGsubPAT_number=0;
}

OSGButtonServer::OSGButtonServer(
   string host_IP_address,qint16 port, QObject* parent) :
   WebServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
OSGButtonServer::~OSGButtonServer()
{
   EarthRegion_ptr->purge_ROI_tracks_group_ptrs();
}

// ---------------------------------------------------------------------
void OSGButtonServer::set_EarthRegionsGroup_ptr(EarthRegionsGroup* ERG_ptr)
{
   EarthRegionsGroup_ptr=ERG_ptr;

   EarthRegion_ptr=EarthRegionsGroup_ptr->get_ID_labeled_EarthRegion_ptr(0);
   movers_group_ptr=EarthRegion_ptr->get_movers_group_ptr();
   ROILinesGroup_ptr=EarthRegion_ptr->get_ROILinesGroup_ptr();
   KOZLinesGroup_ptr=EarthRegion_ptr->get_KOZLinesGroup_ptr();
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void OSGButtonServer::set_Decorations_ptr(Decorations* D_ptr)
{
//   cout << "inside OSGButtonServer::set_Decorations_ptr()" << endl;
   
   Decorations_ptr=D_ptr;

   CylindersGroup_ptr=Decorations_ptr->get_CylindersGroup_ptr();
   TrackLinesGroup_ptr=Decorations_ptr->get_PolyLinesGroup_ptr();
   SignPostsGroup_ptr=Decorations_ptr->get_SignPostsGroup_ptr(0);
   MODELSGROUP_ptr=Decorations_ptr->get_MODELSGROUP_ptr();
}

void OSGButtonServer::set_Operations_ptr(Operations* Op_ptr)
{
   Operations_ptr=Op_ptr;

   ModeController_ptr=Op_ptr->get_ModeController_ptr();
   AnimationController_ptr=Op_ptr->get_AnimationController_ptr();
}

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray OSGButtonServer::get( 
   const QUrl& url, QHttpResponseHeader& responseHeader)
{
//   cout << "inside OSGButtonServer:get()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );
   string URL_path;
   return get(doc,response,url,URL_path,responseHeader);
}

// ---------------------------------------------------------------------
QByteArray OSGButtonServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
   cout << "inside 2nd OSGButtonServer:get() method" << endl;

   Q_UNUSED(responseHeader);

   doc.appendChild( response );

   URL_path=url.path().toStdString();
   cout << "URL path = " << URL_path << endl;

// Video button response:
    
   if (URL_path=="/PLAY_MOVIE/")
   {
//      cout << "Playing movie" << endl;
      AnimationController_ptr->setState(AnimationController::PLAY);
   }
   else if (URL_path=="/PAUSE_MOVIE/")
   {
//      cout << "Pausing movie" << endl;
      AnimationController_ptr->setState(AnimationController::PAUSE);
   }
   else if (URL_path=="/FAST_FORWARD_MOVIE/")
   {
      AnimationController_ptr->setState(
         AnimationController::JUMP_FORWARD);
   }
   else if (URL_path=="/RESTART_MOVIE/")
   {
//      cout << "Restart movie button pressed" << endl;
      AnimationController_ptr->set_curr_framenumber(
         AnimationController_ptr->get_first_framenumber());
   }

// ROI & vehicle track buttons event handling:

   else if (URL_path=="/TOGGLE_ACTIVITY_REGIONS/")
   {
      toggle_activity_regions();
   }
   else if (URL_path=="/TOGGLE_SENSOR/")
   {
      toggle_sensor();
   }
   else if (URL_path=="/TOGGLE_ROADS/")
   {
      toggle_roads();
   }
   else if (URL_path=="/TOGGLE_VIDEO/")
   {
      toggle_video();
   }
   else if (URL_path=="/BIG_LOI/" && SignPostsGroup_ptr != NULL
            && LOI_state==BIG_LOI)
   {
      increase_SignPosts_size();
   }
   else if (URL_path=="/SMALL_LOI/" && SignPostsGroup_ptr != NULL
            && LOI_state==SMALL_LOI)
   {
      decrease_SignPosts_size();
   }
   else if (URL_path=="/ENTER_ROI/" && ModeController_ptr != NULL
            && ROI_state==ENTER_ROI)
   {
      enter_ROI();
   }
   else if (URL_path=="/MONITOR_ROI/" && ModeController_ptr != NULL 
            && ROI_state==MONITOR_ROI)
   {
      monitor_ROI();
   }
   else if (URL_path=="/CLEAR_VEHICLE_TRACKS/")
   {
      clear_vehicle_tracks();
   }
   else if (URL_path=="/CLEAR_ROIS/")
   {
      clear_ROIs();
   }
   else if (URL_path=="/ENTER_KOZ/" && ModeController_ptr != NULL)
   {
      enter_KOZ();
   }
   else if (URL_path=="/CLEAR_KOZS/")
   {
      clear_KOZs();
   }
   else if (URL_path=="/SNAP_SCREEN/" &&
            CylindersGroup_ptr->get_selected_Graphical_ID() >= 0)
   {
      snap_screen();
   } 
   else if (URL_path=="/SHOW_SPEEDS/")
   {
      show_speeds();
   }
   else if (URL_path=="/HIDE_SPEEDS/")
   {
      hide_speeds();
   } 
   else if (URL_path=="/NOMINATE_ROIS/")
   {
      nominate_ROIs();
   }
   else if (URL_path=="/ENTER_UAV_PATH/")
   {
      enter_UAV_path();
   }
   else if (URL_path=="/SELECT_UAV_PATH/")
   {
      select_UAV_path();
   }
   else if (URL_path=="/ALTER_UAV_PATH/")
   {
      alter_UAV_path();
   } 
   else if (URL_path=="/CLEAR_UAV_PATHS/")
   {
      clear_UAV_paths();
   } 
   else if (URL_path=="/COMPUTE_UAV_PATH/")
   {
      compute_UAV_path();
   } // URL path conditional

   return doc.toByteArray();
}

// ==========================================================================
// ROI & vehicle track button event handling member functions
// ==========================================================================

void OSGButtonServer::toggle_activity_regions()
{
//   cout << "inside OSGButtonServer::toggle_activity_regions()" << endl;
   
   if (ARs_PolyhedraGroup_ptr == NULL) return;

   cout << "Toggling Activity Regions" << endl;

   ARs_PolyhedraGroup_ptr->toggle_OSGsubPAT_nodemask(
      ARs_PolyhedraGroup_OSGsubPAT_number);
}

// ---------------------------------------------------------------------
void OSGButtonServer::toggle_sensor()
{
//   cout << "inside OSGButtonServer::toggle_sensor()" << endl;

   if (MODELSGROUP_ptr == NULL) return;

   cout << "Toggling sensor view" << endl;
   for (int i=0; i<MODELSGROUP_ptr->get_n_OSGsubPATs(); i++)
   {
      MODELSGROUP_ptr->toggle_model_mask(i);
   }
}

// ---------------------------------------------------------------------
void OSGButtonServer::toggle_roads()
{
   cout << "Toggling road network" << endl;
   PolyLinesGroup* roadlines_group_ptr=
      EarthRegion_ptr->get_roadlines_group_ptr();
   roadlines_group_ptr->toggle_OSGgroup_nodemask();
      
// As of 5/20/08, we asssume that the last Movie within
// *MoviesGroup_ptr corresponds to a surface texture which
// significantly overlaps the roads network contained in
// *roadlines_group_ptr.  We adjust the road network's altitude so
// that it lies slightly above the current surface texture's altitude:

   if (MoviesGroup_ptr != NULL)
   {
      Movie* movie_texture_ptr=dynamic_cast<Movie*>(
         MoviesGroup_ptr->get_last_Graphical_ptr());

      osg::PositionAttitudeTransform* OSGsubPAT_ptr=
         roadlines_group_ptr->get_OSGsubPAT_ptr(0);
      threevector posn(OSGsubPAT_ptr->getPosition());

      double curr_roadlines_height=
         movie_texture_ptr->get_absolute_altitude(
            MoviesGroup_ptr->get_curr_t(),
            MoviesGroup_ptr->get_passnumber())+5;
      posn.put(2,curr_roadlines_height);

      OSGsubPAT_ptr->setPosition(osg::Vec3d(
         posn.get(0),posn.get(1),posn.get(2)));
   } // MoviesGroup_ptr != NULL conditional
}

// ---------------------------------------------------------------------
// Member function toggle_video toggles MoviesGroup nodemask.  If the
// video is to be toggled from on to off, this method sets the alpha
// values for all Movies within *MoviesGroup_ptr equal to zero.  (UAV
// video chips along with Constant Hawk video are then all hidden, and
// main program throughput is significantly faster.)  If the video is
// to be toggled from off to on, alpha for the zeroth Movie (assumed
// to correspond to the Constant Hawk video) is initialized to zero so
// that it can later be faded in via the Up arrow key.  On the other
// hand, all other Movies (assumed to correspond to UAV video chips)
// have their alphas set equal to unity so that they immediately appear.

void OSGButtonServer::toggle_video()
{
//   cout << "inside OSGButtonServer::toggle_video()" << endl;
   if (MoviesGroup_ptr != NULL) toggle_video(MoviesGroup_ptr);
}

/*
void OSGButtonServer::toggle_video()
{
   cout << "inside OSGButtonServer::toggle_video()" << endl;

   vector<MoviesGroup*> MoviesGroup_ptrs;
   MoviesGroup_ptrs.push_back(MoviesGroup_ptr);

   if (OBSFRUSTAGROUP_ptr != NULL && 
       OBSFRUSTAGROUP_ptr->get_MoviesGroup_ptr() != NULL) 
   {
      MoviesGroup_ptrs.push_back(OBSFRUSTAGROUP_ptr->get_MoviesGroup_ptr());
   }

   if (Predator_MODELSGROUP_ptr != NULL)
   {
      vector<MODEL*> MODEL_ptrs=Predator_MODELSGROUP_ptr->
         get_all_MODEL_ptrs();
      for (int m=0; m<int(MODEL_ptrs.size()); m++)
      {
         cout << "UAV model index m = " << m << endl;
         MODEL* UAV_MODEL_ptr=MODEL_ptrs[m];
         OBSFRUSTAGROUP* UAV_OBSFRUSTAGROUP_ptr=UAV_MODEL_ptr->
            get_OBSFRUSTAGROUP_ptr();
         if (UAV_OBSFRUSTAGROUP_ptr != NULL &&
             UAV_OBSFRUSTAGROUP_ptr->get_MoviesGroup_ptr() != NULL)
         {
            MoviesGroup_ptrs.push_back(
               UAV_OBSFRUSTAGROUP_ptr->get_MoviesGroup_ptr());
         }
      } // loop over index m labeling Predator MODELS
   }

   for (int iter=0; iter<int(MoviesGroup_ptrs.size()); iter++)
   {
      cout << "iter = " << iter
           << " MoviesGroup_ptrs[iter] = " << MoviesGroup_ptrs[iter]
           << endl;
      toggle_video(MoviesGroup_ptrs[iter]);
   } 
}
*/

void OSGButtonServer::toggle_video(MoviesGroup* local_MoviesGroup_ptr)
{
//   cout << "inside OSGButtonServer::toggle_video(local_MoviesGroup_ptr)" 
//        << endl;
//   cout << "local_MoviesGroup_ptr = " << local_MoviesGroup_ptr << endl;
//   cout << "initially, local_MoviesGroup_ptr->get_OSGgroup_nodemask() = "
//        << local_MoviesGroup_ptr->get_OSGgroup_nodemask() << endl;

   if (local_MoviesGroup_ptr->get_OSGgroup_nodemask()==0)
   {
      cout << "Toggling video from off to on" << endl;
      for (int i=0; i<local_MoviesGroup_ptr->get_n_Graphicals(); i++)
      {
         Movie* Movie_ptr=local_MoviesGroup_ptr->get_Movie_ptr(i);

// Reset CH video alpha to zero so that it can be faded in via the up
// arrow key.  But set alphas for all other UAV video chips equal to
// one:

         if (i==0)
         {
            Movie_ptr->set_alpha(0.0);
         }
         else
         {
            Movie_ptr->set_alpha(1.0);
         }
      }
   }
   else
   {
      cout << "Toggling video from on to off" << endl;
      for (int i=0; i<MoviesGroup_ptr->get_n_Graphicals(); i++)
      {
         Movie* Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(i);
         Movie_ptr->set_alpha(0.0);
      }
   } // MoviesGroup.OSGgroup_nodemask <= 0 conditional
   local_MoviesGroup_ptr->toggle_OSGgroup_nodemask();

//   cout << "finally, local_MoviesGroup_ptr->get_OSGgroup_nodemask() = "
//        << local_MoviesGroup_ptr->get_OSGgroup_nodemask() << endl;
}

// ---------------------------------------------------------------------
void OSGButtonServer::increase_SignPosts_size()
{
   cout << "Increasing LOI size" << endl;
   SignPostsGroup_ptr->set_size(40.0);

   LOI_state++;
   LOI_state=modulo(LOI_state,n_LOI_states);
}

// ---------------------------------------------------------------------
void OSGButtonServer::decrease_SignPosts_size()
{
   cout << "Decreasing LOI size" << endl;
//      SignPostsGroup_ptr->set_size(4.0);
   SignPostsGroup_ptr->set_size(3.0);
//      SignPostsGroup_ptr->set_size(2.0);

   LOI_state++;
   LOI_state=modulo(LOI_state,n_LOI_states);
}

// ---------------------------------------------------------------------
void OSGButtonServer::enter_ROI()
{
//   cout << "inside OSGButtonServer::enter_ROI()" << endl;
   cout << "Entering ROI" << endl;

   ROILinePickHandler_ptr->set_disable_input_flag(false);

   ModeController_ptr->setState( ModeController::INSERT_POLYLINE );

   ROI_state++;
   ROI_state=modulo(ROI_state,n_ROI_states);
//   cout << "n_ROI_states = " << n_ROI_states << endl;

   tracks_group* spatially_fixed_tracks_group_ptr=
      EarthRegion_ptr->get_spatially_fixed_tracks_group_ptr();
   mover* new_mover_ptr=movers_group_ptr->generate_new_ROI(
      spatially_fixed_tracks_group_ptr);
//   new_mover_ptr->set_annotation_label("All tracks");

   int curr_ROI_ID=new_mover_ptr->get_ID();
//      cout << "new_mover_ptr->get_ID() = " << curr_ROI_ID << endl;
   string label="ROI "+stringfunc::number_to_string(curr_ROI_ID);
//      cout << "ROI label = " << label << endl;
   ROILinesGroup_ptr->set_next_PolyLine_label(label);
   ROILinesGroup_ptr->set_next_PolyLine_mover_ptr(new_mover_ptr);

// Set PickHandler processing flags so that ROILinePickHandler handles
// ROI vertex input and NOT KOZLinePickHandler:

   ROILinePickHandler_ptr->set_process_pick_flag(true);
   KOZLinePickHandler_ptr->set_process_pick_flag(false);

   movers_group_ptr->add_mover_to_outgoing_queue(new_mover_ptr);
}

// ---------------------------------------------------------------------
void OSGButtonServer::monitor_ROI()
{
//   cout << "inside OSGButtonServer::monitor_ROI()" << endl;
   cout << "Monitoring ROI" << endl;

   ROILinePickHandler_ptr->set_disable_input_flag(false);

   polyline* ROI_polyline_ptr=NULL;
   string SKS_query=EarthRegionsGroup_ptr->
      form_SKS_query_for_tracks_intersecting_polyline(
         ROILinesGroup_ptr,ROI_polyline_ptr);
//   cout << "SKS_query = " << SKS_query << endl;

/*
// Wrote next few lines in order to compute longitude,latitude
// coordinates for ROIs.  On 8/7/08, we gave these coords to Tim
// Schreiner for incorporation into a Bluegrass KML layer:

   polygon ROI_poly(*ROI_polyline_ptr);
   threevector ROI_COM=ROI_poly.compute_COM();
   bool northern_hemisphere_flag=true;
   int UTM_zone=14;	// Lubbock, TX
   geopoint ROI_center(northern_hemisphere_flag,UTM_zone,
                       ROI_COM.get(0),ROI_COM.get(1));
   cout << "ROI_center.longitude = " << ROI_center.get_longitude() << endl;
   cout << "ROI_center.latitude = " << ROI_center.get_latitude() << endl;
*/

   if (SKS_query.size() > 0)
   {
      string URL_subdir="/SKSDataServer/track";
      SKSClient_ptr->query_SKSDataServer(URL_subdir,SKS_query);
      string SKS_response=SKSClient_ptr->get_SKSDataServer_response();
//      cout << "SKS_response = " << SKS_response << endl;

      int specified_UTM_zonenumber=14;
      double secs_offset=0;
      double tracks_altitude=MoviesGroup_ptr->get_total_altitude(0)+5;
//      cout << "tracks_altitude = " << tracks_altitude << endl;

      tracks_group* ROI_tracks_group_ptr=EarthRegion_ptr->
         generate_ROI_tracks_group();

      n_cumulative_tracks += BluegrassClient_ptr->retrieve_mover_tracks(
         SKS_response,secs_offset,tracks_altitude,ROI_tracks_group_ptr);

// Make network associations between vehicles and last entered ROI:

      EarthRegion_ptr->generate_track_colors(
         n_cumulative_tracks,ROI_tracks_group_ptr);

// Recall that message queue broadcasts PolyLine colors for tracks so
// that edges in Michael Yee's social network program can be color
// coordinated.  So the following call to associate_vehicles_with_ROI
// must occur AFTER the previous call to generate_track_colors():

      int latest_ROI_ID=movers_group_ptr->get_latest_ROI_ID();
      movers_group_ptr->associate_vehicles_with_ROI(
         latest_ROI_ID,ROI_tracks_group_ptr);

// Eliminate from ROI_tracks_group_ptr those tracks which already
// exist within STL vector EarthRegion::ROIS_tracks_group_ptrs:

      BluegrassClient_ptr->eliminate_repeated_tracks(
         ROI_tracks_group_ptr,EarthRegion_ptr->get_ROI_tracks_group_ptrs());

// Calculate and store speed dependent vehicle track coloring
// information:

      ROI_tracks_group_ptr->compute_speed_dependent_vehicle_track_colors();

      bool multicolor_tracklines_flag=
         TrackLinesGroup_ptr->get_multicolor_flags();

      EarthRegion_ptr->display_tracks_as_PolyLines(
         n_cumulative_tracks,ROI_tracks_group_ptr,pointfinder_ptr,
         TrackLinesGroup_ptr);
      if (multicolor_tracklines_flag) show_speeds();

// Compute times of closest approach to ROI COM for each vehicle whose
// track was retrieved:

      polygon ROI_polygon(*ROI_polyline_ptr);
      threevector ROI_COM=ROI_polygon.compute_COM();
//      cout << "ROI_COM = " << ROI_COM << endl;

      vector<double> approach_times=
         ROI_tracks_group_ptr->closest_approach_times(ROI_COM);

      vector<int> approach_frames;
      Clock clock;
      for (int t=0; t<approach_times.size(); t++)
      {
         approach_frames.push_back(
            AnimationController_ptr->
            get_frame_corresponding_to_time(approach_times[t]));
      }

      if (approach_frames.size() > 0)
      {
         templatefunc::Quicksort(approach_frames,approach_times);
//            std::sort(approach_frames.begin(),approach_frames.end());

         for (int t=0; t<approach_times.size(); t++)
         {
            clock.convert_elapsed_secs_to_date(approach_times[t]);
//               cout << "Approach time = " << clock.YYYY_MM_DD_H_M_S() 
//                    << " approach frame = " << approach_frames[t] << endl;
         }
            
// Reset master clock to 60 secs before first closest approach event.
// Make sure reset time does not precede earliest possible game time.

         int nframes_prior=60*2; 
         int reset_framenumber=approach_frames.front()-nframes_prior;
         reset_framenumber=basic_math::max(
            reset_framenumber,AnimationController_ptr->
            get_first_framenumber());


// FAKE FAKE:  Thurs Jun 17 at 8:19 am
// Comment out next line for debugging purposes only...

//         AnimationController_ptr->set_curr_framenumber(reset_framenumber);


      } // approach_frames.size() > 0 conditional

   } // SKS_query.size() > 0 conditional

   ROI_state++;
   ROI_state=modulo(ROI_state,n_ROI_states);

//   cout << "ROI_state = " << ROI_state << endl;
//   cout << "n_ROI_states = " << n_ROI_states << endl;

// Print out movers group.  For reasons we don't understand as of 
// 8/12/08, we must NOT append a final endl to the cout following cout
// command :

//   cout << "*movers_group_ptr = " << *movers_group_ptr;
}

// ---------------------------------------------------------------------
void OSGButtonServer::clear_vehicle_tracks()
{
   cout << "Clearing vehicle tracks" << endl;

   ROILinePickHandler_ptr->set_disable_input_flag(false);

   TrackLinesGroup_ptr->destroy_all_PolyLines();
   EarthRegion_ptr->purge_ROI_tracks_group_ptrs();
   n_cumulative_tracks=0;

   if (movers_group_ptr != NULL)
   {
      movers_group_ptr->purge_all_particular_movers(mover::VEHICLE);
   }
}

// ---------------------------------------------------------------------
void OSGButtonServer::clear_ROIs()
{
//   cout << "inside OSGButtonServer::clear_ROIs()" << endl;
   cout << "Clearing ROIs" << endl;

   ROILinePickHandler_ptr->set_disable_input_flag(false);

   EarthRegion_ptr->clear_all_ROIs();

   BluegrassClient_ptr->get_auto_nominated_bboxes().clear();
}

// ---------------------------------------------------------------------
void OSGButtonServer::snap_screen()
{
//      cout << "Taking screen snapshot" << endl;
   string rgb_filename="snap.rgb";

   double t_start=timefunc::elapsed_timeofday_time();
   WindowManager_ptr->snap_screen(rgb_filename);
   double max_sleep_time=10;	// secs

   while (timefunc::elapsed_timeofday_time()-t_start < max_sleep_time &&
          !filefunc::fileexist(rgb_filename))
   {
      string unixcommandstr="sleep 0.1";
      sysfunc::unix_command(unixcommandstr);
      WindowManager_ptr->process();
   }
      
   int rounded_world_time=0;
   if (AnimationController_ptr != NULL)
   {
      double world_time=
         AnimationController_ptr->get_time_corresponding_to_frame(
            AnimationController_ptr->get_true_framenumber());
      rounded_world_time=basic_math::round(world_time);
   }
      
// Place snapshot into subdir labeled by Bluegrass vehicle ID:

   Cylinder* Cylinder_ptr=
      CylindersGroup_ptr->get_ID_labeled_Cylinder_ptr(
         CylindersGroup_ptr->get_selected_Graphical_ID());
   string curr_vehicle_label=Cylinder_ptr->get_track_ptr()->get_label();

   string base_subdir=
      "/data/video/2007/Lubbock/constant_hawk/AR1_002/snapshots/";
   set_curr_snapshot_subdir(base_subdir+curr_vehicle_label);
//      cout << "curr_snapshot_subdir = " << curr_snapshot_subdir << endl;

   string png_filename=curr_snapshot_subdir+
      "snapshot_"+stringfunc::number_to_string(
         rounded_world_time)+".png";
//      cout << "png_filename = " << png_filename << endl;

   string unix_commandstr="convert "+rgb_filename+" "+png_filename;
   sysfunc::unix_command(unix_commandstr);

   while (timefunc::elapsed_timeofday_time()-t_start < max_sleep_time &&
          !filefunc::fileexist(png_filename))
   {
      string unixcommandstr="sleep 0.1";
      sysfunc::unix_command(unixcommandstr);
      WindowManager_ptr->process();
   }

   unix_commandstr="rm "+rgb_filename;
   sysfunc::unix_command(unix_commandstr);
} 

// ---------------------------------------------------------------------
// Member function show_speeds locally colors vehicle tracks according to 
// vehicle speeds.

void OSGButtonServer::show_speeds()
{
//   cout << "inside OSGButtonServer::show_speeds()" << endl;
   cout << "Locally color vehicle tracks by speed" << endl;
   TrackLinesGroup_ptr->set_multicolor_flags(true);
 
   for (int g=0; g<EarthRegion_ptr->get_ROI_tracks_group_ptrs().size(); g++)
   {
      tracks_group* tracks_group_ptr=
         EarthRegion_ptr->get_ROI_tracks_group_ptrs().at(g);

      vector<track*> track_ptrs=tracks_group_ptr->get_all_track_ptrs();
      for (int t=0; t<int(track_ptrs.size()); t++)
      {
         track* curr_track_ptr=track_ptrs[t];
         int PolyLine_ID=curr_track_ptr->get_PolyLine_ID();
         PolyLine* curr_PolyLine_ptr=TrackLinesGroup_ptr->
            get_ID_labeled_PolyLine_ptr(PolyLine_ID);
         vector<osg::Vec4> velocity_colors=curr_track_ptr->
            get_velocity_colors();
         curr_PolyLine_ptr->set_local_colors(velocity_colors);
//         curr_PolyLine_ptr->set_colors(velocity_colors);
      } // loop over index t labeling tracks
   } // loop over index g labeling ROI_tracks_groups

   bool show_arrowheads_flag=false;
   set_vehicle_tracks_arrowheads_nodemask(show_arrowheads_flag);
}

// ---------------------------------------------------------------------
void OSGButtonServer::hide_speeds()
{
   cout << "Globally color vehicle tracks" << endl;
   TrackLinesGroup_ptr->set_multicolor_flags(false);
   TrackLinesGroup_ptr->reset_colors();
   bool show_arrowheads_flag=true;
   set_vehicle_tracks_arrowheads_nodemask(show_arrowheads_flag);
} 

// ---------------------------------------------------------------------
// Member function nominate_ROIs forms an SKS query for all tracklets
// where the vehicle's speed lies within the interval
// [minSpeed,maxSpeed] for at least minDuration seconds.  SKS returns
// a set of bounding box (longitude,latitude) coordinates which are
// used to instantiate bbox ROI PolyLines.  It also returns the
// vehicle labels for those tracks within these bounding boxes which
// satisfy the slow speed criteria.  It is important to note that
// other tracks which do not satisfy the slow speed criteria and yet
// also intersect these bounding boxes are NOT associated with the
// nominated ROIs.

void OSGButtonServer::nominate_ROIs()
{
//    cout << "inside OSGButtonServer::nominate_ROIs()" << endl;
   cout << "Nominating ROIs" << endl;
      
// Experiment with automatically nominating ROIs based upon vehicle
// speed information:

   double minSpeed=0;
   double maxSpeed=0.25;	
//      double maxSpeed=0.5;	
//      double maxSpeed=1.1;	// 1.1 m/s = 2.5 mph
//      double maxSpeed=2.23;	// 2.23 m/s = 5 mph
//      double minDuration=20;	// secs
//      double minDuration=60;	// secs
//      double minDuration=300;	// secs
   double minDuration=600;	// secs		// good bluegrass demo value
//   double minDuration=900;	// secs		// OK value for UAV demo
//      double minDuration=120;	// secs
//      double radius=10;		// meters
   double radius=20;
//      double radius=30;		// meters
//      double radius=35;		// meters
   string SKS_query=EarthRegionsGroup_ptr->form_SKS_query_for_speed_ROIs(
      minSpeed,maxSpeed,minDuration,radius,
      EarthRegion_ptr->get_ROI_tracks_group_ptrs());
//   cout << "SKS speed query = " << SKS_query << endl;

   if (SKS_query.size() > 0)
   {
      string URL_subdir="/SKSDataServer/track";
      SKSClient_ptr->query_SKSDataServer(URL_subdir,SKS_query);
      string SKS_response=SKSClient_ptr->get_SKSDataServer_response();
//      cout << "SKS_response = " << SKS_response << endl;
//      cout << "SKS_response.size() = " <<  SKS_response.size() << endl;

      vector<vector<int> > label_IDs_for_slow_vehicles_passing_thru_ROIs;
      vector<polyline*> ROI_polyline_ptrs=
         BluegrassClient_ptr->generate_ROIs_from_parsed_XML(
            SKS_response,label_IDs_for_slow_vehicles_passing_thru_ROIs);
//      cout << "ROI_polyline_ptrs.size() = " << ROI_polyline_ptrs.size()
//           << endl;

//      for (int l=0; l<label_IDs_for_slow_vehicles_passing_thru_ROIs.size(); 
//           l++)
//      {
//         vector<int> vehicle_label_IDs=
//            label_IDs_for_slow_vehicles_passing_thru_ROIs[l];
//         for (int v=0; v<vehicle_label_IDs.size(); v++)
//         {
//            cout << "l = " << l << " v = " << v
//                 << " label_ID = " << vehicle_label_IDs[v] << endl;
//         }
//      }
   
// FAKE FAKE:  Hack added on Thurs July 24 at 6:10 am...
// Hardwire bbox for Activity Region #1 ( = bbox # 0 !!!)

      bounding_box* Activity_Region_bbox_ptr=
         EarthRegion_ptr->get_bbox_ptr(0);
//      cout << "Activity Region bbox = " << *Activity_Region_bbox_ptr << endl;
      for (int m=0; m<ROI_polyline_ptrs.size(); m++)
      {
//         cout << "m = " << m << endl;
         bool force_display_flag=false;
         bool single_polyline_per_geode_flag=true;

         vector<threevector> vertices;
         polyline* curr_polyline_ptr=ROI_polyline_ptrs[m];
//            cout << "curr_polyline_ptr = " << curr_polyline_ptr << endl;
//            cout << "*curr_polyline_ptr = " << *curr_polyline_ptr << endl;

         bool polyline_outside_bbox_flag=false;
         for (int i=0; i<curr_polyline_ptr->get_n_vertices(); i++)
         {
            threevector curr_vertex(curr_polyline_ptr->get_vertex(i));
            if (!Activity_Region_bbox_ptr->point_inside(
               curr_vertex.get(0),curr_vertex.get(1)))
            {
               polyline_outside_bbox_flag=true;
//                  cout << "*Activity_Region_bbox_ptr = " 
//		         << *Activity_Region_bbox_ptr << endl;
//                  cout << "curr_vertex = " << curr_vertex << endl;
//                  cout << "polyline vertex lies outside bbox!" << endl;
//                  outputfunc::enter_continue_char();
            }
            else
            {
               curr_vertex.put(
                  2,ROILinesGroup_ptr->get_grid_world_origin().get(2));
               vertices.push_back(curr_vertex);
//               cout << "i = " << i << " vertices.back() = " 
//                    << vertices.back() << endl;
            }
         } // loop over index i labeling vertices in curr ROI polyline

// Perform sanity checks upon candidate *curr_polyline_ptr:

         if (curr_polyline_ptr->get_n_vertices() < 4 ||
             polyline_outside_bbox_flag)
         {
            cout << "Current polyline looks bad!!!" << endl;
            cout << "curr_polyline_ptr->get_n_vertices() = "
                 << curr_polyline_ptr->get_n_vertices() << endl;
            cout << "polyline_outside_bbox_flag = "
                 << polyline_outside_bbox_flag << endl;
            continue;
         }

//         colorfunc::Color ROI_color=colorfunc::white;
         string annotation_label="Slow";
         mover* nominated_ROI_mover_ptr=EarthRegion_ptr->
            generate_nominated_ROI(
               vertices,EarthRegionsGroup_ptr->get_ROI_color(),
               annotation_label);

         movers_group_ptr->associate_vehicles_with_ROI(
            nominated_ROI_mover_ptr->get_ID(),
            label_IDs_for_slow_vehicles_passing_thru_ROIs[m]);

      } // loop over index m labeling autogenerated ROI polylines

// Print out movers group.  For reasons we don't understand as of
// 8/12/08, we must NOT append a final endl to the cout following cout
// command:

//      cout << "*movers_group_ptr = " << *movers_group_ptr;
   } // SKS_query.size() > 0 conditional
}

// ---------------------------------------------------------------------
void OSGButtonServer::set_curr_snapshot_subdir(std::string subdir)
{
//   cout << "inside OSGButtonServer::set_curr_snapshot_subdir" << endl;
//   cout << "subdir = " << subdir << endl;
   curr_snapshot_subdir=subdir;
   filefunc::add_trailing_dir_slash(curr_snapshot_subdir);
   filefunc::dircreate(curr_snapshot_subdir);
}

// ---------------------------------------------------------------------
void OSGButtonServer::set_vehicle_tracks_arrowheads_nodemask(
   bool show_arrowheads_flag)
{
//   cout << "inside OSGButtonServer::set_vehicle_tracks_arrowheads_nodemask()"
//        << endl;

   for (int g=0; g<EarthRegion_ptr->get_ROI_tracks_group_ptrs().size(); g++)
   {
      tracks_group* tracks_group_ptr=EarthRegion_ptr->
         get_ROI_tracks_group_ptrs().at(g);

      vector<track*> track_ptrs=tracks_group_ptr->get_all_track_ptrs();
      for (int t=0; t<int(track_ptrs.size()); t++)
      {
         track* curr_track_ptr=track_ptrs[t];
         int PolyLine_ID=curr_track_ptr->get_PolyLine_ID();
         PolyLine* curr_PolyLine_ptr=TrackLinesGroup_ptr->
            get_ID_labeled_PolyLine_ptr(PolyLine_ID);
         curr_PolyLine_ptr->get_ConesGroup_ptr()->
            set_OSGgroup_nodemask(show_arrowheads_flag);
      } // loop over index t labeling tracks
   } // loop over index g labeling ROI_tracks_groups
}

// ==========================================================================
// UAV button event handling member functions
// ==========================================================================

void OSGButtonServer::enter_UAV_path()
{
   cout << "Entering UAV path" << endl;
   ModeController_ptr->setState( ModeController::INSERT_POLYLINE );
   ROILinePickHandler_ptr->set_disable_input_flag(true);
   KOZLinePickHandler_ptr->set_disable_input_flag(true);
}

// ---------------------------------------------------------------------
void OSGButtonServer::select_UAV_path()
{
   cout << "Selecting UAV path" << endl;
   ModeController_ptr->setState( ModeController::MANIPULATE_POLYLINE );
   ROILinePickHandler_ptr->set_disable_input_flag(true);
   KOZLinePickHandler_ptr->set_disable_input_flag(true);

}

// ---------------------------------------------------------------------
void OSGButtonServer::alter_UAV_path()
{
   cout << "Altering UAV path" << endl;

   ROILinePickHandler_ptr->set_disable_input_flag(true);
   KOZLinePickHandler_ptr->set_disable_input_flag(true);

   PolyLinesGroup* Path_PolyLinesGroup_ptr=Predator_MODELSGROUP_ptr->
      get_Path_PolyLinesGroup_ptr();
   if (Path_PolyLinesGroup_ptr != NULL)
   {

// For UAV path altering purposes, reset UAV PolyLine's altitude so
// that it lies close to world grid.  Perspective effects are then
// significantly reduced, and it is much easier to alter the flight
// path relative to the background map:

      Path_PolyLinesGroup_ptr->set_constant_vertices_altitude(
         Path_PolyLinesGroup_ptr->get_grid_world_origin().get(2)+50);
      int selected_PolyLine_ID=Path_PolyLinesGroup_ptr->
         get_selected_Graphical_ID();
      if (selected_PolyLine_ID < 0)
      {
         cout << "No UAV path to alter yet selected!" << endl;
      }
      else
      {
         PolyLine* Path_PolyLine_ptr=Path_PolyLinesGroup_ptr->
            get_ID_labeled_PolyLine_ptr(selected_PolyLine_ID);
         Path_PolyLinesGroup_ptr->reset_PolyLine_altitudes(
            Path_PolyLine_ptr);
         Predator_MODELSGROUP_ptr->set_ID_for_path_to_alter(
            selected_PolyLine_ID);
         Predator_MODELSGROUP_ptr->set_alter_UAV_path_flag(true);
      }
   } // Path_PolyLinesGroup_ptr != NULL conditional
}

// ---------------------------------------------------------------------
void OSGButtonServer::clear_UAV_paths()
{
//   cout << "Inside OSGButtonServer::clear_UAV_paths()" << endl;
   cout << "Clearing UAV paths" << endl;

   ROILinePickHandler_ptr->set_disable_input_flag(true);
   KOZLinePickHandler_ptr->set_disable_input_flag(true);

   PolyLinesGroup* Path_PolyLinesGroup_ptr=Predator_MODELSGROUP_ptr->
      get_Path_PolyLinesGroup_ptr();
   if (Path_PolyLinesGroup_ptr != NULL)
   {
      int selected_PolyLine_ID=Path_PolyLinesGroup_ptr->
         get_selected_Graphical_ID();
//      cout << "selected_PolyLine_ID = " << selected_PolyLine_ID << endl;

      vector<int> destroyed_track_IDs;
      if (selected_PolyLine_ID > -1)
      {
         Path_PolyLinesGroup_ptr->destroy_PolyLine();

         destroyed_track_IDs.push_back(selected_PolyLine_ID);
         Predator_MODELSGROUP_ptr->get_tracks_group_ptr()->
            destroy_track(selected_PolyLine_ID);
         Predator_MODELSGROUP_ptr->destroy_MODEL(selected_PolyLine_ID);

         Predator_MODELSGROUP_ptr->get_movers_group_ptr()->
            delete_mover(mover::UAV,selected_PolyLine_ID);
         Path_PolyLinesGroup_ptr->set_selected_Graphical_ID(-1);
      }
      else
      {
         destroyed_track_IDs=
            Predator_MODELSGROUP_ptr->purge_UAV_MODELS_and_tracks();
      }

//      cout << "After deleting track(s), UAV tracks group = " 
//           << *(Predator_MODELSGROUP_ptr->get_tracks_group_ptr())
//           << endl;

      Predator_MODELSGROUP_ptr->
         broadcast_delete_tracks_to_GoogleEarth_channel(destroyed_track_IDs);
      
   } // Path_PolyLinesGroup_ptr != NULL conditional
}

// ---------------------------------------------------------------------
void OSGButtonServer::compute_UAV_path()
{
   cout << "Computing UAV path" << endl;

   ROILinePickHandler_ptr->set_disable_input_flag(true);
   KOZLinePickHandler_ptr->set_disable_input_flag(true);

//   timefunc::initialize_timeofday_clock();
//   double t_before_path_planning_algs=timefunc::elapsed_timeofday_time();
//   cout << "Before calling path planning algorithms, t = "
//        << t_before_path_planning_algs << endl;

   if (Predator_MODELSGROUP_ptr->get_Path_PolyLinesGroup_ptr() == NULL ||
       Predator_MODELSGROUP_ptr->get_Path_PolyLinePickHandler_ptr() == NULL)
   {
      return;
   }
   
   Predator_MODELSGROUP_ptr->broadcast_sensor_and_target_statevectors();

/*
// ********************************************************************
// FAKE FAKE: Sun Aug 3 at 7 am For now, we simply fill ROI_IDs with a
// trivial ordering of ROI IDs.  This section should be commented out
// when we're connected to Luca's MATLAB codes:

   vector<int> ROI_IDs=movers_group_ptr->
      get_particular_mover_IDs(mover::ROI);

   for (int j=0; j<ROI_IDs.size(); j++)
   {
      cout << "j = " << j << " ROI_IDs[j] = " << ROI_IDs[j]
           << endl;
   }

   int n_UAVs=Predator_MODELSGROUP_ptr->get_n_Graphicals();
   cout << "n_UAVs = " << n_UAVs << endl;
   int ROI_step=ROI_IDs.size()/n_UAVs;
   cout << "ROI_step = " << ROI_step << endl;
   vector<int> start_ROI_ID,stop_ROI_ID;

   start_ROI_ID.push_back(0);
   for (int iter=0; iter<n_UAVs-1; iter++)
   {
      stop_ROI_ID.push_back((iter+1)*ROI_step);
      int next_ROI_ID=stop_ROI_ID.back()+1;
      next_ROI_ID=min(next_ROI_ID,ROI_IDs.back());
      start_ROI_ID.push_back(next_ROI_ID);
   }
   stop_ROI_ID.push_back(ROI_IDs.back());

   for (int i=0; i<start_ROI_ID.size(); i++)
   {
      cout << "i = "<< i
           << " start_ROI_ID = " << start_ROI_ID[i]
           << " stop_ROI_ID = " << stop_ROI_ID[i] << endl;
   }

   vector<Messenger::Property> properties;

   for (int n=0; n < n_UAVs; n++)
   {
      int UAV_ID=n;
      string key="UAVID_"+stringfunc::number_to_string(UAV_ID);
      string value;
      for (int i=start_ROI_ID[n]; i<= stop_ROI_ID[n]; i++)
      {
         value += stringfunc::number_to_string(i);
         if (i < stop_ROI_ID[n]) value += ",";
      }
      cout << "value = " << value << endl;
      properties.push_back(Messenger::Property(key,value));
   
      Predator_MODELSGROUP_ptr->get_Messenger_ptr()->broadcast_subpacket(
         "START_PACKET");
      Predator_MODELSGROUP_ptr->get_Messenger_ptr()->broadcast_subpacket(
         "ASSIGN_TASK",properties);
      Predator_MODELSGROUP_ptr->get_Messenger_ptr()->broadcast_subpacket(
         "STOP_PACKET");
   } // loop over UAV IDs

// ********************************************************************
*/

}

// ==========================================================================
// KOZ button event handling member functions
// ==========================================================================

void OSGButtonServer::enter_KOZ()
{
//   cout << "inside OSGButtonServer::enter_KOZ()" << endl;
   cout << "Entering KOZ" << endl;
   ModeController_ptr->setState( ModeController::INSERT_POLYLINE );

//   tracks_group* spatially_fixed_tracks_group_ptr=
   tracks_group* KOZ_tracks_group_ptr=
      EarthRegion_ptr->get_KOZ_tracks_group_ptr();
//      EarthRegion_ptr->get_spatially_fixed_tracks_group_ptr();
   mover* new_mover_ptr=movers_group_ptr->generate_new_KOZ(
      KOZ_tracks_group_ptr);
//      spatially_fixed_tracks_group_ptr);

   int curr_KOZ_ID=new_mover_ptr->get_ID();
//      cout << "new_mover_ptr->get_ID() = " << curr_KOZ_ID << endl;
   string label="KOZ "+stringfunc::number_to_string(curr_KOZ_ID);
//      cout << "KOZ label = " << label << endl;
   KOZLinesGroup_ptr->set_next_PolyLine_label(label);
   KOZLinesGroup_ptr->set_next_PolyLine_mover_ptr(new_mover_ptr);

// Set PickHandler processing flags so that KOZLinePickHandler handles
// KOZ vertex input and NOT ROILinePickHandler:

   ROILinePickHandler_ptr->set_process_pick_flag(false);
   KOZLinePickHandler_ptr->set_process_pick_flag(true);
}

// ---------------------------------------------------------------------
void OSGButtonServer::clear_KOZs()
{
//   cout << "inside OSGButtonServer::clear_KOZs()" << endl;
   cout << "Clearing KOZs:" << endl;

   if (KOZLinesGroup_ptr != NULL) KOZLinesGroup_ptr->destroy_all_PolyLines();

   if (movers_group_ptr != NULL)
   {
      movers_group_ptr->purge_all_particular_movers(mover::KOZ);
   }
}

