// ==========================================================================
// Methods for synthesizing text character images
// ==========================================================================
// Last updated on 4/17/16; 4/20/16; 8/29/16; 8/30/16
// ==========================================================================

#include <iostream>
#include "video/camera.h"
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "text/imagetext.h"
#include "numrec/nrfuncs.h"
#include "geometry/plane.h"
#include "general/sysfuncs.h"
#include "text/textfuncs.h"
#include "video/texture_rectangle.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::string;
using std::vector;

namespace textfunc
{

// Method generate_character_set() imports text characters or
// digits from precomputed text files.  It appends the imported
// characters onto STL vector cleaned_chars.  

   void generate_character_set(
      char& char_type, string& synthetic_char_set_str,
      vector<string>& cleaned_chars)
   {
      bool more_upper_case_letters_flag = true;   
      int synthetic_character_set_ID = -1;
      synthetic_char_set_str="";
      if (synthetic_character_set_ID >= 0)
      {
         synthetic_char_set_str=stringfunc::number_to_string(
            synthetic_character_set_ID);
      }

      char_type = 'n';
      cout << "Enter 'n' or 'l' to synthesize numbers or letters:" << endl;
      cin >> char_type;
   
      string words_filename="english_words.dat";
      if (char_type=='n')
      {
         words_filename="Long_numbers.dat";
      }
      filefunc::ReadInfile(words_filename);
      int n_total_words=filefunc::text_line.size();

      unsigned const int min_letter_count=8;
   
//   unsigned int max_iters=200;
//   unsigned int max_iters=10000;
//   unsigned int max_iters=20000;
      unsigned int max_iters=50000;
      for (unsigned int iter=0; iter<max_iters; iter++)
      {
         int i=nrfunc::ran1()*n_total_words;
//       cout << filefunc::text_line[i] << endl;

         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               filefunc::text_line[i]);
         for (unsigned int s=0; s<substrings.size(); s++)
         {
            string curr_string=substrings[s];
            unsigned int curr_string_length=curr_string.size();
//         cout << "s = " << s << " curr_string = " << curr_string << endl;
//         cout << "curr_string_length = " << curr_string_length << endl;
            if (curr_string_length < min_letter_count) continue;

            string cleaned_substring="";
            for (unsigned int c=0; c<curr_string_length; c++)
            {
               char curr_char=stringfunc::string_to_char(
                  curr_string.substr(c,1));
//            cout << "c = " << c << " char = " << curr_char << endl;
               bool char_OK=false;

               int ascii_int=stringfunc::char_to_ascii_integer(curr_char);
               if (ascii_int >= 48 && ascii_int <=57) char_OK=true;  // digits
               if (ascii_int >= 65 && ascii_int <=90) char_OK=true;  // upper case
               if (ascii_int >= 97 && ascii_int <=122) // lower case
               {
                  if (more_upper_case_letters_flag)
                  {
                     if (nrfunc::ran1() > 0.5)
                     {
                        ascii_int -= 32;
                        curr_char=stringfunc::ascii_integer_to_char(ascii_int);
                     }
                  }
                  char_OK=true; 
               }

               if (!char_OK) continue;
               cleaned_substring += stringfunc::char_to_string(curr_char);
            }
            unsigned int cleaned_string_length=cleaned_substring.size();
            if (cleaned_string_length < min_letter_count) continue;

            for (unsigned int c=0; c<cleaned_substring.size(); c++)
            {
               cleaned_chars.push_back(cleaned_substring.substr(c,1));
//            cout << cleaned_chars.size() << "  " 
//                 << cleaned_chars.back() << endl;
            }
         }
      } // loop over iter index
   }

// -------------------------------------------------------------------------
// Method generate_string_set() 

   void generate_string_set(
      char& string_type, vector<string>& cleaned_strings)
   {
      bool generate_some_upper_case_letters_flag = false;
      string_type = 'n';
      cout << "Enter 'n' or 'l' to synthesize numbers or letters:" << endl;
      cin >> string_type;
   
      string words_filename="shuffled_english_words.txt";
      if (string_type=='n')
      {
         words_filename="house_numbers.dat";
      }
      else
      {
         generate_some_upper_case_letters_flag = true;
      }
      
      filefunc::ReadInfile(words_filename);
//      int n_total_words=filefunc::text_line.size();
   
//   unsigned int max_iters=200;
//   unsigned int max_iters=10000;
//   unsigned int max_iters=20000;
   unsigned int max_iters=50000;
//      unsigned int max_iters=100000;
      for (unsigned int i=0; i<max_iters; i++)
      {
//       cout << filefunc::text_line[i] << endl;

         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               filefunc::text_line[i]);
         for (unsigned int s=0; s<substrings.size(); s++)
         {
            string curr_string=substrings[s];
            unsigned int curr_string_length=curr_string.size();
//         cout << "s = " << s << " curr_string = " << curr_string << endl;
//         cout << "curr_string_length = " << curr_string_length << endl;

            bool capitalize_first_letter_flag = false;
            bool capitalize_all_letters_flag = false;
            if(generate_some_upper_case_letters_flag)
            {
               if(nrfunc::ran1() < 0.25)
               {
                  capitalize_first_letter_flag = true;
               }
               if(nrfunc::ran1() < 0.15)
               {
                  capitalize_all_letters_flag = true;
               }
            }

            string cleaned_substring="";
            for (unsigned int c=0; c<curr_string_length; c++)
            {
               char curr_char=stringfunc::string_to_char(
                  curr_string.substr(c,1));
//            cout << "c = " << c << " char = " << curr_char << endl;
               bool char_OK=false;

               int ascii_int=stringfunc::char_to_ascii_integer(curr_char);
               if (ascii_int >= 48 && ascii_int <= 57) char_OK=true;  // digits
               if (ascii_int >= 65 && ascii_int <= 90) char_OK=true;  
								 // upper case
               if (ascii_int >= 97 && ascii_int <= 122)          // lower case
               {
                  if (capitalize_all_letters_flag ||
                      (capitalize_first_letter_flag && c==0) )
                  {
                     ascii_int -= 32;
                     curr_char=stringfunc::ascii_integer_to_char(ascii_int);
                  }
                  char_OK=true; 
               }

               if (!char_OK) continue;
               cleaned_substring += stringfunc::char_to_string(curr_char);
            } // loop over index c labeling curr string characters
            
            cleaned_strings.push_back(cleaned_substring);
         }
      } // loop over i index
   }

// -------------------------------------------------------------------------
// Method generate_phrase_set() 

   void generate_phrase_set(vector<string>& string_phrases)
   {
      string text_files_subdir="./text_files/";   
      string phrases_filename=text_files_subdir+"shuffled_all_text.txt";
      
      string curr_phrase;
      filefunc::ReadInfile(phrases_filename);

      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         string curr_string = stringfunc::remove_leading_whitespace(
            filefunc::text_line[i]);
         curr_string = stringfunc::remove_trailing_whitespace(curr_string);

         if(curr_string.size() > 0)
         {
            string_phrases.push_back(curr_string);
         }
      } 
   }

// -------------------------------------------------------------------------
// Method spaced_out_text_label() introduces spaces between every
// character within an input string.

   string spaced_out_text_label(string text_label, int skip)
   {
      string spaced_out_text_label="";
      for(unsigned int c = 0; c < text_label.size(); c++)
      {
         spaced_out_text_label += text_label.at(c);

// Don't add space at very end of input text_label:

         if(c < text_label.size() - 1)
         {
            for(int s = 0; s < skip; s++)
            {
               spaced_out_text_label += " ";
            }
         }
      }
//      cout << "spaced_out_text_label = " << spaced_out_text_label << endl;
      return spaced_out_text_label;
   }

// -------------------------------------------------------------------------
// Method vertical_text_label() introduces a carriage return after
// every character within an input string.

   string vertical_text_label(string text_label)
   {
//      cout << "inside textfunc::vertical_text_label() " << endl;
      string vertical_text_label="";

      text_label = stringfunc::find_and_replace_char(
         text_label, "i", "I");
      text_label = stringfunc::find_and_replace_char(
         text_label, "j", "J");

      for(unsigned int c = 0; c < text_label.size(); c++)
      {
         vertical_text_label += text_label.at(c);

// Don't add return at very end of input text_label:

         if(c < text_label.size() - 1) vertical_text_label += "\n";
      }
//      cout << endl;
//      cout << "vertical_text_label = " << vertical_text_label << endl;
      return vertical_text_label;
   }

// ==========================================================================
// Text image chip background methods
// ==========================================================================

// Method retrieve_nonsymbol_background_filenames() fills STL vector
// nonsymbol_filenames with names of JPG images containing no text
// characters.  It also exports the relative frequencies with which
// such background images should be sampled that are proportional to
// the images' pixel counts.

   void retrieve_nonsymbol_background_filenames(
      vector<string>& nonsymbol_filenames,
      vector<int>& nonsymbol_filename_frequencies)
   {
      string nonsymbols_subdir="./images/internet/non_text/";
//      string nonsymbols_subdir="./images/pwin/backgrounds/";
      nonsymbol_filenames=filefunc::image_files_in_subdir(nonsymbols_subdir);

      compute_background_image_frequencies(
         nonsymbol_filenames, nonsymbol_filename_frequencies);
//      compute_background_image_entropy_frequencies(
//         nonsymbol_filenames, nonsymbol_filename_frequencies);
   }

// -------------------------------------------------------------------------
// Method compute_background_image_entropy_frequencies() experiments
// with setting background image frequencies based upon their inverse
// square entropies.  It favors backgrounds which are smooth and
// penalizes backgrounds which are busy.

   void compute_background_image_entropy_frequencies(
      const vector<string>& nonsymbol_filenames,
      vector<int>& nonsymbol_filename_frequencies)
   {
      string banner=
         "Setting background image frequencies based on their entropies";
      outputfunc::write_banner(banner);

      const double min_entropy = 0.1;
      double filter_intensities_flag = false;
      for (unsigned int n=0; n<nonsymbol_filenames.size(); n++)
      {
         outputfunc::update_progress_fraction(
            n, 25, nonsymbol_filenames.size());

         texture_rectangle curr_tr(nonsymbol_filenames[n],NULL);
         double curr_entropy = curr_tr.compute_image_entropy(
            filter_intensities_flag,0);
         if(curr_entropy < min_entropy) curr_entropy = min_entropy;
         int filename_freq = 
            1 + 4 * basic_math::round(1/ (curr_entropy * curr_entropy) - 1);
         nonsymbol_filename_frequencies.push_back(filename_freq);
//         cout << "n = " << n << " entropy = " << curr_entropy
//              << " freq = " << filename_freq << endl;
      }
   }

// -------------------------------------------------------------------------
// Method compute_background_image_frequencies() takes into account
// different pixel dimensions for non-symbol background images.  It
// generates a "weighted" nonsymbol_filename_frequencies STL vector
// where the number of "mentions" of a particular nonsymbol_filename's
// index is roughly proportional to its pixel count.

   void compute_background_image_frequencies(
      const vector<string>& nonsymbol_filenames,
      vector<int>& nonsymbol_filename_frequencies)
   {
      vector<double> n_pixels_per_file;
      for (unsigned int n=0; n<nonsymbol_filenames.size(); n++)
      {
         unsigned int width,height;
         imagefunc::get_image_width_height(nonsymbol_filenames[n],width,height);
         n_pixels_per_file.push_back( width*height );
      }
      double n_max_pixels = mathfunc::maximal_value(n_pixels_per_file);

      nonsymbol_filename_frequencies.clear();
      for (unsigned int n=0; n<nonsymbol_filenames.size(); n++)
      {
         n_pixels_per_file[n] /= n_max_pixels;
//      cout << "n = " << n << " n_pixels_per_file = " 
//           << n_pixels_per_file[n] << endl;
         int n_file_mentions = basic_math::round(100 * n_pixels_per_file[n]);
         n_file_mentions = basic_math::max(1, n_file_mentions);

         for(int j=0; j<n_file_mentions; j++)
         {
            nonsymbol_filename_frequencies.push_back(n);
         }
      }

      cout << "nonsymbol_filename_frequencies.size() = "
           << nonsymbol_filename_frequencies.size() << endl;
   }

// -------------------------------------------------------------------------
// Method import_all_background_images() loads STL vector
// random_texture_rectangle_ptrs with pointers to dynamically allocated 
// texture rectangles.  Each texture rectangle is initialized with one
// background image that does not contain any text symbols.

   void import_all_background_images(
      const vector<string>& background_image_filenames,
      vector<texture_rectangle*>& random_texture_rectangle_ptrs)
   {
      cout << "Importing all background images:" << endl;

      unsigned int n_background_filenames=background_image_filenames.size();

// As of 3/6/16 at 6:30 pm, we empirically found on M6700 that we
// CANNOT import more than O(425) background internet images without
// text generation via ImageMagick++ failing.  But on 3/7/16, we
// encountered no such restriction on ThinkMate....

//      n_background_filenames = 425;	// OK
//      n_background_filenames = 430;	// OK
//      n_background_filenames = 440;	// OK
//      n_background_filenames = 441;	// OK
//      n_background_filenames = 442;	// bad
//      n_background_filenames = 445;	// bad
//      n_background_filenames = 450;	// bad
      
      for (unsigned int i=0; i<n_background_filenames; i++)
      {
         outputfunc::update_progress_fraction(i,25,n_background_filenames);
         texture_rectangle* random_texture_rectangle_ptr=
            new texture_rectangle();
         if(!random_texture_rectangle_ptr->import_photo_from_file(
               background_image_filenames[i]))
         {
            cout << "Couldn't import " << background_image_filenames[i]
                 << endl;
            outputfunc::enter_continue_char();
            delete random_texture_rectangle_ptr;
         }
         else 
         {
            const unsigned int max_width = 4000;
            const unsigned int max_height = 4000;
            unsigned int width = random_texture_rectangle_ptr->getWidth();
            unsigned int height = random_texture_rectangle_ptr->getHeight();
            
            if(width < max_width && height < max_height)
            {
//               cout << "i = " << i << " width = " << width
//                    << " height = " << height << endl;
//               cout << "  basename = " << filefunc::getbasename(
//                  background_image_filenames[i]) << endl;
               random_texture_rectangle_ptrs.push_back(
                  random_texture_rectangle_ptr);
            }
            else
            {
               delete random_texture_rectangle_ptr;
            }
         }
      }
      cout << endl;
   }

