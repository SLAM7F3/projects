// ==========================================================================
// OPERATIONS class member function definitions
// ==========================================================================
// Last modified on 10/8/11; 4/3/13; 4/2/14; 7/19/16
// ==========================================================================

#include <iostream>
#include <vector>
#include "osg/AbstractOSGCallback.h"
#include "osg/osgGraphicals/AnimationKeyHandler.h"
#include "math/basic_math.h"
#include "astro_geo/Clock.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Operations::initialize_member_objects()
{
   master_world_start_time=master_world_stop_time=0;
   delta_master_world_time_step_per_master_frame=1;
}		       


void Operations::allocate_member_objects()
{
//   cout << "inside Operations::allocate_member_objects()" << endl;
//   cout << "display_movie_number = " << display_movie_number << endl;
//   cout << "display_movie_state = " << display_movie_state << endl;
//   cout << "display_movie_world_time = " << display_movie_world_time << endl;
//   cout << "display_movie_elapsed_time = " << display_movie_elapsed_time << endl;
//   cout << "display_movie_nframes = " << display_movie_nframes << endl;

   OSGgroup_refptr=new osg::Group();
   OSGgroup_refptr->setName("Operations");

   ModeController_ptr=new ModeController();
   ModeKeyHandler_ptr=new ModeKeyHandler(ModeController_ptr);

   ModeHUD_ptr=new ModeHUD(ModeController_ptr,hide_Mode_HUD_flag);
   ModeHUD_ptr->getProjection()->setUpdateCallback( 
      new AbstractOSGCallback<ModeHUD>( 
         ModeHUD_ptr, &ModeHUD::showMode) );
         
   AnimationController_ptr=new AnimationController();
   AnimationKeyHandler_ptr=
      new AnimationKeyHandler(ModeController_ptr,AnimationController_ptr);

   ImageNumberHUD_ptr=new ImageNumberHUD(
      AnimationController_ptr,display_movie_number,display_movie_state,
      display_movie_world_time,display_movie_elapsed_time,
      display_movie_nframes);
}		       

Operations::Operations(
   int ndims,WindowManager* window_mgr_ptr,
   bool display_movie_state,bool display_movie_number,
   bool display_movie_world_time,bool display_movie_elapsed_time,
   bool hide_Mode_HUD_flag, bool display_movie_nframes)
{	
//   cout << "inside Operations constructor #1" << endl;
   
   this->ndims=ndims;
   this->window_mgr_ptr=window_mgr_ptr;
   this->display_movie_state=display_movie_state;
   this->display_movie_number=display_movie_number;
   this->display_movie_world_time=display_movie_world_time;
   this->display_movie_elapsed_time=display_movie_elapsed_time;
   this->display_movie_nframes=display_movie_nframes;
   this->hide_Mode_HUD_flag=hide_Mode_HUD_flag;

   initialize_member_objects();
   allocate_member_objects();

   OSGgroup_refptr->addChild(ModeHUD_ptr->getProjection());
   OSGgroup_refptr->addChild(AnimationController_ptr->get_OSGgroup_ptr());
   OSGgroup_refptr->addChild(ImageNumberHUD_ptr->getProjection());

   if (window_mgr_ptr != NULL)
   {
      window_mgr_ptr->get_EventHandlers_ptr()->push_back(ModeKeyHandler_ptr);
      window_mgr_ptr->get_EventHandlers_ptr()->push_back(
         AnimationKeyHandler_ptr);
   }
}		      

Operations::Operations(
   int ndims,WindowManager* window_mgr_ptr,const PassesGroup& passes_group,
   bool display_movie_state,bool display_movie_number,
   bool display_movie_world_time,bool display_movie_elapsed_time,
   bool hide_Mode_HUD_flag, bool display_movie_nframes)
{	
//   cout << "inside Operations constructor #2" << endl;

   this->ndims=ndims;
   this->window_mgr_ptr=window_mgr_ptr;
   this->display_movie_state=display_movie_state;
   this->display_movie_number=display_movie_number;
   this->display_movie_world_time=display_movie_world_time;
   this->display_movie_elapsed_time=display_movie_elapsed_time;
   this->display_movie_nframes=display_movie_nframes;
   this->hide_Mode_HUD_flag=hide_Mode_HUD_flag;

   initialize_member_objects();
   allocate_member_objects();

   OSGgroup_refptr->addChild(ModeHUD_ptr->getProjection());
   OSGgroup_refptr->addChild(AnimationController_ptr->get_OSGgroup_ptr());
   OSGgroup_refptr->addChild(ImageNumberHUD_ptr->getProjection());

   if (window_mgr_ptr != NULL)
   {
      window_mgr_ptr->get_EventHandlers_ptr()->push_back(ModeKeyHandler_ptr);
      window_mgr_ptr->get_EventHandlers_ptr()->push_back(
         AnimationKeyHandler_ptr);
   }

   set_initial_mode(passes_group);
}		      

