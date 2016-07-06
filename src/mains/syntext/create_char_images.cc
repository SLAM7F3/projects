// ==========================================================================
// CREATE_CHAR_IMAGES imports a set of "reasonable" computer and
// handwriting fonts along with a set of "reasonable" background
// images.  It then loops over a large set of digits or letter
// characters.  The program first selects an image chip's width and
// aspect ratio according to exponential and gaussian distributions.  
// It then either extracts or synthesizes the chip's background.
// CREATE_CHAR_IMAGES subsequently picks text and stroke colors
// quasi-randomly.  The foreground colors are forced to be acceptable
// with the already-selected background color according to a
// pre-trained SVM decision rule.  The text character is then synthesized
// against black or white backgrounds.  

// The foreground character is next rotated in three-dimensions.  
// The UV image is initially oriented in the YZ world-plane.  It is
// rotated through az about the world z axis, el about the world
// y-axis and roll about the the world x-axis.  The az, el and roll
// angles are random variables selected from gaussian distributions
// with reasonable standard deviations to simulate camera views of
// text in the wild.  The rotation's azimuth, elevation and
// roll angles are capped at reasonable upper limits.  
// The rotated synthetic character image is subsequently projected
// back into the YZ world-plane.

// We next require that some sizable fraction (60%) of the text
// character's foreground content actually appear inside the rotated
// image chip.  If the foreground fraction is less than 25%, the image
// chip's classification label is reset to "ambiguous".  Otherwise,
// the candidate rotated image chip is rejected.  We also
// intentionally generate non-text image chips with frequency
// comparable to 1/n_classes.

// The text character's foreground is superposed onto nontrivial
// backgrounds.  The superposed image is compared with background.  If
// they do not significantly differ, the candidate text image chip is
// rejected.

// Surviving text image chips are intentionally corrupted.  Some small
// fraction are given simulated solar shadows.  Random gaussian noise
// is also added to all image chips.  Random bluring is also performed
// on some fraction of the image chips.

// Finally, padded versions of the text and non-text image chips 
// are exported for caffe classification/segmentation input purposes.

//			 ./create_char_images

// ==========================================================================
// Last updated on 2/16/16; 3/5/16; 3/7/16; 3/14/16
// ==========================================================================

//   c='0';	// ascii = 48
//   c='9';	// ascii = 57

//   c='A';	// ascii = 65
//   c='Z';	// ascii = 90

//   c='a';	// ascii = 97
//   c='z';	// ascii = 122

#include <iostream>
#include <Magick++.h>
#include <set>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "text/textfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   nrfunc::init_time_based_seed();

//   bool generate_segmentation_masks_flag = true;
   bool generate_segmentation_masks_flag = false;
//   bool use_only_handwriting_fonts = true;
   bool use_only_handwriting_fonts = false;
   bool export_only_ambiguous_chips = false;
//   bool export_only_ambiguous_chips = true;
   bool randomize_gravity_flag = true;
//   bool randomize_gravity_flag = false;
   cout << "generate_segmentation_masks_flag = "
        << generate_segmentation_masks_flag << endl;
//   cout << "use_only_handwriting_fonts = "
//        << use_only_handwriting_fonts << endl;
   cout << "export_only_ambiguous_chips = "
        << export_only_ambiguous_chips << endl;
//   cout << "randomize_gravity_flag = "
//        << randomize_gravity_flag << endl;



   char char_type;
   string synthetic_char_set_str="";
   vector<string> cleaned_chars;

   textfunc::generate_character_set(
      char_type,synthetic_char_set_str,cleaned_chars);

