// ==========================================================================
// AnimationController class member function definitions
// ==========================================================================
// Last modified on 4/3/13; 4/6/14; 1/22/16; 9/9/16
// ==========================================================================

#include <iostream>
#include <string>
#include "osg/AbstractOSGCallback.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "general/filefuncs.h"
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "filter/piecewise_linear.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void AnimationController::allocate_member_objects()
{
   clock_ptr=new Clock();
   OSGgroup_refptr=new osg::Group;
}		       

void AnimationController::initialize_member_objects()
{
   increment_time_rather_than_frame_number_flag=false;
   m_delay=m_extended_delay=0.05;
   m_currTime=m_prevTime=0.0;
   curr_state=prev_state=PAUSE;
   timefunc::initialize_timeofday_clock();

   indep_time_depend_counter_ptr=NULL;
   indep_counter_depend_time_ptr=NULL;
   master_AnimationController_ptr=NULL;
   messenger_ptr=NULL;

   set_first_framenumber(0);
   curr_framenumber=prev_framenumber=get_first_framenumber();
   set_nframes(1);
   set_frame_skip(1);
   cumulative_framecounter=0;
   loop_to_beginning=loop_to_end=AVI_movie_generation_flag=false;
   frame_counter_offset=0;
   time_offset=delta_world_time_per_frame=0;

   OSGgroup_refptr->setUpdateCallback( 
      new AbstractOSGCallback<AnimationController>(
         this, &AnimationController::update));
}		       

AnimationController::AnimationController()
{
   allocate_member_objects();
   initialize_member_objects();
}

AnimationController::AnimationController(int num_frames)
{
   allocate_member_objects();
   initialize_member_objects();
   set_nframes(num_frames);
}

// This next overloaded constructor takes in an STL vector containing
// times (measured in secs relative to some reference Julian date) and
// corresponding frame counter numbers.  It instantiates two
// piecewise_linear objects which can be used to retrieve a world time
// corresponding to a frame counter and vice-versa:

AnimationController::AnimationController(
   const vector<twovector>& time_counter_samples)
{
   allocate_member_objects();
   initialize_member_objects();

   correlate_frames_to_world_times(time_counter_samples);
}

AnimationController::~AnimationController()
{
   delete clock_ptr;
   delete indep_time_depend_counter_ptr;
   delete indep_counter_depend_time_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const AnimationController& AC)
{
   outstream << endl;
   outstream << "this = " << &AC << endl;
   outstream << "n_frames = " << AC.n_frames << endl;
   outstream << "first_framenumber = " << AC.first_framenumber << endl;
   outstream << "curr_framenumber = " << AC.curr_framenumber << endl;
   outstream << "cumulative_framecounter = " << AC.cumulative_framecounter 
             << endl;

   return outstream;
}

// ==========================================================================
// Set & get methods
// ==========================================================================

void AnimationController::setState(
   AnimationController::eState p_state)
{ 
   prev_state=curr_state;
   curr_state = p_state; 
}

AnimationController::eState AnimationController::getState() const
{
   return curr_state;
}

const AnimationController::eState* AnimationController::getStatePtr() const
{
   return &curr_state;
}

void AnimationController::setDelay(double delay)
{
   m_delay=delay;
   m_delay=basic_math::max(0.0,m_delay);
}

void AnimationController::setExtendedDelay(double e_delay)
{
   m_extended_delay=e_delay;
   m_extended_delay=basic_math::max(m_delay,m_extended_delay);
}

double AnimationController::getDelay() const
{
   return m_delay;
}
  
void AnimationController::increment_delay()
{
   m_delay += 0.05;
   m_extended_delay += 0.05;
}

void AnimationController::decrement_delay()
{
   m_delay -= 0.05;
   m_extended_delay -= 0.05;
   m_delay=basic_math::max(0.0,m_delay);
   m_extended_delay=basic_math::max(0.0,m_extended_delay);
}

void AnimationController::set_first_framenumber(int frame)
{
//   cout << "inside AC::set_first_framenumber()" << endl;
   first_framenumber=frame;
//   cout << "first_framenumber = " << first_framenumber << endl;
}

int AnimationController::get_first_framenumber() const
{
   return first_framenumber;
}

void AnimationController::set_last_framenumber(int final_frame)
{
//   cout << "inside AnimationController::set_last_framenumber(), final_frame = "
//        << final_frame << endl;
   set_nframes(final_frame-get_first_framenumber()+1);
}

