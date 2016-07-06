// ==========================================================================
// header for VolumetricCoincidenceProcessor class
// ==========================================================================
// Last updated on 9/1/12; 1/17/13; 1/22/13; 4/5/14
// ==========================================================================

#ifndef VolumetricCoincidenceProcessor_H
#define VolumetricCoincidenceProcessor_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "datastructures/Linkedlist.h"
#include "math/fourvector.h"
#include "math/mathfuncs.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"
#include "coincidence_processing/voxel_coords.h"

// The following pair type stores both integer voxel counts as well as
// fractional voxel probability information:

typedef std::pair<int,float> Voxel_type;

// VOXEL_MAP stores count,prob pairs as a function of 8-byte long
// integer keys:

typedef std::map<long,Voxel_type> VOXEL_MAP;

class mypolynomial;

class VolumetricCoincidenceProcessor
{

  public:

// Initialization, constructor and destructor functions:

   VolumetricCoincidenceProcessor(int n_points=-1);
   VolumetricCoincidenceProcessor(const VolumetricCoincidenceProcessor& v);
   ~VolumetricCoincidenceProcessor();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const VolumetricCoincidenceProcessor& vcp);

   void initialize_coord_system(
      const threevector& XYZ_minimum,const threevector& XYZ_maximum,
      float binlength);
   void initialize_coord_system(
      const threevector& XYZ_minimum,const threevector& XYZ_maximum,
      float XY_binlength,float Z_binlength);

// Set & get member functions:

   unsigned int size() const;
   unsigned int get_mdim() const;
   unsigned int get_ndim() const;
   unsigned int get_pdim() const;

   double get_xlo() const;
   double get_xhi() const;
   double get_ylo() const;
   double get_yhi() const;
   double get_zlo() const;
   double get_zhi() const;
   double get_dx() const;
   double get_dy() const;
   double get_dz() const;

// Counts accumulation member functions:

   int increment_voxel_counts(long key,int delta_counts=1);
   void set_voxel_prob(
      unsigned int m,unsigned int n,unsigned int p,float prob);
   void set_voxel_prob(long key,float prob);
   void increment_voxel_prob(long key,float prob);

   void accumulate_points(double x,double y,double z);
   void accumulate_points_and_counts(double x,double y,double z,int n_counts);

   void accumulate_points_and_probs(double x,double y,double z,double p);
   void accumulate_points_and_calculate_probs(
      double x,double y,double z,double numer,double denom);

   void accumulate_points_and_increment_probs(
      double x,double y,double z,double p);
   void accumulate_points(const threevector& curr_point);
   void accumulate_points(const threevector& curr_point,int n_counts);
   void generate_counts_histogram();
   void generate_p_vs_z_profiles();
   void generate_p_distribution();
   void generate_xyz_profiles(std::string output_subdir);
   threevector compute_COM();

// Gridding member functions:

   bool xyz_inside_volume(double x,double y,double z) const;
   bool xyz_inside_volume(const threevector& V) const;
   bool mnp_inside_volume(unsigned int m,unsigned int n,unsigned int p) const;
   unsigned int x_to_m(double x) const;
   unsigned int y_to_n(double y) const;
   unsigned int z_to_p(double z) const;
   void xyz_to_mnp(double x,double y,double z,
                   unsigned int& m,unsigned int& n,unsigned int& p) const;
   void xyz_to_mnp(const threevector& V,
                   unsigned int& m,unsigned int& n,unsigned int& p) const;
   voxel_coords xyz_to_mnp(const threevector& V) const;
   threevector mnp_to_xyz(unsigned int m,unsigned int n,unsigned int p) const;
   float m_to_x(unsigned int m) const;
   float n_to_y(unsigned int n) const;
   float p_to_z(unsigned int p) const;
   threevector mnp_to_xyz(const voxel_coords& C) const;
   long mnp_to_key(unsigned int m,unsigned int n,unsigned int p) const;
   long mnp_to_key(const voxel_coords& C) const;
   void key_to_mnp(
      long key,unsigned int& m,unsigned int& n,unsigned int& p) const;
   unsigned int key_to_m(long key) const;
   unsigned int key_to_n(long key) const;
   unsigned int key_to_p(long key) const;
   voxel_coords key_to_mnp(long key) const;
   long xyz_to_key(double x,double y,double z) const;
   long xyz_to_key(const threevector& V) const;
   threevector key_to_xyz(long key) const;
   float key_to_x(long key) const;
   float key_to_y(long key) const;
   float key_to_z(long key) const;

