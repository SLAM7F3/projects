// =========================================================================
// Imagecdf class member function definitions
// =========================================================================
// Last modified on 7/18/06; 7/20/06; 8/2/06; 8/22/06; 10/8/11
// =========================================================================

#include <iostream>
#include <set>
#include <netcdf.h>	// Network common data form used for image storage
#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
#include "astro_geo/geopoint.h"
#include "space/imagecdf.h"
#include "image/imagefuncs.h"

#include "general/outputfuncs.h"
#include "space/satellite.h"
#include "space/satelliteimage.h"
#include "space/satellitepass.h"
#include "astro_geo/ground_radar.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void imagecdf::allocate_member_objects()
{
}

void imagecdf::initialize_member_objects()
{
   nc_id=-1;
   unix_compressed_flag=false;
   pass_ptr=NULL;
   ground_radar_ptr=NULL;
}		 

// ---------------------------------------------------------------------
imagecdf::imagecdf(string sat_name,satellitepass* satpass_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   target_name=sat_name;
   pass_ptr=satpass_ptr;
   ground_radar_ptr=pass_ptr->get_ground_radar_ptr();
}

// ---------------------------------------------------------------------
// Copy constructor:

imagecdf::imagecdf(const imagecdf& i)
{
   docopy(i);
}

imagecdf::~imagecdf()
{
}

// ---------------------------------------------------------------------
void imagecdf::docopy(const imagecdf& i)
{
}

// Overload = operator:

imagecdf& imagecdf::operator= (const imagecdf& i)
{
   if (this==&i) return *this;
   docopy(i);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const imagecdf& i)
{
   outstream << endl;
   return(outstream);
}

// =========================================================================
// Parsing member functions
// =========================================================================

// Member function select_file queries the user to specify the full
// filename for an input imagecdf file.  

void imagecdf::select_file(string input_filename)
{
   if (!filefunc::fileexist(input_filename))
   {
      cout << "No such imagecdf file exists!   Try again..." << endl;
      exit(-1);
   }
   else
   {
      filename=input_filename;
   }

// On 12/27/01, Ken Fields told us that he has started to compress
// imagecdf files using the generic Unix compression utility which
// yields files with a ".Z" suffix.  Fields also says that XELIAS can
// accomodate such compressed files.  We therefore first check to see
// whether the incoming imagecdf file has a ".Z" suffix:

   string suffix=filename.substr(filename.length()-2,2);
   if (suffix==".Z")
   {
      filename=filename.substr(0,filename.length()-2);
   }

// Passname = netcdf filename with ".imagecdf" suffix removed:

   int dot_posn=filename.find_first_of(".",0);
   passname=filename.substr(0,dot_posn);
}

// ---------------------------------------------------------------------
void imagecdf::select_file(
   bool input_param_file,string inputline[],unsigned int& currlinenumber)
{
   bool valid_choice;
   vector<string> outputline;

   do
   {

// FAKE FAKE: for program development purposes, we hardwire in name of
// Deena's imagecdf...4/27/06 at 12:23 pm.

      outputline.push_back(
         "Enter full pathname for input imagecdf file:");
      filename=stringfunc::mygetstring(
         outputline,input_param_file,inputline,currlinenumber);

//      filename="SJ7.imagecdf";
      valid_choice=(filefunc::fileexist(filename));
      if (!valid_choice)
      {
         cout << "No such imagecdf file exists!   Try again..." << endl;
      }
   }
   while (!valid_choice);

// On 12/27/01, Ken Fields told us that he has started to compress
// imagecdf files using the generic Unix compression utility which
// yields files with a ".Z" suffix.  Fields also says that XELIAS can
// accomodate such compressed files.  We therefore first check to see
// whether the incoming imagecdf file has a ".Z" suffix:

   string suffix=filename.substr(filename.length()-2,2);
   if (suffix==".Z")
   {
      filename=filename.substr(0,filename.length()-2);
   }
   
// Passname = netcdf filename with ".imagecdf" suffix removed:

   int dot_posn=filename.find_first_of(".",0);
   passname=filename.substr(0,dot_posn);

//   cout << "passname = " << passname << endl;
}

