// Note added on 10/28/07: We should write a specialized
// write_ztwoDarray(ztwoDarray_ptr) method which outputs a
// ptwoDarray_ptr with constant zero values...

// =========================================================================
// Header file for stand-alone TDP data manipulation functions
// =========================================================================
// Last modified on 2/28/13; 3/4/13; 3/5/13
// =========================================================================

#ifndef TDPFUNCS_H
#define TDPFUNCS_H

#include <osg/Array>
#include <set>
#include <string>
#include <vector>
#include "math/fourvector.h"
#include "model/Metadata.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"

class Tdp_file;

namespace tdpfunc
{

// TDP file input methods:

   unsigned int npoints_in_tdpfile(std::string tdp_filename);
   bool parse_UTM_info(
      Tdp_file& tdp_file,std::string& UTMzone,threevector& UTM_offset);

   void compute_extremal_XYZ_points_in_tdpfile(
      std::string tdp_filename,threevector& XYZ_min,threevector& XYZ_max);
   void compute_extremal_XYZ_values(
      int npoints_to_read,Tdp_file& tdp_file,int& byte_offset,
      const threevector& UTM_offset,threevector& XYZ_min,threevector& XYZ_max);

   void read_curr_XYZ_points(
      int npoints_to_read,Tdp_file& tdp_file,int& byte_offset,
      const threevector& UTM_offset,
      std::vector<double>& X,std::vector<double>& Y,std::vector<double>& Z);
   void read_curr_XYZP_points(
      int npoints_to_read,Tdp_file& tdp_file,int& p_byte_offset,
      const threevector& UTM_offset,std::vector<double>& X,
      std::vector<double>& Y,std::vector<double>& Z,std::vector<double>& P);

   void read_XYZ_points_from_tdpfile(
      std::string tdp_filename,
      std::vector<double>& X,std::vector<double>& Y,std::vector<double>& Z);
   void read_XYZP_points_from_tdpfile(
      std::string tdp_filename,
      std::vector<double>& X,std::vector<double>& Y,std::vector<double>& Z,
      std::vector<double>& P);
   void read_XYZRGB_points_from_tdpfile(
      std::string tdp_filename,
      std::vector<double>& X,std::vector<double>& Y,std::vector<double>& Z,
      std::vector<int>& R,std::vector<int>& G,std::vector<int>& B);

   bool parse_pointcloud_color_data(
      int n_points,Tdp_file& tdp_file,osg::Vec4ubArray* colors);

// TwoDarray input methods:

   twoDarray* generate_ztwoDarray_from_tdpfile(
      std::string tdp_filename,double delta_x,double delta_y);
   void fill_ztwoDarray_from_tdpfile(
      std::string tdp_filename,twoDarray* ztwoDarray_ptr);
   std::pair<twoDarray*,twoDarray*>
      generate_ztwoDarray_and_ptwoDarray_from_tdpfile(
         std::string tdp_filename,double delta_x,double delta_y);

// TDP file output initialization methods:

   int get_n_iters(int n_points);
   void initialize_output_tdpfile(
      std::string tdp_filename,std::string UTMzone,
      const osg::Vec3Array* vertices_ptr,
      Tdp_file& tdp_file,osg::Vec3& zeroth_xyz,int& n_iters);
   void initialize_output_tdpfile(
      std::string tdp_filename,std::string UTMzone,
      Tdp_file& tdp_file,const threevector& zeroth_xyz);
   void write_UTM_zone_and_offset(Tdp_file& tdp_file,std::string UTMzone,
                                  const threevector& zeroth_XYZ);

// Relative vertex output methods:

   void write_relative_xyz_data(
      std::string tdp_filename,std::string UTMzone,
      const osg::Vec3Array* vertices_ptr);
   void write_relative_xyz_data(
      std::string tdp_filename,std::string UTMzone,
      const osg::Vec3& specified_origin,
      const osg::Vec3Array* vertices_ptr);
   void write_relative_xyz_data(
      const osg::Vec3& origin_xyz,Tdp_file& tdp_file,
      const osg::Vec3Array* vertices_ptr);

   void write_relative_xyz_data(
      std::string tdp_filename,
      const std::vector<double>& X,const std::vector<double>& Y,
      const std::vector<double>& Z);

   void write_relative_xyz_data(
      std::string tdp_filename,const std::vector<threevector>& vertices);
   void write_relative_xyz_data(
      std::string tdp_filename,const threevector& origin,
      const std::vector<threevector>& vertices);
   void write_relative_xyz_data(
      const threevector& origin,Tdp_file& tdp_file,
      const std::vector<threevector>& vertices);

