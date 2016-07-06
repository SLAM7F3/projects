// ==========================================================================
// Textfuncs namespace header
// ==========================================================================
// Last updated on 3/27/16; 4/5/16; 4/17/16; 4/20/16
// ==========================================================================

#ifndef TEXTFUNCS_H
#define TEXTFUNCS_H

#include <string>
#include <vector>
#include "dlib/svm.h"
#include "color/colorfuncs.h"

class imagetext;
class texture_rectangle;
class threevector;
class twovector;

// ==========================================================================
// Function declarations
// ==========================================================================

namespace textfunc
{
   void generate_character_set(
      char& char_type, std::string& synthetic_char_set_str,
      std::vector<std::string>& cleaned_chars);
   void generate_string_set(
      char& string_type, std::vector<std::string>& cleaned_strings);
   void generate_phrase_set(std::vector<std::string>& string_phrases);
   std::string spaced_out_text_label(std::string text_label, int skip);
   std::string vertical_text_label(std::string text_label);

// Text image chip background methods

   void retrieve_nonsymbol_background_filenames(
      std::vector<std::string>& nonsymbol_filenames,
      std::vector<int>& nonsymbol_filename_frequencies);
   void compute_background_image_frequencies(
      const std::vector<std::string>& nonsymbol_filenames,
      std::vector<int>& nonsymbol_filename_frequencies);
   void compute_background_image_entropy_frequencies(
      const std::vector<std::string>& nonsymbol_filenames,
      std::vector<int>& nonsymbol_filename_frequencies);
   void import_all_background_images(
      const std::vector<std::string>& background_image_filenames,
      std::vector<texture_rectangle*>& random_texture_rectangle_ptrs);
   void extract_background_patch(
      int rx_start,int ry_start,
      texture_rectangle* random_texture_rectangle_ptr,
      texture_rectangle* texture_rectangle_ptr);
   texture_rectangle* initialize_random_image(
      const std::vector<std::string>& nonsymbol_filenames,
      const std::vector<int>& nonsymbol_filename_frequencies,
      const std::vector<texture_rectangle*>& random_texture_rectangle_ptrs,
      unsigned int cropped_udim,unsigned int cropped_vdim,
      int& rx_start,int& ry_start);
   colorfunc::HSV peak_random_patch_hsv(
      int rx_start,int ry_start,
      unsigned int cropped_udim,unsigned int cropped_vdim,
      texture_rectangle* random_texture_rectangle_ptr);
   void synthesize_background_patch(
      texture_rectangle* background_texture_rectangle_ptr,
      colorfunc::RGB background_RGB);

   void add_random_background_lines(
      texture_rectangle* background_texture_rectangle_ptr,
      colorfunc::HSV background_HSV,
      double s_lambda, double v_mu, double v_sigma);

// Foreground/background color methods

   void import_foreground_background_colors_decision_function();
   bool test_foreground_background_color_acceptability(
      double classification_score_threshold, colorfunc::RGB& foreground_RGB,
      colorfunc::RGB& background_RGB);
   bool generate_acceptable_foreground_color(
      double lambda_s, double mu_v, double sigma_v, 
      double classification_score_threshold, 
      colorfunc::RGB& foreground_RGB, colorfunc::RGB& background_RGB,
      int iter_max = 1000);

   void count_foreground_background_pixels(
      texture_rectangle* texture_rectangle_ptr, 
      colorfunc::RGB& text_background_RGB,
      int& n_foreground_pixels, int& n_background_pixels);
   double foreground_pixel_frac(
      texture_rectangle* texture_rectangle_ptr, 
      colorfunc::RGB& text_background_RGB);

// Text image chip foreground methods:

   void compute_foreground_bbox(
      colorfunc::RGB& text_background_RGB,
      const texture_rectangle* texture_rectangle_ptr,
      int& px_min, int& px_max, int& py_min, int& py_max);
   void draw_bbox_under_text_char(
      colorfunc::RGB& text_background_RGB, colorfunc::RGB& text_foreground_RGB,
      texture_rectangle* foreground_texture_rectangle_ptr,
      int px_min, int px_max, int py_min, int py_max);
   void draw_bbox_under_text_char(
      colorfunc::RGB& text_background_RGB, colorfunc::RGB& foreground_RGB,
      colorfunc::RGB& stroke_RGB, double strokewidth,
      texture_rectangle* foreground_texture_rectangle_ptr);

