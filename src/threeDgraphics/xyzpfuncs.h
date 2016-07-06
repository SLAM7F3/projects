// =========================================================================
// Header file for stand-alone XYZP data manipulation functions.
// =========================================================================
// Last modified on 12/2/10; 11/20/11; 1/29/12; 4/5/14
// =========================================================================

#ifndef XYZPFUNCS_H
#define XYZPFUNCS_H

#include <osg/Array>
#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "math/genvector.h"

class rotation;
class threevector;
template <class T> class TwoDarray;
typedef TwoDarray<double> twoDarray;

namespace xyzpfunc
{

   extern const double null_value;  // Indicates missing data

// XYZP file input methods:

   unsigned int npoints_inside_p_file(std::string xyz_filename,int p_size);
   unsigned int npoints_inside_xyz_file(std::string xyz_filename);
   unsigned int npoints_inside_xyzp_file(std::string xyzp_filename);
   std::vector<threevector>* parse_Hokoyu_binary_datafile(
      std::string hokoyu_filename,threevector& robo_coords);

   void read_xyz_float_data(unsigned int npoints,std::string xyz_filename,
                            double x[],double y[],double z[]);
   std::vector<threevector>* read_xyz_float_data(std::string xyz_filename);
   void read_xyzp_float_data(unsigned int npoints,std::string xyzp_filename,
                             double x[],double y[],double z[],double p[]);
   int read_xyzp_float_data(
      std::string xyzp_filename,std::vector<double>* X_ptr,
      std::vector<double>* Y_ptr,std::vector<double>* Z_ptr,
      std::vector<double>* P_ptr);

   void read_xyzp_float_data(
      std::string xyzp_filename,std::vector<threevector>& XYZ,
      std::vector<double>& P);

   std::vector<fourvector>* read_xyzp_float_data(
      std::string xyzp_filename,double pnull_threshold=0);
   void read_xyzp_float_data(
      std::string xyzp_filename,double pnull_threshold,
      std::vector<fourvector>* xyzp_pnt_ptr);

   std::vector<threevector>* read_just_xyz_float_data(
      std::string xyzp_filename);
   std::vector<fourvector>* convert_xyzrgba_to_xyzp_data(
      std::string xyzrgba_filename,double null_p_value);
   void read_xyz_double_data(unsigned int npoints,std::string xyzp_filename,
                             double x[],double y[],double z[]);

   void read_xyzrgba_data(
      std::string xyzrgba_filename,std::vector<threevector>* xyz_pnt_ptr,
      std::vector<colorfunc::RGB>* rgb_pnt_ptr);

   int fill_image_with_z_and_p_values(
      unsigned int npoints,double x[],double y[],double z[],double p[],
      twoDarray* ztwoDarray_ptr,twoDarray* ptwoDarray_ptr);
   int fill_image_with_z_and_p_values(
      const std::vector<threevector>& XYZ,const std::vector<double>& p,
      twoDarray* ztwoDarray_ptr,twoDarray* ptwoDarray_ptr);
   int fill_image_with_z_and_p_values(
      osg::Vec3Array* vertices_ptr,const osg::FloatArray* probs_ptr,
      twoDarray* ztwoDarray_ptr,twoDarray* ptwoDarray_ptr);
   int fill_image_with_z_values(
      osg::Vec3Array* vertices_ptr,twoDarray* ztwoDarray_ptr);

// XYZP file output methods:

   void write_xyz_ascii_data(
      const std::vector<fourvector>* xyzp_pnt_ptr,std::string xyz_filename);
   void write_xyz_data(
      const std::vector<double>& X,const std::vector<double>& Y,
      const std::vector<double>& Z,std::string xyz_filename);
   void write_xyz_data(
      const std::vector<double>& X,const std::vector<double>& Y,
      const std::vector<double>& Z,
      const std::vector<bool>& include_point_flag,std::string xyzp_filename);

   void write_xyzp_data(
      const twoDarray* ztwoDarray_ptr,const twoDarray* ptwoDarray_ptr,
      std::string xyzp_filename,bool p_represents_genuine_prob=true,
      bool gzip_output_file=true);
   void write_xyzp_data(
      std::string xyzp_filename,std::vector<double>* X_ptr,
      std::vector<double>* Y_ptr,std::vector<double>* Z_ptr,
      std::vector<double>* P_ptr);
   void write_xyzp_data(
      unsigned int n_xyz_points,double x[],double y[],double z[],double p[],
      std::string xyzp_filename,bool p_represents_genuine_prob);
   void write_xyzp_data(
      const std::vector<threevector>& XYZ,const std::vector<double>& P,
      std::string xyzp_filename,bool p_represents_genuine_prob,
      bool gzip_output_file=true);
   