// Counts and probability retrieval member functions:

   VOXEL_MAP::iterator mnp_to_voxel_iterator(
      unsigned int m,unsigned int n,unsigned int p) const;

   bool key_to_voxel_counts_and_prob(long key,int& counts,double& prob) const;
   bool mnp_to_voxel_counts_and_prob(
      int m,int n,int p,int& counts,double& prob) const;
   double column_prob_integral(int m,int n,double z_start);
   void integrate_probs_within_column_integrals();
   double get_cumulative_prob_integral(
      int m,int n,double z_start,bool fall_downwards_flag);
   double get_cumulative_prob_integral(
      unsigned int m,unsigned int n,unsigned int p_start,unsigned int p_stop,
      unsigned int p_step);
   int compute_counts_sum();
   double compute_prob_integral();
   void set_probs_to_renormalized_counts();

// Noise reduction member functions:

   void delete_all_voxels();
   void delete_isolated_voxels(
      int delta_vox,int min_neighbors,int min_avg_counts);
   void delete_isolated_voxels(
      int delta_vox,int min_neighbors,double min_Z_threshold,
      int min_avg_counts);
   bool isolated_voxel(
      long curr_key,int delta_vox,int min_neighbors,
      int min_avg_counts=1) const;
   void delete_small_count_voxels(int min_voxel_counts);
   void delete_large_count_voxels(int max_voxel_counts);

   void bbox_voxel_coords(
      const threevector& curr_point,const threevector& extent,
      unsigned int& min_m,unsigned int& min_n,unsigned int& min_p,
      unsigned int& max_m,unsigned int& max_n,unsigned int& max_p);
   bool locate_nearby_neighbor(
      const threevector& curr_point,const threevector& extent);
   double detect_trivial_ground_plane();
   void delete_points_below_zplane(float z_min);
   void delete_points_above_zplane(float z_max);

// Range tail squishing member functions:

   void squish_range_tails(double squish_distance,const threevector& r_hat);
   Linkedlist<voxel_coords>* relative_voxel_coords_for_line_segment(
         const threevector& start_point,const threevector& stop_point);
   Linkedlist<long>* voxels_along_line_segment(
      const threevector& start_point,const threevector& stop_point);
   void trim_visible_surface_depths(
      Linkedlist<voxel_coords>* rel_coords_list_ptr);
   void order_voxels_by_range(const threevector& r_hat);

// Illumination counts and probabilities member functions:

   void renormalize_counts_into_probs();
   void copy_counts_onto_probs();
   double find_signal_reflectivity_threshold(
      twoDarray* illumpattern_twoDarray_ptr,int min_illum_counts,
      double e_folding_distance,bool enforce_prob_limits_flag=true);

// Voxelized data export member functions:

   void retrieve_XYZ_points(std::vector<threevector>& XYZ_points);
   void retrieve_XYZP_points(
      std::vector<double>* X_ptr,std::vector<double>* Y_ptr,
      std::vector<double>* Z_ptr,std::vector<double>* P_ptr,
      double min_prob_threshold=0,bool perturb_voxels_flag=false);

// Pure noise characterization member functions:

   std::vector<int> extract_pure_noise_counts(
      const threevector& A_hat,const threevector& B_hat,
      const threevector& origin,
      double Amin,double Amax,double Bmin,double Bmax,double Zmin,double Zmax);
   void assign_pure_noise_probabilities(
      const std::vector<double>& pure_noise_probs);

  private:

   unsigned int mdim,ndim,pdim;
   int n_expected_points;
   int imagenumber;
   float xlo,xhi,ylo,yhi,zlo,zhi;
   float dx,dy,dz;
   std::vector<std::pair<float,long> >* ordered_voxels_ptr;

   VOXEL_MAP* voxels_map_ptr;

     // *voxels_map_ptr independent var : voxel key
     // dependent var : pair of integer counts and double p_det

   void initialize_member_objects();
   void allocate_member_objects();
   void docopy(const VolumetricCoincidenceProcessor& v);

};

// ---------------------------------------------------------------------
// Inlined methods:
// ---------------------------------------------------------------------

