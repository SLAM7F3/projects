// ==========================================================================
// ObsFrustum class member function definitions
// ==========================================================================
// Last updated on 2/17/08; 10/29/08; 12/4/10
// ==========================================================================

#include <string>
#include "video/camera.h"
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "math/mathfuncs.h"
#include "osg/osgModels/ModelsGroup.h"
#include "osg/osg2D/Movie.h"
#include "osg/osgModels/ObsFrustum.h"
#include "geometry/polygon.h"
#include "math/rotation.h"

#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ObsFrustum::allocate_member_objects()
{
   polyhedron_ptr=new polyhedron();
}		       

void ObsFrustum::initialize_member_objects()
{
   Graphical_name="ObsFrustum";
   rectangular_movie_flag=true;
   virtual_camera_flag=false;
   display_camera_model_flag=false;
   LineSegmentsGroup_ptr=NULL;
   ModelsGroup_ptr=NULL;
   camera_model_ptr=NULL;
   
   set_permanent_color(colorfunc::white);
}		       

// Angle ranges az_extent and el_extent must be specified in radians.

ObsFrustum::ObsFrustum(
   Pass* PI_ptr,double az_extent,double el_extent,
   const threevector& grid_world_origin,
   AnimationController* AC_ptr,Movie* Movie_ptr,int id):
   Geometrical(3,id,AC_ptr)
{	
   this->Movie_ptr=Movie_ptr;
   initialize_member_objects();
   allocate_member_objects();

   this->az_extent=az_extent;
   this->el_extent=el_extent;
   this->grid_world_origin=grid_world_origin;

   LineSegmentsGroup_ptr=new LineSegmentsGroup(
      3,PI_ptr,AnimationController_ptr);
   initialize_linesegments();
}		       

ObsFrustum::ObsFrustum(
   Pass* PI_ptr,const threevector& grid_world_origin,
   AnimationController* AC_ptr,Movie* Movie_ptr,int id):
   Geometrical(3,id,AC_ptr)
{	
   this->Movie_ptr=Movie_ptr;
   initialize_member_objects();
   allocate_member_objects();
   this->grid_world_origin=grid_world_origin;

   LineSegmentsGroup_ptr=new LineSegmentsGroup(
      3,PI_ptr,AnimationController_ptr);
   initialize_linesegments();

   ModelsGroup_ptr=new ModelsGroup(PI_ptr,const_cast<threevector*>(
      &grid_world_origin),AnimationController_ptr);

   string model_filename="Camera.osg";
   camera_model_ptr=ModelsGroup_ptr->generate_new_Model(model_filename);
   ModelsGroup_ptr->initialize_const_posn(Zero_vector,camera_model_ptr);
}		       

ObsFrustum::~ObsFrustum()
{
   LineSegmentsGroup_ptr->destroy_all_Graphicals();
   delete LineSegmentsGroup_ptr;
   delete polyhedron_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const ObsFrustum& f)
{
   outstream << "inside ObsFrustum::operator<<" << endl;
   outstream << static_cast<const Geometrical&>(f) << endl;
   return(outstream);
}

// ==========================================================================
// Frustum construction and manipulation methods
// ==========================================================================

// Member function initialize_linesegments instantiates 12 canonical
// linesegments of unit length oriented along the +x_hat direction.
// The first 4 line segments correspond to rays propagating outward
// from the camera vertex.  The next 4 line segments correspond to
// ground [or aerial] footprint connectors.  The final 4 line segments
// are reserved for potential ground-to-air connectors.

void ObsFrustum::initialize_linesegments()
{
   LineSegmentsGroup_ptr->generate_canonical_segments(12);
   set_color(get_permanent_color());
}

void ObsFrustum::set_color(const osg::Vec4& color)
{
   set_permanent_color(color);

   for (unsigned int i=0; i<LineSegmentsGroup_ptr->get_n_Graphicals(); i++)
   {
      LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(i);
      curr_segment_ptr->set_permanent_color(color);
      curr_segment_ptr->get_LineWidth_ptr()->setWidth(2.0);
      if (i>=4 && i <8) curr_segment_ptr->get_LineWidth_ptr()->setWidth(6.0);
   }
   LineSegmentsGroup_ptr->reset_colors();
}

