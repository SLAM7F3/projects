// ==========================================================================
// robots_group class member function definitions
// ==========================================================================
// Last updated on 2/14/08; 2/15/08; 2/18/08; 2/21/08
// ==========================================================================

#include <iostream>
#include <string>
#include "astro_geo/latlong2utmfuncs.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "math/constant_vectors.h"
#include "osg/osgGeometry/CylindersGroup.h"
#include "messenger/Messenger.h"
#include "robots/robots_group.h"
#include "math/statevector.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

void robots_group::allocate_member_objects()
{
}

void robots_group::initialize_member_objects()
{
   prev_framenumber=-1;
   start_time=0;

   AnimationController_ptr=NULL;
   CylindersGroup_ptr=NULL;
   Messenger_ptr=NULL;
}

robots_group::robots_group()
{
   allocate_member_objects();
   initialize_member_objects();
}

robots_group::robots_group(CylindersGroup* CylindersGroup_ptr,
                           AnimationController* AnimationController_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   this->CylindersGroup_ptr=CylindersGroup_ptr;
   this->AnimationController_ptr=AnimationController_ptr;

   start_time=AnimationController_ptr->
      get_time_corresponding_to_frame(0);
}

robots_group::~robots_group()
{
   for (int i=0; i<get_n_robots(); i++)
   {
      delete robot_ptrs[i];
   }
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void robots_group::set_Messenger_ptr(Messenger* M_ptr)
{
   Messenger_ptr=M_ptr;
}

Messenger* robots_group::get_Messenger_ptr()
{
   return Messenger_ptr;
}

const Messenger* robots_group::get_Messenger_ptr() const
{
   return Messenger_ptr;
}

// ==========================================================================
// Robot generation and propagation member functions
// ==========================================================================

robot* robots_group::generate_new_robot(int ID)
{
//   cout << "=====================================================" << endl;
//   cout << "inside robots_group::generate_new_robot, ID = " << ID << endl;

   if (ID==-1) ID=get_n_robots();

   robot* curr_robot_ptr=new robot(ID,&ground_space);
   robot_ptrs.push_back(curr_robot_ptr);

//   colorfunc::Color permanent_color=colorfunc::blue;
   colorfunc::Color permanent_color=colorfunc::cyan;

   osg::Quat trivial_q(0,0,0,1);
   int n_text_messages=1;
   double text_displacement=1;
   double text_size=500;
   bool text_screen_axis_alignment_flag=false;

   Cylinder* curr_Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
      Zero_vector,trivial_q,permanent_color,
      n_text_messages,text_displacement,text_size,
      text_screen_axis_alignment_flag,ID);

   curr_Cylinder_ptr->set_stationary_Graphical_flag(false);
   string text_label=stringfunc::number_to_string(
      curr_Cylinder_ptr->get_ID());
   curr_Cylinder_ptr->set_text_label(0,text_label);

   colorfunc::Color text_color=colorfunc::blue;
   curr_Cylinder_ptr->set_text_color(0,text_color);
      
   return curr_robot_ptr;
}

// ---------------------------------------------------------------------
// Member function propagate_all_statevectors

void robots_group::propagate_all_statevectors()
{
//   cout << "inside robots_group::propagate_all_statevectors()" << endl;

   int curr_framenumber=AnimationController_ptr->get_curr_framenumber();
   if (curr_framenumber==prev_framenumber)
   {
      return;
   }
   else
   {
      prev_framenumber=curr_framenumber;
   }
   
   curr_time=AnimationController_ptr->get_time_corresponding_to_curr_frame();
   double delta_time=curr_time-start_time;
   int pass_number=CylindersGroup_ptr->get_passnumber();

   for (int r=0; r<get_n_robots(); r++)
   {
      statevector curr_statevector=get_robot_ptr(r)->propagate_statevector(
         delta_time);

// In order to avoid annoying "race conditions" between updating robot
// statevectors and drawing robot icons within the main OSG event
// loop, we set the robot's statevector at both the current and next
// framenumber equal to curr_statevector.get_position():

      CylindersGroup_ptr->get_Cylinder_ptr(r)->
         set_UVW_coords(
            curr_framenumber,pass_number,curr_statevector.get_position());

      CylindersGroup_ptr->get_Cylinder_ptr(r)->
         set_UVW_coords(
            curr_framenumber+1,pass_number,curr_statevector.get_position());

//      cout << "curr_f = " << curr_framenumber
//           << " curr_t = " << curr_time
//           << " delta_t = " << delta_time
//           << " posn = " << curr_statevector.get_position()
//           << endl;

   } // loop over index r labeling robots

   issue_update_message();
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

// Method issue_update sends a message to the Message Queue containing
// current robot position information

void robots_group::issue_update_message()
{   
//   cout << "inside robots_group::issue_update_message()" << endl;
   
//   int curr_framenumber=AnimationController_ptr->get_curr_framenumber();
//   int pass_number=CylindersGroup_ptr->get_passnumber();

   cout.precision(12);
   for (int r=0; r<get_n_robots(); r++)
   {
//      int curr_ID=get_robot_ptr(r)->get_ID();
      
      statevector curr_statevector=get_robot_ptr(r)->get_statevector();
      threevector curr_posn=curr_statevector.get_position();
      double curr_altitude=curr_posn.get(2);

      double curr_longitude,curr_latitude;
      latlongfunc::UTMtoLL(
         ground_space.get_UTM_zonenumber(),
         ground_space.get_northern_hemisphere_flag(),
         curr_posn.get(1),curr_posn.get(0),
         curr_latitude,curr_longitude);

      double curr_heading=get_robot_ptr(r)->get_heading_angle()*180/PI;

//      cout << "t=" << curr_time
//           << " frame=" << curr_framenumber
//           << " long=" << curr_longitude
//           << " lat=" << curr_latitude 
//           << " heading = " << curr_heading
//           << " robot_posn = " << curr_robot_posn 
//           << " delta_posn = " << delta_posn
//           << endl;

      string text_message = stringfunc::number_to_string(
         get_robot_ptr(r)->get_ID())+" ";
      text_message += stringfunc::number_to_string(curr_time)+" ";
      text_message += stringfunc::number_to_string(curr_longitude,9)+" ";
      text_message += stringfunc::number_to_string(curr_latitude,9)+" ";
      text_message += stringfunc::number_to_string(curr_altitude,9)+" ";
      text_message += stringfunc::number_to_string(curr_heading,9)+" ";

//      cout << text_message << endl;

      if (get_Messenger_ptr() != NULL)
      {
         get_Messenger_ptr()->sendTextMessage(text_message);
         cout << "Sending text message = " << text_message << endl;
      }
      
   } // loop over index r labeling robots

}
