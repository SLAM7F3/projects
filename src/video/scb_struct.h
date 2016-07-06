// Last updated on 12/9/04

/********************************************************************
 *
 *
 * Name: params/scb_struct.h
 *
 *
 * Author: Joe Adams
 *
 * Description:
 *    c struct to describe the packets sent from leo's scan control board
 *
 *
 *
 **********************************************************************/

#ifndef _jsa_scb_structs_
#define _jsa_scb_structs_

#include "video/data_types.h"

// set the byte alignment to 1 byte, return it when you are done
// #pragma pack(1)                                   /* one byte */

struct scb_struct
{

    // start gps
    guint16 ins_mode;                             //0
    guint16 ins_time_tag;                         //2

    gint32 x_velocity;                            //4
    gint32 y_velocity;                            //8
    gint32 z_velocity;                            //12

    gint16 platform_az;                           //16
    gint16 roll_angle;                            //18
    gint16 pitch_angle;                           //20

    gint16 roll_rate;                             //22
    gint16 pitch_rate;                            //24
    gint16 yaw_rate;                              //26

    gint16 longitudinal_acceleration;             //28
    gint16 lateral_acceleration;                  //30
    gint16 normal_acceleration;                   //32

    guint16 platform_az_time_tag;                 //34
    guint16 roll_time_tag;                        //36
    guint16 pitch_time_tag;                       //38

    gint16 roll_axis_angular_acceleration;        //40
    gint16 pitch_axis_angular_acceleration;       //42
    gint16 yaw_axis_angular_acceleration;         //44

    guint16 ins_blended_status_word;              //46

    gint32 latitude;                              //48
    gint32 longitude;                             //52
    gint32 ellipsoid_height;                      //56

    gint32 blended_measurement_position_time;     //60
    // end gps packet

    gint32 fsi_packet_id;                         //64

    guint16 azimuth_servo_error;                  //68
    guint16 bench_temperature_1;                  //70

    guint16 elevation_servo_error;                //72
    guint16 bench_temperature_2;                  //74

    guint16 elevation_drive_current;              //76
    guint16 azimuth_drive_current;                //78

    guint16 temperature_2;                        //80
    guint16 temperature_1;                        //82

// Note added on 12/9/04: According to Ryan Poplin, these next two
// fields were originally listed in flipped order.  So we have
// followed Ryan and flipped them so that elevation precedes
// azimuth...

    guint16 elevation_voltage_command;            //86
    guint16 azimuth_voltage_command;              //84

    gint32 ins_packet_id;                         //88

    // guint16 spare1;
    guint16 true_heading;                         //92
    guint16 scan_controller_status;               //94

    gint32 scan_controller_clock;                 //96

    guint16 gps_geometric_dilution_of_position;   //100
    guint16 gps_position_dilution_of_position;    //102
    guint16 gps_horizontal_dilution_of_position;  //104
    guint16 gps_vertical_dilution_of_position;    //106
    guint16 gps_time_dilution_of_position;        //108

    guint16 no_of_satellites;                     //110

    guint16 gps_status_word1;                     //112
    guint16 gps_status_word2;                     //114
    guint16 test_pattern2;                        //116
    guint16 test_pattern1;                        //118

    guint16 azimuth_axis_output_voltage;          //120
    guint16 elevation_axis_output_voltage;        //122

    guint32 checksum;                             //124

};
//128

#pragma pack(4)                                   /* return to 4 bytes */
#endif
