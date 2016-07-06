// ==========================================================================
// Program GENERATE_FONT_IMAGES loops over all ttf font files within a
// specified subdirectory.  For each font, it generates an output
// image containing all digits, lower case and upper case letters.  We
// wrote this utility program in order to identify "reasonable" fonts
// for synthetic character generation purposes.

//			 ./generate_font_images

// ==========================================================================
// Last updated on 9/1/14
// ==========================================================================

//   c='0';	// ascii = 48
//   c='9';	// ascii = 57

//   c='A';	// ascii = 65
//   c='Z';	// ascii = 90

//   c='a';	// ascii = 97
//   c='z';	// ascii = 122

#include <iostream>
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
#include "video/texture_rectangle.h"

#include "video/camerafuncs.h"
#include "geometry/plane.h"
#include "math/rotation.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;


// Forward declaration of methods:

colorfunc::RGB randomize_foreground_RGB();
void generate_random_rotation(
   threevector& n_hat,threevector& uprime_hat,threevector& vprime_hat);
void initialize_random_image(
   const vector<string>& nonsymbol_filenames,
   const texture_rectangle* random_texture_rectangle_ptr,
   int cropped_udim,int cropped_vdim,
   int& rx_start,int& ry_start);
void rotate_foreground_symbol(
   const texture_rectangle* texture_rectangle_ptr,
   const threevector& n_hat,const threevector& uprime_hat,
   const threevector& vprime_hat,
   texture_rectangle* rotated_texture_rectangle_ptr);
colorfunc::HSV peak_random_patch_hsv(
   int rx_start,int ry_start,
   unsigned int cropped_udim,unsigned int cropped_vdim,
   texture_rectangle* random_texture_rectangle_ptr);
void select_new_foreground_color_and_background_patch(
   bool random_background_flag,
   const vector<string>& nonsymbol_filenames,
   texture_rectangle* random_texture_rectangle_ptr,
   int cropped_udim,int cropped_vdim,
   int& rx_start,int& ry_start,
   colorfunc::RGB& foreground_RGB,colorfunc::RGB& background_RGB,
   colorfunc::RGB& alpha_RGB);
void replace_black_background(
   bool random_background_flag,
   colorfunc::RGB background_RGB,colorfunc::RGB foreground_RGB,
   colorfunc::RGB alpha,int rx_start,int ry_start,
   const twovector& origin,const twovector& e_hat,
   texture_rectangle* random_texture_rectangle_ptr,
   texture_rectangle* texture_rectangle_ptr);

int fluctuate_value(int R);
int shade_value(int R,double alpha,const twovector& q,
                const twovector& origin,const twovector& e_hat);
bool find_rotated_char_edges(
   texture_rectangle* rotated_texture_rectangle_ptr,
   unsigned int& pu_min,unsigned int& pu_max,
   unsigned int& pv_min,unsigned int& pv_max);
void gaussian_blur(string image_filename);

// ==========================================================================

colorfunc::RGB randomize_foreground_RGB()
{
   double hue=360*nrfunc::ran1();
   double saturation=0.7+0.3*nrfunc::ran1();
   double value=0.7+0.3*nrfunc::ran1();

   double r,g,b;
   colorfunc::hsv_to_RGB(hue,saturation,value,r,g,b);
//   int foreground_R=255*r;
//   int foreground_G=255*g;
//   int foreground_B=255*b;
   return colorfunc::RGB(255*r,255*g,255*b);
}

// -------------------------------------------------------------------------
// Method generate_random_rotation() generates a random 3D
// rotation which turns face-on character image to non-nadir
// views.  It returns an orthonormal coordinate system for the rotated
// imageplane.


