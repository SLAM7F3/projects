// ==========================================================================
// GENERATE_STRING_IMAGES imports a set of "reasonable" computer and
// handwriting fonts along with a set of background internet images
// which should contain no text content.  Looping over a large set of
// text phrases, it first randomly selects a particular text label
// along with a font.  RGB colors for the text foreground and possible
// stroke are also randomly chosen.  If an underbox for the text is
// randomly selected, its background color is required to be
// reasonably different from the text foregorund's color.  Otherwise,
// the foreground text is rendered against a transparent background.

// The overall width and height for the entire text phrase is next
// quasi-randomly selected so that a fair number of strings will be
// rendered onto two or more text lines.  To good approximation,
// character height is linearly related to font pointsize.  All
// ImageMagick rendering calls are passed the text image width along
// with font pointsize.  

// After a text phrase is rendered via ImageMagick,
// GENERATE_STRING_IMAGES computes reasonable estimates for all the
// rendered characters' widths.  A very wide image is rendered in
// which the entire string is essentially guaranteed to lie within a
// single textline.  Once rendered character widths are known, pixel
// bounding boxes around each character are derived.

// The synthetic text image is next intentionally corrupted via the
// addition of random solar shadows and foreground occlusions.  The
// text chip is exported to an output subdirectory.  Character and
// word masks are also generated and exported to output folders.

// Finally, GENERATE_STRING_IMAGES rotates the text image chip
// along with its masks by some random az, el and roll angles.  For
// each image, it instantiates a virtual camera whose horizontal FOV
// and aspect ratio are random gaussian variables.  The string image
// and mask are also rotated in 3 dimensions according to random az,
// el and roll gaussian variables.  After perspectively projecting the
// string image and mask into the virtual camera, ImageMagick is used
// to generate and export their 2D imageplane renderings.  The rotated
// & projected images and masks are exported to separate folders.  If
// any character or word mask value lies outside their valid ranges,
// then the rotated image and both masks are discarded.

// Multiple instances of GENERATE_STRING_IMAGES can be run in parallel
// on a single CPU machine.

//			 ./generate_string_images

// ==========================================================================
// Last updated on 4/20/16; 4/22/16; 8/27/16; 8/29/16
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
#include "text/imagetext.h"
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
using std::set;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   nrfunc::init_time_based_seed();

   bool generate_just_digits_flag = true;
   vector<string> cleaned_strings;
   if(generate_just_digits_flag)
   {
      int n_digits = 220000;
      for(int n = 0; n < n_digits; n++)
      {
         int curr_digit = n%10;
         cleaned_strings.push_back(stringfunc::number_to_string(curr_digit));
      }
   }
   else
   {
      textfunc::generate_phrase_set(cleaned_strings);
   }

// Import a variety of ttf files for fonts which we have personally
// confirmed as "reasonable":

   string fonts_subdir="./fonts/reasonable_fonts/";
   string handwriting_fonts_subdir=fonts_subdir+"handwriting_fonts/";
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("ttf");
   allowed_suffixes.push_back("TTF");
   allowed_suffixes.push_back("otf");
   bool search_all_children_dirs_flag = true;
   vector<string> font_paths=filefunc::
      files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,fonts_subdir,search_all_children_dirs_flag);

   int syn_words_ID = -1;
   cout << "Enter synthetic words ID (-1 for default):" << endl;
   cin >> syn_words_ID;
   string synthetic_subdir="./training_data/synthetic_words";
   if(syn_words_ID >= 0)
   {
      synthetic_subdir += "_"+stringfunc::integer_to_string(syn_words_ID,2);
   }
   filefunc::add_trailing_dir_slash(synthetic_subdir);
   filefunc::dircreate(synthetic_subdir);

   string synthetic_subsubdir, word_masks_subsubdir, char_masks_subsubdir;

   string rotated_images_subdir=synthetic_subdir+"rotated_images/";
   filefunc::dircreate(rotated_images_subdir);
   string rotated_char_masks_subdir=synthetic_subdir+"rotated_char_masks/";
   filefunc::dircreate(rotated_char_masks_subdir);
   string rotated_word_masks_subdir=synthetic_subdir+"rotated_word_masks/";
   filefunc::dircreate(rotated_word_masks_subdir);

   int n_cleaned_strings = cleaned_strings.size();
   cout << "Total number of strings which can be processed = " 
        << n_cleaned_strings << endl;
   unsigned int i_start=0;
   cout << "Enter index for starting string to process:" << endl;
   cin >> i_start;

   unsigned int i_stop = n_cleaned_strings;
   cout << "Enter index for stopping string to process:" << endl;
   cin >> i_stop;

   textfunc::import_foreground_background_colors_decision_function();

   string output_filename=synthetic_subdir+"syntext_"+
      stringfunc::number_to_string(i_start)+".txt";
   ofstream outstream;
   filefunc::openfile(output_filename, outstream);
   
