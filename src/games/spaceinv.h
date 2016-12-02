// ==========================================================================
// Header file for spaceinv class 
// ==========================================================================
// Last modified on 12/1/16; 12/2/16
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
   friend std::ostream& operator<< 
      (std::ostream& outstream,const spaceinv& C);

   int get_min_episode_framenumber() const;
   double get_max_score_per_episode() const;
   int get_frame_skip() const;
   ALEInterface& get_ale();
   const ALEInterface& get_ale() const;

//   genvector* get_curr_legal_actions();
   genvector* get_curr_state();
   const genvector* get_curr_state() const;
//   double get_reward() const;

   void crop_pool_difference_curr_frame(bool export_frames_flag);


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

   genvector *curr_state;

   void allocate_member_objects();
   void initialize_member_objects();
   void initialize_output_subdirs();
   void initialize_grayscale_output_buffer();
   void docopy(const spaceinv& S);
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
   return curr_state;
}

inline const genvector* spaceinv::get_curr_state() const
{
   return curr_state;
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


