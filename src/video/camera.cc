// ==========================================================================
// CAMERA class member function definitions
// ==========================================================================
// Last modified on 2/13/13; 4/24/13; 8/12/13; 4/6/14
// ==========================================================================

#include <iostream>
#include "video/camera.h"
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "math/fourvector.h"
#include "video/G99VideoDisplay.h"
#include "geometry/linesegment.h"
#include "geometry/plane.h"
#include "geometry/polyhedron.h"

#include "general/outputfuncs.h"
#include "osg/osgfuncs.h"

using std::cout;
using std::cin;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void camera::allocate_member_objects()
{
   K_ptr=new genmatrix(3,3);
   Kinv_ptr=new genmatrix(3,3);
   M_ptr=new genmatrix(3,3);
   Minv_ptr=new genmatrix(3,3);
   P_ptr=new genmatrix(3,4);
   P_prev_ptr=new genmatrix(3,4);
   Pdot_prev_ptr=new genmatrix(3,4);
   Pdotdot_prev_ptr=new genmatrix(3,4);
   R_camera_ptr=new rotation;
   R0_ptr=new rotation;
   H_ptr=new homography;
}		       

void camera::initialize_member_objects(G99VideoDisplay* VD_ptr)
{
//   cout << "inside camera::init_member_objects(), VD_ptr = " << VD_ptr 
//        << endl;

   georegistered_flag=calibration_flag=false;
   rho=1;
   theta=PI/2;	// rads
   FOV_u=FOV_v=-1*PI/180;	// sentinel dummy value

   imageplane_ptr=NULL;
   imageplane_thru_center_ptr=NULL;
   video_ptr=VD_ptr;
   camera_frustum_ptr=NULL;
   geolocation_ptr=NULL;

   if (video_ptr==NULL)
   {

// Default values for min/max U & V:

      double min_U=0;
      double max_U=1.3385;
      double min_V=0;
      double max_V=1;
   
      set_UV_corners(min_U,max_U,min_V,max_V);
   }
   else
   {
      set_UV_corners(
         video_ptr->get_minU(),
         video_ptr->get_maxU(),
         video_ptr->get_minV(),
         video_ptr->get_maxV());
   }

   fu=fv=1;
   kappa2=kappa4=0;
   sgn_detM=1;
   rel_az=rel_el=rel_roll=0;

   clear_previous_projection_matrices();

// Matrix *R0_ptr maps a vertically downward oriented OBSFRUSTUM onto
// a horizontal one with Uhat = -y_hat, Vhat = +z_hat and pointing
// direction -What = +x_hat:

   R0_ptr->clear_values();
   R0_ptr->put(0,2,-1);
   R0_ptr->put(1,0,-1);
   R0_ptr->put(2,1,1);

   R_noah_to_peter_ptr=NULL;
}

void camera::set_UV_corners(
   double min_U,double max_U,double min_V,double max_V)
{
//   cout << "inside camera::set_UV_corners()" << endl;
//   cout << "min_U = " << min_U << " max_U = " << max_U
//        << " min_V = " << min_V << " max_V = " << max_V << endl;
   
   if (nearly_equal(min_U,max_U))
   {
      cout << "inside camera::set_UV_corners()" << endl;
      cout << "min_U = " << min_U << " max_U = " << max_U << endl;
      cout << "min_V = " << min_V << " max_V = " << max_V << endl;
//      outputfunc::enter_continue_char();
   }

   UV_corner.clear();
   UV_corner.push_back(twovector(min_U,min_V));
   UV_corner.push_back(twovector(max_U,min_V));
   UV_corner.push_back(twovector(max_U,max_V));
   UV_corner.push_back(twovector(min_U,max_V));

   u0=0.5*(min_U+max_U);
   v0=0.5*(min_V+max_V);

   UV_bbox_ptr=new bounding_box(min_U,max_U,min_V,max_V);
}

camera::camera()
{	
   allocate_member_objects();
   initialize_member_objects();
}		       

camera::camera(G99VideoDisplay* VD_ptr)
{	
   allocate_member_objects();
   initialize_member_objects(VD_ptr);
}		       

camera::camera(double fu,double fv,double u0,double v0,double kappa2,
               double theta)
{	
   allocate_member_objects();
   initialize_member_objects();
   set_internal_params(fu,fv,u0,v0,theta,kappa2);
}		       

// Copy constructor:

camera::camera(const camera& c)
{
//   cout << "inside camera copy constructor, this(camera) = " << this << endl;
   allocate_member_objects();
   initialize_member_objects();
   docopy(c);
}

camera::~camera()
{
//   cout << "inside camera destructor" << endl;
//   cout << "this = " << this << endl;

   delete K_ptr;
   delete Kinv_ptr;
   delete M_ptr;
   delete Minv_ptr;
   delete P_ptr;
   delete P_prev_ptr;
   delete Pdot_prev_ptr;
   delete Pdotdot_prev_ptr;
   delete R_camera_ptr;
   delete R0_ptr;
   delete H_ptr;

   delete R_noah_to_peter_ptr;
   delete UV_bbox_ptr;
}

// Overload = operator:

camera& camera::operator= (const camera& c)
{
//   cout << "inside camera::operator=" << endl;
   if (this==&c) return *this;
   docopy(c);
//   cout << "*this = " << *this << endl;
   return *this;
}

void camera::docopy(const camera& c)
{
//   cout << "inside camera::docopy()" << endl;
   calibration_flag=c.calibration_flag;
   rho=c.rho;
   m_time=c.m_time;
   fu=c.fu;
   fv=c.fv;
   theta=c.theta;
   u0=c.u0;
   v0=c.v0;
   kappa2=c.kappa2;
   kappa4=c.kappa4;

   rel_az=c.rel_az;
   rel_el=c.rel_el;
   rel_roll=c.rel_roll;
   
   m_alpha=c.m_alpha;
   m_beta=c.m_beta;
   m_phi=c.m_phi;
   m_roll=c.m_roll;
   m_pitch=c.m_pitch;
   m_yaw=c.m_yaw;
   FOV_u=c.FOV_u;
   FOV_v=c.FOV_v;
   sgn_detM=c.sgn_detM;

   camera_world_posn=c.camera_world_posn;
   Rcamera_times_camera_world_posn=c.Rcamera_times_camera_world_posn;
   p4=c.p4;
   Uhat=c.Uhat;
   Vhat=c.Vhat;
   What=c.What;
   
   *K_ptr=*(c.K_ptr);
   *Kinv_ptr=*(c.Kinv_ptr);
   *P_ptr=*(c.P_ptr);
   *P_prev_ptr=*(c.P_prev_ptr);
   *Pdot_prev_ptr=*(c.Pdot_prev_ptr);
   *Pdotdot_prev_ptr=*(c.Pdotdot_prev_ptr);

   *R_camera_ptr=*(c.R_camera_ptr);
   *R0_ptr=*(c.R0_ptr);

   if (c.R_noah_to_peter_ptr != NULL)
   {
      *R_noah_to_peter_ptr=*(c.R_noah_to_peter_ptr);
   }
   
   *M_ptr=*(c.M_ptr);
   *Minv_ptr=*(c.Minv_ptr);

   if (c.video_ptr != NULL)
   {
//      *video_ptr=*(c.video_ptr);
   }

   UV_corner.clear();
   for (unsigned int i=0; i<c.UV_corner.size(); i++)
   {
      UV_corner.push_back(c.UV_corner[i]);
   }
   UV_corner_world_ray.clear();
   for (unsigned int i=0; i<c.UV_corner_world_ray.size(); i++)
   {
      UV_corner_world_ray.push_back(c.UV_corner_world_ray[i]);
   }
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,camera& c)
{
   outstream << "inside camera::operator<<" << endl;
//    outstream << "&c = " << &c << endl;
   
   outstream.precision(10);
   outstream << "fu = " << c.get_fu() 
             << " fv = " << c.get_fv() << endl;
   outstream << "u0 = " << c.get_u0() 
             << " v0 = " << c.get_v0() << endl;
   outstream << "u0/v0 = " << c.get_u0()/c.get_v0() << endl;
   outstream << "*K_ptr = " << *(c.K_ptr) << endl;
   outstream << "kappa2 = " << c.get_kappa2() 
             << " kappa4 = " << c.get_kappa4() << endl;

//    outstream << "c.R_camera_ptr = " << c.get_Rcamera_ptr() << endl;
   outstream << "Rcamera = " << *(c.get_Rcamera_ptr()) << endl;
   double az,el,roll;
   c.get_az_el_roll_from_Rcamera(az,el,roll);
   outstream << "az = " << az*180/PI << " el = " << el*180/PI
             << " roll = " << roll*180/PI << endl;

   outstream << "Uhat = " << c.get_Uhat() << endl;
   outstream << "Vhat = " << c.get_Vhat() << endl;
   outstream << "What = " << c.get_What() << endl;
   outstream << "camera_world_posn = " << c.camera_world_posn << endl;
   outstream << "FOV_u = " << c.FOV_u*180/PI << " degs"
             << " FOV_v = " << c.FOV_v*180/PI << " degs" << endl;

   if (c.get_P_ptr() != NULL)
   {
      outstream << "Projection matrix = " << endl;
      outstream << *(c.get_P_ptr()) << endl;

      genmatrix M(3,3);
      M=*(c.get_K_ptr()) * *(c.get_Rcamera_ptr());
//      outstream << "M = K R = " << endl;
//      outstream << M << endl;
//      cout << "-M*C = " << -M*c.camera_world_posn << endl;
   }

/*
   for (unsigned int i=0; i<c.UV_corner_world_ray.size(); i++)
   {
      cout << "i = " << i
           << " UV_corner = " << c.UV_corner[i]
           << " UV_corner_world_ray = " << c.UV_corner_world_ray[i]
           << endl;
   }
*/

   return outstream;
}

// ---------------------------------------------------------------------
// Member function print_params() exports intrinsic and extrinsic camera
// parameters in a reasonably user-friendly format.

