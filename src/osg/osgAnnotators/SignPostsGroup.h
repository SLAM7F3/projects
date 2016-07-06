// ==========================================================================
// Header file for SIGNPOSTSGROUP class
// ==========================================================================
// Last modified on 7/1/11; 7/9/11; 9/13/12
// ==========================================================================

#ifndef SIGNPOSTSGROUP_H
#define SIGNPOSTSGROUP_H

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osg/Group>
#include <osg/Node>
#include "osg/osgAnnotators/AnnotatorsGroup.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColorGeodeVisitor.h"
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "datastructures/Linkedlist.h"
#include "general/ltstring.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgAnnotators/SignPost.h"
#include "osg/osgAnnotators/SignPostPickHandler.h"

class geopoint;
class PointCloudsGroup;
class postgis_database;
class postgis_databases_group;

class SignPostsGroup : public GeometricalsGroup, public AnnotatorsGroup
{

  public:

// Initialization, constructor and destructor functions:

   SignPostsGroup(const int p_ndims,Pass* PI_ptr,threevector* GO_ptr);
   SignPostsGroup(
      Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
      threevector* GO_ptr);
   SignPostsGroup(
      Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
      threevector* GO_ptr,postgis_database* SKS_db_ptr,
      PointCloudsGroup* PCG_ptr=NULL);
   SignPostsGroup(
      Pass* PI_ptr,threevector* GO_ptr,
      postgis_database* SKS_db_ptr,PointCloudsGroup* PCG_ptr=NULL);
   virtual ~SignPostsGroup();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const SignPostsGroup& f);

// Set & get methods:

   void set_broadcast_NFOV_aimpoint_flag(bool flag);
   void set_aerial_video_frame_flag(bool flag);
   void set_altitude_dependent_size_flag(bool flag);
   void set_raytrace_visible_terrain_flag(bool flag);
   void set_fixed_label_to_SignPost_ID(int ID,std::string fixed_label);
   SignPost* get_SignPost_ptr(int n) const;
   SignPost* get_ID_labeled_SignPost_ptr(int ID) const;
   void set_size(double size);
   void set_size(double size,double text_size);
   void set_max_text_width(double width);

   void set_MoviesGroup_ptr(MoviesGroup* MoviesGroup_ptr);
   void set_SignPostsGroup_3D_ptr(SignPostsGroup* SPG_ptr);
   void set_imageplane_SignPostsGroup_ptr(SignPostsGroup* ISPG_ptr);
   SignPostsGroup* get_imageplane_SignPostsGroup_ptr();
   void set_imageplane_SignPostPickHandler_ptr(
      SignPostPickHandler* ISPPH_ptr);

   void set_photogroup_ptr(photogroup* pg_ptr);
   void set_postgis_databases_group_ptr(postgis_databases_group* pdg_ptr);

   void set_ColorGeodeVisitor_ptr(ColorGeodeVisitor* CGV_ptr);

// SignPost creation member functions:

   SignPost* generate_new_SignPost(int ID=-1);
   SignPost* generate_new_SignPost(
      double size,double height_multiplier,int ID=-1,
      int OSGsubPAT_number=0);
   SignPost* generate_new_SignPost(
      double size,double height_multiplier,const threevector& UVW,int ID=-1,
      int OSGsubPAT_number=0);
   SignPost* generate_new_SignPost_on_earth(
      double longitude,double latitude,double altitude,int ID=-1);
   SignPost* generate_new_SignPost_on_earth(
      double longitude,double latitude,double altitude,
      int specified_UTM_zonenumber,int ID);

// SignPost manipulation member functions:

   void edit_SignPost_label();
   bool assign_fixed_label(int curr_ID);
   int destroy_SignPost();
   bool destroy_SignPost(int SignPost_ID);
   void destroy_all_SignPosts();
   bool erase_SignPost();
   bool unerase_SignPost();
   void set_altitude_dependent_size();
   SignPost* move_z(int sgn);

   void update_display();

// Ascii feature file I/O methods

   std::string save_info_to_file();
   bool read_info_from_file();
   bool read_info_from_file(std::string input_filename);
   
/*
// SKS database insertion and retrieval methods:

   bool insert_into_SKS_worldmodel_database(
      SignPost* curr_signpost_ptr,int& entity_id);
   void retrieve_signposts_from_SKS_worldmodel_database();
   void update_SKS_worldmodel_database_entry(SignPost* curr_SignPost_ptr);
   void update_SKS_worldmodel_database_nomination_label(
      SignPost* curr_SignPost_ptr);
*/

// PostGIS database methods:

   bool retrieve_all_signposts_from_PostGIS_databases(
      colorfunc::Color signposts_color,double SignPost_size=1.0);
   bool retrieve_signposts_from_PostGIS_database(
      colorfunc::Color signposts_color,
      std::string DatabaseName,std::string TableName,double SignPost_size);
   bool retrieve_signposts_from_Cambridge_database(
      colorfunc::Color signposts_color,std::string DatabaseName,
      std::string TableName,double SignPost_size);
   double retrieved_SignPost_altitude(
      bool altitude_specified_flag,std::string field_entry,double X,double Y);
   SignPost* gazetteer();
   void generate_PostGIS_SignPosts(
      const std::vector<double>& curr_time,
      const std::vector<int>& SignPost_ID,
      const std::vector<double>& latitude,
      const std::vector<double>& longitude,
      const std::vector<double>& altitude,
      const std::vector<threevector>& UVW,
      const std::vector<std::string> label,
      double SignPost_size,colorfunc::Color signposts_color);

