// ==========================================================================
// Header file for MODEL class
// ==========================================================================
// Last updated on 9/30/11; 10/2/11; 10/12/11; 4/6/14
// ==========================================================================

#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <osg/Node>
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "osg/osgGeometry/Geometrical.h"
#include "astro_geo/geopoint.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgModels/OBSFRUSTUMPickHandler.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "math/threevector.h"

class ColorGeodeVisitor;
class EarthRegionsGroup;
class ModeController;
class OBSFRUSTAGROUP;
class Pass;
class PointCloud;
class PointCloudsGroup;
class SignPostsGroup;
class WindowManager;

class MODEL : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   MODEL(threevector* GO_ptr,std::string filename,int id);
   MODEL(Pass* PI_ptr,threevector* GO_ptr,AnimationController* AC_ptr,
         osgGA::Terrain_Manipulator* CM_3D_ptr,
         ModeController* ModeController_ptr,WindowManager* WindowManager_ptr,
         std::string filename,bool instantiate_OBSFRUSTAGROUP_flag,
         bool instantiate_OBSFRUSTUMPickHandler_flag,int id);

   virtual ~MODEL();
   friend std::ostream& operator<< (
      std::ostream& outstream,const MODEL& m);

// Set & get methods:

   void set_dynamically_compute_OBSFRUSTUM_flag(bool flag);
   bool get_dynamically_compute_OBSFRUSTUM_flag() const;
   void set_compute_zface_height_flag(bool flag);
   void set_raytrace_occluded_ground_regions_flag(bool flag);
   bool get_raytrace_occluded_ground_regions_flag() const;
   void set_raytrace_ROI_flag(bool flag);
   bool get_raytrace_ROI_flag() const;
   void set_raytrace_cell_size(double size);
   void set_speed(double s);
   double get_speed() const;
   void push_back_OBSFRUSTUM_alpha(double alpha);
   void push_back_OBSFRUSTUM_beta(double beta);
   void set_OBSFRUSTUM_FOVs(
      unsigned int OBSFRUSTUM_ID,double horiz_FOV,double vert_FOV);
   void set_OBSFRUSTUM_roll(unsigned int OBSFRUSTUM_ID,double roll);
   void push_back_OBSFRUSTUM_roll(double roll);
   void set_OBSFRUSTUM_pitch(unsigned int OBSFRUSTUM_ID,double pitch);
   void push_back_OBSFRUSTUM_pitch(double pitch);
   void push_back_OBSFRUSTUM_yaw(double yaw);
   void push_back_OBSFRUSTUM_z_base_face(double z);
   void set_z_base_face(int OBSFRUSTUM_ID,double z);
   double get_OBSFRUSTUM_z_base_face(int OBSFRUSTUM_ID) const;

   osg::Node* get_model_node_ptr();
   const osg::Node* get_model_node_ptr() const;
   void set_path_update_distance(double d);
   double get_path_update_distance() const;
   void set_min_raytrace_range(double r);
   double get_min_raytrace_range() const;
   void set_max_raytrace_range(double r);
   double get_max_raytrace_range() const;
   void set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr);
   OBSFRUSTAGROUP* get_OBSFRUSTAGROUP_ptr();
   const OBSFRUSTAGROUP* get_OBSFRUSTAGROUP_ptr() const;

// Model manipulation member functions:

   virtual void set_attitude_posn(
      double curr_t,int pass_number,
      const threevector& RPY,const threevector& posn);

// Observation frusta instantiation and manipulation member functions:

   void orient_and_position_OBSFRUSTUM(
      unsigned int first_framenumber,unsigned int last_framenumber,
      int pass_number,double delta_phi,double delta_theta,
      double z_base_face,bool sinusoidal_variation_flag=false,
      int OBSFRUSTUM_ID=0);
   void instantaneously_orient_and_position_OBSFRUSTUM(
      double curr_t,int pass_number,double dt,
      double delta_phi,double delta_theta,double z_base_face,
      OBSFRUSTUM* OBSFRUSTUM_ptr);

   void orient_and_position_instantaneous_dwell_OBSFRUSTUM(
      unsigned int first_framenumber,unsigned int last_framenumber,
      int pass_number,double z_base_face,
      OBSFRUSTUM* dwell_OBSFRUSTUM_ptr,OBSFRUSTUM* FOV_OBSFRUSTUM_ptr,
      const std::vector<geopoint>& GMTI_targets);
   bool instantaneously_orient_and_position_GMTI_dwell_OBSFRUSTUM(
      double curr_t,int pass_number,double z_base_face,
      OBSFRUSTUM* dwell_OBSFRUSTUM_ptr,OBSFRUSTUM* FOV_OBSFRUSTUM_ptr,
      const threevector& GMTI_posn,
      double dwell_range_extent,double dwell_crossrange_extent);