Operations::~Operations()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Operations& O)
{
   return(outstream);
}

// ==========================================================================
// Member function set_initial_mode reads in a single-word mode label
// entered as a command line argument.  We assume that a genuinely
// multi-word phrase such as "Manipulate Annotation Mode" is inputed
// as "Manipulate_Annotation_Mode".  This method replaces the
// underscore characters with single spaces.  It then sets the
// ModeController's state based upon the modified mode label.

// We wrote this method in Oct 2007 to minimize the number of
// keystrokes which need to be entered to run the touchtable NYC demo.

void Operations::set_initial_mode(const PassesGroup& passes_group)
{
//   cout << "inside Operations::set_initial_mode()" << endl;
   string initial_mode_label=passes_group.get_initial_mode_string();
//   cout << "initial_mode_label = " << initial_mode_label << endl;

   string mode_label=stringfunc::find_and_replace_char(
      initial_mode_label,"_"," ");
//   cout << "mode_label = " << mode_label << endl;
   
   for (int n=0; n<ModeController_ptr->get_n_states(); n++)
   {
      if (mode_label==ModeController_ptr->get_state_name(n))
      {
         ModeController_ptr->setState(static_cast<ModeController::eState>(n));
      }
   }
}

// ==========================================================================
// Master world time member functions
// ==========================================================================

void Operations::set_master_world_start_UTC(
   string world_start_UTC_string)
{
//   cout << "inside Operations::set_master_world_start_UTC()" << endl;
   const string separator_chars=",";
   vector<double> world_start_UTC_times=stringfunc::string_to_numbers(
      world_start_UTC_string,separator_chars);

   if (world_start_UTC_times.size() != 6)
   {
      cout << "Error in Operations::set_master_world_start_UTC()!" << endl;
      cout << "world_start_UTC_times.size() = " 
           << world_start_UTC_times.size() << endl;
      exit(-1);
   }

   int year=static_cast<int>(world_start_UTC_times[0]);
   int month=static_cast<int>(world_start_UTC_times[1]);
   int day=static_cast<int>(world_start_UTC_times[2]);
   int hour=static_cast<int>(world_start_UTC_times[3]);
   int minute=static_cast<int>(world_start_UTC_times[4]);
   double secs=world_start_UTC_times[5];

   bool start_time_flag=true;
   set_master_world_UTC(year,month,day,hour,minute,secs,start_time_flag);
}

// ---------------------------------------------------------------------
void Operations::set_master_world_stop_UTC(string world_stop_UTC_string)
{
//   cout << "inside Operations::set_master_world_stop_UTC()" << endl;
   // cout << "world_stop_UTC_string = " << world_stop_UTC_string << endl;
   
   const string separator_chars=",";
   vector<double> world_stop_UTC_times=stringfunc::string_to_numbers(
      world_stop_UTC_string,separator_chars);

   if (world_stop_UTC_times.size() != 6)
   {
      cout << "Error in Operations::set_master_world_stop_UTC()!" << endl;
      cout << "world_stop_UTC_times.size() = " << world_stop_UTC_times.size()
           << endl;
      exit(-1);
   }
   
   int year=static_cast<int>(world_stop_UTC_times[0]);
   int month=static_cast<int>(world_stop_UTC_times[1]);
   int day=static_cast<int>(world_stop_UTC_times[2]);
   int hour=static_cast<int>(world_stop_UTC_times[3]);
   int minute=static_cast<int>(world_stop_UTC_times[4]);
   double secs=world_stop_UTC_times[5];

   bool start_time_flag=false;
   set_master_world_UTC(year,month,day,hour,minute,secs,start_time_flag);
}

// ---------------------------------------------------------------------
void Operations::set_master_world_UTC(
   int year,int month,int day,int hour,int minute,double secs,
   bool start_time_flag)
{
//   cout << "inside Operations::set_master_world_UTC()" << endl;
//   cout << "year = " << year
//        << " month = " << month
//        << " day = " << day
//        << " hour = " << hour
//        << " minute = " << minute
//        << " secs = " << secs << endl;
//   cout << "start_time_flag = " << start_time_flag << endl;
   
   Clock master_clock;
   master_clock.set_UTC_time(year,month,day,hour,minute,secs);

   if (start_time_flag)
   {
      set_master_world_start_time(
         master_clock.secs_elapsed_since_reference_date());
   }
   else
   {
      set_master_world_stop_time(
         master_clock.secs_elapsed_since_reference_date());
   }
}

