// ==========================================================================
// OBSFRUSTUM class member function definitions
// ==========================================================================
// Last updated on 3/4/13; 3/12/13; 3/17/13; 4/5/14
// ==========================================================================

#include <set>
#include <string>
#include <osg/Switch>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "image/binaryimagefuncs.h"
#include "video/camera.h"
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "math/constant_vectors.h"
#include "geometry/contour.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "osg/osgModels/MODEL.h"
#include "osg/osgModels/OBSFRUSTUM.h"
#include "osg/osgModels/OBSFRUSTUMfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "geometry/plane.h"
#include "osg/osg3D/PointCloud.h"
#include "geometry/polygon.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "math/prob_distribution.h"
#include "image/raster_parser.h"
#include "image/recursivefuncs.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osg3D/tdpfuncs.h"
#include "templates/mytemplates.h"
#include "datastructures/Triple.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void OBSFRUSTUM::allocate_member_objects()
{
//    cout << "inside OBSFRUSTUM::allocate_member_objs()" << endl;
//   ModelsGroup_ptr=new MODELSGROUP(pass_ptr,const_cast<threevector*>(
//      grid_world_origin_ptr),AnimationController_ptr);
//   string model_filename="Camera.osg";
//   camera_model_ptr=ModelsGroup_ptr->generate_new_Model(model_filename);
//   ModelsGroup_ptr->initialize_const_posn(Zero_vector,camera_model_ptr);

   ArrowsGroup_ptr=new ArrowsGroup(3,pass_ptr,grid_world_origin_ptr);
   PyramidsGroup_ptr=new PyramidsGroup(pass_ptr,grid_world_origin_ptr);
   ray_tracer_ptr=new ray_tracer();

}		       

void OBSFRUSTUM::initialize_member_objects()
{
   Graphical_name="OBSFRUSTUM";

   portrait_mode_flag=false;
   rectangular_movie_flag=true;
   virtual_camera_flag=false;
   display_camera_model_flag=false;
   viewing_pyramid_above_zplane_exists=false;
   display_ViewingPyramid_flag=false;
   display_ViewingPyramidAboveZplane_flag=true;
   movie_downrange_distance=50;	// meters
   if (grid_world_origin_ptr != NULL) 
      z_ground=grid_world_origin_ptr->get(2);
   ground_bbox_ptr=NULL;
   DTED_ptwoDarray_ptr=NULL;

   TilesGroup_ptr=NULL;
   CylindersGroup_ptr=NULL;
   LineSegmentsGroup_ptr=NULL;
   PyramidsGroup_ptr=NULL;

   volume_alpha=0.0;

   viewing_pyramid_ptr=NULL;
   viewing_pyramid_above_zplane_ptr=NULL;
   ViewingPyramid_ptr=NULL;
   ViewingPyramidAboveZplane_ptr=NULL;

   set_permanent_color(colorfunc::white);

   prev_i_start=prev_i_stop=-1;
}		       

OBSFRUSTUM::OBSFRUSTUM(
   AnimationController* AC_ptr,int id):
   Geometrical(3,id,AC_ptr)
{	
//   cout << "inside OBSFRUSTUM simple constructor" << endl;

   grid_world_origin_ptr=NULL;
   initialize_member_objects();
}

// Angle ranges az_extent and el_extent must be specified in radians.

OBSFRUSTUM::OBSFRUSTUM(
   Pass* PI_ptr,double az_extent,double el_extent,
   threevector* grid_world_origin_ptr,AnimationController* AC_ptr,
   Movie* Movie_ptr,int id):
   Geometrical(3,id,AC_ptr)
{	
   pass_ptr=PI_ptr;
   this->az_extent=az_extent;
   this->el_extent=el_extent;

   this->grid_world_origin_ptr=grid_world_origin_ptr;
   this->Movie_ptr=Movie_ptr;

   initialize_member_objects();
   allocate_member_objects();
}		       

OBSFRUSTUM::OBSFRUSTUM(
   Pass* PI_ptr,threevector* grid_world_origin_ptr,
   AnimationController* AC_ptr,Movie* Movie_ptr,int id):
   Geometrical(3,id,AC_ptr)
{	
//   cout << "inside OBSFRUSTUM constructor #2" << endl;
   pass_ptr=PI_ptr;
   this->grid_world_origin_ptr=grid_world_origin_ptr;
   this->Movie_ptr=Movie_ptr;

   initialize_member_objects();
   allocate_member_objects();
}		       

OBSFRUSTUM::~OBSFRUSTUM()
{
//   cout << "inside OBSFRUSTUM destructor" << endl;

   delete ArrowsGroup_ptr;
   delete PyramidsGroup_ptr;	
   PyramidsGroup_ptr=NULL;
   delete ray_tracer_ptr;

   delete viewing_pyramid_ptr;
   delete viewing_pyramid_above_zplane_ptr;

   if (LineSegmentsGroup_ptr != NULL)
   {
      get_PAT_ptr()->removeChild(LineSegmentsGroup_ptr->get_OSGgroup_ptr());
      delete LineSegmentsGroup_ptr;
      LineSegmentsGroup_ptr=NULL;
   }
   
   if (CylindersGroup_ptr != NULL)
   {
      get_PAT_ptr()->removeChild(CylindersGroup_ptr->get_OSGgroup_ptr());
      delete CylindersGroup_ptr;
      CylindersGroup_ptr=NULL;
   }
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,OBSFRUSTUM& o)
{
   outstream << "inside OBSFRUSTUM::operator<<" << endl;

   Movie* Movie_ptr=o.get_Movie_ptr();
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   outstream << "Movie = " << *Movie_ptr << endl;
   outstream << "camera = " << *camera_ptr << endl;
   outstream << static_cast<const Geometrical&>(o) << endl;
   return outstream;
}

// ==========================================================================
// Set & get methods
// ==========================================================================

void OBSFRUSTUM::set_display_camera_model_flag(
   double curr_t,int pass_number,bool flag)
{
/*
//   cout << "inside OBSFRUSTUM::set_display_camera_model_flag(), flag = "
//        << flag << endl;
   
   display_camera_model_flag=flag;
   camera_model_ptr->set_mask(
      curr_t,pass_number,!display_camera_model_flag);
*/
}

// ---------------------------------------------------------------------
// Member function set_Movie_visibility_flag() turns the osg::Switch
// node on or off depending upon input boolean flag.  Recall that the
// switch node was added in
// OBSFRUSTAGROUP::initialize_new_OBSFRUSTUM() in order to simply
// enable erasing or unerasing of OBSFRUSTA movies.  

//  OBSFRUSTUM_ptr->get_PAT_ptr()	----->
//  osg::Switch				----->
//  Movie_ptr->get_PAT_ptr()

// If iput flag==true [false], Movie is visible [invisible]

void OBSFRUSTUM::set_Movie_visibility_flag(bool flag)
{
   if (Movie_ptr == NULL) return;
   osg::Node* node_ptr=get_PAT_ptr()->getChild(0);
   osg::Switch* MovieSwitch_ptr=dynamic_cast<osg::Switch*>(node_ptr);

   if (flag)
   {
      MovieSwitch_ptr->setAllChildrenOn();
   }
   else
   {
      MovieSwitch_ptr->setAllChildrenOff();
   }
}

// ==========================================================================
// Frustum initialization and construction methods
// ==========================================================================

pyramid* OBSFRUSTUM::generate_viewing_pyramid_ptr()
{
//   cout << "inside OBSFRUSTUM::generate_viewing_pyramid_ptr()" << endl;
   if (viewing_pyramid_ptr==NULL) viewing_pyramid_ptr=new pyramid();
   return viewing_pyramid_ptr;
}

pyramid* OBSFRUSTUM::generate_viewing_pyramid_above_zplane_ptr()
{
   if (viewing_pyramid_above_zplane_ptr==NULL) 
      viewing_pyramid_above_zplane_ptr=new pyramid();
   return viewing_pyramid_above_zplane_ptr;
}

// Member function instantiate_OSG_Pyramids instantiates OSG Pyramid
// graphicals for both the full viewing pyramid as well as the
// generalized pyramid located above some z=Z plane.

