// ========================================================================
// Movie provides functionality for displaycing and manipulating movie
// files.
// ========================================================================
// Last updated on 3/12/13; 4/1/14; 4/2/14; 5/30/14
// ========================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/Quat>
#include "osg/osgGraphicals/AnimationController.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "geometry/homography.h"
#include "geometry/linesegment.h"
#include "math/mathfuncs.h"
#include "osg/osg2D/Movie.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"
#include "image/TwoDarray.h"

#include "general/outputfuncs.h"
#include "osg/osgfuncs.h"
#include <osg/Uniform>

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ----------------------------------------------------------------
void Movie::allocate_member_objects()
{
   geode_refptr = new osg::Geode;
}

void Movie::initialize_member_objects()
{
   Graphical_name="Movie";
   dynamic_window_flag=false;
   watch_subdirectory="";
   geode_refptr->addDrawable(static_cast<osg::Drawable*>(get_Geometry_ptr()));

   warp_onto_imageplane_flag=false;

   fadein_start_time=fadeout_start_time=-1;
   imageplane_ptr=NULL;
   photograph_ptr=NULL;
   tracks_map_ptr=NULL;
   annotations_map_ptr=NULL;
}

void Movie::instantiate_cameras_for_images()
{
//   cout << "inside Movie::instantiate_cameras_for_images()" << endl;
//   cout << "get_Nimages() = " << get_Nimages() << endl;
   if (camera_ptrs.size()==0)
   {
      for (int n=0; n<get_Nimages(); n++)
      {
         camera* curr_camera_ptr=new camera(this);
         camera_ptrs.push_back(curr_camera_ptr);
      }		       
   }
}

// Member function destroy_cameras_for_images() destroys all
// dynamically generated camera objects.  It then clears STL vector
// camera_ptrs.

void Movie::destroy_cameras_for_images()
{
//   cout << "inside Movie::destroy_cameras_for_images()" << endl;
   for (unsigned int n=0; n<camera_ptrs.size(); n++)
   {
      delete camera_ptrs[n];
   }		       
   camera_ptrs.clear();
}

void Movie::clear_camera_ptrs()
{
   camera_ptrs.clear();
}

// ----------------------------------------------------------------
Movie::Movie(
   const int p_ndims,string filename,int id,double alpha,
   AnimationController* AC_ptr,bool hide_backface_flag,
   osgGA::Custom3DManipulator* CM_ptr):
   G99VideoDisplay(filename,AC_ptr,hide_backface_flag),
   Graphical(p_ndims,id,AC_ptr)
{
//    cout << "inside Movie constructor #1" << endl;
   allocate_member_objects();
   initialize_member_objects();
   CM_3D_refptr=CM_ptr;
   enable_alpha_blending(alpha,get_ndims());
}

// ----------------------------------------------------------------
Movie::Movie(
   const int p_ndims,texture_rectangle* texture_rectangle_ptr,
   int id,double alpha,osgGA::Custom3DManipulator* CM_ptr):
   G99VideoDisplay(texture_rectangle_ptr),
   Graphical(p_ndims,id,texture_rectangle_ptr->get_AnimationController_ptr())
{
//    cout << "inside Movie constructor #2" << endl;
   allocate_member_objects();
   initialize_member_objects();
   CM_3D_refptr=CM_ptr;
   enable_alpha_blending(alpha,get_ndims());
}

// ----------------------------------------------------------------
Movie::~Movie()
{
//   cout << "inside Movie destructor" << endl;
//   cout << "this = " << this << endl;

   destroy_cameras_for_images();
   delete imageplane_ptr;
   delete tracks_map_ptr;
   delete annotations_map_ptr;
}

// ========================================================================
// Set & get member functions
// ========================================================================

void Movie::set_camera_ptr(camera* c_ptr)
{
   destroy_cameras_for_images();

// As of 1/16/12, we strongly suspect next set of lines should be
// replaced by call to new method destroy_cameras_for_images():

//   for (unsigned int c=0; c<camera_ptrs.size(); c++)
//   {
//      delete camera_ptrs[c];
//   }

   camera_ptrs.push_back(c_ptr);
}

camera* Movie::get_camera_ptr()
{
//   cout << "inside Movie::get_camera_ptr()" << endl;

   int curr_imagenumber=get_AnimationController_ptr()->
      get_first_framenumber();
//   cout << "curr_imagenumber = " << curr_imagenumber << endl;
   
   if (!get_stationary_Graphical_flag())
   {
      curr_imagenumber=get_imagenumber();
   }

// Note added on 2/4/09:  We CANNOT replace the above few lines with 

//   int curr_imagenumber=get_AnimationController_ptr()->
//      get_curr_framenumber();

// ESB car video example fails if we attempt to do so!

   return get_camera_ptr(curr_imagenumber);
}

camera* Movie::get_camera_ptr(int imagenumber)
{
//   cout << "inside Movie::get_camera_ptr(imagenumber)" << endl;
//   cout << "imagenumber = " << imagenumber << endl;
//   cout << "camera_ptrs.size() = " << camera_ptrs.size() << endl;
   if (camera_ptrs.size()==0) instantiate_cameras_for_images();

   camera* camera_ptr=camera_ptrs[static_cast<unsigned int>(imagenumber)];
   camera_ptr->set_video_ptr(this);
   return camera_ptr;
}

// ----------------------------------------------------------------
// Member function get_absolute_altitude takes the COM of the movie's
// 4 corners as the initial position for the movie.  It then adds the
// z displacement corresponding to the input curr_t and pass_number
// parameters.  This method returns the sum of the initial height and
// z offset as the current absolute altitude for the movie in world
// space.

double Movie::get_absolute_altitude(double curr_t,int pass_number)
{
   double initial_altitude=compute_corners_COM().get(2);
   threevector curr_posn_offset;
   get_UVW_coords(curr_t,pass_number,curr_posn_offset);
   double curr_altitude=initial_altitude+curr_posn_offset.get(2);
   return curr_altitude;
}

// ========================================================================
// Camera member functions:
// ========================================================================

string Movie::enter_camera_params_filename()
{
   string camera_params_filename="camera_params.txt";
/*
  cout << endl;
  cout << "Enter file with interpolated camera intrinsics & extrinsics" 
  << endl;
  cout << "generated by program CAMERA_PARAMS:" << endl;
  cin >> camera_params_filename;
  cout << endl;
*/
   return camera_params_filename;
}

// ----------------------------------------------------------------
// Specialized member function plot_roll_pitch_yaw_vs_time generates 3
// ascii text files containing attitude information (in degrees) which
// can be manually turned into metafile plots.

void Movie::plot_roll_pitch_yaw_vs_time()
{
   string camera_params_filename=enter_camera_params_filename();
   filefunc::ReadInfile(camera_params_filename);

   string X_filename="X.txt";
   string Y_filename="Y.txt";
   string Z_filename="Z.txt";
   string roll_filename="roll.txt";
   string pitch_filename="pitch.txt";
   string yaw_filename="yaw.txt";
   ofstream Xstream,Ystream,Zstream,rollstream,pitchstream,yawstream;
   filefunc::openfile(X_filename,Xstream);
   filefunc::openfile(Y_filename,Ystream);
   filefunc::openfile(Z_filename,Zstream);
   filefunc::openfile(roll_filename,rollstream);
   filefunc::openfile(pitch_filename,pitchstream);
   filefunc::openfile(yaw_filename,yawstream);

   double X[5];
   unsigned int i=0;
   threevector init_camera_posn;
   while (i < filefunc::text_line.size())
   {
      stringfunc::string_to_n_numbers(1,filefunc::text_line[i++],X);
      double curr_time=X[0];
      stringfunc::string_to_n_numbers(5,filefunc::text_line[i++],X);
//      double curr_fu=X[0];
//      double curr_fv=X[1];
//      double curr_u0=X[2];
//      double curr_v0=X[3];
//      double curr_kappa2=X[4];
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i++],X);
//      double curr_alpha=X[0];
//      double curr_beta=X[1];
//      double curr_phi=X[2];
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i++],X);
      threevector camera_posn(X[0],X[1],X[2]);
      if (nearly_equal(curr_time,0)) init_camera_posn=camera_posn;
      threevector rel_camera_posn=camera_posn-init_camera_posn;
      
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i++],X);
      double pitch=X[0]*180/PI;
      double roll=X[1]*180/PI;
      double yaw=X[2]*180/PI;
      Xstream << curr_time << "   " << rel_camera_posn.get(0) << endl;
      Ystream << curr_time << "   " << rel_camera_posn.get(1) << endl;
      Zstream << curr_time << "   " << rel_camera_posn.get(2) << endl;
      rollstream << curr_time << "   " << roll << endl;
      pitchstream << curr_time << "   " << pitch << endl;
      yawstream << curr_time << "   " << yaw << endl;
   }
   filefunc::closefile(X_filename,Xstream);
   filefunc::closefile(Y_filename,Ystream);
   filefunc::closefile(Z_filename,Zstream);
   filefunc::closefile(roll_filename,rollstream);
   filefunc::closefile(pitch_filename,pitchstream);
   filefunc::closefile(yaw_filename,yawstream);
}

// ----------------------------------------------------------------
// Member function read_camera_params_for_sequence parses the ascii
// intrinsic and extrinsic file output generated by main programs
// CAMERA_PARAMS or BACKPROJECT for each image in a video sequence.
// It then dynamically generates a camera for each image in the video
// and stores their pointers in member STL vector camera_ptrs.

