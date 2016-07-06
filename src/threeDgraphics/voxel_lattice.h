// ==========================================================================
// Header file for VOXEL_LATTICE class
// ==========================================================================
// Last modified on 2/22/06; 12/10/06
// ==========================================================================

#ifndef VOXEL_LATTICE_H
#define VOXEL_LATTICE_H

#include <set>
#include <vector>
#include "math/fourvector.h"
#include "datastructures/Hashtable.h"
#include "math/threevector.h"
#include "threeDgraphics/voxel_coords.h"

// The following pair type stores both integer voxel counts as well as
// fractional voxel probability information:

typedef std::pair<int,double> Voxel_type;

class voxel_lattice
{

  public:

// Initialization, constructor and destructor functions:

   voxel_lattice();
   voxel_lattice(const voxel_lattice& c);
   ~voxel_lattice();
   voxel_lattice& operator= (const voxel_lattice& c);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const voxel_lattice& c);

// Set & get member functions:

   void set_delta(const threevector& d);
   Hashtable<Voxel_type>* get_voxels_hashtable_ptr();

// Lattice initialization member functions:

   void initialize(
      int n_expected_points,double min_x,double max_x,
      double min_y,double max_y,double min_z,double max_z);
   void initialize(std::vector<fourvector> const *xyzp_point_ptr);
   void compute_extremal_values(
      std::vector<fourvector> const *xyzp_point_ptr);
   void reset_coordinate_system();

// Discrete <--> continous coordinate conversion member functions:

   bool xyz_inside_volume(const threevector& V) const;
   bool mnp_inside_volume(unsigned int m,unsigned int n,unsigned int p) const;
   bool voxel_inside_volume(const voxel_coords& curr_voxel) const;
   void xyz_to_mnp(
      const threevector& V,unsigned int& m,unsigned int& n,
      unsigned int& p) const;
   voxel_coords xyz_to_mnp(const threevector& V) const;
   threevector key_to_xyz(unsigned int key) const;
   float key_to_y(unsigned int key) const;
   float key_to_z(unsigned int key) const;
   threevector mnp_to_xyz(unsigned int m,unsigned int n,unsigned int p) const;
   float n_to_y(unsigned int n) const;
   float p_to_z(unsigned int p) const;
   threevector mnp_to_xyz(const voxel_coords& C) const;
   unsigned int mnp_to_key(unsigned int m,unsigned int n,unsigned int p) 
      const;
   unsigned int mnp_to_key(const voxel_coords& C) const;
   void key_to_mnp(unsigned int key,unsigned int& m,unsigned int& n,
                   unsigned int& p) const;
   unsigned int key_to_n(unsigned int key) const;
   unsigned int key_to_p(unsigned int key) const;
   voxel_coords key_to_mnp(unsigned int key) const;
   unsigned int xyz_to_key(const threevector& V) const;
   unsigned int xyzp_to_key(const fourvector& XYZP) const;

// Occupied/empty voxel determination member functions:

   void fill_lattice(std::vector<fourvector> const *xyzp_point_ptr);
   void increment_voxel_counts(const fourvector& XYZP,int delta_counts=1);
   void increment_voxel_counts(int key,int delta_counts=1);
   bool empty_voxel(const fourvector& XYZP);

// Ray tracing member functions:

   bool shadowed_voxel(const fourvector& XYZP);
   void identify_shadowed_voxels(
      const threevector& r_hat,Hashtable<Voxel_type>* hashtable_ptr);
   void relative_voxel_coords_for_line_segment(
      const threevector& start_point,const threevector& stop_point,
      std::vector<voxel_coords>& relative_voxel_coords);
   void voxels_along_line_segment(
      const threevector& start_point,const threevector& stop_point,
      std::vector<int>& keys);
   void order_voxels_by_range(
      const threevector& r_hat,Hashtable<Voxel_type>* hashtable_ptr);
   void mark_shadowed_voxels(
      std::vector<voxel_coords>& rel_voxel_coords,
      Hashtable<Voxel_type>* hashtable_ptr);

  private:

   unsigned int mdim,ndim,pdim;
   double shadow_sentinel_value;
   threevector delta,VOI_offset;
   fourvector max_value,min_value;
   Hashtable<Voxel_type>* voxels_hashtable_ptr;
   std::vector<std::pair<float,int> >* ordered_voxels_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const voxel_lattice& c);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void voxel_lattice::set_delta(const threevector& d)
{
   delta=d;
}

inline Hashtable<Voxel_type>* voxel_lattice::get_voxels_hashtable_ptr()
{
   return voxels_hashtable_ptr;
}

// ---------------------------------------------------------------------
inline bool voxel_lattice::xyz_inside_volume(const threevector& V) const
{
   return (V.get(0) > min_value.get(0) && V.get(0) < max_value.get(0) &&
           V.get(1) > min_value.get(1) && V.get(1) < max_value.get(1) && 
           V.get(2) > min_value.get(2) && V.get(2) < max_value.get(2));
}

