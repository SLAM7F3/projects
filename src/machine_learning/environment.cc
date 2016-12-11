// ==========================================================================
// environment class member function definitions
// ==========================================================================
// Last modified on 12/2/16; 12/7/16; 12/8/16; 12/10/16
// ==========================================================================

#include "machine_learning/environment.h"
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

void environment::initialize_member_objects()
{
   frame_skip = -1;
   maze_ptr = NULL;
   tictac3d_ptr = NULL;
   spaceinv_ptr = NULL;
   breakout_ptr = NULL;

   use_big_states_flag = false;
//   use_big_states_flag = true;
}

// ---------------------------------------------------------------------
void environment::allocate_member_objects()
{
}		       

// ---------------------------------------------------------------------
environment::environment(int world_type)
{
   this->world_type = world_type;
   
   if(world_type == MAZE)
   {
      cout << "Maze environment" << endl;
   }
   else if (world_type == TTT)
   {
      cout << "TTT environment" << endl;
   }

   initialize_member_objects();
   allocate_member_objects();
}

// ---------------------------------------------------------------------
environment::~environment()
{
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const environment& E)
{
   outstream << endl;
   return outstream;
}

// ---------------------------------------------------------------------
void environment::start_new_episode(bool random_start)
{
   if(world_type == MAZE)
   {
      maze_ptr->reset_game(random_start);
   }
   else if(world_type == SPACEINV)
   {
      spaceinv_ptr->get_ale().reset_game();
   }
   else if(world_type == BREAKOUT)
   {
      breakout_ptr->get_ale().reset_game();
   }
   else if(world_type == TTT)
   {
      tictac3d_ptr->reset_board_state();
   }
}

// ---------------------------------------------------------------------
genvector* environment::get_curr_state()
{
   genvector* curr_state_ptr = NULL;

   if(world_type == MAZE)
   {
      curr_state_ptr = maze_ptr->get_occupancy_state();
   }
   else if(world_type == SPACEINV)
   {
      if(use_big_states_flag)
      {
         curr_state_ptr = spaceinv_ptr->get_curr_big_state();
      }
      else
      {
         curr_state_ptr = spaceinv_ptr->get_curr_state();
      }
   }
   else if(world_type == BREAKOUT)
   {
      if(use_big_states_flag)
      {
         curr_state_ptr = breakout_ptr->get_curr_big_state();
      }
      else
      {
         curr_state_ptr = breakout_ptr->get_curr_state();
      }
   }
   else if(world_type == TTT)
   {
      curr_state_ptr = tictac3d_ptr->get_board_state_ptr();
   }
   return curr_state_ptr;
}

// ---------------------------------------------------------------------
string environment::get_curr_state_string()
{
   string curr_state_str = "";

   if(world_type == MAZE)
   {
      return maze_ptr->occupancy_state_to_string();
   }
   else if(world_type == TTT)
   {
      return tictac3d_ptr->board_state_to_string();
   }
   return curr_state_str;
}

// ---------------------------------------------------------------------
bool environment::is_legal_action(int a)
{
   bool legal_action_flag = true;
   if(world_type == MAZE)
   {
      legal_action_flag = maze_ptr->legal_turtle_move(a);
   }
   else if(world_type == TTT)
   {
      legal_action_flag = tictac3d_ptr->legal_player_move(a);
   }
   return legal_action_flag;
}

// ---------------------------------------------------------------------
genvector* environment::compute_next_state(int a, int player_value)
{
   genvector* next_state_ptr = NULL;

   if(world_type == MAZE)
   {
      int curr_dir = a;
      bool erase_turtle_path = true;
      maze_ptr->move_turtle(curr_dir, erase_turtle_path);
      next_state_ptr = maze_ptr->get_occupancy_state();
   }
   else if (world_type == SPACEINV)
   {
      bool export_frames_flag = false;
      if(use_big_states_flag)
      {
         spaceinv_ptr->crop_pool_curr_frame(export_frames_flag);
         spaceinv_ptr->update_curr_big_state();
         next_state_ptr = spaceinv_ptr->get_curr_big_state();
      }
      else
      {
         spaceinv_ptr->crop_pool_difference_curr_frame(export_frames_flag);
         next_state_ptr = spaceinv_ptr->get_next_state();
      }
   }
   else if (world_type == BREAKOUT)
   {
      bool export_frames_flag = false;
      if(use_big_states_flag)
      {
         breakout_ptr->crop_pool_curr_frame(export_frames_flag);
         breakout_ptr->update_curr_big_state();
         next_state_ptr = breakout_ptr->get_curr_big_state();
      }
      else
      {
         breakout_ptr->crop_pool_difference_curr_frame(export_frames_flag);
         next_state_ptr = breakout_ptr->get_next_state();
      }
   }
   else if (world_type == TTT)
   {
      tictac3d_ptr->set_player_move(a, player_value);
      next_state_ptr = tictac3d_ptr->get_board_state_ptr();
   }
   return next_state_ptr;
}

