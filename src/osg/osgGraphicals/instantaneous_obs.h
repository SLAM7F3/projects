// ==========================================================================
// Header file for INSTANTANEOUS_OBS class
// ==========================================================================
// Last modified on 5/31/10; 1/21/13; 2/6/13
// ==========================================================================

#ifndef INSTANTANEOUS_OBS_H
#define INSTANTANEOUS_OBS_H

#include <map>
#include <vector>
#include <osg/Quat>
#include "math/fourvector.h"
#include "math/threevector.h"

class instantaneous_obs
{

  public:

   typedef std::map<int,int> INT_MAP;
   typedef std::map<int,double> DOUBLE_MAP;
   typedef std::map<int,threevector> THREEVECTOR_MAP;

// Independent int = pass number
// Dependent threevector = XYZ coords

   typedef std::map<int,fourvector> FOURVECTOR_MAP;
   typedef std::vector< std::vector<threevector> > VECTHREEVECTOR_VEC;

// Initialization, constructor and destructor functions:

   instantaneous_obs();
   instantaneous_obs(double curr_t);
   instantaneous_obs(const instantaneous_obs& io);
   ~instantaneous_obs();
   instantaneous_obs& operator= (const instantaneous_obs& i);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const instantaneous_obs& i);

// Set & get member functions:

   void set_t(double time);
   double get_t() const;
   int get_npasses() const;
   std::vector<int>& get_pass_numbers();
   THREEVECTOR_MAP* get_multiple_image_coords_ptr();
   const THREEVECTOR_MAP* get_multiple_image_coords_ptr() const;

   void set_UVW_scales(double Uscale,double Vscale,double Wscale=1.0);
   void get_UVW_scales(double& Uscale,double& Vscale,double& Wscale);
   void set_UVW_worldspace_dirs(
      const threevector& Uhat,const threevector& Vhat);
   void get_UVW_worldspace_dirs(threevector& Uhat,threevector& Vhat,
                                threevector& What);

// Pass number manipulation member functions:

   int find_first_passnumber_greater_than_input(int p) const;
   void change_passnumber(int p_old,int p_new);

// Coordinate insertion member functions:

   void insert_UVW_coords(int pass_number,const threevector& p);
   void insert_vertices(
      int pass_number,const std::vector<threevector>& v);
   void insert_quaternion(int pass_number,const osg::Quat& q);
   void insert_scale(int pass_number,const threevector& scale);
   void insert_velocity(int pass_number,const threevector& velocity);
   void insert_score(int pass_number,double score);
   void insert_index(int pass_number,int index);

   bool check_for_pass_entry(int pass_number);
   void consolidate_image_coords(instantaneous_obs* other_obs_ptr);

// Coordinate retrieval member functions:

   bool retrieve_UVW_coords(int pass_number,threevector& p3) const;
   threevector retrieve_UVW_coords(int pass_number) const;
   bool retrieve_transformed_UVW_coords(
      int pass_number,threevector& p_transformed) const;
   threevector retrieve_transformed_UVW_coords(int pass_number) const;

   bool retrieve_vertices(int pass_number,std::vector<threevector>& v) const;
//   std::vector<threevector> retrieve_vertices(int pass_number) const;
   bool retrieve_quaternion(int pass_number,osg::Quat& q) const;
   osg::Quat retrieve_quaternion(int pass_number) const;
   bool retrieve_scale(int pass_number,threevector& s) const;
   threevector retrieve_scale(int pass_number) const;
   bool retrieve_velocity(int pass_number,threevector& v) const;
   threevector retrieve_velocity(int pass_number) const;
   bool retrieve_score(int pass_number,double& score) const;
   double retrieve_score(int pass_number) const;
   bool retrieve_index(int pass_number,int& index) const;
   int retrieve_index(int pass_number) const;

// Coordinate modification member functions:

   void change_UVW_coords(int pass_number,const threevector& p);
   void change_vertices(int pass_number,const std::vector<threevector>& v);
   void change_quaternion(int pass_number,const osg::Quat& q);
   void change_scale(int pass_number,const threevector& s);
   void change_velocity(int pass_number,const threevector& v);
   void change_score(int pass_number,double score);
   void change_index(int pass_number,int index);