int AnimationController::get_last_framenumber() const
{
   return (get_first_framenumber()+get_nframes()-1);
}

void AnimationController::set_prev_framenumber(int frame)
{
   prev_framenumber=frame;
}

int AnimationController::get_prev_framenumber() const
{
   return prev_framenumber;
}

void AnimationController::set_curr_framenumber(int frame)
{
   set_prev_framenumber(curr_framenumber);
   curr_framenumber=frame;
   broadcast_curr_frame_and_UTC_time();
}

int AnimationController::get_curr_framenumber() const
{
//   cout << "inside AC::get_curr_framenumber(), this = " << this << endl;
//   cout << "curr_framenumber = " << curr_framenumber << endl;
   return curr_framenumber;
}

bool AnimationController::curr_framenumber_equals_prev_framenumber() const
{
   return (curr_framenumber==prev_framenumber);
}

void AnimationController::set_nframes(int n)
{
//   cout << "inside AnimationController::set_nframes(), n = " << n << endl;
   n_frames=n;
//   outputfunc::enter_continue_char();
}

int AnimationController::get_nframes() const
{
   return n_frames;
}

void AnimationController::set_frame_skip(int skip)
{
   frame_skip=skip;
}

int AnimationController::get_frame_skip() const
{
   return frame_skip;
}

int AnimationController::increment_frame_skip() 
{
   frame_skip++;
   return frame_skip;
}

int AnimationController::decrement_frame_skip() 
{
   frame_skip=basic_math::max(1,frame_skip-1);
   return frame_skip;
}

// ==========================================================================
// Frame and world time correlation member functions
// ==========================================================================

// Member function set_world_time_params() is meant to be called by a
// master game clock to which all other AnimationControllers should be
// slaved.  The starting and stopping times are specified in secs
// since some epoch time (e.g. midnight on Jan 1, 1970).

void AnimationController::set_world_time_params(
   double start_secs_since_epoch,double stop_secs_since_epoch,
   double delta_time_step_per_frame)
{
//   cout << "inside AnimationController::set_world_time_params()" << endl;
//   cout << "start_secs_since_epoch = " << start_secs_since_epoch << endl;
//   cout << "stop_secs_since_epoch = " << stop_secs_since_epoch << endl;
//   cout << "delta_time_step_per_frame = " << delta_time_step_per_frame << endl;

   delta_world_time_per_frame=delta_time_step_per_frame;
   int n_frames=(stop_secs_since_epoch-start_secs_since_epoch)/
      delta_time_step_per_frame+1;
//  cout << "n_frames = " << n_frames << endl;

   stop_secs_since_epoch=start_secs_since_epoch+
      (n_frames-1)*delta_world_time_per_frame;
//   cout << "Revised stop_secs_since_epoch = " << stop_secs_since_epoch 
//        << endl;

   int start_frame=0;
   int stop_frame=start_frame+n_frames-1;
   specify_extremal_frame_world_times(
      start_frame,stop_frame,start_secs_since_epoch,stop_secs_since_epoch);

//  cout << "start_frame = " << start_frame
//       << " stop_frame = " << stop_frame << endl;

   set_first_framenumber(start_frame);
   set_nframes(n_frames);
}

// ---------------------------------------------------------------------
void AnimationController::specify_extremal_frame_world_times(
   int start_framenumber,int stop_framenumber,
   double start_elapsed_secs_since_epoch,
   double stop_elapsed_secs_since_epoch)
{
//   cout << "inside AnimationController::specify_extremal_frame_world_times()"
//        << endl;
   
   vector<twovector> time_counter_samples;
   time_counter_samples.push_back(
      twovector(start_elapsed_secs_since_epoch,start_framenumber));
   time_counter_samples.push_back(
      twovector(stop_elapsed_secs_since_epoch,stop_framenumber)); 
   correlate_frames_to_world_times(time_counter_samples);
}