// -------------------------------------------------------------------------
// Method extract_background_patch() exports a patch from a randomly
// selected input image into an output image chip.

   void extract_background_patch(
      int rx_start,int ry_start,
      texture_rectangle* random_texture_rectangle_ptr,
      texture_rectangle* texture_rectangle_ptr)
   {
//   cout << "inside extract_background_patch()" << endl;

      unsigned int width=texture_rectangle_ptr->getWidth();
      unsigned int height=texture_rectangle_ptr->getHeight();

      int R,G,B;
      for (unsigned int pv=0; pv<height; pv++)
      {
         for (unsigned int pu=0; pu<width; pu++)
         {
            random_texture_rectangle_ptr->get_pixel_RGB_values(
               rx_start+pu,ry_start+pv,R,G,B);
            texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
         } // loop over pu
      } // loop over pv
   }

// -------------------------------------------------------------------------
// Method initialize_random_image() imports some random image that
// should not contain any man-made symbols.  It randomly picks the
// upper left corner's pixel coordinates (rx, ry) for a patch whose
// entropy is not too large.

   texture_rectangle* initialize_random_image(
      const vector<string>& nonsymbol_filenames,
      const vector<int>& nonsymbol_filename_frequencies,
      const vector<texture_rectangle*>& random_texture_rectangle_ptrs,
      unsigned int cropped_udim,unsigned int cropped_vdim,
      int& rx_start,int& ry_start)
   {
//      cout << "inside initialize_random_image() " << endl;
//      cout << "cropped_udim = " << cropped_udim
//           << " cropped_vdim = " << cropped_vdim << endl;

      bool large_enough_random_image_flag = false;
      texture_rectangle* curr_random_texture_rectangle_ptr=NULL;
      while(!large_enough_random_image_flag)
      {
         int random_image_index=
            nrfunc::ran1()*nonsymbol_filename_frequencies.size();
         unsigned int random_image_number=nonsymbol_filename_frequencies[
            random_image_index];
         if(random_image_number < random_texture_rectangle_ptrs.size())
         {
            curr_random_texture_rectangle_ptr=
               random_texture_rectangle_ptrs[random_image_number];
            if(curr_random_texture_rectangle_ptr->getWidth() > cropped_udim &&
               curr_random_texture_rectangle_ptr->getHeight() > cropped_vdim)
            {
               large_enough_random_image_flag = true;
            }
         }
      }
      
      int curr_iter = 0;
      int n_max_iters = 50;
      double max_chip_entropy = 0.8;
      double chip_RGB_entropy = 1;
      do
      {
         rx_start=nrfunc::ran1()*(
            curr_random_texture_rectangle_ptr->getWidth()-cropped_udim);
         ry_start=nrfunc::ran1()*(
            curr_random_texture_rectangle_ptr->getHeight()-cropped_vdim);

         double chip_R_entropy,chip_G_entropy,chip_B_entropy;
         curr_random_texture_rectangle_ptr->compute_RGB_image_entropies(
            rx_start,rx_start+cropped_udim,
            ry_start,ry_start+cropped_vdim,
            chip_R_entropy,chip_G_entropy,chip_B_entropy);
         chip_RGB_entropy = basic_math::max(
            chip_R_entropy,chip_G_entropy,chip_B_entropy);
         curr_iter++;
      }
      while(chip_RGB_entropy > max_chip_entropy && curr_iter < n_max_iters);

      return curr_random_texture_rectangle_ptr;
   }

// -------------------------------------------------------------------------
// Method peak_random_patch_hsv() takes in the starting point and
// horiz/vertical extents of a random patch selected from a random
// image.  It computes the HSV distributions within this patch.  The
// peak values for hue, saturation and value within the patch are
// returned.  

   colorfunc::HSV peak_random_patch_hsv(
      int rx_start,int ry_start,
      unsigned int cropped_udim,unsigned int cropped_vdim,
      texture_rectangle* random_texture_rectangle_ptr)
   {
//   cout << "inside peak_random_patch_hsv()" << endl;

      vector<double> h,s,v;
      for (unsigned int pv=0; pv<cropped_vdim; pv++)
      {
         for (unsigned int pu=0; pu<cropped_udim; pu++)
         {
            double curr_h,curr_s,curr_v;
            if (random_texture_rectangle_ptr->get_pixel_hsv_values(
                   rx_start+pu,ry_start+pv,curr_h,curr_s,curr_v))
            {
               h.push_back(curr_h);
            }
            s.push_back(curr_s);
            v.push_back(curr_v);
         }
      }

      double median_h = mathfunc::median_value(h);
      double median_s = mathfunc::median_value(s);
      double median_v = mathfunc::median_value(v);

      return colorfunc::HSV(median_h,median_s,median_v);
   }

// -------------------------------------------------------------------------
// Method synthesize_background_patch() takes in
// *background_texture_rectangle_ptr and fills its with background_RGB.  

   void synthesize_background_patch(
      texture_rectangle* background_texture_rectangle_ptr,
      colorfunc::RGB background_RGB)
   {
      for(unsigned int py = 0; 
          py < background_texture_rectangle_ptr->getHeight(); py++)
      {
         for(unsigned int px = 0; 
             px < background_texture_rectangle_ptr->getWidth(); px++)
         {
            background_texture_rectangle_ptr->set_pixel_RGB_values(
               px, py, background_RGB.first, background_RGB.second,
               background_RGB.third);
         }
      }
   }

// -------------------------------------------------------------------------
// Method add_random_background_lines() alters the saturation and
// value (but not hue) of either 1 or 2 rows/columns of pixels within
// *background_texture_rectangle_ptr.

   void add_random_background_lines(
      texture_rectangle* background_texture_rectangle_ptr,
      colorfunc::HSV background_HSV,
      double s_lambda, double v_mu, double v_sigma)
   {
      double h_line = background_HSV.first;
      double s_line = colorfunc::exponential_distributed_S(s_lambda);
      double v_line = colorfunc::gaussian_distributed_V(v_mu, v_sigma);

      int n_horiz_lines = 0, n_vert_lines = 0;
      double horiz_random = nrfunc::ran1();
      double vert_random = nrfunc::ran1();

//      const double single_line_threshold = 0.1;
      const double single_line_threshold = 0.85;
//      const double double_line_threshold = 0.50;
      const double double_line_threshold = 0.95;

      if(horiz_random > single_line_threshold)
      {
         n_horiz_lines = 1;
      }
      if(horiz_random > double_line_threshold)
      {
         n_horiz_lines = 2;
      }
      if(vert_random > single_line_threshold)
      {
         n_vert_lines = 1;
      }
      if(vert_random > double_line_threshold)
      {
         n_vert_lines = 2;
      }

      unsigned int xdim = background_texture_rectangle_ptr->getWidth();
      unsigned int ydim = background_texture_rectangle_ptr->getHeight();

      for(int i = 0; i < n_vert_lines; i++)
      {
         unsigned px = nrfunc::ran1() * xdim;
         for(unsigned py = 0; py < ydim; py++)
         {
            background_texture_rectangle_ptr->set_pixel_hsv_values(
               px, py, h_line, s_line, v_line);
         }
      }

      for(int i = 0; i < n_horiz_lines; i++)
      {
         unsigned py = nrfunc::ran1() * ydim;
         for(unsigned px = 0; px < xdim; px++)
         {
            background_texture_rectangle_ptr->set_pixel_hsv_values(
               px, py, h_line, s_line, v_line);
         }
      }
   }
   
// ==========================================================================
// Foreground/background color methods
// ==========================================================================

   const int K=3;
   typedef dlib::matrix<double, K, 1> sample_type;
   typedef dlib::linear_kernel<sample_type> kernel_type;
   typedef dlib::decision_function<kernel_type> dec_funct_type;
   typedef dlib::normalized_function<dec_funct_type> funct_type;
   funct_type learned_bfunct;

   typedef dlib::probabilistic_decision_function<kernel_type> 
      probabilistic_funct_type;
   typedef dlib::normalized_function<probabilistic_funct_type> pfunct_type;
   pfunct_type learned_pfunct; 

// -------------------------------------------------------------------------
// Method import_foreground_background_colors_decision_function()
// imports the binary decision function generated via DLIB's linear
// SVM which classifies foreground/background color pairs as legible
// or unreadable.

   void import_foreground_background_colors_decision_function()
   {
      string learned_bifunc_filename = "./learned_functions/colors_bifunc.dat";
      ifstream fin(learned_bifunc_filename.c_str(),ios::binary);
      dlib::deserialize(learned_bfunct, fin);
   
      string learned_pfunc_filename = "./learned_functions/colors_pfunc.dat";
      ifstream fin2(learned_pfunc_filename.c_str(),ios::binary);
      dlib::deserialize(learned_pfunct, fin2);
   }

// -------------------------------------------------------------------------
// Method test_foreground_background_color_acceptability() runs a
// pre-trained SVM classifier on a candidate pair of foreground and
// background colors.  If the SVM's probabilistic score exceeds an
// input threshold, this boolean method returns true.

   bool test_foreground_background_color_acceptability(
      double classification_score_threshold, colorfunc::RGB& foreground_RGB,
      colorfunc::RGB& background_RGB)   
   {
      double dR = fabs(foreground_RGB.first - background_RGB.first);
      double dG = fabs(foreground_RGB.second - background_RGB.second);
      double dB = fabs(foreground_RGB.third - background_RGB.third);
      sample_type test_sample;
      test_sample(0) = dR / 255.0;
      test_sample(1) = dG / 255.0;
      test_sample(2) = dB / 255.0;
      double classification_score = learned_pfunct(test_sample);
//      cout << "classification_score = " << classification_score << endl;

      if(classification_score > classification_score_threshold)
      {
         return true;
      }
      else
      {
         return false;
      }
   }

// -------------------------------------------------------------------------
// Method generate_acceptable_foreground_color() 

   bool generate_acceptable_foreground_color(
      double lambda_s, double mu_v, double sigma_v,
      double classification_score_threshold, 
      colorfunc::RGB& foreground_RGB, 
      colorfunc::RGB& background_RGB, int iter_max)
   {
      bool OK_color = false;
      int iter = 0;
      while (!OK_color && iter < iter_max)
      {
         foreground_RGB = colorfunc::generate_random_RGB(
            false, lambda_s, mu_v, sigma_v);
         OK_color = test_foreground_background_color_acceptability(
            classification_score_threshold, foreground_RGB, background_RGB);
         iter++;
      }
      return OK_color;
   }

// -------------------------------------------------------------------------
// Method count_foreground_background_pixels()

   void count_foreground_background_pixels(
      texture_rectangle* texture_rectangle_ptr, 
      colorfunc::RGB& text_background_RGB,
      int& n_foreground_pixels, int& n_background_pixels)
   {
      unsigned int width=texture_rectangle_ptr->getWidth();
      unsigned int height=texture_rectangle_ptr->getHeight();

      int R,G,B;
      n_foreground_pixels = n_background_pixels = 0;
      for (unsigned int pv=0; pv<height; pv++)
      {
         for (unsigned int pu=0; pu<width; pu++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(pu, pv, R, G, B);
            double dR = R - text_background_RGB.first;
            double dG = G - text_background_RGB.second;
            double dB = B - text_background_RGB.third;
            double delta = sqrt(dR*dR + dG*dG + dB*dB);
            if(delta > 10)
            {
               n_foreground_pixels++;
            }
            else
            {
               n_background_pixels++;
            }
         } // loop over pu
      } // loop over pv
   }

// -------------------------------------------------------------------------
// Method foreground_pixel_frac()

   double foreground_pixel_frac(
      texture_rectangle* texture_rectangle_ptr, 
      colorfunc::RGB& text_background_RGB)
   {
      int n_foreground_pixels, n_background_pixels;

      count_foreground_background_pixels(
         texture_rectangle_ptr, text_background_RGB,
         n_foreground_pixels, n_background_pixels);
      double foreground_frac = n_foreground_pixels / double(
         n_foreground_pixels + n_background_pixels);
      return foreground_frac;
   }

// ==========================================================================
// Text image chip foreground methods
// ==========================================================================