void generate_random_rotation(
   threevector& n_hat, threevector& uprime_hat, threevector& vprime_hat)
{
//   cout << "inside generate_random_rotation()" << endl;

   rotation R;
   double az=0+45*nrfunc::gasdev();
   double el=0+45*nrfunc::gasdev();
   double roll=0+40*nrfunc::gasdev();

   double max_az = 50;
   double max_el = 50;
   double max_roll = 30;

   az=basic_math::max(az,-max_az);
   az=basic_math::min(az,max_az);
   el=basic_math::max(el,-max_el);
   el=basic_math::min(el,max_el);
   roll=basic_math::max(roll,-max_roll);
   roll=basic_math::min(roll,max_roll);
//      cout << "     az = " << az << " el = " << el << " roll = " << roll 
//           << endl;

/*
   cout << "Enter az:" << endl;
   cin >> az;
   cout << "Enter el:" << endl;
   cin >> el;
   cout << "Enter roll:" << endl;
   cin >> roll;
*/
 

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

// -------------------------------------------------------------------------
// Method initialize_random_image() imports some random internet image
// that should not contain any man-made symbols.

void initialize_random_image(
   const vector<string>& nonsymbol_filenames,
   texture_rectangle* random_texture_rectangle_ptr,
   unsigned int cropped_udim,unsigned int cropped_vdim,
   int& rx_start,int& ry_start)
{
//   cout << "inside initialize_random_image()" << endl;

   bool OK_random_image_flag = false;
   unsigned int xdim,ydim;
   string random_image_filename;
   while (!OK_random_image_flag)
   {
      int random_image_number=nrfunc::ran1()*nonsymbol_filenames.size();
      random_image_filename=nonsymbol_filenames[random_image_number];
//      cout << "random_image_filename = " << random_image_filename << endl;

      imagefunc::get_image_width_height(random_image_filename,xdim,ydim);
      if (xdim > cropped_udim && ydim > cropped_vdim) 
         OK_random_image_flag = true;
   }

   rx_start=nrfunc::ran1()*(xdim-cropped_udim);
   ry_start=nrfunc::ran1()*(ydim-cropped_vdim);
//   cout << "rx_start = " << rx_start 
//        << " ry_start = " << ry_start << endl;

   random_texture_rectangle_ptr->import_photo_from_file(
      random_image_filename);
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
   const texture_rectangle* texture_rectangle_ptr,
   texture_rectangle* rotated_texture_rectangle_ptr)
{
//   cout << "inside randomly_rotate_foreground_symbol()" << endl;
   
   unsigned int width=texture_rectangle_ptr->getWidth();
   unsigned int height=texture_rectangle_ptr->getHeight();
   double u0=0.5*double(width)/double(height);
   double v0=0.5;

   threevector n_hat,uprime_hat,vprime_hat;
   generate_random_rotation(n_hat,uprime_hat,vprime_hat);
   plane imageplane(n_hat,Zero_vector);

   for (unsigned int pu=0; pu<width; pu++)
   {
      double u=double(pu)/(width-1);
      double y=-(u-u0);
      for (unsigned int pv=0; pv<height; pv++)
      {
         double v=1-double(pv)/(height-1);
         double z=v-v0;

         threevector ray_basept(0,y,z);
         threevector intersection_pt;

         int R=0;
         int G=0;
         int B=0;
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
      } // loop over pv index
   } // loop over pu index
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

/*
   double peak_h=0;
   if (h.size() > 10)
   {
      prob_distribution prob_h(h,36,0);
      int n_max_h;
      prob_h.peak_density_value(n_max_h);
      peak_h=prob_h.get_x(n_max_h);
   }
   
   prob_distribution prob_s(s,20,0);
   int n_max_s;
   prob_s.peak_density_value(n_max_s);
   double peak_s=prob_s.get_x(n_max_s);

   prob_distribution prob_v(v,20,0);
   int n_max_v;
   prob_v.peak_density_value(n_max_v);
   double peak_v=prob_v.get_x(n_max_v);
   return colorfunc::HSV(peak_h,peak_s,peak_v);
*/


   return colorfunc::HSV(median_h,median_s,median_v);
}

// -------------------------------------------------------------------------
// Method select_new_foreground_color_and_background_patch() replaces
// synthetic character's pure white foreground and its pure black
// background with random colors.  The foreground and background
// colors are required to be reasonably distinguishable.

void select_new_foreground_color_and_background_patch(
   bool random_background_flag,
   const vector<string>& nonsymbol_filenames,
   texture_rectangle* random_texture_rectangle_ptr,
   unsigned int cropped_udim,unsigned int cropped_vdim,
   int& rx_start,int& ry_start,
   colorfunc::RGB& foreground_RGB,colorfunc::RGB& background_RGB,
   colorfunc::RGB& alpha_RGB)
{
   alpha_RGB.first=0.3*nrfunc::gasdev();
   alpha_RGB.second=0.3*nrfunc::gasdev();
   alpha_RGB.third=0.3*nrfunc::gasdev();

   bool colors_OK_flag=false;
   while (!colors_OK_flag)
   {
      colorfunc::HSV background_HSV;
      if (random_background_flag)
      {
         initialize_random_image(
            nonsymbol_filenames,random_texture_rectangle_ptr,
            cropped_udim,cropped_vdim,rx_start,ry_start);
         background_HSV=peak_random_patch_hsv(
            rx_start,ry_start,cropped_udim,cropped_vdim,
            random_texture_rectangle_ptr);
      }
      else 
      {
         background_RGB.first=nrfunc::ran1();
         background_RGB.second=nrfunc::ran1();
         background_RGB.third=nrfunc::ran1();
         background_HSV=colorfunc::RGB_to_hsv(background_HSV,true);
      }

      int foreground_counter=0;
      while (!colors_OK_flag && foreground_counter < 250)
      {
         foreground_RGB.first=255*nrfunc::ran1();
         foreground_RGB.second=255*nrfunc::ran1();
         foreground_RGB.third=255*nrfunc::ran1();
         colorfunc::HSV foreground_HSV=
            colorfunc::RGB_to_hsv(foreground_RGB,false);

// Require reasonable hue, saturation and value differences between
// character foreground and background colors:

         if (fabs(foreground_HSV.first-background_HSV.first) > 80 &&
             fabs(foreground_HSV.second-background_HSV.second) > 0.5 &&
             fabs(foreground_HSV.third-background_HSV.third) > 0.6) 
            colors_OK_flag=true;
         foreground_counter++;
      }
   
   } // !colors_OK_flag while loop
}

// -------------------------------------------------------------------------
// Method replace_black_background() overwrites the input character image's 
// pure black background with either part of some random image or else a 
// random color.  It then simulates simulates illumination variation
// across the input image.  Finally, it introduces gaussian noise into
// the image.

void replace_black_background(
   bool random_background_flag,
   colorfunc::RGB background_RGB,colorfunc::RGB foreground_RGB,
   colorfunc::RGB alpha_RGB,int rx_start,int ry_start,
   const twovector& origin,const twovector& e_hat,
   texture_rectangle* random_texture_rectangle_ptr,
   texture_rectangle* texture_rectangle_ptr)
{
//   cout << "inside replace_black_background()" << endl;
//   cout << "Random_background_flag = " << random_background_flag << endl;

   unsigned int width=texture_rectangle_ptr->getWidth();
   unsigned int height=texture_rectangle_ptr->getHeight();
//   cout << "width = " << width << " height = " << height << endl;

   const int min_value=128;
   for (unsigned int pv=0; pv<height; pv++)
   {
      for (unsigned int pu=0; pu<width; pu++)
      {
         int R,G,B;
         texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);

         int Rnew,Gnew,Bnew;
         if (R < min_value && G < min_value && B < min_value)
         {
            if (random_background_flag)
            {
               random_texture_rectangle_ptr->get_pixel_RGB_values(
                  rx_start+pu,ry_start+pv,Rnew,Gnew,Bnew);
            }
            else
            {
               Rnew=255*background_RGB.first;
               Gnew=255*background_RGB.second;
               Bnew=255*background_RGB.third;
            }
         }
         else
         {
            Rnew=foreground_RGB.first;
            Gnew=foreground_RGB.second;
            Bnew=foreground_RGB.third;
         }

// Incorporate linear shading to simulate illumination variations
// across characters:

         twovector q(pu,pv);
         Rnew=shade_value(Rnew,alpha_RGB.first,q,origin,e_hat);
         Gnew=shade_value(Gnew,alpha_RGB.second,q,origin,e_hat);
         Bnew=shade_value(Bnew,alpha_RGB.third,q,origin,e_hat);

// Add gaussian noise to foreground and background:

         Rnew=fluctuate_value(Rnew);
         Gnew=fluctuate_value(Gnew);
         Bnew=fluctuate_value(Bnew);

         texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,Rnew,Gnew,Bnew);
      } // loop over pu
   } // loop over pv
}

