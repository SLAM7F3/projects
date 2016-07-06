// ==========================================================================
// Header file for stand-alone Kakadu functions 
// ==========================================================================
// Last updated on 10/16/07; 4/16/11
// ==========================================================================

#ifndef KAKADU_H
#define KAKADU_H

// Kakadu core includes
#include "kdu_elementary.h"
#include "kdu_messaging.h"
#include "kdu_params.h"
#include "kdu_compressed.h"
#include "kdu_sample_processing.h"
#include "kdu_arch.h"
// Application includes
#include "kdu_args.h"
#include "kdu_image.h"
#include "kdu_file_io.h"
#include "jpx.h"
#include "expand_local.h"

namespace kakadufunc
{
   void print_version();
   void print_usage(char* prog, bool comprehensive=false);
   kde_file_binding* parse_simple_args(
      kdu_args& args, char* &ifname,
      std::ostream* &record_stream,
      float& max_bpp, bool& transpose, bool& vflip, bool& hflip,
      bool& allow_shorts, int& skip_components,
      bool& want_alpha, int& jpx_layer, int& raw_codestream,
      kdu_component_access_mode& component_access_mode,
      bool& no_seek, int& max_layers, int& discard_levels,
      int& num_threads, int& double_buffering_height,
      int& cpu_iterations, bool& simulate_parsing,bool& mem, bool& quiet);

   kdu_long get_bpp_dims(siz_params* siz);
   void set_region_of_interest(
      kdu_args& args, kdu_dims& region, siz_params* siz);
   void extract_jp2_resolution_info(
      kdu_image_dims& idims, jp2_resolution resolution,
      kdu_coords ref_size, bool transpose);
   void extract_jp2_colour_info(kdu_image_dims& idims, jp2_channels channels,
                                jp2_colour colour, bool have_alpha,
                                bool alpha_is_premultiplied);
   void set_error_behaviour(kdu_args& args, kdu_codestream codestream);
   void convert_samples_to_palette_indices(
      kdu_line_buf& src, kdu_line_buf& dst,
      int bit_depth, bool is_signed,int palette_bits);
   kdu_long expand_single_threaded(
      kdu_codestream codestream, kdu_dims tile_indices,
      kde_file_binding* outputs, int num_output_channels,
      bool last_output_channel_is_alpha,
      bool alpha_is_premultiplied,
      int num_used_components, int* used_component_indices,
      jp2_channels channels, jp2_palette palette,
      bool allow_shorts, bool skip_ycc,int dwt_stripe_height);
   kdu_long expand_multi_threaded(
      kdu_codestream codestream, kdu_dims tile_indices,
      kde_file_binding* outputs, int num_output_channels,
      bool last_output_channel_is_alpha,
      bool alpha_is_premultiplied,
      int num_used_components, int* used_component_indices,
      jp2_channels channels, jp2_palette palette,
      bool allow_shorts, bool skip_ycc, int& num_threads,
      bool dwt_double_buffering, int dwt_stripe_height);

   short* KDUReadJP2Mono16( 
      char filename[], int& sizeX, int& sizeY, double& res, 
      bool isSigned = true, int precision = 16,short* outBuf = NULL );




}

#endif  // kakadufunc namespace




