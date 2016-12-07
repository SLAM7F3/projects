// ==========================================================================
// Header file for spaceinv class 
// ==========================================================================
// Last modified on 12/2/16; 12/3/16; 12/4/16; 12/7/16
// ==========================================================================

#ifndef SPACEINV_H
#define SPACEINV_H

#include <vector>
#include <SDL.h>
#include <ale_interface.hpp>
#include "math/genvector.h"

class spaceinv
{
   
  public:

// Initialization, constructor and destructor functions:

   spaceinv();
   spaceinv(const spaceinv& C);
   ~spaceinv();

   int get_min_episode_framenumber() const;
   double get_max_score_per_episode() const;
   int get_frame_skip() const;
   ALEInterface& get_ale();
   const ALEInterface& get_ale() const;

   genvector* get_curr_state();
   const genvector* get_curr_state() const;
   genvector* get_next_state();
   const genvector* get_next_state() const;
   void crop_pool_difference_curr_frame(bool export_frames_flag);

   int get_screen_state_counter() const;
   std::vector<genvector*>& get_screen_state_ptrs();
   const std::vector<genvector*>& get_screen_state_ptrs() const;
   void crop_pool_curr_frame(bool export_frames_flag);

   void update_curr_big_state();
   genvector* get_curr_big_state();
   const genvector* get_curr_big_state() const;

  private: 

   int random_seed;
   int difference_counter;
   ALEInterface ale;
   int screen_width, screen_height;
   int min_px, max_px, min_py, max_py;
   int n_reduced_pixels;
   int min_episode_framenumber;
   int frame_skip;
   double max_score_per_episode;
   double mu_zdiff, sigma_zdiff;
   std::string output_subdir, orig_subdir, pooled_subdir, differenced_subdir;

   std::vector<unsigned char> grayscale_output_buffer;

   std::vector<std::vector<unsigned char > > pooled_byte_array0;
   std::vector<std::vector<unsigned char > > pooled_byte_array1;
   std::vector<std::vector<unsigned char > >* pooled_byte_array_ptr;
   std::vector<std::vector<unsigned char > >* other_pooled_byte_array_ptr;

   int n_screen_states;
   int screen_state_counter;
   std::vector<genvector*> screen_state_ptrs;
   genvector *curr_big_state_ptr;


   genvector *screen0_state_ptr;
   genvector *screen1_state_ptr;
   genvector *curr_state_ptr, *next_state_ptr;  // just pointers

   void allocate_member_objects();
   void initialize_member_objects();
   void initialize_output_subdirs();
   void initialize_grayscale_output_buffer();
   void docopy(const spaceinv& S);

   void update_big_state(int screen_counter, genvector* big_state_ptr);
   void pingpong_curr_and_next_states();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline int spaceinv::get_min_episode_framenumber() const
{
   return min_episode_framenumber;
}

inline int spaceinv::get_frame_skip() const
{
   return frame_skip;
}

inline double spaceinv::get_max_score_per_episode() const
{
   return max_score_per_episode;
}

inline ALEInterface& spaceinv::get_ale()
{
   return ale;
}

inline const ALEInterface& spaceinv::get_ale() const
{
   return ale;
}

inline genvector* spaceinv::get_curr_state()
{
   return curr_state_ptr;
}

inline const genvector* spaceinv::get_curr_state() const
{
   return curr_state_ptr;
}

inline genvector* spaceinv::get_next_state()
{
   return next_state_ptr;
}

inline const genvector* spaceinv::get_next_state() const
{
   return next_state_ptr;
}

inline int spaceinv::get_screen_state_counter() const
{
   return screen_state_counter;
}


inline std::vector<genvector*>& spaceinv::get_screen_state_ptrs()
{
   return screen_state_ptrs;
}

inline const std::vector<genvector*>& spaceinv::get_screen_state_ptrs() const
{
   return screen_state_ptrs;
}


inline genvector* spaceinv::get_curr_big_state()
{
   return curr_big_state_ptr;
}

inline const genvector* spaceinv::get_curr_big_state() const
{
   return curr_big_state_ptr;
}

//  enum Action {
//  PLAYER_A_NOOP           = 0,
//  PLAYER_A_FIRE           = 1,
//  PLAYER_A_UP             = 2,
//  PLAYER_A_RIGHT          = 3,
//  PLAYER_A_LEFT           = 4,
//  PLAYER_A_DOWN           = 5,
//  PLAYER_A_UPRIGHT        = 6,
//  PLAYER_A_UPLEFT         = 7,
//  PLAYER_A_DOWNRIGHT      = 8,
//  PLAYER_A_DOWNLEFT       = 9,
//  PLAYER_A_UPFIRE         = 10,
//  PLAYER_A_RIGHTFIRE      = 11,
//  PLAYER_A_LEFTFIRE       = 12,
//  PLAYER_A_DOWNFIRE       = 13,
//  PLAYER_A_UPRIGHTFIRE    = 14,
//  PLAYER_A_UPLEFTFIRE     = 15,
//  PLAYER_A_DOWNRIGHTFIRE  = 16,
//  PLAYER_A_DOWNLEFTFIRE   = 17,
//  }

// Space invaders actions: 0, 1, 3, 4, 11, 12


#endif  // spaceinv.h