void Movie::read_camera_params_for_sequence()
{
   instantiate_cameras_for_images();

   string camera_params_filename=enter_camera_params_filename();
   filefunc::ReadInfile(camera_params_filename);
   double X[5];
   unsigned int i=0;
   while (i < filefunc::text_line.size())
   {
      stringfunc::string_to_n_numbers(1,filefunc::text_line[i++],X);
      double curr_time=X[0];
      int imagenumber=basic_math::round(curr_time);
      stringfunc::string_to_n_numbers(5,filefunc::text_line[i++],X);
      double curr_fu=X[0];
      double curr_fv=X[1];
      double curr_u0=X[2];
      double curr_v0=X[3];
      double curr_kappa2=X[4];
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i++],X);
      double curr_alpha=X[0];
      double curr_beta=X[1];
      double curr_phi=X[2];
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i++],X);
      threevector camera_world_posn(X[0],X[1],X[2]);
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i++],X);
      double pitch=X[0];
      double roll=X[1];
      double yaw=X[2];

//      cout << "i = " << i << " text_line.size() = " 
//           << filefunc::text_line.size() << " time = " << curr_time << endl;
//      cout << "fu = " << curr_fu << " u0 = " << curr_u0
//           << " v0 = " << curr_v0 << " kappa = " << curr_kappa2 << endl;
//      cout << "alpha = " << curr_alpha*180/PI 
//           << " beta = " << curr_beta*180/PI
//           << " phi = " << curr_phi*180/PI << endl;
//      cout << "x = " << camera_world_posn.get(0) 
//           << " y = " << camera_world_posn.get(1)
//           << " z = " << camera_world_posn.get(2) << endl;
//      cout << "pitch = " << pitch*180/PI << " roll = " << roll*180/PI 
//           << " yaw = " << yaw*180/PI << endl << endl;

      camera* curr_camera_ptr=camera_ptrs[imagenumber];
      curr_camera_ptr->set_internal_params(
         curr_fu,curr_fv,curr_u0,curr_v0,PI/2.0,curr_kappa2);
//      new camera(
//         curr_fu,curr_fv,curr_u0,curr_v0,curr_kappa2);
      curr_camera_ptr->set_mount_rotation_angles(
         curr_alpha,curr_beta,curr_phi);
      curr_camera_ptr->set_world_posn(camera_world_posn);
      curr_camera_ptr->set_aircraft_rotation_angles(pitch,roll,yaw);
      curr_camera_ptr->compute_rotation_matrix();

   } // while loop over index i labeling text line
}

// ========================================================================
// FOV member functions:
// ========================================================================

// Member function set_FOV_scale_attitude_posn

void Movie::set_FOV_scale_attitude_posn(
   const vector<double>& curr_time,const vector<int>& pass_number,
   const vector<threevector>& V1,const threevector& grid_origin)
{
   for (unsigned int j=0; j<curr_time.size()/4; j++)
   {
      double t=curr_time[4*j];
      int pass=pass_number[4*j];

      threevector origin_proj=V1[4*j];
      threevector Umax_proj=V1[4*j+1];
      threevector Vmax_proj=V1[4*j+3];

//      cout << "t = " << t 
//           << " origin.x = " << origin_proj.get(0)-grid_origin.get(0)
//           << " y = " << origin_proj.get(1)-grid_origin.get(1) 
//           << " z = " << origin_proj.get(2)-grid_origin.get(2) << endl;

//      cout << "t = " << t
//           << " Umax.x = " << Umax_proj.get(0)-grid_origin.get(0)
//           << " y = " << Umax_proj.get(1)-grid_origin.get(1)
//           << " z = " << Umax_proj.get(2)-grid_origin.get(2) << endl;

//      cout << "t = " << t
//           << " Vmax.x = " << Vmax_proj.get(0)-grid_origin.get(0)
//           << " y = " << Vmax_proj.get(1)-grid_origin.get(1)
//           << " z = " << Vmax_proj.get(2)-grid_origin.get(2) << endl;

      approx_project_onto_worldspace_grid(
         t,pass,origin_proj,Umax_proj,Vmax_proj,grid_origin);
   } // loop over index j labeling times
}

// ----------------------------------------------------------------
// Member function approx_project_onto_worldspace_grid computes the
// scale, rotation and translation factors which need to be applied to
// the current video image in order to approximately project it down
// onto the world grid in XYZ space

void Movie::approx_project_onto_worldspace_grid(
   double curr_t,int pass_number,
   const threevector& origin_proj,const threevector& Umax_proj,
   const threevector& Vmax_proj,const threevector& grid_origin)
{
   linesegment proj_Usegment(origin_proj,Umax_proj);
   linesegment proj_Vsegment(origin_proj,Vmax_proj);
         
   double scale_U=proj_Usegment.get_length()/get_maxU();
   double scale_V=proj_Vsegment.get_length();
   set_scale(curr_t,pass_number,threevector(scale_U,1,scale_V));

/*
  threevector e_hat=proj_Usegment.get_ehat();
  double theta=atan2(e_hat.get(1),e_hat.get(0));
  osg::Quat quat_vid_to_worldXY,quat_about_worldZ;
  quat_vid_to_worldXY.makeRotate ( -PI/2,osg::Vec3f(1,0,0));
  quat_about_worldZ.makeRotate ( theta,osg::Vec3f(0,-1,0));
//   osg::Quat total_quat=quat_about_worldZ*quat_vid_to_worldXY;
osg::Quat total_quat=quat_vid_to_worldXY;
*/

// First construct quaternion that rotates canonical u_hat = x_hat to
// proj_Usegment.e_hat in world space:

   osg::Vec3 vec1a(1,0,0);
   osg::Vec3 vec1b(proj_Usegment.get_ehat().get(0),
                   proj_Usegment.get_ehat().get(1),
                   proj_Usegment.get_ehat().get(2));
   osg::Quat quat1;
   quat1.makeRotate(vec1a,vec1b);

// Next construct quaternion that rotates transformed canonical v_hat
// = z_hat about transformed u_hat to proj_Vsegment.e_hat in world
// space:

   osg::Vec3 vec2a(0,0,1);
   osg::Vec3 vec2b=quat1*vec2a;
   threevector zhat_prime(vec2b.x(),vec2b.y(),vec2b.z());
   threevector zhat_primeprime=proj_Vsegment.get_ehat();
   double dotproduct=zhat_prime.dot(zhat_primeprime);
   double zeta=acos(dotproduct);
   osg::Quat quat2;
   quat2.makeRotate(-zeta,osg::Vec3f(1,0,0));
   osg::Quat total_quat=quat2*quat1;

   set_quaternion(curr_t,pass_number,total_quat);

//   set_UVW_coords(curr_t,pass_number,grid_origin);
//   cout << "grid_origin = " << grid_origin << endl;

   set_UVW_coords(curr_t,pass_number,origin_proj);
}

// ----------------------------------------------------------------
// Member function project_onto_worldspace_grid computes the scale,
// rotation and translation factors which need to be applied to the
// current video image in order to project it down onto the world grid
// in XYZ space

void Movie::project_onto_worldspace_grid(
   double curr_t,int pass_number,const threevector& grid_origin)
{

// On 10/19/05, we empirically determined the following approximate
// scaling values for roadways.vid which comes from the Google map
// covering most of HAFB:

   double scale_U=2732;
   double scale_V=2732;
   set_scale(curr_t,pass_number,threevector(scale_U,1,scale_V));

   threevector e_hat(1,0,0);
   double theta=atan2(e_hat.get(1),e_hat.get(0));
   osg::Quat quat_vid_to_worldXY,quat_about_worldZ;
   quat_vid_to_worldXY.makeRotate ( -PI/2,osg::Vec3f(1,0,0));
   quat_about_worldZ.makeRotate ( theta,osg::Vec3f(0,-1,0));
//   osg::Quat total_quat=quat_about_worldZ*quat_vid_to_worldXY;
   osg::Quat total_quat=quat_vid_to_worldXY;

   set_quaternion(curr_t,pass_number,total_quat);

   double delta_z=-0.1;
   threevector z_hat(0,0,1);
   set_UVW_coords(curr_t,pass_number,grid_origin+delta_z*z_hat);
//   cout << "grid_origin = " << grid_origin << endl;
}

// ========================================================================
// Rotation, scaling and translation member functions
// ========================================================================

// Member function rotate_scale implements the input rotation and
// scaling for the current movie window.

void Movie::rotate_scale(
   double curr_t,int pass_number,
   const threevector& Uaxis,const threevector& Vaxis)
{
//   cout << "inside Movie::rotate_scale()" << endl;
   threevector Uhat=Uaxis.unitvector();
   threevector Vhat=Vaxis.unitvector();
//   threevector What(Uhat.cross(Vhat));
   set_UVW_dirs(curr_t,pass_number,Uhat,Vhat);
   set_UVW_scales(curr_t,pass_number,Uaxis.magnitude(),Vaxis.magnitude());
}

void Movie::rotate_scale(
   double curr_t,int pass_number,const genmatrix& Rimageplane,
   const twovector& UVsize_in_meters)
{
   threevector Uhat,Vhat;
   Rimageplane.get_column(0,Uhat);
   Rimageplane.get_column(1,Vhat);
//   threevector What(Uhat.cross(Vhat));
   set_UVW_dirs(curr_t,pass_number,Uhat,Vhat);

   set_UVW_scales(curr_t,pass_number,UVsize_in_meters.get(0),
                  UVsize_in_meters.get(1));
}

// ----------------------------------------------------------------
// Member function translate takes in ISAR imageplane parameters
// meters_per_pixel, center_shift and trans which are extracted from
// imagecdf or paramcdf files.  It computes the UV translation
// corresponding to these inputs.  It also includes into the
// translation a radial displacement in the +What direction for the
// entire image plane.  This method returns the total translation
// threevector which needs to be applied to the imageplane in order to
// align it with the IGES model.