void camera::print_params(int ID,ofstream& outstream)
{
//   cout << "inside camera::print_params()" << endl;

   outstream << "====================================================" << endl;
   outstream << "Camera " << ID << endl;
   outstream << "====================================================" << endl;
   outstream << endl;
   
   outstream.precision(10);
   outstream << "Dimensionless focal params: fu = " << get_fu() 
             << " fv = " << get_fv() << endl << endl;

   outstream << "FOV_u = " << FOV_u*180/PI << " degs" << endl;
   outstream << "FOV_v = " << FOV_v*180/PI << " degs" << endl << endl;

   outstream << "Camera center: u0 = " << get_u0() 
             << " v0 = " << get_v0() << endl << endl;
   outstream << "3x3 intrinsic parameters K matrix = " << *(K_ptr) << endl;

   double az,el,roll;
   get_az_el_roll_from_Rcamera(az,el,roll);
   outstream << "Camera azimuth = " << az*180/PI << " degs" << endl;
   outstream << "Camera elevation = " << el*180/PI << " degs" << endl;
   outstream << "Camera roll = " << roll*180/PI << " degs" << endl << endl;
   outstream << "3x3 camera rotation matrix = Rz(az) * Ry(-el) * Rx(roll) = " 
             << *(get_Rcamera_ptr()) << endl << endl;

   outstream << "Camera X position = " << camera_world_posn.get(0) << endl;
   outstream << "Camera Y position = " << camera_world_posn.get(1) << endl;
   outstream << "Camera Z position = " << camera_world_posn.get(2) << endl
             << endl;

   if (get_P_ptr() != NULL)
   {
      outstream << "3x4 projection matrix = " << endl;
      outstream << *(get_P_ptr()) << endl;
   }

   outstream << endl;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// Member function set_internal_params can be used to force the
// pinhole camera model's intrinsic parameters to assume specified
// values.  Input pixel skew angle theta is measured in degrees.

void camera::set_internal_params(double fu,double fv,double u0,double v0,
                                 double theta,double kappa2)
{
   this->fu=fu;
   this->fv=fv;
   this->u0=u0;
   this->v0=v0;
   this->theta=theta*PI/180.0;
   this->kappa2=kappa2;

   construct_internal_parameter_K_matrix();
}

// ---------------------------------------------------------------------
// Member function set_Rcamera takes in azimuth, elevation and roll
// angles specified in radians.  It initializes Rcamera so that Uhat =
// -yhat, Vhat = zhat and -What = xhat.  It then sets *R_camera_ptr
// equal to the matrix corresponding to a right handed rotation about
// +z_hat by az followed by a left-handed rotation about y_hat' by el
// followed by a right-handed rotation about x_hat'' by roll.

void camera::set_Rcamera(double az,double el,double roll)
{
//    cout << "inside camera::set_Rcamera()" << endl;

   rotation R;
   R=R.rotation_from_az_el_roll(az,el,roll);
//   cout << "R = " << R << endl;

//   threevector new_xhat,new_yhat,new_zhat;
//   R.get_column(0,new_xhat);
//   R.get_column(1,new_yhat);
//   R.get_column(2,new_zhat);
//   cout << "new_xhat = " << new_xhat << endl;
//   cout << "new_yhat = " << new_yhat << endl;
//   cout << "new_zhat = " << new_zhat << endl;

// The components of new_xhat, new_yhat and new_zhat need to be placed
// into the rows rather than the columns of *R_camera_ptr:

   *R_camera_ptr=R0_ptr->transpose() * R.transpose();

//   cout << "*R0_ptr = " << *R0_ptr << endl;
//   cout << "Rcamera = (R * R0)^transpose = " << *R_camera_ptr << endl;

//   double det;
//   if (R_camera_ptr->determinant(det))
//   {
//      cout << "Rcamera determinant = " << det << endl;
//   }
//   else
//   {
//      cout << "Cannot compute determinant of Rcamera !!!" << endl;
//   }

   R_camera_ptr->get_row(0,Uhat);
   R_camera_ptr->get_row(1,Vhat);
   R_camera_ptr->get_row(2,What);

//   cout << "Uhat = " << Uhat << endl;
//   cout << "Vhat = " << Vhat << endl;
//   cout << "What = " << What << endl;
}

// ---------------------------------------------------------------------
// Member function get_az_el_roll_from_Rcamera returns the "Euler"
// angle decomposition of Rcamera into an azimuthal rotation, followed
// by an elevation, followed by a roll.  The returned angles are
// measured in radians.  This method performs the inverse operation to
// set_Rcamera(az,el,roll).

void camera::get_az_el_roll_from_Rcamera(double& az,double& el,double& roll)
{
//   cout << "inside camera::get_az_el_roll_from_Rcamera()" << endl;
  
   compute_az_el_roll_from_Rcamera();
   az=rel_az;
   el=rel_el;
   roll=rel_roll;
}

void camera::compute_az_el_roll_from_Rcamera()
{
//   cout << "inside camera::compute_az_el_roll_from_Rcamera()" << endl;
//   cout << "Rcamera = " << *R_camera_ptr << endl;

   rotation R=R_camera_ptr->transpose() * (R0_ptr->transpose());
//   cout << "*R_camera_ptr = " << *R_camera_ptr << endl;
//   cout << "*R0_ptr = " << *R0_ptr << endl;
//   cout << "R = " << R << endl;

   R.az_el_roll_from_rotation(rel_az,rel_el,rel_roll);
   cout.precision(12);
//   cout << "rel_az = " << rel_az*180/PI << endl;
//   cout << "rel_el = " << rel_el*180/PI << endl;
//   cout << "rel_roll = " << rel_roll*180/PI << endl;

//   rotation Rcheck;
//   Rcheck=Rcheck.rotation_from_az_el_roll(rel_az,rel_el,rel_roll);

//   rotation Rcamera_check=R0_ptr->transpose() * Rcheck.transpose();
//   cout << "Rcamera_check = " << Rcamera_check << endl;
//   cout << "Rcamera - Rcamera_check = "
//        << *R_camera_ptr - Rcamera_check << endl;
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function set_Rcamera takes in direction vectors Uhat and
// Vhat and places them into the ROWS (and not columns!) of
// *R_camera_ptr.

void camera::set_Rcamera(const threevector& Uhat,const threevector& Vhat)
{
//   cout << "inside camera::set_Rcamera()" << endl;
   this->Uhat=Uhat;
   this->Vhat=Vhat;
   this->What=Uhat.cross(Vhat);
   
   rotation R;
   R.put_column(0,Uhat);
   R.put_column(1,Vhat);
   R.put_column(2,Uhat.cross(Vhat));
   set_Rcamera(R);
//   cout << "R = " << R << endl;
}

void camera::set_Rcamera(const rotation& R)
{
//   cout << "inside camera::set_Rcamera()" << endl;

// The components of new_xhat, new_yhat and new_zhat need to be placed
// into the rows rather than the columns of *R_camera_ptr:

   *R_camera_ptr=R.transpose();

   R_camera_ptr->get_row(0,Uhat);
   R_camera_ptr->get_row(1,Vhat);
   R_camera_ptr->get_row(2,What);

//   cout << "*R_camera_ptr = " << *R_camera_ptr << endl;
//   cout << "Uhat = " << Uhat << endl;
//   cout << "Vhat = " << Vhat << endl;
}

void camera::set_imageplane(const fourvector& pi)
{
//   cout << "inside camera::set_imageplane()" << endl;

   delete imageplane_ptr;
   imageplane_ptr=new plane(pi);
}

plane* camera::get_imageplane_ptr() const
{
   return imageplane_ptr;
}

plane* camera::get_imageplane_thru_center_ptr() const
{
   return imageplane_thru_center_ptr;
}

// ==========================================================================
// 3x4 projection matrix construction based on tiepoint input
// ==========================================================================

// Member function compute_tiepoint_projection_matrix() takes in
// genmatrices *XYZUV_ptr and *XYZABC_ptr holding tiepoint and tieline
// coordinates.  

void camera::compute_tiepoint_projection_matrix(
   const genmatrix* XYZUV_ptr,int n_rows_to_skip,int row_to_skip)
{
   genmatrix* XYZABC_ptr=NULL;
   compute_tiepoint_projection_matrix(XYZUV_ptr,XYZABC_ptr,n_rows_to_skip,
	   row_to_skip);
}

void camera::compute_tiepoint_projection_matrix(
   const genmatrix* XYZUV_ptr,const genmatrix* XYZABC_ptr,
   int n_rows_to_skip,int row_to_skip)
{
   cout << "inside camera::compute_tiepoint_projection_matrix()" << endl;
//   cout << "n_rows_to_skip = " << n_rows_to_skip
//        << " current row to skip = " << row_to_skip << endl;
//   int n_rows_to_skip=1;
//   int row_to_skip=-1;

//   cout << "Enter row to skip:" << endl;
//   cin >> row_to_skip;

   int m_features=0;
   int m_lines=0;
   if (XYZUV_ptr != NULL) m_features=XYZUV_ptr->get_mdim();
   if (XYZABC_ptr != NULL) m_lines=XYZABC_ptr->get_mdim()/2;

   cout << "m_features = " << m_features << " m_lines = " << m_lines << endl;

   int nrows=2*(m_features-n_rows_to_skip+m_lines);
   int ncolumns=12;
   cout << "nrows = " << nrows << endl;
//   outputfunc::enter_continue_char();

   genmatrix* A_ptr=new genmatrix(nrows,ncolumns);
   A_ptr->clear_values();
   
   if (m_features > 0) parse_xyzuv_data(XYZUV_ptr,A_ptr,row_to_skip);
   if (m_lines > 0) 
   {
      int starting_row=nrows-2*m_lines;
      parse_xyzabc_data(XYZABC_ptr,A_ptr,starting_row);
   }
   cout << "m_lines = " << m_lines << endl;

   fill_projection_matrix_entries(A_ptr);
   delete A_ptr;

   bool nonsingular_submatrix_flag=decompose_projection_matrix();
   cout << endl;

   if (nonsingular_submatrix_flag)
   {
      compute_corner_ray_and_UV_axes_dirs();
      cout << "Camera world posn = " << camera_world_posn << endl;
      cout << "Camera pointing direction = " << get_pointing_dir() << endl;

//      for (unsigned int i=0; i<UV_corner_world_ray.size(); i++)
//      {
//         cout << "UV corner world ray #" << i   << " : " 
//              << UV_corner_world_ray[i] << endl;
//      }
//      cout << "Uhat = " << Uhat << " Vhat = " << Vhat << endl;

   }
}

// ---------------------------------------------------------------------
// Member function compute_orthographic_tiepoint_projection_matrix

void camera::compute_orthographic_tiepoint_projection_matrix(
   const genmatrix* XYZUV_ptr)
{
//   cout << "inside camera::compute_orthographic_tiepoint_projection_matrix()" << endl;
   
   int nrows=2*(XYZUV_ptr->get_mdim());
   int ncolumns=4;
   genmatrix* B_ptr=new genmatrix(nrows,ncolumns);
   parse_orthographic_xyzuv_data(XYZUV_ptr,B_ptr);
   fill_orthographic_projection_matrix_entries(B_ptr);
   delete B_ptr;
   
   cout << endl;
   cout << "3x4 projection matrix P = " << *P_ptr << endl;
   cout << "Camera world posn = " << camera_world_posn << endl;
   cout << "Camera pointing direction = " << get_pointing_dir() << endl;
}

// ---------------------------------------------------------------------
// Member function parse_xyzuv_data reads the contents of an input
// genmatrix containing X, Y, Z, U and V columns.  It rearranges this
// information into the input 2r x 12 matrix A defined in our
// "Geometric Camera Calibration" notes dated 1/12/05.  Here r denotes
// the number of lines within the input file.

void camera::parse_xyzuv_data(
   const genmatrix* XYZUV_ptr,genmatrix* A_ptr,int row_to_skip)
{
//   cout << "inside camera::parse_xyzuv_data()" << endl;
//   cout << "XYZUV_ptr->get_mdim() = " << XYZUV_ptr->get_mdim() << endl;
   unsigned int row=0;
   for (unsigned int m=0; m<XYZUV_ptr->get_mdim(); m++)
   {
      if (int(m) != row_to_skip)
      {
//         int ID=basic_math::round(XYZUV_ptr->get(m,0));
         double x=XYZUV_ptr->get(m,1);
         double y=XYZUV_ptr->get(m,2);
         double z=XYZUV_ptr->get(m,3);
         double u=XYZUV_ptr->get(m,4);
         double v=XYZUV_ptr->get(m,5);
//         cout << "ID = " << ID
//              << " X = " << x << " Y = " << y << " Z = " << z
//              << " U = " << u << " V = " << v << endl;
      
//         int row=m*2+0;
         A_ptr->put(row,0,x);
         A_ptr->put(row,1,y);
         A_ptr->put(row,2,z);
         A_ptr->put(row,3,1);

         A_ptr->put(row,4,0);
         A_ptr->put(row,5,0);
         A_ptr->put(row,6,0);
         A_ptr->put(row,7,0);

         A_ptr->put(row,8,-u*x);
         A_ptr->put(row,9,-u*y);
         A_ptr->put(row,10,-u*z);
         A_ptr->put(row,11,-u);

         row++;

         A_ptr->put(row,0,0);
         A_ptr->put(row,1,0);
         A_ptr->put(row,2,0);
         A_ptr->put(row,3,0);

         A_ptr->put(row,4,x);
         A_ptr->put(row,5,y);
         A_ptr->put(row,6,z);
         A_ptr->put(row,7,1);

         A_ptr->put(row,8,-v*x);
         A_ptr->put(row,9,-v*y);
         A_ptr->put(row,10,-v*z);
         A_ptr->put(row,11,-v);

         row++;
         
      } // m != row_to_skip conditional
   } // loop over index m labeling rows
   
//   cout << "A = " << *A_ptr << endl;
//   cout << "A.mdim = " << A_ptr->get_mdim() << " A.ndim = " 
//        << A_ptr->get_ndim() << endl;
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function parse_orthographic_xyzuv_data forms 2r x 4 matrix
// *B_ptr which is a reduced version of the more general matrix
// *A_ptr.  We assume that the camera's horizontal and vertical
// direction vectors Uhat and Vhat have already been preloaded into
// *R_camera_ptr prior to this method being called.  The homogenoues
// solution to *B_ptr X = 0 yields the scaling and translations needed
// to align telescope imagery to 3D satellite models for AZ/EL ground
// telescopes. 

void camera::parse_orthographic_xyzuv_data(
   const genmatrix* XYZUV_ptr,genmatrix* B_ptr)
{

// Threevectors u_hat and v_hat correspond to the optical image
// plane's horizontal and vertical axes in satellite model
// coordinates:

   threevector u_hat,v_hat;
   R_camera_ptr->get_row(0,u_hat);
   R_camera_ptr->get_row(1,v_hat);
   
   for (unsigned int m=0; m<XYZUV_ptr->get_mdim(); m++)
   {
      int ID=basic_math::round(XYZUV_ptr->get(m,0));
      double x=XYZUV_ptr->get(m,1);
      double y=XYZUV_ptr->get(m,2);
      double z=XYZUV_ptr->get(m,3);
      double u=XYZUV_ptr->get(m,4);
      double v=XYZUV_ptr->get(m,5);
      cout << "ID = " << ID
           << " X = " << x << " Y = " << y << " Z = " << z
           << " U = " << u << " V = " << v << endl;

      threevector xyz(x,y,z);

      int row=m*2+0;
      B_ptr->put(row,0,u_hat.dot(xyz));
      B_ptr->put(row,1,1);
      B_ptr->put(row,2,0);
      B_ptr->put(row,3,-u);
      
      row++;

      B_ptr->put(row,0,v_hat.dot(xyz));
      B_ptr->put(row,1,0);
      B_ptr->put(row,2,1);
      B_ptr->put(row,3,-v);
      
      row++;

   } // loop over index m labeling rows
   
//   cout << "B = " << *B_ptr << endl;
//   cout << "B.mdim = " << B_ptr->get_mdim() << " B.ndim = "
//        << B_ptr->get_ndim() << endl;

}

// ---------------------------------------------------------------------
// Member function parse_xyzabc_data() reads the contents of an input
// genmatrix containing X, Y, Z, A, B and C columns.  It rearranges this
// information into the bottom 2s x 12 submatrix of matrix A defined
// in our "Geometric Camera Calibration via Tie-Line Matching" notes
// dated 12/29/10.  Here s denotes the number of lines within the
// input file.

void camera::parse_xyzabc_data(
   const genmatrix* XYZABC_ptr,genmatrix* A_ptr,int starting_row)
{
   cout << "inside camera::parse_xyzabc_data()" << endl;
   cout << "XYZABC_ptr->get_mdim() = " << XYZABC_ptr->get_mdim() << endl;
   
   int row=starting_row;
   for (unsigned int m=0; m<XYZABC_ptr->get_mdim(); m++)
   {
      int ID=basic_math::round(XYZABC_ptr->get(m,0));
      double x=XYZABC_ptr->get(m,1);
      double y=XYZABC_ptr->get(m,2);
      double z=XYZABC_ptr->get(m,3);
      double a=XYZABC_ptr->get(m,4);
      double b=XYZABC_ptr->get(m,5);
      double c=XYZABC_ptr->get(m,6);
      cout << "ID = " << ID
           << " X = " << x << " Y = " << y << " Z = " << z
           << " A = " << a << " B = " << b << " C = " << c << endl;
      
      A_ptr->put(row,0,a*x);
      A_ptr->put(row,1,a*y);
      A_ptr->put(row,2,a*z);
      A_ptr->put(row,3,a);

      A_ptr->put(row,4,b*x);
      A_ptr->put(row,5,b*y);
      A_ptr->put(row,6,b*z);
      A_ptr->put(row,7,b);

      A_ptr->put(row,8,c*x);
      A_ptr->put(row,9,c*y);
      A_ptr->put(row,10,c*z);
      A_ptr->put(row,11,c);

      row++;

   } // loop over index m labeling rows
   
   cout << "A = " << *A_ptr << endl;
   cout << "A.mdim = " << A_ptr->get_mdim() << " A.ndim = " 
        << A_ptr->get_ndim() << endl;
   outputfunc::enter_continue_char();
}
      
// ---------------------------------------------------------------------
// Method fill_projection_matrix_entries works with the 2r x 12
// genmatrix *A_ptr generated by method parse_xyzuv_data_file.  It
// first performs a singular value decomposition of *A_ptr = U diag(w)
// V^T.  It then identifies the smallest singular value (which should
// be close to zero) as well as the corresponding column eigenvector v
// within matrix V.  This column vector should approximately satisfy
// the homogenous system of equantions *A_ptr v = 0.  Finally, this
// method rearranges the 12 entries within v into the 3x4 projection
// genmatrix *P_ptr which maps homogeneous (x,y,z,1) coordinates from
// world space onto their corresponding homogenous (u,v,1) coordinates
// within image space.  The dynamically generated P_ptr is returned by
// this method.  See our "Geometric camera calibration" notes dated
// 1/12/05.
   
void camera::fill_projection_matrix_entries(genmatrix const * const A_ptr)
{
   cout << "inside camera::fill_projection_matrix_entries()" << endl;
   
   genvector X(12);
   A_ptr->homogeneous_soln(X);

// Rearrange X vector's contents within 3x4 projection matrix *P_ptr:

   P_ptr->put(0,0,X.get(0));
   P_ptr->put(0,1,X.get(1));
   P_ptr->put(0,2,X.get(2));
   P_ptr->put(0,3,X.get(3));

   P_ptr->put(1,0,X.get(4));
   P_ptr->put(1,1,X.get(5));
   P_ptr->put(1,2,X.get(6));
   P_ptr->put(1,3,X.get(7));

   P_ptr->put(2,0,X.get(8));
   P_ptr->put(2,1,X.get(9));
   P_ptr->put(2,2,X.get(10));
   P_ptr->put(2,3,X.get(11));

   cout << "3x4 projection matrix P = " << *P_ptr << endl;
}
    
// ---------------------------------------------------------------------
// Method fill_orthographic_projection_matrix_entries
   
void camera::fill_orthographic_projection_matrix_entries(
   genmatrix const * const B_ptr)
{
//    cout << "inside camera::fill_orthographic_projection_matrix_entries()" << endl;
   
   genvector X(4);
   B_ptr->homogeneous_soln(X);

   double k=X.get(0)/X.get(3);
   double u0=X.get(1)/X.get(3);
   double v0=X.get(2)/X.get(3);
      
   cout << "Scaling factor k = " << k << endl;
   cout << "Horizontal translation u0 = " << u0 << endl;
   cout << "Horizontal translation v0 = " << v0 << endl;
      
   threevector k_uhat=k*get_Uhat();
   threevector k_vhat=k*get_Vhat();
      
// Rearrange X vector's contenst within 3x4 projection matrix *P_ptr:

   P_ptr->put(0,0,k_uhat.get(0));
   P_ptr->put(0,1,k_uhat.get(1));
   P_ptr->put(0,2,k_uhat.get(2));
   P_ptr->put(0,3,u0);

   P_ptr->put(1,0,k_vhat.get(0));
   P_ptr->put(1,1,k_vhat.get(1));
   P_ptr->put(1,2,k_vhat.get(2));
   P_ptr->put(1,3,v0);

   P_ptr->put(2,0,0);
   P_ptr->put(2,1,0);
   P_ptr->put(2,2,0);
   P_ptr->put(2,3,1);

   cout << "Projection matrix P = " << *P_ptr << endl;
}

// ---------------------------------------------------------------------
// Method check_projection_matrix reads in the original XYZ-UV
// information from the input file.  It projects the XYZ points down
// into the UV image plane using the 3x4 matrix *P_ptr.  This method's
// direct comparison of the projected UV values with the measured ones
// provides some indication of the projection fit's quality.  Results
// are sorted so that the worst agreement is displayed last.

void camera::check_projection_matrix(const genmatrix* XYZUV_ptr)
{
   double du_sqr=0;
   double dv_sqr=0;
   double chisq=0;

   int n_Upixels=video_ptr->getWidth();
   int n_Vpixels=video_ptr->getHeight();
   double delta_U=video_ptr->get_maxU()-video_ptr->get_minU();
   double delta_V=video_ptr->get_maxV()-video_ptr->get_minV();

   vector<double> ID,x,y,z,u,v;
   vector<double> u_proj,v_proj,d_chisq;
   for (unsigned int i=0; i<XYZUV_ptr->get_mdim(); i++)
   {
      ID.push_back(basic_math::round(XYZUV_ptr->get(i,0)));
      x.push_back(XYZUV_ptr->get(i,1));
      y.push_back(XYZUV_ptr->get(i,2));
      z.push_back(XYZUV_ptr->get(i,3));
      u.push_back(XYZUV_ptr->get(i,4));
      v.push_back(XYZUV_ptr->get(i,5));

      double curr_u_proj,curr_v_proj;
      project_XYZ_to_UV_coordinates(x.back(),y.back(),z.back(),
                                    curr_u_proj,curr_v_proj);
      u_proj.push_back(curr_u_proj);
      v_proj.push_back(curr_v_proj);
            d_chisq.push_back( sqr(u.back()-u_proj.back())+
                         sqr(v.back()-v_proj.back()) );
      chisq += d_chisq.back();

/*
      cout << "ID = " << int(ID.back()) 
           << " X = " << x.back() 
           << " Y = " << y.back() 
           << " Z = " << z.back() << endl;
      cout << "Input U = " << u.back() << " projected U = " << u_proj.back() 
           << " d_pu = " << d_pu << endl;
      cout << "Input V = " << v.back() << " projected V = " << v_proj.back() 
           << " d_pv = " << d_pv << endl;
      cout << " sqrt(d_pu**2+d_pv**2) = " << sqrt(sqr(d_pu)+sqr(d_pv))
           << endl;
      cout << endl;
*/
   } // loop over index i labeling entries in *XYZUV_ptr matrix

// Sort projected and measured XYZ-UV results by d_chisq vector values:

   templatefunc::Quicksort(d_chisq,ID,x,y,z,u,u_proj,v,v_proj);

   cout << "===============================================" << endl;
   cout << "Projected vs measured XYZ-UV results sorted by d_chisq:" << endl;
   cout << "===============================================" << endl << endl;

   for (unsigned int i=0; i<XYZUV_ptr->get_mdim(); i++)
   {
      double d_pu=(u[i]-u_proj[i])/delta_U*n_Upixels;
      double d_pv=(v[i]-v_proj[i])/delta_V*n_Vpixels;
      du_sqr += sqr(u[i]-u_proj[i]);
      dv_sqr += sqr(v[i]-v_proj[i]);

      cout << "ID = " << int(ID[i]) << " X = " << x[i] 
           << " Y = " << y[i]
           << " Z = " << z[i] << endl;
      cout << "Input U = " << u[i] << " projected U = " << u_proj[i] 
           << " d_pu = " << d_pu << endl;
      cout << "Input V = " << v[i] << " projected V = " << v_proj[i]
           << " d_pv = " << d_pv << endl;
      cout << "Sqrt(d_pu**2+d_pv**2) = " << sqrt(sqr(d_pu)+sqr(d_pv))
           << endl;
      cout << endl;
   } // loop over index i labeling lines within xyz_uv text file

// Compute pixel discrepancy in the U and V directions:

   double d_pu=n_Upixels/delta_U*sqrt(du_sqr/XYZUV_ptr->get_mdim());
   double d_pv=n_Vpixels/delta_V*sqrt(dv_sqr/XYZUV_ptr->get_mdim());
   
   cout << endl;
   cout << "===============================================" << endl;
   cout << "RMS residual between measured and calculated UV points = " 
        << sqrt(chisq/XYZUV_ptr->get_mdim()) << endl;
   cout << "RMS U pixel offset = " << d_pu << " out of " << n_Upixels 
        << " = " << d_pu/n_Upixels*100 << " %" << endl;
   cout << "RMS V pixel offset = " << d_pv << " out of " << n_Vpixels 
        << " = " << d_pv/n_Vpixels*100 << " %" << endl;
   cout << "===============================================" << endl;
   cout << endl;
}

// ---------------------------------------------------------------------
// Method fast_check_projection_matrix is a stripped down version of
// check_projection_matrix() which has been streamlined for iterative
// search purposes.

double camera::fast_check_projection_matrix(const genmatrix* XYZUV_ptr)
{
   double u_proj,v_proj;
   double chisq=0;
   for (unsigned int i=0; i<XYZUV_ptr->get_mdim(); i++)
   {
      project_XYZ_to_UV_coordinates(
         XYZUV_ptr->get(i,1),XYZUV_ptr->get(i,2),XYZUV_ptr->get(i,3),
         u_proj,v_proj);
      chisq += sqr(XYZUV_ptr->get(i,4)-u_proj)+
         sqr(XYZUV_ptr->get(i,5)-v_proj);
   } // loop over index i labeling entries in *XYZUV_ptr matrix
   return chisq;
}

// ---------------------------------------------------------------------
// Method check_projection_matrix_for_tielines() takes in genmatrix
// *XYZABC_ptr which is assumed to contain correlated XYZ points
// lying along 3D world lines and abc coefficients for their
// associated image plane line projections.  It first projects each
// XYZ point into the current camera's image plane defined by its
// extrinsic and intrisic parameters.  This method then computes and sums
// the squared 2D distances between the projected world points and the
// infinite 2D lines.  It sorts the world lines according to their
// individual d_chisq values and prints out an RMS residual between
// the 3D and 2D tielines.

void camera::check_projection_matrix_for_tielines(const genmatrix* XYZABC_ptr)
{
//   cout << "inside camera::check_projection_matrix_for_tielines()" << endl;
   
   if (XYZABC_ptr==NULL) return;
//   cout << "XYZABC_ptr->get_mdim() = " << XYZABC_ptr->get_mdim() << endl;
//   cout << "XYZABC_ptr->get_ndim() = " << XYZABC_ptr->get_ndim() << endl;

   double chisq=0;

   vector<int> threeDpt_ID;
   vector<double> x,y,z,a,b,c;
   vector<double> u_proj,v_proj,d_chisq;
   for (unsigned int m=0; m<XYZABC_ptr->get_mdim(); m++)
   {
      threeDpt_ID.push_back(basic_math::round(XYZABC_ptr->get(m,0)));
      x.push_back(XYZABC_ptr->get(m,1));
      y.push_back(XYZABC_ptr->get(m,2));
      z.push_back(XYZABC_ptr->get(m,3));
      a.push_back(XYZABC_ptr->get(m,4));
      b.push_back(XYZABC_ptr->get(m,5));
      c.push_back(XYZABC_ptr->get(m,6));

      double curr_u,curr_v;
      project_XYZ_to_UV_coordinates(x.back(),y.back(),z.back(),
                                    curr_u,curr_v);

// Recall squared 2D distance between homogenous point (u,v,1) and 2D
// line (a,b,c) is given by d*d=sqr(u*a+v*b+c)/(a*a+b*b)

      double numer=sqr(a.back()*curr_u+b.back()*curr_v+c.back());
      double denom=sqr(a.back())+sqr(b.back());
      d_chisq.push_back( numer/denom );
      chisq += d_chisq.back();

//      cout << "ID = " << threeDpt_ID.back() 
//           << " X = " << x.back() 
//           << " Y = " << y.back() 
//           << " Z = " << z.back() << endl;
//      cout << "a = " << a.back() << " b = " << b.back() << " c = " << c.back()
//           << endl;
//      cout << "Projected U = " << curr_u 
//           << " projected V = " << curr_v << endl;
//      cout << "d_chisq = " << d_chisq.back() << endl;
//      cout << endl;
   } // loop over index m labeling rows in *XYZABC_ptr matrix

// Sort projected and measured XYZ-ABC results by d_chisq vector values:

   templatefunc::Quicksort(d_chisq,threeDpt_ID,x,y,z,a,b,c);

   cout << "===============================================" << endl;
   cout << "Projected vs measured XYZ-ABC results sorted by d_chisq:" << endl;
   cout << "===============================================" << endl << endl;

   for (unsigned int i=0; i<XYZABC_ptr->get_mdim(); i++)
   {
      cout << "ID = " << threeDpt_ID[i] 
           << " X = " << x[i] 
           << " Y = " << y[i]
           << " Z = " << z[i] << endl;
      cout << "Projected point's 2D distance to line = " 
           << sqrt(d_chisq[i]) << endl;
      cout << endl;
   } // loop over index i labeling lines within xyz_uv text file

   cout << "===============================================" << endl;
   cout << "RMS residual between measured and calculated 2D lines = " 
        << sqrt(chisq/XYZABC_ptr->get_mdim()) << endl;
   cout << "===============================================" << endl;
   cout << endl;
}

// ---------------------------------------------------------------------
// Method fast_check_projection_matrix_for_tielines() is a stripped
// down version of check_projection_matrix_for_tielines() which has
// been streamlined for iterative search purposes.

double camera::fast_check_projection_matrix_for_tielines(
   const genmatrix* XYZABC_ptr)
{
//   cout << "inside camera::fast_check_projection_matrix_for_tielines()" << endl;
   
   if (XYZABC_ptr==NULL) return 0;
//   cout << "XYZABC_ptr->get_mdim() = " << XYZABC_ptr->get_mdim() << endl;
//   cout << "XYZABC_ptr->get_ndim() = " << XYZABC_ptr->get_ndim() << endl;

   double chisq=0;
   for (unsigned int m=0; m<XYZABC_ptr->get_mdim(); m++)
   {
      double x=XYZABC_ptr->get(m,1);
      double y=XYZABC_ptr->get(m,2);
      double z=XYZABC_ptr->get(m,3);
      double a=XYZABC_ptr->get(m,4);
      double b=XYZABC_ptr->get(m,5);
      double c=XYZABC_ptr->get(m,6);
      
      double curr_u,curr_v;
      project_XYZ_to_UV_coordinates(x,y,z,curr_u,curr_v);

// Recall squared 2D distance between homogenous point (u,v,1) and 2D
// line (a,b,c) is given by d*d=sqr(u*a+v*b+c)/(a*a+b*b)

      double numer=sqr(a*curr_u+b*curr_v+c);
      double denom=a*a+b*b;
      chisq += numer/denom;
   } // loop over index m labeling rows in *XYZABC_ptr matrix

   return chisq;
}

// ---------------------------------------------------------------------
// Member function extract_external_and_internal_params follows
// section 3.2.2 in "Computer Vision" by Forsythe and Ponce.  It
// computes the 5 continuous intrinsic camera parameters
// (fu,fv,theta,u0,v0) for the linear pinhole model as well as the 6
// extrinsic camera parameters (3 translations and 3 rotation angles)
// that are encoded within the 3x4 projection matrix *M_ptr.  It also
// determines the sign of the scale factor rho which accompanies the
// continuous intrinsic parameters.  See our notes entitled "Intrinsic
// and extrinsic camera parameter recovery" dated 8/17/05 for
// nomenclature conventions and derivation of the formulas used below.

void camera::extract_external_and_internal_params()
{
//   cout << "inside camera::extract_external_and_internal_params()" << endl;

   vector<int> sgn;
   sgn.push_back(1);
   sgn.push_back(-1);

   int sgn_rho=1;
   double min_abs_diff_sum=POSITIVEINFINITY;
   genmatrix P(3,4);
   for (unsigned int iter=0; iter<sgn.size(); iter++)
   {
      compute_pinhole_model_params(sgn[iter],P);
      double abs_diff_sum=0;
      for (unsigned int i=0; i<3; i++)
      {
         for (unsigned int j=0; j<4; j++)
         {
            abs_diff_sum += fabs(P_ptr->get(i,j)-P.get(i,j));
         }
      }
      if (abs_diff_sum < min_abs_diff_sum)
      {
         min_abs_diff_sum=abs_diff_sum;
         sgn_rho=sgn[iter];
      }
   } // loop over sgn values

   compute_pinhole_model_params(sgn_rho,P);

//   cout.precision(12);
//   cout << "sgn(rho) = " << sgn_rho << endl;
//   cout << "rho = " << rho << endl;
//   cout << "fu = " << fu << " fv = " << fv << endl;
//   cout << "u0 = " << u0 << " v0 = " << v0 << endl;
//   cout << "theta = " << theta*180/PI << " degs" << endl;
//   cout << "camera_world_posn = " << camera_world_posn << endl;

   double az,el,roll;
   get_az_el_roll_from_Rcamera(az,el,roll);
//   cout << "az = " << az*180/PI << endl;
//   cout << "el = " << el*180/PI << endl;
//   cout << "roll = " << roll*180/PI << endl;
   
//   cout << "*R_camera_ptr = " << *R_camera_ptr << endl;
//   cout << "R*Rtrans = " << *R_camera_ptr * R_camera_ptr->transpose() << endl;

//   cout << "Recovered projection matrix P = " << P << endl;
//   cout << "Input projection matrix *P_ptr = " << *P_ptr << endl;
//   cout << "P - *P_ptr = " << P - *P_ptr << endl;

//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
void camera::print_external_and_internal_params()
{
//   cout << "inside camera::print_external_and_internal_params()" << endl;

   cout.precision(12);
   cout << "# Normalization factor rho = " << rho << endl;
   cout << "# fu = " << fu << " fv = " << fv << endl;
   cout << "# u0 = " << u0 << " v0 = " << v0 << endl;
   cout << "# theta = " << theta*180/PI << " degs" << endl;

   cout << "# Camera world X = " << camera_world_posn.get(0) << endl;
   cout << "# Camera world Y = " << camera_world_posn.get(1) << endl;
   cout << "# Camera world Z = " << camera_world_posn.get(2) << endl;

   double az,el,roll;
   get_az_el_roll_from_Rcamera(az,el,roll);
   cout << "# Camera azimuth = " << az*180/PI << " degs" << endl;
   cout << "# Camera elevation = " << el*180/PI << " degs" << endl;
   cout << "# Camera roll = " << roll*180/PI << " degs" << endl << endl;

   cout << "# Uhat = " << Uhat.get(0) << " , " << Uhat.get(1) << " , "
        << Uhat.get(2) << endl;
   cout << "# Vhat = " << Vhat.get(0) << " , " << Vhat.get(1) << " , "
        << Vhat.get(2) << endl;
   cout << "# What = " << What.get(0) << " , " << What.get(1) << " , "
        << What.get(2) << endl;

   M_ptr->inverse(*Minv_ptr);
   cout << "Minv = " << *Minv_ptr << endl;
}

// ---------------------------------------------------------------------
void camera::compute_pinhole_model_params(int sgn,genmatrix& P)
{
//   cout << "inside camera::compute_pinhole_model_params()" << endl;
//   cout << "*P_ptr = " << *P_ptr << endl;
//   cout << "*M_ptr = " << *M_ptr << endl;
//   cout << "*Minv_ptr = " << *Minv_ptr << endl;

   threevector a1(P_ptr->get(0,0),P_ptr->get(0,1),P_ptr->get(0,2));
   threevector a2(P_ptr->get(1,0),P_ptr->get(1,1),P_ptr->get(1,2));
   threevector a3(P_ptr->get(2,0),P_ptr->get(2,1),P_ptr->get(2,2));
   threevector b(P_ptr->get(0,3),P_ptr->get(1,3),P_ptr->get(2,3));

//   cout << "sgn = " << sgn << endl;
//   cout << "a1 = " << a1 << " a2 = " << a2 << " a3 = " << a3 << endl;
//   cout << "b = " << b << endl;
   
// Scale factor rho that relates *P_ptr which has unit norm to actual
// camera parameters:

   rho=sgn/a3.magnitude();	// scale factor
//   cout << "rho = " << rho << endl;

   u0=sqr(rho)*(a1.dot(a3));
   v0=sqr(rho)*(a2.dot(a3));
//   cout << "u0 = " << u0 << " v0 = " << v0 << endl;

   threevector cross13=a1.cross(a3);
   threevector cross23=a2.cross(a3);
   threevector n13=cross13.unitvector();
   threevector n23=cross23.unitvector();
   double cos_theta=-n13.dot(n23);

// Theta = angle between u_hat and v_hat.  It should be close to 90
// degrees:

   theta=acos(cos_theta);
//   cout << "theta = " << theta*180/PI << endl;

//   double cot_theta=cos(theta)/sin(theta);
//   double csc_theta=1/sin(theta);

// fu [fv] = U [V]-axis focal length parameters.  On 9/14/07, we
// realized that fu and fv should be NEGATIVE.  The comments within
// the Computer Vision book by Ponce and Forsyth that alpha=fvu and
// beta=fv can be taken to be positive are WRONG...

   fu=-sqr(rho)*cross13.magnitude()*sin(theta);
   fv=-sqr(rho)*cross23.magnitude()*sin(theta);
//   cout << "fu = " << fu << " fv = " << fv << endl;

   construct_internal_parameter_K_matrix();

// Reconstruct camera's rotation matrix and translation vector:

   Uhat=-n23;	// sgn changed on 9/30/07
   What=rho*a3;
   Vhat=What.cross(Uhat);

//   cout << "Uhat = " << Uhat << endl;
//   cout << "What = " << What << endl;
//   cout << "Vhat = " << Vhat << endl;

   threevector khat=pixel_ray_direction(u0,v0);
//   cout << "khat = " << khat << endl;
//   cout << "Uhat = " << Uhat << endl;
//   cout << "Vhat = " << Vhat << endl;
//   cout << "What = " << What << endl;

//   cout << "Uhat.Uhat = " << Uhat.dot(Uhat) << endl;
//   cout << "Vhat.Vhat = " << Vhat.dot(Vhat) << endl;
//   cout << "khat.khat = " << khat.dot(khat) << endl;
//   cout << "Uhat.Vhat = " << Uhat.dot(Vhat) << endl;
//   cout << "Vhat.khat = " << Vhat.dot(khat) << endl;
//   cout << "khat.Uhat = " << khat.dot(Uhat) << endl;

// On 9/22/07, we empirically confirmed that khat ( = wavefront
// momentum direction vector) is proportional to What (and in fact
// should precisely equal true - Uhat x Vhat !):

   rotation Rcamera_reconstructed;
   Rcamera_reconstructed.put_row(0,Uhat);
   Rcamera_reconstructed.put_row(1,Vhat);
   Rcamera_reconstructed.put_row(2,What);
   
//   cout << "*K_ptr = " << *K_ptr << endl;
//   cout << "Rcamera_reconstructed = " << Rcamera_reconstructed << endl;
//   double det;
//   if (Rcamera_reconstructed.determinant(det))
//   {
//      cout << "Rcamera_reconstructed.det = " << det << endl;
//   }
//   else
//   {
//      cout << "Cannot compute Rcamera_reconstructed determinant" << endl;
//   }
   
   genmatrix Rt_true(3,4);
   Rt_true=rho * (*Kinv_ptr) * (*P_ptr);
//   cout << "Rt_true = " << Rt_true << endl;

   threevector t=rho* (*Kinv_ptr) *b;	// t = -Rcamera * camera_world_posn
   genmatrix Rt(3,4);
   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         Rt.put(i,j,Rcamera_reconstructed.get(i,j));
      }
      Rt.put(i,3,t.get(i));
   }

//   cout << "t = " << t << endl;
//   cout << "Rt = " << Rt << endl;

   P=1.0/rho*( *K_ptr * Rt);

// Recall member variable camera_world_posn holds camera's absolute
// position:

   threevector camera_world_posn_reconstructed=
      -Rcamera_reconstructed.transpose()*t;
//   cout << "camera_world_posn_reconstructed = " 
//        << camera_world_posn_reconstructed << endl;
//   cout << "camera_world_posn = " << camera_world_posn << endl;
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function project_XYZ_to_UV_coords()

void camera::project_XYZ_to_UV_coordinates(
   const threevector& XYZ,threevector& UVW)
{
//   cout << "inside camera::project_XYZ_to_UV_coordinates()" << endl;

   double U,V;
   project_XYZ_to_UV_coordinates(XYZ.get(0),XYZ.get(1),XYZ.get(2),U,V);
   UVW.put(0,U);
   UVW.put(1,V);
   UVW.put(2,0);
}

/*
// ---------------------------------------------------------------------
// Member function cyl_project_XYZ_to_UV_coords()

void camera::cyl_project_XYZ_to_UV_coordinates(
   double x,double y,double z,double& u,double& v)
{
//   cout << "inside camera::cyl_project_XYZ_to_UV_coordinates()" << endl;

//   threevector rel_P(P-camera_world_posn);
//   double X=-rel_P.dot(What);
//   double Y=-rel_P.dot(Uhat);
//   double Z=rel_P.dot(Vhat);

   double X=
      -(x-camera_world_posn.get(0))*What.get(0)
      -(y-camera_world_posn.get(1))*What.get(1)
      -(z-camera_world_posn.get(2))*What.get(2);
   double Y=
      -(x-camera_world_posn.get(0))*Uhat.get(0)
      -(y-camera_world_posn.get(1))*Uhat.get(1)
      -(z-camera_world_posn.get(2))*Uhat.get(2);
   double Z=
       (x-camera_world_posn.get(0))*Vhat.get(0)
      +(y-camera_world_posn.get(1))*Vhat.get(1)
      +(z-camera_world_posn.get(2))*Vhat.get(2);
//   double R=sqrt(X*X+Y*Y);
//   double theta=atan2(Y,X);
//   u=u0+fu*theta;
//   v=v0-fu*Z/R;

   u=u0+fu*atan2(Y,X);
   v=v0-fu*Z/sqrt(X*X+Y*Y);
}
*/

// ---------------------------------------------------------------------
// Member function project_polyhedron_face_into_imageplane() takes in
// index f labeling some face within input *polyhedron_ptr.  This
// method projects its XYZ vertices into the camera's image plane.
// The projected U,V coordinate pairs are returned.  This method also
// checks whether any of the projected vertices land inside the
// current image's boundaries.  If not, output bool
// face_inside_imageplane_flag is set to false.

vector<threevector> camera::project_polyhedron_face_into_imageplane(
   int f,const polyhedron* polyhedron_ptr,bool& face_inside_imageplane_flag)
{
//   cout << "inside camera::project_polyhedron_face_into_imageplane()"
//        << endl;
   
   face_inside_imageplane_flag=false;
   const face* face_ptr=polyhedron_ptr->get_face_ptr(f);
   
   vector<threevector> projected_vertices;
   for (unsigned int v=0; v<face_ptr->get_n_vertices(); v++)
   {
      threevector curr_vertex=face_ptr->get_vertex_from_chain(v).get_posn();
      double U,V;
      project_XYZ_to_UV_coordinates(
         curr_vertex.get(0),curr_vertex.get(1),curr_vertex.get(2),U,V);
      projected_vertices.push_back(threevector(U,V));
//      cout << "proj_vertices.push_back(threevector("
//           << projected_vertices.back().get(0) << "," 
//           << projected_vertices.back().get(1) << "));" << endl;
//      cout << "v = " << v << " curr_vertex = " << curr_vertex << endl;
//      cout << " projected vertex = " 
//           << projected_vertices.back().get(0) << " , "
//           << projected_vertices.back().get(1) << endl;

//      cout << "video_ptr = " << video_ptr << endl;
//      cout << "minU = " << video_ptr->get_minU() 
//           << " maxU = " << video_ptr->get_maxU() << endl;
//      cout << "minV = " << video_ptr->get_minV() 
//           << " maxV = " << video_ptr->get_maxV() << endl;
      
      if (U >= video_ptr->get_minU() && U <= video_ptr->get_maxU() &&
          V >= video_ptr->get_minV() && V <= video_ptr->get_maxV())
      {
         face_inside_imageplane_flag=true;
      }
   } // loop over v labeling current face vertices

   return projected_vertices;
}

// ==========================================================================
// Camera parameter setting member functions
// ==========================================================================

// Member function construct_internal_parameter_K_matrix fills the
// entries of *K_ptr and *Kinv_ptr.

void camera::construct_internal_parameter_K_matrix()
{
//   cout << "inside camera::construct_internal_parameter_K_matrix()" << endl;
//   cout << "theta = " << theta*180/PI << " degs" << endl;

   double cot_theta=cos(theta)/sin(theta);
   double csc_theta=1/sin(theta);

//   cout << "cot_theta = " << cot_theta << endl;
//   cout << "csc_theta = " << csc_theta << endl;
//   cout << "fu = " << fu << " fv = " << fv 
//        << " u0 = " << u0 << " v0 = " << v0 << endl;

   K_ptr->clear_values();
   K_ptr->put(0,0,fu);
   K_ptr->put(0,1,-fu*cot_theta);
   K_ptr->put(0,2,u0);
   K_ptr->put(1,0,0);
   K_ptr->put(1,1,fv*csc_theta);
   K_ptr->put(1,2,v0);
   K_ptr->put(2,0,0);
   K_ptr->put(2,1,0);
   K_ptr->put(2,2,1);

   K_ptr->inverse(*Kinv_ptr);

//   cout << "*K_ptr = " << *K_ptr << endl;
//   cout << "*Kinv_ptr = " << *Kinv_ptr << endl;
//   cout << "K * Kinv = " << (*K_ptr) * (*Kinv_ptr) << endl;
}

// ---------------------------------------------------------------------
// Member function set_rotation_angles

void camera::set_aircraft_rotation_angles(double pitch,double roll,double yaw)
{
   m_pitch=pitch;
   m_roll=roll;
   m_yaw=yaw;
}

void camera::set_mount_rotation_angles(double alpha,double beta,double phi)
{
   m_alpha=alpha;
   m_beta=beta;
   m_phi=phi;
}

// ---------------------------------------------------------------------
// Member function compute_rotation_matrix takes in time
// dependent pitch, roll and yaw angles along with "constant" camera
// mount angles alpha, beta, phi ( = dpitch, droll and dyaw).  It sets
// "internal member matrix" R_camera equal to Rpitch*Rroll*Ryaw which
// conforms to the angle ordering used in ALIRT imagery generation
// methods.

void camera::compute_rotation_matrix()
{
   rotation R_rpy,R_mount;

   R_rpy=generate_attitude_rotation(m_pitch,m_roll,m_yaw);
   R_mount=generate_attitude_rotation(m_alpha,m_beta,m_phi);
   *R_camera_ptr=R_rpy*R_mount;

// For speedup purposes, we save the product of *R_camera_ptr and
// camera_world_posn within member threevector
// Rcamera_times_camera_world_posn:

   Rcamera_times_camera_world_posn=*R_camera_ptr*camera_world_posn;
}

// ---------------------------------------------------------------------
// Member function generate_attitude_rotation takes in aircraft
// attitude angles x = pitch, y = roll, z = yaw.  It returns the total
// rotation matrix R by which pointing vectors need to be multiplied
// in order to take the aircraft's orientation into account.

// Note: Yaw is measured *CLOCKWISE* relative to north.  This
// convention violates the right hand rule.

rotation camera::generate_attitude_rotation(
   double pitch,double roll,double yaw)
{

//	 	1 	0 	0
// Rx(pitch) = 	0 	cos_pitch	-sin_pitch
// 		0 	sin_pitch	cos_pitch

// 		cos_roll	0	sin_roll
// Ry(roll) = 	0	1	0
//		-sin_roll	0 	cos_roll

// 		cos_yaw	sin_yaw	0
// Rz(-yaw) =  -sin_yaw	cos_yaw	0
//		0	0	1

// R(pitch,roll,yaw) = Rz(-yaw) * Rx(pitch) * Ry(roll)

   double sin_pitch=sin(pitch);
   double cos_pitch=cos(pitch);
   double sin_roll=sin(roll);
   double cos_roll=cos(roll);
   double sin_yaw=sin(yaw);
   double cos_yaw=cos(yaw);
		
   rotation R;
   R.put(0,0,cos_yaw*cos_roll+sin_yaw*sin_pitch*sin_roll);
   R.put(0,1,sin_yaw*cos_pitch);
   R.put(0,2,cos_yaw*sin_roll-sin_yaw*sin_pitch*cos_roll);
   R.put(1,0,cos_yaw*sin_pitch*sin_roll-sin_yaw*cos_roll);
   R.put(1,1,cos_yaw*cos_pitch);
   R.put(1,2,-sin_yaw*sin_roll-cos_yaw*sin_pitch*cos_roll);
   R.put(2,0,-cos_pitch*sin_roll);
   R.put(2,1,sin_pitch);
   R.put(2,2,cos_pitch*cos_roll);

   return R;
}

// ---------------------------------------------------------------------
// Member function extract_attitude_angles takes in attitude rotation
// matrix R and returns its corresponding roll, pitch and yaw angles.
// We (reasonably!) assume that pitch lies in the interval [-90 degs,
// 90 degs].  On 8/28/05, we confirmed that positive pitch angle
// implies that the camera looks in the forward direction.  This
// method is essentially the inverse of generate_attitude_rotation.

void camera::extract_attitude_angles(const rotation& R)
{
   double cos_pitch=sqrt(sqr(R.get(0,1))+sqr(R.get(1,1)));
   m_roll=atan2(-R.get(2,0),R.get(2,2));
   m_yaw=atan2(R.get(0,1),R.get(1,1));
   m_pitch=atan2(R.get(2,1),cos_pitch);
}
   
// ==========================================================================
// Projection matrix manipulation member functions
// ==========================================================================

// Member function clear_previous_projection_matrices() initializes
// *P_prev_ptr and *Pdot_prev_ptr to the zero matrix:

void camera::clear_previous_projection_matrices()
{
   P_prev_ptr->clear_values();
   Pdot_prev_ptr->clear_values();
   Pdotdot_prev_ptr->clear_values();
}

// ---------------------------------------------------------------------
// Member function set_projection_matrix() takes in an alpha value for
// an alpha-beta filter by which the input projection matrix P is
// smoothed if input boolean temporally_filter_flag==true.

void camera::set_projection_matrix(
   genmatrix& P,double alpha,bool temporally_filter_flag)
{
//   cout << "inside camera::set_projection_matrix(P)" << endl;

   if (P.get_mdim() != 3 || P.get_ndim() != 4)
   {
      cout << "Error in camera::set_projection_matrix()!" << endl;
      cout << "mdim = " << P.get_mdim() << " should equal 3" << endl;
      cout << "ndim = " << P.get_ndim() << " should equal 4" << endl;
      outputfunc::enter_continue_char();
      return;
   }
   
   *P_ptr=P;

//   cout << "alpha = " << alpha << " temporally_filter_flag = "
//        << temporally_filter_flag << endl;
   if (alpha > 0 && temporally_filter_flag)
   {
//      double alpha=0.17;	// OK for pre Feb 7 videos
//      double beta=sqr(alpha)/(2-alpha);

//      double descrim=sqrt(sqr(alpha)+4*beta);
//      double lambda_plus=0.5*fabs(alpha+descrim);
//      double lambda_minus=0.5*fabs(alpha-descrim);
//      cout << "alpha = " << alpha << " beta = " << beta << endl;
//      cout << "lambda_plus = " << lambda_plus 
//           << " lambda_minus = " << lambda_minus << endl;
//      alpha_filter_projection_matrix(alpha);

//      alphabeta_filter_projection_matrix(alpha,beta);

      double beta=2*(2-alpha)-4*sqrt(1-alpha);
      double gamma=sqr(beta)/(2*alpha);
      alphabetagamma_filter_projection_matrix(alpha,beta,gamma);
      P=*P_ptr;
   }
   else
   {
      clear_previous_projection_matrices();
   }
   
   bool nonsingular_submatrix_flag=decompose_projection_matrix();
//   cout << "nonsingular_submatrix_flag = " << nonsingular_submatrix_flag
//        << endl;
//   cout << "3x4 projection matrix P = " << *P_ptr << endl;

   if (nonsingular_submatrix_flag)
   {
      compute_corner_ray_and_UV_axes_dirs();
//      cout << "Camera world posn = " << camera_world_posn << endl;
//      cout << "Camera pointing direction = " << get_pointing_dir() << endl;

//      for (unsigned int i=0; i<UV_corner_world_ray.size(); i++)
//      {
//         cout << "UV corner world ray #" << i   << " : " 
//              << UV_corner_world_ray[i] << endl;
//      }
//      cout << "Uhat = " << Uhat << " Vhat = " << Vhat << endl;

   }
}

// ---------------------------------------------------------------------
void camera::alpha_filter_projection_matrix(double alpha)
{
//   cout << "inside camera::alpha_filter_projection_matrix()" << endl;
   
   if (nearly_equal(P_prev_ptr->get(0,0),0) &&
       nearly_equal(P_prev_ptr->get(0,1),0) &&
       nearly_equal(P_prev_ptr->get(0,2),0) &&
       nearly_equal(P_prev_ptr->get(0,3),0) &&
       nearly_equal(P_prev_ptr->get(1,0),0) &&
       nearly_equal(P_prev_ptr->get(1,1),0) &&
       nearly_equal(P_prev_ptr->get(1,2),0) &&
       nearly_equal(P_prev_ptr->get(1,3),0) &&
       nearly_equal(P_prev_ptr->get(2,0),0) &&
       nearly_equal(P_prev_ptr->get(2,1),0) &&
       nearly_equal(P_prev_ptr->get(2,2),0) &&
       nearly_equal(P_prev_ptr->get(2,3),0) &&
       nearly_equal(P_prev_ptr->get(3,0),0) &&
       nearly_equal(P_prev_ptr->get(3,1),0) &&
       nearly_equal(P_prev_ptr->get(3,2),0) &&
       nearly_equal(P_prev_ptr->get(3,3),0) )
   {
      alpha=1.0;	// exclusively use raw input 
   }

   for (unsigned int r=0; r<3; r++)
   {
      for (unsigned int c=0; c<4; c++)
      {
         double filtered_P_entry=filterfunc::alpha_filter(
            P_ptr->get(r,c),P_prev_ptr->get(r,c),alpha);
         P_ptr->put(r,c,filtered_P_entry);
         P_prev_ptr->put(r,c,filtered_P_entry);
      } // loop over index c labeling projection matrix columns
   } // loop over index r labeling projection matrix rows
}

// ---------------------------------------------------------------------
void camera::alphabeta_filter_projection_matrix(double alpha,double beta)
{
//   cout << "inside camera::alphabeta_filter_projection_matrix()" << endl;
   
   if (nearly_equal(P_prev_ptr->get(0,0),0) &&
       nearly_equal(P_prev_ptr->get(0,1),0) &&
       nearly_equal(P_prev_ptr->get(0,2),0) &&
       nearly_equal(P_prev_ptr->get(0,3),0) &&
       nearly_equal(P_prev_ptr->get(1,0),0) &&
       nearly_equal(P_prev_ptr->get(1,1),0) &&
       nearly_equal(P_prev_ptr->get(1,2),0) &&
       nearly_equal(P_prev_ptr->get(1,3),0) &&
       nearly_equal(P_prev_ptr->get(2,0),0) &&
       nearly_equal(P_prev_ptr->get(2,1),0) &&
       nearly_equal(P_prev_ptr->get(2,2),0) &&
       nearly_equal(P_prev_ptr->get(2,3),0) )
   {
      alpha=1.0;	// exclusively use raw input 
      beta=1.0;
   }

   double curr_filtered_P,curr_filtered_Pdot,dt=1.0;
   for (unsigned int r=0; r<3; r++)
   {
      for (unsigned int c=0; c<4; c++)
      {
         filterfunc::alphabeta_filter(
            P_ptr->get(r,c),P_prev_ptr->get(r,c),Pdot_prev_ptr->get(r,c),
            curr_filtered_P,curr_filtered_Pdot,alpha,beta,dt);

         P_ptr->put(r,c,curr_filtered_P);
         P_prev_ptr->put(r,c,curr_filtered_P);
         Pdot_prev_ptr->put(r,c,curr_filtered_Pdot);

      } // loop over index c labeling projection matrix columns
   } // loop over index r labeling projection matrix rows
}

// ---------------------------------------------------------------------
void camera::alphabetagamma_filter_projection_matrix(
   double alpha,double beta,double gamma)
{
//   cout << "inside camera::alphabetagamma_filter_projection_matrix()" << endl;
   
   if (nearly_equal(P_prev_ptr->get(0,0),0) &&
       nearly_equal(P_prev_ptr->get(0,1),0) &&
       nearly_equal(P_prev_ptr->get(0,2),0) &&
       nearly_equal(P_prev_ptr->get(0,3),0) &&
       nearly_equal(P_prev_ptr->get(1,0),0) &&
       nearly_equal(P_prev_ptr->get(1,1),0) &&
       nearly_equal(P_prev_ptr->get(1,2),0) &&
       nearly_equal(P_prev_ptr->get(1,3),0) &&
       nearly_equal(P_prev_ptr->get(2,0),0) &&
       nearly_equal(P_prev_ptr->get(2,1),0) &&
       nearly_equal(P_prev_ptr->get(2,2),0) &&
       nearly_equal(P_prev_ptr->get(2,3),0) )
   {
      *P_prev_ptr=*P_ptr;
   }

   double curr_filtered_P,curr_filtered_Pdot,curr_filtered_Pdotdot,dt=1.0;
   for (unsigned int r=0; r<3; r++)
   {
      for (unsigned int c=0; c<4; c++)
      {
         filterfunc::alphabetagamma_filter(
            P_ptr->get(r,c),P_prev_ptr->get(r,c),Pdot_prev_ptr->get(r,c),
            Pdotdot_prev_ptr->get(r,c),
            curr_filtered_P,curr_filtered_Pdot,curr_filtered_Pdotdot,
            alpha,beta,gamma,dt);

         P_ptr->put(r,c,curr_filtered_P);
         P_prev_ptr->put(r,c,curr_filtered_P);
         Pdot_prev_ptr->put(r,c,curr_filtered_Pdot);
         Pdotdot_prev_ptr->put(r,c,curr_filtered_Pdotdot);

      } // loop over index c labeling projection matrix columns
   } // loop over index r labeling projection matrix rows
}

// ---------------------------------------------------------------------
// Member function construct_seven_param_projection_matrix()

void camera::construct_seven_param_projection_matrix()
{
//   cout << "inside camera::construct_seven_param_projection_matrix()" << endl;

   construct_internal_parameter_K_matrix();
   construct_projection_matrix_for_fixed_K();
   compute_corner_ray_and_UV_axes_dirs();

   double aspect_ratio=get_u0()/get_v0();
   camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
      get_fu(),aspect_ratio,FOV_u,FOV_v);
}

// ---------------------------------------------------------------------
// Member function construct_projection_matrix fills the entries of
// the 3x4 projection matrix *P_ptr which maps 3D world space points
// to 2D image plane coordinates.

bool camera::construct_projection_matrix(bool recompute_internal_params_flag)
{
//   cout << "inside camera::construct_projection_matrix()" << endl;
//   cout << "recompute_internal_params_flag = " 
//        << recompute_internal_params_flag << endl;
//   cout << "camera this = " << this << endl;

   if (recompute_internal_params_flag) 
      construct_internal_parameter_K_matrix();

   construct_projection_matrix_for_fixed_K();

   bool nonsingular_submatrix_flag=decompose_projection_matrix();
   if (nonsingular_submatrix_flag)
   {
      compute_corner_ray_and_UV_axes_dirs();
   }

   return nonsingular_submatrix_flag;
}

// ---------------------------------------------------------------------
// Member function construct_projection_matrix_for_fixed_K fills the
// entries of the 3x4 projection matrix *P_ptr after member variables
// *K_ptr, *R_camera_ptr, camera_world_posn and rho have been
// specified,

void camera::construct_projection_matrix_for_fixed_K()
{
//   cout << "inside camera::construct_projection_matrix_for_fixed_K()" << endl;
//   cout << "rho = " << rho << endl;

   *M_ptr = (*K_ptr) * (*R_camera_ptr);
   M_ptr->inverse(*Minv_ptr);
   p4 = - *M_ptr * camera_world_posn;

//   cout << "*K_ptr = " << *K_ptr << endl;
//   cout << "*R_camera_ptr = " << *R_camera_ptr << endl;
//   cout << "*M_ptr = " << *M_ptr << endl;
//   cout << "p4 = " << p4 << endl;

   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         P_ptr->put(i,j,M_ptr->get(i,j)/rho);               
      }
      P_ptr->put(i,3,p4.get(i)/rho);
   }

//   cout << "Projection matrix P = " << *P_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function decompose_projection_matrix breaks the 3x4 matrix
// *P_ptr into a 3x3 submatrix *M_ptr and a 3x1 column vector p4.  It
// also computes the inverse of *M_ptr.  This method should be called
// before the camera's world position is needed or 2D UV imageplane
// rays are backprojected into 3D XYZ world space.  If *M_ptr is
// singular, this boolean method returns false.

bool camera::decompose_projection_matrix()
{
//   cout << "inside camera::decompose_projection_matrix()" << endl;
//   cout << "*P_ptr = " << *P_ptr << endl;
   
   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         M_ptr->put(i,j,P_ptr->get(i,j));               
      }
      p4.put(i,P_ptr->get(i,3));
   }

   bool nonsingular_submatrix_flag=compute_pointing_direction();