// Coordinate deletion member functions:

   void delete_all_coords(int pass_number);
   void delete_UVW_coords(int pass_number);
   void delete_vertices(int pass_number);
   void delete_quaternion(int pass_number);
   void delete_scale(int pass_number);
   void delete_velocity(int pass_number);
   void delete_score(int pass_number);
   void delete_index(int pass_number);

  private:

// Instantaneous time which need NOT correspond to any particular
// image time of some video pass.  Time t represents the *independent*
// variable:

   double t;

// Multi-pass (U,V,W) coordinate measurements at time t.  The
// indepedent integer indexing entry in the following STL maps
// corresponds to pass number:

   std::vector<int> pass_numbers;
   THREEVECTOR_MAP* multiple_image_coords_ptr;
   VECTHREEVECTOR_VEC* multiple_image_vertices_vec_ptr;

   FOURVECTOR_MAP* multiple_image_attitude_ptr;
   THREEVECTOR_MAP* multiple_image_scale_ptr;
   THREEVECTOR_MAP* multiple_image_velocity_ptr;
   DOUBLE_MAP* multiple_image_score_ptr;
   INT_MAP* multiple_image_index_ptr;

   double U_scale,V_scale,W_scale;
   threevector U_hat,V_hat,W_hat;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const instantaneous_obs& i);

   THREEVECTOR_MAP::iterator retrieve_multiimage_coords_iter(
      int pass_number) const;
   VECTHREEVECTOR_VEC::iterator retrieve_multiimage_vertices_iter(
      int pass_number) const;
   FOURVECTOR_MAP::iterator retrieve_multiimage_attitude_iter(
      int pass_number) const;
   THREEVECTOR_MAP::iterator retrieve_multiimage_scale_iter(
      int pass_number) const;
   THREEVECTOR_MAP::iterator retrieve_multiimage_velocity_iter(
      int pass_number) const;
   DOUBLE_MAP::iterator retrieve_multiimage_score_iter(int pass_number) const;
   INT_MAP::iterator retrieve_multiimage_index_iter(int pass_number) const;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void instantaneous_obs::set_t(double time)
{
   t=time;
}

inline double instantaneous_obs::get_t() const
{
   return t;
}

inline int instantaneous_obs::get_npasses() const
{
   return multiple_image_coords_ptr->size();
}

inline instantaneous_obs::THREEVECTOR_MAP*  
   instantaneous_obs::get_multiple_image_coords_ptr()
{
   return multiple_image_coords_ptr;
}

inline const instantaneous_obs::THREEVECTOR_MAP*  
   instantaneous_obs::get_multiple_image_coords_ptr() const
{
   return multiple_image_coords_ptr;
}

// ---------------------------------------------------------------------
inline void instantaneous_obs::set_UVW_scales(
   double Uscale,double Vscale,double Wscale)
{
   U_scale=Uscale;
   V_scale=Vscale;
   W_scale=Wscale;
}

inline void instantaneous_obs::get_UVW_scales(
   double& Uscale,double& Vscale,double& Wscale)
{
   Uscale=U_scale;
   Vscale=V_scale;
   Wscale=W_scale;
}

// ---------------------------------------------------------------------
// Member function set_UVW_worldspace_dirs takes in two direction
// vectors which are assumed to define an instantaneous right-handed
// orthonormal basis.  It resets the U_hat, V_hat and W_hat member
// threevectors to this basis' direction vectors.

inline void instantaneous_obs::set_UVW_worldspace_dirs(
   const threevector& Uhat,const threevector& Vhat)
{
   U_hat=Uhat;
   V_hat=Vhat;
   W_hat=U_hat.cross(V_hat);
}

inline void instantaneous_obs::get_UVW_worldspace_dirs(
   threevector& Uhat,threevector& Vhat,threevector& What)
{
   Uhat=U_hat;
   Vhat=V_hat;
   What=W_hat;
}



#endif // instantaneous_obs.h