// ---------------------------------------------------------------------
void AnimationController::correlate_frames_to_world_times(
   const vector<twovector>& time_counter_samples)
{
//   cout << "inside AnimationController::correlate_frames_to_world_times()"
//        << endl;
//   cout << "time_counter_samples.size() = " << time_counter_samples.size() 
//        << endl;

   if (time_counter_samples.size()==0) return;

// Note added on Mon, Feb 11, 2008 at 12 noon:

// For reasons which we have not yet tracked down, VIDEOCITIES does
// not display the Activity Region #1 video if first_framenumber != 0.
// So as a temporary cluge, we'll store the first frame's actual
// number within frame_counter_offset.  We can then reset
// first_framenumber to 0.  We will later come back and trace through
// the origin of this bug...

//   cout.precision(15);
//   cout << "time_counter_samples.size() = " << time_counter_samples.size()
//        << endl;
//   cout << "frame_counter_offset = " << get_frame_counter_offset() << endl;
//   cout << "time_counter_samples.front() = " << time_counter_samples.front()
//        << endl;
//   cout << "time_counter_samples.back() = " << time_counter_samples.back()
//        << endl;

// As of July 2010, frame counter offset generally doesn't equal 0
// within Blue Force Tracker application.  Do NOT modify frame counter
// offset, first or last frame numbers for Blue Force Tracker...

   if (get_frame_counter_offset()==0)
   {
      set_frame_counter_offset(time_counter_samples.front().get(1));
      first_framenumber=time_counter_samples.front().get(1)-
         frame_counter_offset;
      set_last_framenumber(time_counter_samples.back().get(1)
      -frame_counter_offset);
   }
   set_nframes(get_last_framenumber()-first_framenumber+1);
   set_curr_framenumber(get_first_framenumber());

//   cout << "frame_counter_offset = " << frame_counter_offset << endl;
//   cout << "first_framenumber = " << get_first_framenumber()
//        << " last_framenumber = " << get_last_framenumber()
//        << " nframes = " << get_nframes() << endl;

   vector<twovector> counter_time_samples;
   for (unsigned int i=0; i<time_counter_samples.size(); i++)
   {
      twovector curr_time_counter_sample=time_counter_samples.at(i);
      counter_time_samples.push_back(
         twovector(curr_time_counter_sample.get(1),
                   curr_time_counter_sample.get(0)));
//      cout.precision(12);
//      cout << "i = " << i
//           << " time = " << curr_time_counter_sample.get(0)
//           << " frame = " << curr_time_counter_sample.get(1)
//           << endl;
   }

   indep_time_depend_counter_ptr=new piecewise_linear(time_counter_samples);
   indep_counter_depend_time_ptr=new piecewise_linear(counter_time_samples);
}

// ---------------------------------------------------------------------
// Member function get_time_corresponding_to_frame takes in some frame
// number.  It returns a piecewise linear interpolated world-time
// corresponding to the input counter value.  

double AnimationController::get_time_corresponding_to_curr_frame()
{
   return get_time_corresponding_to_frame(get_curr_framenumber());
}

double AnimationController::get_time_corresponding_to_frame(int framenumber)
{
//   cout << "inside AnimationController::get_time_corresponding_to_frame()"
//        << endl;
//   cout << "get_curr_framenumber() = " << get_curr_framenumber() << endl;
//   cout << "input framenumber = " << framenumber << endl;

   double elapsed_secs=NEGATIVEINFINITY;
   if (indep_counter_depend_time_ptr != NULL)
   {
      elapsed_secs=indep_counter_depend_time_ptr->value(framenumber)
         +time_offset;
//      cout << "elapsed_secs = " << elapsed_secs
//           << " time_offset = " << time_offset 
//           << " frame_counter_offset = " << frame_counter_offset
//           << endl;
      clock_ptr->convert_elapsed_secs_to_date(elapsed_secs);

//      cout << "world_time = " << get_world_time_string() << endl;
//      cout << "input frame = " << framenumber
//           << " world time = " << get_world_time_string() << endl;
   }

//   cout << "elapsed_secs = " << elapsed_secs << endl;
   return elapsed_secs;
}

// ---------------------------------------------------------------------
// Member function get_frame_corresponding_to_time() takes in some
// time measured in seconds.  If indep_time_depend_counter_ptr !=
// NULL, this method returns a piecewise linear interpolated frame
// number to the input world time. 

int AnimationController::get_frame_corresponding_to_time(double t)
{
//   cout << "inside AnimationController::get_frame_corresponding_to_time(), t = "
//        << t << endl;
   
   int framenumber=-1;
   if (indep_time_depend_counter_ptr != NULL)
   {
      framenumber=indep_time_depend_counter_ptr->value(t)+
         frame_counter_offset;
//      cout << "inside AC::get_frame_corresponding_to_time()" << endl;

//      clock_ptr->convert_elapsed_secs_to_date(t);
//      cout << "world_time = " << get_world_time_string() 
//           << " frame = " << framenumber << endl;
//      cout << "Check: get_time_corresponding_to_frame(frame) = "
//           << get_time_corresponding_to_frame(framenumber) << endl;
//      cout << "............................................." << endl;
   }
   
   return framenumber;
}