//   cout << "nonsingular_submatrix_flag = " << nonsingular_submatrix_flag
//        << endl;
   
   if (nonsingular_submatrix_flag)
   {
      M_ptr->inverse(*Minv_ptr);
      camera_world_posn=-(*Minv_ptr)*p4;

      set_imageplane_thru_center();
      extract_external_and_internal_params();

//      rotation Rderived=Kinv*M;
      

//   cout << "*P_ptr = " << *P_ptr << endl;
//      cout << "*K_ptr = " << *K_ptr << endl;
//   cout << "*M_ptr = " << *M_ptr << endl;
//      cout << "R = Kinv * M = "
//           << *Kinv_ptr * (*M_ptr) << endl;
//      cout << "*Minv_ptr = " << *Minv_ptr << endl;
//      cout << "M*Minv = " << *M_ptr * *Minv_ptr << endl;

//   cout << "p4 = " << p4 << endl;
//      cout << "camera_world_posn = " << camera_world_posn << endl;
   }

//   cout << "nonsingular_submatrix_flag = " << nonsingular_submatrix_flag
//        << endl;
   return nonsingular_submatrix_flag;
}

// ---------------------------------------------------------------------
// Member function compute_corner_ray_and_UV_axes_dirs() calculates
// the image plane corners located away from the camera's world
// position along the corner rays.  It then takes differences of the
// corner positions to fix the signs of Uhat and Vhat coming from the
// 1st and 2nd rows of *R_camera_ptr.
 