threevector Movie::translate(
   double curr_t,int pass_number,const twovector& meters_per_pixel,
   const twovector& center_shift,const twovector& trans,
   double radial_displacement)
{
   threevector Uhat,Vhat,What;
   threevector curr_trans(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   if (get_UVW_dirs(curr_t,pass_number,Uhat,Vhat,What))
   {

// Recall that the image center for imagecdf data does not generally
// coincide with the middle pixel.  Instead, the center location is
// stored as a field within the imagecdf header.

// On 4/3/06, we empircally observed that the center pixel's U
// location very closely matches the naive middle position.  However,
// the center pixel's V location is shifted a bit away from the naive
// middle position:

      double mid_udist=0.5*getWidth()*meters_per_pixel.get(0);
//     double mid_udist=center_shift.get(0);
//     double mid_vdist=0.5*getHeight()*meters_per_pixel.get(1);
      double mid_vdist=center_shift.get(1);
      
      double delta_udist=trans.get(0);
      double delta_vdist=trans.get(1);

// On 4/3/06, we empirically checked that delta_udist [delta_vdist]
// should be SUBTRACTED from (rather than added to) mid_udist
// [mid_vdist]:
      
      double udist=mid_udist-delta_udist;
      double vdist=mid_vdist-delta_vdist;
      curr_trans=udist*Uhat+vdist*Vhat-radial_displacement*What;
   } // get_UVW_dirs conditional
   return curr_trans;
}

// ----------------------------------------------------------------
// Member function retrieve_camera_quaternion retrieves world-space
// image plane direction vectors Uhat, Vhat and What assuming that the
// camera has already been calibrated.  It uses these threevectors to
// form and return the camera's orientation within an osg quaternion.

osg::Quat Movie::retrieve_camera_quaternion(
   double curr_t,int pass_number,bool align_Vhat_with_zhat)
{
//   cout << "inside Movie::retrieve_camera_quaternion()" << endl;
//   cout << "align_Vhat_with_zhat = " << align_Vhat_with_zhat << endl;
   
   threevector Uhat,Vhat,What;
   get_UVW_dirs(curr_t,pass_number,Uhat,Vhat,What);

   rotation R;
   if (align_Vhat_with_zhat)
   {
      R.put_column(0,Uhat);
      R.put_column(1,-What);
      R.put_column(2,Vhat);
   }
   else
   {
      R.put_column(0,Uhat);
      R.put_column(1,Vhat);
      R.put_column(2,What);
   }

   double detR;
   R.determinant(detR);
//   cout << "detR = " << detR << endl;
//   cout << "R = " << R << endl;
   
   osg::Quat q=osgfunc::rotation_to_quaternion(R);
//   cout << "osg quat = " << endl;
//   osgfunc::print_quaternion(q);

//   fourvector my_quat=R.quaternion_corresponding_to_rotation();
//   cout << "my_quat = " << my_quat << endl;
//   q=osg::Quat(my_quat[0],my_quat[1],my_quat[2],my_quat[3]);

   return q;
}

// ----------------------------------------------------------------
// Member function transform_UV_to_XYZ_coords scales, rotates and
// translates the UV image plane into XYZ world space.  It uses
// scaling information stored within member variables Ulength &
// Vlength, It next rotates the image plane axes so that they line up
// with the world-space coordinates stored in member threevectors Uhat
// and Vhat.  Finally, it translates the scaled & rotated image plane
// so that its lower left corner is positioned at member threevector
// frame_origin.

// The size, orientation and location of any image plane in world
// space can be altered via a call to this member function.

void Movie::transform_UV_to_XYZ_coords(double curr_t,int pass_number)
{
//   cout << "inside Movie::transform_UV_to_XYZ_coords()" << endl;

// First rescale the U and V axes from their initial to their final,
// fitted lengths:
   
   double Uscale,Vscale,Wscale;
   get_UVW_scales(curr_t,pass_number,Uscale,Vscale,Wscale);
   set_scale(curr_t,pass_number,threevector(
      Uscale/get_maxU(),1,Vscale/get_maxV()));

//   cout << "Uscale = " << Uscale << " Vscale = " << Vscale << endl;

   osg::Quat q=retrieve_camera_quaternion(curr_t,pass_number);
   set_quaternion(curr_t,pass_number,q);

// Finally, reset the image frame's origin to its world-space
// location:

   set_UVW_coords(curr_t,pass_number,frame_origin);
//   cout << "frame_origin = " << frame_origin.get(0) << ","
//        << frame_origin.get(1) << "," << frame_origin.get(2) << endl;
}

// ----------------------------------------------------------------
// Member function remap_window_corners takes in the camera's location
// camera_XYZ, planar wavefront propagation (= momentum) direction
// vector khat ( = - What), and 2D image plane corner direction
// vectors UV_corner_dir.  It first computes the image plane whose
// normal vector equals khat and which is located downrange from
// camera_XYZ by input distance rho.  This method next computes the
// intercept locations of each UV_corner_dir ray with the movie image
// plane.  Finally, this method resets the G99VideoDisplay's geometry
// vertices to these ray intercept locations.

// On 9/23/07, we explicitly verified that this method maps a
// photograph's 2D rectangle onto a 3D parallelogram which precisely
// touches the view frustum's side edges.  This method yields a
// noticeable improvement between an alpha-blended 2D photo and its 3D
// ladar point cloud background.

void Movie::remap_window_corners(
   double rho,const threevector& camera_XYZ,const threevector& khat,
   const vector<threevector>& UV_corner_dir)
{
//   cout << "inside Movie::remap_window_corners() #1" << endl;
//   cout << "camera_XYZ = " << camera_XYZ << endl;
//   cout << "rho = " << rho << endl;
//   cout << "khat = " << khat << endl;

// First generate geometrical plane in which movie window resides:   

   threevector l=camera_XYZ+rho*khat;  	// l = point on movie's image plane

   delete imageplane_ptr;
   imageplane_ptr=new plane(khat,l);	// khat = movie plane normal
//   cout << "imageplane = " << *imageplane_ptr << endl;
   remap_window_corners(camera_XYZ,imageplane_ptr,UV_corner_dir);
}

// ----------------------------------------------------------------
void Movie::remap_window_corners(
   const threevector& camera_XYZ,plane* image_plane_ptr,
   const vector<threevector>& UV_corner_dir)
{
//   cout << "inside Movie::remap_window_corners() #2" << endl;
//   cout << "*image_plane_ptr = " << *image_plane_ptr << endl;
//   cout << "camera_XYZ = " << camera_XYZ << endl;
//   cout << "khat = " << khat << endl;

// Compute intercept locations of UV corner rays with image plane:

   vector<threevector> UV_corner_posns;
   for (unsigned int n=0; n<UV_corner_dir.size(); n++)
   {
      threevector curr_intersection_pt;
      image_plane_ptr->ray_intersection(
         camera_XYZ,UV_corner_dir[n],curr_intersection_pt);
      UV_corner_posns.push_back(curr_intersection_pt);

//      double phi,theta;
//      mathfunc::decompose_direction_vector(UV_corner_dir[n],phi,theta);
//      cout << "n = " << n 
//           << " UV_corner_dir = " << UV_corner_dir[n]
//           << " UV_corner_posn = " << UV_corner_posns.back() << endl;
//      cout << "UV_corner_dir.khat = "
//           << khat.dot(UV_corner_dir[n]) << endl;
//      cout << "phi = " << phi*180/PI << " theta = " << theta*180/PI
//           << endl;
   }

//   threevector Uaxis(UV_corner_posns[1]-UV_corner_posns[0]);
//   threevector Uaxis2(UV_corner_posns[2]-UV_corner_posns[3]);
//   threevector Vaxis(UV_corner_posns[3]-UV_corner_posns[0]);
//   threevector Vaxis2(UV_corner_posns[2]-UV_corner_posns[1]);
//   cout << "Uaxis = " << Uaxis << endl;
//   cout << "Uaxis2 = " << Uaxis2 << endl;
//   cout << " Vaxis = " << Vaxis << endl;
//   cout << " Vaxis2 = " << Vaxis2 << endl;

//   threevector Uhat=Uaxis.unitvector();
//   threevector Uhat2=Uaxis2.unitvector();
//   threevector Vhat=Vaxis.unitvector();
//   threevector Vhat2=Vaxis2.unitvector();

//   cout << "Uhat = " << Uhat << endl;
//   cout << "Uhat2 = " << Uhat2 << endl;
//   cout << "Vhat = " << Vhat << endl;
//   cout << "Vhat2 = " << Vhat2 << endl;
//   cout << "Uhat.Uhat2 = " << Uhat.dot(Uhat2) << endl;
//   cout << "Vhat.Vhat2 = " << Vhat.dot(Vhat2) << endl;
//   cout << "Uhat.Vhat = " << Uhat.dot(Vhat) << endl;
//   cout << "Uhat2.Vhat2 = " << Uhat2.dot(Vhat2) << endl;

//   double theta=acos(Uhat.dot(Vhat));
//   cout << "theta = " << theta*180/PI << endl;

//   cout << "Uhat.khat = " << Uhat.dot(khat) << endl;
//   cout << "Uhat2.khat = " << Uhat2.dot(khat) << endl;
//   cout << "Vhat.khat = " << Vhat.dot(khat) << endl;
//   cout << "Vhat2.khat = " << Vhat2.dot(khat) << endl;

// Note: On 9/23/07, we explicitly confirmed that for an input
// rectangular photograph, Uaxis = Uaxis2 and Vaxis = Vaxis2.
// Moreover, Uhat.Uhat2 = Vhat.Vhat2 = 1, Uhat.khat = 0 and
// Vhat.khat=0 and acos(Uhat.Vhat) = theta = pixel skew angle (should
// be close to 90 degrees): So 2D photograph generally maps to a
// PARALLELOGRAM in 3D world-space:

   reset_geom_vertices(
      UV_corner_posns[1]-camera_XYZ,
      UV_corner_posns[0]-camera_XYZ,
      UV_corner_posns[3]-camera_XYZ,
      UV_corner_posns[2]-camera_XYZ);
}

// ----------------------------------------------------------------
// This next overloaded version of remap_window_corners() assumes that
// an imageplane has been defined within *get_camera_ptr().  It then
// projects the camera's image onto that image plane.

void Movie::remap_window_corners()
{
//   cout << "inside Movie::remap_window_corners() with no args" << endl;
   remap_window_corners(
      get_camera_ptr()->get_world_posn(),
      get_camera_ptr()->get_imageplane_ptr(),
      get_camera_ptr()->get_UV_corner_world_ray());
}

// ----------------------------------------------------------------
// Member function remap_orthographic_window_corners takes in
// orthographic scaling parameter k and image plane translation
// parameters u0 and v0.  It also takes in parameter rho which governs
// the image plane's displacement along what=uhat x vhat away from the
// 3D origin.  This method maps the 2D coordinates of the Movie's
// corners to their 3-space locations and resets the G99VideoDisplay's
// geometry vertices to these locations.  It also saves the U and V
// axes directions and scales for possible future ImageFrame display.

void Movie::remap_orthographic_window_corners(
   double curr_t,int pass_number,
   double k,double u0,double v0,double rho,
   const threevector& uhat,const threevector& vhat)
{
//   cout << "inside Movie::remap_orthographic_window_corners()" << endl;

   vector<twovector> UV_corners;

// Top right corner:

   UV_corners.push_back(twovector(get_maxU(),get_maxV()));

// Top left corner:

   UV_corners.push_back(twovector(get_minU(),get_maxV()));

// Bottom left corner = image frame origin:

   UV_corners.push_back(twovector(get_minU(),get_minV()));
   set_frame_origin(UV_corners.back());

// Bottom right corner:

   UV_corners.push_back(twovector(get_maxU(),get_minV()));
   
   threevector what=uhat.cross(vhat);
   vector<threevector> UV_3D_corners;
   
   for (unsigned int i=0; i<UV_corners.size(); i++)
   {
      twovector curr_UV_corner=UV_corners[i];
      double u_dotproduct=(curr_UV_corner.get(0)-u0)/k;
      double v_dotproduct=(curr_UV_corner.get(1)-v0)/k;
      UV_3D_corners.push_back(
         u_dotproduct*uhat+v_dotproduct*vhat+rho*what);
   }
// Store UVW scales and direction vectors for animation playback purposes:

   double Uscale=(UV_3D_corners[1]-UV_3D_corners[0]).magnitude();
   double Vscale=(UV_3D_corners[2]-UV_3D_corners[1]).magnitude();

   set_UVW_scales(curr_t,pass_number,Uscale,Vscale);
   set_UVW_dirs(curr_t,pass_number,uhat,vhat);   

// Take movie's bottom left corner as origin point:

   set_UVW_coords(curr_t,pass_number,UV_3D_corners[2]);
   set_stationary_Graphical_flag(false);
}

/*
// ----------------------------------------------------------------
// Member function update_movie_window retrieves the U and V axes'
// directions and scales corresponding to the current time and pass
// number.  This methods uses this U/V axis information to reset the
// movie window's corner vertices relative to the bottom left corner.
// This method is repeatedly called by MoviesGroup::update_display()
// and was written to handle dynamic movie windows not appearing
// within OBSFRUSTA.

// Note added on 2/9/09: We cannot find anywhere within our entire
// code tree where dynamic_window_flag is set to true.  If not, this
// method is never called...

void Movie::update_movie_window(double curr_t,int pass_number)
{
   cout << "inside Movie::update_movie_window()" << endl;
   
   double Uscale,Vscale,Wscale;
   get_UVW_scales(curr_t,pass_number,Uscale,Vscale,Wscale);
//   cout << "Uscale = " << Uscale << " Vscale = " << Vscale 
//        << " Wscale = " << Wscale << endl;

   threevector uhat,vhat,what;
   get_UVW_dirs(curr_t,pass_number,uhat,vhat,what);
//   cout << "uhat = " << uhat << " vhat = " << vhat << endl;
   
   threevector bottom_left=Zero_vector;
   threevector bottom_right=bottom_left+Uscale*uhat;
   threevector top_left=bottom_left+Vscale*vhat;
   threevector top_right=top_left+Uscale*uhat;
   
   reset_geom_vertices(bottom_right,bottom_left,top_left,top_right);
   
   threevector frame_origin;
   get_UVW_coords(curr_t,pass_number,frame_origin);
   set_frame_origin(frame_origin);

//   cout << "frame_origin = " << frame_origin << endl;
}
*/

// ----------------------------------------------------------------
// Member function set_viewpoint translates and rotates the 3D custom
// manipulator's camera so that its position and orientation match
// those stored in *get_camera_ptr().  In general, the two cameras'
// fields-of-view will not precisely match.  But within the common
// angular volumes of their view frusta, one should see good agreement
// between scene objects.

void Movie::set_CM3D_viewpoint(double curr_t,int pass_number)
{
   if (!CM_3D_refptr.valid()) return;
   
   CM_3D_refptr->set_worldspace_center(get_camera_ptr()->get_world_posn());
   CM_3D_refptr->set_eye_to_center_distance(0);

   threevector Uhat,Vhat,What;
   get_UVW_dirs(curr_t,pass_number,Uhat,Vhat,What);

// For 2D image viewing purposes, we want Vhat to be aligned with the
// SCREEN's vertical direction which corresponds to world yhat rather
// than zhat:

   bool align_Vhat_with_zhat=false;
   osg::Quat q=retrieve_camera_quaternion(
      curr_t,pass_number,align_Vhat_with_zhat);

   CM_3D_refptr->set_rotation_quat(q);
}

// ========================================================================
// Average background image generaton member functions:
// ========================================================================

void Movie::generate_average_background_image()
{
   cout << "inside Movie::generate_avg_background_image()" << endl;

   if (get_RGBA_twoDarray(0)==NULL)
   {
      generate_RGBA_twoDarrays();
   }
   
   convert_charstar_array_to_RGBA_twoDarrays(get_m_image_ptr());

   unsigned char* data_ptr=convert_RGBAarrays_to_charstar_array();
   for (int i=0; i<int(getWidth()*getHeight()*getNchannels()); i++)
   {
      if (i%3==0)
      {
         *(get_m_image_ptr()+i)=*(data_ptr+i);
      }
      else
      {
         *(get_m_image_ptr()+i)=0;
      }
      
   } // loop over index i labeling data_ptr array elements
   
   set_image();
}

// ========================================================================
// Georegistation member functions
// ========================================================================

// Member function georegister_subtexture_corners takes in a bounding
// box in longitude,latitude space.  This method computes and resets
// the corresponding sub-texture fractions (ranging from 0 to 1 in
// both the horizontal and vertical directions!) for the movie's 4
// corners. 

// It is important to note that the lower left and upper right corners
// will be correctly georegistered.  However, the longitude and
// latitude coordinates for the lower right and upper left will NOT
// generally be given by (max_longitude,min_latitude) and
// (min_longitude,max_latitude).  And the geocoordinates for all other
// pixel locations within the subtexture will be only approximately
// correct as well.

// If the input longitude-latitude bbox coordinates and output texture
// fraction coordinates are valid, this boolean method returns true.

bool Movie::georegister_subtexture_corners(
   double min_longitude,double max_longitude,
   double min_latitude,double max_latitude)
{
//   cout << "inside Movie::georegister_subtexture_corners()" << endl;
//   cout << "min_long = " << min_longitude
//        << " max_long = " << max_longitude << endl;
//   cout << "min_lat = " << min_latitude
//        << " max_lat = " << max_latitude << endl;

   if (min_longitude >= max_longitude ||
       min_latitude >= max_latitude) return false;

   if (video_corner_UTM_coords.size()==0) return false;

   twovector geo_lower_left(video_corner_UTM_coords[0]);
   twovector geo_lower_right(video_corner_UTM_coords[1]);
   twovector geo_upper_right(video_corner_UTM_coords[2]);
   twovector geo_upper_left(video_corner_UTM_coords[3]);
//   cout << "geo_lower_left = " << geo_lower_left << endl;
//   cout << "geo_upper_right = " << geo_upper_right << endl;

// Recall that the UTM zonenumbers for the bbox corners may not all
// match.  If they do not, we work within the UTM zone whose easting
// and northing coordinates are closest to those within
// video_corner_UTM_coords:

   geopoint lower_left_bbox_corner(min_longitude,min_latitude);
   int lower_left_UTM_zonenumber=lower_left_bbox_corner.get_UTM_zonenumber();
//   cout << "lower_left_UTM_zonenumber = " 
//        << lower_left_UTM_zonenumber << endl;
   geopoint upper_right_bbox_corner(max_longitude,max_latitude);
   int upper_right_UTM_zonenumber=
      upper_right_bbox_corner.get_UTM_zonenumber();
//   cout << "upper_right_UTM_zonenumber = "
//        << upper_right_UTM_zonenumber << endl;

   int UTM_zonenumber=lower_left_UTM_zonenumber;
   if (upper_right_UTM_zonenumber > lower_left_UTM_zonenumber)
   {
      twovector lower_left_delta(
         lower_left_bbox_corner.get_UTM_easting()-geo_lower_left.get(0),
         lower_left_bbox_corner.get_UTM_northing()-geo_lower_left.get(1));
      twovector upper_right_delta(
         upper_right_bbox_corner.get_UTM_easting()-geo_upper_right.get(0),
         upper_right_bbox_corner.get_UTM_northing()-geo_upper_right.get(1));
      if (upper_right_delta.magnitude() < lower_left_delta.magnitude())
      {
         UTM_zonenumber=upper_right_UTM_zonenumber;
      }
   }
//   cout << "UTM_zonenumber = " << UTM_zonenumber << endl;

   double altitude=0;
   lower_left_bbox_corner=geopoint(
      min_longitude,min_latitude,altitude,UTM_zonenumber);
   geopoint lower_right_bbox_corner(
      max_longitude,min_latitude,altitude,UTM_zonenumber);
   geopoint upper_left_bbox_corner(
      min_longitude,max_latitude,altitude,UTM_zonenumber);
   upper_right_bbox_corner=geopoint(
      max_longitude,max_latitude,altitude,UTM_zonenumber);

   twovector screen_lower_left(get_minU(),get_minV());
   twovector screen_lower_right(get_maxU(),get_minV());
   twovector screen_upper_right(get_maxU(),get_maxV());
   twovector screen_upper_left(get_minU(),get_maxV());

// Reconstruct M and trans given input and output corner twovectors:

   twovector Delta_screen_H(screen_lower_right-screen_lower_left);
   twovector Delta_screen_V(screen_upper_left-screen_lower_left);

   twovector Delta_geo_H(geo_lower_right-geo_lower_left);
   twovector Delta_geo_V(geo_upper_left-geo_lower_left);

   double alpha=Delta_geo_H.get(0);
   double beta=Delta_geo_H.get(1);
   double gamma=Delta_geo_V.get(0);
   double delta=Delta_geo_V.get(1);
   double determ=alpha*delta-beta*gamma;
   
   double Mxx=(delta*Delta_screen_H-beta*Delta_screen_V).get(0)/determ;
   double Myx=(delta*Delta_screen_H-beta*Delta_screen_V).get(1)/determ;
   double Mxy=-(gamma*Delta_screen_H-alpha*Delta_screen_V).get(0)/determ;
   double Myy=-(gamma*Delta_screen_H-alpha*Delta_screen_V).get(1)/determ;

   genmatrix M(2,2);
   M.put(0,0,Mxx);
   M.put(0,1,Mxy);
   M.put(1,0,Myx);
   M.put(1,1,Myy);
//   cout << "M = " << M << endl;

   twovector trans=screen_lower_left-M*geo_lower_left;
//   cout << "trans = " << trans << endl;

   twovector geo_lower_left_bbox_corner(
      lower_left_bbox_corner.get_UTM_easting(),
      lower_left_bbox_corner.get_UTM_northing());
   twovector geo_lower_right_bbox_corner(
      lower_right_bbox_corner.get_UTM_easting(),
      lower_right_bbox_corner.get_UTM_northing());
   twovector geo_upper_right_bbox_corner(
      upper_right_bbox_corner.get_UTM_easting(),
      upper_right_bbox_corner.get_UTM_northing());
   twovector geo_upper_left_bbox_corner(
      upper_left_bbox_corner.get_UTM_easting(),
      upper_left_bbox_corner.get_UTM_northing());

//   cout << "geo_lower_left_bbox_corner = " 
//        << geo_lower_left_bbox_corner << endl;
//   cout << "geo_upper_right_bbox_corner = "
//        << geo_upper_right_bbox_corner << endl;

   set_texture_fracs(
      M*geo_lower_left_bbox_corner+trans,
      M*geo_lower_right_bbox_corner+trans,
      M*geo_upper_right_bbox_corner+trans,
      M*geo_upper_left_bbox_corner+trans);

//   cout << "lower_left_corner_fracs = " 
//        << get_lower_left_texture_frac() << endl;
//   cout << "lower_right_corner_fracs = " 
//        << get_lower_right_texture_frac() << endl;
//   cout << "upper_right_corner_fracs = " 
//        << get_upper_right_texture_frac() << endl;
//   cout << "upper_left_corner_fracs = " 
//        << get_upper_left_texture_frac() << endl;

   if ((get_lower_left_texture_frac().get(0) >= 
        get_upper_right_texture_frac().get(0)) ||
       (get_lower_left_texture_frac().get(1) >= 
        get_upper_right_texture_frac().get(1)))
   {
      return false;
   }
   else
   {
      return true;
   }
}

// ----------------------------------------------------------------
// Member function export_current_subframe takes in coordinates for a
// longitude-latitude bounding box.  This method assumes that the
// current Movie object corresponds to an aerial nadir video which is
// reasonably well georegistered.  It chips out from the current frame
// a sub-image corresponding to the input bbox and exports it to a PNG
// file whose name contains the bbox information.

bool Movie::export_current_subframe(
   double min_longitude,double max_longitude,
   double min_latitude,double max_latitude,
   bool draw_central_bbox_flag,int n_horiz_output_pixels)
{
   string output_filename,output_subdir;
   return export_current_subframe(
      min_longitude,max_longitude,min_latitude,max_latitude,
      output_subdir,output_filename,
      draw_central_bbox_flag,n_horiz_output_pixels);
}

bool Movie::export_current_subframe(
   double min_longitude,double max_longitude,
   double min_latitude,double max_latitude,string output_subdir,
   bool draw_central_bbox_flag,int n_horiz_output_pixels)
{
   string output_filename=filefunc::getbasename(get_video_filename());   
   return export_current_subframe(
      min_longitude,max_longitude,min_latitude,max_latitude,
      output_subdir,output_filename,
      draw_central_bbox_flag,n_horiz_output_pixels);
}

bool Movie::export_current_subframe(
   double min_longitude,double max_longitude,
   double min_latitude,double max_latitude,
   string output_subdir,string output_filename,
   bool draw_central_bbox_flag,int n_horiz_output_pixels)
{
//   cout << "inside Movie::export_current_subframe() #3" << endl;
//   cout << "output_subdir = " << output_subdir << endl;
//   cout << "output_filename = " << output_filename << endl;

//   cout << "min_long = " << min_longitude
//        << " max_long = " << max_longitude << endl;
//   cout << "min_lat = " << min_latitude
//        << " max_lat = " << max_latitude << endl;

   if (!georegister_subtexture_corners(
      min_longitude,max_longitude,min_latitude,max_latitude)) return false;

   string output_image_suffix="png";
//   string output_image_suffix="rgb";

// Retrieve current subframe byte data and store within
// G99VideoDisplay member variable output_image_string:

   get_texture_rectangle_ptr()->retrieve_curr_subframe_byte_data(
      get_lower_left_texture_frac(),get_upper_right_texture_frac(),
      output_image_suffix,draw_central_bbox_flag,n_horiz_output_pixels);

   if (output_filename.size() > 0)
   {
      filefunc::add_trailing_dir_slash(output_subdir);
      filefunc::dircreate(output_subdir);

      const int nprecision=6;
      string min_long_str=stringfunc::number_to_string(
         min_longitude,nprecision);
      string max_long_str=stringfunc::number_to_string(
         max_longitude,nprecision);
      string min_lat_str=stringfunc::number_to_string(
         min_latitude,nprecision);
      string max_lat_str=stringfunc::number_to_string(
         max_latitude,nprecision);

      double curr_time=get_AnimationController_ptr()->
         get_time_corresponding_to_curr_frame();
//      cout << "curr_time = " << curr_time
//           << " world time = " 
//           << get_AnimationController_ptr()->get_world_time_string() << endl;
      string time_str=stringfunc::number_to_string(curr_time,2);

//      output_filename += "_"+min_long_str;
//      output_filename += "_"+min_lat_str;
//      output_filename += "_"+max_long_str;
//      output_filename += "_"+max_lat_str;
//      output_filename += "_"+time_str;
      output_filename=output_subdir+output_filename+"."+output_image_suffix;
      string output_filename="currframe.png";
//      cout << "output_filename = " << output_filename << endl;

//      int n_horiz_output_pixels=-1;
//      int n_horiz_output_pixels=512;
//      int n_horiz_output_pixels=800;
//      int n_horiz_output_pixels=1000;
      get_texture_rectangle_ptr()->write_curr_frame(
         get_lower_left_texture_frac(),get_upper_right_texture_frac(),
         output_filename,n_horiz_output_pixels);
   } // output_filename.size() > 0 conditional

// Reset texture fractions back to their default values after subframe
// has been exported:

   set_default_texture_fracs();

   return true;
}

// ---------------------------------------------------------------------
// Member function export_current_frame()

void Movie::export_current_frame()
{
//   cout << "inside Movie::export_current_frame()" << endl;
   string output_subdir="./movie_frames/";
   filefunc::dircreate(output_subdir);
   int curr_framenumber=get_curr_framenumber();
   string suffix=".png";
   string output_filename=output_subdir+"frame_"
      +stringfunc::number_to_string(curr_framenumber)+suffix;
//   cout << "output_filename = " << output_filename << endl;
   get_texture_rectangle_ptr()->write_curr_frame(output_filename);
}
   
// ========================================================================
// 2D movie superposition member functions
// ========================================================================

// Member function construct_video_frame_corners fills and returns an
// STL vector containing threevectors corresponding to the current
// Movie corners' (U,V) coordinates.  This STL vector is needed for
// transforming and displaying an instantaneous video on top of a
// background panorama image.

vector<threevector> Movie::construct_video_frame_corners() const
{
   threevector bottom_left(get_minU(),get_minV(),1);
   threevector bottom_right(get_maxU(),get_minV(),1);
   threevector top_left(get_minU(),get_maxV(),1);
   threevector top_right(get_maxU(),get_maxV(),1);

   vector<threevector> video_frame_corners;
   video_frame_corners.push_back(bottom_right);
   video_frame_corners.push_back(bottom_left);
   video_frame_corners.push_back(top_left);
   video_frame_corners.push_back(top_right);
   return video_frame_corners;
}

// ----------------------------------------------------------------
// Member function compute_video_frame_corners_in_panorama() takes in
// homography *curr_H_ptr which maps the current video frame within
// *video_ptr onto a static background panorama image.  This method
// computes the panorama (U,V) coordinates of the video frame's
// corners.  It resets the video's geometry vertices so that the 2D
// video frame appears overlaid on top of the 2D panorama.

vector<threevector> Movie::compute_video_frame_corners_in_panorama(
   int video_width,int video_height,int panorama_width,int panorama_height,
   homography* curr_H_ptr,const vector<threevector>& video_frame_corners)
{
   double panorama_u,panorama_v;

// For 2D movie viewing, recall U axis points towards screen right
// (+X), V axis points towards screen up (+Z), and W axis points out
// of screen (-Y).  So in order to place the instantaneous video frame
// on top of the background static panorama, we need to displace it by
// a small amount in the negative Y direction:

   const double y_displacement=-0.01;	

   vector<threevector> panorama_frame_coords;
   for (unsigned int c=0; c<video_frame_corners.size(); c++)
   {
      threevector curr_video_frame_corners=video_frame_corners[c];
      curr_H_ptr->convert_video_to_panorama_coords(
         video_width,video_height,panorama_width,panorama_height,
         curr_video_frame_corners.get(0),curr_video_frame_corners.get(1),
         panorama_u,panorama_v);
      panorama_frame_coords.push_back(
         threevector(panorama_u,y_displacement,panorama_v));
   }
   return panorama_frame_coords;
}

// ----------------------------------------------------------------
void Movie::superpose_video_frame_on_panorama(
   const vector<threevector>& panorama_frame_coords)
{
   reset_geom_vertices(panorama_frame_coords[0],panorama_frame_coords[1],
                       panorama_frame_coords[2],panorama_frame_coords[3]);
}

// ----------------------------------------------------------------
// Member function imageplane_location() takes in a pair of 2D UV
// coordinates.  It returns the corresponding 3D point which is
// displaced in the What direction relative to the image plane by
// forward_fraction of the range between the camera's location and the
// image plane.

threevector Movie::imageplane_location(const twovector& UV)
{
//   cout << "inside Movie::imageplane_location()" << endl;
//   cout << "UV = " << UV << endl;

   threevector camera_XYZ=get_camera_ptr()->get_world_posn();
//   cout << "camera_XYZ = " << camera_XYZ << endl;
   threevector intersection_pnt;
   imageplane_ptr->ray_intersection(
      camera_XYZ,get_camera_ptr()->pixel_ray_direction(UV),intersection_pnt);

   const double forward_fraction=0.05;
   double delta_rho=forward_fraction*(
      intersection_pnt-camera_XYZ).magnitude();
//   cout << "delta_rho = " << delta_rho << endl;

   intersection_pnt += delta_rho*get_camera_ptr()->get_What();
   return intersection_pnt;
}

// ========================================================================
// Intensity content manipulation member functions:
// ========================================================================

void Movie::null_region_outside_poly(const polygon& poly)
{
//   cout << "inside Movie::null_region_outside_poly()" << endl;
   
   twoDarray* ztwoDarray_ptr=new twoDarray(getWidth(),getHeight());
   ztwoDarray_ptr->init_coord_system(
      get_minU(),get_maxU(),get_minV(),get_maxV());
   ztwoDarray_ptr->clear_values();

   cout << "Marking pixels lying within polygon interior:" << endl;
   const double intensity_value=1.0;
   drawfunc::color_polygon_interior(poly,intensity_value,ztwoDarray_ptr);

   cout << "Nulling pixels lying outside polygon:" << endl;
   int counter=0;

   int R=0;
   int G=0;
   int B=0;
   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         if (nearly_equal(ztwoDarray_ptr->get(px,py),0))
         {
            set_pixel_RGB_values(px,py,R,G,B);
            counter++;
         }
      }
   }
   delete ztwoDarray_ptr;
   cout << "Number of nulled exterior pixels = " << counter << endl;

   set_image();
}

