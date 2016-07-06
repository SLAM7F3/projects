// Note added on 1/10/10: Member function
// generate_SignPost_at_imageplane_location() should be moved into
// OBSFRUSTUM!


// Note added on 4/8/09: In
// OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup(),
// large section of code should almost certainly be replaced by some
// call to OBSFRUSTUM::build_current_frustum() !!!

// ==========================================================================
// OBSFRUSTAGROUP class member function definitions
// ==========================================================================
// Last modified on 8/15/13; 9/10/13; 12/17/13; 4/5/14
// ==========================================================================

#include <iomanip>
#include <set>
#include <string>
#include <osg/Switch>

#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "math/genmatrix.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "math/mathfuncs.h"
#include "osg/osgModels/MODELSGROUP.h"
#include "numrec/nrfuncs.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "geometry/polyline.h"
#include "math/rotation.h"
#include "time/timefuncs.h"
#include "osg/Transformer.h"
#include "video/videofuncs.h"
#include "osg/ViewFrustum.h"
#include "osg/osgWindow/WindowManager.h"

#include "geometry/pyramid.h"
#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void OBSFRUSTAGROUP::allocate_member_objects()
{
//   cout << "inside OBSFRUSTAGROUP::allocate_member_objs()" << endl;
   MoviesGroup_ptr=new MoviesGroup(3,get_pass_ptr(),AnimationController_ptr);
   set_OSGsubPAT_nodemask(0,1);
}		       

void OBSFRUSTAGROUP::initialize_member_objects()
{
   GraphicalsGroup_name="OBSFRUSTAGROUP";

   enable_OBSFRUSTA_blinking_flag=false;
   cross_fading_flag=false;
   flashlight_mode_flag=false;
   display_Pyramids_flag=true;
   quasirandom_tour_flag=false;
   alpha_vary_selected_image_flag=false;
   before_flyin_flag=before_flyout_flag=true;
   erase_other_OBSFRUSTA_flag=false;
   jump_to_apex_flag=true;
   play_OBSFRUSTA_as_movie_flag=false;
   project_frames_onto_zplane_flag=false;

   new_subfrustum_ID=-1;
   n_still_images=0;
//   frustum_color_counter=6;	// Start random frustum colors at cyan
   frustum_color_counter=3;	// Start random frustum colors at orange
   virtual_frame_counter=0;
   n_3D_rays=0;
   subfrustum_volume_alpha=0.25;
   subfrustum_color=colorfunc::red;
   groundplane_pi=fourvector(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);

   ArrowHUD_ptr=NULL;
   ArrowsGroup_ptr=NULL;
   subfrustum_bbox_ptr=NULL;
   z_ColorMap_ptr=NULL;
   ImageNumberHUD_ptr=NULL;
   PARENT_OBSFRUSTAGROUP_ptr=NULL;
   photogroup_ptr=NULL;
   PointCloudsGroup_ptr=NULL;
   PointCloud_ptr=NULL;
   PointsGroup_ptr=NULL;
   PolygonsGroup_ptr=NULL;
   PolyhedraGroup_ptr=NULL;
   subfrustum_downrange_distance=-1;
   sub_FRUSTUM_ptr=NULL;
   SUBFRUSTAGROUP_ptr=NULL;
   virtual_camera_ptr=NULL;
   virtual_OBSFRUSTUM_ptr=NULL;
   target_MODELSGROUP_ptr=NULL;
   DTED_ztwoDarray_ptr=NULL;

   cameraID_xyz_map_ptr=NULL;
   WindowManager_ptr=NULL;
   prev_OBSFRUSTUM_framenumber=-1;
   filter_alpha_value=-1;
   FOV_excess_fill_factor=1.05;

   image_vs_package_names_map_ptr=NULL;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<OBSFRUSTAGROUP>(
         this, &OBSFRUSTAGROUP::update_display));
}		       

OBSFRUSTAGROUP::OBSFRUSTAGROUP(
   Pass* PI_ptr,AnimationController* AC_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr,GO_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

OBSFRUSTAGROUP::OBSFRUSTAGROUP(
   Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   AnimationController* AC_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr,GO_ptr),
   AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();

   set_CM_3D_ptr(static_cast<osgGA::Terrain_Manipulator*>(CM_ptr));
}		       

OBSFRUSTAGROUP::~OBSFRUSTAGROUP()
{
//   cout << "inside OSBFRUSTAGROUP destructor, this = " << this << endl;

//   cout << "MoviesGroup_ptr = " << MoviesGroup_ptr << endl;

   delete image_vs_package_names_map_ptr;
   
   MoviesGroup_ptr->destroy_all_Movies();
   delete MoviesGroup_ptr;
   MoviesGroup_ptr=NULL;

   if (SUBFRUSTAGROUP_ptr != NULL)
   {
      SUBFRUSTAGROUP_ptr->destroy_all_OBSFRUSTA();
      delete SUBFRUSTAGROUP_ptr;
   }
   
   destroy_all_OBSFRUSTA();
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const OBSFRUSTAGROUP& f)
{
   int node_counter=0;
   for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=f.get_OBSFRUSTUM_ptr(n);
      outstream << "OBSFRUSTUM node # " << node_counter++ << endl;
      outstream << "OBSFRUSTUM = " << *OBSFRUSTUM_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

OBSFRUSTUM* OBSFRUSTAGROUP::get_selected_OBSFRUSTUM_ptr()
{
//   cout << "inside OBSFRUSTAGROUP::get_selected_OBSFRUSTUM_ptr()" <<  endl;
//   cout << "get_selected_Graphical_ID() = "
//        << get_selected_Graphical_ID() << endl;

   if (get_selected_Graphical_ID() < 0) return NULL;

   OBSFRUSTUM* selected_OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(
      get_selected_Graphical_ID());
   return selected_OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
// Member function set_Movie_visibility_flag() globally erases or
// unerases all OBSFRUSTA movies.

void OBSFRUSTAGROUP::set_Movie_visibility_flag(bool flag)
{
   unsigned int n_OBSFRUSTA=get_n_Graphicals();
   for (unsigned int p=0; p<n_OBSFRUSTA; p++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(p);
//      cout << "p= " << p << " OBSFRUSTUM_ptr = " << OBSFRUSTUM_ptr << endl;
      OBSFRUSTUM_ptr->set_Movie_visibility_flag(flag);
   }
}

// ---------------------------------------------------------------------
// Member function get_selected_OBSFRUSTUM_photo_ID() returns the ID
// of the photo associated with the currently selected OBSFRUSTUM.
// Recall that OBSFRUSTA ID's (which generally increase monotonically)
// do NOT necessarily coincide with photo IDs (which may skip if they
// correspond to BUNDLER output).

int OBSFRUSTAGROUP::get_selected_OBSFRUSTUM_photo_ID()
{
//   cout << "inside OBSFRUSTAGROUP::get_selected_OBSFRUSTUM_photo_ID()"
//        <<  endl;

   OBSFRUSTUM* selected_OBSFRUSTUM_ptr=get_selected_OBSFRUSTUM_ptr();
   if (selected_OBSFRUSTUM_ptr==NULL) return -1;

   int photo_ID=selected_OBSFRUSTUM_ptr->get_Movie_ptr()->get_ID();
   return photo_ID;
}

// ---------------------------------------------------------------------
// Member function get_selected_photo_OBSFRUSTUM_ID() takes in the ID
// for some photo.  It returns the ID of the corresponding OBSFRUSTUM
// which holds the photo.  Recall that OBSFRUSTA ID's (which generally
// increase monotonically) do NOT necessarily coincide with photo IDs
// (which may skip if they correspond to BUNDLER output).

int OBSFRUSTAGROUP::get_selected_photo_OBSFRUSTUM_ID(int photo_ID) 
{
//   cout << "inside OBSFRUSTAGROUP::get_selected_photo_OBSFRUSTUM_ID()"
//        <<  endl;

   if (photo_ID < 0) return -1;

   PHOTO_VS_OBSFRUSTUM_ID_MAP::iterator iter=photo_obsfrustum_IDs_map.find(
      photo_ID);
   if (iter != photo_obsfrustum_IDs_map.end())
   {
      return iter->second;
   }
   else
   {
      return -1;
   }
}

// ---------------------------------------------------------------------
// Member function get_OBSFRUSTUM_photograph_ptr() takes in an ID for
// an OBSFRUSTUM which is assumed to hold a static photograph.  If so,
// this method returns a pointer to the photo.  Otherwise, it returns
// NULL.

photograph* OBSFRUSTAGROUP::get_OBSFRUSTUM_photograph_ptr(int OBSFRUSTUM_ID)
{
//   cout << "inside OBSFRUSTAGROUP::get_OBSFRUSTUM_photograph_ptr()"
//        <<  endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);
   if (OBSFRUSTUM_ptr==NULL) return NULL;
   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
   if (Movie_ptr==NULL) return NULL;
   photograph* photograph_ptr=Movie_ptr->get_photograph_ptr();
   return photograph_ptr;
}

// ---------------------------------------------------------------------
// Member function get_OBSFRUSTUM_photo_camera_ptr() takes in an ID
// for an OBSFRUSTUM which is assumed to hold a static photograph.  If
// so, this method returns a pointer to the photo's camera.
// Otherwise, it returns NULL.

camera* OBSFRUSTAGROUP::get_OBSFRUSTUM_photo_camera_ptr(int OBSFRUSTUM_ID)
{
//   cout << "inside OBSFRUSTAGROUP::get_OBSFRUSTUM_photo_camera_ptr()"
//        <<  endl;

   photograph* photograph_ptr=get_OBSFRUSTUM_photograph_ptr(OBSFRUSTUM_ID);
   if (photograph_ptr==NULL) return NULL;
   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   return camera_ptr;
}

// ---------------------------------------------------------------------
void OBSFRUSTAGROUP::set_PARENT_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* O_ptr)
{
   PARENT_OBSFRUSTAGROUP_ptr=O_ptr;
}

// ---------------------------------------------------------------------
void OBSFRUSTAGROUP::set_target_MODELSGROUP_ptr(MODELSGROUP* MG_ptr)
{
   target_MODELSGROUP_ptr=MG_ptr;
}

MODELSGROUP* OBSFRUSTAGROUP::get_target_MODELSGROUP_ptr()
{
   return target_MODELSGROUP_ptr;
}

const MODELSGROUP* OBSFRUSTAGROUP::get_target_MODELSGROUP_ptr() const
{
   return target_MODELSGROUP_ptr;
}

// ==========================================================================
// OBSFRUSTUM creation and manipulation member functions
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_OBSFRUSTUM from all other graphical insertion
// and manipulation methods...

OBSFRUSTUM* OBSFRUSTAGROUP::generate_new_OBSFRUSTUM(
   Movie* Movie_ptr,int OSGsubPAT_ID,int ID)
{
//   cout << "inside OBSFRUSTAGROUP::generate_new_OBSFRUSTUM(Movie_ptr)" 
//        << endl;
//   cout << "OSGsubPAT_ID = " << OSGsubPAT_ID 
//        << " ID = " << ID << endl;
//   cout << "movie_ptr = " << Movie_ptr << endl;
   
   if (ID==-1) ID=get_next_unused_ID();
   OBSFRUSTUM* curr_OBSFRUSTUM_ptr=new OBSFRUSTUM(
      pass_ptr,get_grid_world_origin_ptr(),
      AnimationController_ptr,Movie_ptr,ID);
   curr_OBSFRUSTUM_ptr->set_display_camera_model_flag(
      get_curr_t(),get_passnumber(),false);

   initialize_new_OBSFRUSTUM(curr_OBSFRUSTUM_ptr,Movie_ptr,OSGsubPAT_ID);

   return curr_OBSFRUSTUM_ptr;
}

OBSFRUSTUM* OBSFRUSTAGROUP::generate_new_OBSFRUSTUM(
   double az_extent,double el_extent,Movie* Movie_ptr,
   int OSGsubPAT_ID,int ID)
{
//   cout << "inside OBSFRUSTAGROUP::generate_new_OBSFRUSTUM(az_extent,el_extent,Movie_ptr)" << endl;

   if (ID==-1) ID=get_next_unused_ID();
   OBSFRUSTUM* curr_OBSFRUSTUM_ptr=new OBSFRUSTUM(
      pass_ptr,az_extent,el_extent,get_grid_world_origin_ptr(),
      AnimationController_ptr,Movie_ptr,ID);
   curr_OBSFRUSTUM_ptr->set_display_camera_model_flag(
      get_curr_t(),get_passnumber(),false);

   initialize_new_OBSFRUSTUM(curr_OBSFRUSTUM_ptr,Movie_ptr,OSGsubPAT_ID);

   return curr_OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
void OBSFRUSTAGROUP::initialize_new_OBSFRUSTUM(
   OBSFRUSTUM* OBSFRUSTUM_ptr,Movie* Movie_ptr,int OSGsubPAT_ID)
{
//   cout << "inside OBSFRUSTAGROUP::initialize_new_OBSFRUSTUM()" << endl;
//   cout << "OSGsubPAT_ID = " << OSGsubPAT_ID << endl;

   GraphicalsGroup::insert_Graphical_into_list(OBSFRUSTUM_ptr);
   initialize_Graphical(OBSFRUSTUM_ptr);

// Full and above-z-plane pyramids are specified in absolute
// coordinates.  So they should NOT be attached to the OBSFRUSTUM's
// PAT.  Instead, we directly attach their OSGgroup to the OSGsubPAT:

   insert_OSGgroup_into_OSGsubPAT(
      OBSFRUSTUM_ptr->get_PyramidsGroup_ptr()->get_OSGgroup_ptr(),
      OSGsubPAT_ID);

   OBSFRUSTUM_ptr->get_PyramidsGroup_ptr()->
      set_AnimationController_ptr(AnimationController_ptr);

// On 2/11/11, we followed Ross Anderson's advice to interpose an osg
// Switch between OBSFRUSTUM_ptr->get_PAT_ptr() and
// Movie_ptr->get_PAT_ptr().  The OBSFRUSTUM's movie can then be
// easily erased or unerased by calling the switch methods
// setAllLChildrenOn() and setAllChildrenOff():

   osg::ref_ptr<osg::Switch> Switch_refptr=new osg::Switch();
   if (Movie_ptr != NULL)
   {
      Switch_refptr->addChild(Movie_ptr->get_PAT_ptr());
      OBSFRUSTUM_ptr->get_PAT_ptr()->addChild(Switch_refptr.get());
      
//      OBSFRUSTUM_ptr->get_PAT_ptr()->addChild(Movie_ptr->get_PAT_ptr());
   }

/*
  if (OBSFRUSTUM_ptr->get_ModelsGroup_ptr() != NULL)
  {
  OBSFRUSTUM_ptr->get_PAT_ptr()->addChild(
  OBSFRUSTUM_ptr->get_ModelsGroup_ptr()->get_OSGgroup_ptr());
  }
*/
 
   insert_graphical_PAT_into_OSGsubPAT(OBSFRUSTUM_ptr,OSGsubPAT_ID);
}

// ---------------------------------------------------------------------
// Recall that OBSFRUSTUM's PyramidsGroup are not attached to the
// OBSFRUSTUM's PAT.  Instead, its OSGgroup is directly attached to
// the OSGsubPAT.  So we cannot simply "erase" each OBSFRUSTUM.
// Member function erase_OBSFRUSTUM instead explicitly sets their
// PyramidsGroup's OSGgroup nodemask to 0:

bool OBSFRUSTAGROUP::erase_OBSFRUSTUM(int n)
{
   if (get_Graphical_ptr(n)==NULL) return false;
   bool erased_OBSFRUSTUM_flag=erase_Graphical(get_Graphical_ptr(n)->get_ID());
   if (erased_OBSFRUSTUM_flag)
   {
      get_OBSFRUSTUM_ptr(n)->get_PyramidsGroup_ptr()->set_OSGgroup_nodemask(0);
   }
   return erased_OBSFRUSTUM_flag;
}

bool OBSFRUSTAGROUP::unerase_OBSFRUSTUM(int n,bool display_Pyramids_flag)
{
   if (get_Graphical_ptr(n)==NULL) return false;
   bool unerased_OBSFRUSTUM_flag=unerase_Graphical(
      get_Graphical_ptr(n)->get_ID());

   if (unerased_OBSFRUSTUM_flag && display_Pyramids_flag)
   {
      get_OBSFRUSTUM_ptr(n)->get_PyramidsGroup_ptr()->
         set_OSGgroup_nodemask(1);
   }
   return unerased_OBSFRUSTUM_flag;
}

void OBSFRUSTAGROUP::erase_all_OBSFRUSTA()
{
//   cout << "inside OBSFRUSTAGROUP::erase_all_OBSFRUSTA()" << endl;
//   cout << "get_n_Graphicals() = " << get_n_Graphicals() << endl;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      erase_OBSFRUSTUM(n);
   }
}

void OBSFRUSTAGROUP::unerase_all_OBSFRUSTA()
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      unerase_OBSFRUSTUM(n);
   }
}

void OBSFRUSTAGROUP::fast_erase_all_OBSFRUSTA()
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      get_OBSFRUSTUM_ptr(n)->get_PAT_ptr()->setNodeMask(0);
      get_OBSFRUSTUM_ptr(n)->get_PyramidsGroup_ptr()->set_OSGgroup_nodemask(0);
      get_OBSFRUSTUM_ptr(n)->get_Movie_ptr()->get_PAT_ptr()->setNodeMask(0);
   }
}

void OBSFRUSTAGROUP::fast_unerase_OBSFRUSTUM(int ID)
{
   get_ID_labeled_OBSFRUSTUM_ptr(ID)->get_PAT_ptr()->setNodeMask(1);
   get_ID_labeled_OBSFRUSTUM_ptr(ID)->get_PyramidsGroup_ptr()->
      set_OSGgroup_nodemask(1);
   fast_unerase_OBSFRUSTUM_Movie(ID);
}

void OBSFRUSTAGROUP::fast_unerase_OBSFRUSTUM_Movie(int ID)
{
   get_ID_labeled_OBSFRUSTUM_ptr(ID)->get_Movie_ptr()->
      get_PAT_ptr()->setNodeMask(1);
}

// ---------------------------------------------------------------------
// Member function destroy_OBSFRUSTUM

void OBSFRUSTAGROUP::destroy_OBSFRUSTUM(OBSFRUSTUM* curr_OBSFRUSTUM_ptr)
{
//   cout << "inside OBSFRUSTAGROUP::destroy_OBSFRUSTUM()" << endl;
//   cout << "curr_OBSFRUSTUM_ptr = " << curr_OBSFRUSTUM_ptr << endl;

   if (curr_OBSFRUSTUM_ptr==NULL) return;

   PyramidsGroup* PyramidsGroup_ptr=curr_OBSFRUSTUM_ptr->
      get_PyramidsGroup_ptr();
   if (PyramidsGroup_ptr != NULL)
   {
      remove_OSGgroup_from_OSGsubPAT(PyramidsGroup_ptr->get_OSGgroup_ptr());
   }

   Movie* Movie_ptr=curr_OBSFRUSTUM_ptr->get_Movie_ptr();
   if (Movie_ptr != NULL && curr_OBSFRUSTUM_ptr->get_PAT_ptr() != NULL)
   {
      curr_OBSFRUSTUM_ptr->get_PAT_ptr()->removeChild(
         Movie_ptr->get_PAT_ptr());
   }

   destroy_Graphical(curr_OBSFRUSTUM_ptr);
}

// ---------------------------------------------------------------------
void OBSFRUSTAGROUP::destroy_all_OBSFRUSTA()
{
//   cout << "inside OBSFRUSTAGROUP::destroy_all_OBSFRUSTA()" << endl;

   unsigned int n_OBSFRUSTA=get_n_Graphicals();
   vector<OBSFRUSTUM*> OBSFRUSTA_to_destroy;
   for (unsigned int p=0; p<n_OBSFRUSTA; p++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(p);
//      cout << "p = " << p << " OBSFRUSTUM_ptr = " << OBSFRUSTUM_ptr << endl;
      OBSFRUSTA_to_destroy.push_back(OBSFRUSTUM_ptr);
   }

   for (unsigned int p=0; p<n_OBSFRUSTA; p++)
   {
      destroy_OBSFRUSTUM(OBSFRUSTA_to_destroy[p]);
   }

// Must reset virtual_OBSFRUSTUM_ptr member to NULL!

   virtual_OBSFRUSTUM_ptr=NULL;
}

// ---------------------------------------------------------------------
// Member function generate_Movie() generates a new Movie object.  

Movie* OBSFRUSTAGROUP::generate_Movie(
   string movie_filename,double alpha,int Movie_ID)
{
//   cout << "inside OBSFRUSTAGROUP::generate_movie()" << endl;
//   cout << "movie_filename = " << movie_filename << endl;
//   cout << "Movie_ID = " << Movie_ID << endl;
//   cout << "get_MoviesGroup_ptr() = " << get_MoviesGroup_ptr() << endl;

   Movie* Movie_ptr=MoviesGroup_ptr->generate_new_Movie(
      movie_filename,alpha,Movie_ID);
   if (Movie_ptr==NULL) return NULL;

//   cout << "MoviesGroup_ptr = " << MoviesGroup_ptr << endl;
//   cout << "MoviesGroup_ptr->get_n_Graphicals() = "
//        << MoviesGroup_ptr->get_n_Graphicals() << endl;

   AnimationController_ptr->set_nframes(basic_math::max(
      AnimationController_ptr->get_nframes(),Movie_ptr->get_Nimages()));
   return Movie_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_movie_OBSFRUSTUM generates a new
// OBSFRUSTUM and a new Movie object.  It places the latter within the
// former such that the camera which took the movie frames is set at
// the OBSFRUSTUM's vertex.  A pointer to the dynamically generated
// OBSFRUSTUM object is returned by this method.

OBSFRUSTUM* OBSFRUSTAGROUP::generate_movie_OBSFRUSTUM(
   string movie_filename,int OSGsubPAT_ID,double alpha,int movie_ID,int ID)
{
//   cout << "inside OBSFRUSTAGROUP::generate_movie_OBSFRUSTUM()" << endl;
//   cout << "movie_filename = " << movie_filename << endl;
//   cout << "OSGsubPAT_ID = " << OSGsubPAT_ID << endl;
//   cout << "alpha = " << alpha << endl;

   Movie* Movie_ptr=generate_Movie(movie_filename,alpha,movie_ID);
   OBSFRUSTUM* OBSFRUSTUM_ptr=generate_new_OBSFRUSTUM(
      Movie_ptr,OSGsubPAT_ID,ID);
//   cout << "OBSFRUSTUM_ptr = " << OBSFRUSTUM_ptr << endl;

   return OBSFRUSTUM_ptr;
}

// ==========================================================================
// Circularly ordered OBSFRUSTA member functions
// ==========================================================================

// Member function read_circularly_ordered_OBSFRUSTA_IDs()

void OBSFRUSTAGROUP::read_circularly_ordered_OBSFRUSTA_IDs()
{
//   cout << "inside OBSFRUSTAGROUP::read_circularly_ordered_OBSFRUSTA_IDs()" 
//        << endl;

   string circularly_ordered_OBSFRUSTA_IDs_filename="radial_zones.dat";
   filefunc::ReadInfile(circularly_ordered_OBSFRUSTA_IDs_filename);
   
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      
      int n_circularly_related_IDs=column_values.size();
      for (int c=0; c<n_circularly_related_IDs; c++)
      {
         int curr_ID=column_values[c];
         int right_neighbor_ID=column_values[
            modulo(c+1,n_circularly_related_IDs)];
         int left_neighbor_ID=column_values[
            modulo(c-1,n_circularly_related_IDs)];
         OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(curr_ID);
         OBSFRUSTUM_ptr->set_right_neighbor_ID(right_neighbor_ID);
         OBSFRUSTUM_ptr->set_left_neighbor_ID(left_neighbor_ID);
      } // loop over index c labeling circularly related OBSFRUSTA IDs
   } // loop over index i labeling input file lines
}

// ---------------------------------------------------------------------
// Member function display_left_right_OBSFRUSTA_selection_symbols()
// checks if the Terrain Manipulator is rotating about the current
// eyepoint and an OBSFRUSTUM has been selected.  It also requires
// that the right and/or left neighbor OBSFRUSTA IDs be defined.  And
// it waits until the virtual camera's direction vector is nearly
// aligned with the selected OBSFRUSTUM's.  If all of these criteria
// are satisifed, this method displays left and right arrow HUDs to
// indicate that a user can move from the selected OBSFRUSTUM to its
// nearby neighbors in azimuthal angle.

void OBSFRUSTAGROUP::display_left_right_OBSFRUSTA_selection_symbols()
{
//   cout << "inside OBSFRUSTAGROUP::display_left_right_OBSFRUSTA_selection_symbols()" << endl;

   if (!get_CM_3D_ptr()->get_rotate_about_current_eyepoint_flag())
   {
      ArrowHUD_ptr->set_E_nodemask(0);
      ArrowHUD_ptr->set_W_nodemask(0);
      return;
   }

  OBSFRUSTUM* selected_OBSFRUSTUM_ptr=get_selected_OBSFRUSTUM_ptr();
   if (selected_OBSFRUSTUM_ptr==NULL) return;

   int right_neighbor_ID=selected_OBSFRUSTUM_ptr->get_right_neighbor_ID();
   int left_neighbor_ID=selected_OBSFRUSTUM_ptr->get_left_neighbor_ID();
   
   if (right_neighbor_ID < 0 && left_neighbor_ID < 0) return;

//   cout << "right neighbor ID = " << right_neighbor_ID << endl;
//   cout << "left neighbor ID = " << left_neighbor_ID << endl;

   Movie* Movie_ptr=selected_OBSFRUSTUM_ptr->get_Movie_ptr();
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   threevector What=camera_ptr->get_What();
   double curr_dotproduct=What.dot(get_CM_3D_ptr()->get_camera_Zhat());
//   cout << "What.Zhat = " << What.dot(get_CM_3D_ptr()->get_camera_Zhat())
//        << endl;

   if (curr_dotproduct < 0.999) return;

   ArrowHUD_ptr->set_E_nodemask(1);
   ArrowHUD_ptr->set_W_nodemask(1);
}