   void write_relative_xyzp_data(
      std::string tdp_filename,std::string UTMzone,
      const osg::Vec3Array* vertices_ptr,const model::Metadata* metadata_ptr);
   void write_relative_xyzp_data(
      std::string tdp_filename,std::string UTMzone,
      const osg::Vec3Array* vertices_ptr,const osg::FloatArray* prob_ptr);

// Relative XYZRGBA output methods:

   void write_relative_xyzrgba_data(
      std::string tdp_filename,std::string UTMzone,
      const osg::Vec3Array* vertices_ptr,const osg::Vec4ubArray* colors_ptr);
   void write_relative_xyzrgba_data(
      std::string tdp_filename,std::string UTMzone,
      const threevector& zeroth_xyz,
      const osg::Vec3Array* vertices_ptr,const osg::Vec4ubArray* colors_ptr);
   void write_relative_xyzrgba_data(
      std::string tdp_filename,std::string UTMzone,
      const threevector& zeroth_xyz,
      const std::vector<threevector>* vertices_ptr,
      const osg::Vec4ubArray* colors_ptr);
   void write_relative_xyzrgba_data(
      std::string UTMzone,std::string tdp_filename,
      const std::vector<double>& X,const std::vector<double>& Y,
      const std::vector<double>& Z,const std::vector<int>& R,
      const std::vector<int>& G,const std::vector<int>& B);
   void write_relative_xyzrgba_data(
      std::string tdp_filename,std::string UTMzone,
      const twoDarray* RtwoDarray_ptr,const twoDarray* GtwoDarray_ptr,
      const twoDarray* BtwoDarray_ptr,int nodata_value=-999);
   void write_relative_xyzrgba_data(
      std::string tdp_filename,std::string UTMzone,
      const twoDarray* ztwoDarray_ptr,const twoDarray* RtwoDarray_ptr,
      const twoDarray* GtwoDarray_ptr,const twoDarray* BtwoDarray_ptr);

   void write_zp_twoDarrays(
      std::string tdp_filename,std::string UTMzone,
      const twoDarray* ztwoDarray_ptr,const twoDarray* ptwoDarray_ptr,
      bool insert_fake_coloring_points_flag=true);

// XYZ output methods:

   void write_xyz_data(
      std::string tdp_filename,
      std::vector<double>* X_ptr,
      std::vector<double>* Y_ptr,std::vector<double>* Z_ptr);
   void write_xyz_data(
      std::string tdp_filename,std::string UTMzone,
      const threevector& zeroth_XYZ,
      std::vector<double>* X_ptr,std::vector<double>* Y_ptr,
      std::vector<double>* Z_ptr);
   void write_xyz_data(
      std::string UTMzone,std::string tdp_filename,
      std::vector<fourvector>* xyzp_pnt_ptr);

   void write_xyz_data(
      std::string UTMzone,std::string tdp_filename,
      const std::vector<double>& X,const std::vector<double>& Y,
      const std::vector<double>& Z,
      const std::vector<bool> include_orig_point);
   void write_xyz_data(
      std::string UTMzone,std::string tdp_filename,
      const twoDarray* ztwoDarray_ptr);
   void write_xyz_data(
      std::string UTMzone,std::string tdp_filename,
      const twoDarray* ztwoDarray_ptr,
      const std::vector<double>& X,const std::vector<double>& Y,
      const std::vector<double>& Z,
      const std::vector<bool> include_orig_point);

// XYZP output methods:

   void write_xyzp_data(
      std::string tdp_filename,std::vector<threevector>* XYZ_ptr,
      std::vector<double>* P_ptr);
   void write_xyzp_data(
      std::string tdp_filename,std::vector<double>* X_ptr,
      std::vector<double>* Y_ptr,std::vector<double>* Z_ptr,
      std::vector<double>* P_ptr);
   void write_xyzp_data(
      std::string tdp_filename,
      std::string UTMzone,const threevector& zeroth_XYZ,
      std::vector<double>* X_ptr,std::vector<double>* Y_ptr,
      std::vector<double>* Z_ptr,std::vector<double>* P_ptr);

   void write_xyzp_data(
      std::string tdp_filename,std::vector<fourvector>* xyzp_pnt_ptr);
   void write_xyzp_data(
      std::string tdp_filename,
      std::string UTMzone,const threevector& zeroth_XYZ,
      std::vector<fourvector>* xyzp_pnt_ptr);

}

#endif // tdpfuncs.h




