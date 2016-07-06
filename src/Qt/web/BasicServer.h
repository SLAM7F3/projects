// Note added on 4/5/12: It'd be useful to segregate out all OSG
// dependence into a new OSGServer class.  Then MacheteServer should
// not need to depend upon OSG in any way...

// Note added on 7/10/12: Dave Ceddia taught us that we really should have 
// a more primitive server class which has NO dependence upon any QT
// GUI stuff.  In particular, it should NOT include any Qt headers
// like QtGui/QFileDialog.  Moreover, it should NOT contain member
// variable QWidget* window_ptr.  Then corresponding .pro files do not
// need a "QT += gui" line at their end.  

// ========================================================================
// Header file for BASICSERVER class which enables thin-client
// communication with "photosynth" thick clients via HTTP get and post
// commands
// ========================================================================
// Last updated on 1/26/11; 5/19/11; 4/4/12
// ========================================================================

#ifndef __BASICSERVER_H__
#define __BASICSERVER_H__

#include <set>
#include <vector>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>

#include "osg/osgOperations/Operations.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "Qt/web/WebServer.h"
#include "osg/osgWindow/WindowManager.h"

class BasicServer : public WebServer
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   BasicServer(
      std::string host_IP_address, qint16 port,QObject* parent=NULL);
   ~BasicServer();

// Set & get member functions:

   bool get_movie_recording_flag() const;
   void set_tomcat_subdir(std::string subdir);
   void set_Operations_ptr(Operations* O_ptr);
   void set_AnimationController_ptr(AnimationController* AC_ptr);
   void set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr);
   void set_CM_3D_ptr(osgGA::Terrain_Manipulator* TM_ptr);

   std::vector<std::string> get_keys();
   std::vector<std::string> get_values();

   std::vector<std::string> open_input_file_dialog(
      std::string window_title,std::string starting_image_subdir,
      std::string file_types);

   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QByteArray get(
      const QUrl& url, QHttpResponseHeader& responseHeader);
   virtual QByteArray get(
      QDomDocument& doc,QDomElement& html,const QUrl& url,
      std::string& URL_path,QHttpResponseHeader& responseHeader)=0;

   bool movie_recording_flag;
   int screenshot_counter;
   std::string tomcat_subdir;
   AnimationController* AnimationController_ptr;
   osgGA::Terrain_Manipulator* CM_3D_ptr;
   ModeController* ModeController_ptr;
   Operations* Operations_ptr;
   PointCloudsGroup* PointCloudsGroup_ptr;
   WindowManager* WindowManager_ptr;
   QWidget* window_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

// Movie member functions:

   void Start_Recording_Movie();
   void Stop_Recording_Movie();
   void Generate_Movie();
   std::string retrieve_movie_frame_file(double frame_time);
   std::string get_webapps_movies_subdir_pathname() const;

// JSON response member functions:

   QByteArray generate_JSON_response_to_parameters_request(
      std::string response_msg);
   QByteArray generate_JSON_response_to_clock_parameters_request(
      std::string response_msg);
   QByteArray generate_JSON_response_to_movie_request(std::string movie_path);
   QByteArray generate_JSON_response_to_movie_frame_request(
      std::string movie_frame_PNG_path);
   QByteArray generate_error_JSON_response(std::string error_message);

// AnimationController member functions:

   void display_next_frame(int frame_step);
   void play_movie();
   void pause_movie();
   void reset_clock_to_starting_time();

// Screen capture member functions:

   QByteArray capture_viewer_screen();
   QByteArray generate_AVI_movie();

  private:

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline bool BasicServer::get_movie_recording_flag() const
{
   return movie_recording_flag;
}

inline void BasicServer::set_tomcat_subdir(std::string subdir)
{
   tomcat_subdir=subdir;
}

inline void BasicServer::set_Operations_ptr(Operations* O_ptr)
{
   Operations_ptr=O_ptr;
   AnimationController_ptr=Operations_ptr->get_AnimationController_ptr();
   ModeController_ptr=Operations_ptr->get_ModeController_ptr();
   WindowManager_ptr=Operations_ptr->get_WindowManager_ptr();

//   std::cout << "inside BasicServer::set_Operations_ptr(), this = " << this
//             << std::endl;
//   std::cout << "Operations_ptr = " << Operations_ptr << std::endl;
//   std::cout << "AnimationController_ptr = " << AnimationController_ptr
//             << std::endl;
}

// Ordinarily,
// this->AnimationController_ptr=Operations_ptr->get_AnimationController_ptr.
// But for cases where two or more clocks may exists (e.g. one for
// aircraft and another for ground target), we allow for BasicServer's
// AnimationController's pointer to be reset:

inline void BasicServer::set_AnimationController_ptr(
   AnimationController* AC_ptr)
{
   AnimationController_ptr=AC_ptr;
}

inline void BasicServer::set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr)
{
   PointCloudsGroup_ptr=PCG_ptr;
}

inline void BasicServer::set_CM_3D_ptr(osgGA::Terrain_Manipulator* TM_ptr)
{
   CM_3D_ptr=TM_ptr;
}


#endif // __BASICSERVER_H__
