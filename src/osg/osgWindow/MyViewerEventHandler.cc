// =======================================================================
// MyViewerEventHandler class
// =======================================================================
// Last updated on 10/19/11; 10/24/11; 6/27/12; 4/6/14; 4/14/16
// =======================================================================

#include <fstream>
#include <osgGA/AnimationPathManipulator>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include "osg/Custom2DManipulator.h"
#include "osg/Custom3DManipulator.h"
#include "ffmpeg/FFMPEGVideo.h"
#include "general/filefuncs.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

using namespace osgProducer;

using std::cin;
using std::cout;
using std::endl;
using std::fill;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

// =======================================================================
void MyViewerEventHandler::allocate_member_objects()
{
}		       

void MyViewerEventHandler::initialize_member_objects()
{
   fps=3.0;
   auto_generate_movies_flag=false;
   _recording=false;
   _firstTimeTogglingFullScreen=true;

//   finished_movie_subdir="~/Desktop/movies_and_screen_shots/";
//   filefunc::dircreate(finished_movie_subdir);

   CustomManipulator_ptr=NULL;
   SnapImageDrawCallback_ptr=NULL;
   Viewer_Messenger_ptr=NULL;
}

// -----------------------------------------------------------------------
MyViewerEventHandler::MyViewerEventHandler(WindowManager* WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   WindowManager_ptr=WM_ptr;

   ViewerManager* ViewerManager_ptr=dynamic_cast<ViewerManager*>(WM_ptr);
   _cg=static_cast<osgProducer::OsgCameraGroup*>(
      ViewerManager_ptr->get_Viewer_ptr());

   Producer::CameraConfig* cfg = _cg->getCameraConfig();
//   Producer::Camera* cam = cfg->getCamera(0);

   for(unsigned int i=0;i<cfg->getNumberOfCameras();++i)
   {
      SnapImageDrawCallback_ptr = new SnapImageDrawCallback();
      cfg->getCamera(i)->addPostDrawCallback(SnapImageDrawCallback_ptr);
      _snapImageDrawCallbackList.push_back(SnapImageDrawCallback_ptr);
   }
   image_suffix=SnapImageDrawCallback_ptr->get_image_suffix();

   if (ViewerManager_ptr->get_Viewer_ptr())
   {
      setWriteImageFileName(ViewerManager_ptr->get_Viewer_ptr()->
                            getWriteImageFileName());
   }
   else 
   {
      setWriteImageFileName("saved_image.rgb");
   }
}

// =======================================================================
// Set & get methods
// =======================================================================

void MyViewerEventHandler::setWriteImageOnNextFrame(
   bool writeImageOnNextFrame)
{
   for(SnapImageDrawCallbackList::iterator itr=
          _snapImageDrawCallbackList.begin();
       itr!=_snapImageDrawCallbackList.end(); ++itr)
   {
      (*itr)->setSnapImageOnNextFrame(writeImageOnNextFrame);
   }
}

// -----------------------------------------------------------------------
void MyViewerEventHandler::setWriteImageFileName(const string& filename)
{
   string basename = osgDB::getNameLessExtension(filename);
   string ext = osgDB::getFileExtension(filename);
    
   unsigned int cameraNum = 0;
   for(SnapImageDrawCallbackList::iterator itr=
          _snapImageDrawCallbackList.begin();
       itr!=_snapImageDrawCallbackList.end(); ++itr, ++cameraNum)
   {
      if (cameraNum==0)
      {
         (*itr)->setFileName(filename);
      }
      else
      {
         string name(basename+"_");
         name += ('0'+cameraNum);
         name += '.';
         name += ext;
         (*itr)->setFileName(name);
      }
   }
}

// -----------------------------------------------------------------------
void MyViewerEventHandler::setRecording(bool recording_or_not)
{
   for(SnapImageDrawCallbackList::iterator itr=
          _snapImageDrawCallbackList.begin();
       itr!=_snapImageDrawCallbackList.end(); ++itr)
   {
      (*itr)->setRecordingOnOrOff(recording_or_not);
   }
}