// ---------------------------------------------------------------------
// Member function readin_file first uncompresses any input imagecdf
// file which is unix compressed (and ends with a ".Z" suffix").  It
// then reads in the header information stored at the beginning of the
// imagecdf file which contains data pertaining to the entire pass.
// It next reads in image header information (e.g. satellite
// elevation, azimuth, etc) which pertain to each individual image
// within the pass.  Finally, it reads in image intensity information
// and stores it within a myimage member twoDarray.

bool imagecdf::readin_file(bool regularize_images)
{

// On 12/27/01, Ken Fields told us that he has started to compress
// imagecdf files using the generic Unix compression utility which
// yields files with a ".Z" suffix.  Fields also says that XELIAS can
// accomodate such compressed files.  We therefore first check to see
// whether the incoming imagecdf file has a ".Z" suffix.  If so, we
// uncompress it before reading it in...

   string uncompressed_prefix;
   unix_compressed_flag=sysfunc::unix_uncompress(
      filename+".Z",uncompressed_prefix);

   bool imagecdf_exists=filefunc::fileexist(filename);
   if (imagecdf_exists)
   {
      readin_headerinfo();
      for (int imagenumber=0; imagenumber<number_of_images; imagenumber++)
      {
         satelliteimage* satimage_ptr=new satelliteimage(pass_ptr);
         pass_ptr->get_satimage_ptrs().push_back(satimage_ptr);
      }
      readin_data();
   }

   if (unix_compressed_flag) sysfunc::unix_compress(uncompressed_prefix);
   return imagecdf_exists;
}

// ---------------------------------------------------------------------
// Member function readin_headerinfo opens up an imagecdf file.  It
// then reads in the common header information which is target
// independent.  It places this header info into various member
// variables.

