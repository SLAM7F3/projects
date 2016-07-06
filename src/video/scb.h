/********************************************************************
 *
 *
 * Name: scb.h
 *
 *
 * Author: Joseph Adams
 *
 * Description:
 *   Class for extraction of scb (Scan Control Board) data
 * --------------------------------------------------------------
 *    $Revision: 1.1 $
 * ---------------------------------------------------------------
 *    $Log: scb.h,v $
 *    Revision 1.1  2004/06/23 21:26:18  jadams
 *    *** empty log message ***
 *
 *    Revision 1.1.1.1  2003/06/18 18:02:45  jadams
 *    Starting revamped dll, basically working in the plane
 *
 *
 *
 * 1     6/11/03 4:25p Jadams
 * Rewrite of alirt dll
 * with thread manager and roadrunner class
 *
 * 3     6/02/03 1:46p Jadams
 * graphics broken out into seperate function calls
 * added device context class
 *
 * 2     6/02/03 10:36a Jadams
 * Data file based simulator added
 *
 * 1     6/02/03 10:33a Jadams
 * Ali file support added
 *
 * 1     5/19/03 6:13p Jadams
 *    Revision 1.2  2003/05/13 16:11:32  jadams
 *    working copy flight management looks to be working
 *
 *    Revision 1.1.1.1  2003/04/22 15:16:58  jadams
 *    Alirt Dll with fligh management support
 *
 *
 *
 * 1     4/22/03 10:08a Jadams
 *    Revision 1.1.1.1  2003/03/27 16:54:03  jadams
 *
 *
 *
 *    Revision 1.3  2003/03/06 23:49:10  jadams
 *    Added true heading, mean velocity, velocity heading and crab angle
 *    some fixes to makefile
 *
 *    Revision 1.2  2003/03/05 23:48:30  jadams
 *    *** empty log message ***
 *
 *    Revision 1.1.1.1  2003/02/27 21:59:23  jadams
 *
 *
 *
 *
 *
 **********************************************************************/

#ifndef _JSA_SCB_
#define _JSA_SCB_

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string>

#include "video/scb_struct.h"
#include "video/physical_constants.h"

//#define _JSA_SCB_DEBUG_
#ifdef _JSA_SCB_DEBUG_
#define _JSA_SCB_DEBUG_LEVEL_ 5
#endif

//#include <gsl/gsl_const_mks.h>
//#include <gsl/gsl_math.h>

//#define BAD_COUNT_VALUE GSL_NAN

// this is an integer not a double (NaN is double)
#define BAD_COUNT_VALUE -999

#include <math.h>

//#define USE_SCB_V0

namespace alirt
{

/*
#ifdef USE_SCB_V0
#define UNINTERLEAVE_DATA
   typedef scb_struct_v0 scb_struct;

#else
   typedef scb_struct_v1 scb_struct;
#endif
*/

   // const doubles are better than defines for this stuff because:
   // 1) pows are evaluated at *compile* time
   // 2) no casting required
   const unsigned int TIME_CONSTANT=64;
   const double VELOCITY_CONSTANT=(pow(2.0,-18));

   const double ANGLE_CONSTANT=pow(2.0,-15)*180;
   const double RATE_CONSTANT=pow(2.0,-13)*180;
   const double ACCEL_CONSTANT=0.03125;
   const double ANGLE_ACCEL_CONSTANT=pow(2.0,-12)*180;
   const double LATITUDE_CONSTANT=pow(2.0,-31)*180;
   const double HEIGHT_CONSTANT=pow(2.0,-5);

   const double BLENDED_TIME_CONSTANT=pow(2.0,-14);
   const double BLENDED_STATUS_WORD_CONSTANT=64;
   const double UNITY=1;

   const double TEMPERATURE_OFFSET_CONSTANT=-40;
   const double TEMPERATURE_SCALE_CONSTANT=0.029297;

   const double VOLTAGE_OFFSET_CONSTANT=-10;
   const double VOLTAGE_SCALE_CONSTANT=4.884e-3;

   const double VOLTAGE_COMMAND_OFFSET_CONSTANT=-10;
   const double VOLTAGE_COMMAND_SCALE_CONSTANT=3.052e-4;

   const double GPS_TWOS_SCALE_CONSTANT=0.015625;

   const double my_cons=1;

   const double mean_radius_of_earth=6371.01e3;
   using namespace std;

   class SCB
      {
        public:

         scb_struct  p_scb_struct;