// ========================================================================
// Photograph manipulation member functions
// ========================================================================

// Member function reset_displayed_photograph() takes in the filename
// for some photo.  It overwrites the current Movie's texture with the
// new photo which can differ in size.

void Movie::reset_displayed_photograph(string photo_filename,bool twoD_flag)
{
//   cout << "inside Movie::reset_displayed_photograph()" << endl;
//   cout << "photo_filename = " << photo_filename << endl;

   get_texture_rectangle_ptr()->reset_texture_content(photo_filename);

   if (warp_onto_imageplane_flag)
   {

// As of Dec 2009, we assume that if the movie camera's imageplane has
// been defined, then the camera's corresponding photo should be
// backprojected on this imageplane.  If so, we need to call
// Movie::warp_photo_onto_imageplane() in order to reset the texture
// matrix TexMat:

      plane* imageplane_ptr=get_camera_ptr()->get_imageplane_ptr();
//      cout << "*imageplane_ptr = " << *imageplane_ptr << endl;

      warp_photo_onto_imageplane(*imageplane_ptr);
//      set_alpha(0.5);
      set_alpha(1.0);
   }
   else
   {
      reset_texture_coords();
   }

   if (twoD_flag)
   {
      compute_2D_chip(
         get_lower_left_texture_frac(),
         get_lower_right_texture_frac(),
         get_upper_right_texture_frac(),
         get_upper_left_texture_frac());
   }

// For reasons we don't understand as of 4/19/09, calling
// Movie::set_alpha() resets the thumbnail texture so that it fills
// the entire OBSFRUSTUM image plane when we zoom out from an
// OBSFRUSTUM.

   set_alpha(get_alpha());
}