void imagecdf::readin_headerinfo()
{
   int status=nc_open(filename.c_str(),NC_NOWRITE,&nc_id);
   if (status != NC_NOERR) 
   {
      cout << "Netcdf error in imagecdf::readin_headerinfo()!" 
           << endl;
      cout << nc_strerror(status) << endl;
      exit(-1);
   }

// Get dimension info:

   int zdim_id,cdim_id;
   size_t zdim,cdim;
   nc_inq_dimid(nc_id,"zdim",&zdim_id);
   nc_inq_dimid(nc_id,"cdim",&cdim_id);
   nc_inq_dimlen(nc_id,zdim_id,&zdim);
   nc_inq_dimlen(nc_id,cdim_id,&cdim);
   number_of_images=int(zdim);
//   cout << "number_of_images = " << number_of_images << endl;
//   cout << "cdim = " << cdim << endl;

// Read in global attribute info:

   size_t xelias_type_length,srcname_length,objnum_length,
      imagery_motiontype_length;
   nc_type xeliasfile_type,sourcename,objectnumber,imagery_motiontype;

   nc_inq_att(nc_id,NC_GLOBAL,"file_type",&xeliasfile_type,
              &xelias_type_length);
   int xeliasfile_type_length=int(xelias_type_length);
   char* xeliasfile_type_cstr=(char *) malloc(xeliasfile_type_length+10);
   nc_get_att_text(nc_id,NC_GLOBAL,"file_type",xeliasfile_type_cstr);
   xeliasfile_type_str.assign(xeliasfile_type_cstr,0,xeliasfile_type_length);
   free(xeliasfile_type_cstr);

   nc_inq_att(nc_id,NC_GLOBAL,"source_name",&sourcename,&srcname_length);
   int sourcename_length=int(srcname_length);
   char* sourcename_cstr=(char *) malloc(sourcename_length+10);
   nc_get_att_text(nc_id,NC_GLOBAL,"source_name",sourcename_cstr);
   string radarname_str;
   radarname_str.assign(sourcename_cstr,0,sourcename_length);
   free(sourcename_cstr);

   nc_inq_att(nc_id,NC_GLOBAL,"object_number",&objectnumber,&objnum_length);
   objnum_length=int(objnum_length);
   char* objnum_cstr=(char *) malloc(objnum_length+10);
   nc_get_att_text(nc_id,NC_GLOBAL,"object_number",objnum_cstr);
   object_number_str.assign(objnum_cstr,0,objnum_length);
   free(objnum_cstr);

   nc_inq_att(nc_id,NC_GLOBAL,"motion_type",&imagery_motiontype,
              &imagery_motiontype_length);
   imagery_motiontype_length=int(imagery_motiontype_length);
   char* imagery_motiontype_cstr=
      (char *) malloc(imagery_motiontype_length+10);
   nc_get_att_text(nc_id,NC_GLOBAL,"motion_type",imagery_motiontype_cstr);
   string imagery_motiontype_str;
   imagery_motiontype_str.assign(
      imagery_motiontype_cstr,0,imagery_motiontype_length);
   if (imagery_motiontype_str=="prosol_motion")
   {
      imagery_motion_type=motionfunc::prosol;
   }
   else if (imagery_motiontype_str=="general_ypr")
   {
      imagery_motion_type=motionfunc::general_ypr;
   }
   else if (imagery_motiontype_str=="dynamic_motion")
   {
      imagery_motion_type=motionfunc::dynamic_motion;
   }
   free(imagery_motiontype_cstr);

   double radar_long,radar_lat,radar_alt;
   nc_get_att_double(nc_id,NC_GLOBAL,"longitude",&radar_long);
   nc_get_att_double(nc_id,NC_GLOBAL,"latitude",&radar_lat);
   nc_get_att_double(nc_id,NC_GLOBAL,"height",&radar_alt);
   geopoint radar_geoposn(radar_long,radar_lat,radar_alt);
   ground_radar_ptr->set_geolocation(radar_geoposn);

   cout << "radar_geoposn = " << radar_geoposn << endl;

//   char* date_cstr;
//   size_t d_length;
//   int date_length;
//   nc_type date;
//   nc_inq_att(nc_id,NC_GLOBAL,"date",&date,&d_length);
//   date_length=int(d_length);
//   date_cstr=(char *) malloc(date_length+10);
//   nc_get_att_text(nc_id,NC_GLOBAL,"date",date_cstr);
//   starting_date_and_midtime_str.assign(date_cstr,0,date_length);
//   free(date_cstr);

// Read in variable info:

   int midtime_id,image_center_band_id,bandwidth_id;
   double* midtime_val=new double[number_of_images];
   double* image_center_band_val=new double[number_of_images];
   double* bandwidth_val=new double[number_of_images];
   nc_inq_varid(nc_id,"zdata",&midtime_id);
   nc_inq_varid(nc_id,"image_center_band",&image_center_band_id);
   nc_inq_varid(nc_id,"bandwidth",&bandwidth_id);
   nc_get_var_double(nc_id,midtime_id,midtime_val);
   nc_get_var_double(nc_id,image_center_band_id,image_center_band_val);
   nc_get_var_double(nc_id,bandwidth_id,bandwidth_val);

// In July 02, we discovered that bandwidth information is not saved
// in every imagecdf file (e.g. MMW files).  Moreover, we found that
// sometimes bandwidth values are patently absurd (2 x 10**15 Hz !)
// So we are forced to phenomenologically estimate the bandwidth from
// center frequency information:

   ground_radar_ptr->set_bandwidth(0.1*image_center_band_val[0]);  // Hz

// As of Aug 03, we need to set maximum slew rate information within
// the radar object for automatic statevector improvement purposes:

   ground_radar_ptr->set_max_slew_rate();
   
// Save starting and ending dates and midtimes for pass within
// starting_date_and_midtime_str & ending_date_and_midtime_str member
// variables:

   int pass_date_id;
   nc_inq_varid(nc_id,"pass_date",&pass_date_id);
   nc_get_var_int(nc_id,pass_date_id,&pass_date);
   time_t starting_passdate_time=time_t(pass_date+midtime_val[0]);
   time_t ending_passdate_time=time_t(
      pass_date+midtime_val[number_of_images-1]);

   tm* starting_passdate_time_tm_ptr=gmtime(&starting_passdate_time);
   char* starting_date_and_midtime_cstr=
      asctime(starting_passdate_time_tm_ptr);

// Note: Purify indicated on 1/13/04 that we ought to include the
// following deallocation line.  Yet as of 1/14/04, we choose to
// ignore Purify's complaint and live with a potential memory leak.

//   delete starting_passdate_time_tm_ptr;

   starting_date_and_midtime_str=starting_date_and_midtime_cstr;

   tm* ending_passdate_time_tm_ptr=gmtime(&ending_passdate_time);
   char* ending_date_and_midtime_cstr=
      asctime(ending_passdate_time_tm_ptr);
   ending_date_and_midtime_str=ending_date_and_midtime_cstr;

   delete [] midtime_val;
   delete [] image_center_band_val;
   delete [] bandwidth_val;

   starting_date_and_midtime_str=asctime(gmtime(&starting_passdate_time));
   ending_date_and_midtime_str=asctime(gmtime(&ending_passdate_time));

   cout << "UTC for pass selected = " << starting_date_and_midtime_str;
   cout << "Object number for pass selected = " << object_number_str
        << endl;
   cout << "Number of images = " << number_of_images << endl << endl;
}