         guint16 *data;
         guint16 *data2;

         bool uninterleave_data;

        public:

         SCB(void);
         ~SCB(void);

         void copy(const SCB& X);

        public:

         void ascii_dump(FILE *stream=stdout) const;
         void ascii_dump_raw(FILE *stream=stdout) const;

         void ascii_dump_line(FILE *stream=stdout,char char_sep=' ') const;
         void ascii_dump_line_header(FILE *stream=stdout,char char_sep=' ') const;
         void ascii_dump_line_raw(FILE *stream=stdout) const;

         long size_in_file(void) const;
         void stuff(const  scb_struct& p_scb_);
         void stuff_pvoid(const  void * p_void);
         /*
           void ascii_dump_line(FILE *stream=stdout) const;
           void ascii_dump_line_sep(FILE *stream=stdout,const char sep='|') const;
           void ascii_dump_line_header(FILE *stream=stdout) const;
           int read_from_current_pos(FILE *stream);
         */

        private:
         float offset_binary_to_volts(guint16 ob_val) const;

         static inline double offset_binary_to_double(guint16 ob_val, double offset, double scale)
            {
               //  return scale*(static_cast<double>(ob_val)+offset);

               return scale*(static_cast<double>(ob_val))+offset;

            }

         static inline double twos_complement_to_double(gint16 tc_val, double scale)
            {
               return scale*(static_cast<double>(tc_val));
            }
         void word_swap(guint32& x) const;
         void word_swap(gint32& x) const;
        public:
         int is_checksum_bad(void) const;

         double  az_volts(void) const;
         double  el_volts(void) const;
         double  az_volts_cmd(void) const;
         double  el_volts_cmd(void) const;

         //////////////////////////////////////////////////////

         void get_utm_values(
            double& northing,double& easting,
            int& ZoneNumber,bool& northern_hemisphere_flag) const;
         void get_utm_values(double& northing,double& easting) const;
         double utm_northing(void) const;
         double utm_easting(void) const;

         inline unsigned int ins_mode(void) const
            { return static_cast<unsigned int> (p_scb_struct.ins_mode);}

         inline unsigned short ins_time_tag(void) const
            { return static_cast<unsigned short> (p_scb_struct.ins_time_tag)*
                 TIME_CONSTANT;}

         inline double x_velocity(void) const
            { return static_cast<int> (p_scb_struct.x_velocity)*
                 VELOCITY_CONSTANT;}

         inline double y_velocity(void) const
            { return static_cast<int> (p_scb_struct.y_velocity)*
                 VELOCITY_CONSTANT;}

         inline double z_velocity(void) const
            { return static_cast<int> (p_scb_struct.z_velocity)*
                 VELOCITY_CONSTANT;}

         inline double platform_az(void) const
            { return static_cast<short> (p_scb_struct.platform_az)* 
                 ANGLE_CONSTANT;}

         inline double roll_angle(void) const
            { return static_cast<short> (p_scb_struct.roll_angle)* 
                 ANGLE_CONSTANT;}

         inline double pitch_angle(void) const
            { return static_cast<short> (p_scb_struct.pitch_angle)* 
                 ANGLE_CONSTANT;}

         inline double roll_rate(void) const
            { return static_cast<short> (p_scb_struct.roll_rate)* 
                 RATE_CONSTANT;}

         inline double pitch_rate(void) const
            { return static_cast<short> (p_scb_struct.pitch_rate)* 
                 RATE_CONSTANT;}

         inline double yaw_rate(void) const
            { return static_cast<short> (p_scb_struct.yaw_rate)* 
                 RATE_CONSTANT;}

         inline double longitudinal_acceleration(void) const
            { return static_cast<short> (
               p_scb_struct.longitudinal_acceleration)*ACCEL_CONSTANT;}

         inline double lateral_acceleration(void) const
            { return static_cast<short> (p_scb_struct.lateral_acceleration)*
                 ACCEL_CONSTANT;}

         inline double normal_acceleration(void) const
            { return static_cast<short> (p_scb_struct.normal_acceleration)*
                 ACCEL_CONSTANT;}

         inline unsigned short platform_az_time_tag(void) const
            { return static_cast<unsigned short> (
               p_scb_struct.platform_az_time_tag)*TIME_CONSTANT;}

         inline unsigned short roll_time_tag(void) const
            { return static_cast<unsigned short> (
               p_scb_struct.roll_time_tag)*TIME_CONSTANT;}

