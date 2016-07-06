// ==========================================================================
// VOXEL_LATTICE base class member function definitions
// ==========================================================================
// Last modified on 2/23/06; 12/10/06; 12/4/10; 4/5/14
// ==========================================================================

#include <algorithm>
#include <iostream>
#include "threeDgraphics/voxel_lattice.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void voxel_lattice::allocate_member_objects()
{
}		       

void voxel_lattice::initialize_member_objects()
{
   delta=threevector(0.3,0.3,0.3);	// meter (appropriate for ALIRT)
   VOI_offset=threevector(0,0,0);
   voxels_hashtable_ptr=NULL;
   ordered_voxels_ptr=NULL;
   shadow_sentinel_value=-1.0;
}		       

voxel_lattice::voxel_lattice()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// Copy constructor:

voxel_lattice::voxel_lattice(const voxel_lattice& v)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(v);
}

voxel_lattice::~voxel_lattice()
{
   delete voxels_hashtable_ptr;
   delete ordered_voxels_ptr;
}

// ---------------------------------------------------------------------
void voxel_lattice::docopy(const voxel_lattice& v)
{
}

// ---------------------------------------------------------------------
// Overload = operator:

voxel_lattice& voxel_lattice::operator= (const voxel_lattice& v)
{
   if (this==&v) return *this;
   docopy(v);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const voxel_lattice& v)
{
   outstream << endl;
//   templatefunc::printVector(c.segment);
   return outstream;
}

// ==========================================================================
// Lattice initialization member functions
// ==========================================================================

void voxel_lattice::initialize(
   int n_expected_points,double min_x,double max_x,double min_y,double max_y,
   double min_z,double max_z)
{
   voxels_hashtable_ptr=new Hashtable<Voxel_type>(n_expected_points);

   const double min_prob=0;
   const double max_prob=0;

   max_value.put(0,max_x);
   max_value.put(1,max_y);
   max_value.put(2,max_z);
   max_value.put(3,max_prob);
   
   cout << "max_value = " << max_value << endl;

   min_value.put(0,min_x);
   min_value.put(1,min_y);
   min_value.put(2,min_z);
   min_value.put(3,min_prob);

   cout << " min_value = " << min_value << endl;

   reset_coordinate_system();
}

void voxel_lattice::initialize(vector<fourvector> const *xyzp_point_ptr)
{
   int n_expected_points=10*xyzp_point_ptr->size();
   voxels_hashtable_ptr=new Hashtable<Voxel_type>(n_expected_points);
   ordered_voxels_ptr=new vector<pair<float,int> >(n_expected_points);

   compute_extremal_values(xyzp_point_ptr);
   reset_coordinate_system();
}

// ---------------------------------------------------------------------
// Member function compute_extremal_values loops over every XYZP point
// within the input STL vector and stores the min/max XYZP values
// within member fourvectors max_value and min_value.

void voxel_lattice::compute_extremal_values(
   vector<fourvector> const *xyzp_point_ptr)
{
   max_value=fourvector(NEGATIVEINFINITY,NEGATIVEINFINITY,
                        NEGATIVEINFINITY,NEGATIVEINFINITY);
   min_value=fourvector(POSITIVEINFINITY,POSITIVEINFINITY,
                        POSITIVEINFINITY,POSITIVEINFINITY);

   for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
   {
      fourvector curr_pnt=(*xyzp_point_ptr)[i];
      max_value.put(0,basic_math::max(max_value.get(0),curr_pnt.get(0)));
      max_value.put(1,basic_math::max(max_value.get(1),curr_pnt.get(1)));
      max_value.put(2,basic_math::max(max_value.get(2),curr_pnt.get(2)));
      max_value.put(3,basic_math::max(max_value.get(3),curr_pnt.get(3)));

      min_value.put(0,basic_math::min(min_value.get(0),curr_pnt.get(0)));
      min_value.put(1,basic_math::min(min_value.get(1),curr_pnt.get(1)));
      min_value.put(2,basic_math::min(min_value.get(2),curr_pnt.get(2)));
      min_value.put(3,basic_math::min(min_value.get(3),curr_pnt.get(3)));
   }
}