// ---------------------------------------------------------------------
// Boolean member function readin_data first reads in image (as
// opposed to pass) header information and places it into various
// satellite image member variables.  It then reads in image intensity
// data from the imagecdf file and loads it into previously allocated
// rhimage or a2auimage memory locations.  Finally, this member
// function finds all "oversized" images whose pixel counts in the
// horizontal or vertical directions exceed myimage::Nx_max and
// myimage::Ny_max respectively.  Such raw images are immediately
// subsampled so that their pixel size does not exceed myimage::Nx_max
// x myimage::Ny_max.

void imagecdf::readin_data()
{

// Get dimension info:

   int cdim_id,ddim_id,xdim_id,ydim_id;
   size_t cdim,ddim,xdim,ydim;
   nc_inq_dimid(nc_id,"cdim",&cdim_id);
   nc_inq_dimid(nc_id,"ddim",&ddim_id);
   nc_inq_dimid(nc_id,"xdim",&xdim_id);
   nc_inq_dimid(nc_id,"ydim",&ydim_id);
   nc_inq_dimlen(nc_id,cdim_id,&cdim);
   nc_inq_dimlen(nc_id,ddim_id,&ddim);
   nc_inq_dimlen(nc_id,xdim_id,&xdim);
   nc_inq_dimlen(nc_id,ydim_id,&ydim);
//   cout << "cdim = " << cdim << endl;
//   cout << "ddim = " << ddim << endl;
//   cout << "xdim = " << xdim << endl;
//   cout << "ydim = " << ydim << endl;

// Read in variable info:

   int height_id,width_id,starttime_id,midtime_id,stoptime_id;
   int position_id,velocity_id;
   nc_inq_varid(nc_id,"height",&height_id);
   nc_inq_varid(nc_id,"width",&width_id);
   nc_inq_varid(nc_id,"start_time",&starttime_id);
   nc_inq_varid(nc_id,"zdata",&midtime_id);
   nc_inq_varid(nc_id,"stop_time",&stoptime_id);
   nc_inq_varid(nc_id,"position",&position_id);
   nc_inq_varid(nc_id,"velocity",&velocity_id);

   int azimuth_id,elevation_id,azdot_id,eldot_id;
   int range_id,rangedot_id,x_size_id,y_size_id;
   int x_scale_id,x_center_id,y_center_id,ddata_id;
   nc_inq_varid(nc_id,"azimuth",&azimuth_id);
   nc_inq_varid(nc_id,"elevation",&elevation_id);
   nc_inq_varid(nc_id,"azimuth_rate",&azdot_id);
   nc_inq_varid(nc_id,"elevation_rate",&eldot_id);
   nc_inq_varid(nc_id,"range",&range_id);
   nc_inq_varid(nc_id,"range_rate",&rangedot_id);
   nc_inq_varid(nc_id,"x_size",&x_size_id);
   nc_inq_varid(nc_id,"y_size",&y_size_id);
   nc_inq_varid(nc_id,"x_scale",&x_scale_id);
   nc_inq_varid(nc_id,"x_center",&x_center_id);
   nc_inq_varid(nc_id,"y_center",&y_center_id);
   nc_inq_varid(nc_id,"ddata",&ddata_id);

   short height_val[number_of_images];
   short width_val[number_of_images];
   double starttime_val[number_of_images];
   double midtime_val[number_of_images];
   double stoptime_val[number_of_images];
   double position_val[number_of_images*cdim];
   double velocity_val[number_of_images*cdim];
   nc_get_var_short(nc_id,height_id,height_val);
   nc_get_var_short(nc_id,width_id,width_val);
   nc_get_var_double(nc_id,starttime_id,starttime_val);
   nc_get_var_double(nc_id,midtime_id,midtime_val);
   nc_get_var_double(nc_id,stoptime_id,stoptime_val);
   nc_get_var_double(nc_id,position_id,position_val);
   nc_get_var_double(nc_id,velocity_id,velocity_val);

   double azimuth_val[number_of_images];
   double elevation_val[number_of_images];
   double azdot_val[number_of_images];
   double eldot_val[number_of_images];
   double range_val[number_of_images];
   double rangedot_val[number_of_images];
   float x_size_val[number_of_images];
   float y_size_val[number_of_images];
   float x_scale_val[number_of_images];
   float x_center_val[number_of_images];
   float y_center_val[number_of_images];
   float *ddata_val=new float[number_of_images*ydim*xdim*ddim];

   nc_get_var_double(nc_id,azimuth_id,azimuth_val);
   nc_get_var_double(nc_id,elevation_id,elevation_val);
   nc_get_var_double(nc_id,azdot_id,azdot_val);
   nc_get_var_double(nc_id,eldot_id,eldot_val);
   nc_get_var_double(nc_id,range_id,range_val);
   nc_get_var_double(nc_id,rangedot_id,rangedot_val);
   nc_get_var_float(nc_id,x_size_id,x_size_val);
   nc_get_var_float(nc_id,y_size_id,y_size_val);
   nc_get_var_float(nc_id,x_scale_id,x_scale_val);
   nc_get_var_float(nc_id,x_center_id,x_center_val);
   nc_get_var_float(nc_id,y_center_id,y_center_val);
   nc_get_var_float(nc_id,ddata_id,ddata_val);

   int peak_amplitude_id;
   float peak_amplitude_val[number_of_images];
   if (xeliasfile_type_str=="IMAGE_COMPRESSED")
   {
      nc_inq_varid(nc_id,"peak_amplitude",&peak_amplitude_id);      
      nc_get_var_float(nc_id,peak_amplitude_id,peak_amplitude_val);
   }

   string banner="Loading "+stringfunc::number_to_string(number_of_images)
      +" raw images from imagecdf file:";
   outputfunc::write_banner(banner);
   for (int i=0; i<number_of_images; i++)
   {
      cout << i+1 << " " << flush;

      vector<satelliteimage*> currimage_ptr=pass_ptr->get_satimage_ptrs();
      currimage_ptr[i]->set_imagenumber(i+1);
      currimage_ptr[i]->set_times(
         starttime_val[i],stoptime_val[i],midtime_val[i],
         midtime_val[i]-midtime_val[0]);

      satellite* curr_sat_ptr=currimage_ptr[i]->get_target_ptr();
      curr_sat_ptr->set_time(midtime_val[i]);
      curr_sat_ptr->set_az_el_range(threevector(
         azimuth_val[i],elevation_val[i],range_val[i]));
      curr_sat_ptr->set_az_el_range_dot(threevector(
         azdot_val[i],eldot_val[i],rangedot_val[i]));

      currimage_ptr[i]->set_x_conversion(x_scale_val[i]);
      currimage_ptr[i]->set_p_center(x_center_val[i],y_center_val[i]);
//      cout << "image number = " << i 
//           << " range = " << range_val[i]  
//           << " px_center = " << currimage_ptr[i]->px_center
//           << " py_center = " << currimage_ptr[i]->py_center 
//           << endl;

// On 5/31/01, we learned (to our horror!) from George Zogbi that
// state vector information is stored within imagecdf files in MAXLIK
// rather than ECI coordinates!  (Forrest Hunsberger reconfirmed on
// 8/2/01 that state vector information is reported in ALL imagecdf
// files in MAXLIK coordinates.)  These two coordinate systems are
// related to one another via an azimuthal rotation in the xy plane by
// the Greenwich sidereal time.  After reading in state vector
// information, we immediately transform it to ECI coordinates:

      curr_sat_ptr->set_position(threevector(
         position_val[i*cdim+0],position_val[i*cdim+1],
         position_val[i*cdim+2]));
      curr_sat_ptr->set_velocity(threevector(
         velocity_val[i*cdim+0],velocity_val[i*cdim+1],
         velocity_val[i*cdim+2]));

      curr_sat_ptr->set_position(geofunc::convert_MAXLIK_to_ECI_coords(
         pass_date,currimage_ptr[i]->get_midtime(),
         curr_sat_ptr->get_position()));
      curr_sat_ptr->set_velocity(geofunc::convert_MAXLIK_to_ECI_coords(
         pass_date,currimage_ptr[i]->get_midtime(),
         curr_sat_ptr->get_velocity()));

// On 11/9/00, we verified that the position and velocity state
// vectors for AU/A2 orbits are typically less than 0.5 degree off
// from forming a perfect right angle.  On 5/31/01, we verified for 3
// nominal RH passes that the the RH orbits are typically less than
// 0.1 degrees off from forming a perfect right angle.

      cout << curr_sat_ptr->get_position().get(0) << "  "
           << curr_sat_ptr->get_position().get(1) << "  "
           << curr_sat_ptr->get_position().get(2) << endl;

//      cout << "i = " << i
//           << " sat X = " << curr_sat_ptr->get_position().get(0)
//           << " sat Y = " << curr_sat_ptr->get_position().get(1)
//           << " sat Z = " << curr_sat_ptr->get_position().get(2)
//           << " |sat posn| = " 
//	   << currimage_ptr[i]->satellite_statevector.position.magnitude()
//           << endl;
      
//           << " |sat vel| = "
//           << currimage_ptr[i]->satellite_statevector.velocity.magnitude() 
//	   << endl;
//      costheta=currimage_ptr[i]->satellite_statevector.position.
//		unitvector().dot(
//         satellite_statevector.velocity[i].unitvector());
//      cout << "Angle between r and v vectors = " 
//           << acos(costheta)*180/PI << endl;
//      newline();

// Dynamically allocate memory for current raw image within pass:

      currimage_ptr[i]->set_z2Darray_orig_ptr(
         new twoDarray(width_val[i],height_val[i]));
      currimage_ptr[i]->get_z2Darray_orig_ptr()->set_deltax(
         x_size_val[i]*x_scale_val[i]);
      currimage_ptr[i]->get_z2Darray_orig_ptr()->set_deltay(y_size_val[i]);
      currimage_ptr[i]->get_z2Darray_orig_ptr()->init_coord_system();


      pair<int,int> image_center=currimage_ptr[i]->get_p_center();
      twoDarray* ztwoDarray_ptr=currimage_ptr[i]->get_z2Darray_orig_ptr();
      double dx=ztwoDarray_ptr->get_deltax();
      double dy=ztwoDarray_ptr->get_deltay();
      
      cout.precision(8);
      cout << "image number = " << i << endl;
      cout << "dimensions in pixels = " << ztwoDarray_ptr->get_mdim() 
           << " x " << ztwoDarray_ptr->get_ndim() << endl;
      cout << "pixel size = " << dx << " x " << dy << endl;
      cout << "center shift = " << image_center.first*dx << " x "
           << image_center.second*dy << endl;
      cout << endl;

// We learned from James Wanken on 1/10/01 that XELIAS data files can
// be written in two forms.  In the case where the "file_type" global
// attribute is set equal to "IMAGE", the data corresponds to the
// amplitude information which ranges from 0 to 1.  Alternatively when
// the "file_type" global attribute equals "IMAGE_COMPRESSED",
// floating point INTENSITY (rather than amplitude!) information is
// mapped onto integer values via the relation

// intensity_compressed = 0 + [(2^15-1) - 0]/(2^8)*(intensity_float -
// 					            min_intensity)

// where min_intensity = peak_intensity - 256 dB.  Note: 2^15 = 32768.
// Unfortunately, peak intensity values are stored within the MISNAMED
// imagecdf variable peak_amplitude.  

// On 5/8/01, we learned from James Wanken and Konstantin that netcdf
// null flags generally appear only within portions of the xdim x ydim
// array lying outside the upper left hand nxbins x nybins corner
// which contains genuine image information.  In general, nxbins <
// xdim and nybins < ydim.  So images generally have xdim*ydim -
// nxbins*nybins garbage pixels whose values are typically set equal
// to the netcdf null flag.  To avoid headaches, we should not read in
// these garbage pixels!

      if (xeliasfile_type_str=="IMAGE")
      {
         for (unsigned int n=0; n<currimage_ptr[i]->get_z2Darray_orig_ptr()->
                 get_ndim(); n++)
         {
            for (unsigned int m=0; m<currimage_ptr[i]->get_z2Darray_orig_ptr()
                    ->get_mdim(); m++)
            {
               currimage_ptr[i]->get_z2Darray_orig_ptr()->put(
                  m,n,mathfunc::dB(sqr(ddata_val[i*ydim*xdim*ddim
                                                +n*xdim*ddim+m*ddim])));
            }  // row index m
         }  // column index n
      }
      else if (xeliasfile_type_str=="IMAGE_COMPRESSED")
      {

// Forrest Hunsberger told us on 4/11/01 that (squared
// amplitude,phase) ordered pairs are stored in COMPRESSED imagecdf
// files if ddim==2.  We simply disregard the phase information.

         double curr_value;
         double min_intensity=peak_amplitude_val[i]-256;	// dB!
         for (unsigned int n=0; n<currimage_ptr[i]->get_z2Darray_orig_ptr()->
                 get_ndim(); n++)
         {
            for (unsigned int m=0; m<currimage_ptr[i]->
                    get_z2Darray_orig_ptr()->get_mdim(); m++)
            {
               double compressed_value=ddata_val[
                  i*ydim*xdim*ddim+n*xdim*ddim+m*ddim];

               if (compressed_value < 0)

               {
                  curr_value=POSITIVEINFINITY;	// dB !
               }
               else
               {
                  curr_value=min_intensity+256.0/(32767.0-0.0)*
                     (compressed_value-0.0);	// dB !
               }
               currimage_ptr[i]->get_z2Darray_orig_ptr()->put(m,n,curr_value);
            }	// row index m 
         } // column index n
      }
      else
      {
         cout << "Error inside imagecdf::readin_data() !" << endl;
         cout << "xeliasfile_type_str = " << xeliasfile_type_str << endl;
      }	// xeliasfile_type_str conditional

// Following Dave Chan's suggestion, we flip the rows within the
// *z2Darray_orig_ptr twoDarray so that satellites look "down range"
// rather than "up range":

      currimage_ptr[i]->get_z2Darray_orig_ptr()->flip_upside_down();

// In order to generate G99 video file output which can be viewed with
// our OSG video player program, we renormalize the raw image
// intensities so that they range from 0 to 255:

//      const double renorm_zmin=0.0;	// traditional lower RCS limit
//      const double renorm_zmax=66.0;	// traditional upper RCS limit
      const double renorm_zmin=0.0;
      const double renorm_zmax=255.0;
      currimage_ptr[i]->renormalize_raw_image_intensities(
         renorm_zmin,renorm_zmax);

      currimage_ptr[i]->compute_ARIES_image_center(
         currimage_ptr[i]->get_z2Darray_orig_ptr());
   } // loop over index i labeling image number


/*
// Set reference time and position along target's orbit:

   orbit.set_reference_posn_and_time(
      currimage_ptr[0]->satellite_statevector.position,
      currimage_ptr[0]->midtime);
*/

   outputfunc::newline();
   nc_close(nc_id);
   delete [] ddata_val;
}