// Method compute_foreground_bbox()

   void compute_foreground_bbox(
      colorfunc::HSV& text_background_RGB,
      const texture_rectangle* foreground_texture_rectangle_ptr,
      int& px_min, int& px_max, int& py_min, int& py_max)
   {
      unsigned int width=foreground_texture_rectangle_ptr->getWidth();
      unsigned int height=foreground_texture_rectangle_ptr->getHeight();

      bool dark_text_background = true;
      if(text_background_RGB.first > 128) dark_text_background = false;

      bool left_edge_found = false;
      bool right_edge_found = false;
      bool bottom_edge_found = false;
      bool top_edge_found = false;

      px_min = px_max = py_min = py_max = -1;

      int R, G, B;
      int delta = 25;
      int min_bright = 255 - delta;
      int max_dark = 0 + delta;
      unsigned int pu, pv;

      pu = 0;
      while(pu < width && !left_edge_found)
      {
         for(pv = 0; pv < height; pv++)
         {
            foreground_texture_rectangle_ptr->fast_get_pixel_RGB_values(
               pu, pv, R, G, B);

            if(dark_text_background)
            {
               if(R > max_dark || G > max_dark || B > max_dark)
               {
                  left_edge_found = true;
               }
            }
            else
            {
               if(R < min_bright || G < min_bright || B < min_bright)
               {
                  left_edge_found = true;
               }
            }

            if(left_edge_found)
            {
               px_min = pu;
               break;
            }
            
         } // loop over pv 
         pu++;
      } // pu while loop

      pu = width - 1;
      while(pu >= 0 && !right_edge_found)
      {
         for(pv = 0; pv < height; pv++)
         {
            foreground_texture_rectangle_ptr->fast_get_pixel_RGB_values(
               pu, pv, R, G, B);

            if(dark_text_background)
            {
               if(R > max_dark || G > max_dark || B > max_dark)
               {
                  right_edge_found = true;
               }
            }
            else
            {
               if(R < min_bright || G < min_bright || B < min_bright)
               {
                  right_edge_found = true;
               }
            }

            if(right_edge_found)
            {
               px_max = pu;
               break;
            }
         } // loop over pv 
         pu--;
      } // pu while loop

      pv = 0;
      while(pv < height && !top_edge_found)
      {
         for(pu = 0; pu < width; pu++)
         {
            foreground_texture_rectangle_ptr->fast_get_pixel_RGB_values(
               pu, pv, R, G, B);

            if(dark_text_background)
            {
               if(R > max_dark || G > max_dark || B > max_dark)
               {
                  top_edge_found = true;
               }
            }
            else
            {
               if(R < min_bright || G < min_bright || B < min_bright)
               {
                  top_edge_found = true;
               }
            }

            if(top_edge_found)
            {
               py_min = pv;
               break;
            }
            
         } // loop over pu 
         pv++;
      } // pv while loop

      pv = height - 1;
      while(pv >= 0 && !bottom_edge_found)
      {
         for(pu = 0; pu < width; pu++)
         {
            foreground_texture_rectangle_ptr->fast_get_pixel_RGB_values(
               pu, pv, R, G, B);

            if(dark_text_background)
            {
               if(R > max_dark || G > max_dark || B > max_dark)
               {
                  bottom_edge_found = true;
               }
            }
            else
            {
               if(R < min_bright || G < min_bright || B < min_bright)
               {
                  bottom_edge_found = true;
               }
            }

            if(bottom_edge_found)
            {
               py_max = pv;
               break;
            }
            
         } // loop over pu 
         pv--;
      } // pv while loop

/*
      cout << "px_min = " << px_min << " px_max = " << px_max << endl;
      cout << " py_min = " << py_min << " py_max = " << py_max << endl;
      cout << "width = " << width << " height = " << height << endl;
*/
   }
   
// -------------------------------------------------------------------------
// Method draw_bbox_under_text_char()

   void draw_bbox_under_text_char(
      colorfunc::RGB& text_background_RGB, colorfunc::RGB& text_foreground_RGB,
      texture_rectangle* foreground_texture_rectangle_ptr,
      int px_min, int px_max, int py_min, int py_max)
   {
      bool dark_text_background = true;
      if(text_background_RGB.first > 128) dark_text_background = false;
      int delta = 25;
      int min_bright = 255 - delta;
      int max_dark = 0 + delta;

      colorfunc::HSV text_foreground_HSV = colorfunc::RGB_to_hsv(
         text_foreground_RGB, false);
      
      bool OK_bbox_color_flag = false;
      int iter = 0, max_iters = 500;
      colorfunc::HSV bbox_HSV;
      while(!OK_bbox_color_flag && iter < max_iters)
      {
         bbox_HSV = colorfunc::generate_random_HSV();         
         if(fabs(text_foreground_HSV.first - bbox_HSV.first) > 70 &&
            fabs(text_foreground_HSV.second - bbox_HSV.second) > 0.2 &&
            fabs(text_foreground_HSV.third - bbox_HSV.third) > 0.45)
         {
            OK_bbox_color_flag = true;
         }
         iter++;
      }

      int R, G, B;
      for(int py = py_min; py <= py_max; py++)
      {
         for(int px = px_min; px <= px_max; px++)
         {
            foreground_texture_rectangle_ptr->fast_get_pixel_RGB_values(
               px, py, R, G, B);

            if(dark_text_background)
            {
               if(R > max_dark || G > max_dark || B > max_dark)
               {
                  continue;
               }
            }
            else
            {
               if(R < min_bright || G < min_bright || B < min_bright)
               {
                  continue;
               }
            }
            foreground_texture_rectangle_ptr->set_pixel_hsv_values(
               px, py, bbox_HSV.first, bbox_HSV.second, bbox_HSV.third);
         } // loop over px
      } // loop over py 
   }

// -------------------------------------------------------------------------
// Method draw_bbox_under_text_char()

   void draw_bbox_under_text_char(
      colorfunc::RGB& text_background_RGB, colorfunc::RGB& foreground_RGB,
      colorfunc::RGB& stroke_RGB, double strokewidth,
      texture_rectangle* foreground_texture_rectangle_ptr)
   {
      int px_min, px_max, py_min, py_max;
      compute_foreground_bbox(
         text_background_RGB, foreground_texture_rectangle_ptr,
         px_min, px_max, py_min, py_max);

// Extend box to be drawn underneath text char compared to character's
// snug bbox:

      double px_center = 0.5 * (px_min + px_max);
      double py_center = 0.5 * (py_min + py_max);
      double bbox_width = px_max - px_min;
      double bbox_height = py_max - py_min;
      bbox_width *= 1.5;
      bbox_height *= 1.5;

      int width=foreground_texture_rectangle_ptr->getWidth();
      int height=foreground_texture_rectangle_ptr->getHeight();

      px_min = px_center - 0.5 * bbox_width;
      px_max = px_center + 0.5 * bbox_width;
      px_min = basic_math::max(px_min, 0);
      px_max = basic_math::min(px_max, width - 1);

      py_min = py_center - 0.5 * bbox_height;
      py_max = py_center + 0.5 * bbox_height;
      py_min = basic_math::max(py_min, 0);
      py_max = basic_math::min(py_max, height - 1);

      colorfunc::RGB text_foreground_RGB;
      if(strokewidth > 0)
      {
         text_foreground_RGB = stroke_RGB;
      }
      else
      {
         text_foreground_RGB = foreground_RGB;
      }

      textfunc::draw_bbox_under_text_char(
        text_background_RGB, text_foreground_RGB,
        foreground_texture_rectangle_ptr, px_min, px_max, py_min, py_max);
   }
   
// -------------------------------------------------------------------------
// Method select_new_foreground_color_and_background_patch() replaces
// synthetic character's initial foreground and stroke colors with new
// ones.  The foreground and background colors are required to be
// reasonably distinguishable.

   texture_rectangle* select_new_foreground_color_and_background_patch(
      bool generate_greyscale_chips_flag,
      const vector<string>& nonsymbol_filenames,
      const vector<int>& nonsymbol_filename_frequencies,
      const vector<texture_rectangle*>& random_texture_rectangle_ptrs,
      unsigned int cropped_udim,unsigned int cropped_vdim,
      int& rx_start,int& ry_start,
      colorfunc::RGB& foreground_RGB,colorfunc::RGB& stroke_RGB)
   {
//   cout << "inside select_new_foreground_color_and_background_patch()"
//        << endl;

      int max_iters = 5;
      bool colors_OK_flag=false;
      colorfunc::HSV foreground_HSV;
      texture_rectangle* curr_random_texture_rectangle_ptr=NULL;

      int curr_iter = 0;
      while (!colors_OK_flag && curr_iter < max_iters)
      {
         colorfunc::HSV background_HSV;
         curr_random_texture_rectangle_ptr=initialize_random_image(
            nonsymbol_filenames,nonsymbol_filename_frequencies,
            random_texture_rectangle_ptrs,
            cropped_udim,cropped_vdim,rx_start,ry_start);

         background_HSV=peak_random_patch_hsv(
            rx_start,ry_start,cropped_udim,cropped_vdim,
            curr_random_texture_rectangle_ptr);

         int foreground_counter=0;
         while (!colors_OK_flag && foreground_counter < 250)
         {
            if(generate_greyscale_chips_flag)
            {
               foreground_HSV.second = 0;
            }
            else
            {
               foreground_HSV.second = basic_math::min(
                  0.5 * fabs(nrfunc::gasdev()), 1.0);
            }
         
// Require reasonable hue, saturation and value differences between
// character foreground and background colors:

            if (generate_greyscale_chips_flag ||
                fabs(foreground_HSV.second-background_HSV.second) > 0.3)
            {

               if(generate_greyscale_chips_flag)
               {
                  foreground_HSV.first = 0;
               }
               else
               {

// Force foreground hue to differ from background hue by at least 60 degs:

                  double min_hue=background_HSV.first+60;
                  double max_hue=background_HSV.first+300;
                  foreground_HSV.first=min_hue+nrfunc::ran1()*
                     (max_hue-min_hue);
                  foreground_HSV.first=basic_math::phase_to_canonical_interval(
                     foreground_HSV.first,0,360);
               }

// Force foreground value to differ from background value by at least 0.4:

               double min_delta_v = 0.4;
               double max_value=background_HSV.third-min_delta_v+1.0;
               double min_value=background_HSV.third+min_delta_v;
               foreground_HSV.third=min_value+nrfunc::ran1()*(
                  max_value-min_value);
               if(foreground_HSV.third > 1) foreground_HSV.third -= 1;

               foreground_RGB = colorfunc::hsv_to_RGB(foreground_HSV,false);

               colorfunc::HSV stroke_HSV;

               if(generate_greyscale_chips_flag)
               {
                  stroke_HSV.first = 0;
                  stroke_HSV.second = 0;
               }
               else
               {
                  stroke_HSV.first = foreground_HSV.first;
                  stroke_HSV.second = nrfunc::ran1();
               }
               stroke_HSV.third = nrfunc::ran1();

               stroke_RGB = colorfunc::hsv_to_RGB(stroke_HSV,false);

               colors_OK_flag=true;
            } // foreground and background S values differ conditional
         
            foreground_counter++;
         } // colors_OK_flag and foreground_counter < 250 while loop
      
         curr_iter++;
      } // !colors_OK_flag and curr_iter < max_iter while loop
      return curr_random_texture_rectangle_ptr;
   }

// -------------------------------------------------------------------------
// Method superpose_foreground_on_background_patch() overwrites the
// input character image's (mostly) black or white backgrounds with
// pixels taken from *background_texture_rectangle_ptr.  Individual
// color channel averaging is performed near the text character's
// border.

   double superpose_foreground_on_background_patch(
      colorfunc::RGBA text_background_RGBA,
      texture_rectangle* background_texture_rectangle_ptr,
      texture_rectangle* texture_rectangle_ptr)
   {
      cout << "inside superpose_foreground_on_background_patch()" << endl;

// Recall *texture_rectangle_ptr has RGB and A color channels:

      unsigned int width=texture_rectangle_ptr->getWidth();
      unsigned int height=texture_rectangle_ptr->getHeight();

      bool dark_text_background = true;
      if(text_background_RGBA.first > 128) dark_text_background = false;

//      const int min_value = 10;
      const int min_value = 25;
//      const int min_value = 50;
      const int max_value = 255 - min_value;

      int delta_new_background = 0;
      double delta_frac = 0;
      
      for (unsigned int pv=0; pv<height; pv++)
      {
         for (unsigned int pu=0; pu<width; pu++)
         {
            int Rforeground, Gforeground, Bforeground, Aforeground;
            texture_rectangle_ptr->get_pixel_RGBA_values(
               pu, pv, Rforeground, Gforeground, Bforeground, Aforeground);

            int Rbackground, Gbackground, Bbackground;
            background_texture_rectangle_ptr->get_pixel_RGB_values(
               pu, pv, Rbackground, Gbackground, Bbackground);

            int Rnew = Rforeground;
            int Gnew = Gforeground;
            int Bnew = Bforeground;
            int Anew = 128;
//            int Anew = 255;
            
            if(dark_text_background && Rforeground < min_value)
            {
               double denom = Rforeground + Rbackground;
               if(denom > 0)
               {
                  double alpha = Rforeground / denom;
                  Rnew = alpha * Rforeground + (1 - alpha) * Rbackground;
               }
            }
            else if(!dark_text_background && Rforeground > max_value)
            {
               double denom = (255 - Rforeground) + (255 - Rbackground);
               if(denom > 0)
               {
                  double alpha = (255 - Rforeground) / denom;
                  Rnew = alpha * Rforeground + (1 - alpha) * Rbackground;
               }
            }

            if(dark_text_background && Gforeground < min_value)
            {
               double denom = Gforeground + Gbackground;
               if(denom > 0)
               {
                  double alpha = Gforeground / denom;
                  Gnew = alpha * Gforeground + (1 - alpha) * Gbackground;
               }
            }
            else if(!dark_text_background && Gforeground > max_value)
            {
               double denom = (255 - Gforeground) + (255 - Gbackground);
               if(denom > 0)
               {
                  double alpha = (255 - Gforeground) / denom;
                  Gnew = alpha * Gforeground + (1 - alpha) * Gbackground;
               }
            }
            
            if(dark_text_background && Bforeground < min_value)
            {
               double denom = Bforeground + Bbackground;
               if(denom > 0)
               {
                  double alpha = Bforeground / denom;
                  Bnew = alpha * Bforeground + (1 - alpha) * Bbackground;
               }
            }
            else if(!dark_text_background && Bforeground > max_value)
            {
               double denom = (255 - Bforeground) + (255 - Bbackground);
               if(denom > 0)
               {
                  double alpha = (255 - Bforeground) / denom;
                  Bnew = alpha * Bforeground + (1 - alpha) * Bbackground;
               }
            }
            
            int dR = abs(Rnew - Rbackground);
            int dG = abs(Gnew - Gbackground);
            int dB = abs(Bnew - Bbackground);
            delta_new_background += (dR + dG + dB);

            texture_rectangle_ptr->set_pixel_RGBA_values(
               pu, pv, Rnew, Gnew, Bnew, Anew);
//            texture_rectangle_ptr->set_pixel_RGB_values(
//               pu, pv, Rnew, Gnew, Bnew);
         } // loop over pu
      } // loop over pv

      delta_frac = delta_new_background / (width * height);
      return delta_frac;
   }