   texture_rectangle* select_new_foreground_color_and_background_patch(
      bool generate_greyscale_chips_flag,
      const std::vector<std::string>& nonsymbol_filenames,
      const std::vector<int>& nonsymbol_filename_frequencies,
      const std::vector<texture_rectangle*>& random_texture_rectangle_ptrs,
      unsigned int cropped_udim,unsigned int cropped_vdim,
      int& rx_start,int& ry_start,
      colorfunc::RGB& foreground_RGB,colorfunc::RGB& stroke_RGB);
   double superpose_foreground_on_background_patch(
      colorfunc::RGBA text_background_RGBA,
      texture_rectangle* background_texture_rectangle_ptr,
      texture_rectangle* texture_rectangle_ptr);
   void copy_background_patch(
      texture_rectangle* background_texture_rectangle_ptr,
      texture_rectangle* texture_rectangle_ptr);

// Text image chip rotation methods:

   bool rotate_image_and_mask(
      std::string image_filename, std::string mask_filename,
      std::string rotated_images_subdir, std::string rotated_masks_subdir,
      double max_pixel_width);
   bool rotate_image_and_masks(
      std::string image_filename, std::string mask1_filename, 
      std::string mask2_filename,
      std::string rotated_images_subdir,std::string rotated_masks1_subdir,
      std::string rotated_masks2_subdir,double max_pixel_width,
      double max_abs_az, double max_abs_el, double max_abs_roll,
      std::string& rotated_image_filename, std::string& rotated_mask1_filename,
      std::string& rotated_mask2_filename);

   void perspective_projection(
      double FOV_u, double aspect_ratio, double max_pixel_width,
      double img_az, double img_el, double img_roll, 
      std::string input_image_filename, std::string background_color,
      std::string output_png_filename);
   void randomly_rotate_foreground_symbol(
      double max_az, double max_el, double max_roll,
      colorfunc::RGB& text_background_RGB,
      const texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* rotated_texture_rectangle_ptr);
   void generate_random_rotation(
      threevector& n_hat,threevector& uprime_hat,threevector& vprime_hat,
      double max_az, double max_el, double max_roll);

// Text image chip corruption methods

   void add_gaussian_noise(texture_rectangle* texture_rectangle_ptr,
                           double sigma);
   void add_foreground_occlusions(texture_rectangle* texture_rectangle_ptr);
   int shade_value(int R,double alpha,const twovector& q,
                   const twovector& origin,const twovector& e_hat);
   double random_gaussian_blur_sigma(double udim_lo, double udim_hi, 
                                     double udim);
   void randomly_crop_chip(std::string chip_filename);
   void synthesize_solar_shadow(texture_rectangle* texture_rectangle_ptr);

// Deep learning preparation methods

   double compute_px_COM(texture_rectangle* texture_rectangle_ptr);
   bool is_foreground_pixel(
      bool dark_text_background, int min_value, int max_value,
      int Xforeground, int Xbackground);
   bool generate_segmentation_mask(
      colorfunc::RGBA text_background_RGBA,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* background_texture_rectangle_ptr,
      texture_rectangle* mask_texture_rectangle_ptr);
   bool check_mask_values(
      std::string image_filename, std::string char_mask_filename, 
      std::string word_mask_filename, 
      int min_charmask_value, int max_charmask_value,
      int min_wordmask_value, int max_wordmask_value);

// Text line processing:

   void select_text_phrase(
      std::vector<std::string>& cleaned_strings, int min_n_words,
      int max_n_words, imagetext& curr_imagetext);
   void select_text_phrase(
      std::vector<std::string>& cleaned_strings, int min_n_words,
      int max_n_words, int min_n_chars, int max_n_chars,
      imagetext& curr_imagetext);
   void n_rendered_text_lines(imagetext& curr_imagetext);
   bool extract_char_widths_and_height(
      int img_width, int img_height, imagetext& curr_imagetext);

   void retrieve_substr_width_height(
      int input_img_width, int input_img_height, 
      std::string substr, const imagetext& curr_imagetext,
      int& substr_width, int& substr_height);
   void retrieve_substr_and_image_widths_heights(
      int input_img_width, int input_img_height, std::string substr, 
      const imagetext& curr_imagetext,
      int& substr_width, int& substr_height,
      int& output_img_width, int& output_img_height);

   bool text_line_vertical_separations(imagetext& curr_imagetext);
   void textline_pys(imagetext& curr_imagetext);
   bool text_line_horizontal_limits(imagetext& curr_imagetext);
   bool find_rendered_char_pixel_bboxes(imagetext& curr_imagetext);
   bool find_textlines_and_char_bboxes(imagetext& curr_imagetext);

// ==========================================================================
// Inlined methods
// ==========================================================================

}

#endif  // textfuncs.h