// -----------------------------------------------------------------------
void MyViewerEventHandler::set_frame_cycle_size(int fcs)
{
   for(SnapImageDrawCallbackList::iterator itr=
          _snapImageDrawCallbackList.begin();
       itr!=_snapImageDrawCallbackList.end(); ++itr)
   {
      (*itr)->set_frame_cycle_size(fcs);
   }
}

// -----------------------------------------------------------------------
void MyViewerEventHandler::set_horiz_scale_factor(double f)
{
   for(SnapImageDrawCallbackList::iterator itr=
          _snapImageDrawCallbackList.begin();
       itr!=_snapImageDrawCallbackList.end(); ++itr)
   {
      (*itr)->set_horiz_scale_factor(f);
   }
}

// =======================================================================
// Event handling member functions
// =======================================================================

bool MyViewerEventHandler::handle(const osgGA::GUIEventAdapter& ea,
                                  osgGA::GUIActionAdapter& aa)
{
   if (!_cg) return false;

// Shut off movie recording when cumulative image counter exceeds
// user-specified upper bound:
   
   if (_recording && SnapImageDrawCallback_ptr->get_snapped_image_counter() 
       > max_snapped_images)
   {
      setRecording(false);
      _recording=false;
      cout << endl;
      cout << "Recording Stopped." << endl;
   }

   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      switch( ea.getKey() )
      {

         case osgGA::GUIEventAdapter::KEY_F1 :
         {
            Producer::CameraConfig* cfg = _cg->getCameraConfig();

            bool shouldBeFullScreen = false;
            for( unsigned int i = 0; i < cfg->getNumberOfCameras(); ++i )
            {
               Producer::Camera *cam = cfg->getCamera(i);
               Producer::RenderSurface* rs = cam->getRenderSurface();
                  
               if(i==0) shouldBeFullScreen =! rs->isFullScreen(); 
// Remember the initial state of the first render surface

               if ( shouldBeFullScreen!=rs->isFullScreen() )       

// If the current render surface hasn't been modified already
               {
                  if (_firstTimeTogglingFullScreen && 
                      rs->isFullScreen())
                  {
                     unsigned int screenWidth;
                     unsigned int screenHeight;
                     unsigned int windowWidth;
                     unsigned int windowHeight;
                     rs->getScreenSize( screenWidth, screenHeight );
                     if( screenHeight > screenWidth )
                     {
                        windowWidth  = 
                           (unsigned int)((float)screenWidth * 0.625);
                        windowHeight = 
                           (unsigned int)((float)windowWidth * 0.75);
                     }
                     else
                     {
                        windowHeight = 
                           (unsigned int)((float)screenHeight*0.625);
                        windowWidth  = 
                           (unsigned int)((float)windowHeight*1.334);
                     }
                     int x = (screenWidth - windowWidth) >> 1;
                     int y = (screenHeight - windowHeight) >> 1;
#ifndef WIN32                    
                     rs->useBorder(true);
#else                        
                     rs->fullScreen(false);
#endif
                     rs->setWindowRectangle(x,y,windowWidth,windowHeight);
                  }
                  else
                  {
                     rs->fullScreen(!rs->isFullScreen());
                  }
               }
            }
            _firstTimeTogglingFullScreen = false;

            return true;
         }

         case 'O' :
         {
            setWriteImageOnNextFrame(true);                
            return true;
         }

/*
         case '!' :
         {
            cout << "Trying to write out entire scenegraph" << endl;
            osg::Node* root=WindowManager_ptr->getSceneData_ptr();
            
            ofstream binary_outstream;
//            string output_filename="root.ive";
            string output_filename="root.osg";
            filefunc::deletefile(output_filename);
            
            if ( osgDB::writeNodeFile( *root, output_filename) )
               osg::notify(osg::NOTICE) << "Wrote .ive file: " 
                                        << output_filename << "\n";
            else
               osg::notify(osg::WARN) << "Could not write .ive file.\n";

            return true;
         }
*/

         case 'q' :
         {
            if (_recording == true)
            {
               return end_recording_movie();
            }
            else //start recording
            {
               return begin_recording_movie();
            }
         }

//         case osgGA::GUIEventAdapter::KEY_Help :
//         case 'h' :
//         {
//            setDisplayHelp(!getDisplayHelp());
//            return true;
//         }

         case 'Z' :
         case 'z' :
         {
            osgProducer::Viewer* viewer = 
               dynamic_cast<osgProducer::Viewer*>(_cg);
            cout << "viewer = " << viewer << endl;
            if (viewer)
            {
               if (viewer->getRecordingAnimationPath())
               {
                  // have already been recording so switch of recording.
                  viewer->setRecordingAnimationPath(false);

                  osg::notify(osg::NOTICE) 
                     << "Finished recording camera animation; Press 'Z' to replay."
                     << endl;

                  if (viewer->getAnimationPath())
                  {
                     ofstream fout("saved_animation.path");
                     viewer->getAnimationPath()->write(fout);
                     fout.close();

                     osg::notify(osg::NOTICE) << 
                        "Saved camera animation to 'saved_animation.path'"
                                              << endl;
                  }
               }
               else if (ea.getKey()=='z') 
               {
                  viewer->setRecordingAnimationPath(true);
                  viewer->setAnimationPath(new osg::AnimationPath());
                  viewer->getAnimationPath()->setLoopMode(
                     osg::AnimationPath::LOOP);
//                           osg::AnimationPath::NO_LOOPING);
                  osg::notify(osg::NOTICE) 
                     << "Recording camera animation, press 'z' to finish recording."<< endl;
               }

               if (ea.getKey()=='Z')
               {
                  //always only read in the path as
                  //saved_animation.path, this could later be changed
                  //so that you can read in any filename
                  string pathfile = "saved_animation.path";
                  if (!osgDB::fileExists(pathfile))
                  {
                     cout << 
                        "Error!  No file named saved_animation.path to play!" 
                          << endl;
                     return false;
                  }

                  if (CustomManipulator_ptr->get_ndims()==3)
                  {
                     osgGA::Custom3DManipulator* CM_3D_ptr=
                        dynamic_cast<osgGA::Custom3DManipulator*>(
                           CustomManipulator_ptr);
                     apm_refptr=new osgGA::CustomAnimationPathManipulator(
                        pathfile,WindowManager_ptr,CM_3D_ptr);
                     apm_refptr->set_grid_origin_ptr(
                        CM_3D_ptr->get_grid_origin_ptr());
                  }
                  else if (CustomManipulator_ptr->get_ndims()==2)
                  {
                     osgGA::Custom2DManipulator* CM_2D_ptr=
                        dynamic_cast<osgGA::Custom2DManipulator*>(
                           CustomManipulator_ptr);
                     apm_refptr=new osgGA::CustomAnimationPathManipulator(
                        pathfile,WindowManager_ptr,CM_2D_ptr);
                  }
                  
                  if ( apm_refptr.valid() )
                  {
                     //only play the animation path once, you
                     //can set it to LOOP to have it loop over
                     //and over NOTE: once the video plays, you
                     //must reselect the Custom Manipulator by
                     //pressing the 1 key if you want to
                     //continue to manipulate the data
                     apm_refptr.get()->getAnimationPath()->setLoopMode(
                        osg::AnimationPath::LOOP);
//                               osg::AnimationPath::NO_LOOPING);

                     unsigned int num=viewer->addCameraManipulator(
                        apm_refptr.get());
                     viewer->selectCameraManipulator(num);
                  }
               }
               break;
            }
            return true;
         }

         default:
            break;

      }
   }
   return false;

}