// Dynamic OBSFRUSTA manipulation member functions:

// Note added on 10/7/09: All member functions related to LOST should
// someday move into the LOSMODEL class...

   std::vector<OBSFRUSTUM*> compute_dynamic_OBSFRUSTA(
      double curr_t,int pass_number,EarthRegionsGroup* EarthRegionsGroup_ptr,
      unsigned int n_future_repeats,bool& OBSFRUSTA_previously_built_flag);
   bool recover_or_compute_OBSFRUSTA_occlusion(
      bool OBSFRUSTA_previously_built_flag,
      double curr_t,int pass_number,EarthRegionsGroup* EarthRegionsGroup_ptr,
      ColorGeodeVisitor* ColorGeodeVisitor_ptr,bounding_box* ground_bbox_ptr,
      Messenger* viewer_Messenger_ptr,double raytrace_cellsize);
   void compute_and_display_OBSFRUSTA_occlusion(
      double curr_t,int pass_number,bool previously_raytraced_flag,
      EarthRegionsGroup* EarthRegionsGroup_ptr,
      ColorGeodeVisitor* ColorGeodeVisitor_ptr,bounding_box* ground_bbox_ptr,
      Messenger* viewer_Messenger_ptr,double raytrace_cellsize);
   OBSFRUSTUM* compute_dynamic_OBSFRUSTUM(
      double curr_t,int pass_number,
      const threevector& posn,const threevector& v_hat,
      double alpha,double beta,double z_base_face,int OBSFRUSTUM_ID=0);
   void display_average_LOS_results(
      std::vector<std::string>& geotif_filenames,
      ColorGeodeVisitor* ColorGeodeVisitor_ptr,
      EarthRegionsGroup* EarthRegionsGroup_ptr,
      Messenger* viewer_messenger_ptr);

   void update_NFOV_OBSFRUSTUM_roll_and_pitch(
      double curr_t,int pass_number,const geopoint& lookpoint);

// ActiveMQ broadcast member functions:

   void broadcast_average_occlusion_fractions(
      const std::vector<double>& frac,Messenger* viewer_Messenger_ptr,
      bool clear_results_flag=false);
   void broadcast_ground_target_visibility(
      double curr_t,int target_ID,int visibility,
      Messenger* viewer_Messenger_ptr);

// Human model member functions

   void position_and_orient_man_MODEL(
      double curr_t,int pass_number,
      const threevector& posn,double roll,double pitch,double yaw);
   void broadcast_human_geoposn(const geopoint& human_geopoint,
                                Messenger* GPS_messenger_ptr);

  protected:

  private:

   bool dynamically_compute_OBSFRUSTUM_flag;
   bool compute_zface_height_flag;
   bool raytrace_occluded_ground_regions_flag,raytrace_ROI_flag;
   std::string model_filename;
   double speed;	// meters/sec
   double path_update_distance;	// Sampling distance for model polyline path
   double min_raytrace_range,max_raytrace_range;
   double raytrace_cell_size;
   std::vector<double> OBSFRUSTUM_alpha,OBSFRUSTUM_beta;
   std::vector<double> OBSFRUSTUM_roll,OBSFRUSTUM_pitch,OBSFRUSTUM_yaw;
   std::vector<double> OBSFRUSTUM_z_base_face;

   threevector prev_posn,prev_vhat;

   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;
   osg::ref_ptr<OBSFRUSTUMPickHandler> OBSFRUSTUMPickHandler_refptr;
   osg::ref_ptr<osg::Node> model_node_refptr;
   WindowManager* WindowManager_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
//   void docopy(const MODEL& m);

   PointCloudsGroup* extract_PointCloudsGroup_ptr(
      EarthRegionsGroup* EarthRegionsGroup_ptr);
   PointCloud* extract_PointCloud_ptr(
      EarthRegionsGroup* EarthRegionsGroup_ptr);

//   std::vector<std::string> find_avg_LOS_geotif_files(
//      std::string geotif_Ptiles_subdir,
//      ColorGeodeVisitor* ColorGeodeVisitor_ptr);
   bool import_avg_LOS_ptwoDarray_contents(
      std::vector<std::string> geotif_filenames,
      std::vector<twoDarray*>& avg_LOS_ptwoDarray_ptrs);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void MODEL::set_dynamically_compute_OBSFRUSTUM_flag(bool flag)
{
   dynamically_compute_OBSFRUSTUM_flag=flag;
}

inline bool MODEL::get_dynamically_compute_OBSFRUSTUM_flag() const
{
   return dynamically_compute_OBSFRUSTUM_flag;
}

