// Note added on 9/30/05: We should explicitly destroy all the
// dynamically allocated features within this class' destructor !!!

// ==========================================================================
// FEATURESGROUP class member function definitions
// ==========================================================================
// Last modified on 6/19/14; 6/20/14; 6/21/14; 7/1/14
// ==========================================================================

#include <algorithm>
#include <iomanip>
#include <set>
#include <osg/Geode>
#include <osg/Group>
#include <osgText/Text>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/Center.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "astro_geo/Clock.h"
#include "datastructures/containerfuncs.h"
#include "color/colorfuncs.h"
#include "delaunay/Delaunay_tree.h"
#include "astro_geo/Ellipsoid_model.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "astro_geo/geopoint.h"
#include "geometry/homography.h"
#include "image/imagefuncs.h"
#include "osg/osgGraphicals/instantaneous_obs.h"
#include "kdtree/kdtreefuncs.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "datastructures/Linkedlist.h"
#include "osg/ModeController.h"
#include "osg/osg2D/Movie.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "video/photogroup.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "xml/table.h"
#include "time/timefuncs.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "datastructures/Triple.h"
#include "image/TwoDarray.h"
#include "video/VidFile.h"

#include "templates/mytemplates.h"

#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::setw;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void FeaturesGroup::allocate_member_objects()
{
   marked_feature_list_ptr=new Linkedlist<int>;
}		       

void FeaturesGroup::initialize_member_objects()
{
   GraphicalsGroup_name="FeaturesGroup";

   display_selected_bbox_flag=false;
   display_feature_pair_bboxes_flag=false;
   display_geocoords_flag=false;
   display_range_alt_flag=true;
   UV_image_flag=false;
   dragging_feature_flag=false;
   stabilize_imagery_flag=false;
   center_image_on_selected_feature_flag=false;
   centered_feature_number=-1;
   display_image_appearances_flag=false;
   display_feature_scores_flag=false;
   erase_unmarked_features_flag=false;
   prev_imagenumber=-1;
   min_unpropagated_feature_ID=-1;
   max_unpropagated_feature_ID=-1;
   bbox_sidelength=32;	// pixels

   minimum_ground_height=0;			// meters
   maximum_raytrace_range=100*1000;		// meters
   raytrace_stepsize=1.0;			// meters
   
   CentersGroup_ptr=NULL;
   fundamental_ptr=NULL;
   LineSegmentsGroup_ptr=NULL;
   EarthRegionsGroup_ptr=NULL;
   counterpart_FeaturesGroup_2D_ptr=NULL;
   FeaturesGroup_3D_ptr=NULL;
   montage_map_ptr=NULL;
   MoviesGroup_ptr=NULL;
   Movie_ptr=NULL;
   OBSFRUSTAGROUP_ptr=NULL;
   photogroup_ptr=NULL;
   PointFinder_ptr=NULL;
   PolyLinesGroup_ptr=NULL;
   SignPostsGroup_ptr=NULL;
   TrianglesGroup_ptr=NULL;

   kdtree_ptr=NULL;
   ntiepoints_matrix_ptr=NULL;
   DTED_ztwoDarray_ptr=NULL;
   image_matcher_ptr=NULL;
   features_map_ptr=NULL;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<FeaturesGroup>(
         this, &FeaturesGroup::update_display));
}		       

FeaturesGroup::FeaturesGroup(
   const int p_ndims,Pass* PI_ptr,
   osgGA::CustomManipulator* CM_ptr,threevector* GO_ptr):
   osgGeometry::PointsGroup(p_ndims,PI_ptr,GO_ptr),
	AnnotatorsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   CustomManipulator_refptr=CM_ptr;
}		       

FeaturesGroup::FeaturesGroup(
   const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
   Clock* clock_ptr,Ellipsoid_model* EM_ptr,threevector* GO_ptr):
   osgGeometry::PointsGroup(p_ndims,PI_ptr,clock_ptr,EM_ptr,GO_ptr),
   AnnotatorsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   CustomManipulator_refptr=CM_ptr;
}		       

FeaturesGroup::FeaturesGroup(
   const int p_ndims,Pass* PI_ptr,
   Movie* movie_ptr,osgGA::CustomManipulator* CM_ptr,threevector* GO_ptr):
   osgGeometry::PointsGroup(p_ndims,PI_ptr,GO_ptr),
   AnnotatorsGroup(p_ndims,PI_ptr)
{
   initialize_member_objects();
   allocate_member_objects();
   Movie_ptr=movie_ptr;
   CustomManipulator_refptr=CM_ptr;
}

FeaturesGroup::FeaturesGroup(
   const int p_ndims,Pass* PI_ptr,
   CentersGroup* CG_ptr,Movie* movie_ptr,
   osgGA::CustomManipulator* CM_ptr,AnimationController* AC_ptr,
   threevector* GO_ptr):
   osgGeometry::PointsGroup(p_ndims,PI_ptr,AC_ptr,GO_ptr),
   AnnotatorsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   Movie_ptr=movie_ptr;
   CentersGroup_ptr=CG_ptr;
   CustomManipulator_refptr=CM_ptr;
}		       

FeaturesGroup::FeaturesGroup(
   const int p_ndims,Pass* PI_ptr,
   CentersGroup* CG_ptr,Movie* movie_ptr,
   osgGA::CustomManipulator* CM_ptr,
   LineSegmentsGroup* LSG_ptr,threevector* GO_ptr):
   osgGeometry::PointsGroup(p_ndims,PI_ptr,GO_ptr),
   AnnotatorsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   Movie_ptr=movie_ptr;
   CentersGroup_ptr=CG_ptr;
   CustomManipulator_refptr=CM_ptr;
   LineSegmentsGroup_ptr=LSG_ptr;
}		       

FeaturesGroup::FeaturesGroup(
   const int p_ndims,Pass* PI_ptr,
   CentersGroup* CG_ptr,Movie* movie_ptr,
   osgGA::CustomManipulator* CM_ptr,TrianglesGroup* TG_ptr,
   LineSegmentsGroup* LSG_ptr,AnimationController* AC_ptr,
   threevector* GO_ptr):
   osgGeometry::PointsGroup(p_ndims,PI_ptr,AC_ptr,GO_ptr),
   AnnotatorsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   Movie_ptr=movie_ptr;
   CentersGroup_ptr=CG_ptr;
   CustomManipulator_refptr=CM_ptr;
   TrianglesGroup_ptr=TG_ptr;
   LineSegmentsGroup_ptr=LSG_ptr;
}		       

FeaturesGroup::FeaturesGroup(
   const int p_ndims,Pass* PI_ptr,
   osgGA::CustomManipulator* CM_ptr,TrianglesGroup* TG_ptr,
   threevector* GO_ptr):
   osgGeometry::PointsGroup(p_ndims,PI_ptr,GO_ptr),
   AnnotatorsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   CustomManipulator_refptr=CM_ptr;
   TrianglesGroup_ptr=TG_ptr;
}		       

FeaturesGroup::~FeaturesGroup()
{
   delete marked_feature_list_ptr;
   delete kdtree_ptr;
   delete ntiepoints_matrix_ptr;
   delete features_map_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const FeaturesGroup& f)
{
   outstream << (osgGeometry::PointsGroup&)f << endl;
/*
   int node_counter=0;
   for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
   {
      Feature* Feature_ptr=f.get_Feature_ptr(n);
      outstream << "feature node # " << node_counter++ << endl;
      outstream << "feature = " << *Feature_ptr << endl;
   }
*/
   return outstream;
}

// ==========================================================================
// Feature creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Feature from all other Graphical insertion
// and manipulation methods...

Feature* FeaturesGroup::generate_new_Feature(bool earth_feature_flag)
{
   int ID=-1;
   int OSGsubPAT_number=0;
   return generate_new_Feature(ID,OSGsubPAT_number,earth_feature_flag);
}

Feature* FeaturesGroup::generate_new_Feature(
   int ID,int OSGsubPAT_number,bool earth_feature_flag)
{
//    cout << "inside FeaturesGroup::generate_new_Feature()" << endl;
   if (ID==-1) ID=get_next_unused_ID();
//   cout << "ID = " << ID << endl;
   Feature* curr_Feature_ptr=new Feature(
      get_ndims(),ID,AnimationController_ptr);

   initialize_new_Feature(curr_Feature_ptr,OSGsubPAT_number);

   if (earth_feature_flag)
   {
      curr_Feature_ptr->set_UVW_dirs(
         get_curr_t(),get_passnumber(),
         Ellipsoid_model_ptr->get_east_ECI_hat(),
         Ellipsoid_model_ptr->get_north_ECI_hat());
   }

   osg::Geode* geode_ptr=curr_Feature_ptr->generate_drawable_geode(
      get_passnumber(),get_crosshairs_size(),get_crosshairs_text_size(),
      earth_feature_flag);
   curr_Feature_ptr->get_PAT_ptr()->addChild(geode_ptr);

   reset_colors();

   return curr_Feature_ptr;
}

// ---------------------------------------------------------------------
// This stripped-down variant of generate_new_Feature() can be called
// from within accumulate_feature_info() when crosshairs do not need
// to be instantiated and displayed within an output OSG viewer.

Feature* FeaturesGroup::minimal_generate_new_Feature(int ID)
{
//   cout << "inside FeaturesGroup::minimal_generate_new_Feature()" << endl;
   if (ID==-1) ID=get_next_unused_ID();
//   cout << "ID = " << ID << endl;
   Feature* curr_feature_ptr=new Feature(
      get_ndims(),ID,AnimationController_ptr);
   GraphicalsGroup::insert_Graphical_into_list(curr_feature_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_feature_ptr,0);

   return curr_feature_ptr;
}

// ---------------------------------------------------------------------
void FeaturesGroup::initialize_new_Feature(
   Feature* curr_Feature_ptr,int OSGsubPAT_number)
{
//   cout << "inside FeaturesGroup::initialize_new_Feature()" << endl;

   GraphicalsGroup::insert_Graphical_into_list(curr_Feature_ptr);

   initialize_Graphical(curr_Feature_ptr);

   insert_graphical_PAT_into_OSGsubPAT(curr_Feature_ptr,OSGsubPAT_number);
}

// --------------------------------------------------------------------------
// Member function edit_feature_label allows the user to change the ID
// number associated with a feature.  The new ID number must not
// conflict with any other existing feature's ID.  It must also be
// non-negative.  The user enters the replacement ID for a selected
// feature within the main console window.  (As of 7/10/05, we are
// unfortunately unable to robustly retrieve user input from the
// feature text dialog window...)

void FeaturesGroup::edit_feature_label()
{   
   osgGeometry::PointsGroup::edit_label();
}

// --------------------------------------------------------------------------
// Member function erase_feature sets boolean entries within the
// member map coords_erased to true for the current feature.  When
// feature crosshairs are drawn within
// FeaturesGroup::reassign_PAT_ptrs(), entries within this STL map are
// first checked and their positions are set to large negative values
// to prevent them from appearing within the OSG data window.  Yet the
// feature itself continues to exist.

bool FeaturesGroup::erase_feature()
{   
//   cout << "inside FG::erase_feature()" << endl;
   bool feature_erased=erase_point();
   return feature_erased;
}

// --------------------------------------------------------------------------
// Member function unerase_feature queries the user to enter the ID
// for some erased feature.  It then unerases that feature within the
// current image.

bool FeaturesGroup::unerase_feature()
{   
//   cout << "inside FG::unerase_feature()" << endl;
   bool feature_unerased_flag=unerase_point();
   return feature_unerased_flag;
}

// --------------------------------------------------------------------------
// Method mark_feature stores input curr_ID within member
// *marked_feature_list_ptr if it does not already exist within the
// linked list of integers.  If curr_ID already resides in the list,
// it is deleted from the list.  This toggling allows for the user to
// click on/off features which he wants to mark for some special
// purpose.

void FeaturesGroup::mark_feature(int curr_ID)
{   
   if (curr_ID >= 0)
   {
      Mynode<int>* currnode_ptr=marked_feature_list_ptr->data_in_list(
         curr_ID);
      if (currnode_ptr==NULL) 
      {
         marked_feature_list_ptr->append_node(curr_ID);
      }
      else
      {
         marked_feature_list_ptr->delete_node(currnode_ptr);
      }
   } // curr_ID >= 0 conditional
      
   cout << "Number of marked features = " 
        << marked_feature_list_ptr->size() << endl;
//   cout << *marked_feature_list_ptr << endl;
}

// --------------------------------------------------------------------------
// Member function destroy_feature removes the selected feature from
// the featurelist and the OSG features group.  If the feature is
// successfully destroyed, its number is returned by this method.
// Otherwise, -1 is returned.

int FeaturesGroup::destroy_feature(bool update_text_flag)
{   
//   cout << "inside FG::destroy_feature()" << endl;
   int destroyed_feature_number=destroy_Point();
//   cout << 
//      "Before returning from FG::destroy_feature, destroyed_feature_number = "
//        << destroyed_feature_number << endl;
   return destroyed_feature_number;
}

bool FeaturesGroup::destroy_feature(int Feature_ID)
{   
//   cout << "inside FG::destroy_feature()" << endl;
   return destroy_Point(Feature_ID);
}

// --------------------------------------------------------------------------
// Member function destroy_all_Features() first fills an STL vector
// with Feature IDs.  It then iterates over each vector entry and
// calls destroy_Feature for each Feature.  On 5/3/08, we learned the
// hard and painful way that this two-step process is necessary in
// order to correctly purge all Features.

void FeaturesGroup::destroy_all_Features()
{   
//   cout << "inside FeaturesGroup::destroy_all_Features()" << endl;
   unsigned int n_Features=get_n_Graphicals();
//   cout << "n_Features = " << n_Features << endl;

   vector<int> Feature_IDs;
   for (unsigned int p=0; p<n_Features; p++)
   {
      Feature* Feature_ptr=get_Feature_ptr(p);
      Feature_IDs.push_back(Feature_ptr->get_ID());
   }

   for (unsigned int p=0; p<n_Features; p++)
   {
      destroy_feature(Feature_IDs[p]);
   }
}

// --------------------------------------------------------------------------
// Member function change_size multiplies the size parameter for cross
// hair objects corresponding to the current dimension by input
// parameter factor.

void FeaturesGroup::change_size(double factor)
{   
   cout << "inside FeaturesGroup::change_size(), factor = " << factor
        << endl;
   
   osgGeometry::PointsGroup::change_size(factor);

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Feature* Feature_ptr=get_Feature_ptr(n);
      Feature_ptr->set_nimages_appearance_text_posn(
         get_crosshairs_text_size());
   }
}

// --------------------------------------------------------------------------
// Member function move_z moves 3D features up or down in world-space
// z.  This specialized method was constructed to facilitate movement
// of 3D cursors in the z direction which is highly special for ALIRT
// imagery.

void FeaturesGroup::move_z(int sgn)
{
   osgGeometry::PointsGroup::move_z(sgn);
}

// --------------------------------------------------------------------------
// Member function update_display should be repeatedly executed by a
// callback in a main program.

void FeaturesGroup::update_display()
{   
//   cout << "inside FeaturesGroup::update_display()" << endl;
//   cout << "this = " << this << endl;
//   cout << "get_curr_t() = " << get_curr_t() << endl;

// Only update feature text box if current image number has changed:

   if (get_curr_framenumber() != prev_imagenumber)
   {
      prev_imagenumber=get_curr_framenumber();
   }

   if (get_ndims()==3 && PointFinder_ptr != NULL)
   {
      retrieve_3D_feature_coords();
   }

// Call reset_colors to enable Feature blinking:

   reset_colors();

   if (photogroup_ptr==NULL)
   {
      backproject_2D_features_into_3D();
   }
   else
   {

// FAKE FAKE:  Mon Mar 4, 2013
// comment out 2nd line in favor of 1st line for GEO display purposes

      backproject_selected_photo_features_as_3D_rays();
//      backproject_selected_photo_features_into_3D();
//      display_3D_features_in_selected_photo();
   }

   if (stabilize_imagery_flag)
   {
      threevector UVW;
      Center* Center_ptr=CentersGroup_ptr->get_Center_ptr();
      if (Center_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW))
      {
         if (get_ndims()==2)
         {
            get_CustomManipulator_ptr()->set_worldspace_center(
               threevector(UVW.get(0),0,UVW.get(1)));
         }
      }
   }

   if (display_feature_scores_flag) update_feature_scores();
   
// Recenter imagery upon current selected feature if boolean flag==true:

   if (center_image_on_selected_feature_flag && !stabilize_imagery_flag)
   {
      Feature* selected_Feature_ptr=get_ID_labeled_Feature_ptr(
         centered_feature_number);
      
      threevector UVW;
      if (selected_Feature_ptr != NULL && 
          selected_Feature_ptr->get_UVW_coords(
             get_curr_t(),get_passnumber(),UVW))
      {
         if (get_ndims()==2)
         {
            get_CustomManipulator_ptr()->set_worldspace_center(
               threevector(UVW.get(0),0,UVW.get(1)));
         }
         else if (get_ndims()==3)
         {
            get_CustomManipulator_ptr()->set_worldspace_center(UVW);
         }
      } // get_UVW_coords conditional
   }  // center_image_on_selected_feature_flag conditional

   if (fundamental_ptr != NULL)
   {
      update_epipolar_lines();
   }

   if (get_ndims()==2 && counterpart_FeaturesGroup_2D_ptr != NULL && 
   ntiepoints_matrix_ptr != NULL && image_matcher_ptr != NULL)
   {
      update_feature_tiepoints();
   }

   if (montage_map_ptr != NULL)
      display_montage_corresponding_to_selected_3D_feature_point();
   
   if (display_selected_bbox_flag)
   {
      display_selected_feature_bbox();
   }
   else if (display_feature_pair_bboxes_flag)
   {
      display_feature_pair_bboxes();
   }

   GraphicalsGroup::update_display();
}

// --------------------------------------------------------------------------
// Member function update_feature_tiepoints() checks if the number of
// features for *this is less than the number in
// *counterpart_FeaturesGroup_2D_ptr.  If so, it estimates tiepoint
// locations within *this for all features in
// *counterpart_FeaturesGroup_2D_ptr which don't currently have
// correspondences.  

void FeaturesGroup::update_feature_tiepoints()
{
//   cout << "inside FeaturesGroup::match_feature_tiepoints()" << endl;

   unsigned int n_features=get_n_Graphicals();
   unsigned int n_counterpart_features=counterpart_FeaturesGroup_2D_ptr->
      get_n_Graphicals();
   if (n_features >= n_counterpart_features) return;


//   cout << "n_features = " << n_features << endl;
//   cout << "n_counterpart_features = " << n_counterpart_features << endl;

// Loop over all features in *counterpart_FeaturesGroup_2D_ptr and
// find those which do not have tiepoint correspondences with features
// in *this:

   threevector counterpart_UVW;
   for (unsigned int i=0; i<n_counterpart_features; i++)
   {
      Feature* counterpart_Feature_ptr=counterpart_FeaturesGroup_2D_ptr->
         get_Feature_ptr(i);
      int counterpart_ID=counterpart_Feature_ptr->get_ID();
      counterpart_Feature_ptr->get_UVW_coords(
         counterpart_FeaturesGroup_2D_ptr->get_curr_t(),
         counterpart_FeaturesGroup_2D_ptr->get_passnumber(),counterpart_UVW);

      Feature* Feature_ptr=get_ID_labeled_Feature_ptr(counterpart_ID);
      if (Feature_ptr != NULL) continue;

      Feature_ptr=generate_new_Feature(counterpart_ID);
      const osg::Quat trivial_q(0,0,0,1);
      const threevector trivial_scale(1,1,1);
      Feature_ptr->set_quaternion(get_curr_t(),get_passnumber(),trivial_q);
      Feature_ptr->set_scale(get_curr_t(),get_passnumber(),trivial_scale);

      if (get_passnumber()==0)
      {
         cout << "UV feature w/ ID = " << counterpart_ID 
              << " has no tiepoint" << endl;
         twovector UV(counterpart_UVW);
         cout << "UV = " << UV << endl;
         cout << "image_matcher_ptr = " << image_matcher_ptr << endl;
         twovector XY=image_matcher_ptr->find_XY_matching_UV(UV);
         cout << "XY = " << XY << endl;

         Feature_ptr->set_UVW_coords(
            get_curr_t(),get_passnumber(),threevector(XY));
      }
      else if (get_passnumber()==1)
      {
         cout << "XY feature w/_ID = " << counterpart_ID 
              << " has no tiepoint" << endl;
         twovector XY(counterpart_UVW);
         cout << "XY = " << XY << endl;
         twovector UV=image_matcher_ptr->find_UV_matching_XY(XY);
         cout << "UV = " << UV << endl;
         Feature_ptr->set_UVW_coords(
            get_curr_t(),get_passnumber(),threevector(UV));
      }

      set_selected_Graphical_ID(counterpart_ID);
      counterpart_FeaturesGroup_2D_ptr->set_selected_Graphical_ID(
         counterpart_ID);
  
   } // loop over index i labeling counterpart Features
}

// ==========================================================================
// Instantaneous observation manipulation member functions
// ==========================================================================

