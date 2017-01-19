// ==========================================================================
// Header file for pong class 
// ==========================================================================
// Last modified on 12/30/16; 12/31/16; 1/5/17; 1/19/17
// ==========================================================================

#ifndef PONG_H
#define PONG_H

#include <vector>
#include <SDL.h>
#include <ale_interface.hpp>
#include "math/genvector.h"
#include "image/TwoDarray.h"

class pong
{
   
  public:

// Initialization, constructor and destructor functions:

   pong(int n_screen_states);
   pong(const pong& C);
   ~pong();

   void set_forced_game_over(bool flag);
   bool get_forced_game_over() const;

   void set_compute_difference_flag(bool flag);
   bool get_compute_difference_flag() const;
   void set_compute_max_flag(bool flag);
   bool get_compute_max_flag() const;

   int get_n_actions() const;
   int get_n_screen_states() const;
   int get_n_reduced_xdim() const;
   int get_n_reduced_ydim() const;

   int get_min_episode_framenumber() const;
   int get_frame_skip() const;
   ALEInterface& get_ale();
   const ALEInterface& get_ale() const;

   genvector* get_curr_state();
   const genvector* get_curr_state() const;
   genvector* get_next_state();
   const genvector* get_next_state() const;

   int get_min_paddle_y() const;
   int get_max_paddle_y() const;
   int get_default_starting_paddle_y() const;
   int get_center_paddle_y() const;
   void set_paddle_y(int x);
   int get_paddle_y() const;
   bool increment_paddle_y();
   bool decrement_paddle_y();
   void push_back_paddle_y();
   void plot_paddle_y_dist(std::string output_subdir);
   void plot_tracks(std::string output_subdir, int episode_number,
                    double cum_reward);
   void plot_paddle_accel(std::string output_subdir, int episode_number,
                          double cum_reward);
   void get_ball_posn(int& ball_px, int& ball_py);
   double update_tracks();
   void clear_tracks();
   std::vector<double>& get_ball_px_track();
   std::vector<double>& get_ball_py_track();
   std::vector<double>& get_paddle_track();
   double delayed_paddle_accel();

   bool crop_pool_difference_curr_frame(bool export_frames_flag);
   bool crop_pool_sum_curr_frame(bool export_frames_flag);

   int get_screen_state_counter() const;
   std::vector<genvector*>& get_screen_state_ptrs();
   const std::vector<genvector*>& get_screen_state_ptrs() const;
   bool crop_pool_curr_frame(bool export_frames_flag);
   void mu_and_sigma_for_pooled_zvalues();

   genvector* update_curr_big_state();
   genvector* get_curr_big_state();
   const genvector* get_curr_big_state() const;
   void append_wtwoDarray(twoDarray* wtwoDarray_ptr);

   void set_screen_exports_subdir(std::string subdir);
   std::string save_screen(int episode_number, 
                           std::string curr_screen_filename);
   std::string save_screen(int episode_number, 
                           std::string curr_screen_filename,
                           std::string caption);
  private: 

   bool forced_game_over;
   bool compute_difference_flag, compute_max_flag;
   bool paddle_y_values_filled;
   int random_seed;
   int rskip, cskip;
   int prev_framenumber;
   int difference_counter;
   int n_reduced_xdim, n_reduced_ydim;
   ALEInterface ale;
   int screen_width, screen_height;
   int min_px, max_px, min_py, max_py;
   int n_reduced_pixels;
   int min_episode_framenumber;
   int frame_skip;
   int paddle_y;
   int px_ball, py_ball;  // (0,0) corresponds to upper right corner
   double mu_z, sigma_z;
   double mu_zdiff, sigma_zdiff;
   std::string output_subdir, orig_subdir, pooled_subdir;
   std::string maxed_subdir, differenced_subdir;
   std::string screen_exports_subdir;

   std::vector<unsigned char> grayscale_output_buffer;

   std::vector<std::vector<unsigned char > > pooled_byte_array0;
   std::vector<std::vector<unsigned char > > pooled_byte_array1;
   std::vector<std::vector<unsigned char > >* pooled_byte_array_ptr;
   std::vector<std::vector<unsigned char > >* other_pooled_byte_array_ptr;

