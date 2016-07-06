// =========================================================================
// Photograph class member function definitions
// =========================================================================
// Last modified on 4/25/13; 8/16/13; 12/27/13; 3/8/14
// =========================================================================

#include <iostream>
#include <vector>
#include <Magick++.h>
#include <osgDB/ReadFile>

#include "image.hpp"
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "templates/mytemplates.h"
#include "video/photograph.h"
#include "general/stringfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void photograph::initialize_member_objects()
{
//   cout << "inside photograph::initialize_member_objs()" << endl;
   xdim=ydim=0;
   n_matching_SIFT_features=0;
   UTM_zonenumber=-1;
   
   focal_length=frustum_sidelength=movie_downrange_distance=-1;
   bundler_covariance_trace=-1;
   filename=URL="";
   score=-1;
   camera_ptr=NULL;
}		 

void photograph::allocate_member_objects()
{
   camera_ptr=new camera();
}

// ---------------------------------------------------------------------
photograph::photograph(int id) :
   node(id)
{
//   cout << "inside photograph constructor #0, id = " << id << endl;
   initialize_member_objects();
   allocate_member_objects();
}

photograph::photograph(string filename,int id,bool parse_exif_metadata_flag) : 
   node(id)
{
//   cout << "inside photograph constructor #1, id = " << id << endl;
//   cout << "filename = " << filename << endl;
//   cout << "parse_exif_metadata_flag = " << parse_exif_metadata_flag
//        << endl;

   initialize_member_objects();
   allocate_member_objects();
   this->filename=filename;

   if (parse_exif_metadata_flag) parse_Exif_metadata();

// On 3/16/09, we discovered to our horror that exif metadata
// sometimes can report INCORRECT pixel dimensions for photographs.
// So we now use OSG to determine the size of all input photos...

// On 6/27/09, we realized that the following call to
// set_image_dimensions() is very expensive.  So we temporarily
// comment it out.  In the future, we should write a separate program
// to read in all images, call set_image_dimension() and then output
// corrected exif metadata containing image sizes...

// On 6/29/09, we realized that commenting out the expensive call to
// set_image_dimensions() leads to catastrophic misalignment in
// NEW_FOV and BUNDLECIITES!  So we make the following relatively fast call:

   imagefunc::get_image_width_height(filename,xdim,ydim);
//   cout << "filename = " << filename
//        << " xdim = " << xdim << " ydim = " << ydim << endl;

   compute_UV_bounds();
}

// ---------------------------------------------------------------------
// This next overloaded constructor takes in the photograph's
// horizontal and vertical dimensions which have presumably been stored
// from a previous run.

photograph::photograph(
   string filename,int xdim,int ydim,int id,bool parse_exif_metadata_flag) :
   node(id)
{
//   cout << "inside photograph constructor #2" << endl;
//   cout << "xdim = " << xdim << " ydim = " << ydim << endl;
 //   cout << "parse_exif_metadata_flag = " << parse_exif_metadata_flag
//         << endl;

   initialize_member_objects();
   allocate_member_objects();
   this->filename=filename;

   if (parse_exif_metadata_flag) parse_Exif_metadata();

   this->xdim=xdim;
   this->ydim=ydim;

   compute_UV_bounds();
}

// ---------------------------------------------------------------------
// Bogus constructor for timestamp extraction only...

photograph::photograph(int id,string filename):
   node(id)
{
//   cout << "inside photograph constructor #3" << endl;
//   cout << "xdim = " << xdim << " ydim = " << ydim << endl;

   initialize_member_objects();
//   allocate_member_objects();
   this->filename=filename;
}

// ---------------------------------------------------------------------
// Copy constructor:

photograph::photograph(const photograph& p) : 
   node(p)
{
//    cout << "inside copy constructor" << endl;
   initialize_member_objects();
   allocate_member_objects();
   docopy(p);
}

photograph::~photograph()
{
//   cout << "inside photograph destructor" << endl;
//   cout << "this = " << this << endl;

   delete camera_ptr;
   camera_ptr=NULL;
}

