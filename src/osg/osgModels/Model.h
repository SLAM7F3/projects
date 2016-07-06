// ==========================================================================
// Header file for Model class
// ==========================================================================
// Last updated on 2/21/07; 6/16/07; 8/27/07; 12/23/07; 2/10/08
// ==========================================================================

#ifndef Model_H
#define Model_H

#include <string>
#include <vector>
#include <osg/Node>
#include "osg/osgGeometry/Geometrical.h"
#include "color/colorfuncs.h"
#include "astro_geo/geopoint.h"
#include "osg/osgModels/ObsFrustaGroup.h"
#include "math/threevector.h"

class ObsFrustaGroup;
class Pass;

class Model : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   Model(threevector* GO_ptr,std::string filename,int id);
   Model(Pass* PI_ptr,threevector* GO_ptr,AnimationController* AC_ptr,
         std::string filename,int id);
   virtual ~Model();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Model& m);

// Set & get methods:

   osg::Node* get_model_node_ptr();
   const osg::Node* get_model_node_ptr() const;
   void set_path_update_distance(double d);
   double get_path_update_distance() const;
   ObsFrustaGroup* get_ObsFrustaGroup_ptr();
   const ObsFrustaGroup* get_ObsFrustaGroup_ptr() const;

// Model manipulation member functions:

   virtual void set_attitude_posn(
      double curr_t,int pass_number,
      const threevector& RPY,const threevector& posn);

// Observation frusta instantiation and manipulation member functions:

   void orient_and_position_ObsFrustum(
      int first_framenumber,int last_framenumber,
      int pass_number,double delta_phi,double delta_theta,
      double z_offset,bool sinusoidal_variation_flag=false,
      int ObsFrustum_ID=0);
   void instantaneously_orient_and_position_ObsFrustum(
      double curr_t,int pass_number,double dt,
      double delta_phi,double delta_theta,double z_offset,
      int ObsFrustum_ID=0);

   void orient_and_position_instantaneous_dwell_ObsFrustum(
      int first_framenumber,int last_framenumber,
      int pass_number,double z_offset,
      ObsFrustum* dwell_ObsFrustum_ptr,ObsFrustum* FOV_ObsFrustum_ptr,
      const std::vector<geopoint>& GMTI_targets);
   bool instantaneously_orient_and_position_GMTI_dwell_ObsFrustum(
      double curr_t,int pass_number,double z_offset,
      ObsFrustum* dwell_ObsFrustum_ptr,ObsFrustum* FOV_ObsFrustum_ptr,
      const threevector& GMTI_posn,
      double dwell_range_extent,double dwell_crossrange_extent);

  protected:

  private:

   std::string model_filename;
   double path_update_distance;	// Sampling distance for model polyline path
   ObsFrustaGroup* ObsFrustaGroup_ptr;
   osg::ref_ptr<osg::Node> model_node_refptr;
   osg::ref_ptr<osg::Group> group_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Model& m);

//   void set_attitude_posn(
//      int pass_number,
//      const std::vector<threevector>& RPY,
//      const std::vector<threevector>& posn);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline osg::Node* Model::get_model_node_ptr()
{
   return model_node_refptr.get();
}

inline const osg::Node* Model::get_model_node_ptr() const
{
   return model_node_refptr.get();
}

inline void Model::set_path_update_distance(double d)
{
   path_update_distance=d;
}

inline double Model::get_path_update_distance() const
{
   return path_update_distance;
}

inline ObsFrustaGroup* Model::get_ObsFrustaGroup_ptr() 
{
   return ObsFrustaGroup_ptr;
}

inline const ObsFrustaGroup* Model::get_ObsFrustaGroup_ptr() const
{
   return ObsFrustaGroup_ptr;
}


#endif // Model.h



 