// ---------------------------------------------------------------------
// Member function build_current_frustum generates an ObsFrustum at
// time t whose camera position lies at input threevector V and whose
// symmetry axis is aligned with input threevector n_hat. The
// footprint corners of the frustum are assumed to lie a distance
// z_offset above the world z-plane grid.  The rotations, translations
// and scalings needed to transform canonical unit-length linesegments
// into the frustum's sides are stored for later callback retrieval.

void ObsFrustum::build_current_frustum(
   double curr_t,int pass_number,const threevector& V,
   const threevector& n_hat,double z_offset)
{
//   cout << "inside ObsFrustum::build_current_frustum(V,nhat)" << endl;

   set_UVW_coords(curr_t,pass_number,V);

   double phi,theta;
   mathfunc::decompose_direction_vector(n_hat,phi,theta);

   corner_ray.clear();
   if (theta+0.5*el_extent > PI/2)
   {
      corner_ray.push_back(mathfunc::construct_direction_vector(
         phi-0.5*az_extent,theta+0.5*el_extent));
      corner_ray.push_back(mathfunc::construct_direction_vector(
         phi+0.5*az_extent,theta+0.5*el_extent));
   }
   else
   {
      corner_ray.push_back(mathfunc::construct_direction_vector(
         phi+0.5*az_extent,theta+0.5*el_extent));
      corner_ray.push_back(mathfunc::construct_direction_vector(
         phi-0.5*az_extent,theta+0.5*el_extent));
   }
   
   if (theta-0.5*el_extent < -PI/2)
   {
      corner_ray.push_back(mathfunc::construct_direction_vector(
         phi+0.5*az_extent,theta-0.5*el_extent));
      corner_ray.push_back(mathfunc::construct_direction_vector(
         phi-0.5*az_extent,theta-0.5*el_extent));
   }
   else
   {
      corner_ray.push_back(mathfunc::construct_direction_vector(
         phi-0.5*az_extent,theta-0.5*el_extent));
      corner_ray.push_back(mathfunc::construct_direction_vector(
         phi+0.5*az_extent,theta-0.5*el_extent));
   }

// LineSegment coordinates are specified relative to camera position V:

   footprint_corner.clear();
   for (int i=0; i<4; i++)
   {
      double lambda=(grid_world_origin.get(2)-V.get(2))/
         corner_ray[i].get(2);
      footprint_corner.push_back(V+lambda*corner_ray[i]+z_offset*z_hat);
      LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(i);
      curr_segment_ptr->set_scale_attitude_posn(
         curr_t,pass_number,Zero_vector,footprint_corner.back()-V);
   }

// Manipulate linesegments 4-7 to hold observation frustum's footprint
// which generally corresponds to a trapezoid along the world z-plane.
// Copy linesegments 8-11 equal to linesegments 4-7.

   for (int i=4; i<8; i++)
   {
      threevector V1=footprint_corner[i-4];
      threevector V2=footprint_corner[modulo(i-4+1,4)];
      LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(i);
      curr_segment_ptr->set_scale_attitude_posn(
         curr_t,pass_number,V1-V,V2-V);
      LineSegment* next_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(i+4);
      next_segment_ptr->set_scale_attitude_posn(
         curr_t,pass_number,V1-V,V2-V);
   }
}

