// =========================================================================
// Xsens_Ins class member function definitions
// =========================================================================
// Last modified on 7/7/11; 8/16/11; 8/18/11; 8/24/11
// =========================================================================

#include "math/constant_vectors.h"
#include "filter/filterfuncs.h"
#include "general/outputfuncs.h"
#include "ins/xsens_ins.h"

#include "xsens/cmtdef.h"
#include "xsens/xsens_time.h"
#include "xsens/xsens_list.h"
#include "xsens/cmtscan.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;
using namespace xsens;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void xsens_ins::allocate_member_objects()
{
}

void xsens_ins::initialize_member_objects()
{
   rapid_lateral_accel_detected_flag=false;
   msg_counter=0;
   az_el_roll_counter=0;
   lat_lon_alt_counter=0;
   GPS_SignPostsGroup_ptr=NULL;

//   cout << "Enter port name (e.g. /dev/ttyUSB0)" << endl;
//   cin >> port_name;

   port_name="/dev/ttyUSB0";
   CM_3D_ptr=NULL;

   mtCount=0;
//   nmax_raw_measurements=2;
//   nmax_raw_measurements=5;
   nmax_raw_measurements=10;
//   nmax_raw_measurements=20;
//   nmax_raw_measurements=30;
   curr_az_el_roll=Zero_vector;
   prev_t_select=NEGATIVEINFINITY;
}		 