   void write_xyz_data(
      std::string xyz_filename,std::vector<threevector>* xyz_pnt_ptr,
      bool gzip_output_file=true);
   void write_xyzp_data(
      std::string xyzp_filename,std::vector<fourvector>* xyzp_pnt_ptr,
      bool gzip_output_file=true);
   void write_xyzp_data(
      std::string xyzp_filename,std::vector<genvector>* xyzp_pnt_ptr,
      bool gzip_output_file=true);

   void write_local_xyzp_data(
      double x0,double y0,double r0,
      const twoDarray* ztwoDarray_ptr,const twoDarray* ptwoDarray_ptr,
      std::string xyzp_filename,bool p_represents_genuine_prob=true);
   void write_local_xyzp_data(
      const threevector& center_posn,double rho,
      std::string xyzp_filename,std::vector<fourvector>* xyzp_pnt_ptr,
      bool gzip_output_file=false);
   void write_local_xyzp_data(
      const threevector& center_posn,double rho,
      std::string xyzp_filename,std::vector<genvector>* xyzp_pnt_ptr,
      bool gzip_output_file=false);
   void write_annotated_xyzp_data(
      const twoDarray* ztwoDarray_ptr,const twoDarray* ptwoDarray_ptr,
      std::string xyzp_filename,double z_threshold=POSITIVEINFINITY);
   void write_annotated_xyzp_data(
      const twoDarray* ztwoDarray_ptr,const twoDarray* ptwoDarray_ptr,
      const twoDarray* ftwoDarray_ptr,std::string xyzp_filename,
      double z_threshold=POSITIVEINFINITY);

   void write_single_xyz_point(
      std::ofstream& binary_outstream,float x,float y,float z);
   void write_single_xyz_point(
      std::ofstream& binary_outstream,const threevector& curr_point);

   void write_single_xyzp_point(
      std::ofstream& binary_outstream,float x,float y,float z,float p);
   void write_single_xyzp_point(
      std::ofstream& binary_outstream,float x,float y,float z,float p,
      float& zmax,float& zmin,float& pmax,float& pmin,
      unsigned int& npoints_written);
   void write_single_xyzp_point(
      std::ofstream& binary_outstream,const threevector& curr_point,
      double curr_p);
   void write_single_xyzp_point(
      std::ofstream& binary_outstream,const fourvector& curr_point);
   void write_single_xyzp_point(
      std::ofstream& binary_outstream,const genvector& curr_point);

   void write_single_xyzrgba_point(
      std::ofstream& binary_outstream,const threevector& curr_point,
      colorfunc::RGB curr_RGB,bool normalized_input_RGB_values);
   void write_single_xyzrgba_point(
      std::ofstream& binary_outstream,const threevector& curr_point,
      osg::Vec4ub& curr_RGB);
   void write_single_xyzrgba_point(
      std::ofstream& binary_outstream,const osg::Vec3& curr_point,
      osg::Vec4ub& curr_RGB);

// Height and intensity image fusion methods:

   twoDarray* fuse_z_and_p_images(
      double p_hi,double p_lo,
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr);
   twoDarray* fuse_z_and_binary_p_images(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr);
   twoDarray* fuse_z_and_p_images(
      double v_hi,double v_lo,double zmax,double zmin,
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr,
      bool map_prob_onto_intensity_rather_than_saturation=true);

   void convert_zp_to_hue_intensity_and_saturation(
      double wrap_frac,double zmax,double zmin,double z,double p,
      double v_hi,double v_lo,double& h,double& v,double& s);
   void convert_zp_to_hue_and_intensity(
      double zmax,double zmin,double z,double p,double v_hi,double v_lo,
      double& h,double& v,double hue_hi=300,double hue_lo=0);
   void convert_zp_to_hue_and_saturation(
      double z,double p,double zmax,double zmin,
      double s_hi,double s_lo,double& h,double& s);
   void convert_hue_and_intensity_to_f(double h,double v,double& f);
   void convert_hue_and_saturation_to_f(double h,double s,double& f);

// XYZ point cloud properties:

   double interpoint_RMS_distance(
      double minimal_separation,std::vector<fourvector>* XYZP_ptr);
   threevector center_of_mass(std::vector<fourvector> const *xyzp_point_ptr);
   threevector center_of_mass(std::vector<genvector> const *xyzp_point_ptr);
   fourvector max_values(std::vector<fourvector> const *xyzp_point_ptr);
   fourvector min_values(std::vector<fourvector> const *xyzp_point_ptr);
   double zdist_percentile_height(
      std::vector<genvector> const *xyzp_point_ptr,double cum_prob);
   void plot_p_distribution(std::vector<genvector> const *xyzp_point_ptr);
   void compute_extremal_XYZ_points_in_xyzpfile(
      std::string xyzp_filename,threevector& XYZ_min,threevector& XYZ_max);

// XYZ point manipulation methods:

