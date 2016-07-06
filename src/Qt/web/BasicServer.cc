// ==========================================================================
// BASICSERVER class file
// ==========================================================================
// Last updated on 12/14/10; 1/26/11; 4/4/12
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>
#include <QtGui/QFileDialog>
#include "Qt/web/BasicServer.h"

#include "osg/osgWindow/MyViewerEventHandler.h"
#include "general/stringfuncs.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

using std::ofstream;

// ---------------------------------------------------------------------
void BasicServer::allocate_member_objects()
{
//   cout << "inside BasicServer::allocate_member_objects()" << endl;
   window_ptr=new QWidget;
}		       

void BasicServer::initialize_member_objects()
{
   movie_recording_flag=false;
   CM_3D_ptr=NULL;
   ModeController_ptr=NULL;
   Operations_ptr=NULL;
   PointCloudsGroup_ptr=NULL;
   WindowManager_ptr=NULL;
   screenshot_counter=1;
   tomcat_subdir="";
}

BasicServer::BasicServer(
   string host_IP_address,qint16 port, QObject* parent) :
   WebServer(host_IP_address,port,parent)
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
BasicServer::~BasicServer()
{
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// In July 2010, we learned the hard way that javascript will never
// transmit the full path for any selected file from a client to a
// server for security reasons.  So Zach Sun suggested that we use a
// Qt file dialog box instead to enable a user to effectively select a 
// local subdirectory containing a set of video frames.  

vector<string> BasicServer::open_input_file_dialog(
   string window_title,string starting_image_subdir,string file_types)
{
   cout << "inside BasicServer::open_input_file_dialog()" << endl;
   cout << "starting_image_subdir = "
        << starting_image_subdir << endl;

   window_ptr->move(835,0);

   QStringList inputfileNames = QFileDialog::getOpenFileNames(
      window_ptr,window_title.c_str(), starting_image_subdir.c_str(), 
      file_types.c_str());
   cout << "inputfileNames.size() = " << inputfileNames.size() << endl;

   vector<string> input_filenames;
   for (int i=0; i<inputfileNames.size(); i++)
   {
      input_filenames.push_back(
         string(inputfileNames.at(i).toLocal8Bit().constData()));
//      cout << "i = " << i 
//           << " inputfileName = " << input_filenames[i] << endl;
   }
   return input_filenames;
}

// ==========================================================================
// HTTP processing member functions
// ==========================================================================

QByteArray BasicServer::get( 
   const QUrl& url, QHttpResponseHeader& responseHeader)
{
//   cout << "inside BasicServer:get()" << endl;

   QDomDocument doc;
   QDomElement response = doc.createElement( "response" );
   string URL_path;
   return get(doc,response,url,URL_path,responseHeader);
}

// ==========================================================================
// Movie member functions
// ==========================================================================

void BasicServer::Start_Recording_Movie()
{
   cout << "inside BasicServer::Start_Recording_Movie()" << endl;
   
   movie_recording_flag=true;
   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      WindowManager_ptr);
   osgProducer::MyViewerEventHandler* MyViewerEventHandler_ptr
      =ViewerManager_ptr->get_MyViewerEventHandler_ptr();
   MyViewerEventHandler_ptr->report_progress(0);
   MyViewerEventHandler_ptr->begin_recording_movie();
}

void BasicServer::Stop_Recording_Movie()
{
   cout << "inside BasicServer::Stop_Recording_Movie()" << endl;
   
   movie_recording_flag=false;
   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      WindowManager_ptr);
   osgProducer::MyViewerEventHandler* MyViewerEventHandler_ptr
      =ViewerManager_ptr->get_MyViewerEventHandler_ptr();
   MyViewerEventHandler_ptr->end_recording_movie();

// If AnimationController mode was set to PLAY during movie creation,
// be sure to reset it to PAUSE once movie recording is done:

   reset_clock_to_starting_time();
}

void BasicServer::Generate_Movie()
{
   cout << "inside BasicServer::Generate_Movie()" << endl;
   
   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      WindowManager_ptr);
   osgProducer::MyViewerEventHandler* MyViewerEventHandler_ptr=
      ViewerManager_ptr->get_MyViewerEventHandler_ptr();
}

// ---------------------------------------------------------------------
// Member function retrieve_movie_frame_file() takes in a frame time
// ranging from 0 to total duration for some movie.  It retrieves a
// filename of the form movie_frame00XX.image_suffix (where
// image_suffix=png,jpg,rgb, etc) from ./recorded_video/movie_frame
// subdir.  It copies this file to the the /movies/movie_frames/
// subdir of the tomcat webapps directory.  And this method returns
// the image files relative path.