// ---------------------------------------------------------------------
void ObsFrustum::build_current_frustum(
   double curr_t,int pass_number,double z_offset,
   const threevector& camera_posn,const vector<threevector>& UV_corner_dir,
   double max_lambda)
{
//   cout << "inside ObsFrustum::build_current_frustum(UV_corner_dir)" << endl;
//   cout << "camera_posn = " << camera_posn << endl;
//   for (unsigned int c=0; c<UV_corner_dir.size(); c++)
//   {
//      cout << " UV_corner_dir.push_back(threevector(" << endl;
//      cout << UV_corner_dir[c] << endl;
//      cout << "));" << endl;
//   }

   set_UVW_coords(curr_t,pass_number,camera_posn);

   const double default_lambda=3000;	// meters
   vector<int> aerial_corner_index;
   vector<double> lambda;

   for (int i=0; i<4; i++)
   {
      lambda.push_back((z_offset-camera_posn.get(2))/UV_corner_dir[i].get(2));
      
//      cout << "i = " << i << " lambda = " << lambda.back() << endl;
//      cout << "UV_corner_dir.z = " << UV_corner_dir[i].get(2) << endl;
      if (lambda.back() < 0 || lambda.back() > max_lambda)
      {
         aerial_corner_index.push_back(i);
         lambda[i]=basic_math::min(default_lambda,max_lambda);
      }
//      cout << "i = " << i << " final lambda = " << lambda[i] << endl;
   }

// LineSegment coordinates are specified relative to camera position:

   aerial_corner.clear();
   footprint_corner.clear();

//   cout << "aerial_corner_index.size() = "
//        << aerial_corner_index.size() << endl;
   
//   if (is_odd(aerial_corner_index.size()))
   if (aerial_corner_index.size()==3)
   {
      cout << "Error in ObsFrustum::build_current_frustum()" << endl;
      cout << "aerial_corner_index.size() = "
           << aerial_corner_index.size() << endl;
      exit(-1);
   }

   if (aerial_corner_index.size()==0 || aerial_corner_index.size()==4)
   {
      for (int i=0; i<4; i++)
      {
         footprint_corner.push_back(camera_posn+lambda[i]*UV_corner_dir[i]);
         LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
            get_LineSegment_ptr(i);
         curr_segment_ptr->set_scale_attitude_posn(
            curr_t,pass_number,Zero_vector,
            footprint_corner.back()-camera_posn);
      }

// Manipulate linesegments 4-7 to hold observation frustum's
// footprint.  It generally corresponds to a trapezoid parallel to the
// world z-plane [floating in the air] if n_upward_pointing_corners ==
// 0 [4].  Copy linesegments 4-7 onto linesegments 8-11.

      for (int i=4; i<8; i++)
      {
         threevector V1=footprint_corner[i-4];
         threevector V2=footprint_corner[modulo(i-4+1,4)];
         LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
            get_LineSegment_ptr(i);
         curr_segment_ptr->set_scale_attitude_posn(
            curr_t,pass_number,V1-camera_posn,V2-camera_posn);
         LineSegment* next_segment_ptr=LineSegmentsGroup_ptr->
            get_LineSegment_ptr(i+4);
         next_segment_ptr->set_scale_attitude_posn(
            curr_t,pass_number,V1-camera_posn,V2-camera_posn);
      }
   }
   else if (aerial_corner_index.size()==2)
   {

// First loop over rays emanating from camera's posn:

      for (int i=0; i<4; i++)
      {

// Check whether current index i corresponds to an aerial corner
// index:

         bool aerial_corner_flag=false;
         for (unsigned int j=0; j<aerial_corner_index.size(); j++)
         {
            if (i==aerial_corner_index[j])
            {
               aerial_corner_flag=true;
            }
         }
         
         threevector curr_corner=camera_posn+lambda[i]*UV_corner_dir[i];
         if (aerial_corner_flag)
         {
            aerial_corner.push_back(curr_corner);
         }
         else
         {
            footprint_corner.push_back(curr_corner);
         }

         LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
            get_LineSegment_ptr(i);
         curr_segment_ptr->set_scale_attitude_posn(
            curr_t,pass_number,Zero_vector,curr_corner-camera_posn);
      } // loop over index i labeling rays emanating from camera posn
      
// Next complete aerial and footprint corners by projecting aerial
// corners into world z-plane:

      threevector corner1=aerial_corner[1];
      corner1.put(2,z_offset);
      aerial_corner.push_back(corner1);
      threevector corner0=aerial_corner[0];
      corner0.put(2,z_offset);
      aerial_corner.push_back(corner0);
//      cout << "corner1 = " << corner1 << endl;
//      cout << "corner0 = " << corner0 << endl;
      footprint_corner.push_back(corner0);
      footprint_corner.push_back(corner1);

/*
// For debugging purposes, compute coords of aerial and footprint
// corners relative to their avg locations:

      threevector aerial_avg,footprint_avg;
      for (int a=0; a<4; a++)
      {
         aerial_avg += 0.25*aerial_corner[a];
         footprint_avg += 0.25*footprint_corner[a];
      }

      for (int a=0; a<4; a++)
      {
         cout << "a = " << a << " aerial_corner[a]-aerial_avg = "
              << aerial_corner[a]-aerial_avg << endl;
      }

      for (int a=0; a<4; a++)
      {
         cout << "a = " << a << " footprint_corner[a]-footprint_avg = "
              << footprint_corner[a]-footprint_avg << endl;
      }
*/
  
// Manipulate linesegments 4-7 to hold observation frustum's
// footprint.  It generally corresponds to a trapezoid parallel to the
// world z-plane.  

      for (int i=4; i<8; i++)
      {
         threevector V1=footprint_corner[i-4];
         threevector V2=footprint_corner[modulo(i-4+1,4)];
         LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
            get_LineSegment_ptr(i);
         curr_segment_ptr->set_scale_attitude_posn(
            curr_t,pass_number,V1-camera_posn,V2-camera_posn);
      }
      
// Manipulate linesegments 8-11 to hold rectangle which is orthogonal
// to world z-plane:

      for (int i=8; i<12; i++)
      {
         threevector V1=aerial_corner[i-8];
         threevector V2=aerial_corner[modulo(i-8+1,4)];
         LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
            get_LineSegment_ptr(i);
         curr_segment_ptr->set_scale_attitude_posn(
            curr_t,pass_number,V1-camera_posn,V2-camera_posn);
      }
   }
   
   else if (aerial_corner_index.size()==1)
   {

// First loop over rays emanating from camera's posn:

      for (int i=0; i<4; i++)
      {

// Check whether current index i corresponds to an aerial corner
// index:

         bool aerial_corner_flag=false;
         for (unsigned int j=0; j<aerial_corner_index.size(); j++)
         {
            if (i==aerial_corner_index[j])
            {
               aerial_corner_flag=true;
            }
         }
         
         threevector curr_corner=camera_posn+lambda[i]*UV_corner_dir[i];
         if (aerial_corner_flag)
         {
            aerial_corner.push_back(curr_corner);
         }
         else
         {
            footprint_corner.push_back(curr_corner);
         }

         LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
            get_LineSegment_ptr(i);
         curr_segment_ptr->set_scale_attitude_posn(
            curr_t,pass_number,Zero_vector,curr_corner-camera_posn);
      } // loop over index i labeling rays emanating from camera posn
      
// Complete aerial and footprint corners by projecting aerial corners
// into world z-plane:

      cout << "Aerial_corner_index.size() = " 
           << aerial_corner_index.size() << endl;
      cout << "aerial_corner[0] = " << aerial_corner[0] << endl;
      vector<threevector> overlapping_corner;
      for (unsigned int a=0; a<aerial_corner_index.size(); a++)
      {
         overlapping_corner.push_back(
            aerial_corner[aerial_corner_index.size()-1-a]);
         overlapping_corner.back().put(2,z_offset);
         cout << "a = " << a 
              << " overlapping_corner.back() = "
              << overlapping_corner.back() << endl;
//         aerial_corner.push_back(overlapping_corner.back());
      }
      
      for (unsigned int a=0; a<aerial_corner_index.size(); a++)
      {
         footprint_corner.push_back(overlapping_corner[a]);
      }

// For debugging purposes, compute coords of aerial and footprint
// corners relative to their avg locations:

      threevector aerial_avg,footprint_avg;
      for (int a=0; a<4; a++)
      {
//         aerial_avg += 0.25*aerial_corner[a];
         footprint_avg += 0.25*footprint_corner[a];
      }

//      for (int a=0; a<4; a++)
//      {
//         cout << "a = " << a << " aerial_corner[a]-aerial_avg = "
//              << aerial_corner[a]-aerial_avg << endl;
//      }

      for (int a=0; a<4; a++)
      {
         cout << "a = " << a << " footprint_corner[a]-footprint_avg = "
              << footprint_corner[a]-footprint_avg << endl;
      }
  
// Manipulate linesegments 4-7 to hold observation frustum's
// footprint.  It generally corresponds to a trapezoid parallel to the
// world z-plane.  

      for (int i=4; i<8; i++)
      {
         threevector V1=footprint_corner[i-4];
         threevector V2=footprint_corner[modulo(i-4+1,4)];
         LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
            get_LineSegment_ptr(i);
         curr_segment_ptr->set_scale_attitude_posn(
            curr_t,pass_number,V1-camera_posn,V2-camera_posn);
      }
      
// Manipulate linesegments 8-11 to hold triangle which is orthogonal
// to world z-plane:

      for (int i=8; i<12; i++)
      {

// FAKE FAKE: for alg develop only we hardwire in footprint corners
// again as of 7:37 am on Weds, June 20:

         threevector V1=footprint_corner[i-8];
         threevector V2=footprint_corner[modulo(i-8+1,4)];
//         threevector V1=aerial_corner[i-8];
//         threevector V2=aerial_corner[modulo(i-8+1,4)];
         LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
            get_LineSegment_ptr(i);
         curr_segment_ptr->set_scale_attitude_posn(
            curr_t,pass_number,V1-camera_posn,V2-camera_posn);
      }

   }  // aerial_corner_index.size()  conditional

// Set node mask for OSGsubPAT containing camera model based upon
// display_camera_model_flag value:

   if (ModelsGroup_ptr != NULL)
      ModelsGroup_ptr->get_OSGsubPAT_ptr(0)->setNodeMask(
         display_camera_model_flag);
}