// ----------------------------------------------------------------
// Member function warp_photo_onto_imageplane() takes in an infinite
// imageplane.  This method first checks whether the imageplane lies
// entirely within the camera's forward field-of-view.  If not, this
// boolean method returns false.  Otherwise, it extracts the camera's
// 3x4 projection matrix and promotes it to a 4x4 matrix P4.  It next
// forms 4x4 matrix Q4 which maps (U,V,W,W) fourvectors onto 2D
// texture coordinate fourvectors such as (width,height,1,1).  The
// total 4x4 texture matrix which maps 3D space points on the
// imageplane into 2D texture coordinates Mtot=Q4*P4.  This method
// resets both the geometrical vertices and the texture coordinates
// for the camera's photo equal to the 3D points corresponding to the
// intercepted image plane's corners.

// We wrote this method in Dec 2009 after a week of struggling with
// trying to reproduce Noah Snavely's approach to warping rectangular
// photos onto OpenGL quads in 3-space.  We learned the painful and
// hard way that we CANNOT just reset the quad corner 3D vertices.  By
// default, OpenGL breaks apart a quad into 2 triangles and textures
// those independently.  So there is an obvious first-derivative
// discontinuity across the common inner edge of the two triangles
// making up the quad.  Instead, we have to use an OpenGL texture
// matrix in order to perspectively project a rectangular photo onto
// an arbitrary quad!

