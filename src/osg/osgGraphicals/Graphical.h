// ==========================================================================
// Header file for (pure virtual) GRAPHICAL class
// ==========================================================================
// Last modified on 6/3/09; 1/15/11; 1/21/13; 4/6/14
// ==========================================================================

#ifndef GRAPHICAL_H
#define GRAPHICAL_H

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <osg/Array>
#include <osg/Depth>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Quat>
#include "osg/osgGraphicals/instantaneous_obs.h"
#include "math/lttwovector.h"
#include "math/threevector.h"
#include "datastructures/Triple.h"

class AnimationController;

class Graphical
{

  public:

// Initialization, constructor and destructor functions:

   Graphical();
   Graphical(const int p_ndims,int id,AnimationController* AC_ptr=NULL);
   virtual ~Graphical();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Graphical& g);

// Set & get member functions:

   std::string get_name() const;
   void set_ndims(int d);
   int get_ndims() const;
   void set_ID(int id);
   int get_ID() const;
   void set_size(float s);
   float get_size() const;
   osg::Depth* get_depth_on_ptr();
   const osg::Depth* get_depth_on_ptr() const;

// Instantaneous observation manipulation member functions:

   bool particular_time_obs_exists(double t,int pass_number) const;
   instantaneous_obs* get_particular_time_obs(double t,int pass_number) const;
   instantaneous_obs* get_all_particular_time_observations(double t) const;
   std::vector<instantaneous_obs*> get_all_observations() const;
   void consolidate_instantaneous_observations(
      double t,Graphical* other_graphical_ptr);

// Drawing member functions:

   void adjust_depth_buffering(bool force_display_flag,osg::Geode* geode_ptr);
   virtual void dirtyDisplay();

// Animation methods:

   void set_AnimationController_ptr(AnimationController* AC_ptr);
   AnimationController* get_AnimationController_ptr() const;
   void set_stationary_Graphical_flag(bool flag);
   bool get_stationary_Graphical_flag() const;
   int get_curr_framenumber() const;
   int get_first_framenumber() const;
   int get_last_framenumber() const;
   double get_curr_t() const; 
   double get_initial_t() const; 

// PAT methods:

   void set_PAT(double t,int pass_number);
   void set_PAT_pivot(const threevector& p);
   osg::PositionAttitudeTransform* get_PAT_ptr();
   const osg::PositionAttitudeTransform* get_PAT_ptr() const;

// Coordinate set & get methods:

   void set_UVW_scales(
      double curr_t,int pass_number,
      double Uscale,double Vscale,double Wscale=1.0);
   bool get_UVW_scales(
      double curr_t,int pass_number,
      double& Uscale,double& Vscale,double& Wscale) const;
   void set_UVW_dirs(
      double curr_t,int pass_number,
      const threevector& Uhat,const threevector& Vhat);
   bool get_UVW_dirs(
      double curr_t,int pass_number,
      threevector& Uhat,threevector& Vhat,threevector& What) const;

   void set_UVW_coords(double curr_t,int pass_number,const threevector& p3);
   bool get_UVW_coords(double curr_t,int pass_number,threevector& p3) const;
   void set_UVW_velocity(double curr_t,int pass_number,const threevector& v3);
   bool get_UVW_velocity(double curr_t,int pass_number,threevector& v3) const;
   bool get_velocity(
      double curr_t,int pass_number,double dt,threevector& velocity) const;

   bool get_transformed_UVW_coords(
      double curr_t,int pass_number,threevector& p_transformed) const;
   void adjust_UVW_coords(
      double curr_t,int pass_number,const threevector& dp);

   void set_vertices(double curr_t,int pass_number,
                     const std::vector<threevector>& v);
   bool get_vertices(double curr_t,int pass_number,
                     std::vector<threevector>& v) const;
   void set_quaternion(double curr_t,int pass_number,
                       const threevector& u_hat,const threevector& v_hat);
   void set_quaternion(double curr_t,int pass_number,const osg::Quat& q);
   bool get_quaternion(double curr_t,int pass_number,osg::Quat& q);
   void set_scale(double curr_t,int pass_number,const threevector& s);
   bool get_scale(double curr_t,int pass_number,threevector& s) const;
   void set_score(double curr_t,int pass_number,double score);
   bool get_score(double curr_t,int pass_number,double& score);
   void reset_all_scores(double score);
   void set_index(double curr_t,int pass_number,int index);
   bool get_index(double curr_t,int pass_number,int& index);
   
