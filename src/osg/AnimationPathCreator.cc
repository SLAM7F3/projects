// ==========================================================================
// AnimationPathCreator class 
// ==========================================================================
// Last modified on 3/10/07; 9/5/07; 9/20/07
// ==========================================================================

#include "osg/AnimationPathCreator.h"
#include "osg/osgWindow/WindowManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void AnimationPathCreator::allocate_member_objects()
{
}		       

void AnimationPathCreator::initialize_member_objects()
{
   WindowManager_ptr=NULL;
}		       

AnimationPathCreator::AnimationPathCreator(WindowManager* WM_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   WindowManager_ptr=WM_ptr;
   time = 0;

//   WindowManager_ptr->get_EventHandlers_ptr()->push_back(this);
}

void AnimationPathCreator::add_current_waypoint()
{
   cout << "inside APC::add_curr_waypoint()" << endl;

   osg::Vec3    eye, center, up;
   osg::Quat    quat;

   if (WindowManager_ptr != NULL)
   {
      WindowManager_ptr->retrieve_camera_posn_and_direction_vectors(
         eye,center,up);
      WindowManager_ptr->getViewMatrix().get(quat);
   }
   else
   {
      cout << "Error in AnimationPathCreator::add_current_waypoint()" << endl;
      cout << "WindowManager_ptr = NULL !!!" << endl;
      return;
   }
   

   //create the pathinfo object from this information
   //only the 'eye' variable is of us, which are the xyz values
   //that the built-in OSG object to play back AnimationPaths
   //uses to decide how to position the camera

   PathInfo* current_path_entry_ptr = new PathInfo;
   current_path_entry_ptr->time = time;
   current_path_entry_ptr->xcoord = eye.x();
   current_path_entry_ptr->ycoord = eye.y();
   current_path_entry_ptr->zcoord = eye.z();
    
   //all the xyz quats need to be inverted (not sure why)
   current_path_entry_ptr->quatx = quat.x() * -1;
   current_path_entry_ptr->quaty = quat.y() * -1;
   current_path_entry_ptr->quatz = quat.z() * -1;
   current_path_entry_ptr->quatw = quat.w();
    
   //writes out the path to stdout so that the users can
   //see that they chose a waypoint
   cout << "waypoint: " <<
      current_path_entry_ptr->time << " " <<
      current_path_entry_ptr->xcoord << " " <<
      current_path_entry_ptr->ycoord << " " <<
      current_path_entry_ptr->zcoord << " " <<
      current_path_entry_ptr->quatx << " " <<
      current_path_entry_ptr->quaty << " " <<
      current_path_entry_ptr->quatz << " " <<
      current_path_entry_ptr->quatw << endl;
   path_vector.push_back(current_path_entry_ptr);

   //here is where the time gets added by seconds
   //ideally, this would be controllable by the user, so that the speed
   //and acceleration of the camera was also controllable
   time+=5;
}

void AnimationPathCreator::write_out_path_file(string filename)
{
   ofstream pathfile(filename.c_str(), ios::out);
   if (pathfile.is_open())
   {
      pathfile.precision(12);
      for (unsigned int i = 0; i < path_vector.size(); i++)
      {
         pathfile << path_vector[i]->time << " "
                  << path_vector[i]->xcoord << " "
                  << path_vector[i]->ycoord << " "
                  << path_vector[i]->zcoord << " "
                  << path_vector[i]->quatx << " "
                  << path_vector[i]->quaty << " "
                  << path_vector[i]->quatz << " "
                  << path_vector[i]->quatw << "\n";
      }
      pathfile.close();
   }
}

bool AnimationPathCreator::handle(const osgGA::GUIEventAdapter& ea, 
                                  osgGA::GUIActionAdapter& aa)
{
   if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      switch(ea.getKey())
      {
         case 'w' :
         {
            add_current_waypoint();
            return true;
         }
         case 'W' :
//         case 'x' :
         {
            time = 0;
            string filename;
            cout << "Please enter a file name for this path: ";
            cin >> filename;
            filename += ".path";
            write_out_path_file(filename);
            cout << "File " << filename << " written." << endl;
            return true;
         }
         default:
            break;
      }
   }
   return false;
}