         inline  unsigned short pitch_time_tag(void) const
            { return static_cast<unsigned short> (
               p_scb_struct.pitch_time_tag)*TIME_CONSTANT;}

         inline double roll_axis_angular_acceleration(void) const
            { return static_cast<short> (
               p_scb_struct.roll_axis_angular_acceleration)*
                 ANGLE_ACCEL_CONSTANT;}

         inline double pitch_axis_angular_acceleration(void) const
            { return static_cast<short> (
               p_scb_struct.pitch_axis_angular_acceleration)*
                 ANGLE_ACCEL_CONSTANT;}

         inline double yaw_axis_angular_acceleration(void) const
            { return static_cast<short> (
               p_scb_struct.yaw_axis_angular_acceleration)*
                 ANGLE_ACCEL_CONSTANT;}

         inline double latitude(void) const
            { return static_cast<int> (
               p_scb_struct.latitude)*LATITUDE_CONSTANT;}

         inline double longitude(void) const
            { return static_cast<int> (p_scb_struct.longitude)*
                 LATITUDE_CONSTANT;}

         inline double ellipsoid_height(void) const
            { return static_cast<int> (
               p_scb_struct.ellipsoid_height)* HEIGHT_CONSTANT;}

         inline double blended_measurement_position_time(void) const
            { return static_cast<int> (
               p_scb_struct.blended_measurement_position_time)*
                 BLENDED_TIME_CONSTANT;}

         inline unsigned int ins_blended_status_word(void) const
            { return static_cast<unsigned int> (
               p_scb_struct.ins_blended_status_word);}

         inline int fsi_packet_id(void) const
            { return static_cast<int> (p_scb_struct.fsi_packet_id);}

