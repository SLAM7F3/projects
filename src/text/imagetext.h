// =========================================================================
// Header file for imagetext class
// =========================================================================
// Last modified on 4/15/16; 4/17/16; 4/20/16; 4/22/16
// =========================================================================

#ifndef IMAGETEXT_H
#define IMAGETEXT_H

#include <iostream>
#include <string>
#include <vector>
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"

class texture_rectangle;

class imagetext
{
  public:

   enum mask_type
   {
      char_posns__word_numbering, // char posns inside words + word numbering
      digit_letter__word_n_chars, // digit/vowel/consonant chars + 
				  //   word n_chars
      char_posns__digit_letter,  // char posns in words + 
				 //   digit/vowel/consonant chars
      char_posns__vertical_halves // char posns in words + top vs bottom halves
				  //  of chars in textlines
   };
   
   enum char_type
   {
      start_char = 1, middle_char, stop_char, single_char, space_char, 
      vertical_start_char, vertical_middle_char, vertical_stop_char, no_char
   };

   enum char_type2
   {
      digit = 1, vowel, consonant, space, punctuation, not_char
   };

   enum char_vertical_halves
   {
      top_half_val = 1, bottom_half_val
   };

   imagetext();

   ~imagetext();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const imagetext& it);

// Set & get member functions:

   void set_ID(int ID);
   int get_ID() const;
   void set_max_n_chars(int n);
   void set_debug_annotate_flag(bool flag);
   bool get_debug_annotate_flag() const;
   void set_randomize_gravity_flag(bool flag);
   bool get_randomize_gravity_flag() const;
   void set_underbox_flag(bool flag);
   bool get_underbox_flag() const;
   void set_spaced_out_skip(int skip);
   int get_spaced_out_skip() const;
   void set_drop_shadow_flag(bool flag);
   bool get_drop_shadow_flag() const;
   void set_variegated_3D_flag(bool flag);
   bool get_variegated_3D_flag() const;
   void set_vertical_text_flag(bool flag);
   bool get_vertical_text_flag() const;
   

   void set_text_image_filename(std::string filename);
   std::string get_text_image_filename() const;
   void set_phrase(std::string input_phrase);
   void set_phrase(std::string input_phrase, 
                   const std::vector<char_type>& un_spaced_out_char_types);
   std::string get_phrase() const;
   int get_n_characters() const;
   std::string get_char(int c) const;

   void set_mask_type(mask_type mt);
   char_type get_char_type(int c) const;
   char_type2 get_char_type2(int c) const;

   int get_textline(int c) const;
   void set_n_textlines(int n);
   int get_n_textlines() const;
   int get_word_ID(int c) const;
   int get_word_nchars(int c) const;
   int get_n_words() const;
   int get_n_whitespaces() const;

   void set_pointsize(int pointsize);
   int get_pointsize() const;
   void set_char_height(int height);
   int get_char_height() const;

   void set_font(std::string font);
   std::string get_font() const;
   void set_image_width(int w);
   void set_image_height(int h);
   int get_image_width() const;
   int get_image_height() const;
   void set_origin(int px, int py);
   int get_origin_px() const;
   int get_origin_py() const;
   void set_strokewidth(double w);
   double get_strokewidth() const;

   void set_foreground_RGB(int R, int G, int B);
   void set_foreground_RGB(colorfunc::RGB RGB);
   colorfunc::RGB& get_foreground_RGB();
   const colorfunc::RGB& get_foreground_RGB() const;

   void set_stroke_RGB(int R, int G, int B);
   void set_stroke_RGB(colorfunc::RGB RGB);
   colorfunc::RGB& get_stroke_RGB();
   const colorfunc::RGB& get_stroke_RGB() const;

   void set_background_RGBA(int R, int G, int B, int A);
   colorfunc::RGBA& get_background_RGBA();
   const colorfunc::RGBA& get_background_RGBA() const;

   void set_undercolor_RGB(int R, int G, int B);
   void set_undercolor_RGB(colorfunc::RGB RGB);
   colorfunc::RGB& get_undercolor_RGB();
   const colorfunc::RGB& get_undercolor_RGB() const;

   std::vector<int>& get_char_widths();
   const std::vector<int>& get_char_widths() const;
   std::vector<int>& get_char_textlines();
   const std::vector<int>& get_char_textlines() const;

   std::vector<int>& get_py_tops();
   const std::vector<int>& get_py_tops() const;
   std::vector<int>& get_py_bottoms();
   const std::vector<int>& get_py_bottoms() const;
   std::vector<int>& get_px_lefts();
   const std::vector<int>& get_px_lefts() const;
   std::vector<int>& get_px_rights();
   const std::vector<int>& get_px_rights() const;
   std::vector<bounding_box>& get_char_pixel_bboxes();
   const std::vector<bounding_box>& get_char_pixel_bboxes() const;
   std::vector<int>& get_textline_pys();
   const std::vector<int>& get_textline_pys() const;

   void print_info(std::string image_filename, std::ofstream& outstream);
   void assign_chars_to_words();
   void assign_char_types(
      const std::vector<char_type>& un_spaced_out_char_types);
   void assign_char_type2s();
   std::string get_char_type_string(int c) const;
   std::string get_char_type2_string(int c) const;
   void check_char_types() const;