// Member function consolidate_feature_coords takes in times curr_t
// and other_t corresponding to FeaturesGroups *this and
// *other_FeaturesGroup_ptr.  Looping over each feature within *this,
// it looks for a counterpart feature within *other_FeaturesGroup_ptr
// with the same ID.  If a counterpart feature is found, its
// coordinates are pushed onto the current feature's observation list.

void FeaturesGroup::consolidate_feature_coords(
   double curr_t,double other_t,FeaturesGroup* other_FeaturesGroup_ptr)
{
   
// Loop over this FeaturesGroup's features:

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Feature* Feature_ptr=get_Feature_ptr(n);
      instantaneous_obs* curr_obs_ptr=Feature_ptr->
         get_particular_time_obs(curr_t,get_passnumber());
      if (curr_obs_ptr != NULL)
      {
         if (!Feature_ptr->get_mask(curr_t,get_passnumber()))
         {

// Search for counterpart feature information corresponding to ID:

            int ID=Feature_ptr->get_ID();
            Feature* other_Feature_ptr=other_FeaturesGroup_ptr->
               get_ID_labeled_Feature_ptr(ID);
            if (other_Feature_ptr != NULL)
            {
               threevector q;
               if (other_Feature_ptr->get_UVW_coords(
                      other_t,other_FeaturesGroup_ptr->get_passnumber(),q))
               {
                  curr_obs_ptr->insert_UVW_coords(
                     other_FeaturesGroup_ptr->get_passnumber(),q);

                  cout << "curr_t = " << curr_t
                       << " other_t = " << other_t
                       << " feature ID = " << ID 
                       << " rel XYZ = " << q << endl;
                  cout << "*curr_obs_ptr = " << *curr_obs_ptr << endl;
               }
            } // other_Feature_ptr != NULL conditional
         } // !get_mask_flag conditional
      } // curr_obs_ptr != NULL conditional

   } // loop over index n labeling this FeaturesGroup features
}

// ==========================================================================
// Photograph feature member functions
// ==========================================================================

// High-level member function read_in_photo_features() reads in
// feature files for input photos within *photogroup_ptr.  It then
// fills a features map with pass number, U, V as a function of
// feature ID.  Feature coordinate measurements are accumulated inside
// *this.  Rows correspond to distinct feature IDs, while columns
// correspond to individual passes.  This method returns the number of
// feature tracks found.

int FeaturesGroup::read_in_photo_features(
   photogroup* photogroup_ptr,string subdir,
   bool display_OSG_features_flag,bool hide_singleton_features_flag)
{
//   cout << "inside FeaturesGroup::read_in_photo_features()" << endl;
//   cout << "hide_singleton_features_flag = "
//        << hide_singleton_features_flag << endl;
   
   string banner="Reading in photo features";
   outputfunc::write_banner(banner);

   features_map_ptr=new FEATURES_MAP();
   generate_features_map_for_photos(
      features_map_ptr,photogroup_ptr,subdir);
   int n_feature_tracks=accumulate_feature_info(
      features_map_ptr,display_OSG_features_flag,hide_singleton_features_flag);
   cout << "Number of imported feature tracks = " 
        << n_feature_tracks << endl;

   return n_feature_tracks;
}

// --------------------------------------------------------------------------
// Member function generate_features_map_for_photos reads in 2D
// feature coordinates for all photos contained inside input
// *photogroup_ptr.  It parses this information into input
// FEATURES_MAP features_map for subsequent processing.  As of Jan
// 2013, we store feature index as the last element within fourvector
// (pass_number,U,V,feature_index).

void FeaturesGroup::generate_features_map_for_photos(
   FEATURES_MAP* features_map_ptr,photogroup* photogroup_ptr,string subdir)
{
//   cout << "inside FeaturesGroup::generate_features_map_for_photos()" << endl;
   unsigned int n_photos=photogroup_ptr->get_n_photos();
   unsigned int n_start=photogroup_ptr->get_first_node_ID();

   for (unsigned int p=n_start; p<n_start+n_photos; p++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(p);
      int pass_number=n_start+photograph_ptr->get_ID();
      string basename=stringfunc::prefix(filefunc::getbasename(
         photograph_ptr->get_filename()));

      string features_filename=subdir+"features_2D_"+basename+".txt";
//      cout << "p = " << p 
//           << " features_filename = " << features_filename << endl;
      filefunc::ReadInfile(features_filename);

      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<double> columns=stringfunc::string_to_numbers(
            filefunc::text_line[i]);
         int feature_ID=columns[1];
//         int pass_number=columns[2]; // Pass number set by order of
//         photos into PANORAMA rather than by photo order input for TIEPOINTS
         double U=columns[3];
         double V=columns[4];
         double feature_index=-1;
         if (columns.size() >= 7)
         {
            feature_index=columns[6];
         }

         fourvector feature_coords(pass_number,U,V,feature_index);
         FEATURES_MAP::iterator feature_iter=features_map_ptr->find(
            feature_ID);
         if (feature_iter != features_map_ptr->end())
         {
            feature_iter->second.push_back(feature_coords);
         }
         else
         {
            vector<fourvector> V;
            V.push_back(feature_coords);
            (*features_map_ptr)[ feature_ID ] = V;
         }

/*
         feature_iter=features_map_ptr->find(feature_ID);
         vector<fourvector> feature_vec=feature_iter->second;
         if (feature_vec.size() > 1)
         {
            cout << "feature ID = " << feature_iter->first << endl;
            for (unsigned int v=0; v<feature_vec.size(); v++)
            {
               cout << "pass = " << feature_vec[v].get(0)
                    << " U = " << feature_vec[v].get(1)
                    << " V = " << feature_vec[v].get(2) 
                    << " index = " << feature_vec[v].get(3)
                    << "  ";
            }
            cout << endl;
         }
*/
          
      } // loop over index i labeling feature file text line

   } // loop over index p labeling photos
}

// --------------------------------------------------------------------------
// Member function accumulate_feature_info takes in *features_map_ptr
// containing feature ID keys and an STL vector of fourvectors ( =
// pass_number, U, V, feature_index for current pass).  Iterating over
// over all FEATURES_MAP entries, this method instantiates features
// and loads their multi-pass UV coordinates.  This method runs
// *much* faster than deprecated member
// consolidate_instantaneous_observations().  It also returns the
// number of feature tracks containing at least 2 sets of coordinates
// for the same world-space feature.

int FeaturesGroup::accumulate_feature_info(
   FEATURES_MAP* features_map_ptr,bool display_OSG_features_flag,
   bool hide_singleton_features_flag,int individual_pass_number)
{
//   cout << "inside FeaturesGroup::accumulate_feature_info()" << endl;
//   cout << "display_OSG_features_flag = " 
//        << display_OSG_features_flag << endl;
//   cout << "hide_singleton_features_flag = "
//        << hide_singleton_features_flag << endl;
   outputfunc::write_banner("Accumulating features from input map:");

   int counter=0;
   int n_feature_tracks=0;
   for (FEATURES_MAP::const_iterator iter=features_map_ptr->begin();
        iter != features_map_ptr->end(); iter++)
   {
      if (counter%1000==0) cout << counter/1000 << " " << flush;
      counter++;

      int feature_ID=iter->first;
      Feature* Feature_ptr=NULL;
      
      if (display_OSG_features_flag)
      {
         Feature_ptr=generate_new_Feature(feature_ID);
      }
      else
      {
         Feature_ptr=minimal_generate_new_Feature(feature_ID);
      }

// Initially erase/hide every new feature:

      double curr_time=0;
      Feature_ptr->set_mask(curr_time,get_passnumber(),true);

      vector<fourvector> feature_triples=iter->second;
      if (feature_triples.size() > 1) n_feature_tracks++;

      for (unsigned int j=0; j<feature_triples.size(); j++)
      {
         int pass_number=feature_triples[j].get(0);

         if (individual_pass_number >= 0)
         {
            if (pass_number != individual_pass_number) continue;
         }
         
         double U=feature_triples[j].get(1);
         double V=feature_triples[j].get(2);
         int feature_index=feature_triples[j].get(3);

         if (feature_index < 0)
         {
            cout << "Feature ID = " << feature_ID 
                 << " pass# = " << pass_number
                 << " U = " << U << " V = " << V 
                 << " index = " << feature_index << endl;
            outputfunc::enter_continue_char();
         }
         
// Load time, imagenumber, passnumber and UVW coordinate information
// into current feature.  Set manually_manipulated flag to true and
// coords_erased flag to false for each STL vector entry:

         const osg::Quat trivial_q(0,0,0,1);
         const threevector trivial_scale(1,1,1);

         Feature_ptr->set_UVW_coords(curr_time,pass_number,threevector(U,V));
         Feature_ptr->set_quaternion(curr_time,pass_number,trivial_q);
         Feature_ptr->set_scale(curr_time,pass_number,trivial_scale);
         Feature_ptr->set_index(curr_time,pass_number,feature_index);

         int index;
         Feature_ptr->get_index(curr_time,pass_number,index);
         if (index < 0)
         {
            cout << "index = " << index << endl;
            outputfunc::enter_continue_char();
         }
         

// Unerase/unhide feature if hide_singleton_features_flag==true and
// current feature belongs to a nontrivial track or if
// hide_singleton_features_flag==false:

         if ( (feature_triples.size() > 1 && hide_singleton_features_flag)
         || !hide_singleton_features_flag)
         {
            Feature_ptr->set_mask(curr_time,pass_number,false);
         }
         else
         {
            Feature_ptr->set_mask(curr_time,pass_number,true);
         }
         
      } // loop over index j labeling feature triples
   } // loop over *features_map_ptr iterator
   outputfunc::newline();

   cout << "Number of features read in from features_map = " 
        << get_n_Graphicals() << endl;
   min_unpropagated_feature_ID=minimum_point_ID();
   cout << "Minimum feature ID = " << min_unpropagated_feature_ID << endl;
   max_unpropagated_feature_ID=maximum_point_ID();
   cout << "Maximum feature ID = " << max_unpropagated_feature_ID << endl;
   cout << "n_feature_tracks = " << n_feature_tracks << endl;
//   cout << "features_map_ptr->size() = " << features_map_ptr->size() << endl;

   return n_feature_tracks;
}

// -------------------------------------------------------------------------
// Member function reorder_passes_to_maximize_tiepoint_overlap

void FeaturesGroup::reorder_passes_to_maximize_tiepoint_overlap(
   double t,photogroup* photogroup_ptr,bool temporal_ordering_flag)
{
   cout << "inside FeaturesGroup::reorder_passes_to_maximize_tiepoint_overlap()"
        << endl;

   count_tiepoints(t,photogroup_ptr);
   compute_pass_compositing_order(t,photogroup_ptr,temporal_ordering_flag);
}

// -------------------------------------------------------------------------
// Member function count_tiepoints loops over all pass pairs and
// counts numbers of feature tiepoints for a specified input time.  It
// returns its results within symmetric genmatrix member
// *ntiepoints_matrix_ptr.

genmatrix* FeaturesGroup::count_tiepoints(double t,photogroup* photogroup_ptr)
{
   cout << "inside FeaturesGroup::count_tiepoints()" << endl;
   string banner="Counting numbers of feature tiepoints:";
   outputfunc::write_banner(banner);

   unsigned int n_photos=photogroup_ptr->get_n_photos();
   ntiepoints_matrix_ptr=new genmatrix(n_photos,n_photos);
   ntiepoints_matrix_ptr->clear_values();

   for (unsigned int f=0; f<get_n_Graphicals(); f++)
   {
      Feature* Feature_ptr=get_Feature_ptr(f);
   
      for (unsigned int i=0; i<n_photos; i++)
      {

// First check whether *Feature_ptr exists within photo indexed by integer i:

         if (!Feature_ptr->particular_time_obs_exists(
                t,photogroup_ptr->get_photograph_ptr(i)->get_ID()))
         {
            continue;
         }

         ntiepoints_matrix_ptr->put(i,i,ntiepoints_matrix_ptr->get(i,i)+1);
             
         for (unsigned int j=i+1; j<n_photos; j++)
         {
            if (Feature_ptr->particular_time_obs_exists(
                   t,photogroup_ptr->get_photograph_ptr(j)->get_ID()))
            {
               ntiepoints_matrix_ptr->put(
                  i,j,ntiepoints_matrix_ptr->get(i,j)+1);
               ntiepoints_matrix_ptr->put(
                  j,i,ntiepoints_matrix_ptr->get(i,j));
            }
//         cout << "i = " << i << " photo.ID = " << photographs[i].get_ID() 
//              << " photo name = " << photographs[i].get_filename() 
//              << endl;
//         cout << "j = " << i << " photo.ID = " << photographs[j].get_ID() 
//              << " photo name = " << photographs[j].get_filename() 
//              << endl;
         } // loop over index j
         
      } // loop over index i labeling photos
   } // loop over index f labeling features

//   cout << "*ntiepoints_matrix_ptr = " << *ntiepoints_matrix_ptr << endl;
//   outputfunc::enter_continue_char();

   return ntiepoints_matrix_ptr;
}

// -------------------------------------------------------------------------
// Member function compute_pass_compositing_order determines the order
// in which input passes should be composited in order to maximize
// their feature tiepoint overlap.  Using the tiepoint count results
// stored in *ntiepoints_matrix_ptr, it first identifies the pair of
// passes which have the greatest tiepoint overlap.  It then
// sequentially adds more passes so that at each stage the total
// tiepoint count is maximal.  This method returns the order in which
// each pass should be composited within STL vector
// photogroup_ptr->get_photo_order().

void FeaturesGroup::compute_pass_compositing_order(
   double t,photogroup* photogroup_ptr,bool temporal_ordering_flag)
{
   string banner="Computing pass compositing order:";
   outputfunc::write_banner(banner);

// Purge any previous contents within STL vector
// photogroup_ptr->get_photo_order():

   photogroup_ptr->get_photo_order().clear();

   unsigned int n_photos=photogroup_ptr->get_n_photos();

   if (n_photos==1)
   {
      photogroup_ptr->get_photo_order().push_back(
         photogroup_ptr->get_photograph_ptr(0)->get_ID());
      return;
   }
   else if (temporal_ordering_flag)
   {
      for (unsigned int n=0; n<n_photos; n++)
      {
         photogroup_ptr->get_photo_order().push_back(
            photogroup_ptr->get_photograph_ptr(n)->get_ID());
      }
      photogroup_ptr->print_compositing_order();
      return;
   }

// First find pass pair with largest number of tiepoints:

   int imax,jmax;
   imax=jmax=-1;
   int max_n_tiepoints=0;
   for (unsigned int i=0; i<n_photos; i++)
   {
      for (unsigned int j=i+1; j<n_photos; j++)
      {
         int curr_n_tiepoints=ntiepoints_matrix_ptr->get(i,j);
//         cout << "i = " << i << " j = " << j << endl;
//         cout << "curr_n_tiepoints = " << curr_n_tiepoints << endl;
         if (curr_n_tiepoints > max_n_tiepoints)
         {
            max_n_tiepoints=curr_n_tiepoints;
            imax=i;
            jmax=j;
         }
      } // loop over index j
   } // loop over index i
   
   cout << "max_n_tiepoints = " << max_n_tiepoints
//        << " imax = " << imax 
//        << " jmax = " << jmax 
        << endl;
   
   map<int,int> tiepoint_map;
   tiepoint_map[imax]=0;
   tiepoint_map[jmax]=0;
   
   photogroup_ptr->get_photo_order().push_back(
      photogroup_ptr->get_photograph_ptr(imax)->get_ID());
   photogroup_ptr->get_photo_order().push_back(
      photogroup_ptr->get_photograph_ptr(jmax)->get_ID());

// Loop over remaining passes.  Sequentially add into tiepoint_map the
// pass whose pair-wise overlap with the existing entries in
// tiepoint_map is greatest:

   for (unsigned int pass_counter=2; pass_counter < n_photos; pass_counter++)
   {
      int kmax=-1;
      int max_tiepoint_sum=0;
      for (unsigned int k=0; k<n_photos; k++)
      {
//         cout << "k = " << k << endl;
     
// Skip over any passes which already exist within tiepoint_map:

         map<int,int>::iterator iter=tiepoint_map.find(k);      
         if (iter != tiepoint_map.end()) 
         {
//            cout << "Pass already included in tiepoint_map" << endl;
            continue;
         }
      
// Iterate over all passes already within tiepoint_map.  Sum their
// tiepoint counts with candidate pass k:

         int tiepoint_sum=0;
         for (iter=tiepoint_map.begin(); iter != tiepoint_map.end(); iter++)
         {
            tiepoint_sum += ntiepoints_matrix_ptr->get(k,iter->first);
         }

         if (tiepoint_sum > max_tiepoint_sum)
         {
            max_tiepoint_sum=tiepoint_sum;
            kmax=k;
         }
      } // loop over index k labeling current pass to be added to composite
      
      int curr_photo_ID=photogroup_ptr->get_photograph_ptr(kmax)->get_ID();

//      cout << "pass_counter = " << pass_counter 
//           << " kmax = " << kmax
//           << " ID = " << curr_photo_ID
//           << " max_tiepoint_sum = " << max_tiepoint_sum << endl;

// Add candidate pass k with maximal tiepoint_sum into tiepoint_map:
   
      tiepoint_map[kmax]=pass_counter-1;
      photogroup_ptr->get_photo_order().push_back(curr_photo_ID);
   } // loop over pass_counter

   photogroup_ptr->print_compositing_order();
}

// -------------------------------------------------------------------------
// Member function photo_feature_overlap() returns true if the number
// of tiepoints between the two photos labeled by input indices i and
// j exceeds some minimum threshold.  This boolean method returns
// false if i==j.

vector<int> FeaturesGroup::photo_feature_overlap(
   unsigned int p, unsigned int q_start, unsigned int q_stop, 
   int n_qvalues)
{
//   cout << "inside FeaturesGroup::photo_feature_overlap(), p = " << p
//        <<  endl;

   const int min_tiepoint_pairs=20;

   vector<unsigned int> q_values;
   vector<unsigned int> n_tiepoints;
   for (unsigned int q=q_start; q<=q_stop; q++)
   {
      if (q != p)
      {
         int curr_n_tiepoints=ntiepoints_matrix_ptr->get(p,q);
         if (curr_n_tiepoints > min_tiepoint_pairs)
         {
            q_values.push_back(q);
            n_tiepoints.push_back(curr_n_tiepoints);
         }
      } // q != p conditional
   } // loop over q index labeling photos overlapping with p photo
   templatefunc::Quicksort(n_tiepoints,q_values);

// Recall input argument n_qvalues represents an upper bound on actual
// number of photos indexed by q within interval [q_start,q_stop]
// which have nontrivial feature overlap with the photo index by p:

   n_qvalues=basic_math::min(n_qvalues,int(q_values.size()));

   vector<int> overlapping_q_photo_indices;
   for (int i=0; i<n_qvalues; i++)
   {
      overlapping_q_photo_indices.push_back(q_values[i]);
//      cout << "i =  " << i << " overlapping_q_photo_indices = "
//           << overlapping_q_photo_indices[i] << endl;
   }

   return overlapping_q_photo_indices;
}

bool FeaturesGroup::photo_feature_overlap(int i, int j)
{
//   cout << "inside FeaturesGroup::photo_feature_overlap(), i = " << i
//        << " j = " << j << endl;
   const int min_tiepoint_pairs=20;
   
   if (i==j) return false;
//   cout << "ntiepoints_matrix_ptr->get(i,j) = "
//        << ntiepoints_matrix_ptr->get(i,j) << endl;
   
   return (ntiepoints_matrix_ptr->get(i,j) > min_tiepoint_pairs);
}

// -------------------------------------------------------------------------
// Member function convert_2D_coords_to_3D_rays() takes in
// *photogroup_ptr which is assumed to contain calibrated cameras for
// each of its photo members.  For each photograph, this method loops
// over the 2D feature coordinates within *this and calculates
// corresponding 3D ray directions.  

void FeaturesGroup::convert_2D_coords_to_3D_rays(photogroup* photogroup_ptr)
{
   for (unsigned int i=0; i<photogroup_ptr->get_n_photos(); i++)
   {
      convert_2D_coords_to_3D_rays(photogroup_ptr->get_photograph_ptr(i));
   }
}

// -------------------------------------------------------------------------
void FeaturesGroup::convert_2D_coords_to_3D_rays(photograph* photograph_ptr)
{
//   cout << "inside FeaturesGroup::convert_2D_corods_to_3D_rays()" << endl;

   int max_n_passes=get_max_n_passes(get_curr_t());
   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   for (unsigned int f=0; f<get_n_Graphicals(); f++)
   {
      Feature* Feature_ptr=get_Feature_ptr(f);
      threevector UVW;
      Feature_ptr->get_UVW_coords(
         get_curr_t(),get_passnumber(),UVW);
      threevector ray=camera_ptr->pixel_ray_direction(twovector(UVW));
//      cout << "f = " << f << " UV = " << UVW 
//           << " ray = " << ray << endl;
      Feature_ptr->set_UVW_coords(
         get_curr_t(),max_n_passes,ray);
   } // loop over index f labeling 2D features
}