// Import a variety of ttf files for fonts which we have personally
// confirmed as "reasonable":

   string fonts_subdir="./fonts/reasonable_fonts/";
   string handwriting_fonts_subdir=fonts_subdir+"handwriting_fonts/";
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("ttf");
   allowed_suffixes.push_back("otf");
   bool search_all_children_dirs_flag = true;
   vector<string> font_paths=filefunc::
      files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,fonts_subdir,search_all_children_dirs_flag);

   int n_classes;
   string synthetic_subdir;
   if (char_type=='n')
   {
      synthetic_subdir="./training_data/synthetic_numbers";
      n_classes = 10;
   }
   else
   {
      synthetic_subdir="./training_data/synthetic_letters";
      n_classes = 26;
   }
   
   int synthetic_ID = -1;
   cout << "Enter ID for synthetic character set (0=default):" << endl;
   cin >> synthetic_ID;
   if(synthetic_ID > 0)
   {
      synthetic_subdir += stringfunc::number_to_string(synthetic_ID);
   }
   filefunc::add_trailing_dir_slash(synthetic_subdir);
   filefunc::dircreate(synthetic_subdir);

// Destroy and recreate char_images subdirectory holding initial image
// chips:

   string char_pics_subdir="./char_images";
   if(synthetic_ID > 0)
   {
     char_pics_subdir += stringfunc::number_to_string(synthetic_ID);
   }
   filefunc::add_trailing_dir_slash(char_pics_subdir);
   string unix_cmd = "/bin/rm -r -f "+char_pics_subdir;
   sysfunc::unix_command(unix_cmd);
   filefunc::dircreate(char_pics_subdir);

   string deeplab_inputs_subdir=synthetic_subdir+"deeplab_inputs/";
   if(generate_segmentation_masks_flag){
     filefunc::dircreate(deeplab_inputs_subdir);
   }

   string synthetic_subsubdir, extended_synthetic_subsubdir;
   string masks_subsubdir, extended_masks_subsubdir;

   cout << "Total number of characters which can be processed = " 
        << cleaned_chars.size() << endl;
   unsigned int i_start=0;
   cout << "Enter index for starting character to process:" << endl;
   cin >> i_start;

   unsigned int i_stop = cleaned_chars.size();
   cout << "Enter index for stopping character to process:" << endl;
   cin >> i_stop;

   unsigned int synthetic_char_counter = i_start;

// Initialize non-symbol backgrounds:

   vector<string> nonsymbol_filenames;   
   vector<int> nonsymbol_filename_frequencies;
   textfunc::retrieve_nonsymbol_background_filenames(
      nonsymbol_filenames, nonsymbol_filename_frequencies);
   cout << "nonsymbol_filenames.size = " 
        << nonsymbol_filenames.size() << endl;

   vector<texture_rectangle*> random_texture_rectangle_ptrs;
   textfunc::import_all_background_images(
      nonsymbol_filenames, random_texture_rectangle_ptrs);
   cout << "Random_texture_rectangle_ptrs.size() = "
        << random_texture_rectangle_ptrs.size() << endl;

   textfunc::import_foreground_background_colors_decision_function();

// Store pairs of tile and corresponding mask filenames within an STL
// map:

   typedef pair<string, string> STRING_PAIR;
   typedef std::map<int, vector<STRING_PAIR> > IMAGE_TILES_MAP;
// independent int: image ID
// dependent vector<string,string > : pairs of tile and mask filenames associated with image

   IMAGE_TILES_MAP image_tiles_map;
   IMAGE_TILES_MAP::iterator image_tiles_map_iter;
   
