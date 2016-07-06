// ========================================================================
// OSGBUTTONSERVER header file
// ========================================================================
// Last updated on 9/16/08; 10/31/08; 12/5/08; 12/14/08
// ========================================================================

#ifndef __OSGBUTTONSERVER_H__
#define __OSGBUTTONSERVER_H__

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
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "Qt/web/SKSClient.h"
#include "Qt/web/WebServer.h"
#include "osg/osgWindow/WindowManager.h"

class EarthRegionsGroup;
class MODELSGROUP;

class OSGButtonServer : public WebServer
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   OSGButtonServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~OSGButtonServer();

// Set & get member functions:

   void set_WindowManager_ptr(WindowManager* WM_ptr);
   void set_EarthRegionsGroup_ptr(EarthRegionsGroup* ERG_ptr);
   void set_ROILinePickHandler_ptr(RegionPolyLinePickHandler* RPH_ptr);
   void set_KOZLinePickHandler_ptr(RegionPolyLinePickHandler* KPH_ptr);
   void set_BluegrassClient_ptr(BluegrassClient* BGC_ptr);
   void set_SKSClient_ptr(SKSClient* SC_ptr);
   void set_pointfinder_ptr(PointFinder* pf_ptr);
   void set_MoviesGroup_ptr(MoviesGroup* MG_ptr);
   void set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr);
   void set_Decorations_ptr(Decorations* D_ptr);
   void set_Operations_ptr(Operations* Op_ptr);
   void set_Predator_MODELSGROUP_ptr(MODELSGROUP* MG_ptr);
   void set_ARs_PolyhedraGroup_ptr(PolyhedraGroup* PHG_ptr);
   void set_ARs_PolyhedraGroup_OSGsubPAT_number_to_toggle(
      int OSGsubPAT_number);
   void set_KOZs_CylindersGroup_ptr(CylindersGroup* CG_ptr);
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
   
   enum LOI_state
   {
      BIG_LOI=0, SMALL_LOI=1
   };
   
   int video_state,ROI_state,n_video_states,n_ROI_states;
   int LOI_state,n_LOI_states;
   int n_cumulative_tracks;
   int ARs_PolyhedraGroup_OSGsubPAT_number;

   std::string curr_snapshot_subdir;
   ModeController* ModeController_ptr;
   WindowManager* WindowManager_ptr;
   AnimationController* AnimationController_ptr;
   CylindersGroup* CylindersGroup_ptr;
   CylindersGroup* KOZs_CylindersGroup_ptr;
   PolyLinesGroup *TrackLinesGroup_ptr;
   RegionPolyLinesGroup *ROILinesGroup_ptr,*KOZLinesGroup_ptr;
   RegionPolyLinePickHandler *ROILinePickHandler_ptr,*KOZLinePickHandler_ptr;
   EarthRegionsGroup* EarthRegionsGroup_ptr;
   EarthRegion* EarthRegion_ptr;
   BluegrassClient* BluegrassClient_ptr;
   SKSClient* SKSClient_ptr;
   PointFinder* pointfinder_ptr;
   MODELSGROUP *MODELSGROUP_ptr,*Predator_MODELSGROUP_ptr;
   MoviesGroup* MoviesGroup_ptr;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;
   SignPostsGroup* SignPostsGroup_ptr;
   Decorations* Decorations_ptr;
   Operations* Operations_ptr;
   movers_group* movers_group_ptr;
   PolyhedraGroup* ARs_PolyhedraGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

// ROI & vehicle track button event handling member functions:

   void toggle_activity_regions();
   void toggle_sensor();
   void toggle_roads();
   void toggle_video();
   void toggle_video(MoviesGroup* local_MoviesGroup_ptr);
   void increase_SignPosts_size();
   void decrease_SignPosts_size();
   void enter_ROI();
   void monitor_ROI();
   void clear_vehicle_tracks();
   void clear_ROIs();

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

// KOZ button event handling member functions:

   void enter_KOZ();
   void clear_KOZs();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void OSGButtonServer::set_WindowManager_ptr(WindowManager* VM_ptr)
{
   WindowManager_ptr=VM_ptr;
}

inline void OSGButtonServer::set_ROILinePickHandler_ptr(
   RegionPolyLinePickHandler* RPH_ptr)
{
   ROILinePickHandler_ptr=RPH_ptr;
}

inline void OSGButtonServer::set_KOZLinePickHandler_ptr(
   RegionPolyLinePickHandler* KPH_ptr)
{
   KOZLinePickHandler_ptr=KPH_ptr;
}

inline void OSGButtonServer::set_BluegrassClient_ptr(BluegrassClient* BGC_ptr)
{
   BluegrassClient_ptr=BGC_ptr;
}
 
inline void OSGButtonServer::set_SKSClient_ptr(SKSClient* SC_ptr)
{
   SKSClient_ptr=SC_ptr;
}

inline void OSGButtonServer::set_pointfinder_ptr(PointFinder* pf_ptr)
{
   pointfinder_ptr=pf_ptr;
}

inline void OSGButtonServer::set_MoviesGroup_ptr(MoviesGroup* MG_ptr)
{
   MoviesGroup_ptr=MG_ptr;
}

inline void OSGButtonServer::set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr)
{
   OBSFRUSTAGROUP_ptr=OFG_ptr;
}

inline void OSGButtonServer::set_Predator_MODELSGROUP_ptr(MODELSGROUP* MG_ptr)
{
   Predator_MODELSGROUP_ptr=MG_ptr;
}

inline void OSGButtonServer::set_ARs_PolyhedraGroup_ptr(
   PolyhedraGroup* PHG_ptr)
{
   ARs_PolyhedraGroup_ptr=PHG_ptr;
}

inline void OSGButtonServer::set_KOZs_CylindersGroup_ptr(
   CylindersGroup* CG_ptr)
{
   KOZs_CylindersGroup_ptr=CG_ptr;
}

inline void OSGButtonServer::
set_ARs_PolyhedraGroup_OSGsubPAT_number_to_toggle(int OSGsubPAT_number)
{
   ARs_PolyhedraGroup_OSGsubPAT_number=OSGsubPAT_number;
}

inline void OSGButtonServer::set_n_ROI_states(int nstates)
{
   n_ROI_states=nstates;
}

#endif // __OSGBUTTONSERVER_H__