// ==========================================================================
// Feature output methods
// ==========================================================================

// This first version of member function write_feature_html_file()
// attempts to determine the number of columns needed to hold all the
// FeaturesGroup's passes.

void FeaturesGroup::write_feature_html_file()
{
//   cout << "inside FeaturesGroup::write_features_html_file() #1" << endl;
   int n_columns=get_max_n_passes(get_curr_t())+1;
   write_feature_html_file(get_curr_t(),n_columns);
}

void FeaturesGroup::write_feature_html_file(unsigned int n_columns)
{
//   cout << "inside FeaturesGroup::write_features_html_file() #2" << endl;
   write_feature_html_file(get_curr_t(),n_columns);
}

void FeaturesGroup::write_feature_html_file(double t,unsigned int n_columns)
{
   cout << "inside FeaturesGroup::write_features_html_file() #3" << endl;
   cout << "n_columns = " << n_columns << endl;
   vector<string> column_labels;
   column_labels.push_back("ID");

   vector<int> pass_IDs=get_all_pass_numbers(get_curr_t());
   for (unsigned int c=1; c<n_columns; c++)
   {
      column_labels.push_back("Pass"+stringfunc::number_to_string(
         pass_IDs[c-1]));
   }

   bool output_only_multicoord_features_flag=false;
   string output_html_filename="pass_features.html";
   write_feature_html_file(
      t,column_labels,output_only_multicoord_features_flag,
      output_html_filename);
}

// -------------------------------------------------------------------------
// This next overloaded version of write_feature_html_file takes in an
// STL vector of photographs as well as a vector indicating the order
// in which these photographs should be displayed within the columns
// of an output html table.  The columns are labeled by the names of
// the photos.

void FeaturesGroup::write_feature_html_file(
   photogroup* photogroup_ptr,bool output_only_multicoord_features_flag,
   bool output_3D_rays_flag)
{
//   cout << "inside FeaturesGroup::write_features_html_file() #4" << endl;
   vector<string> column_labels;
   column_labels.push_back("Feature ID");
   for (unsigned int p=0; p<photogroup_ptr->get_n_photos(); p++)
   {
      cout << "Photo order p = " << p 
           << " photo index  = " 
           << photogroup_ptr->get_photo_index_given_order(p) << endl;
      string curr_photo_filename=photogroup_ptr->get_photograph_ptr(p)->
         get_filename();
      string curr_label=filefunc::getbasename(curr_photo_filename);
      column_labels.push_back(curr_label);
   }
   if (output_3D_rays_flag)
   {
//      column_labels.push_back("3D ray");
      column_labels.push_back("3D point");
   }

   string output_html_filename="photograph_features.html";
   write_feature_html_file(get_curr_t(),column_labels,
                           output_only_multicoord_features_flag,
                           output_html_filename);
}

// -------------------------------------------------------------------------
// Member function write_feature_html_file() outputs multi-pass
// coordinates for every feature within *this to an html table which
// can be viewed with any web browser.

void FeaturesGroup::write_feature_html_file(
   double t,const vector<string>& column_labels,
   bool output_only_multicoord_features_flag,string output_html_filename)
{
   cout << "inside FeaturesGroup::write_feature_html_file() #5" << endl;
   string banner="Writing features HTML file:";
   outputfunc::write_banner(banner);

   unsigned int nrows=get_n_Graphicals();
   unsigned int ncolumns=column_labels.size();
   cout << "nrows = " << nrows << " ncolumns = " << ncolumns << endl;
   
   table* table_ptr=new table(nrows,ncolumns);

   string title="Time = "+stringfunc::number_to_string(t);
   table_ptr->set_caption(title);
   table_ptr->set_caption_text_color(colorfunc::blue);

   table_ptr->set_header_text_color(colorfunc::red);

   for (unsigned int c=0; c<ncolumns; c++)
   {
      table_ptr->set_header_label(c,column_labels[c]);
      cout << "c = " << c << " column_labels[c] = " << column_labels[c]
           << endl;
   }
   vector<int> pass_IDs=get_all_pass_numbers(get_curr_t());

   for (unsigned int f=0; f<nrows; f++)
   {
      Feature* Feature_ptr=get_Feature_ptr(f);
      instantaneous_obs* obs_ptr=
         Feature_ptr->get_all_particular_time_observations(t);

      if (output_only_multicoord_features_flag && 
          obs_ptr->get_npasses() <= 1) continue;

      table_ptr->set(f,0,Feature_ptr->get_ID());
      vector<int> obs_pass_numbers=obs_ptr->get_pass_numbers();

      for (unsigned int c=1; c<ncolumns; c++)
      {
         int p=pass_IDs[c-1];

// If FeaturesGroup contains only a single pass' set of features,
// their pass number does not necessarily equal 0...

//         if (obs_pass_numbers.size()==1) p=obs_pass_numbers.front();
         
         if (obs_ptr->check_for_pass_entry(p))
         {
            threevector curr_UVW=obs_ptr->retrieve_UVW_coords(p);
            table_ptr->set(f,c,curr_UVW);
         }
      } // loop over index c labeling table columns
   } // loop over index f labeling features

   table_ptr->write_html_file(output_html_filename);
   delete table_ptr;

   banner="Feature observations written to "+output_html_filename;
   outputfunc::write_banner(banner);
}

// -------------------------------------------------------------------------
// Member function write_GPUSIFT_feature_file() outputs multi-pass
// coordinates for every feature within *this to an ascii file which
// can be read in by GPUSIFT.  In each row, this method first writes
// the XYZ world coordinates for a feature followed by one or more
// sets of UV image plane coordinates.

void FeaturesGroup::write_GPUSIFT_feature_file(double t)
{
//   cout << "inside FeaturesGroup::write_GPUSIFT_feature_file()" << endl;

   string output_filename="sift_features.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   outstream << "# X Y Z  nframes  frame0 x0 y0  frame1 x1 y1 ..."
             << endl;
   
   unsigned int max_n_passes=get_max_n_passes(t);
   for (unsigned int f=0; f<get_n_Graphicals(); f++)
   {
      Feature* Feature_ptr=get_Feature_ptr(f);

      instantaneous_obs* obs_ptr=
         Feature_ptr->get_all_particular_time_observations(t);
      int n_passes=obs_ptr->get_npasses();

//      cout << "f = " << f
//           << " ID = " << Feature_ptr->get_ID()
//           << " n_passes = " << n_passes << endl;

      threevector XYZ=obs_ptr->retrieve_UVW_coords(max_n_passes-1);
      outstream << XYZ.get(0) << " "
                << XYZ.get(1) << " "
                << XYZ.get(2) << " "
                << n_passes-1 << "  ";

      for (unsigned int p=0; p<max_n_passes-1; p++)
      {
         twovector UV=obs_ptr->retrieve_UVW_coords(p);
         double U=UV.get(0);
         double V=UV.get(1);
         if (!nearly_equal(U,NEGATIVEINFINITY,100.0)  ||
             ! nearly_equal(V,NEGATIVEINFINITY,100.0))
         {
            outstream << p << " "
                      << UV.get(0) << " "
                      << UV.get(1) << " ";
         }
      }
      outstream << endl;
   } // loop over index f labeling features
   
   filefunc::closefile(output_filename,outstream);
}


// ==========================================================================
// Ascii feature file I/O methods
// ==========================================================================

// Member function save_feature_info_to_file loops over all features
// within *get_featurelist_ptr() and prints their times, IDs, pass numbers
// and UVW coordinates to the output ofstream.  This feature
// information can later be read back in via member function
// read_feature_info_from_file.  This method returns the name of the
// output text file.

string FeaturesGroup::new_save_feature_info_to_file()
{
   int n_dims=get_ndims();
   string features_filename;
   if (n_dims==2)
   {
      AnimationController* AC_ptr=get_AnimationController_ptr();
      int curr_framenumber = AC_ptr->get_curr_framenumber();
//   cout << "curr_framenumber = " << curr_framenumber << endl;
      string image_filename = AC_ptr->get_ordered_image_filename(curr_framenumber);
//   cout << "image_filename = " << image_filename << endl;

      string image_subdir=filefunc::getdirname(image_filename);
      string image_prefix=filefunc::getprefix(image_filename);
      features_filename=image_subdir+"features_"+stringfunc::number_to_string(
         get_ndims())+"D_"+image_prefix+".txt";
   }
   else
   {
      features_filename="features_"+stringfunc::number_to_string(n_dims)
         +"D.txt";
   }
   save_feature_info_to_file(features_filename);

   write_feature_html_file();

   string banner="Exported features to "+features_filename;
   outputfunc::write_banner(banner);
   return features_filename;
}

string FeaturesGroup::save_feature_info_to_file()
{
//   string output_filename=get_output_filename("features");
   string output_filename="features_"+stringfunc::number_to_string(
      get_ndims())+"D.txt";
   cout << "get_ndims() = " << get_ndims() << endl;
   cout << "output_filename = " << output_filename << endl;

   return save_feature_info_to_file(output_filename);
}

string FeaturesGroup::save_feature_info_to_file(string output_filename)
{
   int output_passnumber=get_passnumber();
//   cout << "Current working passnumber = " << get_passnumber() << endl;
//   cout << "Enter output passnumber for features to be saved to ascii file:"
//        << endl;
//   cin >> output_passnumber;

   ofstream outstream;
   outstream.precision(6);
   filefunc::openfile(output_filename,outstream);

   if (get_ndims()==3)
   {
      outstream << "# Time   Feature_ID   Passnumber   X  Y  Z  Score"
                << endl << endl;
   }
   else if (get_ndims()==2)
   {
      outstream << "# Time   Feature_ID   Passnumber   X  Y  Score"
                << endl << endl;
   }

   for (unsigned int imagenumber=get_first_framenumber(); 
        imagenumber <= get_last_framenumber(); imagenumber++)
   {
      bool data_written_flag=false;
      double curr_t=static_cast<double>(imagenumber);
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         Feature* Feature_ptr=get_Feature_ptr(n);
         instantaneous_obs* curr_obs_ptr=Feature_ptr->get_particular_time_obs(
            curr_t,get_passnumber());
         if (curr_obs_ptr != NULL)
         {
            bool erased_point_flag=false;

// Store UVW coords measured wrt instantaneous U_hat, V_hat and W_hat
// which need not coincide with X_hat, Y_hat and Z_hat within
// p_transformed:
         
            threevector p_transformed;
            vector<double> column_data;
            if (Feature_ptr->get_transformed_UVW_coords(
               curr_t,get_passnumber(),p_transformed))
            {
               if (Feature_ptr->get_mask(curr_t,get_passnumber()))
               {
                  erased_point_flag=true;
               }
               else
               {
                  for (unsigned int j=0; j<get_ndims(); j++)
                  {
                     column_data.push_back(p_transformed.get(j));
                  }
               }
            }
            if (!erased_point_flag) 
            {
               outstream.setf(ios::showpoint);
               outstream << setw(5) << stringfunc::number_to_string(curr_t,5)
                         << setw(6) << Feature_ptr->get_ID() 
                         << setw(5) << output_passnumber << "   ";
               for (unsigned int j=0; j<get_ndims(); j++)
               {
                  outstream.precision(10);
                  outstream << setw(13) << column_data[j] << " ";
             }

               double curr_score;
               Feature_ptr->get_score(curr_t,get_passnumber(),curr_score);
               outstream << setw(14) << curr_score << endl;
//               outstream << endl;
               data_written_flag=true;
            } // !erased_point_flag conditional
         } // curr_obs_ptr != NULL conditional
      } // loop over nodes in *get_featurelist_ptr()
      if (data_written_flag) outstream << endl;
   } // loop over imagenumber index

   filefunc::closefile(output_filename,outstream);

   return filefunc::get_pwd()+output_filename;
}

// -------------------------------------------------------------------------
// Member function read_feature_info_from_file parses ascii text
// files generated by member function save_feature_info_to_file().
// After purging the featurelist, this method regenerates the features
// within the list based upon the ascii text file information.

string FeaturesGroup::read_info_from_file(
   bool query_passnumber_conversion_flag)
{
//   cout << "inside FeaturesGroup::read_info_from_file()" << endl;

   string input_filename="features_"+stringfunc::number_to_string(
      get_ndims())+"D.txt";
   return read_feature_info_from_file(
      input_filename,query_passnumber_conversion_flag);
}

string FeaturesGroup::read_feature_info_from_file(
   bool query_passnumber_conversion_flag)
{
//   cout << "inside FeaturesGroup::read_feature_info_from_file()" << endl;

   string Graphicals_prefix="features";
   string input_filename=get_default_info_filename(Graphicals_prefix);
   cout << "Default input filename = " << input_filename << endl;
   input_filename=get_input_filename(Graphicals_prefix);
   return read_feature_info_from_file(
      input_filename,query_passnumber_conversion_flag);
}

string FeaturesGroup::read_feature_info_from_file(
   string input_filename,bool query_passnumber_conversion_flag)
{
   vector<int> pass_number;
   return read_feature_info_from_file(input_filename,pass_number,
      query_passnumber_conversion_flag);
}

string FeaturesGroup::read_feature_info_from_file(
   string input_filename,vector<int>& pass_number,
   bool query_passnumber_conversion_flag)
{
//   cout << "inside FeaturesGroup::read_feature_info_from_file()" << endl;
//   cout << "get_ndims() = " << get_ndims() << endl;

   char convert_char;
   if (query_passnumber_conversion_flag) 
   {
      cout << "Enter 'c' to convert pass numbers read from input file"
           << endl;
      cout << "to FeaturesGroup current passnumber:" << endl;
      cin >> convert_char;
   }

// FAKE FAKE:  Weds Mar 13, 2013 at 4:58 pm
// FAKE FAKE:  Thurs Apr 25, 2013 at 10:40 am

// Change convert_passnumber_flag to false for 2D/3D Deer Island
// feature ingestion

   bool convert_passnumber_flag=true;
//   bool convert_passnumber_flag=false;
/*

   if (convert_char=='c') convert_passnumber_flag=true;
   cout << "input_filename = " << input_filename << endl;
//   cout << "convert_passnumber_flag = " << convert_passnumber_flag
//        << endl;
*/

   filefunc::ReadInfile(input_filename);
   int approx_n_features=filefunc::text_line.size();

   vector<double> curr_time,score;
   vector<int> feature_ID;
   vector<threevector> UVW;

   curr_time.reserve(approx_n_features);
   feature_ID.reserve(approx_n_features);
   pass_number.reserve(approx_n_features); 
   UVW.reserve(approx_n_features);

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> X=stringfunc::string_to_numbers(filefunc::text_line[i]);

      if (X.size() < 6) continue;
      
      curr_time.push_back(X[0]);
      feature_ID.push_back(basic_math::round(X[1]));
      pass_number.push_back(basic_math::round(X[2]));
      UVW.push_back(threevector(X[3],X[4],X[5]));

      if (get_ndims()==2)
      {
         score.push_back(X[5]);
      }

/*
      cout << "i = " << i
           << " t = " << curr_time.back()
           << " ID = " << feature_ID.back()
           << " pass = " << pass_number.back()
           << " U = " << UVW.back().get(0) 
           << " V = " << UVW.back().get(1) 
           << " W = " << UVW.back().get(2) 
           << endl;
*/
   } // loop over index i labeling ascii file line number

// Destroy all existing features before creating a new feature list
// from the input ascii file:

   destroy_all_Graphicals();

   outputfunc::write_banner("Instantiating & loading new set of features:");
   for (unsigned int i=0; i<feature_ID.size(); i++)
   {
      if (i%1000==0) cout << i/1000 << " " << flush;

      Feature* Feature_ptr=get_ID_labeled_Feature_ptr(feature_ID[i]);
      if (Feature_ptr == NULL)
      {
         Feature_ptr=generate_new_Feature(feature_ID[i]);

// Initially erase/hide feature for all images in current pass by
// setting its mask flag to true:
      
         for (unsigned int n=get_first_framenumber(); n<=get_last_framenumber(); n++)
         {
            double curr_t=static_cast<double>(n);
            Feature_ptr->set_mask(curr_t,get_passnumber(),true);
         }
      } // Feature_ptr==NULL conditional

// Load time, imagenumber, passnumber and UVW coordinate information
// into current feature.  Set manually_manipulated flag to true and
// coords_erased flag to false for each STL vector entry:

      const osg::Quat trivial_q(0,0,0,1);
      const threevector trivial_scale(1,1,1);

      int curr_passnumber=pass_number[i];
      if (convert_passnumber_flag)
      {
         curr_passnumber=get_passnumber();
      }

      Feature_ptr->set_UVW_coords(curr_time[i],curr_passnumber,UVW[i]);
      Feature_ptr->set_quaternion(
         curr_time[i],curr_passnumber,trivial_q);
      Feature_ptr->set_scale(curr_time[i],curr_passnumber,trivial_scale);
      Feature_ptr->set_coords_manually_manipulated(
         curr_time[i],curr_passnumber);
      Feature_ptr->set_mask(curr_time[i],curr_passnumber,false);

      if (get_ndims()==2)
      {
         Feature_ptr->set_score(curr_time[i],curr_passnumber,score[i]);
//         cout << "i = " << i << " score = " << score[i] << endl;
      }

/*
      if (i < 2)
      {
         cout << "i = " << i
              << " ID = " << Feature_ptr->get_ID()
              << " t = " << curr_time[i]
              << " pass = " << curr_passnumber
              << " UVW = " << UVW[i] << endl;
      }
*/
    
/*
// FAKE FAKE:  Sun Mar 6, 2011 at 10:57 am
// Try writing out instantaneous observations current feature

      cout << "Writing out instantaneous obs for curr feature after loading"
           << endl;
      cout << "-----------------------------------------------------" << endl;
      instantaneous_obs* obs_ptr=
         Feature_ptr->get_all_particular_time_observations(curr_time[i]);
      vector<int> obs_pass_numbers=obs_ptr->get_pass_numbers();
      for (unsigned int p=0; p<25; p++)
      {
         if (obs_ptr->check_for_pass_entry(p))
         {
            threevector curr_UVW=obs_ptr->retrieve_UVW_coords(p);
            cout << "pass = " << p 
                 << " Feature ID = " << Feature_ptr->get_ID()
                 << " curr_UVW = " << curr_UVW;
         }
      } // loop over index p labeling passes
      cout << "-----------------------------------------------------" << endl;
*/


   } // loop over index i labeling entries in STL time, feature_ID
     // and UVW vectors

   outputfunc::newline();
   cout << "Number of features read in from text file = " 
        << get_n_Graphicals() << endl;
   min_unpropagated_feature_ID=minimum_point_ID();
   max_unpropagated_feature_ID=maximum_point_ID();
   cout << "Minimum feature ID = " << min_unpropagated_feature_ID << endl;
   cout << "Maximum feature ID = " << max_unpropagated_feature_ID << endl;
//   outputfunc::enter_continue_char();

// Recall that we typically initialize the Features OSG group's
// nodemask to zero.  In order to effectively reattach the read-in
// features to the scene graph, we need to reset their group's node
// mask to 1:

   get_OSGgroup_ptr()->setNodeMask(1);

   return filefunc::get_pwd()+input_filename;
}

// ==========================================================================
// KLT tracking methods
// ==========================================================================

// Member function prune_short_trackfeatures queries the user to enter
// the minimal number of images in which features should appear
// unerased.  It then deletes from the features list any feature which
// does not meet this requirement.  We wrote this method in Sept 2005
// to prune automatically generated optic flow tracks for purposes of
// matching onto 3D feature information.

void FeaturesGroup::prune_short_track_features()
{   
   outputfunc::write_banner("Pruning short track features:");

   int min_n_image_appearances=10;
   cout << "Enter minimal number of images in which features should appear:"
        << endl;
   cin >> min_n_image_appearances;

   vector<int> features_to_prune;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Feature* Feature_ptr=get_Feature_ptr(n);
      int n_image_appearances=Feature_ptr->count_image_appearances(
         get_passnumber(),get_first_framenumber(),get_last_framenumber());
      if (n_image_appearances < min_n_image_appearances)
      {
         features_to_prune.push_back(Feature_ptr->get_ID());
      }
   } // loop over index n labeling features

   unsigned int n_features_before_pruning=get_n_Graphicals();
   for (unsigned int n=0; n<features_to_prune.size(); n++)
   {
      destroy_Graphical(features_to_prune[n]);
   }

   unsigned int n_features_after_pruning=get_n_Graphicals();
   cout << "# features before pruning = " << n_features_before_pruning
        << endl;
   cout << "# features after pruning = " << n_features_after_pruning << endl;
}

// --------------------------------------------------------------------------
// Member function center_imagery_on_selected_feature toggles the
// center_image_on_selected_feature_flag member boolean flag on & off
// each time this method is entered.  If the flag is toggled on, the
// the update_display() callback sets the CustomManipulator's center
// so that the selected feature remains stabilized for as long as it
// is within the field-of-view.

