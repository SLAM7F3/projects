// ==========================================================================
// Header file for text_detector class
// ==========================================================================
// Last modified on 6/3/14; 6/22/14; 6/24/14; 6/26/14
// ==========================================================================

#ifndef TEXT_DETECTOR_H
#define TEXT_DETECTOR_H

#include <map>
#include <set>
#include <string>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>
#include "math/genmatrix.h"
#include "math/ltduple.h"
#include "video/texture_rectangle.h"


class text_detector
{

  public:

   typedef std::pair<int,int> DUPLE;

   text_detector(std::string dictionary_subdir,bool RGB_pixels_flag);
   text_detector(const text_detector& t);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~text_detector();
   text_detector& operator= (const text_detector& t);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const text_detector& t);

// Set and get member functions:

   int get_D() const;
   int get_K() const;
   void set_window_width(int w);
   void set_window_height(int h);

   float get_Dictionary_value(unsigned int d,unsigned int k);
   float get_Dtrans_inverse_sqrt_covar(unsigned int k,unsigned int d);

   void set_texture_rectangle_ptr(texture_rectangle* tr_ptr);
   texture_rectangle* get_texture_rectangle_ptr();
   const texture_rectangle* get_texture_rectangle_ptr() const;

   int get_pixel_ID(int px,int py) const;
   void get_pixel_px_py(int ID,int& px,int& py) const;
   int get_avg_pixel_ID(int px,int py) const;
   void get_avg_pixel_px_py(int ID,int& px,int& py) const;

// Dictionary importing and computation member functions:

   void import_dictionary();
   void import_inverse_sqrt_covar_matrix();
   void compute_Dtrans_inverse_sqrt_covar_matrix();
   float* whiten_patch(float* patch_descriptor);

// Feature patch member functions:

   bool import_image_from_file(std::string image_filename);
   float compute_recog_prob(unsigned int px,unsigned int py,
                            const std::vector<float>& weights);
   bool average_window_features(unsigned int px,unsigned int py);
//   bool compute_window_features(unsigned int px,unsigned int py);
   std::vector<threevector> compute_sector_start_stop(int w) const;
   float* get_nineK_window_descriptor();
   


  protected:

  private: 

   bool RGB_pixels_flag;
   unsigned int D,sqrt_D,K;
   unsigned int window_width,window_height;
   unsigned int avg_window_width,avg_window_height;
   unsigned int nx_cells, ny_cells;
   std::string dictionary_subdir;
   flann::Matrix<float> *mean_coeffs_ptr,*inverse_sqrt_covar_ptr;
   flann::Matrix<float> *Dtrans_inverse_sqrt_covar_ptr;

   float *patch_descriptor,*whitened_descriptor;
   float* window_histogram;
   flann::Matrix<float> Dictionary;
   texture_rectangle* texture_rectangle_ptr;

     // *avg_window_features_vector indep var (0,0) ; (0,1) ; ... ; (2,2)
     // dependent var = K-dimensional descriptor vector

   std::vector<float*> avg_window_features_vector;
   float* curr_feature_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const text_detector& p);

// Average window features maps member functions:

   void initialize_avg_window_features_vector();
   void clear_avg_window_features_vector();
   void destroy_avg_window_features_vector();

   void prepare_patch(unsigned int py,unsigned int pv);
   void fill_RGB_patch_descriptor(unsigned int px,unsigned int py);
   void fill_greyscale_patch_descriptor(unsigned int px,unsigned int py);
   void contrast_normalize_patch_descriptor();
   void encode_patch_values_into_feature();
   float encode_patch_values_into_feature(unsigned int k);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int text_detector::get_D() const
{
   return D;
}

inline int text_detector::get_K() const
{
   return K;
}

inline void text_detector::set_window_width(int w)
{
   window_width=w;
}

inline void text_detector::set_window_height(int h)
{
   window_height=h;
}

inline float text_detector::get_Dictionary_value(unsigned int d,unsigned int k)
{
  return Dictionary[k][d];
}

inline float text_detector::get_Dtrans_inverse_sqrt_covar(unsigned int k,unsigned int d)
{
  return (*Dtrans_inverse_sqrt_covar_ptr)[k][d];
}

inline void text_detector::set_texture_rectangle_ptr(
   texture_rectangle* tr_ptr)
{
   texture_rectangle_ptr=tr_ptr;
}

inline texture_rectangle* text_detector::get_texture_rectangle_ptr()
{
   return texture_rectangle_ptr;
}

inline const texture_rectangle* text_detector::get_texture_rectangle_ptr() const
{
   return texture_rectangle_ptr;
}

inline int text_detector::get_pixel_ID(int px,int py) const
{
   int ID=py*window_width+px;
   return ID;
}

inline void text_detector::get_pixel_px_py(int ID,int& px,int& py) const
{
   py=ID/window_width;
   px=ID%window_width;
}

inline int text_detector::get_avg_pixel_ID(int px,int py) const
{
   int ID=py*avg_window_width+px;
   return ID;
}

inline void text_detector::get_avg_pixel_px_py(int ID,int& px,int& py) const
{
   py=ID/avg_window_width;
   px=ID%avg_window_width;
}


#endif  // text_detector.h



