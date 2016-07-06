// ==========================================================================
// Header file for extremal_region class
// ==========================================================================
// Last modified on 10/8/12; 10/13/12; 10/16/12; 4/5/14
// ==========================================================================

#ifndef EXTREMAL_REGION_H
#define EXTREMAL_REGION_H

#include <map>
#include <set>
#include <vector>
#include "geometry/bounding_box.h"
#include "pool/objpool.h"
#include "geometry/polygon.h"
#include "dlib/svm.h"
#include "image/TwoDarray.h"

class extremal_region: public ObjectPool< extremal_region >
{

  public:

   typedef std::pair<int,int> INT_PAIR;
   
   typedef std::map<int,int> PIXEL_MAP;
// independent int var: pixel ID
// dependent int var: dummy label

   typedef std::map<int,double> PROB_MAP;
// independent int var: object ID
// dependent double var: probability

   typedef std::map<int,extremal_region*> ADJACENT_REGIONS_MAP;
// independent int var: adjacent extremal region ID
// dependent var: pointer to adjacent extremal region 

//   const int K_shapes=11;
   typedef dlib::matrix<double, 11, 1> SHAPES_SAMPLE_TYPE;

   typedef dlib::radial_basis_kernel<SHAPES_SAMPLE_TYPE> SHAPES_KERNEL_TYPE;
   typedef dlib::probabilistic_decision_function<SHAPES_KERNEL_TYPE> 
      SHAPES_PROBABILISTIC_FUNCT_TYPE;  
   typedef dlib::normalized_function<SHAPES_PROBABILISTIC_FUNCT_TYPE> 
      SHAPES_PFUNCT_TYPE;


   extremal_region();
   extremal_region(int id);
   extremal_region(const extremal_region& er);
   ~extremal_region();
   extremal_region& operator= (const extremal_region& er);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const extremal_region& er);

// Set and get methods:

   void set_bright_region_flag(bool flag);
   bool get_bright_region_flag() const;
   void set_visited_flag(bool flag);
   bool get_visited_flag() const;
   void set_ID(int id);
   int get_ID() const;

   void set_image_height(unsigned int h);

   void set_pixel_area(int a);
   int get_pixel_area() const;
   void set_pixel_perim(int p);
   int get_pixel_perim() const;
   int get_pixel_width() const;
   int get_pixel_height() const;

   int get_min_py() const;
   void set_bbox(unsigned int min_px,unsigned int min_py,
                 unsigned int max_px,unsigned int max_py);
   void get_bbox(unsigned int& min_px,unsigned int& min_py,
                 unsigned int& max_px,unsigned int& max_py);
   void set_Euler_number(int e);
   int get_Euler_number() const;

   void set_n_horiz_crossings(int c);

   double get_aspect_ratio() const;
   double get_compactness() const;
   int get_n_holes() const;
   int get_n_horiz_crossings() const;

   void set_entropy(double e);
   double get_entropy() const;
   void set_shapes_pfunct_ptr(SHAPES_PFUNCT_TYPE* s_ptr);

   void set_horiz_crossings(int r,int h_cross);
   int get_horiz_crossings(int r) const;
   void append_horiz_crossings(int r,int h_cross);

   void set_RLE_pixel_IDs(const std::vector<int>& RLE_pixel_IDs);
   std::vector<int>& get_RLE_pixel_IDs();
   const std::vector<int>& get_RLE_pixel_IDs() const;

   void set_bbox_polygon_ptr(polygon* poly_ptr);
   polygon* get_bbox_polygon_ptr();
   const polygon* get_bbox_polygon_ptr() const;

// Property manipulation member functions:

   int increment_pixel_area(int dA=1);
   void update_pixel_perim(int n_prev_neighbors,int n_new_neighbors);
   void update_bbox(unsigned int px,unsigned int py);
   void update_bbox(unsigned int min_px,unsigned int min_py,
                    unsigned int max_px,unsigned int max_py);
   void update_bbox(extremal_region* neighbor_extremal_region_ptr);

   void increment_Euler_number(int d_Euler);