// ---------------------------------------------------------------------
bool environment::is_terminal_state()
{
   bool terminal_state_flag = false;
   if(world_type == MAZE)
   {
      terminal_state_flag = maze_ptr->get_maze_solved();
   }
   else if(world_type == TTT)
   {
      terminal_state_flag = tictac3d_ptr->get_game_over();
   }
   return terminal_state_flag;
}

// ---------------------------------------------------------------------
bool environment::get_game_over()
{
   bool game_over_flag = false;
   if(world_type == MAZE)
   {
      return maze_ptr->get_game_over();
   }
   else if(world_type == SPACEINV)
   {
      return spaceinv_ptr->get_ale().game_over();
   }
   else if(world_type == BREAKOUT)
   {
      return breakout_ptr->get_ale().game_over();
   }
   else if(world_type == TTT)
   {
      return tictac3d_ptr->get_game_over();
   }
   return game_over_flag;
}

// ---------------------------------------------------------------------
void environment::set_game_over(bool flag)
{
   if(world_type == MAZE)
   {
      return maze_ptr->set_game_over(flag);
   }
   else if(world_type == TTT)
   {
      return tictac3d_ptr->set_game_over(flag);
   }
}

// ---------------------------------------------------------------------
int environment::get_episode_framenumber() const
{
   if(world_type == SPACEINV)
   {
      return spaceinv_ptr->get_ale().getEpisodeFrameNumber();
   }
   else if(world_type == BREAKOUT)
   {
      return breakout_ptr->get_ale().getEpisodeFrameNumber();
   }
   else
   {
      return -1;
   }
}

// ---------------------------------------------------------------------
int environment::get_min_episode_framenumber() const
{
   if(world_type == SPACEINV)
   {
      return spaceinv_ptr->get_min_episode_framenumber();
   }
   else if(world_type == BREAKOUT)
   {
      return breakout_ptr->get_min_episode_framenumber();
   }
   else
   {
      return -1;
   }
}

// ---------------------------------------------------------------------
double environment::get_max_score_per_episode() const
{
   if(world_type == SPACEINV)
   {
      return spaceinv_ptr->get_max_score_per_episode();
   }
   else if(world_type == BREAKOUT)
   {
      return breakout_ptr->get_max_score_per_episode();
   }
   else
   {
      return -1;
   }
}

// ---------------------------------------------------------------------
// Member function get_state_action_string()

string environment::get_state_action_string(genvector* curr_s, int a)
{
   return get_curr_state_string()+stringfunc::number_to_string(a);
}

// ---------------------------------------------------------------------
string environment::get_state_action_string(string state_str, int a)
{
   return state_str + stringfunc::number_to_string(a);
}

// ---------------------------------------------------------------------
vector<genvector*> environment::get_all_curr_states()
{
   vector<genvector*> curr_states;
   if(world_type == MAZE)
   {
      return maze_ptr->get_curr_maze_states();
   }
   return curr_states;
}

// ---------------------------------------------------------------------
vector<string> environment::get_all_curr_state_strings()
{
   vector<string> curr_state_strings;
   if(world_type == MAZE)
   {
      curr_state_strings = maze_ptr->get_curr_maze_state_strings();
   }
   return curr_state_strings;
}

// ---------------------------------------------------------------------
void environment::append_wtwoDarray(twoDarray* wtwoDarray_ptr)
{
   if(world_type == MAZE)
   {
      maze_ptr->append_wtwoDarray(wtwoDarray_ptr);
   }
   else if(world_type == TTT)
   {
      tictac3d_ptr->append_wtwoDarray(wtwoDarray_ptr);
   }
   else if(world_type == SPACEINV)
   {
      spaceinv_ptr->append_wtwoDarray(wtwoDarray_ptr);
   }
   else if(world_type == BREAKOUT)
   {
      breakout_ptr->append_wtwoDarray(wtwoDarray_ptr);
   }
   else
   {
      cout << "Error in environment::append_wtwoDarray()!" << endl;
      exit(-1);
   }
}

// ---------------------------------------------------------------------
vector<genvector*>* environment::get_all_afterstates(int player_value)
{
   if(world_type == TTT)
   {
      return &tictac3d_ptr->compute_all_afterstates(player_value);
   }
   return NULL;
}

