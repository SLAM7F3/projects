// ==========================================================================
// RTPSBUTTONSERVER class file
// ==========================================================================
// Last updated on 5/3/09; 5/4/09; 5/5/09; 5/10/10
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
#include "Qt/web/RTPSButtonServer.h"
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
void RTPSButtonServer::allocate_member_objects()
{
}		       

void RTPSButtonServer::initialize_member_objects()
{
   ModeController_ptr=NULL;
   WindowManager_ptr=NULL;
   AnimationController_ptr=NULL;
   CylindersGroup_ptr=NULL;
   TrackLinesGroup_ptr=NULL;
   SIALinesGroup_ptr=NULL;
   SIALinePickHandler_ptr=NULL;
   EarthRegionsGroup_ptr=NULL;
   EarthRegion_ptr=NULL;
   BluegrassClient_ptr=NULL;
   SKSClient_ptr=NULL;
   pointfinder_ptr=NULL;
   MoviesGroup_ptr=NULL;
   OBSFRUSTAGROUP_ptr=NULL;
   NFOV_aimpoint_SignPostsGroup_ptr=NULL;
   aircraft_MODELSGROUP_ptr=NULL;
   Predator_MODELSGROUP_ptr=NULL;
   Decorations_ptr=NULL;
   Operations_ptr=NULL;
   movers_group_ptr=NULL;
   ROI_PolyhedraGroup_ptr=NULL;

   video_state=0;
   cyl_height_state=0;
   n_video_states=n_cyl_height_states=2;
   n_cumulative_tracks=0;
   ROI_PolyhedraGroup_OSGsubPAT_number=0;
}

RTPSButtonServer::RTPSButtonServer(
   string host_IP_address,qint16 port, QObject* parent) :
   WebServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
RTPSButtonServer::~RTPSButtonServer()
{
   EarthRegion_ptr->purge_ROI_tracks_group_ptrs();
}