// Moments member functions:   

   void update_XY_moments(unsigned int px,unsigned int py);
   void update_Z_moments(double z);

   void set_px_sum(double s);
   void set_py_sum(double s);
   double get_px_sum() const;
   double get_py_sum() const;
   double get_mean_px() const;
   double get_mean_py() const;
   void set_z_sum(double s);
   double get_z_sum() const;
   double get_mean_z() const;

   void set_sqr_px_sum(double s);
   void set_sqr_py_sum(double s);
   void set_px_py_sum(double s);
   double get_sqr_px_sum() const;
   double get_sqr_py_sum() const;
   double get_px_py_sum() const;
   void set_sqr_z_sum(double s);
   double get_sqr_z_sum() const;

   void set_cube_px_sum(double s);
   void set_sqr_px_py_sum(double s);
   void set_sqr_py_px_sum(double s);
   void set_cube_py_sum(double s);
   void set_cube_z_sum(double s);
   double get_cube_px_sum() const;
   double get_sqr_px_py_sum() const;
   double get_sqr_py_px_sum() const;
   double get_cube_py_sum() const;
   double get_cube_z_sum() const;

   void set_quartic_z_sum(double s);
   double get_quartic_z_sum() const;

   double get_mean_sqr_px() const;
   double get_mean_sqr_py() const;
   double get_mean_px_py() const;
   double get_mean_sqr_z() const;

   double get_sigma_px() const;
   double get_sigma_py() const;
   double get_dimensionless_px_py_covar() const;
   void compute_covariance_matrix(twoDarray* cc_twoDarray_ptr);
   double get_sigma_z() const;

   double get_mean_cube_px() const;
   double get_mean_sqr_px_py() const;
   double get_mean_sqr_py_px() const;
   double get_mean_cube_py() const;
   double get_mean_cube_z() const;

   double get_mean_quartic_z() const;

   double get_skew_px() const;
   double get_dimensionless_sqr_px_py() const;
   double get_dimensionless_sqr_py_px() const;
   double get_skew_py() const;
   double get_skew_z() const;

   double get_dimensionless_quartic_z() const;

// Pixel member functions:

   int get_n_pixel_IDs() const;   
   void insert_pixel(int pixel_ID);
   void insert_pixels(const std::vector<int>& input_pixel_IDs);
   void print_pixel_IDs(int image_width=-1);
   int reset_pixel_iterator();
   int get_next_pixel_ID();

// Probability member functions:

   void set_object_prob(double p);
   double get_object_prob();
   void set_object_prob(int i,double p);
   double get_object_prob(int i);

// Text character detection member functions:

   bool region_too_small_or_too_big(
      double max_reasonable_pixel_width,double max_reasonable_pixel_height);
   bool compute_shape_text_prob(
      SHAPES_PFUNCT_TYPE* shapes_ptr,double shapes_prob_threshold,
      int object_ID,bool print_flag=false);

// Run-length encoding member functions:

   void run_length_encode(const twoDarray* cc_twoDarray_ptr);
   void run_length_decode(twoDarray* cc_twoDarray_ptr,double output_value=-1);
   std::vector<std::pair<int,int> > run_length_decode(int xdim);
   void compute_bbox_from_RLE_pixels(unsigned int xdim);
   std::vector<std::pair<int,int> >& compute_border_pixels(
      int border_thickness,twoDarray* cc_twoDarray_ptr);