   void translate(
      const threevector& trans,std::vector<fourvector>* xyzp_point_ptr);
   void translate(
      const threevector& trans,std::vector<genvector>* xyzp_point_ptr);
   void scale(double sfactor,std::vector<genvector>* xyzp_point_ptr);
   void rotate(const threevector& rotation_origin,
               std::vector<fourvector>* xyzp_point_ptr,
               double thetax,double thetay,double thetaz);
   void rotate(const threevector& rotation_origin,
               std::vector<genvector>* xyzp_point_ptr,
               double thetax,double thetay,double thetaz);
   void rotate(const rotation& R,std::vector<fourvector>* xyzp_point_ptr);
   void rotate(const rotation& R,std::vector<genvector>* xyzp_point_ptr);
   void rotate(const threevector& rotation_origin,const rotation& R,
               std::vector<fourvector>* xyz_point_ptr);
   void rotate(const threevector& rotation_origin,const rotation& R,
               std::vector<genvector>* xyz_point_ptr);
   std::vector<fourvector>* XY0P_projection(
      std::vector<fourvector> const *xyzp_point_ptr);
   void subdivide_xy_spacing(
      std::string subdivided_xyzp_filename,double ds_old,double ds_new,
      const threevector& center_posn,double rho,
      std::vector<fourvector>* xyzp_point_ptr);
   void subdivide_xy_spacing(
      std::string subdivided_xyzp_filename,double ds_old,double ds_new,
      const threevector& center_posn,double rho,
      std::vector<genvector>* xyzp_point_ptr);

// P value manipulation methods:

   void renormalize_p_values(
      double min_p,double max_p,std::vector<fourvector>* xyzp_point_ptr);
   void renormalize_p_values(
      double min_p,double max_p,double p_threshold,
      std::vector<fourvector>* xyzp_point_ptr);
   void reset_null_p_values(double null_value,double new_null_value,
                            std::vector<fourvector>* xyzp_point_ptr);
   void reset_null_p_values(
      double min_value,double max_value,double new_null_value,
      std::vector<fourvector>* xyzp_point_ptr);
   void threshold_p_values(
      double min_p,std::vector<genvector>*& xyzp_point_ptr);
   void recolor_p_values_for_RGB_colormap(
      std::string xyzp_filename,std::string RGB_xyzp_filename);
   void recolor_p_values_for_RGB_colormap(
      std::vector<fourvector>* xyzp_point_ptr);
   void scale_p_values_to_heights(
      double z_min,double z_max,double p_min,double p_max,
      std::vector<genvector>* xyzp_point_ptr);

   void write_p_values_as_RGBA_bytes(
      std::string xyzp_filename,std::string RGBA_xyzp_filename);
   void write_p_values_as_RGBA_bytes(
      std::vector<fourvector>* xyzp_point_ptr,
      std::ofstream& binary_outstream);

   void density_filter(
      double maximum_separation,std::vector<fourvector>* XYZP_ptr);


// ==========================================================================
// Inlined methods
// ==========================================================================

   template <class T> inline void write_p_data(
      std::string p_filename,std::vector<T>* pdata_ptr,
      bool gzip_output_file)
      {
         std::ofstream binary_outstream;
         binary_outstream.open(p_filename.c_str(),
                               std::ios::app|std::ios::binary);

         for (unsigned int n=0; n<pdata_ptr->size(); n++)
         {
            filefunc::writeobject(binary_outstream,(*pdata_ptr)[n]);
         } 
         binary_outstream.close();  
         if (gzip_output_file) filefunc::gzip_file(p_filename);
      }

// ---------------------------------------------------------------------
   template <class T> inline void read_p_data(
      std::string p_filename,std::vector<T>* pdata_ptr)
      {
         std::ifstream binary_instream;
         bool file_opened=false;
         do
         {
            filefunc::gunzip_file_if_gzipped(p_filename);
            file_opened=filefunc::open_binaryfile(
               p_filename,binary_instream);
         }
         while (!file_opened);
         unsigned int npoints=npoints_inside_p_file(p_filename,sizeof(T));

         T curr_p;
         for (unsigned int n=0; n<npoints; n++)
         {
            filefunc::readobject(binary_instream,curr_p);
            pdata_ptr->push_back(curr_p);
         } // loop over index n labeling input points

         binary_instream.close();  
      }

}

#endif // xyzpfuncs.h