         inline double bench_temperature_1(void) const
            {
               guint16 ob_val=p_scb_struct.bench_temperature_1;
               double offset=TEMPERATURE_OFFSET_CONSTANT;
               double scale=TEMPERATURE_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline double azimuth_servo_error(void) const
            {
               guint16 ob_val=p_scb_struct.azimuth_servo_error;
               double offset=VOLTAGE_OFFSET_CONSTANT;
               double scale=VOLTAGE_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline double bench_temperature_2(void) const
            {
               guint16 ob_val=p_scb_struct.bench_temperature_2;
               double offset=TEMPERATURE_OFFSET_CONSTANT;
               double scale=TEMPERATURE_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline double elevation_servo_error(void) const
            {
               guint16 ob_val=p_scb_struct.elevation_servo_error;
               double offset=VOLTAGE_OFFSET_CONSTANT;
               double scale=VOLTAGE_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline double azimuth_drive_current(void) const
            {
               guint16 ob_val=p_scb_struct.azimuth_drive_current;
               double offset=VOLTAGE_OFFSET_CONSTANT;
               double scale=VOLTAGE_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline double elevation_drive_current(void) const
            {
               guint16 ob_val=p_scb_struct.elevation_drive_current;
               double offset=VOLTAGE_OFFSET_CONSTANT;
               double scale=VOLTAGE_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline double temperature_1(void) const
            {
               guint16 ob_val=p_scb_struct.bench_temperature_1;
               double offset=TEMPERATURE_OFFSET_CONSTANT;
               double scale=TEMPERATURE_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline double temperature_2(void) const
            {
               guint16 ob_val=p_scb_struct.bench_temperature_2;
               double offset=TEMPERATURE_OFFSET_CONSTANT;
               double scale=TEMPERATURE_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline double elevation_voltage_command(void) const
            {
               guint16 ob_val=p_scb_struct.elevation_voltage_command;
               double offset=VOLTAGE_COMMAND_OFFSET_CONSTANT;
               double scale=VOLTAGE_COMMAND_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline double azimuth_voltage_command(void) const
            {
               guint16 ob_val=p_scb_struct.azimuth_voltage_command;
               double offset=VOLTAGE_COMMAND_OFFSET_CONSTANT;
               double scale=VOLTAGE_COMMAND_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline int ins_packet_id(void) const
            { return static_cast<int> (p_scb_struct.ins_packet_id);}

         inline unsigned int scan_controller_status(void) const
            { return static_cast<unsigned int> (
               p_scb_struct.scan_controller_status);}

         inline double true_heading(void) const
            { return static_cast<short> (p_scb_struct.true_heading)* 
                 ANGLE_CONSTANT;}

         inline int scan_controller_clock(void) const
            { return static_cast<int> (p_scb_struct.scan_controller_clock);}

         inline double gps_geometric_dilution_of_position(void) const
            {
               gint16 tc_val=p_scb_struct.gps_geometric_dilution_of_position;
               return twos_complement_to_double(
                  tc_val, GPS_TWOS_SCALE_CONSTANT);
            }

         inline double gps_position_dilution_of_position(void) const
            {
               gint16 tc_val=p_scb_struct.gps_position_dilution_of_position;
               return twos_complement_to_double(
                  tc_val, GPS_TWOS_SCALE_CONSTANT);
            }
         inline double gps_horizontal_dilution_of_position(void) const
            {
               gint16 tc_val=p_scb_struct.gps_horizontal_dilution_of_position;
               return twos_complement_to_double(
                  tc_val, GPS_TWOS_SCALE_CONSTANT);
            }

         inline double gps_vertical_dilution_of_position(void) const
            {
               gint16 tc_val=p_scb_struct.gps_vertical_dilution_of_position;
               return twos_complement_to_double(
                  tc_val, GPS_TWOS_SCALE_CONSTANT);
            }

         inline double gps_time_dilution_of_position(void) const
            {
               gint16 tc_val=p_scb_struct.gps_time_dilution_of_position;
               return twos_complement_to_double(
                  tc_val, GPS_TWOS_SCALE_CONSTANT);
            }
         inline int no_of_satellites(void) const
            { return static_cast<int> (p_scb_struct.no_of_satellites);}

         inline unsigned int gps_status_word1(void) const
            { return static_cast<unsigned int> (
               p_scb_struct.gps_status_word1);}

         inline unsigned int gps_status_word2(void) const
            { return static_cast<unsigned int> (
               p_scb_struct.gps_status_word2);}

         inline unsigned int test_pattern2(void) const
            { return static_cast<unsigned int> (p_scb_struct.test_pattern2);}

         inline unsigned int test_pattern1(void) const
            { return static_cast<unsigned int> (p_scb_struct.test_pattern1);}

         inline double elevation_axis_output_voltage(void) const
            {
               guint16 ob_val=p_scb_struct.elevation_axis_output_voltage;
               double offset=VOLTAGE_COMMAND_OFFSET_CONSTANT;
               double scale=VOLTAGE_COMMAND_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline double azimuth_axis_output_voltage(void) const
            {
               guint16 ob_val=p_scb_struct.azimuth_axis_output_voltage;
               double offset=VOLTAGE_COMMAND_OFFSET_CONSTANT;
               double scale=VOLTAGE_COMMAND_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         inline unsigned int checksum(void) const
            { return static_cast<unsigned int> (p_scb_struct.checksum);}

         inline double mean_velocity(void) const
            {
               return sqrt(x_velocity()*x_velocity()
                           + y_velocity()*y_velocity()
                           + z_velocity()*z_velocity());
            }

         double velocity_heading(void) const;
         double crab_angle(void) const;

         static inline double deg2rad(double x) { return (M_PI/180.0)*x;}
         static inline double rad2deg(double x) { return (180.0/M_PI)*x;}

         static inline double elevation_voltage_command(const scb_struct *ptr)
            {
               guint16 ob_val=ptr->elevation_voltage_command;
               double offset=VOLTAGE_COMMAND_OFFSET_CONSTANT;
               double scale=VOLTAGE_COMMAND_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         static inline double azimuth_voltage_command(const scb_struct *ptr)
            {
               guint16 ob_val=ptr->azimuth_voltage_command;
               double offset=VOLTAGE_COMMAND_OFFSET_CONSTANT;
               double scale=VOLTAGE_COMMAND_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         static inline double elevation_axis_output_voltage(
            const scb_struct *ptr)
            {
               guint16 ob_val=ptr->elevation_axis_output_voltage;
               double offset=VOLTAGE_COMMAND_OFFSET_CONSTANT;
               double scale=VOLTAGE_COMMAND_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

         static inline double  azimuth_axis_output_voltage(
            const scb_struct *ptr)
            {
               guint16 ob_val=ptr->azimuth_axis_output_voltage;
               double offset=VOLTAGE_COMMAND_OFFSET_CONSTANT;
               double scale=VOLTAGE_COMMAND_SCALE_CONSTANT;
               return offset_binary_to_double(ob_val, offset, scale);
            }

      };

}                                                 // end of namespace alirt
#endif