void FeaturesGroup::center_imagery_on_selected_feature()
{   
   if (center_image_on_selected_feature_flag)
   {
      center_image_on_selected_feature_flag=false;
   }
   else
   {
      centered_feature_number=get_selected_Graphical_ID() ;
      if (centered_feature_number < 0)
      {
         cout << "Enter ID of feature to center imagery upon:" << endl;
         cin >> centered_feature_number;
      }
      
      Feature* Feature_ptr=get_ID_labeled_Feature_ptr(
         centered_feature_number);
      if (Feature_ptr != NULL)
      {
         center_image_on_selected_feature_flag=true;
      }
      else
      {
         center_image_on_selected_feature_flag=false;
      } // Feature_ptr != NULL conditional
   } // center_image_on_selected_feature_flag conditional
}

// --------------------------------------------------------------------------
// Member function update_image_appearances toggles boolean member
// display_image_appearances_flag.  If this flag==true, it sets
// text_ptr[1] for each feature equal to the number of times it
// appears in all video images for a certain pass.  This information
// is displayed to the lower left of each 2D feature crosshair.

void FeaturesGroup::update_image_appearances()
{   
   display_image_appearances_flag = !display_image_appearances_flag;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Feature* Feature_ptr=get_Feature_ptr(n);
      if (Feature_ptr != NULL)
      {
//         int ID=Feature_ptr->get_ID();
         int n_image_appearances=Feature_ptr->count_image_appearances(
            get_passnumber(),get_first_framenumber(),get_last_framenumber());
//         cout << "Feature " << ID << " appears in " << n_image_appearances 
//              << " images" << endl;
         osgText::Text* text_ptr=Feature_ptr->get_text_ptr(1);
         string buffer="";
         if (display_image_appearances_flag) 
            buffer=stringfunc::number_to_string(n_image_appearances);
         text_ptr->setText(buffer);
         text_ptr->setColor(colorfunc::get_OSG_color(
            colorfunc::brightcyan));
//            colorfunc::brightyellow));
      } // Feature_ptr != NULL conditional
   } // loop over index n labeling features
}

// --------------------------------------------------------------------------
// Member function update_feature_scores toggles boolean member
// display_feature_scores_flag.  If this flag==true, it sets
// text_ptr[1] for each feature equal to its KLT score.  This
// information is displayed to the lower left of each 2D feature
// crosshair.

void FeaturesGroup::update_feature_scores()
{   
//   display_feature_scores_flag = !display_feature_scores_flag;

   double min_score=POSITIVEINFINITY;
   double curr_score=-1;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Feature* Feature_ptr=get_Feature_ptr(n);
      if (Feature_ptr != NULL)
      {
//         int ID=Feature_ptr->get_ID();
         Feature_ptr->get_score(get_curr_t(),get_passnumber(),curr_score);
         if (curr_score > 0) min_score=basic_math::min(min_score,curr_score);
//         cout << "Feature " << ID << " has score = " << curr_score << endl;
         osgText::Text* text_ptr=Feature_ptr->get_text_ptr(1);
         string buffer="";
         if (display_feature_scores_flag) 
            buffer=stringfunc::number_to_string(curr_score,1);
         text_ptr->setText(buffer);
         text_ptr->setColor(colorfunc::get_OSG_color(
            colorfunc::brightcyan));
//            colorfunc::brightyellow));
      } // Feature_ptr != NULL conditional
   } // loop over index n labeling features
//   cout << "Minimum non-negative score = " << min_score << endl;
}

// --------------------------------------------------------------------------
// Member function erase_unmarked_features loops over all features and
// checks whether their IDs exist in member *marked_feature_list_ptr.
// If not and if boolean member erase_unmarked_features_flag==true,
// this method erases those features not appearing in the linked list
// for all image times.  It also toggles the boolean
// erase_unmarked_features_flag so that upon the next invocation of
// this method, the erased features are restored.  This allows a user
// to toggle on/off the unmarked features within all images of a video
// pass.

void FeaturesGroup::erase_unmarked_features()
{   
   erase_unmarked_features_flag=!erase_unmarked_features_flag;
   string banner="Unerasing unmarked features:";
   if (erase_unmarked_features_flag) banner="Erasing unmarked features:";
   outputfunc::write_banner(banner);

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Feature* Feature_ptr=get_Feature_ptr(n);
      if (Feature_ptr != NULL)
      {
         int ID=Feature_ptr->get_ID();

         Mynode<int>* currnode_ptr=marked_feature_list_ptr->data_in_list(ID);
         if (currnode_ptr==NULL)
         {
            for (unsigned int imagenumber=get_first_framenumber(); 
                 imagenumber <= get_last_framenumber(); imagenumber++)
            {
               double t=static_cast<double>(imagenumber);
               Feature_ptr->set_mask(t,get_passnumber(),
                                     erase_unmarked_features_flag);
            } // loop over image numbers
         } // currnode_ptr==NULL conditional
      } // Feature_ptr != NULL conditional
   } // loop over index n labeling features
}

// -------------------------------------------------------------------------
// Member function renumber_all_features loops over all features and
// assigns their IDs equal to integers ranging from 0 to
// n_Graphicals-1.  We wrote this little utility method to facilitate
// 3D tie-point picking for down-selected KLT tracked 2D video
// features.

void FeaturesGroup::renumber_all_features()
{   
   outputfunc::write_banner("Renumbering all features:");
   GraphicalsGroup::renumber_all_Graphicals();

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      get_Feature_ptr(n)->reset_text_label();
   }
}

// -------------------------------------------------------------------------
// Member function shift_all_feature_IDs loops over all features and
// shifts their IDs by delta_ID.  We wrote this little utility method
// to facilitate merging of 2D tie points with KLT tracked 2D video
// features.

void FeaturesGroup::shift_all_feature_IDs(int delta_ID)
{   
   outputfunc::write_banner("Shifting all feature IDs:");
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Feature* Feature_ptr=get_Feature_ptr(n);
      Feature_ptr->set_ID(Feature_ptr->get_ID()+delta_ID);
      Feature_ptr->reset_text_label();
      min_unpropagated_feature_ID += delta_ID;
      max_unpropagated_feature_ID += delta_ID;
   }
}

// -------------------------------------------------------------------------
// Member function compute_avg_feature_offsets_from_midpass_location
// loops over every feature's UVW location for each image.  It
// computes the average of every visible feature's UVW coordinates as
// a function of image number.  The results are saved within
// CentersGroup avg_delta_UVW_per_image member STL vector.  This
// information is useful for stabilizing entire sequences of video
// imagery.

void FeaturesGroup::compute_avg_feature_offsets_from_midpass_location(
   bool temporally_filter_avg_offsets_flag)
{   
   outputfunc::write_banner(
      "Computing average feature offsets from midpass location:");
   
   int mid_imagenumber=get_Nimages()/2;
   double mid_t=static_cast<double>(mid_imagenumber);

   vector<bool> mid_UVW_found_flag;
   vector<threevector> mid_pass_UVW;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Feature* Feature_ptr=get_Feature_ptr(n);
      threevector mid_UVW(0,0,0);
      mid_UVW_found_flag.push_back(
         Feature_ptr->get_UVW_coords(mid_t,get_passnumber(),mid_UVW) &&
         !Feature_ptr->get_mask(mid_t,get_passnumber()));
      mid_pass_UVW.push_back(mid_UVW);
   }

// Initialize number of KLT features and their offsets from their
// mid-pass locations to zero in next 2 STL vectors:

   int imagenumber_with_no_features=-1;
   vector<threevector> delta_UVW_per_image;
   vector<threevector> avg_delta_UVW_per_image;
   for (unsigned int imagenumber=get_first_framenumber(); 
        imagenumber <= get_last_framenumber(); imagenumber++)
   {
      double curr_t=static_cast<double>(imagenumber);
      delta_UVW_per_image.clear();

      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         if (mid_UVW_found_flag[n])
         {
            Feature* Feature_ptr=get_Feature_ptr(n);
            threevector curr_UVW;
            if (Feature_ptr->get_UVW_coords(curr_t,get_passnumber(),curr_UVW)
                && !Feature_ptr->get_mask(curr_t,get_passnumber()))
            {
               delta_UVW_per_image.push_back(mid_pass_UVW[n]-curr_UVW);
            } 
         }
      } // loop over index n labeling features

// Store averaged feature offset from mid-pass location within
// CentersGroup avg_delta_UVW_per_image STL vector member:

      threevector avg_delta(0,0,0);
      for (unsigned int n=0; n<delta_UVW_per_image.size(); n++)
      {
         avg_delta += delta_UVW_per_image[n];
      }
      
      if (delta_UVW_per_image.size() > 0)
      {
         avg_delta_UVW_per_image.push_back(
            avg_delta/delta_UVW_per_image.size());
      }
      else
      {
         avg_delta_UVW_per_image.push_back(threevector(0,0,0));
         imagenumber_with_no_features=imagenumber;
      }
   } // loop over imagenumber index

// Copy avg_delta_UVW_per_image from image n =
// imagenumber_with_no_features+1 back onto first images 0 through n-1
// that have zero values for avg_delta_UVW_per_image:

   for (int n=0; n <= imagenumber_with_no_features; n++)
   {
      avg_delta_UVW_per_image[n]=avg_delta_UVW_per_image[
         imagenumber_with_no_features+1];
   }
   if (temporally_filter_avg_offsets_flag)
      temporally_filter_feature(-1,0,avg_delta_UVW_per_image);

   for (unsigned int imagenumber=get_first_framenumber(); 
        imagenumber <= get_last_framenumber(); imagenumber++)
   {
      CentersGroup_ptr->set_avg_delta_UVW_per_image(
         imagenumber,avg_delta_UVW_per_image[imagenumber]);
   }
}

/*
// -------------------------------------------------------------------------
// This variant of member function
// compute_avg_feature_offsets_from_midpass_location was created as an
// experiment.  We believed that averaging over 100s of features to
// determine the average offset per image would yield a better
// stabilization estimate than over a few dozen judiciously selected
// features.  As of 10/6/05, it looks like our original hunch was
// WRONG.  So this following method is deprecated compared to
// compute_avg_feature_offset_from_midpass_location...

void FeaturesGroup::compute_avg_feature_offsets(
   bool temporally_filter_avg_offsets_flag)
{   
   outputfunc::write_banner("Computing average feature offsets:");

// Initialize number of KLT features and their curr-prev image offsets
// to zero in next 2 STL vectors:

   vector<threevector> feature_delta;
   vector<threevector> avg_delta_UVW_per_image;
   avg_delta_UVW_per_image.push_back(threevector(0,0,0));	// image 0

   int imagenumber_with_no_features=-1;
   for (unsigned int imagenumber=1; imagenumber < get_Nimages(); imagenumber++)
   {
      double curr_t=static_cast<double>(imagenumber);
      double prev_t=static_cast<double>(imagenumber-1);

      feature_delta.clear();
      for (unsigned int f=0; f<get_n_Graphicals(); f++)
      {
         Feature* Feature_ptr=get_Feature_ptr(f);
         threevector curr_UVW,prev_UVW;
         if (Feature_ptr->get_UVW_coords(curr_t,get_passnumber(),curr_UVW)
             && !Feature_ptr->get_mask(curr_t,get_passnumber()) 
             && Feature_ptr->get_UVW_coords(prev_t,get_passnumber(),prev_UVW)
             && !Feature_ptr->get_mask(prev_t,get_passnumber()))
         {
            feature_delta.push_back(curr_UVW-prev_UVW);
         } 
      } // loop over index f labeling features

      threevector delta_sum(0,0,0);
      for (unsigned int i=0; i<feature_delta.size(); i++)
      {
         delta_sum += feature_delta[i];
      }

      if (feature_delta.size() > 0)
      {
         avg_delta_UVW_per_image.push_back(
            avg_delta_UVW_per_image[imagenumber-1]+
            delta_sum/feature_delta.size());
      }
      else
      {
         avg_delta_UVW_per_image.push_back(threevector(0,0,0));
         imagenumber_with_no_features=imagenumber;
      }
   } // loop over imagenumber index

// Translate avg_delta_UVW_per_image back by n=get_Nimages()/2 to
// stabilize imagery wrt to mid-pass image:

   int mid_imagenumber=get_Nimages()/2;
   double mid_t=static_cast<double>(mid_imagenumber);
   threevector avg_delta_UVW_mid_pass=avg_delta_UVW_per_image[
      mid_imagenumber];
   for (unsigned int imagenumber=0; imagenumber < get_Nimages(); imagenumber++)
   {
      avg_delta_UVW_per_image[imagenumber] -= avg_delta_UVW_mid_pass;
   }
   
// Copy avg_delta_UVW_per_image from image n =
// imagenumber_with_no_features+1 back onto first images 0 through n-1
// that have zero values for avg_delta_UVW_per_image:

   cout << "imagenumber_with_no_features = "
        << imagenumber_with_no_features << endl;
   for (unsigned int n=0; n <= imagenumber_with_no_features; n++)
   {
      avg_delta_UVW_per_image[n]=avg_delta_UVW_per_image[
         imagenumber_with_no_features+1];
   }
   if (temporally_filter_avg_offsets_flag)
      temporally_filter_feature(-1,0,avg_delta_UVW_per_image);

   for (unsigned int imagenumber=0; imagenumber < get_Nimages(); imagenumber++)
   {
      CentersGroup_ptr->set_avg_delta_UVW_per_image(
         imagenumber,-avg_delta_UVW_per_image[imagenumber]);
//      cout << "i = " << imagenumber
//           << " -avg_delta = " << -avg_delta_UVW_per_image[imagenumber].get(0)
//           << " " << -avg_delta_UVW_per_image[imagenumber].get(1)
//           << endl;
   }
}
*/

// -------------------------------------------------------------------------
// Member function temporally_filter_stabilized_features loops over
// every node in the features linked list and stores their stabilized
// coordinates into a local STL vector.  It then passes that vector to
// member function temporally_filter_feature().  After all features
// have been filtered, their original raw positions are replaced with
// their temporally smoothed counterparts in the features linkedlist.

void FeaturesGroup::temporally_filter_stabilized_features()
{   
   outputfunc::write_banner("Temporally filtering stabilized features:");

   vector<threevector> stabilized_UVW;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      cout << n << " " << flush;
      Feature* Feature_ptr=get_Feature_ptr(n);
      int feature_ID=Feature_ptr->get_ID();

      int zeroth_imagenumber=-1;
      stabilized_UVW.clear();
      for (unsigned int imagenumber=get_first_framenumber(); 
           imagenumber <= get_last_framenumber(); imagenumber++)
      {
         threevector avg_delta=CentersGroup_ptr->get_avg_delta_UVW_per_image(
            imagenumber);
         double curr_t=static_cast<double>(imagenumber);
         threevector curr_UVW;
         if (Feature_ptr->get_UVW_coords(curr_t,get_passnumber(),curr_UVW))
         {
            if (zeroth_imagenumber < 0) 
            {
               zeroth_imagenumber=imagenumber;
//               cout << "feature = " << feature_ID 
//                    << " zeroth imagenumber = " << zeroth_imagenumber << endl;
            }
            stabilized_UVW.push_back(curr_UVW+avg_delta);
         }
      } // loop over imagenumber index
//      cout << "stabilized_UVW.size() = "
//           << stabilized_UVW.size() << endl;
//      cout << "last imagenumber = " << zeroth_imagenumber+
//         stabilized_UVW.size() << endl;

      vector<threevector> orig_stabilized_UVW;
      for (unsigned int i=0; i<stabilized_UVW.size(); i++)
      {
         orig_stabilized_UVW.push_back(stabilized_UVW[i]);
      }

      temporally_filter_feature(feature_ID,zeroth_imagenumber,stabilized_UVW);

// Replace features' raw UVW coordinates with their temporally
// filtered counterparts:

      int counter=0;
      vector<double> Uorig,Vorig,Unew,Vnew;
      for (unsigned int imagenumber=get_first_framenumber(); 
           imagenumber < get_last_framenumber(); imagenumber++)
      {
         threevector avg_delta=CentersGroup_ptr->get_avg_delta_UVW_per_image(
            imagenumber);

         double curr_t=static_cast<double>(imagenumber);
         threevector curr_UVW;
         if (Feature_ptr->get_UVW_coords(curr_t,get_passnumber(),curr_UVW))
         {
            Feature_ptr->set_UVW_coords(curr_t,get_passnumber(),
                                        stabilized_UVW[counter++]-avg_delta);

//            cout << "feature " << n << " image = " << imagenumber
//                 << " U = " << orig_stabilized_UVW[counter-1].get(0)
//                 << " U' = " << stabilized_UVW[counter-1].get(0)
//                 << " V = " << orig_stabilized_UVW[counter-1].get(1)
//                 << " V' = " << stabilized_UVW[counter-1].get(1)
//                 << endl;

/*
            if (counter >= 2)
            {
               double orig_deltaU=orig_stabilized_UVW[counter-1].get(0)-
                  orig_stabilized_UVW[counter-2].get(0);
               double new_deltaU=stabilized_UVW[counter-1].get(0)-
                  stabilized_UVW[counter-2].get(0);
               double orig_deltaV=orig_stabilized_UVW[counter-1].get(1)-
                  orig_stabilized_UVW[counter-2].get(1);
               double new_deltaV=stabilized_UVW[counter-1].get(1)-
                  stabilized_UVW[counter-2].get(1);

               Uorig.push_back(orig_deltaU);
               Vorig.push_back(orig_deltaV);
               Unew.push_back(new_deltaU);
               Vnew.push_back(new_deltaV);
            }
*/
         } // get_UVW_coords conditional
      } // loop over imagenumber index

//      prob_distribution prob_Uorig(Uorig,50);
//      prob_distribution prob_Vorig(Vorig,50);
//      prob_distribution prob_Unew(Unew,50);
//      prob_distribution prob_Vnew(Vnew,50);

//      cout << "feature = " << n << endl;
//      cout << "Uorig median = " << prob_Uorig.median()
//           << " width = " << prob_Uorig.quartile_width() << endl;
//      cout << "Unew median = " << prob_Unew.median()
//           << " width = " << prob_Unew.quartile_width() << endl;
//      cout << "Vorig median = " << prob_Vorig.median()
//           << " width = " << prob_Vorig.quartile_width() << endl;
//      cout << "Vnew median = " << prob_Vnew.median()
//           << " width = " << prob_Vnew.quartile_width() << endl;

   } // loop over index n labeling feature number
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Member function temporally_filter_feature performs a brute force
// convolution of raw feature positions within STL vector UVW with a
// variable sized gaussian filter.  At the beginning of the raw data
// set, the gaussian's width linearly ramps up from zero to max_sigma.
// After plateauing at this maximum value, the filter's width linearly
// ramps back down to zero at the end of the data set.  This method
// overwrites the raw positions within UVW with their filtered
// counterparts.

void FeaturesGroup::temporally_filter_feature(
   int feature_ID,int zeroth_imagenumber,vector<threevector>& UVW)
{
   const double dt=1.0;
   const int max_n_sigma=10;
   const double max_sigma=max_n_sigma*dt;
   const double e_folding_distance=3;
   int max_n_size=filterfunc::gaussian_filter_size(
      max_sigma,dt,e_folding_distance);
   double *filter=new double[max_n_size];
   double *velocity_filter=new double[max_n_size];

   vector<threevector> filtered_posn,filtered_velocity;
   filtered_posn.reserve(UVW.size());
   filtered_velocity.reserve(UVW.size());

   int prev_n_size=-1;
   for (unsigned int counter=0; counter<UVW.size(); counter++)
   {

// To avoid divizion-by-zero problems, do not filter the first and
// last few UVW threevectors:

      if (counter < 2 || counter > UVW.size()-1-2)
      {
         filtered_posn.push_back(UVW[counter]);
         filtered_velocity.push_back(threevector(0,0,0));
      }
      else
      {
         double sigma=basic_math::min(
            max_sigma,(counter-1)*dt/(e_folding_distance*SQRT_TWO),
            (UVW.size()-counter-1)*dt/(e_folding_distance*SQRT_TWO));
         int n_size=filterfunc::gaussian_filter_size(
            sigma,dt,e_folding_distance);

// Don't waste time recomputing gaussian filter unless its width has
// changed:

         if (n_size != prev_n_size)
         {
            filterfunc::gaussian_filter(n_size,0,sigma,dt,filter);
            filterfunc::gaussian_filter(n_size,1,sigma,dt,velocity_filter);
            prev_n_size=n_size;
         }
               
//         cout << "image = " << counter
//              << " n_size=" << n_size 
//              << " max_n_size=" << max_n_size
//              << " sigma=" << sigma
//              << " max_sigma=" << max_sigma
//              << endl;

         int w=n_size/2;
         double filter_sum=0;
         double velocity_filter_sum=0;
         threevector posn(0,0,0);
         threevector velocity(0,0,0);
         for (int i=0; i<n_size; i++)
         {
            int j=counter-w+i;
            if (j >= 0 && j < int(UVW.size()))
            {
               posn += UVW[j]*filter[i]*dt;
               filter_sum += filter[i]*dt;
               velocity += UVW[j]*velocity_filter[i]*dt;
               velocity_filter_sum += velocity_filter[i]*dt;
            }
         } 
         posn /= filter_sum;
         filtered_posn.push_back(posn);

// Recall velocity_filter_sum should be very close to zero.  So we
// should NOT attempt to renormalize the smoothed velocity vector by
// dividing by velocity_filter_sum!

//	 velocity /= velocity_filter_sum;

         filtered_velocity.push_back(velocity);
         
//         for (unsigned int i=0; i<n_size; i++)
//         {
//            cout << "i = " << i << " filter = " << filter[i]
 //                << " velocity_filter = " << velocity_filter[i] << endl;
 //        }
//         cout << "filter_sum = " << filter_sum 
//              << " velocity_filter_sum = " << velocity_filter_sum
//              << endl;
//         outputfunc::enter_continue_char();
         
      } // counter < max_n_sigma || counter > UVW.size()-1-max_n_sigma
	//  conditional

      double feature_speed=sqrt(sqr(filtered_velocity.back().get(0))+
         sqr(filtered_velocity.back().get(1)));

      if (feature_speed > 0.0005)
      {
//         cout << "feature = " << feature_ID
//              << " image = " << counter+zeroth_imagenumber
//              << " U = " << UVW[counter].get(0)
//                 << " U' = " << filtered_posn.back().get(0)
//              << " V = " << UVW[counter].get(1)
//                 << " V' = " << filtered_posn.back().get(1)
//                 << " v_U = " << filtered_velocity.back().get(0)
//                 << " v_V = " << filtered_velocity.back().get(1)
//              << " speed = " << feature_speed
//              << endl;

         int imagenumber=counter+zeroth_imagenumber;
         double t=static_cast<double>(imagenumber);
         erase_Graphical(t,feature_ID);
      }

   } // loop over counter index
   delete [] filter;
   delete [] velocity_filter;

// Overwrite raw positions in UVW STL vector with filtered ones:

   for (unsigned int counter=0; counter < filtered_posn.size(); counter++)
   {
      UVW[counter]=filtered_posn[counter];
   }
}

