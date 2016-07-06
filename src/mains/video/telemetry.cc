// ========================================================================
// Program TELEMETRY is a linux port of Mike Braun's RawImgFooterview
// C program which runs under Windows.  In performing this port, we
// have benefitted from Jason Cardema's earlier (and well documented!)
// getheader and gettelemetry main programs.  TELEMETRY extracts meta
// information from Group 99 .raw video files.  It creates output
// files times.txt and telemetry.txt.  The former contains date, IRIG
// time and GPS time information.  The latter contains aircraft
// GPS/IMU position and orientation information from packets received
// during the data collect.
// ========================================================================
// Last updated on 8/26/05; 6/21/06; 11/21/07
// ========================================================================

#include <fstream>
#include <iostream>
#include <string>
#include "video/D10GrabberFileHdr.h"
#include "video/Div10ImgInfo.h"
#include "general/filefuncs.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "general/outputfuncs.h"
#include "video/scb.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::setw;
using std::string;

int main(int argc, char* argv[])
{
   FILE* fpImg = fopen( argv[1], "rb" );
   if ( !fpImg )
   {
      cout << "Input raw G99 video file is bad !!" << endl;
      exit(-1);
   }

// First compute origin nearby HAFB for sensor:

   const double FIXED_OFFSET_LONG=-71+17.186/60.;
   const double FIXED_OFFSET_LAT=42+27.905/60.;
   const double FIXED_OFFSET_ALT=50;

   double offset_long = FIXED_OFFSET_LONG;
   double offset_lat = FIXED_OFFSET_LAT;
   double offset_alt = FIXED_OFFSET_ALT;

   bool northern_hemisphere_flag;
   int UTM_zonenumber;
   double UTMNorthing,UTMEasting;
   latlongfunc::LLtoUTM(
      offset_lat,offset_long,UTM_zonenumber,northern_hemisphere_flag,
      UTMNorthing,UTMEasting);
   threevector sensor_offset(UTMEasting,UTMNorthing,offset_alt);
   cout << "Sensor's gross origin = " << sensor_offset << endl;

   tD10GrabberFileHdr* hdrD10in_ptr=new tD10GrabberFileHdr;
   off_t hdr_size=sizeof(*hdrD10in_ptr);
   memset(hdrD10in_ptr,0,hdr_size);
	
// Read input header:

   fseeko(fpImg, 0, SEEK_SET);
   if(fread(hdrD10in_ptr, 1, hdr_size, fpImg) == 0)
   {
      cout << "No elements read in telemetry.cc" << endl;
   }
   

   unsigned int total_bytes_per_raw_image = 
      hdrD10in_ptr->dwBytesPerImgDiv10 + hdrD10in_ptr->wBytesPerImgExtraDiv10;
   cout << "# bytes in raw video file header = " << hdr_size << endl;
   cout << "# bytes in each image = " << hdrD10in_ptr->dwBytesPerImgDiv10
        << endl;
   cout << "# bytes in image footer = " 
        << hdrD10in_ptr->wBytesPerImgExtraDiv10 << endl;
   cout << "# total bytes per raw image = " 
        << total_bytes_per_raw_image << endl;

   unsigned char* pbyImgIn = new unsigned char[total_bytes_per_raw_image];
   tDiv10ImgInfo* info_ptr = (tDiv10ImgInfo *)(
      pbyImgIn + hdrD10in_ptr->dwBytesPerImgDiv10);

// Get total file size in bytes:

   fseeko(fpImg,0,SEEK_END);
   off_t total_raw_file_size=ftello(fpImg);
   unsigned int n_raw_images=
      (total_raw_file_size-hdr_size)/total_bytes_per_raw_image;
   cout << "Number raw images = " << n_raw_images << endl;

   int startFrame,endFrame;
   outputfunc::newline();
   cout << "Enter starting video frame number:" << endl;
   cin >> startFrame;
   cout << "Enter ending video frame number:" << endl;
   cin >> endFrame;

// First skip over raw file's header section:

   fseeko(fpImg,hdr_size,SEEK_SET);

// Next skip over frames 0 through start_frame-1:

   fseeko(fpImg,startFrame*total_bytes_per_raw_image,SEEK_CUR);

   string times_filename="times.txt";
   string telemetry_filename="telemetry.txt";
   string TPA_filename="time_posn_attitude.txt";
   filefunc::deletefile(times_filename);
   filefunc::deletefile(telemetry_filename);

   ofstream time_stream,telemetry_stream,TPA_stream;
   filefunc::openfile(times_filename,time_stream);
   filefunc::openfile(telemetry_filename,telemetry_stream);
   filefunc::openfile(TPA_filename,TPA_stream);

   time_stream.precision(5);
   telemetry_stream.precision(5);
   TPA_stream.precision(7);
   
// Jason Cardema's comments about telemetry information:

/*
 * Extracted fields:
 *   Date       - Date, in yymmdd
 *   Time (UTC) - Timestamp (UTC), in hhmmss.ssssss
 *   GPS Time   - GPS timestamp (UTC), in hhmmss.ssssss
 *   Latitude   - Latitude, in degrees N
 *   Longitude  - Longitude, in degrees E
 *   Lat Acc    - Lateral acceleration, in degrees/second
 *   Long Acc   - Longitudinal acceleration, in degrees/second
 *   Ell Height - Height above WGS84 ellipsoid, in meters
 *   Roll       - UAV roll, in degrees
 *   Pitch      - UAV pitch, in degrees
 *   Yaw        - UAV yaw, in degrees from true north
 *   Heading    - Heading, in degrees from magnetic north
 *   Roll Rate  - UAV roll rate, in degrees/second
 *   Pitch Rate - UAV pitch rate, in degrees/second
 *   Yaw Rate   - UAV yaw rate, in degrees/second
 *   Roll Acc   - UAV roll acceleration, in degrees/second^2
 *   Pitch Acc  - UAV pitch acceleration, in degrees/second^2
 *   Yaw Acc    - UAV yaw acceleration, in degrees/second^2
 *   X Vel      - Velocity in the X direction, in meters/second
 *   Y Vel      - Velocity in the Y direction, in meters/second
 *   Z Vel      - Velocity in the Z direction, in meters/second
 *   Speed      - Magnitude of vector sum of X, Y, Z velocity, in meters/sec
 *   GDOP       - Geometric Dilution of Precision
 *   PDOP       - Position Dilution of Precision
 *   HDOP       - Horizontal Dilution of Precision
 *   VDOP       - Vertical Dilution of Precision
 *   TDOP       - Time Dilution of Precision
 *   Num Sats   - Number of satellites used for GPS position
 */
   
   time_stream << "# Img     DayMonthYear	      Irig time		GPS time" 
               << endl << endl;
   telemetry_stream << 
      "Img#  rel_time   Lat   Lon   rel_X   rel_Y   rel_Z" 
                    << endl;
   telemetry_stream << "  roll   pitch   yaw   n_sats"
                    << endl << endl;
   TPA_stream << "# Img   rel_time   rel_X   rel_Y   rel_Z   roll   pitch   yaw"
              << endl;

   const int column_width=9;
//   const double PRF=24;

   bool first_time_found_flag=false;
   char irig_timestr[32],datestr[32],gps_timestr[32];
//    double irig_time;
   double first_irig_time = 0, rel_irig_time;
   alirt::SCB scbobj;

   for (int i = startFrame; i <= endFrame; i++)
   {
      cout << i << " " << flush;

      fread( pbyImgIn, 1, total_bytes_per_raw_image, fpImg );
      if (info_ptr->uCntPAS <= 0)
      {
         cout << "No telemetry for image i = " << i << endl;
      }

      for (unsigned int j = 0; j < info_ptr->uCntPAS; j++)
      {
         
// Convert the date from IRIG to UTC :

         int numdays = info_ptr->asPA[j].irig_days_bcd;
         videofunc::irig2date(datestr, numdays);
			
// Convert IRIG time to UTC:

         double irig_secs=info_ptr->asPA[j].irig_seconds_17_bit;
         double irig_microsecs=info_ptr->asPA[j].irig_useconds_20_bit;
         double irig_totsecs=irig_secs+irig_microsecs*1E-6;

         videofunc::irig2utc(
            irig_timestr, info_ptr->asPA[j].irig_seconds_17_bit, 
            info_ptr->asPA[j].irig_useconds_20_bit);
         string irig_timestring=irig_timestr;
//          irig_time=stringfunc::string_to_number(irig_timestring);

         if (first_time_found_flag==false)
         {
            first_irig_time=irig_totsecs;
            first_time_found_flag=true;
         }
         rel_irig_time=irig_totsecs-first_irig_time;

// Convert the GPS time to UTC:

         const double BLENDED_TIME_CONSTANT = pow(2.0,-14);
         videofunc::irig2utc(
            gps_timestr, 
            floor(info_ptr->asPA[j].scb.blended_measurement_position_time*
                  BLENDED_TIME_CONSTANT),
            floor(1e6*(info_ptr->asPA[j].scb.blended_measurement_position_time
                       *BLENDED_TIME_CONSTANT - floor(
                          info_ptr->asPA[j].scb.
                          blended_measurement_position_time*
                          BLENDED_TIME_CONSTANT))));
         time_stream << i << "    " << datestr << "\t\t" << irig_timestr 
                     << "\t\t" << gps_timestr << endl;


         double utmX, utmY;
         bool northern_hemisphere_flag;
         int UTM_zonenumber;
         scbobj.stuff(info_ptr->asPA[j].scb);
         scbobj.get_utm_values(
            utmY, utmX, UTM_zonenumber,northern_hemisphere_flag);
         threevector rel_sensor_posn(
            utmX-sensor_offset.get(0),
            utmY-sensor_offset.get(1),
            scbobj.ellipsoid_height()-sensor_offset.get(2));

//         cout.precision(7);
//         cout << "X = " << utmX << " Y = " << utmY
//              << " Z = " << scbobj.ellipsoid_height() << endl;
//              << " roll = " << scbobj.roll_angle()
//              << " pitch = " << scbobj.pitch_angle()
//              << " yaw = " << scbobj.true_heading() << endl;
         
         telemetry_stream << setw(4) << i << "   "
//                          << double(i)/PRF << "   "
                          << setw(column_width) << rel_irig_time << "   "
//                          << irig_timestr << "  "
                          << setw(8)
                          << scbobj.latitude() << "   "
                          << setw(8)
                          << scbobj.longitude() << "   "
                          << setw(8)
                          << rel_sensor_posn.get(0) << "   "
                          << setw(8)
                          << rel_sensor_posn.get(1) << "   "
                          << setw(8)
                          << rel_sensor_posn.get(2) << "   "
//                          << utmX << "  "
//                          << utmY << "  " 
//                          << scbobj.ellipsoid_height() 
                          << endl;
         telemetry_stream << "     " 
                          << setw(8)
                          << scbobj.roll_angle() << "   "
                          << setw(8)
                          << scbobj.pitch_angle() << "   "
                          << setw(8)
                          << scbobj.true_heading() << "   "
                          << setw(5)
                          << scbobj.no_of_satellites() << endl;

         TPA_stream << i << "   "
                    << setw(column_width)
                    << rel_irig_time << "   "
                    << setw(column_width)
                    << rel_sensor_posn.get(0) << "   "
                    << setw(column_width)
                    << rel_sensor_posn.get(1) << "   "
                    << setw(column_width)
                    << rel_sensor_posn.get(2) << "   "
                    << setw(column_width)
                    << scbobj.roll_angle() << "   "
                    << setw(column_width)
                    << scbobj.pitch_angle() << "   "
                    << setw(column_width)
                    << scbobj.true_heading() << "   "
                    << endl;

      } // loop over j index labeling PA structure
   } // loop over index i labeling frame number

   outputfunc::newline();
   filefunc::closefile(times_filename,time_stream);
   filefunc::closefile(telemetry_filename,telemetry_stream);
   filefunc::closefile(TPA_filename,TPA_stream);

   return 0;
}