// ---------------------------------------------------------------------
bool OBSFRUSTAGROUP::move_to_right_OBSFRUSTUM_neighbor()
{
//   cout << "inside OBSFRUSTAGROUP::move_to_right_OBSFRUSTUM_neighbor()" 
//        << endl;
   
   OBSFRUSTUM* OBSFRUSTUM_ptr=get_selected_OBSFRUSTUM_ptr();
   if (OBSFRUSTUM_ptr==NULL) return false;
   
   int right_neighbor_ID=OBSFRUSTUM_ptr->get_right_neighbor_ID();
   if (right_neighbor_ID < 0) return false;

   cout << "right_neighbor_ID = " << right_neighbor_ID << endl;
   int n_anim_steps=5;
   double t_flight=1;	// secs
   fly_to_entered_OBSFRUSTUM(right_neighbor_ID,n_anim_steps,t_flight);
   return true;
}

bool OBSFRUSTAGROUP::move_to_left_OBSFRUSTUM_neighbor()
{
//   cout << "inside OBSFRUSTAGROUP::move_to_left_OBSFRUSTUM_neighbor()" 
//        << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_selected_OBSFRUSTUM_ptr();
   if (OBSFRUSTUM_ptr==NULL) return false;
   
   int left_neighbor_ID=OBSFRUSTUM_ptr->get_left_neighbor_ID();
   if (left_neighbor_ID < 0) return false;

   cout << "left_neighbor_ID = " << left_neighbor_ID << endl;
   int n_anim_steps=5;
   double t_flight=1;	// sec
   fly_to_entered_OBSFRUSTUM(left_neighbor_ID,n_anim_steps,t_flight);
   return true;
}

// ==========================================================================
// OBSFRUSTA display member functions
// ==========================================================================

// Member function set_altitude_dependent_OBSFRUSTA_volume_alpha()
// retrieves the current altitude of the virtual camera.  It resets
// the alpha for the OBSFRUSTA within input OBSFRUSTUM_ptrs as a
// linear function of the altitude.

void OBSFRUSTAGROUP::set_altitude_dependent_OBSFRUSTA_volume_alpha()
{
//   cout << "inside OBSFRUSTAGROUP::set_altitude_dependent_OBSFRUSTA_volume_alpha()" 
//        << endl;

   const double zmin=250*1000;	// meters
   const double zmax=1500*1000;	// meters
   const double alpha_max=0.35;

   double volume_alpha=compute_altitude_dependent_alpha(zmin,zmax,alpha_max);

   colorfunc::Color SideEdgeColor=colorfunc::white;
   colorfunc::Color ZplaneEdgeColor=colorfunc::white;
   colorfunc::Color VolumeColor=colorfunc::white;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      get_OBSFRUSTUM_ptr(n)->set_color(
         SideEdgeColor,ZplaneEdgeColor,VolumeColor,volume_alpha);
   }
}

// ==========================================================================
// Still imagery OBSFRUSTA generation member functions:
// ==========================================================================

// Member function generate_still_imagery_frusta_from_photogroup()
// loops over all photographs within input *photogroup_ptr.  It
// extracts intrinsic camera calibration parameters as well as az, el
// and roll rotation plus camera position parameters.  This method
// instantiates an OBSFRUSTUM for each image for which such camera
// calibration information is specified at the command line.

void OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup(
   photogroup* photogroup_ptr,bool multicolor_frusta_flag,
   bool thumbnails_flag)
{
//   cout << "inside OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup #1" << endl;

   double frustum_sidelength=-1;
   double movie_downrange_distance=-1;
   generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,photogroup_ptr->get_n_photos(),
      frustum_sidelength,movie_downrange_distance,
      multicolor_frusta_flag,thumbnails_flag);
}

void OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup(
   photogroup* photogroup_ptr,
   double frustum_sidelength,double movie_downrange_distance,
   bool multicolor_frusta_flag,bool thumbnails_flag)
{
//   cout << "inside OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup #2" << endl;

   generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,photogroup_ptr->get_n_photos(),
      frustum_sidelength,movie_downrange_distance,
      multicolor_frusta_flag,thumbnails_flag);
}

void OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup(
   photogroup* photogroup_ptr,int n_still_images,
   double frustum_sidelength,double movie_downrange_distance,
   bool multicolor_frusta_flag,bool thumbnails_flag)
{
//   cout << "inside OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup #3" << endl;

   threevector camera_posn(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,n_still_images,camera_posn,
      frustum_sidelength,movie_downrange_distance,
      multicolor_frusta_flag,thumbnails_flag);
}

void OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup(
   photogroup* photogroup_ptr,const threevector& camera_posn,
   double frustum_sidelength,double movie_downrange_distance,
   bool multicolor_frusta_flag,bool thumbnails_flag)
{
//   cout << "inside OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup #4" << endl;

   generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,photogroup_ptr->get_n_photos(),camera_posn,
      frustum_sidelength,movie_downrange_distance,
      multicolor_frusta_flag,thumbnails_flag);
}

void OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup(
   photogroup* photogroup_ptr,int n_still_images,
   const threevector& camera_posn,
   double common_frustum_sidelength,double common_movie_downrange_distance,
   bool multicolor_frusta_flag,bool thumbnails_flag)
{
//   cout << "inside OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup #5" << endl;

   threevector global_camera_translation(0,0,0);
   double global_daz=0;
   double global_del=0;
   double global_droll=0;
   double local_spin_daz=0;
   threevector rotation_origin(0,0,0);

   generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,photogroup_ptr->get_n_photos(),camera_posn,
      global_camera_translation,global_daz,global_del,global_droll,
      rotation_origin,local_spin_daz,common_frustum_sidelength,
      common_movie_downrange_distance,multicolor_frusta_flag,thumbnails_flag);
}

// This next overloaded version of
// generate_still_imagery_frusta_for_photogroup() is tailored for use
// by RASR main program GSTREETS:

void OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup(
   photogroup* photogroup_ptr,const threevector& global_camera_translation,
   double global_daz,double global_del,double global_droll,
   const threevector& rotation_origin,double local_spin_daz,
   colorfunc::Color OBSFRUSTUM_color,bool thumbnails_flag)
{
//   cout << "inside OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup #6" << endl;

   int n_still_images=photogroup_ptr->get_n_photos();
   threevector camera_posn(NEGATIVEINFINITY,NEGATIVEINFINITY,
                           NEGATIVEINFINITY);
   double common_frustum_sidelength=-1;
   double common_movie_downrange_distance=-1;
   bool multicolor_frusta_flag=false;
   generate_still_imagery_frusta_for_photogroup(
      photogroup_ptr,n_still_images,camera_posn,global_camera_translation,
      global_daz,global_del,global_droll,rotation_origin,local_spin_daz,
      common_frustum_sidelength,common_movie_downrange_distance,
      multicolor_frusta_flag,thumbnails_flag,OBSFRUSTUM_color);
}

// This final version of
// generate_still_imagery_frusta_for_photogroup() takes in
// global_camera_translation as well as global azimuth, elevation and
// roll rotation angles (along with a rotation origin).  As of January
// 2010, this method can rotate the cameras of every OBSFRUSTUM about
// rotation_origin within *this about +z_hat by global_daz (measured
// in radians).  It then translates each camera by
// global_camera_translation.  We built in this extra flexibility in
// order to qualitatively register the RASR hallway panorama sequence
// with the G76 robot ladar map.

void OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup(
   photogroup* photogroup_ptr,int n_still_images,
   const threevector& camera_posn,
   const threevector& global_camera_translation,
   double global_daz,double global_del,double global_droll,
   const threevector& rotation_origin,double local_spin_daz,
   double common_frustum_sidelength,double common_movie_downrange_distance,
   bool multicolor_frusta_flag,bool thumbnails_flag,
   colorfunc::Color OBSFRUSTUM_color)
{
//   cout << "inside OBSFRUSTAGROUP::generate_still_imagery_frusta_for_photogroup #7" << endl;
//   cout << "n_still_images = " << n_still_images << endl;
//   cout << "thumbnails_flag = " << thumbnails_flag << endl;
//   cout << "multicolor_frusta_flag = " << multicolor_frusta_flag << endl;
//   cout << "global_daz = " << global_daz*180/PI << endl;
//   cout << "global_del = " << global_del*180/PI << endl;
//   cout << "global_roll = " << global_droll*180/PI << endl;
//   cout << "local_spin_daz = " << local_spin_daz*180/PI << endl;
//   cout << "common_frustum_sidelength = " << common_frustum_sidelength << endl;
//   cout << "OBSFRUSTUM_color = " 
//        <<  colorfunc::get_colorstr(OBSFRUSTUM_color) << endl;

   this->photogroup_ptr=photogroup_ptr;

//    int n_start=photogroup_ptr->get_first_node_ID();
//   cout << "n_start = " << n_start 
//        << " n_still_images = " << n_still_images << endl;

//   for (int n=n_start; n< n_start+n_still_images; n++)
   for (int n=0; n< n_still_images; n++)
   {
      photograph* photograph_ptr=photogroup_ptr->
         get_ordered_photograph_ptr(n);

//      colorfunc::Color OBSFRUSTUM_color=colorfunc::white;
      if (multicolor_frusta_flag)
      {
         OBSFRUSTUM_color=colorfunc::get_color(n);
      }
//      cout << "OBSFRUSTUM_color = " << OBSFRUSTUM_color << endl;

// FAKE FAKE:  hardwire OBSFRUSTUM color to red for viewgraph purposes only
// Thurs Jun 7, 2012 at 11:51 am
//      OBSFRUSTUM_color=colorfunc::red;

      generate_still_image_frustum_for_photograph(
         photograph_ptr,camera_posn,global_camera_translation,
         global_daz,global_del,global_droll,rotation_origin,local_spin_daz,
         common_frustum_sidelength,common_movie_downrange_distance,
         OBSFRUSTUM_color,thumbnails_flag);
   } // loop over index n labeling photographs
}

// ---------------------------------------------------------------------
// Member function generate_still_image_frustum_for_photograph()
// takes in global_camera_translation as well as global azimuth,
// elevation and roll rotation angles (along with a rotation origin).
// As of January 2010, this method can rotate an OBSFRUSTUM's camera
// about rotation_origin about +z_hat by global_daz (measured in
// radians).  It then translates the camera by
// global_camera_translation.  We built in this extra flexibility in
// order to qualitatively register the RASR hallway panorama sequence
// with the G76 robot ladar map.

OBSFRUSTUM* OBSFRUSTAGROUP::generate_still_image_frustum_for_photograph(
   photograph* photograph_ptr,const threevector& camera_posn,
   const threevector& global_camera_translation,
   double global_daz,double global_del,double global_droll,
   const threevector& rotation_origin,
   double common_frustum_sidelength,double common_movie_downrange_distance,
   colorfunc::Color OBSFRUSTUM_color,bool thumbnails_flag)
{
   double local_spin_daz=0;
   return generate_still_image_frustum_for_photograph(
      photograph_ptr,camera_posn,global_camera_translation,
      global_daz,global_del,global_droll,rotation_origin,local_spin_daz,
      common_frustum_sidelength,common_movie_downrange_distance,
      OBSFRUSTUM_color,thumbnails_flag);
}

OBSFRUSTUM* OBSFRUSTAGROUP::generate_still_image_frustum_for_photograph(
   photograph* photograph_ptr,const threevector& camera_posn,
   const threevector& global_camera_translation,
   double global_daz,double global_del,double global_droll,
   const threevector& rotation_origin,double local_spin_daz,
   double common_frustum_sidelength,double common_movie_downrange_distance,
   colorfunc::Color OBSFRUSTUM_color,bool thumbnails_flag,
   Movie* input_Movie_ptr)
{
//   cout << "inside OBSFRUSTAGROUP::generate_still_image_frustum_for_photograph" << endl;

//   cout << "camera_posn = " << camera_posn << endl;
//   cout << "global_camera_translation = " 
//        << global_camera_translation << endl;
//   cout << "global_daz = " << global_daz 
//        << " global_del = " << global_del
//        << " global_droll = " << global_droll << endl;
//   cout << "rotation_origin = " << rotation_origin << endl;
//   cout << "local_spin_daz = " << local_spin_daz << endl;
//   cout << "common frustum_sidelength = " 
//        << common_frustum_sidelength << endl;
//   cout << "common downrange = " << common_movie_downrange_distance << endl;
//   cout << "OBSFRUSTUM_color = "
//        << colorfunc::get_colorstr(OBSFRUSTUM_color) << endl;
//   cout << "thumbnails_flag = " << thumbnails_flag << endl;

   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   double fu=camera_ptr->get_fu();
   double fv=camera_ptr->get_fv();
   double U0=camera_ptr->get_u0();
   double V0=camera_ptr->get_v0();

   threevector initial_camera_posn(camera_ptr->get_world_posn());
   double x=initial_camera_posn.get(0);
   double y=initial_camera_posn.get(1);
   double z=initial_camera_posn.get(2);
   double xprime=rotation_origin.get(0)+
      cos(global_daz)*x-sin(global_daz)*y;
   double yprime=rotation_origin.get(1)+
      sin(global_daz)*x+cos(global_daz)*y;
   threevector transformed_camera_posn=threevector(xprime,yprime,z)
      +global_camera_translation;

   double az=camera_ptr->get_rel_az()+global_daz;
   az += local_spin_daz;
   double el=camera_ptr->get_rel_el()+global_del;
   double roll=camera_ptr->get_rel_roll()+global_droll;
//   cout << "az = " << az*180/PI << " el = " << el*180/PI
//        << " roll = " << roll*180/PI << endl;

   photograph_ptr->reset_camera_parameters(
      fu,fv,U0,V0,az,el,roll,transformed_camera_posn);

// Group together all still images within common subgroup:

   int OSGsubPAT_ID=0;

// If thumbnails_flag==true, load thumbnail rather than full
// resolution versions of photographs into OBSFRUSTUM's Movie.  But
// set movie's filename to high-resolution photo even if
// thumbnails_flag==true so that full photo can be later loaded on
// demand:

   double alpha=1.0;
   string photo_filename=photograph_ptr->get_filename();
   int photo_ID=photograph_ptr->get_ID();
//   cout << "photo_ID = " << photo_ID
//        << " photo_filename = " << photo_filename << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=NULL;
   if (thumbnails_flag)
   {
      string thumbnail_filename=videofunc::get_thumbnail_filename(
         photo_filename);
      OBSFRUSTUM_ptr=generate_movie_OBSFRUSTUM(
         thumbnail_filename,OSGsubPAT_ID,alpha,photo_ID);
   }
   else
   {
      if (input_Movie_ptr==NULL)
      {
         OBSFRUSTUM_ptr=generate_movie_OBSFRUSTUM(
            photo_filename,OSGsubPAT_ID,alpha,photo_ID);
      }
      else
      {
         OBSFRUSTUM_ptr=generate_new_OBSFRUSTUM(
            input_Movie_ptr,OSGsubPAT_ID,photo_ID);
      }
   }

// Store OBSFRUSTUM_ID as a function of photo_ID within
// photo_obsfrustum_IDs_map:

   photo_obsfrustum_IDs_map[photo_ID]=OBSFRUSTUM_ptr->get_ID();

   OBSFRUSTUM_ptr->get_Movie_ptr()->set_video_filename(photo_filename);
   OBSFRUSTUM_ptr->get_Movie_ptr()->set_photograph_ptr(photograph_ptr);

// Instantiate initially unrotated camera corresponding to
// OBSFRUSTUM's movie:

   if (input_Movie_ptr==NULL)
      OBSFRUSTUM_ptr->get_Movie_ptr()->set_camera_ptr(camera_ptr);

// Reset camera's world position only if input camera_posn argument is
// "finite":

   if (camera_posn.get(0)+camera_posn.get(1)+camera_posn.get(2) >
       NEGATIVEINFINITY)
   {
      camera_ptr->set_world_posn(camera_posn);
      bool recompute_internal_params_flag=false;
      camera_ptr->construct_projection_matrix(recompute_internal_params_flag);
   }

   double frustum_sidelength=common_frustum_sidelength;
   double movie_downrange_distance=common_movie_downrange_distance;
      
// Note added on 4/8/09: This next section of code should almost
// certainly be replaced by some call to
// OBSFRUSTUM::build_current_frustum() !!!

//   cout << "common frustum length = " << common_frustum_sidelength
//        << " common downrange dist = " << common_movie_downrange_distance
//        << endl;

   if (common_frustum_sidelength < 0 && common_movie_downrange_distance < 0)
   {
      frustum_sidelength=photograph_ptr->get_frustum_sidelength();
      movie_downrange_distance=
         photograph_ptr->get_movie_downrange_distance();
   }
//   cout << "frustum_sidelength = " << frustum_sidelength
//        << " movie_downrange_distance = " << movie_downrange_distance
//        << endl;

   double volume_alpha=0;
//   cout << "flashlight_mode_flag = " << flashlight_mode_flag << endl;
   if (flashlight_mode_flag)
   {
      double z_grid=get_grid_world_origin_ptr()->get(2);
      double delta_z=0;
      double Zplane_altitude=z_grid+delta_z;

      OBSFRUSTUM_ptr->build_OBSFRUSTUM(
         get_curr_t(),get_passnumber(),Zplane_altitude,
         camera_ptr->get_world_posn(),camera_ptr->get_UV_corner_world_ray(),
         OBSFRUSTUM_color,volume_alpha);
   }
   else
   {
      OBSFRUSTUM_ptr->build_OBSFRUSTUM(
         get_curr_t(),get_passnumber(),
         frustum_sidelength,movie_downrange_distance,
         camera_ptr->get_world_posn(),camera_ptr->get_UV_corner_world_ray(),
         OBSFRUSTUM_color,volume_alpha);
   }

//   cout << "camera posn = " << camera_ptr->get_world_posn() << endl;
   return OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_second_image_plane() takes in the ID for
// some existing OBSFRUSTUM along with the filename for some photo.
// This method generates a second Movie and adds it to the
// OBSFRUSTUM's PAT.  It places the second movie slightly further
// downrange within the OBSFRUSTUM compared to the first image plane.

Movie* OBSFRUSTAGROUP::generate_second_image_plane(
   int OBSFRUSTUM_ID,string second_photo_filename)
{
   OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);
   
   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
   Movie* Movie2_ptr=generate_Movie(second_photo_filename);

   OBSFRUSTUM_ptr->get_PAT_ptr()->addChild(Movie2_ptr->get_PAT_ptr());

   double downrange_distance=
      OBSFRUSTUM_ptr->get_movie_downrange_distance()*1.001;
   OBSFRUSTUM_ptr->set_relative_Movie_window(
      Movie2_ptr,Movie_ptr->get_camera_ptr(),downrange_distance);

   return Movie2_ptr;
}

// ---------------------------------------------------------------------
// Member function
// generate_still_imagery_frusta_from_projection_matrices() extracts
// video filename, downrange distance and 3x4 projection matrix
// information from each video pass within input passes_group.  It
// then instantiates an OBSFRUSTUM for each still image.  The number
// of such frusta is returned by this method.

int OBSFRUSTAGROUP::generate_still_imagery_frusta_from_projection_matrices(
   PassesGroup& passes_group,double Zplane_altitude,
   bool multicolor_frusta_flag,bool initially_mask_all_frusta_flag)
{
//   cout << "inside OBSFRUSTAGROUP::generate_still_imagery_frusta_from_projection_matrices() #1" << endl;
   threevector relative_camera_translation(0,0,0);
   return generate_still_imagery_frusta_from_projection_matrices(
      passes_group,relative_camera_translation,Zplane_altitude,
      multicolor_frusta_flag,initially_mask_all_frusta_flag);
}

int OBSFRUSTAGROUP::generate_still_imagery_frusta_from_projection_matrices(
   PassesGroup& passes_group,
   const threevector& relative_camera_translation,double Zplane_altitude,
   bool multicolor_frusta_flag,bool initially_mask_all_frusta_flag)
{
//   cout << "inside OBSFRUSTAGROUP::generate_still_imagery_frusta_from_projection_matrices() #2" << endl;
//   cout << "relative_camera_translation = " << relative_camera_translation
//        << endl;
//   cout << "Zplane_altitude = " << Zplane_altitude << endl;
//   cout << "multicolor_frusta_flag = " << multicolor_frusta_flag << endl;
//   cout << "initially_mask_all_frusta_flag = " 
//        << initially_mask_all_frusta_flag << endl;

   int n_still_images=0;
   int curr_OSGsubPAT_ID,prev_OSGsubPAT_ID=-1;

   for (unsigned int n=0; n<passes_group.get_n_passes(); n++)
   {
      Pass* curr_pass_ptr=passes_group.get_pass_ptr(n);
      
      if (curr_pass_ptr->get_passtype()==Pass::video)
      {
         if (!extract_still_imagery_info(curr_pass_ptr))
         {
            continue;
         }

         frustum_color=colorfunc::white;
         if (multicolor_frusta_flag)
         {
            set_next_frustum_color(n_still_images);
         }
//         cout << "frustum_color = " << frustum_color << endl;

// The next several lines of ugly code map an input integer sequence
// consisting of -1 entries and/or continguous subsequences of random
// integer values into an ordered sequence of integers:

/*
  e.g. 

  Input integer		OSGsubPAT number

  -1				0
  -1				1
  7				2
  7				2
  -1				3
  8				4
  8				4
  8				4
  -1				5
  -1				6
  2				7
  2				7

*/

         curr_OSGsubPAT_ID=OSGsubPAT_number_given_Graphical[n_still_images];
         if (curr_OSGsubPAT_ID==-1)
         {
            prev_OSGsubPAT_ID=-1;
            curr_OSGsubPAT_ID=get_n_OSGsubPATs();
         }
         else
         {
            if (curr_OSGsubPAT_ID==prev_OSGsubPAT_ID)
            {
               curr_OSGsubPAT_ID=get_n_OSGsubPATs()-1;
            }
            else
            {
               prev_OSGsubPAT_ID=curr_OSGsubPAT_ID;
               curr_OSGsubPAT_ID=get_n_OSGsubPATs();
            }
         }
         
//         cout << "n_still_images = " << n_still_images
//              << " curr_OSGsubPAT_ID = " << curr_OSGsubPAT_ID
//              << endl;
         
         OBSFRUSTUM* OBSFRUSTUM_ptr=generate_still_image_OBSFRUSTUM(
            n_still_images,frustum_color,relative_camera_translation,
            Zplane_altitude,curr_OSGsubPAT_ID);

// Recall that OBSFRUSTUM's PyramidsGroup are not attached to the
// OBSFRUSTUM's PAT.  Instead, its OSGgroup is directly attached to
// the OSGsubPAT.  So we cannot simply "erase" each OBSFRUSTUM, but
// rather must explicitly set their PyramidsGroup's OSGgroup nodemask
// to 0:

         if (initially_mask_all_frusta_flag)
         {
            erase_OBSFRUSTUM(OBSFRUSTUM_ptr->get_ID());
         }

//          color_counter++;
         n_still_images++;
      }
   } // loop over index n labeling individual passes within passes_group

   return n_still_images;
}

// ---------------------------------------------------------------------
// Auxilliary member function set_next_frustum_color checks whether
// the next available color corresponding to input integer
// color_counter is too close to the Grid's purple color.  If so, it
// sets member variable frustum_color to a significantly different
// color.

void OBSFRUSTAGROUP::set_next_frustum_color(int n_frustum)
{
   frustum_color=colorfunc::string_to_color(frustum_colors[n_frustum]);
   if (frustum_color==colorfunc::null)
   {
      frustum_color=colorfunc::get_color(frustum_color_counter);
      
      if (frustum_color==colorfunc::pink ||
          frustum_color==colorfunc::purple ||
          frustum_color==colorfunc::magenta ||
          frustum_color==colorfunc::brightpurple)
      {
         frustum_color_counter++;
         frustum_color=colorfunc::get_color(frustum_color_counter);
      }
      frustum_color_counter++;
      if (frustum_color_counter >= 12) frustum_color_counter=0;
   }
}

// ---------------------------------------------------------------------
// Boolean member function extract_still_imagery_info() returns false
// if input videopass has a zero-valued 3x4 projection matrix.