// Mask member functions:

   void generate_masks(texture_rectangle* charmask_tr_ptr,
                       texture_rectangle* wordmask_tr_ptr,
                       bool visualize_mask_flag,
                       int& max_charmask_val, int& max_wordmask_val);
   void generate_mask_montage(
      std::string charmask_filename, std::string wordmask_filename,
      texture_rectangle* charmask_texture_rectangle_ptr,
      texture_rectangle* wordmask_texture_rectangle_ptr,
      int montage_ID);

  private:

   bool debug_annotate_flag, randomize_gravity_flag, underbox_flag;
   bool drop_shadow_flag, variegated_3D_flag;
   bool vertical_text_flag;
   int ID, max_n_chars, spaced_out_skip;
   int n_textlines, n_words, n_whitespaces;
   int pointsize, char_height;
   std::string phrase, font, text_image_filename;
   int image_width, image_height;
   int origin_px, origin_py;
   double strokewidth;
   colorfunc::RGB foreground_RGB, stroke_RGB, undercolor_RGB;
   colorfunc::RGBA background_RGBA;

   mask_type curr_mask_type;
   std::vector<int> word_IDs, word_char_counts;
   std::vector<std::string> phrase_words;
   std::vector<char_type> char_types;
   std::vector<char_type2> char_type2s;
   std::vector<int> char_widths, char_textlines;
   std::vector<int> py_tops, py_bottoms, px_lefts, px_rights;
   std::vector<bounding_box> char_pixel_bboxes;
   std::vector<int> textline_pys;
   
   void allocate_member_objects();
   void initialize_member_objects();

   void clear_all_params();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void imagetext::set_ID(int ID)
{
   this->ID = ID;
}

inline int imagetext::get_ID() const
{
   return ID;
}

inline void imagetext::set_max_n_chars(int n)
{
   max_n_chars = n;
}

inline void imagetext::set_debug_annotate_flag(bool flag)
{
   debug_annotate_flag = flag;
}

inline bool imagetext::get_debug_annotate_flag() const
{
   return debug_annotate_flag;
}
inline void imagetext::set_randomize_gravity_flag(bool flag)
{
   randomize_gravity_flag = flag;
}

inline bool imagetext::get_randomize_gravity_flag() const
{
   return randomize_gravity_flag;
}

inline void imagetext::set_underbox_flag(bool flag)
{
   underbox_flag = flag;
}

inline bool imagetext::get_underbox_flag() const
{
   return underbox_flag;
}

inline void imagetext::set_spaced_out_skip(int skip)
{
   spaced_out_skip = skip;
}

inline int imagetext::get_spaced_out_skip() const
{
   return spaced_out_skip;
}

inline void imagetext::set_drop_shadow_flag(bool flag)
{
   drop_shadow_flag = flag;
}

inline bool imagetext::get_drop_shadow_flag() const
{
   return drop_shadow_flag;
}

inline void imagetext::set_variegated_3D_flag(bool flag)
{
   variegated_3D_flag = flag;
}

inline bool imagetext::get_variegated_3D_flag() const
{
   return variegated_3D_flag;
}

inline void imagetext::set_vertical_text_flag(bool flag)
{
   vertical_text_flag = flag;
}

inline bool imagetext::get_vertical_text_flag() const
{
   return vertical_text_flag;
}