// -----------------------------------------------------------------------
void MyViewerEventHandler::accept(osgGA::GUIEventHandlerVisitor& gehv)
{
   gehv.visit(*this);
}

// -----------------------------------------------------------------------
// Member function generate_movie() runs the MENCODER program on a set
// of images within a specified subdirectory.  It uses the specified
// codec for the output file.  We have chosen default values for the
// bitrate and frame rate for reasonable playback of auto-generated
// Afghanistan LOS movies.

bool MyViewerEventHandler::generate_movie(string video_codec)
{
//   cout << "inside MyViewerEventHandler::generate_movie()" << endl;
//   cout << "video_codec = " << video_codec << endl;
//   cout << "output_imagery_subdir = " << output_imagery_subdir << endl;

// In May 2010, we experienced bad caching problems for both FLV and
// AVI movies generated with names which repeat with each LOST
// session.  So Jennifer Drexler recommended that we assign unique
// time stamps to every output FLV and AVI file.  The time stamps
// should ensure that LOST's flash movie player loads the latest FLV
// file and that the latest AVI movies appear within the
// movies_and_screen_shots folder on the LOST desktop:

   clock.current_local_time_and_UTC();
   string day_hour_separator_char="_";
   string time_stamp=clock.YYYY_MM_DD_H_M_S(day_hour_separator_char);
//   cout << "time_stamp = " << time_stamp << endl;
   string output_movie_filename_prefix="movie_"+time_stamp;

//   videofunc::generate_AVI_movie(
//      video_codec,output_imagery_subdir,image_suffix,
//      fps,output_movie_filename_prefix,finished_movie_subdir);

   string unix_command=
      "mencoder \"mf://"+output_imagery_subdir+"*."+image_suffix+"\" ";
//   cout << "unix_command = " << unix_command << endl;
   
   unix_command += 
//      "-ovc lavc -lavcopts vcodec=msmpeg4v2:vbitrate=24000000 -mf fps=5 ";
      "-ovc lavc -lavcopts ";
   unix_command += "vcodec="+video_codec+":";

   string output_suffix;

// On 6/15/09, we empirically found that MPEG1 and MPEG2 codecs cannot
// support frame rates smaller than 5 fps.  On the other hand, MPEG4
// codec can support a 3 fps rate:

   if (video_codec=="mpeg1video")
   {
      output_suffix="_mp1.avi";
      unix_command += "vbitrate=24000000 -mf fps=5 ";
   }
   else if (video_codec=="mpeg2video")
   {
      output_suffix="_mp2.avi";
      unix_command += "vbitrate=24000000 -mf fps=5 ";
   }
   else if (video_codec=="msmpeg4v2")
   {
      output_suffix="_mp4.avi";
      unix_command += "vbitrate=24000000 -mf fps="
         + stringfunc::number_to_string(fps)+" ";
   }

   string output_movie_filename=output_movie_filename_prefix+
      output_suffix;

   unix_command += "-o "+output_movie_filename;
//         cout << "unix_command = " << unix_command << endl;

   sysfunc::unix_command(unix_command);

// Move movie into subdirectory on the computer's desktop:

//   unix_command="mv "+output_movie_filename+" "+finished_movie_subdir;
//   sysfunc::unix_command(unix_command);
//    output_movie_filename=finished_movie_subdir+output_movie_filename;

   return true;
}