bool OBSFRUSTAGROUP::extract_still_imagery_info(Pass* videopass_ptr)
{
//   cout << "inside OBSFRUSTAGROUP::extract_still_imagery_info()" << endl;

   string curr_still_filename=videopass_ptr->get_first_filename();

   genmatrix* curr_P_ptr=
      videopass_ptr->get_PassInfo_ptr()->get_projection_matrix_ptr();
//   cout << "curr_P_ptr = " << curr_P_ptr << endl;
   if (curr_P_ptr==NULL) return false;

   still_movie_filenames.push_back(curr_still_filename);
   OSGsubPAT_number_given_Graphical.push_back(
      videopass_ptr->get_PassInfo_ptr()->get_OSGsubPAT_ID());
   portrait_mode_flags.push_back(
      videopass_ptr->get_PassInfo_ptr()->get_portrait_mode_flag());
   frustum_colors.push_back(
      videopass_ptr->get_PassInfo_ptr()->get_frustum_color());
   frustum_sidelengths.push_back(
      videopass_ptr->get_PassInfo_ptr()->get_frustum_sidelength());
   downrange_distances.push_back(
      videopass_ptr->get_PassInfo_ptr()->get_downrange_distance());
   P_ptrs.push_back(curr_P_ptr);

   set_package_subdir(
      videopass_ptr->get_PassInfo_ptr()->get_package_subdir());
   set_package_filename_prefix(
      videopass_ptr->get_PassInfo_ptr()->get_package_filename_prefix());
   filter_alpha_value=
      videopass_ptr->get_PassInfo_ptr()->get_filter_alpha_value();
   static_camera_world_posn=
      videopass_ptr->get_PassInfo_ptr()->get_camera_posn();

//   cout << "portrait mode flag = " << portrait_mode_flags.back() << endl;
//   cout << "OSGsubPAT_number_given_Graphical = " 
//        << OSGsubPAT_number_given_Graphical.back() << endl;
//   cout << "frustum sidelength = " << frustum_sidelengths.back() << endl;
//   cout << "downrange dist = " << downrange_distances.back() << endl;
//   cout << "P = " << *(P_ptrs.back()) << endl;
//   cout << "package_subdir = " << package_subdir << endl;
//   cout << "package_filename_prefix = " << package_filename_prefix << endl;
//   cout << "static_camera_world_posn = " << static_camera_world_posn << endl;
   return true;
}

// ---------------------------------------------------------------------
// Member function reset_frustum_colors_based_upon_Zcolormap is a
// specialized method which we wrote in order to force OBSFRUSTA to be
// colored when viewed against grey scale pointclouds.  It first
// checks whether a height colormap has been defined and whether it
// currently corresponds to the grey colormap.  If so, this method
// resets OBSFRUSTUM within the current OBSFRUSTAGROUP to a separate
// color.  Otherwise, all OBSFRUSTA are recolored white.

void OBSFRUSTAGROUP::reset_frustum_colors_based_on_Zcolormap()
{
//   cout << "inside OBSFRUSTAGROUP::reset_frustum_colors_based_on_Zcolormap()" 
//        << endl;
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
//   cout << "multicolor_frusta_flag = " << multicolor_frusta_flag << endl;

   frustum_color_counter=3;
   double volume_alpha=0.0;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      frustum_color=colorfunc::white;

      if (multicolor_frusta_flag)
      {
         set_next_frustum_color(n);
      }

      OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(n);
      OBSFRUSTUM_ptr->set_permanent_color(frustum_color,volume_alpha);
      
      if (n_Graphical_siblings(OBSFRUSTUM_ptr) > 0)
      {
         if (multicolor_frusta_flag)
         {
            OBSFRUSTUM_ptr->set_selected_color(colorfunc::white,volume_alpha);
         }
         else
         {
//          OBSFRUSTUM_ptr->set_selected_color(colorfunc::white,volume_alpha);
          OBSFRUSTUM_ptr->set_selected_color(colorfunc::red,volume_alpha);

// As of 6/2/09, we do NOT want OBSFRUSTA in the LOS program to change
// color when they are selected:

//            OBSFRUSTUM_ptr->set_selected_color(
//               OBSFRUSTUM_ptr->get_permanent_color());
         }
      }
      else
      {
         if (multicolor_frusta_flag)
         {
            OBSFRUSTUM_ptr->set_selected_color(
               OBSFRUSTUM_ptr->get_permanent_color());
         }
         else
         {
            OBSFRUSTUM_ptr->set_selected_color(colorfunc::white,volume_alpha);
         }
      }

   } // loop over index n labeling OBSFRUSTA

   reset_colors();
}

// ---------------------------------------------------------------------
// Member function generate_still_image_OBSFRUSTUM

OBSFRUSTUM* OBSFRUSTAGROUP::generate_still_image_OBSFRUSTUM(
   int n_frustum,colorfunc::Color frustum_color,
   const threevector& relative_camera_translation,double Zplane_altitude,
   int OSGsubPAT_ID)
{
//   cout << "inside OBSFRUSTAGROUP::generate_still_image_OBSFRUSTUM()" << endl;
//   cout << "n_frustum = " << n_frustum << endl;
//   cout << "frustum_color = " << frustum_color << endl;
   
   OBSFRUSTUM* OBSFRUSTUM_ptr=generate_movie_OBSFRUSTUM(
      still_movie_filenames[n_frustum],OSGsubPAT_ID);
   if (OBSFRUSTUM_ptr==NULL) return NULL;

   camera* curr_camera_ptr=OBSFRUSTUM_ptr->get_Movie_ptr()->get_camera_ptr();
   curr_camera_ptr->set_projection_matrix(*(P_ptrs[n_frustum]));

//   cout << "camera posn = " << curr_camera_ptr->get_world_posn() << endl;
//   cout << "relative_camera_trans = " << relative_camera_translation
//        << endl;
//   cout << "UV_corner_world_ray = " << endl;
//   templatefunc::printVector(curr_camera_ptr->get_UV_corner_world_ray());
//   cout << "Uhat = " << curr_camera_ptr->get_Uhat() << endl;
//   cout << "Vhat = " << curr_camera_ptr->get_Vhat() << endl;

   OBSFRUSTUM_ptr->set_display_ViewingPyramid_flag(false);
   OBSFRUSTUM_ptr->set_display_ViewingPyramidAboveZplane_flag(true);
   OBSFRUSTUM_ptr->instantiate_OSG_Pyramids();

   curr_camera_ptr->set_world_posn(
      curr_camera_ptr->get_world_posn()+relative_camera_translation);
   if (!static_camera_world_posn.nearly_equal(Zero_vector))
   {
      curr_camera_ptr->set_world_posn(static_camera_world_posn);
   }

   double frustum_sidelength=frustum_sidelengths[n_frustum];
   double movie_downrange_distance=downrange_distances[n_frustum];

   OBSFRUSTUM_ptr->build_OBSFRUSTUM(
      get_curr_t(),get_passnumber(),
      frustum_sidelength,movie_downrange_distance,
      curr_camera_ptr->get_world_posn(),
      curr_camera_ptr->get_UV_corner_world_ray(),frustum_color);

   return OBSFRUSTUM_ptr;
}   

// ---------------------------------------------------------------------
// Member function generate_virtual_camera_OBSFRUSTUM() 