// ---------------------------------------------------------------------
void RTPSButtonServer::set_EarthRegionsGroup_ptr(EarthRegionsGroup* ERG_ptr)
{
   EarthRegionsGroup_ptr=ERG_ptr;

   EarthRegion_ptr=EarthRegionsGroup_ptr->get_ID_labeled_EarthRegion_ptr(0);
   movers_group_ptr=EarthRegion_ptr->get_movers_group_ptr();
   SIALinesGroup_ptr=EarthRegion_ptr->get_ROILinesGroup_ptr();
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void RTPSButtonServer::set_Decorations_ptr(Decorations* D_ptr)
{
//   cout << "inside RTPSButtonServer::set_Decorations_ptr()" << endl;
   
   Decorations_ptr=D_ptr;

   CylindersGroup_ptr=Decorations_ptr->get_CylindersGroup_ptr();
   TrackLinesGroup_ptr=Decorations_ptr->get_PolyLinesGroup_ptr();
   aircraft_MODELSGROUP_ptr=Decorations_ptr->get_MODELSGROUP_ptr();
}

void RTPSButtonServer::set_Operations_ptr(Operations* Op_ptr)
{
   Operations_ptr=Op_ptr;

   ModeController_ptr=Op_ptr->get_ModeController_ptr();
   AnimationController_ptr=Op_ptr->get_AnimationController_ptr();
}

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray RTPSButtonServer::get( 
   const QUrl& url, QHttpResponseHeader& responseHeader)
{
//   cout << "inside RTPSButtonServer:get()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );
   string URL_path;
   return get(doc,response,url,URL_path,responseHeader);
}

// ---------------------------------------------------------------------
QByteArray RTPSButtonServer::get(
   QDomDocument& doc,QDomElement& response,const QUrl& url,
   string& URL_path, QHttpResponseHeader& responseHeader)
{
//   cout << "inside 2nd RTPSButtonServer:get() method" << endl;

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

// ROI button event handling:

   else if (URL_path=="/ENTER_ROI/")
   {
      enter_ROI();
   }
   else if (URL_path=="/MOVE_ROI/")
   {
      move_ROI();
   }
   else if (URL_path=="/TOGGLE_ROIS/")
   {
      toggle_ROIs();
   }
   else if (URL_path=="/CLEAR_ROIS/")
   {
      clear_ROIs();
   }

// Sensor button handling:

   else if (URL_path=="/TOGGLE_WFOV/")
   {
      toggle_WFOV_sensor();
   }
   else if (URL_path=="/ENTER_NFOV_AIMPT/")
   {
      enter_NFOV_aimpoint();
   }
   else if (URL_path=="/CLEAR_AIMPTS/")
   {
      clear_NFOV_aimpoints();
   }

// SIA button handling:

   else if (URL_path=="/ENTER_SIA/")
   {
      enter_SIA();
   }
   else if (URL_path=="/CLEAR_SIAS/")
   {
      clear_SIAs();
   }

// Tracks button handling:

   else if (URL_path=="/TALL_CYL/" && CylindersGroup_ptr != NULL
            && cyl_height_state==TALL_CYL)
   {
      increase_Cylinders_size();
   }
   else if (URL_path=="/SHORT_CYL/" && CylindersGroup_ptr != NULL
            && cyl_height_state==SHORT_CYL)
   {
      decrease_Cylinders_size();
   }




   else if (URL_path=="/TOGGLE_ROADS/")
   {
      toggle_roads();
   }
   else if (URL_path=="/TOGGLE_VIDEO/")
   {
      toggle_video();
   }

   else if (URL_path=="/CLEAR_VEHICLE_TRACKS/")
   {
      clear_vehicle_tracks();
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
// ROI button event handling member functions
// ==========================================================================

void RTPSButtonServer::enter_ROI()
{
//   cout << "inside RTPSButtonServer::enter_ROI()" << endl;
   cout << "Entering ROI" << endl;
   ModeController_ptr->setState( ModeController::INSERT_POLYHEDRON );
}

// ---------------------------------------------------------------------
void RTPSButtonServer::move_ROI()
{
//   cout << "inside RTPSButtonServer::move_ROI()" << endl;
   cout << "Moving ROI" << endl;
   ModeController_ptr->setState( ModeController::MANIPULATE_POLYHEDRON );
}

// ---------------------------------------------------------------------
void RTPSButtonServer::toggle_ROIs()
{
//   cout << "inside RTPSButtonServer::toggle_ROIs()" << endl;

   if (ROI_PolyhedraGroup_ptr == NULL) return;

   cout << "Toggling ROIs" << endl;

   ROI_PolyhedraGroup_ptr->toggle_OSGsubPAT_nodemask(
      ROI_PolyhedraGroup_OSGsubPAT_number);
}

// ---------------------------------------------------------------------
void RTPSButtonServer::clear_ROIs()
{
//   cout << "inside RTPSButtonServer::clear_ROIs()" << endl;

   ROI_PolyhedraGroup_ptr->destroy_all_Polyhedra();
}

// ==========================================================================
// Sensor button member functions
// ==========================================================================

void RTPSButtonServer::toggle_WFOV_sensor()
{
//   cout << "inside RTPSButtonServer::toggle_WFOV_sensor()" << endl;

   if (aircraft_MODELSGROUP_ptr == NULL) return;

   cout << "Toggling sensor view" << endl;
   for (int i=0; i<aircraft_MODELSGROUP_ptr->get_n_OSGsubPATs(); i++)
   {
      aircraft_MODELSGROUP_ptr->toggle_model_mask(i);
   }
}

void RTPSButtonServer::enter_NFOV_aimpoint()
{
//   cout << "inside RTPSButtonServer::enter_NFOV_aimpoint()" << endl;
   int n_aimpoint=NFOV_aimpoint_SignPostsGroup_ptr->get_n_Graphicals();
   string banner="Enter NFOV aimpoint "+stringfunc::number_to_string(
      n_aimpoint)+":";
   outputfunc::write_banner(banner);
   ModeController_ptr->setState( ModeController::INSERT_ANNOTATION );
}

void RTPSButtonServer::clear_NFOV_aimpoints()
{
//   cout << "inside RTPSButtonServer::clear_NFOV_aimpoints()" << endl;
   string banner="Clearing NFOV aimpoints";
   outputfunc::write_banner(banner);
   NFOV_aimpoint_SignPostsGroup_ptr->destroy_all_SignPosts();
}

// ==========================================================================
// SIA button member functions
// ==========================================================================

void RTPSButtonServer::enter_SIA()
{
//   cout << "inside RTPSButtonServer::enter_SIA()" << endl;
   cout << "Entering SIA" << endl;
   ModeController_ptr->setState( ModeController::INSERT_LINE );

   tracks_group* spatially_fixed_tracks_group_ptr=
      EarthRegion_ptr->get_spatially_fixed_tracks_group_ptr();
   mover* new_mover_ptr=movers_group_ptr->generate_new_ROI(
      spatially_fixed_tracks_group_ptr);

//   new_mover_ptr->set_annotation_label("All tracks");

   int curr_SIA_ID=new_mover_ptr->get_ID();
//      cout << "new_mover_ptr->get_ID() = " << curr_ROI_ID << endl;
   string label="SIA "+stringfunc::number_to_string(curr_SIA_ID);
//   cout << "SIA label = " << label << endl;
   
   SIALinesGroup_ptr->set_next_PolyLine_label(label);
   SIALinesGroup_ptr->set_next_PolyLine_mover_ptr(new_mover_ptr);

   SIALinePickHandler_ptr->set_process_pick_flag(true);

   movers_group_ptr->add_mover_to_outgoing_queue(new_mover_ptr);
}

// ---------------------------------------------------------------------
void RTPSButtonServer::clear_SIAs()
{
//   cout << "inside RTPSButtonServer::clear_SIAs()" << endl;
   cout << "Clearing SIAs" << endl;

   EarthRegion_ptr->clear_all_ROIs();
}

// ==========================================================================
// Track buttons member functions
// ==========================================================================

void RTPSButtonServer::increase_Cylinders_size()
{
   cout << "Increasing cylinders size" << endl;
   CylindersGroup_ptr->set_tall_RTPS_size_flag(true);

   cyl_height_state++;
   cyl_height_state=modulo(cyl_height_state,n_cyl_height_states);
}

// ---------------------------------------------------------------------
void RTPSButtonServer::decrease_Cylinders_size()
{
   cout << "Decreasing cylinders size" << endl;
   CylindersGroup_ptr->set_tall_RTPS_size_flag(false);

   cyl_height_state++;
   cyl_height_state=modulo(cyl_height_state,n_cyl_height_states);
}




// ---------------------------------------------------------------------
void RTPSButtonServer::toggle_roads()
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

void RTPSButtonServer::toggle_video()
{
//   cout << "inside RTPSButtonServer::toggle_video()" << endl;
   if (MoviesGroup_ptr != NULL) toggle_video(MoviesGroup_ptr);
}

/*
void RTPSButtonServer::toggle_video()
{
   cout << "inside RTPSButtonServer::toggle_video()" << endl;

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

void RTPSButtonServer::toggle_video(MoviesGroup* local_MoviesGroup_ptr)
{
//   cout << "inside RTPSButtonServer::toggle_video(local_MoviesGroup_ptr)" 
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


/*
// ---------------------------------------------------------------------
void RTPSButtonServer::enter_ROI()
{
//   cout << "inside RTPSButtonServer::enter_ROI()" << endl;
   cout << "Entering ROI" << endl;
   ModeController_ptr->setState( ModeController::INSERT_LINE );

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

   movers_group_ptr->add_mover_to_outgoing_queue(new_mover_ptr);
}

// ---------------------------------------------------------------------
void RTPSButtonServer::monitor_ROI()
{
//   cout << "inside RTPSButtonServer::monitor_ROI()" << endl;
   cout << "Monitoring ROI" << endl;

   ROI_state++;
   ROI_state=modulo(ROI_state,n_ROI_states);

//   cout << "ROI_state = " << ROI_state << endl;
//   cout << "n_ROI_states = " << n_ROI_states << endl;

}
*/

// ---------------------------------------------------------------------
void RTPSButtonServer::clear_vehicle_tracks()
{
   cout << "Clearing vehicle tracks" << endl;
   TrackLinesGroup_ptr->destroy_all_PolyLines();
   EarthRegion_ptr->purge_ROI_tracks_group_ptrs();
   n_cumulative_tracks=0;

   if (movers_group_ptr != NULL)
   {
      movers_group_ptr->purge_all_particular_movers(mover::VEHICLE);
   }
}

// ---------------------------------------------------------------------
void RTPSButtonServer::snap_screen()
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

void RTPSButtonServer::show_speeds()
{
//   cout << "inside RTPSButtonServer::show_speeds()" << endl;
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
void RTPSButtonServer::hide_speeds()
{
   cout << "Globally color vehicle tracks" << endl;
   TrackLinesGroup_ptr->set_multicolor_flags(false);
   TrackLinesGroup_ptr->reset_colors();
   bool show_arrowheads_flag=true;
   set_vehicle_tracks_arrowheads_nodemask(show_arrowheads_flag);
} 

// ---------------------------------------------------------------------
void RTPSButtonServer::set_curr_snapshot_subdir(std::string subdir)
{
//   cout << "inside RTPSButtonServer::set_curr_snapshot_subdir" << endl;
//   cout << "subdir = " << subdir << endl;
   curr_snapshot_subdir=subdir;
   filefunc::add_trailing_dir_slash(curr_snapshot_subdir);
   filefunc::dircreate(curr_snapshot_subdir);
}

// ---------------------------------------------------------------------
void RTPSButtonServer::set_vehicle_tracks_arrowheads_nodemask(
   bool show_arrowheads_flag)
{
//   cout << "inside RTPSButtonServer::set_vehicle_tracks_arrowheads_nodemask()"
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

void RTPSButtonServer::enter_UAV_path()
{
   cout << "Entering UAV path" << endl;
   ModeController_ptr->setState( ModeController::INSERT_POLYLINE );
}

// ---------------------------------------------------------------------
void RTPSButtonServer::select_UAV_path()
{
   cout << "Selecting UAV path" << endl;
   ModeController_ptr->setState( ModeController::MANIPULATE_POLYLINE );
}

// ---------------------------------------------------------------------
void RTPSButtonServer::alter_UAV_path()
{
   cout << "Altering UAV path" << endl;

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
void RTPSButtonServer::clear_UAV_paths()
{
//   cout << "Inside RTPSButtonServer::clear_UAV_paths()" << endl;
   cout << "Clearing UAV paths" << endl;

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
void RTPSButtonServer::compute_UAV_path()
{
   cout << "Computing UAV path" << endl;

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