bool Movie::warp_photo_onto_imageplane(const plane& imageplane)
{
//   cout << "inside Movie::warp_photo_onto_imageplane()" << endl;
   
   return warp_photo_onto_imageplane(imageplane,get_camera_ptr());
}

bool Movie::warp_photo_onto_imageplane(
   const plane& imageplane,camera* camera_ptr)
{
//   cout << "inside Movie::warp_photo_onto_imageplane()" << endl;
   
// First make sure all corner rays from camera's view frustum actually
// intercept image plane.  If not, we cannot project the photo onto
// the image plane!

   threevector true_camera_world_posn=camera_ptr->get_world_posn();

   vector<threevector> UV_corner_world_ray=
      camera_ptr->get_UV_corner_world_ray();
   bool all_corner_rays_intersect_imageplane_flag=true;
   for (unsigned int c=0; c<UV_corner_world_ray.size(); c++)
   {
      bool corner_ray_intersects_imageplane_flag=
         imageplane.ray_intersection(true_camera_world_posn,
         UV_corner_world_ray[c]);
      if (!corner_ray_intersects_imageplane_flag)
         all_corner_rays_intersect_imageplane_flag=false;
         
//      cout << "c = " << c << " corner ray = "
//           << UV_corner_world_ray[c] << endl;
//      cout << "ray_intersects_plane = " 
//           << corner_ray_intersects_imageplane_flag << endl;
   }
   if (!all_corner_rays_intersect_imageplane_flag) return false;

//   cout << "true_camera_world_posn = " << true_camera_world_posn << endl;
//   cout << "imageplane origin = " << imageplane.get_origin() << endl;

// Set quad corners equal to intercept of UV corner rays with input
// imageplane.  Recall call to remap_window_corners() resets the
// quad's vertices *RELATIVE* to the camera's world position.  So we
// need to temporarily reset the camera's world position to (0,0,0)
// and recompute the camera's projection matrix.  Only then can we
// proceed to compute the texture matrix and assign the photo's
// texture coordinates equal to the *RELATIVE* 3D geometry vertices':

   camera_ptr->set_imageplane(imageplane.get_pi());
   remap_window_corners();
   camera_ptr->set_world_posn(Zero_vector);
   camera_ptr->construct_projection_matrix();

// First promote 3x4 projection matrix *P_ptr to a 4x4 matrix P4 by
// copying the 3rd row of the former into the 4th row of the latter.
// Output (U,V,W) threevectors from *P_ptr become (U,V,W,W)
// fourvectors from P4:

   const genmatrix* P_ptr=camera_ptr->get_P_ptr();
//   cout << "*P_ptr = " << *P_ptr << endl;
   genmatrix P4(4,4);

   for (int c=0; c<4; c++)
   {
      for (int r=0; r<3; r++)
      {
         P4.put(r,c,P_ptr->get(r,c));
      }
      P4.put(3,c,P_ptr->get(2,c));
   }
//   cout << "P4 = " << P4 << endl;

// Recover 3D space points for 4 corners of imageplane quad onto which
// photo will be projected:

   osg::Vec3Array* vertices_ptr=get_vertices_ptr();
   bottom_left_XYZ=threevector(vertices_ptr->at(0));
   top_left_XYZ=threevector(vertices_ptr->at(1));
   top_right_XYZ=threevector(vertices_ptr->at(2));
   bottom_right_XYZ=threevector(vertices_ptr->at(3));
         
//   cout << "bottom_left_XYZ = " << bottom_left_XYZ << endl;
//   cout << "bottom_right_XYZ = " << bottom_right_XYZ << endl;
//   cout << "top_right_XYZ = " << top_right_XYZ << endl;
//   cout << "top_left_XYZ = " << top_left_XYZ << endl;

//    double width=getWidth();
   double height=getHeight();

/*
   string homography_subdir="./homographies/";
   filefunc::dircreate(homography_subdir);
   string movie_filename=filefunc::getprefix(get_video_filename());
   int thumbnail_posn=
      stringfunc::first_substring_location(movie_filename,"thumbnail");
//   cout << "movie_filename = " << movie_filename
//        << " thumbnail_posn = " << thumbnail_posn << endl;

   string homography_filename=homography_subdir+movie_filename+".H";
   if (!filefunc::fileexist(homography_filename) && thumbnail_posn < 0)
   {
      threevector bottom_left_UV(0,0);
      threevector top_left_UV(0,height);
      threevector top_right_UV(width,height);
      threevector bottom_right_UV(width,0);

      vector<threevector> XYZ;
      XYZ.push_back(bottom_left_XYZ);
      XYZ.push_back(bottom_right_XYZ);
      XYZ.push_back(top_right_XYZ);
      XYZ.push_back(top_left_XYZ);

      vector<threevector> UVW;
      UVW.push_back(bottom_left_UV);
      UVW.push_back(bottom_right_UV);
      UVW.push_back(top_right_UV);
      UVW.push_back(top_left_UV);
      
      homography H;
      H.parse_homography_inputs(UVW,XYZ);
      H.compute_homography_matrix();
      H.check_homography_matrix(UVW,XYZ);
      H.export_matrix(homography_filename);
      cout << "Exported homography to " << homography_filename << endl;
   }
*/

//   threevector bottom_left_UV=*P_ptr*fourvector(bottom_left_XYZ,1);
//   threevector top_left_UV=*P_ptr*fourvector(top_left_XYZ,1);
//   threevector top_right_UV=*P_ptr*fourvector(top_right_XYZ,1);
//   threevector bottom_right_UV=*P_ptr*fourvector(bottom_right_XYZ,1);

//   cout << "bottom_left_UV = " << bottom_left_UV << endl;
//   cout << "top_left_UV = " << top_left_UV << endl;
//   cout << "top_right_UV = " << top_right_UV << endl;
//   cout << "bottom_right_UV = " << bottom_right_UV << endl;

/*
   fourvector bottom_left_UVW=P4*fourvector(bottom_left_XYZ,1);
   fourvector top_left_UVW=P4*fourvector(top_left_XYZ,1);
   fourvector top_right_UVW=P4*fourvector(top_right_XYZ,1);
   fourvector bottom_right_UVW=P4*fourvector(bottom_right_XYZ,1);

   cout << "P*bottom_left_XYZ = " << bottom_left_UV << endl;
   cout << "P*bottom_right_XYZ = " << bottom_right_UV << endl;
   cout << "P*top_right_XYZ = " << top_right_UV << endl;
   cout << "P*top_left_XYZ = " << top_left_UV << endl;

   cout << "P4*bottom_left_XYZ = " << bottom_left_UVW << endl;
   cout << "P4*bottom_right_XYZ = " << bottom_right_UVW << endl;
   cout << "P4*top_right_XYZ = " << top_right_UVW << endl;
   cout << "P4*top_left_XYZ = " << top_left_UVW << endl;
*/

// As of Dec 2009, our photograph texture coordinates range over the
// following limits:

// Top left photo corner: (0,0)
// Top right photo corner: (width,0)
// Bottom right photo corner: (width,height)
// Bottom left photo corner: (0,height)

// Construct 4x4 matrix Q4 which maps (U,V,W,W) corners onto these
// texture coordinates:


   genmatrix Q4(4,4);
   Q4.identity();
   Q4.put(0,0,height);
   Q4.put(1,1,-height);
   Q4.put(1,2,height);
//   cout << "Q4 = " << Q4 << endl;

// Total texture matrix which maps 3-space corner XYZ points onto 2D
// (expressed as four vectors) texel coordinates Mtot=Q4 * P4:

   genmatrix Mtot(4,4);
   Mtot=Q4*P4;

   osg::Matrixd M;
   M.set(Mtot.get(0,0) , Mtot.get(0,1) , Mtot.get(0,2), Mtot.get(0,3) ,
         Mtot.get(1,0) , Mtot.get(1,1) , Mtot.get(1,2), Mtot.get(1,3) ,
         Mtot.get(2,0) , Mtot.get(2,1) , Mtot.get(2,2), Mtot.get(2,3) ,
         Mtot.get(3,0) , Mtot.get(3,1) , Mtot.get(3,2), Mtot.get(3,3));

/*
   fourvector bottom_left_WH=Mtot*fourvector(bottom_left_XYZ,1);
   fourvector top_left_WH=Mtot*fourvector(top_left_XYZ,1);
   fourvector top_right_WH=Mtot*fourvector(top_right_XYZ,1);
   fourvector bottom_right_WH=Mtot*fourvector(bottom_right_XYZ,1);

   cout << "Mtot*bottom_left_XYZ = " << bottom_left_WH << endl;
   cout << "Mtot*bottom_right_XYZ = " << bottom_right_WH << endl;
   cout << "Mtot*top_right_XYZ = " << top_right_WH << endl;
   cout << "Mtot*top_left_XYZ = " << top_left_WH << endl;
*/
         
// Set up texture matrix TexMat:

   osg::TexMat* TexMat_ptr=get_TexMat_ptr();
   TexMat_ptr->setMatrix(osgfunc::transpose_matrix(M));

// Follow Noah Snavely and set corner texture coordinates equal to 3D
// vertex coordinates.  Recall that OpenGL multiplies these initial
// texture coordinates by *TexMat_ptr before looking up texture color
// information:

   equate_texture_coords_to_vertices();

// Reset camera's world position to its true value and recompute
// projection matrix:

   camera_ptr->set_world_posn(true_camera_world_posn);
   camera_ptr->construct_projection_matrix();

   return true;
}

