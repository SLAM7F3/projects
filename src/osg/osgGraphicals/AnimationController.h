// ==========================================================================
// AnimationController header file 
// ==========================================================================
// Last modified on 12/31/11; 10/3/12; 1/22/16; 9/9/16
// ==========================================================================

#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osg/Group>
#include "math/basic_math.h"
#include "astro_geo/Clock.h"
#include "messenger/Messenger.h"
#include "math/twovector.h"

class GraphicalsGroup;
class piecewise_linear;

class AnimationController 
{
  public:

   enum eState {PAUSE=0, PLAY=1, REVERSE=2, INCREMENT_FRAME=3,
                DECREMENT_FRAME=4, JUMP_TO_FRAME=5, JUMP_FORWARD=6, 
                JUMP_BACKWARD=7};

   AnimationController();
   AnimationController(int num_frames);
   AnimationController(const std::vector<twovector>& time_counter_samples);
   virtual ~AnimationController();
   friend std::ostream& operator<< (
      std::ostream& outstream,const AnimationController& AC);

// Set & get methods:

   void set_increment_time_rather_than_frame_number_flag(bool flag);
   bool get_increment_time_rather_than_frame_number_flag() const;
   void set_loop_to_beginning(bool flag);
   bool get_loop_to_beginning() const;
   void set_loop_to_end(bool flag);
   bool get_loop_to_end() const;
   void set_AVI_movie_generation_flag(bool flag);
   bool get_AVI_movie_generation_flag() const;
   
   void setState(eState p_state);
   eState getState() const;
   const eState* getStatePtr() const;

   void setDelay(double delay);
   void setExtendedDelay(double e_delay);
   double getDelay() const;
   void increment_delay();
   void decrement_delay();

   void set_first_framenumber(int frame);
   int get_first_framenumber() const;
   void set_last_framenumber(int frame);
   int get_last_framenumber() const;
   void set_curr_framenumber(int frame);
   int get_curr_framenumber() const;
   void set_prev_framenumber(int framenumber);
   int get_prev_framenumber() const;
   bool curr_framenumber_equals_prev_framenumber() const;
   void set_nframes(int n);
   int get_nframes() const;

   void set_frame_skip(int skip);
   int get_frame_skip() const;
   int increment_frame_skip();
   int decrement_frame_skip();
   void set_cumulative_framecounter(int count);
   int get_cumulative_framecounter() const;

   Clock* get_clock_ptr();
   const Clock* get_clock_ptr() const;
   osg::Group* get_OSGgroup_ptr();

   double get_delta_world_time_per_frame() const;
   void set_master_AnimationController_ptr(AnimationController* MAC_ptr);
   void set_Messenger_ptr(Messenger* m_ptr);

// Frame and world time correlation member functions:

   void set_world_time_params(
      double start_secs_since_epoch,double stop_secs_since_epoch,
      double delta_time_step_per_frame);
   void specify_extremal_frame_world_times(
      int start_framenumber,int stop_framenumber,
      double start_elapsed_secs_since_epoch,
      double stop_elapsed_secs_since_epoch);
   void correlate_frames_to_world_times(
      const std::vector<twovector>& time_counter_samples);
   void set_time_offset(double t_offset);
   void set_frame_counter_offset(int counter_offset);
   int get_frame_counter_offset() const;
   int get_true_framenumber() const;

   double get_time_corresponding_to_curr_frame();
   double get_time_corresponding_to_frame(int framenumber);
   int get_frame_corresponding_to_time(double t);
   int get_frame_corresponding_to_elapsed_secs();

// Frame manipulation member functions

   bool time_to_display_new_frame();
   int increment_time();
   int increment_frame_counter();
   int decrement_frame_counter();
   void sync_to_master();

// Graphicals registration member functions

   void register_GraphicalsGroup(GraphicalsGroup* GG_ptr);
   bool unregister_GraphicalsGroup(GraphicalsGroup* GG_ptr);

// Animation member functions:

   void update();
   void update_all_Graphicals_animation();
   void broadcast_curr_frame_and_UTC_time();
   std::string get_world_time_string(bool display_UTC_flag=true) const;

// Ordered image storage and retrieval member functions:

   typedef std::map<int,std::string > IMAGE_NUMBERS_MAP;
// Independent int = frame number
// Dependent string = image filename

   IMAGE_NUMBERS_MAP image_numbers_map;

   typedef std::map<std::string, int > IMAGE_FILENAMES_MAP;
// Indepedent string = image filename
// Dependent int = frame number