// ---------------------------------------------------------------------
void ObsFrustum::build_frustum_with_movie(
   double curr_t,int pass_number,double z_offset,
   double movie_downrange_distance,double max_lambda)
{
//   cout << "inside ObsFrustum::build_frustum_with_movie()" << endl;
//   cout << "movie_downrange_dist = " << movie_downrange_distance
//        << " max_lambda = " << max_lambda << endl;
   
   camera* curr_camera_ptr=Movie_ptr->get_camera_ptr();

   build_current_frustum(
      curr_t,pass_number,z_offset,
      curr_camera_ptr->get_world_posn(),
      curr_camera_ptr->get_UV_corner_world_ray(),max_lambda);

// Scale, rotate and translate movie window so that it lies inside
// ObsFrustum.  Recall that we insert the Movie_ptr->get_PAT_ptr()
// into ObsFrustum_ptr->get_PAT_ptr() within
// ObsFrustaGroup::generate_movie_Frustum().  So when we translate and
// rotate the ObsFrustum, *Movie_ptr is automatically transformed as
// well provided we pass its relative posn = Zero_vector into the
// following Movie method:

   Movie_ptr->remap_window_corners(
      movie_downrange_distance,Zero_vector,
      -Movie_ptr->get_camera_ptr()->get_What(),
      Movie_ptr->get_camera_ptr()->get_UV_corner_world_ray());
}

