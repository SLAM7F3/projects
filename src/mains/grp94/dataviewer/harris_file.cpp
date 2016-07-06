/********************************************************************
 *
 *
 * Name: harris_file.cpp
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * harris file class
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/


#include "harris_file.h"

#define make_key(k,a) keys[k].key=k;keys[k].length=sizeof(a);keys[k].address=&a,keys[k].valid=0,keys[k].element_size=sizeof((&a)[0]);keys[k].number_of_elements=sizeof(a)/sizeof((&a)[0]);
typedef unsigned int uint;
//#define SUB_HEADER_SIZE			40
//#define HEADER_SIZE				2048

const long FILE_HEADER_SIZE=2048;
// default constructor;
harris_file::harris_file(const char * filename)
{
   if(filename!=NULL) set_filename(filename);
   //setSubHeader(filename,1,2,"jigsaw01");
   header_size=FILE_HEADER_SIZE;
   stride=4;
   type_size=sizeof(float);
   descriptions=NULL;
   good=0;
   ByteOrder[4]='\0';
   default_poffset=0;
   set_endian(little);
   updir=2;
   memset(keys,0,sizeof(keys));
   make_key(CAMPAIGN_NUMBER,campaign_number)
      make_key(FLIGHT_NUMBER,flight_number)
      make_key(PASS_NUMBER,pass_number)
      make_key(NUMBER_OF_DATA_ELEMENTS,number_of_data_elements)
      make_key(DATA_ELEMENT_TYPE,data_element_type)
      make_key(BEGIN_FRAME_TIME_STAMP,begin_frame_time_stamp)
      make_key(END_FRAME_TIME_STAMP,end_frame_time_stamp)
      make_key(TARGET_CUE_POSITION,target_cue_position)
      make_key(AOI_REFERENCE_POINT,AOI_reference_point)
      make_key(AOI_DIMENSIONS,AOI_dimensions)
      make_key(VOXEL_DIMENSIONS,voxel_dimensions)
      make_key(SENSOR_POSITION,sensor_position)
      make_key(SENSOR_ORIENTATION,sensor_orientation)
      make_key(SENSOR_FOV,sensor_FOV)
      make_key(PLATFORM_POSITION,platform_position)
      make_key(PLATFORM_ATTITUDE,platform_attitude)
      make_key(PLATFORM_HEADING,platform_heading)
      make_key(GIMBAL_ANGLES,gimbal_angles)
      make_key(APD_NAME,apd_name);
   make_key(FIRMWARE_VERSION_NUMBER,firmware_version_number);
   make_key(CONTROL_SOFTWARE_VERSION_NUMBER,control_software_version_number);
   make_key(CALIBRATION_PROGRAM_VERSION_NUMBER,calibration_program_version_number);
   make_key(NUMBER_OF_PIXELS_IN_FOCAL_PLANE,number_of_pixels_in_focal_plane);
   make_key(PIXEL_FOV,pixel_fov);
   make_key(CMOS_CLOCK_FREQUENCY,cmos_clock_frequency);
   make_key(UNCERTAINTY_IN_CMOS_CLOCK_FREQUENCY,uncertainty_in_cmos_clock_frequency);
   make_key(DETECTION_COUNT_THRESHOLD,detection_count_threshold);
   make_key(MINIMUM_OF_ALL_DATA_POINTS,minimum_of_all_data_points);
   make_key(MAXIMUM_OF_ALL_DATA_POINTS,maximum_of_all_data_points);
   make_key(PAN_ENCODER_SCALE_FACTOR,pan_encoder_scale_factor);
   make_key(TILT_ENCODER_SCALE_FACTOR,tilt_encoder_scale_factor);
   make_key(UNCERTAINTY_OF_PAN_TILT_SCALE_FACTORS,uncertainty_of_pan_tilt_scale_factors);
   make_key(HORIZONTAL_FOV,horizontal_fov);
   make_key(VERTICAL_FOV,vertical_fov);
   make_key(TEMP,stride);
//	calculate_sizes();
}
harris_file::~harris_file()
{
   if(descriptions!=NULL) delete [] descriptions;
}
void harris_file::setSubHeader(const char * filename, int major, int minor, const char * sensorname)
{
   const char * s=strstr(filename,".3dp");
   if(s!=NULL) memcpy(FileName,s-18,24);
   if(sensorname!=NULL) memcpy(SensorName,sensorname,8);
   FileVersionMajor=major;
   FileVersionMinor=minor;
}
void harris_file::setSubHeader(SYSTEMTIME &t, const char * dir,const char * sensorname)
{
   sprintf(FileName,"%04d%02d%02d_%02d%02d%02d%03d.3dp",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
   //char * change;
   //while((change=strchr(FileName,' '))!=NULL) *change='0';
   FileVersionMajor=FILE_VERSION_MAJOR;
   FileVersionMinor=FILE_VERSION_MINOR;
   if(sensorname!=NULL) memcpy(SensorName,sensorname,8);
   char buffer[1024];
#ifdef WIN32 || _WIN32
   sprintf(buffer,"%s\\%s",dir,FileName);
#else
   sprintf(buffer,"%s/%s",dir,FileName);
#endif
//	printf("Creating file %s\n",buffer);
   set_filename(buffer);
}
char harris_file::isGood()
{
   return good;
}
void harris_file::read_descriptions(int length)
{
   if(descriptions!=NULL) delete [] descriptions;
   descriptions=new string[stride];
   char * d=new char[length+1];
	
   fread(d,sizeof(char),length,fp);
   d[length]='\0';
   string s=string(d);
   int cpos=0;
   int lpos=0;
   for(int i=0;i<stride-1;i++){
      cpos=s.find(';');
      descriptions[i]=s.substr(lpos,cpos-lpos);
      lpos=cpos+1;
   }
   descriptions[stride-1]=s.substr(lpos,length-lpos);
}
void harris_file::set_endian(endian_t e){
   base_file::set_endian(e);
   if(e==little){
      char v[5]={'I','I','I','I','\0'};
      memcpy(&ByteOrder,v,sizeof(v));
   }
   else{
      char v[5]={'M','M','M','M','\0'};
      memcpy(&ByteOrder,v,sizeof(v));
   }
}
void harris_file::read_header(void)
{
   open_input_read_only_binary();
   harris_key * curkey;
   int keyread;
   int lengthread;
   move_to_beginning();
   read_binary(ByteOrder,sizeof(char),4,fp);
   good=1;
   if(strcmp(ByteOrder,"IIII")==0)
   {
      printf("Little-Endian File\n");
      set_endian(little);
   }
   else if(strcmp(ByteOrder,"MMMM")==0)
   {
      printf("Big-Endian File\n");
      set_endian(big);
   }
   else{
//		printf("BAD Harris File\n");
      good=0;
      close();
      return;
   }
   char updirchar='z';
   printf(".3dp File Detected\n");
   read_binary(&FileVersionMajor,sizeof(short),1,fp);
   read_binary(&FileVersionMinor,sizeof(short),1,fp);
   read_binary(&SensorName,sizeof(char),8,fp);
   read_binary(&FileName,sizeof(char),24,fp);
   int i=FILE_SUB_HEADER_SIZE;
   int key=0;
   int length=0;
   while(i<FILE_HEADER_SIZE){
      read_binary(&keyread,sizeof(keyread),1,fp);
      read_binary(&lengthread,sizeof(lengthread),1,fp);
      i+=sizeof(keyread)+sizeof(lengthread);
      if((curkey=getKey(keyread))==NULL)
      {
         skip_forward(lengthread);
      }
      else{
         if(curkey->key==NODATA) break;
         else{
            read_binary(curkey->address,curkey->element_size,curkey->number_of_elements,fp);
            curkey->valid=1;
         }
      }
      i+=length;
   }
   if(updirchar=='z') updir=2;
   int fn_data_points=(file_size()-FILE_HEADER_SIZE)/stride/sizeof(float);
   if(number_of_data_elements!=fn_data_points) printf("Data points according to header=%d, Data points found in file=%d\n",number_of_data_elements,fn_data_points);
   n_data_points=number_of_data_elements;
   data_size=n_data_points*type_size*stride;
   close();
   //calculate_sizes();
}
float * harris_file::read_data(long max_points)
{

   open_input_read_only_binary();
   float *data;
   long read_data_size=data_size;
   long read_n_data_points=n_data_points;
   if(max_points>0 && n_data_points>max_points)
   {
      read_data_size=max_points*stride*type_size;
      read_n_data_points=max_points;
   }
   data = new float[read_data_size/type_size];//malloc(data_size);
   if(data!=NULL)
   {
      printf("Allocated %d bytes for data\n",read_data_size);
   }
   else
   {
      //exit(1);
   }
   move_to_data_beginning();    
   //(*read_function)(data,read_data_size,1,fp);
   read_binary(data,type_size,read_data_size/type_size,fp);
   printf("Read all points into memory\n");
   int p=stride-1;
   if(data_element_type==1){ // float float float unsigned int
      printf("Converting data to all floats\n");
      for(int i=0;i<read_n_data_points;i++){
         ((float*)data)[p]=(float)(((unsigned int *)data)[p]); // convert last element in each tuple to float
         p+=stride;
      }
   }
   else if(data_element_type!=1000){//data_element_type==2 || data_element_type==3){
      printf("Unsupported data format %d\n",data_element_type);
      exit(1);
   }
   close();
   return data;

}

harris_key * harris_file::getKey(int k){
   int i;
   for(i=0;i<sizeof(keys)/sizeof(keys[0]);i++){
      if(keys[i].key==k) return &keys[i];
   }
   return NULL;
}

void harris_file::setKey(int k, const void * value){
   harris_key * thekey=getKey(k);
   memcpy(thekey->address,value,thekey->element_size*thekey->number_of_elements);
   thekey->valid=1;
}
void harris_file::getKeyValue(int k, void * value){
   harris_key * thekey=getKey(k);
   memcpy(value,thekey->address,thekey->element_size*thekey->number_of_elements);
}
void harris_file::writeHeader(){
   int i;
   move_to_beginning();
   fwrite(ByteOrder,sizeof(char),4,fp);
   write_binary(&FileVersionMajor,sizeof(short),1,fp);
   write_binary(&FileVersionMinor,sizeof(short),1,fp);
   write_binary(&SensorName,sizeof(char),8,fp);
   write_binary(&FileName,sizeof(char),24,fp);
   for(i=1;i<sizeof(keys)/sizeof(keys[0]);i++){
      if(keys[i].valid){
         //	printf("Valid Key %d\n",i);
         //	if(keys[i].key==DATA_ELEMENT_TYPE) printf("DATA_ELEMENT_TYPE = %d\n",*((int*)(keys[i].address)));
         write_binary(&(keys[i].key),sizeof(keys[i].key),1,fp);
         write_binary(&(keys[i].length),sizeof(keys[i].length),1,fp);
         write_binary(keys[i].address,keys[i].element_size,keys[i].number_of_elements,fp);
      }
   }
   move_to_data_beginning();
   skip_forward(-1);
   char zero=0;
   write_binary(&zero,1,1,fp);
}
void harris_file::writePoints(float * xyzh, long num_points){
   int i;
   int temp;
   int det=1;
   harris_key thiskey;
   if(get_current_pos()<FILE_HEADER_SIZE) move_to(FILE_HEADER_SIZE);
   fwrite(xyzh,sizeof(float),num_points*4,fp);
   move_to(FILE_HEADER_SIZE+3);
   thiskey=*getKey(DATA_ELEMENT_TYPE);
   if(!thiskey.valid) setKey(DATA_ELEMENT_TYPE,&det);
   getKeyValue(DATA_ELEMENT_TYPE,&det);
   if(det==1){
      for(i=0;i<num_points;i++){
         temp=xyzh[i*4];
         write_binary(&temp,sizeof(float),1,fp);
         skip_forward(3);
      }
   }
}

void harris_file::writePointsSOA(float * xyzh,long num_points){
   int i,j,temp;
   for(i=0;i<3;i++){
      for(j=0;j<num_points;j++){
         write_binary(xyzh+j*4+i,sizeof(float),1,fp);
      }
   }
   for(j=0;j<num_points;j++){
      temp=xyzh[j*4+3];
      write_binary(&temp,sizeof(float),1,fp);
   }
}
void harris_file::getViewPoint(float &x, float &y, float &z)
{
   x=sensor_position[0];
   y=sensor_position[1];
   z=sensor_position[2];
}