// --------------------------------------------------------------------------
// Member function stabilize_video_imagery

void FeaturesGroup::stabilize_video_imagery()
{   
   stabilize_imagery_flag=!stabilize_imagery_flag;
   if (stabilize_imagery_flag)
   {
      cout << "Video imagery will be coarsely stabilized" << endl;
   }
   else
   {
      cout << "Video imagery will flow freely" << endl;
   }
}

// -------------------------------------------------------------------------
// Utility member function KLT_wander_test was written to determine a
// reasonable quantitative estimate for how much KLT features wander
// over the course of the HAFB pass.  It is a special-purpose method
// which is only meant to be run once on a manually selected sample of
// KLT features contained in "marked_features.txt" (as of 12/28/05).

void FeaturesGroup::KLT_wander_test()
{
   outputfunc::write_banner("Testing KLT feature wander");
   
   const unsigned int start_imagenumber=57;
   const unsigned int stop_imagenumber=250;
   const unsigned int n_images=stop_imagenumber-start_imagenumber+1;
   genmatrix Ucoord(n_images,get_n_Graphicals());
   genmatrix Vcoord(n_images,get_n_Graphicals());
   
   for (unsigned int imagenumber=start_imagenumber; 
        imagenumber <=stop_imagenumber; imagenumber++)
   {
      double t=static_cast<double>(imagenumber);
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         Feature* Feature_ptr=get_Feature_ptr(n);
         threevector curr_UVW;
         Feature_ptr->get_UVW_coords(t,get_passnumber(),curr_UVW);
         Ucoord.put(imagenumber-start_imagenumber,n,curr_UVW.get(0));
         Vcoord.put(imagenumber-start_imagenumber,n,curr_UVW.get(1));
      } // loop over index n labeling features
   } // loop over imagenumber

// Renormalize all KLT features' UV coordinates for image m relative
// to UV coordinates for KLT feature #0.  This subtraction implements
// a poor-man's stabilization:

   for (unsigned int m=0; m<Ucoord.get_mdim(); m++)
   {
      double U0=Ucoord.get(m,0);
      double V0=Vcoord.get(m,0);
      for (unsigned int n=0; n<Ucoord.get_ndim(); n++)
      {
         double Un=Ucoord.get(m,n);
         double Vn=Vcoord.get(m,n);
         Ucoord.put(m,n,Un-U0);
         Vcoord.put(m,n,Vn-V0);
      } // loop over index n
   } // loop over index m 

// Next extract deviations of renormalized UV coordinates about their
// renormalized means:

   vector<double> dU,dV;
   for (unsigned int n=1; n<Ucoord.get_ndim(); n++)
   {
      for (unsigned int m=1; m<Ucoord.get_mdim(); m++)
      {
          dU.push_back(Ucoord.get(m,n)-Ucoord.get(m-1,n));
          dV.push_back(Vcoord.get(m,n)-Vcoord.get(m-1,n));
//          dU.push_back(Ucoord.get(m,n)-Ucoord.get(0,n));
//          dV.push_back(Vcoord.get(m,n)-Vcoord.get(0,n));
//          cout << "feature = " << n << " imagenumber = " << m 
//               << " dU = " << dU << " dV = " << dV << endl;
      } // loop over index m 
   } // loop over index n   

   prob_distribution probU(dU,50);
   probU.set_densityfilenamestr("U_dev.meta");
   probU.set_xlabel("U deviations (meters)");
   probU.write_density_dist();

   prob_distribution probV(dV,50);
   probV.set_densityfilenamestr("V_dev.meta");
   probV.set_xlabel("V deviations (meters)");
   probV.write_density_dist();
}

// -------------------------------------------------------------------------
// Member function find_zero_value_feature loops over every image
// within *VidFile_ptr.  It checks whether the input *Feature_ptr is
// unerased within each image.  If so, this method next fetches the
// greyscale intensity value located at the feature's UV position.  If
// the intensity == 0, we reset it erasure flag to true.  We
// implemented this member function to help erase features with
// interpolated positions coming from automatic KLT tracking that lie
// outside the active area of a greyscale image.

void FeaturesGroup::find_zero_value_feature(
   VidFile* VidFile_ptr,unsigned char* data_ptr,Feature* Feature_ptr)
{   
   for (unsigned int imagenumber=get_first_framenumber(); 
        imagenumber <= get_last_framenumber(); imagenumber++)
   {
      double curr_t=static_cast<double>(imagenumber);

      threevector UVW;
      if (Feature_ptr->get_UVW_coords(curr_t,get_passnumber(),UVW)
          && !Feature_ptr->get_mask(curr_t,get_passnumber()))
      {
         pair<int,int> p=Movie_ptr->get_pixel_coords(
            UVW.get(0),UVW.get(1));
         int px=p.first;
         int py=p.second;
//         int py=(VidFile_ptr->getHeight()-1)-p.second;
         VidFile_ptr->read_image(imagenumber,data_ptr);
         if (VidFile_ptr->pixel_greyscale_intensity_value(px,py,data_ptr)==0)
         {
            cout << "In imagenumber " << imagenumber 
                 << " feature ID = " << Feature_ptr->get_ID() 
                 << " has zero intensity" << endl;
            Feature_ptr->set_mask(curr_t,get_passnumber(),true);
         }
      }  // feature not erased conditional
   } // loop over imagenumber
}

// ==========================================================================
// Triangulation methods
// ==========================================================================

// Member function Delaunay_triangulate copies every feature into an
// STL vector.  Using this vector as input, it computes a list of
// Delaunay triangles and writes every one of their edges to output
// ascii file "Delaunay.txt".  The format for the output file is
// compatible with LineSegmentsGroup::read_info_from_file() which can
// be used to later read back and display the Delaunay triangles.

void FeaturesGroup::Delaunay_triangulate()
{   
//   cout << "inside FeaturesGroup::Delaunay_triangulate(), TriGroup_ptr = "
//        << TrianglesGroup_ptr << endl;

   if (TrianglesGroup_ptr==NULL)
   {
      cout << "Error in FeaturesGroup::Delaunay_triangulate()!" << endl;
      cout << "TrianglesGroup_ptr=NULL ! " << endl;
      exit(-1);
   }
   
   outputfunc::write_banner("Triangulating features:");

   threevector UVW;
   vector<int> feature_ID;
   vector<threevector> feature_vertices;

   vector<double> curr_time;
   vector<int> ID,pass_number;
   vector<threevector> V1,V2,V3;
   vector<int> vertex1_ID,vertex2_ID,vertex3_ID;
   vector<colorfunc::Color> color;
   vector<Triple<int,int,int> > triangles;

   for (unsigned int imagenumber=get_first_framenumber(); 
        imagenumber <= get_last_framenumber(); imagenumber++)
   {
      cout << imagenumber << " " << flush;
      double curr_t=static_cast<double>(imagenumber);

      feature_ID.clear();
      feature_vertices.clear();
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         Feature* Feature_ptr=get_Feature_ptr(n);
         if (!Feature_ptr->get_mask(curr_t,get_passnumber()) &&
             Feature_ptr->get_UVW_coords(curr_t,get_passnumber(),UVW))
         {
            feature_ID.push_back(Feature_ptr->get_ID());
            feature_vertices.push_back(
               threevector(UVW.get(0),UVW.get(1),UVW.get(2)) );
         }
      } // loop over index n labeling feature 
   
      triangles.clear();

      Delaunay_tree DT(feature_ID,&feature_vertices);
      triangles=DT.compute_triangles();

      for (unsigned int i=0; i<triangles.size(); i++)
      {
         Triple<int,int,int> curr_triangle=triangles[i];
         vertex1_ID.push_back(curr_triangle.first);
         vertex2_ID.push_back(curr_triangle.second);
         vertex3_ID.push_back(curr_triangle.third);

         int index1=containerfunc::locate_vector_index(
            feature_ID,curr_triangle.first);
         int index2=containerfunc::locate_vector_index(
            feature_ID,curr_triangle.second);
         int index3=containerfunc::locate_vector_index(
            feature_ID,curr_triangle.third);

         curr_time.push_back(curr_t);
         string ID_string=stringfunc::number_to_string(curr_triangle.third)
            +stringfunc::number_to_string(curr_triangle.second)
            +stringfunc::number_to_string(curr_triangle.first);

//         cout << "Triangle # = " << i << endl;
//         cout << "first = " << curr_triangle.first
//              << " second = " << curr_triangle.second
//              << " third = " << curr_triangle.third
//              << " ID_string = " << ID_string << endl;
         ID.push_back(basic_math::round(stringfunc::string_to_number(
            ID_string)));
//         cout << "ID = " << ID.back() << endl;
         pass_number.push_back(get_passnumber());

         threevector v1(feature_vertices[index1]);
         threevector v2(feature_vertices[index2]);
         threevector v3(feature_vertices[index3]);

         if (get_ndims()==2)
         {
            V1.push_back(threevector(v1.get(0),0,v1.get(1)));
            V2.push_back(threevector(v2.get(0),0,v2.get(1)));
            V3.push_back(threevector(v3.get(0),0,v3.get(1)));
         }
         else if (get_ndims()==3)
         {
            V1.push_back(v1);
            V2.push_back(v2);
            V3.push_back(v3);
         }
         
         int randomint=modulo(mathfunc::digit_sum(ID.back()),12);
         color.push_back(colorfunc::get_color(randomint));

      } // loop over index i labeling triangles at t=curr_time
   } // loop over imagenumber
   cout << endl;

   TrianglesGroup_ptr->regenerate_triangles(
      curr_time,ID,pass_number,V1,vertex1_ID,V2,vertex2_ID,V3,vertex3_ID,
      color);
   TrianglesGroup_ptr->generate_triangles_network(ID);
   TrianglesGroup_ptr->color_triangles();
   TrianglesGroup_ptr->update_display();
}

// ==========================================================================
// Feature scoring methods
// ==========================================================================

// Member function compute_features_scores_and_adjust_their_locations
// starts with the video image in the middle of a pass.  For each
// feature in that image, it computes a score based upon the product
// of the intensity gradient and laplacian at pixels located within a
// small neighborhood about the feature's position.  The feature's
// position is then reset to the pixel where the score function is
// maximized.  The search neighborhood for subsequent images (in both
// the increasing and decreasing image number directions) are centered
// upon their predecessors' best-fit positions.  We also reduce the
// search area after the best-fit location in the mid-pass image has
// been found.

void FeaturesGroup::compute_features_scores_and_adjust_their_locations()
{
   outputfunc::write_banner("Computing feature scores:");

   int imagenumber_start=get_Nimages()/2;
   int imagenumber_stop=get_Nimages();
   int delta_imagenumber=1;
   int dp_start=4;
   int dp_subsequent=2;

   compute_features_scores_and_adjust_their_locations(
      imagenumber_start,imagenumber_stop,delta_imagenumber,
      dp_start,dp_subsequent);

   imagenumber_start--;
   imagenumber_stop=0;
   delta_imagenumber=-1;
   dp_start=dp_subsequent;
   compute_features_scores_and_adjust_their_locations(
      imagenumber_start,imagenumber_stop,delta_imagenumber,
      dp_start,dp_subsequent);
}

void FeaturesGroup::compute_features_scores_and_adjust_their_locations(
   unsigned int imagenumber_start,unsigned int imagenumber_stop,
   unsigned int delta_imagenumber,
   int dp_start,int dp_subsequent)
{   
   
// First set up Gaussian first and second derivative filters:

   const double dx=1.0;
//   const double spatial_resolution=0.25*dx;
   const double spatial_resolution=0.35*dx;
//   const double spatial_resolution=0.5*dx;
   int n_size=filterfunc::gaussian_filter_size(spatial_resolution,dx);

   double *deriv_filter=new double[n_size];
   double *second_deriv_filter=new double[n_size];
   filterfunc::gaussian_filter(n_size,1,spatial_resolution,dx,deriv_filter);
   filterfunc::gaussian_filter(
      n_size,2,spatial_resolution,dx,second_deriv_filter);

   cout << "n_size = " << n_size << endl;
//   for (unsigned int n=0; n<n_size; n++)
//   {
//      cout << "n = " << n << " deriv_filter = " << deriv_filter[n]
//           << endl;
//   }

   VidFile* VidFile_ptr=Movie_ptr->get_VidFile_ptr();
   VidFile_ptr->generate_intensity_twoDarray();

   for (unsigned int imagenumber=imagenumber_start; 
        imagenumber != imagenumber_stop; 
        imagenumber += delta_imagenumber)
   {
      cout << imagenumber << " " << flush;

      int dp=dp_subsequent;
      if (imagenumber==imagenumber_start)
      {
         dp=dp_start;
      }

      Movie_ptr->displayFrame(imagenumber);

      double curr_t=static_cast<double>(imagenumber);
      double delta_t=static_cast<double>(delta_imagenumber);

      VidFile_ptr->convert_charstar_array_to_intensity_twoDarray(
         Movie_ptr->get_m_image_ptr());
      twoDarray* intensity_twoDarray_ptr=VidFile_ptr->
         get_intensity_twoDarray_ptr();
      unsigned int mdim=intensity_twoDarray_ptr->get_mdim();
      unsigned int ndim=intensity_twoDarray_ptr->get_ndim();

      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         Feature* Feature_ptr=get_Feature_ptr(n);
//         int feature_ID=Feature_ptr->get_ID();

         threevector UVW;
         if (Feature_ptr->get_UVW_coords(curr_t,get_passnumber(),UVW)
             && !Feature_ptr->get_mask(curr_t,get_passnumber()))
         {
            pair<int,int> p=Movie_ptr->get_pixel_coords(
               UVW.get(0),UVW.get(1));
            unsigned int px_init=p.first;
            unsigned int py_init=(VidFile_ptr->getHeight()-1)-p.second;

            unsigned int px_best=0;
            unsigned int py_best=0;
            double best_score=NEGATIVEINFINITY;
            for (unsigned int px=basic_math::max(Unsigned_Zero,px_init-dp); 
                 px<basic_math::min(mdim-1,px_init+dp); px++)
            {
               for (unsigned int py=basic_math::max(Unsigned_Zero,py_init-dp); 
                    py<basic_math::min(ndim-1,py_init+dp); py++)
               {
                  double curr_score=evaluate_feature_score(
                     px,py,n_size,deriv_filter,second_deriv_filter,
                     VidFile_ptr->get_intensity_twoDarray_ptr());
                  if (curr_score > best_score)
                  {
                     best_score=curr_score;
                     px_best=px;
                     py_best=py;
                  }
               } // loop over py
            } // loop over px

            Feature_ptr->set_score(curr_t,get_passnumber(),best_score);

            int flipped_py_best=(VidFile_ptr->getHeight()-1)-py_best;
            pair<double,double> p_uv=Movie_ptr->
               get_uv_coords(px_best,flipped_py_best);
            threevector UVW_new(p_uv.first,p_uv.second,0);
            threevector delta_UVW=UVW_new-UVW;
            Feature_ptr->adjust_UVW_coords(curr_t,get_passnumber(),delta_UVW);
            Feature_ptr->adjust_UVW_coords(
               curr_t+delta_t,get_passnumber(),delta_UVW);
            
//            cout << "image = " << imagenumber 
//                 << " feature = " << feature_ID  
//                 << " dpx = " << px_best-px_init
//                 << " dpy = " << py_best-py_init
//                 << " score = " << best_score << endl;

//            cout << "UVW = " << UVW << endl;
//            cout << "new UVW = " << uv_new << endl;
            
         } // feature not erased conditional
      } // loop over index n labeling features

   } // loop over imagenumber index


   outputfunc::newline();

   delete [] deriv_filter;
   delete [] second_deriv_filter;
   VidFile_ptr->delete_intensity_twoDarray();
}

// --------------------------------------------------------------------------
// Member function evaluate_feature_score computes the gradient and
// laplacian at the location within *intensity_twoDarray_ptr specified
// by input pixel coordinates (px,py).  The best 2D features to track
// are those which have both large absolute values for the squared
// gradient magnitude and laplacian (= divergence of gradient flow
// field).  So we take the score function to depend upon their
// product.

double FeaturesGroup::evaluate_feature_score(
   unsigned int px,unsigned int py,unsigned int n_size,
   double deriv_filter[],double second_deriv_filter[],
   twoDarray const *intensity_twoDarray_ptr)
{   
   const double null_value=NEGATIVEINFINITY;

   double grad_x=imagefunc::horiz_derivative_filter(
      px,py,n_size,deriv_filter,intensity_twoDarray_ptr,null_value);
   double grad_y=imagefunc::vert_derivative_filter(
      px,py,n_size,deriv_filter,intensity_twoDarray_ptr,null_value);
   double gradient_sqr_mag=sqr(grad_x)+sqr(grad_y);

   double grad_xx=imagefunc::horiz_derivative_filter(
      px,py,n_size,second_deriv_filter,intensity_twoDarray_ptr,null_value);
   double grad_yy=imagefunc::vert_derivative_filter(
      px,py,n_size,second_deriv_filter,intensity_twoDarray_ptr,null_value);

   double laplacian=grad_xx+grad_yy;

   double score=log10(gradient_sqr_mag*fabs(laplacian));
   return score;
}

// ==========================================================================
// Homography methods
// ==========================================================================

// Member function common_unerased_feature_IDs takes in two times t1
// and t2.  It also takes in a max_feature_ID which can be used to
// distinguish KLT features from new, non-KLT features.  This method
// returns an STL vector containing feature IDs which are unerased at
// both input times.  Such features can be used to develop
// homographies between video images at the two separate input times.

vector<int> FeaturesGroup::common_unerased_feature_IDs(double t1,double t2)
{
   vector<int> common_feature_ID;
   vector<int> uncommon_feature_ID;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Feature* Feature_ptr=get_Feature_ptr(n);
      int feature_ID=Feature_ptr->get_ID();
      if (!Feature_ptr->get_mask(t1,get_passnumber()) &&
          !Feature_ptr->get_mask(t2,get_passnumber()) &&
          feature_ID >= min_unpropagated_feature_ID && 
          feature_ID <= max_unpropagated_feature_ID)
      {
         common_feature_ID.push_back(feature_ID);
      }
      else
      {
         uncommon_feature_ID.push_back(feature_ID);
      }
   } // loop over index n labeling features 

//   cout << "Features NOT both appearing at t = " << t1 
//        << " and t = " << t2 << endl;
//   templatefunc::printVector(common_feature_ID);
//   templatefunc::printVector(uncommon_feature_ID);
   return common_feature_ID;
}

// -------------------------------------------------------------------------
// Member function generate_feature_kdtree takes in two image times t1
// and t2.  It first finds the IDs for all features which are unerased
// in both images.  It then destroys any existing KDtree and generates
// and a new one using the UV coordinates of the time t1 features.
// Feature IDs are stored in the 3rd entry of each threevector within the
// KDtree.