// -------------------------------------------------------------------------
// Method copy_background_patch() simply transfers the pixel content
// of *background_texture_rectangle_ptr into *texture_rectangle_ptr.

   void copy_background_patch(
      texture_rectangle* background_texture_rectangle_ptr,
      texture_rectangle* texture_rectangle_ptr)
   {
//   cout << "inside copy_background_patch()" << endl;

      unsigned int width=texture_rectangle_ptr->getWidth();
      unsigned int height=texture_rectangle_ptr->getHeight();

      for (unsigned int pv=0; pv<height; pv++)
      {
         for (unsigned int pu=0; pu<width; pu++)
         {
            int Rbackground, Gbackground, Bbackground;
            background_texture_rectangle_ptr->get_pixel_RGB_values(
               pu, pv, Rbackground, Gbackground, Bbackground);
            texture_rectangle_ptr->set_pixel_RGB_values(
               pu,pv,Rbackground,Gbackground,Bbackground);
         } // loop over pu
      } // loop over pv
   }

// ==========================================================================
// Text image chip rotation methods
// ==========================================================================

   bool rotate_image(
      string image_filename, string rotated_images_subdir, 
      double max_pixel_width, string& rotated_image_filename,
      string phrase)
   {
//      cout << "inside textfunc::rotate_image()" << endl;

      if(!filefunc::fileexist(image_filename)) return false;

      string image_basename=filefunc::getbasename(image_filename);      

      bool non_rotate_flag = false;
      double non_rotate_frac = 0.40;
      if(nrfunc::ran1() < non_rotate_frac)
      {
         non_rotate_flag = true;
         string prefix = stringfunc::prefix(image_basename);
         string suffix = stringfunc::suffix(image_basename);
         image_basename=prefix+"_NoRot."+suffix;
      }
      
      if(phrase.size() > 0)
      {
         string prefix = stringfunc::prefix(image_basename);
         string suffix = stringfunc::suffix(image_basename);
         image_basename = prefix+"_"+phrase+"."+suffix;
      }
      
      rotated_image_filename=rotated_images_subdir+image_basename;

// For a nontrivial fraction of input images, do NOT apply any
// rotation:

      if(non_rotate_flag)
      {
         string unix_cmd="cp "+image_filename+" "+rotated_image_filename;
         sysfunc::unix_command(unix_cmd);
      }
      else
      {
         double FOV_u = (45 + 15 * nrfunc::gasdev());
         FOV_u = basic_math::max(FOV_u, 10.0);
         FOV_u = basic_math::min(FOV_u, 80.0);
         FOV_u *= PI/180;

         double aspect_ratio = 1.333 + 0.2 * nrfunc::gasdev();
         aspect_ratio = basic_math::max(0.5, aspect_ratio);
         aspect_ratio = basic_math::min(2.0, aspect_ratio);

         double img_az = 0 + 50 * nrfunc::gasdev();
         img_az = basic_math::max(img_az, -80.0);
         img_az = basic_math::min(img_az, 80.0);
         img_az *= PI/180;

         double img_el = 0 + 15 * nrfunc::gasdev();
         img_el = basic_math::max(img_el, -30.0);
         img_el = basic_math::min(img_el, 30.0);
         img_el *= PI/180;

         double img_roll = 0 + 3 * nrfunc::gasdev();
         img_roll = basic_math::max(img_roll, -15.0);
         img_roll = basic_math::min(img_roll, 15.0);
         img_roll *= PI/180;

         string background_color="none";
         textfunc::perspective_projection(
            FOV_u, aspect_ratio, max_pixel_width,
            img_az, img_el, img_roll, 
            image_filename, background_color, rotated_image_filename);
      } // non-rotation conditional

      bool rot_img_exists = filefunc::fileexist(rotated_image_filename);
      if(rot_img_exists)
      {
         return true;
      }
      else
      {
         cout << "Rotated image = " << rotated_image_filename
              << " rot_img_exists = " << rot_img_exists << endl;
         filefunc::deletefile(rotated_image_filename);
         return false;
      }
   }

// -------------------------------------------------------------------------
// Method rotate_image_and_mask() takes in image and corresponding
// mask filenames.  For some sizable fraction of calls, this method
// performs no rotation and simply copies the input image and mask to
// output files.  But for those cases where nontrivial rotation is
// performed, this method instantiates a virtual camera whose
// horizontal FOV and aspect ratio are random gaussian variables.  The
// image and mask are also rotated in 3 dimensions according to random
// az, el and roll gaussian variables.  After perspectively projecting
// the image and mask into the virtual camera, ImageMagick is used to
// generate and export their 2D imageplane renderings.

// If the rotated image and mask files are successfully exported to
// disk, this boolean method returns true.

   bool rotate_image_and_mask(
      string image_filename, string mask_filename,
      string rotated_images_subdir,string rotated_masks_subdir,
      double max_pixel_width)
   {
//      cout << "inside textfunc::rotate_image_and_mask()" << endl;

      if(!filefunc::fileexist(image_filename)) return false;
      if(!filefunc::fileexist(mask_filename)) return false;

      string image_basename=filefunc::getbasename(image_filename);      
      string rotated_image_filename=rotated_images_subdir+image_basename;

      string mask_basename=filefunc::getbasename(mask_filename);
      string rotated_mask_filename=rotated_masks_subdir+mask_basename;

// For a nontrivial fraction of input images, do NOT apply any
// rotation:

      double non_rotate_frac = 0.40;
      if(nrfunc::ran1() < non_rotate_frac)
      {
         rotated_image_filename=stringfunc::prefix(rotated_image_filename)+
            "_NoRot.png";
         string unix_cmd="cp "+image_filename+" "+rotated_image_filename;
         sysfunc::unix_command(unix_cmd);
         rotated_mask_filename=stringfunc::prefix(rotated_mask_filename)+
            "_NoRot.png";
         unix_cmd="cp "+mask_filename+" "+rotated_mask_filename;
         sysfunc::unix_command(unix_cmd);
      }
      else
      {
         double FOV_u = (45 + 15 * nrfunc::gasdev());
         FOV_u = basic_math::max(FOV_u, 10.0);
         FOV_u = basic_math::min(FOV_u, 80.0);
         FOV_u *= PI/180;

         double aspect_ratio = 1.333 + 0.2 * nrfunc::gasdev();
         aspect_ratio = basic_math::max(0.5, aspect_ratio);
         aspect_ratio = basic_math::min(2.0, aspect_ratio);

         double img_az = 0 + 50 * nrfunc::gasdev();
         img_az = basic_math::max(img_az, -80.0);
         img_az = basic_math::min(img_az, 80.0);
         img_az *= PI/180;

         double img_el = 0 + 15 * nrfunc::gasdev();
         img_el = basic_math::max(img_el, -30.0);
         img_el = basic_math::min(img_el, 30.0);
         img_el *= PI/180;

         double img_roll = 0 + 3 * nrfunc::gasdev();
         img_roll = basic_math::max(img_roll, -15.0);
         img_roll = basic_math::min(img_roll, 15.0);
         img_roll *= PI/180;

         string background_color="none";
         textfunc::perspective_projection(
            FOV_u, aspect_ratio, max_pixel_width,
            img_az, img_el, img_roll, 
            image_filename, background_color, rotated_image_filename);

         background_color="black";
         textfunc::perspective_projection(
            FOV_u, aspect_ratio, max_pixel_width,
            img_az, img_el, img_roll, 
            mask_filename, background_color, rotated_mask_filename);
      } // non-rotation conditional

      bool rot_img_exists = filefunc::fileexist(rotated_image_filename);
      bool rot_mask_exists= filefunc::fileexist(rotated_mask_filename);
      if(rot_img_exists && rot_mask_exists)
      {
         return true;
      }
      else
      {
         cout << "Rotated image = " << rotated_image_filename
              << " rot_img_exists = " << rot_img_exists << endl;
         cout << "Rotated mask = " << rotated_mask_filename
              << " rot_mask_exists = " << rot_mask_exists << endl;
         filefunc::deletefile(rotated_image_filename);
         filefunc::deletefile(rotated_mask_filename);
         return false;
      }
   }

// -------------------------------------------------------------------------
   bool rotate_image_and_masks(
      string image_filename, string mask1_filename, string mask2_filename,
      string rotated_images_subdir,string rotated_masks1_subdir,
      string rotated_masks2_subdir,double max_pixel_width,
      double max_abs_az, double max_abs_el, double max_abs_roll,
      string& rotated_image_filename, string& rotated_mask1_filename,
      string& rotated_mask2_filename)
   {
//      cout << "inside textfunc::rotate_image_and_masks()" << endl;

      if(!filefunc::fileexist(image_filename)) return false;
      if(!filefunc::fileexist(mask1_filename)) return false;
      if(!filefunc::fileexist(mask2_filename)) return false;

      string image_basename=filefunc::getbasename(image_filename);      
      rotated_image_filename=rotated_images_subdir+image_basename;

      string mask1_basename=filefunc::getbasename(mask1_filename);
      rotated_mask1_filename=rotated_masks1_subdir+mask1_basename;

      string mask2_basename=filefunc::getbasename(mask2_filename);
      rotated_mask2_filename=rotated_masks2_subdir+mask2_basename;

// For a nontrivial fraction of input images, do NOT apply any
// rotation:

      double non_rotate_frac = 0.30;

      if(nrfunc::ran1() < non_rotate_frac)
      {
         rotated_image_filename=stringfunc::prefix(rotated_image_filename)+
            "_NoRot.png";
         string unix_cmd="cp "+image_filename+" "+rotated_image_filename;
         sysfunc::unix_command(unix_cmd);

         rotated_mask1_filename=stringfunc::prefix(rotated_mask1_filename)+
            "_NoRot.png";
         unix_cmd="cp "+mask1_filename+" "+rotated_mask1_filename;
         sysfunc::unix_command(unix_cmd);

         rotated_mask2_filename=stringfunc::prefix(rotated_mask2_filename)+
            "_NoRot.png";
         unix_cmd="cp "+mask2_filename+" "+rotated_mask2_filename;
         sysfunc::unix_command(unix_cmd);
      }
      else
      {
         double FOV_u = (45 + 15 * nrfunc::gasdev());
         FOV_u = basic_math::max(FOV_u, 10.0);
         FOV_u = basic_math::min(FOV_u, 80.0);
         FOV_u *= PI/180;

         double aspect_ratio = 1.333 + 0.2 * nrfunc::gasdev();
         aspect_ratio = basic_math::max(0.5, aspect_ratio);
         aspect_ratio = basic_math::min(2.0, aspect_ratio);

         double img_az = 0 + 70 * nrfunc::gasdev();
         img_az = basic_math::max(img_az, -max_abs_az);
         img_az = basic_math::min(img_az, max_abs_az);
         img_az *= PI/180;

         double img_el = 0 + 15 * nrfunc::gasdev();
         img_el = basic_math::max(img_el, -max_abs_el);
         img_el = basic_math::min(img_el, max_abs_el);
         img_el *= PI/180;

         double img_roll = 0 + 5 * nrfunc::gasdev();
         img_roll = basic_math::max(img_roll, -max_abs_roll);
         img_roll = basic_math::min(img_roll, max_abs_roll);
         img_roll *= PI/180;

         string background_color="none";
         textfunc::perspective_projection(
            FOV_u, aspect_ratio, max_pixel_width,
            img_az, img_el, img_roll, 
            image_filename, background_color, rotated_image_filename);

         background_color="black";
         textfunc::perspective_projection(
            FOV_u, aspect_ratio, max_pixel_width,
            img_az, img_el, img_roll, 
            mask1_filename, background_color, rotated_mask1_filename);

         textfunc::perspective_projection(
            FOV_u, aspect_ratio, max_pixel_width,
            img_az, img_el, img_roll, 
            mask2_filename, background_color, rotated_mask2_filename);
      } // non-rotation conditional

      bool rot_img_exists = filefunc::fileexist(rotated_image_filename);
      bool rot_mask1_exists= filefunc::fileexist(rotated_mask1_filename);
      bool rot_mask2_exists= filefunc::fileexist(rotated_mask2_filename);
      if(rot_img_exists && rot_mask1_exists && rot_mask2_exists)
      {
         return true;
      }
      else
      {
         cout << "Rotated image = " << rotated_image_filename
              << " rot_img_exists = " << rot_img_exists << endl;
         cout << "Rotated mask1 = " << rotated_mask1_filename
              << " rot_mask1_exists = " << rot_mask1_exists << endl;
         cout << "Rotated mask2 = " << rotated_mask2_filename
              << " rot_mask2_exists = " << rot_mask2_exists << endl;
         filefunc::deletefile(rotated_image_filename);
         filefunc::deletefile(rotated_mask1_filename);
         filefunc::deletefile(rotated_mask2_filename);
         return false;
      }
   }

