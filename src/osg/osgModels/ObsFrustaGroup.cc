// ==========================================================================
// OBSFRUSTAGROUP class member function definitions
// ==========================================================================
// Last modified on 8/24/09; 9/14/09; 1/6/11; 4/6/14
// ==========================================================================

#include <iomanip>
#include <string>
#include "osg/osgGraphicals/AnimationController.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/ModeController.h"
#include "osg/osgModels/ModelsGroup.h"
#include "osg/osgModels/ObsFrustaGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "geometry/polyhedron.h"
#include "math/rotation.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/ViewFrustum.h"

#include "general/outputfuncs.h"
#include "geometry/plane.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ObsFrustaGroup::allocate_member_objects()
{
   MoviesGroup_ptr=new MoviesGroup(3,get_pass_ptr(),AnimationController_ptr);
   PolyhedraGroup_ptr=new PolyhedraGroup(
      get_pass_ptr(),get_grid_world_origin_ptr());
}		       

void ObsFrustaGroup::initialize_member_objects()
{
   GraphicalsGroup_name="ObsFrustaGroup";

   n_still_images=0;
   z_ColorMap_ptr=NULL;
   CM_3D_ptr=NULL;
   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<ObsFrustaGroup>(
         this, &ObsFrustaGroup::update_display));
}		       

