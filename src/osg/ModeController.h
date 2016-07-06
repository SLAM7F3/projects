// ==========================================================================
// ModeController header file 
// ==========================================================================
// Last modified on 6/17/09; 1/1/11; 11/17/11
// ==========================================================================

// The ModeController class controls the State of the viewer operation
// mode the Node Callback

#ifndef MODECONTROLLER_H
#define MODECONTROLLER_H

#include <iostream>

class ModeController 
{
  public:

   // For modes to work correctly with help, the modes must start with
   // 0, and be consecutive in number. Also, be sure to add new mode
   // name to the get_state_name() function.  And increment number of
   // states within get_n_states() method.

   enum eState {VIEW_DATA=0, GENERATE_AVI_MOVIE=1, 
		SET_CENTER=2, MANIPULATE_CENTER=3,
                INSERT_FEATURE=4, MANIPULATE_FEATURE=5, PROPAGATE_FEATURE=6,
		INSERT_ANNOTATION=7, MANIPULATE_ANNOTATION=8,
   		TRACK_FEATURE=9, MANIPULATE_MAPIMAGE=10,
	 	RUN_MOVIE=11, MANIPULATE_MOVIE=12,
   		INSERT_BOX=13, MANIPULATE_BOX=14,
   		INSERT_CONE=15, MANIPULATE_CONE=16,
   		INSERT_RECTANGLE=17, MANIPULATE_RECTANGLE=18,
	        INSERT_LINE=19, MANIPULATE_LINE=20,
   		INSERT_HEMISPHERE=21,MANIPULATE_HEMISPHERE=22,
		INSERT_POINT=23,MANIPULATE_POINT=24,
		INSERT_MODEL=25,MANIPULATE_MODEL=26,
		INSERT_CYLINDER=27,MANIPULATE_CYLINDER=28,
   		INSERT_PYRAMID=29, MANIPULATE_PYRAMID=30,
    		INSERT_POLYGON=31, MANIPULATE_POLYGON=32,
    		INSERT_POLYLINE=33, MANIPULATE_POLYLINE=34,
    		INSERT_POLYHEDRON=35, MANIPULATE_POLYHEDRON=36,
		FUSE_DATA=37,MANIPULATE_FUSED_DATA=38,
                MANIPULATE_TRIANGLE=39, 
                MANIPULATE_PLANE=40, 
                MANIPULATE_GRAPHNODE=41,
                MANIPULATE_EARTH=42,
                MANIPULATE_OBSFRUSTUM=43,
                MANIPULATE_POLYLINE_VERTEX=44,
                MANIPULATE_FISHNET=45
   };

   ModeController(bool hide_Mode_HUD_flag=false);

// Set & get member functions:

   bool get_picking_mode_flag() const;
   void set_allow_manipulator_translation_flag(bool flag);
   bool get_allow_manipulator_translation_flag() const;
   int get_n_states() const;
   void setState(eState p_state);
   void set_prev_State();
   eState getState() const;
   eState get_prev_State() const;

   std::string get_state_name();
   std::string get_state_name(int state);

  protected:

  private:

   bool picking_mode_flag,allow_manipulator_translation_flag;
   bool hide_Mode_HUD_flag;
   eState m_state,m_prev_state;
   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline bool ModeController::get_picking_mode_flag() const
{
   return picking_mode_flag;
}

inline void ModeController::set_allow_manipulator_translation_flag(bool flag)
{
   allow_manipulator_translation_flag=flag;
}

inline bool ModeController::get_allow_manipulator_translation_flag() const
{
   return allow_manipulator_translation_flag;
}

inline int ModeController::get_n_states() const
{ 
   const int n_states=46;
   return n_states;
}

inline ModeController::eState ModeController::getState() const
{ 
   return m_state; 
}

inline void ModeController::set_prev_State() 
{
   setState(m_prev_state);
}

inline ModeController::eState ModeController::get_prev_State() const
{
   return m_prev_state;
}
  
inline std::string ModeController::get_state_name()
{
   return get_state_name(m_state);
}

#endif 