// ---------------------------------------------------------------------------
// Main loop over individual characters starts here:

   timefunc::initialize_timeofday_clock();

   string banner="Generating synthetic text";
   outputfunc::write_banner(banner);

   texture_rectangle* texture_rectangle_ptr = NULL;
   texture_rectangle* rotated_texture_rectangle_ptr = NULL;
   texture_rectangle* background_texture_rectangle_ptr = NULL;
   texture_rectangle* mask_texture_rectangle_ptr = NULL;
   texture_rectangle* big_texture_rectangle_ptr = NULL;

   int counter = 0;
   unsigned int i = i_start;
   while(i < i_stop)
   {
      counter++;

// Initially assume that the current image chip will contain a
// genuine text character:

      bool nontext_image_chip_flag = false;

      delete texture_rectangle_ptr;
      delete rotated_texture_rectangle_ptr;
      delete background_texture_rectangle_ptr;
      delete mask_texture_rectangle_ptr;
      delete big_texture_rectangle_ptr;

      texture_rectangle_ptr = NULL;
      rotated_texture_rectangle_ptr = NULL;
      background_texture_rectangle_ptr = NULL;
      mask_texture_rectangle_ptr = NULL;
      big_texture_rectangle_ptr = NULL;
      
      if ((i-i_start)%100 == 0)
      {
         cout << "Processing i = " << i << " i_start = " << i_start 
              << " i_stop = " << i_stop << endl;
         double progress_frac = double(i - i_start)/(i_stop-i_start);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      string text_label = cleaned_chars[counter%(i_stop - i_start)];
//      cout << "Text_label = " << text_label << endl;
      string curr_font_path=font_paths[counter%font_paths.size()];
      string font_subdir=filefunc::getdirname(curr_font_path);
      bool handwriting_font_flag = false;
      if(font_subdir==handwriting_fonts_subdir)
      {
         handwriting_font_flag = true;
      }

      if(use_only_handwriting_fonts && !handwriting_font_flag) continue;

// STEP 1: Select current image chip's pixel width and pixel height:

//      unsigned int width = 32;
//      unsigned int height = 54;

// On 1/28/16, we used program SVHN_PIXEL_SIZES to empirically compute
// distributions for SVHN image chip pixel widths and aspect ratios.
// The values below are based upon the results from this program.

// To good approximation, pixel width for SVHN image chips follows an
// exponetial distribution:

      unsigned int width_lo = 27;
      unsigned int width_hi = 80;
      double lambda = 0.111556;
      if(handwriting_font_flag)
      {
         width_hi = 50;
         lambda *= 5;
      }

      unsigned int width = width_lo + nrfunc::expdev(lambda);
      width = basic_math::min(width_hi, width);

// To good approximation, aspect ratio for SVHN image chips follows a
// gaussian distribution:

      double aspect_ratio_lo = 0.35;
      double aspect_ratio_hi = 0.85;
      double aspect_ratio = 0.62 + 0.11 * nrfunc::gasdev();
      aspect_ratio = basic_math::max(aspect_ratio_lo, aspect_ratio);
      aspect_ratio = basic_math::min(aspect_ratio_hi, aspect_ratio);
      unsigned int height = width / aspect_ratio;
//      cout << "width = " << width << " height = " << height << endl;

// STEP 2: Extract or synthesize background for current image chip:

      texture_rectangle* background_texture_rectangle_ptr = 
        new texture_rectangle(width, height, 1, 3, NULL);
      background_texture_rectangle_ptr->initialize_general_image(width,height);

      double background_frac = nrfunc::ran1();
      double nonsynthetic_background_frac = 0.33;
      if(handwriting_font_flag)
      {
         nonsynthetic_background_frac = 0.1;
      }

      double classification_score_threshold = 0.99;
      colorfunc::RGB background_RGB;
      colorfunc::HSV background_HSV;

// Parameters for exponential saturation density and gaussian value
// density distributions were derived via fits to SVHN pixel color
// data on 2/1/16:

      double lambda_s = 4.6399;
      double mu_v = 0.485;
      double sigma_v = 0.193;

      if(background_frac < nonsynthetic_background_frac)
      {
         int rx_start, ry_start;
         classification_score_threshold = 0.999;

         texture_rectangle* curr_random_texture_rectangle_ptr = 
            textfunc::initialize_random_image(
               nonsymbol_filenames, nonsymbol_filename_frequencies,
               random_texture_rectangle_ptrs,
               width, height, rx_start, ry_start);

         background_HSV = textfunc::peak_random_patch_hsv(
            rx_start, ry_start, width, height,
            curr_random_texture_rectangle_ptr);
         background_RGB = colorfunc::hsv_to_RGB(background_HSV, false);

         textfunc::extract_background_patch(
            rx_start, ry_start, curr_random_texture_rectangle_ptr, 
            background_texture_rectangle_ptr);
      }
      else
      {

// Intentionally skew background color to shades of grey for
// synthetic handwriting characters:

         if(handwriting_font_flag)
         {
            background_RGB = colorfunc::generate_random_RGB(
               false, 0 ,360, 0, 0.2, 0, 1);
         }
         else
         {
            background_RGB = colorfunc::generate_random_RGB(
               false, lambda_s, mu_v, sigma_v);
         }

         textfunc::synthesize_background_patch(
            background_texture_rectangle_ptr, background_RGB);
         background_HSV = colorfunc::RGB_to_hsv(background_RGB, false);

         if(!handwriting_font_flag ||
            (handwriting_font_flag && nrfunc::ran1() < 0.2))
         {
            textfunc::add_random_background_lines(
               background_texture_rectangle_ptr, background_HSV,
               lambda_s, mu_v, sigma_v);
         }
      }

//      string background_filename=
//         "background_"+stringfunc::integer_to_string(i,2)+".jpg";
//      background_texture_rectangle_ptr->write_curr_frame(
//         background_filename);
//      cout << "Background filename = " << background_filename << endl;

// STEP 3: Randomly pick text and stroke colors.  Make sure they are
// acceptable with already-selected background color according to a
// pre-trained SVM decision rule:

      colorfunc::RGB foreground_RGB, stroke_RGB;

// Only add stroke to no more than 20% of all synthetic characters:

      double strokewidth = -1;
      if(nrfunc::ran1() < 0.2)
      {
         strokewidth = basic_math::min(5*nrfunc::ran1(), 5.0);
         textfunc::generate_acceptable_foreground_color(
            lambda_s, mu_v, sigma_v, classification_score_threshold, 
            stroke_RGB, background_RGB);

         if(nrfunc::ran1() < 0.25)
         {
            foreground_RGB = background_RGB;
         }
         else
         {
            foreground_RGB = colorfunc::generate_random_RGB(
               false, lambda_s, mu_v, sigma_v);
         }
      }
      else
      {
         if(handwriting_font_flag)
         {
            lambda_s *= 5;
         }
         
         textfunc::generate_acceptable_foreground_color(
            lambda_s, mu_v, sigma_v, classification_score_threshold, 
            foreground_RGB, background_RGB);
      }
      
      double size_frac = double(width) / 64.0;
//      int point_size = 50 * size_frac; 
//      int point_size = 100 * size_frac;  // use for 32x54 image chips
      int point_size = (85 + nrfunc::ran1()*30) * size_frac; 
//      cout << "point_size = " << point_size << endl;
	       
// STEP 4: Synthesize text character against black or white
// backgrounds:

      colorfunc::RGB text_background_RGB;
      colorfunc::RGBA text_background_RGBA;

      if(background_HSV.third < 0.5)
      {
         text_background_RGB.first = 0;
         text_background_RGB.second = 0;
         text_background_RGB.third = 0;
         text_background_RGBA.first = 0;
         text_background_RGBA.second = 0;
         text_background_RGBA.third = 0;
         text_background_RGBA.fourth = 255;
      }
      else
      {
         text_background_RGB.first = 255;
         text_background_RGB.second = 255;
         text_background_RGB.third = 255;
         text_background_RGBA.first = 255;
         text_background_RGBA.second = 255;
         text_background_RGBA.third = 255;
         text_background_RGBA.fourth = 255;
      }

      string foreground_char_filename=char_pics_subdir+"char_"+
         stringfunc::integer_to_string(i,6)+".jpg";
      imagefunc::generate_text_via_ImageMagick(
         randomize_gravity_flag, 
         foreground_RGB, stroke_RGB, 
         text_background_RGBA,
         strokewidth,curr_font_path,point_size,width,height,
         text_label,foreground_char_filename);

// Note added on 1/31/16: We should find a way to transfer text
// generated via ImageMagick in memory directly into
// texture_rectangle_ptr without having to write out to disk!

      texture_rectangle* texture_rectangle_ptr = new texture_rectangle();
      bool image_imported_flag = texture_rectangle_ptr->import_photo_from_file(
         foreground_char_filename);
      if(!image_imported_flag)
      {
         cout << "Failed to import foreground char for image i = " 
              << i << endl;
         continue;
      }

// Randomly add colored box underneath text characters for some small
// fraction of synthetic image chips:

      if(background_frac > nonsynthetic_background_frac &&
         !generate_segmentation_masks_flag)
      {
         double colored_box_threshold = 0.2;
         if(handwriting_font_flag)
         {
            colored_box_threshold = 0.05;
         }
         if(nrfunc::ran1() < colored_box_threshold)
         {
            textfunc::draw_bbox_under_text_char(
               text_background_RGB, foreground_RGB, stroke_RGB, strokewidth,
               texture_rectangle_ptr);
         }
      }
      
//      string orig_char_filename=
//         "orig_char_"+stringfunc::integer_to_string(i,2)+".jpg";
//      texture_rectangle_ptr->write_curr_frame(orig_char_filename);
//      cout << "orig char filename = " << orig_char_filename << endl;

// STEP 5:  Rotate synthetic text character in three-dimensions:

      texture_rectangle* rotated_texture_rectangle_ptr = 
        new texture_rectangle(width, height, 1, 3, NULL);
      rotated_texture_rectangle_ptr->initialize_general_image(width, height);

      double max_az = 30;
      double max_el = 30;
      double max_roll = 7.5;
      if(handwriting_font_flag)
      {
         max_az = max_el = max_roll = 5;
      }
      
      textfunc::randomly_rotate_foreground_symbol(
         max_az, max_el, max_roll,
         text_background_RGB, texture_rectangle_ptr,
         rotated_texture_rectangle_ptr);

//      string rotated_char_filename=
//         "rotated_char_"+stringfunc::integer_to_string(i,2)+".jpg";
//      rotated_texture_rectangle_ptr->write_curr_frame(rotated_char_filename);
//      cout << "Rotated char filename = " << rotated_char_filename
//           << endl;

// STEP 6: Require some sizable fraction of text character's
// foreground content to actually appear inside rotated image chip.
// If not, reset image chip's classification label to "non":

      int n_all_foreground_pixels, n_rotated_foreground_pixels, 
         n_background_pixels, n_all_background_pixels;

      textfunc::count_foreground_background_pixels(
         rotated_texture_rectangle_ptr, text_background_RGB,
         n_rotated_foreground_pixels, n_background_pixels);

      string big_foreground_char_filename=char_pics_subdir+"big_char_"+
         stringfunc::integer_to_string(i,6)+".jpg";
      imagefunc::generate_text_via_ImageMagick(
         false, 
         foreground_RGB, stroke_RGB, 
         text_background_RGBA,
         strokewidth,curr_font_path,
         point_size,3*width,3*height,
         text_label,big_foreground_char_filename);

      big_texture_rectangle_ptr = new texture_rectangle();
      image_imported_flag = big_texture_rectangle_ptr->import_photo_from_file(
         big_foreground_char_filename);
      if(!image_imported_flag)
      {
         cout << "Failed to import big foreground char for image i = " 
              << i << endl;
         continue;
      }

      textfunc::count_foreground_background_pixels(
         big_texture_rectangle_ptr, text_background_RGB,
         n_all_foreground_pixels, n_all_background_pixels);

      if(n_all_foreground_pixels < 10)
      {
         cout << "counter = " << counter << " i = " << i << endl;
         cout << " text_label = " << text_label << endl;
         cout << "n_all_foreground_pixels = " << n_all_foreground_pixels 
              << endl;
         if(export_only_ambiguous_chips) continue;
         nontext_image_chip_flag = true;
      }

      double foreground_pixel_ratio = double(n_rotated_foreground_pixels)/
         double(n_all_foreground_pixels);
//      cout << "foreground_pixel_ratio = " << foreground_pixel_ratio << endl;

      if(export_only_ambiguous_chips && foreground_pixel_ratio > 0.25) 
         continue;

      if(foreground_pixel_ratio < 0.60)
      {
//         cout << "counter = " << counter << " i = " << i << endl;
//         cout << "  text_label = " << text_label << endl;
//         cout << "  font_path = " << curr_font_path << endl;
//         cout << "  width = " << width << " height = " << height << endl;
//         cout << "  n_rotated_foreground_pixels = " 
//              << n_rotated_foreground_pixels
//              << "  n_all_foreground_pixels = "
//              << n_all_foreground_pixels << endl;
//         cout << "  foreground pixel ratio = " << foreground_pixel_ratio 
//              << endl;
         if(foreground_pixel_ratio < 0.25 && foreground_pixel_ratio > 0.05)
         {
            text_label = "ambiguous";
//            cout << "counter = " << counter << " i = " << i << endl;
//            cout << "  text_label = " << text_label << endl;
//            cout << "  font_path = " << curr_font_path << endl;
//            cout << "  width = " << width << " height = " << height << endl;
//            cout << "  n_rotated_foreground_pixels = " 
//                 << n_rotated_foreground_pixels
//                 << "  n_all_foreground_pixels = "
//                 << n_all_foreground_pixels << endl;
//            cout << "  foreground pixel ratio = " << foreground_pixel_ratio 
//                 << endl;
         }
         else
         {
            continue;
         }
      }

// STEP 7a: Intentionally generate non-text image chips with frequency
// comparable to 1/n_classes:

      if(!export_only_ambiguous_chips && nrfunc::ran1() < 1.0 / n_classes) 
        nontext_image_chip_flag = true;

      if(generate_segmentation_masks_flag)
      {
         mask_texture_rectangle_ptr = 
            new texture_rectangle(width, height, 1, 3, NULL);
         mask_texture_rectangle_ptr->initialize_general_image(width,height);
      }
      
      double delta_frac = 0;
      if(nontext_image_chip_flag)
      {
         textfunc::copy_background_patch(
            background_texture_rectangle_ptr, rotated_texture_rectangle_ptr);
         text_label = "non";

         if(generate_segmentation_masks_flag)
            mask_texture_rectangle_ptr->clear_all_RGB_values();
      }
      else
      {

// ... or ... STEP 7b:  Superpose text foreground onto nontrivial background.
// Then compare superposed image with background.  If they do not
// significantly differ, reject candidate text image chip:

         if(generate_segmentation_masks_flag)
         {
            textfunc::generate_segmentation_mask(
               text_background_RGBA, rotated_texture_rectangle_ptr,
               background_texture_rectangle_ptr, mask_texture_rectangle_ptr);
         }

         delta_frac = 
            textfunc::superpose_foreground_on_background_patch(
               text_background_RGBA, background_texture_rectangle_ptr, 
               rotated_texture_rectangle_ptr);
         if (delta_frac <= 8.0)
         {
            continue;
         }
      }

//      string superposed_char_filename="superposed_char_"+
//         stringfunc::integer_to_string(i,2)+"_"+
//         stringfunc::number_to_string(delta_frac,5)+"_.jpg";
//      rotated_texture_rectangle_ptr->write_curr_frame(
//         superposed_char_filename);
//      cout << "Superposed char filename = " << superposed_char_filename
//           << endl;

// STEP 8:  Intentionally corrupt superposed image:

// Simulate solar shadows in small fraction of image chips:

      double solar_shadow_threshold = 0.15;
      if(handwriting_font_flag)
      {
         solar_shadow_threshold = 0.02;
      }
      
      if(nrfunc::ran1() < solar_shadow_threshold){
        textfunc::synthesize_solar_shadow(rotated_texture_rectangle_ptr);
      }

// Add noise to entire image chip:

      textfunc::add_gaussian_noise(rotated_texture_rectangle_ptr);
         
      Magick::Image superposed_image(
         rotated_texture_rectangle_ptr->getWidth(),
         rotated_texture_rectangle_ptr->getHeight(), 
         "RGB", MagickCore::CharPixel,
         rotated_texture_rectangle_ptr->get_image_ptr()->data());

// Randomly blur entire image chip:

      double sigma=textfunc::random_gaussian_blur_sigma(
         width_lo, width_hi, width);
      if(handwriting_font_flag) sigma *= 0.5;
      if (sigma > 0)
      {
         videofunc::gaussian_blur_image(superposed_image,sigma);
      } // sigma > 0 conditional

// STEP 9: Export 256x256 or 321x321 padded versions of text image
// chip:

// We don't want to export more than 100K image chips to any one
// subdirectory.  So create subsubdirectories to hold 100K chips:

      synthetic_subsubdir=synthetic_subdir+
         stringfunc::integer_to_string(synthetic_char_counter/100000,5)+"/";
      filefunc::dircreate(synthetic_subsubdir);
      extended_synthetic_subsubdir = synthetic_subsubdir+"extended_chips/";
      filefunc::dircreate(extended_synthetic_subsubdir);

      if(generate_segmentation_masks_flag)
      {
         masks_subsubdir = synthetic_subsubdir+"masks/";
         filefunc::dircreate(masks_subsubdir);
         extended_masks_subsubdir = masks_subsubdir+"extended_masks/";
         filefunc::dircreate(extended_masks_subsubdir);
      }
      
      string resized_superposed_filename=synthetic_subsubdir+
         text_label+"_";
      if(generate_segmentation_masks_flag)
      {
         resized_superposed_filename=synthetic_subsubdir+"char_";
      }
      resized_superposed_filename += stringfunc::integer_to_string(i,6)+".jpg";
      superposed_image.write(resized_superposed_filename);

      int padded_pixel_size = 256;
      if(generate_segmentation_masks_flag)
      {
         padded_pixel_size = 321;
      }
      
      imagefunc::pad_image(padded_pixel_size, padded_pixel_size, 
                           resized_superposed_filename,
                           extended_synthetic_subsubdir);
//      banner="Exported "+extended_synthetic_subsubdir+
//         filefunc::getbasename(resized_superposed_filename);
//      outputfunc::write_banner(banner);

      if(generate_segmentation_masks_flag)
      {
         Magick::Image mask_image(
            mask_texture_rectangle_ptr->getWidth(),
            mask_texture_rectangle_ptr->getHeight(), 
            "RGB", MagickCore::CharPixel,
            mask_texture_rectangle_ptr->get_image_ptr()->data());

         Magick::Image* padded_mask_ptr = imagefunc::pad_image(
            padded_pixel_size, padded_pixel_size, mask_image);

         string mask_filename=masks_subsubdir+"mask_"+
            stringfunc::integer_to_string(i,6)+".jpg";
         padded_mask_ptr->write(mask_filename);

         string bytearray_filename=extended_masks_subsubdir+"bytearray_"+
            stringfunc::integer_to_string(i,6)+".png";

//         bool visualize_mask_flag = true;
         bool visualize_mask_flag = false;
         vector<vector<unsigned char> > byte_array = 
            videofunc::generate_byte_array(
//               mask_texture_rectangle_ptr, NULL,
               NULL, padded_mask_ptr, 
               0, padded_pixel_size - 1, 0, padded_pixel_size - 1,
               visualize_mask_flag);
         delete padded_mask_ptr;
         
         videofunc::write_8bit_greyscale_pngfile(
            byte_array, bytearray_filename);

// Save correspondence between extended image tile and extended image
// mask to STL map:

         string reduced_tile_filename="/extended_chips/"+
            stringfunc::prefix(filefunc::getbasename(
                                  resized_superposed_filename))+"_"+
            stringfunc::number_to_string(padded_pixel_size)+"x"+
            stringfunc::number_to_string(padded_pixel_size)+".jpg";

         string reduced_mask_filename="/extended_masks/"+
            filefunc::getbasename(bytearray_filename);

         STRING_PAIR P;
         P.first = reduced_tile_filename;
         P.second = reduced_mask_filename;

         image_tiles_map_iter = image_tiles_map.find(i);
         if(image_tiles_map_iter == image_tiles_map.end()){
            vector<STRING_PAIR> V;
            V.push_back(P);
            image_tiles_map[i] = V;
         }
         else{
            vector<STRING_PAIR> *V_ptr = &image_tiles_map_iter->second;
            V_ptr->push_back(P);
         }
      } // generate_segmentation_masks_flag conditiaonl
      
      synthetic_char_counter++;
      i++;

   } // loop over index i labeling cleaned input characters
   cout << endl;

   if(generate_segmentation_masks_flag)
   {
      cout << "image_tiles_map.size() = " << image_tiles_map.size() << endl;

// Take first 80% of randomized image IDs for training and last 20%
// for testing:

      vector<int> random_seq = mathfunc::random_sequence(
         i_start, i_stop, i_stop - i_start + 1);
      unsigned int n_images = i_stop - i_start + 1;

      const double training_frac = 0.8;
      unsigned int n_training = training_frac * n_images;
      cout << "n_training = " << n_training << endl;

      string images_masks_training_filename=deeplab_inputs_subdir+
         "images_masks_training.txt";
      ofstream outstream;
      filefunc::openfile(images_masks_training_filename, outstream);   

      for(unsigned int i = 0; i < n_training; i++)
      {
         int curr_image_ID = random_seq[i];
         image_tiles_map_iter = image_tiles_map.find(curr_image_ID);
         if(image_tiles_map_iter == image_tiles_map.end()) continue;
         vector<STRING_PAIR> *V_ptr = &image_tiles_map_iter->second;
         for(unsigned int j = 0; j < V_ptr->size(); j++)
         {
            STRING_PAIR P = V_ptr->at(j);
            outstream << P.first << " " << P.second << endl;
         }
      }
      filefunc::closefile(images_masks_training_filename, outstream);

      string images_masks_testing_filename=deeplab_inputs_subdir+
         "images_masks_testing.txt";
      filefunc::openfile(images_masks_testing_filename, outstream);   

      for(unsigned int i = n_training; i < n_images; i++)
      {
         int curr_image_ID = random_seq[i];
         image_tiles_map_iter = image_tiles_map.find(curr_image_ID);
         if(image_tiles_map_iter == image_tiles_map.end()) continue;
         vector<STRING_PAIR> *V_ptr = &image_tiles_map_iter->second;
         for(unsigned int j = 0; j < V_ptr->size(); j++)
         {
            STRING_PAIR P = V_ptr->at(j);
            outstream << P.first << " " << P.second << endl;
         }
      }
      filefunc::closefile(images_masks_testing_filename, outstream);

// Recall several authors stress that training and validation data
// sets should be randomized!  As John Wood pointed out, we can use
// the linux utility "shuf" to generate random permutations:

      string shuffled_images_masks_training_filename=deeplab_inputs_subdir+
         "shuffled_images_masks_training.txt";
      unix_cmd = "shuf "+images_masks_training_filename+" > "+
         shuffled_images_masks_training_filename;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      string shuffled_images_masks_testing_filename=deeplab_inputs_subdir+
         "shuffled_images_masks_testing.txt";
      unix_cmd = "shuf "+images_masks_testing_filename+" > "+
         shuffled_images_masks_testing_filename;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

// Move folders containing extended chips and masks into
// deeplab_inputs_subdir:

      unix_cmd="mv "+extended_synthetic_subsubdir+" "+deeplab_inputs_subdir;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
      
      unix_cmd="mv "+extended_masks_subsubdir+" "+deeplab_inputs_subdir;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

// Create object names file containing background and text classes:

      string object_names_filename=deeplab_inputs_subdir+
         "object_names.classes";
      filefunc::openfile(object_names_filename, outstream);
      outstream << "__background__" << endl;
      outstream << "text" << endl;
      filefunc::closefile(object_names_filename, outstream);
    
      banner="Exported deeplab inputs to "+deeplab_inputs_subdir;
      outputfunc::write_big_banner(banner);
   } // generate_segmentation_masks_flag conditional
   

   for (unsigned int r=0; r<random_texture_rectangle_ptrs.size(); r++)
   {
      delete random_texture_rectangle_ptrs[r];
   }
} 