string BasicServer::retrieve_movie_frame_file(double frame_time)
{
   cout << "inside BasicServer::retrieve_movie_frame_file()" << endl;
   
   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      WindowManager_ptr);
   osgProducer::MyViewerEventHandler* MyViewerEventHandler_ptr
      =ViewerManager_ptr->get_MyViewerEventHandler_ptr();
   string relative_image_path= 
      MyViewerEventHandler_ptr->retrieve_movie_frame_file(frame_time);
//   cout << "relative_image_path = " << relative_image_path << endl;

// Copy image file to webapps subdirectory of tomcat so that it can
// readily be loaded into a thin client browser:

   string movie_frames_subdir=tomcat_subdir+"movies/movie_frames";
   string unix_command="cp "+relative_image_path+" "+movie_frames_subdir;
//   cout << "unix_command = " << unix_command << endl;
   sysfunc::unix_command(unix_command);

   return relative_image_path;
}

// ---------------------------------------------------------------------
// Member function get_webapps_movies_subdir_pathname() returns a
// string of the form 127.0.0.1:8080/LOST/movies/.

string BasicServer::get_webapps_movies_subdir_pathname() const
{
   cout << "inside BasicServer::get_webapps_movies_subdir_pathname()" << endl;
   
   string webapps_subdir=filefunc::getbasename(tomcat_subdir);
   string webapps_subdir_pathname="127.0.0.1:8080/"+webapps_subdir+"movies/";
//   cout << "tomcat_subdir = " << tomcat_subdir << endl;
//   cout << "webapps_subdir = " << webapps_subdir << endl;
   cout << "webapps_subdir_pathname = " << webapps_subdir_pathname << endl;
   return webapps_subdir_pathname;
}

// ==========================================================================
// JSON response member functions
// ==========================================================================

// Member function generate_JSON_response_to_parameters_request()
// returns a JSON string to Michael Yee's thin client 
 