OBSFRUSTUM* OBSFRUSTAGROUP::generate_virtual_camera_OBSFRUSTUM()
{
   OBSFRUSTUM* OBSFRUSTUM_ptr=generate_new_OBSFRUSTUM();
   OBSFRUSTUM_ptr->set_virtual_camera_flag(true);
   OBSFRUSTUM_ptr->set_display_camera_model_flag(
      get_curr_t(),get_passnumber(),true);

   OBSFRUSTUM_ptr->set_display_ViewingPyramid_flag(false);
   OBSFRUSTUM_ptr->set_display_ViewingPyramidAboveZplane_flag(true);
   OBSFRUSTUM_ptr->instantiate_OSG_Pyramids();

   double Zplane_altitude=0;
   OBSFRUSTUM_ptr->build_OBSFRUSTUM(get_curr_t(),get_passnumber(),
                                    Zplane_altitude);

   double volume_alpha=0.1;
   OBSFRUSTUM_ptr->set_color(osg::Vec4(1,1,1,1),volume_alpha);	// white
   return OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_test_OBSFRUSTUM

OBSFRUSTUM* OBSFRUSTAGROUP::generate_test_OBSFRUSTUM(
   double z_ground,double sidelength,camera* camera_ptr)
{
   return generate_test_OBSFRUSTUM(
      z_ground,sidelength,camera_ptr->get_UV_corner_world_ray());
}

OBSFRUSTUM* OBSFRUSTAGROUP::generate_test_OBSFRUSTUM(
   double z_ground,double sidelength,const vector<threevector>& UV_corner_dir)
{
//   cout << "inside OBSFRUSTAGROUP::generate_test_OBSFRUSTUM" << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=generate_new_OBSFRUSTUM();

   OBSFRUSTUM_ptr->generate_or_reset_viewing_pyramid(
      Zero_vector,UV_corner_dir,sidelength);

   OBSFRUSTUM_ptr->orient_camera_model(
      get_curr_t(),get_passnumber(),UV_corner_dir);

   threevector Uhat((UV_corner_dir[1]-UV_corner_dir[0]).unitvector());
   threevector Vhat((UV_corner_dir[2]-UV_corner_dir[1]).unitvector());
   OBSFRUSTUM_ptr->set_UVW_dirs(get_curr_t(),get_passnumber(),Uhat,Vhat);

   return OBSFRUSTUM_ptr;
}   


// ==========================================================================
// Subfrusta member functions
// ==========================================================================

// Member function generate_SUBFRUSTAGROUP()

OBSFRUSTAGROUP* OBSFRUSTAGROUP::generate_SUBFRUSTAGROUP()
{
   cout << "inside OBSFRUSTAGROUP::generate_SUBFRUSTAGROUP()" << endl;

   SUBFRUSTAGROUP_ptr=new OBSFRUSTAGROUP(
      get_pass_ptr(),AnimationController_ptr,get_grid_world_origin_ptr());
   get_OSGgroup_ptr()->addChild(SUBFRUSTAGROUP_ptr->get_OSGgroup_ptr());
   
   SUBFRUSTAGROUP_ptr->set_PARENT_OBSFRUSTAGROUP_ptr(this);
   SUBFRUSTAGROUP_ptr->set_photogroup_ptr(photogroup_ptr);

   return SUBFRUSTAGROUP_ptr;
}

// ---------------------------------------------------------------------
// Member function load_SUBFRUSTA() is invoked by pressing 'c' within
// Manipulate Fused Data Mode.  It instantiates colored sub-frusta as
// well as 3D human models.

bool OBSFRUSTAGROUP::load_SUBFRUSTA()
{
   cout << "inside OBSFRUSTAGROUP::load_SUBFRUSTA()" << endl;

// Purge all existing subfrusta bounding boxes:

   SUBFRUSTAGROUP_ptr->get_subfrusta_bbox_ptrs().clear();

   if (get_selected_Graphical_ID() < 0) return false;

   OBSFRUSTUM* selected_OBSFRUSTUM_ptr=get_selected_OBSFRUSTUM_ptr();
   string photo_filename=
      selected_OBSFRUSTUM_ptr->get_Movie_ptr()->get_video_filename();
//   cout << "photo_filename = " << photo_filename << endl;

   string bundler_IO_subdir=photogroup_ptr->get_bundler_IO_subdir();
   string subfrusta_subdir=bundler_IO_subdir+"subfrusta/";
   string bboxes_filename=subfrusta_subdir+filefunc::getbasename(
      photo_filename)+".bboxes";
//   cout << "bboxes_filename = " << bboxes_filename << endl;

   if (!filefunc::fileexist(bboxes_filename)) return false;
   filefunc::ReadInfile(bboxes_filename);

   int subfrusta_counter=0;
   for (unsigned int i=0; i<filefunc::text_line.size(); i += 6)
   {
      double u_lo=stringfunc::string_to_number(filefunc::text_line[i+0]);
      double u_hi=stringfunc::string_to_number(filefunc::text_line[i+1]);
      double v_lo=stringfunc::string_to_number(filefunc::text_line[i+2]);
      double v_hi=stringfunc::string_to_number(filefunc::text_line[i+3]);
      double region_height=stringfunc::string_to_number(
         filefunc::text_line[i+4]);
      int color_code=stringfunc::string_to_number(
         filefunc::text_line[i+5]);
      bounding_box* subfrustum_bbox_ptr=new bounding_box(u_lo,u_hi,v_lo,v_hi);
      subfrustum_bbox_ptr->set_physical_deltaY(region_height);

      colorfunc::Color curr_color=colorfunc::white;
      if (color_code==0)
      {
         curr_color=colorfunc::cyan;
      }
      else if (color_code==1)
      {
         curr_color=colorfunc::yellow;
      }
      if (color_code==2)
      {
         curr_color=colorfunc::green;
      }
      
      subfrustum_bbox_ptr->set_color(curr_color);
      cout << "subfrustum bbox = " << *subfrustum_bbox_ptr << endl;
      subfrusta_counter++;

      SUBFRUSTAGROUP_ptr->get_subfrusta_bbox_ptrs().push_back(
         subfrustum_bbox_ptr);

   } // loop over index i labeling lines within input bbox text file

   return true;
}

// ---------------------------------------------------------------------
// Member function destroy_all_SUBFRUSTA() purges all existing
// SUBFRUSTA and human target MODELS.

void OBSFRUSTAGROUP::destroy_all_SUBFRUSTA()
{
   cout << "inside OBSFRUSTAGROUP::destroy_all_SUBFRUSTA()" << endl;

   cout << "get_sub_FRUSTUM_ptr() = " << get_sub_FRUSTUM_ptr() << endl;
   destroy_OBSFRUSTUM(get_sub_FRUSTUM_ptr());

   SUBFRUSTAGROUP_ptr->destroy_all_OBSFRUSTA();
   SUBFRUSTAGROUP_ptr->get_target_MODELSGROUP_ptr()->
      destroy_all_MODELS();
}

// ---------------------------------------------------------------------
// Member function update_subfrustum()

void OBSFRUSTAGROUP::update_subfrustum()
{
   cout << "inside OBSFRUSTAGROUP::update_subfrustum()" << endl;

   if (PARENT_OBSFRUSTAGROUP_ptr==NULL) return;

   set_new_subfrustum_ID(-1);
   
   OBSFRUSTUM* OBSFRUSTUM_ptr=PARENT_OBSFRUSTAGROUP_ptr->
      get_selected_OBSFRUSTUM_ptr();
   if (OBSFRUSTUM_ptr==NULL)
   {
      cout << "No parent OBSFRUSTUM selected!" << endl;
      return;
   }

   int photo_ID=PARENT_OBSFRUSTAGROUP_ptr->get_selected_OBSFRUSTUM_photo_ID();
//   cout << "photo_ID = " << photo_ID << endl;
   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(photo_ID);
//   cout << "photograph_ptr = " << photograph_ptr << endl;
   camera* camera_ptr=photograph_ptr->get_camera_ptr();
//   cout << "camera_ptr = " << camera_ptr << endl;

   OBSFRUSTUM* sub_FRUSTUM_ptr=
      generate_subfrustum(subfrustum_bbox_ptr,photograph_ptr,camera_ptr);

   if (subfrustum_downrange_distance < 0)
   {
      threevector feet_posn=insert_human_MODEL(sub_FRUSTUM_ptr,camera_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function update_subfrusta() is called when this =
// SUBFRUSTAGROUP_ptr.  It extracts UV imageplane coordinates for
// subfrusta from STL member vector subfrusta_bbox_ptrs.  

void OBSFRUSTAGROUP::update_subfrusta(double z_ground)
{
   cout << "inside OBSFRUSTAGROUP::update_subfrusta()" << endl;

   if (PARENT_OBSFRUSTAGROUP_ptr==NULL) return;

   OBSFRUSTUM* OBSFRUSTUM_ptr=PARENT_OBSFRUSTAGROUP_ptr->
      get_selected_OBSFRUSTUM_ptr();
   if (OBSFRUSTUM_ptr==NULL)
   {
      cout << "No parent OBSFRUSTUM selected!" << endl;
      return;
   }

   int photo_ID=PARENT_OBSFRUSTAGROUP_ptr->get_selected_OBSFRUSTUM_photo_ID();
//   cout << "photo_ID = " << photo_ID << endl;
   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(photo_ID);
//   cout << "photograph_ptr = " << photograph_ptr << endl;
   camera* camera_ptr=photograph_ptr->get_camera_ptr();
//   cout << "camera_ptr = " << camera_ptr << endl;

   vector<bounding_box*> subfrusta_bbox_ptrs=get_subfrusta_bbox_ptrs();
//   cout << "subfrusta_bbox_ptrs.size() = " << subfrusta_bbox_ptrs.size()
//        << endl;

   bool false_alarm_filter_flag=false;
   for (unsigned int b=0; b<subfrusta_bbox_ptrs.size(); b++)
   {
      OBSFRUSTUM* sub_FRUSTUM_ptr=
         generate_subfrustum(subfrusta_bbox_ptrs[b],photograph_ptr,camera_ptr);
//      cout << "b = " << b << " sub_FRUSTUM_ptr = " 
//           << sub_FRUSTUM_ptr << endl;

      if (subfrustum_downrange_distance < 0)
      {
         threevector feet_posn=insert_human_MODEL(
            sub_FRUSTUM_ptr,camera_ptr);

         if (!false_alarm_filter_flag) continue;

// Perform sanity check on human MODEL's altitude above ground.  If
// human MODEL lies significantly below or above ground, reject
// subfrustum bounding box as a false alarm:

         double distance_above_ground=feet_posn.get(2)-z_ground;

         const double min_rel_zfeet=-1.8;	// human height in meters
         const double max_rel_zfeet=1.8;	// human height in meters

         if (distance_above_ground < min_rel_zfeet ||
         distance_above_ground > max_rel_zfeet)
         {
            destroy_OBSFRUSTUM(sub_FRUSTUM_ptr);
            target_MODELSGROUP_ptr->destroy_last_created_MODEL();
         }
      }
      
   } // loop over index b labeling subfrusta bboxes

}

// ---------------------------------------------------------------------
// Member function generate_subfrustum() is called when this =
// SUBFRUSTAGROUP_ptr.  It takes in UV imageplane bounding box
// *bbox_ptr and computes the (human) target's range based upon the
// physical deltaY parameter specified within *bbox_ptr.  If the
// bounding box contains a non-white color, the subfrustum is given
// that color.  Otherwise, the subfrustum is randomly colored.

OBSFRUSTUM* OBSFRUSTAGROUP::generate_subfrustum(
   bounding_box* bbox_ptr,photograph* photograph_ptr,camera* camera_ptr)
{
   cout << "inside OBSFRUSTAGROUP::generate_subfrustum" << endl;
//   cout << "this = " << this << endl;
//   cout << "sub_FRUSTUM_ptr = " << sub_FRUSTUM_ptr << endl;
//   cout << "n_graphicals = " << get_n_Graphicals() << endl;

//   if (sub_FRUSTUM_ptr != NULL && get_n_Graphicals() > 0) 
//      destroy_OBSFRUSTUM(sub_FRUSTUM_ptr);
   
   Movie* Movie_ptr=NULL;
   int OSGsubPAT_ID=0;

   int Graphical_ID=get_Graphical_counter();
   cout << "Graphical_ID = " << Graphical_ID << endl;
   set_Graphical_counter(Graphical_ID-1);
   OBSFRUSTUM* sub_FRUSTUM_ptr=generate_new_OBSFRUSTUM(
      Movie_ptr,OSGsubPAT_ID,Graphical_ID);

   vector<threevector> bbox_corner_rays=camera_ptr->pixel_bbox_ray_directions(
      *bbox_ptr);

//   double width_to_height_ratio=camera_ptr->FOV_width_to_height_ratio(
//      bbox_corner_rays);
//   cout << "W/H = " << width_to_height_ratio << endl;

   double frustum_sidelength,downrange_distance;
   if (subfrustum_downrange_distance > 0)
   {
      downrange_distance=subfrustum_downrange_distance;
      const double fudge_factor=0.94;
      frustum_sidelength=fudge_factor
         *camera_ptr->compute_pyramid_sidelength(downrange_distance);
   }
   else
   {
      camera_ptr->target_downrange_distance(
         bbox_corner_rays,bbox_ptr->get_physical_deltaY(),
         frustum_sidelength,downrange_distance);
   }
//   cout << "frustum_sidelength = " << frustum_sidelength
//        << " downrange_distance = " << downrange_distance << endl;

   colorfunc::Color bbox_color=bbox_ptr->get_color();
   if (bbox_color != colorfunc::white)
   {
      subfrustum_color=bbox_color;
   }
   else
   {
      subfrustum_color=colorfunc::get_color(modulo(get_n_Graphicals(),15));
   }

   sub_FRUSTUM_ptr->build_OBSFRUSTUM(
      get_curr_t(),get_passnumber(),
      frustum_sidelength,downrange_distance,
      camera_ptr->get_world_posn(),bbox_corner_rays,
      subfrustum_color,subfrustum_volume_alpha);
   sub_FRUSTUM_ptr->set_permanent_color(subfrustum_color);
   
   return sub_FRUSTUM_ptr;
}

// ---------------------------------------------------------------------
// Member function insert_human_MODEL() generates a 3D human MODEL.
// It positions the MODEL so that its face matches the COM of the sub
// FRUSTUM's base pyramid.  We also assume that the human faces
// directly towards the camera.  So this method azimuthally rotates
// the 3D MODEL so that it looks towards the camera.  The position of
// the human model's feet in UTM gecoordinates is returned.

threevector OBSFRUSTAGROUP::insert_human_MODEL(
   OBSFRUSTUM* sub_FRUSTUM_ptr,camera* camera_ptr)
{
//   cout << "inside OBSFRUSTAGROUP::insert_human_MODEL()" << endl;
 
   if (target_MODELSGROUP_ptr==NULL) return Zero_vector;

//   target_MODELSGROUP_ptr->destroy_all_MODELS();
   int OSGsubPAT_number=0;
   MODEL* human_MODEL_ptr=target_MODELSGROUP_ptr->generate_man_MODEL(
      OSGsubPAT_number);

// Reset 3D human figure's position so that its [face's] height equals
// OBSFRUSTUM pyramid base's COM:

   pyramid* pyramid_ptr=sub_FRUSTUM_ptr->get_viewing_pyramid_ptr();
//   cout << "*sub frustum pyramid = " << *pyramid_ptr << endl;
   
   vertex apex=pyramid_ptr->get_apex();
   face* base_ptr=pyramid_ptr->get_base_ptr();
//   cout << "apex = " << apex << endl;
//   cout << "*base_ptr = " << *base_ptr << endl;
//   cout << "base_ptr->get_COM() = " << base_ptr->get_COM() << endl;
   threevector model_posn=base_ptr->get_COM();

   const double male_height=1.75;	// meters
//   model_posn -= 7.0/7.5*male_height*z_hat;	// manipulate human model's face  
   model_posn -= 0.5*male_height*z_hat;	// translate entire human model
   cout << "Human model's COM posn = " << model_posn << endl;

// Assume human faces straight towards camera.  Orient 3D human model
// so that it also looks toward camera:

   double roll=0;	// About -y_hat
   double pitch=0;	// About -x_hat
   double yaw=0;	// About +z_hat
   threevector What(camera_ptr->get_What());
   double theta=atan2(What.get(1),What.get(0));
//   cout << "theta = " << theta*180/PI << endl;
   yaw=theta+PI/2;

   human_MODEL_ptr->position_and_orient_man_MODEL(
      target_MODELSGROUP_ptr->get_curr_t(),
      target_MODELSGROUP_ptr->get_passnumber(),model_posn,roll,pitch,yaw);
//   target_MODELSGROUP_ptr->set_OSGsubPAT_nodemask(0,1);

   bool northern_hemisphere_flag=true;
   int UTM_zone=19;	// Boston
   geopoint human_geopoint(
      northern_hemisphere_flag,UTM_zone,
      model_posn.get(0),model_posn.get(1),model_posn.get(2));
   cout << "human_geopoint = " << human_geopoint << endl;

   human_MODEL_ptr->broadcast_human_geoposn(
      human_geopoint,target_MODELSGROUP_ptr->get_Messenger_ptr());

// Return position of human model's feet:

   model_posn -= 0.5*male_height*z_hat;
   return model_posn;
}

// ==========================================================================
// Animation member functions
// ==========================================================================

// Member function update_display()

void OBSFRUSTAGROUP::update_display()
{   
   if (!update_display_flag) return;

//   cout << "******************************************************" << endl;
//   cout << "inside OBSFRUSTAGROUP::update_display()" << endl;
//   cout << "this OBSFRUSTAGROUP  = " << this << endl;
//   cout << "OBSFRUSTAGROUP_ptr->get_n_Graphicals() = "
//        << get_n_Graphicals() << endl;

//   cout << "MoviesGroup_ptr = " << MoviesGroup_ptr << endl;
   if (MoviesGroup_ptr != NULL) 
   {
//      cout << "MoviesGroup_ptr->get_n_Graphicals() = "
//           << MoviesGroup_ptr->get_n_Graphicals() << endl;
      MoviesGroup_ptr->update_display();
   }

   if (play_OBSFRUSTA_as_movie_flag)
   {
      int curr_framenumber=AnimationController_ptr->get_curr_framenumber();
      int prev_framenumber=AnimationController_ptr->get_prev_framenumber();
      
      set_selected_Graphical_ID(curr_framenumber);
      OBSFRUSTUM* selected_OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(
         curr_framenumber);
      OBSFRUSTUM* prev_selected_OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(
         prev_framenumber);

      prev_selected_OBSFRUSTUM_ptr->set_permanent_color(colorfunc::white);
      selected_OBSFRUSTUM_ptr->set_permanent_color(colorfunc::red);

      issue_select_vertex_message();

      if (project_frames_onto_zplane_flag) project_curr_frame_onto_zplane();
   }

   parse_latest_messages();

// Call reset_colors to enable OBSFRUSTUM blinking:

   if (enable_OBSFRUSTA_blinking_flag) reset_colors();

   if (get_CM_refptr().valid() && erase_other_OBSFRUSTA_flag)
   {
      finalize_after_flying_in_and_out();
   }

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(n);
//      cout << "n = " << n << " OBSFRUSTUM_ptr = " << OBSFRUSTUM_ptr << endl;

// For Empire State Building video car tracking example, we need to
// update the OBSFRUSTUM's LineSegments and Cylinders:

      if (OBSFRUSTUM_ptr->get_LineSegmentsGroup_ptr() != NULL)
      {
         OBSFRUSTUM_ptr->get_LineSegmentsGroup_ptr()->update_display();
      }

      if (OBSFRUSTUM_ptr->get_CylindersGroup_ptr() != NULL)
      {
         OBSFRUSTUM_ptr->get_CylindersGroup_ptr()->update_display();
      }
    
      if (OBSFRUSTUM_ptr->get_PyramidsGroup_ptr() != NULL)
      {
         OBSFRUSTUM_ptr->get_PyramidsGroup_ptr()->update_display();

// Retrieve pyramid apex and z-plane face corners saved within
// ViewingPyramidAboveZplane graphical's time-dependent vertices.
// Recall that zeroth vertex contains pyramid's apex:

         vector<threevector> vertices,relative_vertices;

         Pyramid* ViewingPyramidAboveZplane_ptr=
            OBSFRUSTUM_ptr->get_last_Pyramid_ptr();

         ViewingPyramidAboveZplane_ptr->get_vertices(
            get_curr_t(),get_passnumber(),vertices);

//         pyramid* viewing_pyramid_above_zplane_ptr=
//            ViewingPyramidAboveZplane_ptr->get_pyramid_ptr();
//         vertices_handler* vertices_handler_ptr=
//            viewing_pyramid_above_zplane_ptr->get_vertices_handler_ptr();
      
//         for (int v=0; v<vertices_handler_ptr->get_n_vertices(); v++)
//         {
//            relative_vertices.push_back(
//               vertices_handler_ptr->get_vertex(v).get_posn()-
//               viewing_pyramid_above_zplane_ptr->get_apex().get_posn());
//            cout << "v = " << v 
//                 << " new relative vertex = " << relative_vertices.back()
//                 << endl;
//         }

// Attempt to update alpha-blended triangle mesh for translucent
// OBSFRUSTUM volume rendering purposes:

         if (vertices.size() > 0)
         {
            for (int v=0; v<int(vertices.size()); v++)
            {
//               cout << "v = " << v
//                    << " vertices[v] = " << vertices[v] << endl;
               relative_vertices.push_back(vertices[v]-vertices[0]);
//               cout << "relative_vertices[v] = "
//                    << relative_vertices.back() << endl;
            }

            ViewingPyramidAboveZplane_ptr->
               update_square_pyramid_triangle_mesh(relative_vertices);

         } // vertices.size() > 0 conditional

         update_OBSFRUSTUM_using_Movie_info(OBSFRUSTUM_ptr,vertices);

      } // OBSFRUSTUM_ptr->get_PyramidsGroup_ptr() != NULL conditional

   } // loop over index n labeling OBSFRUSTA

   int selected_ID=get_selected_Graphical_ID();

// For RASR viewer, fade selected photo to simulate zooming from 
// one wagon-wheel photo to next:

   if (selected_ID >= 0 && cross_fading_flag)
   {
      cross_fade_photo_pair();
   }

/*
   if (selected_ID >= 0 && !cross_fading_flag &&
       PolyhedraGroup_ptr != NULL && PolygonsGroup_ptr != NULL)
   {
      project_Polyhedra_into_selected_photo(selected_ID);
   }
*/

   if (selected_ID >= 0 && cameraID_xyz_map_ptr != NULL)
   {
//      display_visible_reconstructed_XYZ_points(selected_ID);
   }

   if (new_subfrustum_ID >= 0)
   {
      update_subfrustum();
   }

   if (quasirandom_tour_flag) conduct_quasirandom_tour();

   if (ArrowHUD_ptr != NULL)
   {
      display_left_right_OBSFRUSTA_selection_symbols();
   }

   GraphicalsGroup::update_display();
}

// ---------------------------------------------------------------------
// Member function project_curr_frame_onto_zplane() first retrieves
// the IDs for the currently selected OBSFRUSTUM plus some small
// number of its temporal predecessors.  It then retrieves the Movies
// for the selected OBSFRUSTA and sets their
// warp_onto_imageplane_flag members to true.  The Movies' cameras
// have their imageplanes set to Z-planes with
// Z=grid_world_origin.get(2)+delta_z.  All OBSFRUSTA except for the
// selected ones are erased.  Low-res thumbnails are loaded into the
// erased OBSFRUSTA, while hires photos are loaded into the selected
// OBSFRUSTA.

void OBSFRUSTAGROUP::project_curr_frame_onto_zplane()
{
//   cout << "inside OBSFRUSTAGROUP::project_curr_frame_onto_zplane()" 
//        << endl;

   int n_images_to_simultaneously_display=1;
//   int n_images_to_simultaneously_display=3;
//   int n_images_to_simultaneously_display=5;

   vector<int> selected_OBSFRUSTA_IDs;
   vector<OBSFRUSTUM*> selected_OBSFRUSTA_ptrs;
   for (int n=0; n<n_images_to_simultaneously_display; n++)
   {
      int curr_selected_OBSFRUSTUM_ID=get_selected_Graphical_ID(
         n_images_to_simultaneously_display-1-n);
//      cout << "curr_selected_OBSFRUSTUM_ID = "
//           << curr_selected_OBSFRUSTUM_ID << endl;
      
      if (curr_selected_OBSFRUSTUM_ID < 0) continue;
      
      selected_OBSFRUSTA_IDs.push_back(curr_selected_OBSFRUSTUM_ID);
      selected_OBSFRUSTA_ptrs.push_back(get_ID_labeled_OBSFRUSTUM_ptr(
         curr_selected_OBSFRUSTUM_ID));

      Movie* selected_Movie_ptr=selected_OBSFRUSTA_ptrs.back()->
         get_Movie_ptr();
//      cout << "selected_Movie_ptr = " << selected_Movie_ptr << endl;
      
      selected_Movie_ptr->set_warp_onto_imageplane_flag(true);

// Project imagery into a z-plane located delta_z meters below z_grid:

      double z_grid=get_grid_world_origin_ptr()->get(2);
      double delta_z=0;		// meters	
      camera* selected_camera_ptr=selected_Movie_ptr->get_camera_ptr();
      fourvector grid_plane_pi(0,0,1,-(z_grid+delta_z));
      selected_camera_ptr->set_imageplane(grid_plane_pi);

//      cout << "n = " << n
//           << " selected OBSFRUSTUM ID = " 
//           << selected_OBSFRUSTA_IDs.back() << endl;
   }

   fast_erase_all_OBSFRUSTA();
   fast_unerase_OBSFRUSTUM(selected_OBSFRUSTA_IDs.back());
   for (unsigned int n=0; n<selected_OBSFRUSTA_IDs.size()-1; n++)
   {
      int prev_selected_OBSFRUSTUM_ID=selected_OBSFRUSTA_IDs[n];
      if ((prev_selected_OBSFRUSTUM_ID) >= 0)
      {
         fast_unerase_OBSFRUSTUM_Movie(prev_selected_OBSFRUSTUM_ID);
      }
   }

   for (unsigned int n=0; n<selected_OBSFRUSTA_IDs.size(); n++)
   {
      load_hires_photo(selected_OBSFRUSTA_IDs[n]);
   }
   
/*
      OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(
         selected_OBSFRUSTA_IDs[n]);
      Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
      Movie_ptr->get_texture_rectangle_ptr()->
         convert_color_image_to_greyscale();
   }
*/
   
   int unselected_OBSFRUSTUM_ID=get_selected_Graphical_ID(
      n_images_to_simultaneously_display);
//   cout << "unselected_OBSFRUSTUM_ID = " 
//        << unselected_OBSFRUSTUM_ID << endl;
   load_thumbnail_photo(unselected_OBSFRUSTUM_ID);



   

//   cout << "At end of OBSFRUSTAGROUP::project_curr_frame_onto_zplane()"
//        << endl;
}

// ---------------------------------------------------------------------
// Member function update_OBSFRUSTUM_using_Movie_info() first checks
// whether the input OBSFRUSTUM has a nontrivial Movie associated with
// it.  If so, it tries to reset the OBSFRUSTUM's params if the Movie
// contains more than one frame.  It also projects the current track
// points (for the NYC ESB demo).  For the aerial HAFB video demo,
// this method projects the video clip onto a trapezoid within the
// world z-plane.  We separated off these lines from update_display()
// in order to make the latter method easier to read.

void OBSFRUSTAGROUP::update_OBSFRUSTUM_using_Movie_info(
   OBSFRUSTUM* OBSFRUSTUM_ptr,const vector<threevector>& vertices)
{
//   cout << "inside OBSFRUSTAGROUP::update_OBSFRUSTUM_using_Movie_info()"
//        << endl;
//   cout << "OBSFRUSTUM_ptr->get_ID() = "
//        << OBSFRUSTUM_ptr->get_ID() << endl;

   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
   if (Movie_ptr == NULL) return;

// Update OBSFRUSTUM parameters from package files if its Movie
// contains more than a single still frame:

   if (Movie_ptr->get_Nimages() > 1)
   {
      reset_OBSFRUSTUM_from_package_file(OBSFRUSTUM_ptr);
   }

// Following call made for NYC ESB and flight facility D7 video demos:

   OBSFRUSTUM_ptr->project_curr_track_points(
      Movie_ptr->get_tracks_map_ptr(),groundplane_pi); 

   if (!OBSFRUSTUM_ptr->get_rectangular_movie_flag() && vertices.size() > 0)
   {

// Recall HAFB video projects onto a trapezoidal "keystone" within the
// world z-plane rather than a rectangle.  In this case, we need to
// reset the video geometry's vertices on a per-frame basis:

      string video_basename=filefunc::getbasename(
         Movie_ptr->get_video_filename());
      if (video_basename=="HAFB_overlap_corrected_grey.vid")
      {
         Movie_ptr->reset_geom_vertices(
            vertices[3],vertices[4],vertices[1],vertices[2]);
      }
      else
      {

// Recompute Constant Hawk video chips based upon their instantaneous
// Zplane face corner locations:

         Movie_ptr->compute_3D_chip(
            vertices[3],vertices[4],vertices[1],vertices[2]);
      }
   } // !rectangular_movie_flag conditional
}

// --------------------------------------------------------------------------
// Member function fly_to_entered_OBSFRUSTUM() takes in the ID for
// some OBSFRUSTUM derived from either doubleclicking or by manually
// entering.  It resets the selected_Graphical_ID equal to the input
// ID.  This method then flies the virtual camera to the position and
// orientation of the selected OBSFRUSTUM's camera.  If the 

bool OBSFRUSTAGROUP::fly_to_entered_OBSFRUSTUM(
   int OBSFRUSTUM_ID,int n_anim_steps,double t_flight)
{
//   cout << "inside OBSFRUSTAGROUP::fly_to_entered_OBSFRUSTUM()" << endl;
//   cout << "OBSFRUSTUM_ID = " << OBSFRUSTUM_ID << endl;
//   cout << "get_selected_Graphical_ID() = "
//        << get_selected_Graphical_ID() << endl;

   if (OBSFRUSTUM_ID < 0) return false;

// When working with Noah Snavely's BUNDLER output, we need to unload
// any previously loaded hires photo before loading in a new hires
// photo:

   if (OBSFRUSTUM_ID != get_selected_Graphical_ID())
   {
      if (!cross_fading_flag) 
         load_thumbnail_photo(get_prev_selected_Graphical_ID());
      set_selected_Graphical_ID(OBSFRUSTUM_ID);
   }

// When working with Noah Snavely's BUNDLER output, erase all
// OBSFRUSTA except selected one in order to clearly see individual
// photos:

   if (get_erase_other_OBSFRUSTA_flag())
   {
      erase_all_OBSFRUSTA();

      if (cross_fading_flag)
      {
         unerase_OBSFRUSTUM(
            get_prev_selected_Graphical_ID(),display_Pyramids_flag);
         unerase_OBSFRUSTUM(OBSFRUSTUM_ID,display_Pyramids_flag);
      }
      else
      {
         unerase_OBSFRUSTUM(OBSFRUSTUM_ID);
      }

// FAKE FAKE:  Weds, Dec 16, 2009 at 11:30 am
// Experiment with warping all photos onto precalculated imageplanes

//      int Movie_ID=get_selected_OBSFRUSTUM_photo_ID();
//      Movie* selected_Movie_ptr=get_MoviesGroup_ptr()->
//         get_ID_labeled_Movie_ptr(Movie_ID);
//      selected_Movie_ptr->set_warp_onto_imageplane_flag(true);
//      cout << "Selected video filename = " 
//           << selected_Movie_ptr->get_video_filename() << endl;
//      OBSFRUSTUM* selected_OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(
//         get_selected_Graphical_ID());
//      cout << "selected_OBSFRUSTUM = " << *selected_OBSFRUSTUM_ptr << endl;

// FAKE FAKE:  Monday, Dec 7, 2009 at 6 am

// Experiment with resetting image planes for all OBSFRUSTA except
// selected one to common plane with selected one.  

//      reset_to_common_imageplane();

      load_hires_photo();

   } // erase_other_OBSFRUSA_flag conditional

   issue_select_vertex_message();

   flyto_camera_location(OBSFRUSTUM_ID,n_anim_steps,t_flight);

   if (play_OBSFRUSTA_as_movie_flag)
   {
      AnimationController_ptr->set_curr_framenumber(OBSFRUSTUM_ID);
   }

   return true;
}

// ---------------------------------------------------------------------
// Member function flyto_camera_location constructs an animation path
// from the virtual camera's current location to the apex of the
// OBSFRUSTUM labeled by the input ID.

void OBSFRUSTAGROUP::flyto_camera_location(
   int ID,int n_anim_steps,double t_flight)
{
//   cout << "inside OBSFRUSTAGroup::flyto_camera_location()" << endl;
//   cout << "n_anim_steps = " << n_anim_steps << endl;
//   cout << "ID = " << ID << endl;
//   cout << "AC_ptr->get_nframes() = " << AnimationController_ptr->
//      get_nframes() << endl;
   
   OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(ID);
//   cout << "OBSFRUSTUM_ptr = " << OBSFRUSTUM_ptr << endl;
   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
//   cout << "Movie_ptr = " << Movie_ptr << endl;
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
//   cout << "camera_ptr = " << camera_ptr << endl;

   threevector camera_world_posn=camera_ptr->get_world_posn();
   rotation Rcamera(*(camera_ptr->get_Rcamera_ptr()));
//   cout << "Rcamera = " << Rcamera << endl;

   threevector eye_world_posn=get_CM_3D_ptr()->get_eye_world_posn();
//   cout << "eye_world_posn = " << eye_world_posn << endl;
//   cout << "camera_world_posn = " << camera_world_posn << endl;
//   cout << "range = " << (eye_world_posn-camera_world_posn).magnitude()
//        << endl;

// Spin camera about line-of-sight if necessary:

   bool image_rotated_by_ninety_degrees_flag=false;
   Rcamera=OBSFRUSTUM_ptr->rotation_about_LOS(
      image_rotated_by_ninety_degrees_flag)*Rcamera;

//   cout << "camera posn = " << camera_world_posn << endl;
//   cout << "Rcamera.transpose() = " << Rcamera.transpose() << endl;

/*
// Before virtual camera flies to real camera's location, reset all
// OSGsubPAT images so that their alpha values equal unity:

   vector<Movie*> Movie_ptrs=find_Movies_in_OSGsubPAT();
   for (int i=0; i<int(Movie_ptrs.size()); i++)
   {
      Movie* Movie_ptr=Movie_ptrs.at(i);
      if (Movie_ptr != NULL)
      {
         Movie_ptr->set_alpha(1.0);
      }
   } // loop over Movies within Movie_ptrs
*/

// Adopt real camera's horizontal and vertical FOVs as final values
// for virtual camera's fields-of-view:

   double final_FOV_u=FOV_excess_fill_factor*(camera_ptr->get_FOV_u()*180/PI);
   double final_FOV_v=FOV_excess_fill_factor*(camera_ptr->get_FOV_v()*180/PI);

   if (image_rotated_by_ninety_degrees_flag)
   {
      templatefunc::swap(final_FOV_u,final_FOV_v);
   }

   if (n_anim_steps==0)
   {
      get_CM_3D_ptr()->jumpto(camera_world_posn,Rcamera.transpose(),
                              final_FOV_u,final_FOV_v);
   }
   else
   {
      bool write_to_file_flag=false;
//   bool write_to_file_flag=true;
      bool no_final_slowdown_flag=false;
      get_CM_3D_ptr()->flyto(
         camera_world_posn,Rcamera.transpose(),
         write_to_file_flag,no_final_slowdown_flag,final_FOV_u,final_FOV_v,
         n_anim_steps,t_flight);
   }

// Compute reasonable values for the final position and orientation of
// the animation path which is automatically calculated when the user
// eventually wants to move the virtual camera away from the
// OBSFRUSTUM's photo:

   threevector flyout_camera_posn;
   rotation Rcamera_flyout;
   compute_camera_flyout_posn_and_orientation(
      ID,flyout_camera_posn,Rcamera_flyout);

   get_CM_3D_ptr()->set_initial_camera_posn(camera_world_posn);
   get_CM_3D_ptr()->set_initial_camera_rotation(Rcamera);
   get_CM_3D_ptr()->set_final_camera_posn(flyout_camera_posn);
   get_CM_3D_ptr()->set_final_camera_rotation(Rcamera_flyout);

// Once Terrain Manipulator's virtual camera has flown to real
// camera's location, set its rotate_about_curr_eyepoint_flag member
// boolean to true:
   
   get_CM_3D_ptr()->set_rotate_about_current_eyepoint_flag(true);
}

// ---------------------------------------------------------------------
// Member function find_Movies_in_OSGsubPAT() loops over all OBSFRUSTA
// within *this.  It pushes back each OBSFRUSTUM's Movie pointer onto
// an STL vector.  This method returns the STL vector of Movie pointers.

vector<Movie*> OBSFRUSTAGROUP::find_Movies_in_OSGsubPAT()
{
//   cout << "inside OBSFRUSTAGROUP::find_Movies_in_OSGsubPAT()" << endl;
//   cout << "MoviesGroup_ptr = " << MoviesGroup_ptr << endl;
//   cout << "MoviesGroup_ptr->get_n_Graphicals() = "
//        << MoviesGroup_ptr->get_n_Graphicals() << endl;

   vector<Movie*> Movie_ptrs;
   if (MoviesGroup_ptr != NULL)
   {
      int selected_OSGsubPAT_ID=get_selected_OSGsubPAT_ID();
//      cout << "selected_OSGsubPAT_ID = "
//           << selected_OSGsubPAT_ID << endl;

// Loop over all OBSFRUSTA within OBSFRUSTAGROUP.  Find those which
// belong to OSGsubPAT labeled by selected_OSGsubPAT_ID.  Append their
// movies onto STL vector Movie_ptrs:

//      cout << "n_OBSFRUSTA = " << get_n_Graphicals() << endl;
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(n);
         if (OSGsubPAT_parent_of_Graphical(OBSFRUSTUM_ptr)==
             selected_OSGsubPAT_ID)
         {
            Movie_ptrs.push_back(
               MoviesGroup_ptr->get_Movie_ptr(OBSFRUSTUM_ptr->get_ID()));
//            cout << "Movie for OBSFRUSTUM ID = " << OBSFRUSTUM_ptr->get_ID()
//                 << " added to Movie_ptrs " << endl;
//            cout << "Movie_ptr = " << Movie_ptrs.back() << endl;
         }
      } // loop over index n labeling individual OBSFRUSTA
   } // MoviesGroup_ptr != NULL conditional

//   cout << "Movie_ptrs.size() = " << Movie_ptrs.size() << endl;
   return Movie_ptrs;
}

// ---------------------------------------------------------------------
// Member function pause_all_videos

void OBSFRUSTAGROUP::pause_all_videos()
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      get_OBSFRUSTUM_ptr(n)->get_AnimationController_ptr()->
         setState(AnimationController::PAUSE);
   }
}

// ---------------------------------------------------------------------
// Member function fly_out_from_OBSFRUSTUM generates an animation path
// which starts at the input OBSFRUSTUM's camera's location and zooms
// outwards and upwards.  This method effectively performs an inverse
// operation to flyto_camera_location().  