void camera::compute_corner_ray_and_UV_axes_dirs()
{
//   cout << "inside camera::compute_corner_ray_and_UV_axes_dirs()" << endl;
//   cout << "initially, Uhat = " << Uhat << endl;
//   cout << "initially, Vhat = " << Vhat << endl;

   UV_corner_world_ray.clear();
   vector<threevector> UV_corner_posn;

   for (unsigned int i=0; i<UV_corner.size(); i++)
   {
//      cout << "c = " << i
//           << " UV_corner = " 
//           << UV_corner[i].get(0) << " , " 
//           << UV_corner[i].get(1) 
//           << endl;

      UV_corner_world_ray.push_back(pixel_ray_direction(UV_corner[i]));
      UV_corner_posn.push_back(camera_world_posn+UV_corner_world_ray.back());

//      cout << "camera_world_posn = " << camera_world_posn << endl;
//      cout << " UV_corner_world_ray = " 
//           << UV_corner_world_ray.back().get(0) << " , " 
//           << UV_corner_world_ray.back().get(1) << " , " 
//           << UV_corner_world_ray.back().get(2) << endl;
//      cout << " UV_corner_posn = " << UV_corner_posn.back() << endl;
   } // loop over index i labeling UV imageplane corners
   threevector approx_Uhat=(UV_corner_posn[1]-UV_corner_posn[0]).unitvector();
   threevector approx_Vhat=(UV_corner_posn[3]-UV_corner_posn[0]).unitvector();

//   cout << "*R_camera_ptr = " << *R_camera_ptr << endl;
//   R_camera_ptr->get_row(0,Uhat);
//   R_camera_ptr->get_row(1,Vhat);

   if (Uhat.dot(approx_Uhat) < 0)
   {
//      cout << "Uhat = " << Uhat << endl;
//      cout << "approx_Uhat = " << approx_Uhat << endl;
      Uhat *= -1;
   }
   
   if (Vhat.dot(approx_Vhat) < 0)
   {
//      cout << "Vhat = " << Vhat << endl;
//      cout << "approx_Vhat = " << approx_Vhat << endl;
      Vhat *= -1;
   }
   What=Uhat.cross(Vhat);

   R_camera_ptr->put_row(0,Uhat);
   R_camera_ptr->put_row(1,Vhat);
   R_camera_ptr->put_row(2,What);

//   cout << "Uhat = " << Uhat << endl;
//   cout << "Vhat = " << Vhat << endl;
//   cout << "Uhat.dot(Vhat) = " << Uhat.dot(Vhat) << endl;
//   cout << "acos(uhat.vhat) = " << acos(Uhat.dot(Vhat))*180/PI << endl;
//   cout << "What = " << What << endl;
//   cout << "*R_camera_ptr = " << *R_camera_ptr << endl;

//   double curr_az,curr_el,curr_roll;
//   get_az_el_roll_from_Rcamera(curr_az,curr_el,curr_roll);

   delete camera_frustum_ptr;
   camera_frustum_ptr=new camera_frustum(
      camera_world_posn,Uhat,Vhat,What,UV_corner_world_ray);
//   cout << "*camera_frustum_ptr = " << *camera_frustum_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function compute_pointing_direction() initially checks if
// the first 3x3 submatrix M within P_3x4 is nonsingular.  If not, we
// assume the current camera points along -Z_hat (e.g. nadir aerial
// image) and return false.  Otherwise, this method recovers the
// pointing direction from M and returns true.

bool camera::compute_pointing_direction()
{
//   cout << "inside camera::compute_pointing_dir()" << endl;
   double detM;
   bool flag=M_ptr->determinant(detM);

   if (!flag || nearly_equal(detM,0,1E-99))
   {
      cout << "WARNING in camera::compute_pointing_direction" << endl;
      cout << "M_inv is not defined!" << endl;
      cout << "*M_ptr = " << *M_ptr << endl;

      cout << "We assume camera points along -z_hat..." << endl;
      What=z_hat;
      return false;
   }
   double abs_detM=fabs(detM);
//   cout << "abs_detM = " << abs_detM << endl;

// Since the entries in matrix M can be very small (e.g. 1E-7), the
// determinant can be extremely small (e.g. 1E-21).  We therefore
// renormalize M so that its determinant has unit magnitude:

   genmatrix M_renorm(3,3);
   M_renorm=*M_ptr;
   M_renorm /= pow(abs_detM,0.33333333);

//   cout << "M_renorm = " << M_renorm << endl;
   double detM_renorm;
   M_renorm.determinant(detM_renorm);
//   cout << "det(M_renorm) = " << detM_renorm << endl;
   sgn_detM=sgn(detM_renorm);

   threevector m3;
   M_ptr->get_row(2,m3);

// On 5/31/07, we realized that the right-handed camera coordinate
// system ("UVW") defined by Hartley and Zisserman in their figure 6.1
// of Multiple View Geometry in Computer Vision is rotated relative to
// our UVW coordinate system (with U increasing rightwards, V
// increasing upwards, and W increasing outwards)!  It is rotated by
// 180 degs about our common V axis.  So Hartley & Zimmerman's U and W
// coordinates are opposite in sign to ours.  To compensate for this
// coordinate system difference, we need to introduce a minus sign
// into the equation below:

   threevector pointing_dir=-sgn_detM*m3.unitvector();
   What=-pointing_dir;
   
//   cout << "m3 = " << m3 << endl;
//   cout << "m3.unitvector() = " << m3.unitvector() << endl;
//   cout << "sgn(detM) = " << sgn(detM) << endl;
//   cout << "pointing_dir = " << pointing_dir << endl;

   return true;
}

// ---------------------------------------------------------------------
// Member function set_imageplane_thru_center()

void camera::set_imageplane_thru_center()
{
//   cout << "inside camera::set_imageplane_thru_center()" << endl;

   delete imageplane_thru_center_ptr;
   imageplane_thru_center_ptr=new plane(get_pointing_dir(),camera_world_posn);
//   cout << "imageplane_thru_center = " << *imageplane_thru_center_ptr
//        << endl;
}

// ---------------------------------------------------------------------
// Member function construct_projection_pseudo_inverse returns the
// pseudo-inverse of projection matrix P.
// Pplus=Ptrans*(P*Ptrans)**-1.  P*Pplus = I_3x3.

genmatrix camera::construct_projection_pseudo_inverse()
{
//   cout << "inside camera::construct_projection_pseudo_inverse()" << endl;
   genmatrix Pplus(4,3);
   *M_ptr=(*P_ptr)*P_ptr->transpose();
   M_ptr->inverse(*Minv_ptr);
   Pplus=P_ptr->transpose()*(*Minv_ptr);
//         cout << "P = " << P << endl;
//         cout << "Pplus = " << Pplus << endl;
//         cout << "P*Pplus = " << P*Pplus << endl;

   return Pplus;
}


// ==========================================================================
// Field of view member functions
// ==========================================================================

// Member function compute_pyramid_sidelength takes in the downrange
// distance for a movie window.  This method returns the length of the
// side edges of a pyramid (those which connect onto the apex) for
// which the base lies at the same downrange distance as the movie window.

double camera::compute_pyramid_sidelength(double movie_downrange_distance)
   const
{
   threevector a_hat=-get_What();
   threevector w_hat=get_UV_corner_world_ray().at(0);
   double pyramid_sidelength=movie_downrange_distance/a_hat.dot(w_hat);
   return pyramid_sidelength;
}

double camera::compute_movie_downrange_distance(double pyramid_sidelength)
   const
{
   threevector a_hat=-get_What();
   threevector w_hat=get_UV_corner_world_ray().at(0);
   double movie_downrange_distance=pyramid_sidelength*a_hat.dot(w_hat);
   return movie_downrange_distance;
}

// ---------------------------------------------------------------------
// Member function compute_fields_of_view() calculates the horizontal
// and vertical fields-of-view for an entire image using the focal
// length and camera center parameters.  FOV_u and FOV_v are somewhat
// smaller than n_horizontal_pixels * square_pixel_FOV and
// n_vertical_pixels * square_pixel_FOV.

void camera::compute_fields_of_view(
   double Umax, double Umin, double Vmax, double Vmin)
{
//   cout << "inside camera::compute_fields_of_view()" << endl;

   double Ucenter=0.5*(Umax+Umin);
   double Vcenter=0.5*(Vmax+Vmin);

   threevector horiz_max_ray=pixel_ray_direction(Umax,Vcenter);
   threevector horiz_min_ray=pixel_ray_direction(Umin,Vcenter);
   FOV_u=acos(horiz_max_ray.dot(horiz_min_ray));
//   cout << "Image FOV_u = " << FOV_u*180/PI << " degs " << endl;
   
   threevector vert_max_ray=pixel_ray_direction(Ucenter,Vmax);
   threevector vert_min_ray=pixel_ray_direction(Ucenter,Vmin);
   FOV_v=acos(vert_max_ray.dot(vert_min_ray));
//   cout << "Image FOV_v = " << FOV_v*180/PI << " degs " << endl;

//   cout << "FOV_v approx_equals 1/fv = " << 1.0/fv*180/PI 
//	  << " degs" << endl;
}

// ---------------------------------------------------------------------
// Member function focal_param_corresponding_to_image_rescaling()
// takes in parameter rho which represents a scale factor for an image
// (and also equals square pixel rescaling factor).  It returns the
// new focal parameter which yields the requested angle-space image
// rescaling in 3D.

double camera::focal_param_corresponding_to_image_rescaling(
   int n_vertical_pixels,double fu_init,double rho)
{
   double term1=0.5/(n_vertical_pixels*fu_init);
   double term2=rho*atan(term1);
   double term3=2*n_vertical_pixels*tan(term2);
   double fnew=1.0/term3;
   return fnew;
}

// ---------------------------------------------------------------------
// Member function rescale_focal_length() takes in an initial focal
// length parameter fu_init along with initial azimuth, elevation and
// roll angles (in radians).  It computes the new focal parameter
// corresponding to the photograph being rescaled by input parameter
// scale_factor.  It also stretches the photo's azimuth, elevation and
// rotation angles.  This method resets the camera's focal and
// rotation parameters, and it recomputes the camera's 3x4 projection
// matrix.

void camera::rescale_focal_length(
   int n_vertical_pixels,double fu_init,double scale_factor,
   double az_init,double el_init,double roll_init)
{
//   cout << "inside camera::rescale_focal_length()" << endl;
   
// First compute new focal parameter which corresponds to photograph
// being rescaled by input scale_factor:

   double fnew=focal_param_corresponding_to_image_rescaling(
      n_vertical_pixels,fu_init,scale_factor);
//      cout << "f = " << f << " fnew = " << fnew << " fnew/f = "
//           << fnew/f << endl;

//   cout << "initial az = " << az_init*180/PI
//        << " initial el = " << el_init*180/PI
//        << " initial roll = " << roll_init*180/PI << endl;

   rotation R;
   R=R.rotation_from_az_el_roll(az_init,el_init,roll_init);

   double init_gamma;
   threevector n_hat;
   mathfunc::decompose_orthogonal_matrix(R,init_gamma,n_hat);

// Compute stretch factor by which all angles are multiplied when
// focal parameter is multiplied by input scale_factor:

   double numer=atan(0.5/(n_vertical_pixels*fnew));
   double denom=atan(0.5/(n_vertical_pixels*fu_init));
   double rho=numer/denom;
   double final_gamma=rho*init_gamma;

// Final relative rotation for pth photograph mosaic component has the
// same rotation axis as the initial relative rotation.  Only its
// rotation angle gamma about that axis changes when the camera's
// focal parameter f is varied:
      
   rotation Rfinal(n_hat,final_gamma);
   double az_final,el_final,roll_final;
   Rfinal.az_el_roll_from_rotation(az_final,el_final,roll_final);

//      cout << "final az = " << az_final*180/PI
//           << " final el = " << el_final*180/PI
//           << " final roll = " << roll_final*180/PI << endl;

   set_internal_params(fnew,fnew,get_u0(),get_v0());
   set_Rcamera(az_final,el_final,roll_final);
   construct_projection_matrix();
}

// ---------------------------------------------------------------------
// Member function save_initial_f_az_el_roll_params() and
// restore_initial_f_az_el_roll_params() store and restore initial
// focal and rotation angle parameters.  We wrote these little methods
// in Jan 2011 for panorama bundle adjustment purposes.

void camera::save_initial_f_az_el_roll_params()
{
//   cout << "inside camera::save_initial_f_az_el_roll_params()" << endl;
   fu_init=get_fu();
   fv_init=get_fv();
//   cout << "fu_init = " << fu_init << " fv_init = " << fv_init << endl;
   get_az_el_roll_from_Rcamera(az_init,el_init,roll_init);
}

void camera::restore_initial_f_az_el_roll_params()
{
//   cout << "inside camera::restore_initial_f_az_el_roll_params()" << endl;
//   cout << "fu_init = " << fu_init << " fv_init = " << fv_init << endl;
   set_internal_params(fu_init,fv_init,get_u0(),get_v0());
   set_Rcamera(az_init,el_init,roll_init);
   construct_projection_matrix();
}

// ---------------------------------------------------------------------
// Member function FOV_width_to_height_ratio() takes in a set of
// corner_ray unitvectors.  It computes and returns the
// width-to-height ratio for the image plane subtended by the input
// corner rays.  We wrote this little utility function in Mar 2010 in
// order to determine the ratio for subfrusta.  We empirically
// determined that the exact trigonometric value for this ratio
// returned by this method is well-approximated by the simpler ratio
// dU/dV = (U_hi-U_lo)/(V_hi-V_lo) which can be simply computed within
// the thin client.

double camera::FOV_width_to_height_ratio(
   const vector<threevector>& corner_rays)
{
   cout << "inside camera::FOV_width_to_height_ratio()" << endl;
   
   threevector e0_hat=corner_rays[0];
   threevector e1_hat=corner_rays[1];
   threevector e2_hat=corner_rays[2];
   
   double cos_2alpha=e1_hat.dot(e2_hat);
   double cos_2beta=e0_hat.dot(e1_hat);
   double sin_2alpha=(e1_hat.cross(e2_hat)).magnitude();
   double sin_2beta=(e0_hat.cross(e1_hat)).magnitude();
   
   double twoalpha=atan2(sin_2alpha,cos_2alpha);
   double twobeta=atan2(sin_2beta,cos_2beta);
   double alpha=0.5*twoalpha;
   double beta=0.5*twobeta;

   double width_to_height_ratio=fabs(sin(beta)/sin(alpha));
   return width_to_height_ratio;
}

// ==========================================================================
// Bundler to world coordinate transformation member functions
// ==========================================================================

// Member function get_R_noah_to_peter_ptr() returns the constant
// rotation matrix which converts from Noah's coord system to Peter's
// where world point clouds are aligned with Z_hat in the standard
// way.  If input georegistered_flag==true, the returned rotation
// equals the identity matrix.

rotation* camera::get_R_noah_to_peter_ptr(bool VSFM_flag)
{
   if (R_noah_to_peter_ptr==NULL)
   {
      R_noah_to_peter_ptr=new rotation();

      if (!VSFM_flag)
      {

// Bundler

         R_noah_to_peter_ptr->put(0,0,-1);
         R_noah_to_peter_ptr->put(0,1,0);
         R_noah_to_peter_ptr->put(0,2,0);

         R_noah_to_peter_ptr->put(1,0,0);
         R_noah_to_peter_ptr->put(1,1,0);
         R_noah_to_peter_ptr->put(1,2,1);
   
         R_noah_to_peter_ptr->put(2,0,0);
         R_noah_to_peter_ptr->put(2,1,1);
         R_noah_to_peter_ptr->put(2,2,0);
      }
      else
      {

// Visual Structure from Motion (VSFM):

// Note added on 5/16/12: We do NOT need to use the next 9 lines for
// VSFM output if global rotation, translation and scaling params from
// mains/modeling/PUSHCARTREG are incorporated into a new
// peter_inputs.pkg file.  Instead, we can just run
// mains/photosynth/BUNDLER_PHOTOS but with georegistered_flag set to
// true:

         R_noah_to_peter_ptr->put(0,0,1);
         R_noah_to_peter_ptr->put(0,1,0);
         R_noah_to_peter_ptr->put(0,2,0);

         R_noah_to_peter_ptr->put(1,0,0);
         R_noah_to_peter_ptr->put(1,1,0);
         R_noah_to_peter_ptr->put(1,2,1);
         
         R_noah_to_peter_ptr->put(2,0,0);
         R_noah_to_peter_ptr->put(2,1,-1);
         R_noah_to_peter_ptr->put(2,2,0);
      } // VSFM_flag conditional
  
   }

   if (georegistered_flag) R_noah_to_peter_ptr->identity();

//   cout << "*R_noah_to_peter_ptr = " << *R_noah_to_peter_ptr << endl;

   return R_noah_to_peter_ptr;
}

// ---------------------------------------------------------------------
// Member function convert_bundler_to_world_coords() takes in
// extrinsic rotation and translation camera parameters extracted from
// Noah Snavely's BUNDLER program output.  It also takes in a scale
// factor, global translation and global rotation which are used to
// convert Noah's relative camera parameters to absolute world
// coordinates.  This method resets the current camera's world
// position and az, el, roll member variables so that the camera can
// be placed directly into ladar maps in UTM world coordinates.

// As of Dec 2010, this method is DEPRECATED...

void camera::convert_bundler_to_world_coords(
   double x_trans,double y_trans,double z_trans,
   const threevector& bundler_rotation_origin,
   double global_az,double global_el,double global_roll,
   double global_scalefactor)
{
//   cout << "inside camera::convert_bundler_to_world_coords() #1" << endl;

   rotation R_noah(*R_camera_ptr);
//   cout << "R_noah = " << R_noah << endl;
   threevector photo_posn(get_world_posn());

// Convert from Noah's original coordinate system to Peter's world
// coordinates where Z represents height:

   photo_posn = (*get_R_noah_to_peter_ptr()) * photo_posn;
//   photo_posn=threevector(-photo_posn.get(0),photo_posn.get(2),
//                          photo_posn.get(1));

// Rescale from Noah's original coordinate system to stretch them to
// world coordinates:

   photo_posn *= global_scalefactor;

// Add global translation offset to bring photo's position to true
// world coordinates:
   
   photo_posn += threevector(x_trans,y_trans,z_trans);

   rotation global_R;
   global_R=global_R.rotation_from_az_el_roll(
      global_az,global_el,global_roll);
//      cout << "global_R = " << global_R << endl;

   threevector rel_photo_posn(photo_posn-bundler_rotation_origin);
   rel_photo_posn = global_R * rel_photo_posn;
   camera_world_posn=rel_photo_posn+bundler_rotation_origin;

//   cout << "camera_world_posn = " << camera_world_posn << endl;

// Transform camera's orientation from Noah's original coordinate
// system to Peter's world coordinates:

   rotation photo_R = (*get_R_noah_to_peter_ptr() * 
                       R_noah.transpose()).transpose();

// Perform global rotation to bring photo's orientation to true world
// coordinates:

   photo_R = global_R * photo_R.transpose();
   set_Rcamera(photo_R);

   double az,el,roll;
   get_az_el_roll_from_Rcamera(az,el,roll);

//   cout << "Camera az = " << az*180/PI << endl;
//   cout << "Camera el = " << el*180/PI << endl;
//   cout << "Camera roll = " << roll*180/PI << endl;
}

// ---------------------------------------------------------------------
// This overloaded version of convert_bundler_to_world_coords() takes
// in a rotation origin for Bundler points and cameras (e.g. COM from
// raw Bundler output).  After converting to Peter's world coordinates
// where Z represents height, this method translates the camera
// position so that it's centered about (0,0,0).  It then scales and
// rotates the camera according to the input parameters.  This
// method subsequently translates the camera to its final world
// position (e.g. trans = GPS_COM).  This method resets the current
// camera's world position and az, el, roll member variables so that
// the camera is absolutely georegistered.

void camera::convert_bundler_to_world_coords(
   const threevector& bundler_COM,
   double global_az,double global_el,double global_roll,
   double global_scalefactor,const threevector& world_COM)
{
//   cout << "inside camera::convert_bundler_to_world_coords() #2" << endl;
//   cout << "global_scalefactor = " << global_scalefactor << endl;
//   cout << "world_COM = " << world_COM << endl;
//   cout << "global_az = " << global_az*180/PI << endl;
//   cout << "global_el = " << global_el*180/PI << endl;
//   cout << "global_roll = " << global_roll*180/PI << endl;
//   cout << "bundler_COM = " << bundler_COM << endl;

   threevector photo_posn(get_world_posn());
//   cout << "Initial photo posn = " << photo_posn << endl;

// Convert from Noah's original coordinate system to Peter's world
// coordinates where Z represents height:

//   cout << "R_noah_to_peter = " << *(get_R_noah_to_peter_ptr()) << endl;

   photo_posn = (*get_R_noah_to_peter_ptr()) * photo_posn;
//   cout << "After converting from Noah's to Peter's coords, photo_posn = "
//        << endl;
//   cout << photo_posn << endl;

// First translate bundler's camera position so that it is 
// recentered about (0,0,0):

   photo_posn -= bundler_COM;
//   cout << "After subtracting off bundler_COM, photo_posn = " 
//        << photo_posn << endl;

// Rescale Noah's original coordinate system to stretch it to world
// coordinates:

   photo_posn *= global_scalefactor;
//   cout << "After scaling, photo_posn = " << photo_posn << endl;

// Rotate photo about bundler_rotation_origin:

   rotation global_R;
   global_R=global_R.rotation_from_az_el_roll(
      global_az,global_el,global_roll);
//   cout << "global_R = " << global_R << endl;

   photo_posn = global_R * photo_posn;
//   cout << "After rotating by global_R, photo_posn = " << photo_posn << endl;

// Add global translation offset to bring photo's position to true
// world coordinates:

   camera_world_posn=photo_posn+world_COM;
//   cout << "FINAL RESULT !!!" << endl;
//   cout << "camera_world_posn = " << camera_world_posn << endl;

// Transform camera's orientation from Noah's original coordinate
// system to Peter's world coordinates:

   rotation R_noah(*R_camera_ptr);

   rotation photo_R = (*get_R_noah_to_peter_ptr() * 
                       R_noah.transpose()).transpose();
//   cout << "After first multiplication and transpositions, photo_R = "
//        << photo_R << endl;

// Perform global rotation to bring photo's orientation to true world
// coordinates:

   photo_R = global_R * photo_R.transpose();
//   cout << "After multiplying by global_R, photo_R = " 
//        << photo_R << endl;

   set_Rcamera(photo_R);

   double az,el,roll;
   get_az_el_roll_from_Rcamera(az,el,roll);

//   cout << "FINAL RESULT !!!" << endl;
//   cout << "Camera az = " << az*180/PI << endl;
//   cout << "Camera el = " << el*180/PI << endl;
//   cout << "Camera roll = " << roll*180/PI << endl;
}

// ---------------------------------------------------------------------
// Member function convert_world_to_bundler_coords() works with the
// current extrinsic parameters contained in *this.  It transforms
// their values from world coordinates back into Noah's original
// coordinate system in which results are reported in bundle.out.

void camera::convert_world_to_bundler_coords(
   rotation& R_noah,threevector& t_noah)
{
//   cout << "inside camera::convert_world_to_bundler()" << endl;

   R_noah=*R0_ptr * (*R_camera_ptr) * (*get_R_noah_to_peter_ptr());
   rotation R_fudge;
   R_fudge.clear_values();
   R_fudge.put(0,2,-1);
   R_fudge.put(1,0,-1);
   R_fudge.put(2,1,1);

   R_noah = R_fudge.transpose() * R_noah;

   t_noah=-R_noah * get_R_noah_to_peter_ptr()->transpose() * 
      get_world_posn();
}

// ---------------------------------------------------------------------
// Member function write_camera_package_file() outputs the current
// camera's intrinsic and extrinsic parameters to a package file which
// can subsequently be read in to form an OBSFRUSTUM by program
// TESTCITIES, PANCITIES, etc.

void camera::write_camera_package_file(
   string package_filename,int photo_ID,string image_filename,
   double frustum_sidelength,double downrange_distance)
{
//   cout << "inside camera::write_camera_package_file()" << endl;
//   cout << "image_filename = " << image_filename << endl;
//   cout << "package_filename = " << package_filename << endl;
//   cout << "get_fu() = " << get_fu() << endl;

// First check that focal parameters are negative.  If not, do not
// write out package file:
   
   double SMALL_NEGATIVE=-1E-5;
   if (get_fu() > SMALL_NEGATIVE && get_fv() > SMALL_NEGATIVE)
   {
      cout << "Error in camera::write_camera_package_file()" << endl;
      cout << "Focal parameter fu = " << get_fu() << endl;
      cout << "Focal parameter fv = " << get_fv() << endl;
      cout << "These focal parameters should be negative..." << endl;
//      outputfunc::enter_continue_char();
      return;
   }

   ofstream outstream;
   filefunc::openfile(package_filename,outstream);

   outstream << image_filename << endl;
   outstream << "--photo_ID "+stringfunc::number_to_string(photo_ID)
             << endl;
   outstream << "--Uaxis_focal_length "+stringfunc::number_to_string(get_fu())
             << endl;
   outstream << "--Vaxis_focal_length "+stringfunc::number_to_string(get_fv())
             << endl;
   outstream << "--U0 "+stringfunc::number_to_string(get_u0()) << endl;
   outstream << "--V0 "+stringfunc::number_to_string(get_v0()) << endl;

   double az,el,roll;
   get_az_el_roll_from_Rcamera(az,el,roll);
   outstream << "--relative_az "+stringfunc::number_to_string(az*180/PI)
             << endl;
   outstream << "--relative_el "+stringfunc::number_to_string(el*180/PI)
             << endl;
   outstream << "--relative_roll "+stringfunc::number_to_string(roll*180/PI) 
             << endl;
   if (geolocation_ptr != NULL)
   {
      outstream << "--camera_longitude "+stringfunc::number_to_string(
         geolocation_ptr->get_longitude(),9) << endl;
      outstream << "--camera_latitude "+stringfunc::number_to_string(
         geolocation_ptr->get_latitude(),9) << endl;
   }
   outstream << "--camera_x_posn "+stringfunc::number_to_string(
      get_world_posn().get(0)) << endl;
   outstream << "--camera_y_posn "+stringfunc::number_to_string(
      get_world_posn().get(1)) << endl;
   outstream << "--camera_z_posn "+stringfunc::number_to_string(
      get_world_posn().get(2)) << endl;
   outstream << "--frustum_sidelength "+stringfunc::number_to_string(
      frustum_sidelength) << endl;
   if (imageplane_ptr != NULL)
   {
      fourvector pi(imageplane_ptr->get_pi());
      outstream << "--imageplane_x "+stringfunc::number_to_string(
         pi.get(0)) << endl;
      outstream << "--imageplane_y "+stringfunc::number_to_string(
         pi.get(1)) << endl;
      outstream << "--imageplane_z "+stringfunc::number_to_string(
         pi.get(2)) << endl;
      outstream << "--imageplane_w "+stringfunc::number_to_string(
         pi.get(3)) << endl;
   }
   else
   {
      if (downrange_distance > 0)
      {
         outstream << "--downrange_distance "+stringfunc::number_to_string(
            downrange_distance) << endl;
      }
   }

   filefunc::closefile(package_filename,outstream);
}

// ==========================================================================
// Ray tracing member functions
// ==========================================================================

// Member function dotproduct_between_rays takes in (U,V) coordinates
// for two imageplane points.  It computes the dot product between the
// two rays corresponding to these points.  Note that this dot product
// does not depend upon the camera's world position nor its absolute
// orientation.  So it can be calculated with knowledge of only
// *K_ptr.
 
double camera::dotproduct_between_rays(
   double u1,double v1,double u2,double v2)
{
   threevector qhat1_rel=((*Kinv_ptr) * threevector(u1,v1,1)).unitvector();
   threevector qhat2_rel=((*Kinv_ptr) * threevector(u2,v2,1)).unitvector();
   return qhat1_rel.dot(qhat2_rel);
}

// ---------------------------------------------------------------------
threevector camera::pixel_ray_direction(double u,double v) const
{
//   cout << "inside camera::pixel_ray_dir(), U = " << u
//        << " V = " << v << endl;
//   cout << "*Minv_ptr = " << *Minv_ptr << endl;
//   cout << "threevector(u,v,1) = " << threevector(u,v,1) << endl;
   
   threevector ray=((*Minv_ptr)*threevector(u,v,1)).unitvector();

// We do not know the overall sign for threevector ray which causes it
// to point in the camera's forward direction.  So check the dot
// product between ray and pointing_dir.  For any real-world digital
// camera, this dotproduct should be positive.  If it is not, switch
// the sign of ray:

   double dot_product=ray.dot(get_pointing_dir());
   if (dot_product < 0) ray=-ray;

//   cout << "ray = " << ray << endl;

   return ray;
}

threevector camera::pixel_ray_direction(const twovector& UV) const
{
//   cout << "inside camera::pixel_ray_dir(), UV = " << UV << endl;

   return pixel_ray_direction(UV.get(0),UV.get(1));
}

// ---------------------------------------------------------------------
// Member function pixel_bbox_ray_directions() fills and returns an
// STL vector with threevector rays corresponding to the input
// bounding box's corners.

vector<threevector> camera::pixel_bbox_ray_directions(
   const bounding_box& bbox)
{
   twovector bottom_left_corner(bbox.get_xmin(),bbox.get_ymin());
   twovector bottom_right_corner(bbox.get_xmax(),bbox.get_ymin());
   twovector top_right_corner(bbox.get_xmax(),bbox.get_ymax());
   twovector top_left_corner(bbox.get_xmin(),bbox.get_ymax());

   vector<threevector> UV_rays;
   UV_rays.push_back(pixel_ray_direction(bottom_left_corner));
   UV_rays.push_back(pixel_ray_direction(bottom_right_corner));
   UV_rays.push_back(pixel_ray_direction(top_right_corner));
   UV_rays.push_back(pixel_ray_direction(top_left_corner));
   return UV_rays;
}

// ==========================================================================
// Plane projection member functions
// ==========================================================================

// Member function backproject_imagepoint_to_zplane takes in the UV
// coordinates for some image point as well as the z value in 3-space.
// It projects a ray from the camera's center point through the image
// plane point onto the world Z=z plane.  The world coordinates of the
// ray's intercept with the Z=z plane are returned by this method.

threevector camera::backproject_imagepoint_to_zplane(
   double u,double v,double world_z)
{
   threevector UV_world_ray(pixel_ray_direction(u,v));
   double lambda=(world_z-camera_world_posn.get(2))/UV_world_ray.get(2);
   return camera_world_posn+lambda*UV_world_ray;
}

threevector camera::backproject_imagepoint_to_zplane(
   const twovector& UV,double world_z)
{
   threevector UV_world_ray=pixel_ray_direction(UV);
   double lambda=(world_z-camera_world_posn.get(2))/UV_world_ray.get(2);
   return camera_world_posn+lambda*UV_world_ray;
}
   
// ---------------------------------------------------------------------
// Member function principle_lookpoint_on_zplane returns the world
// location where the camera's principle axis intercepts the input z
// world-plane.

threevector camera::principle_lookpoint_on_zplane(double world_z)
{
   double lambda=(world_z-camera_world_posn.get(2))/(-What.get(2));
   return camera_world_posn+lambda*(-What);
}

// ---------------------------------------------------------------------
// Member function corner_ray_intercepts_with_zplane backprojects the
// four rays from the camera's center to the corners of its imageplane
// into world space.  This method returns the world locations for the
// projected corners in an STL vector.
 
vector<threevector> camera::corner_ray_intercepts_with_zplane(double world_z)
{
//   cout << "inside camera::corner_ray_intercepts_with_zplane()" << endl;
   vector<threevector> XYZ_corner;
   for (unsigned int i=0; i<UV_corner_world_ray.size(); i++)
   {
      XYZ_corner.push_back(backproject_imagepoint_to_zplane(
         UV_corner[i],world_z));
//      cout << "i = " << i << " XYZ corner = " << XYZ_corner.back() << endl;
   } // loop over index i labeling UV imageplane corners
   return XYZ_corner;
}

// ---------------------------------------------------------------------
// Member function homography_from_imageplane_to_zplane() fills *H_ptr
// with the homography that maps the current camera's UV image corners
// to corresponding XY points within the Z-plane defined by input
// parameter world_z.

homography* camera::homography_from_zplane_to_imageplane(double world_z)
{
//   cout << "inside camera::homography_from_zplane_to_imageplane()" << endl;

   vector<threevector> world_corner=
      corner_ray_intercepts_with_zplane(world_z);

   vector<twovector> XY_corner;
   for (unsigned int c=0; c<world_corner.size(); c++)
   {
      XY_corner.push_back(twovector(world_corner[c]));
//      cout << "c = " << c << " world corner = " << world_corner[c] << endl;
   }

// UV_corner refers to camera's imageplane corner coordinates.
// XY_corners refers to corresponding world-space points on z-plane:

   H_ptr->parse_homography_inputs(XY_corner,UV_corner);
   H_ptr->compute_homography_matrix();
   H_ptr->compute_homography_inverse();

//   double chisq=H_ptr->check_homography_matrix(UV_corner,XY_corner);
//   outputfunc::enter_continue_char();
   return H_ptr;
}

// ---------------------------------------------------------------------
homography* camera::homography_from_imageplane_to_zplane(double world_z)
{
//   cout << "inside camera::homography_from_imageplane_to_zplane()" << endl;

   vector<threevector> world_corner=
      corner_ray_intercepts_with_zplane(world_z);

   vector<twovector> XY_corner;
   for (unsigned int c=0; c<world_corner.size(); c++)
   {
      XY_corner.push_back(twovector(world_corner[c]));
//      cout << "c = " << c << " world corner = " << world_corner[c] << endl;
   }

// UV_corner refers to camera's imageplane corner coordinates.
// XY_corners refers to corresponding world-space points on z-plane:

   H_ptr->parse_homography_inputs(UV_corner,XY_corner);
   H_ptr->compute_homography_matrix();
   H_ptr->compute_homography_inverse();

//   double chisq=H_ptr->check_homography_matrix(UV_corner,XY_corner);
//   outputfunc::enter_continue_char();
   return H_ptr;
}

/*

// ---------------------------------------------------------------------
bool camera::backproject_imagepoint_to_worldplane(
   double u,double v,plane& world_plane,threevector& intersection_pt)
{
   threevector UV_world_ray(pixel_ray_direction(u,v));
   bool intersect_flag=world_plane.ray_intersection(
      get_world_posn(),UV_world_ray,intersection_pt);
   return intersect_flag;
}

bool camera::backproject_imagepoint_to_worldplane(
   const twovector& UV,plane& world_plane,threevector& intersection_pt)
{
   return backproject_imagepoint_to_worldplane(
      UV.get(0),UV.get(1),world_plane,intersection_pt);
}

*/

// ---------------------------------------------------------------------
// Member function backproject_into_world_plane() takes in a set of UV
// coordinates for some 2D feature within the camera's image plane.
// It also takes in world-plane P.  This method computes the 3D
// intersection of the ray with P and returns the result within
// intersection_pnt.  If the ray does not intersect the plane, this
// boolean method returns false.
 
bool camera::backproject_into_world_plane(
   double u,double v,const plane& P,threevector& intersection_pnt) const
{
//   cout << "inside camera::backproject_into_world_plane()" << endl;
//   cout << "u = " << u << " v = " << v << endl;

   if (P.ray_intersection(
      get_world_posn(),pixel_ray_direction(u,v),intersection_pnt))
   {
//      cout << "intersection_pnt = " << intersection_pnt << endl;
      return true;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
// Member function backproject_corner_into_world_plane() takes in
// corner index c=0,1,2,3.  It returns within intersection_pnt the
// projection of the world ray corresponding to the selected corner
// with the specified plane P.  If the ray doesn't intersect the
// plane, this boolean method returns false.

bool camera::backproject_corner_into_world_plane(
   int c,const plane& P,threevector& intersection_pnt)
{
   twovector UV=UV_corner[c];
   return backproject_into_world_plane(
      UV.get(0),UV.get(1),P,intersection_pnt);
}

bool camera::backproject_corner_into_world_plane(
   int c,const plane& P,twovector& corner_AB)
{
   threevector intersection_pnt;
   bool intersect_flag=backproject_corner_into_world_plane(
      c,P,intersection_pnt);
   if (intersect_flag)
   {
      P.planar_coords(intersection_pnt,corner_AB);
   }
   return intersect_flag;
}

// ---------------------------------------------------------------------
// Member function homography_from_worldplane_to_imageplane() fills *H_ptr
// with the homography that maps points within the input world plane
// to the current camera's UV image corners.

homography* camera::homography_from_worldplane_to_imageplane(plane& P)
{
//   cout << "inside camera::homography_from_worldplane_to_imageplane()" << endl;

   twovector curr_AB;
   threevector intersection_pnt;
   vector<twovector> AB_corner;
   for (unsigned int c=0; c<UV_corner.size(); c++)
   {
      backproject_corner_into_world_plane(c,P,intersection_pnt);
      P.planar_coords(intersection_pnt,curr_AB);
      AB_corner.push_back(curr_AB);
   }

// UV_corner refers to camera's imageplane corner coordinates.
// AB_corner refers to corresponding world-space points in world plane P:

   H_ptr->parse_homography_inputs(AB_corner,UV_corner);
   H_ptr->compute_homography_matrix();
   H_ptr->compute_homography_inverse();

//   double chisq=H_ptr->check_homography_matrix(UV_corner,XY_corner);
//   outputfunc::enter_continue_char();
   return H_ptr;
}


// ==========================================================================
// DTED ray tracing member functions
// ==========================================================================

// Member function trace_individual_ray() takes in a ray's direction
// vector e_hat, minimum ground height min_Z_ground, maximum raytrace
// range max_raytrace_range and ray trace step size ds.  If the ray
// intercepts the ground or flies underneath some local patch of
// *DTED_ztwoDarray_ptr, we assume the ray is occluded at its impact
// point and this boolean method returns true.  Otherwise, if the ray
// reaches max_raytrace_range without being occluded, this method
// returns false.

bool camera::trace_individual_ray(
   const threevector& e_hat,double min_Z_ground,
   double max_raytrace_range,double ds,
   twoDarray* DTED_ztwoDarray_ptr,threevector& impact_point)
{
   return trace_individual_ray(
      e_hat,min_Z_ground,max_raytrace_range,ds,1.0,
      DTED_ztwoDarray_ptr,NULL,impact_point);
}

bool camera::trace_individual_ray(
   const threevector& e_hat,double min_Z_ground,
   double max_raytrace_range,double ds,double scale_factor,
   twoDarray* DTED_ztwoDarray_ptr,twoDarray* reduced_DTED_ztwoDarray_ptr,
   threevector& impact_point)
{
//   cout << "inside camera::trace_individual_ray() #1" << endl;
            
   double d_length = ds/sqrt(1-sqrt(sqr(e_hat.get(2))));
   threevector d_length_e_hat(d_length*e_hat);
   unsigned int n_steps=static_cast<int>(max_raytrace_range/d_length);

// Start ray tracing at OBSFRUSTUM's apex:

   double x=get_world_posn().get(0);
   double y=get_world_posn().get(1);
   double z=get_world_posn().get(2);
   unsigned int i_start=0;

// Take large steps along ray to determine if it's occluded anywhere.
// If not, don't waste time evaluating ray occlusion along finer steps:

   if (reduced_DTED_ztwoDarray_ptr != NULL)
   {
      threevector scaled_d_length_e_hat(scale_factor*d_length_e_hat);
      bool ray_occluded_flag=false;
      for (unsigned int i=scale_factor; i<n_steps && !ray_occluded_flag; 
           i += scale_factor)
      {
         if (z < min_Z_ground)
         {
            ray_occluded_flag=true;
         }

         unsigned int px,py;
         if (reduced_DTED_ztwoDarray_ptr->point_to_pixel(x,y,px,py))
         {
            double curr_ground_z=reduced_DTED_ztwoDarray_ptr->get(px,py);
//      cout << "curr_ground_z = " << curr_ground_z << endl;
            if (z < curr_ground_z)
            {
               ray_occluded_flag=true;
            }
         }

         if (ray_occluded_flag) break;

         x += scaled_d_length_e_hat.get(0);
         y += scaled_d_length_e_hat.get(1);
         z += scaled_d_length_e_hat.get(2);
      } // loop over index i labeling steps along ray

      if (!ray_occluded_flag) 
      {
//         cout << "Ray not occluded" << endl;
         return false;
      }

      i_start=basic_math::round(
         (threevector(x,y,z)-get_world_posn()).magnitude()/d_length)-1;
      i_start=basic_math::max(Unsigned_Zero,i_start);
   } // reduced_DTED_ztwoDarray_ptr != NULL conditional
   
//   cout << "i_start = " << i_start << endl;
   for (unsigned int i=i_start; i<n_steps; i++)
   {
//      cout << "i = " << i 
//           << " x = " << x << " y = " << y << " z = " << z << endl;
      
      if (z < min_Z_ground)
      {
         impact_point=threevector(x,y,z);
//         cout << "impact_point lies below ground!" << endl;
//         cout << "i = " << i << endl;
//         cout << "impact_point = " << impact_point << endl;
         return true;
      }

      unsigned int px,py;
      if (DTED_ztwoDarray_ptr->point_to_pixel(x,y,px,py))
      {
         double curr_ground_z=DTED_ztwoDarray_ptr->get(px,py);
//      cout << "curr_ground_z = " << curr_ground_z << endl;
         if (z < curr_ground_z)
         {
            impact_point=threevector(x,y,z);
//            cout << "i = " << i << endl;
//            cout << "curr_ground_z = " << curr_ground_z << endl;
//            cout << "Occluded impact_point = " << impact_point << endl;
            return true;
         }
      }
      
      x += d_length_e_hat.get(0);
      y += d_length_e_hat.get(1);
      z += d_length_e_hat.get(2);

   } // loop over index i labeling steps along ray

   return false;
}

// ---------------------------------------------------------------------
// This overloaded version of trace_individual_ray() takes an input
// ground target point as the starting location for a ray and traces
// it backwards towards the camera's world position.  If the ray ever
// flies underneath a local ground patch, we assume the ground target
// point is occluded.  This boolean method returns true if the target
// point is occluded.

bool camera::trace_individual_ray(
   const threevector& starting_ray_posn,const threevector& e_hat,
   double min_Z_ground,double ds,twoDarray* DTED_ztwoDarray_ptr,
   threevector& impact_point)
{
//   cout << "inside camera::trace_individual_ray() #2" << endl;
//   cout << "e_hat = " << e_hat << endl;
//   cout << "DTED_ztwoDarray_ptr = " << DTED_ztwoDarray_ptr << endl;
            
   double length=(starting_ray_posn-get_world_posn()).magnitude();
   double d_length = ds/sqrt(1-sqrt(sqr(e_hat.get(2))));
   int n_steps=static_cast<int>(length/d_length);
   threevector d_length_e_hat(d_length*e_hat);

// Perform ray tracing a few displacements away from ray's starting
// position in order to compensate for errors in DTED height map:

   int i_start=5;
//   int i_start=10;
   threevector curr_ray_posn=starting_ray_posn+i_start*d_length_e_hat;
   double x=curr_ray_posn.get(0);
   double y=curr_ray_posn.get(1);
   double z=curr_ray_posn.get(2);

   for (int i=i_start; i<n_steps; i++)
   {
//      cout << "i = " << i 
//           << " x = " << x << " y = " << y << " z = " << z << endl;
      
// In order to not occlude features selected on ocean water which may
// be a little bit below sea-level, subtract some small
// ground_tolerance away from min_Z_ground before declaring a ground
// occlusion to have occurred:

      const double ground_tolerance=1.0;       // meter
      if (z < min_Z_ground-ground_tolerance)
      {
         impact_point=threevector(x,y,z);
//         cout << "z = " << z << " impact_point = " << impact_point << endl;
         return true;
      }

      unsigned int px,py;
      if (DTED_ztwoDarray_ptr->point_to_pixel(x,y,px,py))
      {
         double curr_ground_z=DTED_ztwoDarray_ptr->get(px,py);
         if (z < curr_ground_z)
         {
            impact_point=threevector(x,y,z);
//            cout << "i = " << i << " n_steps = " << n_steps 
//                 << " ray occluded" << endl;
//            cout << "curr_ground_z = " << curr_ground_z << endl;
//            cout << "camera_posn = " << get_world_posn() << endl;
//            cout << "impact_point = " << impact_point << endl;
            return true;
         }
      } // x,y lies inside *DTED_ztwoDarray_ptr conditional
      
      x += d_length_e_hat.get(0);
      y += d_length_e_hat.get(1);
      z += d_length_e_hat.get(2);

   } // loop over index i labeling steps along ray

//   cout << "ray NOT occluded" << endl;
   return false;
}

// ---------------------------------------------------------------------
// Member function target_downrange_distance() takes in a set of
// corner_ray unitvectors.  It computes and returns the
// width-to-height ratio for the image plane subtended by the input
// corner rays.  We wrote this little utility function in Mar 2010 in
// order to determine the ratio for subfrusta.  We empirically
// determined that the exact trigonometric value for this ratio
// returned by this method is well-approximated by the simpler ratio
// dU/dV = (U_hi-U_lo)/(V_hi-V_lo) which can be simply computed within
// the thin client.

void camera::target_downrange_distance(
   const vector<threevector>& corner_rays,double bbox_height,
   double& pyramid_sidelength,double& downrange_distance)
{
   cout << "inside camera::target_downrange_distance()" << endl;
   
   threevector e0_hat=corner_rays[0];
   threevector e1_hat=corner_rays[1];
   threevector e2_hat=corner_rays[2];
   
   double cos_2alpha=e1_hat.dot(e2_hat);
//   double cos_2beta=e0_hat.dot(e1_hat);
   double sin_2alpha=(e1_hat.cross(e2_hat)).magnitude();
//   double sin_2beta=(e0_hat.cross(e1_hat)).magnitude();
   
   double twoalpha=atan2(sin_2alpha,cos_2alpha);
//   double twobeta=atan2(sin_2beta,cos_2beta);
   double alpha=0.5*twoalpha;
//   double beta=0.5*twobeta;

   pyramid_sidelength=0.5*bbox_height/sin(alpha);
   cout << "pyramid_sidelength = " << pyramid_sidelength << endl;
   downrange_distance=compute_movie_downrange_distance(pyramid_sidelength);
   cout << "downrange_distance = " << downrange_distance << endl;
}