inline bool voxel_lattice::mnp_inside_volume(
   unsigned int m,unsigned int n,unsigned int p) const
{
   return (m >= 0 && m < mdim && n >= 0 && n < ndim &&
           p >= 0 && p < pdim);
}

inline bool voxel_lattice::voxel_inside_volume(
   const voxel_coords& voxel) const
{
   return mnp_inside_volume(voxel.get(0),voxel.get(1),voxel.get(2));
}

// ---------------------------------------------------------------------
inline voxel_coords voxel_lattice::xyz_to_mnp(const threevector& V) const 
{
   unsigned int m,n,p;
   xyz_to_mnp(V,m,n,p);
   return voxel_coords(m,n,p);
}

// ---------------------------------------------------------------------
// Member function key_to_xyz takes in an integer key which labels a
// unique, discrete voxel.  If the key >= 0, it returns the voxel's x,
// y and z coordinates.

inline threevector voxel_lattice::key_to_xyz(unsigned int key) const
{
   unsigned int m,n,p;
   key_to_mnp(key,m,n,p);
   return mnp_to_xyz(m,n,p);
}

inline float voxel_lattice::key_to_y(unsigned int key) const
{
   return n_to_y(key_to_n(key));
}

inline float voxel_lattice::key_to_z(unsigned int key) const
{
   return p_to_z(key_to_p(key));
}

// ---------------------------------------------------------------------
// Member function mnp_to_xyz takes in discrete voxel coordinates
// (m,n,p) (which do not necessarily have to lie within the allowed
// voxel lattice volume) and returns the voxel's (x,y,z) coordinates.
// We subtract x,y,z member variable offsets to allow for image
// dependent (x,y,z) renormalizations.

inline threevector voxel_lattice::mnp_to_xyz(
   unsigned int m,unsigned int n,unsigned int p) const
{
   return threevector(min_value.get(0)+m*delta.get(0)-VOI_offset.get(0),
                      min_value.get(1)+n*delta.get(1)-VOI_offset.get(1),
                      min_value.get(2)+p*delta.get(2)-VOI_offset.get(2));
}

inline float voxel_lattice::n_to_y(unsigned int n) const
{
   return min_value.get(1)+n*delta.get(1)-VOI_offset.get(1);
}

inline float voxel_lattice::p_to_z(unsigned int p) const
{
   return min_value.get(2)+p*delta.get(2)-VOI_offset.get(2);
}

inline threevector voxel_lattice::mnp_to_xyz(const voxel_coords& C) const
{
   return mnp_to_xyz(C.get(0),C.get(1),C.get(2));
}

// ---------------------------------------------------------------------
inline unsigned int voxel_lattice::mnp_to_key(
   unsigned int m,unsigned int n,unsigned int p) const
{
   return mdim*ndim*p+mdim*n+m;
}

inline unsigned int voxel_lattice::mnp_to_key(const voxel_coords& C) const
{
   return mdim*ndim*C.get(2)+mdim*C.get(1)+C.get(0);
}

// ---------------------------------------------------------------------
inline void voxel_lattice::key_to_mnp(
   unsigned int key,unsigned int& m,unsigned int& n,unsigned int& p) const
{
   m=key%mdim;
   key=(key-m)/mdim;
   n=key%ndim;
   p=(key-n)/ndim;
}

inline unsigned int voxel_lattice::key_to_n(unsigned int key) const
{
   key=(key-key%mdim)/mdim;
   return key%ndim;
}

inline unsigned int voxel_lattice::key_to_p(unsigned int key) const
{
   key=(key-key%mdim)/mdim;
   return (key-key%ndim)/ndim;
}

inline voxel_coords voxel_lattice::key_to_mnp(unsigned int key) const
{
   unsigned int m=key%mdim;
   key=(key-m)/mdim;
   unsigned int n=key%ndim;
   unsigned int p=(key-n)/ndim;
   return voxel_coords(m,n,p);
}

// ---------------------------------------------------------------------
// Member function xyz_to_key takes in an (x,y,z) point which is
// assumed to reside within the allowed voxel_lattice
// volume.  It returns a unique integer which labels the voxel to
// which this point belongs.

inline unsigned int voxel_lattice::xyz_to_key(const threevector& V) const
{
   unsigned int m=basic_math::round((V.get(0)-min_value.get(0))/delta.get(0));
   unsigned int n=basic_math::round((V.get(1)-min_value.get(1))/delta.get(1));
   unsigned int p=basic_math::round((V.get(2)-min_value.get(2))/delta.get(2));
   return mnp_to_key(m,n,p);
}

inline unsigned int voxel_lattice::xyzp_to_key(const fourvector& XYZP) const
{
   return xyz_to_key(threevector(XYZP.get(0),XYZP.get(1),XYZP.get(2)));
}

#endif // voxel_lattice.h