void FeaturesGroup::generate_feature_kdtree(double t1,double t2)
{
   vector<int> common_feature_ID=common_unerased_feature_IDs(t1,t2);
   vector<threevector> V;

   threevector UVW;
   for (unsigned int n=0; n<common_feature_ID.size(); n++)
   {
      Feature* Feature_ptr=get_ID_labeled_Feature_ptr(common_feature_ID[n]);
      Feature_ptr->get_UVW_coords(t1,get_passnumber(),UVW);
//      cout << "n = " << n << " ID = " << common_feature_ID[n]
//           << " UVW = " << UVW << endl;
      V.push_back(threevector(UVW.get(0),UVW.get(1),common_feature_ID[n]));
   }
   
   delete kdtree_ptr;
   kdtree_ptr=kdtreefunc::generate_2D_kdtree(V);
}

// -------------------------------------------------------------------------
// Member function find_nearby_KLT_features searches through
// *kdtree_ptr which is assumed to contain all KLT feature location
// information for time t=curr_t.  It returns the UV locations and IDs
// of the n_closest_nodes KLT features within output STL vector
// closest_curr_node.

void FeaturesGroup::find_nearby_KLT_features(
   const threevector& UV1,vector<threevector>& closest_curr_node)
{
   const double rho=0.2;	// Initial search radius in UV space units
   const int n_closest_nodes=8;

   threevector curr_uv(UV1);
   closest_curr_node.clear();
   kdtreefunc::find_closest_nodes(
      kdtree_ptr,curr_uv,rho,n_closest_nodes,closest_curr_node);
//      cout << "Closest current KLT features:" << endl;
//      templatefunc::printVector(closest_curr_node);
}

// -------------------------------------------------------------------------
// Member function compare_nearby_KLT_features performs a brute force
// comparison between the KLT feature IDs within input STL vectors
// closest_curr_node and closest_orig_node.  It returns the number of
// common IDs existing in both vectors.

int FeaturesGroup::compare_nearby_KLT_features(
   const vector<threevector>& closest_curr_node,
   const vector<threevector>& closest_orig_node)
{
   int n_same_nodes=0;
   for (unsigned int i=0; i<closest_curr_node.size(); i++)
   {
      int curr_ID=basic_math::round(closest_curr_node[i].get(2));
      for (unsigned int j=0; j<closest_orig_node.size(); j++)
      {
         int orig_ID=basic_math::round(closest_orig_node[j].get(2));
         if (curr_ID==orig_ID)
         {
            n_same_nodes++;
            break;
         }
      } // loop over j index
   } // loop over i index
//   cout << "n_same_nodes = " << n_same_nodes << endl;
   return n_same_nodes;
}

// -------------------------------------------------------------------------
// Member function propagate_feature takes in two image times t1 and
// t2 along with an STL vector containing UV coordinates for some set
// of features.  It assumes that a KDtree for unerased KLT features
// existing at both times has already been generated.  For each input
// feature, it searches for close KLT features at time t1.  A
// homography H is subsequently formed from the KLT features'
// positions at t1 and t2.  Homography H is then used to propagate the
// input feature in time from t1 to t2.  This method returns an STL
// vector containing the estimated U'V' coordinates for the input
// features at time t2.

bool FeaturesGroup::propagate_feature(
   double t1,double t2,const threevector& UV1,
   const vector<threevector>& closest_orig_node,threevector& UV2)
{

// First identify small number of KLT features nearby input UV
// position at time t=t1:

   vector<threevector> closest_curr_node;
   find_nearby_KLT_features(UV1,closest_curr_node);

//   cout << "t = " << t1 << " ";
   int n_overlap_nodes=
      compare_nearby_KLT_features(closest_curr_node,closest_orig_node);

   const int min_overlap_nearby_KLT_features=2;
   if (n_overlap_nodes < min_overlap_nearby_KLT_features) return false;

// Next retrieve KLT features' locations at time t=t2:

   vector<threevector> closest_next_node;
//   closest_next_node.clear();
   for (unsigned int n=0; n<closest_curr_node.size(); n++)
   {
      int node_ID=basic_math::round(closest_curr_node[n].get(2));
//         cout << "node ID = " << node_ID << endl;
      Feature* Feature_ptr=get_ID_labeled_Feature_ptr(node_ID);
      threevector next_UVW;
      Feature_ptr->get_UVW_coords(t2,get_passnumber(),next_UVW);
      threevector next_uv(next_UVW.get(0),next_UVW.get(1),node_ID);
      closest_next_node.push_back(next_uv);
   } // loop over index n labeling KLT features nearby UV1 feature
//      cout << "Closest next KLT features:" << endl;
//      templatefunc::printVector(closest_next_node);
      
// Form homography H that relates KLT feature positions at times t1
// and t2:

   homography H;
   H.parse_homography_inputs(closest_curr_node,closest_next_node);
   H.compute_homography_matrix();

// Use homography H to estimate new U'V' position at time t=t2:

   UV2=H.project_world_plane_to_image_plane(UV1);

//      cout << "next_uv = " << UV2.back() << endl;
   return true;
}

// -------------------------------------------------------------------------
// Member function propagate_feature_over_pass

void FeaturesGroup::propagate_feature_over_pass(Feature* Feature_ptr)
{
   string banner="Propagating feature "+stringfunc::number_to_string(
      Feature_ptr->get_ID())+" over entire pass";
   outputfunc::write_banner(banner);

// Recall GraphicalsGroup initialize_Graphical() erases Graphicals for
// all images less than current image.  For propagation purposes, we
// also want to erase new feature for all images greater than current
// image:

   for (unsigned int n=get_curr_framenumber()+1; n <= get_last_framenumber(); n++)
   {
      double curr_t=static_cast<double>(n);
      Feature_ptr->set_mask(curr_t,get_passnumber(),true);
   }

// Use local homographies to propagate new feature forward in time:

   double curr_t=get_curr_t();
   threevector curr_UVW;
   Feature_ptr->get_UVW_coords(curr_t,get_passnumber(),curr_UVW);
   generate_feature_kdtree(curr_t,curr_t);
   vector<threevector> closest_orig_KLT_feature;
   find_nearby_KLT_features(curr_UVW,closest_orig_KLT_feature);

   double next_t=curr_t+1;
   double t_max=double(get_Nimages());
   threevector next_UVW;
   while (next_t < t_max)
   {
      generate_feature_kdtree(curr_t,next_t);
      bool propagation_OK_flag=propagate_feature(
         curr_t,next_t,curr_UVW,closest_orig_KLT_feature,next_UVW);
      Feature_ptr->set_mask(
         next_t,get_passnumber(),!propagation_OK_flag);
      Feature_ptr->set_UVW_coords(next_t,get_passnumber(),next_UVW);

      curr_UVW=next_UVW;
      curr_t=next_t;
      next_t += 1;
   }

// Use local homographies to propagate new feature backward in time:

   curr_t=get_curr_t();
   Feature_ptr->get_UVW_coords(curr_t,get_passnumber(),curr_UVW);

   double prev_t=curr_t-1;
   double t_min=0;
   threevector prev_UVW;
   while (prev_t >= t_min)
   {
      generate_feature_kdtree(curr_t,prev_t);
      bool propagation_OK_flag=propagate_feature(
         curr_t,prev_t,curr_UVW,closest_orig_KLT_feature,prev_UVW);
      Feature_ptr->set_mask(
         prev_t,get_passnumber(),!propagation_OK_flag);
      Feature_ptr->set_UVW_coords(prev_t,get_passnumber(),prev_UVW);

      curr_UVW=prev_UVW;
      curr_t=prev_t;
      prev_t -= 1;
   }
}
          
  
// ==========================================================================
// 2D parallelogram methods
// ==========================================================================

// Member function fit_features_to_parallelogram performs a best
// parallelogram fit to the first 4 features within the current
// FeaturesGroup object.  It draws the fitted parallelogram via 4
// LineSegments within *LineSegmentsGroup_ptr.

void FeaturesGroup::fit_features_to_parallelogram()
{
   outputfunc::write_banner("Fitting first 4 features to a parallelogram:");

   threevector UVW;
   vector<Feature*> Feature_ptrs;
   vector<threevector> vertex_posn;
   for (unsigned int ID=0; ID<4; ID++)
   {
      Feature_ptrs.push_back(get_ID_labeled_Feature_ptr(ID));
      Feature_ptrs.back()->get_UVW_coords(get_curr_t(),get_passnumber(),UVW);
      vertex_posn.push_back(UVW);
   } // loop over first 4 feature ID's

   parallelogram p(vertex_posn);
//   cout << "parallelogram = " << p << endl;

   if (LineSegmentsGroup_ptr==NULL)
   {
      cout << "Error in FeaturesGroup::fit_features_to_parallelogram()" 
           << endl;
      cout << "LineSegmentsGroup_ptr=NULL" << endl;
      exit(-1);
   }
   unsigned int n_segments=LineSegmentsGroup_ptr->get_n_Graphicals();
   for (unsigned int i=0; i<4-n_segments; i++)
   {
      LineSegmentsGroup_ptr->generate_new_canonical_LineSegment(-1,false);
   }
   
   for (unsigned int n=0; n<4; n++)
   {
      LineSegment* LineSegment_ptr=LineSegmentsGroup_ptr->
         get_LineSegment_ptr(n);
      LineSegment_ptr->set_curr_color(colorfunc::white);
      LineSegment_ptr->set_permanent_color(colorfunc::white);

      threevector V1(p.get_vertex(n));
      threevector V2(p.get_vertex(modulo(n+1,4)));
      LineSegment_ptr->set_scale_attitude_posn(
         LineSegmentsGroup_ptr->get_curr_t(),
         LineSegmentsGroup_ptr->get_passnumber(),V1,V2);
   } // loop over index n labeling parallelogram line segments
}

// ==========================================================================
// Ray backprojection member functions:
// ==========================================================================

// Member function backproject_rays takes in time t and some feature
// ID along with internal camera parameters for multiple video passes.
// It reconstructs the rays in world-space for each individual photo
// and then averages together their phi and theta angles.  The
// averaged ray's world coordinates are stored as instantaneous
// observation number n_video_passes.

void FeaturesGroup::backproject_rays(
   double t,int feature_ID,OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr,
   unsigned int n_video_passes)
{
//   cout << "inside FeaturesGroup::backproject_rays()" << endl;
//   cout << "t = " << t 
//        << " feature_ID = " << feature_ID << endl;

   Feature* Feature_ptr=get_ID_labeled_Feature_ptr(feature_ID);
   if (Feature_ptr==NULL) return;

   instantaneous_obs* obs_ptr=
      Feature_ptr->get_all_particular_time_observations(t);

   vector<double> phis,thetas;
   for (unsigned int p=0; p<n_video_passes; p++)
   {
      if (obs_ptr->check_for_pass_entry(p))
      {
         twovector curr_UV=obs_ptr->retrieve_UVW_coords(p);
//         cout << "ID = " << Feature_ptr->get_ID()
//              << " UV = " << curr_UV.get(0) << " , " 
//              << curr_UV.get(1) << endl;

         camera* camera_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(p)->
            get_Movie_ptr()->get_camera_ptr();
         threevector ray_hat=camera_ptr->pixel_ray_direction(curr_UV);
         double curr_phi,curr_theta;
         mathfunc::decompose_direction_vector(ray_hat,curr_phi,curr_theta);
         phis.push_back(curr_phi);
         thetas.push_back(curr_theta);
//         cout << "p = " << p
//              << " phi = " << curr_phi*180/PI
//              << " theta = " << curr_theta*180/PI << endl;
      }  // check_for_pass_entry conditional
   } // loop over index p labeling passes

   double avg_phi=mathfunc::mean(phis);
   double avg_theta=mathfunc::mean(thetas);
   
//   cout << "n_obs = "<< phis.size()
//        << " avg_phi = " << avg_phi*180/PI
//        << " avg_theta = " << avg_theta*180/PI << endl;
   threevector ray_hat_avg=
      mathfunc::construct_direction_vector(avg_phi,avg_theta);
//   cout << "avg ray hat = " 
//        << ray_hat_avg.get(0) << " , "
//        << ray_hat_avg.get(1) << " , "
//        << ray_hat_avg.get(2) << endl;

// Add averaged ray direction vector's world-space coordinates as
// final instantaneous observation within *Feature_ptr:

   Feature_ptr->set_UVW_coords(t,n_video_passes,ray_hat_avg);
}

// -------------------------------------------------------------------------
// Member function backproject_2D_features_into_3D() first retrieves
// the projection matrix corresponding to the current video frame
// number.  It then backprojects every 2D feature which has been
// manually picked and finds a corresponding 3D worldspace point in a
// ladar cloud.  A new, stationary 3D feature within
// *FeaturesGroup_3D_ptr is instantiated.  The 3D feature is
// subsequently reprojected back into the 2D UV image plane for all
// dynamic video frames to guarantee a valid relationship between 3D
// and 2D feature tiepoints.  Hopefully, the reprojected UV
// coordinates are close to the initially selected UV coordinates for
// the 2D feature.  The 2D feature is annotated with its 3D
// counterpart's range and altitude.

bool FeaturesGroup::backproject_2D_features_into_3D()
{
//   cout << "Inside FeaturesGroup::backproject_2D_features_into_3D()" << endl;

   if (MoviesGroup_ptr==NULL) return false;
   if (PointFinder_ptr==NULL) return false;
   if (FeaturesGroup_3D_ptr==NULL) return false;
   if (EarthRegionsGroup_ptr==NULL) return false;
   if (get_ndims() != 2) return false;

   unsigned n_2D_Features=get_n_Graphicals();
//   cout << "n_2D_Features = " << n_2D_Features << endl;
   if (n_2D_Features==0) return false;

// As of 2/16/09, we assume that *MoviesGroup_ptr contains just a
// single video clip:

   Movie* Movie_ptr=dynamic_cast<Movie*>(
      MoviesGroup_ptr->get_last_Graphical_ptr());
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   threevector camera_posn=MoviesGroup_ptr->
      get_static_camera_posn_offset();
//   cout << "camera_posn = " << camera_posn << endl;

   double frustum_sidelength;
   genmatrix raw_P(3,4),filtered_P(3,4);

   if (!import_package_params(
          get_curr_framenumber(),frustum_sidelength,raw_P)) return false;
//   cout << "raw_P = " << raw_P << endl;

// Temporally filter projection matrices prior to projecting 3D
// features onto 2D counterparts for UV feature tracking purposes:

   double alpha=0.38;
   bool temporally_filter_flag=true;
   filtered_P=raw_P;
   camera_ptr->set_projection_matrix(filtered_P,alpha,temporally_filter_flag);

// Loop over every 2D feature and backproject it into the 3D world
// space.

   for (unsigned int f=0; f<n_2D_Features; f++)
   {
      Feature* Feature_ptr=get_Feature_ptr(f);
      Feature_ptr->set_stationary_Graphical_flag(false);

      Feature* Feature_3D_ptr=FeaturesGroup_3D_ptr->
         get_ID_labeled_Feature_ptr(Feature_ptr->get_ID());
      if (Feature_3D_ptr != NULL)
      {

// If a stationary 3D feature counterpart to a previously picked 2D
// feature exists, project 3D feature's XYZ coordinates into current
// UV image plane to determine 2D feature's updated coordinates:

         threevector XYZ,UVW;
         Feature_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW);
         Feature_3D_ptr->get_UVW_coords(
            FeaturesGroup_3D_ptr->get_curr_t(),
            FeaturesGroup_3D_ptr->get_passnumber(),XYZ);
         threevector rel_XYZ=XYZ-camera_posn;
         camera_ptr->project_XYZ_to_UV_coordinates(rel_XYZ,UVW);
//         cout << "f = " << f 
//              << " curr_t = " << get_curr_t()
//              << " passnumber = " << get_passnumber()
 //             << " XYZ = " << XYZ 
//              << " rel_XYZ = " << rel_XYZ 
//              << " new projected UVW = " << UVW << endl;
         Feature_ptr->set_UVW_coords(get_curr_t(),get_passnumber(),UVW);

// Mask any 2D feature whose coordinates lie outside movie's allowed
// UV extents:

         bool mask_flag=false;
         if (UVW.get(0) < Movie_ptr->get_minU() ||
             UVW.get(0) > Movie_ptr->get_maxU() ||
             UVW.get(1) < Movie_ptr->get_minV() ||
             UVW.get(1) > Movie_ptr->get_maxV())
         {
            mask_flag=true;
         }
         Feature_ptr->set_mask(get_curr_t(),get_passnumber(),mask_flag);
         continue;
      }
    
      threevector UVW;
      if (!Feature_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW))
      {
         continue;
      }

// As of 2/22/09, we believe that our naive temporal filtering of
// individual matrix elements within the raw 3x4 projection matrices
// leads to sizable backprojection inaccuracies.  In the future, we
// should try to temporally filter the physical angles, focal params,
// etc.  But for now, we work with raw projection matrices to avoid
// bad backprojection inaccuracies:

      bool temporally_filter_flag=false;
      camera_ptr->set_projection_matrix(raw_P,alpha,temporally_filter_flag);

      threevector ray_hat=camera_ptr->pixel_ray_direction(twovector(UVW));
//      cout << "f = " << f 
//           << " n_2D_Features = " << n_2D_Features 
//           << " UV = " << twovector(UVW) 
//           << " ray = " << ray_hat << endl;

      threevector closest_worldspace_point;
      geopoint closest_geopoint;
      if (PointFinder_ptr->find_smallest_relative_angle_world_point(
             camera_posn,ray_hat,closest_worldspace_point))
      {
         cout << "3D counterpart found" << endl;
         EarthRegion* EarthRegion_ptr=EarthRegionsGroup_ptr->
            get_EarthRegion_ptr(0);
         closest_geopoint=geopoint(
            EarthRegion_ptr->get_northern_hemisphere_flag(),
            EarthRegion_ptr->get_UTM_zonenumber(),
            closest_worldspace_point.get(0),
            closest_worldspace_point.get(1),
            closest_worldspace_point.get(2));
      }
      else
      {
         cout << "No 3D counterpart point located!" << endl;
      }
      
      Feature_3D_ptr=FeaturesGroup_3D_ptr->
         generate_new_Feature(Feature_ptr->get_ID());

      cout << "closest geopoint = " << closest_geopoint << endl;

      Feature_3D_ptr->set_UVW_coords(
         FeaturesGroup_3D_ptr->get_curr_t(),
         FeaturesGroup_3D_ptr->get_passnumber(),closest_worldspace_point);
      Feature_3D_ptr->set_blinking_color(colorfunc::black);

      const double blink_period=10;	// secs
      FeaturesGroup_3D_ptr->blink_Geometrical(
         Feature_3D_ptr->get_ID(),blink_period);

      string label=update_feature_label(
         closest_worldspace_point,camera_posn);
      Feature_ptr->set_text_label(1,label);

   } // loop over index f labeling 2D features

   return true;
}

// -------------------------------------------------------------------------
// Member function backproject_selected_photo_features_into_3D() first
// retrieves the projection matrix corresponding to the current video
// frame number.  It then backprojects every 2D feature which has been
// manually picked and finds a corresponding 3D worldspace point in a
// ladar cloud.  A new, stationary 3D feature within
// *FeaturesGroup_3D_ptr is instantiated.  The 3D feature is
// subsequently reprojected back into the 2D UV image plane for all
// dynamic video frames to guarantee a valid relationship between 3D
// and 2D feature tiepoints.  Hopefully, the reprojected UV
// coordinates are close to the initially selected UV coordinates for
// the 2D feature.  The 2D feature may be annotated with its 3D
// counterpart's lon, lat, alt and range.

