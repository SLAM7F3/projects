// ==========================================================================
// Header file for MODELSGROUP class
// ==========================================================================
// Last modified on 2/21/07; 2/22/07; 2/27/07; 4/5/14
// ==========================================================================

#ifndef MODELSGROUP_H
#define MODELSGROUP_H

#include <iostream>
#include <osg/Group>
#include <osg/Node>
#include "osg/osgAnnotators/AnnotatorsGroup.h"
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "osg/osgModels/Model.h"

class AnimationController;
class EarthRegion;
class geopoint;
class PolyLinesGroup;

class ModelsGroup : public GraphicalsGroup, public AnnotatorsGroup
{

  public:

// Initialization, constructor and destructor functions:

   ModelsGroup(Pass* PI_ptr,threevector* GO_ptr=NULL,
               AnimationController* AC_ptr=NULL);
   ModelsGroup(Pass* PI_ptr,PolyLinesGroup* PLG_ptr,
               AnimationController* AC_ptr=NULL);
   ModelsGroup(Pass* PI_ptr,PolyLinesGroup* PLG_ptr,threevector* GO_ptr,
               AnimationController* AC_ptr=NULL);
   virtual ~ModelsGroup();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const ModelsGroup& M);

// Set & get methods:

   void set_model_filename(std::string filename);
   std::string get_model_filename() const;
   Model* get_Model_ptr(int n) const;
   Model* get_ID_labeled_Model_ptr(int ID) const;

// Model creation and manipulation methods:

   Model* generate_new_Model(
      std::string model_filename,int OSGsubPAT_number=0,int ID=-1);
   Model* generate_new_Model(int OSGsubPAT_number=0,int ID=-1);
   ObsFrustum* instantiate_ObsFrustum(
      Model* Model_ptr,double az_extent,double el_extent,
      int OSGsubPAT_number=0);

   Model* move_z(int sgn);
   void change_scale(double scalefactor);

   void unmask_next_model();
   void update_display();

// Model path creation methods:

   void record_waypoint();
   void finish_waypoint_entry();
   void generate_racetrack_orbit(
      Model* curr_model_ptr,double center_longitude,double center_latitude,
      double radius,double height_above_center,double omega,
      double phase_offset=0);
   void generate_racetrack_orbit(
      Model* curr_model_ptr,const threevector& center_origin,double radius,
      double height_above_center,double omega,double phase_offset=0);

// ISDS demo methods:

   Model* generate_predator_Model(int& OSGsubPAT_number);
   Model* generate_LiMIT_Model(int& OSGsubPAT_number);
   void generate_predator_racetrack_orbit(
      int n_total_frames,Model* predator_ptr);
   void generate_predator_racetrack_orbit(
      int n_total_frames,double racetrack_center_longitude,
      double racetrack_center_latitude,Model* predator_ptr);
   void generate_elliptical_LiMIT_racetrack_orbit(
      int n_total_frames,const geopoint& racetrack_center,
      Model* LiMIT_ptr);
   void generate_elliptical_LiMIT_racetrack_orbit(
      int n_total_frames,const threevector& racetrack_center,
      Model* LiMIT_ptr);
   ObsFrustum* generate_predator_ObsFrustrum(
      int predator_OSGsubPAT_number,Model* predator_ptr);
   ObsFrustum* generate_LiMIT_FOV_ObsFrustrum(
      int LiMIT_OSGsubPAT_number,Model* LiMIT_ptr);
   ObsFrustum* generate_LiMIT_instantaneous_dwell_ObsFrustrum(
      int LiMIT_OSGsubPAT_number,Model* LiMIT_ptr,
      ObsFrustum* FOV_ObsFrustum_ptr,EarthRegion* region_ptr);

// HAFB video3D methods:

   Model* generate_HAFB_video_pass_model(
      double z_rot_angle,const threevector& first_frame_aircraft_posn,
      AnimationController* HAFB_AnimationController_ptr);
   void read_HAFB_plane_info(
      std::vector<threevector>& plane_posn,
      std::vector<threevector>& plane_attitude);
   ObsFrustum* instantiate_HAFB_video_ObsFrustum(
      Model* Model_ptr,const std::vector<threevector>& aircraft_posn,
      double z_offset,int OSGsubPAT_number=0);

  protected:

  private:

   unsigned int model_counter;
   std::string model_filename;
   std::vector<threevector> candidate_waypoint,waypoint,even_path_point;
   PolyLinesGroup* PolyLinesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ModelsGroup& M);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ModelsGroup::set_model_filename(std::string filename)
{
   model_filename=filename;
}

inline std::string ModelsGroup::get_model_filename() const
{
   return model_filename;
}

// --------------------------------------------------------------------------
inline Model* ModelsGroup::get_Model_ptr(int n) const
{
   return dynamic_cast<Model*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Model* ModelsGroup::get_ID_labeled_Model_ptr(int ID) const
{
   return dynamic_cast<Model*>(get_ID_labeled_Graphical_ptr(ID));
}



#endif // ModelsGroup.h



