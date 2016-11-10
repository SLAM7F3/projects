// ==========================================================================
// environment class member function definitions
// ==========================================================================
// Last modified on 11/9/16
// ==========================================================================

#include "machine_learning/environment.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void environment::initialize_member_objects()
{
   maze_ptr = NULL;
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
void environment::start_new_episode()
{
   if(world_type == MAZE)
   {
      maze_ptr->generate_maze();
//      maze_ptr->DrawMaze();
      maze_ptr->reset_game();
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
   return curr_state_ptr;
}

// ---------------------------------------------------------------------
bool environment::is_legal_action(int a)
{
   bool legal_action_flag = true;
   if(world_type == MAZE)
   {
      legal_action_flag = maze_ptr->legal_turtle_move(a);
      
   }
   return legal_action_flag;
}

// ---------------------------------------------------------------------
genvector* environment::get_next_state(int a)
{
   genvector* next_state_ptr = NULL;

   if(world_type == MAZE)
   {
      int curr_dir = a;
      bool erase_turtle_path = true;
      maze_ptr->move_turtle(curr_dir, erase_turtle_path);
      next_state_ptr = maze_ptr->get_occupancy_state();
   }
   return next_state_ptr;
}

// ---------------------------------------------------------------------
bool environment::is_terminal_state(genvector* s)
{
   bool terminal_state_flag = false;
   if(world_type == MAZE)
   {
      terminal_state_flag = maze_ptr->get_maze_solved();
   }
   return terminal_state_flag;
}

// ---------------------------------------------------------------------
double environment::get_reward_for_next_state(genvector* next_state_ptr)
{
   double reward = 0;
   if(world_type == MAZE)
   {
      if(is_terminal_state(next_state_ptr))
      {
         reward = 1;
      }
   }
   return reward;
}