inline unsigned int VolumetricCoincidenceProcessor::size() const
{
   return voxels_map_ptr->size();
}

inline unsigned int VolumetricCoincidenceProcessor::get_mdim() const
{
   return mdim;
}

inline unsigned int VolumetricCoincidenceProcessor::get_ndim() const
{
   return ndim;
}

inline unsigned int VolumetricCoincidenceProcessor::get_pdim() const
{
   return pdim;
}

inline double VolumetricCoincidenceProcessor::get_xlo() const
{
   return xlo;
}

inline double VolumetricCoincidenceProcessor::get_xhi() const
{
   return xhi;
}

inline double VolumetricCoincidenceProcessor::get_ylo() const
{
   return ylo;
}

inline double VolumetricCoincidenceProcessor::get_yhi() const
{
   return yhi;
}

inline double VolumetricCoincidenceProcessor::get_zlo() const
{
   return zlo;
}

inline double VolumetricCoincidenceProcessor::get_zhi() const
{
   return zhi;
}

inline double VolumetricCoincidenceProcessor::get_dx() const
{
   return dx;
}

inline double VolumetricCoincidenceProcessor::get_dy() const
{
   return dy;
}

inline double VolumetricCoincidenceProcessor::get_dz() const
{
   return dz;
}

// ---------------------------------------------------------------------
inline bool VolumetricCoincidenceProcessor::xyz_inside_volume(
   double x,double y,double z) const
{
   return (
      x > xlo && x < xhi && y > ylo && y < yhi && z > zlo && z < zhi);
}

inline bool VolumetricCoincidenceProcessor::xyz_inside_volume(
   const threevector& V) const
{
   return (
      V.get(0) > xlo && V.get(0) < xhi && V.get(1) > ylo && V.get(1) < yhi 
      && V.get(2) > zlo && V.get(2) < zhi);
}

inline bool VolumetricCoincidenceProcessor::mnp_inside_volume(
   unsigned int m,unsigned int n,unsigned int p) const
{
   return (m >= 0 && m < mdim && n >= 0 && n < ndim
           && p >= 0 && p < pdim);
}

inline voxel_coords VolumetricCoincidenceProcessor::xyz_to_mnp(
   const threevector& V) const 
{
   unsigned int m,n,p;
   xyz_to_mnp(V,m,n,p);
   return voxel_coords(m,n,p);
}

// ---------------------------------------------------------------------
// Member function key_to_xyz takes in an integer key which labels a
// unique, discrete voxel.  If the key >= 0, it returns the voxel's x,
// y and z coordinates.

inline threevector VolumetricCoincidenceProcessor::key_to_xyz(long key) const
{
   unsigned int m,n,p;
   key_to_mnp(key,m,n,p);
   return mnp_to_xyz(m,n,p);
}

inline float VolumetricCoincidenceProcessor::key_to_x(long key) const
{
   return m_to_x(key_to_m(key));
}

inline float VolumetricCoincidenceProcessor::key_to_y(long key) const
{
   return n_to_y(key_to_n(key));
}

inline float VolumetricCoincidenceProcessor::key_to_z(long key) const
{
   return p_to_z(key_to_p(key));
}

// ---------------------------------------------------------------------
// Member function mnp_to_xyz takes in discrete voxel coordinates
// (m,n,p) (which do not necessarily have to lie within the allowed
// VCP volume) and returns the voxel's (x,y,z) coordinates.  We
// subtract x,y,z member variable offsets to allow for image dependent
// (x,y,z) renormalizations.

inline threevector VolumetricCoincidenceProcessor::mnp_to_xyz(
   unsigned int m,unsigned int n,unsigned int p) const
{
   return threevector(xlo+m*dx,ylo+n*dy,zlo+p*dz);
}

inline float VolumetricCoincidenceProcessor::m_to_x(unsigned int m) const
{
   return xlo+m*dx;
}

inline float VolumetricCoincidenceProcessor::n_to_y(unsigned int n) const
{
   return ylo+n*dy;
}

inline float VolumetricCoincidenceProcessor::p_to_z(unsigned int p) const
{
   return zlo+p*dz;
}

inline threevector VolumetricCoincidenceProcessor::mnp_to_xyz(
   const voxel_coords& C) const
{
   return mnp_to_xyz(C.m,C.n,C.p);
}

