// ==========================================================================
// Header file for MYIMAGE base class
// ==========================================================================
// Last modified on 4/26/06; 8/5/06; 8/22/06; 3/28/07; 4/5/14
// ==========================================================================

#ifndef MYIMAGE_H
#define MYIMAGE_H

#include <fftw.h>
#include <string>
#include "general/filefuncs.h"
#include "general/sysfuncs.h"
class polygon;
class threevector;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

class myimage
{

  public:

// Initialization, constructor and destructor functions:

   myimage(void);
   myimage(const myimage& m);
   myimage(int nxbins,int nybins);
   myimage(const twoDarray& T);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~myimage();
   myimage& operator= (const myimage& m);

// Set & get member functions:
 
   void set_imagenumber(int i);
   void set_classified(bool c);
   void set_adjust_xscale(bool a);
   void set_black_corresponds_to_minimumz(bool bctmz);
   void set_xtic(double x);
   void set_ytic(double y);
   void set_imagedir(std::string subdir);
   void set_colortable_filename(std::string filename);
   void set_title(std::string t);
   void set_xaxis_label(std::string label);
   void set_yaxis_label(std::string label);
   void set_z2Darray_orig_ptr(twoDarray* ztwoDarray_ptr);
   void set_z2Darray_ptr(twoDarray* ztwoDarray_ptr);
   void set_ztwoDarray_tmp_ptr(twoDarray* ztwoDarray_ptr);

   bool get_black_corresponds_to_minimumz() const;
   int get_imagenumber() const;
   void set_min_z(double zmin);
   void set_max_z(double zmax);
   double get_min_z() const;
   double get_max_z() const;
   std::string get_imagedir();
   twoDarray* get_z2Darray_orig_ptr();
   twoDarray* get_z2Darray_ptr();
   twoDarray* get_ztwoDarray_tmp_ptr();

   void readin_png_image(std::string png_filename);
   void renormalize_raw_image_intensities(
      double renorm_zmin,double renorm_zmax);

// FFT member functions:
                                                
/*
   void init_fftw();
   void fouriertransform(
      const complex value_in[Nx_max][Ny_max],
      complex tilde_out[Nx_max][Ny_max]);
   void inversefouriertransform(
      const complex tilde_in[Nx_max][Ny_max],
      complex value_out[Nx_max][Ny_max]);
   void correlation(
      twoDarray const *value1_twoDarray_ptr,
      twoDarray const *value2_twoDarray_ptr,
      twoDarray* corr_twoDarray_ptr,double& max_corr_value,
      double& x_max,double& y_max);
   void conjugate_transform_array(
      twoDarray const *value2_twoDarray_ptr,
      complex tilde2_conj[Nx_max][Ny_max]);
   void correlation(
      twoDarray const *value1_twoDarray_ptr,
      const complex tilde2_conj[Nx_max][Ny_max],
      twoDarray* corr_twoDarray_ptr,
      double& max_corr_value,double& x_max,double& y_max);
*/

   double brute_binary_correlation(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      int px_offset,int py_offset,double maskvalue,
      int& npixels_inside_mask,twoDarray const *binarymask_twoDarray_ptr,
      twoDarray const *ztwoDarray_binary);

// Metafile manipulation member functions:

   void setup_imageheader();
   void setup_imageheader(double relative_xsize,double relative_ysize);
   void adjust_horizontal_scale(twoDarray *ztwoDarray_ptr);
   void reset_horizontal_scale(twoDarray *ztwoDarray_ptr) const;
   void axes_info(twoDarray const *ztwoDarray_ptr);
   void axes_info_nolabels(twoDarray const *ztwoDarray_ptr);
   void insert_colortable_header(
      double height,double width,double legloc_x,double legloc_y);
   void generate_colortable(
      double minimumz,double maximumz,double height,double width,
      double legloc_x,double legloc_y);
   void generate_colortable(
      double legloc_x,double legloc_y,twoDarray const *ztwoDarray_ptr);
   void addextrainfo(twoDarray const *ztwoDarray_ptr);
   void imagearray_info(twoDarray const *ztwoDarray_ptr);
   void imagefileheader(twoDarray *ztwoDarray_ptr);
   void singletfileheader(twoDarray *ztwoDarray_ptr);
   void doubletfileheader(
      bool display_titles,int doublet_pair_member,std::string doublet_title,
      twoDarray *ztwoDarray_ptr);
   void dual_singlets_header(
      bool display_titles,std::string doublet_title,
      int doublet_pair_member,double imagesize,double yphysor,
      twoDarray *ztwoDarray_ptr);
   void tripletfileheader(
      bool display_titles,int triplet_member,std::string triplet_title,
      twoDarray *ztwoDarray_ptr);
   void writeimagedata(twoDarray const *ztwoDarray_ptr);
   void writeimage(std::string base_imagefilename,twoDarray *ztwoDarray_ptr);
   void writeimage(std::string base_imagefilename,
                   std::string currtitle,twoDarray *ztwoDarray_ptr);
   void writeimage(std::string base_imagefilename,int imgnumber,
                   std::string currtitle,twoDarray *ztwoDarray_ptr);
   void writeimage(
      std::string base_imagefilename,int imgnumber,std::string currtitle,
      double relative_xsize,double relative_ysize,
      twoDarray *ztwoDarray_ptr);
   std::string write_first_singlet_member(
      bool display_titles,std::string doublet_title,
      std::string base_imagefilename,
      double imagesize,double yphysor,twoDarray *ztwoDarray_ptr);
   void write_second_singlet_member(
      std::string doublet_title,double imagesize,double yphysor,
      twoDarray *ztwoDarray_ptr);
   void write_second_singlet_data(twoDarray *ztwoDarray_ptr);
   void writedoublet(
      std::string base_imagefilename,std::string title1,std::string title2,
      twoDarray *ztwoDarray1_ptr,twoDarray *ztwoDarray2_ptr);
   void crop_jpegimage(int n_tableau);

// Image processing member functions:

