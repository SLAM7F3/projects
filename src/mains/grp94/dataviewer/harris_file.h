/********************************************************************
 *
 *
 * Name: harris_file.h
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * Harris ICD file class
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/

#ifndef HARRIS_FILE
#define HARRIS_FILE

typedef unsigned int uint;

#include "SYSTEMTIME.h"
#include "xyzp_file.h"
#define FILE_VERSION_MAJOR 1
#define FILE_VERSION_MINOR 3
#define FILE_SUB_HEADER_SIZE 40
#define NODATA					0
#define CAMPAIGN_NUMBER			1
#define FLIGHT_NUMBER			2
#define PASS_NUMBER				3
#define NUMBER_OF_DATA_ELEMENTS 4
#define DATA_ELEMENT_TYPE		5
#define BEGIN_FRAME_TIME_STAMP	6
#define END_FRAME_TIME_STAMP	7
#define TARGET_CUE_POSITION		8
#define AOI_REFERENCE_POINT		9
#define AOI_DIMENSIONS			10
#define VOXEL_DIMENSIONS		11
#define SENSOR_POSITION			12
#define SENSOR_ORIENTATION		13
#define SENSOR_FOV				14
#define PLATFORM_POSITION		15
#define PLATFORM_ATTITUDE		16
#define PLATFORM_HEADING		17
#define GIMBAL_ANGLES			18

#define APD_NAME 1000
#define FIRMWARE_VERSION_NUMBER 1001
#define CONTROL_SOFTWARE_VERSION_NUMBER 1002
#define CALIBRATION_PROGRAM_VERSION_NUMBER 1003
#define NUMBER_OF_PIXELS_IN_FOCAL_PLANE 1004
#define PIXEL_FOV 1005
#define CMOS_CLOCK_FREQUENCY 1006
#define UNCERTAINTY_IN_CMOS_CLOCK_FREQUENCY 1007
#define DETECTION_COUNT_THRESHOLD 1008
#define MINIMUM_OF_ALL_DATA_POINTS 1009
#define MAXIMUM_OF_ALL_DATA_POINTS 1010
#define PAN_ENCODER_SCALE_FACTOR 1100
#define TILT_ENCODER_SCALE_FACTOR 1101
#define UNCERTAINTY_OF_PAN_TILT_SCALE_FACTORS 1102
#define HORIZONTAL_FOV 1103
#define VERTICAL_FOV 1104

#define TEMP					1900
#define DESCRIPTIONS			1901

#define NUMBER_OF_KEYS	4000
#define NUMBER_OF_USED_KEYS		19

typedef struct{
	int key;
	int length;
	void * address;
	int valid;
	int element_size;
	int number_of_elements;
	int fp;
}harris_key;



class harris_file : public xyzp_file
{
    public:
    
  harris_file::harris_file(const char * filename=NULL);   //constructor

	char ByteOrder[5];
	short FileVersionMajor;
	short FileVersionMinor;
	char SensorName[8];
	char FileName[24];
	string * descriptions;
//	int default_poffset;

//    int ntuple_order;
//    int n_ntuples;
//    int n_data_points;
//	int data_point_size_bytes;
//    int file_size_bytes;
//    double file_size_Mbytes;
	
    float * read_data(long max_points=0);
//    void calculate_sizes(void);
	void read_header(void);
	char isGood(void);
	
	harris_file::~harris_file(void);

	void getKeyValue(int k, void * value);
	void setKey(int k, const void * value);
//	void readHeader();
	void writeHeader();
	void writePoints(float * xyzh, long num_points);
	void writePointsSOA(float * xyzh,long num_points);
	void setSubHeader(const char * filename, int major, int minor, const char * sensorname);
	void setSubHeader(SYSTEMTIME &t, const char * dir, const char * sensorname);
	void harris_file::set_endian(endian_t e);
	virtual void getViewPoint(float &x, float &y, float &z);
	private:
	uint campaign_number;
	uint flight_number;
	uint pass_number;
	uint number_of_data_elements;
	uint data_element_type;
	short begin_frame_time_stamp[8];
	short end_frame_time_stamp[8];
	double target_cue_position[3];
	double AOI_reference_point[3];
	float AOI_dimensions[3];
	float voxel_dimensions[3];
	double sensor_position[3];
	double sensor_orientation[3];
	double sensor_FOV;
	double platform_position[3];
	double platform_attitude[3];
	double platform_heading;
	double gimbal_angles[2];
	char apd_name[8];
	uint firmware_version_number;
	uint control_software_version_number;
	uint calibration_program_version_number;
	uint number_of_pixels_in_focal_plane[2];
	float pixel_fov[2];
	float cmos_clock_frequency;
	float uncertainty_in_cmos_clock_frequency;
	uint detection_count_threshold;
	float minimum_of_all_data_points[3];
	float maximum_of_all_data_points[3];
	float pan_encoder_scale_factor;
	float tilt_encoder_scale_factor;
	float uncertainty_of_pan_tilt_scale_factors;
	double horizontal_fov;
	double vertical_fov;

	harris_key keys[NUMBER_OF_KEYS];

	harris_key * getKey(int k);
	
	void read_descriptions(int length);
	char good;
		
};






#endif