// -------------------------------------------------------------------------
// Method takes in the the horizontal field-of-view for a virtual
// camera along with the camera's aspect ratio.  The virtual camera
// points along +x_hat, and it is positioned along the negative
// world-X axis so that its field-of-view encompasses max_pixel_width
// in the X=0 world-plane.  This method also takes in the path for
// some image on disk.  It rotates this image in world-space about
// (0,0,0) by R = Rz(img_az) * Ry(-img_el) * Rx(img_roll).  The image
// plane's perspectively-projected pre-rotated and post-rotated pixel
// coordinates define a homography.  We pass these 8 corner
// coordinates into an ImageMagick system call which warps the input
// image into an output image.  ImageMagick's built-in 2D rotation is
// then used to perform a final roll.  The fully-rotated image is
// written as a PNG file to output_image_filename.

// Note: background_color = "none" for standard RGB input images
//       background_color = "black" for mask input images

   void perspective_projection(
      double FOV_u, double aspect_ratio, double max_pixel_width,
      double img_az, double img_el, double img_roll, 
      string input_image_filename, string background_color,
      string output_png_filename)
   {
//      cout << "inside textfunc::perspective_projection()" << endl;

// Make sure output is a png image:

      string suffix=stringfunc::suffix(output_png_filename);
      if(suffix=="png" || suffix=="PNG")
      {
      }
      else
      {
         cout << "Output image from textfunc::perspective_projection must be a PNG"
              << endl;
         exit(-1);
      }

      string unix_cmd="convert "+input_image_filename;
      unix_cmd += " -matte -virtual-pixel transparent ";
      unix_cmd += "-distort Perspective '";

      double FOV_v, f;
      camerafunc::f_and_vert_FOV_from_horiz_FOV_and_aspect_ratio(
         FOV_u, aspect_ratio, f, FOV_v);
      double u0 = 0.5 * aspect_ratio;
      double v0 = 0.5;
      camera virtual_cam(f, f, u0, v0);

// Orient virtual camera s.t. it points along +Xhat:

      virtual_cam.set_Rcamera(0,0,0); 

// Virtual camera's origin lies along -X_hat axis:

      double Xcamera = - 0.5 * max_pixel_width / tan(0.5 * FOV_u);
      threevector world_posn(Xcamera,0,0);
      virtual_cam.set_world_posn(world_posn);
      
      virtual_cam.construct_projection_matrix();

      threevector midpoint(0,0,0), projected_midpoint;
      virtual_cam.project_XYZ_to_UV_coordinates(
         midpoint,projected_midpoint);

      unsigned int chip_width, chip_height;
      imagefunc::get_image_width_height(
         input_image_filename, chip_width, chip_height);
      double halfW=0.5 * chip_width;
      double halfH=0.5 * chip_height;
//      cout << "chip_width = " << chip_width
//           << " chip_height = " << chip_height << endl;

// Set input image center's world position to (0,0,0) and to lie
// within YZ world-plane:

      vector<threevector> init_corners;
      init_corners.push_back(threevector(0, halfW, halfH));
      init_corners.push_back(threevector(0, halfW, -halfH));
      init_corners.push_back(threevector(0, -halfW, -halfH));
      init_corners.push_back(threevector(0, -halfW, halfH));

      rotation R;
      R = R.rotation_from_az_el_roll(img_az, img_el, 0);

//      cout << endl;
//      cout << "img_roll = " << 180/PI * img_roll << endl << endl;

// First project starting (unrotated) 3D corners into 2D image plane:

      vector<threevector> projected_init_corners;
      for(unsigned int c = 0; c < init_corners.size(); c++)
      {
         threevector projected_corner;
         virtual_cam.project_XYZ_to_UV_coordinates(
            init_corners[c], projected_corner);
         projected_init_corners.push_back(projected_corner);
      }

      double beta = chip_width / (projected_init_corners[3].get(0) - 
                                  projected_init_corners[0].get(0));
      double delta = chip_height / (projected_init_corners[0].get(1) - 
                                    projected_init_corners[1].get(1));
      
      for(unsigned int c = 0; c < init_corners.size(); c++)
      {
         double U0 = projected_init_corners[0].get(0);
         double V0 = projected_init_corners[0].get(1);
         int pu0 = beta * U0;
         int pv0 = delta * (1-V0);

         double U = projected_init_corners[c].get(0);
         double V = projected_init_corners[c].get(1);
         int rel_pu = beta * U;
         int rel_pv = delta * (1-V);

         int pu = rel_pu - pu0;
         int pv = rel_pv - pv0;

//         cout << "---------------------------------" << endl;
//         cout << "c = " << c 
//              << " init_corner: X = " << init_corners[c].get(0) 
//              << " Y = " << init_corners[c].get(1) 
//              << " Z = " << init_corners[c].get(2) 
//              << endl;
//         cout << "   UV projection: U = " << U
//              << " V = " << V << endl;
//         cout << "   pu = " << pu << " pv = " << pv << endl;

         unix_cmd += stringfunc::number_to_string(pu)+",";
         unix_cmd += stringfunc::number_to_string(pv)+" ";

// Project 3D rotated corner into 2D UV image plane.  Then compute
// projected points' corresponding pixel coordinates:

         threevector rotated_corner(R * init_corners[c]);
         threevector projected_rotated_corner;

         virtual_cam.project_XYZ_to_UV_coordinates(
            rotated_corner, projected_rotated_corner);

         U = projected_rotated_corner.get(0);
         V = projected_rotated_corner.get(1);
         rel_pu = beta * U;
         rel_pv = delta * (1-V);

         pu = rel_pu - pu0;
         pv = rel_pv - pv0;

//         cout << "    rotated_corner: X = " << rotated_corner.get(0) 
//              << " Y = " << rotated_corner.get(1) 
//              << " Z = " << rotated_corner.get(2) 
 //             << endl;
//         cout << "    UV projection: U = " << U
//              << " V = " << V << endl;
//         cout << "   pu = " << pu << " pv = " << pv << endl;

         unix_cmd += stringfunc::number_to_string(pu)+",";
         unix_cmd += stringfunc::number_to_string(pv)+" ";
      }

      string intermediate_png_filename="intermediate_"+
         filefunc::getbasename(output_png_filename);
      unix_cmd += "' png32:"+intermediate_png_filename;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

// Call ImageMagick's build-in 2D rotation command in order to roll
// image.  Note that output image generated via this call generally
// has a larger pixel-size than the input image:

      unix_cmd = "convert "+intermediate_png_filename
         +" -alpha set -background "+background_color;
      unix_cmd += " -rotate "+stringfunc::number_to_string(180/PI*img_roll)
         +" png32:"+output_png_filename;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      filefunc::deletefile(intermediate_png_filename);
   }

// -------------------------------------------------------------------------
// Method randomly_rotate_foreground_symbol() imports an image within
// *texture_rectangle_ptr as well as an orthonormal basis for
// a rotated coordinate system.  It positions the input image within
// the YZ plane.  Looping over each of its pixels, this method
// projects each YZ pixel along +x_hat until it intersects the rotated
// image plane.  The RGB values at the rotated image plane pixel are
// transfered to its YZ progenitor within
// *rotated_texture_rectangle_ptr.

   void randomly_rotate_foreground_symbol(
      double max_az, double max_el, double max_roll,
      colorfunc::RGB& text_background_RGB,
      const texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* rotated_texture_rectangle_ptr)
   {
      unsigned int width=texture_rectangle_ptr->getWidth();
      unsigned int height=texture_rectangle_ptr->getHeight();
      double u0=0.5*double(width)/double(height);
      double v0=0.5;

      threevector n_hat,uprime_hat,vprime_hat;
      generate_random_rotation(n_hat,uprime_hat,vprime_hat,
                               max_az, max_el, max_roll);
// Note: If rotation = identity, n_hat = X_hat.

      plane imageplane(n_hat,Zero_vector);

      for (unsigned int pv=0; pv<height; pv++)
      {
         double v=1-double(pv)/(height-1);
         double z=v-v0;

         for (unsigned int pu=0; pu<width; pu++)
         {
            double u=double(pu)/(height-1);
            double y=-(u-u0);

            threevector ray_basept(0,y,z);
            threevector intersection_pt;

            int R = text_background_RGB.first;
            int G = text_background_RGB.second;
            int B = text_background_RGB.third;
            if (!imageplane.infinite_line_intersection(
                   ray_basept,x_hat,intersection_pt))
            {
               cout << "Ray didn't intersect rotated plane" << endl;
               outputfunc::enter_continue_char();
            }
            else
            {
               double uprime=intersection_pt.dot(uprime_hat);
               double vprime=intersection_pt.dot(vprime_hat);
               uprime += u0;
               vprime += v0;

//               cout << "u = " << u << " uprime = " << uprime
//                    << " v = " << v << " vprime = " << vprime << endl;
               texture_rectangle_ptr->get_RGB_values(uprime,vprime,R,G,B);
            }
        
//         cout << "ray_basept = " << ray_basept << endl;
//         cout << "intersection_pt = " << intersection_pt << endl;
            rotated_texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
         } // loop over pu index
      } // loop over pv index
   }

// -------------------------------------------------------------------------
// Method generate_random_rotation() generates a random 3D
// rotation which turns face-on character image to non-nadir
// views.  It returns an orthonormal coordinate system for the rotated
// imageplane.

   void generate_random_rotation(
      threevector& n_hat, threevector& uprime_hat, threevector& vprime_hat,
      double max_az, double max_el, double max_roll)
   {
//   cout << "inside generate_random_rotation()" << endl;

      rotation R;
      double az=0+40*nrfunc::gasdev();
      double el=0+40*nrfunc::gasdev();
      double roll=0+20*nrfunc::gasdev();

//      double max_az = 30;
//      double max_el = 30;

// As of 2/3/16, we tighly restrict the roll angle's magnitude in
// order to lessen confusion between rotated 1s and nonrotated 7s,
// etc.  Eventually we will hopefully be able to relax this constraint
// if/when we're able to extract out horizontal text line angles...

//      double max_roll = 7.5;

      az=basic_math::max(az,-max_az);
      az=basic_math::min(az,max_az);
      el=basic_math::max(el,-max_el);
      el=basic_math::min(el,max_el);
      roll=basic_math::max(roll,-max_roll);
      roll=basic_math::min(roll,max_roll);
//      cout << "     az = " << az << " el = " << el << " roll = " << roll 
//           << endl;

      az *= PI/180;
      el *= PI/180;
      roll *= PI/180;
      R=R.rotation_from_az_el_roll(az,el,roll);
//      cout << "R = " << R << endl;

      n_hat=R*x_hat;
      uprime_hat=-R*y_hat;
      vprime_hat=R*z_hat;

//   cout << "n_hat = " << n_hat << endl;
//   cout << "uprime_hat = " << uprime_hat << endl;
//   cout << "vprime_hat = " << vprime_hat << endl;
   }

// ==========================================================================
// Text image chip corruption methods
// ==========================================================================

// Method add_gaussian_noise() fluctuates very RGB values for every
// pixel within a 3 or 4 channel texture rectangle.

   void add_gaussian_noise(
      texture_rectangle* texture_rectangle_ptr, double sigma)
   {
//      cout << "inside textfunc::add_gaussian_noise()" << endl;

      unsigned int width=texture_rectangle_ptr->getWidth();
      unsigned int height=texture_rectangle_ptr->getHeight();
      int n_channels = texture_rectangle_ptr->getNchannels();
//      cout << "width = " << width << " height = " << height << endl;
//      cout << "n_channels = " << n_channels << endl;

      for (unsigned int pv=0; pv<height; pv++)
      {
         for (unsigned int pu=0; pu<width; pu++)
         {
            int R,G,B,A;
            
            if(n_channels == 3)
            {
               texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
            }
            else if (n_channels == 4)
            {
               texture_rectangle_ptr->get_pixel_RGBA_values(pu,pv,R,G,B,A);
            }

            int Rnew=colorfunc::fluctuate_value(R, sigma);
            int Gnew=colorfunc::fluctuate_value(G, sigma);
            int Bnew=colorfunc::fluctuate_value(B, sigma);

            if(n_channels == 3)
            {
               texture_rectangle_ptr->set_pixel_RGB_values(
                  pu,pv,Rnew,Gnew,Bnew);
            }
            else if (n_channels == 4)
            {
               int Anew = A;
               texture_rectangle_ptr->set_pixel_RGBA_values(
                  pu,pv,Rnew,Gnew,Bnew,Anew);
            }
         } // loop over pu
      } // loop over pv
   }

