// ==========================================================================
// Header file for xsens_ins class
// ==========================================================================
// Last modified on 7/6/11; 8/18/11; 8/24/11
// ==========================================================================

#ifndef XSENS_INS_H
#define XSENS_INS_H

#include <deque>
#include <iostream>
#include <string>
#include <vector>

#include "xsens/cmt2.h"
#include "xsens/cmt3.h"               
#include "xsens/cmtdef.h"

#include "math/rotation.h"
#include "math/threevector.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgAnnotators/SignPostsGroup.h"

class xsens_ins
{

  public:

   xsens_ins();
   xsens_ins(const xsens_ins& s);
   ~xsens_ins();
   xsens_ins& operator= (const xsens_ins& s);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const xsens_ins& s);

// Set and get member functions:

   double set_rapid_lateral_accel_detected_flag(bool flag);
   bool get_rapid_lateral_accel_detected_flag() const;
   
   double get_lat() const;
   double get_lon() const;
   double get_alt() const;

   double get_alpha_filtered_lat() const;
   double get_alpha_filtered_lon() const;
   double get_alpha_filtered_alt() const;

   double get_az() const;
   double get_el() const;
   double get_roll() const;

   double get_daz() const;
   double get_del() const;
   double get_droll() const;

   double get_alpha_filtered_az() const;
   double get_alpha_filtered_el() const;
   double get_alpha_filtered_roll() const;

   double get_elapsed_time() const;
   int get_timestamp() const;
   void set_CM_3D_ptr(osgGA::Terrain_Manipulator* TM_ptr);
   threevector& get_acceleration();
   const threevector& get_acceleration() const;

   void set_GPS_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr);

   bool initialize_xsens();
   bool EXIT_ERROR(std::string loc);
   void initialize_xsens_2();
   void update_ins_metadata();
   void update_ins_metadata_2();

// Member functions lifted from example_linux.cpp

   int doHardwareScan(xsens::Cmt3& cmt3);
   void doMtSettings(
      xsens::Cmt3 &cmt3, CmtOutputMode &mode, CmtOutputSettings &settings);
   void getUserInputs(CmtOutputMode& mode, CmtOutputSettings& settings);

// Time-averaged XSENS output

   void update_avg_az_el_roll();
   void update_median_az_el_roll();
   void alpha_filter_az_el_roll();

   void update_median_lat_lon_alt();
   void alpha_filter_lat_lon_alt();

   void update_avg_physical_acceleration();

  private: 

   bool rapid_lateral_accel_detected_flag;
   int msg_counter,timestamp;
   int nmax_raw_measurements;
   int az_el_roll_counter,lat_lon_alt_counter;

   std::string port_name;
   xsens::Cmt2s serial;
   xsens::Cmt3 cmt3;
   xsens::Message msg,reply;
   unsigned long mtCount;
   CmtDeviceId deviceIds[256];

   double t_start,prev_t_select;
   rotation R;
   osgGA::Terrain_Manipulator* CM_3D_ptr;
   threevector ins_xhat,ins_yhat,ins_zhat;
//   threevector theta; // measured relative to Xsens' X,Y,Z directions

   std::deque<double> raw_lat,raw_lon,raw_alt;
   std::deque<double> raw_az,raw_el,raw_roll;
   std::deque<threevector> raw_accel_phys;
   threevector curr_az_el_roll,prev_az_el_roll;
   threevector curr_alpha_filtered_az_el_roll,prev_alpha_filtered_az_el_roll;

   threevector curr_lat_lon_alt,prev_lat_lon_alt;
   threevector curr_alpha_filtered_lat_lon_alt,prev_alpha_filtered_lat_lon_alt;

   threevector curr_phys_accel;	// measured relative to earth-fixed X,Y,Z dirs

   SignPostsGroup* GPS_SignPostsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const xsens_ins& x);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline double xsens_ins::set_rapid_lateral_accel_detected_flag(bool flag)
{
   rapid_lateral_accel_detected_flag=flag;
}

inline bool xsens_ins::get_rapid_lateral_accel_detected_flag() const
{
   return rapid_lateral_accel_detected_flag;
}

inline double xsens_ins::get_elapsed_time() const
{
   return 0.001*(get_timestamp()-t_start);
}

inline double xsens_ins::get_lat() const
{
   return curr_lat_lon_alt.get(0);
}

inline double xsens_ins::get_lon() const
{
   return curr_lat_lon_alt.get(1);
}

inline double xsens_ins::get_alt() const
{
   return curr_lat_lon_alt.get(2);
}

inline double xsens_ins::get_alpha_filtered_lat() const
{
   return curr_alpha_filtered_lat_lon_alt.get(0);
}

inline double xsens_ins::get_alpha_filtered_lon() const
{
   return curr_alpha_filtered_lat_lon_alt.get(1);
}

inline double xsens_ins::get_alpha_filtered_alt() const
{
   return curr_alpha_filtered_lat_lon_alt.get(2);
}

inline double xsens_ins::get_az() const
{
   return curr_az_el_roll.get(0);
}

inline double xsens_ins::get_el() const
{
   return curr_az_el_roll.get(1);
}

inline double xsens_ins::get_roll() const
{
   return curr_az_el_roll.get(2);
}

inline double xsens_ins::get_daz() const
{
   return curr_alpha_filtered_az_el_roll.get(0)-
      prev_alpha_filtered_az_el_roll.get(0);
//   return curr_az_el_roll.get(0)-prev_az_el_roll.get(0);
}

inline double xsens_ins::get_del() const
{
   return curr_alpha_filtered_az_el_roll.get(1)-
      prev_alpha_filtered_az_el_roll.get(1);
//   return curr_az_el_roll.get(1)-prev_az_el_roll.get(1);
}

inline double xsens_ins::get_droll() const
{
   return curr_alpha_filtered_az_el_roll.get(2)-
      prev_alpha_filtered_az_el_roll.get(2);
//   return curr_az_el_roll.get(2)-prev_az_el_roll.get(2);
}

inline double xsens_ins::get_alpha_filtered_az() const
{
   return curr_alpha_filtered_az_el_roll.get(0);
}

inline double xsens_ins::get_alpha_filtered_el() const
{
   return curr_alpha_filtered_az_el_roll.get(1);
}

inline double xsens_ins::get_alpha_filtered_roll() const
{
   return curr_alpha_filtered_az_el_roll.get(2);
}

inline int xsens_ins::get_timestamp() const
{
   return timestamp;
}

inline void xsens_ins::set_CM_3D_ptr(osgGA::Terrain_Manipulator* TM_ptr)
{
   CM_3D_ptr=TM_ptr;
}

inline threevector& xsens_ins::get_acceleration()
{
   return curr_phys_accel;	
}

inline const threevector& xsens_ins::get_acceleration() const
{
   return curr_phys_accel;	
}

inline void xsens_ins::set_GPS_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr)
{
   GPS_SignPostsGroup_ptr=SPG_ptr;
}



#endif  // xsens_ins.h