   IMAGE_FILENAMES_MAP image_filenames_map;

   void store_unordered_image_filenames(std::string subdir);
   void store_image_filenames_ordered_by_width(std::string subdir);
   void store_ordered_image_filenames(std::string subdir);
   std::string get_ordered_image_filename(int frame_number);
   int get_image_framenumber(std::string image_filename);

   std::string get_curr_image_filename();
   std::string get_next_ordered_image_filename();
   int get_n_ordered_image_filenames() const;

  protected:
   
   bool loop_to_beginning,loop_to_end;
   bool AVI_movie_generation_flag;
   eState curr_state,prev_state;
   int first_framenumber,curr_framenumber,prev_framenumber,n_frames;
   int frame_skip,cumulative_framecounter;
   double m_delay,m_extended_delay,m_currTime,m_prevTime;

  private:

   bool increment_time_rather_than_frame_number_flag;
   std::vector<GraphicalsGroup*> GraphicalsGroupPtrs;

   double delta_world_time_per_frame;
   Clock* clock_ptr;
   osg::ref_ptr<osg::Group> OSGgroup_refptr;
   AnimationController* master_AnimationController_ptr;
   Messenger* messenger_ptr;

   int frame_counter_offset;
   double time_offset;
   piecewise_linear* indep_time_depend_counter_ptr;
   piecewise_linear* indep_counter_depend_time_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void 
AnimationController::set_increment_time_rather_than_frame_number_flag(
   bool flag)
{
   increment_time_rather_than_frame_number_flag=flag;
}

inline bool
AnimationController::get_increment_time_rather_than_frame_number_flag() const
{
   return increment_time_rather_than_frame_number_flag;
}

inline void AnimationController::set_loop_to_beginning(bool flag)
{
   loop_to_beginning=flag;
}

inline bool AnimationController::get_loop_to_beginning() const
{
   return loop_to_beginning;
}

inline void AnimationController::set_loop_to_end(bool flag)
{
   loop_to_end=flag;
}

inline bool AnimationController::get_loop_to_end() const
{
   return loop_to_end;
}

inline void AnimationController::set_AVI_movie_generation_flag(bool flag)
{
   AVI_movie_generation_flag=flag;
}

inline bool AnimationController::get_AVI_movie_generation_flag() const
{
   return AVI_movie_generation_flag;
}

inline void AnimationController::set_cumulative_framecounter(int count)
{
   cumulative_framecounter=count;
}

inline int AnimationController::get_cumulative_framecounter() const
{
   return cumulative_framecounter;
}

inline osg::Group* AnimationController::get_OSGgroup_ptr()
{
   return OSGgroup_refptr.get();
}

inline void AnimationController::set_time_offset(double t_offset)
{
   time_offset=t_offset;
}

inline void AnimationController::set_frame_counter_offset(int counter_offset)
{
   frame_counter_offset=counter_offset;
}

inline int AnimationController::get_frame_counter_offset() const
{
   return frame_counter_offset;
}

inline Clock* AnimationController::get_clock_ptr()
{
   return clock_ptr;
}

inline const Clock* AnimationController::get_clock_ptr() const 
{
   return clock_ptr;
}

inline double AnimationController::get_delta_world_time_per_frame() const
{
   return delta_world_time_per_frame;
}

inline void AnimationController::set_master_AnimationController_ptr(
   AnimationController* MAC_ptr)
{
   master_AnimationController_ptr=MAC_ptr;
}

inline int AnimationController::get_true_framenumber() const
{
   return get_curr_framenumber()+get_frame_counter_offset();
}

inline void AnimationController::set_Messenger_ptr(Messenger* m_ptr)
{
   messenger_ptr=m_ptr;
}

// ---------------------------------------------------------------------
inline std::string AnimationController::get_curr_image_filename()
{
//   cout << "inside AnimationController::get_curr_image_filename()" 
//        << endl;

   return get_ordered_image_filename(get_curr_framenumber());
}

// ---------------------------------------------------------------------
// Member function get_next_ordered_image_filename() searches STL map
// member ordered_image_filenames_map for a filename corresponding to
// the current true framenumber.  If one exists, this method returns
// the retrieved filename.  Otherwise, it returns an empty string.

inline std::string AnimationController::get_next_ordered_image_filename()
{
//   cout << "inside AnimationController::get_next_ordered_image_filename()" 
//        << endl;
//   cout << "true framenumber = " << get_true_framenumber() << endl;

   return get_ordered_image_filename(get_true_framenumber());
}


#endif 