// ---------------------------------------------------------------------
void voxel_lattice::reset_coordinate_system()
{
   mdim=basic_math::round((max_value.get(0)-min_value.get(0))/delta.get(0))+1;
   ndim=basic_math::round((max_value.get(1)-min_value.get(0))/delta.get(1))+1;
   pdim=basic_math::round((max_value.get(2)-min_value.get(0))/delta.get(2))+1;

   max_value.put(0,min_value.get(0)+(mdim-1)*delta.get(0));
   max_value.put(1,min_value.get(1)+(ndim-1)*delta.get(1));
   max_value.put(2,min_value.get(2)+(pdim-1)*delta.get(2));
   
   cout << "mdim = " << mdim << " ndim = " << ndim << " pdim = " << pdim
        << endl;
   cout << "xhi = " << max_value.get(0) 
        << " xlo = " << min_value.get(0) << endl;
   cout << "yhi = " << max_value.get(1) 
        << " ylo = " << min_value.get(1) << endl;
   cout << "zhi = " << max_value.get(2) 
        << " zlo = " << min_value.get(2) << endl;
   cout << "dx = " << delta.get(0) 
        << " dy = " << delta.get(1) 
        << " dz = " << delta.get(2) << endl;
}

// ==========================================================================
// Discrete <--> continous coordinate conversion member functions:
// ==========================================================================

// Member function xyz_to_mnp returns the pixel coordinates of the
// voxel closest to the input (x,y,z) point.

void voxel_lattice::xyz_to_mnp(
   const threevector& V,unsigned int& m,unsigned int& n,unsigned int& p) const
{
   m=basic_math::round((V.get(0)+VOI_offset.get(0)-min_value.get(0))/
                       delta.get(0));
   n=basic_math::round((V.get(1)+VOI_offset.get(1)-min_value.get(1))/
                       delta.get(1));
   p=basic_math::round((V.get(2)+VOI_offset.get(2)-min_value.get(2))/
                       delta.get(2));

   if (m < 0) m=0;
   m=basic_math::min(m,mdim-1);
   if (n < 0) n=0;
   n=basic_math::min(n,ndim-1);
   if (p < 0) p=0;
   p=basic_math::min(p,pdim-1);
}

// ==========================================================================
// Occupied/empty voxel determination member functions:
// ==========================================================================

// Member function fill_lattice loops over all XYZP points within
// input STL vector *xyzp_point_ptr and converts their continuous
// spatial locations into discrete hashtable form.

void voxel_lattice::fill_lattice(vector<fourvector> const *xyzp_point_ptr)
{
   for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
   {
      increment_voxel_counts( (*xyzp_point_ptr)[i] );
   } 
}

// ---------------------------------------------------------------------
// Member function increment_voxel_counts increases the counts
// corresponding to the voxel specified by input key within
// *voxels_hashtable_ptr by delta_counts.

void voxel_lattice::increment_voxel_counts(
   const fourvector& XYZP,int delta_counts)
{
   increment_voxel_counts(xyzp_to_key(XYZP),delta_counts);
}

void voxel_lattice::increment_voxel_counts(int key,int delta_counts)
{
   int location;
   Mynode<Voxel_type>* currnode_ptr=
      voxels_hashtable_ptr->retrieve_key(key,location);
   if (currnode_ptr==NULL)
   {
      voxels_hashtable_ptr->insert_key(
         key,location,Voxel_type(delta_counts,0.0));
   }
   else
   {
      currnode_ptr->set_data(Voxel_type(
         currnode_ptr->get_data().first+delta_counts,
         currnode_ptr->get_data().second));
   }
}

// ---------------------------------------------------------------------
bool voxel_lattice::empty_voxel(const fourvector& XYZP)
{
   int location;
   int key=xyzp_to_key(XYZP);
   return (voxels_hashtable_ptr->retrieve_key(key,location) == NULL);
}

// ==========================================================================
// Ray tracing member functions
// ==========================================================================

bool voxel_lattice::shadowed_voxel(const fourvector& XYZP)
{
   bool voxel_in_shadow=false;
   int location;
   int key=xyzp_to_key(XYZP);

   Mynode<Voxel_type>* currnode_ptr=
      voxels_hashtable_ptr->retrieve_key(key,location);
   if (currnode_ptr != NULL)
   {
      voxel_in_shadow=nearly_equal(
         currnode_ptr->get_data().second,shadow_sentinel_value);
   }
   return voxel_in_shadow;
}

// ---------------------------------------------------------------------
// Member function identify_shadowed_voxels is a high-level method
// which takes in the range direction vector r_hat.  It transfers
// counts from voxels in the hashtable which have too many neighbors
// with nonzero counts located upstream in the -r_hat direction.

