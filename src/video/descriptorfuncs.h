// ==========================================================================
// Header file for descriptorfunc namespace
// ==========================================================================
// Last modified on 10/5/13; 10/6/13; 10/8/13
// ==========================================================================

#ifndef DESCRIPTORFUNCS_H
#define DESCRIPTORFUNCS_H

#include <string>
#include <utility>

class descriptor;
class genmatrix;
class RGB_analyzer;
class texture_rectangle;

#include "image/TwoDarray.h"

namespace descriptorfunc
{

// GIST descriptor methods:

   bool compute_gist_descriptor(
      std::string image_filename,std::string gist_filename);
   genmatrix* GIST_descriptor_matrix(
      const std::vector<std::string>& gist_filenames,
      int n_descriptors=-1);
   genmatrix* GIST_descriptor_matrix(
      std::string gist_subdir,const std::vector<std::string>& image_filenames);

// Color histogram methods:

   std::vector<double> compute_color_histogram(
      std::string image_filename,std::string color_histogram_filename,
      texture_rectangle* texture_rectangle_ptr,RGB_analyzer* RGB_analyzer_ptr);
   std::vector<double> compute_sector_color_histogram(
      int n_rows,int n_columns,int row_ID,int column_ID,
      std::string image_filename,
      texture_rectangle* texture_rectangle_ptr,RGB_analyzer* RGB_analyzer_ptr);

// Edge histogram descriptor methods:

   std::vector<double> compute_edge_histogram(
      std::string image_filename,
      texture_rectangle* texture_rectangle_ptr);
   std::pair<double,int> filter_image_block(
      bool& monotone_flag,texture_rectangle* texture_rectangle_ptr,
      const std::vector<double>& fv,const std::vector<double>& fh,
      const std::vector<double>& f45,const std::vector<double>& f135,
      const std::vector<double>& fnd,
      int pu_start,int pu_stop,int pv_start,int pv_stop);

// Line segments descriptor methods:

   std::vector<double> compute_linesegments_histogram(
      std::string image_filename);
   std::vector<double> compute_linesegments_histogram(
      int n_frac_mag_bins,int n_theta_bins,std::string image_filename);
   std::vector<double> compute_image_segments_histogram(
      int n_horiz_subimage_bins,int n_vert_subimage_bins,
      int n_frac_mag_bins,int n_theta_bins,std::string image_filename);
//   std::vector<double> compute_image_segments_histogram(
//      int n_subimage_bins,int n_frac_mag_bins,std::string image_filename);

// Texture descriptor methods:

   std::vector<double> compute_RGB_texture_histogram(
      std::string image_filename,
      int n_horiz_subimage_bins,int n_vert_subimage_bins,
      int n_frac_mag_bins,int n_theta_bins);
   void compute_texture_channel_histogram(
      int n_horiz_subimage_bins,int n_vert_subimage_bins,
      int n_frac_mag_bins,int n_theta_bins,
      twoDarray* ptwoDarray_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      twoDarray* gradient_mag_twoDarray_ptr,
      twoDarray* gradient_phase_twoDarray_ptr,
      std::vector<double>& texture_channel_histogram);
   void compute_texture_channel_histogram(
      int n_horiz_subimage_bins,int n_vert_subimage_bins,
      int n_frac_mag_bins,int n_theta_bins,
      twoDarray const *gradient_mag_twoDarray_ptr,
      twoDarray const *gradient_phase_twoDarray_ptr,
      std::vector<double>& texture_channel_histogram);

// Self similarity descriptor methods:

   std::vector<double> compute_SSIM_descriptor(
      std::string image_filename,
      int n_radial_bins,int n_theta_bins);
   twoDarray* log_polar_bins(
      int patch_length,int neighborhood_radius,
      int n_radial_bins,int n_theta_bins);

// Local Binary Pattern descriptor methods:

   std::vector<double> compute_LBP_descriptor(std::string image_filename);

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // descriptorfunc namespace

#endif // descriptorfuncs.h