bool FeaturesGroup::backproject_selected_photo_features_into_3D()
{
//   cout << "Inside FeaturesGroup::backproject_selected_photo_features_into_3D()" 
//        << endl;

   if (photogroup_ptr==NULL) return false;
   if (PointFinder_ptr==NULL) return false;
   if (FeaturesGroup_3D_ptr==NULL) return false;
   if (EarthRegionsGroup_ptr==NULL) return false;
   if (get_ndims() != 2) return false;

   unsigned int n_2D_Features=get_n_Graphicals();
//   cout << "n_2D_Features = " << n_2D_Features << endl;
   if (n_2D_Features==0) return false;

   int photo_ID=photogroup_ptr->get_selected_photo_ID();
   if (photo_ID < 0) photo_ID=0;
   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(photo_ID);
//   cout << "photo_ID = " << photo_ID << endl;

   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   threevector camera_posn=camera_ptr->get_world_posn();
//   cout << "camera_posn = " << camera_posn << endl;

// Loop over every 2D feature and backproject it into the 3D world
// space.

   vector<int> IDs_of_features_to_delete;
   for (unsigned int f=0; f<n_2D_Features; f++)
   {
      bool continue_flag=false;
      Feature* Feature_ptr=get_Feature_ptr(f);
      Feature_ptr->set_stationary_Graphical_flag(false);

      Feature* Feature_3D_ptr=FeaturesGroup_3D_ptr->
         get_ID_labeled_Feature_ptr(Feature_ptr->get_ID());

      if (Feature_3D_ptr != NULL)
      {

// If a stationary 3D feature counterpart to a previously picked 2D
// feature exists, project 3D feature's XYZ coordinates into current
// UV image plane to determine 2D feature's updated coordinates:

         threevector XYZ,UVW;
         Feature_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW);
         Feature_3D_ptr->get_UVW_coords(
            FeaturesGroup_3D_ptr->get_curr_t(),
            FeaturesGroup_3D_ptr->get_passnumber(),XYZ);

         threevector ray_hat=(camera_posn-XYZ).unitvector();

         bool feature_occluded_flag=false;
         threevector impact_point;
         if (DTED_ztwoDarray_ptr != NULL)
         {
            feature_occluded_flag=camera_ptr->trace_individual_ray(
               XYZ,ray_hat,minimum_ground_height,raytrace_stepsize,
               DTED_ztwoDarray_ptr,impact_point);
         }

         camera_ptr->project_XYZ_to_UV_coordinates(XYZ,UVW);
//         cout << "f = " << f 
//              << " curr_t = " << get_curr_t()
//              << " passnumber = " << get_passnumber()
//              << " XYZ = " << XYZ 
//              << " new projected UVW = " << UVW << endl;
//         cout << "feature_occluded_flag = " << feature_occluded_flag << endl;
         Feature_ptr->set_UVW_coords(get_curr_t(),get_passnumber(),UVW);

// Mask any 2D feature whose coordinates lie outside photo's allowed
// UV extents or which is occluded:

         bool feature_outside_viewframe_flag=false;
            
         if (UVW.get(0) < photograph_ptr->get_minU() ||
             UVW.get(0) > photograph_ptr->get_maxU() ||
             UVW.get(1) < photograph_ptr->get_minV() ||
             UVW.get(1) > photograph_ptr->get_maxV())
         {
//            cout << "Feature lies outside view frame" << endl;
            feature_outside_viewframe_flag=true;
         }
         Feature_ptr->set_mask(get_curr_t(),get_passnumber(),
                               feature_outside_viewframe_flag ||
                               feature_occluded_flag);

         string label=update_feature_label(XYZ,camera_posn);
         Feature_ptr->set_text_label(1,label);

         continue_flag=true;
      } // Feature_3D_ptr != NULL conditional
    
      threevector UVW;
      if (!Feature_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW))
      {
         continue_flag=true;
      }

      if (continue_flag) continue;

      threevector ray_hat=camera_ptr->pixel_ray_direction(twovector(UVW));
//      cout << "f = " << f 
//           << " n_2D_Features = " << n_2D_Features 
//           << " UV = " << twovector(UVW) 
 //          << " ray = " << ray_hat << endl;
//      cout << "camera_posn = " << camera_posn << endl;

// If DTED_ztwoDarray_ptr holds a height map, trace ray from camera's
// position along ray_hat out to a range of max_raytrace_range.  If
// ray flies underneath any height patch, we assume it must have first
// hit a wall or some other impenetrable surface:

      bool raytrace_flag=false;
      threevector impact_point(
         NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
      if (DTED_ztwoDarray_ptr != NULL)
      {
         raytrace_flag=camera_ptr->trace_individual_ray(
            ray_hat,minimum_ground_height,maximum_raytrace_range,
            raytrace_stepsize,DTED_ztwoDarray_ptr,impact_point);
         if (!raytrace_flag)
         {
            cout << "Ray does not intercept any occluding surface" << endl;
            IDs_of_features_to_delete.push_back(Feature_ptr->get_ID());
         }
//         cout << "raytrace_flag = " << raytrace_flag 
//              << " impact_point = " << impact_point << endl;
      } // DTED_ztwoDarray_ptr != NULL conditional

      threevector closest_worldspace_point;
      geopoint closest_geopoint;
      EarthRegion* EarthRegion_ptr=EarthRegionsGroup_ptr->
         get_EarthRegion_ptr(0);

      double ds=0.5;	// meter
      double min_raytrace_range=1;		// meter
      double max_raytrace_range=10*1000;	// meters

      int ray_occluded_flag=-1;
      if (get_raytracer_ptr() != NULL)
      {
         ray_occluded_flag=get_raytracer_ptr()->trace_individual_ray(
            camera_posn,ray_hat,max_raytrace_range,min_raytrace_range,
            ds,closest_worldspace_point);
      }
      cout << "ray_occluded_flag = " << ray_occluded_flag << endl;

      if (ray_occluded_flag != 0)
      {
         if (PointFinder_ptr->find_smallest_relative_angle_world_point(
            camera_posn,ray_hat,closest_worldspace_point))
         {
            ray_occluded_flag=0;
         }
      }
      
      if (ray_occluded_flag==0)
      {
         cout << "3D counterpart found" << endl;
         closest_geopoint=geopoint(
            EarthRegion_ptr->get_northern_hemisphere_flag(),
            EarthRegion_ptr->get_UTM_zonenumber(),
            closest_worldspace_point.get(0),
            closest_worldspace_point.get(1),
            closest_worldspace_point.get(2));
      }
      else
      {
         cout << "No 3D counterpart point located!" << endl;
      }
      
// Compare ranges to closest 3D point and raytraced impact point.  If
// latter is smaller than former, reset closest_geopoint to impact
// point:

      double closest_point_range=
         (closest_worldspace_point-camera_posn).magnitude();
      double impact_point_range=(impact_point-camera_posn).magnitude();
//      cout << "closest_point_range = " << range << " impact_point_range = "
//           << impact_point_range << endl;
      if (raytrace_flag && impact_point_range < closest_point_range)
      {
         closest_worldspace_point=impact_point;
         closest_geopoint=geopoint(
            EarthRegion_ptr->get_northern_hemisphere_flag(),
            EarthRegion_ptr->get_UTM_zonenumber(),
            closest_worldspace_point.get(0),
            closest_worldspace_point.get(1),
            closest_worldspace_point.get(2));
      }
      cout << "closest geopoint = " << closest_geopoint << endl;

      if (DTED_ztwoDarray_ptr==NULL ||
          (DTED_ztwoDarray_ptr != NULL && raytrace_flag))
      {
         Feature_3D_ptr=FeaturesGroup_3D_ptr->
            generate_new_Feature(Feature_ptr->get_ID());

         Feature_3D_ptr->set_UVW_coords(
            FeaturesGroup_3D_ptr->get_curr_t(),
            FeaturesGroup_3D_ptr->get_passnumber(),closest_worldspace_point);
         Feature_3D_ptr->set_blinking_color(colorfunc::black);

         const double blink_period=60;	// secs
         cout << "blink_period = " << blink_period << endl;
         FeaturesGroup_3D_ptr->blink_Geometrical(
            Feature_3D_ptr->get_ID(),blink_period);

         string label=update_feature_label(
            closest_worldspace_point,camera_posn);
         Feature_ptr->set_text_label(1,label);
      }

   } // loop over index f labeling 2D features

// If no 3D counterpart to a 2D feature can be found, delete 2D
// feature from *this:

   for (unsigned int f=0; f<IDs_of_features_to_delete.size(); f++)
   {
      destroy_feature(IDs_of_features_to_delete[f]);
   }

   return true;
}

// -------------------------------------------------------------------------
// Member function backproject_selected_photo_features_as_3D_rays()
// loops over all 2D features and backprojects them into 3D space.  It
// draws colored rays emanating from OBSFRUSTA apexes through the 2D
// image plane features into world space.

bool FeaturesGroup::backproject_selected_photo_features_as_3D_rays()
{
//   cout << "Inside FeaturesGroup::backproject_selected_photo_features_as_3D_rays()" 
//        << endl;

   if (photogroup_ptr==NULL) return false;
   if (get_ndims() != 2) return false;

   unsigned int n_2D_Features=get_n_Graphicals();
//   cout << "n_2D_Features = " << n_2D_Features << endl;
   if (n_2D_Features==0) return false;

   unsigned int photo_ID=photogroup_ptr->get_selected_photo_ID();
   if (photo_ID < 0) photo_ID=0;

   if (photo_ID != get_curr_framenumber())
   {
//      cout << "photo_ID = " << photo_ID
//           << " get_curr_framenumber() = " << get_curr_framenumber()
//           << endl;
      return false;
   }

   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(photo_ID);
//   cout << "photo_ID = " << photo_ID << endl;

   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   threevector camera_posn=camera_ptr->get_world_posn();
//   cout << "camera_posn = " << camera_posn << endl;

// Loop over every 2D feature and backproject it into 3D world space:

//    bool new_ray_flag=false;
   for (unsigned int f=0; f<n_2D_Features; f++)
   {
      Feature* Feature_ptr=get_Feature_ptr(f);
      Feature_ptr->set_stationary_Graphical_flag(false);
    
      threevector UVW;
      if (!Feature_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW))
      {
         continue;
      }

      if (!Feature_ptr->get_coords_manually_manipulated(
         get_curr_t(),get_passnumber()))
      {
         continue;
      }

      twovector UV(UVW);
      double U=UV.get(0);
      double V=UV.get(1);
      if (nearly_equal(U,0) && nearly_equal(V,0)) continue;

      if (U < 0 || V < 0)
      {
         cout << "U = " << U << " V = " << V << endl;
         outputfunc::enter_continue_char();
      }

      threevector ray_hat=camera_ptr->pixel_ray_direction(UV);
//      cout << "f = " << f 
//           << " t = " << get_curr_t() 
//           << " n_2D_Features = " << n_2D_Features 
//           << " U = " << U << " V = " << V 
//           << " ray = " << ray_hat << endl;

      int Arrow_ID=Feature_ptr->get_ID();
      DUPLE curr_duple(photo_ID,Arrow_ID);

      THREEDRAYS_MAP::iterator iter=threeDrays_map.find(curr_duple);
      int color_index=-1;
      if (iter==threeDrays_map.end())
      {
         color_index=1+OBSFRUSTAGROUP_ptr->get_n_3D_rays()%7;
         threeDrays_map[curr_duple]=color_index;
//         if (!get_dragging_feature_flag()) new_ray_flag=true;
      }
      else
      {
         color_index=iter->second;
      }
      double linewidth=5.0;

      double raytrace_range=maximum_raytrace_range;
      fourvector groundplane_pi=OBSFRUSTAGROUP_ptr->get_groundplane_pi();
      if (groundplane_pi.get(3) > 0.5*NEGATIVEINFINITY)
      {
         plane groundplane(groundplane_pi);
         threevector intersection_point;
         if (groundplane.ray_intersection(
                camera_posn,ray_hat,intersection_point))
         {
            raytrace_range=1.1*(intersection_point-camera_posn).magnitude();
         }
      }
   
      OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
         Arrow_ID,photo_ID,UV,raytrace_range,
         colorfunc::get_color(color_index),linewidth);

   } // loop over index f labeling 2D features

/*
   cout << "n_3D_rays = " << OBSFRUSTAGROUP_ptr->get_n_3D_rays() << endl;
   cout << "new_ray_flag = " << new_ray_flag << endl;
   cout << "dragging_flag = " << get_dragging_feature_flag() << endl;
   if (OBSFRUSTAGROUP_ptr->get_n_3D_rays() >= 4 && new_ray_flag &&
       !get_dragging_feature_flag())
   {
      compute_multi_line_intersection();
   }
*/

   return true;
}

// -------------------------------------------------------------------------
// Member function display_3D_features_in_selected_photo()

bool FeaturesGroup::display_3D_features_in_selected_photo()
{
   cout << "Inside FeaturesGroup::display_3D_features_in_selected_photo()" 
        << endl;

   if (photogroup_ptr==NULL) return false;
   if (OBSFRUSTAGROUP_ptr==NULL) return false;
   if (get_ndims() != 2) return false;

   OBSFRUSTUM* selected_OBSFRUSTUM_ptr=dynamic_cast<OBSFRUSTUM*>(
      OBSFRUSTAGROUP_ptr->get_selected_Graphical_ptr());
   if (selected_OBSFRUSTUM_ptr==NULL) return false;

/*
// Display occluded ray impact points in 3D map:

   vector<threevector> occluded_ray_impact_points=selected_OBSFRUSTUM_ptr->
      get_occluded_ray_impact_points();
   unsigned int n_occluded_Features=occluded_ray_impact_points.size();
//   cout << "n_occluded_Features = " << n_occluded_Features << endl;
   if (FeaturesGroup_3D_ptr->get_n_Graphicals() != n_occluded_Features)
   {
      for (unsigned int i=0; i<n_occluded_Features; i++)
      {
         Feature* Feature_3D_ptr=FeaturesGroup_3D_ptr->generate_new_Feature();
         Feature_3D_ptr->set_UVW_coords(
            FeaturesGroup_3D_ptr->get_curr_t(),
            FeaturesGroup_3D_ptr->get_passnumber(),
            occluded_ray_impact_points[i]);
      } // loop over index i labeling occluded Features
   }
*/

   vector<threevector> projected_polyhedron_points=selected_OBSFRUSTUM_ptr->
      get_projected_polyhedron_points();
   unsigned int n_2D_Features=projected_polyhedron_points.size();
   if (n_2D_Features==0) return false;

// Don't run this method twice!

   if (n_2D_Features==get_n_Graphicals()) return false;
   destroy_all_Features();

   int photo_ID=photogroup_ptr->get_selected_photo_ID();
   if (photo_ID < 0) photo_ID=0;
   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(photo_ID);
//   cout << "photo_ID = " << photo_ID << endl;

   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   threevector camera_posn=camera_ptr->get_world_posn();
//   cout << "camera_posn = " << camera_posn << endl;

// Loop over every projected 3D feature and display it in the photo's
// image plane:

   for (unsigned int f=0; f<n_2D_Features; f++)
   {
      threevector UVW=projected_polyhedron_points[f];
      Feature* Feature_ptr=generate_new_Feature();
      Feature_ptr->set_UVW_coords(get_curr_t(),get_passnumber(),UVW);

// Mask any 2D feature whose coordinates lie outside photo's allowed
// UV extents or which is occluded:

      bool feature_outside_viewframe_flag=false;
      if (UVW.get(0) < photograph_ptr->get_minU() ||
          UVW.get(0) > photograph_ptr->get_maxU() ||
          UVW.get(1) < photograph_ptr->get_minV() ||
          UVW.get(1) > photograph_ptr->get_maxV())
      {
//         cout << "Feature lies outside view frame" << endl;
         feature_outside_viewframe_flag=true;
         Feature_ptr->set_mask(get_curr_t(),get_passnumber(),
                               feature_outside_viewframe_flag);
      }
   } // loop over index f labeling 2D features
   cout << "n_2D_Features = " << n_2D_Features << endl;

//   outputfunc::enter_continue_char();
   return true;
}

// -------------------------------------------------------------------------
// Member function draw_backprojected_2D_features() reads matching UV
// feature information from the specified input consolidated features
// file.  It backprojects the UV features into 3D space.  This method
// draws the backprojected rays as colored Arrows.

void FeaturesGroup::draw_backprojected_2D_features(
   string consolidated_features_filename)
{
   if (OBSFRUSTAGROUP_ptr==NULL) return;
 
   vector<int> pass_number;
   read_feature_info_from_file(
      consolidated_features_filename,pass_number);
   unsigned int n_features=get_n_Graphicals();
   cout << "n_features = " << n_features << endl;
   outputfunc::enter_continue_char();

   for (unsigned int i=0; i<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); i++)
   {

// Loop over all UV features for current OBSFRUSTUM labeled by ID i.
// Check if an UV features exist corresponding to passnumber = i:

      const double magnitude=25;	// meters      
//      const double magnitude=50;	// meters      

      for (unsigned int f=0; f<n_features; f++)
      {
         Feature* UV_Feature_ptr=get_Feature_ptr(f);
         threevector feature_UVW;
         bool flag=UV_Feature_ptr->get_UVW_coords(
            get_curr_t(),i,feature_UVW);

         if (flag)
         {
            twovector feature_UV(feature_UVW);
            cout << "pass index = " << i << " feature f = " << f 
                 << " feature_UV = " << feature_UV
                 << endl;

            int Arrow_ID=UV_Feature_ptr->get_ID();
            colorfunc::Color feature_color=
               colorfunc::get_color(UV_Feature_ptr->get_ID());
            OBSFRUSTAGROUP_ptr->draw_ray_through_imageplane(
               Arrow_ID,i,feature_UV,magnitude,feature_color);
         }
      } // loop over index f labeling UV features
   } // loop over index i labeling OBSFRUSTA
}

// -------------------------------------------------------------------------
// Member function update_feature_label() generates a string
// containing a 3D feature's longitude and latitude coords if member
// display_geocoords_flag==true.  If member
// display_range_alt_flag==true, the label contains the
// range between the camera and the 3D feature as well as the
// feature's altitude.

string FeaturesGroup::update_feature_label(
   const threevector& closest_worldspace_point,const threevector& camera_posn)
{
   EarthRegion* EarthRegion_ptr=EarthRegionsGroup_ptr->
      get_EarthRegion_ptr(0);

   geopoint closest_geopoint(
      EarthRegion_ptr->get_northern_hemisphere_flag(),
      EarthRegion_ptr->get_UTM_zonenumber(),
      closest_worldspace_point.get(0),
      closest_worldspace_point.get(1),
      closest_worldspace_point.get(2));

   double range=(closest_geopoint.get_UTM_posn()-camera_posn).magnitude();
   double longitude=closest_geopoint.get_longitude();
   double latitude=closest_geopoint.get_latitude();
   double altitude=closest_geopoint.get_altitude();
      
   string geocoords_label=update_geocoords_label(longitude,latitude);
   string range_alt_label=update_range_label(range,altitude);

   string label;
   if (display_geocoords_flag)
   {
      label += geocoords_label;
   }
   if (display_range_alt_flag)
   {
      label += range_alt_label;
   }
   return label;
}

// -------------------------------------------------------------------------
// Member function update_geocoords_label() returns a string label
// containing the current feature's long and lat geocoordinates.

string FeaturesGroup::update_geocoords_label(
   double longitude,double latitude)
{
//   cout << "Inside FeaturesGroup::update_geocoords_label()" << endl;

   string label="     "+stringfunc::number_to_string(fabs(longitude));
   if (longitude < 0)
   {
      label += " W";
   }
   else
   {
      label += " E";
   }
   label += "\n";
   
   label += "     "+stringfunc::number_to_string(fabs(latitude));
   if (latitude < 0)
   {
      label += " S";
   }
   else
   {
      label += " N";
   }
   label += "\n";
      
   return label;
}

// -------------------------------------------------------------------------
// Member function update_range_label() returns a string label
// containing the input range and altitude information.

string FeaturesGroup::update_range_label(double range,double altitude)
{
//   cout << "Inside FeaturesGroup::update_range_label()" << endl;

   int rounded_altitude=basic_math::round(altitude);      
//   cout << "rounded_altitude = " << rounded_altitude << endl;
   string label= "     Altitude: "+stringfunc::number_to_string(
      rounded_altitude)+"m \n";
   label += "     Range: "+stringfunc::number_to_string(range,0)+" m";
   return label;
}

// ==========================================================================
// Fundamental matrix display member functions
// ==========================================================================

// Member function update_epipolar_lines() instantiates a new PolyLine
// member  of *PolyLinesGroup_ptr  which corresponds  to  each feature
// within *counterpart_FeaturesGroup_2D_ptr.  

void FeaturesGroup::update_epipolar_lines()
{
//   cout << "inside FeaturesGroup::update_epipolar_lines()" << endl;

   if (counterpart_FeaturesGroup_2D_ptr==NULL) return;

   unsigned int n_counterpart_features=counterpart_FeaturesGroup_2D_ptr->
      get_n_Graphicals();
   unsigned int n_polylines=PolyLinesGroup_ptr->get_n_Graphicals();
   if (n_counterpart_features==n_polylines) return;

   PolyLinesGroup_ptr->destroy_all_PolyLines();
   for (unsigned int f=0; f<n_counterpart_features; f++)
   {
      Feature* counterpart_Feature_ptr=
         counterpart_FeaturesGroup_2D_ptr->get_Feature_ptr(f);
      threevector UVW;
      counterpart_Feature_ptr->get_UVW_coords(
         counterpart_FeaturesGroup_2D_ptr->get_curr_t(),
         counterpart_FeaturesGroup_2D_ptr->get_passnumber(),UVW);

// Reset 3rd component of homogeneous point to equal 1:

      UVW.put(2,1);

      threevector l;
      if (UV_image_flag)
      {
         l=(*fundamental_ptr) * UVW;
      }
      else 
      {
         l=fundamental_ptr->transpose() * UVW;
      }

//      cout << "2D point = " << UVW << endl;
//      cout << "l = " << l << endl;
//      cout << "l . point = " << l.dot(UVW) << endl;
      
//       PolyLine* epipolar_line_ptr=
         PolyLinesGroup_ptr->generate_UV_PolyLine(l);
   } // loop over index f labeling features within counterpart FeaturesGroup
}

// -------------------------------------------------------------------------
// Member function compute_multi_line_intersection() calculates the 3D
// point that lies closest to multiple 3D lines.  It instantiates a
// SignPost which displays the 3D point's easting and northing
// geocoordinates.  This method subsequently projects the 3D
// intersection point into all 2D video frames.