// -------------------------------------------------------------------------
// Method add_foreground_occlusions() randomly selects an occlusion
// size ranging from 2% - 10% of the input texture rectangle's width.
// It then either creates an opaque vertical rectangle, horizontal
// line or circle of that size which can intentionally block some
// foreground text content.

   void add_foreground_occlusions(texture_rectangle* texture_rectangle_ptr)
   {
//      cout << "inside textfunc::add_foreground_occlusions()" << endl;

      int width = texture_rectangle_ptr->getWidth();
      int height = texture_rectangle_ptr->getHeight();
      int n_channels = texture_rectangle_ptr->getNchannels();
      bool normalized_RGB_values = false;
      colorfunc::RGB occlusion_RGB = colorfunc::generate_random_RGB(
         normalized_RGB_values, 0, 360, 0, 0.5, 0, 0.5);

      int occlusion_width = 0, px_occlusion_start = 0;
      int occlusion_height, py_occlusion_start;
      int pu_center = 0, pv_center = 0;

      bool vertical_pole_occlusion = false;
      bool horizontal_line_occlusion = false;
      bool circular_occlusion = false;

      double random_occlusion = nrfunc::ran1();
      if(random_occlusion > 0.25 && random_occlusion < 0.75)
      {
         vertical_pole_occlusion = true;
         int min_occlusion_width = basic_math::min(1.0, 0.02 * width);
         int max_occlusion_width = 0.1 * width;
         occlusion_width = min_occlusion_width + nrfunc::ran1() * 
            (max_occlusion_width - min_occlusion_width);
         px_occlusion_start = 0.99 * nrfunc::ran1() * 
            (width - occlusion_width);
      }
      else if (random_occlusion >= 0.75)
      {
         horizontal_line_occlusion = true;
         int min_occlusion_height = basic_math::min(1.0, 0.2 * height);
         int max_occlusion_height = 0.1 * height;
         occlusion_height = min_occlusion_height + nrfunc::ran1() * 
            (max_occlusion_height - min_occlusion_height);
         py_occlusion_start = 0.99 * nrfunc::ran1() * (
            height - occlusion_height);
      }
      else
      {
         circular_occlusion = true;
         pu_center = nrfunc::ran1() * width;
         pv_center = nrfunc::ran1() * height;

         int min_occlusion_width = basic_math::min(1.0, 0.02 * width);
         int max_occlusion_width = 0.1 * width;
         occlusion_width = min_occlusion_width + nrfunc::ran1() * 
            (max_occlusion_width - min_occlusion_width);
      }

      for (int pv=0; pv<height; pv++)
      {
         if(horizontal_line_occlusion)
         {
            if(pv < py_occlusion_start || pv >= py_occlusion_start + 
               occlusion_height) continue;
         }
         
         for (int pu=0; pu<width; pu++)
         {
            if(vertical_pole_occlusion)
            {
               if(pu < px_occlusion_start || pu >= px_occlusion_start + 
                  occlusion_width) continue;
            }
            else if(circular_occlusion)
            {
               double rsq = sqr(pu - pu_center) + sqr(pv - pv_center);
               if(rsq > sqr(occlusion_width)) continue;
            } 

            if(n_channels == 3)
            {
               texture_rectangle_ptr->set_pixel_RGB_values(
                  pu, pv, occlusion_RGB.first, occlusion_RGB.second,
                  occlusion_RGB.third);
            }
            else if (n_channels == 4)
            {
               texture_rectangle_ptr->set_pixel_RGBA_values(
                  pu, pv, occlusion_RGB.first,
                  occlusion_RGB.second, occlusion_RGB.third, 255);
            }
         } // loop over pu
      } // loop over pv
   }

// -------------------------------------------------------------------------
   int shade_value(int R,double alpha,const twovector& q,
                   const twovector& origin,const twovector& e_hat)
   {
      twovector p(q-origin);
      twovector p_perp(p-p.dot(e_hat)*e_hat);
      twovector f_hat(-e_hat.get(1),e_hat.get(0));
      double d=p_perp.dot(f_hat);

      R += alpha*d;
      R = basic_math::max(0,R);
      R = basic_math::min(255,R);
      return R;
   }

// -------------------------------------------------------------------------
   double random_gaussian_blur_sigma(
      double udim_lo, double udim_hi, double udim)
   {
      double sigma = 0;

// Don't blur if udim is too small:

      if(udim < 28)
      {
         return sigma;
      }

      double blur_flag=nrfunc::ran1();
      if (blur_flag < 0.4)
      {
         sigma=0;
      }
      else if (blur_flag < 0.5)
      {
         sigma=0.2;
      }
      else if (blur_flag < 0.6)
      {
         sigma=0.4;
      }
      else if (blur_flag < 0.7)
      {
         sigma=0.7;
      }
      else if (blur_flag < 0.8)
      {
         sigma=1.1;
      }
      else if (blur_flag < 0.9)
      {
         sigma=1.6;
      }
      else 
      {
         sigma=2.2;
      }

// Soften blurring for small image chips:

      double udim_frac = (udim - udim_lo) / (udim_hi - udim_lo);
      sigma  = 0.7 * sigma + udim_frac * 0.3 * sigma;
      return sigma;
   }

// -------------------------------------------------------------------------
// Method randomly_crop_chip() 

   void randomly_crop_chip(string chip_filename)
   {
// As of 1/18/16, we believe that calling this method is expensive.
// So we only do so for 50% of all input image chips:

      if(nrfunc::ran1() < 0.5) return;

      unsigned xdim, ydim;
      imagefunc::get_image_width_height(chip_filename,xdim, ydim);
      unsigned int width = (0.85 + nrfunc::ran1() * 0.15) * xdim;
      unsigned int height = (0.85 + nrfunc::ran1() * 0.15) * ydim;
      unsigned int xoffset = nrfunc::ran1() * (xdim - width);
      unsigned int yoffset = nrfunc::ran1() * (ydim - height);
      width = basic_math::min(width, xdim - xoffset);
      height = basic_math::min(height, ydim - yoffset);
      if (width != xdim || height != ydim)
      {
         imagefunc::crop_image(chip_filename, width, height, xoffset, yoffset);
      }
   }

// -------------------------------------------------------------------------
// Method synthesize_solar_shadow() picks a random line passing
// somewhere through the input texture rectangle.  The intensities for
// all pixels lying below this line are decreased by a constant
// fraction (50%) relative to their original values.  The resulting
// image (hopefully) simulates a solar shadow.

   void synthesize_solar_shadow(texture_rectangle* texture_rectangle_ptr)
   {
      double shadow_frac = 0.5;

      int xdim = texture_rectangle_ptr->getWidth();
      int ydim = texture_rectangle_ptr->getHeight();
      int n_channels = texture_rectangle_ptr->getNchannels();

// Randomly pick pixel through which shadow line passes:

      double px_shadow = nrfunc::ran1() * xdim;
      double py_shadow = nrfunc::ran1() * ydim;
      
// Randomly pick shadow line's angle:

      double theta = PI/180 * (-89 + nrfunc::ran1() * 178);
      double cos_theta = cos(theta);
      double sin_theta = sin(theta);

      for(int px = 0; px < xdim; px++)
      {
         double lambda = (px - px_shadow) / cos_theta;
         int py = py_shadow + lambda * sin_theta;
         
         for(int qy = py; qy < ydim; qy++)
         {
            double h, s, v;
            if(n_channels == 3)
            {
               texture_rectangle_ptr->get_pixel_hsv_values(px, qy, h, s, v);
               texture_rectangle_ptr->set_pixel_hsv_values(
                  px, qy, h, s, shadow_frac * v); 
            }
            else if (n_channels == 4)
            {
               double a;
               texture_rectangle_ptr->get_pixel_hsva_values(px, qy, h, s, v, a);
               texture_rectangle_ptr->set_pixel_hsva_values(
                  px, qy, h, s, shadow_frac * v, a); 
            }
         } // loop over qy
      } // loop over px
   }
   
// ==========================================================================
// Deep learning preparation methods
// ==========================================================================

// Method compute_px_COM

   double compute_px_COM(texture_rectangle* texture_rectangle_ptr)
   {
      int n_channels = texture_rectangle_ptr->getNchannels();
      double px_COM = 0, py_COM = 0;
      unsigned int xdim = texture_rectangle_ptr->getWidth();
      unsigned int ydim = texture_rectangle_ptr->getHeight();
      int denom = 1;
      for(unsigned int py = 0; py < ydim; py++)
      {
         for(unsigned int px = 0; px < xdim; px++)
         {
            if(n_channels == 1)
            {
               int curr_v = texture_rectangle_ptr->
                  fast_get_pixel_intensity_value(px, py);
               if (curr_v > 0)
               {
                  px_COM += double(px)/xdim - 0.5;
                  py_COM += double(py)/ydim - 0.5;
               }
            }
            else if (n_channels == 3)
            {
               int R, G, B;
               texture_rectangle_ptr->
                  fast_get_pixel_RGB_values(px, py, R, G, B);
               if (R+G+B > 0)
               {
                  px_COM += double(px)/xdim - 0.5;
                  py_COM += double(py)/ydim - 0.5;
                  denom++;
               }
            }
            else
            {
               cout << "n_channels = " << n_channels << endl;
               outputfunc::enter_continue_char();
            }
            
         }
      }
//   cout << "Init:  px_COM = " << px_COM << " py_COM = " << py_COM << endl;
      px_COM /= denom;
      py_COM /= denom;
//   cout << "px_COM = " << px_COM << " py_COM = " << py_COM << endl;
      return px_COM;
   }

// -------------------------------------------------------------------------
   bool is_foreground_pixel(
      bool dark_text_background, int min_value, 
      int Xforeground, int Xbackground)
   {
      bool foreground_pixel = false;
      if(dark_text_background && Xforeground >= 0 + 0.5 * min_value)
      {
         foreground_pixel = true;
      }
      else if(!dark_text_background && Xforeground <= 255 - 0.5 * min_value)
      {
         foreground_pixel = true;
      }
      return foreground_pixel;
   }

// -------------------------------------------------------------------------
// Method generate_segmentation_mask() overwrites the
// input character image's (mostly) black or white backgrounds with
// black pixels.  

   bool generate_segmentation_mask(
      colorfunc::RGBA text_background_RGBA,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* background_texture_rectangle_ptr,
      texture_rectangle* mask_texture_rectangle_ptr)
   {
//      cout << "inside generate_segmentation_mask()" << endl;

      unsigned int width=texture_rectangle_ptr->getWidth();
      unsigned int height=texture_rectangle_ptr->getHeight();
      int n_channels = texture_rectangle_ptr->getNchannels();
      bool dark_text_background = true;
      if(text_background_RGBA.first > 128) dark_text_background = false;

      int n_white_pixels = 0;

//      const int min_value = 10;
      const int min_value = 25;
//      const int min_value = 50;
//      const int max_value = 255 - min_value;

      for (unsigned int pv=0; pv<height; pv++)
      {
         for (unsigned int pu=0; pu<width; pu++)
         {
            int Rforeground, Gforeground, Bforeground;
            if(n_channels == 3)
            {
               texture_rectangle_ptr->get_pixel_RGB_values(
                  pu, pv, Rforeground, Gforeground, Bforeground);
            }
            else if (n_channels == 4)
            {
               int Aforeground;
               texture_rectangle_ptr->get_pixel_RGBA_values(
                  pu, pv, Rforeground, Gforeground, Bforeground, Aforeground);
            }
            
            int Rbackground, Gbackground, Bbackground;
            background_texture_rectangle_ptr->get_pixel_RGB_values(
               pu, pv, Rbackground, Gbackground, Bbackground);

            bool foreground_pixel = 
               is_foreground_pixel(
                  dark_text_background, min_value, Rforeground, Rbackground) ||
               is_foreground_pixel(
                  dark_text_background, min_value, Gforeground, Gbackground) ||
               is_foreground_pixel(
                  dark_text_background, min_value, Bforeground, Bbackground);

            int Rmask = 0, Gmask = 0, Bmask = 0;
            if(foreground_pixel)
            {
               Rmask = Gmask = Bmask = 255;
               n_white_pixels++;
            }
            mask_texture_rectangle_ptr->set_pixel_RGB_values(
               pu, pv, Rmask, Gmask, Bmask);
         } // loop over pu
      } // loop over pv

      double white_pixel_frac = double(n_white_pixels)/(width * height);

// On 3/10/16, we empirically observed that masks sometimes contain
//  zero white pixel content.  In this case, this boolean method
//  returns false:

      return (white_pixel_frac > 0.001);
   }

// -------------------------------------------------------------------------
// Method check_mask_values() iterates over all pixels within the
// character and word masks specified by their respective input
// filenames.  It counts the number of character {word} mask values
// that lie outside the interval [min_char{word}mask_value,
// max_char{word}mask_value].  If the total number of bad mask values
// is less than a small threshold, the bad mask values are reset to 0
// and this boolean method returns true. Otherwise, the triple
// {image_filename, char_mask_filename, word_mask_filename} is deleted
// from disk.

   bool check_mask_values(
      string image_filename, string char_mask_filename, 
      string word_mask_filename, 
      int min_charmask_value, int max_charmask_value,
      int min_wordmask_value, int max_wordmask_value)
   {
      texture_rectangle charmask_tr(char_mask_filename,NULL);
      texture_rectangle wordmask_tr(word_mask_filename,NULL);

      int nonzero_charmask_values = 0;
      int nonzero_wordmask_values = 0;
      int xdim = charmask_tr.getWidth();
      int ydim = charmask_tr.getHeight();
      int R,G,B,A;
      int n_bad_charmask_values = 0, n_bad_wordmask_values = 0;
      for(int py = 0; py < ydim; py++)
      {
         for(int px = 0; px < xdim; px++)
         {
            charmask_tr.get_pixel_RGBA_values(px,py,R,G,B,A);
            if(R < min_charmask_value || R > max_charmask_value) 
            {
               charmask_tr.set_pixel_RGBA_values(px,py,0,0,0,255);
               n_bad_charmask_values++;
            }
            else if(R > 0) 
            {
               nonzero_charmask_values++;
            }

            wordmask_tr.get_pixel_RGBA_values(px,py,R,G,B,A);
            if(R < min_wordmask_value || R > max_wordmask_value) 
            {
               wordmask_tr.set_pixel_RGBA_values(px,py,0,0,0,255);
               n_bad_wordmask_values++;
            }
            else if(R > 0)
            {
               nonzero_wordmask_values++;
            }
         } // loop over px
      } // loop over py

      double bad_charmask_values_frac = double(n_bad_charmask_values) / 
         nonzero_charmask_values;
      if(n_bad_charmask_values > 0)
      {
//         cout << "n_bad_charmask_values = " << n_bad_charmask_values
//              << endl;
      }

      double bad_wordmask_values_frac = double(n_bad_wordmask_values) / 
         nonzero_wordmask_values;

      if(n_bad_wordmask_values > 0)
      {
//         cout << "n_bad_wordmask_values = " << n_bad_wordmask_values
//              << endl;
      }

//      cout << endl;
//      cout << "nonzero_charmask_values = "
//           << nonzero_charmask_values 
//           << " bad_charmask_values_frac = "
//           << bad_charmask_values_frac << endl;
//      cout << "nonzero_wordmask_values = "
//           << nonzero_wordmask_values 
//           << " bad_wordmask_values_frac = "
//           << bad_wordmask_values_frac 
//           << endl;

      if(nonzero_charmask_values == 0 || nonzero_wordmask_values == 0 ||
         bad_charmask_values_frac > 0.01 || bad_wordmask_values_frac > 0.01)
      {
         filefunc::deletefile(image_filename);
         filefunc::deletefile(char_mask_filename);
         filefunc::deletefile(word_mask_filename);
//         cout << "Deleted " << image_filename << " , " 
//              << char_mask_filename << " , " << word_mask_filename
//              << endl;
         return false;
      }

// Re-export char and/or word masks if they had any bad mask values
// which have been reset to zero:

      if(bad_charmask_values_frac > 0)
      {
         charmask_tr.write_curr_frame(char_mask_filename);
      }
      if(bad_wordmask_values_frac > 0)
      {
         wordmask_tr.write_curr_frame(word_mask_filename);
      }
      return true;
   }