void OBSFRUSTAGROUP::fly_out_from_OBSFRUSTUM(
   int ID,bool quasirandom_flyout_flag,double backoff_distance_factor)
{
//   cout << "inside OBSFRUSTAGROUP::fly_out_from_OBSFRUSTUM()" << endl;

   threevector flyout_camera_posn;
   rotation Rcamera_flyout;
   compute_camera_flyout_posn_and_orientation(
      ID,flyout_camera_posn,Rcamera_flyout,
      quasirandom_flyout_flag,backoff_distance_factor);

   bool write_to_file_flag=false;
//   bool write_to_file_flag=true;
   bool no_final_slowdown_flag=true;
   get_CM_3D_ptr()->flyto(
      flyout_camera_posn,Rcamera_flyout.transpose(),write_to_file_flag,
      no_final_slowdown_flag);

// Once Terrain Manipulator's virtual camera has flown away from real
// camera's location, set its rotate_about_curr_eyepoint_flag member
// boolean to false:
   
   get_CM_3D_ptr()->set_rotate_about_current_eyepoint_flag(false);

   unerase_all_OBSFRUSTA();
   load_thumbnail_photo();
   OBSFRUSTA_reset_flag=true;

   set_selected_Graphical_ID(-1);
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

void OBSFRUSTAGROUP::compute_camera_flyout_posn_and_orientation(
   int ID,threevector& flyout_camera_posn,rotation& Rcamera_flyout,
   bool quasirandom_flyout_flag,double backoff_distance_factor)
{
//   cout << "inside OBSFRUSTAGroup::compute_flyout_posn_and_orientation()"
//        << endl;
   
   if (quasirandom_flyout_flag)
   {
      compute_quasirandom_camera_flyout_posn_and_orientation(
         ID,flyout_camera_posn,Rcamera_flyout);
      return;
   }

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(ID);
   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
   camera* camera_ptr=Movie_ptr->get_camera_ptr();

   threevector camera_world_posn=camera_ptr->get_world_posn();
   rotation Rcamera(*(camera_ptr->get_Rcamera_ptr()));  

// Spin camera about line-of-sight if necessary:

   Rcamera=OBSFRUSTUM_ptr->rotation_about_LOS()*Rcamera;

//   cout << "get_portrait_mode_flag() = " 
//        << OBSFRUSTUM_ptr->get_portrait_mode_flag() << endl;
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
   
//    const double backoff_distance_factor=3;
   double backoff_dist=
      backoff_distance_factor*OBSFRUSTUM_ptr->get_movie_downrange_distance();
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

//   cout << "Rcamera_flyout = " << Rcamera_flyout << endl;
}

// ---------------------------------------------------------------------
// Member function compute_quasirandomcamera_flyout_posn_and_orientation()
// intentionally tries to reposition the virtual camera far away after
// it has finished flying away from a reconstructed camera location.
// The virtual camera's final azimuth relative to the Grid center is
// 180 degrees off from the reconstructed camera's.  And its elevation
// angle randomly ranges from 30 to 60 degrees, while its final roll
// angle = 0.  We wrote this specialized method for Family Day 2011
// display purposes.

void OBSFRUSTAGROUP::compute_quasirandom_camera_flyout_posn_and_orientation(
   int ID,threevector& flyout_camera_posn,rotation& Rcamera_flyout)
{
   cout << "inside OBSFRUSTAGroup::compute_quasirandom_flyout_posn_and_orientation()"
        << endl;
   
   OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(ID);
   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   threevector camera_world_posn=camera_ptr->get_world_posn();
   rotation Rcamera(*(camera_ptr->get_Rcamera_ptr()));  

   EarthRegion* EarthRegion_ptr=EarthRegionsGroup_ptr->get_EarthRegion_ptr(0);
   LatLongGrid* LLG_ptr=EarthRegion_ptr->get_LatLongGrid_ptr();
   threevector Grid_center=LLG_ptr->get_world_middle();
//   cout << "Grid_center = " << Grid_center << endl;
   const double flyout_radius=1000;	// meters	sailplane 3
//   const double flyout_radius=12*1000;	// meters	HAFB

   threevector rel_camera_world_posn=
      (camera_world_posn-Grid_center).unitvector();
   double start_az=atan2(
      rel_camera_world_posn.get(1),rel_camera_world_posn.get(0));
   double azimuth=start_az+PI;
   double elevation_lo=15*PI/180;	// sailplane 3
   double elevation_hi=85*PI/180;	// sailplane 3
//   double elevation_lo=30*PI/180;	// HAFB
//   double elevation_hi=60*PI/180;	// HAFB
   double elevation=elevation_lo+(elevation_hi-elevation_lo)*nrfunc::ran1();
   cout << "Elevation = " << elevation*180/PI << endl;

   threevector What(
      cos(azimuth)*cos(elevation),sin(azimuth)*cos(elevation),sin(elevation));
   threevector Uhat(-sin(azimuth),cos(azimuth),0);
   threevector Vhat=What.cross(Uhat);
   
   flyout_camera_posn=Grid_center+flyout_radius*What;

// 0th row of Rcamera = effective Uhat (even if portrait mode == true)
// 1st row of Rcamera = effective Vhat (even if portrait mode == true)
// 2nd row of Rcamera = What

   Rcamera_flyout.put_row(0,Uhat);
   Rcamera_flyout.put_row(1,Vhat);
   Rcamera_flyout.put_row(2,What);

//   cout << "Rcamera_flyout = " << Rcamera_flyout << endl;
}

// ---------------------------------------------------------------------
// Member function conduct_quasirandom_tour() is a specialized method
// which we wrote for Family Day 2011.  It waits 10 seconds before
// flying in to a new, automatically selected OBSFRUSTUM.  Over the
// next 30 seconds, this method alpha blends up and down the selected
// OBSFRUSTUM's image plane.  Finally after the 40th second, this
// method flies the virtual camera away from the selected OBSFRUSTUM
// and resets the clock back to zero.

void OBSFRUSTAGROUP::conduct_quasirandom_tour()
{
//    cout << "inside OBSFRUSTAGROUP::conduct_quasirandom_tour()" << endl;

   double curr_elapsed_time=timefunc::elapsed_timeofday_time();
//      cout << "curr_elapsed_time = " << curr_elapsed_time << endl;
   if (curr_elapsed_time > 10 && before_flyin_flag)
   {
      int selected_ID=get_selected_Graphical_ID();
      selected_ID += 17;
      selected_ID = selected_ID % get_n_Graphicals();
//         int selected_ID=get_n_Graphicals()*nrfunc::ran1();
//         selected_ID=basic_math::max(0,selected_ID);
//         selected_ID=basic_math::min(get_n_Graphicals()-1,selected_ID);
      cout << "Selected_ID = " << selected_ID << endl;
            
      set_selected_Graphical_ID(selected_ID);
      fly_to_entered_OBSFRUSTUM(get_selected_Graphical_ID());
      before_flyin_flag=false;
      before_flyout_flag=true;
   }
   if (before_flyout_flag &&
      ( (curr_elapsed_time > 20 && curr_elapsed_time < 23) ||
        (curr_elapsed_time > 26 && curr_elapsed_time < 29) ||
        (curr_elapsed_time > 32 && curr_elapsed_time < 35) ) )
   {
      vector<Movie*> Movie_ptrs=find_Movies_in_OSGsubPAT();
      for (unsigned int i=0; i<Movie_ptrs.size(); i++)
      {
         Movie* Movie_ptr=Movie_ptrs[i];
         if (Movie_ptr != NULL)
         {
            double alpha=Movie_ptr->get_alpha();
//               cout << "alpha = " << alpha << endl;
            alpha=basic_math::max(0.0,alpha-0.005);
            Movie_ptr->set_alpha(alpha);
         }
      } // loop over Movies within Movie_ptrs
   }
   if (before_flyout_flag &&
      ( (curr_elapsed_time > 23 && curr_elapsed_time < 26) ||
      (curr_elapsed_time > 29 && curr_elapsed_time < 32) ||
      (curr_elapsed_time > 35 && curr_elapsed_time < 38) ) )
   {
      vector<Movie*> Movie_ptrs=find_Movies_in_OSGsubPAT();
      for (unsigned int i=0; i<Movie_ptrs.size(); i++)
      {
         Movie* Movie_ptr=Movie_ptrs[i];
         if (Movie_ptr != NULL)
         {
            double alpha=Movie_ptr->get_alpha();
//               cout << "alpha = " << alpha << endl;
            alpha=basic_math::min(1.0,alpha+0.005);
            Movie_ptr->set_alpha(alpha);
         }
      } // loop over Movies within Movie_ptrs
   }

   if (curr_elapsed_time > 40 && before_flyout_flag)
   {
      int selected_ID=get_selected_Graphical_ID();
      bool quasirandom_flyout_flag=true;
      fly_out_from_OBSFRUSTUM(selected_ID,quasirandom_flyout_flag);
      set_selected_Graphical_ID(selected_ID);
      before_flyout_flag=false;
      before_flyin_flag=true;
      timefunc::initialize_timeofday_clock();
   }
}

// ---------------------------------------------------------------------
// Member function finalize_after_flying_in_and_out() resets
// maneuver_finished_flag to false.  It also stop frustum blinking if
// the virtual camera glides in smoothly to the selected OBSFRUSTUM.
// This method also reloads a lo-res thumbnail of a photo when the
// user flies out from a selected OBSFRUSTUM.

void OBSFRUSTAGROUP::finalize_after_flying_in_and_out()
{   
//   cout << "inside OBSFRUSTAGROUP::finalize_after_flying_in_and_out()"
//        << endl;
   
   if (get_CM_3D_ptr()->get_flying_maneuver_finished_flag())
   {
      get_CM_3D_ptr()->set_flying_maneuver_finished_flag(false);
//      compute_other_visible_imageplanes();

// If virtual camera smoothly glides to selected OBSFRUSTUM, stop
// selected frustum from blinking by reseting its blinking and current
// color to its permanent color:

      if (!jump_to_apex_flag)
      {
         OBSFRUSTUM* selected_OBSFRUSTUM_ptr=
            dynamic_cast<OBSFRUSTUM*>(get_selected_Graphical_ptr());
         selected_OBSFRUSTUM_ptr->set_blinking_color(
            selected_OBSFRUSTUM_ptr->get_permanent_color());
         selected_OBSFRUSTUM_ptr->set_curr_color(
            selected_OBSFRUSTUM_ptr->get_permanent_color());
         jump_to_apex_flag=true;
      }
   }

   int flyout_zoom_counter=get_CM_3D_ptr()->get_flyout_zoom_counter();
//   cout << "flyout_zoom_counter = " << flyout_zoom_counter << endl;
   if (flyout_zoom_counter > 0 && flyout_zoom_counter < 35)
   {
      OBSFRUSTA_reset_flag=false;
   }
      
   if (flyout_zoom_counter >=35 && flyout_zoom_counter < 75 &&
       !OBSFRUSTA_reset_flag)
   {

// Recall that when we work with Noah Snavely's BUNDLER output, we
// erase all OBSFRUSTA except the selected one in order to clearly see
// individual photos.  When the user zooms away from the selected
// OBSFRUSTUM, restore all the previously erased OBSFRUSTA:

// Note added on Oct 9, 2009 at 1:10 pm...need to sometimes NOT run
// following unerease_all_OBSFRUSTA() when we want to see where
// individual OBSFRUSTA lie within NYC map...

      unerase_all_OBSFRUSTA();

      load_thumbnail_photo();
      OBSFRUSTA_reset_flag=true;
   }
}

// --------------------------------------------------------------------------
// Member function cross_fade_photo_pair() performs a time-dependent
// fade-out of the previously selected OBSFRUSTUM's movie and a
// time-dependent fade-in of the currently selected OBSFRUSTUM's
// movie.  We wrote this method in Feb 2010 in order to mimic Noah
// Snavely's transitions from one photo to the next.

void OBSFRUSTAGROUP::cross_fade_photo_pair()
{
//   cout << "inside OBSFRUSTAGROUP::cross_fade_photo_pair()" << endl;

/*
   const double begin_fall_time=0;	// sec
   const double end_fall_time=0.75;	// sec
   double begin_rise_time=0.75;		// sec
   double end_rise_time=1.5;       	// sec
*/


   const double begin_fall_time=0;	// sec
   const double end_fall_time=1;	// sec
   double begin_rise_time=1;		// sec
   double end_rise_time=2;       	// sec
   
   bool min_alpha_reached_flag=false;
   bool max_alpha_reached_flag=false;
   Movie* prev_Movie_ptr=NULL;
   Movie* curr_Movie_ptr=NULL;

   OBSFRUSTUM* prev_OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(
      get_prev_selected_Graphical_ID());
   if (prev_OBSFRUSTUM_ptr != NULL)
   {
//      cout << "prev_OBSFRUSTUM_ptr->get_ID() = "
//           << prev_OBSFRUSTUM_ptr->get_ID() << endl;
      prev_Movie_ptr=prev_OBSFRUSTUM_ptr->get_Movie_ptr();
      min_alpha_reached_flag=prev_Movie_ptr->
         time_dependent_fade_out(begin_fall_time,end_fall_time);
   }
   else
   {
      min_alpha_reached_flag=true;
      begin_rise_time=0;
      end_rise_time=1;
   }
   
   OBSFRUSTUM* curr_OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(
      get_selected_Graphical_ID());
   if (curr_OBSFRUSTUM_ptr != NULL)
   {
//      cout << "curr_OBSFRUSTUM_ptr->get_ID() = "
//           << curr_OBSFRUSTUM_ptr->get_ID() << endl;
      curr_Movie_ptr=curr_OBSFRUSTUM_ptr->get_Movie_ptr();
      max_alpha_reached_flag=curr_Movie_ptr->
         time_dependent_fade_in(begin_rise_time,end_rise_time);
   }
   else
   {
      max_alpha_reached_flag=true;
   }

   if (min_alpha_reached_flag && max_alpha_reached_flag)
   {
      if (prev_Movie_ptr != NULL) 
         prev_Movie_ptr->set_fadeout_start_time(-1.0);
      if (curr_Movie_ptr != NULL)
         curr_Movie_ptr->set_fadein_start_time(-1.0);
      set_cross_fading_flag(false);
   }
}

// --------------------------------------------------------------------------
// Member function fade_away_photo() fades down the photo
// corresponding to the OBSFRUSTUM specified by the input ID.  We
// wrote this method in Aug 2009 in order to simulate zooming movement
// from one "wagon-wheel" photo to another for the RASR viewer.

void OBSFRUSTAGROUP::fade_away_photo(int OBSFRUSTUM_ID)
{
//   cout << "inside OBSFRUSTAGROUP::fade_away_photo()" << endl;
//   cout << "OBSFRUSTUM_ID = " << OBSFRUSTUM_ID << endl;
//   cout << "get_selected_Graphical_ID() = "
//        << get_selected_Graphical_ID() 
//        << " get_prev_selected_Graphical_ID() = "
//        << get_prev_selected_Graphical_ID() << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);   
   if (OBSFRUSTUM_ptr==NULL) return;
   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
   if (Movie_ptr != NULL)
   {
      const double total_fadeout_time=3.0; // secs
      Movie_ptr->time_dependent_fade_out(total_fadeout_time);
//      double delta_alpha=0.005;
//      Movie_ptr->decrease_alpha(delta_alpha);
   }
}

// --------------------------------------------------------------------------
// Member function fade_in_photo() fades in the photo corresponding to
// the OBSFRUSTUM specified by the input ID over a fixed time
// interval.  We wrote this method in Feb 2010 in order to implement
// cross fading between the previous and current selected photos
// within VIEWBUNDLER.

void OBSFRUSTAGROUP::fade_in_photo(int OBSFRUSTUM_ID)
{
//   cout << "inside OBSFRUSTAGROUP::fade_in_photo()" << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);   
   if (OBSFRUSTUM_ptr==NULL) return;
   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
   if (Movie_ptr != NULL)
   {
      const double total_fadein_time=3; // secs
      if (Movie_ptr->time_dependent_fade_in(total_fadein_time))
      {
         set_cross_fading_flag(false);
      }
   }
}

// ---------------------------------------------------------------------
// Member function reset_OBSFRUSTUM_from_package_file() searches for a
// package whose filename corresponds to the input OBSFRUSTUM's movie.
// If a package file is found, this method extracts its 3x4 projection
// matrix P.  P is subsequently used to build the current frustum, and
// this boolean method returns true.  If *OBSFRUSTUM_ptr is not
// updated, this method returns false.

bool OBSFRUSTAGROUP::reset_OBSFRUSTUM_from_package_file(
   OBSFRUSTUM* OBSFRUSTUM_ptr)
{
//   cout << "inside OBSFRUSTAGROUP::reset_OBSFRUSTUM_from_package_file()"
//        << endl;

   if (int(get_curr_framenumber()) == prev_OBSFRUSTUM_framenumber) 
      return false;
//   cout << "get_curr_framenumber() = " << get_curr_framenumber() << endl;

   double frustum_sidelength;
   genmatrix P(3,4);
   if (!import_package_params(get_curr_framenumber(),frustum_sidelength,P)) 
      return false;
    
// When a movie loops to its beginning, we do not want to temporally
// pollute position and velocities from movie's end with those at its
// beginning.  So turn off temporal filtering for first few movie
// frames:

   bool temporally_filter_flag=true;
//      bool temporally_filter_flag=false;
   if (get_curr_framenumber() < 3)
   {
      temporally_filter_flag=false;
   }

   OBSFRUSTUM_ptr->build_current_frustum(
      get_curr_t(),get_passnumber(),&P,frustum_sidelength,
      filter_alpha_value,temporally_filter_flag);

// Recall that input projection matrices generally have a zero-valued
// translation vector for the video camera.  So recover camera's
// actual world position from member static_camera_posn_offset:

//      threevector relative_camera_trans(80,85,50);
//      threevector camera_posn=get_grid_world_origin()+relative_camera_trans;

// As of 2/16/09, we assume that calibrated video frame projection
// matrices have zero entries for camera world position.  We further
// assume that the camera's actual world position is read in from one
// package file and stored within member threevector
// static_camera_world_posn of OBSFRUSTAGROUP:

   threevector camera_posn=static_camera_world_posn;

   OBSFRUSTUM_ptr->absolute_position(
      get_curr_t(),get_passnumber(),camera_posn);


   prev_OBSFRUSTUM_framenumber=get_curr_framenumber();

/*
// FAKE FAKE:  Weds January 28, 2009 at 7:42 pm.
// Experiment with locking virtual camera to current OBSFRUSTUM:

      ViewFrustum* ViewFrustum_ptr=get_CM_3D_ptr()->get_ViewFrustum_ptr();
      camera* camera_ptr=OBSFRUSTUM_ptr->get_Movie_ptr()->get_camera_ptr();

      ViewFrustum_ptr->reset_viewmatrix(
         x_hat,y_hat,
         camera_ptr->get_world_posn());

//      ViewFrustum_ptr->reset_viewmatrix(
//         camera_ptr->get_Uhat(),camera_ptr->get_Vhat(),
//         camera_ptr->get_world_posn());
*/
      
   return true;
}

// ---------------------------------------------------------------------
// Member function display_OBSFRUSTA_as_time_sequence() enables a
// group of reconstructed 3D OBSFRUSTA to be displayed over time as a
// movie sequence.  It sets the AnimationController's number of
// frames equal to the number of OBSFRUSTA within *this.  And it
// resets member bool flag play_OBSFRUSTA_as_movie_flag to true.

void OBSFRUSTAGROUP::display_OBSFRUSTA_as_time_sequence()
{
//   cout << "inside OBSFRUSTAGROUP::display_OBSFRUSTA_as_time_sequence()" 
//        << endl;
//   cout << "n_frames = " << get_n_Graphicals() << endl;

   AnimationController_ptr->set_nframes(get_n_Graphicals());
//   AnimationController_ptr->setDelay(0.0);
   AnimationController_ptr->setDelay(0.25);
//   AnimationController_ptr->setDelay(0.5);
   play_OBSFRUSTA_as_movie_flag=true;
}

// ---------------------------------------------------------------------
// Member function find_OBSFRUSTUM_closest_to_screen_center() loops
// over all OBSFRUSTA and computes the 2D screen projection of their 3D
// camera world positions.  It ignores any OBSFRUSTUM which lies too
// far away from the middle of the screen.  This method returns the ID
// of the OBSFRUSTUM whose camera location lies closest to the center
// of the screen.

int OBSFRUSTAGROUP::find_OBSFRUSTUM_closest_to_screen_center()
{
//   cout << "inside OBSFRUSTAGROUP::find_OBSFRUSTUM_closest_to_screen_center()"
//        << endl;

   int OBSFRUSTUM_ID=-1;
   if (get_CM_3D_ptr()->get_rotate_about_current_eyepoint_flag())
   {
//      cout << "rot about curr eyept flag = true!" << endl;
      return OBSFRUSTUM_ID;
   }

   double min_sqrd_distance_to_center=POSITIVEINFINITY;
   for (unsigned int i=0; i<get_n_Graphicals(); i++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(i);
      Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
      if (Movie_ptr==NULL) continue;

      camera* camera_ptr=Movie_ptr->get_camera_ptr();
      threevector camera_world_posn=camera_ptr->get_world_posn();
      threevector camera_screen_posn=
         get_CM_3D_ptr()->get_Transformer_ptr()->
         world_to_screen_transformation(camera_world_posn);

      double Xren=camera_screen_posn.get(0);
      double Yren=camera_screen_posn.get(1);

      if (fabs(Xren) > 0.5 || fabs(Yren) > 0.5) continue;

      double curr_sqrd_distance_to_center=sqr(Xren-0.0)+sqr(Yren-0.0);
      if (curr_sqrd_distance_to_center < min_sqrd_distance_to_center)
      {
         min_sqrd_distance_to_center=curr_sqrd_distance_to_center;
         OBSFRUSTUM_ID=OBSFRUSTUM_ptr->get_ID();
      }
   }
//   cout << "OBSFRUSTUM_ID = " << OBSFRUSTUM_ID << endl;

   return OBSFRUSTUM_ID;
}

// ---------------------------------------------------------------------
// Member function alpha_vary_selected_image() performs 3 rounds of
// alpha fading followed by alpha brightening for a selected
// OBSFRUSTUM's image plane.  The entire cycle of alpha varying takes
// approximately 25 seconds to complete.  We wrote this specialized
// utility method for Family Day 2011 presentation purposes.

void OBSFRUSTAGROUP::alpha_vary_selected_image()
{
//   cout << "inside OBSFRUSTAGROUP::alpha_vary_selected_image()" << endl;
//   cout << "Selected OBSFRUSTUM ID = " << get_selected_Graphical_ID() << endl;

   if (get_selected_Graphical_ID() < 0) return;

   double curr_elapsed_time=timefunc::elapsed_timeofday_time();
//   cout << "curr_elapsed_time = " << curr_elapsed_time << endl;

   if (curr_elapsed_time > 5 && before_flyin_flag)
   {
      before_flyin_flag=false;
      before_flyout_flag=true;
   }
   if (before_flyout_flag &&
      ( (curr_elapsed_time > 7 && curr_elapsed_time < 10) ||
        (curr_elapsed_time > 13 && curr_elapsed_time < 16) ||
        (curr_elapsed_time > 19 && curr_elapsed_time < 22) ) )
   {
      vector<Movie*> Movie_ptrs=find_Movies_in_OSGsubPAT();
      for (unsigned int i=0; i<Movie_ptrs.size(); i++)
      {
         Movie* Movie_ptr=Movie_ptrs[i];
         if (Movie_ptr != NULL)
         {
            double alpha=Movie_ptr->get_alpha();
//            cout << "alpha = " << alpha << endl;
            alpha=basic_math::max(0.0,alpha-0.08);
            Movie_ptr->set_alpha(alpha);
         }
      } // loop over Movies within Movie_ptrs
   }
   if (before_flyout_flag &&
      ( (curr_elapsed_time > 10 && curr_elapsed_time < 13) ||
        (curr_elapsed_time > 16 && curr_elapsed_time < 19) ||
        (curr_elapsed_time > 22 && curr_elapsed_time < 25) ) )
   {
      vector<Movie*> Movie_ptrs=find_Movies_in_OSGsubPAT();
      for (unsigned int i=0; i<Movie_ptrs.size(); i++)
      {
         Movie* Movie_ptr=Movie_ptrs[i];
         if (Movie_ptr != NULL)
         {
            double alpha=Movie_ptr->get_alpha();
//            cout << "alpha = " << alpha << endl;
            alpha=basic_math::min(1.0,alpha+0.08);
            Movie_ptr->set_alpha(alpha);
         }
      } // loop over Movies within Movie_ptrs
   }

   if (curr_elapsed_time > 25 && before_flyout_flag)
   {
      before_flyout_flag=false;
      before_flyin_flag=true;
      get_CM_3D_ptr()->set_flying_maneuver_finished_flag(true);
   }
}

// ---------------------------------------------------------------------
// Member function
// select_and_alpha_vary_OBSFRUSTUM_closest_to_screen_center() is a
// high-level method which first searches for an OBSFRUSTUM whose
// camera position lies within some reasonable distance of the screen
// space center.  It next checks if the virtual camera's distance from
// the OBSFRUSTUM's camera is less than some maximal range.  If so,
// this method flies the virtual camera into the selected OBSFRUSTUM
// and periodically alpha fades/brightens the OBSFRUSTUM's image
// plane.  We wrote this specialized method for Family Day 2011.
// 

bool OBSFRUSTAGROUP::select_and_alpha_vary_OBSFRUSTUM_closest_to_screen_center()
{
//   cout << "inside OBSFRUSTAGROUP::select_and_alpha_vary_OBSFRUSTUM_closest_to_screen_center()" << endl;
   int selected_ID=find_OBSFRUSTUM_closest_to_screen_center();
//   cout << "Selected_ID = " << selected_ID << endl;
            
   if (selected_ID < 0) return false;

// Do not select OBSFRUSTUM unless it lies within some minimal range
// of the virtual camera's world position:

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(selected_ID);
   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   threevector camera_world_posn=camera_ptr->get_world_posn();

   threevector eye_world_posn=get_CM_3D_ptr()->get_eye_world_posn();
   double curr_range=(eye_world_posn-camera_world_posn).magnitude();
//   cout << "eye_world_posn = " << eye_world_posn << endl;
//   cout << "camera_world_posn = " << camera_world_posn << endl;
//   cout << "range = " << curr_range << endl;

//   const double max_range_for_selection=2000;	// meters
   const double max_range_for_selection=4000;	// meters
   if (curr_range > max_range_for_selection) 
   {
      cout << "curr_range = " << curr_range << " exceeds max range limit = "
           << max_range_for_selection << endl;
      return false;
   }
   
   set_selected_Graphical_ID(selected_ID);
   fly_to_entered_OBSFRUSTUM(get_selected_Graphical_ID());
   timefunc::initialize_timeofday_clock();
   alpha_vary_selected_image();
   return true;
}

// ---------------------------------------------------------------------
// Member function deselect_OBSFRUSTUM()

bool OBSFRUSTAGROUP::deselect_OBSFRUSTUM()
{
   cout << "inside OBSFRUSTAGROUP::deselect_OBSFRUSTUM()" << endl;

   int selected_ID=get_selected_Graphical_ID();
   if (selected_ID < 0) return false;
   
/*
   if (!get_CM_3D_ptr()->get_flying_maneuver_finished_flag()) 
   {
      cout << "Fly out not finished yet!" << endl;
      return false;
   }
*/

   bool quasirandom_flyout_flag=false; // NYC 1K demo
   double backoff_distance_factor=30;  // NYC 1K demo
//   bool quasirandom_flyout_flag=true; // sailplane demo

   fly_out_from_OBSFRUSTUM(
      selected_ID,quasirandom_flyout_flag,backoff_distance_factor);
   return true;
}

// ==========================================================================
// HAFB video3D member functions
// ==========================================================================

// Member function generate_HAFB_movie_OBSFRUSTUM is a specialized
// method which initializes time-dependent ViewingPyramid,
// ViewingPyramidAboveZplane and Movie graphicals based upon HAFB
// video pass data read from input files.  It first scales, rotates
// and translates canonical square pyramids so that they form
// untruncated viewing pyramids which emanate downwards from the
// moving aircraft.  It next computes the instantaneous viewing
// pyramid above the z-plane and saves its information within a second
// ViewingPyramidAboveZplane graphical.  (This last graphical would
// need to be generalized for more complicated cases where the above
// z-plane frustum is not also a simple extension of a conventional
// pyramid.)  The time-dependent vertices of the zplane face of this
// last pyramid are saved within the graphical for instantaneous movie
// texturing purposes within update_display().  Finally, both pyramid
// graphicals' colors and edge widths are adjusted by this method.

// On 7/20/07, we empirically confirmed that (to good approximation)
// the four direction vectors curr_UV_corner_dir[0-3] define a plane.
// u_hat = curr_UV_corner_dir[1]-curr_UV_corner_dir[0]
// v_hat = curr_UV_corner_dir[2]-curr_UV_corner_dir[1]
// Angle between u_hat and v_hat = 90.16 degs.

OBSFRUSTUM* OBSFRUSTAGROUP::generate_HAFB_movie_OBSFRUSTUM(
   const vector<threevector>& aircraft_posn,double z_offset)
{
//   cout << "inside OBSFRUSTAGROUP::generate_HAFB_movie_OBSFRUSTUM()" << endl;

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
// to new movie object into *OBSFRUSTUM_ptr. Then transform movie so
// that it lies inside *OBSFRUSTUM_ptr:
   
   string movie_filename=subdir+"HAFB_overlap_corrected_grey.vid";

   OBSFRUSTUM* OBSFRUSTUM_ptr=generate_movie_OBSFRUSTUM(movie_filename);
   OBSFRUSTUM_ptr->set_rectangular_movie_flag(false);

   OBSFRUSTUM_ptr->instantiate_OSG_Pyramids();
   OBSFRUSTUM_ptr->generate_Pyramid_geodes();

   for (int i=0; i<int(V1.size())/4; i++)
   {
      double curr_t=double(i);

      vector<threevector> curr_UV_corner_dir;
      for (int c=0; c<4; c++)
      {
         curr_UV_corner_dir.push_back(UV_corner_dir[i*4+c]);
      }
      double pyramid_altitude=5000;
      double Uscale,Vscale;
      threevector U_hat,V_hat;
      OBSFRUSTUM_ptr->get_viewing_pyramid_ptr()->
         generate_scaled_rotated_translated_square_pyramid(
            aircraft_posn[i],curr_UV_corner_dir,pyramid_altitude,
            Uscale,Vscale,U_hat,V_hat);
      OBSFRUSTUM_ptr->get_viewing_pyramid_ptr()->
         ensure_faces_handedness(face::right_handed);

// Compute instantaneous above_Zplane pyramid:

      OBSFRUSTUM_ptr->compute_viewing_pyramid_above_Zplane(
         z_offset,OBSFRUSTUM_ptr->get_viewing_pyramid_ptr());
//      cout << "*OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr() = "
//           << *(OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr())
//           << endl;

      OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->
         build_current_pyramid(
            curr_t,get_passnumber(),
            OBSFRUSTUM_ptr->get_viewing_pyramid_ptr());

// Mask ViewingPyramid at each time step if it is not to be displayed
// in final output:

//      bool display_ViewingPyramid_flag=true;
      bool display_ViewingPyramid_flag=false;
      OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->set_mask(
         curr_t,get_passnumber(),!display_ViewingPyramid_flag);

      OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->
         build_current_pyramid(
            curr_t,get_passnumber(),
            OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr());

// In order to instantaneously position movie within Z-plane, save
// vertex chain from zplane_face within *Pyramid_above_zplane_ptr
// graphical's time-dependent vertices:

      OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->
         store_apex_and_zplane_vertices(
            curr_t,get_passnumber(),
            OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr());

// Color ViewingPyramid and ViewPyramidAboveZplane graphicals.  Then
// set their edge widths:

      OBSFRUSTUM_ptr->get_ViewingPyramid_ptr()->set_color(
         OBSFRUSTUM_ptr->get_viewing_pyramid_ptr(),
         colorfunc::get_OSG_color(colorfunc::green),
         colorfunc::get_OSG_color(colorfunc::cyan),
         colorfunc::get_OSG_color(colorfunc::blue),
         osg::Vec4( 0 , 1 , 1 , 0.7 ));

      OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->set_color(
         OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr(),
//         colorfunc::get_OSG_color(colorfunc::yellow),
         colorfunc::get_OSG_color(colorfunc::white),
         colorfunc::get_OSG_color(colorfunc::red),
         colorfunc::get_OSG_color(colorfunc::orange),
         osg::Vec4( 0.5 , 0.5 , 0.5 , 0.0 ));

//      OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr()->set_color(
//         OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr(),
//         colorfunc::get_OSG_color(colorfunc::yellow),
//         colorfunc::get_OSG_color(colorfunc::red),
//         colorfunc::get_OSG_color(colorfunc::orange),
//         osg::Vec4( 0.5 , 0.5 , 0.5 , 0.0 ));

      OBSFRUSTUM_ptr->set_typical_pyramid_edge_widths();

   } // loop over index i labeling HAFB movie frames

   return OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
// Member function read_HAFB_frusta_info parses the ascii text file
// holding the HAFB pass which program VIDEO3D displays.  This boolean
// member function returns false if it cannot successfully parse the
// input ascii file.

void OBSFRUSTAGROUP::read_HAFB_frusta_info(
   string segments_filename,vector<double>& curr_time,
   vector<int>& segment_ID,vector<int>& pass_number,
   vector<threevector>& V1,vector<threevector>& V2,
   vector<colorfunc::Color>& color)
{
   if (!filefunc::ReadInfile(segments_filename))
   {
      cout << "Trouble in OBSFRUSTAGROUP::read_info_from_file()"
           << endl;
      cout << "Couldn't open segments_filename = " << segments_filename
           << endl;
   }

   int nlines=filefunc::text_line.size();
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

void OBSFRUSTAGROUP::reconstruct_HAFB_corner_dirs(
   const vector<threevector>& aircraft_posn,
   const vector<threevector>& corner_posns,
   vector<threevector>& UV_corner_dir)
{
   int corner_counter=0;

   for (unsigned int i=0; i<aircraft_posn.size(); i++)
   {
      threevector curr_aircraft_posn=aircraft_posn[i];
      for (int c=0; c<4; c++)
      {
         threevector curr_corner_posn=corner_posns[corner_counter++];
         UV_corner_dir.push_back(
            (curr_corner_posn-curr_aircraft_posn).unitvector());
      }
   } // loop over index i labeling frame number
}

// ==========================================================================
// 2D photo member functions
// ==========================================================================

// Member function compute_current_zplane_UV_bboxes loops over every
// OBSFRUSTUM whose movie windows we assume lie within the Z-plane.
// This method computes the current UV bounding box for each
// OBSFRUSTUM Z-plane face.  The results are stored within member STL
// vector photo_zplane_bboxes.

void OBSFRUSTAGROUP::compute_current_zplane_UV_bboxes()
{
//   cout << "inside OFG::compute_current_zplane_UV_bboxes()" << endl;

   photo_zplane_bboxes.clear();
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(n);

      pyramid* viewing_pyramid_above_zplane_ptr=
         OBSFRUSTUM_ptr->get_viewing_pyramid_above_zplane_ptr();
      if (viewing_pyramid_above_zplane_ptr==NULL) continue;

      face* zplane_face_ptr=
         viewing_pyramid_above_zplane_ptr->get_zplane_face_ptr();
      if (zplane_face_ptr==NULL) continue;

// Compute UV bbox for current photo:

      double min_U=POSITIVEINFINITY;
      double max_U=NEGATIVEINFINITY;
      double min_V=POSITIVEINFINITY;
      double max_V=NEGATIVEINFINITY;
      for (unsigned int v=0; v<zplane_face_ptr->get_n_vertices(); v++)
      {
         vertex curr_vertex=zplane_face_ptr->get_vertex_from_chain(v);
//         cout << "v = " << v << " vertex_chain[v] = " << curr_vertex
//              << endl;
         threevector curr_vertex_screen_posn=
            get_CM_3D_ptr()->get_Transformer_ptr()->
            world_to_screen_transformation(curr_vertex.get_posn());
         min_U=basic_math::min(min_U,curr_vertex_screen_posn.get(0));
         max_U=basic_math::max(max_U,curr_vertex_screen_posn.get(0));
         min_V=basic_math::min(min_V,curr_vertex_screen_posn.get(1));
         max_V=basic_math::max(max_V,curr_vertex_screen_posn.get(1));
      }
//      cout << "min_U = " << min_U << " max_U = " << max_U << endl;
//      cout << "min_V = " << min_V << " max_V = " << max_V << endl;
      bounding_box bbox(min_U,max_U,min_V,max_V);
//      cout << "n = " << n << " bbox = " << bbox << endl;
      photo_zplane_bboxes.push_back(bbox);
   } // loop over index n labeling OBSFRUSTA
}

// ---------------------------------------------------------------------
// Member function reset_zface_color sets the color of the z-plane
// border around the currently selected photo to red.  All other
// photos' borders are colored blue.

void OBSFRUSTAGROUP::reset_zface_color()
{
//   cout << "inside OBSFRUSTAGROUP::reset_zface_color()" << endl;
   
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(n);
      Pyramid* ViewingPyramidAboveZplane_ptr=
         OBSFRUSTUM_ptr->get_ViewingPyramidAboveZplane_ptr();

      osg::Vec4 side_edge_color(0,0,0,0);
      osg::Vec4 zplane_edge_color=colorfunc::get_OSG_color(colorfunc::blue);
      osg::Vec4 base_edge_color(0,0,0,0);
      osg::Vec4 volume_color(0,0,0,0);
      if (get_selected_Graphical_ID()==OBSFRUSTUM_ptr->get_ID())
      {
         zplane_edge_color=colorfunc::get_OSG_color(colorfunc::red);
      }

      ViewingPyramidAboveZplane_ptr->set_color(
         ViewingPyramidAboveZplane_ptr->get_pyramid_ptr(),
         side_edge_color,zplane_edge_color,base_edge_color,volume_color);

      bool side_edge_mask=true;
      bool zplane_edge_mask=false;
      bool base_edge_mask=true;
      ViewingPyramidAboveZplane_ptr->set_edge_masks(
         get_curr_t(),get_passnumber(),
         ViewingPyramidAboveZplane_ptr->get_pyramid_ptr(),
         side_edge_mask,zplane_edge_mask,base_edge_mask);

   } // loop over index n labeling OBSFRUSTA
}

// ---------------------------------------------------------------------
// Member function compute_tabular_rows_and_columns takes in some
// integer number of photos.  It returns the integer pair
// (nrows,ncolumns) for a table which can reasonably display the
// photos.

void OBSFRUSTAGROUP::compute_tabular_rows_and_columns(
   int n_photos,int& n_rows,int& n_columns)
{
   int trunc_sqrroot=basic_math::mytruncate(sqrt(n_photos));
//   cout << "inside OBSFRUSTAGROUP::compute_tabular_rows_&_cols()" << endl;
//   cout << "n_photos = " << n_photos << endl;
//   cout << "trunc_sqrroot = " << trunc_sqrroot << endl;

   if (nearly_equal(trunc_sqrroot,sqrt(n_photos)))
   {
      n_rows=n_columns=trunc_sqrroot;
   }
   else
   {
      n_columns=trunc_sqrroot+1;
      n_rows=n_columns-1;
      if (n_photos > n_rows*n_columns) n_rows++;
   }
   
//   cout << "n_rows = " << n_rows << endl;
//   cout << "n_columns = " << n_columns << endl;
}

// ---------------------------------------------------------------------
// Member function convert_screenspace_to_photo_coords takes in the
// screen coords -1 <= U,V <= 1 for some feature.  It first updates
// the locations of all OBSFRUSTA z-plane faces.  It then determines
// whether the feature lies inside any of the photo zplane bboxes.  If
// so, it computes and returns the OBSFRUSTUM's ID and the coordinates
// of the feature relative to the photo's UV axes.

void OBSFRUSTAGROUP::convert_screenspace_to_photo_coords(
   const threevector& UVW,int& OBSFRUSTUM_ID,twovector& video_UV)
{
//   cout << "inside OBSFRUSTAGROUP::convert_screenspace_to_photo_coords(), UVW = "
//        << UVW << endl;

   compute_current_zplane_UV_bboxes();

   OBSFRUSTUM_ID=-1;
   video_UV=twovector(-1,-1);
   for (unsigned int n=0; n<photo_zplane_bboxes.size(); n++)
   {
      if (photo_zplane_bboxes[n].point_inside(UVW.get(0),UVW.get(1)))
      {
         OBSFRUSTUM_ID=get_OBSFRUSTUM_ptr(n)->get_ID();
//         cout << "Picked point lies inside photo n = " << n << endl;
         Movie* Movie_ptr=get_OBSFRUSTUM_ptr(n)->get_Movie_ptr();
//         cout << "minU = " << Movie_ptr->get_minU() << endl;
//         cout << "maxU = " << Movie_ptr->get_maxU() << endl;
//         cout << "minV = " << Movie_ptr->get_minV() << endl;
//         cout << "maxV = " << Movie_ptr->get_maxV() << endl;
         photo_zplane_bboxes[n].set_UV_bounds(
            Movie_ptr->get_minU(),Movie_ptr->get_maxU(),
            Movie_ptr->get_minV(),Movie_ptr->get_maxV());
         video_UV=photo_zplane_bboxes[n].XY_to_UV_coords(
            UVW.get(0),UVW.get(1));
      }
   } // loop over index n labeling OBSFRUSTA

//   cout << "video_UV = " << video_UV << endl;
}

// ---------------------------------------------------------------------
// Member function convert_photo_to_screenspace_coords performs the
// inverse operation of convert_screenspace_to_photo_coords().

void OBSFRUSTAGROUP::convert_photo_to_screenspace_coords(
   int OBSFRUSTUM_ID,const twovector& video_UV,threevector& UVW)
{
//   cout << "inside OBSFRUSTAGROUP::convert_photo_to_screenspace_coords()"
//        << endl;

   compute_current_zplane_UV_bboxes();

   int ID=OBSFRUSTUM_ID;
   Movie* Movie_ptr=get_ID_labeled_OBSFRUSTUM_ptr(ID)->get_Movie_ptr();
   photo_zplane_bboxes[ID].set_UV_bounds(
      Movie_ptr->get_minU(),Movie_ptr->get_maxU(),
      Movie_ptr->get_minV(),Movie_ptr->get_maxV());
   
   UVW=threevector(photo_zplane_bboxes[ID].UV_to_XY_coords(
                      video_UV.get(0),video_UV.get(1)), 1 );
//   cout << "UVW = " << UVW << endl;
}

// ---------------------------------------------------------------------
void OBSFRUSTAGROUP::zoom_virtual_camera(double angular_scale_factor)
{
   if (WindowManager_ptr != NULL)
      WindowManager_ptr->rescale_viewer_FOV(angular_scale_factor);
}

// ---------------------------------------------------------------------
// Member function load_hires_photo() retrieves the Movie
// corresponding to the selected OBSFRUSTUM.  This method fills the
// Movie's texture rectangle with the photo corresponding to the
// Movie's full resolution file name.  We wrote this method in April
// 2009 in order to dynamically load [unload] full resolution photos
// whenever a user zooms into [out from] a selected OBSFRUSTUM.

void OBSFRUSTAGROUP::load_hires_photo()
{
//   cout << "inside OBSFRUSTAGROUP::load_hires_photo() #1" << endl;
//   cout << "selected_ID = " << get_selected_Graphical_ID() << endl;
   load_hires_photo(get_selected_Graphical_ID());
}

void OBSFRUSTAGROUP::load_hires_photo(int OBSFRUSTUM_ID)
{
//   cout << "inside OBSFRUSTAGROUP::load_hires_photo() #2" << endl;
//   cout << "OBSFRUSTUM_ID = " << OBSFRUSTUM_ID << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);
   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
//   cout << "Movie_ptr = " << Movie_ptr << endl;
   
   string photo_filename=Movie_ptr->get_video_filename();
//   cout << "photo_filename = " << photo_filename << endl;

   bool twoD_flag=false;
   Movie_ptr->reset_displayed_photograph(photo_filename,twoD_flag);
}

// ---------------------------------------------------------------------
// Member function load_thumbnail_photo() retrieves the Movie
// corresponding to the input OBSFRUSTUM_ID.  It resets the displayed
// photograph to the Movie's thumbnail version.

void OBSFRUSTAGROUP::load_thumbnail_photo()
{
//   cout << "inside OBSFRUSTAGROUP::load_thumbnail_photo()" << endl;
//   cout << "selected_ID = " << get_selected_Graphical_ID() << endl;
   load_thumbnail_photo(get_selected_Graphical_ID());
}

void OBSFRUSTAGROUP::load_thumbnail_photo(int OBSFRUSTUM_ID)
{
//   cout << "inside OBSFRUSTAGROUP::load_thumbnail_photo()" << endl;
//   cout << "OBSFRUSTUM_ID = " << OBSFRUSTUM_ID << endl;
//   cout << "get_ndims() = " << get_ndims() << endl;

   OBSFRUSTUM* thumbnail_OBSFRUSTUM_ptr=
      get_ID_labeled_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);
   if (thumbnail_OBSFRUSTUM_ptr==NULL) return;
//   cout << "thumbnail_OBSFRUSTUM_ptr = " << thumbnail_OBSFRUSTUM_ptr
//        << endl;
   Movie* Movie_ptr=thumbnail_OBSFRUSTUM_ptr->get_Movie_ptr();
//   cout << "Movie_ptr = " << Movie_ptr << endl;

   string photo_filename=Movie_ptr->get_video_filename();
   string thumbnail_filename=videofunc::get_thumbnail_filename(photo_filename);
//   cout << "photo_filename = " << photo_filename << endl;
//   cout << "thumbnail_filename = " << thumbnail_filename << endl;

   bool twoD_flag=false;
   Movie_ptr->reset_displayed_photograph(thumbnail_filename,twoD_flag);

// On 7/18/10, we realized that we need to reset the video filename to
// the full resolution photo filename so that program VIEWBUNDLER will 
// reload the full photo if an OBSFRUSTUM is viewed more than once...

   Movie_ptr->set_video_filename(photo_filename);
}