// --------------------------------------------------------------------------
// Member function time_dependent_fade_out() resets member var
// fade_start_time if its value is negative.  It then computes a
// fade-in fraction based upon the elapsed time since this method was
// first called and the current time.  The current movie's alpha value
// is set equal to the fade-in fraction with a maximum unity ceiling.

bool Movie::time_dependent_fade_out(double total_fadeout_time)
{
   return time_dependent_fade_out(0,total_fadeout_time);
}

bool Movie::time_dependent_fade_out(
   double begin_fall_time,double end_fall_time)
{
//   cout << "inside Movie::time_dependent_fade_out()" << endl;
//   cout << "get_ID() = " << get_ID() << endl;

   bool min_alpha_reached_flag=false;

   double curr_time=timefunc::elapsed_timeofday_time();
   if (fadeout_start_time < 0)
   {
      fadeout_start_time=curr_time;
   }
   begin_fall_time += fadeout_start_time;
   end_fall_time += fadeout_start_time;

   double fall_time=curr_time-begin_fall_time;
   if (fall_time < 0) fall_time=0;
   if (fall_time > end_fall_time) fall_time=end_fall_time;
   double elapsed_fall_time=curr_time-begin_fall_time;
   double fadeout_frac=1-elapsed_fall_time/(end_fall_time-begin_fall_time);
   
//   cout << "curr_time = " << curr_time
//        << " fadeout_start_time = " << fadeout_start_time 
//        << " elapsed_fadeout_time = " << elapsed_fadeout_time << endl;
//   cout << "fadeout_frac = " << fadeout_frac << endl;

   if (fadeout_frac < 0)
   {
      fadeout_frac=0;
      min_alpha_reached_flag=true;
   }
   set_alpha(fadeout_frac);

//   cout << "alpha = " << get_alpha() << endl;
   return min_alpha_reached_flag;
}

// --------------------------------------------------------------------------
// Member function time_dependent_fade_in() resets member var
// fadein_start_time if its value is negative.  It then computes a
// fade-in fraction based upon the elapsed time since this method was
// first called and the current time.  The current movie's alpha value
// is set equal to the fade-in fraction with a maximum unity ceiling.

bool Movie::time_dependent_fade_in(double total_fadein_time)
{
   return time_dependent_fade_in(0,total_fadein_time);
}

bool Movie::time_dependent_fade_in(
   double begin_rise_time,double end_rise_time)
{
//   cout << "inside Movie::time_dependent_fade_in()" << endl;
//   cout << "get_ID() = " << get_ID() << endl;
//   cout << "fadein_start_time = " << fadein_start_time << endl;

   bool max_alpha_reached_flag=false;

   double curr_time=timefunc::elapsed_timeofday_time();
   if (fadein_start_time < 0)
   {
      fadein_start_time=curr_time;
   }
   begin_rise_time += fadein_start_time;
   end_rise_time += fadein_start_time;
   
   double rise_time=curr_time-begin_rise_time;
   if (rise_time < 0) rise_time=0;
   if (rise_time > end_rise_time) rise_time=end_rise_time;
   double elapsed_rise_time=curr_time-begin_rise_time;
   double fadein_frac=elapsed_rise_time/(end_rise_time-begin_rise_time);

//   cout << "curr_time = " << curr_time
//        << " fadein_start_time = " << fadein_start_time
//        << " elapsed_fadein_time = " << elapsed_fadein_time << endl;
//   cout << "fadein_frac = " << fadein_frac << endl;
   if (fadein_frac >= 1.0)
   {
      fadein_frac=1.0;
      max_alpha_reached_flag=true;
   }
   set_alpha(fadein_frac);

//   cout << "alpha = " << get_alpha() << endl;
   return max_alpha_reached_flag;
}