// ---------------------------------------------------------------------
xsens_ins::xsens_ins()
{
   initialize_member_objects();
   allocate_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

xsens_ins::xsens_ins(const xsens_ins& x)
{
//   cout << "inside xsens_ins copy constructor, this(xsens_ins) = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();
   docopy(x);
}

xsens_ins::~xsens_ins()
{
}

// ---------------------------------------------------------------------
void xsens_ins::docopy(const xsens_ins& x)
{
//   cout << "inside xsens_ins::docopy()" << endl;
//   cout << "this = " << this << endl;
}

// Overload = operator:

xsens_ins& xsens_ins::operator= (const xsens_ins& x)
{
//   cout << "inside xsens_ins::operator=" << endl;
//   cout << "this(xsens_ins) = " << this << endl;
   if (this==&x) return *this;
   docopy(x);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

   ostream& operator<< (ostream& outstream,const xsens_ins& x)
   {
      outstream << endl;
//   outstream << "Xsens_Ins ID = " << s.ID << endl;
      outstream << endl << endl;
   
      return outstream;
   }

// =========================================================================
bool xsens_ins::initialize_xsens()
{
   cout << "Connect to the MT, configure it for euler output at 100Hz" << endl;
   cout << " and read data messages until Control-C is pressed" << endl;

   if (serial.open(port_name.c_str(), B115200) != XRV_OK)
      return EXIT_ERROR("open");
	
   msg.setMessageId(CMT_MID_GOTOCONFIG);
   cout << "Putting MT in config mode" << endl;
   if (serial.writeMessage(&msg))
      return EXIT_ERROR("goto config");
   if (serial.waitForMessage(&reply, CMT_MID_GOTOCONFIGACK, 0,  1) != XRV_OK)
      return EXIT_ERROR("goto config ack");
   cout << "MT now in config mode" << endl;

   msg.setMessageId(CMT_MID_SETPERIOD);
   msg.setDataShort(1152);
   if (serial.writeMessage(&msg))
      return EXIT_ERROR("set period");
   if (serial.waitForMessage(&reply, CMT_MID_SETPERIODACK, 0,  1) != XRV_OK)
      return EXIT_ERROR("set period ack");
   cout << "Period is now set to 100 Hz" << endl;

   msg.setMessageId(CMT_MID_SETOUTPUTMODE);
   msg.setDataShort(CMT_OUTPUTMODE_POSITION | CMT_OUTPUTMODE_ORIENT);
//   msg.setDataShort(CMT_OUTPUTMODE_ORIENT);
   if (serial.writeMessage(&msg))
      return EXIT_ERROR("set output mode");
   if (serial.waitForMessage(&reply, CMT_MID_SETOUTPUTMODEACK, 0,  1) != XRV_OK)
      return EXIT_ERROR("set output mode ack");
   cout << "Output mode is now set to orientation" << endl;

   msg.setMessageId(CMT_MID_SETOUTPUTSETTINGS);
   msg.setDataLong(CMT_OUTPUTSETTINGS_ORIENTMODE_EULER | 
   CMT_OUTPUTSETTINGS_TIMESTAMP_SAMPLECNT);
   if (serial.writeMessage(&msg))
      return EXIT_ERROR("set output settings");
   if (serial.waitForMessage(&reply, CMT_MID_SETOUTPUTSETTINGSACK, 0,  1) 
   != XRV_OK)
      return EXIT_ERROR("set output settings ack");
   cout << "Output settings now set to euler + timestamp" << endl;

   msg.setMessageId(CMT_MID_GOTOMEASUREMENT);
   msg.resizeData(0);
   if (serial.writeMessage(&msg))
      return EXIT_ERROR("goto measurement");
   if (serial.waitForMessage(&reply, CMT_MID_GOTOMEASUREMENTACK, 0,  1) 
   != XRV_OK)
      return EXIT_ERROR("goto measurement ack");
   cout << "Now in measurement mode, Time of day = " << getTimeOfDay() 
        << endl;

   t_start=getTimeOfDay();
   return true;
}

// ---------------------------------------------------------------------
bool xsens_ins::EXIT_ERROR(string loc) 
{
   cout << "Error " << serial.getLastResult() 
        << " occured during " << loc 
        << " : " 
        << xsensResultText(serial.getLastResult()) << endl;
   return false;
}

// ---------------------------------------------------------------------
void xsens_ins::initialize_xsens_2()
{
   cout << "inside xsens_ins::initialize_xsens_2()" << endl;
//   outputfunc::enter_continue_char();
   
   // Perform hardware scan
   mtCount = doHardwareScan(cmt3);
//   cout << "mtCount = " << mtCount << endl;

   CmtOutputMode mode;
   CmtOutputSettings settings;
   getUserInputs(mode,settings);
   doMtSettings(cmt3, mode, settings);
}

// ---------------------------------------------------------------------
void xsens_ins::update_ins_metadata()
{
//   cout << "inside xsens_ins::update_ins_metadata()" << endl;

   if (serial.waitForMessage(&reply, 0, 0, 1) != XRV_OK)
      EXIT_ERROR("read data message");

   msg_counter++;
   timestamp=getTimeOfDay();

   double roll=(double) reply.getDataFloat(0*4);  
   double pitch=(double) reply.getDataFloat(1*4);  
   double yaw=(double) reply.getDataFloat(2*4);   
//   cout << "roll = " << roll << " pitch = " << pitch << " yaw = " << yaw
//        << endl;

/*
   double a=reply.getDataFloat(3*4);
   double b=reply.getDataFloat(4*4);
   double c=reply.getDataFloat(5*4);
   double d=reply.getDataFloat(6*4);
   double e=reply.getDataFloat(7*4);
   
   cout << "a = " << a
        << " b = " << b
        << " c = " << c << endl;
   cout << "d = " << d << " e = " << e << endl;
*/
   
}

// ---------------------------------------------------------------------
void xsens_ins::update_ins_metadata_2()
{
//   cout << "inside xsens_ins::update_ins_metadata_2()" << endl;

   Packet* packet = new Packet((unsigned short) mtCount, cmt3.isXm());
   cmt3.waitForDataMessage(packet);

   CmtVector positionLLA=packet->getPositionLLA();
   double curr_raw_lat=positionLLA.m_data[0];
   double curr_raw_lon=positionLLA.m_data[1];
   double curr_raw_alt=positionLLA.m_data[2];
   
//   cout << "raw_lat = " << curr_raw_lat
//        << " raw_lon = " << curr_raw_lon
//        << " raw_alt = " << curr_raw_alt << endl;

//   CmtVector lla;
//   cmt3.getLatLonAlt(lla);
//   double curr_lat=lla.m_data[0];
//   double curr_lon=lla.m_data[1];
 //  double curr_alt=lla.m_data[2];
   
   if (mathfunc::my_isnan(curr_raw_lat) || mathfunc::my_isnan(curr_raw_lon)
   || mathfunc::my_isnan(curr_raw_alt))
   {
      cout << "NAN in raw geolocation detected!" << endl;
      return;
   }

   raw_lat.push_back(curr_raw_lat);
   raw_lon.push_back(curr_raw_lon);
   raw_alt.push_back(curr_raw_alt);
   if (raw_lat.size() > nmax_raw_measurements) raw_lat.pop_front();
   if (raw_lon.size() > nmax_raw_measurements) raw_lon.pop_front();
   if (raw_alt.size() > nmax_raw_measurements) raw_alt.pop_front();

   CmtCalData caldata;
   CmtEuler euler_data;
   CmtMatrix matrix_data;

   for (unsigned int i = 0; i < mtCount; i++) 
   {	

//      cout << "i = " << i << " mtCount = " << mtCount << endl;
/*


      euler_data = packet->getOriEuler(i);
      double theta_x=euler_data.m_roll;
      double theta_y=euler_data.m_pitch;
      double theta_z=euler_data.m_yaw;
      theta=threevector(theta_x,theta_y,theta_z);
      cout << "theta_x = " << theta_x 
           << " theta_y = " << theta_y 
           << " theta_z = " << theta_z 
           << endl;
*/

      matrix_data = packet->getOriMatrix(i);
      R.put(0,0,matrix_data.m_data[0][0]);
      R.put(0,1,matrix_data.m_data[0][1]);
      R.put(0,2,matrix_data.m_data[0][2]);

      R.put(1,0,matrix_data.m_data[1][0]);
      R.put(1,1,matrix_data.m_data[1][1]);
      R.put(1,2,matrix_data.m_data[1][2]);

      R.put(2,0,matrix_data.m_data[2][0]);
      R.put(2,1,matrix_data.m_data[2][1]);
      R.put(2,2,matrix_data.m_data[2][2]);

      R=R.transpose();

      ins_xhat=threevector(R.get(0,0),R.get(1,0),R.get(2,0));
      ins_yhat=threevector(R.get(0,1),R.get(1,1),R.get(2,1));
      ins_zhat=threevector(R.get(0,2),R.get(1,2),R.get(2,2));
      
/*
      cout << "ins_xhat = " 
           << ins_xhat.get(0) << " "
           << ins_xhat.get(1) << " "
           << ins_xhat.get(2) << endl;

      cout << "ins_yhat = " 
           << ins_yhat.get(0) << " "
           << ins_yhat.get(1) << " "
           << ins_yhat.get(2) << endl;

      cout << "ins_zhat = " 
           << ins_zhat.get(0) << " "
           << ins_zhat.get(1) << " "
           << ins_zhat.get(2) << endl;
*/

// Az = rotation angle about ins_zhat
// El = rotation angle about -ins_yhat
// Roll = rotation angle about ins_xhat
    
//      cout << "R = " << R << endl;
      double curr_az,curr_el,curr_roll;
      R.az_el_roll_from_rotation(curr_az,curr_el,curr_roll);
//      cout << "raw az = " << curr_az*180/PI
//           << " raw el = " << curr_el*180/PI
//           << " raw roll = " << curr_roll*180/PI << endl;

      if (mathfunc::my_isnan(curr_az) || mathfunc::my_isnan(curr_el)
          || mathfunc::my_isnan(curr_roll))
      {
         cout << "NAN in raw rotation angles detected!" << endl;
//         initialize_xsens();
//         initialize_xsens_2();
//         cout << "XSENS reset..." << endl;
         return;
      }

      raw_az.push_back(curr_az);
      raw_el.push_back(curr_el);
      raw_roll.push_back(curr_roll);

      if (raw_az.size() > nmax_raw_measurements) raw_az.pop_front();
      if (raw_el.size() > nmax_raw_measurements) raw_el.pop_front();
      if (raw_roll.size() > nmax_raw_measurements) raw_roll.pop_front();

/*
      caldata = packet->getCalData(i);

      double accel_x_ins=caldata.m_acc.m_data[0];
      double accel_y_ins=caldata.m_acc.m_data[1];
      const double g=9.812687357684514;
      double accel_z_ins=caldata.m_acc.m_data[2]-g;

      if (mathfunc::my_isnan(accel_x_ins) ||
          mathfunc::my_isnan(accel_y_ins) ||
          mathfunc::my_isnan(accel_z_ins) )
      {
         cout << "NAN in raw acceleration detected!" << endl;
         return;
      }

      raw_accel_phys.push_back(
         accel_x_ins*ins_xhat + accel_y_ins*ins_yhat + accel_z_ins*ins_zhat);
      
      if (raw_accel_phys.size() > nmax_raw_measurements) 
         raw_accel_phys.pop_front();

//      cout << "accel_x_ins = " << accel_x_ins
//           << " accel_y_ins = " << accel_y_ins
//           << " accel_z_ins = " << accel_z_ins
//           << endl; 

//      cout << "raw_accel_phys = " << raw_accel_phys.back() << endl;
*/

   } // i < mtCount for loop
}

// =========================================================================
// Member function lifted from example_linux.cpp
// =========================================================================

// Member function doHardwareScan() checks available COM ports and
// scans for MotionTrackers

int xsens_ins::doHardwareScan(xsens::Cmt3& cmt3)
{
   XsensResultValue res;
   List<CmtPortInfo> portInfo;
   unsigned long portCount = 0;
   int mtCount;
	
//   printw("Scanning for connected Xsens devices...");
   xsens::cmtScanPorts(portInfo);
   portCount = portInfo.length();
//   printw("done\n");

   if (portCount == 0) {
//      printw("No MotionTrackers found\n\n");
      return 0;
   }

   for(int i = 0; i < (int)portCount; i++) {	
//      printw("Using COM port %s at ", portInfo[i].m_portName);
		
      switch (portInfo[i].m_baudrate) {
//         case B9600  : printw("9k6");   break;
//         case B19200 : printw("19k2");  break;
//         case B38400 : printw("38k4");  break;
//         case B57600 : printw("57k6");  break;
//         case B115200: printw("115k2"); break;
//         case B230400: printw("230k4"); break;
//         case B460800: printw("460k8"); break;
//         case B921600: printw("921k6"); break;
//         default: printw("0x%lx", portInfo[i].m_baudrate);
      }
//      printw(" baud\n\n");
   }

//   printw("Opening ports...");
   //open the port which the device is connected to and connect at the device's baudrate.
   for(int p = 0; p < (int)portCount; p++){
      res = cmt3.openPort(portInfo[p].m_portName, portInfo[p].m_baudrate);
//      EXIT_ON_ERROR(res,"cmtOpenPort");  
   }
//   printw("done\n\n");

   //get the Mt sensor count.
//   printw("Retrieving MotionTracker count (excluding attached Xbus Master(s))\n");
   mtCount = cmt3.getMtCount();
//   printw("MotionTracker count: %d\n\n", mtCount);

   // retrieve the device IDs 
//   printw("Retrieving MotionTrackers device ID(s)\n");
   for(int j = 0; j < mtCount; j++){
      res = cmt3.getDeviceId((unsigned char)(j+1), deviceIds[j]);
//      EXIT_ON_ERROR(res,"getDeviceId");
//      printw("Device ID at busId %i: %08lx\n\n",j+1,(long) deviceIds[j]);
   }
	
   return mtCount;
}

// ---------------------------------------------------------------------
// Method getUserInputs() queries the user for output data

void xsens_ins::getUserInputs(CmtOutputMode &mode, CmtOutputSettings &settings)
{

//   mode = CMT_OUTPUTMODE_CALIB | CMT_OUTPUTMODE_ORIENT;
   mode = CMT_OUTPUTMODE_POSITION | CMT_OUTPUTMODE_ORIENT;
//   settings = CMT_OUTPUTSETTINGS_ORIENTMODE_EULER;
   settings = CMT_OUTPUTSETTINGS_ORIENTMODE_MATRIX;
   settings |= CMT_OUTPUTSETTINGS_TIMESTAMP_SAMPLECNT;
}

// ---------------------------------------------------------------------
// Member function doMTSettings() sets user settings in MTi/MTx
// Assumes initialized global MTComm class

void xsens_ins::doMtSettings(
   xsens::Cmt3 &cmt3, CmtOutputMode &mode, 
   CmtOutputSettings &settings) 
{
   unsigned long mtCount = cmt3.getMtCount();

   // set sensor to config sate
   XsensResultValue res= cmt3.gotoConfig();
//   EXIT_ON_ERROR(res,"gotoConfig");

   unsigned short sampleFreq = cmt3.getSampleFrequency();

   // set the device output mode for the device(s)
//   printw("Configuring your mode selection\n");

   for (unsigned int i = 0; i < mtCount; i++) 
   {
      CmtDeviceMode deviceMode(mode, settings, sampleFreq);
      if ((deviceIds[i] & 0xFFF00000) != 0x00500000) {
         // not an MTi-G, remove all GPS related stuff
         deviceMode.m_outputMode &= 0xFF0F;
      }
      res = cmt3.setDeviceMode(deviceMode, true, deviceIds[i]);
//      EXIT_ON_ERROR(res,"setDeviceMode");
   }

   // start receiving data
   res = cmt3.gotoMeasurement();
//   EXIT_ON_ERROR(res,"gotoMeasurement");
}

// =========================================================================
// Time-averaged XSENS output
// =========================================================================

// Member function update_avg_az_el_roll()

void xsens_ins::update_avg_az_el_roll()
{
   if (raw_az.size() < nmax_raw_measurements) return;

   prev_az_el_roll=curr_az_el_roll;
   
   curr_az_el_roll=Zero_vector;
   for (int i=0; i<raw_az.size(); i++)
   {
      double curr_raw_az=raw_az[i];
//      cout << "i = " << i << " curr_raw_az = " << curr_raw_az << endl;

      curr_raw_az=basic_math::phase_to_canonical_interval(
         curr_raw_az,prev_az_el_roll.get(0)-PI,prev_az_el_roll.get(0)+PI);

      double curr_raw_el=raw_el[i];
      curr_raw_el=basic_math::phase_to_canonical_interval(
         curr_raw_el,prev_az_el_roll.get(1)-PI,prev_az_el_roll.get(1)+PI);

      double curr_raw_roll=raw_roll[i];
      curr_raw_roll=basic_math::phase_to_canonical_interval(
       curr_raw_roll,prev_az_el_roll.get(2)-PI,prev_az_el_roll.get(2)+PI);

      curr_az_el_roll += threevector(curr_raw_az,curr_raw_el,curr_raw_roll);
   }
   curr_az_el_roll /= raw_az.size();

//   cout << "curr_az = " << curr_az_el_roll.get(0)*180/PI
//        << " curr_el = " << curr_az_el_roll.get(1)*180/PI
//        << " curr_roll = " << curr_az_el_roll.get(2)*180/PI 
//        << endl;

//   cout << "prev_az = " << prev_az_el_roll.get(0)*180/PI
//        << " prev_el = " << prev_az_el_roll.get(1)*180/PI
//        << " prev_roll = " << prev_az_el_roll.get(2)*180/PI << endl;

//   cout << "d_az = " << get_daz() << endl;

/*
   if (CM_3D_ptr != NULL)
   {
      CM_3D_ptr->set_mouse_input_device_flag(false);
      CM_3D_ptr->reset_az_el(get_daz(),get_del());
      CM_3D_ptr->set_mouse_input_device_flag(true);
   }
*/

}

// ---------------------------------------------------------------------
// Member function update_median_az_el_roll()

void xsens_ins::update_median_az_el_roll()
{
//   cout << "inside xsens_ins::update_median_az_el_roll()" << endl;
   
   if (raw_az.size() < nmax_raw_measurements) return;

   prev_az_el_roll=curr_az_el_roll;
   
   vector<double> az_values,el_values,roll_values;
   for (int i=0; i<raw_az.size(); i++)
   {
      double curr_raw_az=raw_az[i];

      curr_raw_az=basic_math::phase_to_canonical_interval(
         curr_raw_az,prev_az_el_roll.get(0)-PI,prev_az_el_roll.get(0)+PI);
      az_values.push_back(curr_raw_az);

      double curr_raw_el=raw_el[i];
      curr_raw_el=basic_math::phase_to_canonical_interval(
         curr_raw_el,prev_az_el_roll.get(1)-PI,prev_az_el_roll.get(1)+PI);
      el_values.push_back(curr_raw_el);

      double curr_raw_roll=raw_roll[i];

      curr_raw_roll=basic_math::phase_to_canonical_interval(
       curr_raw_roll,prev_az_el_roll.get(2)-PI,prev_az_el_roll.get(2)+PI);
      roll_values.push_back(curr_raw_roll);

//      cout << "i = " << i 
//           << " raw az = " << curr_raw_az*180/PI
//           << " raw roll = " << curr_raw_roll*180/PI << endl;
   }
   curr_az_el_roll=threevector(
      mathfunc::median_value(az_values),
      mathfunc::median_value(el_values),
      mathfunc::median_value(roll_values));

//   cout << "curr_az = " << curr_az_el_roll.get(0)*180/PI
//        << " curr_el = " << curr_az_el_roll.get(1)*180/PI
//        << " curr_roll = " << curr_az_el_roll.get(2)*180/PI 
//        << endl;

//   cout << "prev_az = " << prev_az_el_roll.get(0)*180/PI
//        << " prev_el = " << prev_az_el_roll.get(1)*180/PI
//        << " prev_roll = " << prev_az_el_roll.get(2)*180/PI << endl;

//   cout << "d_az = " << get_daz() << endl;

/*
   if (CM_3D_ptr != NULL)
   {
      CM_3D_ptr->set_mouse_input_device_flag(false);
      CM_3D_ptr->reset_az_el(get_daz(),get_del());
      CM_3D_ptr->set_mouse_input_device_flag(true);
   }
*/

}

// ---------------------------------------------------------------------
// Member function update_median_lat_lon_alt()

void xsens_ins::update_median_lat_lon_alt()
{
//   cout << "update_median_lat_lon_alt()" << endl;
   
   if (raw_lat.size() < nmax_raw_measurements) return;

   prev_lat_lon_alt=curr_lat_lon_alt;

   vector<double> lat_values,lon_values,alt_values;
   for (int i=0; i<raw_lat.size(); i++)
   {
      lat_values.push_back(raw_lat[i]);
      lon_values.push_back(raw_lon[i]);
      alt_values.push_back(raw_alt[i]);
   }
   
   curr_lat_lon_alt=threevector(
      mathfunc::median_value(lat_values),
      mathfunc::median_value(lon_values),
      mathfunc::median_value(alt_values));

//   cout << "curr_lat = " << curr_lat_lon_alt.get(0)
//        << " curr_lon = " << curr_lat_lon_alt.get(1)
//        << " curr_roll = " << curr_lat_lon_alt.get(2)
//        << endl;

//   cout << "prev_lat = " << prev_lat_lon_alt.get(0)
//        << " prev_el = " << prev_lat_lon_alt.get(1)
//        << " prev_roll = " << prev_lat_lon_alt.get(2) << endl;
   
}

// ---------------------------------------------------------------------
// Member function update_avg_physical_acceleration()

void xsens_ins::update_avg_physical_acceleration()
{
//   cout << "inside xsens_ins::update_avg_physical_acceleration()" << endl;

   const int n_recent_measurements=10;

   int n_stop=raw_accel_phys.size();
   if (n_stop < n_recent_measurements) return;
   int n_start=n_stop-n_recent_measurements;

   curr_phys_accel=Zero_vector;
   for (int i=n_start; i<n_stop; i++)
   {
      curr_phys_accel += raw_accel_phys[i];
   }
   curr_phys_accel /= n_recent_measurements;
   
   twovector lateral_accel_phys(curr_phys_accel);
//   cout << "lateral_accel_magnitude = " << lateral_accel_phys.magnitude()
//        << endl;

   double lateral_accel_magnitude=lateral_accel_phys.magnitude();

   const double threshold_accel_magnitude=1;	// m / sec**2
//   const double threshold_accel_magnitude=2;	// m / sec**2
   if (lateral_accel_magnitude > threshold_accel_magnitude)
   {
      rapid_lateral_accel_detected_flag=true;
      string banner="Rapid lateral acceleration detected!";
      outputfunc::write_big_banner(banner);
//      outputfunc::enter_continue_char();

      const double min_dt_since_prev_selection=5;	// secs
      double t_curr=get_elapsed_time();
      if (t_curr-prev_t_select > min_dt_since_prev_selection &&
          CM_3D_ptr != NULL)
      {
         CM_3D_ptr->set_hmi_select_flag(true);
         prev_t_select=get_elapsed_time();
      }
   }
}

// ---------------------------------------------------------------------
// Member function alpha_filter_az_el_roll()

void xsens_ins::alpha_filter_az_el_roll()
{
//   cout << "inside xsens_ins::alpha_filter_az_el_roll()" << endl;
   
// Perform alpha filtering of raw orientation measurements.  Recall
// smaller values for alpha imply more filtering with previously
// measured values:

//   const double alpha=0.01;
//   const double alpha=0.03;
   const double alpha=0.1;
//   const double alpha=0.3;
//   const double alpha=0.5;
   
   if (az_el_roll_counter==0)
   {
      curr_alpha_filtered_az_el_roll=curr_az_el_roll;
   }
   else
   {
      curr_alpha_filtered_az_el_roll=filterfunc::alpha_filter(
         curr_az_el_roll,prev_alpha_filtered_az_el_roll,alpha);
   }   

   if (CM_3D_ptr != NULL)
   {
      CM_3D_ptr->set_mouse_input_device_flag(false);
      CM_3D_ptr->reset_az_el(get_daz(),get_del());
      CM_3D_ptr->set_mouse_input_device_flag(true);
   }

   prev_alpha_filtered_az_el_roll=curr_alpha_filtered_az_el_roll;
   az_el_roll_counter++;
}
// ---------------------------------------------------------------------
// Member function alpha_filter_lat_lon_alt()

void xsens_ins::alpha_filter_lat_lon_alt()
{
//   cout << "inside xsens_ins::alpha_filter_lat_lon_alt()" << endl;
   
// Perform alpha filtering of raw GPS measurements.  Recall smaller
// values for alpha imply more filtering with previously measured
// values:

//   const double alpha=0.01;
//   const double alpha=0.03;
//   const double alpha=0.1;
//   const double alpha=0.2;
   const double alpha=0.5;
//   const double alpha=0.999;
   
   if (lat_lon_alt_counter==0)
   {
      curr_alpha_filtered_lat_lon_alt=curr_lat_lon_alt;
   }
   else
   {
      curr_alpha_filtered_lat_lon_alt=filterfunc::alpha_filter(
         curr_lat_lon_alt,prev_alpha_filtered_lat_lon_alt,alpha);
   }   

   prev_alpha_filtered_lat_lon_alt=curr_alpha_filtered_lat_lon_alt;
   lat_lon_alt_counter++;

   if (GPS_SignPostsGroup_ptr==NULL) return;

   double curr_lat=curr_alpha_filtered_lat_lon_alt.get(0);
   double curr_lon=curr_alpha_filtered_lat_lon_alt.get(1);
   geopoint GPSpoint(curr_lon,curr_lat);
   threevector curr_GPS_posn=GPSpoint.get_UTM_posn();
   
   SignPost* GPS_SignPost_ptr=GPS_SignPostsGroup_ptr->get_SignPost_ptr(0);
   GPS_SignPost_ptr->set_UVW_coords(
      GPS_SignPostsGroup_ptr->get_curr_t(),
      GPS_SignPostsGroup_ptr->get_passnumber(),curr_GPS_posn);

//   cout << "lat = " << curr_lat << " lon = " << curr_lon << endl;
//   cout << "Easting = " << curr_GPS_posn.get(0)
//        << " Northing = " << curr_GPS_posn.get(1) << endl;
}