// ---------------------------------------------------------------------
// Member function get_frame_corresponding_to_elapsed_secs() returns a
// quantized frame number corresponding to the member clock's elapsed
// number of seconds since initial truetime:

int AnimationController::get_frame_corresponding_to_elapsed_secs()
{
   int framenumber=clock_ptr->secs_elapsed_since_initial_truetime()/
      delta_world_time_per_frame+frame_counter_offset;
   return framenumber;
}

// ========================================================================
// Frame manipulation member functions
// ========================================================================

bool AnimationController::time_to_display_new_frame()
{
   m_currTime = timefunc::elapsed_timeofday_time();
//   cout << "inside AnimationController::time_to_display_new_frame()" << endl;
//   cout << "this = " << this << endl;
//   cout << "currtime = " << m_currTime
//             << " prevTime = " << m_prevTime
//             << " delta = " << m_currTime-m_prevTime 
//        << " delay = " << m_delay
//        << " extended delay = " << m_extended_delay
//        << endl;

   if (m_prevTime > m_currTime)
   {
      cout << "ERROR in AnimationController::time_to_display_new_frame()" 
           << endl;
      cout << "prevTime = " << m_prevTime 
           << " currTime = " << m_currTime << endl;
      outputfunc::enter_continue_char();
   }
   
// As of 12/31/2011, we allow for the AnimationController to linger on
// both its very first and very last frames.  This helps avoid
// confusion with the time dicontinuity associated with jumping from
// the last frame back to the first frame in a looping movie...

   bool display_new_frame_flag=(m_currTime-m_prevTime > m_delay);
   if (curr_framenumber==0 || curr_framenumber==n_frames-1)
   {
      display_new_frame_flag=(m_currTime-m_prevTime > m_extended_delay);
   }

   if (display_new_frame_flag) m_prevTime=m_currTime;
   return display_new_frame_flag;
}

// ---------------------------------------------------------------------
int AnimationController::increment_time()
{
   cout << "inside AnimationController::increment_time(), curr_framenumber = " 
        << curr_framenumber << endl;

   curr_framenumber=get_frame_corresponding_to_elapsed_secs();
   cout << "curr_framenumber = " << curr_framenumber << endl;
   outputfunc::enter_continue_char();
   return curr_framenumber;
}

// ---------------------------------------------------------------------
int AnimationController::increment_frame_counter()
{
//   cout << "inside AnimationController::increment_frame_counter(), curr_framenumber = " 
//        << curr_framenumber << endl;

   int new_framenumber=curr_framenumber+frame_skip;
   cumulative_framecounter++;
   if (new_framenumber > get_last_framenumber()) 
   {
      new_framenumber=first_framenumber;
      loop_to_beginning=true;
//       cout << "AnimationController looping to first frame number" << endl;
   }
   else
   {
      loop_to_beginning=false;
   }

   set_curr_framenumber(new_framenumber);
   return curr_framenumber;
}

// ---------------------------------------------------------------------
int AnimationController::decrement_frame_counter()
{

   int new_framenumber=curr_framenumber-frame_skip;
   cumulative_framecounter++;
   if (new_framenumber < first_framenumber)
   {
      new_framenumber=get_last_framenumber();
      loop_to_end=true;
//      cout << "AnimationController looping to last frame number" << endl;
   }
   else
   {
      loop_to_end=false;
   }
   set_curr_framenumber(new_framenumber);
   return curr_framenumber;
}

// ---------------------------------------------------------------------
// Member function sync_to_master synchronizes the local
// AnimationController's time to that of the master
// AnimationController.  It then resets the local
// AnimationController's frame number to correspond to the master
// time.

void AnimationController::sync_to_master()
{
//   cout << "inside AC::sync_to_master()" << endl;
   
   if (master_AnimationController_ptr==NULL) return;

//   cout << "this = " << this << endl;
//   cout << "master_AC_ptr = " << master_AnimationController_ptr << endl;
   
   double curr_master_time=master_AnimationController_ptr->
      get_time_corresponding_to_curr_frame();
   int curr_local_frame=get_frame_corresponding_to_time(curr_master_time);
   set_curr_framenumber(curr_local_frame);

//   cout << "curr_master_time = " << curr_master_time << endl;
//   cout << "local framenumber = " << curr_local_frame << endl;
}