// Adjacent extremal region membere functions:

   void add_adjacent_region(extremal_region* extremal_region_ptr);
   void delete_adjacent_region(int region_ID);
   

  private: 

   bool bright_region_flag,visited_flag;
   int ID;
   int pixel_area,pixel_perim;
   unsigned int min_px,min_py,max_px,max_py;
   polygon* bbox_polygon_ptr;	// Just pointer and not dynamic object

   int min_reasonable_pixel_width,min_reasonable_pixel_height;
   int Euler_number,n_horiz_crossings;
   double entropy;
   
   double px_sum,sqr_px_sum;
   double py_sum,sqr_py_sum;
   double px_py_sum;
   double cube_px_sum,sqr_px_py_sum,sqr_py_px_sum,cube_py_sum;

   double z_sum,sqr_z_sum,cube_z_sum,quartic_z_sum;

   PIXEL_MAP* pixel_map_ptr;
   PIXEL_MAP::iterator pixel_map_iter;

   PROB_MAP* prob_map_ptr;
   PROB_MAP::iterator prob_map_iter;

   SHAPES_SAMPLE_TYPE shapes_sample;

   unsigned int image_height;
   std::vector<int> horiz_crossings;

   ADJACENT_REGIONS_MAP adjacent_regions_map;
   ADJACENT_REGIONS_MAP::iterator adjacent_regions_iter;

   std::vector<int> RLE_pixel_IDs;
   std::vector<std::pair<int,int> > border_pixels;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const extremal_region& er);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void extremal_region::set_bright_region_flag(bool flag)
{
   bright_region_flag=flag;
}

inline bool extremal_region::get_bright_region_flag() const
{
   return bright_region_flag;
}

inline void extremal_region::set_visited_flag(bool flag)
{
   visited_flag=flag;
}

inline bool extremal_region::get_visited_flag() const
{
   return visited_flag;
}

inline void extremal_region::set_ID(int id)
{
   ID=id;
}

inline int extremal_region::get_ID() const
{
   return ID;
}

inline void extremal_region::set_pixel_area(int a)
{
   pixel_area=a;
}

inline int extremal_region::get_pixel_area() const
{
   return pixel_area;
}

inline void extremal_region::set_pixel_perim(int p)
{
   pixel_perim=p;
}

inline int extremal_region::get_pixel_perim() const
{
   return pixel_perim;
}

inline int extremal_region::get_pixel_width() const
{
   return max_px-min_px+1;
}

inline int extremal_region::get_pixel_height() const
{
   return max_py-min_py+1;
}

inline void extremal_region::set_Euler_number(int e)
{
   Euler_number=e;
}

inline int extremal_region::get_Euler_number() const
{
   return Euler_number;
}

inline void extremal_region::set_n_horiz_crossings(int c)
{
   n_horiz_crossings=c;
}

inline int extremal_region::get_n_horiz_crossings() const
{
   return n_horiz_crossings;
}

inline void extremal_region::set_entropy(double e)
{
   entropy=e;
}

inline double extremal_region::get_entropy() const
{
   return entropy;
}

inline int extremal_region::increment_pixel_area(int dA)
{
   pixel_area += dA;
   return pixel_area;
}

inline void extremal_region::update_pixel_perim(
   int n_prev_neighbors,int n_new_neighbors)
{
//   cout << "inside extremal_region::update_pixel_perim()" << endl;
//   cout << "Old pixel perim = " << pixel_perim << endl;
   pixel_perim += 4-2*n_prev_neighbors-1*n_new_neighbors;
//   cout << "Updated pixel_perim = " << pixel_perim << endl;
}

inline void extremal_region::increment_Euler_number(int d_Euler)
{
   Euler_number += d_Euler;
}


inline void extremal_region::set_px_sum(double s)
{
   px_sum=s;
}

inline void extremal_region::set_py_sum(double s)
{
   py_sum=s;
}

inline double extremal_region::get_px_sum() const
{
   return px_sum;
}

inline double extremal_region::get_py_sum() const
{
   return py_sum;
}

inline double extremal_region::get_mean_px() const
{
   return px_sum/pixel_area;
}

inline double extremal_region::get_mean_py() const
{
   return py_sum/pixel_area;
}


inline void extremal_region::set_z_sum(double s)
{
   z_sum=s;
}

inline double extremal_region::get_z_sum() const
{
   return z_sum;
}

inline double extremal_region::get_mean_z() const
{
   return z_sum/pixel_area;
}



inline void extremal_region::set_sqr_px_sum(double s)
{
   sqr_px_sum=s;
}

inline void extremal_region::set_sqr_py_sum(double s)
{
   sqr_py_sum=s;
}