/*
// ---------------------------------------------------------------------
void OBSFRUSTAGROUP::load_new_photo(int OBSFRUSTUM_ID,string photo_filename)
{
   cout << "inside OBSFRUSTAGROUP::load_new_photo()" << endl;
   cout << "OBSFRUSTUM_ID = " << OBSFRUSTUM_ID << endl;
   cout << "photo_filename = " << photo_filename << endl;
//   cout << "get_ndims() = " << get_ndims() << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=
      get_ID_labeled_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);
   if (OBSFRUSTUM_ptr==NULL) return;
//   cout << "OBSFRUSTUM_ptr = " << OBSFRUSTUM_ptr << endl;
   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
//   cout << "Movie_ptr = " << Movie_ptr << endl;

   bool twoD_flag=false;
   Movie_ptr->reset_displayed_photograph(photo_filename,twoD_flag);
}
*/

// ==========================================================================
// SignPost projection into image plane member functions
// ==========================================================================

// Member function generate_SignPost_at_imageplane_location takes in
// the UV coordinates for the tip of a new SignPost to annotate the
// image plane within the OBSFRUSTUM labeled by the input ID argument.
// This method instantiates and returns a new SignPosts member of
// input *SignPostsGroup_ptr whose cone tip is located at a 3D
// location corresponding to UV and which is parallel to the specified
// image plane.

SignPost* OBSFRUSTAGROUP::generate_SignPost_at_imageplane_location(
   const twovector& UV,int OBSFRUSTUM_ID,
   SignPostsGroup* imageplane_SignPostsGroup_ptr)
{
   OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);

   double curr_size=imageplane_SignPostsGroup_ptr->
      get_common_geometrical_size();
   double height_multiplier=1.0;
//   double height_multiplier=2.0;
   
// As of 2/11/09, we assume that imageplane SignPosts are stationary
// over time.  So we instantiate them at t=t_initial rather than
// t=curr_t:

   SignPost* imageplane_SignPost_ptr=OBSFRUSTUM_ptr->
      generate_SignPost_at_imageplane_location(
         UV,imageplane_SignPostsGroup_ptr,get_initial_t(),get_passnumber(),
         curr_size,height_multiplier);

   imageplane_SignPost_ptr->set_color(
      colorfunc::get_OSG_color(colorfunc::red));
   imageplane_SignPost_ptr->set_permanent_color(
      colorfunc::get_OSG_color(colorfunc::red));
   imageplane_SignPost_ptr->set_selected_color(
      colorfunc::get_OSG_color(colorfunc::yellow));

   imageplane_SignPost_ptr->change_text_size(0,1.7);
   imageplane_SignPost_ptr->set_max_text_width("1234567");
   return imageplane_SignPost_ptr;
}

// ---------------------------------------------------------------------
// Member function project_SignPosts_into_imageplanes() loops over
// every SignPost within input *SignPostsGroup_ptr and extracts its
// XYZ tip position in world-space coordinates.  It then loops over
// every OBSFRUSTUM within *this and projects XYZ into each image
// plane.  For each set of UV coordinates lying within the valid
// intervals [Umin,Umax] and [Vmin,Vmax], this method computes the 3D
// distance between the projected image plane SignPost and the
// panorama camera's position.  It instantiates a new SignPost for the
// projection lying closest to the camera.

void OBSFRUSTAGROUP::project_SignPosts_into_imageplanes(
   SignPostsGroup* SignPostsGroup_ptr)
{
//   cout << "inside OBSFRUSTAGROUP::project_SignPosts_into_imageplanes()"
//        << endl;
   unsigned int n_3D_SignPosts=SignPostsGroup_ptr->get_n_Graphicals();
//   cout << "n_3D_signPosts = " << n_3D_SignPosts << endl;
   for (unsigned int s=0; s<n_3D_SignPosts; s++)
   {
      SignPost* SignPost_ptr=SignPostsGroup_ptr->get_SignPost_ptr(s);
      threevector XYZ;
      if (SignPost_ptr->get_UVW_coords(
         SignPostsGroup_ptr->get_curr_t(),
         SignPostsGroup_ptr->get_passnumber(),XYZ))
      {
//         cout << "XYZ = " << XYZ << endl;

         unsigned int n_panorama_stills=get_n_Graphicals()-1;
         int n_closest_OBSFRUSTUM=-1;
         double min_imageplane_proj_distance_to_camera=POSITIVEINFINITY;
         twovector closest_UV;
         for (unsigned int n=0; n<n_panorama_stills; n++)
         {
            OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(n);
            Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
            camera* camera_ptr=Movie_ptr->get_camera_ptr();

            const genmatrix* P_ptr=camera_ptr->get_P_ptr();
            threevector UVW=(*P_ptr) * fourvector(XYZ,1);
            double U=UVW.get(0)/UVW.get(2);
            double V=UVW.get(1)/UVW.get(2);

            if (U > Movie_ptr->get_minU() && U < Movie_ptr->get_maxU() &&
                V > Movie_ptr->get_minV() && V < Movie_ptr->get_maxV())
            {
//               cout << "SignPost index s = " << s << " XYZ = " << XYZ
//                    << " label = " << SignPost_ptr->get_label()
//                    << endl;

//               cout << "OBSFRUSTUM index n = " << n << " U = " << U 
//                    << " V = " << V << endl;
               
               twovector UV(U,V);
               threevector imageplane_posn=OBSFRUSTUM_ptr->get_Movie_ptr()->
                  imageplane_location(UV);
               double imageplane_proj_distance_to_camera=(
                  imageplane_posn-camera_ptr->get_world_posn()).magnitude();
//               cout << "camera_world_posn = " << camera_ptr->get_world_posn()
//                    << endl;
//               cout << "imageplane_posn = " << imageplane_posn << endl;
               if (imageplane_proj_distance_to_camera < 
                   min_imageplane_proj_distance_to_camera)
               {
                  min_imageplane_proj_distance_to_camera=
                     imageplane_proj_distance_to_camera;
                  n_closest_OBSFRUSTUM=n;
                  closest_UV=UV;
               }
            } // U,V within valid ranges for OBSFRUSTUM n               
//            outputfunc::enter_continue_char();
         } // loop over index n labeling OBSFRUSTA

//         cout << "n_closest_OBSFRUSTUM = " << n_closest_OBSFRUSTUM << endl;
         if (n_closest_OBSFRUSTUM >= 0)
         {
            SignPostsGroup* imageplane_SignPostsGroup_ptr=
               SignPostsGroup_ptr->get_imageplane_SignPostsGroup_ptr();
            if (imageplane_SignPostsGroup_ptr==NULL)
            {
               cout << "Error in OBSFRUSTAGROUP::project_SignPosts_into_imageplanes()" << endl;
               cout << "imageplane_SignPostsGroup_ptr=NULL!" << endl;
               continue;
            }

            SignPost* imageplane_SignPost_ptr=
               generate_SignPost_at_imageplane_location(
                  closest_UV,n_closest_OBSFRUSTUM,
                  imageplane_SignPostsGroup_ptr);
            imageplane_SignPost_ptr->set_label(SignPost_ptr->get_label());
            imageplane_SignPost_ptr->copy_size(
               imageplane_SignPostsGroup_ptr->get_curr_t(),
               imageplane_SignPostsGroup_ptr->get_passnumber(),SignPost_ptr);
//            cout << "imageplane_SignPost_ptr = "
//                 << imageplane_SignPost_ptr << endl;
//            cout << "*imageplane_SignPost_ptr = "
//                 << *imageplane_SignPost_ptr << endl;
//            cout << "SignPostsGroup_ptr->get_n_Graphicals() = "
//                 << SignPostsGroup_ptr->get_n_Graphicals() << endl;
//            cout << "imageplane_SignPostsGroup_ptr->get_n_Graphicals() = "
//                 << imageplane_SignPostsGroup_ptr->get_n_Graphicals() << endl;
         }
         
      } // SignPost get UVW coords conditional
   } // loop over index s labeling 3D SignPosts
}

// ---------------------------------------------------------------------
// Member function project_SignPosts_into_video_plane() 