// ========================================================================
// Graphicals registration member functions
// ========================================================================

void AnimationController::register_GraphicalsGroup(GraphicalsGroup* GG_ptr)
{
   GraphicalsGroupPtrs.push_back(GG_ptr);
}

bool AnimationController::unregister_GraphicalsGroup(GraphicalsGroup* GG_ptr)
{
//   cout << "inside AnimationController::unregister_GraphicalsGroup()" << endl;
//   cout << "GG_ptr->name = " << GG_ptr->get_name() << endl;

   vector<GraphicalsGroup*> GraphicalGroup_ptrs_to_unregister;
   for (vector<GraphicalsGroup*>::iterator iter=GraphicalsGroupPtrs.begin();
        iter != GraphicalsGroupPtrs.end(); iter++)
   {
      if (*iter==GG_ptr)
      {
         GraphicalsGroupPtrs.erase(iter);
         return true;
      }
   }
   return false;
}

// ========================================================================
// Animation member functions
// ========================================================================

void AnimationController::update()
{
//   cout << "inside AnimationController::update()" << endl;
   switch (getState())
   {
      case AnimationController::PAUSE :
         break;

      case AnimationController::PLAY :
         if (time_to_display_new_frame())
         {
            if (!increment_time_rather_than_frame_number_flag)
            {
               increment_frame_counter();
            }
            update_all_Graphicals_animation();
         }
         break;

      case AnimationController::REVERSE :
         if (time_to_display_new_frame())
         {
            decrement_frame_counter();
            update_all_Graphicals_animation();
         }
         break;

      case AnimationController::INCREMENT_FRAME :
         {
            increment_frame_counter();
            update_all_Graphicals_animation();
            setState(AnimationController::PAUSE);
         }
         break;
         
      case AnimationController::DECREMENT_FRAME :
         {
            decrement_frame_counter();
            update_all_Graphicals_animation();
            setState(AnimationController::PAUSE);
         }
         break;

      case AnimationController::JUMP_TO_FRAME :
         update_all_Graphicals_animation();
         setState(AnimationController::PAUSE);
         break;

      case AnimationController::JUMP_FORWARD :
         {
            int frames_to_jump=trunclog(get_nframes());
//            cout << "frames_to_jump = " << frames_to_jump << endl;
            int next_framenumber=(get_curr_framenumber()/frames_to_jump+1)
               *frames_to_jump;
            if (next_framenumber > get_nframes()) next_framenumber=0;
//            cout << "next_framenumber = " << next_framenumber << endl;
            set_curr_framenumber(next_framenumber);
            setState(prev_state);
//            setState(AnimationController::PAUSE);
         }
         break;

      case AnimationController::JUMP_BACKWARD :
         {
            int frames_to_jump=trunclog(get_nframes());
            int next_framenumber;
            if (get_curr_framenumber()==0)
            {
               next_framenumber=get_nframes()-1;
            }
            else if (get_curr_framenumber()%frames_to_jump==0)
            {
               next_framenumber=get_curr_framenumber()-frames_to_jump;
            }
            else
            {
               next_framenumber=(get_curr_framenumber()/frames_to_jump)
                  *frames_to_jump;
            }
            if (next_framenumber < 0) next_framenumber=get_nframes()-1;

            set_curr_framenumber(next_framenumber);
            setState(prev_state);
//            setState(AnimationController::PAUSE);
         }
         break;

      default:
         break;
   }
}

// ---------------------------------------------------------------------
void AnimationController::update_all_Graphicals_animation()
{
//   cout << "inside AnimationController::update_all_Graphicals_animation()" 
//        << endl;
//   cout << "curr_framenumber = " << curr_framenumber << endl;
   
   for (unsigned int i=0; i<GraphicalsGroupPtrs.size(); i++)
   {
//      cout << "i = " << i
//           << " GraphicalsGroupPtrs[i] = "
//           << GraphicalsGroupPtrs[i] << endl;
//      cout << "GraphicalsGroupPtrs[i]->get_name() = "
//           << GraphicalsGroupPtrs[i]->get_name() << endl;
      GraphicalsGroupPtrs[i]->update_display();
   }
}