// Message handling member functions:

   bool issue_invocation_message();
   void broadcast_NFOV_aimpoint();

// LOST ground target member functions:

   threevector get_ground_target_posns(std::vector<twovector>& target_posns);
   void generate_ground_target_w_track();

// SignPost video plane projection member functions:

   void reset_video_SignPost(
      SignPost* SignPost_ptr,const twovector& SignPost_tip_UV,
      double Umin,double Umax,double Vmin,double Vmax,
      bool downward_only_arrows_flag=false);
   bool project_SignPosts_into_video_plane();
   bool project_SignPosts_into_selected_photo();
   bool project_SignPosts_into_selected_aerial_video_frame();
   bool backproject_selected_photo_SignPosts_into_3D();

// Raytracing member functions:

   void raytrace_visible_terrain();
   void display_raytracing_results(twoDarray* ptwoDarray_ptr);
   void test_raytracing();

  protected:

  private:

   bool broadcast_NFOV_aimpoint_flag,aerial_video_frame_flag;
   bool altitude_dependent_size_flag,raytrace_visible_terrain_flag;
   int prev_SignPost_framenumber,prev_photo_ID;
   double prev_relative_time_database_accessed;
   postgis_database* SKS_worldmodel_database_ptr;
   postgis_databases_group* postgis_databases_group_ptr;

   std::vector<std::pair<int,std::string> > ID_fixed_label_pairs;
   std::vector<std::string> previously_retrieved_tables;
   ColorGeodeVisitor* ColorGeodeVisitor_ptr;
   MoviesGroup* MoviesGroup_ptr;
   photogroup* photogroup_ptr;
   PointCloudsGroup* PointCloudsGroup_ptr;

   SignPostsGroup* SignPostsGroup_3D_ptr;
   SignPostsGroup* imageplane_SignPostsGroup_ptr;
   SignPostPickHandler* imageplane_SignPostPickHandler_ptr;

   typedef std::map<std::string,SignPost*,ltstring > SIGNPOSTS_MAP;
   SIGNPOSTS_MAP* signposts_map_ptr;

   threevector prev_SignPost_posn;


   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const SignPostsGroup& f);

   void initialize_new_SignPost(
      double size,double height_multiplier,const threevector& UVW,
      SignPost* curr_SignPost_ptr,int OSGsubPAT_number=0);

// Bluegrass specific member functions:

   bool identify_and_revise_Bluegrass_labels(
      postgis_database* postgis_database_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void SignPostsGroup::set_broadcast_NFOV_aimpoint_flag(bool flag)
{
   broadcast_NFOV_aimpoint_flag=flag;
}

inline void SignPostsGroup::set_aerial_video_frame_flag(bool flag)
{
   aerial_video_frame_flag=flag;
}

inline void SignPostsGroup::set_altitude_dependent_size_flag(bool flag)
{
   altitude_dependent_size_flag=flag;
}

inline void SignPostsGroup::set_raytrace_visible_terrain_flag(bool flag)
{
   raytrace_visible_terrain_flag=flag;
}

// --------------------------------------------------------------------------
inline SignPost* SignPostsGroup::get_SignPost_ptr(int n) const
{
   return dynamic_cast<SignPost*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline SignPost* SignPostsGroup::get_ID_labeled_SignPost_ptr(int ID) const
{
   return dynamic_cast<SignPost*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline void SignPostsGroup::set_MoviesGroup_ptr(MoviesGroup* MoviesGroup_ptr)
{
   this->MoviesGroup_ptr=MoviesGroup_ptr;
}

// --------------------------------------------------------------------------
inline void SignPostsGroup::set_SignPostsGroup_3D_ptr(
   SignPostsGroup* SPG_ptr)
{
   SignPostsGroup_3D_ptr=SPG_ptr;
}

inline void SignPostsGroup::set_imageplane_SignPostsGroup_ptr(
   SignPostsGroup* ISPG_ptr)
{
   imageplane_SignPostsGroup_ptr=ISPG_ptr;
}

inline SignPostsGroup* SignPostsGroup::get_imageplane_SignPostsGroup_ptr()
{
   return imageplane_SignPostsGroup_ptr;
}

inline void SignPostsGroup::set_imageplane_SignPostPickHandler_ptr(
   SignPostPickHandler* ISPPH_ptr)
{
   imageplane_SignPostPickHandler_ptr=ISPPH_ptr;
}

// --------------------------------------------------------------------------
inline void SignPostsGroup::set_photogroup_ptr(photogroup* pg_ptr)
{
   photogroup_ptr=pg_ptr;
}

inline void SignPostsGroup::set_ColorGeodeVisitor_ptr(
   ColorGeodeVisitor* CGV_ptr)
{
   ColorGeodeVisitor_ptr=CGV_ptr;
}

#endif // SignPostsGroup.h