// ---------------------------------------------------------------------
void Operations::set_master_world_UTC(
   double elapsed_epoch_secs,bool start_time_flag)
{
   if (start_time_flag)
   {
      set_master_world_start_time(elapsed_epoch_secs);
   }
   else
   {
      set_master_world_stop_time(elapsed_epoch_secs);
   }
}

// ---------------------------------------------------------------------
// Member function set_current_master_clock_time_duration_and_step()
// takes in the duration for a simulation.  It sets the master clock's
// starting time to the current UTC and its stopping time to the
// starting time + simulation duration.  This method also sets the
// master clock's time step per master frame.

void Operations::set_current_master_clock_time_duration_and_step(
   int n_simulation_hours,int n_simulation_mins,
   double world_time_step_in_secs_per_frame)
{
//   cout << "inside Operations::set_current_master_clock_time_and_duration() #1" 
//        << endl;
//   cout << "n_simul_hours = " << n_simulation_hours
//        << " n_simul_mins = " << n_simulation_mins << endl;
//   cout << "world_time_step_in_secs_per_frame = "
//        << world_time_step_in_secs_per_frame << endl;

   Clock* clock_ptr=get_Clock_ptr();
   clock_ptr->current_local_time_and_UTC();

// First set master clock's stopping time to current UTC + simulation
// duration:

   clock_ptr->set_UTC_time(
      clock_ptr->get_year(),clock_ptr->get_month(),clock_ptr->get_day(),
      clock_ptr->get_UTC_hour()+n_simulation_hours,
      clock_ptr->get_minute()+n_simulation_mins,clock_ptr->get_seconds());

   bool start_time_flag=false;
   set_master_world_UTC(
      clock_ptr->get_year(),clock_ptr->get_month(),clock_ptr->get_day(),
      clock_ptr->get_UTC_hour(),clock_ptr->get_minute(),
      clock_ptr->get_seconds(),start_time_flag);

// Next set master clock's starting time to current UTC:

   clock_ptr->current_local_time_and_UTC();
   start_time_flag=true;
   set_master_world_UTC(
      clock_ptr->get_year(),clock_ptr->get_month(),clock_ptr->get_day(),
      clock_ptr->get_UTC_hour(),clock_ptr->get_minute(),clock_ptr->
      get_seconds(),start_time_flag);

   set_delta_master_world_time_step_per_master_frame(
      world_time_step_in_secs_per_frame);

//   cout.precision(12);
//   cout << "get_master_world_start_time() = "
//        << get_master_world_start_time() << endl;
//   cout << "get_master_world_stop_time() = "
//        << get_master_world_stop_time() << endl;
//   cout << "get_delta_master_world_time_step_per_master_frame() = "
//        << get_delta_master_world_time_step_per_master_frame() << endl;

   reset_AnimationController_world_time_params();
}

// This overloaded version of
// set_current_master_clock_time_duration_and_step() takes in the
// total simulation time measured in fractional hours.

void Operations::set_current_master_clock_time_duration_and_step(
   double n_simulation_hours,double world_time_step_in_secs_per_frame)
{
   int n_hours=basic_math::mytruncate(n_simulation_hours);
   int n_mins=60*(n_simulation_hours-n_hours);

   if (n_mins >= 60)
   {
      n_hours++;
      n_mins -= 60;
   }

   set_current_master_clock_time_duration_and_step(
      n_hours,n_mins,world_time_step_in_secs_per_frame);
}

void Operations::set_current_master_clock_time_duration(
   double n_simulation_hours)
{
//   cout << "inside Operations::set_current_master_clock_time_duration() #3"
//        << endl;
   set_current_master_clock_time_duration_and_step(
      n_simulation_hours,get_delta_master_world_time_step_per_master_frame());
}

// ---------------------------------------------------------------------
// Member function reset_AnimationController_world_time_params()

void Operations::reset_AnimationController_world_time_params()
{
//   cout << "inside Operations::reset_AnimationController_world_time_params()" 
//        << endl;

   AnimationController_ptr->set_world_time_params(
      get_master_world_start_time(),
      get_master_world_stop_time(),
      get_delta_master_world_time_step_per_master_frame());
}