// ---------------------------------------------------------------------------
// Main loop over individual strings starts here:

   timefunc::initialize_timeofday_clock();

   string banner="Generating synthetic text";
   outputfunc::write_banner(banner);

   texture_rectangle* composite_texture_rectangle_ptr = NULL;
   texture_rectangle* charmask_texture_rectangle_ptr = NULL;
   texture_rectangle* wordmask_texture_rectangle_ptr = NULL;
   imagetext curr_imagetext;
   curr_imagetext.set_max_n_chars(200);
   curr_imagetext.set_randomize_gravity_flag(false);

   unsigned int i = i_start;
   unsigned int synthetic_string_counter = i_start;
   curr_imagetext.set_ID(i_start);

   while(i < i_stop)
   {
      delete composite_texture_rectangle_ptr;
      delete charmask_texture_rectangle_ptr;
      delete wordmask_texture_rectangle_ptr;

      composite_texture_rectangle_ptr = NULL;
      charmask_texture_rectangle_ptr = NULL;
      wordmask_texture_rectangle_ptr = NULL;

      if ((i-i_start)%10 == 0)
      {
         cout << "Processing i = " << i << " i_start = " << i_start 
              << " i_stop = " << i_stop << endl;
         cout << " Current time = " << timefunc::getcurrdate() << endl;
         double progress_frac = double(i - i_start)/(i_stop-i_start);
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

// STEP 1. Select text label and font:

      cout << "Steps: 1" << flush;

      if(generate_just_digits_flag)
      {
         curr_imagetext.set_phrase(
            cleaned_strings[nrfunc::ran1() * cleaned_strings.size()] );
      }
      else
      {
         int min_n_words = 1;
         int max_n_words = 15;
//      int max_n_words = 20;
         textfunc::select_text_phrase(
            cleaned_strings, min_n_words, max_n_words, curr_imagetext);

//      int n_words = curr_imagetext.get_n_words();
//      cout << "min_n_words = " << min_n_words 
//           << " n_words = " << n_words << endl;
//      cout << "phrase = " << curr_imagetext.get_phrase() << endl;
      }

// Select current font:

//      string curr_font_path=font_paths[synthetic_string_counter];
      string curr_font_path=font_paths[nrfunc::ran1()*font_paths.size()];
      curr_imagetext.set_font(curr_font_path);
      string font_subdir=filefunc::getdirname(curr_font_path);
      bool handwriting_font_flag = false;
      if(font_subdir==handwriting_fonts_subdir)
      {
         handwriting_font_flag = true;
      }

// STEP 2. Select foreground, stroke and underbox colors:

      cout << " 2" << flush;

      colorfunc::RGB foreground_RGB = colorfunc::generate_random_RGB(false);
      curr_imagetext.set_foreground_RGB(foreground_RGB);
      curr_imagetext.set_background_RGBA(0,0,0,0);

// Add stroke to 25% of all synthetic text images:

      double strokewidth = -1;
      if(nrfunc::ran1() < 0.25)
      {
         strokewidth = basic_math::min(5*nrfunc::ran1(), 5.0);
         curr_imagetext.set_stroke_RGB(
            colorfunc::generate_random_RGB(false));
      }
      curr_imagetext.set_strokewidth(strokewidth);

// Add colored underboxes to 20% of all synthetic text images:

      curr_imagetext.set_underbox_flag(false);

      if(nrfunc::ran1() < 0.20)
      {
         curr_imagetext.set_underbox_flag(true);

// Randomly pick underbox color which is acceptably different from
// foreground color:

         bool OK_underbox_color = false;
         int iter = 0, iter_max = 20;
         double classification_score_threshold = 0.99;
         colorfunc::RGB underbox_RGB;
         while(iter < iter_max && !OK_underbox_color)
         {
            underbox_RGB = colorfunc::generate_random_RGB(false);
            OK_underbox_color = 
               textfunc::test_foreground_background_color_acceptability(
                  classification_score_threshold,foreground_RGB,underbox_RGB);
         }
         curr_imagetext.set_undercolor_RGB(underbox_RGB);
      }

// STEP 3. Select generic character and text label widths.  Then
// compute individual character widths:

      cout << " 3" << flush;

      unsigned int char_width_lo = 5;
      unsigned int char_width_hi = 100;
      if(generate_just_digits_flag)
      {
         char_width_lo = 50;
         char_width_hi = 96;
      }
      double lambda = 0.09;

      if(handwriting_font_flag)
      {
         char_width_hi = 50;
         lambda *= 5;
      }

      unsigned int character_width =char_width_lo + nrfunc::expdev(lambda);
      character_width = basic_math::min(char_width_hi, character_width);
      unsigned int character_height = 1.5 * character_width;

// On 3/26/16, we plotted O(50) sets of character height vs point size
// pairs.  To good approximation (chi-by-eye!), the following linear
// relation holds:

      int point_size = 0.797 * character_height + 3.35;
      curr_imagetext.set_pointsize(point_size);

// Specify text label's width in pixels.  Then generate dummy image
// chip:

      int max_n_chars_per_line = 21;
      unsigned int text_label_width = basic_math::min(
         curr_imagetext.get_n_characters(),max_n_chars_per_line) 
         * character_width;
      unsigned int max_text_label_width = 3 * 321;
      text_label_width = basic_math::min(
         max_text_label_width, text_label_width);
      int estimated_n_textlines = 1 + curr_imagetext.get_n_characters() 
         / max_n_chars_per_line;
      curr_imagetext.set_image_width(text_label_width);

//      cout << "character_width = " << character_width
//           << " text_label_width = " << text_label_width << endl;
//      cout << "character_height = " << character_height << endl;
 //     cout << "point_size = " << point_size << endl;
//      cout << "estimated_n_textlines = " << estimated_n_textlines << endl;

      string dummy_chip_filename="dummy_chip_"+stringfunc::number_to_string(
         i_start)+".png";
      curr_imagetext.set_text_image_filename(dummy_chip_filename);

      imagefunc::generate_text_via_ImageMagick(
         curr_imagetext.get_debug_annotate_flag(),
         curr_imagetext.get_randomize_gravity_flag(),
         curr_imagetext.get_foreground_RGB(), 
         curr_imagetext.get_stroke_RGB(), 
         curr_imagetext.get_background_RGBA(),
         curr_imagetext.get_underbox_flag(),
         curr_imagetext.get_undercolor_RGB(), 
         curr_imagetext.get_strokewidth(),
         curr_imagetext.get_font(),
         curr_imagetext.get_pointsize(), 
         curr_imagetext.get_image_width(), 
         -1,
         curr_imagetext.get_phrase(), 
         curr_imagetext.get_text_image_filename(),
         curr_imagetext.get_drop_shadow_flag());

      unsigned int text_label_height;
      if(!imagefunc::get_image_width_height(
            dummy_chip_filename, text_label_width, text_label_height))
      {
         cout << "get_image_width_height() failed" << endl;
         continue;
      }
//      cout << "text_label_width = " << text_label_width
//           << " text_label_height = " << text_label_height << endl;

// STEP 4.  Compute reasonable estimates for all the text_label
// characters' widths by forming a very wide image in which the string
// is essentially guaranteed to have no line break:

      cout << " 4" << flush;

// Generate very wide string image to make sure it contains exactly 
// one line of text:

      int wide_width = (estimated_n_textlines+2)*
         curr_imagetext.get_image_width();
      if(!textfunc::extract_char_widths_and_height(
            wide_width,-1,curr_imagetext)) 
      {
         cout << "textfunc::extract_char_widths_and_height() failed" << endl;
         continue;
      }
      
      curr_imagetext.set_image_height(text_label_height);

// STEP 5. Locate bounding boxes around all rendered characters
// within synthetic text image:

      cout << " 5" << flush;
      if(!textfunc::find_textlines_and_char_bboxes(curr_imagetext)) 
      {
         cout << "textfunc::find_textlines_and_char_bboxes() failed" << endl;
         continue;
      }
      
//      cout << curr_imagetext << endl;

// STEP 6: Add 3D effects (e.g. drop shadows, character shine) to a
// nontrivial fraction of text images that do NOT contain colored
// underboxes:

      cout << " 6" << flush;

      double ran_3D = nrfunc::ran1();
      if(!curr_imagetext.get_underbox_flag() && ran_3D < 0.05)
      {
         curr_imagetext.set_drop_shadow_flag(true);
         imagefunc::generate_text_via_ImageMagick(
            curr_imagetext.get_debug_annotate_flag(),
            curr_imagetext.get_randomize_gravity_flag(),
            curr_imagetext.get_foreground_RGB(), 
            curr_imagetext.get_stroke_RGB(), 
            curr_imagetext.get_background_RGBA(),
            curr_imagetext.get_underbox_flag(),
            curr_imagetext.get_undercolor_RGB(), 
            curr_imagetext.get_strokewidth(),
            curr_imagetext.get_font(),
            curr_imagetext.get_pointsize(), 
            curr_imagetext.get_image_width(), 
            -1,
            curr_imagetext.get_phrase(), 
            curr_imagetext.get_text_image_filename(),
            curr_imagetext.get_drop_shadow_flag());
         curr_imagetext.set_drop_shadow_flag(false);
      }
      else if(!curr_imagetext.get_underbox_flag() && ran_3D < 0.35)
      {
         int shade_x = 10 + nrfunc::ran1() * 60;
         int shade_y = 10 + nrfunc::ran1() * 60;
         imagefunc::generate_ImageMagick_3D_variegated_text(
            curr_imagetext.get_text_image_filename(),
            curr_imagetext.get_text_image_filename(),
            shade_x, shade_y);
      }

// STEP 7.  Intentionally corrupt foreground synthetic image:

      cout << " 7" << flush;

      composite_texture_rectangle_ptr = new texture_rectangle();
      composite_texture_rectangle_ptr->import_photo_from_file(
         curr_imagetext.get_text_image_filename());

// Simulate solar shadows in small fraction of image chips which
// contain both foreground and background content:

      double solar_shadow_threshold = 0.20;
      if(handwriting_font_flag)
      {
         solar_shadow_threshold = 0.10;
      }
  
      if(nrfunc::ran1() < solar_shadow_threshold)
      {
         textfunc::synthesize_solar_shadow(composite_texture_rectangle_ptr);
      }

// Add vertical/horizontal rectangles or circular occlusions into a
// non-negligible fraction of synthetic image chips:

      if(nrfunc::ran1() < 0.15)
      {
         textfunc::add_foreground_occlusions(composite_texture_rectangle_ptr);
      }

      Magick::Image superposed_image(
         composite_texture_rectangle_ptr->getWidth(),
         composite_texture_rectangle_ptr->getHeight(), 
         "RGBA", MagickCore::CharPixel,
         composite_texture_rectangle_ptr->get_image_ptr()->data());
      superposed_image.depth(8);

// STEP 8. Export text foreground chip:

      cout << " 8" << flush;
      
// We don't want to export more than 100K image chips to any one
// subdirectory.  So create subsubdirectories to hold 100K chips:

      synthetic_subsubdir=synthetic_subdir+
         stringfunc::integer_to_string(synthetic_string_counter/100000,5)+"/";
      filefunc::dircreate(synthetic_subsubdir);
      word_masks_subsubdir = synthetic_subsubdir+"word_masks/";
      char_masks_subsubdir = synthetic_subsubdir+"char_masks/";
      filefunc::dircreate(word_masks_subsubdir);
      filefunc::dircreate(char_masks_subsubdir);

      string synthetic_chip_filename=synthetic_subsubdir+"string_"
         +stringfunc::integer_to_string(synthetic_string_counter,6);
      string charmask_filename=char_masks_subsubdir+"charmask_"
         +stringfunc::integer_to_string(synthetic_string_counter,6);
      string wordmask_filename=word_masks_subsubdir+"wordmask_"
         +stringfunc::integer_to_string(synthetic_string_counter,6);
      
      if(!curr_imagetext.get_underbox_flag())
      {
         synthetic_chip_filename += "_JustForeground";
         charmask_filename += "_JustForeground";
         wordmask_filename += "_JustForeground";
      }

      synthetic_chip_filename += ".png";
      charmask_filename += ".png";
      wordmask_filename += ".png";

      superposed_image.write(synthetic_chip_filename);

// STEP 9. Generate and export word/character mask chips:

      cout << " 9" << flush;

      charmask_texture_rectangle_ptr = new texture_rectangle(
         curr_imagetext.get_image_width(),
         curr_imagetext.get_image_height(), 1, 4, NULL);
      wordmask_texture_rectangle_ptr = new texture_rectangle(
         curr_imagetext.get_image_width(),
         curr_imagetext.get_image_height(), 1, 4, NULL);

      curr_imagetext.set_mask_type(imagetext::char_posns__vertical_halves);
//      curr_imagetext.set_mask_type(imagetext::char_posns__digit_letter);
//      curr_imagetext.set_mask_type(imagetext::char_posns__word_numbering);
//      curr_imagetext.set_mask_type(imagetext::digit_letter__word_n_chars);


      bool visualize_mask_flag = false;
//      bool visualize_mask_flag = true;
      int max_charmask_val, max_wordmask_val;

      if(!generate_just_digits_flag)
      {
         curr_imagetext.generate_masks(
            charmask_texture_rectangle_ptr, wordmask_texture_rectangle_ptr,
            visualize_mask_flag, max_charmask_val, max_wordmask_val);
//      cout << "max_charmask_val = " << max_charmask_val
//           << " max_wordmask_val = " << max_wordmask_val << endl;

         charmask_texture_rectangle_ptr->write_curr_frame(charmask_filename);
         wordmask_texture_rectangle_ptr->write_curr_frame(wordmask_filename);
      }

      if(visualize_mask_flag)
      {
         curr_imagetext.generate_mask_montage(
            charmask_filename, wordmask_filename,
            charmask_texture_rectangle_ptr, wordmask_texture_rectangle_ptr, 
            synthetic_string_counter);
      }

// Step 10:  Rotate image chip and masks:

      cout << " 10" << flush;
      
      string rotated_image_filename, rotated_charmask_filename,
         rotated_wordmask_filename;

      double max_abs_az = 85;
      double max_abs_el = 30;
      double max_abs_roll = 20;

      if(generate_just_digits_flag)
      {
         if(!textfunc::rotate_image(
               synthetic_chip_filename, rotated_images_subdir,
               max_text_label_width, rotated_image_filename, 
               curr_imagetext.get_phrase()))
         {
            cout << "textfunc::rotate_image() failed" << endl;
            continue;
         }
      }
      else
      {
         if(!textfunc::rotate_image_and_masks(
               synthetic_chip_filename, charmask_filename, wordmask_filename,
               rotated_images_subdir, rotated_char_masks_subdir, 
               rotated_word_masks_subdir, max_text_label_width,
               max_abs_az, max_abs_el, max_abs_roll,
               rotated_image_filename, rotated_charmask_filename,
               rotated_wordmask_filename))
         {
            cout << "textfunc::rotate_image_and_masks() failed" << endl;
            continue;
         }
      }
      
// Delete (rotated_image, rotated_charmask, rotated_wordmask) if any
// charmask or wordmask value is invalid:

      if(!generate_just_digits_flag && !visualize_mask_flag)
      {
         if(!textfunc::check_mask_values(
            rotated_image_filename, rotated_charmask_filename, 
            rotated_wordmask_filename, 
            0, max_charmask_val, 0, max_wordmask_val))
         {
            cout << "textfunc::check_mask_values() failed" << endl;
            continue;
         }
      }
      cout << endl;

      curr_imagetext.print_info(rotated_image_filename, outstream);

      cout << "Exported synthetic text and masks " << synthetic_string_counter
           << endl;

      synthetic_string_counter++;
      i++;
      cout << endl;

   } // loop over index i labeling cleaned input strings

   filefunc::closefile(output_filename, outstream);   
   cout << endl;
} 