inline void imagetext::set_text_image_filename(std::string filename)
{
   text_image_filename = filename;
}

inline std::string imagetext::get_text_image_filename() const
{
   return text_image_filename;
}

inline std::string imagetext::get_phrase() const
{
   return phrase;
}

inline int imagetext::get_n_characters() const
{
   return phrase.size();
}

inline std::string imagetext::get_char(int c) const
{
   if(c < 0 || c >= get_n_characters())
   {
      return "";
   }
   else
   {
      return phrase.substr(c,1);
   }
}

inline void imagetext::set_mask_type(mask_type mt)
{
   curr_mask_type = mt;
}

inline imagetext::char_type2 imagetext::get_char_type2(int c) const
{
   if(c < 0 || c >= get_n_characters())
   {
      return not_char;
   }
   else
   {
      return char_type2s[c];
   }
}

inline int imagetext::get_textline(int c) const
{
   if(c < 0 || c >= get_n_characters())
   {
      return -1;
   }
   else
   {
      return char_textlines[c];
   }
}


inline void imagetext::set_n_textlines(int n)
{
   n_textlines = n;
}

inline int imagetext::get_n_textlines() const
{
   return n_textlines;
}

inline int imagetext::get_word_ID(int c) const
{
   if(c < 0 || c >= get_n_characters())
   {
      return -1;
   }
   else
   {
      return word_IDs[c];
   }
}

inline int imagetext::get_n_words() const
{
   return n_words;
}

inline int imagetext::get_n_whitespaces() const
{
   return n_whitespaces;
}

inline void imagetext::set_pointsize(int psize)
{
   pointsize = psize;
}

inline int imagetext::get_pointsize() const
{
   return pointsize;
}

inline void imagetext::set_char_height(int height)
{
   char_height = height;
}

inline int imagetext::get_char_height() const
{
   return char_height;
}

inline void imagetext::set_font(std::string font)
{
   this->font = font;
}

inline std::string imagetext::get_font() const
{
   return font;
}

inline void imagetext::set_image_width(int w)
{
   image_width = w;
}

inline void imagetext::set_image_height(int h)
{
   image_height = h;
}

inline int imagetext::get_image_width() const
{
   return image_width;
}

inline int imagetext::get_image_height() const
{
   return image_height;
}

inline void imagetext::set_origin(int px, int py)
{
   origin_px = px;
   origin_py = py;
}

inline int imagetext::get_origin_px() const
{
   return origin_px;
}

inline int imagetext::get_origin_py() const
{
   return origin_py;
}

inline void imagetext::set_strokewidth(double w)
{
   strokewidth = w;
}

inline double imagetext::get_strokewidth() const
{
   return strokewidth;
}


inline std::vector<int>& imagetext::get_char_widths()
{
   return char_widths;
}

inline const std::vector<int>& imagetext::get_char_widths() const
{
   return char_widths;
}

inline std::vector<int>& imagetext::get_char_textlines()
{
   return char_textlines;
}

inline const std::vector<int>& imagetext::get_char_textlines() const
{
   return char_textlines;
}

inline std::vector<int>& imagetext::get_py_tops()
{
   return py_tops;
}

inline const std::vector<int>& imagetext::get_py_tops() const
{
   return py_tops;
}

inline std::vector<int>& imagetext::get_py_bottoms()
{
   return py_bottoms;
}

inline const std::vector<int>& imagetext::get_py_bottoms() const
{
   return py_bottoms;
}

inline std::vector<int>& imagetext::get_px_lefts()
{
   return px_lefts;
}

inline const std::vector<int>& imagetext::get_px_lefts() const
{
   return px_lefts;
}

inline std::vector<int>& imagetext::get_px_rights()
{
   return px_rights;
}

inline const std::vector<int>& imagetext::get_px_rights() const
{
   return px_rights;
}

inline std::vector<bounding_box>& imagetext::get_char_pixel_bboxes()
{
   return char_pixel_bboxes;
}

inline const std::vector<bounding_box>& imagetext::get_char_pixel_bboxes() const
{
   return char_pixel_bboxes;
}

inline std::vector<int>& imagetext::get_textline_pys()
{
   return textline_pys;
}

inline const std::vector<int>& imagetext::get_textline_pys() const
{
   return textline_pys;
}


#endif // imagetext.h