void OBSFRUSTAGROUP::project_SignPosts_into_video_plane(
   SignPostsGroup* SignPostsGroup_ptr)
{
//   cout << "inside OBSFRUSTAGROUP::project_SignPosts_into_video_plane()"
//        << endl;
   
   int n_3D_SignPosts=SignPostsGroup_ptr->get_n_Graphicals();
   for (int s=0; s<n_3D_SignPosts; s++)
   {
      SignPost* SignPost_ptr=SignPostsGroup_ptr->get_SignPost_ptr(s);
      threevector XYZ;
      if (SignPost_ptr->get_UVW_coords(
         SignPostsGroup_ptr->get_curr_t(),
         SignPostsGroup_ptr->get_passnumber(),XYZ))
      {
         unsigned int n_video=get_n_Graphicals()-1;
         OBSFRUSTUM* video_OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(n_video);
         Movie* Movie_ptr=video_OBSFRUSTUM_ptr->get_Movie_ptr();
         camera* camera_ptr=Movie_ptr->get_camera_ptr();

         int first_framenumber=AnimationController_ptr->
            get_first_framenumber();
         int last_framenumber=AnimationController_ptr->
            get_last_framenumber();
         cout << "first_framenumber = " << first_framenumber
              << " last_framenumber = " << last_framenumber << endl;

         double frustum_sidelength;
         genmatrix P(3,4);
         if (!import_package_params(
            get_curr_framenumber(),frustum_sidelength,P)) continue;

         threevector UVW=P * fourvector(
            XYZ-camera_ptr->get_world_posn(),1);
         double U=UVW.get(0)/UVW.get(2);
         double V=UVW.get(1)/UVW.get(2);

         if (U > Movie_ptr->get_minU() && U < Movie_ptr->get_maxU() &&
             V > Movie_ptr->get_minV() && V < Movie_ptr->get_maxV())
         {
            cout << "curr_framenumber = " << get_curr_framenumber() 
                 << " U = " << U << " V = " << V << endl;
            cout << "s = " << s << " XYZ = " << XYZ
                 << " label = " << SignPost_ptr->get_label()
                 << endl;
         } // U,V within valid ranges for video OBSFRUSTUM              
      } // SignPost get_UVW_coords conditional
   } // loop over index s labeling SignPosts
} 

// ==========================================================================
// Polyhedra projection into image plane member functions
// ==========================================================================

// Member function compute_photo_views_of_polyhedra_bboxes() loops
// over all members of *PolyhedraGroup_ptr.  For each polyhedron, it
// generates a surface point cloud and visualizes those points via an
// Arrow vector field.  It then projects each polyhedron into each
// OBSFRUSTUM within *this and calculates an imageplane visibility
// score.  The visibility scores are sorted and stored within the
// corresponding photographs.

void OBSFRUSTAGROUP::compute_photo_views_of_polyhedra_bboxes(
   photogroup* photogroup_ptr)
{
   cout << "inside OBSFRUSTAGROUP::compute_photo_views_of_polyhedra_bboxes()"
        << endl;
   
   for (unsigned int p=0; p<PolyhedraGroup_ptr->get_n_Graphicals(); p++)
   {
      Polyhedron* Polyhedron_ptr=PolyhedraGroup_ptr->get_Polyhedron_ptr(p);
      polyhedron* polyhedron_ptr=Polyhedron_ptr->get_polyhedron_ptr();

      const double ds_frac=0.2;
      vector<pair<threevector,threevector> > pnts_normals=
         polyhedron_ptr->generate_surface_point_cloud(ds_frac);

      if (ArrowsGroup_ptr != NULL)
         ArrowsGroup_ptr->display_polyhedron_surface_points(polyhedron_ptr);

      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(n);
         Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
         bounding_box UVW_bbox(100.0*POSITIVEINFINITY,100.0*NEGATIVEINFINITY,
                               100.0*POSITIVEINFINITY,100.0*NEGATIVEINFINITY);
         double integrated_points_visibility=
            OBSFRUSTUM_ptr->project_polyhedron_point_cloud_into_imageplane(
               polyhedron_ptr,Movie_ptr->get_camera_ptr(),
               DTED_ztwoDarray_ptr,UVW_bbox);
         double integrated_proj_face_area=
            OBSFRUSTUM_ptr->integrate_projected_polyhedron_faces_area(
               polyhedron_ptr);
         OBSFRUSTUM_ptr->calculate_imageplane_score(
            integrated_points_visibility,integrated_proj_face_area,UVW_bbox);

// Experiment with transfering OBSFRUSTUM's geolocation into
// corresponding photograph:

         photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(
            OBSFRUSTUM_ptr->get_ID());
         
         threevector XYZ;
         if (OBSFRUSTUM_ptr->get_UVW_coords(
            get_curr_t(),get_passnumber(),XYZ))
         {
            bool northern_hemisphere_flag=true;
            int UTM_zone=18;	// NYC
            geopoint geolocation(
               northern_hemisphere_flag,UTM_zone,
               XYZ.get(0),XYZ.get(1),XYZ.get(2));
            cout << "geolocation = " << geolocation << endl;
            photograph_ptr->set_geolocation(geolocation);
         }
         
      } // loop over index n labeling OBSFRUSTA

      photogroup_ptr->order_photos_by_their_scores();
   } // loop over index p labeling Polyhedra
}

// ---------------------------------------------------------------------
// Member function project_Polyhedra_into_selected_photo() loops over
// each Polyhedron within *PolyhedraGroup_ptr and projects their faces
// into *PolygonsGroup_ptr.  We wrote this method in Sep 2009 in order
// to project NYC skyscraper bounding boxes into georegistered photos
// reconstructed by Noah Snavely's BUNDLER algorithms.

bool OBSFRUSTAGROUP::project_Polyhedra_into_selected_photo(int selected_ID)
{
   cout << "inside OBSFRUSTAGROUP::project_Polyhedra_into_selected_photo()"
        << endl;
//   cout << "selected_ID = " << selected_ID 
//        << " most recently selected ID = "
//        << get_most_recently_selected_ID()
//        << endl;

//   cout << "PolyhedraGroup_ptr->get_selected_Graphical_ID() = "
//        << PolyhedraGroup_ptr->get_selected_Graphical_ID() << endl;
//   cout << "PolyhedraGroup_ptr->get_prev_selected_Graphical_ID() = "
//        << PolyhedraGroup_ptr->get_prev_selected_Graphical_ID() << endl;

// Project 3D Polyhedra into 2D image plane if either selected
// OBSFRUSTUM OR selected Polyhedron has changed:

   if ( selected_ID==get_most_recently_selected_ID() &&
   PolyhedraGroup_ptr->get_selected_Graphical_ID()==
   PolyhedraGroup_ptr->get_most_recently_selected_ID() ) return false;

   set_most_recently_selected_ID(selected_ID);
   PolyhedraGroup_ptr->set_most_recently_selected_ID(
      PolyhedraGroup_ptr->get_selected_Graphical_ID());

   PolygonsGroup_ptr->destroy_all_Polygons();

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(selected_ID);
   if (!OBSFRUSTUM_ptr->project_Polyhedra_into_imageplane(
      PolyhedraGroup_ptr,PolygonsGroup_ptr,DTED_ztwoDarray_ptr))
   {
      PolygonsGroup_ptr->destroy_all_Polygons();
   }
   return true;
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

bool OBSFRUSTAGROUP::parse_next_message_in_queue(message& curr_message)
{
//   cout << "inside OBSFRUSTAGROUP::parse_next_message_in_queue()" << endl;
//   cout << "curr_message.get_text_message() = "
//        << curr_message.get_text_message() << endl;

   bool message_handled_flag=false;
   if (curr_message.get_text_message()=="SELECT_VERTEX")
   {
//      cout << "Received SELECT_VERTEX message from ActiveMQ" << endl;
      curr_message.extract_and_store_property_keys_and_values();
      string type=curr_message.get_property_value("TYPE");
//      cout << "type = " << type << endl;

      if (type=="PHOTO")
      {
         string photo_ID_str=curr_message.get_property_value("ID");  
//         cout << "photo_ID string = " << photo_ID_str << endl;
         int photo_ID=stringfunc::string_to_integer(photo_ID_str);
         int OBSFRUSTUM_ID=get_selected_photo_OBSFRUSTUM_ID(photo_ID);
         OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(
            OBSFRUSTUM_ID);

         if (OBSFRUSTUM_ptr != NULL)
         {
//            set_selected_Graphical_ID(-1);
            set_selected_Graphical_ID(OBSFRUSTUM_ID);
            double max_blink_period=20;		// secs
            OBSFRUSTUM_ptr->set_blinking_color(colorfunc::red);
            blink_OBSFRUSTUM(OBSFRUSTUM_ID,max_blink_period);

// Call reset_colors to enable OBSFRUSTUM blinking:

            reset_colors();

// On 6/27/09, we experiment with resetting the virtual camera's XY
// coordinates to those of the selected OBSFRUSTUM's apex in order
// to simplify locating of the selected OBSFRUSTUM:

            if (get_CM_refptr().valid() && get_CM_3D_ptr()->
                get_active_control_flag() && jump_to_apex_flag)
            {
               threevector apex_posn=OBSFRUSTUM_ptr->
                  get_ViewingPyramid_ptr()->get_pyramid_ptr()->get_apex().
                  get_posn();
               threevector eye_world_posn=get_CM_3D_ptr()->
                  get_eye_world_posn();
               eye_world_posn.put(0,apex_posn.get(0));
               eye_world_posn.put(1,apex_posn.get(1));

               double min_Z_eye=3000;	// meters
               double Z_eye=basic_math::max(min_Z_eye,eye_world_posn.get(2));
               eye_world_posn.put(2,Z_eye);
               rotation final_R;
               final_R.identity();
               get_CM_3D_ptr()->jumpto(eye_world_posn,final_R);
            }

         } // OBSFRUSTUM_ptr != NULL conditional
      }
      message_handled_flag=true;
   } // curr_message.get_text_message() conditional

   return message_handled_flag;
}

// --------------------------------------------------------------------------
// Member function blink_OBSFRUSTUM() sets the blinking flag and start
// time for the selected OBSFRUSTUM.

bool OBSFRUSTAGROUP::blink_OBSFRUSTUM(
   int OBSFRUSTUM_ID,double max_blink_period)
{
//   cout << "inside blink_OBSFRUSTUM(), OBSFRUSTUM_ID = " << OBSFRUSTUM_ID << endl;

   if (OBSFRUSTUM_ID < 0) return false;

// Initialize all OBSFRUSTA blinking flags to false:
      
   for (unsigned int p=0; p<get_n_Graphicals(); p++)
   {
      get_OBSFRUSTUM_ptr(p)->set_blinking_flag(false);
   }

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);
   if (OBSFRUSTUM_ptr==NULL) return false;
   
   OBSFRUSTUM_ptr->set_blinking_flag(true);
   OBSFRUSTUM_ptr->set_blinking_start_time(
      timefunc::elapsed_timeofday_time());
   OBSFRUSTUM_ptr->set_max_blink_period(max_blink_period);	// secs
   return true;

}
// --------------------------------------------------------------------------
// Member function issue_select_vertex_message() broadcasts the ID of
// the photo/Movie corresponding to the most recently selected
// OBSFRUSTUM provided it does not equal -1.

void OBSFRUSTAGROUP::issue_select_vertex_message()
{
//   cout << "inside OBSFRUSTAGROUP::issue_select_vertex_message()" << endl;

   if (get_Messenger_ptr()==NULL ||
       !get_Messenger_ptr()->connected_to_broker_flag() ||
       get_selected_Graphical_ID() < 0) return;

//   cout << "photo_ID = " << get_selected_OBSFRUSTUM_photo_ID() << endl;

   string command="SELECT_VERTEX";

   string key,value;
   typedef pair<string,string> property;
   vector<property> properties;
      
   key="TYPE";
   value="PHOTO";		
   properties.push_back(property(key,value));

   key="ID";
   value=stringfunc::number_to_string(get_selected_OBSFRUSTUM_photo_ID());
   properties.push_back(property(key,value));
      
   get_Messenger_ptr()->sendTextMessage(command,properties);

   jump_to_apex_flag=false;
}

// --------------------------------------------------------------------------
// Member function reset_to_common_imageplane()

void OBSFRUSTAGROUP::reset_to_common_imageplane()
{
   cout << "inside OBSFRUSTAGROUP::reset_to_common_imageplane()" << endl;

//    int selected_photo_ID=get_selected_Graphical_ID();
   for (unsigned int i=0; i<get_n_Graphicals(); i++)
   {
      cout << "imageplane ID = " << i << endl;

      Movie* other_Movie_ptr=get_MoviesGroup_ptr()->
         get_ID_labeled_Movie_ptr(i);

      cout << "bottom_left_corner = " 
           << other_Movie_ptr->get_bottom_left_XYZ() << endl;
      cout << "top_right_corner = " 
           << other_Movie_ptr->get_top_right_XYZ() << endl;

/*
      if (i==selected_photo_ID) continue;

      camera* other_camera_ptr=other_Movie_ptr->get_camera_ptr();
      plane* other_imageplane_ptr=get_photogroup_ptr()->get_image_plane(i);

      other_Movie_ptr->remap_window_corners(
         other_camera_ptr->get_world_posn(),other_imageplane_ptr,
         other_camera_ptr->get_UV_corner_world_ray());
*/

   } // loop over index i labeling OBSFRUSTA
   

}



/*
// ---------------------------------------------------------------------
// Member function project_imageplanes_into_selected_photo()

void OBSFRUSTAGROUP::project_imageplanes_into_selected_photo(
   int selected_photo_ID)
{
//   cout << "inside OBSFRUSTAGROUP::project_imageplanes_into_selected_photo()"
//        << endl;
//   cout << "selected_ID = " << selected_ID 
//        << endl;

   if (selected_ID==get_most_recently_selected_ID()) return false;
   set_most_recently_selected_ID(selected_ID);
   

   PolygonsGroup_ptr->destroy_all_Polygons();

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(selected_ID);
   if (!OBSFRUSTUM_ptr->project_Polyhedra_into_imageplane(
      PolyhedraGroup_ptr,PolygonsGroup_ptr,DTED_ztwoDarray_ptr))
   {
      PolygonsGroup_ptr->destroy_all_Polygons();
   }
   return true;
}
*/

// --------------------------------------------------------------------------
// Member function compute_other_visible_imageplanes()

void OBSFRUSTAGROUP::compute_other_visible_imageplanes()
{
   cout << "inside OBSFRUSTAGROUP::compute_other_visible_imageplanes()"
        << endl;

/*
   for (unsigned int m=0; m<get_MoviesGroup_ptr()->get_n_Graphicals(); m++)
   {
      Movie* Movie_ptr=get_MoviesGroup_ptr()->get_Movie_ptr(m);
      cout << "m = " << m << " Movie ID = " << Movie_ptr->get_ID()
           << endl;
   }
*/

   int selected_photo_ID=get_selected_Graphical_ID();
   Movie* selected_Movie_ptr=get_MoviesGroup_ptr()->
      get_ID_labeled_Movie_ptr(selected_photo_ID);

   if (!selected_Movie_ptr->get_warp_onto_imageplane_flag()) return;

   camera* selected_camera_ptr=selected_Movie_ptr->get_camera_ptr();

   vector<int> plane_orders=
      photogroup_ptr->get_overlapping_imageplane_orders(selected_photo_ID);

   PolygonsGroup_ptr->destroy_all_Polygons();
   PolyLinesGroup* PolyLinesGroup_ptr=
      PolygonsGroup_ptr->get_PolyLinesGroup_ptr();
//   cout << "PolyLinesGroup_ptr = " << PolyLinesGroup_ptr << endl;

   cout << "# overlapping planes = " << plane_orders.size() << endl;

   plane* curr_imageplane_ptr=selected_camera_ptr->get_imageplane_ptr();
   for (unsigned int p=0; p<plane_orders.size(); p++)
   {
      Movie* other_Movie_ptr=get_MoviesGroup_ptr()->
         get_Movie_ptr(plane_orders[p]);
      camera* other_camera_ptr=other_Movie_ptr->get_camera_ptr();

      vector<threevector> vertices;
      for (int c=0; c<4; c++)
      {
         threevector intersection_point;
         other_camera_ptr->backproject_corner_into_world_plane(
            c,*curr_imageplane_ptr,intersection_point);
//         cout << "c = " << c << " intersection point = "
//              << intersection_point << endl;
         vertices.push_back(intersection_point);
      } // loop over index c labeling selected image corners

// Add starting vertex to back of vertices vector in order for
// PolyLine to close on itself:

      vertices.push_back(vertices.front());

      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
         Zero_vector,vertices,colorfunc::get_OSG_color(
            colorfunc::get_color(modulo(p,colorfunc::get_n_colors()) )));
      PolyLine_ptr->set_linewidth(2);

   } // loop over index p labeling other image planes
}

// ==========================================================================
// Bundler member functions
// ==========================================================================

// Member function display_visible_reconstructed_XYZ_points()

void OBSFRUSTAGROUP::display_visible_reconstructed_XYZ_points(int selected_ID)
{
//   cout << "inside OBSFRUSTAGROUP::display_visible_reconstructed_XYZ_points()"
//        << endl;

   if (selected_ID==get_most_recently_selected_ID()) return;
   set_most_recently_selected_ID(selected_ID);

   PointsGroup_ptr->destroy_all_Points();
 
   videofunc::CAMERAID_XYZ_MAP::iterator camID_xyz_iter=
      cameraID_xyz_map_ptr->find(selected_ID);
   if (camID_xyz_iter==cameraID_xyz_map_ptr->end()) return;
   
   vector<threevector> XYZ_points=camID_xyz_iter->second;
   cout << "XYZ_points.size() = " << XYZ_points.size() << endl;

   bool draw_text_flag=false;
   PointsGroup_ptr->set_crosshairs_size(0.1);

   for (unsigned int i=0; i<XYZ_points.size(); i++)
   {
//      cout << "i = " << i << endl;
      osgGeometry::Point* Point_ptr=
         PointsGroup_ptr->generate_new_Point(XYZ_points[i],draw_text_flag);
      Point_ptr->set_color(colorfunc::get_OSG_color(colorfunc::purple));
   }
}

// --------------------------------------------------------------------------
// Member function populate_image_vs_package_names_map()

void OBSFRUSTAGROUP::populate_image_vs_package_names_map(
   string packages_subdir)
{
   cout << "inside OBSFRUSTAGROUP::populate_image_vs_package_names_map()"
        << endl;

   image_vs_package_names_map_ptr=new IMAGE_VS_PACKAGE_NAMES_MAP;

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("pkg");
   
   vector<string> package_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,packages_subdir);

   for (unsigned int p=0; p<package_filenames.size(); p++)
   {
      filefunc::ReadInfile(package_filenames[p]);
      string image_filename=filefunc::text_line[0];
      (*image_vs_package_names_map_ptr)[image_filename]=package_filenames[p];
   }
}

// --------------------------------------------------------------------------
// Member function get_package_filename() takes in some image
// filename.  If a counterpart package filenames exists within
// *image_vs_package_names_map_ptr, this method retrieves and returns
// it.

string OBSFRUSTAGROUP::get_package_filename(string image_filename)
{
   string package_filename="";
   if (image_vs_package_names_map_ptr != NULL)
   {
      IMAGE_VS_PACKAGE_NAMES_MAP::iterator iter=
         image_vs_package_names_map_ptr->find(image_filename);
      if (iter != image_vs_package_names_map_ptr->end())
      {
         package_filename=iter->second;
      }
   }
   return package_filename;
}

// ==========================================================================
// Raytracing member functions
// ==========================================================================

// Member function compute_D7_FOV()

void OBSFRUSTAGROUP::compute_D7_FOV(
   double& FOV_triangle_min_az,double& FOV_triangle_max_az)
{
   cout << "inside OBSFRUSTAGROUP::compute_D7_FOV()" << endl;

//   bool northern_hemisphere_flag=true;
//   int UTM_zone=19;	// Boston
   double D7_Z=11;	// meters

   FOV_triangle_min_az=POSITIVEINFINITY;
   FOV_triangle_max_az=NEGATIVEINFINITY;

   unsigned int n_OBSFRUSTA=get_n_Graphicals();
   for (unsigned int p=0; p<n_OBSFRUSTA; p++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(p);
//      cout << "p= " << p << " OBSFRUSTUM_ptr = " << OBSFRUSTUM_ptr << endl;

      pyramid viewing_pyramid(*(OBSFRUSTUM_ptr->get_viewing_pyramid_ptr()));
//      cout << "viewing_pyramid = " << viewing_pyramid << endl;

      threevector apex=viewing_pyramid.get_apex().get_posn();
      twovector apex_XY(apex);

      threevector scalefactor(300,300,300);
      viewing_pyramid.scale(apex,scalefactor);

      vector<vertex> vertices_above_Zplane;
      vector<edge> edges_above_Zplane;
      vector<face> triangles_above_Zplane;
      viewing_pyramid.extract_parts_above_Zplane(
         D7_Z,vertices_above_Zplane,edges_above_Zplane,triangles_above_Zplane);
//      cout << "vertices_above_Zplane = " << endl;
//      templatefunc::printVector(vertices_above_Zplane);

      for (unsigned int i=0; i<vertices_above_Zplane.size(); i++)
      {
         threevector curr_vertex(vertices_above_Zplane[i].get_posn());
         if (nearly_equal(curr_vertex.get(2),D7_Z))
         {
            twovector vertex_XY(curr_vertex);
            twovector ground_ray=(vertex_XY-apex_XY).unitvector();
            double curr_theta=atan2(ground_ray.get(1),ground_ray.get(0));
            FOV_triangle_min_az=basic_math::min(
               FOV_triangle_min_az,curr_theta);
            FOV_triangle_max_az=basic_math::max(
               FOV_triangle_max_az,curr_theta);
         }
      } // loop over index i labeling vertices above Zplane
   } // loop over index p labeling OBSFRUSTA

   cout << "FOV_triangle_min_az = " << FOV_triangle_min_az*180/PI << endl;
   cout << "FOV_triangle_max_az = " << FOV_triangle_max_az*180/PI << endl;

   for (unsigned int p=0; p<n_OBSFRUSTA; p++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(p);
      OBSFRUSTUM_ptr->set_FOV_triangle_min_az(FOV_triangle_min_az);
      OBSFRUSTUM_ptr->set_FOV_triangle_max_az(FOV_triangle_max_az);
   }
}

// ---------------------------------------------------------------------
// Member function draw_ray_through_imageplane() returns a linesegment
// whose V1 vertex corresponds to the specified OBSFRUSTUM's camera
// position and V2 vertex corresponds to the unit ray from the camera
// position along the ray through the image plane.

linesegment OBSFRUSTAGROUP::draw_ray_through_imageplane(
   int Arrow_ID,int OBSFRUSTUM_ID,const twovector& UV,
   double magnitude,colorfunc::Color color,double linewidth)
{
//   cout << "inside OBSFRUSTAGROUP::draw_ray_through_imageplane()" << endl;
//   cout << "color = " << colorfunc::get_colorstr(color) << endl;

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);
   Arrow* Arrow_ptr=OBSFRUSTUM_ptr->get_Arrow_ptr(Arrow_ID);
   if (Arrow_ptr==NULL)
   {
      Arrow_ptr=OBSFRUSTUM_ptr->draw_ray_through_imageplane(
         Arrow_ID,UV,magnitude,color,linewidth);
      insert_graphical_PAT_into_OSGsubPAT(Arrow_ptr);
      n_3D_rays++;
   }
   else
   {
      Arrow_ptr=OBSFRUSTUM_ptr->draw_ray_through_imageplane(
         Arrow_ID,UV,magnitude,color,linewidth);
   }
   
   camera* camera_ptr=OBSFRUSTUM_ptr->get_Movie_ptr()->get_camera_ptr();
   threevector camera_posn=camera_ptr->get_world_posn();
//   cout << "camera_posn = " << camera_posn << endl;
   threevector V1(camera_posn);
   threevector V2(camera_posn+Arrow_ptr->get_V_tip()-Arrow_ptr->get_V_base());
   linesegment l(V1,V2);
//   cout << "linesegment = " << l << endl;

   return l;
}

// ---------------------------------------------------------------------
// Member function triangulate_rays() takes in the ID for some set of
// rays which we assume intersect at a common location in 3D space.
// Looping over all OBSFRUSTA, this method instantiates line segments
// corresponding to Arrows stored within each OBSFRUSTUM.  It then
// computes the least-squares best fit intersection location for the
// rays.  The 3D intersection point is displayed as a purple Point.

