// ==========================================================================
// Header file for wiimote class
// ==========================================================================
// Last modified on 9/1/11; 9/2/11; 9/5/11
// ==========================================================================

#ifndef WIIMOTE_H
#define WIIMOTE_H

#include <iostream>
#include <string>
#include <vector>
#include <cwiid.h>

#include "osg/Custom3DManipulator.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "math/twovector.h"

class wiimote
{

  public:

   wiimote();
   wiimote(const wiimote& w);
   ~wiimote();
   wiimote& operator= (const wiimote& w);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const wiimote& w);

// Set and get member functions:

   void set_rpt_mode(cwiid_wiimote_t* wiimote, unsigned char rpt_mode);

   void initialize_wiimote();
   bool update_state();
   void print_state();

   int get_n_IR_sources() const;
   bool get_IR_source_detected_flag(int i) const;
   twovector get_IR_source_posn(int i) const;
   int get_IR_source_size(int i) const;
   bool get_button_clicked_flag();
   int get_curr_button_value();
   void set_CM_3D_ptr(osgGA::Custom3DManipulator* CM_ptr);
   void set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr);
   void set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr);

// Custom3DManipulator manipulation via Wii member functions:

   void check_for_ultrasound_control_input();
   void check_for_NYC_1Kdemo_control_input();

  private: 

   cwiid_wiimote_t* wiimote_ptr;	// wiimote pointer
   struct cwiid_state state;		// wiimote state 
   bdaddr_t bdaddr;			// bluetooth device address 
   unsigned char rpt_mode;
   double t_previous_button_click,t_previous_button_value;
   double min_dt_between_button_clicks;
   double min_dt_between_button_values;
   osgGA::Custom3DManipulator* CM_3D_ptr;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;
   PointCloudsGroup* PointCloudsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const wiimote& x);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void wiimote::set_CM_3D_ptr(osgGA::Custom3DManipulator* CM_ptr)
{
   CM_3D_ptr=CM_ptr;
}

inline void wiimote::set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OFG_ptr)
{
   OBSFRUSTAGROUP_ptr=OFG_ptr;
}

inline void wiimote::set_PointCloudsGroup_ptr(PointCloudsGroup* PCG_ptr)
{
   PointCloudsGroup_ptr=PCG_ptr;
}


#endif  // wiimote.h