   void minmax_zarray_values(twoDarray const *ztwoDarray);
   void minmax_zarray_values(
      twoDarray const *ztwoDarray_ptr,double& x_min,double& y_min,
      double& x_max,double& y_max);
   void next_to_minmax_values(
      double& next_to_minz,double& next_to_maxz,
      twoDarray const *ztwoDarray);
   double max_intensity_inside_bbox(
      double minimum_x,double maximum_x,double minimum_y,double maximum_y,
      twoDarray const *ztwoDarray_ptr);
   void reset_zero_level(twoDarray* ztwoDarray_ptr);
   void add_constant_to_zarray(
      twoDarray* ztwoDarray_ptr,double z_fixed,double value);
   void rescale_zarray(twoDarray *ztwoDarray_ptr,double alpha);
   void renormalize_pixel_intensities(
      twoDarray* ztwoDarray_ptr,double zthresh,double zlo,double zhi);
   void renormalize_zarray(double zthresh,twoDarray* ztwoDarray_ptr,
                           double mu,double sigma);
   void zarray_mean_and_stddev(twoDarray const *ztwoDarray_ptr,double zthresh,
                               double& mean,double& std_dev);
   void add_zarrays(
      unsigned int px_lo,unsigned int px_hi,
      unsigned int py_lo,unsigned int py_hi,double w1,double w2,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr,
      twoDarray* ztwoDarray_sum_ptr);
   double integrate_zarray(twoDarray const *ztwoDarray_ptr);
   double integrate_zarray(
      twoDarray const *ztwoDarray_ptr,double& area_integral);
   double integrate_zarray_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      twoDarray const *ztwoDarray_ptr,double& area_integral);
   double integrate_zarray(
      twoDarray const *ztwoDarray_ptr,
      unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,double& area_integral);
   double integrate_difference_image(
      bool absolute_difference_flag,
      unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr);

   double chisq_difference(
      double x2_offset,double y2_offset,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr,
      double black_pixel_penalty_factor,double z_threshold);
   void null_half_plane(
      const threevector& center_pnt,const threevector& null_axis,
      twoDarray const *ztwoDarray_ptr,twoDarray* zhalfnull_twoDarray_ptr);
   double linear_correlation(
      polygon& poly,double threshold_value,twoDarray const *ztwoDarray_ptr);
   void add_noise(double sigma,twoDarray* ztwoDarray_ptr);

// Profiling member functions

   void generate_horizontal_profile(
      double xmin,double xmax,double ymin,double ymax,
      twoDarray const *ztwoDarray_ptr,double horizontal_profile[],
      bool plot_profile=false);

   void generate_horizontal_profile(
      double xmin,double xmax,double ymin,double ymax,
      twoDarray const *ztwoDarray_ptr,
      unsigned int& px_min,unsigned int& px_max,int& imax,
      double horizontal_profile[],double &profile_median,
      bool plot_profile=false);
   void generate_horizontal_profile(
      bool intensity_normalization,
      double xmin,double xmax,double ymin,double ymax,
      twoDarray const *ztwoDarray_ptr,
      unsigned int& px_min,unsigned int& px_max,int& imax,
      double crossrange_profile[],double &profile_median,
      bool plot_profile=false);

   bool generate_vertical_profile(
      double xmin,double xmax,double ymin,double ymax,
      twoDarray const *ztwoDarray_ptr,double range_profile[],
      bool plot_profile=false);
   bool generate_vertical_profile(
      double xmin,double xmax,double ymin,double ymax,
      twoDarray const *ztwoDarray_ptr,
      unsigned int& py_min,unsigned int& py_max,int& jmax,
      double range_profile[],double &profile_median,bool plot_profile=false);
   bool generate_vertical_profile(
      bool remove_floor,double profile_endregion,
      double xmin,double xmax,double ymin,double ymax,
      twoDarray const *ztwoDarray_ptr,
      unsigned int& py_min,unsigned int& py_max,int& jmax,
      double range_profile[],double &profile_median,bool plot_profile=false);
   bool generate_vertical_profile(
      bool plot_profile,bool remove_floor,
      double profile_endregion,
      double xmin,double xmax,double ymin,double ymax,
      twoDarray const *ztwoDarray_ptr,
      unsigned int& py_min,unsigned int& py_max,int& jmax,
      double range_profile[],double &profile_median);

   int compute_radial_intensity_profile(
      double*& filtered_r_profile,twoDarray* ztwoDarray_polar_ptr);
   int compute_polar_angle_intensity_profile(
      double*& filtered_theta_profile,twoDarray const *ztwoDarray_polar_ptr);