void voxel_lattice::identify_shadowed_voxels(
   const threevector& r_hat,Hashtable<Voxel_type>* hashtable_ptr)
{

// First compute maximum possible upstream distance.  Then compute
// relative voxel coordinates for this extreme case:

   threevector bbox_delta(max_value.get(0)-min_value.get(0),
                          max_value.get(1)-min_value.get(1),
                          max_value.get(2)-min_value.get(2));
   const double max_upstream_distance=1.1*bbox_delta.magnitude();    
   threevector start_point(-max_upstream_distance*r_hat);
   threevector stop_point(0,0,0);

   const int max_n_relcoords=max_upstream_distance/
      basic_math::min(delta.get(0),delta.get(1),delta.get(2));
   vector<voxel_coords> relative_voxel_coords;
   relative_voxel_coords.reserve(max_n_relcoords);

   relative_voxel_coords_for_line_segment(
      start_point,stop_point,relative_voxel_coords);

   order_voxels_by_range(r_hat,hashtable_ptr);
   mark_shadowed_voxels(relative_voxel_coords,hashtable_ptr);
}

// ---------------------------------------------------------------------
// Member function relative_voxel_coords_for_line_segment takes in the
// starting and stopping points for some line segment.  This method
// returns within STL vector relative_voxel_coords the voxel
// coordinates for all voxels which lie as close as possible along the
// line segment.

void voxel_lattice::relative_voxel_coords_for_line_segment(
   const threevector& start_point,const threevector& stop_point,
   vector<voxel_coords>& relative_voxel_coords)
{
   unsigned int m_stop,n_stop,p_stop;
   xyz_to_mnp(stop_point,m_stop,n_stop,p_stop);
   vector<int> keys;
   keys.reserve(relative_voxel_coords.size());
   voxels_along_line_segment(start_point,stop_point,keys);

   unsigned int curr_m,curr_n,curr_p;
   for (unsigned int i=0; i<keys.size(); i++)
   {
      key_to_mnp(keys[i],curr_m,curr_n,curr_p);
      relative_voxel_coords.push_back(
         voxel_coords(curr_m-m_stop,curr_n-n_stop,curr_p-p_stop));
   } // loop over nodes in *keylist_ptr
}

// ---------------------------------------------------------------------
// Member function voxels_along_line_segment returns within STL vector
// keys the integer keys for all voxels located along the line segment
// defined by the starting and stop coordinates passed as inputs.
// This method uses a 3D generalization of the midpoint line algorithm
// described in section 3.2.2. of "Computer graphics: principles and
// practice", 2nd edition by Fley, van Dam, Feiner and Hughes.

void voxel_lattice::voxels_along_line_segment(
   const threevector& start_point,const threevector& stop_point,
   vector<int>& keys)
{

// Clear STL vector that will hold keys for voxels encountered along
// line segment:

   keys.clear();

// Next initialize starting voxel:

   unsigned int m_start,m_stop,n_start,n_stop,p_start,p_stop;
   xyz_to_mnp(start_point,m_start,n_start,p_start);
   xyz_to_mnp(stop_point,m_stop,n_stop,p_stop);
   unsigned int m=m_start;
   unsigned int n=n_start;
   unsigned int p=p_start;
   keys.push_back(mnp_to_key(m,n,p));

// Sequentially progress from first to last voxel:

   int dx=m_stop-m_start;
   int dy=n_stop-n_start;
   int dz=p_stop-p_start;
   int abs_dx=abs(dx);
   int abs_dy=abs(dy);
   int abs_dz=abs(dz);
   int sgn_dx=sgn(dx);
   int sgn_dy=sgn(dy);
   int sgn_dz=sgn(dz);

   if (abs_dx > abs_dy && abs_dx > abs_dz) // x step dominates
   {
      int dxy=2*abs_dy-abs_dx;
      int increE=2*abs_dy;
      int increNE=2*(abs_dy-abs_dx);

      int dxz=abs_dx-2*abs_dz;
      int increN=-2*abs_dz;
      int increEN=2*(abs_dx-abs_dz);
      
      while (sgn_dx*m < sgn_dx*m_stop)
      {
         m += sgn_dx;
         if (dxy <= 0)
         {
            dxy += increE;	// midpoint below line
         }
         else
         {
            dxy += increNE;	// midpoint above line
            n += sgn_dy;
         }

         if (dxz >= 0)
         {
            dxz += increN;
         }
         else
         {
            dxz += increEN;
            p += sgn_dz;
         }

         keys.push_back(mnp_to_key(m,n,p));
      } // while loop over m index 
   }
   else if (abs_dy > abs_dx && abs_dy > abs_dz) // y step dominates
   {
      int dxy=abs_dy-2*abs_dx;
      int increN=-2*abs_dx;
      int increNE=2*(abs_dy-abs_dx);

      int dyz=2*abs_dz-abs_dy;
      int increE=2*abs_dz;
      int increEN=2*(abs_dz-abs_dy);

      while (sgn_dy*n < sgn_dy*n_stop)
      {
         n += sgn_dy;
         if (dxy >= 0)
         {
            dxy += increN;
         }
         else
         {
            dxy += increNE;
            m += sgn_dx;
         }

         if (dyz <= 0)
         {
            dyz += increE;	// midpoint below line
         }
         else
         {
            dyz += increEN;	// midpoint above line
            p += sgn_dz;
         }

         keys.push_back(mnp_to_key(m,n,p));
      } // while loop over n index
   }
   else	 // z step dominates
   {
      int dzx=2*abs_dx-abs_dz;
      int increE=2*abs_dx;
      int increNE=2*(abs_dx-abs_dz);

      int dzy=abs_dz-2*abs_dy;
      int increN=-2*abs_dy;
      int increEN=2*(abs_dz-abs_dy);
      
      while (sgn_dz*p < sgn_dz*p_stop)
      {
         p += sgn_dz;
         if (dzx <= 0)
         {
            dzx += increE;	// midpoint below line
         }
         else
         {
            dzx += increNE;	// midpoint above line
            m += sgn_dx;
         }

         if (dzy >= 0)
         {
            dzy += increN;
         }
         else
         {
            dzy += increEN;
            n += sgn_dy;
         }

         keys.push_back(mnp_to_key(m,n,p));

      } // while loop over n index
   } // largest step in x, y or z direction conditional
}