void FeaturesGroup::compute_multi_line_intersection()
{
   cout << "Inside FeaturesGroup::compute_multi_line_intersection()" 
        << endl;

   int feature_ID=0;
   threevector r_intersection=
      OBSFRUSTAGROUP_ptr->triangulate_rays(feature_ID);

   double height_above_ground=NEGATIVEINFINITY;
   fourvector groundplane_pi=OBSFRUSTAGROUP_ptr->get_groundplane_pi();
   if (groundplane_pi.get(3) > 0.5*NEGATIVEINFINITY)
   {
      plane groundplane(groundplane_pi);
      double Zground=groundplane.get_origin().get(2);
      height_above_ground=r_intersection.get(2)-Zground;
      cout << "height above ground = " << height_above_ground << endl;
   }

// Add 3D SignPost which displays intersection point's geocoordinates:

//   cout << "SignPostsGroup_ptr = " << SignPostsGroup_ptr << endl;
   if (SignPostsGroup_ptr != NULL)
   {
      SignPostsGroup_ptr->destroy_all_SignPosts();

//      double size=0.5;		// Puma
//      double height_multiplier=7;	// Puma

      double size=3;			// GEO
      double height_multiplier=2;	// GEO

      SignPost* SignPost_ptr=SignPostsGroup_ptr->generate_new_SignPost(
         size,height_multiplier,r_intersection);

      int easting=basic_math::round(r_intersection.get(0));
      int northing=basic_math::round(r_intersection.get(1));
      int altitude=basic_math::round(height_above_ground);
      string label="E="+stringfunc::number_to_string(easting)+
         " N="+stringfunc::number_to_string(northing);
      if (height_above_ground > 0.5*NEGATIVEINFINITY)
      {
         label += " H="+stringfunc::number_to_string(altitude);
      }
      
      SignPost_ptr->set_label(label);
   }

// Project 3D intersection point into all 2D image planes.  Then
// update 2D feature's UV coordinates for all video frames:

   for (unsigned int n=0; n<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); n++)
   {
      OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(n);
      Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
      camera* camera_ptr=Movie_ptr->get_camera_ptr();

      threevector curr_UVW;
      camera_ptr->project_XYZ_to_UV_coordinates(r_intersection,curr_UVW);
//      cout << "n = " << n << " curr_U = " << curr_UVW.get(0)
//           << " curr_V = " << curr_UVW.get(1) << endl;

      Feature* Feature_ptr=get_Feature_ptr(0);
      Feature_ptr->set_UVW_coords(n,get_passnumber(),curr_UVW);

   } // loop over index n labeling OBSFRUSTA
}

// ==========================================================================
// 3D point picking member functions
// ==========================================================================

// Member function
// display_montage_corresponding_to_selected_3D_feature_point()
// retrieves the most recently selected feature's 3D coordinates.  It
// performs a brute-force search through member STL map
// *montage_map_ptr for this input threevector.  If a corresponding
// pair of images is found, this method generates their montage and
// displays it on the screen.  

// We wrote this specialized member function in order to interact with
// 3D feature clouds for "matching" and "non-matching" image pairs as
// part of the 2013 JAV line program.

int prev_feature_counter=-1;
void FeaturesGroup::display_montage_corresponding_to_selected_3D_feature_point()
{
//   cout << "inside FeaturesGroup::display_montage_corresponding_to_3D_feature_pint()" << endl;

   if (get_n_Graphicals()==0) return;

   vector<threevector> feature_coords=retrieve_3D_feature_coords();
   threevector curr_feature=feature_coords.back();
//   cout << "curr_feature = " << curr_feature << endl;

   int curr_feature_counter=0;
   for (MONTAGE_MAP::iterator iter=montage_map_ptr->begin(); 
        iter != montage_map_ptr->end(); iter++)
   {
      double TINY=1E-6;
      if (curr_feature.nearly_equal(iter->first,TINY) &&
          curr_feature_counter != prev_feature_counter)
      {
         prev_feature_counter=curr_feature_counter;
         pair<string,string> P=iter->second;
         cout << "P.first = " << P.first
              << " P.second = " << P.second << endl;

         vector<int> process_IDs=sysfunc::my_get_pid("display");
         for (unsigned int p=0; p<process_IDs.size(); p++)
         {
//         cout << "p = " << p << " process_ID = " << process_IDs[p] << endl;
            string unix_cmd="kill -9 "+
               stringfunc::number_to_string(process_IDs[p]);
            sysfunc::unix_command(unix_cmd);
         }

         string unix_cmd="montageview "+P.first+" "+P.second;
         sysfunc::unix_command(unix_cmd);
      }
      curr_feature_counter++;
   } // iterator loop over montage map
      
}

// -------------------------------------------------------------------------
// Member function retrieve_3D_feature_coords() loops over all
// features which are assumed to be three-dimensional.  It forms and
// returns an STL vector containing the features' 3D coordinates.

vector<threevector> FeaturesGroup::retrieve_3D_feature_coords()
{
//   cout << "inside FeaturesGroup::retrieve_3D_feature_coords()" << endl;

   vector<threevector> feature_XYZs;

   unsigned int n_Features=get_n_Graphicals();
//   cout << "n_Features = " << n_Features << endl;

   threevector curr_XYZ;
   for (unsigned int p=0; p<n_Features; p++)
   {
      Feature* Feature_ptr=get_Feature_ptr(p);
      Feature_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),curr_XYZ);
      feature_XYZs.push_back(curr_XYZ);
//      cout << "feature_ID = " << Feature_ptr->get_ID()
//           << " XYZ = " << curr_XYZ << endl;
   }
   return feature_XYZs;
}
         
// ==========================================================================
// Feature chip member functions
// ==========================================================================

// Member function display_selected_feature_bbox() first purges all
// PolyLines in *PolyLinesGroup_ptr.  It then retrieves the currently
// selected feature's UV imageplane location.  A new PolyLine bbox is
// formed around the selected feature's location whose side dimensions
// correspond to bbox_sidelength measured in pixels.

// We wrote this member function in June 2014 in order to facility
// picking tree, building, road etc patches in aerial imagery of
// different length scales.

void FeaturesGroup::display_selected_feature_bbox()
{
   if (Movie_ptr==NULL) return;
   if (PolyLinesGroup_ptr==NULL) return;
   PolyLinesGroup_ptr->destroy_all_PolyLines();

   int selected_feature_ID=get_selected_Graphical_ID();
   if (selected_feature_ID < 0) return;

   texture_rectangle* texture_rectangle_ptr=
      Movie_ptr->get_texture_rectangle_ptr();

//   cout << "inside FeaturesGroup::display_selected_feature_bbox()" << endl;
//   cout << "selected_feature_ID = " << selected_feature_ID << endl;
   Feature* selected_Feature_ptr=get_ID_labeled_Feature_ptr(
      selected_feature_ID);

   if (selected_Feature_ptr->get_mask(get_curr_t(),get_passnumber()))
      return;

   threevector UVW;
   selected_Feature_ptr->get_UVW_coords(get_curr_t(), get_passnumber(),UVW);
//   cout << "UVW = " << UVW << endl;

   double delta=double(bbox_sidelength)/texture_rectangle_ptr->getHeight();
   double Ulo=UVW.get(0)-0.5*delta;
   double Uhi=UVW.get(0)+0.5*delta;
   double Vlo=UVW.get(1)-0.5*delta;
   double Vhi=UVW.get(1)+0.5*delta;

//   cout << "Image width = " << texture_rectangle_ptr->getWidth() << endl;
//   cout << "Image height = " << texture_rectangle_ptr->getHeight() << endl;

   vector<threevector> bbox_vertices;
   bbox_vertices.push_back(UVW+threevector(Ulo,Vlo));
   bbox_vertices.push_back(UVW+threevector(Uhi,Vlo));
   bbox_vertices.push_back(UVW+threevector(Uhi,Vhi));
   bbox_vertices.push_back(UVW+threevector(Ulo,Vhi));
   bbox_vertices.push_back(UVW+threevector(Ulo,Vlo));
   osg::Vec4 uniform_color=colorfunc::get_OSG_color(colorfunc::purple);

//   PolyLine* bbox_PolyLine_ptr=
      PolyLinesGroup_ptr->generate_new_PolyLine(
         UVW,bbox_vertices,uniform_color);
//   polyline* bbox_polyline_ptr=bbox_PolyLine_ptr->get_polyline_ptr();
//   cout << "bbox polyline = " << *bbox_polyline_ptr << endl;
}


// -------------------------------------------------------------------------
// Member function display_feature_pair_bboxes() first purges all
// PolyLines in *PolyLinesGroup_ptr.  It then retrieves the currently
// selected feature's UV imageplane location.  A new PolyLine bbox is
// formed around the selected feature's location whose side dimensions
// correspond to bbox_sidelength measured in pixels.

// We wrote this member function in June 2014 in order to facility
// picking tree, building, road etc patches in aerial imagery of
// different length scales.

void FeaturesGroup::display_feature_pair_bboxes()
{
   if (Movie_ptr==NULL) return;
   if (PolyLinesGroup_ptr==NULL) return;
   PolyLinesGroup_ptr->destroy_all_PolyLines();

   int n_features=get_n_Graphicals();
   if (is_odd(n_features))
   {
      n_features--;
   }

   for (int f=0; f<n_features; f += 2)
   {
      Feature* Feature_ptr=get_Feature_ptr(f);
      if (Feature_ptr->get_mask(get_curr_t(),get_passnumber())) continue;
//      int feature_ID=Feature_ptr->get_ID();
      Feature* next_Feature_ptr=get_Feature_ptr(f+1);
      if (next_Feature_ptr->get_mask(get_curr_t(),get_passnumber())) continue;
//      int feature2_ID=next_Feature_ptr->get_ID();

//      cout << "feature1_ID = " << feature_ID
//           << " feature2_ID = " << feature2_ID << endl;

      threevector UVW1,UVW2;
      Feature_ptr->get_UVW_coords(get_curr_t(), get_passnumber(),UVW1);
      next_Feature_ptr->get_UVW_coords(get_curr_t(), get_passnumber(),UVW2);
      threevector UVW=0.5*(UVW1+UVW2);
//      cout << "UVW = " << UVW << endl;

      double Ulo=basic_math::min(UVW1.get(0),UVW2.get(0));
      double Uhi=basic_math::max(UVW1.get(0),UVW2.get(0));
      double Vlo=basic_math::min(UVW1.get(1),UVW2.get(1));
      double Vhi=basic_math::max(UVW1.get(1),UVW2.get(1));
//      cout << "Ulo = " << Ulo << " Uhi = " << Uhi
//           << " Vlo = " << Vlo << " Vhi = " << Vhi
//           << endl;

      vector<threevector> bbox_vertices;
      bbox_vertices.push_back(UVW+threevector(Ulo,Vlo));
      bbox_vertices.push_back(UVW+threevector(Uhi,Vlo));
      bbox_vertices.push_back(UVW+threevector(Uhi,Vhi));
      bbox_vertices.push_back(UVW+threevector(Ulo,Vhi));
      bbox_vertices.push_back(UVW+threevector(Ulo,Vlo));

      osg::Vec4 uniform_color=colorfunc::get_OSG_color(colorfunc::orange);
      
//      PolyLine* bbox_PolyLine_ptr=
         PolyLinesGroup_ptr->generate_new_PolyLine(
            UVW,bbox_vertices,uniform_color);

   } // loop over index f labeling features
}

// -------------------------------------------------------------------------
// Member function extract_feature_chips() loops over an input set of
// images which can be assumed to correspond to a temporal sequence.
// If at a particular time slice some selected features exist, this
// method crops out a chip surrounding the feature's location.  The
// cropped chips are exported to the subdirectory specified as
// an input argument to this member function.

void FeaturesGroup::extract_feature_chips(
   unsigned int chip_size,string chips_subdir,string chip_prefix,
   int chip_offset,int start_feature_ID,int stop_feature_ID)
{
//   cout << "inside FeaturesGroup::extract_feature_chips()" << endl;

   if (stop_feature_ID < 0) stop_feature_ID=10000000;
   
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   for (int framenumber=AnimationController_ptr->get_first_framenumber(); 
	framenumber <= AnimationController_ptr->get_last_framenumber(); 
        framenumber++)
   {
      string image_filename = AnimationController_ptr->
         get_ordered_image_filename(framenumber);
      texture_rectangle_ptr->import_photo_from_file(image_filename);
      unsigned int width=texture_rectangle_ptr->getWidth();
      unsigned int height=texture_rectangle_ptr->getHeight();

//      cout << "frame = " << framenumber
//           << " image file = " << image_filename << endl;

      double curr_t=framenumber;
      int pass_number = 0;
      threevector UVW;
      for (unsigned int f=0; f<get_n_Graphicals(); f++)
      {
         Feature* Feature_ptr=get_Feature_ptr(f);
         int feature_ID=Feature_ptr->get_ID();

	 if (feature_ID < start_feature_ID || feature_ID > stop_feature_ID) 
            continue;

         Feature_ptr->get_UVW_coords(curr_t,pass_number,UVW);
         double U=UVW.get(0);
         double V=UVW.get(1);
         
         if (nearly_equal(U,0) && nearly_equal(V,0)) continue;

         unsigned int pu_center,pv_center;
         texture_rectangle_ptr->get_pixel_coords(U,V,pu_center,pv_center);

/*
         cout << "t = " << curr_t
//              << " image = " << image_filename
              << filefunc::getbasename(image_filename) 
              << endl;
         cout << " featureID = " << feature_ID
              << " U = " << UVW.get(0)
              << " V = " << UVW.get(1)
              << " pu = " << pu_center
              << " pv = " << pv_center
              << endl;
*/

         if (pv_center > height)
         {
            pv_center = 0;
         }
         else
         {
            pv_center = height - pv_center;
         }

         unsigned int pu_start=basic_math::max(
            int(0),int(pu_center-chip_size/2));
         unsigned int pu_stop=basic_math::min(
            int(width-1),int(pu_center+chip_size/2-1));
         unsigned int pv_start=basic_math::max(
            int(0),int(pv_center-chip_size/2));
         unsigned int pv_stop=basic_math::min(
            int(height-1),int(pv_center+chip_size/2-1));
         
	 int chip_ID=feature_ID+chip_offset;
         string chip_filename=chips_subdir+chip_prefix+"_"+
            stringfunc::integer_to_string(chip_size,4)+"_"+
            stringfunc::integer_to_string(chip_ID,5)+".jpg";
         texture_rectangle_ptr->write_curr_frame(
            pu_start,pu_stop,pv_start,pv_stop,chip_filename);

         string banner="Exported "+chip_filename;
         outputfunc::write_banner(banner);

      } // loop over index f labeling features
   } // loop over framenumber index 

   delete texture_rectangle_ptr;
}

// -------------------------------------------------------------------------
// Member function renormalize_feature_coords() converts all feature
// coordinates from common, multi-image U,V coordinates to
// individual image (u,v) coordinates.  Feature coordinates relative
// to a centralized image embedded within a larger blank image are converted
// to coordinates relative to the original image's pixel dimensions.
// The renormalized feature coordinates are written to an output text file.

void FeaturesGroup::renormalize_feature_coords()
{
   cout << "inside FeaturesGroup::renormalize_feature_coords()" << endl;

   texture_rectangle* movie_texture_rectangle_ptr=get_Movie_ptr()->
      get_texture_rectangle_ptr();
   unsigned int max_width=movie_texture_rectangle_ptr->getWidth();
   unsigned int max_height=movie_texture_rectangle_ptr->getHeight();
//   double AR=double(max_width)/double(max_height);

   for (int framenumber=AnimationController_ptr->get_first_framenumber(); 
	framenumber <= AnimationController_ptr->get_last_framenumber(); 
        framenumber++)
   {
      string image_filename = AnimationController_ptr->
         get_ordered_image_filename(framenumber);

      unsigned int width,height;
      imagefunc::get_image_width_height(
         image_filename,width,height);
      double ar=double(width)/double(height);

      cout << "max_width = " << max_width 
           << " max_height = " << max_height << endl;
      cout << "width = " << width 
           << " height = " << height << endl;
      cout << "ar = " << ar << endl;

      double U0=double(max_width-width)/double(2*max_height);
      double V0=double(max_height-height)/double(2*max_height);
      double U1=double(max_width+width)/double(2*max_height);
      double V1=double(max_height+height)/double(2*max_height);

      cout << "(U1-U0)/(V1-V0) = "
           << (U1-U0)/(V1-V0) << endl;

      double alpha=(U1-U0)/ar;
      double beta=V1-V0;

      cout << "U0 = " << U0 << " V0 = " << V0 << endl;
      cout << "U1 = " << U1 << " V1 = " << V1 << endl;
      cout << "ar = " << ar << " alpha = " << alpha
           << " beta = " << beta << endl;

// U = U0 + alpha * u
// V = V0 + beta * v

// u = (U-U0)/alpha
// v = (V-V0)/beta

//      unsigned int pu,pv;

      double curr_t=framenumber;
      threevector UVW;
      unsigned int n_features=get_n_Graphicals();
      for (unsigned int f=0; f<n_features; f++)
      {
         Feature* Feature_ptr=get_Feature_ptr(f);
//         int feature_ID=Feature_ptr->get_ID();
         Feature_ptr->get_UVW_coords(curr_t,get_passnumber(),UVW);
         double U=UVW.get(0);
         double V=UVW.get(1);

         if (nearly_equal(U,0) && nearly_equal(V,0)) continue;

         double u=(U-U0)/alpha;
         double v=(V-V0)/beta;
         UVW.put(0,u);
         UVW.put(1,v);
         Feature_ptr->set_UVW_coords(curr_t,get_passnumber(),UVW);

//         cout << "t = " << curr_t 
//              << " image = " << image_filename << endl;
//         cout << "   f = " << f << " U = " << U << " u = " << u 
//              << " V = " << V << " v = " << v << endl;
      } // loop over index f labeling features

   } // loop over framenumber index 

   string output_filename="./renormalized_features_2D.txt";
   save_feature_info_to_file(output_filename);   

   string banner="Exported renormalized feature coordinates to "+
      output_filename;
   outputfunc::write_big_banner(banner);
}

// -------------------------------------------------------------------------
// This overloaded version of extract_feature_chips() is designed to
// work with manually-specified bbox corner pairs.  Looping over
// images, this method imports their originally-sized jpg files.  It
// extracts a bounding box chip for each pair of feature corners which
// overlap with the current image.  Bbox chips are written to the
// specified chips_subdir with filenames that begin with the specified
// chip_prefix.

void FeaturesGroup::extract_feature_chips(
   string chips_subdir,string chip_prefix)
{
   filefunc::dircreate(chips_subdir);

   int chip_ID=0;
   for (int framenumber=AnimationController_ptr->get_first_framenumber(); 
	framenumber <= AnimationController_ptr->get_last_framenumber(); 
        framenumber++)
   {
      string image_filename = AnimationController_ptr->
         get_ordered_image_filename(framenumber);
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         image_filename,NULL);

//      cout << "frame = " << framenumber
//           << " image file = " << image_filename << endl;

      double curr_t=framenumber;
      int pass_number = 0;
      threevector UVW,UVW2;
      unsigned int n_features=get_n_Graphicals();
//      cout << "n_features = " << n_features << endl;
      for (unsigned int f=0; f<n_features; f += 2)
      {
         Feature* Feature_ptr=get_Feature_ptr(f);
//         int feature_ID=Feature_ptr->get_ID();

         Feature* next_Feature_ptr=get_Feature_ptr(f+1);
//         int next_feature_ID=next_Feature_ptr->get_ID();

         Feature_ptr->get_UVW_coords(curr_t,pass_number,UVW);
         double movie_U1=UVW.get(0);
         double movie_V1=UVW.get(1);
         if (nearly_equal(movie_U1,0) && nearly_equal(movie_V1,0)) continue;

         next_Feature_ptr->get_UVW_coords(curr_t,pass_number,UVW2);
         double movie_U2=UVW2.get(0);
         double movie_V2=UVW2.get(1);
         if (nearly_equal(movie_U2,0) && nearly_equal(movie_V2,0)) continue;

         unsigned int movie_pu1,movie_pv1,movie_pu2,movie_pv2;
         texture_rectangle_ptr->get_pixel_coords(
            movie_U1,movie_V1,movie_pu1,movie_pv1);
         texture_rectangle_ptr->get_pixel_coords(
            movie_U2,movie_V2,movie_pu2,movie_pv2);

         int height=texture_rectangle_ptr->getHeight();
         unsigned pv2=basic_math::max(Unsigned_Zero,height-movie_pv1);
         unsigned pv1=basic_math::max(Unsigned_Zero,height-movie_pv2);

         string chip_filename=chips_subdir+chip_prefix+"_"+
            stringfunc::integer_to_string(chip_ID++,5)+".jpg";

         texture_rectangle_ptr->write_curr_frame(
            movie_pu1,movie_pu2,pv1,pv2,chip_filename);

         string banner="Exported "+chip_filename;
         outputfunc::write_banner(banner);

      } // loop over index f labeling features

      delete texture_rectangle_ptr;

   } // loop over framenumber index 
}