// ==========================================================================
// Text line processing
// ==========================================================================

// Method select_text_phrase() takes in a (large) set of cleaned
// strings which contain variable numbers of words.  It randomly
// returns one cleaned string with the phrase member of curr_imagetext
// whose number of words lies within the interval [min_n_words,
// max_n_words].

   void select_text_phrase(vector<string>& cleaned_strings, int min_n_words,
                           int max_n_words, imagetext& curr_imagetext)
   {
      int min_n_chars = 1;
      int max_n_chars = 1000;
      select_text_phrase(cleaned_strings, min_n_words, max_n_words,
                         min_n_chars, max_n_chars, curr_imagetext);
   }

   void select_text_phrase(vector<string>& cleaned_strings, int min_n_words,
                           int max_n_words, int min_n_chars, int max_n_chars,
                           imagetext& curr_imagetext)
   {
//      cout << "textfunc::inside select_text_phrase() " << endl;
      int iter = 0, max_iter = 20;
      bool OK_n_words_flag = false;
      while(!OK_n_words_flag && iter < max_iter)
      {
         curr_imagetext.set_spaced_out_skip(0);
         curr_imagetext.set_phrase(
            cleaned_strings[nrfunc::ran1() * cleaned_strings.size()] );
         if (curr_imagetext.get_n_words() >= min_n_words &&
             curr_imagetext.get_n_words() <= max_n_words &&
             curr_imagetext.get_n_characters() >= min_n_chars &&
             curr_imagetext.get_n_characters() <= max_n_chars)
         {
            OK_n_words_flag = true;
         }
      }

      if(curr_imagetext.get_vertical_text_flag()) return;

// For some nonegligible fraction of text labels, we intentionally
// introduce spaces between all characters:

      if(nrfunc::ran1() < 0.15 && curr_imagetext.get_phrase().size() > 1)
      {

// First copy curr_imagetext.char_types into temporary array before
// it's cleared:
         
         vector<imagetext::char_type> un_spaced_out_char_types;
         for(int c = 0; c < curr_imagetext.get_n_characters(); c++)
         {
            un_spaced_out_char_types.push_back(
               curr_imagetext.get_char_type(c));
         }

         int skip = 1;
         if(nrfunc::ran1() > 0.5)
         {
            skip = 2;
         }

         curr_imagetext.set_spaced_out_skip(skip);
         
         curr_imagetext.set_phrase(
            textfunc::spaced_out_text_label(
               curr_imagetext.get_phrase(),skip), un_spaced_out_char_types);
      }

// On Tues Apr 5, 2016, GENERATE_STRING_IMAGES died due to an
// unterminated quote string.  So we explicitly print out any phrase
// containing single or double quotes:

      bool phrase_contains_quotes = false;
      for(int c = 0; c < curr_imagetext.get_n_characters(); c++)
      {
         string curr_char=curr_imagetext.get_char(c);
         if(curr_char == "'" || curr_char == "\"" || curr_char=="`")
         {
            phrase_contains_quotes = true;
            break;
         }
      }

      if(phrase_contains_quotes)
      {
         cout << "Phrase = " << curr_imagetext.get_phrase() << endl;
      }
   }
 
// ---------------------------------------------------------------------
// Method n_rendered_text_lines() assumes increase jumps within
// image_height of a rendered text line as a function of character
// number indicates a new rendered text line.  It returns the total
// number of such text image height jumps as the number of rendered
// text lines.

   void n_rendered_text_lines(imagetext& curr_imagetext)
   {
      int substr_width, substr_height;
      int prev_image_height = -1;
      curr_imagetext.set_n_textlines(1);

      if(curr_imagetext.get_vertical_text_flag())
      {
         curr_imagetext.set_n_textlines(curr_imagetext.get_n_characters());
         return ;
      }

      for(int c = 0; c < curr_imagetext.get_n_characters(); c++)
      {
         string curr_substr = curr_imagetext.get_phrase().substr(0,c+1);
         int output_img_width, output_img_height;
         retrieve_substr_and_image_widths_heights(
            curr_imagetext.get_image_width(), -1, curr_substr, curr_imagetext, 
            substr_width, substr_height, output_img_width, output_img_height);
         if(c > 0)
         {
            if(output_img_height > prev_image_height)
            {
               curr_imagetext.set_n_textlines(
                  curr_imagetext.get_n_textlines()+1);
            }
         }
         prev_image_height = output_img_height;

//         cout << "c = " << c
//              << " curr_char = " << curr_imagetext.get_phrase().substr(c,1)
//              << " img_width = " << output_img_width
//              << " output_img_height = " << output_img_height
//              << " n_text_lines = " << curr_imagetext.get_n_textlines()
//              << endl;
      }

/*
// Another possible method for estimating number of text lines: 

      cout << "curr_imagetext.char_height = " 
           << curr_imagetext.get_char_height() << endl;
      cout << "curr_imagetext.get_image_height() = "
           << curr_imagetext.get_image_height() << endl;
      cout << "image height / char height = "
           << double(curr_imagetext.get_image_height()) / 
         double(curr_imagetext.get_char_height()) << endl;
      cout << "curr_imagetext.get_n_textlines() = "
           << curr_imagetext.get_n_textlines() << endl;
*/
   }

// ---------------------------------------------------------------------

// Note added on Tues Apr 5 at 12:40 pm

// This method needs to be generalized to handle strings containing
// carriage returns!

   bool extract_char_widths_and_height(
      int img_width,int img_height, imagetext& curr_imagetext)
   {
//      cout << "inside textfunc::extract_char_widths_and_height() " << endl;

      int prev_substr_width = 0;
      int non_space_char = -1;
      int n_space_chars = 0;	 // Number of individual space chars within
				 //  a potentially large white space
      string label = curr_imagetext.get_phrase();
      int n_chars = curr_imagetext.get_n_characters();
      vector<int> substr_widths;

      for(int c = 0; c < n_chars; c++)
      {
         string curr_substr = label.substr(0,c+1);
         string curr_char = curr_imagetext.get_char(c);

         if(curr_char==" ")
         {
            n_space_chars++;
            if(non_space_char == -1)
            {
               non_space_char = c - 1;
            }
//            cout << "Encountered space char; non_space_char = "
//                 << non_space_char << endl;
            continue;
         }

//         cout << "c = " << c 
//              << " curr_char = " << curr_char
//              << " curr_substr = " << curr_substr 
//              << " curr_substr.size() = " << curr_substr.size() 
//              << endl;

         int substr_width, substr_height;
         textfunc::retrieve_substr_width_height(
            img_width, img_height, curr_substr, curr_imagetext, 
            substr_width, substr_height);
         curr_imagetext.set_char_height(substr_height);

         int curr_char_width = -1, curr_char_height;
//         cout << "non_space_char = " << non_space_char << endl;
         if(non_space_char >= 0)
         {
            textfunc::retrieve_substr_width_height(
               img_width, img_height, curr_char, curr_imagetext, 
               curr_char_width, curr_char_height);
            int space_width = substr_width - curr_char_width 
               - substr_widths.back();
//            cout << "Number of char widths before appending space_width = "
//                 << curr_imagetext.get_char_widths().size()
//                 << endl;
            for(int s = 0; s < n_space_chars; s++)
            {
               int individual_space_width = space_width / n_space_chars;
               if(s == n_space_chars - 1)
               {
                  individual_space_width = space_width - 
                     s * (space_width / n_space_chars);
               }
               curr_imagetext.get_char_widths().push_back(
                  individual_space_width);
            }
            n_space_chars = 0;

            substr_widths.push_back(substr_width - curr_char_width);
            prev_substr_width = substr_widths.back();
            non_space_char = -1;
         }

         substr_widths.push_back(substr_width);
         curr_char_width = substr_width - prev_substr_width;
         if(curr_char_width <= 0) 
         {
//            cout << "inside textfunc::extract_char_widths_and_height() " 
//                 << endl;
//            cout << "label = " << label << endl;
//            cout << "curr_char = " << curr_char << endl;
//            cout << "c = " << c << " n_chars - 1 = " << n_chars - 1
//                 << " curr_char_width = " << curr_char_width << endl;
            return false;
         }
            
//         cout << "substr_width = " << substr_width
//              << " curr_char_width = " << curr_char_width
//              << " c = " << c << " char(c) = " << curr_imagetext.get_char(c)
//              << " char_height = " << curr_imagetext.get_char_height()
//              << endl;
//         cout << "Number of char widths before appending char_width = "
//              << curr_imagetext.get_char_widths().size()
//              << endl;
            
         curr_imagetext.get_char_widths().push_back(curr_char_width);
         prev_substr_width = substr_width;
      } // loop over index c labeling chars inside label

      if(int(curr_imagetext.get_char_widths().size()) != 
         curr_imagetext.get_n_characters())
      {
//         cout << "At end of extract_char_widths_and_heights()" << endl;
//         cout << "curr_imagetext.get_char_widths().size()  = "
//              << curr_imagetext.get_char_widths().size() << endl;
//         cout << "curr_imagetext.get_n_characters()  = "
//              << curr_imagetext.get_n_characters() << endl;
       return false;
      }
      return true;
   }

// ---------------------------------------------------------------------

   void retrieve_substr_width_height(
      int input_img_width, int input_img_height, string substr, 
      const imagetext& curr_imagetext,
      int& substr_width, int& substr_height)
   {
      int output_img_width, output_img_height;
      retrieve_substr_and_image_widths_heights(
         input_img_width, input_img_height, substr, curr_imagetext,
         substr_width, substr_height, output_img_width, output_img_height);
   }

   void retrieve_substr_and_image_widths_heights(
      int input_img_width, int input_img_height, string substr, 
      const imagetext& curr_imagetext,
      int& substr_width, int& substr_height,
      int& output_img_width, int& output_img_height)
   {
      string chars_info_filename="./charinfo_"+stringfunc::number_to_string(
         curr_imagetext.get_ID())+".dat";
      int foreground_R = curr_imagetext.get_foreground_RGB().first;
      int foreground_G = curr_imagetext.get_foreground_RGB().second;
      int foreground_B = curr_imagetext.get_foreground_RGB().third;

      int stroke_R = curr_imagetext.get_stroke_RGB().first;
      int stroke_G = curr_imagetext.get_stroke_RGB().second;
      int stroke_B = curr_imagetext.get_stroke_RGB().third;

      int background_R = curr_imagetext.get_background_RGBA().first;
      int background_G = curr_imagetext.get_background_RGBA().second;
      int background_B = curr_imagetext.get_background_RGBA().third;
      int background_A = curr_imagetext.get_background_RGBA().fourth;

      int undercolor_R = curr_imagetext.get_undercolor_RGB().first;
      int undercolor_G = curr_imagetext.get_undercolor_RGB().second;
      int undercolor_B = curr_imagetext.get_undercolor_RGB().third;
      
      string unix_cmd = imagefunc::generate_ImageMagick_text_convert_cmd(
         curr_imagetext.get_debug_annotate_flag(),
         curr_imagetext.get_randomize_gravity_flag(),
         foreground_R, foreground_G, foreground_B, 
         stroke_R, stroke_G, stroke_B, 
         background_R, background_G, background_B, background_A,
         curr_imagetext.get_underbox_flag(), 
         undercolor_R, undercolor_G, undercolor_B, 
         curr_imagetext.get_strokewidth(), 
         curr_imagetext.get_font(), 
         curr_imagetext.get_pointsize(),
         input_img_width, input_img_height, substr,
         curr_imagetext.get_drop_shadow_flag());

      unix_cmd += " -trim info: > "+chars_info_filename;
      sysfunc::unix_command(unix_cmd);

      filefunc::ReadInfile(chars_info_filename);
      string curr_line = filefunc::text_line[0];
      int start_posn = 16 + substr.size();
      string info_substr = curr_line.substr(
         start_posn, curr_line.size() - start_posn);
//      cout << "info_substr = " << info_substr << endl;
      string separator_chars="x+ ";
      vector<string> info_substrings = 
         stringfunc::decompose_string_into_substrings(
            info_substr, separator_chars);

      substr_width = stringfunc::string_to_number(info_substrings[0]);
      substr_height = stringfunc::string_to_number(info_substrings[1]);
      output_img_width = stringfunc::string_to_number(info_substrings[2]);
      output_img_height = stringfunc::string_to_number(info_substrings[3]);
   }