inline void extremal_region::set_px_py_sum(double s)
{
   px_py_sum=s;
}

inline double extremal_region::get_sqr_px_sum() const
{
   return sqr_px_sum;
}

inline double extremal_region::get_sqr_py_sum() const
{
   return sqr_py_sum;
}

inline double extremal_region::get_px_py_sum() const
{
   return px_py_sum;
}

inline void extremal_region::set_sqr_z_sum(double s)
{
   sqr_z_sum=s;
}

inline double extremal_region::get_sqr_z_sum() const
{
   return sqr_z_sum;
}


inline void extremal_region::set_cube_px_sum(double s)
{
   cube_px_sum=s;
}

inline void extremal_region::set_sqr_px_py_sum(double s)
{
   sqr_px_py_sum=s;
}

inline void extremal_region::set_sqr_py_px_sum(double s)
{
   sqr_py_px_sum=s;
}

inline void extremal_region::set_cube_py_sum(double s)
{
   cube_py_sum=s;
}

inline void extremal_region::set_cube_z_sum(double s)
{
   cube_z_sum=s;
}


inline double extremal_region::get_cube_px_sum() const
{
   return cube_px_sum;
}

inline double extremal_region::get_sqr_px_py_sum() const
{
   return sqr_px_py_sum;
}

inline double extremal_region::get_sqr_py_px_sum() const
{
   return sqr_py_px_sum;
}

inline double extremal_region::get_cube_py_sum() const
{
   return cube_py_sum;
}

inline double extremal_region::get_cube_z_sum() const
{
   return cube_z_sum;
}


inline void extremal_region::set_quartic_z_sum(double s)
{
   quartic_z_sum=s;
}

inline double extremal_region::get_quartic_z_sum() const
{
   return quartic_z_sum;
}


inline double extremal_region::get_mean_sqr_px() const
{
   return sqr_px_sum/pixel_area;
}

inline double extremal_region::get_mean_sqr_py() const
{
   return sqr_py_sum/pixel_area;
}

inline double extremal_region::get_mean_px_py() const
{
   return px_py_sum/pixel_area;
}

inline double extremal_region::get_mean_sqr_z() const
{
   return sqr_z_sum/pixel_area;
}


inline double extremal_region::get_mean_cube_px() const
{
   return cube_px_sum/pixel_area;
}

inline double extremal_region::get_mean_sqr_px_py() const
{
   return sqr_px_py_sum/pixel_area;
}

inline double extremal_region::get_mean_sqr_py_px() const
{
   return sqr_py_px_sum/pixel_area;
}

inline double extremal_region::get_mean_cube_py() const
{
   return cube_py_sum/pixel_area;
}

inline double extremal_region::get_mean_cube_z() const
{
   return cube_z_sum/pixel_area;
}


inline double extremal_region::get_mean_quartic_z() const
{
   return quartic_z_sum/pixel_area;
}


inline void extremal_region::set_horiz_crossings(int r,int h_cross) 
{
   horiz_crossings[r]=h_cross;
}

inline int extremal_region::get_horiz_crossings(int r) const
{
   return horiz_crossings[r];
}

inline void extremal_region::append_horiz_crossings(int r,int h_cross) 
{
   horiz_crossings[r] += h_cross;
}

inline int extremal_region::get_min_py() const
{
   return min_py;
}

inline std::vector<int>& extremal_region::get_RLE_pixel_IDs()
{
   return RLE_pixel_IDs;
}

inline const std::vector<int>& extremal_region::get_RLE_pixel_IDs() const
{
   return RLE_pixel_IDs;
}

inline void extremal_region::set_bbox_polygon_ptr(polygon* poly_ptr)
{
   bbox_polygon_ptr=poly_ptr;
}

inline polygon* extremal_region::get_bbox_polygon_ptr()
{
   return bbox_polygon_ptr;
}

inline const polygon* extremal_region::get_bbox_polygon_ptr() const
{
   return bbox_polygon_ptr;
}

#endif  // extremal_region.h