// ---------------------------------------------------------------------
void photograph::docopy(const photograph& p)
{
//   cout << "inside photograph::docopy()" << endl;

   xdim=p.xdim;
   ydim=p.ydim;
   n_matching_SIFT_features=p.n_matching_SIFT_features;
   focal_length=p.focal_length;
   min_U=p.min_U;
   max_U=p.max_U;
   min_V=p.min_V;
   max_V=p.max_V;
   frustum_sidelength=p.frustum_sidelength;
   movie_downrange_distance=p.movie_downrange_distance;

   bundler_covariance_trace=p.bundler_covariance_trace;
   score=p.score;
   
   filename=p.filename;
   URL=p.URL;
   camera_make=p.camera_make;
   camera_model=p.camera_model;
   focal_plane_array_size=p.focal_plane_array_size;

   geolocation=p.geolocation;
   clock=p.clock;
   pointing=p.pointing;

   if (p.camera_ptr != NULL)
   {
      *camera_ptr = *(p.camera_ptr);      
   }

   if (p.image_refptr.valid())
   {
      image_refptr=new osg::Image(*(p.image_refptr.get()));
   }

//   if (p.segmentation_twoDarray_ptr != NULL)
//   {
//      segmentation_twoDarray_ptr=new twoDarray(*p.segmentation_twoDarray_ptr);
//   }
   
}

// Overload = operator:

photograph& photograph::operator= (const photograph& p)
{
   if (this==&p) return *this;
   node::operator=(p);
   docopy(p);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,photograph& p)
{
   outstream << endl;
   outstream << (node&)p << endl;
   outstream << "Filename = " << p.get_filename() << endl;
   outstream << "URL = " << p.get_URL() << endl;
   outstream << "Camera maker = " << p.get_camera_make() << endl;
   outstream << "Camera model = " << p.get_camera_model() << endl;
   outstream << "xdim = " << p.get_xdim() << " pixels" << endl;
   outstream << "ydim = " << p.get_ydim() << " pixels" << endl;
   outstream << "Focal length = " << p.get_focal_length() << " mm" << endl;
   outstream << "Focal plane array width = " 
             << p.get_focal_plane_array_size().get(0) << " mm " << endl;
   outstream << "Focal plane array height = " 
             << p.get_focal_plane_array_size().get(1) << " mm " << endl;
   outstream << "Frustum sidelength = " 
             << p.get_frustum_sidelength() 
             << " movie downrange distance = "
             << p.get_movie_downrange_distance() << endl;
   outstream << "Bundler covariance trace = "
             << p.get_bundler_covariance_trace() << endl;
   outstream << "Geolocation: " << p.get_geolocation() << endl;
   outstream << "Clock: " << p.get_clock() << endl;
   outstream << "Pointing: " << p.get_pointing() << endl;
   outstream << endl;

   return outstream;
}

// =========================================================================
// Set & get member functions:
// =========================================================================

// =========================================================================
// Data & metadata parsing member functions:
// =========================================================================

void photograph::read_image_from_file(string photo_filename)
{
   set_filename(photo_filename);
   read_image_from_file();
}

void photograph::read_image_from_file()
{
//   cout << "inside photograph::read_image_from_file()" << endl;
//   cout << "photograph filename = " << filename << endl;
   image_refptr=osgDB::readImageFile(filename);
//   cout << "image_refptr = " << get_image_ptr() << endl;
   xdim=image_refptr->s();
   ydim=image_refptr->t();
}

void photograph::rescale_image_size(
   double horiz_scale_factor,double vert_scale_factor)
{
   image_refptr->scaleImage(
      horiz_scale_factor*image_refptr->s(),
      horiz_scale_factor*image_refptr->t(),image_refptr->r());
}

// ---------------------------------------------------------------------
// Member function parse_timestamp_exiftag() reads timestamp
// exif metadata.  If successful, this boolean method returns true.

