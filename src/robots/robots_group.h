// ==========================================================================
// Header file for robots_group class.  
// ==========================================================================
// Last updated on 2/14/08; 2/21/08
// ==========================================================================

#ifndef ROBOTSGROUP_H
#define ROBOTSGROUP_H

#include <vector>
#include "robots/groundspace.h"
#include "messenger/message.h"
#include "robots/robot.h"

class AnimationController;
class CylindersGroup;
class Messenger;

class robots_group
{

  public:

   robots_group();
   robots_group(CylindersGroup* CylindersGroup_ptr,
                AnimationController* AnimationController_ptr);
   ~robots_group();

// Set & get member functions:

   int get_n_robots() const;
   robot* get_robot_ptr(int ID);
   const robot* get_robot_ptr(int ID) const;
   groundspace& get_groundspace();
   const groundspace& get_groundspace() const;
   void set_Messenger_ptr(Messenger* M_ptr);
   Messenger* get_Messenger_ptr();
   const Messenger* get_Messenger_ptr() const;

// Robot generation and propagation member functions:

   robot* generate_new_robot(int ID=-1);
   void propagate_all_statevectors();

// Message handling member functions:

  private:

   int curr_robot_ID;
   int prev_framenumber;
   double start_time,curr_time;

   std::vector<robot*> robot_ptrs;
   AnimationController* AnimationController_ptr;
   CylindersGroup* CylindersGroup_ptr;
   groundspace ground_space;

   Messenger* Messenger_ptr;
   std::vector<message> messages;
   std::vector<std::string> message_substrings;

   void allocate_member_objects();
   void initialize_member_objects();

   void issue_update_message();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline int robots_group::get_n_robots() const
{
   return robot_ptrs.size();
}

inline robot* robots_group::get_robot_ptr(int ID)
{
   if (ID >= 0 && ID < get_n_robots())
   {
      return robot_ptrs[ID];
   }
   else
   {
      return NULL;
   }
}

inline const robot* robots_group::get_robot_ptr(int ID) const
{
   if (ID >= 0 && ID < get_n_robots())
   {
      return robot_ptrs[ID];
   }
   else
   {
      return NULL;
   }
}

inline groundspace& robots_group::get_groundspace()
{
   return ground_space;
}

inline const groundspace& robots_group::get_groundspace() const
{
   return ground_space;
}

#endif // robots_group.h

