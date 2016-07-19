// ==========================================================================
// Header file for OPERATIONS class
// ==========================================================================
// Last modified on 10/8/11; 4/3/13; 4/2/14; 7/19/16
// ==========================================================================

// Need to swap order of hide_Mode_HUG_flag and display_movie_nframes
// arguments to Operations constructors !!!


#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <osg/Group>
#include "osg/osgGraphicals/AnimationController.h"
#include "astro_geo/Clock.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "osg/ModeController.h"
#include "osg/ModeHUD.h"
#include "osg/osgWindow/WindowManager.h"

class AnimationKeyHandler;
class ModeKeyHandler;
class PassesGroup;

class Operations 
{

  public:

// Initialization, constructor and destructor functions:

   Operations(
      int ndims,WindowManager* window_mgr_ptr,bool display_movie_state=false,
      bool display_movie_number=false,bool display_movie_world_time=false,
      bool display_movie_elapsed_time=false,bool hide_Mode_HUD_flag=false,
      bool display_movie_nframes=false);
   Operations(
      int ndims,WindowManager* window_mgr_ptr,const PassesGroup& passes_group,
      bool display_movie_state=false,bool display_movie_number=false,
      bool display_movie_world_time=false,
      bool display_movie_elapsed_time=false,bool hide_Mode_HUD_flag=false,
      bool display_movie_nframes=false);
   virtual ~Operations();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Operations& O);

// Set & get methods:

   osg::Group* get_OSGgroup_ptr();
   const osg::Group* get_OSGgroup_ptr() const;
   void set_initial_mode(const PassesGroup& passes_group);

   osg::Node* get_root_ptr();
   AnimationController* get_AnimationController_ptr();
   ImageNumberHUD* get_ImageNumberHUD_ptr();
   ModeController* get_ModeController_ptr();
   ModeHUD* get_ModeHUD_ptr();
   Clock* get_Clock_ptr();
   const Clock* get_Clock_ptr() const;
   WindowManager* get_WindowManager_ptr();
   const WindowManager* get_WindowManager_ptr() const;

// Master clock member functions:

   void set_master_world_start_UTC(std::string world_start_UTC_string);
   void set_master_world_stop_UTC(std::string world_start_UTC_string);
   void set_master_world_UTC(
      int year,int month,int day,int hour,int minute,double secs,
      bool start_time_flag);
   void set_master_world_UTC(double elapsed_epoch_secs,bool start_time_flag);

   void set_master_world_start_time(double t);
   double get_master_world_start_time() const;
   void set_master_world_stop_time(double t);
   double get_master_world_stop_time() const;
   void set_delta_master_world_time_step_per_master_frame(double t);
   double get_delta_master_world_time_step_per_master_frame() const;

   void set_current_master_clock_time_duration_and_step(
      int n_simulation_hours,int n_simulation_mins,
      double world_time_step_in_secs_per_frame);
   void set_current_master_clock_time_duration_and_step(
      double n_simulation_hours,double world_time_step_in_secs_per_frame);
   void set_current_master_clock_time_duration(double n_simulation_hours);

   void reset_AnimationController_world_time_params();

  protected:

  private:

   bool display_movie_number,display_movie_state,display_movie_world_time;
   bool display_movie_elapsed_time, display_movie_nframes;
   bool hide_Mode_HUD_flag;
   int ndims;
   AnimationController* AnimationController_ptr;
   AnimationKeyHandler* AnimationKeyHandler_ptr;
   ImageNumberHUD* ImageNumberHUD_ptr;
   ModeController* ModeController_ptr;
   ModeHUD* ModeHUD_ptr;
   ModeKeyHandler* ModeKeyHandler_ptr;
   WindowManager* window_mgr_ptr;

   osg::ref_ptr<osg::Group> OSGgroup_refptr;

// Master game clock time parameters:

   double master_world_start_time,master_world_stop_time;
   double delta_master_world_time_step_per_master_frame;

   void allocate_member_objects();
   void initialize_member_objects();
//    void docopy(const Operations& D);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline osg::Group* Operations::get_OSGgroup_ptr() 
{
   return OSGgroup_refptr.get();
}

inline const osg::Group* Operations::get_OSGgroup_ptr() const
{
   return OSGgroup_refptr.get();
}

// --------------------------------------------------------------------------
inline AnimationController* Operations::get_AnimationController_ptr()
{
   return AnimationController_ptr;
}

inline ImageNumberHUD* Operations::get_ImageNumberHUD_ptr()
{
   return ImageNumberHUD_ptr;
}

inline ModeHUD* Operations::get_ModeHUD_ptr()
{
   return ModeHUD_ptr;
}

inline ModeController* Operations::get_ModeController_ptr()
{
   return ModeController_ptr;
}

inline Clock* Operations::get_Clock_ptr()
{
   return AnimationController_ptr->get_clock_ptr();
}

inline const Clock* Operations::get_Clock_ptr() const
{
   return AnimationController_ptr->get_clock_ptr();
}

// --------------------------------------------------------------------------
inline void Operations::set_master_world_start_time(double t)
{
   master_world_start_time=t;
}

inline double Operations::get_master_world_start_time() const
{
   return master_world_start_time;
}

inline void Operations::set_master_world_stop_time(double t)
{
   master_world_stop_time=t;
}

inline double Operations::get_master_world_stop_time() const
{
   return master_world_stop_time;
}

inline void Operations::set_delta_master_world_time_step_per_master_frame(
   double t)
{
   delta_master_world_time_step_per_master_frame=t;
}

inline double Operations::get_delta_master_world_time_step_per_master_frame() const
{
   return delta_master_world_time_step_per_master_frame;
}

// --------------------------------------------------------------------------
inline WindowManager* Operations::get_WindowManager_ptr()
{
   return window_mgr_ptr;
}

inline const WindowManager* Operations::get_WindowManager_ptr() const
{
   return window_mgr_ptr;
}

#endif // Operations.h