bool photograph::parse_timestamp_exiftag()
{
//   cout << "inside photograph::parse_timestamp_metadata()" << endl;

   if (!filefunc::fileexist(filename)) return false;
   
   Exiv2::Image::AutoPtr image_ptr = Exiv2::ImageFactory::open(
      filename.c_str());
   image_ptr->readMetadata();
 
   Exiv2::ExifData& exifData = image_ptr->exifData();
   if (exifData.empty()) return false;

   parse_timestamp_metadata(exifData);
   return true;
}

// =========================================================================
// Camera parameter member functions
// =========================================================================

// Boolean member function parse_Exif_metadata() returns false if no
// exif metadata is extracted from the current photograph.

bool photograph::parse_Exif_metadata()
{
//   cout << "inside photograph::parse_Exif_metadata()" << endl;
//   cout << "filename = " << filename << endl;
   if (!filefunc::fileexist(filename)) return false;
   
   Exiv2::Image::AutoPtr image_ptr = Exiv2::ImageFactory::open(
      filename.c_str());
   image_ptr->readMetadata();
 
   Exiv2::ExifData& exifData = image_ptr->exifData();
   if (exifData.empty()) 
   {
      cout << "No Exif data found in file " << filename << endl;
      return false;
   }

   parse_internal_params_metadata(exifData);
   parse_geolocation_metadata(exifData);
   parse_timestamp_metadata(exifData);
   parse_pointing_metadata(exifData);

   return true;
}

// ---------------------------------------------------------------------
// Member function parse_internal_params_metadata extracts the camera's
// internal parameters.

void photograph::parse_internal_params_metadata(Exiv2::ExifData& exifData)
{
//   cout << "inside photograph::parse_internal_params_metadata()" << endl;
   
   Exiv2::Exifdatum make_Datum=exifData["Exif.Image.Make"];
   camera_make=stringfunc::remove_trailing_whitespace(
      make_Datum.toString());

   Exiv2::Exifdatum model_Datum=exifData["Exif.Image.Model"];
   camera_model=stringfunc::remove_trailing_whitespace(
      model_Datum.toString());

   Exiv2::Exifdatum xdim_Datum=exifData["Exif.Photo.PixelXDimension"];
   xdim=xdim_Datum.toLong(0);
   Exiv2::Exifdatum ydim_Datum=exifData["Exif.Photo.PixelYDimension"];
   ydim=ydim_Datum.toLong(0);
   Exiv2::Exifdatum focallength_Datum=exifData["Exif.Photo.FocalLength"];
   focal_length=focallength_Datum.toFloat(0);
//   cout << "focal_length = " << focal_length << endl;

   set_focal_plane_array_size();
}

// ---------------------------------------------------------------------
void photograph::compute_UV_bounds()
{
//   cout << "inside photograph::compute_UV_bounds() " << endl;
   min_U=0;
   max_U=double(xdim)/double(ydim);
   if (ydim==0)
   {
      cout << "xdim = " << xdim << " ydim = " << ydim 
           << " max_U = " << max_U << endl;
      exit(-1);
   }
   
   if (nearly_equal(max_U,min_U))
   {
      cout << "inside photograph::compute_UV_bounds()" << endl;
      cout << "max_U = " << max_U << " xdim = " << xdim
           << " ydim = " << ydim << endl;
      outputfunc::enter_continue_char();
   }

   min_V=0;
   max_V=1;
   
   camera_ptr->set_UV_corners(min_U,max_U,min_V,max_V);
}

// ---------------------------------------------------------------------
// Member function set_focal_plane_array_size

void photograph::set_focal_plane_array_size()
{
   if (camera_make=="RICOH" && camera_model=="Caplio 500SE")
   {
      focal_plane_array_size=twovector(7.176,5.319);	// 1/1.8"
   }
}

twovector& photograph::get_focal_plane_array_size()
{
   return focal_plane_array_size;
}

const twovector& photograph::get_focal_plane_array_size() const
{
   return focal_plane_array_size;
}

// ---------------------------------------------------------------------
// Member function parse_geolocation_metadata extracts the camera's
// longitude, latitude and altitude at the time the photo was shot
// from the Exif metadata.