// ---------------------------------------------------------------------
// Member function writeout_file first copies the contents of the
// input imagecdf file into an output imagecdf file whose name ends
// with a specified suffix.  It then transfers the values within the z
// image array to the new imagecdf file.  This method is meant to be
// called whenever an entire pass of raw imagery has been cleaned and
// a new imagecdf file needs to be created for subsequent image
// processing.  The output imagecdf file is automatically unix
// compressed in order to save disk space, independent of whether the
// input imagecdf was unix compressed or not.

void imagecdf::writeout_file(string filename_descriptor)
{
   outputfunc::write_banner("Writing out imagecdf file:");
   
   int nc_id,ddim_id,xdim_id,ydim_id,ddata_id;
   int peak_amplitude_id;
   float peak_amplitude_val[number_of_images];
   double min_intensity,compressed_value;
//    double uncompressed_value;
   size_t ddim,xdim,ydim;

// On 12/27/01, Ken Fields told us that he has started to compress
// imagecdf files using the generic Unix compression utility which
// yields files with a ".Z" suffix.  Fields also says that XELIAS can
// accomodate such compressed files.  We therefore first check to see
// whether the incoming imagecdf file has a ".Z" suffix.  If so, we
// uncompress it before reading it in...

   string uncompressed_prefix;
   unix_compressed_flag=sysfunc::unix_uncompress(
      filename+".Z",uncompressed_prefix);

// Copy input imagecdf file onto output imagecdf file:

   string prefix=stringfunc::prefix(filefunc::getbasename(filename));
   string output_filename=prefix+"_"+filename_descriptor+".imagecdf";
   string unixcommandstr="cp "+filename+" "+output_filename;
   sysfunc::unix_command(unixcommandstr);

   nc_open(output_filename.c_str(),NC_WRITE,&nc_id);

// If the original input imagecdf file was compressed, write out the
// current peak intensity information stored within the MISNAMED
// imagecdf variable peak_amplitude to the new output imagecdf file:

   vector<satelliteimage*> currimage_ptr=pass_ptr->get_satimage_ptrs();

   if (xeliasfile_type_str=="IMAGE_COMPRESSED")
   {
      nc_inq_varid(nc_id,"peak_amplitude",&peak_amplitude_id);      

      for (int image_number=0; image_number<number_of_images; image_number++)
      {
         currimage_ptr[image_number]->minmax_zarray_values(
            currimage_ptr[image_number]->get_z2Darray_ptr());
         peak_amplitude_val[image_number]=currimage_ptr[image_number]->
            get_max_z();
      }
      nc_put_var_float(nc_id,peak_amplitude_id,peak_amplitude_val);      
   }

// Get dimension info:

   nc_inq_dimid(nc_id,"ddim",&ddim_id);
   nc_inq_dimid(nc_id,"xdim",&xdim_id);
   nc_inq_dimid(nc_id,"ydim",&ydim_id);
   nc_inq_dimlen(nc_id,ddim_id,&ddim);
   nc_inq_dimlen(nc_id,xdim_id,&xdim);
   nc_inq_dimlen(nc_id,ydim_id,&ydim);
   float *ddata_val=new float[number_of_images*ydim*xdim*ddim];
//   float ddata_val[number_of_images*ydim*xdim*ddim];
  
// Write out z array data:

   nc_inq_varid(nc_id,"ddata",&ddata_id);
   for (int image_number=0; image_number<number_of_images; image_number++)
   {
     
// Flip rows within *z2Darray_ptr before writing out data to imagecdf
// file:

      twoDarray* zflip_twoDarray_ptr=new twoDarray(
         *currimage_ptr[image_number]->get_z2Darray_ptr());
      zflip_twoDarray_ptr->flip_upside_down();

      for (int j=0; j<int(ydim); j++)
      {
         for (int i=0; i<int(xdim); i++)
         {
            if (xeliasfile_type_str=="IMAGE")
            {
               ddata_val[image_number*ydim*xdim*ddim+j*xdim*ddim+i*ddim]=
                  sqrt(mathfunc::dBinv(zflip_twoDarray_ptr->get(i,j)));
            }
            else if (xeliasfile_type_str=="IMAGE_COMPRESSED")
            {
               min_intensity=peak_amplitude_val[image_number]-256;	// dB!
               compressed_value=0+(32767.0-0.0)/256.0*
                  (zflip_twoDarray_ptr->get(i,j)-min_intensity); 
									// dB!
//               uncompressed_value=min_intensity+256.0/(32767.0-0.0)*
//                  (compressed_value-0.0);	// dB !
               ddata_val[image_number*ydim*xdim*ddim+j*xdim*ddim+i*ddim]
                  =compressed_value;
            } // xeliasfile_type_str conditional
         } // i loop
      } // j loop
      delete zflip_twoDarray_ptr;
   } // image number loop
   nc_put_var_float(nc_id,ddata_id,ddata_val);
   nc_close(nc_id);
   delete [] ddata_val;

// Unix compress input imagecdf if it was originally compressed:

   if (unix_compressed_flag) sysfunc::unix_compress(uncompressed_prefix);

// Unix compress output imagecdf regardless of whether input imagecdf
// was unix compressed or not:

   sysfunc::unix_compress(output_filename);
}