// -----------------------------------------------------------------------
// Member function generate_flash_movie() runs the MENCODER program on
// a set of images within a specified subdirectory.  It first
// generates a movie with an AVI format and MPEG4 codec using
// parameters chosen for good intermediary quality output.  It then
// calls MENCODER again to convert the intermediate movie into flash
// FLV format for playback within a web browser.

bool MyViewerEventHandler::generate_flash_movie()
{
//   cout << "inside MyViewerEventHandler::generate_flash_movie()" << endl;
//   cout << "output_imagery_subdir = " << output_imagery_subdir << endl;

   string unix_command=
      "mencoder \"mf://"+output_imagery_subdir+"*."+image_suffix+"\" ";
   unix_command += "-ovc lavc -lavcopts ";
   unix_command += "vcodec=msmpeg4v2:vbitrate=24000000 ";

/*
   unix_command += "vcodec=mpeg4:vbitrate=24000000:mbd=2:trell:v4mv:last_pred=2:dia=-1:vmax_b_frames=2:vb_strategy=1:cmp=3:subcmp=3:precmp=0:vqcomp=0.6:turbo ";
*/

   avi_movie_filename="movie_mpeg4.avi";
   unix_command += "-mf fps="+stringfunc::number_to_string(fps)
      +" -o "+avi_movie_filename;
   
//    cout << "unix_command = " << unix_command << endl << endl;
   sysfunc::unix_command(unix_command);

   double progress=0.25;
   report_progress(progress);

// Generate temporary FLV flash movie:

   unix_command="mencoder "+avi_movie_filename+" -o movie.flv ";
   unix_command += 
      "-of lavf -oac mp3lame -lameopts abr:br=56 -srate 22050 -ovc lavc ";
   unix_command += 
      "-lavcopts  vcodec=flv:vbitrate=24000000:mbd=2:trell:v4mv:last_pred=2";
//   cout << "unix_command = " << unix_command << endl << endl;
   sysfunc::unix_command(unix_command);

// Move movie.flv to webapps subdirectory of tomcat so that it can
// readily be loaded into a thin client browser.  

//   cout << "tomcat_subdir = " << tomcat_subdir << endl;
   string movies_subdir=tomcat_subdir+"movies/";

   clock.current_local_time_and_UTC();
   string day_hour_separator_char="_";
   string time_stamp=clock.YYYY_MM_DD_H_M_S(day_hour_separator_char);
   cout << "time_stamp = " << time_stamp << endl;

   string flv_movie_filename="movie_"+time_stamp+".flv";
   unix_command="mv movie.flv "+movies_subdir+flv_movie_filename;
   flv_movie_path=movies_subdir+flv_movie_filename;
//   cout << "flv_movie_path = " << flv_movie_path << endl;
//   cout << "unix_command = " << unix_command << endl << endl;
   sysfunc::unix_command(unix_command);

/*
// Generate SWF flash movie:

   unix_command="mencoder movie_mpeg4.avi -o movie.swf ";
   unix_command += 
      "-of lavf -oac mp3lame -lameopts abr:br=56 -srate 22050 -ovc lavc ";
   unix_command += 
      "-lavcopts  vcodec=flv:vbitrate=24000000:mbd=2:trell:v4mv:last_pred=2";

//   cout << "unix_command = " << unix_command << endl;
   sysfunc::unix_command(unix_command);
*/


   return true;
}