// ==========================================================================
// Translation and rotaton member functions
// ==========================================================================

void ObsFrustum::absolute_position_and_orientation(
   double curr_t,int pass_number,
   const threevector& absolute_position,double az,double el,double roll,
   bool recompute_internal_params_flag)
{
//   cout << "inside ObsFrustum::absolute_position_and_orientation()" << endl;
   rotate_about_camera_posn(curr_t,pass_number,az,el,roll,
                            recompute_internal_params_flag);
   absolute_posn(curr_t,pass_number,absolute_position);

   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   camera_ptr->set_world_posn(absolute_position);
   camera_ptr->construct_projection_matrix(recompute_internal_params_flag);
}

// ---------------------------------------------------------------------
// Member function rotate_about_camera_posn takes in azimuth,
// elevation and roll angles measured in radians about +z_hat, -y_hat'
// and +x_hat''.  

void ObsFrustum::rotate_about_camera_posn(
   double curr_t,int pass_number,double az,double el,double roll,
   bool recompute_internal_params_flag)
{
//   cout << "inside ObsFrustum::rotate_about_camera_posn()" << endl;
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   camera_ptr->set_Rcamera(az,el,roll);
   rotation R=camera_ptr->get_Rcamera_ptr()->transpose();

   threevector xhat_p,yhat_p,zhat_p;
   R.get_column(0,xhat_p);
   R.get_column(1,yhat_p);
   R.get_column(2,zhat_p);

   rotate_about_specified_origin(
      curr_t,pass_number,Zero_vector,xhat_p,yhat_p);
   camera_ptr->construct_projection_matrix(recompute_internal_params_flag);
}