// -------------------------------------------------------------------------
int fluctuate_value(int R)
{
//   cout << "inside fluctuate_value()" << endl;
   const int sigma=0.1 * 256;
//   const int sigma=100;
   int R_fluctuation=sigma*nrfunc::gasdev();
   R += R_fluctuation;

   R = basic_math::max(0,R);
   R = basic_math::min(255,R);
   return R;
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
// Method find_rotated_char_edges returns the left/right and
// top/bottom pixel locations for the rotated character contained
// inside *rotated_texture_rectangle_ptr.  If this boolean method is
// unable to determine the character's edges, it returns false.

bool find_rotated_char_edges(
   texture_rectangle* rotated_texture_rectangle_ptr,
   unsigned int& pu_min,unsigned int& pu_max,
   unsigned int& pv_min,unsigned int& pv_max)
{
   unsigned int width=rotated_texture_rectangle_ptr->getWidth();
   unsigned int height=rotated_texture_rectangle_ptr->getHeight();

   unsigned int total_column_sum=0;
   vector<int> column_sum;

   for (unsigned int pu=0; pu<width; pu++)
   {
      unsigned int curr_column_sum=0;
      for (unsigned int pv=0; pv<height; pv++)
      {
         int R,G,B;
         rotated_texture_rectangle_ptr->get_pixel_RGB_values(
            pu,pv,R,G,B);
         curr_column_sum += R+G+B;
      } // loop over pv index
      column_sum.push_back(curr_column_sum);
      total_column_sum += curr_column_sum;
   } // loop over pu index

   const unsigned int min_total_column_sum=100;
   if (column_sum[0] > 0 || column_sum[width-1] > 0 ||
       total_column_sum < min_total_column_sum)
   {
      return false;
   }

   pu_min=0;
   for (unsigned int c=0; c<column_sum.size(); c++)
   {
      if (column_sum[c] > 0)
      {
         pu_min=c;
         break;
      }
   }
//   cout << "pu_min = " << pu_min << endl;

   pu_max=width-1;
   for (unsigned int c=column_sum.size()-1; c >= 0; c--)
   {
      if (column_sum[c] > 0)
      {
         pu_max=c;
         break;
      }
   }
//   cout << "pu_max = " << pu_max << endl;

// Find top and bottom edges of rotated character:

   vector<int> row_sum;
   for (unsigned int pv=0; pv<height; pv++)
   {
      unsigned int curr_row_sum=0;
      for (unsigned int pu=0; pu<width; pu++)
      {
         int R,G,B;
         rotated_texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
         curr_row_sum += R+G+B;
      } // loop over pv index
      row_sum.push_back(curr_row_sum);
//      cout << "pv = " << pv
//           << " row_sum = " << row_sum.back() << endl;
   } // loop over pu index

   if (row_sum[0] > 0 || row_sum[height-1] > 0) return false;

   pv_min=0;
   for (unsigned int r=0; r<row_sum.size(); r++)
   {
      if (row_sum[r] > 0)
      {
         pv_min=r;
         break;
      }
   }
//   cout << "pv_min = " << pv_min << endl;

   pv_max=height-1;
   for (unsigned int r=row_sum.size()-1; r >= 0; r--)
   {
      if (row_sum[r] > 0)
      {
         pv_max=r;
         break;
      }
   }
//   cout << "pv_max = " << pv_max << endl;

   return true;
}

// -------------------------------------------------------------------------
// Method gaussian_blur() smears the input image file with variable
// sigma values.

void gaussian_blur(string image_filename)
{
//   cout << "inside gaussian_blur()" << endl;
   double sigma;
   double blur_flag=nrfunc::ran1();
   if (blur_flag < 0.5)
   {
      sigma=0;
   }
   else if (blur_flag < 0.7)
   {
      sigma=0.3;
   }
   else if (blur_flag < 0.9)
   {
      sigma=0.75;
   }
   else 
   {
      sigma=1.5;
   }
      
   if (sigma > 0)
   {
     string unix_cmd="convert -blur 0x"
        +stringfunc::number_to_string(sigma)+" "+image_filename+" "+
        image_filename;
     sysfunc::unix_command(unix_cmd);
   }
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Import a variety of ttf font files:

   string fonts_subdir="./fonts/";
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("ttf");
   vector<string> font_paths=filefunc::
      files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,fonts_subdir);

   string char_pics_subdir="./char_pics/";
   filefunc::dircreate(char_pics_subdir);
//   string synthetic_subdir="./synthetic_numbers/";
   string synthetic_subdir="./training_data/synthetic_chars/";
   filefunc::dircreate(synthetic_subdir);

   int fontsize=50;
   int npx=2.25*fontsize;
   int npy=2.25*fontsize;
   unsigned int width=64 * npx;
   unsigned int height=npy;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   string blank_filename="blank.jpg";
   if (!filefunc::fileexist(blank_filename))
   {
      texture_rectangle_ptr->generate_blank_image_file(
         width,height,blank_filename,0);
   }

   string font_pics_subdir="./fonts/font_pictures/";
   int i_start=0;
   unsigned int n_fonts = font_paths.size();
   for (unsigned int i=i_start; i<n_fonts; i++)
   {
      string background_color="white";
      colorfunc::RGB foreground_RGB(0,0,0);

      string curr_font_path=font_paths[i%font_paths.size()];
      string font_basename=filefunc::getbasename(curr_font_path);
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         font_basename," .\t\n");

      int point_size=100;
      string image_basename=font_pics_subdir+
         +"font_"+substrings[0];
      string jpg_filename=image_basename+".jpg";
      string label="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      label += "abcdefghijklmnopqrstuvwxyz";
      imagefunc::generate_text_via_ImageMagick(
	background_color,
        foreground_RGB.first,foreground_RGB.second,foreground_RGB.third,
	curr_font_path,point_size,width,height,
	label,jpg_filename);

      string banner="Exported "+jpg_filename;
      outputfunc::write_banner(banner);

   } // loop over index i labeling cleaned input characters

   cout << endl;

   delete texture_rectangle_ptr;

} 