// -----------------------------------------------------------------------
// Member function purge_flash_movies() deletes all movies within
// tomcat_subdir/movies/.

void MyViewerEventHandler::purge_flash_movies()
{
   cout << "inside MyViewerEventHandler::purge_flash_movies()" << endl;

//   cout << "tomcat_subdir = " << tomcat_subdir << endl;
   string movies_subdir=tomcat_subdir+"movies/";

   if (!filefunc::direxist(movies_subdir))
   {
      filefunc::dircreate(movies_subdir);
   }
   else
   {
      filefunc::purge_files_in_subdir(movies_subdir);
   }
}

// =======================================================================
bool MyViewerEventHandler::begin_recording_movie()
{
   cout << "inside MyViewerEventHandler::begin_recording_movie()" << endl;

   _recording = true;

   string moviename;
   int frame_cycle_size=10;

   cout << "auto_generate_movies_flag = " << auto_generate_movies_flag
        << endl;
   
   if (auto_generate_movies_flag)
   {
      moviename="movie_frame";
      frame_cycle_size=1;
      max_snapped_images=1000;
   }
   else
   {
      cout << "Enter name for movie to be recorded: " << endl;
      cin >> moviename;

      cout << "Enter frame cycle size (default = 10):" << endl;
      cout << "Larger values imply SPARSER sampling of flight trajectory" 
           << endl;
      cin >> frame_cycle_size;
                  
      cout << "Enter maximum cumulative number of images to be snapped:"
           << endl;
      cin >> max_snapped_images;
   }

   string base_output_imagery_subdir="./recorded_video/";
   filefunc::dircreate(base_output_imagery_subdir);
   output_imagery_subdir=base_output_imagery_subdir+moviename+"/";
   cout << "output_imagery_subdir = " << output_imagery_subdir << endl;

   if (auto_generate_movies_flag)
   {
      cout << "output_imagery_subdir = " << output_imagery_subdir << endl;
//      cout << "Before call to purge_files_in_subdir()" << endl;  
//      outputfunc::enter_continue_char();
      filefunc::purge_files_in_subdir(output_imagery_subdir);
   }
               
   set_frame_cycle_size(frame_cycle_size);

   if (!osgDB::fileExists("recorded_video"))
   {
      osgDB::makeDirectory("recorded_video");
   }
   if (!osgDB::makeDirectory("recorded_video/"+moviename))
   {
      cout << "Error!  Could not create new directory" << endl;
      return false;
   }

   setRecording(true);
   setWriteImageFileName(moviename);
   cout << "moviename = " << moviename << endl;
   cout << "Recording movie..." << endl;
   return true;
}