// ==========================================================================
// Footprint member functions
// ==========================================================================

// Member function GMTI_dwell_frustum_footprint generates an
// ObsFrustum at time t whose camera position lies at input
// threevector V and whose footprint on the ground has a specified
// range and crossrange extent.  The footprint forms a rectangle which
// is located a distance z_offset above the world z-plane grid.  The
// rotations, translations and scalings needed to transform canonical
// unit-length linesegments into the frustum's sides are stored for
// later callback retrieval.

polygon ObsFrustum::GMTI_dwell_frustum_footprint(
   double curr_t,int pass_number,const threevector& V,const threevector& G,
   double range_extent,double crossrange_extent,double z_offset)
{
   set_UVW_coords(curr_t,pass_number,V);

   threevector rhat=(G-V).unitvector();
   threevector rho=rhat-(rhat.get(2)*z_hat);
   threevector rho_hat=rho.unitvector();
   threevector kappa_hat=rho_hat.cross(z_hat);
   
   footprint_corner.clear();
   footprint_corner.push_back(
      G+0.5*crossrange_extent*kappa_hat+0.5*range_extent*rho_hat);
   footprint_corner.push_back(
      G-0.5*crossrange_extent*kappa_hat+0.5*range_extent*rho_hat);
   footprint_corner.push_back(
      G-0.5*crossrange_extent*kappa_hat-0.5*range_extent*rho_hat);
   footprint_corner.push_back(
      G+0.5*crossrange_extent*kappa_hat-0.5*range_extent*rho_hat);

   for (int c=0; c<4; c++)
   {
      footprint_corner[c].put(2,0);
   }

   return polygon(footprint_corner);
}

// ---------------------------------------------------------------------
// Member function reconstruct_footprint regenerates the trapezoid
// corresponding to the endpoints of the rays emanating outward from
// the camera's position.  The coordinates for the trapezoid returned
// by this method are in absolute space (rather than relative to the
// camera's instantaneous position).

polygon ObsFrustum::reconstruct_footprint(double curr_t,int pass_number)
{
   threevector camera_posn;
   get_UVW_coords(curr_t,pass_number,camera_posn);

   threevector V1,V2;
   footprint_corner.clear();
   for (int i=0; i<4; i++)
   {
      LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(i);
      curr_segment_ptr->recover_V1_and_V2(curr_t,pass_number,V1,V2);
      footprint_corner.push_back(camera_posn+V2);
   }
   return polygon(footprint_corner);
}

// ==========================================================================
// Polyhedron member functions
// ==========================================================================

// Member function generate_curr_polyhedron