void photograph::parse_geolocation_metadata(Exiv2::ExifData& exifData)
{
//   cout << "inside photograph::parse_geolocation_metadata()" << endl;
   
   Exiv2::Exifdatum longitude_Datum=exifData["Exif.GPSInfo.GPSLongitude"];
   if (longitude_Datum.size() > 0)
   {
      double longitude;
      double arcsecs=longitude_Datum.toFloat(2);
      if (nearly_equal(arcsecs,0,0.0000001))
      {
         longitude=latlongfunc::dm_to_decimal_degs(
            longitude_Datum.toFloat(0),longitude_Datum.toFloat(1));
      }
      else
      {
         longitude=latlongfunc::dms_to_decimal_degs(
            longitude_Datum.toFloat(0),longitude_Datum.toFloat(1),
            longitude_Datum.toFloat(2));
      }

      Exiv2::Exifdatum longitude_ref_Datum=exifData[
         "Exif.GPSInfo.GPSLongitudeRef"];
      if (longitude_ref_Datum.toString()=="W")
      {
         longitude=-longitude;
      }

      Exiv2::Exifdatum latitude_Datum=exifData["Exif.GPSInfo.GPSLatitude"];
      
      double latitude;
      arcsecs=latitude_Datum.toFloat(2);
      if (nearly_equal(arcsecs,0,0.0000001))
      {
         latitude=latlongfunc::dm_to_decimal_degs(
            latitude_Datum.toFloat(0),latitude_Datum.toFloat(1));
      }
      else
      {
         latitude=latlongfunc::dms_to_decimal_degs(
            latitude_Datum.toFloat(0),latitude_Datum.toFloat(1),
            latitude_Datum.toFloat(2));
      }

      Exiv2::Exifdatum altitude_Datum=exifData["Exif.GPSInfo.GPSAltitude"];
      double altitude=altitude_Datum.toFloat(0);

//      cout << "lon = " << longitude
//           << " lat = " << latitude
//           << " alt = " << altitude << endl;
      geolocation=geopoint(longitude,latitude,altitude);
//      cout << "Extracted geolocation = " << geolocation << endl;
      UTM_zonenumber=geolocation.get_UTM_zonenumber();
   } // longitude_Datum.size() > 0 conditional
}

// ---------------------------------------------------------------------
void photograph::parse_timestamp_metadata(Exiv2::ExifData& exifData)
{
//   cout << "inside photograph::parse_timestamp_metadata()" << endl;
   Exiv2::Exifdatum timestamp_Datum=exifData["Exif.Image.DateTime"];
//   cout << "timestamp_Datum = " << timestamp_Datum << endl;

   if (timestamp_Datum.size() > 0)
   {
      string timestamp=timestamp_Datum.toString()+" ";
//      cout << "timestamp = " << timestamp << endl;
 
      vector<string> time_strings=
         stringfunc::decompose_string_into_substrings(timestamp);
      if (time_strings.size()==0) return;
//      templatefunc::printVector(time_strings);
   
      string year_str=time_strings[0].substr(0,4);
      string month_str=time_strings[0].substr(5,2);
      string day_str=time_strings[0].substr(8,2);
      int year=stringfunc::string_to_integer(year_str);
      int month=stringfunc::string_to_integer(month_str);
      int day=stringfunc::string_to_integer(day_str);

//      cout << "year_str = " << year_str << endl;
//      cout << "month_str = " << month_str << endl;
//      cout << "day_str = " << day_str << endl;

      string hour_str=time_strings[1].substr(0,2);
      string min_str=time_strings[1].substr(3,2);
      string sec_str=time_strings[1].substr(6,2);
//      cout << "hour_str = " << hour_str << endl;
//      cout << "min_str = " << min_str << endl;
//      cout << "sec_str = " << sec_str << endl;
      int hour=stringfunc::string_to_integer(hour_str);
      int minute=stringfunc::string_to_integer(min_str);
      double sec=stringfunc::string_to_number(sec_str);
      
//      cout << "year = " << year << " month = " << month << " day = " << day
//           << endl;
//      cout << "hour = " << hour << " min = " << minute << " sec = " << sec 
//           << endl;
      clock.set_local_time(year,month,day,hour,minute,sec);

//      cout << "UTM_zonenumber = " << UTM_zonenumber << endl;
      if (UTM_zonenumber >= 0)
      {
//         int UTM_zone_time_offset=
            clock.compute_UTM_zone_time_offset(UTM_zonenumber);
//         cout << "UTM_zone_time_offset = " << UTM_zone_time_offset << endl;
      }

//      double secs_elapsed_since_epoch=
//         clock.secs_elapsed_since_reference_date();
//      cout.precision(16);
//      cout << "Elapsed secs since midnight, Jan 1, 1970 = "
//           << secs_elapsed_since_epoch << endl;
      
   } // timestamp_Datum.size() > 0 conditional
}