inline void MODEL::set_compute_zface_height_flag(bool flag)
{
   compute_zface_height_flag=flag;
}

inline bool MODEL::get_raytrace_occluded_ground_regions_flag() const
{
   return raytrace_occluded_ground_regions_flag;
}

inline void MODEL::set_raytrace_ROI_flag(bool flag)
{
   raytrace_ROI_flag=flag;
}

inline bool MODEL::get_raytrace_ROI_flag() const
{
   return raytrace_ROI_flag;
}

inline void MODEL::set_raytrace_cell_size(double size)
{
   raytrace_cell_size=size;
}

inline void MODEL::set_speed(double s)
{
   speed=s;
}

inline double MODEL::get_speed() const
{
   return speed;
}

inline void MODEL::push_back_OBSFRUSTUM_alpha(double alpha)
{
   OBSFRUSTUM_alpha.push_back(alpha);
}

inline void MODEL::push_back_OBSFRUSTUM_beta(double beta)
{
   OBSFRUSTUM_beta.push_back(beta);
}

inline void MODEL::set_OBSFRUSTUM_roll(unsigned int OBSFRUSTUM_ID,double roll)
{
   if (OBSFRUSTUM_roll.size() <= OBSFRUSTUM_ID)
   {
      std::cout << "Error in set_OBSFRUSTUM_roll()!" << std::endl;
      std::cout << "OBSFRUSTUM_ID = " << OBSFRUSTUM_ID
                << " OBSFRUSTUM_roll.size() = " << OBSFRUSTUM_roll.size()
                << std::endl;
   }
   else
   {
      OBSFRUSTUM_roll[OBSFRUSTUM_ID]=roll;
   }
}

inline void MODEL::push_back_OBSFRUSTUM_roll(double roll)
{
   OBSFRUSTUM_roll.push_back(roll);
}

inline void MODEL::set_OBSFRUSTUM_pitch(
   unsigned int OBSFRUSTUM_ID,double pitch)
{
   if (OBSFRUSTUM_pitch.size() <= OBSFRUSTUM_ID)
   {
      std::cout << "Error in set_OBSFRUSTUM_pitch()!" << std::endl;
      std::cout << "OBSFRUSTUM_ID = " << OBSFRUSTUM_ID
                << " OBSFRUSTUM_pitch.size() = " << OBSFRUSTUM_pitch.size()
                << std::endl;
   }
   else
   {
      OBSFRUSTUM_pitch[OBSFRUSTUM_ID]=pitch;
   }
}

inline void MODEL::push_back_OBSFRUSTUM_pitch(double pitch)
{
   OBSFRUSTUM_pitch.push_back(pitch);
}

inline void MODEL::push_back_OBSFRUSTUM_yaw(double yaw)
{
   OBSFRUSTUM_yaw.push_back(yaw);
}

inline void MODEL::push_back_OBSFRUSTUM_z_base_face(double z)
{
   OBSFRUSTUM_z_base_face.push_back(z);
}

inline void MODEL::set_z_base_face(int OBSFRUSTUM_ID,double z)
{
   OBSFRUSTUM_z_base_face[OBSFRUSTUM_ID]=z;
}

inline double MODEL::get_OBSFRUSTUM_z_base_face(int OBSFRUSTUM_ID) const
{
   return OBSFRUSTUM_z_base_face[OBSFRUSTUM_ID];
}

inline osg::Node* MODEL::get_model_node_ptr()
{
   return model_node_refptr.get();
}

inline const osg::Node* MODEL::get_model_node_ptr() const
{
   return model_node_refptr.get();
}

inline void MODEL::set_path_update_distance(double d)
{
   path_update_distance=d;
}

inline double MODEL::get_path_update_distance() const
{
   return path_update_distance;
}

inline void MODEL::set_min_raytrace_range(double r)
{
   min_raytrace_range=r;
}

inline double MODEL::get_min_raytrace_range() const
{
   return min_raytrace_range;
}

inline void MODEL::set_max_raytrace_range(double r)
{
   max_raytrace_range=r;
}

inline double MODEL::get_max_raytrace_range() const
{
   return max_raytrace_range;
}

inline OBSFRUSTAGROUP* MODEL::get_OBSFRUSTAGROUP_ptr() 
{
   return OBSFRUSTAGROUP_ptr;
}

inline const OBSFRUSTAGROUP* MODEL::get_OBSFRUSTAGROUP_ptr() const
{
   return OBSFRUSTAGROUP_ptr;
}

inline void MODEL::set_OBSFRUSTAGROUP_ptr(OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr)
{
   this->OBSFRUSTAGROUP_ptr=OBSFRUSTAGROUP_ptr;
}


#endif // Model.h



 