// ---------------------------------------------------------------------
// Notes: Need to robustly find py values corresponding to breaks
// between lines in text images containing >=2 text lines.  Naive
// version below is NOT sufficiently robust.

   bool text_line_vertical_separations(imagetext& curr_imagetext)
   {
//      cout << "inside textfunc::text_line_vertical_separations()" << endl;

      texture_rectangle curr_tr(curr_imagetext.get_text_image_filename(), 
                                NULL);      
//      cout << " n_channels = " << curr_tr.getNchannels() << endl;
      int img_width = curr_tr.getWidth();
      int img_height = curr_tr.getHeight();
      unsigned int n_textlines = curr_imagetext.get_n_textlines();

//      cout << "img_width = " << img_width
//           << " img_height = " << img_height << endl;
//      cout << "n_textlines = " << n_textlines << endl;

      int undercolor_R, undercolor_G, undercolor_B;
      undercolor_R = undercolor_G = undercolor_B = -1;
      if(curr_imagetext.get_underbox_flag())
      {
         undercolor_R = curr_imagetext.get_undercolor_RGB().first;
         undercolor_G = curr_imagetext.get_undercolor_RGB().second;
         undercolor_B = curr_imagetext.get_undercolor_RGB().third;
      }

      if(n_textlines == 1)
      {
         curr_imagetext.get_py_tops().push_back(0);
         curr_imagetext.get_py_bottoms().push_back(img_height - 1);
         return true;
      }

      vector<double> x_profile, nonzero_x_profile;
      for(int py = 0; py < img_height; py++)
      {
         double x_integral = 0;
         for(int px = 0; px < img_width; px++)
         {
            int R, G, B, A;
            curr_tr.get_pixel_RGBA_values(px, py, R, G, B, A);
            if(A == 0) continue;
            if(R == undercolor_R && G == undercolor_G && B == undercolor_B)
               continue;

            double curr_x_contrib = (R+G+B)/(3*255.0);
            x_integral += curr_x_contrib;
         } // loop over px
         x_profile.push_back(x_integral);
         if(x_integral > 0)
         {
            nonzero_x_profile.push_back(x_integral);
         }
//         cout << "py = " << py << " x_profile = " << x_profile.back() << endl;
      } // loop over py

// To avoid counting the dot on an "i" or "j" as a separate text line,
// reset to zero any x_profile value less than 1% of nonzero median
// value:

      double median_x_profile = mathfunc::median_value(nonzero_x_profile);
//      cout << "median_x_profile = " << median_x_profile << endl;
      for(unsigned int py = 0; py < x_profile.size(); py++)
      {
         if(x_profile[py] < 0.01 * median_x_profile)
         {
            x_profile[py] = 0;
         }
//         cout << "py = " << py << " revised x_profile = " 
//              << x_profile[py] << endl;
      }

// Add top row to py_tops() if its x profile != 0:

      if(!nearly_equal(x_profile[0],0))
         curr_imagetext.get_py_tops().push_back(0);

      for(unsigned int py = 0; py < x_profile.size() - 1; py++)
      {
         if(nearly_equal(x_profile[py],0) && !nearly_equal(x_profile[py+1],0))
         {
            curr_imagetext.get_py_tops().push_back(py);
         }
      }

      if(curr_imagetext.get_vertical_text_flag())
      {
         if(curr_imagetext.get_py_tops().size() + 
            curr_imagetext.get_n_whitespaces() != n_textlines)
         {
            return false;
         }
         else
         {

// Reset n_textlines --> curr_imagetext.get_py_tops.size():

            curr_imagetext.set_n_textlines(
               curr_imagetext.get_py_tops().size());
            n_textlines = curr_imagetext.get_n_textlines();
         }
      }
      else
      {
         if(curr_imagetext.get_py_tops().size() != n_textlines)
         {
//            cout << "phrase = " << curr_imagetext.get_phrase() << endl;
//            cout << "curr_imagetext.get_n_chars() = "
//                 << curr_imagetext.get_n_characters() << endl;
//            cout << "n_textlines = " << n_textlines << endl;
//            cout << "py_tops.size = " << curr_imagetext.get_py_tops().size() 
//                 << endl;
            return false;
         }
      }
      
      for(unsigned int py = 1; py < x_profile.size(); py++)
      {
         if(!nearly_equal(x_profile[py-1],0) && nearly_equal(x_profile[py],0))
         {
            curr_imagetext.get_py_bottoms().push_back(py);
         }
      }

// Add bottom row to py_bottoms() if its x profile != 0:

      if(!nearly_equal(x_profile[x_profile.size()-1],0))
         curr_imagetext.get_py_bottoms().push_back(x_profile.size()-1);

      if (curr_imagetext.get_py_bottoms().size() != n_textlines)
      {
         return false;
      }

//      cout << "n_textlines = " << n_textlines << endl;
//      cout << "py_bottoms.size() = " << curr_imagetext.get_py_bottoms().size()
//           << " py_tops.size() = " << curr_imagetext.get_py_tops().size() 
//           << endl;
      
//      for(unsigned int t = 0; t < n_textlines; t++)
//      {
//         cout << "t = " << t 
//              << " py_top = " << curr_imagetext.get_py_tops().at(t)
//              << " py_bottom = " << curr_imagetext.get_py_bottoms().at(t)
//              << endl;
//      }
      return true;
   }

// ---------------------------------------------------------------------
// Method textline_pys() fills STL vector member textline_pys of
// imagetext class with vertical pixel values which represent text
// line tops/bottom positions.

   void textline_pys(imagetext& curr_imagetext)
   {
//      cout << "inside textfunc::textline_pys()" << endl;

      unsigned int n_textlines = curr_imagetext.get_n_textlines();
///      cout << "n_textlines = " << n_textlines << endl;

      curr_imagetext.get_textline_pys().push_back(
         curr_imagetext.get_py_tops().front());
      for(unsigned int t = 0; t < n_textlines-1; t++)
      {
         double avg_textline_py = 0.5 * (curr_imagetext.get_py_bottoms().at(t)+
                                         curr_imagetext.get_py_tops().at(t+1));
         curr_imagetext.get_textline_pys().push_back(avg_textline_py);
      }
      curr_imagetext.get_textline_pys().push_back(
         curr_imagetext.get_py_bottoms().back());

//      for(unsigned int t = 0; t < curr_imagetext.get_textline_pys().size(); 
//          t++)
//      {
//         cout << "t = " << t 
//              << " textline_py = " << curr_imagetext.get_textline_pys().at(t)
//              << endl;
//      }
   }

// -------------------------------------------------------------------------
// Perform vertical profilling to search for beginning and end of each
// rendered text line.

   bool text_line_horizontal_limits(imagetext& curr_imagetext)
   {
//      cout << "inside textfunc::text_line_horizontal_limits()" << endl;

      int n_textlines = curr_imagetext.get_n_textlines();
      texture_rectangle curr_tr(curr_imagetext.get_text_image_filename(), 
                                NULL);      
      int img_width = curr_tr.getWidth();
      const int min_value = 5;

      int undercolor_R, undercolor_G, undercolor_B;
      undercolor_R = undercolor_G = undercolor_B = -1;
      if(curr_imagetext.get_underbox_flag())
      {
         undercolor_R = curr_imagetext.get_undercolor_RGB().first;
         undercolor_G = curr_imagetext.get_undercolor_RGB().second;
         undercolor_B = curr_imagetext.get_undercolor_RGB().third;
      }

      bool limits_found = true;
      int R, G, B, A;
      for(int t = 0; t < n_textlines; t++)
      {
         bool left_edge_found = false;
         bool right_edge_found = false;

         int px = 0;
         while(px < img_width && !left_edge_found)
         {
            for(int py = curr_imagetext.get_py_tops().at(t); 
                py <= curr_imagetext.get_py_bottoms().at(t) 
                   && !left_edge_found; py++)
            {
               curr_tr.get_pixel_RGBA_values(px, py, R, G, B, A);
               if(A == 0) continue;
               if(R == undercolor_R && G == undercolor_G && B == undercolor_B)
                  continue;

               if (R > min_value || G > min_value || B > min_value)
               {
                  curr_imagetext.get_px_lefts().push_back(px);
                  left_edge_found = true;
               }
            }
            px++;
         } // px while loop

         if(!left_edge_found)
         {
            limits_found = false;
            return limits_found;
         }

         px = img_width - 1;
         while(px >= 0 && !right_edge_found)
         {
            for(int py = curr_imagetext.get_py_tops().at(t); 
                py <= curr_imagetext.get_py_bottoms().at(t) 
                   && !right_edge_found; py++)
            {
               curr_tr.get_pixel_RGBA_values(px, py, R, G, B, A);
               if(A == 0) continue;
               if(R == undercolor_R && G == undercolor_G && B == undercolor_B)
                  continue;
               if (R > min_value || G > min_value || B > min_value)
               {
                  curr_imagetext.get_px_rights().push_back(px);
                  right_edge_found = true;
               }
            }
            px--;
         } // px while loop

         if(!right_edge_found)
         {
            limits_found = false;
            return limits_found;
         }
         
//         cout << "t = " << t 
//              << " px_left = " << curr_imagetext.get_px_lefts().at(t)
//              << " px_right = " << curr_imagetext.get_px_rights().at(t)
//              << endl;
      } // loop over index t labeling text lines

      return limits_found;
   }

// -------------------------------------------------------------------------
   bool find_rendered_char_pixel_bboxes(imagetext& curr_imagetext)
   {
//      cout << "inside textfunc::find_rendered_char_pixel_bboxes()" << endl;

      int n_textlines = curr_imagetext.get_n_textlines();
      string text_label = curr_imagetext.get_phrase();
      unsigned int n_chars = curr_imagetext.get_n_characters();
      unsigned int char_counter = 0;

      for(int t = 0; t < n_textlines; t++)
      {
//         cout << "text line t = " << t << endl;
         int char_py_start = curr_imagetext.get_py_tops().at(t);
         int char_py_stop = curr_imagetext.get_py_bottoms().at(t);

         int char_px_start = curr_imagetext.get_px_lefts().at(t);
         int curr_textline_width = curr_imagetext.get_px_rights().at(t) - 
            curr_imagetext.get_px_lefts().at(t);

         string curr_char = text_label.substr(char_counter,1);
         bounding_box curr_bbox(-1,-1,-1,-1);

// Ignore any initial spaces at start of current text line:

         while(curr_char == " ")
         {
            bounding_box curr_bbox(
               char_px_start, char_px_start,
               char_py_start, char_py_stop);
            curr_imagetext.get_char_pixel_bboxes().push_back(curr_bbox);
            curr_imagetext.get_char_textlines().push_back(t);
            char_counter++;
            curr_char = text_label.substr(char_counter,1);
         }

//         cout << "char_widths.size() = "
//              << curr_imagetext.get_char_widths().size() 
//              << " get_n_chars = " << curr_imagetext.get_n_characters()
//              << endl;
//         cout << "text_lable.size() = " << text_label.size() << endl;

         while(curr_textline_width > 0 && char_counter < text_label.size())
         {
            curr_char = text_label.substr(char_counter,1);
            int curr_width = 
               curr_imagetext.get_char_widths().at(char_counter);
            bounding_box curr_bbox(
               char_px_start, char_px_start + curr_width,
               char_py_start, char_py_stop);
            curr_imagetext.get_char_pixel_bboxes().push_back(curr_bbox);
            curr_imagetext.get_char_textlines().push_back(t);

//            cout << "char_counter = " << char_counter
//                 << " char = " << curr_char
//                 << " char_width = " << curr_width
//                 << " curr_textline_width = " << curr_textline_width 
//                 << endl;
            char_counter++;
            char_px_start += curr_width;
            curr_textline_width -= curr_width;

            if(curr_imagetext.get_vertical_text_flag())
               curr_textline_width = 0;
         }
      } // loop over index t labeling text lines
      
//      cout << "n_chars = " << n_chars
//           << " char_counter = " << char_counter << endl;
//      cout << "n_char_bboxes = "
//           << curr_imagetext.get_char_pixel_bboxes().size() << endl;

      return (n_chars == char_counter);
   }
   
// -------------------------------------------------------------------------
// High-level method find_textlines_and_char_bboxes() takes in a
// rendered text image within curr_imagetext.  It counts the number of
// text lines within the rendered image and finds horizontal lines
// which vertically separate the lines of text.  This method then
// locates the left and right boders for each text line via vertical
// profiling.  Finally, pixel bounding boxes are placed around each
// character within the phrase corresponding to curr_imagetext.

   bool find_textlines_and_char_bboxes(imagetext& curr_imagetext)
   {
//      cout << "inside textfunc::find_textlines_and_char_bboxes()" << endl;

      cout << " 5a" << flush;         
      textfunc::n_rendered_text_lines(curr_imagetext);

      cout << " 5b" << flush;         
      if(!textfunc::text_line_vertical_separations(curr_imagetext)) 
      {
         cout << " text line vertical separations failed" << endl << endl;
         return false;
      }

      textfunc::textline_pys(curr_imagetext);

      cout << " 5c" << flush;      
      if(!textfunc::text_line_horizontal_limits(curr_imagetext)) 
      {
         cout << " text line horizontal limits failed" << endl << endl;
         return false;
      }
      
      cout << " 5d" << flush;
      if(!textfunc::find_rendered_char_pixel_bboxes(curr_imagetext))
      {
         cout << " find_rendered_char_pixel_bboxes failed" << endl << endl;
         return false;
      }
      
      return true;
   }
   

} // textfunc namespace