// ---------------------------------------------------------------------
// Member function order_voxels_by_range takes in a range direction
// vector r_hat.  Looping over all entries within hashtable
// *voxels_hashtable_ptr, this method fills member STL vector
// *ordered_voxels_ptr with <range,key> pairs.  It subquently reverse
// sorts the STL vector in range.  (Voxels with maximum range appear
// in the first position of the STL vector.)

void voxel_lattice::order_voxels_by_range(
   const threevector& r_hat,Hashtable<Voxel_type>* hashtable_ptr)
{
   pair<float,int> p;
   ordered_voxels_ptr->clear();

   for (unsigned int n=0; n<hashtable_ptr->get_table_capacity(); n++)
   {
      Linkedlist<Voxel_type>* currlist_ptr=hashtable_ptr->get_list_ptr(n);
      if (currlist_ptr->size() > 0)
      {
         for (Mynode<Voxel_type>* currnode_ptr=currlist_ptr->get_start_ptr();
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            p.second=currnode_ptr->get_ID();
            p.first=key_to_xyz(p.second).dot(r_hat);
            ordered_voxels_ptr->push_back(p);
         } // loop over voxel nodes in *currlist_ptr
      }
   } // loop over index n labeling hashtable's linked lists
   std::sort(ordered_voxels_ptr->rbegin(),ordered_voxels_ptr->rend());
}

// ---------------------------------------------------------------------
// Member function mark_shadowed_voxels scans over every entry within
// the input hashtable *hashtable_ptr.  For a given voxel in the
// hashtable, this method searches over voxels located upstream by the
// voxel coordinate displacements within the nodes of the relative
// coordinates linked list.  The related voxels are oriented in the
// negative range direction with respect to the starting voxel.  This
// member function counts the number of upstream illuminated voxels
// containing positive numbers of counts.  If the number exceeds
// jigparam::visible_surface_depth (measured in numbers of voxels),
// the starting voxel's counts are added (with some amplification
// factor) to the threshold upstream voxel's.  The starting voxel is
// subsequently deleted from *hashtable_ptr as well as the hot
// hashtable.

void voxel_lattice::mark_shadowed_voxels(
   vector<voxel_coords>& rel_voxel_coords,
   Hashtable<Voxel_type>* hashtable_ptr)
{
   for (unsigned int n=0; n<ordered_voxels_ptr->size(); n++)
   {
      int curr_key=(*ordered_voxels_ptr)[n].second;
      Mynode<Voxel_type>* currnode_ptr=hashtable_ptr->retrieve_key(curr_key);
      voxel_coords curr_voxel=key_to_mnp(curr_key);

      for (unsigned int i=0; i<rel_voxel_coords.size(); i++)
      {
         voxel_coords rel_voxel=curr_voxel+rel_voxel_coords[i];
         if (!voxel_inside_volume(rel_voxel)) break;
         
         int rel_key=mnp_to_key(rel_voxel);
         Mynode<Voxel_type>* relnode_ptr=hashtable_ptr->retrieve_key(rel_key);
         if (relnode_ptr != NULL)
         {
            currnode_ptr->get_data().second=shadow_sentinel_value;
            break;
         } // relnode_ptr != NULL conditional
      } // loop over nodes in *rel_coords_list_ptr 
   } // loop over index n labeling ordered voxels
}