void OBSFRUSTUM::instantiate_OSG_Pyramids()
{
//   cout << "inside OBSFRUSTUM::instantiate_OSG_Pyramids()" << endl;
//   cout << "display_ViewingPyramid_flag = " 
//        << display_ViewingPyramid_flag << endl;
//   cout << "display_ViewingPyramidAboveZplane_flag = " 
//        << display_ViewingPyramidAboveZplane_flag << endl;

   if (ViewingPyramid_ptr==NULL)
      ViewingPyramid_ptr=PyramidsGroup_ptr->generate_new_Pyramid();

   generate_viewing_pyramid_ptr();
   ViewingPyramid_ptr->set_pyramid_ptr(viewing_pyramid_ptr);

   if (ViewingPyramidAboveZplane_ptr==NULL)
      ViewingPyramidAboveZplane_ptr=PyramidsGroup_ptr->generate_new_Pyramid();

   if (display_ViewingPyramidAboveZplane_flag)
   {
      generate_viewing_pyramid_above_zplane_ptr();
      ViewingPyramidAboveZplane_ptr->set_pyramid_ptr(
         viewing_pyramid_above_zplane_ptr);
   }

//   cout << "viewing_pyramid_ptr = " << viewing_pyramid_ptr << endl;
//   cout << "viewing_pyramid_above_zplane_ptr = " 
//        << viewing_pyramid_above_zplane_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function generate_Pyramid_geodes() must be called AFTER
// *viewing_pyramid_ptr and *viewing_pyramid_above_zplane_ptr are
// constructed in order for volume coloring to work...

void OBSFRUSTUM::generate_Pyramid_geodes()
{
//   cout << "inside OBSFRUSTUM::generate_Pyramid_geodes()" << endl;
//   cout << "*viewing_pyramid_ptr = " << *viewing_pyramid_ptr << endl;

   PyramidsGroup_ptr->generate_pyramid_geode(ViewingPyramid_ptr);
   
   if (display_ViewingPyramidAboveZplane_flag)
   {
      PyramidsGroup_ptr->generate_pyramid_geode(
         ViewingPyramidAboveZplane_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function generate_or_reset_viewing_pyramid() takes in the camera
// position, the direction vectors corresponding to the photo's four
// corners and the length of the viewfrustum's side rays.  It
// instantiates geometrical pyramid *viewing_pyramid_ptr from this
// input data.

void OBSFRUSTUM::generate_or_reset_viewing_pyramid(
   const threevector& camera_posn,const vector<threevector>& UV_corner_dir,
   double sidelength)
{
//   cout << "inside OBSFRUSTUM::generate_or_reset_viewing_pyramid()" << endl;
//   cout << "sidelength = " << sidelength << endl;
//   cout << "camera_posn = " << camera_posn << endl;
//   cout << "UV_corner_dir: " << endl;
//   templatefunc::printVector(UV_corner_dir);

   vector<threevector> base_vertices;
   for (unsigned int i=0; i<4; i++)
   {
      base_vertices.push_back(camera_posn+sidelength*UV_corner_dir[i]);
   }
//   cout << "camera_posn = " << camera_posn << endl;
//   cout << "base_vertices[0] = " << base_vertices[0] << endl;
//   cout << "base_vertices[1] = " << base_vertices[1] << endl;
//   cout << "base_vertices[2] = " << base_vertices[2] << endl;
//   cout << "base_vertices[3] = " << base_vertices[3] << endl;
   viewing_pyramid_ptr->generate_or_reset_square_pyramid(
      camera_posn,base_vertices);

//   viewing_pyramid_ptr->ensure_faces_handedness(face::right_handed);
//   cout << "*viewing_pyramid_ptr = " << *viewing_pyramid_ptr << endl;
//   outputfunc::enter_continue_char();
}  

// ---------------------------------------------------------------------
// Member function initialize_frustum_with_movie instantiates an
// OBSFRUSTUM with a Movie object located downrange from the apex by
// input distance movie_downrange_distance.  The entire OBSFRUSTUM may
// be rotated and translated over time via calls to
// absolute_position_and_orientation().

void OBSFRUSTUM::initialize_frustum_with_movie(
   double& frustum_sidelength,double movie_downrange_distance,
   bool portrait_mode_flag)
{
//   cout << "inside OBSFRUSTUM::initialize_frustum_with_movie()" << endl;
//   cout << "frustum_sidelength = " << frustum_sidelength << endl;
//   cout << "movie_downrange_distance = " << movie_downrange_distance
//        << endl;

   if (frustum_sidelength < 0 && movie_downrange_distance > 0)
   {
      frustum_sidelength=compute_pyramid_sidelength(movie_downrange_distance);
   }
   else if (movie_downrange_distance < 0 && frustum_sidelength > 0)
   {
      movie_downrange_distance=compute_movie_downrange_distance(
         frustum_sidelength);
   }
   else if (movie_downrange_distance < 0 && frustum_sidelength < 0)
   {
      cout << "Error in OBSFRUSTUM::initialize_frustum_with_movie() !!!" 
           << endl;
      cout << "frustum_sidelength = " << frustum_sidelength 
           << " movie_downrange_distance = " << movie_downrange_distance
           << endl;
      cout << "Movie_ptr->get_video_filename() = "
           << Movie_ptr->get_video_filename() << endl;
      exit(-1);
   }

   this->movie_downrange_distance=movie_downrange_distance;
   this->portrait_mode_flag=portrait_mode_flag;
}

// ---------------------------------------------------------------------
// Member function compute_pyramid_sidelength takes in the downrange
// distance for a movie window.  This method returns the length of the
// side edges of a pyramid (those which connect onto the apex) for
// which the base lies at the same downrange distance as the movie window.

double OBSFRUSTUM::compute_pyramid_sidelength(double movie_downrange_distance)
{
   double pyramid_sidelength=-1;
   if (Movie_ptr != NULL)
   {
      return Movie_ptr->get_camera_ptr()->
         compute_pyramid_sidelength(movie_downrange_distance);
//      threevector a_hat=
//         -Movie_ptr->get_camera_ptr()->get_What();
//      threevector w_hat=
//         Movie_ptr->get_camera_ptr()->get_UV_corner_world_ray().at(0);
//      pyramid_sidelength=movie_downrange_distance/a_hat.dot(w_hat);
   }
   return pyramid_sidelength;
}

double OBSFRUSTUM::compute_movie_downrange_distance(double pyramid_sidelength)
{
   double movie_downrange_distance=-1;
   if (Movie_ptr != NULL)
   {
      return Movie_ptr->get_camera_ptr()->
         compute_movie_downrange_distance(pyramid_sidelength);
//      threevector a_hat=
//         -Movie_ptr->get_camera_ptr()->get_What();
//      threevector w_hat=
//         Movie_ptr->get_camera_ptr()->get_UV_corner_world_ray().at(0);
//      movie_downrange_distance=pyramid_sidelength*a_hat.dot(w_hat);
   }
   return movie_downrange_distance;
}

// ---------------------------------------------------------------------
// Member function compute_viewing_pyramid_above_Zplane computes the
// vertices,edges and faces of pyramid *viewing_pyramid_ptr which lie
// above Z=z.  It then forms a new, generalized pyramid
// *viewing_pyramid_above_zplane_ptr from this information.  A second
// OSG Pyramid object is added to PyramidsGroup containing for
// displaying the part of the OBSFRUSTUM which lies above Z=z. 

pyramid* OBSFRUSTUM::compute_viewing_pyramid_above_Zplane(
   double z,pyramid* pyramid_ptr)
{
//   cout << "inside OBSFRUSTUM::compute_viewing_pyramid_above_Zplane()" 
//        << endl;
//   cout << "pyramid_ptr = " << pyramid_ptr << endl;
//   cout << "*pyramid_ptr = " << *pyramid_ptr << endl;
//   cout << "viewing_pyramid_ptr = " << viewing_pyramid_ptr << endl;
//   cout << "viewing_pyramid_above_zplane_ptr = "
//        << viewing_pyramid_above_zplane_ptr << endl;
//   cout << "z = " << z << endl;

//   outputfunc::enter_continue_char();

//   cout << "display_ViewingPyramidAboveZplane_flag = "
//        << display_ViewingPyramidAboveZplane_flag << endl;
   if (!display_ViewingPyramidAboveZplane_flag) return NULL;

// First check whether input *pyramid_ptr lies completely above Z=z
// plane.  If so *viewing_pyramid_above_zplane_ptr = *pyramid_ptr:

   if (pyramid_ptr->lies_above_Zplane_check(z))
   {
      *viewing_pyramid_above_zplane_ptr=*pyramid_ptr;
      return viewing_pyramid_above_zplane_ptr;
   }

   vector<vertex> vertices_above_Zplane;
   vector<edge> edges_above_Zplane;
   vector<face> triangles_above_Zplane;


   pyramid_ptr->extract_parts_above_Zplane(
      z,vertices_above_Zplane,edges_above_Zplane,triangles_above_Zplane);
//   cout << "vertices_above_Zplane = " << endl;
//   templatefunc::printVector(vertices_above_Zplane);

   viewing_pyramid_above_zplane_exists=false;
   if (vertices_above_Zplane.size() > 0)
   {
      viewing_pyramid_above_zplane_exists=true;
      viewing_pyramid_above_zplane_ptr->reset_vertices_edges_faces(
         pyramid_ptr->get_apex().get_posn(),vertices_above_Zplane,
         edges_above_Zplane,triangles_above_Zplane);
      face* zplane_face_ptr=pyramid_ptr->get_zplane_face_ptr();
      if (zplane_face_ptr != NULL)
      {
         viewing_pyramid_above_zplane_ptr->set_zplane_face(*zplane_face_ptr);
      }
      
//      cout << "viewing_pyramid_above_zplane_ptr = " 
//           << viewing_pyramid_above_zplane_ptr << endl;
//      cout << "*viewing_pyramid_above_zplane_ptr = " 
//           << *viewing_pyramid_above_zplane_ptr << endl;
   } // > 0 vertices above Zplane conditional

//   cout << "At end of OBSFRUSTUM::compute_viewing_pyramid_above_Zplane()"
//        << endl;
//   cout << "*viewing_pyramid_above_zplane_ptr = "
//        << *viewing_pyramid_above_zplane_ptr << endl;
//   cout << "viewing_pyramid_above_zplane_ptr = "
//        << viewing_pyramid_above_zplane_ptr << endl;

   return viewing_pyramid_above_zplane_ptr;
}

// ---------------------------------------------------------------------
// Member function build_current_frustum generates an OBSFRUSTUM at
// time t whose camera position lies at input threevector camera_posn
// and whose symmetry axis is aligned with input threevector
// n_hat. The footprint corners of the frustum are assumed to lie a
// distance z_base_face above the world z-plane grid.  

// The rotations, translations and scalings needed to transform
// canonical unit-length linesegments into the frustum's sides are
// stored for later callback retrieval.

void OBSFRUSTUM::build_current_frustum(
   double curr_t,int pass_number,const threevector& camera_posn,
   const threevector& v_hat,double yaw,double pitch,double z_base_face)
{
//   cout << "inside OBSFRUSTUM::build_current_frustum() #1" << endl;
   threevector w_hat(v_hat.cross(z_hat));
   threevector n_hat=cos(pitch)*cos(yaw)*v_hat-cos(pitch)*sin(yaw)*w_hat
      +sin(pitch)*z_hat;
   build_current_frustum(curr_t,pass_number,camera_posn,n_hat,z_base_face);
}

void OBSFRUSTUM::build_current_frustum(
   double curr_t,int pass_number,const threevector& camera_posn,
   const threevector& n_hat,double z_base_face)
{
//     cout << "inside OBSFRUSTUM::build_current_frustum() #2 , t = " 
//          << curr_t << endl;

   double phi,theta;
   mathfunc::decompose_direction_vector(n_hat,phi,theta);
   build_current_frustum(curr_t,pass_number,camera_posn,phi,theta,
                         z_base_face);
}

void OBSFRUSTUM::build_current_frustum(
   double curr_t,int pass_number,const threevector& camera_posn,
   double phi,double theta,double z_base_face)
{
//     cout << "inside OBSFRUSTUM::build_current_frustum(camera_posn,phi,theta), t = " 
//          << curr_t << endl;
//     cout << "camera_posn = " << camera_posn << endl;
//     cout << "this = " << this << endl;
//     cout << "theta = " << theta*180/PI 
//          << " phi = " << phi*180/PI << endl;
//     cout << "az_extent = " << az_extent*180/PI 
//          << " el_extent = " << el_extent*180/PI << endl;
//     cout << "z_base_face = " << z_base_face << endl;

//   int t_int=curr_t;
//   if (t_int%1000==0) cout << t_int/1000 << "  " << flush;

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

// First check whether all corner rays point downwards (e.g. aerial
// sensor looking at ground):

   bool all_downward_rays_flag=true;
   const double SMALL_NEG=-0.01;
   for (unsigned int c=0; c<4; c++)
   {
      if (corner_ray[c].get(2) > SMALL_NEG) all_downward_rays_flag=false;
   }

// If all corner rays point downwards, constructing
// *viewing_pyramid_above_zplane_ptr is *MUCH* easier and faster than
// if not...

//   cout << "all_downward_rays_flag = " << all_downward_rays_flag
//        << endl;
   if (all_downward_rays_flag)
   {
      vector<threevector> base_vertices;
      for (unsigned int c=0; c<4; c++)
      {
         double curr_sidelength=(z_base_face-camera_posn.get(2))/
            corner_ray[c].get(2);
         base_vertices.push_back(camera_posn+curr_sidelength*corner_ray[c]);
//         cout << "c = " << c 
//              << " corner_ray = " << corner_ray[c] 
//              << " sidelength = " << curr_sidelength 
//              << " base_vertex = " << base_vertices.back()
//              << endl;
      }

// If *viewing_pyramid_above_zplane_ptr was previously instantiated,
// simply reset its vertices rather than recompute the entire
// polyhedron graph:

//      viewing_pyramid_above_zplane_ptr->generate_or_reset_square_pyramid(
//         camera_posn,base_vertices);

// As of 9 pm on Sun, July 20, instantiation of ~6000 OBSFRUSTA
// Viewing Pyramids slows to a crawl for the Constant Hawk model in
// our Bluegrass demo.  We suspect that we should eliminate all calls
// to update_vertex_map() except at the very end of pyramid
// construction.  We should also eventually take the Constant Hawk
// OBSFRUSTUM to be dynamically computed in real-time rather than
// pre-calculated at the start of the Bluegrass demo.  

// For now, we live with continuing to generate each OBSFRUSTUM
// pyramid in advance...

      viewing_pyramid_above_zplane_ptr->generate_square_pyramid(
         camera_posn,base_vertices);

      viewing_pyramid_above_zplane_ptr->set_zplane_face(
         *viewing_pyramid_above_zplane_ptr->get_base_ptr());
   }
   else
   {
      double max_sidelength=0;
      for (unsigned int c=0; c<4; c++)
      {
         double curr_sidelength=(z_base_face-camera_posn.get(2))/
            corner_ray[c].get(2);
         max_sidelength=basic_math::max( max_sidelength,curr_sidelength);
      }
      generate_or_reset_viewing_pyramid(
         camera_posn,corner_ray,2*max_sidelength);
      compute_viewing_pyramid_above_Zplane(z_base_face,viewing_pyramid_ptr);
   }
//   cout << "*viewing_pyramid_above_zplane_ptr = "
//        << *viewing_pyramid_above_zplane_ptr << endl;

//   cout << "display_ViewingPyramid_flag = " 
//        << display_ViewingPyramid_flag << endl;
//   cout << "display_ViewingPyramidAboveZplane_flag = "
//        << display_ViewingPyramidAboveZplane_flag << endl;

//   cout << "ViewingPyramid_ptr = " << ViewingPyramid_ptr << endl;
//   cout << "ViewingPyramidAboveZplane_ptr = "
//        << ViewingPyramidAboveZplane_ptr << endl;

   ViewingPyramid_ptr->set_mask(
      curr_t,pass_number,!display_ViewingPyramid_flag);
   ViewingPyramidAboveZplane_ptr->set_mask(
      curr_t,pass_number,!display_ViewingPyramidAboveZplane_flag);

// Don't bother to build ViewingPyramid if it is not to be displayed:

   if (display_ViewingPyramid_flag)
   {
      ViewingPyramid_ptr->build_current_pyramid(
         curr_t,pass_number,viewing_pyramid_ptr);
   }
   if (display_ViewingPyramidAboveZplane_flag)
   {
      ViewingPyramidAboveZplane_ptr->build_current_pyramid(
         curr_t,pass_number,viewing_pyramid_above_zplane_ptr);

   
// In order to instantaneously position movie within Z-plane as well
// as colored, translucent Pyramid volume, save vertex chain from
// zplane_face along with the pyramid's apex within
// *Pyramid_above_zplane_ptr graphical's time-dependent vertices:

      ViewingPyramidAboveZplane_ptr->store_apex_and_zplane_vertices(
         curr_t,pass_number,viewing_pyramid_above_zplane_ptr);
   }
   set_typical_pyramid_edge_widths();
}

// ---------------------------------------------------------------------
// Member function compute_corner_rays() takes in OBSFRUSTUM extent
// angles alpha and beta (which are functions of az_extent and
// el_extent) as well as orientation angles roll (measured from nadir)
// and pitch along with velocity direction vector v_hat.  It computes
// the direction vectors of the OBSFRUSTUM's four corners and checks
// whether they all have nonzero projection along -z_hat.  If so, this
// boolean method returns true.

bool OBSFRUSTUM::compute_corner_rays(
   const threevector& v_hat,
   double horiz_FOV,double vert_FOV,double roll,double pitch)
{
//   cout << "inside OBSFRUSTUM::compute_corner_rays()" << endl;
   double alpha,beta;
   OBSFRUSTUMfunc::convert_FOVs_to_alpha_beta_angles(
      horiz_FOV,vert_FOV,alpha,beta);
   return compute_corner_rays(alpha,beta,roll,pitch,v_hat);
}

bool OBSFRUSTUM::compute_corner_rays(
   double alpha,double beta,double roll,double pitch,
   const threevector& v_hat)
{
   return OBSFRUSTUMfunc::compute_corner_rays(
      alpha,beta,roll,pitch,v_hat,corner_ray);
}

// ---------------------------------------------------------------------
// This next overloaded version of build_current_frustum() is
// high-level.  It takes in the OBSFRUSTUM's roll angle away from
// nadir about the velocity axis.  It also takes in the subsequent
// pitch angle along the direction of the velocity axis.  This method
// computes the corresponding alpha and beta opening angles
// corresponding to the pre-defined horizontal and vertical
// field-of-view angles for the OBSFRUSTUM.  

void OBSFRUSTUM::build_current_frustum(
   double curr_t,int pass_number,
   double roll,double pitch,double z_base_face,
   const threevector& apex_posn,const threevector& v_hat)
{
//   cout << "inside OBSFRUSTUM::build_current_frustum(roll,pitch)" << endl;

   double alpha,beta;
   OBSFRUSTUMfunc::convert_FOVs_to_alpha_beta_angles(
      get_az_extent(),get_el_extent(),alpha,beta);
//   cout << "alpha = " << alpha*180/PI << " beta = " << beta*180/PI << endl;
//    bool all_downward_rays_flag=
      build_current_frustum(
         curr_t,pass_number,apex_posn,v_hat,alpha,beta,roll,pitch,z_base_face);
//   cout << "all_downward_rays_flag = " << all_downward_rays_flag << endl;
}

// ---------------------------------------------------------------------
// This overloaded version of build_current_frustum takes in the
// viewing pyramid's instantaneous position and velocity direction
// vector.  It also takes in opening angle alpha between the velocity
// direction vector (assumed to lie exclusively in the XY plane) and
// the observation frustum's side projected into the XY plane.  Angle
// beta between the observation frustum's 3D side and -z_hat is also
// passed as an input parameter.  The OBSFRUSTUM is also rotated about
// the MODEL's velocity direction vector by angle roll, and it is then
// rotated about w_hat=v_hat x z_hat by angle pitch.  This method
// recomputes the vertices for the viewing pyramid above the Z plane
// and rebuilds the corresponding OSG Pyramid.

// We wrote this method in July 2008 in order to dynamically compute
// and display Predator observation frusta in real time.  Unlike the
// preceding versions of build_current_frustum(), the angular size of
// the frustum calculated in this version does NOT depend upon n_hat.

bool OBSFRUSTUM::build_current_frustum(
   double curr_t,int pass_number,
   const threevector& apex_posn,const threevector& v_hat,
   double alpha,double beta,double roll,double pitch,double z_base_face)
{
//   cout << "inside OBSFRUSTUM::build_current_frustum(apex_posn,v_hat,alpha,beta)" << endl;
//   cout << "t = " << curr_t << endl;
//   cout << "apex_posn = " << apex_posn << endl;
//   cout << "this = " << this << endl;
//   cout << "v_hat = " << v_hat << endl;
//   cout << "alpha = " << alpha*180/PI 
//        << " beta = " << beta*180/PI << endl;
//   cout << "z_base_face = " << z_base_face << endl;

   bool all_downward_rays_flag=
      compute_corner_rays(alpha,beta,roll,pitch,v_hat);

   if (!all_downward_rays_flag)
   {
      cout << "Error in build_current_frustum(vhat, etc)" << endl;
      cout << "Expecting downward looking frustum" << endl;
      return all_downward_rays_flag;
   }
   
// If all corner rays point downwards, constructing
// *viewing_pyramid_above_zplane_ptr is *MUCH* easier and faster than
// if not...

   vector<threevector> base_vertices;
   for (unsigned int c=0; c<4; c++)
   {
      double curr_sidelength=(z_base_face-apex_posn.get(2))/
         corner_ray[c].get(2);
      base_vertices.push_back(apex_posn+curr_sidelength*corner_ray[c]);
//      cout << "c = " << c 
//           << " corner_ray = " << corner_ray[c]
//           << " curr_sidelength = " << curr_sidelength << endl;
//      cout << "base_vertex = " << base_vertices.back() << endl;
   }

   build_current_frustum(curr_t,pass_number,apex_posn,base_vertices);

   return all_downward_rays_flag;
}

// ---------------------------------------------------------------------
// This overloaded version of build_current_frustum takes in the
// current positions of a square viewing pyramid's apex and base
// vertices.  (The viewing pyramid must be a simple square pyramid and
// not some more complicated form when it partially intersects a
// ground Z-plane.)  This method builds the current
// *ViewingPyramidAboveZplane_ptr and optionally ViewingPyramid_ptr.

void OBSFRUSTUM::build_current_frustum(
   double curr_t,int pass_number,
   const threevector& apex_posn,const vector<threevector>& base_vertices)
{
//   cout << "inside OBSFRUSTUM::build_current_frustum(apex_posn,base_vertices), t = " 
//        << curr_t << endl;
//   cout << "viewing_pyramid_ptr = " << viewing_pyramid_ptr << endl;
//   cout << "apex_posn = " << apex_posn << endl;
//   for (unsigned int v=0; v<base_vertices.size(); v++)
//   {
//      cout << "v = " << v << " base_vertex = " << base_vertices[v] << endl;
//   }
   
// If *viewing_pyramid_above_zplane_ptr was previously instantiated,
// simply reset its vertices rather than recompute the entire
// polyhedron graph:
         
   viewing_pyramid_above_zplane_ptr->generate_or_reset_square_pyramid(
      apex_posn,base_vertices);
   viewing_pyramid_above_zplane_ptr->set_zplane_face(
      *viewing_pyramid_above_zplane_ptr->get_base_ptr());
   
//   cout << "*viewing_pyramid_above_zplane_ptr = "
//        << *viewing_pyramid_above_zplane_ptr << endl;

   ViewingPyramid_ptr->set_mask(
      curr_t,pass_number,!display_ViewingPyramid_flag);
   ViewingPyramidAboveZplane_ptr->set_mask(
      curr_t,pass_number,!display_ViewingPyramidAboveZplane_flag);

// Don't bother to build ViewingPyramid if it is not to be displayed:

   if (display_ViewingPyramid_flag)
   {
      ViewingPyramid_ptr->build_current_pyramid(
         curr_t,pass_number,viewing_pyramid_ptr);
   }
   if (display_ViewingPyramidAboveZplane_flag)
   {
      ViewingPyramidAboveZplane_ptr->build_current_pyramid(
         curr_t,pass_number,viewing_pyramid_above_zplane_ptr);
   
// In order to instantaneously position movie within Z-plane as well
// as colored, translucent Pyramid volume, save vertex chain from
// zplane_face along with the pyramid's apex within
// *Pyramid_above_zplane_ptr graphical's time-dependent vertices:

//      cout << "ViewingPyramidAboveZplane_ptr = "
//           << ViewingPyramidAboveZplane_ptr << endl;
      ViewingPyramidAboveZplane_ptr->store_apex_and_zplane_vertices(
         curr_t,pass_number,viewing_pyramid_above_zplane_ptr);
   }

   set_typical_pyramid_edge_widths();
}

// ---------------------------------------------------------------------
// This overloaded version of build_current_frustum takes in 3x4
// projection matrix *curr_P_ptr which is assumed to correspond to
// *Movie_ptr.  It computes the view frustum's apex and base vertices
// from *curr_P_ptr and then calls build_current_frustum() with these
// latter inputs.

void OBSFRUSTUM::build_current_frustum(
   double curr_t,int pass_number,genmatrix* curr_P_ptr,
   double frustum_sidelength,double filter_alpha_value,
   bool temporally_filter_flag)
{
//   cout << "inside OBSFRUSTUM::build_current_frustum(P_ptr)" << endl;

   camera* camera_ptr=Movie_ptr->get_camera_ptr();

   camera_ptr->set_projection_matrix(
      *curr_P_ptr,filter_alpha_value,temporally_filter_flag);

   threevector apex_posn=camera_ptr->get_world_posn();
//   cout << "apex_posn = " << apex_posn << endl;

   vector<threevector> UV_corner_world_ray=camera_ptr->
      get_UV_corner_world_ray();
   vector<threevector> base_vertices;
   for (unsigned int c=0; c<UV_corner_world_ray.size(); c++)
   {
      base_vertices.push_back(
         apex_posn+frustum_sidelength*UV_corner_world_ray[c]);
//      cout << "c = " << c
//           << " base_vertex = " << base_vertices.back() << endl;
   }

   build_current_frustum(curr_t,pass_number,apex_posn,base_vertices);
}

// ---------------------------------------------------------------------
// This first version of member function build_OBSFRUSTUM() is a
// high-level method for generating an OBSFRUSTUM corresponding to a
// ground camera looking in a general direction.

void OBSFRUSTUM::build_OBSFRUSTUM(
   double curr_t,int pass_number,
   double frustum_sidelength,double movie_downrange_distance,
   const threevector& camera_world_posn,const vector<threevector>& corner_ray,
   colorfunc::Color OBSFRUSTUM_color,double vol_alpha)
{
//   cout << "inside OBSFRUSTUM::build_OBSFRUSTUM() #1" << endl;
   
   set_display_ViewingPyramid_flag(true);
   set_display_ViewingPyramidAboveZplane_flag(false);
   instantiate_OSG_Pyramids();

   initialize_frustum_with_movie(frustum_sidelength,movie_downrange_distance);
   generate_or_reset_viewing_pyramid(
      Zero_vector,corner_ray,frustum_sidelength);

   absolute_position(curr_t,pass_number,camera_world_posn);
   
   set_display_camera_model_flag(curr_t,pass_number,false);

   double Zplane_altitude=0;
   build_OBSFRUSTUM(curr_t,pass_number,Zplane_altitude);

   set_volume_alpha(vol_alpha);
//   cout << "volume_alpha = " << volume_alpha << endl;
   set_color(colorfunc::get_OSG_color(OBSFRUSTUM_color),volume_alpha);
   set_permanent_color(OBSFRUSTUM_color);
}

// ---------------------------------------------------------------------
void OBSFRUSTUM::build_OBSFRUSTUM(
   double curr_t,int pass_number,double Zplane_altitude,
   const threevector& camera_world_posn,const vector<threevector>& corner_ray,
   colorfunc::Color OBSFRUSTUM_color,double vol_alpha)
{
//    cout << "inside OBSFRUSTUM::build_OBSFRUSTUM() #2" << endl;
//    cout << "Zplane_altitude = " << Zplane_altitude << endl;

   set_display_ViewingPyramid_flag(false);
   set_display_ViewingPyramidAboveZplane_flag(true);

//   double frustum_sidelength=1000;	// meters
   double frustum_sidelength=10000;	// meters
   double movie_downrange_distance=-1;

   instantiate_OSG_Pyramids();

   initialize_frustum_with_movie(frustum_sidelength,movie_downrange_distance);
   generate_or_reset_viewing_pyramid(
      Zero_vector,corner_ray,frustum_sidelength);

   absolute_position(curr_t,pass_number,camera_world_posn);
   
   set_display_camera_model_flag(curr_t,pass_number,false);

   build_OBSFRUSTUM(curr_t,pass_number,Zplane_altitude);

   set_volume_alpha(vol_alpha);
//   cout << "volume_alpha = " << volume_alpha << endl;
   set_color(
      colorfunc::yellow,colorfunc::red,colorfunc::white,
      volume_alpha);
}

// ---------------------------------------------------------------------
// Member function build_OBSFRUSTUM()

void OBSFRUSTUM::build_OBSFRUSTUM(
   double curr_t,int pass_number,double Zplane_altitude)
{
//   cout << "inside OBSFRUSTUM::build_OBSFRUSTUM() #3" << endl;
//   cout << "display_ViewingPyramidAboveZplane_flag = "
//        << get_display_ViewingPyramidAboveZplane_flag() 
//        << endl;
//   cout << "Zplane_altitude = " << Zplane_altitude << endl;

// Compute instantaneous above_Zplane pyramid.  Then build
// instantaneous ViewingPyramidAboveZplane graphical from this pyramid
// object:

   if (get_display_ViewingPyramidAboveZplane_flag())
   {
      generate_viewing_pyramid_ptr();
      compute_viewing_pyramid_above_Zplane(
         Zplane_altitude,get_viewing_pyramid_ptr());
   }
   
   generate_Pyramid_geodes();

   get_ViewingPyramid_ptr()->set_mask(
      curr_t,pass_number,!get_display_ViewingPyramid_flag());
   get_ViewingPyramidAboveZplane_ptr()->set_mask(
      curr_t,pass_number,!get_display_ViewingPyramidAboveZplane_flag());

   if (get_display_ViewingPyramid_flag())
   {
      get_ViewingPyramid_ptr()->build_current_pyramid(
         curr_t,pass_number,get_viewing_pyramid_ptr());
   }

   if (get_display_ViewingPyramidAboveZplane_flag())
   {
      get_ViewingPyramidAboveZplane_ptr()->
         build_current_pyramid(
            curr_t,pass_number,get_viewing_pyramid_above_zplane_ptr());
   }
   
   set_typical_pyramid_edge_widths();
}

// ---------------------------------------------------------------------
// This overloaded version of build_OBSFRUSTUM() takes in camera
// position and az, el and roll angles defined in world
// geocoordinates.  It updates the position and orientation of the
// OBSFRUSTUM along with its Movie and camera objects.

void OBSFRUSTUM::build_OBSFRUSTUM(
   double curr_t,int pass_number,
   double frustum_sidelength,double movie_downrange_distance,
   const threevector& camera_world_posn,double az,double el,double roll,
   colorfunc::Color OBSFRUSTUM_color,double volume_alpha)
{
   camera* virtual_camera_ptr=get_Movie_ptr()->get_camera_ptr();

   vector<threevector> UV_corner_rays=reorient_camera_corner_rays(
      az,el,roll,virtual_camera_ptr);

   build_OBSFRUSTUM(
      curr_t,pass_number,frustum_sidelength,movie_downrange_distance,
      camera_world_posn,UV_corner_rays,OBSFRUSTUM_color,volume_alpha);

   virtual_camera_ptr->set_world_posn(camera_world_posn);
   virtual_camera_ptr->set_Rcamera(az,el,roll);
   bool recompute_internal_params_flag=false;
   virtual_camera_ptr->construct_projection_matrix(
      recompute_internal_params_flag);

   set_relative_Movie_window();

   outputfunc::enter_continue_char();
}

// ==========================================================================
// Frustum drawing methods
// ==========================================================================

// Member function set_color

void OBSFRUSTUM::set_color(const colorfunc::Color& c)
{
   set_color(colorfunc::get_OSG_color(c));
}

void OBSFRUSTUM::set_color(const osg::Vec4& color)
{
   double VolumeAlpha=0.0;
   set_color(color,VolumeAlpha);
}

void OBSFRUSTUM::set_color(
   const osg::Vec4& color,double VolumeAlpha)
{
//   cout << "inside OBSFRUSTUM::set_color(color,VolumeAlpha) #3" << endl;
//   cout << "VolumeAlpha = " << VolumeAlpha << endl;

   set_volume_alpha(VolumeAlpha);
   osg::Vec4 volume_color(color.r(),color.g(),color.b(),volume_alpha);
   set_color(color,color,volume_color);
}

void OBSFRUSTUM::set_color(
   const colorfunc::Color& SideEdgeColor,
   const colorfunc::Color& ZplaneEdgeColor,
   const colorfunc::Color& VolumeColor,double VolumeAlpha)
{
   set_volume_alpha(VolumeAlpha);

   osg::Vec4 side_edge_color=colorfunc::get_OSG_color(SideEdgeColor);
   osg::Vec4 zplane_edge_color=colorfunc::get_OSG_color(ZplaneEdgeColor);
   osg::Vec4 volume_color=colorfunc::get_OSG_color(
      VolumeColor,volume_alpha);

   set_color(side_edge_color,zplane_edge_color,volume_color);
}

void OBSFRUSTUM::set_color(
   const osg::Vec4& side_edge_color,const osg::Vec4& zplane_edge_color,
   const osg::Vec4& volume_color)
{
//   cout << "inside OBSFRUSTUM::set_color()" << endl;
//   cout << "display_ViewingPyramid_flag = " << display_ViewingPyramid_flag
//        << endl;
//   cout << "display_ViewingPyramidAboveZplane_flag = " 
//        << display_ViewingPyramidAboveZplane_flag
//        << endl;

//   cout << "viewing_pyramid_ptr = " << viewing_pyramid_ptr << endl;
//   cout << "viewing_pyramid_above_zplane_ptr = "
//        << viewing_pyramid_above_zplane_ptr << endl;

//   cout << "side_edge_color = " << endl;
//   osgfunc::print_Vec4(side_edge_color);

//   cout << "zplane_edge_color = " << endl;
//   osgfunc::print_Vec4(zplane_edge_color);

//   cout << "volume_color = " << endl;
//   osgfunc::print_Vec4(volume_color);
   
// On 4/13/09, we empirically determined that we must generally
// execute the following call to ViewingPyramid_ptr->set_color() even
// if display_ViewingPyramid_flag==false
 
   ViewingPyramid_ptr->set_color(
      viewing_pyramid_ptr,side_edge_color,zplane_edge_color,
      zplane_edge_color,volume_color);

   if (display_ViewingPyramidAboveZplane_flag)
      ViewingPyramidAboveZplane_ptr->set_color(
         viewing_pyramid_above_zplane_ptr,side_edge_color,zplane_edge_color,
         zplane_edge_color,volume_color);
}

// ---------------------------------------------------------------------
// Member function set_typical_pyramid_edge_widths

void OBSFRUSTUM::set_typical_pyramid_edge_widths()
{
   const double side_edge_width=2;
   const double base_edge_width=5;
   const double zplane_edge_width=6;

   if (display_ViewingPyramid_flag)
   {
      ViewingPyramid_ptr->set_edge_widths(
         viewing_pyramid_ptr,
         side_edge_width,base_edge_width,zplane_edge_width);
   }

   if (display_ViewingPyramidAboveZplane_flag)
   {
      ViewingPyramidAboveZplane_ptr->set_edge_widths(
         viewing_pyramid_above_zplane_ptr,
         side_edge_width,base_edge_width,zplane_edge_width);
   }
}
      
// ==========================================================================
// Time-dependent translation and rotation member functions
// ==========================================================================

// Member function absolute_position_and_orientation resets the
// camera's pyramid, model and (potentially) movie so that their
// common origin resides at input threevector absolute_position and
// the photo's Uhat and Vhat axes align with input threevectors
// new_Uhat and new_Vhat.

void OBSFRUSTUM::translate(
   double curr_t,int pass_number,const threevector& trans)
{
//   cout << "inside OBSFRUSTUM::translate()" << endl;

   threevector curr_posn;
   get_UVW_coords(curr_t,pass_number,curr_posn);
   threevector absolute_posn=curr_posn+trans;

   absolute_position(curr_t,pass_number,absolute_posn);
}

void OBSFRUSTUM::absolute_position(
   double curr_t,int pass_number,const threevector& absolute_posn)
{
//   cout << "inside OBSFRUSTUM::absolute_posn()" << endl;

   if (Movie_ptr != NULL)
   {
      camera* camera_ptr=Movie_ptr->get_camera_ptr();
//      cout << "camera_ptr = " << camera_ptr << endl;
//      cout << "*camera_ptr = " << *camera_ptr << endl;
//      cout << "Abs_posn =  " << absolute_posn << endl;
      camera_ptr->set_world_posn(absolute_posn);
      bool recompute_internal_params_flag=false;
      camera_ptr->construct_projection_matrix(recompute_internal_params_flag);
   }

   threevector curr_posn;
   get_UVW_coords(curr_t,pass_number,curr_posn);
   threevector delta_posn=absolute_posn-curr_posn;
//   cout << "delta_posn = " << delta_posn << endl;

   Graphical::translate(curr_t,pass_number,delta_posn);

   if (viewing_pyramid_ptr != NULL)
      viewing_pyramid_ptr->absolute_position(absolute_posn);

   if (viewing_pyramid_above_zplane_ptr != NULL)
      viewing_pyramid_above_zplane_ptr->absolute_position(absolute_posn);

   if (get_ViewingPyramid_ptr() != NULL)
   {
      get_ViewingPyramid_ptr()->absolute_posn(
         curr_t,pass_number,absolute_posn);
   }
   if (get_ViewingPyramidAboveZplane_ptr() != NULL)
   {
      get_ViewingPyramidAboveZplane_ptr()->absolute_posn(
         curr_t,pass_number,absolute_posn);
   }
   
   set_relative_Movie_window();
}

// Note added on 9/25/07: Probably need to pass in boolean parameter
// recompute_internal_params_flag rather than hardwiring it to false
// inside this next method.  This param should equal true in main
// program RELROT which computes intrinsic and extrinsic camera params
// based upon 2D tiepoint feature matching...

void OBSFRUSTUM::absolute_position_and_orientation(
   double curr_t,int pass_number,const threevector& absolute_posn,
   const threevector& new_Uhat,const threevector& new_Vhat)
{
//   cout << "inside OBSFRUSTUM::absolute_position_and_orientation()" << endl;
//   cout << "new_Uhat = " << new_Uhat << " new_Vhat = " << new_Vhat << endl;
   
   threevector curr_posn;
   get_UVW_coords(curr_t,pass_number,curr_posn);
   threevector delta_posn=absolute_posn-curr_posn;

   threevector curr_Uhat,curr_Vhat,curr_What;
   get_UVW_dirs(curr_t,pass_number,curr_Uhat,curr_Vhat,curr_What);

   rotation Rinit,Rinv,Rnew,Rtotal;
   Rinit.put_column(0,curr_Uhat);
   Rinit.put_column(1,curr_Vhat);
   Rinit.put_column(2,curr_Uhat.cross(curr_Vhat));

   Rinit.inverse(Rinv);

   Rnew.put_column(0,new_Uhat);
   Rnew.put_column(1,new_Vhat);
   Rnew.put_column(2,new_Uhat.cross(new_Vhat));

   Rtotal=Rnew*Rinv;

   rotate_about_specified_origin_then_translate(
      curr_t,pass_number,Zero_vector,Rtotal,delta_posn);
   set_UVW_dirs(curr_t,pass_number,new_Uhat,new_Vhat);
   set_relative_Movie_window();

   if (viewing_pyramid_ptr != NULL)
   {
      viewing_pyramid_ptr->rotate(Zero_vector,Rtotal);
      viewing_pyramid_ptr->absolute_position(absolute_posn);
   }
   
// Reset camera's Uhat, Vhat and What direction vectors:

   if (Movie_ptr != NULL)
   {
      camera* camera_ptr=Movie_ptr->get_camera_ptr();
      camera_ptr->set_world_posn(absolute_posn);
      camera_ptr->set_Rcamera(Rnew);
      bool recompute_internal_params_flag=false;
      camera_ptr->construct_projection_matrix(recompute_internal_params_flag);
   }
}

// ---------------------------------------------------------------------
// This overloaded version of member function
// absolute_position_and_orientation takes in azimuth, elevation and
// roll angles measured in radians about +z_hat, -y_hat' and +x_hat''.
// It reorients the OBSFRUSTUM's pyramids, camera model and movie
// image.  It also relocates these objects so that their origin
// resides in world-space at input threevector absolute_posn.

void OBSFRUSTUM::absolute_position_and_orientation(
   double curr_t,int pass_number,
   const threevector& absolute_posn,double az,double el,double roll)
{
   rotation R;
   if (Movie_ptr != NULL)
   {
      camera* camera_ptr=Movie_ptr->get_camera_ptr();
      camera_ptr->set_Rcamera(az,el,roll);
      R=camera_ptr->get_Rcamera_ptr()->transpose();
   }
   else
   {
      camera curr_camera;
      curr_camera.set_Rcamera(az,el,roll);
      R=curr_camera.get_Rcamera_ptr()->transpose();
   }

// Matrix R contains new camera direction vectors Uhat, Vhat and What
// within its zeroth, first and second columns:

//   cout << "R = " << R << endl;
   threevector new_Uhat,new_Vhat;
   R.get_column(0,new_Uhat);
   R.get_column(1,new_Vhat);

   absolute_position_and_orientation(
      curr_t,pass_number,absolute_posn,new_Uhat,new_Vhat);
}

// ---------------------------------------------------------------------
// Member function set_relative_Movie_window remaps an input 2D
// rectangular movie window onto a 3D parallelogram so that it
// precisely touches the side edge rays of the current OBSFRUSTUM.
// Recall that we insert the Movie_ptr->get_PAT_ptr() into
// OBSFRUSTUM_ptr->get_PAT_ptr() within
// OBSFRUSTAGROUP::initialize_new_OBSFRUSTUM().  So when we translate
// and rotate the OBSFRUSTUM, *Movie_ptr is automatically transformed
// as well provided we pass its relative posn = Zero_vector into the
// following Movie method:

void OBSFRUSTUM::set_relative_Movie_window()
{
   if (Movie_ptr == NULL) return;
   set_relative_Movie_window(
      Movie_ptr,Movie_ptr->get_camera_ptr(),movie_downrange_distance);
}

void OBSFRUSTUM::set_relative_Movie_window(
   Movie* movie_ptr,camera* camera_ptr,double downrange_distance)
{
//   cout << "inside OBSFRUSTUM::set_relative_Movie_window()" << endl;
//   cout << "this OBSFRUSTUM = " << this << endl;
//   cout << "movie_ptr = " << movie_ptr << endl;
   
   if (movie_ptr == NULL) return;
   
//      threevector a_hat=-camera_ptr->get_What();
//      threevector w_hat=camera_ptr->get_UV_corner_world_ray().at(0);
//      double rho=movie_downrange_distance/a_hat.dot(w_hat);
//      cout << "downrange_dist = " << downrange_distance << endl;
//      cout << "rho = " << rho << endl;

//      for (unsigned int i=0; i<4; i++)
//      {
//         cout << "i= " << i << " UV_corner_world_ray = " 
//              << camera_ptr->get_UV_corner_world_ray().at(i) << endl;
//      }

// As of Nov 2009, we assume that if an imageplane has been explicitly
// defined within camera_ptr, then we should project the movie window
// into that pre-defined imageplane:

   plane* imageplane_ptr=camera_ptr->get_imageplane_ptr();
//   cout << "imageplane_ptr = " << imageplane_ptr << endl;
//   cout << "warp_onto_imageplane_flag = "
//        << movie_ptr->get_warp_onto_imageplane_flag() << endl;
      
   if (movie_ptr->get_camera_ptr()==camera_ptr && imageplane_ptr != NULL 
       && movie_ptr->get_warp_onto_imageplane_flag())
   {
      movie_ptr->warp_photo_onto_imageplane(*imageplane_ptr);
      movie_ptr->set_alpha(0.5);
   }
   else
   {
      movie_ptr->remap_window_corners(
         downrange_distance,
         camera_ptr->get_world_posn(),-camera_ptr->get_What(),
         camera_ptr->get_UV_corner_world_ray());
   }
   movie_ptr->set_dynamic_window_flag(false);

}

// ---------------------------------------------------------------------
void OBSFRUSTUM::orient_camera_model(
   double curr_t,int pass_number,const vector<threevector>& UV_corner_dir)
{
/*
//   cout << "inside OBSFRUSTUM::orient_camera_model()" << endl;

   threevector scale(1,1,1);

   threevector u_hat((UV_corner_dir[1]-UV_corner_dir[0]).unitvector());
   threevector v_hat((UV_corner_dir[2]-UV_corner_dir[1]).unitvector());
   threevector w_hat=u_hat.cross(v_hat);

//   cout << "u_hat = " << u_hat << " v_hat = " << v_hat << endl;
//   cout << "w_hat = " << w_hat << endl;

   rotation R;
   R.put_column(0,u_hat);
   R.put_column(1,v_hat);
   R.put_column(2,w_hat);
//   cout << "R = " << R << endl;

   camera_model_ptr->scale_and_rotate(curr_t,pass_number,Zero_vector,R,scale);
*/
}

// ==========================================================================
// Footprint member functions
// ==========================================================================

// Member function GMTI_dwell_frustum_footprint generates an
// OBSFRUSTUM at time t whose camera position lies at input
// threevector V and whose footprint on the ground has a specified
// range and crossrange extent.  The footprint forms a rectangle which
// is located a distance z_base_face above the world z-plane grid.  The
// rotations, translations and scalings needed to transform canonical
// unit-length linesegments into the frustum's sides are stored for
// later callback retrieval.

polygon OBSFRUSTUM::GMTI_dwell_frustum_footprint(
   const threevector& V,const threevector& G,
   double range_extent,double crossrange_extent,double z_base_face)
{
   threevector rhat=(G-V).unitvector();
   threevector rho=rhat-(rhat.get(2)*z_hat);
   threevector rho_hat=rho.unitvector();
   threevector kappa_hat=rho_hat.cross(z_hat);
   
   vector<threevector> footprint_corner;
   footprint_corner.push_back(
      G+0.5*crossrange_extent*kappa_hat+0.5*range_extent*rho_hat);
   footprint_corner.push_back(
      G-0.5*crossrange_extent*kappa_hat+0.5*range_extent*rho_hat);
   footprint_corner.push_back(
      G-0.5*crossrange_extent*kappa_hat-0.5*range_extent*rho_hat);
   footprint_corner.push_back(
      G+0.5*crossrange_extent*kappa_hat-0.5*range_extent*rho_hat);

   for (unsigned int c=0; c<4; c++)
   {
      footprint_corner[c].put(2,0);
   }

   return polygon(footprint_corner);
}

// ==========================================================================
// Projection & backprojection member functions
// ==========================================================================

// Member function estimate_z_ground() forms a contour from the
// perimeter of the viewing_pyramid_above_zplane's zplane face.  It
// then samples the input point cloud's height values at some large
// number of points along the contour.  Member variable z_ground is
// set equal to the median value of the sampled heights.  If no valid
// ground value can be estimated, this method returns a
// NEGATIVEINFINITY sentinel value.

double OBSFRUSTUM::estimate_z_ground(
   PointCloud* PointCloud_ptr,unsigned int n_frac_bins,bool compute_avg_flag)
{
//   cout << "inside OBSFRUSTUM::estimate_z_ground()" << endl;
//   cout << "n_frac_bins = " << n_frac_bins << endl;

// Estimate ground plane height by sampling points along viewing
// pyramid's z-plane face:

   face* zplane_face_ptr=
      get_viewing_pyramid_above_zplane_ptr()->get_zplane_face_ptr();
//   cout << "zplane_face = " << *zplane_face_ptr << endl;

   double frac_start=0;
   double frac_stop=1.0;
   double d_frac=(frac_stop-frac_start)/n_frac_bins;
   vector<double> sampled_heights;

//   cout << "compute_avg_flag = " << compute_avg_flag << endl;
   if (compute_avg_flag)
   {
      polygon zplane_poly=zplane_face_ptr->get_polygon();

      for (unsigned int n=0; n<n_frac_bins; n++)
      {
         double frac=frac_start+n*d_frac;
         threevector curr_point;
         zplane_poly.edge_point(frac,curr_point);
         double curr_z;
         if (PointCloud_ptr->find_Z_given_XY(
            curr_point.get(0),curr_point.get(1),curr_z))
         {
//            cout << "curr_z = " << curr_z << endl;
            if (!nearly_equal(curr_z,0))
            {
               sampled_heights.push_back(curr_z);
//               cout << "n = " << n << " curr_z = " << curr_z << endl;
            }
         }
      } // loop over index n labeling contour sample points
//      cout << "sampled_heights.size() = " << sampled_heights.size() << endl;
      if (sampled_heights.size() > 0)
      {
         double avg_zground=mathfunc::mean(sampled_heights);
//         cout << "Average z_ground = " << avg_zground << endl;
         return avg_zground;
      }
   }
   else
   {
      contour zplane_contour(*zplane_face_ptr);
      for (unsigned int n=0; n<n_frac_bins; n++)
      {
         double frac=frac_start+n*d_frac;
         threevector curr_point;
         zplane_contour.edge_point(frac,curr_point);
         double curr_z;
         if (PointCloud_ptr->find_Z_given_XY(
                curr_point.get(0),curr_point.get(1),curr_z))
         {
            if (!nearly_equal(curr_z,0))
            {
               sampled_heights.push_back(curr_z);
//            cout << "n = " << n << " curr_z = " << curr_z << endl;
            }
         }
      } // loop over index n labeling contour sample points

      prob_distribution prob(sampled_heights,n_frac_bins/2);
      double cum_prob=0.5;
      z_ground=prob.find_x_corresponding_to_pcum(cum_prob);

//      cout << "z_ground = " << z_ground << endl;
//      cout << "grid_world_origin_ptr->get(2) = " 
//           << grid_world_origin_ptr->get(2) << endl;

      return z_ground;
   }

   return NEGATIVEINFINITY;
}

// ---------------------------------------------------------------------
// Member function backproject_pixels_into_second_imageplane loops
// over every pixel within the current OBSFRUSTUM's movie.  It
// backprojects each pixel's ray onto the grid's z-plane.  The
// backprojected (X,Y,Z) position on the grid is then reprojected into
// the image plane for the input *OBSFRUSTUM2_ptr.  If the reprojected
// point lies inside the 2nd image plane, the original pixel is
// translucently recolored using the input delta_R, delta_G and
// delta_B values.  The modified, annotated image is written to an
// output PNG file.

// This method implements an approximate (and unfortunately quite
// slow) overlap computation between two different image planes.  

void OBSFRUSTUM::backproject_pixels_into_second_imageplane(
   OBSFRUSTUM* OBSFRUSTUM2_ptr,double overlap_hue)
{
   string banner="Backprojecting OBSFRUSTUM pixels into 2nd imageplane";
   outputfunc::write_banner(banner);
   
   cout << "inside OBSFRUSTUM::backproject_pixels_into_second_imageplane()"
        << endl;

   if (Movie_ptr==NULL ) return;
   unsigned int width=Movie_ptr->getWidth();
   unsigned int height=Movie_ptr->getHeight();

   Movie* Movie2_ptr=OBSFRUSTUM2_ptr->get_Movie_ptr();
   if (Movie2_ptr==NULL) return;
//   int width2=Movie2_ptr->getWidth();
//   int height2=Movie2_ptr->getHeight();
   camera* camera2_ptr=Movie2_ptr->get_camera_ptr();
   
   int n_movie1_pixels_in_movie2=0;
   for (unsigned int pu=0; pu<width; pu++)
   {
      if (pu%100==0) cout << pu << " " << flush;
      for (unsigned int pv=0; pv<height; pv++)
      {
         double u,v;
         Movie_ptr->get_uv_coords(pu,pv,u,v);
         threevector backproj_posn=
            Movie_ptr->get_camera_ptr()->backproject_imagepoint_to_zplane(
               u,v,z_ground);
//         cout << "pu = " << pu << " pv = " << pv
//              << " backproj.x = " << backproj_posn.get(0)
//              << " backproj.y = " << backproj_posn.get(1)
//              << " backproj.z = " << backproj_posn.get(2) << endl;

         double u2,v2;
         camera2_ptr->project_XYZ_to_UV_coordinates(
            backproj_posn.get(0),backproj_posn.get(1),backproj_posn.get(2),
            u2,v2);

         if (u2 > Movie2_ptr->get_minU() && u2 < Movie2_ptr->get_maxU() &&
             v2 > Movie2_ptr->get_minV() && v2 < Movie2_ptr->get_maxV())
         {
            n_movie1_pixels_in_movie2++;

// Reset hue and saturation values for pixels within *Movie_ptr which
// overlap *Movie2_ptr:

            int R,G,B;
            Movie_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
            double r=R/double(255);
            double g=G/double(255);
            double b=B/double(255);
            
            double hue,sat,value;
            colorfunc::RGB_to_hsv(r,g,b,hue,sat,value);
            sat=(1-0.5)*sat+0.5;
            value=(1-0.2)*value+0.2;

            colorfunc::hsv_to_RGB(overlap_hue,sat,value,r,g,b);
            R=int(255*r);
            G=int(255*g);
            B=int(255*b);
            R=basic_math::min(255,R);
            B=basic_math::min(255,B);
            G=basic_math::min(255,G);
            Movie_ptr->set_pixel_RGB_values(pu,pv,R,G,B);

/*
// Reset hue and saturation values for pixels within *Movie2_ptr which
// are mapped onto by pixels in *Movie_ptr:

            Movie2_ptr->get_RGB_values(u2,v2,R,G,B);

            r=R/double(255);
            g=G/double(255);
            b=B/double(255);
            
            colorfunc::RGB_to_hsv(r,g,b,hue,sat,value);
            sat=(1-0.5)*sat+0.5;
            value=(1-0.2)*value+0.2;

            colorfunc::hsv_to_RGB(overlap_hue,sat,value,r,g,b);
            R=255*r;
            G=255*g;
            B=255*b;
            R=basic_math::min(255,R);
            B=basic_math::min(255,B);
            G=basic_math::min(255,G);
            Movie2_ptr->set_RGB_values(u2,v2,R,G,B); */

         } // u2,v2 inside Movie2 conditional

      } // loop over pv index
   } // loop over pu index

   cout << endl;

   cout << "n_movie1_pixels_in_movie2 = " << n_movie1_pixels_in_movie2 
        << endl;
   cout << "n_movie1_pixels=width1*height1 = " << width*height << endl;
   cout << "Overlap fraction = " << double(n_movie1_pixels_in_movie2)/
      double(width*height) << endl;

   Movie_ptr->set_image();

   string directory_name=filefunc::getdirname(
      Movie_ptr->get_video_filename());
   string base_filename=filefunc::getbasename(
      Movie_ptr->get_video_filename());
   string output_filename=directory_name+"modified_"
      +stringfunc::prefix(base_filename)+".png";
   
// Flip image vertically prior to writing it to output file.
// Afterwards, flip it again:

   Movie_ptr->get_image_ptr()->flipVertical();

   if (osgDB::writeImageFile(*(Movie_ptr->get_image_ptr()),output_filename))
   {
      cout << "Modified image written to " << output_filename << endl;
   }

   Movie_ptr->get_image_ptr()->flipVertical();


/*
   Movie2_ptr->set_image();

   directory_name=filefunc::getdirname(Movie2_ptr->get_video_filename());
   base_filename=filefunc::getbasename(Movie2_ptr->get_video_filename());
   output_filename=directory_name+"modified_"
      +stringfunc::prefix(base_filename)+".png";
   
// Flip image vertically prior to writing it to output file.
// Afterwards, flip it again:

   Movie2_ptr->get_image_ptr()->flipVertical();

   if (osgDB::writeImageFile(*(Movie2_ptr->get_image_ptr()),output_filename))
   {
      cout << "Modified image written to " << output_filename << endl;
   }

   Movie2_ptr->get_image_ptr()->flipVertical();
*/

}

// ---------------------------------------------------------------------
// Member function project_SignPosts_into_imageplane loops over every
// SignPost within input *SignPostsGroup_ptr and computes its range to
// the current OBSFRUSTUM's camera position.  If certain range and
// (U,V) conditions are satisfied, this method prints out SignPost
// (U,V,label) information which can be captured into a text file.
// The projected SignPost information can later be used to instantiate
// SignPosts superposed on top of a still image for 3D -> 2D knowledge
// projection display purposes.
 
void OBSFRUSTUM::project_SignPosts_into_imageplane(
   SignPostsGroup* SignPostsGroup_ptr)
{
//   cout << "inside OBSFRUSTUM::project_SignPosts_into_imageplane()" << endl;
   
   threevector camera_posn=Movie_ptr->get_camera_ptr()->get_world_posn();

   int n_projected_signposts=0;
   for (unsigned int n=0; n<SignPostsGroup_ptr->get_n_Graphicals(); n++)
   {
      SignPost* SignPost_ptr=SignPostsGroup_ptr->get_SignPost_ptr(n);
      
      threevector UVW;
      if (SignPost_ptr->get_UVW_coords(
         SignPostsGroup_ptr->get_curr_t(),
         SignPostsGroup_ptr->get_passnumber(),UVW))
      {
         double Z=UVW.get(2);
         double U,V;
         Movie_ptr->get_camera_ptr()->project_XYZ_to_UV_coordinates(
            UVW.get(0),UVW.get(1),Z,U,V);

         double curr_range=(UVW-camera_posn).magnitude();

         const double skyscraper_height=100;	// meters
         const double max_range=2000;		// meters
         if ( (curr_range < max_range && Z > skyscraper_height &&
               V < 1.2*Movie_ptr->get_maxV())  ||
              (curr_range < 0.5*max_range && V < Movie_ptr->get_maxV()) )
         {
            if (U > Movie_ptr->get_minU() &&
                U < Movie_ptr->get_maxU() &&
                V > Movie_ptr->get_minV())
//                && V < Movie_ptr->get_maxV())
            {

//               cout << "n = " << n 
//                    << " curr_range = " << curr_range
//                    << " SignPost label = " << SignPost_ptr->get_label()
//                    << endl;
//               cout << "U = " << U << " V = " << V 
//                    << " Z = " << Z << endl << endl;

               cout << n_projected_signposts++ << "  " 
                    << U << "  " << V << "  "
                    << SignPost_ptr->get_label() << "  " << endl;
            }
         }
      } 
   } // loop over index n labeling SignPosts

   cout << "Total number of projected SignPosts = " << n_projected_signposts
        << endl;
}

// ---------------------------------------------------------------------
// Member function generate_SignPost_at_imageplane_location takes in a
// pair of UV coordinates along with a *SignPostsGroup_ptr.  It
// instantiates a new SignPost at the 3D location corresponding to the
// UV imageplane coordinates which is displaced forward in range
// (along the +What direction).  The SignPost's pole is force to lie
// parallel to the image plane (assuming that the SignPost's direction
// approximately equals z_hat).

SignPost* OBSFRUSTUM::generate_SignPost_at_imageplane_location(
   const twovector& UV,SignPostsGroup* SignPostsGroup_ptr,
   double curr_t,int passnumber,double size,double height_multiplier,int ID)
{
   SignPost* curr_SignPost_ptr=SignPostsGroup_ptr->generate_new_SignPost(
      size,height_multiplier,Zero_vector,ID);
   threevector tip_posn=get_Movie_ptr()->imageplane_location(UV);
   curr_SignPost_ptr->set_UVW_coords(curr_t,passnumber,tip_posn);
 
// We assume that the imageplane's V axis approximately equals z_hat.
// In order to force the SignPost to align with V_hat, we subtract off
// from z_hat the component perpendicular to the image plane.  We then
// reset the SignPost's attitude so that it lies parallel to the image
// plane:

   threevector n_hat=get_Movie_ptr()->get_imageplane_ptr()->get_nhat();
   threevector z_parallel=z_hat-(z_hat.dot(n_hat))*n_hat;
   threevector V2=tip_posn+z_parallel;
   curr_SignPost_ptr->set_attitude_posn(curr_t,passnumber,tip_posn,V2);

   return curr_SignPost_ptr;
}

// ---------------------------------------------------------------------
// Member function project_curr_track_points takes in STL vector
// *tracks_ptr and backprojects its UV contents from image plane space
// onto the z-plane in world space.  [As of Aug 2008, we believe this
// method is highly specialized for our NYC demo program where we
// backproject cars found via OpenCV's tracker onto 37th Ave below the
// Empire State Building.]  As of Feb 2011, this method can be used
// for both our NYC demo program as well as our flight facility
// D7 video.  

void OBSFRUSTUM::project_curr_track_points(
   Movie::TRACKS_MAP* tracks_map_ptr,const fourvector& groundplane_pi)
{
//   cout << "inside OBSFRUSTUM::project_curr_track_pnts()" << endl;
//   cout << "ID = " << get_ID() << endl;
//   cout << "groundplane_pi = " << groundplane_pi << endl;

   if (tracks_map_ptr==NULL) return;

   threevector curr_posn;
   vector<int> track_IDs;
   vector<twovector> UV;

   for (Movie::TRACKS_MAP::iterator iter=tracks_map_ptr->begin();
        iter != tracks_map_ptr->end(); iter++)
   {
      track* curr_track_ptr=iter->second;

      if (curr_track_ptr->get_XYZ_coords(get_curr_t(),curr_posn))
      {
         track_IDs.push_back(curr_track_ptr->get_ID());
         UV.push_back(twovector(curr_posn.get(0),curr_posn.get(1)));
//         cout << "track ID = " << curr_track.get_ID()
//              << " u = " << curr_posn.get(0)
//              << " v = " << curr_posn.get(1) << "  ";
      }
   } // loop over index i labeling track number

   if (UV.size() > 0)
   {
      plane ground_plane(groundplane_pi);
      draw_rays_thru_imageplane_features_to_worldplane(
         track_IDs,UV,ground_plane);
   }
}

// ---------------------------------------------------------------------
// Member function draw_rays_thru_imageplane_features_to_worldplane
// takes in STL vector UV containing 2D feature points.  It also takes
// in plane P which exists in 3D worldspace.  This method propagates
// the 3D rays outwards from the camera's position through the 2D
// imageplane feature points.  It determines where these rays
// intersect the world-plane.  Colored lines representing the rays and
// small cylinders representing the planar intersection locations are
// stored within member objects LineSegmentsGroup and CylindersGroup.
 
void OBSFRUSTUM::draw_rays_thru_imageplane_features_to_worldplane(
   const vector<int>& track_IDs,const vector<twovector>& UV,
   const plane& ground_plane) 
{
//   cout << "inside OBSFRUSTUM::draw_rays_thru_imageplane_features_to_worldplane()" 
//        << endl;
   
   if (LineSegmentsGroup_ptr==NULL)
   {
      LineSegmentsGroup_ptr=new LineSegmentsGroup(
         3,pass_ptr,AnimationController_ptr);
      get_PAT_ptr()->addChild(LineSegmentsGroup_ptr->get_OSGgroup_ptr());
   }
   
   if (CylindersGroup_ptr==NULL)
   {
      CylindersGroup_ptr=new CylindersGroup(
         pass_ptr,AnimationController_ptr,grid_world_origin_ptr);
      CylindersGroup_ptr->unmask_all_OSGsubPATs();
      get_PAT_ptr()->addChild(CylindersGroup_ptr->get_OSGgroup_ptr());
   }

   threevector imageplane_intersection_point,ground_intersection_point;

   threevector a_hat=Movie_ptr->get_imageplane_ptr()->get_ahat();
   threevector b_hat=-Movie_ptr->get_imageplane_ptr()->get_bhat();
   threevector n_hat=-Movie_ptr->get_imageplane_ptr()->get_nhat();
//   cout <<  "a_hat = " << a_hat << " b_hat = " << b_hat << " n_hat = " 
//        << n_hat << endl;
//   cout << "a_hat.dot(b_hat) = " << a_hat.dot(b_hat) 
//        << " b_hat.dot(n_hat) = " << b_hat.dot(n_hat)
//        << " n_hat.dot(a_hat) = " << n_hat.dot(a_hat) << endl;
//   cout << "a_hat.cross(b_hat) = " << a_hat.cross(b_hat) << endl;
//   cout << "b_hat.cross(n_hat) = " << b_hat.cross(n_hat) << endl;
//   cout << "n_hat.cross(a_hat) = " << n_hat.cross(a_hat) << endl;

   rotation R;
   R.rotation_taking_pqr_to_uvw(x_hat,y_hat,z_hat,a_hat,b_hat,n_hat);
//   cout << "R = " << R << endl;

// For reasons we do not understand as of Oct 2007, OSG quaternions
// have a permuted set of values as compared to those returned by our
// method rotation.quaternion_corresponding_to_rotation().  By brute
// force trial-and-error, we have found that the following quaternion
// computation yields image plane cylinders whose symmetry axes are
// normal to the image plane:

   osg::Matrixd Rmat(R.get(0,0) , R.get(1,0) , R.get(2,0) , 0 ,
                     R.get(0,1) , R.get(1,1) , R.get(2,1) , 0 ,
                     R.get(0,2) , R.get(1,2) , R.get(2,2) , 0 ,
                     0 , 0 , 0 , 1);
   osg::Quat quat;
   quat.set(Rmat);
//   cout << "quat = " << endl;
//   osgfunc::print_quaternion(quat);
   
//   fourvector Q(R.quaternion_corresponding_to_rotation());
//   quat(Q.get(0),Q.get(1),Q.get(2),Q.get(3));
//   cout << "Q = " << Q << endl;

   for (unsigned int i=0; i<UV.size(); i++)
   {
      camera* camera_ptr=Movie_ptr->get_camera_ptr();
      if (camera_ptr->backproject_into_world_plane(
         UV[i].get(0),UV[i].get(1),ground_plane,ground_intersection_point) &&
          camera_ptr->backproject_into_world_plane(
             UV[i].get(0),UV[i].get(1),*(Movie_ptr->get_imageplane_ptr()),
             imageplane_intersection_point))
      {

// Specify ray emanating from camera's location to intersection point
// in camera's coordinate system (and not in absolute XYZ world space
// coordinates):

         threevector camera_posn=
            Movie_ptr->get_camera_ptr()->get_world_posn();
         threevector relative_ground_posn=ground_intersection_point-
            camera_posn;
         threevector relative_imageplane_posn=imageplane_intersection_point-
            camera_posn;

         int curr_track_ID=track_IDs[i];
         string ID_label(stringfunc::number_to_string(curr_track_ID));
         LineSegment* curr_ray_ptr=
            LineSegmentsGroup_ptr->get_ID_labeled_LineSegment_ptr(
               curr_track_ID);
         if (curr_ray_ptr==NULL)
         {
            bool draw_arrow_flag=false;
            curr_ray_ptr=LineSegmentsGroup_ptr->
               generate_new_canonical_LineSegment(
                  curr_track_ID,draw_arrow_flag);	
            curr_ray_ptr->set_permanent_color(colorfunc::grey); // NYC ESB
//            curr_ray_ptr->set_permanent_color(colorfunc::red);	  // NYC ESB
//            curr_ray_ptr->set_permanent_color(colorfunc::blue);	  // HAFB D7
            curr_ray_ptr->set_curr_color(curr_ray_ptr->get_permanent_color());
         }
         curr_ray_ptr->set_scale_attitude_posn(
            get_curr_t(),pass_ptr->get_ID(),relative_imageplane_posn,
            relative_ground_posn);

         colorfunc::Color ground_color=colorfunc::cyan;
         colorfunc::Color imageplane_color=colorfunc::cyan;
         osg::Quat trivial_quat(0,0,0,1);

         Cylinder* ground_cylinder_ptr=
            CylindersGroup_ptr->get_ID_labeled_Cylinder_ptr(2*curr_track_ID);
         Cylinder* imageplane_cylinder_ptr=
            CylindersGroup_ptr->get_ID_labeled_Cylinder_ptr(
               2*curr_track_ID+1);

         if (ground_cylinder_ptr==NULL)
         {
//            cout << "Generating new ground_cylinder/imageplane_cylinder" 
//                 << endl;
//            cout << "ID_label = " << ID_label << endl << endl;
            int n_text_messages=1;
            bool text_screen_axis_alignment_flag=true;

            CylindersGroup_ptr->set_radius(2.0);
            CylindersGroup_ptr->set_height(2.0);
            double text_size=CylindersGroup_ptr->get_radius();
            double text_displacement=2*CylindersGroup_ptr->get_height();
            ground_cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
               Zero_vector,trivial_quat,ground_color,
               n_text_messages,text_displacement,text_size,
               text_screen_axis_alignment_flag,2*curr_track_ID);
            ground_cylinder_ptr->set_stationary_Graphical_flag(false);
            ground_cylinder_ptr->set_text_label(0,ID_label);

            CylindersGroup_ptr->set_radius(0.33);	// NYC ESB
//            CylindersGroup_ptr->set_radius(0.03);	// HAFB D7
            CylindersGroup_ptr->set_height(0.2);

            text_size=2*CylindersGroup_ptr->get_radius();	// NYC ESB
//            text_size=20*CylindersGroup_ptr->get_radius();	// HAFB D7

            text_displacement=2*CylindersGroup_ptr->get_height();
            imageplane_cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
               Zero_vector,quat,imageplane_color,
               n_text_messages,text_displacement,text_size,
               text_screen_axis_alignment_flag,2*curr_track_ID+1);
            imageplane_cylinder_ptr->set_stationary_Graphical_flag(false);
            imageplane_cylinder_ptr->set_text_label(0,ID_label);
         }

// Since ground and imageplane cylinders are generally visible at some
// times and not at others, we explicitly mask the current ground and
// image plane cylinders for all times and then unmask them for only
// the current time.  This procedure ensures that no unwanted
// cylinders end up at the origin of the OBSFRUSTUM when they are not
// supposed to appear...

         CylindersGroup_ptr->mask_Graphical_for_all_times(
            ground_cylinder_ptr);
         CylindersGroup_ptr->mask_Graphical_for_all_times(
            imageplane_cylinder_ptr);

         bool mask_flag=false;
         ground_cylinder_ptr->set_mask(
            get_curr_t(),pass_ptr->get_ID(),mask_flag);
         imageplane_cylinder_ptr->set_mask(
            get_curr_t(),pass_ptr->get_ID(),mask_flag);

         ground_cylinder_ptr->set_UVW_coords(
            get_curr_t(),pass_ptr->get_ID(),relative_ground_posn);
         imageplane_cylinder_ptr->set_UVW_coords(
            get_curr_t(),pass_ptr->get_ID(),relative_imageplane_posn);

//         cout << "relative_ground_posn = " << relative_ground_posn << endl;
//         cout << "relative_imageplane_posn = " << relative_imageplane_posn
//              << endl;
      }

   } // loop over index i labeling UV features
}

// ---------------------------------------------------------------------
// Member function project_Polyhedra_into_imageplane() loops over all
// Polyhedra within input *PolyhedraGroup_ptr.  It projects each
// Polyhedron's faces into the OBSFRUSTUM's image plane.  The
// projected faces are drawn as translucent Polygons and returned
// within output *PolygonsGroup_ptr.  If some Polyhedra faces lie
// inside the OBSFRUSTUM's image plane, this boolean method returns true.
 
bool OBSFRUSTUM::project_Polyhedra_into_imageplane(
   PolyhedraGroup* PolyhedraGroup_ptr,
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr,
   twoDarray* DTED_ztwoDarray_ptr)
{
   cout << "inside OBSFRUSTUM::project_Polyhedra_into_imageplane()" << endl;

   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   threevector pointing_dir=-camera_ptr->get_What();

   PolyLinesGroup* PolyLinesGroup_ptr=
      PolygonsGroup_ptr->get_PolyLinesGroup_ptr();
//   cout << "PolyLinesGroup_ptr = " << PolyLinesGroup_ptr << endl;

   bool some_faces_inside_imageplane_flag=false;
   threevector bounding_sphere_center;
   double bounding_sphere_radius;

   for (unsigned int p=0; p<PolyhedraGroup_ptr->get_n_Graphicals(); p++)
   {
      Polyhedron* Polyhedron_ptr=PolyhedraGroup_ptr->get_Polyhedron_ptr(p);
      polyhedron* polyhedron_ptr=Polyhedron_ptr->get_polyhedron_ptr();

// Test whether polyhedron's bounding sphere lies outside OBSFRUSTUM:

      polyhedron_ptr->get_bounding_sphere(
         bounding_sphere_center,bounding_sphere_radius);
//      cout << "p = " << p 
//           << " bounding_sphere_center = " << bounding_sphere_center
//           << " radius = " << bounding_sphere_radius 
//           << endl;
      if (!camera_ptr->get_camera_frustum_ptr()->SphereInsideFrustum(
         bounding_sphere_center,bounding_sphere_radius)) continue;

// Test whether polyhedron's bounding box lies outside OBSFRUSTUM:

      bool inside_flag,outside_flag,intersects_flag;
      bounding_box bbox=polyhedron_ptr->get_bbox();
      camera_ptr->get_camera_frustum_ptr()->BoxInsideFrustum(
         &bbox,inside_flag,outside_flag,intersects_flag);
//      cout << "inside_flag = " << inside_flag
//           << " outside_flag = " << outside_flag
//           << " intersects_flag = " << intersects_flag << endl;
      if (outside_flag) continue;

      colorfunc::Color bbox_color=colorfunc::white;
      if (Polyhedron_ptr->get_ID()==
      PolyhedraGroup_ptr->get_selected_Graphical_ID())
      {
         bbox_color=colorfunc::red;
      }

      for (unsigned int f=0; f<polyhedron_ptr->get_n_faces(); f++)
      {
         face* face_ptr=polyhedron_ptr->get_face_ptr(f);
//         double dotproduct=pointing_dir.dot(face_ptr->get_normal());

//         cout << "f = " << f
//              << " normal = " << face_ptr->get_normal() 
//              << " dotproduct = " << dotproduct 
//              << endl;

// Only draw faces of polyhedron which are oriented towards the
// camera:

//         if (dotproduct >= 0) continue;

// Recall *face_ptr has both internal and external edges.  Do not draw
// internal edges:

         for (unsigned int e=0; e<face_ptr->get_n_edges(); e++)
         {
            edge* edge_ptr=&(face_ptr->get_edge(e));

            edge* polyhedron_edge_ptr=polyhedron_ptr->
               find_edge_given_vertices(
                  edge_ptr->get_V1(),edge_ptr->get_V2());
            if (polyhedron_edge_ptr->get_internal_edge_flag()) continue;
            

            vector<threevector> proj_vertices;
            threevector UVW;
            camera_ptr->project_XYZ_to_UV_coordinates(
               edge_ptr->get_V1().get_posn(),UVW);
            proj_vertices.push_back(UVW);
            camera_ptr->project_XYZ_to_UV_coordinates(
               edge_ptr->get_V2().get_posn(),UVW);
            proj_vertices.push_back(UVW);

// Check if sampled points along the 3D edge's 2D projected line segment lies
// within camera's view frustum.  If not, don't generate PolyLine
// corresponding to current projected edge:

            linesegment l(proj_vertices.front(),proj_vertices.back());
            const unsigned int n_fracs=11;
            int df=0.1;
            bool projected_edge_in_view_flag=false;
            for (unsigned int nf=0; nf<n_fracs; nf++)
            {
               double curr_frac=0+nf*df;
               threevector curr_UVW=l.get_frac_posn(curr_frac);
               if (camera_ptr->get_UV_bbox_ptr()->point_inside(
                  curr_UVW.get(0),curr_UVW.get(1)))
               {
                  projected_edge_in_view_flag=true;
                  break;
               }
               
            } // loop over index nf labeling fractional positions

            if (!projected_edge_in_view_flag) continue;

               cout << "proj_vertices = " << proj_vertices[0] << " "
                    << proj_vertices[1] << endl;
            PolyLine* PolyLine_ptr=
               PolyLinesGroup_ptr->generate_new_PolyLine(
                  Zero_vector,proj_vertices,colorfunc::get_OSG_color(
                     bbox_color));
            PolyLine_ptr->set_linewidth(2);

         } // loop over index e labeling edges 

// Project current polyhedron face into image plane:

         bool face_inside_imageplane_flag;
         vector<threevector> proj_vertices=camera_ptr->
            project_polyhedron_face_into_imageplane(
               f,polyhedron_ptr,face_inside_imageplane_flag);
         if (face_inside_imageplane_flag)
            some_faces_inside_imageplane_flag=true;

//         double alpha=0.25;
//         bool draw_border_flag=false;
//         PolygonsGroup_ptr->generate_translucent_Polygon(
//            bbox_color,proj_vertices,alpha,draw_border_flag);

      } // loop over index f labeling polyhedron faces

      project_polyhedron_point_cloud_into_imageplane(
         polyhedron_ptr,camera_ptr,DTED_ztwoDarray_ptr);

   } // loop over index p labeling Polyhedra
   
   return some_faces_inside_imageplane_flag;
}

// ---------------------------------------------------------------------
// Member function project_polyhedron_point_cloud_into_imageplane()
// takes in surface points and normal vectors for some polyhedron.
// This method projects each surface point lying on a face oriented
// towards the current OBSFRUSTUM's camera into its image plane.

double OBSFRUSTUM::project_polyhedron_point_cloud_into_imageplane(
   polyhedron* polyhedron_ptr,camera* camera_ptr,
   twoDarray* DTED_ztwoDarray_ptr)
{
   bounding_box UVW_bbox;
   return project_polyhedron_point_cloud_into_imageplane(
      polyhedron_ptr, camera_ptr,DTED_ztwoDarray_ptr,UVW_bbox);
}

double OBSFRUSTUM::project_polyhedron_point_cloud_into_imageplane(
   polyhedron* polyhedron_ptr,camera* camera_ptr,
   twoDarray* DTED_ztwoDarray_ptr,bounding_box& UVW_bbox)
{
//   cout << "inside OBSFRUSTUM::project_polyhedron_point_cloud_into_imageplane()" 
//        << endl;

   vector<pair<threevector,threevector> > pnts_normals=
      polyhedron_ptr->get_surfacepnts_normals();
//   cout << "pnts_normals.size() = " << pnts_normals.size() << endl;

   int n_pnts_facing_camera=0;
   int n_visible_points=0;
   double integrated_points_visibility=0;
   projected_polyhedron_points.clear();
   threevector pointing_dir(-camera_ptr->get_What());   
   for (unsigned int i=0; i<pnts_normals.size(); i++)
   {
      threevector n_hat(pnts_normals[i].second);

// First check whether current surface point lies on polyhedron face
// visible to camera:
         
      double dotproduct=pointing_dir.dot(n_hat);
      if (dotproduct >= 0) continue;

      threevector XYZ=pnts_normals[i].first;
      threevector UVW;
      camera_ptr->project_XYZ_to_UV_coordinates(XYZ,UVW);
      UVW_bbox.update_bounds(UVW);

// Next raytrace current surface point to check whether it's occluded
// by some foreground object:

      double delta_rho=1;		// meter
      double min_Z_ground=-1;	// meter
      n_pnts_facing_camera++;
      
      threevector e_hat=(camera_ptr->get_world_posn()-XYZ).unitvector();
//      cout << "e_hat = " << e_hat << endl;
      threevector impact_point;
      if (!camera_ptr->trace_individual_ray(
         XYZ,e_hat,min_Z_ground,delta_rho,DTED_ztwoDarray_ptr,impact_point))
      {
         if (UVW.get(0) >= Movie_ptr->get_minU() &&
             UVW.get(0) <= Movie_ptr->get_maxU() &&
             UVW.get(1) >= Movie_ptr->get_minV() &&
             UVW.get(1) <= Movie_ptr->get_maxV())
         {
            n_visible_points++;
            integrated_points_visibility += -dotproduct;
//            cout << "i = " << i << " U = " << UVW.get(0) 
//                 << " V = " << UVW.get(1) << endl;
            projected_polyhedron_points.push_back(UVW);
         }
      }
      else
      {
         occluded_ray_impact_points.push_back(impact_point);
      }
   } // loop over index i labeling polyhedron surface points

//   return n_visible_points;
   return integrated_points_visibility;
}

// ---------------------------------------------------------------------
// Member function 
 
double OBSFRUSTUM::integrate_projected_polyhedron_faces_area(
   polyhedron* polyhedron_ptr)
{
//   cout << "inside OBSFRUSTUM::integrate_projected_polyhedron_faces_area()" << endl;
   
   double integrated_proj_area=0;

   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   threevector pointing_dir=-camera_ptr->get_What();
   for (unsigned int f=0; f<polyhedron_ptr->get_n_faces(); f++)
   {
      face* face_ptr=polyhedron_ptr->get_face_ptr(f);
      double dotproduct=pointing_dir.dot(face_ptr->get_normal());

//         cout << "f = " << f
//              << " normal = " << face_ptr->get_normal() 
//              << " dotproduct = " << dotproduct 
//              << endl;

// Only draw faces of polyhedron which are oriented towards the
// camera:

      if (dotproduct >= 0) continue;

      bool face_inside_imageplane_flag;
      vector<threevector> proj_vertices=camera_ptr->
         project_polyhedron_face_into_imageplane(
            f,polyhedron_ptr,face_inside_imageplane_flag);
      polygon poly(proj_vertices);
      integrated_proj_area += poly.compute_area();
   } // loop over index f labeling polyhedron faces
   
   return integrated_proj_area;
}

// ---------------------------------------------------------------------
// Member function calculate_imageplane_score() first computes the
// bounding box within the OBSFRUSTUM's image plane that encloses the
// projected UV coordinates for a polyhedron's surface points.  After
// renormalizing this 2D area by the total UV area of the OBSFRUSTUM's
// image, this method computes the number of pixels within the 2D
// bbox.  It multiplies this pixel count by the input number of
// visible polyhedron surface points to yield a total score which
// quantifies how well a NYC skyscraper can be seen within the imageplane.

double OBSFRUSTUM::calculate_imageplane_score(
   double integrated_points_visibility,double integrated_proj_face_area,
   const bounding_box& UVW_bbox)
{
   cout << "inside OBSFRUSTUM::calculate_imageplane_score()" << endl;
//   cout << "UVW_bbox = " << UVW_bbox << endl;

// Integrated_points_visibility* factor in score function penalizes
// photos for which too much of building structure is occluded.

   double visible_umin=basic_math::max(
      Movie_ptr->get_minU(),UVW_bbox.get_xmin());
   visible_umin=basic_math::min(visible_umin,Movie_ptr->get_maxU());
   double visible_umax=basic_math::min(Movie_ptr->get_maxU(),
   UVW_bbox.get_xmax());
   visible_umax=basic_math::max(visible_umax,Movie_ptr->get_minU());

// Horiz[vert]_middle_frac factors in score function penalize photos
// for which too much of building structure lies away from the image
// center:

   double visible_uavg=0.5*(visible_umin+visible_umax);
   double uavg=0.5*(Movie_ptr->get_minU()+Movie_ptr->get_maxU());
   double horiz_middle_frac=1-fabs(uavg-visible_uavg)/uavg;

   double visible_vmin=basic_math::max(
      Movie_ptr->get_minV(),UVW_bbox.get_ymin());
   visible_vmin=basic_math::min(visible_vmin,Movie_ptr->get_maxV());
   double visible_vmax=basic_math::min(
      Movie_ptr->get_maxV(),UVW_bbox.get_ymax());
   visible_vmax=basic_math::max(visible_vmax,Movie_ptr->get_minV());

   double visible_vavg=0.5*(visible_vmin+visible_vmax);
   double vavg=0.5*(Movie_ptr->get_minV()+Movie_ptr->get_maxV());
   double vert_middle_frac=1-fabs(vavg-visible_vavg)/vavg;

// Visible_bbox_frac factor in score function penalizes photos for
// which too much of building structure lies outside of UV window.
// This factor discourages photos which are too zoomed-in...

   double visible_bbox_area=(visible_umax-visible_umin)*
      (visible_vmax-visible_vmin);
   double bbox_area=(UVW_bbox.get_xmax()-UVW_bbox.get_xmin())*
      (UVW_bbox.get_ymax()-UVW_bbox.get_ymin());
   double visible_bbox_frac=visible_bbox_area/bbox_area;

// Integrated_proj_face_area_frac factor in score function penalizes
// photos for which fractional area of building projected into image
// plane is small compared to total UV window:

   double total_imageplane_area=(Movie_ptr->get_maxU()-Movie_ptr->get_minU())*
      (Movie_ptr->get_maxV()-Movie_ptr->get_minV());
   double integrated_proj_face_area_frac=integrated_proj_face_area/
      total_imageplane_area;
   double imageplane_polyhedron_score=integrated_points_visibility*
      integrated_proj_face_area_frac*visible_bbox_frac*
      sqrt(horiz_middle_frac*vert_middle_frac);

//   cout << "integrated_points_visibility = "
//        << integrated_points_visibility << endl;
   cout << "imageplane_polyhedron_score = "
        << imageplane_polyhedron_score << endl;
   Movie_ptr->get_photograph_ptr()->set_score(imageplane_polyhedron_score);
   return imageplane_polyhedron_score;
}

// ==========================================================================
// Empire State Building video member functions
// ==========================================================================

// Member function crop_ESB_region_voxels is a one-time, specialized
// utility method which chips out an XYZP tile around the Empire State
// Building video region and writes it out to a TDP file.

void OBSFRUSTUM::crop_ESB_region_voxels(PointCloud* PointCloud_ptr)
{
   cout << "inside OBSFRUSTUM::crop_ESB_region_voxels()" << endl;
   
// Parallelogram bounding box for ESB car video:

   vector<threevector> V;
   V.push_back(threevector(585650.1250 , 4511183.000 , 13.05000210  ));
   V.push_back(threevector(585801.3750 , 4511470.500 , 21.74999809 ));
   V.push_back(threevector(585036.3125 , 4511881.000 , 9.900000572  ));
   V.push_back(threevector(584774.5625 , 4511386.000 , 8.459998131  ));
   parallelogram data_bbox(V);

   double min_X=POSITIVEINFINITY;
   double max_X=NEGATIVEINFINITY;
   double min_Y=POSITIVEINFINITY;
   double max_Y=NEGATIVEINFINITY;
   for (unsigned int i=0; i<4; i++)
   {
      min_X=basic_math::min(min_X,V[i].get(0));
      max_X=basic_math::max(max_X,V[i].get(0));
      min_Y=basic_math::min(min_Y,V[i].get(1));
      max_Y=basic_math::max(max_Y,V[i].get(1));
   }

   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;

   HiresDataVisitor* HiresDataVisitor_ptr=
      PointCloud_ptr->get_HiresDataVisitor_ptr();
   
   HiresDataVisitor_ptr->
      set_application_type(HiresDataVisitor::retrieve_XYZPs);
   PointCloud_ptr->get_DataNode_ptr()->accept(*HiresDataVisitor_ptr);

   vector<fourvector>* xyzp_pnt_ptr=new vector<fourvector>;
   xyzp_pnt_ptr->reserve(PointCloud_ptr->get_ntotal_leaf_vertices());

   vector<osg::Vec3> hires_XYZ=HiresDataVisitor_ptr->get_XYZ();
   osg::FloatArray* probs_ptr=HiresDataVisitor_ptr->get_probs_ptr();
   for (unsigned int n=0; n<hires_XYZ.size(); n++)
   {
      if (n%1000000==0) cout << n/1000000 << " " << flush;
      double X=hires_XYZ[n]._v[0];
      double Y=hires_XYZ[n]._v[1];
      if (X > min_X && X < max_X && Y > min_Y && Y < max_Y)
      {
         double Z=hires_XYZ[n]._v[2];
         double P=probs_ptr->at(n);
         xyzp_pnt_ptr->push_back(fourvector(X,Y,Z,P));
      }
   }
   cout << endl;
   cout << "xyzp_pnt.size() = " << xyzp_pnt_ptr->size() << endl;

   string tdp_filename="ESB_region.tdp";
   const string UTMzone="18N";		// NYC
   tdpfunc::write_xyzp_data(
      tdp_filename,UTMzone,PointCloud_ptr->get_zeroth_vertex(),xyzp_pnt_ptr);
   cout << "Finished writing tdp file = " << tdp_filename << endl;

//   string xyzp_filename="ESB_region.xyzp";
//   xyzpfunc::write_xyzp_data(xyzp_filename,xyzp_pnt_ptr,false);
//   cout << "Finished writing xyzp file = " << xyzp_filename << endl;
   delete xyzp_pnt_ptr;
}

// ---------------------------------------------------------------------
// Member function project_ground_info_into_imageplane iterates over
// all XYZ and P values within the input point cloud using a
// HiresDataVisitor.  It traces a ray from each XYZ point back to the
// OBSFRUSTUM's camera location.  If any small segment of the ray lies
// within a chimney where the point cloud's height map has a larger z
// value, this method declares the ray to be occluded at that chimney
// location.  If the ray is not occluded, binary ground mask
// information is projected from the point cloud XYZ voxel onto the
// corresponding UV pixel.  An alpha-blended version of the projected
// ground mask is output by this method to a PNG file.

void OBSFRUSTUM::project_ground_info_into_imageplane(
   PointCloud* PointCloud_ptr)
{
   cout << "inside OBSFRUSTUM::project_ground_info_imageplane()" << endl;

// First generate height array for occlusion ray tracing purposes:

   double delta_x=0.5;	// meter
   double delta_y=0.5;	// meter
   PointCloud_ptr->generate_ladarimage(delta_x,delta_y);
   twoDarray* ztwoDarray_ptr=
      PointCloud_ptr->get_ladarimage_ptr()->get_z2Darray_ptr();

//   string tdp_filename="ztwoDarray.tdp";
//   string UTMzone="18N";	// NYC
//   twoDarray* ptwoDarray_ptr=
//      PointCloud_ptr->get_ladarimage_ptr()->get_p2Darray_ptr();
//   tdpfunc::write_zp_twoDarrays(
//      tdp_filename,UTMzone,ztwoDarray_ptr,ptwoDarray_ptr,true);
   
// Parallelogram bounding box for ESB car video:

//   vector<threevector> V;
//   V.push_back(threevector(585650.1250 , 4511183.000 , 13.05000210  ));
//   V.push_back(threevector(585801.3750 , 4511470.500 , 21.74999809 ));
//   V.push_back(threevector(585036.3125 , 4511881.000 , 9.900000572  ));
//   V.push_back(threevector(584774.5625 , 4511386.000 , 8.459998131  ));
//   parallelogram data_bbox(V);

   HiresDataVisitor* HiresDataVisitor_ptr=
      PointCloud_ptr->get_HiresDataVisitor_ptr();
   
   HiresDataVisitor_ptr->
      set_application_type(HiresDataVisitor::retrieve_XYZPs);
   PointCloud_ptr->get_DataNode_ptr()->accept(*HiresDataVisitor_ptr);

   vector<osg::Vec3> hires_XYZ=HiresDataVisitor_ptr->get_XYZ();
   osg::FloatArray* probs_ptr=HiresDataVisitor_ptr->get_probs_ptr();
   camera* camera_ptr=Movie_ptr->get_camera_ptr();

// For binary mask generation purposes, we first initialize all pixels
// within the output video frame to black:

   twoDarray* binary_mask_twoDarray_ptr=new twoDarray(
      Movie_ptr->getWidth(),Movie_ptr->getHeight());
   binary_mask_twoDarray_ptr->initialize_values(0);

   int n_total_rays=0;
   int n_occluded_rays=0;
   for (unsigned int n=0; n<hires_XYZ.size(); n++)
   {
      if (n%100000==0) cout << n/100000 << " " << flush;
      double X=hires_XYZ[n]._v[0];
      double Y=hires_XYZ[n]._v[1];
      double Z=hires_XYZ[n]._v[2];
      double P=probs_ptr->at(n);

      double U,V;
      camera_ptr->project_XYZ_to_UV_coordinates(X,Y,Z,U,V);
      if (U > Movie_ptr->get_minU() && U < Movie_ptr->get_maxU() &&
          V > Movie_ptr->get_minV() && V < Movie_ptr->get_maxV() &&
          nearly_equal(P,0))
      {

// Form ray from XYZ to camera location.  Then check whether it is
// occluded:
         
         threevector XYZ(X,Y,Z);
         linesegment ray(XYZ,camera_ptr->get_world_posn());
         double length=ray.get_length();
         double d_length=0.5; // meters
         unsigned int n_steps=static_cast<int>(length/d_length);
         bool ray_occluded_flag=false;
         for (unsigned int i=0; i<n_steps && !ray_occluded_flag; i++)
         {
            threevector curr_posn=XYZ+i*d_length*ray.get_ehat();
            unsigned int px,py;
            if (ztwoDarray_ptr->point_to_pixel(curr_posn,px,py))
            {
               if (curr_posn.get(2) < ztwoDarray_ptr->get(px,py))
               {
                  ray_occluded_flag=true;
                  n_occluded_rays++;
               }
            }
         } // loop over index i labeling steps along ray
         n_total_rays++;

         if (!ray_occluded_flag)
         {
            unsigned int pu,pv;
            Movie_ptr->get_pixel_coords(U,V,pu,pv);

            int smudge_dist=4;
            for (int i=-smudge_dist; i<=smudge_dist; i++)
            {
               for (int j=-smudge_dist; j<=smudge_dist; j++)
               {
                  int curr_pu=pu+i;
                  int curr_pv=pv+j;
                  curr_pu=basic_math::max(0,curr_pu);
                  curr_pu=basic_math::min(
                     int(Movie_ptr->getWidth()-1),curr_pu);
                  curr_pv=basic_math::max(0,curr_pv);
                  curr_pv=basic_math::min(
                     int(Movie_ptr->getHeight()-1),curr_pv);
                  binary_mask_twoDarray_ptr->put(curr_pu,curr_pv,1);
               } // loop over index j labeling vertical smudging
            } // loop over index i labeling horizontal smudging
         } // !ray_occluded_flag conditional
         
      } // (U,V) inside movie frame && P==0 conditional
   } // loop over index n labeling XYZ points
   cout << endl;

//   cout << "n_total_rays = " << n_total_rays
//        << " n_occluded_rays = " << n_occluded_rays << endl;
//   cout << "occlusion frac = " 
//        << double(n_occluded_rays)/double(n_total_rays) << endl;

// Perform 2D recursive emptying and filling in order to eliminate
// small islands within video frame's binary mask:

   cout << "Recursive emptying black islands surrounded by white:" << endl;
//   int n_recursion_iters=60;
   int n_recursion_iters=70;
//   int n_recursion_iters=80;
   binaryimagefunc::binary_reverse(binary_mask_twoDarray_ptr);
   recursivefunc::recursive_empty(
      n_recursion_iters,binary_mask_twoDarray_ptr,false);

   cout << "Recursive emptying white islands surrounded by black:" << endl;
   binaryimagefunc::binary_reverse(binary_mask_twoDarray_ptr);
   recursivefunc::recursive_empty(
      n_recursion_iters,binary_mask_twoDarray_ptr,false);

   int n_ground_pixels=imagefunc::count_pixels_above_zmin(
      0.5,binary_mask_twoDarray_ptr);
   int n_total_pixels=imagefunc::count_pixels_above_zmin(
      -0.5,binary_mask_twoDarray_ptr);
   cout << "Ground pixel fraction = " 
        << double(n_ground_pixels)/double(n_total_pixels) << endl;

// Generate alpha-blended version of binary ground mask within video
// frame:

   cout << "Transfering binary mask results to Movie" << endl;
   for (unsigned int pu=0; pu<Movie_ptr->getWidth(); pu++)
   {
      for (unsigned int pv=0; pv<Movie_ptr->getHeight(); pv++)
      {
         if (binary_mask_twoDarray_ptr->get(pu,pv)==1)
         {
            int R,G,B;
            Movie_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
            double r=R/255.0;
            double g=G/255.0;
            double b=B/255.0;
            double h,s,v;
            colorfunc::RGB_to_hsv(r,g,b,h,s,v);

            const double overlap_hue=0;
            double hnew=overlap_hue;
            double snew=(1-0.5)*s+0.5;
            double vnew=(1-0.1)*v+0.1;
//               double vnew=v;
            double rnew,gnew,bnew;
            colorfunc::hsv_to_RGB(hnew,snew,vnew,rnew,gnew,bnew);
            R=int(255*rnew);
            G=int(255*gnew);
            B=int(255*bnew);
            R=basic_math::min(255,R);
            G=basic_math::min(255,G);
            B=basic_math::min(255,B);
            Movie_ptr->set_pixel_RGB_values(pu,pv,R,G,B);

//            Movie_ptr->set_pixel_RGB_values(pu,pv,255,255,255);
         }
         else
         {
//            Movie_ptr->set_pixel_RGB_values(pu,pv,0,0,0);
         }
      } // loop over pv index
   } // loop over pu index

// Write alpha-blended ground mask to output PNG file:

   Movie_ptr->set_image();

   string directory_name=filefunc::getdirname(Movie_ptr->get_video_filename());
   string base_filename=filefunc::getbasename(Movie_ptr->get_video_filename());
   string output_filename=directory_name+"modified_"
      +stringfunc::prefix(base_filename)+".png";
   
// Flip image vertically prior to writing it to output file.
// Afterwards, flip it again:

   Movie_ptr->get_image_ptr()->flipVertical();

   if (osgDB::writeImageFile(*(Movie_ptr->get_image_ptr()),output_filename))
   {
      cout << "Modified image written to " << output_filename << endl;
   }

   Movie_ptr->get_image_ptr()->flipVertical();
}

// ==========================================================================
// UAV OBSFRUSTA member functions
// ==========================================================================

// Member function set_UAV_OBSFRUSTUM_colorings takes in an ID for
// some UAV MODEL.  In order to readily distinguish up to 4 UAV's,
// this method assigns different colors to the UAV's OBSFRUSTUM based
// upon the input ID modulo 4.

void OBSFRUSTUM::set_UAV_OBSFRUSTUM_colorings(int UAV_ID)
{
//   cout << "inside OBSFRUSTUM::set_UAV_OBSFRUSTUM_colorings()" << endl;

// Note added on 8/22/08: In order to maximally differentiate between
// greyscale Constant Hawk video chips (used as surrogates for genuine
// UAV video) and colored satellite imagery background, set UAV
// OBSFRUSTUM volume alpha to zero:

   double volume_alpha=0.0;	
//   double volume_alpha=0.05;
//   double volume_alpha=0.2;

   vector<colorfunc::Color> colors;
   colors.push_back(colorfunc::cyan);
   colors.push_back(colorfunc::green);
   colors.push_back(colorfunc::yellow);
   colors.push_back(colorfunc::blue);
   colors.push_back(colorfunc::orange);
   colors.push_back(colorfunc::purple);
   colors.push_back(colorfunc::brick);
   colors.push_back(colorfunc::pink);
   colors.push_back(colorfunc::blgr);
   colors.push_back(colorfunc::yegr);

   int color_scheme=UAV_ID%(int(colors.size()));
//   cout << "UAV_ID = " << UAV_ID << endl;
//   cout << "color_scheme = " << color_scheme << endl;

   colorfunc::Color SideEdgeColor,ZplaneEdgeColor,VolumeColor;
   SideEdgeColor=ZplaneEdgeColor=VolumeColor=colors[color_scheme];
   set_color(SideEdgeColor,ZplaneEdgeColor,VolumeColor,volume_alpha);
}

// ==========================================================================
// DTED raytracing member functions
// ==========================================================================

// Member function compute_latlong_pnts_inside_footprint() first
// computes the OBSFRUSTUM's zplane projection.  It then determines
// which latlong tiles lie inside the projected frustum's z-plane
// trapezoid.  This method returns an STL vector of
// threevectors(longitude,latitude) containing the intercepted latlong
// tiles.

void OBSFRUSTUM::compute_latlong_pnts_inside_footprint(
   double max_raytrace_range,double raytrace_cellsize,
   vector<threevector>& latlong_points_inside_polygon)
{
   double theta_min,theta_max;
   threevector apex;
   polygon zplane_triangle;

   compute_latlong_pnts_inside_footprint(
      max_raytrace_range,raytrace_cellsize,theta_min,theta_max,apex,
      zplane_triangle,latlong_points_inside_polygon);
}

void OBSFRUSTUM::compute_latlong_pnts_inside_footprint(
   double max_raytrace_range,double raytrace_cellsize,
   double& theta_min,double& theta_max,threevector& apex,
   polygon& zplane_triangle,
   vector<threevector>& latlong_points_inside_polygon)
{
//   cout << "inside OBSFRUSTUM::compute_latlong_pnts_inside_footprint()" 
//        << endl;
//   cout << "max_raytrace_range = " << max_raytrace_range << endl;

   bounding_box zplane_face_bbox;
   vector<geopoint> triangle_vertices;

   extract_viewfrustum_components(
      TilesGroup_ptr->get_northern_hemisphere_flag(),
      TilesGroup_ptr->get_specified_UTM_zonenumber(),
      apex,zplane_face_bbox,zplane_triangle,
      triangle_vertices,max_raytrace_range,theta_min,theta_max);

//   cout << "apex = " << apex << endl;
//   cout << "zplane_face_bbox = " << zplane_face_bbox << endl;

// On 7/20/09, we discovered to our horror that the logic described
// within the following comment is badly FLAWED!  So we have discarded
// this faulty logic which was originally implemented for speed
// purposes only...

// If a ground bbox has been specified, only read in those geotif
// tiles which immediately surround the bbox.  If the bbox doesn't lie
// within the current OBSFRUSTUM, simply return an occlusion
// percentage of zero and don't bother performing expensive
// raytracing.  !!! Ignore this BAD comment !!!

   latlong_points_inside_polygon=
      TilesGroup_ptr->individual_latlong_tiles_intercepting_polygon(
         triangle_vertices[0],triangle_vertices[1],triangle_vertices[2],
         apex,max_raytrace_range);
//   cout << "latlong_points_inside_polygon.size() = "
//        << latlong_points_inside_polygon.size() << endl;
}

// ---------------------------------------------------------------------
// Member function load_DTED_height_data() instantiates
// *DTED_ztwoDarray_ptr within TilesGroup class and creates a local
// copy of the TilesGroup pointer.  It then loads *DTED_ztwoDarray_ptr
// with height data read from geotif Z-tiles.

twoDarray* OBSFRUSTUM::load_DTED_height_data(
   double max_raytrace_range,double raytrace_cellsize)
{
   double theta_min,theta_max;
   threevector apex;
   vector<threevector> latlong_points_inside_polygon;
   polygon zplane_triangle;

   return load_DTED_height_data(
      max_raytrace_range,raytrace_cellsize,theta_min,theta_max,
      apex,latlong_points_inside_polygon,zplane_triangle);
}

twoDarray* OBSFRUSTUM::load_DTED_height_data(
   double max_raytrace_range,double raytrace_cellsize,
   double& theta_min,double& theta_max,threevector& apex,
   std::vector<threevector>& latlong_points_inside_polygon,
   polygon& zplane_triangle)
{
//   cout << "inside OBSFRUSTUM::load_DTED_height_data()" << endl;
//   cout << "max_raytrace_range = " << max_raytrace_range << endl;

   compute_latlong_pnts_inside_footprint(
      max_raytrace_range,raytrace_cellsize,theta_min,theta_max,apex,
      zplane_triangle,latlong_points_inside_polygon);

   double delta_x=raytrace_cellsize;
   double delta_y=raytrace_cellsize;
   twoDarray* DTED_ztwoDarray_ptr=TilesGroup_ptr->
      load_DTED_subtiles_overlapping_polygon_into_ztwoDarray(
         latlong_points_inside_polygon,delta_x,delta_y);

   return DTED_ztwoDarray_ptr;
}

twoDarray* OBSFRUSTUM::load_DTED_height_data(
   double raytrace_cellsize,
   const std::vector<threevector>& latlong_points_inside_polygon)
{
//   cout << "inside OBSFRUSTUM::load_DTED_height_data()" << endl;
//   cout << "max_raytrace_range = " << max_raytrace_range << endl;
//   cout << "latlong_points_inside_polygon.size() = "
//        << latlong_points_inside_polygon.size() << endl;

   twoDarray* DTED_ztwoDarray_ptr=NULL;
   if (get_ladar_height_data_flag())
   {
      DTED_ztwoDarray_ptr=TilesGroup_ptr->
         load_ladar_height_data_into_ztwoDarray();
   }
   else
   {
      double delta_x=raytrace_cellsize;
      double delta_y=raytrace_cellsize;
      DTED_ztwoDarray_ptr=TilesGroup_ptr->
         load_DTED_subtiles_overlapping_polygon_into_ztwoDarray(
            latlong_points_inside_polygon,delta_x,delta_y);
   }
   return DTED_ztwoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function raytrace_occluded_ground_region() checks if member
// *DTED_ptwoDarray_ptr is filled with previously calculated LOS
// occlusion information.  If not, it extracts the current
// OBSFRUSTUM's viewing components and computes the triangle which it
// projects onto the ground z-plane.  This method then reads in DTED
// height information from pre-calculated geotif tiles into the
// triangular region.  After masking out the region in
// *DTED_ptwoDarray_ptr lying outside the ground triangle, this method
// loops over every remaining pixel and shoots a ray from its ground
// location towards the OBSFRUSTUM's apex.  The heights of line
// segment chunks along the ray are compared with corresponding DTED
// heights.  If the former is less than the latter, we assume that the
// ray must have passed through some opaque surface and that the ray
// is occluded.  The contents of *DTED_ptwoDarray_ptr are written out
// and averaged LOS probability values are also updated at the end of
// this method.  This method also returns the occlusion percentage.
// If the percentage was previously calculated, -1 is returned.

double OBSFRUSTUM::raytrace_occluded_ground_region(
   double min_raytrace_range,double max_raytrace_range,
   double raytrace_cellsize,unsigned int& n_total_rays,
   unsigned int& n_occluded_rays)
{
//   cout << "inside OBSFRUSTUM::raytrace_occluded_ground_region()" << endl;
//   cout << "OBSFRUSTM ID = " << get_ID() << endl;
//   cout << "raytrace_cellsize = " << raytrace_cellsize << endl;
//   cout << "max_raytrace_range = " << max_raytrace_range << endl;

   double occlusion_percentage=-1;
   raster_parser RasterParser;
   if (import_DTED_ptwoDarray_contents(
      RasterParser,TilesGroup_ptr->get_geotif_Ptiles_subdir())) 
   {
      return occlusion_percentage;
   }

   threevector apex;
   if (viewing_pyramid_above_zplane_ptr != NULL)
   {
      apex=viewing_pyramid_above_zplane_ptr->get_apex().get_posn();
   }
   else if (get_Movie_ptr()->get_photograph_ptr() != NULL)
   {
      Movie* Movie_ptr=get_Movie_ptr();
      photograph* photograph_ptr=Movie_ptr->get_photograph_ptr();
      camera* camera_ptr=photograph_ptr->get_camera_ptr();
      apex=camera_ptr->get_world_posn();
   }
//   cout << "apex = " << apex << endl;

   twoDarray* DTED_ztwoDarray_ptr=NULL;
   polygon zplane_triangle;
   vector <threevector> latlong_points_inside_polygon;

   if (get_ladar_height_data_flag())
   {
      double delta_x=raytrace_cellsize;
      double delta_y=raytrace_cellsize;
      DTED_ztwoDarray_ptr=TilesGroup_ptr->
         load_ladar_height_data_into_ztwoDarray(delta_x,delta_y);

      vector<threevector> corners;
      corners.push_back(apex);
      threevector rhat_min(
         cos(FOV_triangle_min_az),sin(FOV_triangle_min_az),0);
      threevector rhat_max(
         cos(FOV_triangle_max_az),sin(FOV_triangle_max_az),0);
      corners.push_back(apex+rhat_min*max_raytrace_range);
      corners.push_back(apex+rhat_max*max_raytrace_range);
      zplane_triangle=polygon(corners);
   }
   else
   {

// Recall TilesGroup class takes care of memory management for
// *DTED_ztwoDarray_ptr and *reduced_DTED_ztwoDarray_ptr.  So the
// following variables are just local copies of TilesGroup member
// pointer.  We do not need to delete *DTED_ztwoDarray_ptr or
// *reduced_DTED_ztwoDarray_ptr in this method.  

      DTED_ztwoDarray_ptr=load_DTED_height_data(
         max_raytrace_range,raytrace_cellsize,
         FOV_triangle_min_az,FOV_triangle_max_az,apex,
         latlong_points_inside_polygon,zplane_triangle);
   }
//   cout << "FOV_triangle_min_az = " << FOV_triangle_min_az*180/PI << endl;
//   cout << "FOV_triangle_max_az = " << FOV_triangle_max_az*180/PI << endl;
//   cout << "zplane_triangle = " << zplane_triangle << endl;
   
   twoDarray* reduced_DTED_ztwoDarray_ptr=TilesGroup_ptr->
      generate_reduced_DTED_ztwoDarray();

   delete get_DTED_ptwoDarray_ptr();
   DTED_ptwoDarray_ptr=new twoDarray(DTED_ztwoDarray_ptr);
   ray_tracer_ptr->set_DTED_ptwoDarray_ptr(DTED_ptwoDarray_ptr);

   get_DTED_ptwoDarray_ptr()->initialize_values(-1);

// On 3/19/09, we empirically found that it is MUCH faster to use fast
// rasterization filling algorithms to mark the OBSFRUSTUM's ground
// z-plane trapezoid than to loop over each pixel and individually test
// whether it lies inside a convex polygon:

   const double interior_intensity_value=-0.5;
   if (FOV_triangle_max_az-FOV_triangle_min_az > 180*PI/180)
   {
      drawfunc::color_triangle_exterior(
         zplane_triangle,interior_intensity_value,get_DTED_ptwoDarray_ptr());
   }
   else
   {
      drawfunc::color_triangle_interior(
         zplane_triangle,interior_intensity_value,get_DTED_ptwoDarray_ptr());
   }

// Loop over points inside z-plane trapezoid and find maximal terrain
// values within 1-degree angular sectors:

//   cout << "Finding maximal terrain heights in 1-degree angular sectors:"
//        << endl;
   double max_interior_Z=NEGATIVEINFINITY;
   double sqrd_min_range=sqr(min_raytrace_range);
   double sqrd_max_range=sqr(max_raytrace_range);
   unsigned int mdim=DTED_ztwoDarray_ptr->get_mdim();
   unsigned int ndim=DTED_ztwoDarray_ptr->get_ndim();

   cout << "Computing terrain points which possibly lie within field-of-view:"
        << endl;

//   const double elev_max=10.13*PI/180.0;
//   const double max_tan_elev=tan(elev_max);
   threevector candidate_target_point;
   for (unsigned int px=0; px<mdim; px++)
   {
//      cout << "px = " << px << " " << flush;
      for (unsigned int py=0; py<ndim; py++)
      {
         if (nearly_equal(get_DTED_ptwoDarray_ptr()->get(px,py),
                          interior_intensity_value))
         {
            DTED_ztwoDarray_ptr->pixel_to_threevector(
               px,py,candidate_target_point);

// FAKE FAKE:  Friday, Jun 3, 2011 at 12:29 pm
// Comment out next lines for WISP raytracing around FOB Blessing
// only...

            if (get_ladar_height_data_flag())
            {
               max_interior_Z=basic_math::max(
                  max_interior_Z,DTED_ztwoDarray_ptr->get(px,py));
            }
            else
            {

/*
// FAKE FAKE:  Friday, Jun 3, 2011 at 1:57 pm
// Check elevation constraints for WISP-360 sensor:
// Recall FOV_u = 36 and FOV_v = 20.26 degrees for each of the 10 WISP
// panels....

               threevector ray=candidate_target_point-apex;
               double rho=sqrt(sqr(ray.get(0))+sqr(ray.get(1)));
               double z=ray.get(2);
               if (fabs(z/rho) > max_tan_elev)
               {
                  get_DTED_ptwoDarray_ptr()->put(px,py,-1);
               }
*/
        
               double curr_sqrd_range=
                  (apex-candidate_target_point).sqrd_magnitude();

// If current ground point's range lies outside visible interval,
// reset ptwoDarray to -1 sentinel value indicating invisibility to
// sensor:

               if ( (curr_sqrd_range < sqrd_min_range) ||
                    (curr_sqrd_range > sqrd_max_range) )
               {
//               double ratio=sqrt(curr_sqrd_range/sqrd_min_range);
//               cout << "range/min_range = " << ratio << endl;
//               cout << "r = " << sqrt(curr_sqrd_range) << endl;
//               cout << "min_range = " << sqrt(sqrd_min_range) 
//                    << " max_range = " << sqrt(sqrd_max_range) << endl;

                  get_DTED_ptwoDarray_ptr()->put(px,py,-1);
               }
               else
               {
                  max_interior_Z=basic_math::max(
                     max_interior_Z,DTED_ztwoDarray_ptr->get(px,py));
               }
            } // get_ladar_height_data_flag() conditional

// If a ground bounding box has been defined, check whether candidate
// target point lies inside.  If not, reset ptwoDarray to -1 sentinel
// value:

            if (ground_bbox_ptr != NULL)
            {
               if (!ground_bbox_ptr->point_inside(
                      candidate_target_point.get(0),
                      candidate_target_point.get(1)))
               {
                  get_DTED_ptwoDarray_ptr()->put(px,py,-1);
               }
            }
            
         } // DTED_ptwoDarray(px,py)==interior_intensity_value conditional
      } // loop over py index
   } // loop over px index
   cout << endl;
   cout << "max_interior_Z = " << max_interior_Z << endl;

// Loop over lattice defined by zplane_face bbox.  Raytrace each
// lattice site back to apex:

   double ds;
   if (!get_ladar_height_data_flag())
   {
      ds=2*basic_math::min(DTED_ztwoDarray_ptr->get_deltax(),
         DTED_ztwoDarray_ptr->get_deltay());
//    ds=SQRT_TWO*basic_math::min(DTED_ztwoDarray_ptr->get_deltax(),
//                          DTED_ztwoDarray_ptr->get_deltay());
   }
   else
   {
      ds=0.5;	// meter
   }
//   cout << "ds = " << ds << endl;

   cout << "Raytracing all candidate ground region points:" << endl;
   for (unsigned int px=0; px<mdim; px++)
   {
      if (px%100==0) cout << px << " " << flush;
      for (unsigned int py=0; py<ndim; py++)
      {

// Check whether ground point lies inside OBSFRUSTUM's z-plane
// trapezoid.  If not, don't perform expensive raytrace operation on it:
            
         if (nearly_equal(get_DTED_ptwoDarray_ptr()->get(px,py),
                          interior_intensity_value))
         {
            int tracing_result=ray_tracer_ptr->trace_individual_ray(
               px,py,apex,max_interior_Z,max_raytrace_range,ds,
               DTED_ztwoDarray_ptr,reduced_DTED_ztwoDarray_ptr);

            if (tracing_result==0)
            {
               n_total_rays++;
               n_occluded_rays++;
            }
            else if (tracing_result==1)
            {
               n_total_rays++;
            }
         } // DTED_ptwoDarray(px,py)==interior_intensity_value conditional
         
      } // loop over py index
   } // loop over px index
   cout << endl;

//   cout << "n_total_rays = " << n_total_rays << endl;
//   cout << "n_occluded_rays = " << n_occluded_rays << endl;

   occlusion_percentage=0;
   if (n_total_rays > 1)
   {
      occlusion_percentage=double(n_occluded_rays)/double(n_total_rays)*100;
   }
   cout << "Occlusion percentage = " << occlusion_percentage << endl;
 
//   cout << "Before export_DTED_ptwoDarray_contents()" << endl;
   export_DTED_ptwoDarray_contents(
      RasterParser,TilesGroup_ptr->get_geotif_Ptiles_subdir(),
      TilesGroup_ptr->get_northern_hemisphere_flag(),
      TilesGroup_ptr->get_specified_UTM_zonenumber());

   if (!get_ladar_height_data_flag())
   {
      TilesGroup_ptr->update_avg_LOS_tiles(
         raytrace_cellsize,raytrace_cellsize,latlong_points_inside_polygon,
         get_DTED_ptwoDarray_ptr(),AnimationController_ptr);
   }

//   if (get_ladar_height_data_flag())
//   {
//      compute_occlusion_frac_vs_maxrange(apex);
//   }

   return occlusion_percentage;
}

// ---------------------------------------------------------------------
// Member function raytrace_ground_targets() takes in a set of ground
// target positions.  It first instantiates the current OBSFRUSTUM's
// zplane trapezoid and marks terrain points inside the trapezoid
// within *DTED_ptwoDarray_ptr.  This method then raytraces each
// ground target lying inside the zplane trapezoid and returns the
// number of ground targets visible to the aerial sensor.

int OBSFRUSTUM::raytrace_ground_targets(
   const vector<twovector>& target_posns,double max_ground_Z,
   double max_raytrace_range,double min_raytrace_range,double ds,
   twoDarray* DTED_ztwoDarray_ptr,twoDarray* DTED_ptwoDarray_ptr,
   twoDarray* reduced_DTED_ztwoDarray_ptr,
   vector<pair<int,threevector> >& target_tracing_result)
{
//   cout << "inside OBSFRUSTUM::raytrace_ground_targets()" << endl;
//   cout << "ds = " << ds << endl;

   double theta_min,theta_max; // angles of triangle wedge sides
   threevector apex;
   bounding_box zplane_face_bbox;
   polygon zplane_triangle;
   vector<geopoint> triangle_vertices;
   extract_viewfrustum_components(
      TilesGroup_ptr->get_northern_hemisphere_flag(),
      TilesGroup_ptr->get_specified_UTM_zonenumber(),
      apex,zplane_face_bbox,zplane_triangle,triangle_vertices,
      max_raytrace_range,theta_min,theta_max);

//   cout << "apex = " << apex << endl;
//   cout << "zplane_face_bbox = " << zplane_face_bbox << endl;

// Initialize *ptwoDarray_ptr values to -1:

   DTED_ptwoDarray_ptr->initialize_values(-1);

// On 3/19/09, we empirically found that it is MUCH faster to use fast
// rasterization filling algorithms to mark the OBSFRUSTUM's ground
// z-plane trapezoid than to loop over each pixel and individually test
// whether it lies inside a convex polygon:

   const double interior_intensity_value=-0.5;
   drawfunc::color_triangle_interior(
      zplane_triangle,interior_intensity_value,DTED_ptwoDarray_ptr);

// Loop over all ground targets and determine which lie inside
// OBSFRUSTUM's z-plane trapezoid.  Ray trace such ground targets back
// to apex:

   unsigned int n_ground_targets=target_posns.size();
   int n_visible_targets=0;
   int n_occluded_targets=0;
   target_tracing_result.clear();
   for (unsigned int t=0; t<n_ground_targets; t++)
   {
      int tracing_result=-1;
      threevector occluded_ray_posn(Zero_vector);
      unsigned int px,py;
      if (DTED_ptwoDarray_ptr->point_to_pixel(
             target_posns[t].get(0),target_posns[t].get(1),px,py))
      {
         if (nearly_equal(DTED_ptwoDarray_ptr->get(px,py),
                          interior_intensity_value))
         {
            tracing_result=ray_tracer_ptr->trace_individual_ray(
               apex,target_posns[t].get(0),target_posns[t].get(1),
               max_ground_Z,max_raytrace_range,
               min_raytrace_range,ds,
               DTED_ztwoDarray_ptr,reduced_DTED_ztwoDarray_ptr,
               occluded_ray_posn);
//            cout << "t = " << t 
//                 << " tracing_result = " << tracing_result 
//                 << " occluded_ray_posn = " << occluded_ray_posn 
//                 << endl;

            if (tracing_result==0)
            {
               n_occluded_targets++;
            }
            if (tracing_result==1)
            {
               n_visible_targets++;
            }
         } // px,py==interior_intensity_value conditional
      } // DTED_ptwoDarray_ptr->point_to_pixel() conditional

      target_tracing_result.push_back(
         pair<int,threevector>(tracing_result,occluded_ray_posn));
      
   } // loop over index t labeling ground targets

//   cout << "n_targets = " << target_posns.size() << endl;
//   cout << "n_occluded_targets = " << n_occluded_targets << endl;
//   cout << "n_visible_targets = " << n_visible_targets << endl;

//   outputfunc::enter_continue_char();

   return n_visible_targets;
}

// ---------------------------------------------------------------------
// This overloaded version of raytrace_ground_targets() is intended
// for tracing moving ground targets.  It calls
// TilesGroup::get_ladar_z_given_xy() and does NOT use
// DTED_ztwoDarray_ptr.  We wrote this version for the red actor path
// problem of TOC11.

int OBSFRUSTUM::raytrace_ground_targets(
   const vector<twovector>& target_posns,double max_ground_Z,
   double max_raytrace_range,double min_raytrace_range,double ds,
   vector<pair<int,threevector> >& target_tracing_result)
{
//   cout << "inside ladar version of OBSFRUSTUM::raytrace_ground_targets()" 
//        << endl;
//   cout << "ds = " << ds << endl;

   pyramid* viewing_pyramid_above_zplane_ptr=
      get_viewing_pyramid_above_zplane_ptr();
   threevector apex=viewing_pyramid_above_zplane_ptr->get_apex().get_posn();
//   cout << "apex = " << apex << endl;

// Loop over all ground targets and determine which lie inside
// OBSFRUSTUM's z-plane trapezoid.  Ray trace such ground targets back
// to apex:

   unsigned int n_ground_targets=target_posns.size();
//   cout << "n_ground_targets = " << n_ground_targets << endl;
   
   int n_visible_targets=0;
   int n_occluded_targets=0;
   target_tracing_result.clear();
   for (unsigned int t=0; t<n_ground_targets; t++)
   {
      int tracing_result=-1;
      threevector occluded_ray_posn(Zero_vector);

      tracing_result=ray_tracer_ptr->trace_individual_ray(
         apex,target_posns[t].get(0),target_posns[t].get(1),
         max_ground_Z,max_raytrace_range,min_raytrace_range,ds,
         occluded_ray_posn);
//      cout << "t = " << t 
//           << " tracing_result = " << tracing_result 
//           << " occluded_ray_posn = " << occluded_ray_posn 
//           << endl;

      if (tracing_result==0)
      {
         n_occluded_targets++;
      }
      if (tracing_result==1)
      {
         n_visible_targets++;
      }

      target_tracing_result.push_back(
         pair<int,threevector>(tracing_result,occluded_ray_posn));
      
   } // loop over index t labeling ground targets

//   cout << "n_targets = " << target_posns.size() << endl;
//   cout << "n_occluded_targets = " << n_occluded_targets << endl;
//   cout << "n_visible_targets = " << n_visible_targets << endl;

//   outputfunc::enter_continue_char();

   return n_visible_targets;
}

// ---------------------------------------------------------------------
// Member function omni_raytrace_ground_targets() takes in a set of
// ground target positions as well as a single observation position.
// It traces rays from each of the ground targets to the observation
// position.  Raytracing results are returned within STL vector
// target_tracing_result.

int OBSFRUSTUM::omni_raytrace_ground_targets(
   const threevector& obs_posn,const vector<twovector>& target_posns,
   double max_ground_Z,double max_raytrace_range,double min_raytrace_range,
   double ds,twoDarray* DTED_ztwoDarray_ptr,
   twoDarray* reduced_DTED_ztwoDarray_ptr,
   vector<pair<int,threevector> >& target_tracing_result)
{
//   cout << "inside OBSFRUSTUM::omni_raytrace_ground_targets()" << endl;
//   cout << "ds = " << ds << endl;
//   cout << "obs_posn = " << obs_posn << endl;

// Loop over all ground targets and raytrace them back to obs_posn:

   unsigned int n_ground_targets=target_posns.size();
   int n_visible_targets=0;
   int n_occluded_targets=0;
   target_tracing_result.clear();
   for (unsigned int t=0; t<n_ground_targets; t++)
   {
      int tracing_result=-1;
      threevector occluded_ray_posn(Zero_vector);

      tracing_result=ray_tracer_ptr->trace_individual_ray(
         obs_posn,target_posns[t].get(0),target_posns[t].get(1),
         max_ground_Z,max_raytrace_range,min_raytrace_range,ds,
         DTED_ztwoDarray_ptr,reduced_DTED_ztwoDarray_ptr,occluded_ray_posn);
//      cout << "t = " << t 
//           << " tracing_result = " << tracing_result 
//           << " occluded_ray_posn = " << occluded_ray_posn 
//           << endl;

      if (tracing_result==0)
      {
         n_occluded_targets++;
      }
      if (tracing_result==1)
      {
         n_visible_targets++;
      }

      target_tracing_result.push_back(
         pair<int,threevector>(tracing_result,occluded_ray_posn));
      
   } // loop over index t labeling ground targets

//   cout << "n_targets = " << target_posns.size() << endl;
//   cout << "n_occluded_targets = " << n_occluded_targets << endl;
//   cout << "n_visible_targets = " << n_visible_targets << endl;
//   outputfunc::enter_continue_char();

   return n_visible_targets;
}

// ---------------------------------------------------------------------
// Member function import_DTED_ptwoDarray_contents() reads in a DTED
// LOS geotif file corresponding to the current frame number.  After
// converting the quantized 2-byte integers back to the continuous
// range [-1.0, +1.0], it reinstantiates and fills member twoDarray
// *DTED_ptwoDarray_ptr with the reconstructed probabilities.

bool OBSFRUSTUM::import_DTED_ptwoDarray_contents(
   raster_parser& RasterParser,string geotif_Ptiles_subdir)
{
//   cout << "inside OBSFRUSTUM::import_DTED_ptwoDarray_contents()" << endl;
   
// As of 2/6/11, we assume that any input ladar map is sufficiently
// small that it can be fully loaded into DTED_ztwoDarray_ptr just
// once:

   if (get_ladar_height_data_flag() && 
       get_DTED_ptwoDarray_ptr() != NULL) return true;

   string geotiff_filename=get_geotiff_filename(geotif_Ptiles_subdir);
   if (!filefunc::fileexist(geotiff_filename)) return false;

   if (!RasterParser.open_image_file(geotiff_filename)) return false;

   int channel_ID=0; 
   RasterParser.fetch_raster_band(channel_ID);

   delete get_DTED_ptwoDarray_ptr();
   DTED_ptwoDarray_ptr=new twoDarray(RasterParser.get_ztwoDarray_ptr());
   ray_tracer_ptr->set_DTED_ptwoDarray_ptr(DTED_ptwoDarray_ptr);

   RasterParser.read_raster_data(get_DTED_ptwoDarray_ptr());

   const double p_min=-1;
   const double p_max=1;
   RasterParser.convert_GUInts_to_doubles(
      p_min,p_max,get_DTED_ptwoDarray_ptr());

//   cout << "imported *DTED_ptwoDarray_ptr = " << *get_DTED_ptwoDarray_ptr() 
//        << endl;
   return true;
}

// ---------------------------------------------------------------------
// Member function extract_viewfrustum_components() returns the apex
// and z-plane trapezoidal face polygon for the viewing pyramid
// located above the zplane.  It also returns a bounding box enclosing
// the zplane trapezoid.  Finally, this method computes and returns
// the geopoints corresponding to the apex projected into the z-plane
// and the two radially extremal trapezoid vertices.

void OBSFRUSTUM::extract_viewfrustum_components(
   bool northern_hemisphere_flag,int specified_UTM_zonenumber,
   threevector& apex,bounding_box& zplane_face_bbox,
   polygon& zplane_triangle,
   vector<geopoint>& triangle_vertices,
   double max_raytrace_range,double& theta_min,double& theta_max)
{
//   cout << "inside OBSFRUSTUM::extract_viewfrustum_components()" << endl;

   pyramid* viewing_pyramid_above_zplane_ptr=
      get_viewing_pyramid_above_zplane_ptr();
//   cout << "viewing_pyramid_above_zplane_ptr = "
//        << viewing_pyramid_above_zplane_ptr << endl;
   
//   cout << "zplane_face = " << *(viewing_pyramid_above_zplane_ptr->
//                                 get_zplane_face_ptr()) << endl;
//   cout << "max_raytrace_range = " << max_raytrace_range << endl;

   apex=viewing_pyramid_above_zplane_ptr->get_apex().get_posn();
   face* zplane_face_ptr=viewing_pyramid_above_zplane_ptr->
      get_zplane_face_ptr();
//   cout << "*zplane_face_ptr = " << zplane_face_ptr << endl;

// Compute bounding box around OBSFRUSTUM's zplane_face:

   double xmin=apex.get(0)-max_raytrace_range;
   double xmax=apex.get(0)+max_raytrace_range;
   double ymin=apex.get(1)-max_raytrace_range;
   double ymax=apex.get(1)+max_raytrace_range;
   
// Compute distance from each zplane face vertex to the pyramid's
// apex.  Keep two vertices whose distances are maximal.

   vector<threevector> zplane_face_vertex_posns;
   vector<double> sidelengths;
   for (unsigned int i=0; i<zplane_face_ptr->get_n_vertices(); i++)
   {
      threevector curr_zplane_face_posn=
         zplane_face_ptr->get_vertex_from_chain(i).get_posn();
      zplane_face_vertex_posns.push_back(curr_zplane_face_posn);

      sidelengths.push_back((curr_zplane_face_posn-apex).magnitude());
      xmin=basic_math::min(xmin,curr_zplane_face_posn.get(0));
      xmax=basic_math::max(xmax,curr_zplane_face_posn.get(0));
      ymin=basic_math::min(ymin,curr_zplane_face_posn.get(1));
      ymax=basic_math::max(ymax,curr_zplane_face_posn.get(1));
//       double z_face=curr_zplane_face_posn.get(2);
   } // loop over index i labeling zplane face vertices

   zplane_face_bbox=bounding_box(xmin,xmax,ymin,ymax);

   templatefunc::Quicksort_descending(sidelengths,zplane_face_vertex_posns);
   geopoint triangle_vertex1(
      northern_hemisphere_flag,specified_UTM_zonenumber,
      zplane_face_vertex_posns[0].get(0),
      zplane_face_vertex_posns[0].get(1),z_ground);
   geopoint triangle_vertex2(
      northern_hemisphere_flag,specified_UTM_zonenumber,
      zplane_face_vertex_posns[1].get(0),
      zplane_face_vertex_posns[1].get(1),z_ground);
   geopoint triangle_vertex0(
      northern_hemisphere_flag,specified_UTM_zonenumber,
      apex.get(0),apex.get(1),z_ground);
   triangle_vertices.push_back(triangle_vertex0);
   triangle_vertices.push_back(triangle_vertex1);
   triangle_vertices.push_back(triangle_vertex2);

//   cout << "triangle_vertex0 = " << triangle_vertex0 << endl;
//   cout << "triangle_vertex1 = " << triangle_vertex1 << endl;
//   cout << "triangle_vertex2 = " << triangle_vertex2 << endl;

// Calculate angles theta_min < theta_max of triangular wedge's sides
// relative to XY coordinate system.  theta_max-theta_min should equal
// OBSFRUSTUM's azimuthal extent.

   threevector f1(triangle_vertex1.get_UTM_posn()-
                  triangle_vertex0.get_UTM_posn());
   f1.put(2,0);
   threevector f1hat(f1.unitvector());
   
   threevector f2(triangle_vertex2.get_UTM_posn()-
                  triangle_vertex0.get_UTM_posn());
   f2.put(2,0);
   threevector f2hat(f2.unitvector());
   
   double theta1=atan2(f1hat.get(1),f1hat.get(0));
   double theta2=atan2(f2hat.get(1),f2hat.get(0));
   theta2=basic_math::phase_to_canonical_interval(
      theta2,theta1-PI,theta1+PI);
   double theta3=basic_math::min(theta1,theta2);
   double theta4=basic_math::max(theta1,theta2);
   theta_min=theta3;
   theta_max=theta4;
   theta_min=basic_math::phase_to_canonical_interval(theta_min,-PI,PI);
   theta_max=basic_math::phase_to_canonical_interval(
      theta_max,theta_min-PI,theta_min+PI);

//   cout << "theta_min = " << theta_min*180/PI << endl;
//   cout << "theta_max = " << theta_max*180/PI << endl;
//   cout << "theta_max-theta_min = " << (theta_max-theta_min)*180/PI << endl;

// Form and return zplane triangle polygon:

   vector<threevector> triangle_pts;
   for (unsigned int t=0; t<3; t++)
   {
      triangle_pts.push_back(triangle_vertices[t].get_UTM_posn());
   }
   zplane_triangle=polygon(triangle_pts);
//   cout << "zplane_triangle = " << zplane_triangle << endl;
}

// ---------------------------------------------------------------------
// Member function export_DTED_ptwoDarray_contents() generates an
// output geotiff file containing the current frame's
// *DTED_ptwoDarray_ptr() contents.

void OBSFRUSTUM::export_DTED_ptwoDarray_contents(
   raster_parser& RasterParser,string geotif_Ptiles_subdir,
   bool northern_hemisphere_flag,int specified_UTM_zonenumber)
{
//   cout << "inside OBSFRUSTUM::export_DTED_ptwoDarray_contents()" << endl;
   
   bool output_floats_flag=false;
   string geotiff_filename=get_geotiff_filename(geotif_Ptiles_subdir);

   const double p_min=-1;
   const double p_max=1;
   RasterParser.write_raster_data(
      output_floats_flag,geotiff_filename,
      specified_UTM_zonenumber,northern_hemisphere_flag,
      get_DTED_ptwoDarray_ptr(),p_min,p_max);

//   cout << "Exported *DTED_ptwoDarray_ptr = " << *get_DTED_ptwoDarray_ptr() 
//        << endl;
}

// ---------------------------------------------------------------------
// Member function get_geotiff_filename() returns a filename
// containing OBSFRUSTUM ID and frame number information.

string OBSFRUSTUM::get_geotiff_filename(string geotif_Ptiles_subdir)
{
//   cout << "inside OBSFRUSTUM::geotiff_filename()" << endl;

   int curr_framenumber=AnimationController_ptr->get_curr_framenumber();
   string geotiff_filename=geotif_Ptiles_subdir+"ptwoDarray_OBSFRUSTUM_"
      +stringfunc::number_to_string(get_ID())+"_frame_"
      +stringfunc::number_to_string(curr_framenumber)+".tif";
//   cout << "geotiff_filename = " << geotiff_filename << endl;
   return geotiff_filename;
}

// ---------------------------------------------------------------------
// Member function compute_occlusion_frac_vs_maxrange() is a special
// utility which we wrote to quantify terrain occlusion as a
// function of maximum range for the D7 video camera mounted on a mast
// nearby the flight facility in July 2010 viewing the ALIRT HAFB
// minimap.  It takes in *(get_DTED_ptwoDarray_ptr()) which we assume
// has already been filled via raytrace_occluded_ground_region().
// This method computes the ratio of
// n_visible_groundcells/n_groundcells for a variety of different
// maximum ranges from the D7 camera's location.

void OBSFRUSTUM::compute_occlusion_frac_vs_maxrange(const threevector& apex)
{
   cout << "inside OBSFRUSTUM::compute_occlusion_frac_vs_maxrange()" << endl;

   twoDarray* ptwoDarray_ptr=get_DTED_ptwoDarray_ptr();
   vector<double> maxrange;
   maxrange.push_back(100);	// meters
   maxrange.push_back(250);	// meters
   maxrange.push_back(500);	// meters
   maxrange.push_back(1000);	// meters
   
   twovector apex_XY(apex);
   threevector currpoint;
   for (unsigned int i=0; i<maxrange.size(); i++)
   {
      int n_groundcells=0;
      int n_visible_groundcells=0;
      for (unsigned int px=0; px<ptwoDarray_ptr->get_mdim(); px++)
      {
         for (unsigned int py=0; py<ptwoDarray_ptr->get_ndim(); py++)
         {
            ptwoDarray_ptr->pixel_to_point(px,py,currpoint);
            twovector curr_XY(currpoint);
            double curr_range=(curr_XY-apex_XY).magnitude();
            if (curr_range > maxrange[i]) continue;
            
            double curr_p=ptwoDarray_ptr->get(px,py);
            if (nearly_equal(curr_p,1))
            {
               n_visible_groundcells++;
               n_groundcells++;
            }
            else if (nearly_equal(curr_p,0,0.1))
            {
               n_groundcells++;
            }
         } // loop over py index
      } // loop over px index
      
      double visible_fraction=double(n_visible_groundcells)/
         double(n_groundcells);
      cout << "i = " << i
           << " maxrange = " << maxrange[i] << endl;
      cout << "n_visible_groundcells = " << n_visible_groundcells
           << " n_groundcells = " << n_groundcells
           << " visible frac = " << visible_fraction << endl;
   } // loop over index i labeling max ranges
}

// ==========================================================================
// OBSFRUSTUM orientation member functions:
// ==========================================================================

// Member function rotation_about_LOS() spins the virtual camera by 90
// degrees about -What axis ( = LOS direction) if OBSFRUSTUM's movie
// should be viewed in portrait mode.  It also spins the virtual
// camera by any previously calculated roll angle if the roll is close
// to +/- 90 degs or +/- 180 degs.

rotation OBSFRUSTUM::rotation_about_LOS()
{
   bool image_rotated_by_ninety_degrees_flag=false;
   return rotation_about_LOS(image_rotated_by_ninety_degrees_flag);
}

rotation OBSFRUSTUM::rotation_about_LOS(
   bool& image_rotated_by_ninety_degrees_flag)
{
//   cout << "inside OBSFRUSTUM::rotation_about_LOS()" << endl;

   rotation Rot_about_LOS;
   Rot_about_LOS.identity();

   Movie* Movie_ptr=get_Movie_ptr();
   if (Movie_ptr==NULL) return Rot_about_LOS;
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   if (camera_ptr==NULL) return Rot_about_LOS;

   double roll=camera_ptr->get_rel_roll()*180/PI;
//   cout << "roll = " << roll << endl;
   
   if (get_portrait_mode_flag() || nearly_equal(roll,90,15))
   {
      Rot_about_LOS.clear_values();
      Rot_about_LOS.put(0,1,1);
      Rot_about_LOS.put(1,0,-1);
      Rot_about_LOS.put(2,2,1);
      image_rotated_by_ninety_degrees_flag=true;
   }
   else if (nearly_equal(roll,-90,15))
   {
      Rot_about_LOS.clear_values();
      Rot_about_LOS.put(0,1,-1);
      Rot_about_LOS.put(1,0,1);
      Rot_about_LOS.put(2,2,1);
      image_rotated_by_ninety_degrees_flag=true;
   }
   else if (nearly_equal(roll,180,10) ||
            nearly_equal(roll,-180,10))
   {
      Rot_about_LOS.clear_values();
      Rot_about_LOS.put(0,0,-1);
      Rot_about_LOS.put(1,1,-1);
      Rot_about_LOS.put(2,2,1);
   }
   return Rot_about_LOS;
}

// ==========================================================================
// Ray drawing member functions
// ==========================================================================

// Member function draw_ray_through_imageplane() instantiates a 3D
// arrow whose base is located at the OBSFRUSTUM's camera center.  It
// pierces the imageplane at 2D location (U,V).  

// As of 8/28/11, we are unable to get the arrow's cone to appear...

Arrow* OBSFRUSTUM::draw_ray_through_imageplane(
   int Arrow_ID,const twovector& UV,double magnitude,colorfunc::Color color,
   double linewidth)
{
   Movie* Movie_ptr=get_Movie_ptr();
   if (Movie_ptr==NULL) return NULL;
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   if (camera_ptr==NULL) return NULL;
   
   Arrow* Arrow_ptr=get_Arrow_ptr(Arrow_ID);
   if (Arrow_ptr==NULL)
      Arrow_ptr=ArrowsGroup_ptr->generate_new_Arrow();

   threevector base=camera_ptr->get_world_posn();
   if (grid_world_origin_ptr != NULL)
   {
      base += *grid_world_origin_ptr;
   }

   const double arrowhead_size_prefactor=100;
   threevector e_hat=camera_ptr->pixel_ray_direction(UV.get(0),UV.get(1));
   Arrow_ptr->set_magnitude_direction_and_base(
      magnitude,e_hat,base,arrowhead_size_prefactor);
   Arrow_ptr->set_color(color);
   Arrow_ptr->set_linewidth(linewidth);

   return Arrow_ptr;
}

// ---------------------------------------------------------------------
// Member function reset_OBSFRUSTUM() is a
// high-level method for generating an OBSFRUSTUM corresponding to a
// ground camera looking in a general direction.

void OBSFRUSTUM::reset_OBSFRUSTUM(
   double curr_t,int pass_number,
   double frustum_sidelength,double movie_downrange_distance,
   double az,double el,double roll,
   camera* camera_ptr,colorfunc::Color OBSFRUSTUM_color,double vol_alpha)
{
//   cout << "inside OBSFRUSTUM::reset_OBSFRUSTUM() #1" << endl;

   vector<threevector> UV_corner_rays=reorient_camera_corner_rays(
      az,el,roll,camera_ptr);

   reset_OBSFRUSTUM(
      curr_t,pass_number,frustum_sidelength,movie_downrange_distance,
      camera_ptr->get_world_posn(),UV_corner_rays,OBSFRUSTUM_color,vol_alpha);
}

// ---------------------------------------------------------------------
// Member function reset_OBSFRUSTUM() is a
// high-level method for generating an OBSFRUSTUM corresponding to a
// ground camera looking in a general direction.

void OBSFRUSTUM::reset_OBSFRUSTUM(
   double curr_t,int pass_number,
   double frustum_sidelength,double movie_downrange_distance,
   const threevector& camera_world_posn,const vector<threevector>& corner_ray,
   colorfunc::Color OBSFRUSTUM_color,double vol_alpha)
{
//   cout << "inside OBSFRUSTUM::reset_OBSFRUSTUM() #2" << endl;

//   initialize_frustum_with_movie(frustum_sidelength,movie_downrange_distance);

   generate_or_reset_viewing_pyramid(
      Zero_vector,corner_ray,frustum_sidelength);

   set_Movie_ptr(NULL);
   absolute_position(curr_t,pass_number,camera_world_posn);

   get_ViewingPyramid_ptr()->set_mask(
      curr_t,pass_number,!get_display_ViewingPyramid_flag());
   get_ViewingPyramidAboveZplane_ptr()->set_mask(
      curr_t,pass_number,!get_display_ViewingPyramidAboveZplane_flag());

   if (get_display_ViewingPyramid_flag())
   {
      get_ViewingPyramid_ptr()->build_current_pyramid(
         curr_t,pass_number,get_viewing_pyramid_ptr());
   }

   set_volume_alpha(vol_alpha);
   set_color(colorfunc::get_OSG_color(OBSFRUSTUM_color),volume_alpha);
   set_permanent_color(OBSFRUSTUM_color);
}

// ---------------------------------------------------------------------
// Member function reorient_camera_corner_rays() takes in camera
// *camera_ptr which is assumed to have a set of UV corner
// rays corresponding to its az, el and roll angles.  This method
// computes the differential rotation dR needed to transform
// the existing corner rays into a new set which is oriented according
// to the input az, el, and roll angles.  The reoriented UV corners
// rays are returned within an STL vector.

vector<threevector> OBSFRUSTUM::reorient_camera_corner_rays(
   double az,double el,double roll,camera* camera_ptr)
{
//   cout << "inside OBSFRUSTUM::reorient_camera_corner_rays()" << endl;
//   cout << "az = " << az*180/PI
//        << " el = " << el*180/PI
//        << " roll = " << roll*180/PI << endl;
   
   double curr_az,curr_el,curr_roll;
   camera_ptr->get_az_el_roll_from_Rcamera(curr_az,curr_el,curr_roll);

//   cout << "curr_az = " << curr_az*180/PI
//        << " curr_el = " << curr_el*180/PI 
//        << " curr_roll = " << curr_roll*180/PI << endl;
   
   rotation Rcurr;
   Rcurr=Rcurr.rotation_from_az_el_roll(curr_az,curr_el,curr_roll);

   vector<threevector> curr_UV_corner_rays=camera_ptr-> 
      get_UV_corner_world_ray();
//   for (unsigned int c=0; c<curr_UV_corner_rays.size(); c++)
//   {
//      cout << "c = " << c
//           << " curr_UV_corner_ray = " << curr_UV_corner_rays[c]
//           << endl;
//   }

   rotation Rnew,dR;
   Rnew=Rnew.rotation_from_az_el_roll(az,el,roll);
   dR=Rnew*Rcurr.transpose();

   vector<threevector> UV_corner_rays;
   for (unsigned int c=0; c<curr_UV_corner_rays.size(); c++)
   {
      UV_corner_rays.push_back(dR*curr_UV_corner_rays[c]);
//      cout << "c = " << c << " UV_corner_ray = " 
//           << UV_corner_rays.back() << endl;
   }

// Reset camera's az,el,roll angles along with its UV corner rays:

   camera_ptr->set_Rcamera(az,el,roll);
   camera_ptr->set_UV_corner_world_ray(UV_corner_rays);
   return UV_corner_rays;
}



// ---------------------------------------------------------------------
// Member function project_Polyhedra_into_imageplane() loops over all
// Polyhedra within input *PolyhedraGroup_ptr.  It projects each
// Polyhedron's faces into the OBSFRUSTUM's image plane.  The
// projected faces are drawn as translucent Polygons and returned
// within output *PolygonsGroup_ptr.  If some Polyhedra faces lie
// inside the OBSFRUSTUM's image plane, this boolean method returns true.
 
bool OBSFRUSTUM::new_project_Polyhedra_into_imageplane(
   PolyhedraGroup* PolyhedraGroup_ptr,
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr,
   twoDarray* DTED_ztwoDarray_ptr)
{
   cout << "inside OBSFRUSTUM::new_project_Polyhedra_into_imageplane()" << endl;

   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   threevector pointing_dir=-camera_ptr->get_What();

   PolyLinesGroup* PolyLinesGroup_ptr=
      PolygonsGroup_ptr->get_PolyLinesGroup_ptr();
//   cout << "PolyLinesGroup_ptr = " << PolyLinesGroup_ptr << endl;

   bool some_faces_inside_imageplane_flag=false;
   threevector bounding_sphere_center;
   double bounding_sphere_radius;

   for (unsigned int p=0; p<PolyhedraGroup_ptr->get_n_Graphicals(); p++)
   {
      Polyhedron* Polyhedron_ptr=PolyhedraGroup_ptr->get_Polyhedron_ptr(p);
      polyhedron* polyhedron_ptr=Polyhedron_ptr->get_polyhedron_ptr();

// Test whether polyhedron's bounding sphere lies outside OBSFRUSTUM:

      polyhedron_ptr->get_bounding_sphere(
         bounding_sphere_center,bounding_sphere_radius);
//      cout << "p = " << p 
//           << " bounding_sphere_center = " << bounding_sphere_center
//           << " radius = " << bounding_sphere_radius 
//           << endl;
      if (!camera_ptr->get_camera_frustum_ptr()->SphereInsideFrustum(
         bounding_sphere_center,bounding_sphere_radius)) continue;

// Test whether polyhedron's bounding box lies outside OBSFRUSTUM:

      bool inside_flag,outside_flag,intersects_flag;
      bounding_box bbox=polyhedron_ptr->get_bbox();
      camera_ptr->get_camera_frustum_ptr()->BoxInsideFrustum(
         &bbox,inside_flag,outside_flag,intersects_flag);
//      cout << "inside_flag = " << inside_flag
//           << " outside_flag = " << outside_flag
//           << " intersects_flag = " << intersects_flag << endl;
      if (outside_flag) continue;

      colorfunc::Color bbox_color=colorfunc::white;
      if (Polyhedron_ptr->get_ID()==
      PolyhedraGroup_ptr->get_selected_Graphical_ID())
      {
         bbox_color=colorfunc::red;
      }

      for (unsigned int f=0; f<polyhedron_ptr->get_n_faces(); f++)
      {
         face* face_ptr=polyhedron_ptr->get_face_ptr(f);
//         double dotproduct=pointing_dir.dot(face_ptr->get_normal());

//         cout << "f = " << f
//              << " normal = " << face_ptr->get_normal() 
//              << " dotproduct = " << dotproduct 
//              << endl;

// Only draw faces of polyhedron which are oriented towards the
// camera:

//         if (dotproduct >= 0) continue;

// Recall *face_ptr has both internal and external edges.  Do not draw
// internal edges:

         for (unsigned int e=0; e<face_ptr->get_n_edges(); e++)
         {
            edge* edge_ptr=&(face_ptr->get_edge(e));

            edge* polyhedron_edge_ptr=polyhedron_ptr->
               find_edge_given_vertices(
                  edge_ptr->get_V1(),edge_ptr->get_V2());
            if (polyhedron_edge_ptr->get_internal_edge_flag()) continue;
            

            vector<threevector> proj_vertices;
            threevector UVW;
            camera_ptr->project_XYZ_to_UV_coordinates(
               edge_ptr->get_V1().get_posn(),UVW);
            proj_vertices.push_back(UVW);
            camera_ptr->project_XYZ_to_UV_coordinates(
               edge_ptr->get_V2().get_posn(),UVW);
            proj_vertices.push_back(UVW);

// Check if sampled points along the 3D edge's 2D projected line segment lies
// within camera's view frustum.  If not, don't generate PolyLine
// corresponding to current projected edge:

            linesegment l(proj_vertices.front(),proj_vertices.back());
            const unsigned int n_fracs=11;
            int df=0.1;
            bool projected_edge_in_view_flag=false;
            for (unsigned int nf=0; nf<n_fracs; nf++)
            {
               double curr_frac=0+nf*df;
               threevector curr_UVW=l.get_frac_posn(curr_frac);
               if (camera_ptr->get_UV_bbox_ptr()->point_inside(
                  curr_UVW.get(0),curr_UVW.get(1)))
               {
                  projected_edge_in_view_flag=true;
                  break;
               }
               
            } // loop over index nf labeling fractional positions

            if (!projected_edge_in_view_flag) continue;

               cout << "proj_vertices = " << proj_vertices[0] << " "
                    << proj_vertices[1] << endl;
            PolyLine* PolyLine_ptr=
               PolyLinesGroup_ptr->generate_new_PolyLine(
                  Zero_vector,proj_vertices,colorfunc::get_OSG_color(
                     bbox_color));
            PolyLine_ptr->set_linewidth(2);

         } // loop over index e labeling edges 

// Project current polyhedron face into image plane:

         bool face_inside_imageplane_flag;
         vector<threevector> proj_vertices=camera_ptr->
            project_polyhedron_face_into_imageplane(
               f,polyhedron_ptr,face_inside_imageplane_flag);
         if (face_inside_imageplane_flag)
            some_faces_inside_imageplane_flag=true;

//         double alpha=0.25;
//         bool draw_border_flag=false;
//         PolygonsGroup_ptr->generate_translucent_Polygon(
//            bbox_color,proj_vertices,alpha,draw_border_flag);

      } // loop over index f labeling polyhedron faces

      project_polyhedron_point_cloud_into_imageplane(
         polyhedron_ptr,camera_ptr,DTED_ztwoDarray_ptr);

   } // loop over index p labeling Polyhedra
   
   return some_faces_inside_imageplane_flag;
}