QByteArray BasicServer::generate_JSON_response_to_parameters_request(
   string response_msg)
{
   cout << "Inside BasicServer::generate_JSON_response_to_parameters_request()"
        << endl;

   string json_string = "{ \n";
   json_string += "  \"message\": \"";
   json_string += response_msg;
   json_string += "\" \n";
   json_string += "} ";
   json_string += "\n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_clock_parameters_request()
// returns a JSON string to Michael Yee's thin client 
 
QByteArray BasicServer::generate_JSON_response_to_clock_parameters_request(
   string response_msg)
{
   cout << "Inside BasicServer::generate_JSON_response_to_clock_parameters_request()"
        << endl;

   string json_string = "{ \n";
   json_string += "  \"message\": \"";
   json_string += response_msg;
   json_string += "\",\n";
   json_string += "  \"n_frames\": \"";
   json_string += stringfunc::number_to_string(
      AnimationController_ptr->get_nframes());
   json_string += "\" \n";
   json_string += "} ";
   json_string += "\n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_movie_request() returns a
// JSON string to Michael Yee's thin client which should automatically
// load and play a FLV movie generated by our 3D thick client.
 
QByteArray BasicServer::generate_JSON_response_to_movie_request(
   string movie_path)
{
   cout << "Inside BasicServer::generate_JSON_response_to_movie_request()"
        << endl;
   cout << "movie_path = " << movie_path << endl;

   string json_string = "{ \n";

   string url=" \"url\" : ";

   url += "\"http://"+get_webapps_movies_subdir_pathname();
   url += filefunc::getbasename(movie_path) +"\" \n";
   json_string += url;
   
   json_string += "} ";
   json_string += "\n";

   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_JSON_response_to_movie_frame_request()
// returns a JSON string to Michael Yee's thin client which contains
// the URL for the requested screen capture PNG file.
 
QByteArray BasicServer::generate_JSON_response_to_movie_frame_request(
   string movie_frame_path)
{
//   cout << "Inside BasicServer::generate_JSON_response_to_movie_frame_request()"
//        << endl;
//   cout << "movie_frame_path = " << movie_frame_path << endl;

   string json_string = "{ \n";

   string url=" \"url\" : ";
//   url += "\"http://127.0.0.1:8080/pathplanning/movies/movie_frames/";
   url += "\"http://"+get_webapps_movies_subdir_pathname()+"movie_frames/";;

   url += filefunc::getbasename(movie_frame_path) +"\" \n";
   json_string += url;
   
   json_string += "} ";
   json_string += "\n";

//   cout << " Final json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ---------------------------------------------------------------------
// Member function generate_error_JSON_response() returns a JSON
// string with key='message' and value=error_message contents.

QByteArray BasicServer::generate_error_JSON_response(string error_message)
{
//   cout << "inside BasicServer::generate_error_JSON_response()" << endl;
   string json_string = "{ \"message\": \"";
   json_string += error_message;
   json_string += "\" }";
//   cout << "Error json_string = " << json_string << endl;
   return QByteArray(json_string.c_str());
}

// ==========================================================================
// AnimationController member functions
// ==========================================================================

// Member function display_next_frame() 

void BasicServer::display_next_frame(int frame_step)
{
//   cout << "inside BasicServer::display_next_frame()" << endl;
//   cout << "frame_step = " << frame_step << endl;
//   cout << "AnimationController_ptr = " << AnimationController_ptr << endl;

   if (frame_step==1)
   {
      AnimationController_ptr->setState( 
         AnimationController::INCREMENT_FRAME );
   }
   else if (frame_step==-1)
   {
      AnimationController_ptr->setState( 
         AnimationController::DECREMENT_FRAME );
   }
}

// ---------------------------------------------------------------------
// Member function play_movie() sets the AnimationController's clock
// running.

void BasicServer::play_movie()
{
//   cout << "inside BasicServer::play_movie()" << endl;
//   cout << "Playing movie" << endl;
   AnimationController_ptr->setState(AnimationController::PLAY);
}

void BasicServer::pause_movie()
{
//   cout << "inside BasicServer::pause_movie()" << endl;
//   cout << "Pausing movie" << endl;
   AnimationController_ptr->setState(AnimationController::PAUSE);
}

// ---------------------------------------------------------------------
// Member function reset_clock_to_starting_time()

void BasicServer::reset_clock_to_starting_time()
{
//   cout << "inside BasicServer::reset_clock_to_starting_time()" << endl;

   AnimationController_ptr->set_curr_framenumber(
      AnimationController_ptr->get_first_framenumber());
   pause_movie();
//   outputfunc::enter_continue_char();
}

// ==========================================================================
// Screen capture member functions
// ==========================================================================

// Member function capture_viewer_screen()

QByteArray BasicServer::capture_viewer_screen()
{
   cout << "inside BasicServer::capture_viewer_screen()" << endl;

   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      WindowManager_ptr);

   osgProducer::MyViewerEventHandler* MyViewerEventHandler_ptr
      =ViewerManager_ptr->get_MyViewerEventHandler_ptr();

   ofstream outstream;
   string test_filename="/tmp/junk.dat";
   filefunc::openfile(test_filename,outstream);
   outstream << "sysfunc::get_loginname() = " << endl;
   outstream << sysfunc::get_loginname() << endl;

   string subdir="/home/"+sysfunc::get_loginname();      
   subdir += "/Desktop/movies_and_screen_shots/";
//   cout << "subdir = " << subdir << endl;
   string output_filename="viewer_screenshot_"
      +stringfunc::number_to_string(screenshot_counter++)+".png";
   string full_filename=subdir+output_filename;
//   cout << "full_filename = " << full_filename << endl;

   outstream << "full_filename = " << full_filename << endl;
   filefunc::closefile(test_filename,outstream);

   MyViewerEventHandler_ptr->setWriteImageFileName(full_filename);
   MyViewerEventHandler_ptr->setWriteImageOnNextFrame(true);           

   string response_msg="Saved viewer window screenshot into "+
      output_filename+" in movies_and_screen_shots folder on Desktop";
   return generate_JSON_response_to_parameters_request(response_msg);
}

// ---------------------------------------------------------------------
// Member function generate_AVI_movie()

QByteArray BasicServer::generate_AVI_movie()
{
//   cout << "inside BasicServer::generate_AVI_movie()" << endl;

   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(
      WindowManager_ptr);
   osgProducer::MyViewerEventHandler* MyViewerEventHandler_ptr
      =ViewerManager_ptr->get_MyViewerEventHandler_ptr();
   while (MyViewerEventHandler_ptr->get_recording_flag())
   {
      WindowManager_ptr->process();
   }
   return generate_JSON_response_to_movie_request(
      MyViewerEventHandler_ptr->get_flv_movie_path());
}