polyhedron* ObsFrustum::generate_curr_polyhedron(
   double curr_t,int pass_number)
{
//   cout << "inside ObsFrustum::generate_curr_polyhedron()" << endl;

// First determine whether ObsFrustum corresponds to "5-face" or
// "6-face" type by comparing linesegments 4-7 with 8-11:

   bool five_face_frustum_type=true;
   threevector V1,V2,W1,W2;
   for (int i=4; i<8; i++)
   {
      LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(i);
      LineSegment* next_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(i+4);
      curr_segment_ptr->recover_V1_and_V2(curr_t,pass_number,V1,V2);
      next_segment_ptr->recover_V1_and_V2(curr_t,pass_number,W1,W2);
      if (!V1.nearly_equal(W1) || !V2.nearly_equal(W2)) 
         five_face_frustum_type=false;
   } // loop over index i 

// Extract vertices corresponding to camera position and V2 endpoints
// of first four line segments:

   vector<threevector> vertices;

   for (int i=0; i<4; i++)
   {
      LineSegment* curr_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(i);
      curr_segment_ptr->recover_V1_and_V2(curr_t,pass_number,V1,V2);
      if (i==0) vertices.push_back(V1);
      if (five_face_frustum_type) vertices.push_back(V2);
   } // loop over index i labeling linesegments

   if (five_face_frustum_type)
   {
      polyhedron_ptr->set_vertices(vertices);

      polyhedron_ptr->set_edge(0,1);
      polyhedron_ptr->set_edge(0,2);
      polyhedron_ptr->set_edge(0,3);
      polyhedron_ptr->set_edge(0,4);
      polyhedron_ptr->set_edge(1,2);
      polyhedron_ptr->set_edge(2,3);
      polyhedron_ptr->set_edge(3,4);
      polyhedron_ptr->set_edge(4,1);
      polyhedron_ptr->set_edge(1,3);

// Anye Li pointed out on 3/9/07 that GTS uses a LEFT-handed ordering
// of vertices for triangle faces.  If a surface's number of triangle
// faces is even, then we can adopt a more mathematically conventional
// RIGHT-handed ordering of vertices, for (-1)**even = +1.  But if the
// surface's number of triangles is odd, we must conform to GTS'
// left-handed ordering convention.  According to the GTS
// documentation, triangle normals for all closed surfaces always
// point outward from the polyhedron's center.

      polyhedron_ptr->set_face(4,1,0);
      polyhedron_ptr->set_face(1,5,2);
      polyhedron_ptr->set_face(6,3,2);
      polyhedron_ptr->set_face(7,0,3);
      polyhedron_ptr->set_face(4,8,5);
      polyhedron_ptr->set_face(8,7,6);
   }
   else
   {
      LineSegment* fourth_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(4);
      fourth_segment_ptr->recover_V1_and_V2(curr_t,pass_number,V1,V2);
      vertices.push_back(V1);
      vertices.push_back(V2);
      LineSegment* tenth_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(10);
      tenth_segment_ptr->recover_V1_and_V2(curr_t,pass_number,V1,V2);
      vertices.push_back(V2);
      vertices.push_back(V1);
      LineSegment* eighth_segment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(8);
      eighth_segment_ptr->recover_V1_and_V2(curr_t,pass_number,V1,V2);
      vertices.push_back(V2);
      vertices.push_back(V1);

      polyhedron_ptr->set_vertices(vertices);

      polyhedron_ptr->set_edge(0,1);
      polyhedron_ptr->set_edge(0,2);
      polyhedron_ptr->set_edge(0,5);
      polyhedron_ptr->set_edge(0,6);

      polyhedron_ptr->set_edge(1,2);
      polyhedron_ptr->set_edge(2,3);
      polyhedron_ptr->set_edge(3,4);
      polyhedron_ptr->set_edge(4,1);

      polyhedron_ptr->set_edge(4,5);
      polyhedron_ptr->set_edge(5,6);
      polyhedron_ptr->set_edge(6,3);
      
      polyhedron_ptr->set_edge(1,3);
      polyhedron_ptr->set_edge(3,5);
      polyhedron_ptr->set_edge(0,3);
      polyhedron_ptr->set_edge(0,4);
      
      polyhedron_ptr->set_face(0,4,1);
      polyhedron_ptr->set_face(14,2,8);
      polyhedron_ptr->set_face(14,7,0);

      polyhedron_ptr->set_face(2,3,9);

      polyhedron_ptr->set_face(13,10,3);
      polyhedron_ptr->set_face(13,1,5);

      polyhedron_ptr->set_face(11,5,4);
      polyhedron_ptr->set_face(11,7,6);

      polyhedron_ptr->set_face(12,9,10);
      polyhedron_ptr->set_face(12,6,8);
   } // 5-face frustum type conditional

//   cout << "*polyhedron_ptr = " << *polyhedron_ptr << endl;
   return polyhedron_ptr;
}

// ==========================================================================
// Aerial & footprint corner computation member functions
// ==========================================================================