// -----------------------------------------------------------------------
bool MyViewerEventHandler::end_recording_movie()
{
   cout << "inside MyViewerEventHandler::end_recording_movie()" << endl;

   _recording = false;

   double progress=0;
   report_progress(progress);

   n_snapped_images=SnapImageDrawCallback_ptr->get_snapped_image_counter();
   cout << "total n_snapped_images = " << n_snapped_images << endl;
   total_movie_duration=n_snapped_images/fps;	// secs
   cout << "total movie duration = " << total_movie_duration << endl;

   setRecording(false);
   cout << endl;
   cout << "Recording Stopped." << endl;

   if (auto_generate_movies_flag)
   {
      double progress=0.1;
      report_progress(progress);

      generate_flash_movie();
      progress=0.5;
      report_progress(progress);
      
      string video_codec="mpeg1video";
      generate_movie(video_codec);
      progress=0.60;
      report_progress(progress);

      video_codec="mpeg2video";
      generate_movie(video_codec);
      progress=0.80;
      report_progress(progress);

      video_codec="msmpeg4v2";
      generate_movie(video_codec);
      progress=1.0;
      report_progress(progress);

// Move AVI movie generated with MPEG4 codec into movie subdir:

// On 7/28/09, we empirically determined that the AVI movie with the
// MPEG4 codec does NOT play under windows.  So there's no point in
// bothering to move it to the output movie subdir.

//      string unix_command="mv "+avi_movie_filename+" "+finished_movie_subdir;
      string unix_command="/bin/rm "+avi_movie_filename;
      sysfunc::unix_command(unix_command);
   }

   return true;
}

// -----------------------------------------------------------------------
// Member function retrieve_movie_frame_file() takes in a frame time
// ranging between 0 and a captured movie's total duration.  It
// computes the screen capture file number corresponding to the input
// fraction and returns the pathname to the image file.

string MyViewerEventHandler::retrieve_movie_frame_file(double frame_time)
{
   cout << "inside MyViewerEventHandler::retrieve_movie_frame_file()" 
        << endl;
   double frac=frame_time/total_movie_duration;
   cout << "n_snapped_images = " << n_snapped_images << endl;
   cout << "frac = " << frac << endl;
//   int requested_imagenumber=basic_math::round(frac*n_snapped_images);
   int requested_imagenumber=basic_math::round(frac*n_snapped_images)+1;
   requested_imagenumber=basic_math::min(
      n_snapped_images,requested_imagenumber);

   string requested_filename=output_imagery_subdir+"movie_frame"+
      stringfunc::integer_to_string(requested_imagenumber,4)+"."
      +image_suffix;
   cout << "requested_filename = " << requested_filename << endl;
   return requested_filename;
}

// -----------------------------------------------------------------------
// Member function report_progress()

void MyViewerEventHandler::report_progress(double progress)
{
   cout << "inside MyViewerEventHandler::report_progress, progress = "
        << progress << endl;
   
   if (Viewer_Messenger_ptr != NULL)
   {
      string progress_type="movie generation";
      Viewer_Messenger_ptr->broadcast_progress(progress,progress_type);
   }
//   cout << "at end of MyViewerEventHandler::report_progress, progress = "
//        << progress << endl;
}