// ---------------------------------------------------------------------
// Member function import_next_to_latest_photo()

bool Movie::import_next_to_latest_photo()
{
   string latest_photo_filename,next_to_latest_photo_filename;
   if (!filefunc::get_latest_files_in_subdir(
      watch_subdirectory,latest_photo_filename,next_to_latest_photo_filename))
   {
      return false;
   }

   cout << "inside Movie::import_next_to_latest_photo()" << endl;
//   cout << "watch_subdirectory = " << watch_subdirectory << endl;
   cout << "latest_photo_filename = " << latest_photo_filename << endl;
   cout << "next_to_latest_photo_filename = " 
        << next_to_latest_photo_filename << endl;

   bool twoD_flag=true;
//   reset_displayed_photograph(latest_photo_filename,twoD_flag);
   reset_displayed_photograph(next_to_latest_photo_filename,twoD_flag);
   set_video_filename(filefunc::getbasename(next_to_latest_photo_filename));

   return true;
}

// ========================================================================
// Tracking member functions
// ========================================================================

// Specialized member function generate_ESB_car_tracks reads in the
// comma-separated value contents of ESB_tracks_filename, It
// instantiates a set of car tracks and returns them within the output
// dynamically generated STL vector.

Movie::TRACKS_MAP* Movie::generate_ESB_car_tracks() 
{
   cout << "inside Movie::generate_ESB_car_tracks()" << endl;
   
   if (tracks_map_ptr != NULL) return tracks_map_ptr;
   tracks_map_ptr=new TRACKS_MAP;

   string ESB_tracks_filename="improved_blob_tracks.txt";
   filefunc::ReadInfile(ESB_tracks_filename);

   const double xmin=0;
   const double xmax=720;
   const double ymin=0;
   const double ymax=1280;

   int track_counter=0;
   vector<int> imagenumber;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i],",");
      imagenumber.push_back(stringfunc::string_to_integer(substrings[0]));
      
      track* track_ptr=new track(track_counter++);
      unsigned int n_points=(substrings.size()-1)/4;
      for (unsigned int n=0; n<n_points; n++)
      {
         double t=imagenumber.back()+n;
         double x=stringfunc::string_to_number(substrings[n*4+0+1]);
         double y=stringfunc::string_to_number(substrings[n*4+1+1]);

         double u=(y-ymin)/(ymax-ymin) * (ymax-ymin)/(xmax-xmin);
         double v=(x-xmin)/(xmax-xmin);
         track_ptr->set_XYZ_coords(t,threevector(u,v));
      } // loop over index n labeling points along track
//      cout << "i = " << i << " curr_track = " << *track_ptr << endl;
      
      (*tracks_map_ptr)[track_counter++]=track_ptr;
   } // loop over index i labeling text lines corresponding to tracks
   cout << "n_tracks = " << tracks_map_ptr->size() << endl;

   return tracks_map_ptr;
}

// ---------------------------------------------------------------------
// Specialized member function generate_Luke_blob_tracks reads in the
// comma-separated value contents of
 
Movie::TRACKS_MAP* Movie::generate_Luke_blob_tracks(int p)
{
//   cout << "inside Movie::generate_Luke_blob_tracks()" << endl;
   
   if (tracks_map_ptr != NULL) return tracks_map_ptr;
   tracks_map_ptr=new TRACKS_MAP;

   string Luke_tracks_filename="features_panel"+
      stringfunc::number_to_string(p)+"_lukeblobs.txt";
   filefunc::ReadInfile(Luke_tracks_filename);

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double curr_t=column_values[0];
      int curr_track_ID=column_values[1];
      double curr_u=column_values[3];
      double curr_v=column_values[4];

      track* curr_track_ptr=NULL;
      TRACKS_MAP::iterator iter=tracks_map_ptr->find(curr_track_ID);
      if (iter==tracks_map_ptr->end()) 
      {
         curr_track_ptr=new track(curr_track_ID);
         (*tracks_map_ptr)[curr_track_ID]=curr_track_ptr;
      }
      else
      {
         curr_track_ptr=iter->second;
      }
      curr_track_ptr->set_XYZ_coords(curr_t,threevector(curr_u,curr_v));      

   } // loop over index i labeling text lines corresponding to tracks
//   cout << "n_tracks = " << tracks_map_ptr->size() << endl;

   return tracks_map_ptr;
}

// ========================================================================
// Video annotation member functions
// ========================================================================

// Member function annotate_curr_frame() queries the user to enter a
// text annotation for the current movie frame.  It then adds the
// entered annotation to *annotations_map_ptr.  A list of all
// annotations within the STL map is printed at the end of this
// method.

void Movie::annotate_curr_frame()
{
//   cout << "inside Movie::annotate_curr_frame()" << endl;

   texture_rectangle* texture_rectangle_ptr = 
      get_texture_rectangle_ptr(0);

   AnimationController* AnimationController_ptr = 
      texture_rectangle_ptr->get_AnimationController_ptr();
   int curr_imagenumber = AnimationController_ptr->get_true_framenumber();
//   int curr_imagenumber = texture_rectangle_ptr->get_imagenumber();
   string image_filename = AnimationController_ptr->
      get_curr_image_filename();

//   Clock* clock_ptr = AnimationController_ptr->get_clock_ptr();

   string curr_annotation;
   cout << "Enter annotation for image " 
        << filefunc::getbasename(image_filename) << " :" << endl;
   getline(cin,curr_annotation);
//   cout << "annotation = " << curr_annotation << endl;

   if (annotations_map_ptr==NULL)
   {
      annotations_map_ptr = new ANNOTATIONS_MAP;
   }
   ANNOTATIONS_MAP::iterator iter = annotations_map_ptr->find(
      curr_imagenumber);
   if (iter==annotations_map_ptr->end())
   {
      double curr_epoch=
         AnimationController_ptr->get_time_corresponding_to_curr_frame();

      pair<double,string> P(curr_epoch,curr_annotation);
      (*annotations_map_ptr)[curr_imagenumber]=P;
   }
   else
   {
      iter->second.second = curr_annotation;
   }

   cout << "========================================================" << endl;
   cout << "List of image annotations:" << endl << endl;

   for (iter=annotations_map_ptr->begin(); iter != annotations_map_ptr->end(); 
        iter++)
   {
      int imagenumber = iter->first;
      AnimationController_ptr->set_curr_framenumber(imagenumber);
      string image_basename=filefunc::getbasename(
         AnimationController_ptr->get_curr_image_filename());
      
//      double epoch_time = iter->second.first;
//      clock_ptr->convert_elapsed_secs_to_date(epoch_time);
//      string epoch_str = clock_ptr->YYYY_MM_DD_H_M_S();
      string annotation = iter->second.second;
      cout << "Image number: " << imagenumber 
           << "  Filename: " << image_basename
           << "  Annotation: " << annotation << endl;
   }
}

// ----------------------------------------------------------------
// Member function export_video_annotations() queries user to enter
// name of an output CSV file.  It then exports all entries within STL
// map *annotations_map_ptr to the CSV file.

void Movie::export_video_annotations()
{
//   cout << "inside Movie::export_video_annotations()" << endl; 

   if (annotations_map_ptr==NULL)
   {
      cout << "No annotations have been entered so far!" << endl;
      return;
   }

   string annotations_export_filename;
   cout << "Enter name of CSV file for video annotations to be exported:"
        << endl;
   cin >> annotations_export_filename;

   ofstream outstream;
   outstream.precision(12);
   filefunc::openfile(annotations_export_filename,outstream);
   outstream << "# Image frame number, Image filename, Elapsed secs, Annotation"
             << endl << endl;

   texture_rectangle* texture_rectangle_ptr = 
      get_texture_rectangle_ptr(0);
   AnimationController* AnimationController_ptr = 
      texture_rectangle_ptr->get_AnimationController_ptr();
   Clock* clock_ptr = AnimationController_ptr->get_clock_ptr();

   double starting_epoch_time=
      AnimationController_ptr->get_time_corresponding_to_frame(
         AnimationController_ptr->get_first_framenumber());
   for (ANNOTATIONS_MAP::iterator iter=annotations_map_ptr->begin(); 
        iter != annotations_map_ptr->end(); iter++)
   {
      int imagenumber = iter->first;
      cout << "imagenumber = " << imagenumber << endl;
      AnimationController_ptr->set_curr_framenumber(imagenumber);
      string image_basename=filefunc::getbasename(
         AnimationController_ptr->get_curr_image_filename());

      double epoch_time = iter->second.first;
      double elapsed_secs=epoch_time - starting_epoch_time;
      clock_ptr->convert_elapsed_secs_to_date(epoch_time);
      string epoch_str = clock_ptr->YYYY_MM_DD_H_M_S();

      string annotation = iter->second.second;
      outstream << stringfunc::integer_to_string(imagenumber,6) << " , "
                << image_basename << " , "
                << stringfunc::number_to_string(elapsed_secs,2) << " , "
//                << stringfunc::number_to_string(epoch_time,2) << ","
//                << epoch_str << ","
                << annotation << endl;
   }

   filefunc::closefile(annotations_export_filename,outstream);

   string banner="Exported video frame numbers, epoch times, dates and annotations to "+annotations_export_filename;
   outputfunc::write_big_banner(banner);
}
