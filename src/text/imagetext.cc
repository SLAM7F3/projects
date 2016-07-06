// ==========================================================================
// Imagetext class member functions
// ==========================================================================
// Last modified on 4/15/16; 4/17/16; 4/20/16; 4/22/16
// ==========================================================================

#include "general/filefuncs.h"
#include "text/imagetext.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cout;
using std::cin;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void imagetext::allocate_member_objects()
{
}

void imagetext::initialize_member_objects()
{
   ID = -1;
   curr_mask_type = char_posns__word_numbering;
   n_textlines = -1;
   max_n_chars = 100;
   debug_annotate_flag = false;
   spaced_out_skip = 0;
   drop_shadow_flag = false;
   variegated_3D_flag = false;
   vertical_text_flag = false;
}		       

imagetext::imagetext()
{
   allocate_member_objects();
   initialize_member_objects();
}

imagetext::~imagetext()
{
//    cout << "inside imagetext destructor" << endl;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const imagetext& it)
{
   outstream << endl;
   outstream << "phrase = " << it.get_phrase() << endl;
   outstream << "n_characters = " << it.get_n_characters() << endl;
   outstream << "n_textlines = " << it.get_n_textlines() << endl;
   outstream << "font = " << it.get_font() << endl;

   for(int c = 0; c < it.get_n_characters(); c++)
   {
      bounding_box curr_bbox = it.get_char_pixel_bboxes().at(c);
      int px_lo = curr_bbox.get_xmin();
      int px_hi = curr_bbox.get_xmax();
      int py_lo = curr_bbox.get_ymin();
      int py_hi = curr_bbox.get_ymax();
      outstream << "c = " << c 
                << " textline = " << it.get_textline(c)
                << " char = " << it.get_char(c) 
                << " word_ID = " << it.get_word_ID(c)
                << " chartype = " << it.get_char_type_string(c)
                << " chartype2 = " << it.get_char_type2_string(c)
                << " px_lo = " << px_lo
                << " px_hi = " << px_hi
                << " py_lo = " << py_lo
                << " py_hi = " << py_hi
                << endl;
   }
   return outstream;
}

// ---------------------------------------------------------------------
void imagetext::print_info(string image_filename, ofstream& outstream)
{
   outstream << "image_filename = " << filefunc::getbasename(image_filename)
             << endl;
   outstream << "n_textlines = " << get_n_textlines() << endl;
   outstream << "n_words = " << get_n_words() << endl;
   outstream << "n_characters = " << get_n_characters() << endl;
   outstream << "pointsize = " << get_pointsize() << endl;
   outstream << "char_height = " << get_char_height() << endl;
   outstream << "image_width = " << get_image_width() << endl;
   outstream << "image_height = " << get_image_height() << endl;
   outstream << "underbox_flag = " << get_underbox_flag() << endl;
   outstream << "spaced_out_skip = " << get_spaced_out_skip() << endl;
   outstream << "vertical_flag = " << get_vertical_text_flag() << endl;
   outstream << "font = " << filefunc::getbasename(get_font()) << endl;
   outstream << "phrase = " << get_phrase() << endl;
   outstream << endl;
}

// ==========================================================================
// ==========================================================================

void imagetext::set_foreground_RGB(int R, int G, int B)
{
   foreground_RGB.first=R;
   foreground_RGB.second=G;
   foreground_RGB.third=B;
}

void imagetext::set_foreground_RGB(colorfunc::RGB RGB)
{
   foreground_RGB.first = RGB.first;
   foreground_RGB.second = RGB.second;
   foreground_RGB.third = RGB.third;
}

colorfunc::RGB& imagetext::get_foreground_RGB() 
{
   return foreground_RGB;
}

const colorfunc::RGB& imagetext::get_foreground_RGB() const
{
   return foreground_RGB;
}

void imagetext::set_stroke_RGB(int R, int G, int B)
{
   stroke_RGB.first=R;
   stroke_RGB.second=G;
   stroke_RGB.third=B;
}

void imagetext::set_stroke_RGB(colorfunc::RGB RGB)
{
   stroke_RGB.first = RGB.first;
   stroke_RGB.second = RGB.second;
   stroke_RGB.third = RGB.third;
}

colorfunc::RGB& imagetext::get_stroke_RGB() 
{
   return stroke_RGB;
}

const colorfunc::RGB& imagetext::get_stroke_RGB() const
{
   return stroke_RGB;
}

void imagetext::set_background_RGBA(int R, int G, int B, int A)
{
   background_RGBA.first=R;
   background_RGBA.second=G;
   background_RGBA.third=B;
   background_RGBA.fourth=A;
}

colorfunc::RGBA& imagetext::get_background_RGBA() 
{
   return background_RGBA;
}

const colorfunc::RGBA& imagetext::get_background_RGBA() const
{
   return background_RGBA;
}

void imagetext::set_undercolor_RGB(int R, int G, int B)
{
   undercolor_RGB.first=R;
   undercolor_RGB.second=G;
   undercolor_RGB.third=B;
}

void imagetext::set_undercolor_RGB(colorfunc::RGB RGB)
{
   undercolor_RGB.first = RGB.first;
   undercolor_RGB.second = RGB.second;
   undercolor_RGB.third = RGB.third;
}

colorfunc::RGB& imagetext::get_undercolor_RGB() 
{
   return undercolor_RGB;
}

const colorfunc::RGB& imagetext::get_undercolor_RGB() const
{
   return undercolor_RGB;
}

// ---------------------------------------------------------------------
void imagetext::clear_all_params()
{
   n_words = 0;
   word_IDs.clear();
   word_char_counts.clear();
   phrase_words.clear();
   char_types.clear();
   char_type2s.clear();
   char_widths.clear();
   char_textlines.clear();

   py_tops.clear();
   py_bottoms.clear();
   px_lefts.clear();
   px_rights.clear();
   textline_pys.clear();

   char_pixel_bboxes.clear();
}		       

// ---------------------------------------------------------------------
void imagetext::set_phrase(string input_phrase)
{
   vector<char_type> un_spaced_out_char_types;
   set_phrase(input_phrase, un_spaced_out_char_types);
}

void imagetext::set_phrase(
   string input_phrase, const vector<char_type>& un_spaced_out_char_types)
{
//   cout << "inside imagetext::set_phrase()" << endl;
   clear_all_params();
  
   phrase = input_phrase;
   if(get_n_characters() > max_n_chars)
   {
      phrase=phrase.substr(0,max_n_chars);
   }
   assign_chars_to_words();
   assign_char_types(un_spaced_out_char_types);
   assign_char_type2s();

//   check_char_types();
}

// ---------------------------------------------------------------------
// Member function assign_chars_to_words() loops over every
// character within *this.  Characters belonging to the nth word in
// the phrase are assigned word ID = n.  The number of words in the
// current phrase is also set by this method.

void imagetext::assign_chars_to_words()
{
//   cout << "inside imagetext::assign_chars_to_words()" << endl;
//   cout << "phrase = " << get_phrase() << endl;
   
   n_words = 0;
   word_IDs.clear();
   word_char_counts.clear();
   phrase_words.clear();
   int curr_word_ID = 0;
   int max_word_ID = 0;
   string curr_word="";
   for(int c = 0; c < get_n_characters(); c++)
   {
      string curr_char = get_char(c);
      if(curr_char == " ")
      {
         word_IDs.push_back(-1);
         phrase_words.push_back(curr_word);
         curr_word_ID++;
         curr_word="";
      }
      else
      {
         word_IDs.push_back(curr_word_ID);
         max_word_ID = basic_math::max(curr_word_ID, max_word_ID);
         curr_word += curr_char;
      }
   } // loop over index c labeling phrase characters
   phrase_words.push_back(curr_word);
   
   n_words = phrase_words.size();

//   for(int w = 0; w < n_words; w++)
//   {
//      cout << "w = " << w << " word = " << phrase_words[w]
//           << " n_chars_in_word = " << phrase_words[w].size() << endl;
//   }
   
//   for(int c = 0 ; c < get_n_characters(); c++)
//   {
//      string curr_char = get_char(c);
//      cout << "c = " << c << " curr_char = " << curr_char 
//           << " n_chars_in_word = " << get_word_nchars(c) << endl;
//   }
}

// ---------------------------------------------------------------------
// Member function get_word_nchars() takes in character index c.  If
// the character belongs to a word, this method returns the number of
// characters in that word.  Otherwise, it returns 0.

int imagetext::get_word_nchars(int c) const
{
   if(c < 0 || c >= get_n_characters())
   {
      return 0;
   }
   else
   {
      int curr_word_ID = get_word_ID(c);
      if(curr_word_ID < 0)
      {
         return 0;
      }
      else
      {
         string curr_word = phrase_words[curr_word_ID];
         return curr_word.size();
      }
   }
}
// ---------------------------------------------------------------------
// Member function assign_char_types() loops over all characters
// within *this.  If the current char is a space, its type is set to
// space_char.  Otherwise, the character's type is set to start_char,
// middle_char, stop_char or single_char depending upon its distance
// from spaces within the phrase member string.

void imagetext::assign_char_types(
   const vector<char_type>& un_spaced_out_char_types)
{
//   cout << "inside assign_char_types()" << endl;
//   cout << "phrase = " << phrase << endl;
//   cout << "n_chars = " << get_n_characters() << endl;
//   cout << "un_spaced_out_char_types.size = "
//        << un_spaced_out_char_types.size() << endl;

   int skip = get_spaced_out_skip();
   if(skip > 0)
   {
      for(int c = 0; c < get_n_characters(); c++)
      {
//         char_types.push_back(un_spaced_out_char_types[c/2]);
         char_types.push_back(un_spaced_out_char_types[c/(skip+1)]);
      }
      return;
   }

   char_type curr_char_type = start_char;
   if(get_vertical_text_flag())
   {
      curr_char_type = vertical_start_char;
   }

   if(get_char(1) == " ")
   {
      curr_char_type = single_char;
   }
   char_types.push_back(curr_char_type);
   char_type prev_char_type = curr_char_type;

   n_whitespaces = 0;
   for(int c = 1; c < get_n_characters() - 1; c++)
   {
//      cout << "c = " << c << " char[c] = " << get_char(c)
//           << " is_space = " << (get_char(c) == " ") 
//           << " next_is_space = " << (get_char(c+1) == " ")
//           << endl;
      if(get_char(c) == " ")
      {
         curr_char_type = space_char;
         n_whitespaces++;
         char_types.push_back(curr_char_type);
         prev_char_type = curr_char_type;
         continue;
      }

      if(get_char(c+1) == " ")
      {
         if(prev_char_type == space_char)
         {
            curr_char_type = single_char;
         }
         else
         {
            curr_char_type = stop_char;
            if(get_vertical_text_flag())
            {
               curr_char_type = vertical_stop_char;
            }
         }
      }
      else if(prev_char_type == space_char)
      {
         curr_char_type = start_char;
         if(get_vertical_text_flag())
         {
            curr_char_type = vertical_start_char;
         }
      }
      else
      {
         curr_char_type = middle_char;
         if(get_vertical_text_flag())
         {
            curr_char_type = vertical_middle_char;
         }
      }

      char_types.push_back(curr_char_type);
      prev_char_type = curr_char_type;
   } // loop over index c labeling phrase characters

   string last_char=get_char(get_n_characters()-1);
   if(last_char == " ")
   {
      curr_char_type = space_char;
      n_whitespaces++;
   }
   else
   {
      if(prev_char_type == space_char)
      {
         curr_char_type = single_char;
      }
      else
      {
         curr_char_type = stop_char;
         if(get_vertical_text_flag())
         {
            curr_char_type = vertical_stop_char;
         }
      }
   }
   char_types.push_back(curr_char_type);
}

// ---------------------------------------------------------------------
imagetext::char_type imagetext::get_char_type(int c) const
{
   if(c < 0 || c >= get_n_characters())
   {
      return no_char;
   }
   else
   {
      return char_types[c];
   }
}

// ---------------------------------------------------------------------
string imagetext::get_char_type_string(int c) const
{
   string type_str = "no_char";
   char_type curr_char_type = get_char_type(c);

   if(curr_char_type == start_char)
   {
      type_str = "start_char";
   }
   if(curr_char_type == vertical_start_char)
   {
      type_str = "vertical_start_char";
   }
   else if (curr_char_type == middle_char)
   {
      type_str = "middle_char";
   }
   else if (curr_char_type == vertical_middle_char)
   {
      type_str = "vertical_middle_char";
   }
   else if (curr_char_type == stop_char)
   {
      type_str = "stop_char";
   }
   else if (curr_char_type == vertical_stop_char)
   {
      type_str = "vertical_stop_char";
   }
   else if (curr_char_type == single_char)
   {
      type_str = "single_char";
   }
   else if (curr_char_type == space_char)
   {
      type_str = "space_char";
   }
   return type_str;
}

// ---------------------------------------------------------------------
string imagetext::get_char_type2_string(int c) const
{
   char_type2 curr_char_type2 = get_char_type2(c);
   string type2_str = "not_char";
   if(curr_char_type2 == digit)
   {
      type2_str = "digit";
   }
   else if (curr_char_type2 == vowel)
   {
      type2_str = "vowel";
   }
   else if (curr_char_type2 == consonant)
   {
      type2_str = "consonant";
   }
   else if (curr_char_type2 == space)
   {
      type2_str = "space";
   }
   else if (curr_char_type2 == punctuation)
   {
      type2_str = "punctuation";
   }
   return type2_str;
}

// ---------------------------------------------------------------------
// Member function assign_char_type2s() loops over all characters
// within *this.  Depending up the current character's ascii value,
// its type2 is set to digit, vowel, consonant, space, puncutation or
// not_char.

void imagetext::assign_char_type2s()
{
//   cout << "inside assign_char_type2s()" << endl;
//   cout << "phrase = " << phrase << endl;

   for(int c = 0; c < get_n_characters(); c++)
   {
      
      int ascii_val = 32;
      if(get_char(c) != " " && get_char(c) != "\n")
      {
         char curr_char = stringfunc::string_to_char(get_char(c));
         ascii_val = stringfunc::char_to_ascii_integer(curr_char);
      }

      if(ascii_val <= 31)
      {
         char_type2s.push_back(not_char);
      }
      else if(ascii_val == 32)
      {
         char_type2s.push_back(space);
      }
      else if (ascii_val >= 33 && ascii_val <= 47)
      {
         char_type2s.push_back(punctuation);
      }
      else if (ascii_val >= 48 && ascii_val <= 57) // 0-9
      {
         char_type2s.push_back(digit);
      }
      else if (ascii_val >= 58 && ascii_val <= 64)
      {
         char_type2s.push_back(punctuation);
      }
      else if (ascii_val >= 65 && ascii_val <= 90) // A-Z
      {
         if(ascii_val == 65 || ascii_val == 69 || ascii_val == 73 ||
            ascii_val == 79 || ascii_val == 85)
         {
            char_type2s.push_back(vowel);
         }
         else
         {
            char_type2s.push_back(consonant);
         }
      }
      else if (ascii_val >= 91 && ascii_val <= 96) 
      {
         char_type2s.push_back(punctuation);
      }
      else if (ascii_val >= 97 && ascii_val <= 122) // a-z
      {
         if(ascii_val == 97 || ascii_val == 101 || ascii_val == 105 ||
            ascii_val == 111 || ascii_val == 117)
         {
            char_type2s.push_back(vowel);
         }
         else
         {
            char_type2s.push_back(consonant);
         }
      }
      else if (ascii_val >= 123 && ascii_val <= 126)
      {
         char_type2s.push_back(punctuation);
      }
      else if (ascii_val >= 127)
      {
         char_type2s.push_back(not_char);
      }
   }
}

// ---------------------------------------------------------------------
void imagetext::check_char_types() const
{
   cout << "inside check_char_types()" << endl;
   cout << "phrase = " << phrase << endl;

   for(int c = 0; c < get_n_characters(); c++)
   {
      cout << "c = " << c << " char = " << phrase.substr(c,1)
           << " chartype = " << get_char_type_string(c)
           << endl;
   }

//   outputfunc::enter_continue_char();
}

// ==========================================================================
// Mask generation member functions
// ==========================================================================
   
// Member function generate_masks() loops over each character within
// *this.  It retrieves the character and work mask values associated
// with the current character.  If visualize_mask_flag == true, the
// mask values are converted into corresponding colorfunc colors.
// Otherwise, the (relatively low) mask values are used to generate
// greyscale colors which are effectively indistinguishable from
// black.

void imagetext::generate_masks(texture_rectangle* charmask_tr_ptr,
                               texture_rectangle* wordmask_tr_ptr,
                               bool visualize_mask_flag,
                               int& max_charmask_val, int& max_wordmask_val)
{        
   max_charmask_val = max_wordmask_val = -1;
   charmask_tr_ptr->reset_all_RGBA_values(0,0,0,255);
   wordmask_tr_ptr->reset_all_RGBA_values(0,0,0,255);
   int Rchar,Gchar,Bchar,Rword,Gword,Bword;
   int charmask_value = -1, wordmask_value = -1;
   int Amask = 255;
   for(int c = 0; c < get_n_characters(); c++)
   {
      bounding_box curr_bbox = get_char_pixel_bboxes().at(c);
      int px_lo = curr_bbox.get_xmin();
      int px_hi = curr_bbox.get_xmax();
      int py_lo = curr_bbox.get_ymin();
      int py_hi = curr_bbox.get_ymax();
      int curr_char_type = get_char_type(c);
      int curr_char_type2 = get_char_type2(c);

      Rchar=Gchar=Bchar=Rword=Gword=Bword=0;

      if(visualize_mask_flag)
      {
         if(curr_mask_type == char_posns__word_numbering ||
            curr_mask_type == char_posns__vertical_halves)
         {
            charmask_value = curr_char_type - 1;
            wordmask_value = get_word_ID(c);
         }
         else if (curr_mask_type == digit_letter__word_n_chars)
         {
            charmask_value = curr_char_type2 - 1;
            wordmask_value = get_word_nchars(c);
         }
         else if (curr_mask_type == char_posns__digit_letter)
         {
            charmask_value = curr_char_type - 1;
            wordmask_value = curr_char_type2 - 1;
         }

         colorfunc::Color char_color=colorfunc::get_color(charmask_value);
         colorfunc::Color word_color=colorfunc::get_color(wordmask_value);
         colorfunc::RGB char_rgb = colorfunc::get_RGB_values(char_color);
         colorfunc::RGB word_rgb = colorfunc::get_RGB_values(word_color);
         Rchar = char_rgb.first * 255;
         Gchar = char_rgb.second * 255;
         Bchar = char_rgb.third * 255;

         if(wordmask_value >= 0)
         {
            Rword = word_rgb.first * 255;
            Gword = word_rgb.second * 255;
            Bword = word_rgb.third * 255;

         }
      }
      else
      {
         if(curr_mask_type == char_posns__word_numbering ||
            curr_mask_type == char_posns__vertical_halves)
         {
            charmask_value = curr_char_type;
            wordmask_value = get_word_ID(c)+2;
         }
         else if (curr_mask_type == digit_letter__word_n_chars)
         {
            charmask_value = curr_char_type2;
            wordmask_value = get_word_nchars(c);
         }
         else if (curr_mask_type == char_posns__digit_letter)
         {
            charmask_value = curr_char_type;
            wordmask_value = curr_char_type2;
         }

         Rchar = Gchar = Bchar = charmask_value;
         Rword = Gword = Bword = wordmask_value;
         max_charmask_val = basic_math::max(charmask_value,max_charmask_val);
         max_wordmask_val = basic_math::max(wordmask_value,max_wordmask_val);

         if(curr_mask_type == char_posns__vertical_halves)
         {
            max_wordmask_val = bottom_half_val;
         }
      } // visualize_mask_flag conditional

      for(int py = py_lo; py <= py_hi; py++)
      {
         for(int px = px_lo; px <= px_hi; px++)
         {
            charmask_tr_ptr->set_pixel_RGBA_values(
               px,py,Rchar,Gchar,Bchar,Amask);

            if(curr_mask_type == char_posns__vertical_halves)
            {
               if(py > 0.5*(py_lo+py_hi))
               {
                  if(visualize_mask_flag)
                  {
                     Rword = 255; Gword = Bword = 0;
                  }
                  else
                  {
                     Rword = Gword = Bword = top_half_val;
                  }
               }
               else
               {
                  if(visualize_mask_flag)
                  {
                     Rword = Gword = 0; Bword = 255;
                  }
                  else
                  {
                     Rword = Gword = Bword = bottom_half_val;
                  }
               } // py > 0.5*(py_lo+py_hi) conditional
            } // curr_mask_type == char_posns__vertical_halves conditional
            
            wordmask_tr_ptr->set_pixel_RGBA_values(
               px,py,Rword,Gword,Bword,Amask);
         } // loop over px
      } // loop over py
   } // loop over index c labeling phrase characters

/*
// Render horizontal lines separating different textlines:

   if(visualize_mask_flag)
   {
      Rchar = Bchar = 255;
      Gchar = 0;
   }
   else
   {
      charmask_value = textline_separator;
      max_charmask_val = basic_math::max(charmask_value,max_charmask_val);
      Rchar = Gchar = Bchar = charmask_value;
   }

   int curr_left = px_lefts.front();
   int curr_right = px_rights.front();
   for(int px = curr_left; px <= curr_right; px++)
   {
      charmask_tr_ptr->set_pixel_RGBA_values(
         px, textline_pys.front(), Rchar, Gchar, Bchar, Amask);
   }

   unsigned int n_textlines = get_n_textlines();   
   for(unsigned int t = 1; t <= n_textlines - 1; t++)
   {
      curr_left = basic_math::min(px_lefts[t - 1], px_lefts[t]);
      curr_right = basic_math::max(px_rights[t - 1], px_rights[t]);
      for(int px = curr_left; px<= curr_right; px++)
      {
         charmask_tr_ptr->set_pixel_RGBA_values(
            px, textline_pys[t], Rchar, Gchar, Bchar, Amask);
      }
   }

   curr_left = px_lefts.back();
   curr_right = px_rights.back();
   for(int px = curr_left; px <= curr_right; px++)
   {
      charmask_tr_ptr->set_pixel_RGBA_values(
         px, textline_pys.back(), Rchar, Gchar, Bchar, Amask);
   }
*/

}

// ---------------------------------------------------------------------
// Member function generate_mask_montage() first superposes text onto
// char and word masks.  It then forms a vertical montage of the
// synthetic text image, the character and word masks.

void imagetext::generate_mask_montage(
   string charmask_filename, string wordmask_filename, 
   texture_rectangle* charmask_texture_rectangle_ptr,
   texture_rectangle* wordmask_texture_rectangle_ptr,
   int montage_ID)
{
// Superpose text onto char and word masks:

   string unix_cmd="composite -compose Screen "
      +get_text_image_filename()+" "+charmask_filename+" char_composite.png";
   sysfunc::unix_command(unix_cmd);

   unix_cmd="composite -compose Screen "
      +get_text_image_filename()+" "+wordmask_filename+" word_composite.png";
   sysfunc::unix_command(unix_cmd);


  unix_cmd = "montage -tile 1x3 -geometry ";
   unix_cmd += 
      stringfunc::number_to_string(get_image_width()+20)+"x";
   unix_cmd += 
      stringfunc::number_to_string(get_image_height()+20);
   unix_cmd += " "+get_text_image_filename();
   unix_cmd += " char_composite.png word_composite.png ";
   string montage_filename=
       " montage_"+stringfunc::integer_to_string(montage_ID,5)+".png";
   unix_cmd += montage_filename;
//   cout << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);

   string banner="Exported "+montage_filename;
   outputfunc::write_banner(banner);
}

   
   
 