// ---------------------------------------------------------------------
// Member function parse_pointing_metadata

void photograph::parse_pointing_metadata(Exiv2::ExifData& exifData)
{
   Exiv2::Exifdatum img_dir_Datum=exifData["Exif.GPSInfo.GPSImgDirection"];
   if (img_dir_Datum.size() > 0)
   {
      double yaw=img_dir_Datum.toFloat(0);
      pointing.set_yaw(yaw);
   } // img_dir_Datum.size() > 0 conditional
}

// ---------------------------------------------------------------------
// If input parameter FOV_u is reasonably valued, we use it to
// estimate focal parameter f for a square pixel.  If
// FOV_u==NEGATIVEINFINITY, we then try to extract f from EXIF
// metadata along with a known value for the focal plane size.

void photograph::estimate_internal_camera_params(double FOV_u)
{
//   cout << "inside photograph::estimate_internal_camera_params()" << endl;
//   cout << "photo ID = " << ID << endl;
//   cout << "FOV_u = " << FOV_u << endl;

// Recall dimensionless f parameter is negative!

   double f;
   if (FOV_u > 0.5*NEGATIVEINFINITY)
   {
      double aspect_ratio=double(xdim)/double(ydim);
      double FOV_v=camerafunc::vert_FOV_from_horiz_FOV_and_aspect_ratio(
         FOV_u,aspect_ratio);
      camerafunc::f_and_aspect_ratio_from_horiz_vert_FOVs(
         FOV_u,FOV_v,f,aspect_ratio);
   }
   else
   {
      parse_Exif_metadata();
         
// As of Nov 16 2008, we assume input camera equals Fujifilm FinePix
// S8000fd.  We'll relax this strong assumption later...

//       double focal_plane_size=3.76;	// millimeters

// As of 3/8/2014, we assume input camera equals iPhone 5:

// 	According to http://en.wikipedia.org/wiki/Image_sensor_format:
//      double focal_plane_size = 4.54; // 4.54 mm width, 3.42 mm height 
					//  for iPhone 5   [ ' 1/3.2" ' ]

      double focal_plane_size = 3.38; 	// Bundler computation for iPhone 5 
      f=-focal_length/focal_plane_size;
   }

   double U0=0.5*double(xdim)/double(ydim);
   double V0=0.5;

   camera_ptr->set_internal_params(f,f,U0,V0);

   cout << "Photo = " << filename
        << " Focal length = " << focal_length << " mm" << endl;
   cout << "Initial param estimates: f = " << f 
        << " U0 = " << U0 << " V0 = " << V0 << endl;
   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function export_camera_parameters generates a package file
// based upon the parameters passed within the input camera object.

void photograph::export_camera_parameters(
   camera* input_camera_ptr,string filename_descriptor,
   string packages_subdir,double frustum_side_length)
{
//   cout << "inside photograph::export_camera_parameters()" << endl;
//   cout << "photograph.get_filename() = "
//        << get_filename() << endl;

// Recover camera's internal and external parameters:

//   string subdir="./packages/";
   string package_filename=
      packages_subdir+stringfunc::prefix(filefunc::getbasename(filename))
      +filename_descriptor+".pkg";
//   cout << "package_filename = " << package_filename << endl;
      
   ofstream outstream;
   outstream.precision(10);
   filefunc::openfile(package_filename,outstream);
      
   outstream << filename << endl;
   outstream << "--Uaxis_focal_length " << input_camera_ptr->get_fu() << endl;
   outstream << "--Vaxis_focal_length " << input_camera_ptr->get_fv() << endl;
   outstream << "--U0 " << input_camera_ptr->get_u0() << endl;
   outstream << "--V0 " << input_camera_ptr->get_v0() << endl;

   input_camera_ptr->compute_az_el_roll_from_Rcamera();
   outstream << "--relative_az " << input_camera_ptr->get_rel_az()*180/PI 
             << endl;
   outstream << "--relative_el " << input_camera_ptr->get_rel_el()*180/PI 
             << endl;
   outstream << "--relative_roll " << input_camera_ptr->get_rel_roll()*180/PI 
             << endl;
   
   threevector camera_posn=input_camera_ptr->get_world_posn();
   outstream << "--camera_x_posn " << camera_posn.get(0) << endl;
   outstream << "--camera_y_posn " << camera_posn.get(1) << endl;
   outstream << "--camera_z_posn " << camera_posn.get(2) << endl;

   if (frustum_side_length > 0) frustum_sidelength=frustum_side_length;
   if (frustum_sidelength <= 0) frustum_sidelength=60;
   outstream << "--frustum_sidelength " << frustum_sidelength << endl;

   filefunc::closefile(package_filename,outstream);

   string banner="Exported camera parameters to "+package_filename;
   outputfunc::write_banner(banner);
}

// -------------------------------------------------------------------------
// Member function reset_camera_parameters() takes in a pinhole camera
// model's intrinsic and extrinsic parameters.  This method resets all
// these parameters within the current photograph's camera object.  It
// also recomputes the 3x4 projection matrix as well as the camera's
// horiz & vertical fields-of-view.

bool photograph::reset_camera_parameters(
   double fu,double fv,double U0,double V0,
   double az,double el,double roll,const threevector& camera_posn)
{
//   cout << "inside photograph::reset_camera_parameters()" << endl;

//   cout.precision(10);
//   cout << " fu = " << fu << " fv = " << fv << endl;
//   cout << "U0 = " << U0 << " V0 = " << V0 << endl;
//   cout << "az = " << az*180/PI << " el = " << el*180/PI
//        << " roll = " << roll*180/PI << endl;
//   cout << "camera_posn = " << camera_posn << endl;

   camera* camera_ptr=get_camera_ptr();
   camera_ptr->set_internal_params(fu,fv,U0,V0);
   camera_ptr->set_Rcamera(az,el,roll);
   camera_ptr->set_world_posn(camera_posn);
   bool nonsingular_submatrix_flag=camera_ptr->construct_projection_matrix();
   if (!nonsingular_submatrix_flag)
   {
      cout << "Trouble in photograph::reset_camera_parameters()" << endl;
      cout << "Photograph ID = " << get_ID() << endl;
      cout << "Photo filename = " << get_filename() << endl;

      cout.precision(10);
      cout << " fu = " << fu << " fv = " << fv << endl;
      cout << "U0 = " << U0 << " V0 = " << V0 << endl;
      cout << "az = " << az*180/PI << " el = " << el*180/PI
           << " roll = " << roll*180/PI << endl;
      cout << "camera_posn = " << camera_posn << endl;

      camera_ptr->set_calibration_flag(false);
      outputfunc::enter_continue_char();
   }
   else
   {
      camera_ptr->set_calibration_flag(true);

//   cout.precision(10);
//   cout << "P = " << *(camera_ptr->get_P_ptr()) << endl;

      camera_ptr->compute_fields_of_view(
         get_maxU(),get_minU(),get_maxV(),get_minV());
//   cout << "*camera_ptr = " << *camera_ptr << endl;
   }
   return camera_ptr->get_calibration_flag();
} 