void ObsFrustum::compute_aerial_and_footprint_corners(
   double z_ground,const threevector& camera_posn,
   const vector<threevector>& UV_corner_dir,double max_lambda)
{
   const double default_lambda=3000;	// meters
   vector<int> aerial_corner_index;
   vector<double> lambda;

   for (int i=0; i<4; i++)
   {
      lambda.push_back((z_ground-camera_posn.get(2))/UV_corner_dir[i].get(2));
      
//      cout << "i = " << i << " lambda = " << lambda.back() << endl;
//      cout << "ray.z = " << UV_corner_dir[i].get(2) << endl;
      if (lambda.back() < 0 || lambda.back() > max_lambda)
      {
         aerial_corner_index.push_back(i);
         lambda[i]=basic_math::min(default_lambda,max_lambda);
      }
//      cout << "i = " << i << " final lambda = " << lambda[i] << endl;
   }

// Aerial and footprint corner coords are specified relative to camera
// position:

   aerial_corner.clear();
   footprint_corner.clear();

//   cout << "aerial_corner_index.size() = "
//        << aerial_corner_index.size() << endl;
   
   if (is_odd(aerial_corner_index.size()))
//   if (aerial_corner_index.size()==3)
   {
      cout << "Error in ObsFrustum::build_current_frustum()" << endl;
      cout << "aerial_corner_index.size() = "
           << aerial_corner_index.size() << endl;
      exit(-1);
   }

   if (aerial_corner_index.size()==0 || aerial_corner_index.size()==4)
   {
      for (int i=0; i<4; i++)
      {
         footprint_corner.push_back(camera_posn+lambda[i]*UV_corner_dir[i]);
      }
   }
   else if (aerial_corner_index.size()==2)
   {

// First loop over rays emanating from camera's posn:

      for (int i=0; i<4; i++)
      {

// Check whether current index i corresponds to an aerial corner
// index:

         bool aerial_corner_flag=false;
         for (unsigned int j=0; j<aerial_corner_index.size(); j++)
         {
            if (i==aerial_corner_index[j])
            {
               aerial_corner_flag=true;
            }
         }
         
         threevector curr_corner=camera_posn+lambda[i]*UV_corner_dir[i];
         if (aerial_corner_flag)
         {
            aerial_corner.push_back(curr_corner);
         }
         else
         {
            footprint_corner.push_back(curr_corner);
         }
      } // loop over index i labeling rays emanating from camera posn
      
// Next complete aerial and footprint corners by projecting aerial
// corners into world z-plane:

/*
      threevector corner1=aerial_corner[1];
      corner1.put(2,z_ground);
//      aerial_corner.push_back(corner1);
      threevector corner0=aerial_corner[0];
      corner0.put(2,z_ground);
//      aerial_corner.push_back(corner0);
      cout << "corner1 = " << corner1 << endl;
      cout << "corner0 = " << corner0 << endl;
//      footprint_corner.push_back(corner0);
//      footprint_corner.push_back(corner1);
*/

//      cout << "Aerial_corner.size() = " << aerial_corner.size() << endl;
      vector<threevector> overlapping_corner;
      for (unsigned int a=0; a<aerial_corner_index.size(); a++)
      {
         overlapping_corner.push_back(
            aerial_corner[aerial_corner_index.size()-1-a]);
         overlapping_corner.back().put(2,z_ground);
//         cout << "a = " << a 
//              << " overlapping_corner.back() = "
//              << overlapping_corner.back() << endl;
         aerial_corner.push_back(overlapping_corner.back());
      }
      
      for (unsigned int a=0; a<aerial_corner_index.size(); a++)
      {
         footprint_corner.push_back(overlapping_corner[a]);
      }

/*
// For debugging purposes, compute coords of aerial and footprint
// corners relative to their avg locations:

      threevector aerial_avg,footprint_avg;
      for (int a=0; a<4; a++)
      {
         aerial_avg += 0.25*aerial_corner[a];
         footprint_avg += 0.25*footprint_corner[a];
      }

      for (int a=0; a<4; a++)
      {
         cout << "a = " << a << " aerial_corner[a]-aerial_avg = "
              << aerial_corner[a]-aerial_avg << endl;
      }

      for (int a=0; a<4; a++)
      {
         cout << "a = " << a << " footprint_corner[a]-footprint_avg = "
              << footprint_corner[a]-footprint_avg << endl;
      }
*/
   } // aerial_corner_index.size() conditional
   
}