// Global manipulation member functions:

   void absolute_posn(double curr_t,int pass_number,const threevector& posn);
   void translate(double curr_t,int pass_number,const threevector& trans);
   void scale(double curr_t,int pass_number,const threevector& scale_origin,
              const threevector& scale);
   void scale_and_translate(
      double curr_t,int pass_number,const threevector& scale_origin,
      const threevector& scale,const threevector& trans);
   void rotate_about_specified_origin(
      double curr_t,int pass_number,const threevector& rotation_origin,
      const threevector& new_xhat,const threevector& new_yhat);
   void rotate_about_specified_origin_then_translate(
      double curr_t,int pass_number,const threevector& rotation_origin,
      const threevector& new_xhat,const threevector& new_yhat,
      const threevector& trans);
   void rotate_about_specified_origin_then_translate(
      double curr_t,int pass_number,const threevector& rotation_origin,
      const genmatrix& R,const threevector& trans);
   void scale_and_rotate(
      double curr_t,int pass_number,const threevector& origin,
      const genmatrix& R,const threevector& scale);
   void scale_rotate_and_then_translate(
      double curr_t,int pass_number,const threevector& origin,
      const genmatrix& R,const threevector& scale,const threevector& trans);
   void rotate_about_zaxis(
      double curr_t,int pass_number,const threevector& rotation_origin,
      double phi_z);

// Roll-pitch-yaw manipulation member functions:

   virtual void set_attitude_posn(
      double curr_t,int pass_number,
      const threevector& RPY,const threevector& posn);
   void set_attitude_posn(
      double curr_t,int pass_number,
      const threevector& RPY,const threevector& posn,double init_yaw_angle);
   void set_attitude_posn(
      int pass_number,const std::vector<threevector>& RPY,
      const std::vector<threevector>& posn,int init_frame=0);

   void set_scale_attitude_posn(
      double curr_t,int pass_number,
      double scale_factor,const threevector& RPY,const threevector& posn);
   void set_scale_attitude_posn(
      double curr_t,int pass_number,
      const threevector& scale,const threevector& RPY,
      const threevector& posn);

   void set_scale_attitude_posn(
      int pass_number,double scale_factor,
      const std::vector<threevector>& RPY,
      const std::vector<threevector>& posn);
   void set_scale_attitude_posn(
      int pass_number,const threevector& scale,
      const std::vector<threevector>& RPY,
      const std::vector<threevector>& posn);

// Coordinate manipulation methods:

   void set_mask(double t,int pass_number,bool mask_flag);
   bool get_mask(double t,int pass_number) const;
   
   int count_image_appearances(
      int pass_number,unsigned int start_imagenumber,
      unsigned int stop_imagenumber);
   void set_coords_manually_manipulated(double t,int pass_number);
   bool get_coords_manually_manipulated(double t,int pass_number);

  protected:

   bool stationary_Graphical_flag;
   int ID;
   std::string ndims_label,Graphical_name;
   float size[4];
   AnimationController* AnimationController_ptr;
   osg::ref_ptr<osg::PositionAttitudeTransform> PAT_refptr;
   osg::ref_ptr<osg::Depth> depth_off_refptr;
   osg::ref_ptr<osg::Depth> depth_on_refptr;

  private:

   int ndims;

// Store Graphical's raw UVW and quaternion (U'V'W' Q', U"V"W" Q"",
// etc) measured coordinates as functions of (time,passnumber) within
// STL map *coords_map_ptr.  For some time t, there can be zero, one
// or many UVW Q coordinates associated with the same Graphical...