ObsFrustaGroup::ObsFrustaGroup(
   Pass* PI_ptr,AnimationController* AC_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr,GO_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

ObsFrustaGroup::ObsFrustaGroup(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   AnimationController* AC_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr,GO_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();

   CM_3D_ptr=static_cast<osgGA::Terrain_Manipulator*>(CM_ptr);
}		       

ObsFrustaGroup::~ObsFrustaGroup()
{
   MoviesGroup_ptr->destroy_all_Graphicals();
   delete MoviesGroup_ptr;
   PolyhedraGroup_ptr->destroy_all_Graphicals();
   delete PolyhedraGroup_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const ObsFrustaGroup& f)
{
   int node_counter=0;
   for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
   {
      ObsFrustum* ObsFrustum_ptr=f.get_ObsFrustum_ptr(n);
      outstream << "ObsFrustum node # " << node_counter++ << endl;
      outstream << "ObsFrustum = " << *ObsFrustum_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// ObsFrustum creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_ObsFrustum from all other graphical insertion
// and manipulation methods...

ObsFrustum* ObsFrustaGroup::generate_new_ObsFrustum(Movie* Movie_ptr,int ID)
{
   if (ID==-1) ID=get_next_unused_ID();
//   cout << "inside OFG::generate_new_OF, ID = " << ID << endl;
   ObsFrustum* curr_ObsFrustum_ptr=new ObsFrustum(
      pass_ptr,get_grid_world_origin(),AnimationController_ptr,Movie_ptr,ID);

   int OSGsubPAT_number=ID;
   initialize_new_ObsFrustum(curr_ObsFrustum_ptr,OSGsubPAT_number);
   return curr_ObsFrustum_ptr;
}

ObsFrustum* ObsFrustaGroup::generate_new_ObsFrustum(
   double az_extent,double el_extent,int ID)
{
//   cout << "inside ObsFrustGroup::generate_new_ObsFrustum()" << endl;

   if (ID==-1) ID=get_next_unused_ID();
   ObsFrustum* curr_ObsFrustum_ptr=new ObsFrustum(
      pass_ptr,az_extent,el_extent,get_grid_world_origin(),
      AnimationController_ptr,NULL,ID);

   int OSGsubPAT_number=ID;
   initialize_new_ObsFrustum(curr_ObsFrustum_ptr,OSGsubPAT_number);
   return curr_ObsFrustum_ptr;
}

// ---------------------------------------------------------------------
void ObsFrustaGroup::initialize_new_ObsFrustum(
   ObsFrustum* ObsFrustum_ptr,int OSGsubPAT_number)
{
//   cout << "inside ObsFrustaGroup::initialize_new_ObsFrustum()" << endl;
//   cout << "OSGsubPAT_number = " << OSGsubPAT_number << endl;

   GraphicalsGroup::insert_Graphical_into_list(ObsFrustum_ptr);

// Note added on 1/17/07: We should someday write a
// destroy_ObsFrustum() method which should explicitly remove the
// following LineSegmentsGroup->OSGGroup_ptr from curr_ObsFrustum
// before deleting curr_ObsFrustum...

   ObsFrustum_ptr->get_LineSegmentsGroup_ptr()->
      set_AnimationController_ptr(AnimationController_ptr);

   initialize_Graphical(ObsFrustum_ptr);
   ObsFrustum_ptr->get_PAT_ptr()->addChild(
      ObsFrustum_ptr->get_LineSegmentsGroup_ptr()->get_OSGgroup_ptr());

   if (ObsFrustum_ptr->get_ModelsGroup_ptr() != NULL)
   {
      ObsFrustum_ptr->get_PAT_ptr()->addChild(
         ObsFrustum_ptr->get_ModelsGroup_ptr()->get_OSGgroup_ptr());
   }
      
   insert_graphical_PAT_into_OSGsubPAT(ObsFrustum_ptr,OSGsubPAT_number);
}

// ---------------------------------------------------------------------
// Member function generate_movie_ObsFrustum generates a new
// ObsFrustum and a new Movie object.  It places the latter within the
// former such that the camera which took the movie frames is set at
// the ObsFrustum's vertex.  A pointer to the dynamically generated
// ObsFrustum object is returned by this method.

ObsFrustum* ObsFrustaGroup::generate_movie_ObsFrustum(
   string movie_filename,double alpha)
{
//   cout << "inside ObsFrustaGroup::generate_movie_ObsFrustum()" << endl;
   
// Clone a copy of the video within a new movie object.  Copy pointer
// to new movie object into *ObsFrustum_ptr.

   Movie* Movie_ptr=get_MoviesGroup_ptr()->generate_new_Movie(
      movie_filename,alpha);
   if (Movie_ptr==NULL) return NULL;

   ObsFrustum* ObsFrustum_ptr=generate_new_ObsFrustum(Movie_ptr);
   ObsFrustum_ptr->get_PAT_ptr()->addChild(Movie_ptr->get_PAT_ptr());

//   cout << "Movie_ptr->get_PAT_ptr() = "
//        << Movie_ptr->get_PAT_ptr() << endl;
//   cout << "ObsFrustum_ptr->get_PAT_ptr() = "
//        << ObsFrustum_ptr->get_PAT_ptr() << endl;

   return ObsFrustum_ptr;
}

// ==========================================================================
// Still imagery ObsFrusta generation methods:
// ==========================================================================

// Member function generate_still_imagery_frusta extracts video
// filename, downrange distance and 3x4 projection matrix information
// from each video pass within input passes_group.  It then
// instantiates an ObsFrustum for each still image.  The number of
// such frusta is returned by this method.

int ObsFrustaGroup::generate_still_imagery_frusta(
   PassesGroup& passes_group,bool multicolor_frusta_flag,
   bool initially_mask_all_frusta_flag)
{
//   cout << "inside OFG::generate_still_imagery_frusta()" << endl;
   
   n_still_images=0;
//   cout << "n_passes = " << passes_group.get_n_passes() << endl;
   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
      Pass* curr_pass_ptr=passes_group.get_pass_ptr(n);
//      cout << "curr_pass = " << *curr_pass_ptr << endl;
      if (curr_pass_ptr->get_passtype()==Pass::video)
      {
         extract_still_imagery_info(curr_pass_ptr);

         colorfunc::Color frustum_color=colorfunc::white;
         if (multicolor_frusta_flag)
         {
            frustum_color=colorfunc::get_color(n_still_images);
         }

         ObsFrustum* ObsFrustum_ptr=
            generate_still_image_ObsFrustum(n_still_images,frustum_color);

         if (initially_mask_all_frusta_flag)
         {
            erase_Graphical(ObsFrustum_ptr->get_ID());
         }

         n_still_images++;
      }
   } // loop over index n labeling individual passes within passes_group

   return n_still_images;
}

// ---------------------------------------------------------------------
void ObsFrustaGroup::extract_still_imagery_info(Pass* videopass_ptr)
{
//   cout << "inside OFG::extract_still_imagery_info()" << endl;
   still_movie_filenames.push_back(videopass_ptr->get_first_filename());
   downrange_distances.push_back(
      videopass_ptr->get_PassInfo_ptr()->get_downrange_distance());
   P_ptrs.push_back(
      videopass_ptr->get_PassInfo_ptr()->get_projection_matrix_ptr());
//   cout << "still movie filename = "
//        << still_movie_filenames.back() << endl;
//   cout << "downrange dist = " << downrange_distances.back() << endl;
//   cout << "P = " << *(P_ptrs.back()) << endl;
}

// ---------------------------------------------------------------------
// Member function reset_frustum_colors_based_upon_Zcolormap is a
// specialized method which we wrote in order to force ObsFrusta to be
// colored when viewed against grey scale pointclouds.  It first
// checks whether a height colormap has been defined and whether it
// currently corresponds to the grey colormap.  If so, this method
// resets ObsFrustum within the current ObsFrustaGroup to a separate
// color.  Otherwise, all ObsFrusta are recolored white.

void ObsFrustaGroup::reset_frustum_colors_based_on_Zcolormap()
{
//   cout << "inside OFG::reset_frustum_colors_based_on_colormap()" << endl;
//   cout << "z_ColorMap_ptr->get_map_number() = "
//        << z_ColorMap_ptr->get_map_number() << endl;

   bool multicolor_frusta_flag=false;
   if (z_ColorMap_ptr != NULL)
   {
      if (z_ColorMap_ptr->get_map_number()==6) // grey colormap
      {
         multicolor_frusta_flag=true;
      }
   }
   
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      colorfunc::Color frustum_color=colorfunc::white;
      if (multicolor_frusta_flag)
      {
         frustum_color=colorfunc::get_color(n);
      }
      get_ObsFrustum_ptr(n)->set_color(
         colorfunc::get_OSG_color(frustum_color));
   } // loop over index n labeling ObsFrusta
}

// ---------------------------------------------------------------------
// Member function generate_virtual_camera_ObsFrustum()

ObsFrustum* ObsFrustaGroup::generate_virtual_camera_ObsFrustum()
{
   ObsFrustum* ObsFrustum_ptr=generate_new_ObsFrustum();
   ObsFrustum_ptr->set_virtual_camera_flag(true);
   ObsFrustum_ptr->set_display_camera_model_flag(true);
   ObsFrustum_ptr->set_color(osg::Vec4(1,1,1,1));	// white
   return ObsFrustum_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_still_image_ObsFrusta 

ObsFrustum* ObsFrustaGroup::generate_still_image_ObsFrustum(
   int n_frustum,colorfunc::Color frustum_color)
{
//   cout << "inside ObsFrustaGroup::generate_still_image_ObsFrustum" << endl;
//    cout << "n_frustum = " << n_frustum << endl;
   
   ObsFrustum* ObsFrustum_ptr=generate_movie_ObsFrustum(
      still_movie_filenames[n_frustum]);
   if (ObsFrustum_ptr==NULL) return NULL;

   ObsFrustum_ptr->set_color(colorfunc::get_OSG_color(frustum_color));
//   ObsFrustum_ptr->set_color(osg::Vec4(1,1,1,1));	// white
//   if (n_frustum==0)
//   {
//      ObsFrustum_ptr->set_color(osg::Vec4(1,1,0,1));	// yellow
//   }
//   else if (n_frustum==1)
//   {
//      ObsFrustum_ptr->set_color(osg::Vec4(0,0.5,1,1));	// blue-green
//   }

   camera* curr_camera_ptr=ObsFrustum_ptr->get_Movie_ptr()->get_camera_ptr();
   curr_camera_ptr->set_projection_matrix(*(P_ptrs[n_frustum]));
//   curr_camera_ptr->extract_external_and_internal_params();

//   cout << "camera.world_posn = " << curr_camera_ptr->get_world_posn() 
//        << endl;

   double z_offset=20;			// meters
   ObsFrustum_ptr->build_frustum_with_movie(
      get_curr_t(),get_passnumber(),z_offset,downrange_distances[n_frustum]);

   polyhedron* polyhedron_ptr=ObsFrustum_ptr->generate_curr_polyhedron(
      get_curr_t(),get_passnumber());
   polyhedron_ptr->translate(curr_camera_ptr->get_world_posn());
//   cout.precision(12);

//   cout << "*polyhedron_ptr = " << *polyhedron_ptr << endl;
   
//   polyhedron_ptr->generate_GNU_triangulated_surface();

//   Polyhedron* Polyhedron_ptr=
      PolyhedraGroup_ptr->generate_new_Polyhedron(polyhedron_ptr);
//   PolyhedraGroup_ptr->generate_polyhedron_geode(Polyhedron_ptr);
//   Polyhedron_ptr->set_color(osg::Vec4(1,1,1,0));
//   Polyhedron_ptr->set_edge_color(osg::Vec4(1,1,1,0));

//   Polyhedron_ptr->set_color(osg::Vec4(1,0,0,0));
//   Polyhedron_ptr->set_edge_color(osg::Vec4(1,0,0,0));

// To display Polyhedron which has been translated so that its origin
// matches the camera's world position, chant

//   insert_graphical_PAT_into_OSGsubPAT(
//      Polyhedron_ptr,ObsFrustum_ptr->get_ID());

// To display Polyhedron which has NOT been translated, chant

//   ObsFrustum_ptr->get_PAT_ptr()->addChild(
//      Polyhedron_ptr->get_PAT_ptr());

   return ObsFrustum_ptr;
}   

// ==========================================================================
// Animation methods
// ==========================================================================

// Member function update_display

void ObsFrustaGroup::update_display()
{   
//   cout << "inside ObsFrustaGroup::update_display()" << endl;

   MoviesGroup_ptr->update_display();

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      ObsFrustum* ObsFrustum_ptr=get_ObsFrustum_ptr(n);
      if (ObsFrustum_ptr->get_virtual_camera_flag())
      {
//         cout << "CM_3D_ptr = " << CM_3D_ptr << endl;
         ViewFrustum* ViewFrustum_ptr=CM_3D_ptr->get_ViewFrustum_ptr();
//         cout << "VF_ptr = " << ViewFrustum_ptr << endl;
         ViewFrustum_ptr->compute_params_planes_and_vertices();

         double z_offset=0;
         double max_lambda=100000;	// meters = 100 km
         ObsFrustum_ptr->build_current_frustum(
            get_curr_t(),get_passnumber(),z_offset,
            ViewFrustum_ptr->get_camera_posn(),
            ViewFrustum_ptr->get_ray(),max_lambda);
      }

      LineSegmentsGroup* LineSegmentsGroup_ptr=ObsFrustum_ptr->
         get_LineSegmentsGroup_ptr();
      LineSegmentsGroup_ptr->update_display();

      ModelsGroup* ModelsGroup_ptr=ObsFrustum_ptr->get_ModelsGroup_ptr();
      if (ModelsGroup_ptr != NULL) ModelsGroup_ptr->update_display();

      if (!ObsFrustum_ptr->get_rectangular_movie_flag())
      {
         Movie* Movie_ptr=ObsFrustum_ptr->get_Movie_ptr();
         if (Movie_ptr != NULL)
         {
            threevector V1,V2;
            vector<threevector> corner;
            for (unsigned int c=0; c<4; c++)
            {
               LineSegment* LineSegment_ptr=LineSegmentsGroup_ptr->
                  get_LineSegment_ptr(c);
               LineSegment_ptr->recover_V1_and_V2(
                  get_curr_t(),get_passnumber(),V1,V2);
               corner.push_back(V2);
         
//            cout << "n=" << n << " c=" << c
//                 << " corner=" 
//                 << corner.back().get(0) << " , "
//                 << corner.back().get(1) << " , " 
//                 << corner.back().get(2) 
//                 << endl;

            } // loop over index c labeling corners

// Recall HAFB video projects onto a trapezoidal "keystone" within the
// world z-plane rather than a rectangle.  In this case, we need to
// reset the video geometry's vertices on a per-frame basis:

            Movie_ptr->reset_geom_vertices(
               corner[1],corner[0],corner[3],corner[2]);
         } // Movie_ptr != NULL conditional
      } // !rectangular_movie_flag conditional

   } // loop over index n labeling ObsFrusta

   GraphicalsGroup::update_display();
}

// ---------------------------------------------------------------------
// Member function hide_nonselected_ObsFrusta

void ObsFrustaGroup::hide_nonselected_ObsFrusta()
{
//   cout << "inside OFG::hide_nonselected_ObsFrusta()" << endl;
   int selected_ObsFrustum_number=get_selected_Graphical_ID();
   if (selected_ObsFrustum_number==-1)
   {
      for (unsigned int i=0; i<get_n_OSGsubPATs(); i++)
      {
         set_OSGsubPAT_nodemask(i,1);
      } 
   }
   else
   {
      for (unsigned int i=0; i<get_n_OSGsubPATs(); i++)
      {
         set_OSGsubPAT_nodemask(i,0);
      } 
      set_OSGsubPAT_nodemask(selected_ObsFrustum_number,1);
   }
   update_display();
}

// ---------------------------------------------------------------------
// Member function flyto_camera_location

void ObsFrustaGroup::flyto_camera_location(int ID)
{
   ObsFrustum* ObsFrustum_ptr=get_ID_labeled_ObsFrustum_ptr(ID);
   Movie* Movie_ptr=ObsFrustum_ptr->get_Movie_ptr();
   
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   threevector camera_world_posn=camera_ptr->get_world_posn();
   rotation* Rcamera_ptr=camera_ptr->get_Rcamera_ptr();
//   cout << "camera posn = " << camera_world_posn << endl;
//   cout << "Rcamera.transpose() = " << Rcamera.transpose() << endl;

//   bool write_to_file_flag=true;
   bool write_to_file_flag=false;
   CM_3D_ptr->flyto(camera_world_posn,Rcamera_ptr->transpose(),
                    write_to_file_flag);

// Compute reasonable values for the final position and orientation of
// the animation path which is automatically calculated when the user
// eventually wants to move the virtual camera away from the
// OBSFRUSTUM's photo:

   threevector flyout_camera_posn;
   rotation Rcamera_flyout;
   compute_camera_flyout_posn_and_orientation(
      ID,flyout_camera_posn,Rcamera_flyout);

   CM_3D_ptr->set_initial_camera_posn(camera_world_posn);
   CM_3D_ptr->set_initial_camera_rotation(*Rcamera_ptr);
   CM_3D_ptr->set_final_camera_posn(flyout_camera_posn);
   CM_3D_ptr->set_final_camera_rotation(Rcamera_flyout);

// Once Terrain Manipulator's virtual camera has flown to real
// camera's location, set its rotate_about_curr_eyepoint_flag member
// boolean to true:
   
   get_Terrain_Manipulator_ptr()->
      set_rotate_about_current_eyepoint_flag(true);
}

// ---------------------------------------------------------------------
// Member function compute_camera_flyout_posn_and_orientation computes
// reasonable position and orientation values for the camera after it
// has finished flying away from the OBSFRUSTUM's apex via a user zoom
// out command.  In order to smoothly switch the Terrain_Manipulator
// from rotating about the OBSFRUSTUM's apex location to the ground
// point corresponding to the screen center, we need to force the zoom
// out to pass through the flyout position and orientation returned by
// this method.

void ObsFrustaGroup::compute_camera_flyout_posn_and_orientation(
   int ID,threevector& flyout_camera_posn,genmatrix& Rcamera_flyout)
{
//   cout << "inside OBSFRUSTAGroup::compute_flyout_posn_and_orientation()"
//        << endl;
   
   ObsFrustum* ObsFrustum_ptr=get_ID_labeled_ObsFrustum_ptr(ID);
   Movie* Movie_ptr=ObsFrustum_ptr->get_Movie_ptr();
   camera* camera_ptr=Movie_ptr->get_camera_ptr();

   threevector camera_world_posn=camera_ptr->get_world_posn();
//   genmatrix Rcamera=camera_ptr->get_world_rotation();
   rotation Rcamera(*(camera_ptr->get_Rcamera_ptr()));

//   cout << "get_portrait_mode_flag() = " 
//        << ObsFrustum_ptr->get_portrait_mode_flag() << endl;
//   cout << "virtual camera posn = " << camera_world_posn << endl;
//   cout << "Rcamera = " << Rcamera << endl;

// 0th row of Rcamera = effective Uhat (even if portrait mode == true)
// 1st row of Rcamera = effective Vhat (even if portrait mode == true)
// 2nd row of Rcamera = What

   threevector eff_Vhat,What;
   Rcamera.get_row(1,eff_Vhat);
   Rcamera.get_row(2,What);

   threevector What_proj(What);
   What_proj.put(2,0);
   What_proj=What_proj.unitvector();
   
   double backoff_dist=500;	// meters
   flyout_camera_posn=camera_world_posn+
      0.5*backoff_dist*What_proj+backoff_dist*z_hat;

// Spin camera about Uhat axis so that it ends up pointing downwards
// by some constant elevation angle independent of the photo's
// worldspace orientation:

   rotation Rot_about_Uhat;
   Rot_about_Uhat.put(0,0,1);

   double theta_init=acos(What.dot(z_hat));
//   double dtheta=-(theta_init-0);	// end up looking straight down
   double dtheta=-(theta_init-PI/6);
//   double dtheta=(theta_init-PI/2);	// end up looking parallel to ground 
//   cout << "dtheta = " << dtheta*180/PI << endl;

   double cos_theta=cos(dtheta);
   double sin_theta=sin(dtheta);
   Rot_about_Uhat.put(1,1,cos_theta);
   Rot_about_Uhat.put(1,2,sin_theta);
   Rot_about_Uhat.put(2,1,-sin_theta);
   Rot_about_Uhat.put(2,2,cos_theta);
   Rcamera_flyout=Rot_about_Uhat*Rcamera;
}

// ==========================================================================
// HAFB video3D methods
// ==========================================================================

// Member function generate_HAFB_movie_ObsFrustum

// On 7/20/07, we empirically confirmed that (to good approximation)
// the four direction vectors curr_UV_corner_dir[0-3] define a plane.
// u_hat = curr_UV_corner_dir[1]-curr_UV_corner_dir[0]
// v_hat = curr_UV_corner_dir[2]-curr_UV_corner_dir[1]
// Angle between u_hat and v_hat = 90.16 degs.


ObsFrustum* ObsFrustaGroup::generate_HAFB_movie_ObsFrustum(
   const vector<threevector>& aircraft_posn,double z_offset)
{
//   cout << "inside ObsFrustaGroup::generate_HAFB_movie_ObsFrustum()" << endl;   
   string subdir="/home/cho/programs/c++/svn/projects/src/mains/fusion/";
   string corners_filename="UV_corners.txt";
   corners_filename=subdir+corners_filename;
   vector<int> segment_ID,pass_number;
   vector<double> curr_time;
   vector<threevector> V1,V2;
   vector<colorfunc::Color> color;
   read_HAFB_frusta_info(
      corners_filename,curr_time,segment_ID,pass_number,V1,V2,color);

   vector<threevector> UV_corner_dir;
   reconstruct_HAFB_corner_dirs(aircraft_posn,V1,UV_corner_dir);

// Clone a copy of the video within a new movie object.  Copy pointer
// to new movie object into *ObsFrustum_ptr. Then transform movie so
// that it lies inside *ObsFrustum_ptr:
   
   string movie_filename=subdir+"HAFB_overlap_corrected_grey.vid";
   ObsFrustum* ObsFrustum_ptr=generate_movie_ObsFrustum(movie_filename);
   ObsFrustum_ptr->set_rectangular_movie_flag(false);
   
   vector<threevector> curr_UV_corner_dir;
   for (unsigned int i=0; i<V1.size()/4; i++)
   {
      double curr_t=double(i);

      curr_UV_corner_dir.clear();
      for (unsigned int c=0; c<4; c++)
      {
         curr_UV_corner_dir.push_back(UV_corner_dir[i*4+c]);
      }

//      threevector u_hat(curr_UV_corner_dir
//      cout << "i = " << i
//           << " UV(0).UV(1) = " 
//           << curr_UV_corner_dir[0].dot(curr_UV_corner_dir[1])
//           << " UV(1).UV(2) = "
//           << curr_UV_corner_dir[1].dot(curr_UV_corner_dir[2])
//           << endl;

      ObsFrustum_ptr->build_current_frustum(
         curr_t,get_passnumber(),z_offset,aircraft_posn[i],
         curr_UV_corner_dir);
      ObsFrustum_ptr->set_stationary_Graphical_flag(false);

   } // loop over index i labeling HAFB movie frames

   get_MoviesGroup_ptr()->update_display();
   return ObsFrustum_ptr;
}

// ---------------------------------------------------------------------
// Member function read_HAFB_frusta_info parses the ascii text file
// holding the HAFB pass which program VIDEO3D displays.  This boolean
// member function returns false if it cannot successfully parse the
// input ascii file.

void ObsFrustaGroup::read_HAFB_frusta_info(
   string segments_filename,vector<double>& curr_time,
   vector<int>& segment_ID,vector<int>& pass_number,
   vector<threevector>& V1,vector<threevector>& V2,
   vector<colorfunc::Color>& color)
{
   if (!filefunc::ReadInfile(segments_filename))
   {
      cout << "Trouble in ObsFrustaGroup::read_info_from_file()"
           << endl;
      cout << "Couldn't open segments_filename = " << segments_filename
           << endl;
   }

   unsigned int nlines=filefunc::text_line.size();
   curr_time.reserve(nlines);
   pass_number.reserve(nlines);
   V1.reserve(nlines);
   V2.reserve(nlines);
   color.reserve(nlines);

   const int n_fields=10;
   double X[n_fields];
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << "i = " << i
//           << " text_line = " << filefunc::text_line[i] << endl;
      stringfunc::string_to_n_numbers(n_fields,filefunc::text_line[i],X);
      curr_time.push_back(X[0]);
      segment_ID.push_back(basic_math::round(X[1]));
      pass_number.push_back(basic_math::round(X[2]));
      double V1_x=X[3];
      double V1_y=X[4];
      double V1_z=X[5];
      V1.push_back(threevector(V1_x,V1_y,V1_z));
      double V2_x=X[6];
      double V2_y=X[7];
      double V2_z=X[8];
      V2.push_back(threevector(V2_x,V2_y,V2_z));
      color.push_back(static_cast<colorfunc::Color>(int(X[9])));
   } // loop over index i labeling ascii file line number
   
//   for (unsigned int i=0; i<curr_time.size(); i++)
//   {
//      threevector delta_V=V2[i]-V1[i];
//      cout << "time = " << curr_time[i]
//           << " ID = " << segment_ID[i]
//           << " pass = " << pass_number[i]
//           << " V1 = " << V1[i] 
//           << " V2 = " << V2[i] 
//           << " dV = " << delta_V.magnitude()
//           << " color = " << color[i] 
//           << endl;
//   }
}

// ---------------------------------------------------------------------
// Member function reconstruct_HAFB_corner_dirs takes in aircraft and
// imageplane corner worldspace positions.  It returns the direction
// vectors from the camera's instantaneous location of the
// imageplane's corners.

void ObsFrustaGroup::reconstruct_HAFB_corner_dirs(
   const vector<threevector>& aircraft_posn,
   const vector<threevector>& corner_posns,
   vector<threevector>& UV_corner_dir)
{
   int corner_counter=0;

   for (unsigned int i=0; i<aircraft_posn.size(); i++)
   {
      threevector curr_aircraft_posn=aircraft_posn[i];
      for (unsigned int c=0; c<4; c++)
      {
         threevector curr_corner_posn=corner_posns[corner_counter++];
         UV_corner_dir.push_back(
            (curr_corner_posn-curr_aircraft_posn).unitvector());
      }
   } // loop over index i labeling frame number
}