// ---------------------------------------------------------------------
inline long VolumetricCoincidenceProcessor::mnp_to_key(
   unsigned int m,unsigned int n,unsigned int p) const
{
   return long(mdim)*long(ndim)*long(p)+long(mdim)*long(n)+long(m);

/*
   long key=long(mdim)*long(ndim)*long(p)+long(mdim)*long(n)+long(m);

   if (key < 0) 
   {

      std::cout << "inside VCP::mnp_to_key()" << std::endl;
      std::cout << "m = " << m << " n = " << n << " p = " << p << std::endl;
      std::cout << "mdim = " << mdim << " ndim = " << ndim << std::endl;
      std::cout << "key = " << key << std::endl;
      std::cout << "sizeof(long) = " << sizeof(long) << std::endl;
      outputfunc::enter_continue_char();
   }
   return key;
*/
}

inline long VolumetricCoincidenceProcessor::mnp_to_key(const voxel_coords& C)
   const
{
   return mdim*ndim*C.p+mdim*C.n+C.m;
}

// ---------------------------------------------------------------------
inline void VolumetricCoincidenceProcessor::key_to_mnp(
   long key,unsigned int& m,unsigned int& n,unsigned int& p) const
{
   m=key%mdim;
   key=(key-m)/mdim;
   n=key%ndim;
   p=(key-n)/ndim;
}

inline unsigned int VolumetricCoincidenceProcessor::key_to_m(long key) const
{
   return key%mdim;
}

inline unsigned int VolumetricCoincidenceProcessor::key_to_n(long key) const
{
   key=(key-key%mdim)/mdim;
   return key%ndim;
}

inline unsigned int VolumetricCoincidenceProcessor::key_to_p(long key) const
{
   key=(key-key%mdim)/mdim;
   return (key-key%ndim)/ndim;
}

inline voxel_coords VolumetricCoincidenceProcessor::key_to_mnp(long key) const
{
   unsigned int m=key%mdim;
   key=(key-m)/mdim;
   unsigned int n=key%ndim;
   unsigned int p=(key-n)/ndim;
   return voxel_coords(m,n,p);
}

// ---------------------------------------------------------------------
// Member function xyz_to_key takes in an (x,y,z) point which is
// assumed to reside within the allowed VolumetricCoincidenceProcessor
// volume.  It returns a unique integer which labels the voxel to
// which this point belongs.

inline long VolumetricCoincidenceProcessor::xyz_to_key(
   double x,double y,double z) const
{
   unsigned int m=basic_math::round((x-xlo)/dx);
   unsigned int n=basic_math::round((y-ylo)/dy);
   unsigned int p=basic_math::round((z-zlo)/dz);
//   std::cout << "m = " << m << " n = " << n << " p = " << p << std::endl;
//   std::cout << "mdim = " << mdim << " ndim = " << ndim << " pdim = " << pdim
//             << std::endl;
   return mnp_to_key(m,n,p);
}

inline long VolumetricCoincidenceProcessor::xyz_to_key(const threevector& V)
   const
{
   unsigned int m=basic_math::round((V.get(0)-xlo)/dx);
   unsigned int n=basic_math::round((V.get(1)-ylo)/dy);
   unsigned int p=basic_math::round((V.get(2)-zlo)/dz);
   return mnp_to_key(m,n,p);

// On 5/7/04, we discovered that replacing rounding with truncation
// leads to Risley illumination flower pattern irregularities which
// look visibly jarring (though they might turn out to be fairly
// insignificant for image processing purposes).  As of 5/7/04, we
// have decided to restore rounding in this method...

/*
// By truncating rather than rounding, we sacrifice precision for
// speed:

   int m=int((V.x-xlo)/dx);
   int n=int((V.y-ylo)/dy);
   int p=int((V.z-zlo)/dz);
   return mdim*ndim*p+mdim*n+m;
*/
}

inline VOXEL_MAP::iterator 
VolumetricCoincidenceProcessor::mnp_to_voxel_iterator(
   unsigned int m,unsigned int n,unsigned int p) const
{
//   std::cout << "inside VCP::mnp_to_voxel_iterator()" << std::endl;
//   std::cout << "m = " << m << " n = " << n << " p = " << p << std::endl;
   return voxels_map_ptr->find(mnp_to_key(m,n,p));
}

#endif //  VolumetricCoincidenceProcessor_H