// Second bool element of Triple stores mask bool value.

// Third bool element of Triple stores coords_manually_manipulated
// bool value.

   typedef std::map<twovector,Triple<instantaneous_obs,bool,bool>,
      lttwovector > COORDS_MAP;
   COORDS_MAP* coords_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Graphical& g);

   void set_coords_obs(
      double t,int pass_number,const instantaneous_obs& curr_obs);
   void check_Graphical_stationarity(
      int pass_number,const instantaneous_obs& curr_obs);

   COORDS_MAP::iterator get_coords_map_iterator(
      double t,int pass_number) const
      {
         return coords_map_ptr->find(twovector(t,pass_number));
      }

   void update_PAT_posn(const threevector& p);
   void update_PAT_attitude(const osg::Quat& q);
   void update_PAT_scale(const threevector& s);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline std::string Graphical::get_name() const
{
   return Graphical_name;
}

inline void Graphical::set_ndims(int d)
{
   ndims=d;
}

inline int Graphical::get_ndims() const
{
   return ndims;
}

inline void Graphical::set_size(float s)
{
   size[get_ndims()]=s;
}

inline void Graphical::set_ID(int id) 
{
   ID=id;
}

inline int Graphical::get_ID() const
{
   return ID;
}

inline void Graphical::set_AnimationController_ptr(
   AnimationController* AC_ptr)
{
   AnimationController_ptr=AC_ptr;
}

inline AnimationController* Graphical::get_AnimationController_ptr() const
{
   return AnimationController_ptr;
}

inline void Graphical::set_stationary_Graphical_flag(bool flag) 
{
   stationary_Graphical_flag=flag;
}

inline bool Graphical::get_stationary_Graphical_flag() const
{
   return stationary_Graphical_flag;
}

inline float Graphical::get_size() const
{
   if (ndims < 2)
   {
      std::cout << "Error in Graphical::get_size()! ndims = " << ndims
                << std::endl;
   }
   return size[ndims];
}

inline void Graphical::update_PAT_posn(const threevector& p)
{
   if (ndims==2)
   {
      PAT_refptr->setPosition(osg::Vec3d(p.get(0),0,p.get(1)));
   }
   else if (ndims==3)
   {
      PAT_refptr->setPosition(osg::Vec3d(p.get(0),p.get(1),p.get(2)));
   }
}

inline void Graphical::update_PAT_attitude(const osg::Quat& q)
{
   PAT_refptr->setAttitude(q);
}


inline void Graphical::update_PAT_scale(const threevector& s)
{
   if (ndims==2)
   {
      PAT_refptr->setScale(osg::Vec3d(s.get(0),1,s.get(1)));
   }
   else if (ndims==3)
   {
      PAT_refptr->setScale(osg::Vec3d(s.get(0),s.get(1),s.get(2)));
   }
}

inline osg::PositionAttitudeTransform* Graphical::get_PAT_ptr()
{
   return PAT_refptr.get();
}

inline const osg::PositionAttitudeTransform* Graphical::get_PAT_ptr() const
{
   return PAT_refptr.get();
}

inline osg::Depth* Graphical::get_depth_on_ptr()
{
   return depth_on_refptr.get();
}

inline const osg::Depth* Graphical::get_depth_on_ptr() const
{
   return depth_on_refptr.get();
}

// Member function 

inline bool Graphical::particular_time_obs_exists(double t,int pass_number) 
   const
{
//   cout << "inside Graphical::particular_time_obs_exists()" << endl;
//   cout << "t = " << t << " pass_number = " << pass_number << endl;
//   COORDS_MAP::iterator coords_iter=get_coords_map_iterator(t,pass_number);
//   return (coords_iter != coords_map_ptr->end());

   return ( coords_map_ptr->find(twovector(t,pass_number)) != 
            coords_map_ptr->end());
}

#endif // Graphical.h



