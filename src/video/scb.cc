/********************************************************************
 *
 *
 * Name: scb.cc
 *
 *
 * Author: Joseph Adams
 *
 * Description:
 *   Class for extraction of scb (Scan Control Board) data
 *
 *
 * --------------------------------------------------------------
 *    $Revision: 1.1 $
 * ---------------------------------------------------------------
 *    $Log: scb.cpp,v $
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

#include <iostream>
#include "astro_geo/latlong2utmfuncs.h"
#include "video/scb.h"

#define NaN 0
#define NaS "????"

using std::string;

namespace alirt
{

   SCB::SCB(void)
      {
         p_scb_struct.ins_mode=999;

         data= new guint16 [64];
         data2= new guint16 [64];

         uninterleave_data=false;
      }

   SCB::~SCB(void)
      {
         delete[] data ;
         delete[] data2 ;
      }

   long SCB::size_in_file(void) const
      {

         return sizeof(p_scb_struct);

      }

   void SCB::copy(const SCB& X)
      {

         p_scb_struct=X.p_scb_struct;

      }

   void SCB::stuff(const  scb_struct& p_scb_struct_)
      {
         p_scb_struct=p_scb_struct_;
      }

   void SCB::stuff_pvoid(const  void * p_void)
      {

         guint16 *ptr;
         ptr=(guint16 *) p_void;

         for (int i=0; i<64; i++)
         {
            data[i]=ptr[i];
            data2[i]=ptr[i];
         }

#ifdef UNINTERLEAVE_DATA
         uninterleave_data=true;
         if (uninterlevae_data)
         {
            for (int i=0; i<64; i=i+2)
            {

               data[i]=data2[i+1];
               data[i+1]=data2[i];
            }
         }
#endif

         //  p_scb_struct=* (scb_struct *)p_void;
         p_scb_struct=* (scb_struct *)data;

#ifdef UNINTERLEAVE_DATA
         if (uninterlevae_data)
         {
            word_swap(p_scb_struct.x_velocity);
            word_swap(p_scb_struct.y_velocity);
            word_swap(p_scb_struct.z_velocity);
            word_swap(p_scb_struct.latitude);
            word_swap(p_scb_struct.longitude);
            word_swap(p_scb_struct.ellipsoid_height);
            word_swap(p_scb_struct.blended_measurement_position_time);
            // word_swap(p_scb_struct.fsi_packet_id);
            //  word_swap(p_scb_struct.ins_packet_id);
            //  word_swap(p_scb_struct.scan_controller_clock);
         }
#endif

         return ;

      }

   int SCB::is_checksum_bad(void) const
      {

         //	guint32 x;
         guint16 y;

         //	guint16 upper;
         //	guint16 lower;

         guint16 checksum;

         checksum=0x0;

         guint16 *base_ptr;

         base_ptr=(guint16 *)(&p_scb_struct);

         for (int j=0; j<64; j++)
         {

            //        y=p_scb_struct.scanctrl[j];

            y=*(base_ptr + j );
            checksum = checksum ^ y;

         }

         return checksum;
      }

   double SCB::az_volts(void) const
      {

         guint16 az_voltage_ob;
         double v_az;

         az_voltage_ob=p_scb_struct.azimuth_axis_output_voltage;

         v_az=offset_binary_to_volts(az_voltage_ob);

         return v_az;
      }

   double SCB::el_volts(void) const
      {

         guint16 el_voltage_ob;
         double v_el;

         el_voltage_ob=p_scb_struct.elevation_axis_output_voltage;

         v_el=offset_binary_to_volts(el_voltage_ob);

         return v_el;
      }

   double SCB::az_volts_cmd(void) const
      {

         guint16 az_voltage_ob;
         double v_az;

         az_voltage_ob=p_scb_struct.azimuth_voltage_command;

         v_az=offset_binary_to_volts(az_voltage_ob);

         return v_az;
      }

   double SCB::el_volts_cmd(void) const
      {

         guint16 el_voltage_ob;
         double v_el;

         el_voltage_ob=p_scb_struct.elevation_voltage_command;

         v_el=offset_binary_to_volts(el_voltage_ob);

         return v_el;
      }

   // convert Leo's sign extended value to volts
   float SCB::offset_binary_to_volts(guint16 ob_val) const
      {

         float value=-999;

         //    result = (guint32) ( (((double)0xFFFF)/20.0) *(volts+10));

         double scale_factor=(20.0/((double)0xFFFF));

         value=(scale_factor*(double)(ob_val))-10.0;

         return value;

      }

   void SCB::word_swap(guint32& x) const
      {

         guint32 y;
         guint16 upper;
         guint16 lower;

         //	x=ifov.p_ifov_struct.scanctrl[j];
         upper = (guint16)(x>> 16);

         lower = (guint16)(x & 0x0000FFFF);

         y=((guint32)(lower << 16) + upper);

         x=y;

         return;

      }

   void SCB::word_swap(gint32& x) const
      {

         // guint32 x;
         guint32 y;
         guint16 upper;
         guint16 lower;

         //x=&x_;
         //	x=ifov.p_ifov_struct.scanctrl[j];
         upper = (guint16)(x>> 16);

         lower = (guint16)(x & 0x0000FFFF);

         y=((guint32)(lower << 16) + upper);

         x=y;

         return;

      }

   void SCB::ascii_dump_raw(FILE *stream) const
      {

         guint16 x;

         for (int i=0; i<64; i++)
         {

            x=data[i];

            //        y=word_swap(x, upper, lower);

            fprintf(stream,"\n\tscanctrl(%02d)=\t(%04ud)=\t<0x%04X>",i+1,x,x);
            fprintf(stream,"\t<0x%04X>",data2[i]);

            //fprintf(stream,"\t[%04X %04x]",upper,lower);

            // fprintf(stream,"\t[%04X %04x]",lower,upper);

         }

      }

   void SCB::ascii_dump_line_raw(FILE *stream) const
      {

//         guint16 x;

         for (int i=0; i<64; i++)
         {

//            x=data[i];

            //  fprintf(stream,"\n\tscanctrl(%02d)=\t(%04ud)=\t<0x%04X>",i+1,x,x);
            fprintf(stream,"%u ",data2[i]);

            //fprintf(stream,"\t[%04X %04x]",upper,lower);

            // fprintf(stream,"\t[%04X %04x]",lower,upper);

         }

      }

   void SCB::ascii_dump_line_header(FILE *stream,  char char_sep) const
      {

         fprintf(stream,"ins_mode");
         fprintf(stream,"%cins_time_tag",char_sep);
         fprintf(stream,"%cx_velocity",char_sep);
         fprintf(stream,"%cy_velocity",char_sep);
         fprintf(stream,"%cz_velocity",char_sep);
         fprintf(stream,"%cmean_velocity",char_sep);
         fprintf(stream,"%cvelocity_heading",char_sep);
         fprintf(stream,"%ccrab_angle",char_sep);
         fprintf(stream,"%cplatform_az",char_sep);
         fprintf(stream,"%croll_angle",char_sep);
         fprintf(stream,"%cpitch_angle",char_sep);
         fprintf(stream,"%croll_rate",char_sep);
         fprintf(stream,"%cpitch_rate",char_sep);
         fprintf(stream,"%cyaw_rate",char_sep);
         fprintf(stream,"%clongitudinal_acceleration",char_sep);
         fprintf(stream,"%clateral_acceleration",char_sep);
         fprintf(stream,"%cnormal_acceleration",char_sep);
         fprintf(stream,"%cplatform_az_time_tag",char_sep);
         fprintf(stream,"%croll_time_tag",char_sep);
         fprintf(stream,"%cpitch_time_tag",char_sep);
         fprintf(stream,"%croll_axis_angular_acceleration",char_sep);
         fprintf(stream,"%cpitch_axis_angular_acceleration",char_sep);
         fprintf(stream,"%cyaw_axis_angular_acceleration",char_sep);
         fprintf(stream,"%clatitude",char_sep);
         fprintf(stream,"%clongitude",char_sep);
         fprintf(stream,"%cellipsoid_height",char_sep);
         fprintf(stream,"%ceasting",char_sep);
         fprintf(stream,"%cnorthing",char_sep);
         fprintf(stream,"%cblended_measurement_position_time",char_sep);
         fprintf(stream,"%cins_blended_status_word",char_sep);
         fprintf(stream,"%cfsi_packet_id",char_sep);
         fprintf(stream,"%cbench_temperature_1",char_sep);
         fprintf(stream,"%cazimuth_servo_error",char_sep);
         fprintf(stream,"%cbench_temperature_2",char_sep);
         fprintf(stream,"%celevation_servo_error",char_sep);
         fprintf(stream,"%cazimuth_drive_current",char_sep);
         fprintf(stream,"%celevation_drive_current",char_sep);
         fprintf(stream,"%ctemperature_1",char_sep);
         fprintf(stream,"%ctemperature_2",char_sep);
         fprintf(stream,"%celevation_voltage_command",char_sep);
         fprintf(stream,"%cazimuth_voltage_command",char_sep);
         fprintf(stream,"%cins_packet_id",char_sep);
         fprintf(stream,"%cscan_controller_status",char_sep);
#ifdef USE_SCB_V0
         fprintf(stream,"%cspare1",char_sep);
#else
         fprintf(stream,"%ctrue_heading",char_sep);
#endif
         fprintf(stream,"%cscan_controller_clock",char_sep);
         fprintf(stream,"%cgps_geometric_dilution_of_position",char_sep);
         fprintf(stream,"%cgps_position_dilution_of_position",char_sep);
         fprintf(stream,"%cgps_horizontal_dilution_of_position",char_sep);
         fprintf(stream,"%cgps_vertical_dilution_of_position",char_sep);
         fprintf(stream,"%cgps_time_dilution_of_position",char_sep);
         fprintf(stream,"%cno_of_satellites",char_sep);
         fprintf(stream,"%cgps_status_word1",char_sep);
         fprintf(stream,"%cgps_status_word2",char_sep);
         fprintf(stream,"%ctest_pattern2",char_sep);
         fprintf(stream,"%ctest_pattern1",char_sep);
         fprintf(stream,"%celevation_axis_output_voltage",char_sep);
         fprintf(stream,"%cazimuth_axis_output_voltage",char_sep);
         fprintf(stream,"%cchecksum",char_sep);
         fprintf(stream,"%cis_checksum_bad",char_sep);

      }

   void SCB::ascii_dump_line(FILE *stream, char char_sep) const
      {

         fprintf(stream,"%04u",ins_mode());
         fprintf(stream,"%c%d",char_sep,ins_time_tag());
         fprintf(stream,"%c%f",char_sep,x_velocity());
         fprintf(stream,"%c%f",char_sep,y_velocity());
         fprintf(stream,"%c%f",char_sep,z_velocity());
         fprintf(stream,"%c%f",char_sep,mean_velocity());
         fprintf(stream,"%c%f",char_sep,velocity_heading());
         fprintf(stream,"%c%f",char_sep,crab_angle());
         fprintf(stream,"%c%f",char_sep,platform_az());
         fprintf(stream,"%c%f",char_sep,roll_angle());
         fprintf(stream,"%c%f",char_sep,pitch_angle());
         fprintf(stream,"%c%f",char_sep,roll_rate());
         fprintf(stream,"%c%f",char_sep,pitch_rate());
         fprintf(stream,"%c%f",char_sep,yaw_rate());
         fprintf(stream,"%c%f",char_sep,longitudinal_acceleration());
         fprintf(stream,"%c%f",char_sep,lateral_acceleration());
         fprintf(stream,"%c%f",char_sep,normal_acceleration());
         fprintf(stream,"%c%d",char_sep,platform_az_time_tag());
         fprintf(stream,"%c%d",char_sep,roll_time_tag());
         fprintf(stream,"%c%d",char_sep,pitch_time_tag());
         fprintf(stream,"%c%f",char_sep,roll_axis_angular_acceleration());
         fprintf(stream,"%c%f",char_sep,pitch_axis_angular_acceleration());
         fprintf(stream,"%c%f",char_sep,yaw_axis_angular_acceleration());
         fprintf(stream,"%c%f",char_sep,latitude());
         fprintf(stream,"%c%f",char_sep,longitude());
         fprintf(stream,"%c%f",char_sep,ellipsoid_height());
         fprintf(stream,"%c%f",char_sep,utm_easting());
         fprintf(stream,"%c%f",char_sep,utm_northing());
         fprintf(stream,"%c%f",char_sep,blended_measurement_position_time());
         fprintf(stream,"%c%04u",char_sep,ins_blended_status_word());
         fprintf(stream,"%c%d",char_sep,fsi_packet_id());
         fprintf(stream,"%c%f",char_sep,bench_temperature_1());
         fprintf(stream,"%c%f",char_sep,azimuth_servo_error());
         fprintf(stream,"%c%f",char_sep,bench_temperature_2());
         fprintf(stream,"%c%f",char_sep,elevation_servo_error());
         fprintf(stream,"%c%f",char_sep,azimuth_drive_current());
         fprintf(stream,"%c%f",char_sep,elevation_drive_current());
         fprintf(stream,"%c%f",char_sep,temperature_1());
         fprintf(stream,"%c%f",char_sep,temperature_2());
         fprintf(stream,"%c%f",char_sep,elevation_voltage_command());
         fprintf(stream,"%c%f",char_sep,azimuth_voltage_command());
         fprintf(stream,"%c%u",char_sep,ins_packet_id());
         fprintf(stream,"%c%04u",char_sep,scan_controller_status());
#ifdef USE_SCB_V0
         fprintf(stream,"%c%f",char_sep,true_heading());
#else
         fprintf(stream,"%c%f",char_sep,true_heading());
#endif
         fprintf(stream,"%c%04d",char_sep,scan_controller_clock());
         fprintf(stream,"%c%f",char_sep,gps_geometric_dilution_of_position());
         fprintf(stream,"%c%f",char_sep,gps_position_dilution_of_position());
         fprintf(stream,"%c%f",char_sep,gps_horizontal_dilution_of_position());
         fprintf(stream,"%c%f",char_sep,gps_vertical_dilution_of_position());
         fprintf(stream,"%c%f",char_sep,gps_time_dilution_of_position());
         fprintf(stream,"%c%d",char_sep,no_of_satellites());
         fprintf(stream,"%c%04u",char_sep,gps_status_word1());
         fprintf(stream,"%c%04u",char_sep,gps_status_word2());
         fprintf(stream,"%c%4u",char_sep,test_pattern2());
         fprintf(stream,"%c%4u",char_sep,test_pattern1());
         fprintf(stream,"%c%f",char_sep,elevation_axis_output_voltage());
         fprintf(stream,"%c%f",char_sep,azimuth_axis_output_voltage());
         fprintf(stream,"%c%u",char_sep,checksum());
         fprintf(stream,"%c%u",char_sep,is_checksum_bad());

      }

#if 0
   void SCB::binary_dump(FILE *stream, char char_sep) const
      {
         items_written=fwrite(&width,  sizeof(&width), 1, stream);

      }
#endif

   void SCB::get_utm_values(
      double& northing,double& easting,
      int& ZoneNumber,bool& northern_hemisphere_flag) const
      {
         const int reference_ellipsoid = 23;             //WGS-84

         latlongfunc::LLtoUTM(
            latitude(),longitude(),
            ZoneNumber,northern_hemisphere_flag,
            northing,easting,reference_ellipsoid);
      }

   void SCB::get_utm_values(double& northing, double& easting) const
      {
         int ZoneNumber;
         bool northern_hemisphere_flag;
         get_utm_values(
            northing, easting, ZoneNumber, northern_hemisphere_flag);
      }

   double SCB::utm_northing(void) const
      {
         double northing;
         double easting;
         get_utm_values(northing, easting);
         return northing;
      }

   double  SCB::utm_easting(void) const
      {
         double northing;
         double easting;
         get_utm_values(northing, easting);
         return easting;
      }

   double SCB::velocity_heading(void) const
      {
         double  velocity_angle;

         // this seems to be the right angle but I need to confirm with leo
         velocity_angle=atan2(-y_velocity(),x_velocity());
         velocity_angle=rad2deg(velocity_angle);
         return velocity_angle;
      }

   double SCB::crab_angle(void) const
      {

         double velocity_angle;
         double crab_angle;
         double heading_angle;

         velocity_angle=velocity_heading();

         heading_angle=true_heading();

         crab_angle=velocity_angle-heading_angle;

         return crab_angle;
      }

}                                                 // end of namespace alirt
