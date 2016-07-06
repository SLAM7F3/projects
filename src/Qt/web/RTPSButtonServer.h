// ========================================================================
// RTPSBUTTONSERVER header file
// ========================================================================
// Last updated on 5/1/09; 5/2/09; 5/3/09; 5/4/09; 12/21/09
// ========================================================================

#ifndef __RTPSBUTTONSERVER_H__
#define __RTPSBUTTONSERVER_H__

#include <set>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "osg/osgGraphicals/AnimationController.h"
#include "Qt/web/BluegrassClient.h"
#include "osg/osgGeometry/CylindersGroup.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgOperations/Operations.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgRegions/RegionPolyLinesGroup.h"
#include "osg/osgRegions/RegionPolyLinePickHandler.h"
#include "osg/osgRTPS/ROI_PolyhedraGroup.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "Qt/web/SKSClient.h"
#include "Qt/web/WebServer.h"
#include "osg/osgWindow/WindowManager.h"

class EarthRegionsGroup;
class MODELSGROUP;

class RTPSButtonServer : public WebServer
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   RTPSButtonServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~RTPSButtonServer();

// Set & get member functions:

   void set_NFOV_aimpoint_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr);
   void set_WindowManager_ptr(WindowManager* WM_ptr);
   void set_EarthRegionsGroup_ptr(EarthRegionsGroup* ERG_ptr);
   void set_SIALinePickHandler_ptr(RegionPolyLinePickHandler* RPH_ptr);
   void set_BluegrassClient_ptr(BluegrassClient* BGC_ptr);
   void set_SKSClient_ptr(SKSClient* SC_ptr);
   void set_pointfinder_ptr(PointFinder* pf_ptr);
   void set_MoviesGroup_ptr(MoviesGroup* MG_ptr);
   void set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr);
   void set_Decorations_ptr(Decorations* D_ptr);
   void set_Operations_ptr(Operations* Op_ptr);
   void set_Predator_MODELSGROUP_ptr(MODELSGROUP* MG_ptr);
   void set_ROI_PolyhedraGroup_ptr(ROI_PolyhedraGroup* PHG_ptr);
   void set_ROI_PolyhedraGroup_OSGsubPAT_number_to_toggle(
      int OSGsubPAT_number);
   void set_n_ROI_states(int nstates);

   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QByteArray get(
      const QUrl& url, QHttpResponseHeader& responseHeader);
   virtual QByteArray get(
      QDomDocument& doc,QDomElement& html,const QUrl& url,
      std::string& URL_path,QHttpResponseHeader& responseHeader);

  private:

   enum video_state 
   {
      RUN_VIDEO=0, MOVE_VIDEO=1
   };
   
   enum ROI_state
   {
      ENTER_ROI=0, MONITOR_ROI=1
   };
   
   enum CYL_height_state
   {
      TALL_CYL=0, SHORT_CYL=1
   };
   
   int video_state,ROI_state,n_video_states,n_ROI_states;
   int cyl_height_state,n_cyl_height_states;
   int n_cumulative_tracks;
   int ROI_PolyhedraGroup_OSGsubPAT_number;

   std::string curr_snapshot_subdir;
   ModeController* ModeController_ptr;
   WindowManager* WindowManager_ptr;
   AnimationController* AnimationController_ptr;
   CylindersGroup* CylindersGroup_ptr;
   PolyLinesGroup *TrackLinesGroup_ptr;
   EarthRegionsGroup* EarthRegionsGroup_ptr;
   EarthRegion* EarthRegion_ptr;
   BluegrassClient* BluegrassClient_ptr;
   SKSClient* SKSClient_ptr;
   PointFinder* pointfinder_ptr;
   MODELSGROUP *aircraft_MODELSGROUP_ptr,*Predator_MODELSGROUP_ptr;
   MoviesGroup* MoviesGroup_ptr;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;
   RegionPolyLinesGroup *SIALinesGroup_ptr;
   RegionPolyLinePickHandler *SIALinePickHandler_ptr;
   SignPostsGroup* NFOV_aimpoint_SignPostsGroup_ptr;
   Decorations* Decorations_ptr;
   Operations* Operations_ptr;
   movers_group* movers_group_ptr;
   ROI_PolyhedraGroup* ROI_PolyhedraGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

// ROI button event handling member functions:

   void enter_ROI();
   void move_ROI();
   void toggle_ROIs();
   void clear_ROIs();

// Sensor button member functions:

   void toggle_WFOV_sensor();
   void enter_NFOV_aimpoint();
   void clear_NFOV_aimpoints();

// SIA button event handling member functions:

   void enter_SIA();
   void clear_SIAs();

// Tracks button handling member functions:

   void increase_Cylinders_size();
   void decrease_Cylinders_size();



   void toggle_roads();
   void toggle_video();
   void toggle_video(MoviesGroup* local_MoviesGroup_ptr);



   void clear_vehicle_tracks();

   void snap_screen();
   void show_speeds();
   void hide_speeds();
   void nominate_ROIs();

   void set_curr_snapshot_subdir(std::string subdir);
   void set_vehicle_tracks_arrowheads_nodemask(bool show_arrowheads_flag);

// UAV button event handling member functions

   void enter_UAV_path();
   void select_UAV_path();
   void alter_UAV_path();
   void clear_UAV_paths();
   void compute_UAV_path();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void RTPSButtonServer::set_NFOV_aimpoint_SignPostsGroup_ptr(
   SignPostsGroup* SPG_ptr)
{
   NFOV_aimpoint_SignPostsGroup_ptr=SPG_ptr;
}

inline void RTPSButtonServer::set_WindowManager_ptr(WindowManager* VM_ptr)
{
   WindowManager_ptr=VM_ptr;
}

inline void RTPSButtonServer::set_SIALinePickHandler_ptr(
   RegionPolyLinePickHandler* RPH_ptr)
{
   SIALinePickHandler_ptr=RPH_ptr;
}

inline void RTPSButtonServer::set_BluegrassClient_ptr(BluegrassClient* BGC_ptr)
{
   BluegrassClient_ptr=BGC_ptr;
}
 
inline void RTPSButtonServer::set_SKSClient_ptr(SKSClient* SC_ptr)
{
   SKSClient_ptr=SC_ptr;
}

inline void RTPSButtonServer::set_pointfinder_ptr(PointFinder* pf_ptr)
{
   pointfinder_ptr=pf_ptr;
}

inline void RTPSButtonServer::set_MoviesGroup_ptr(MoviesGroup* MG_ptr)
{
   MoviesGroup_ptr=MG_ptr;
}

inline void RTPSButtonServer::set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr)
{
   OBSFRUSTAGROUP_ptr=OFG_ptr;
}

inline void RTPSButtonServer::set_Predator_MODELSGROUP_ptr(MODELSGROUP* MG_ptr)
{
   Predator_MODELSGROUP_ptr=MG_ptr;
}

inline void RTPSButtonServer::set_ROI_PolyhedraGroup_ptr(
   ROI_PolyhedraGroup* PHG_ptr)
{
   ROI_PolyhedraGroup_ptr=PHG_ptr;
}

inline void RTPSButtonServer::
set_ROI_PolyhedraGroup_OSGsubPAT_number_to_toggle(int OSGsubPAT_number)
{
   ROI_PolyhedraGroup_OSGsubPAT_number=OSGsubPAT_number;
}

inline void RTPSButtonServer::set_n_ROI_states(int nstates)
{
   n_ROI_states=nstates;
}

#endif // __RTPSBUTTONSERVER_H__