// Miscellaneous member functions

   void loop_test(twoDarray const *ztwoDarray2_ptr);

  protected:

   bool classified;
   bool adjust_x_scale;		// true by default.  Flag checked before
				//  adjust_horizontal_scale member function
				//  is called.
   bool imagecrop;		// false by default.  Flag checked before
				//  crop_image member function is called.
   bool viewgraph_mode;		// false by default.  Set to true if 
				//  image is to be used in movie for viewgraph
				//  presentation
   bool dynamic_colortable;	// false by default.  If true, colortable
				//  based upon image values is dynamically
				//  generated.
   bool black_corresponds_to_minimum_z;

   int imagenumber,extra_linenumber;



   double xtic,xsubtic,ytic,ysubtic,xsize,ysize; // metafile params
   double minz,maxz;
   double dynamic_colortable_minz,dynamic_colortable_maxz;

   std::string imagedir,imagefilenamestr,colortablefilename;
   std::vector<std::string> extrainfo;
   
   std::string pagetitle,pagesubtitle;
   std::string title,subtitle,xaxislabel,yaxislabel;
   std::ofstream imagestream;

// theforward and thebackward are pointers to fftwnd_data structures:

   fftwnd_plan theforward,thebackward;	

// TwoDarray objects z2Darray_orig and z2Darray contain intensity
// values for each pixel.  z2Darray_orig essentially contains raw
// imagecdf intensity information; after it is initialized, it is
// never to be touched.  In contrast, z2Darray is the "working" object
// whose contents are altered by all sorts of recursive filters,
// differential thresholds, etc...

   twoDarray *z2Darray_orig_ptr;
   twoDarray *z2Darray_ptr;

// For caching purposes, we include a pointer to a temporary twoDarray
// which should be instantiated only after all initial processing is
// complete:

   twoDarray *ztwoDarray_tmp_ptr;

  private:

   int NSUBTICS;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const myimage& m);

};

// =====================================================================
// Inlined methods
// =====================================================================

inline void myimage::set_imagenumber(int i)
{
   imagenumber=i;
}

inline void myimage::set_classified(bool c)
{
   classified=c;
}

inline void myimage::set_adjust_xscale(bool a)
{
   adjust_x_scale=a;
}

inline void myimage::set_black_corresponds_to_minimumz(bool bctmz)
{
   black_corresponds_to_minimum_z=bctmz;
}

inline void myimage::set_xtic(double x)
{
   xtic=x;
}

inline void myimage::set_ytic(double y)
{
   ytic=y;
}

inline void myimage::set_imagedir(std::string subdir)
{
// If incoming subdirectory name is not terminated with a "/"
// character, append "/" onto the end:

   unsigned slashpos=subdir.find_last_of("/");
   if (slashpos != subdir.length()-1)
   {
      subdir=subdir+"/";
   }
//   imagedir=sysfunc::get_cplusplusrootdir()+subdir;
   imagedir=filefunc::get_pwd()+subdir;
   filefunc::dircreate(imagedir);
}

inline void myimage::set_colortable_filename(std::string filename)
{
   colortablefilename=filename;
}

inline void myimage::set_title(std::string t)
{
   title=t;
}

inline void myimage::set_xaxis_label(std::string label)
{
   xaxislabel=label;
}

inline void myimage::set_yaxis_label(std::string label)
{
   yaxislabel=label;
}

inline void myimage::set_z2Darray_orig_ptr(twoDarray* ztwoDarray_ptr)
{
   z2Darray_orig_ptr=ztwoDarray_ptr;
}

inline void myimage::set_z2Darray_ptr(twoDarray* ztwoDarray_ptr)
{
   z2Darray_ptr=ztwoDarray_ptr;
}

inline void myimage::set_ztwoDarray_tmp_ptr(twoDarray* ztwoDarray_ptr)
{
   ztwoDarray_tmp_ptr=ztwoDarray_ptr;
}

inline bool myimage::get_black_corresponds_to_minimumz() const
{
   return black_corresponds_to_minimum_z;
}

inline int myimage::get_imagenumber() const
{
   return imagenumber;
}

inline void myimage::set_min_z(double zmin)
{
   minz=zmin;
}

inline void myimage::set_max_z(double zmax)
{
   maxz=zmax;
}

inline double myimage::get_min_z() const
{
   return minz;
}

inline double myimage::get_max_z() const
{
   return maxz;
}

inline std::string myimage::get_imagedir()
{
   return imagedir;
}

inline twoDarray* myimage::get_z2Darray_orig_ptr()
{
   return z2Darray_orig_ptr;
}

inline twoDarray* myimage::get_z2Darray_ptr()
{
   return z2Darray_ptr;
}

inline twoDarray* myimage::get_ztwoDarray_tmp_ptr()
{
   return ztwoDarray_tmp_ptr;
}

#endif // myimage.h