// ---------------------------------------------------------------------
// Member function broadcast_curr_frame_and_UTC_time()

void AnimationController::broadcast_curr_frame_and_UTC_time()
{
//   cout << "inside AnimationController::broadcast_curr_frame_and_UTC_time()" 
//	  << endl;

   if (messenger_ptr==NULL) return;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   vector<Messenger::Property> properties;

   command="SEND_CURRENT_FRAME_AND_TIME";	

   key="Frame Number";
   value=stringfunc::number_to_string(curr_framenumber);
   properties.push_back(Messenger::Property(key,value));

   key="Time";
//   value=stringfunc::number_to_string(
//      get_clock_ptr()->secs_elapsed_since_reference_date());
   value=stringfunc::number_to_string(
      get_time_corresponding_to_curr_frame());
   properties.push_back(Messenger::Property(key,value));
 
   messenger_ptr->broadcast_subpacket(command,properties);
}

// ---------------------------------------------------------------------
// Member function get_world_time_string() returns YYYY_MM_DD_H_M_S
// either in UTC or local time depending upon input bool display_UTC_flag.

string AnimationController::get_world_time_string(bool display_UTC_flag) const
{
   string world_time_string;
   if (display_UTC_flag)
   {
//      world_time_string=clock_ptr->YYYY_MM_DD_H_M_S()+" UTC";
      world_time_string=clock_ptr->YYYY_MM_DD_H_M_S();
   }
   else
   {
//      const int n_secs_digits=1;
      const int n_secs_digits=2;
      world_time_string=clock_ptr->YYYY_MM_DD_H_M_S(
         " ",":",false,n_secs_digits);
//         " ",":",false,n_secs_digits)+" local";
   }
   return world_time_string;
}

// ==========================================================================
// Ordered image storage and retrieveal member functions
// ==========================================================================

// Member function store_unordered_image_filenames() takes in a
// subdirectory which is assumed to hold a some quasi-random set of
// image files.  This method fills STL map member image_numbers_map
// with imported image filename as a function of frame number.  It
// fills STL map member image_filenames_map with frame number as as
// function of image filename.

void AnimationController::store_unordered_image_filenames(string subdir)
{
   cout << "inside AnimationController::store_unordered_image_filenames()" 
        << endl;

   vector<string> image_filenames;
   image_filenames=filefunc::image_files_in_subdir(subdir);
//   cout << "image_filenames.size() = " << image_filenames.size() << endl;

   image_numbers_map.clear();
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      image_numbers_map[i]=image_filenames[i];
      image_filenames_map[image_filenames[i]] = i;
   }

   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function store_image_filenames_ordered_by_width() takes in a
// subdirectory which is assumed to hold a some quasi-random set of
// image files.  The input images are sorted according to their pixel
// widths.  This method then fills STL map member
// ordered_image_filenames with the sorted images' file names.

void AnimationController::store_image_filenames_ordered_by_width(string subdir)
{
//   cout << "inside AnimationController::store_image_filenames_ordered_by_width()" 
//        << endl;
//   cout << "subdir = " << subdir << endl;

   vector<string> image_filenames;
   image_filenames=filefunc::image_files_in_subdir(subdir);

   vector<unsigned int> image_widths;
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      unsigned int curr_width, curr_height;
      imagefunc::get_image_width_height(
         image_filenames[i], curr_width, curr_height);
      image_widths.push_back(curr_width);
   }
   
   templatefunc::Quicksort(image_widths, image_filenames);

   image_numbers_map.clear();
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      image_numbers_map[i]=image_filenames[i];
//      cout << "i = " << i << " ordered filename = "
//           << image_filenames[i] << endl;
   }
}

// ---------------------------------------------------------------------
// Member function store_ordered_image_filenames() takes in a
// subdirectory which is assumed to hold a temporally ordered sequence
// of image files. The images are assumed to have names of the forms
// XXXX-NNNNN.jpg or XXXX_NNNNN.jpg where XXXX may contain any number
// of dashes or underscores.  Alternatively, this method searches for
// image filenames of the form written out by Ross Anderson's
// low-definition screen capture program for the FLIR EO/IR camera
// operating on LL's Twin Otter.  This method fills STL map member
// ordered_image_filenames with the true image numbers NNN and the
// corresponding image file name.