   int n_screen_states;
   int screen_state_counter;
   std::vector<genvector*> screen_state_ptrs;
   genvector *curr_big_state_ptr;
   std::vector<double> pooled_scrn_values;

   genvector *screen0_state_ptr;
   genvector *screen1_state_ptr;
   genvector *curr_state_ptr, *next_state_ptr;  // just pointers

// wtwoDarray_ptrs holds pointers to twoDarrays which contain
// renormalized trained weight values:

   std::vector<twoDarray*> wtwoDarray_ptrs;

   int d_paddle;
   int max_paddle_y_size;
   std::vector<int> paddle_y_values;
   std::vector<double> ball_px_track, ball_py_track;
   std::vector<double> paddle_track;
   std::vector<double> paddle_accel;

   ScreenExporter* screen_exporter_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void initialize_output_subdirs();
   void initialize_grayscale_output_buffer();
   void docopy(const pong& S);

   void crop_center_ROI(std::vector<std::vector<unsigned char > >& byte_array);
   unsigned char max_pool(
      int r, int c, const std::vector<std::vector<unsigned char > >& 
      byte_array);
   unsigned char avg_pool(
      int r, int c, const std::vector<std::vector<unsigned char > >& 
      byte_array);

   void pingpong_curr_and_next_states();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void pong::set_forced_game_over(bool flag)
{
   forced_game_over = flag;
}

inline bool pong::get_forced_game_over() const
{
   return forced_game_over;
}

inline void pong::set_compute_difference_flag(bool flag)
{
   compute_difference_flag = flag;
}

inline bool pong::get_compute_difference_flag() const
{
   return compute_difference_flag;
}

inline void pong::set_compute_max_flag(bool flag)
{
   compute_max_flag = flag;
}

inline bool pong::get_compute_max_flag() const
{
   return compute_max_flag;
}

inline int pong::get_n_actions() const
{
//   return 2;  // move right, move left
   return 3;  // no_op, move right, move left
}

inline int pong::get_n_screen_states() const
{
   return n_screen_states;
}

inline int pong::get_n_reduced_xdim() const
{
   return n_reduced_xdim;
}

inline int pong::get_n_reduced_ydim() const
{
   return n_reduced_ydim;
}

inline int pong::get_min_episode_framenumber() const
{
   return min_episode_framenumber;
}

inline int pong::get_frame_skip() const
{
   return frame_skip;
}

inline ALEInterface& pong::get_ale()
{
   return ale;
}

inline const ALEInterface& pong::get_ale() const
{
   return ale;
}

inline genvector* pong::get_curr_state()
{
   return curr_state_ptr;
}

inline const genvector* pong::get_curr_state() const
{
   return curr_state_ptr;
}

inline genvector* pong::get_next_state()
{
   return next_state_ptr;
}

inline const genvector* pong::get_next_state() const
{
   return next_state_ptr;
}

inline int pong::get_screen_state_counter() const
{
   return screen_state_counter;
}


inline std::vector<genvector*>& pong::get_screen_state_ptrs()
{
   return screen_state_ptrs;
}

inline const std::vector<genvector*>& pong::get_screen_state_ptrs() const
{
   return screen_state_ptrs;
}


inline genvector* pong::get_curr_big_state()
{
   return curr_big_state_ptr;
}

inline const genvector* pong::get_curr_big_state() const
{
   return curr_big_state_ptr;
}

inline void pong::append_wtwoDarray(twoDarray* wtwoDarray_ptr)
{
   wtwoDarray_ptrs.push_back(wtwoDarray_ptr);
}

inline void pong::set_screen_exports_subdir(std::string subdir)
{
   screen_exports_subdir = subdir;
   screen_exporter_ptr = ale.createScreenExporter(screen_exports_subdir);
}

inline void pong::get_ball_posn(int& ball_px, int& ball_py)
{
   ball_px = px_ball;
   ball_py = py_ball;
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

// Pong actions: 0, 1, 3, 4

#endif  // pong.h


