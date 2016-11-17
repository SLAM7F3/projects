// ==========================================================================
// environment class member function definitions
// ==========================================================================
// Last modified on 11/9/16; 11/10/16; 11/11/16; 11/13/16
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
void environment::start_new_episode(bool random_start)
{
   if(world_type == MAZE)
   {
      maze_ptr->reset_game(random_start);
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
string environment::get_curr_state_string()
{
   string curr_state_str = "";

   if(world_type == MAZE)
   {
      return maze_ptr->occupancy_state_to_string();
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
   return legal_action_flag;
}

// ---------------------------------------------------------------------
genvector* environment::compute_next_state(int a)
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
bool environment::is_terminal_state()
{
   bool terminal_state_flag = false;
   if(world_type == MAZE)
   {
      terminal_state_flag = maze_ptr->get_maze_solved();
   }
   return terminal_state_flag;
}

// ---------------------------------------------------------------------
void environment::set_reward(double r)
{
   reward = r;
}

double environment::get_reward() const
{
   return reward;
}

// ---------------------------------------------------------------------
// Member function get_state_action_string()

string environment::get_state_action_string(genvector* curr_s, int a)
{
   string state_action_str = "";
   if(world_type == MAZE)
   {
      return get_curr_state_string()+stringfunc::number_to_string(a);
   }
   return state_action_str;
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