void AnimationController::store_ordered_image_filenames(string subdir)
{
//   cout << "inside AnimationController::store_ordered_image_filenames()" 
//        << endl;
//   cout << "subdir = " << subdir << endl;

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   allowed_suffixes.push_back("JPG");
   allowed_suffixes.push_back("jpeg");
   allowed_suffixes.push_back("JPEG");
   allowed_suffixes.push_back("png");
   allowed_suffixes.push_back("PNG");
   allowed_suffixes.push_back("rgb");
   allowed_suffixes.push_back("tif");

   vector<string> image_filenames;
   image_filenames=filefunc::files_in_subdir_matching_specified_suffixes(
      allowed_suffixes,subdir);
//   cout << "image_filenames.size() = " << image_filenames.size() << endl;

   image_numbers_map.clear();

   string separator_chars="-_.";
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      string base_filename=filefunc::getbasename(image_filenames[i]);
//      cout << "i = " << i << " base_filename = " << base_filename << endl;

      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(
            base_filename,separator_chars);

// As of January 2011, we assume true imagenumber occurs within
// next-to-last substring.  If this substring is NOT numeric, search
// for genuinely numeric substring in next-to-next-to-last substring, 
// next-to-next-to-next-to-last substring, etc:

// Recall Ross Anderson's program running onboard FLIR generates
// low-definition JPEG captures which have names like
// 20110511_231742.184.jpg . As of October 2011, we specifically
// search for FLIR file names of this form:

      if (substrings.size()==4 && stringfunc::is_number(substrings[0]) 
      && stringfunc::is_number(substrings[1]) 
      && stringfunc::is_number(substrings[2]) && substrings[3]=="jpg")
      {
//         cout << "FLIR image detected" << endl;
         continue;
      }
      
      string curr_substring;
      for (unsigned int s=substrings.size()-2; s>=0; s--)
      {
         curr_substring=substrings[s];
//         cout << "s = " << s << " curr_substring = " << curr_substring << endl;
         if (stringfunc::is_number(curr_substring)) break;
      }
      int true_imagenumber=stringfunc::string_to_number(curr_substring);

      if (i==0) set_frame_counter_offset(true_imagenumber);

      image_numbers_map[true_imagenumber]=image_filenames[i];

//      cout << "i = " << i
//           << " true_imagenumber = " << true_imagenumber
//           << " image_filename = " << image_filenames[i] << endl;
   } // loop over index i labeling image filenames

// If images within input subdirectory do NOT have filenames of the form
// XXXX-NNNN.jpg or XXXX_NNNN.jpg, we assume that they are still
// temporally ordered.  In this case, assign frame number to equal
// order value:

//   cout << "get_frame_counter_offset() = "
//        << get_frame_counter_offset() << endl;
//   cout << "n_ordered_image_filenames = " << get_n_ordered_image_filenames()
//        << endl;

   if (get_n_ordered_image_filenames() <= 1)
   {
      for (unsigned int i=0; i<image_filenames.size(); i++)
      {
         image_numbers_map[i]=image_filenames[i];
      }
   }

//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function get_ordered_image_filename() searches STL map
// member image_numbers_map for a filename corresponding to
// the input framenumber.  If one exists, this method returns
// the retrieved filename.  Otherwise, it returns an empty string.

string AnimationController::get_ordered_image_filename(int frame_number)
{
//   cout << "inside AnimationController::get_ordered_image_filename()" 
//        << endl;
//   cout << "frame_number = " << frame_number << endl;

   IMAGE_NUMBERS_MAP::iterator iter=image_numbers_map.find(frame_number);
   if (iter==image_numbers_map.end()) 
   {
//      cout << "ordered image filename not found" << endl;
//      outputfunc::enter_continue_char();
      return "";
   }
   else
   {
      string ordered_image_filename=iter->second;
      return ordered_image_filename;
   }
}

// ---------------------------------------------------------------------
int AnimationController::get_image_framenumber(string image_filename)
{

   IMAGE_FILENAMES_MAP::iterator iter=image_filenames_map.find(image_filename);
   if (iter==image_filenames_map.end()) 
   {
      return -1;
   }
   else
   {
      return iter->second;
   }
}

// ---------------------------------------------------------------------
// Member function get_n_ordered_image_filenames() 

int AnimationController::get_n_ordered_image_filenames() const
{
//   cout << "inside AnimationController::get_n_ordered_image_filenames()" 
//        << endl;

   return image_numbers_map.size();
}