threevector OBSFRUSTAGROUP::triangulate_rays(int Arrow_ID)
{
   cout << "inside OBSFRUSTAGROUP::triangulate_rays(), Arrow_ID = " 
        << Arrow_ID << endl;

// Export 3D rays to output text file for importing into Brendan
// Edwards Cesium thin client:

   bool northern_hemisphere_flag=true;
   int UTM_zone=19;
   string rays_filename="rays_3D.dat";
   ofstream outstream;
   outstream.precision(12);
   filefunc::openfile(rays_filename,outstream);
 
   vector<linesegment> lines;
   for (unsigned int i=0; i<get_n_Graphicals(); i++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(i);
      ArrowsGroup* ArrowsGroup_ptr=OBSFRUSTUM_ptr->get_ArrowsGroup_ptr();
      if (ArrowsGroup_ptr==NULL) continue;
//      cout << " ArrowsGroup_ptr = " << ArrowsGroup_ptr 
//           << " n_arrows = " << ArrowsGroup_ptr->get_n_Graphicals() << endl;

      Arrow* Arrow_ptr=ArrowsGroup_ptr->get_ID_labeled_Arrow_ptr(Arrow_ID);
      if (Arrow_ptr==NULL) continue;
      
//      cout << "Arrow_ID = " << Arrow_ID << endl;
//      cout << "Arrow_ptr = " << Arrow_ptr << endl;
//      cout << "Base = " << Arrow_ptr->get_V_base() << endl;
//      cout << "Tip = " << Arrow_ptr->get_V_tip() << endl;

      camera* camera_ptr=OBSFRUSTUM_ptr->get_Movie_ptr()->get_camera_ptr();
      threevector camera_posn=camera_ptr->get_world_posn();
//      cout << "camera_posn = " << camera_posn << endl;
      threevector V1(camera_posn);
      threevector V2=V1+Arrow_ptr->get_V_tip()-Arrow_ptr->get_V_base();
      lines.push_back(linesegment(V1,V2));

      geopoint V1_geopt(
         northern_hemisphere_flag,UTM_zone,V1.get(0),V1.get(1),V1.get(2));
      geopoint V2_geopt(
         northern_hemisphere_flag,UTM_zone,V2.get(0),V2.get(1),V2.get(2));
         
      outstream << i 
                << "   " << V1_geopt.get_longitude()
                << "   " << V1_geopt.get_latitude()
                << "   " << V1_geopt.get_altitude()
                << endl;

      outstream << i 
                << "   " << V2_geopt.get_longitude()
                << "   " << V2_geopt.get_latitude()
                << "   " << V2_geopt.get_altitude()
                << endl;


   }
   filefunc::closefile(rays_filename,outstream);

   threevector r_intersection(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   if (!geometry_func::multi_line_intersection_point(lines,r_intersection))
      return r_intersection;
   cout << "r_intersection = " << r_intersection << endl;

// Display triangulated locations as 3D points:

//   r_intersection -= *(get_grid_world_origin_ptr());

   PointsGroup_ptr->destroy_all_Points();
//   PointsGroup_ptr->set_crosshairs_size(0.1);
//   PointsGroup_ptr->set_crosshairs_size(5);		// Puma
    PointsGroup_ptr->set_crosshairs_size(15);		// GEO

   bool draw_text_flag=false;
   osgGeometry::Point* Point_ptr=
      PointsGroup_ptr->generate_new_Point(r_intersection,draw_text_flag);
   
//   Point_ptr->set_permanent_color(colorfunc::purple);
   Point_ptr->set_permanent_color(colorfunc::white);

   return r_intersection;
}

// ==========================================================================
// Virtual OBSFRUSTUM creation & manipulation member functions
// ==========================================================================

// Member function
// physical_OBSFRUSTUM_pointing_closest_to_virtual_OBSFRUSTUM()
// assumes that the current OBSFRUSTAGROUP is filled with frusta
// corresponding to physical cameras plus one final frustum which is
// virtual.  Looping over all physical cameras, it finds the one whose
// pointing direction vector is closest to the virtual camera's.  This
// method returns a pointer to the physical OBSFRUSTUM whose pointing
// most closely matches that of *virtual_OBSFRUSTUM_ptr.

OBSFRUSTUM* 
OBSFRUSTAGROUP::physical_OBSFRUSTUM_pointing_closest_to_virtual_OBSFRUSTUM(
   double z_ground)
{
   cout << "inside OBSFRUSTAGROUP::physical_OBSFRUSTUM_pointing_closest_to_virtual_OBSFRUSTUM()" << endl;
   
   virtual_camera_ptr=virtual_OBSFRUSTUM_ptr->get_Movie_ptr()->
      get_camera_ptr();
   threevector virtual_khat=-virtual_camera_ptr->get_What();
//   cout << "virtual_khat = " << virtual_khat << endl;

   vector<threevector> virtual_corners=virtual_camera_ptr->
      corner_ray_intercepts_with_zplane(z_ground);
   polygon virtual_footprint(virtual_corners);
//   cout << "virtual_footprint = " << virtual_footprint << endl;

   polyline virtual_footprint_polyline(virtual_corners);
   bounding_box virtual_bbox(&virtual_footprint_polyline);
//   cout << "virtual_bbox = " << virtual_bbox << endl;
   double xmin=virtual_bbox.get_xmin();
   double xmax=virtual_bbox.get_xmax();
   double ymin=virtual_bbox.get_ymin();
   double ymax=virtual_bbox.get_ymax();
   int n_steps=15;
   double dx=(xmax-xmin)/(n_steps-1);
   double dy=(ymax-ymin)/(n_steps-1);

// Find physical image whose pointing vector matches as closely as
// possible with virtual image's point direction:

   int closest_OBSFRUSTUM_ID=-1;
   double max_combined_product=NEGATIVEINFINITY;
   for (unsigned int p=0; p<get_n_Graphicals()-1; p++)
   {
      camera* camera_ptr=get_OBSFRUSTUM_ptr(p)->get_Movie_ptr()->
         get_camera_ptr();
      vector<threevector> physical_corners=camera_ptr->
         corner_ray_intercepts_with_zplane(z_ground);
      polygon physical_footprint(physical_corners);
//      cout << "physical_footprint = " << physical_footprint << endl;

// Loop over virtual bbox cells.  Count number of cells that lie
// inside both virtual and physical footprints:

      int n_virtual_cells=0;
      int n_overlap_cells=0;
      for (int i=0; i<n_steps; i++)
      {
         double curr_x=xmin+i*dx;
         for (int j=0; j<n_steps; j++)
         {
            double curr_y=ymin+j*dy;
            threevector curr_point(curr_x,curr_y,z_ground);
            if (virtual_footprint.point_inside_polygon(curr_point))
            {
               n_virtual_cells++;
               if (physical_footprint.point_inside_polygon(curr_point))
               {
                  n_overlap_cells++;
               }
            }
         } // loop over index j 
      } // loop over index i 
      double overlap_frac=double(n_overlap_cells)/double(sqr(n_steps));

      threevector khat=-camera_ptr->get_What();
      double curr_dotproduct=khat.dot(virtual_khat);
      double curr_combined_product=curr_dotproduct*sqr((overlap_frac));
      if (curr_combined_product > max_combined_product)
      {
         max_combined_product=curr_combined_product;
         closest_OBSFRUSTUM_ID=p;
         cout << "curr_dotproduct = " << curr_dotproduct
              << " overlap/virtual cells = " 
              << double(n_overlap_cells)/double(n_virtual_cells) 
              << " closest_OBSFRUSTUM_ID = " << closest_OBSFRUSTUM_ID 
              << endl;
      }
   } // loop over index p labeling physical images

   OBSFRUSTUM* closest_OBSFRUSTUM_ptr=get_OBSFRUSTUM_ptr(
      closest_OBSFRUSTUM_ID);
   Movie* closest_Movie_ptr=closest_OBSFRUSTUM_ptr->get_Movie_ptr();
   cout << "image = " << closest_Movie_ptr->get_video_filename() << endl;
   return get_OBSFRUSTUM_ptr(closest_OBSFRUSTUM_ID);
}

// ---------------------------------------------------------------------
// Member function instantiate_virtual_photo() generates a
// virtual OBSFRUSTUM located at the grid's origin and oriented in the
// canonical east direction.  The input horiz_FOV and vert_FOV
// parameters are measured in degrees.

photograph* OBSFRUSTAGROUP::instantiate_virtual_photo(
   double horiz_FOV,double vert_FOV,double frustum_sidelength,
   double blank_grey_level)
{
//   cout << "inside OBSFRUSTAGROUP::instantiate_virtual_photo()" << endl;
   
   double az=0;
   double el=0;
   double roll=0;
   threevector virtual_camera_posn=get_grid_world_origin();

   photogroup_ptr->destroy_all_photos();

   photograph* virtual_photo_ptr=photogroup_ptr->generate_blank_photograph(
      horiz_FOV,vert_FOV,az,el,roll,virtual_camera_posn,frustum_sidelength,
      blank_grey_level);
   virtual_camera_ptr=virtual_photo_ptr->get_camera_ptr();

   return virtual_photo_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_virtual_OBSFRUSTUM() takes in
// photograph *photograph_ptr which is assumed to be have previously
// been filled with valid extrinsic & extrinsic camera parameters.  It
// instantiates and returns an OBSFRUSTUM which may correspond to a
// blank photo.

OBSFRUSTUM* OBSFRUSTAGROUP::generate_virtual_OBSFRUSTUM(
   photograph* photograph_ptr)
{
//   cout << "inside OBSFRUSTAGROUP::generate_virtual_image_frustum()" << endl;
//   cout << "*photograph_ptr = " << *photograph_ptr << endl;

   threevector rotation_origin(0,0,0);
   threevector global_camera_translation(0,0,0);
   double global_daz=0*PI/180;
   double global_del=0*PI/180;
   double global_droll=0*PI/180;
   double movie_downrange_distance=-1;
   colorfunc::Color OBSFRUSTUM_color=colorfunc::white;
   bool thumbnails_flag=false;

   camera* camera_ptr=photograph_ptr->get_camera_ptr();

   destroy_OBSFRUSTUM(virtual_OBSFRUSTUM_ptr);

   virtual_OBSFRUSTUM_ptr=generate_still_image_frustum_for_photograph(
      photograph_ptr,camera_ptr->get_world_posn(),global_camera_translation,
      global_daz,global_del,global_droll,
      rotation_origin,photograph_ptr->get_frustum_sidelength(),
      movie_downrange_distance,OBSFRUSTUM_color,thumbnails_flag);

   Movie* Movie_ptr=virtual_OBSFRUSTUM_ptr->get_Movie_ptr();
   
   if (Movie_ptr != NULL)
   {
      Movie_ptr->get_texture_rectangle_ptr()->set_first_frame_to_display(0);
      Movie_ptr->get_texture_rectangle_ptr()->set_last_frame_to_display(0);
   }

   return virtual_OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
// Member function update_virtual_OBSFRUSTUM() repositions and
// reorients member *virtual_OBSFRUSTUM_ptr.  

OBSFRUSTUM* OBSFRUSTAGROUP::update_virtual_OBSFRUSTUM(
   const threevector& camera_world_posn,
   double az,double el,double roll,double frustum_sidelength)
{
//   cout << "inside OBSFRUSTAGROUP::update_virtual_OBSFRUSTUM() #1" << endl;
   
   colorfunc::Color virtual_OBSFRUSTUM_color=colorfunc::white;

   double movie_downrange_distance=-1;
   virtual_OBSFRUSTUM_ptr->build_OBSFRUSTUM(
      get_curr_t(),get_passnumber(),
      frustum_sidelength,movie_downrange_distance,
      camera_world_posn,az,el,roll,virtual_OBSFRUSTUM_color);
   return virtual_OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
OBSFRUSTUM* OBSFRUSTAGROUP::update_virtual_OBSFRUSTUM(
   double az,double el,double roll,double frustum_sidelength)
{
//   cout << "inside OBSFRUSTAGROUP::update_virtual_OBSFRUSTUM() #1" << endl;
//   cout << "Az = " << az*180/PI 
//        << " el = " << el*180/PI 
//        << " roll = " << roll*180/PI << endl;

   colorfunc::Color virtual_OBSFRUSTUM_color=colorfunc::white;

   double movie_downrange_distance=-1;
   virtual_OBSFRUSTUM_ptr->reset_OBSFRUSTUM(
      get_curr_t(),get_passnumber(),
      frustum_sidelength,movie_downrange_distance,
      az,el,roll,virtual_camera_ptr,virtual_OBSFRUSTUM_color);

   return virtual_OBSFRUSTUM_ptr;
}

// ---------------------------------------------------------------------
// Member function update_virtual_OBSFRUSTUM() repositions and
// reorients member *virtual_OBSFRUSTUM_ptr.  It also finds the
// physical OBSFRUSTUM whose pointing direction is closest to that of
// the updated virtual OBSFRUSTUM.  The physical image is
// backprojected onto a ground Z-plane and reprojected into the
// virtual OBSFRUSTUM.

void OBSFRUSTAGROUP::update_virtual_OBSFRUSTUM(
   const threevector& camera_world_posn,
   double az,double el,double roll,double frustum_sidelength,double z_ground)
{
//   cout << "inside OBSFRUSTAGROUP::update_virtual_OBSFRUSTUM() #2" << endl;
   
   colorfunc::Color virtual_OBSFRUSTUM_color=colorfunc::purple;

   double movie_downrange_distance=-1;
   virtual_OBSFRUSTUM_ptr->build_OBSFRUSTUM(
      get_curr_t(),get_passnumber(),
      frustum_sidelength,movie_downrange_distance,
      camera_world_posn,az,el,roll,virtual_OBSFRUSTUM_color);

//    OBSFRUSTUM* physical_OBSFRUSTUM_ptr=
      physical_OBSFRUSTUM_pointing_closest_to_virtual_OBSFRUSTUM(z_ground);

   int n_horiz_pixels=300;
   int n_vertical_pixels=200;
   string virtual_image_filename=photogroup_ptr->generate_blank_imagefile(
      n_horiz_pixels,n_vertical_pixels);

   export_virtual_camera_package(virtual_image_filename);
   virtual_frame_counter++;
}

// ---------------------------------------------------------------------
// Member function
// compute_virtual_OBSFRUSTA_for_circular_staring_orbit() takes in
// some ground target position (in UTM geocoordinates) along with the
// height above ground for a virtual camera.  It forms a circular
// orbit above the ground target's location and aims the virtual
// camera towards the ground target at each point in the orbit.
// Reprojected images from the virtual camera as it moves along the
// orbit are written to output PNG files.

void OBSFRUSTAGROUP::compute_virtual_OBSFRUSTA_for_circular_staring_orbit(
   const threevector& ground_target_posn,
   double virtual_camera_height_above_ground)
{
   cout << "inside OBSFRUSTAGROUP::compute_virtual_OBSFRUSTA_for_circular_staring_orbit()" << endl;

//   int n_steps=5;
   int n_steps=50;
//   int n_steps=360;

   double z_ground=ground_target_posn.get(2);
   double omega=2*PI/n_steps;

   for (int n=0; n<n_steps; n++)
   {
      double theta=omega*n;

      double rel_cam_z=virtual_camera_height_above_ground;
      double rel_cam_x=2*rel_cam_z*cos(theta);
      double rel_cam_y=2*rel_cam_z*sin(theta);
      threevector virtual_camera_posn=ground_target_posn+
         threevector(rel_cam_x,rel_cam_y,rel_cam_z);
   
      threevector k_hat=(ground_target_posn-virtual_camera_posn).unitvector();
//   cout << "k_hat = " << k_hat << endl;
      double az=atan2(k_hat.get(1),k_hat.get(0));
      double el=atan2(k_hat.get(2),sqrt(sqr(k_hat.get(0))+sqr(k_hat.get(1))));
      double roll=0;
      cout << "n = " << n << " theta = " << theta*180/PI
           << " az = " << az*180/PI
           << " el = " << el*180/PI << endl;

      double frustum_sidelength=100;	// meters
      update_virtual_OBSFRUSTUM(
         virtual_camera_posn,az,el,roll,frustum_sidelength,z_ground);

   } // loop over index n labeling steps along staring orbit
}

// ---------------------------------------------------------------------
// Member function reproject_physical_image_into_virtual_OBSFRUSTUM()
// warps input the imageplane contents for *physical_OBSFRUSTUM_ptr
// onto the imageplane for *virtual_OBSFRUSTUM_ptr.  It uses a joint
// homography to warp the former onto the latter via a ground Z-plane.
// The warped image is written to an output PNG file, and an
// associated package file for the virtual OBSFRUSTUM is also output.

string OBSFRUSTAGROUP::reproject_physical_image_into_virtual_OBSFRUSTUM(
   OBSFRUSTUM* physical_OBSFRUSTUM_ptr,double z_ground)
{
//   cout << "inside OBSFRUSTAGROUP::reproject_physical_image_into_virtual_OBSFRUSTUM()" << endl;
   
// Compute homographies from physical and virtual OBSFRUSTUM to ground
// z-plane:

   Movie* physical_Movie_ptr=physical_OBSFRUSTUM_ptr->get_Movie_ptr();
   camera* physical_camera_ptr=physical_Movie_ptr->get_camera_ptr();
   homography* physical_H_ptr=
      physical_camera_ptr->homography_from_imageplane_to_zplane(z_ground);

   Movie* virtual_Movie_ptr=virtual_OBSFRUSTUM_ptr->get_Movie_ptr();
   virtual_camera_ptr=virtual_Movie_ptr->get_camera_ptr();
   homography* virtual_H_ptr=
      virtual_camera_ptr->homography_from_imageplane_to_zplane(z_ground);

// Load hires version of physical image into *physical_Movie_ptr:

   string hires_photo_filename=physical_Movie_ptr->get_video_filename();
   bool twoD_flag=false;
   physical_Movie_ptr->reset_displayed_photograph(
      hires_photo_filename,twoD_flag);

// Loop over pixel rows & columns of virtual camera's image plane.
// Backproject physical camera's pixels onto ground Z-plane.  Then
// reproject ground pixels into virtual camera's image plane:

   for (unsigned int px=0; px<virtual_Movie_ptr->getWidth(); px++)
   {
      for (unsigned int py=0; py<virtual_Movie_ptr->getHeight(); py++)
      {
         double U,V,X,Y,u_lh,v_lh;
         virtual_Movie_ptr->get_uv_coords(px,py,U,V);
         virtual_H_ptr->project_world_plane_to_image_plane(U,V,X,Y);
         physical_H_ptr->project_image_plane_to_world_plane(X,Y,u_lh,v_lh);

         int R,G,B;
         if (u_lh < physical_Movie_ptr->get_minU() ||
             u_lh > physical_Movie_ptr->get_maxU() ||
             v_lh < physical_Movie_ptr->get_minV() ||
             v_lh > physical_Movie_ptr->get_maxV())
         {
            R=0;
            G=B=128;
         }
         else
         {
            unsigned int qx,qy;
            physical_Movie_ptr->get_pixel_coords(u_lh,v_lh,qx,qy);
            physical_Movie_ptr->get_pixel_RGB_values(qx,qy,R,G,B);

         }
         virtual_Movie_ptr->set_pixel_RGB_values(px,py,R,G,B);
      } // loop over py index
   } // loop over px index

// Export virtual frame to output file:

   string virtual_subdir="./virtual_frames/";
   string virtual_imagenumber_str=stringfunc::integer_to_string(
      virtual_frame_counter,3);
   string virtual_image_filename=virtual_subdir+
      "virtual_frame_"+virtual_imagenumber_str+".png";
   virtual_Movie_ptr->get_texture_rectangle_ptr()->write_curr_frame(
      virtual_image_filename);

   return virtual_image_filename;

}

// ---------------------------------------------------------------------
// Member function export_virtual_camera_package()

// associated package file for the virtual OBSFRUSTUM is also output.

void OBSFRUSTAGROUP::export_virtual_camera_package(
   string virtual_image_filename)
{
   cout << "inside OBSFRUSTAGROUP::export_virtual_camera_package()" 
        << endl;
   
// Compute homographies from physical and virtual OBSFRUSTUM to ground
// z-plane:

   Movie* virtual_Movie_ptr=virtual_OBSFRUSTUM_ptr->get_Movie_ptr();
   virtual_camera_ptr=virtual_Movie_ptr->get_camera_ptr();

// Write out package file for virtual camera:

   string packages_subdir="./packages/";
   string virtual_imagenumber_str=stringfunc::integer_to_string(
      virtual_frame_counter,3);
   string package_filename=packages_subdir+"virtual_frame_"+
      virtual_imagenumber_str+".pkg";
//   double frustum_sidelength=20;	// meters
   double frustum_sidelength=100;	// meters
   double downrange_distance=-1;
   virtual_camera_ptr->write_camera_package_file(
      package_filename,virtual_frame_counter,
      virtual_image_filename,frustum_sidelength,downrange_distance);
}


// ---------------------------------------------------------------------
// Member function virtual_tour_from_animation_path()

void OBSFRUSTAGROUP::virtual_tour_from_animation_path(
   string path_filename,double z_ground)
{
   cout << "inside OBSFRUSTAGROUP::virtual_tour_from_animation_path()" << endl;

   vector<threevector> virtual_camera_posn;
   vector<fourvector> virtual_camera_quat;
   filefunc::ReadInfile(path_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      threevector curr_posn(
         column_values[1],column_values[2],column_values[3]);
      virtual_camera_posn.push_back(curr_posn);
      fourvector curr_quat(
         column_values[4],column_values[5],column_values[6],column_values[7]);
      virtual_camera_quat.push_back(curr_quat);
   } // loop over index i 

   int n_steps=5;
//   int n_steps=20;
//   cout << "Enter number of steps along virtual aerial tour:" << endl;
//   cin >> n_steps;

   vector<threevector> ground_targets;
//   ground_targets.push_back(threevector(232937,3720430,1010));
//   ground_targets.push_back(threevector(233146,3720208,988));
//   ground_targets.push_back(threevector(233352,3720403,986));

   ground_targets.push_back(threevector(232943,3720413,1011));
   ground_targets.push_back(threevector(233022,3720332,982));
   ground_targets.push_back(threevector(233119,3720237,984));
   
   rotation R;
   camera curr_camera;
   for (int n=0; n<n_steps; n++)
   {
      int i=n*virtual_camera_posn.size()/double(n_steps);

      for (unsigned int g=0; g<ground_targets.size(); g++)
      {
         threevector k_hat=(ground_targets[g]-virtual_camera_posn[i]).
            unitvector();
//      threevector k_hat=(virtual_camera_posn[i+1]-virtual_camera_posn[i]).
//         unitvector();
         cout << "k_hat = " << k_hat << endl;
         double curr_az=atan2(k_hat.get(1),k_hat.get(0));
         double curr_el=atan2(
            k_hat.get(2),sqrt(sqr(k_hat.get(0))+sqr(k_hat.get(1))));
         double curr_roll=0;

/*
  fourvector curr_q=virtual_camera_quat[i];
  fourvector qtrans_OSG(
  curr_q.get(2),curr_q.get(0),curr_q.get(1),curr_q.get(3));
  rotation Rtrans=R.rotation_corresponding_to_OSG_quat(qtrans_OSG);

  double az,el,roll;
  Rtrans.az_el_roll_from_rotation(az,el,roll);

  double curr_az=az+90*osg::PI/180;
  double curr_el=roll-90*osg::PI/180;
  double curr_roll=el;
*/

         cout << "n = " << n 
              << " X = " << virtual_camera_posn[i].get(0)
              << " Y = " << virtual_camera_posn[i].get(1)
              << " Z = " << virtual_camera_posn[i].get(2) << endl;
         cout << " az = " << curr_az*180/PI
              << " el = " << curr_el*180/PI 
              << " roll = " << curr_roll*180/PI 
              << endl;

         double frustum_sidelength=100;	// meters
         update_virtual_OBSFRUSTUM(
            virtual_camera_posn[i],curr_az,curr_el,curr_roll,
            frustum_sidelength,z_ground);

      } // loop over index g labeling ground targets
   } // loop over index n labeling animation path steps
}


// ---------------------------------------------------------------------
// Member function adjust_frustum_angles() takes in small offsets in
// azimuth, elevation and roll angles.  It destroys the OBSFRUSTUM
// specified by the input ID and builds a new one with adjusted angles.

OBSFRUSTUM* OBSFRUSTAGROUP::adjust_frustum_angles(
   double d_az,double d_el,double d_roll)
{
//   int selected_OBSFRUSTUM_ID=0;
   int selected_OBSFRUSTUM_ID=get_selected_Graphical_ID();
   return adjust_frustum_angles(selected_OBSFRUSTUM_ID,d_az,d_el,d_roll);
}

OBSFRUSTUM* OBSFRUSTAGROUP::adjust_frustum_angles(
   int OBSFRUSTUM_ID,double d_az,double d_el,double d_roll)
{
//   cout << "inside OBSFRUSTAGROUP::adjust_frustum_angles()" << endl;
//   double frustum_sidelength=20;

   OBSFRUSTUM* OBSFRUSTUM_ptr=get_ID_labeled_OBSFRUSTUM_ptr(OBSFRUSTUM_ID);
   if (OBSFRUSTUM_ptr==NULL)
   {
      return NULL;
   }

   photograph* photo_ptr=get_OBSFRUSTUM_photograph_ptr(OBSFRUSTUM_ID);
   camera* camera_ptr=photo_ptr->get_camera_ptr();

   double az,el,roll;
   camera_ptr->get_az_el_roll_from_Rcamera(az,el,roll);
   az += d_az;
   el += d_el;
   roll += d_roll;

   cout << "Adjusted az = " << az*180/PI << endl;
   cout << "Adjusted el = " << el*180/PI << endl;
   cout << "Adjusted roll = " << roll*180/PI << endl;

   camera_ptr->set_rel_az(az);
   camera_ptr->set_rel_el(el);
   camera_ptr->set_rel_roll(roll);
   camera_ptr->set_Rcamera(az,el,roll);

   threevector global_camera_translation(0,0,0);
   double global_daz=0;
   double global_del=0;
   double global_droll=0;
   double local_spin_daz=0;
   threevector rotation_origin(0,0,0);

   double frustum_sidelength=photo_ptr->get_frustum_sidelength();
   double common_movie_downrange_distance=-1;
   colorfunc::Color OBSFRUSTUM_color=colorfunc::white;
   bool thumbnails_flag=false;

   Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
   destroy_OBSFRUSTUM(OBSFRUSTUM_ptr);

   OBSFRUSTUM_ptr=generate_still_image_frustum_for_photograph(
      photo_ptr,camera_ptr->get_world_posn(),
      global_camera_translation,
      global_daz,global_del,global_droll,
      rotation_origin,local_spin_daz,
      frustum_sidelength,common_movie_downrange_distance,
      OBSFRUSTUM_color,thumbnails_flag,Movie_ptr);

   OBSFRUSTUM_ptr->set_ID(OBSFRUSTUM_ID);

//   int n_anim_steps=0;
//   double t_flight=-1;
//   flyto_camera_location(OBSFRUSTUM_ID,n_anim_steps,t_flight);
   set_selected_Graphical_ID(OBSFRUSTUM_ID);

   return OBSFRUSTUM_ptr;
}
