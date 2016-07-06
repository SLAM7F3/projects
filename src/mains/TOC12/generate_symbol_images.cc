// ==========================================================================
// Program GENERATE_SYMBOL_IMAGES is a variant of
// GENERATE_CHAR_IMAGES.  It imports PNG images of TOC12 symbol
// "words".  The UV image is initially oriented in the YZ world-plane.
// It is rotated through az about the world z axis, el about the world
// y-axis and roll about the the world x-axis.  The az, el and roll
// angles are random variables selected from gaussian distributions
// with reasonable standard deviations to simulate camera views of
// TOC12 symbols in the wild.

// The rotated synthetic colored symbol image is subsequently
// projected back into the YZ world-plane.  The projected image is
// pasted onto a randomly colored background.  On 8/15/12, Geoff Brown
// recommended introducing more random variation into the background
// in order to desensitize the classifier to actual background
// variations.

// The projected synthetic image is cropped so that the rotated
// word fills most of the projection.  The cropped word is scaled so
// that its height equals 32 pixels in size.  Finally, the cropped
// word is blurred by some random amount.

//			  generate_symbol_images

// ==========================================================================
// Last updated on 10/1/12; 10/20/12; 10/30/12
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
#include "image/pngfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

#include "video/camerafuncs.h"
#include "geometry/plane.h"
#include "math/rotation.h"
#include "video/videofuncs.h"

int fluctuate_value(int R)
{
   const int sigma=10;
   int R_fluctuation=sigma*nrfunc::gasdev();
   R += R_fluctuation;

   R = basic_math::max(0,R);
   R = basic_math::min(255,R);
   return R;
}

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

void shade_values(
   int& R,int& G,int& B,double alpha,const twovector& q,
   const twovector& origin,const twovector& e_hat)
{
   twovector p(q-origin);
   twovector p_perp(p-p.dot(e_hat)*e_hat);
   twovector f_hat(-e_hat.get(1),e_hat.get(0));
   double d=p_perp.dot(f_hat);

   R += alpha*d;
   R = basic_math::max(0,R);
   R = basic_math::min(255,R);

   G += alpha*d;
   G = basic_math::max(0,G);
   G = basic_math::min(255,G);

   B += alpha*d;
   B = basic_math::max(0,B);
   B = basic_math::min(255,B);
}

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);

//   nrfunc::init_time_based_seed();

   vector<string> symbol_names;
   symbol_names.push_back("yellow_radiation");
//   symbol_names.push_back("orange_biohazard");
   symbol_names.push_back("blue_radiation");
   symbol_names.push_back("blue_water");
//   symbol_names.push_back("blue_gas");
//   symbol_names.push_back("red_stop");
//   symbol_names.push_back("green_start");
//   symbol_names.push_back("bw_skull");
//   symbol_names.push_back("bw_eat");

   string final_signs_subdir="./images/final_signs/";
//   string ppt_signs_subdir="./images/ppt_signs/";
   string symbols_input_subdir=final_signs_subdir;

   string nonsymbols_subdir="./images/non_signs/more_internet/";
   vector<string> nonsymbol_filenames=filefunc::image_files_in_subdir(
      nonsymbols_subdir);
   cout << "nonsymbol-filenames.size() = " << nonsymbol_filenames.size()
        << endl;

//   string symbol_name;
//   cout << "Enter symbol name:" << endl;
//   cout << "  yellow_radiation,orange_biohazard,blue_water" << endl;
//   cout << "  blue_radiation,blue_gas,red_stop" << endl;
//   cout << "  green_start:" << endl;
//   cin >> symbol_name;

   for (int s=0; s<symbol_names.size(); s++)
   {
      string symbol_name=symbol_names[s];

      string symbol_filename=symbols_input_subdir+symbol_name+".png";
      string synthetic_subdir=symbols_input_subdir+"synthetic_symbols/";
      string synthetic_output_subdir=synthetic_subdir+symbol_name+"/";
      filefunc::dircreate(synthetic_output_subdir);

      int fontsize=50;
      int npx=2.25*fontsize;
      int npy=2.25*fontsize;
      int width=npx;
      int height=npy;

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
      texture_rectangle* random_texture_rectangle_ptr=new texture_rectangle();
      texture_rectangle* warped_texture_rectangle_ptr=new texture_rectangle();
      string blank_filename="blank.jpg";

//   int n_iters=5;
//   int n_iters=20;
//      int n_iters=100;
//   int n_iters=500;
//   int n_iters=1000;
//   int n_iters=2000;
//   int n_iters=5000;
//   int n_iters=10000;
//      int n_iters=12000;
      int n_iters=15000;
      int synthetic_char_counter=0;
      timefunc::initialize_timeofday_clock();

      for (int iter=0; iter<n_iters; iter++)
      {
         cout << "iter = " << iter << " of " << n_iters << endl;

         if (iter > 0 && iter%25==0)
         {
            double elapsed_time=timefunc::elapsed_timeofday_time();
            double processing_rate=elapsed_time/iter;
            int n_remaining=n_iters-iter;
            double remaining_time=n_remaining*processing_rate;
            cout << "-----------------------------------------------------"
                 << endl;
            cout << "Elapsed time = " << elapsed_time << " secs = " 
                 << elapsed_time / 60.0 << " minutes" << endl;
            cout << "Remaining time = " << remaining_time/60.0 
                 << " minutes" << endl;
            cout << "-----------------------------------------------------"
                 << endl << endl;
         }

// Generate rotation which turns face-on character image to non-nadir
// views:

         rotation R;
         double az=0+40*nrfunc::gasdev();
         double el=0+75*(2*nrfunc::ran1()-1.0);
         double roll=0+180*(2*nrfunc::ran1()-1.0);

//         az=basic_math::max(az,-1.0);
//         az=basic_math::min(az,1.0);
         az=basic_math::max(az,-55.0);
         az=basic_math::min(az,55.0);

//         el=basic_math::max(el,-1.0);
//         el=basic_math::min(el,1.0);
         el=basic_math::max(el,-65.0);
         el=basic_math::min(el,65.0);

//         roll=basic_math::max(roll,-1.0);
//         roll=basic_math::min(roll,1.0);
//         roll=basic_math::max(roll,-60.0);
//         roll=basic_math::min(roll,60.0);
//         roll=basic_math::max(roll,-180.0);
//         roll=basic_math::min(roll,180.0);
         cout << "     az = " << az << " el = " << el << " roll = " << roll 
              << endl;

         az *= PI/180;
         el *= PI/180;
         roll *= PI/180;
         R=R.rotation_from_az_el_roll(az,el,roll);
//      cout << "R = " << R << endl;

         threevector n_hat=R*x_hat;
         threevector uprime_hat=-R*y_hat;
         threevector vprime_hat=R*z_hat;

//      cout << "n_hat = " << n_hat << endl;
//      cout << "uprime_hat = " << uprime_hat << endl;
//      cout << "vprime_hat = " << vprime_hat << endl;

         plane imageplane(n_hat,Zero_vector);
         double u0=0.5*double(width)/double(height);
         double v0=0.5;

// Perform gaussian blurring of symbol with variable sigma values:

         int sigma_blur=0;
         double blur_flag=nrfunc::ran1();
         if (blur_flag < 0.5)
         {
         }
         else if (blur_flag < 0.7)
         {
            sigma_blur=0.3;
         }
         else if (blur_flag < 0.9)
         {
            sigma_blur=0.75;
         }
         else 
         {
            sigma_blur=1.5;
         }

         texture_rectangle_ptr->generate_blank_image_file(
            width,height,blank_filename,128);

         bool generate_blank_symbol_flag=false;
//      bool generate_blank_symbol_flag=true;
         if (generate_blank_symbol_flag)
         {
            texture_rectangle_ptr->import_photo_from_file(blank_filename);
            double blank_foreground_red=255*nrfunc::ran1();
            double blank_foreground_green=255*nrfunc::ran1();
            double blank_foreground_blue=255*nrfunc::ran1();

            for (int pu=0; pu<width; pu++)
            {
               for (int pv=0; pv<height; pv++)
               {
                  texture_rectangle_ptr->set_pixel_RGB_values(
                     pu,pv,blank_foreground_red,blank_foreground_green,
                     blank_foreground_blue);
               }
            }
         }
         else
         {
            cout << "symbol_filename = " << symbol_filename << endl;
            texture_rectangle_ptr->import_photo_from_file(symbol_filename);
         }

         warped_texture_rectangle_ptr->import_photo_from_file(blank_filename);


         bool random_background_flag=false;
         int rx_start,ry_start;
         double background_red,background_green,background_blue;
         double random_image_background=nrfunc::ran1();
         const double random_image_background_frac=0.5;
         if (random_image_background < random_image_background_frac)
         {
            random_background_flag=true;
            int random_image_number=nrfunc::ran1()*nonsymbol_filenames.size();
            string random_image_filename=nonsymbol_filenames[
               random_image_number];
//            cout << "random_image_filename = " << random_image_filename
//                 << endl;
            unsigned int xdim,ydim;
            imagefunc::get_image_width_height(
               random_image_filename,xdim,ydim);
            rx_start=nrfunc::ran1()*(xdim-32);
            ry_start=nrfunc::ran1()*(ydim-32);
//            cout << "rx_start = " << rx_start 
//                 << " ry_start = " << ry_start << endl;
            random_texture_rectangle_ptr->import_photo_from_file(
               random_image_filename);
         }
         else
         {

// Replace character's pure black background with random color:

            background_red=nrfunc::ran1();
            background_green=nrfunc::ran1();
            background_blue=nrfunc::ran1();
            double background_h,background_s,background_v;
            colorfunc::RGB_to_hsv(
               background_red,background_green,background_blue,
               background_h,background_s,background_v);

// Bias TOC12 sign background toward "washed-out" colors and away from
// pure hues:

         background_s -= 0.375;
//            background_s -= 0.5;
            background_s=basic_math::max(0.0,background_s);
            colorfunc::hsv_to_RGB(
               background_h,background_s,background_v,
               background_red,background_green,background_blue);
         }
         

         double alpha_R=0.3*nrfunc::gasdev();
         double alpha_G=0.3*nrfunc::gasdev();
         double alpha_B=0.3*nrfunc::gasdev();

         twovector origin(width*nrfunc::ran1(),height*nrfunc::ran1());
         double theta=2*PI*nrfunc::ran1();
         twovector e_hat(cos(theta),sin(theta));

// As of 10/1/12, we experiment with generating simulated TOC12 signs
// with saturations and values which range from 0.5 to 1:

// As of 10/20/12, we experiment with generating simulated TOC12 signs
// with saturations and values which range from 0.33 to 1:

         double s_factor=0.33+0.67*nrfunc::ran1();
         double v_factor=0.33+0.67*nrfunc::ran1();

         double x,y;
         for (int pu=0; pu<width; pu++)
         {
            double u=double(pu)/(width-1);
            double y=-(u-u0);
            for (int pv=0; pv<height; pv++)
            {
               double v=1-double(pv)/(height-1);
               double z=v-v0;

               threevector ray_basept(0,y,z);
               threevector intersection_pt;

               int R=-1;
               int G=-1;
               int B=-1;
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
                  if (R >= 0 && G >= 0 && B >= 0)
                  {
                     double curr_h,curr_s,curr_v;
                     texture_rectangle_ptr->get_hsv_values(
                        uprime,vprime,curr_h,curr_s,curr_v);

                     curr_s *= s_factor;
                     curr_v *= v_factor;
                     double r,g,b;
                     colorfunc::hsv_to_RGB(curr_h,curr_s,curr_v,r,g,b);
                     R=255*r;
                     G=255*g;
                     B=255*b;
                  }
               }

//            cout << "ray_basept = " << ray_basept << endl;
//            cout << "intersection_pt = " << intersection_pt << endl;

               if (R < 0 || G < 0 || B < 0)
               {
                  if (random_background_flag)
                  {
                     random_texture_rectangle_ptr->get_pixel_RGB_values(
                        rx_start+pu,ry_start+pv,R,G,B);
                  }
                  else
                  {
                     R=255*background_red;
                     G=255*background_green;
                     B=255*background_blue;
                  }
               }

// Incorporate linear shading to simulate illumination variations
// across characters:

               twovector q(pu,pv);
               R=shade_value(R,alpha_R,q,origin,e_hat);
               G=shade_value(G,alpha_G,q,origin,e_hat);
               B=shade_value(B,alpha_B,q,origin,e_hat);

// Add gaussian noise to letter color values:

               R=fluctuate_value(R);
               G=fluctuate_value(G);
               B=fluctuate_value(B);

               warped_texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);

            } // loop over pv index
         } // loop over pu index

         int pu_min=0;
         int pv_min=0;
         int pu_max=111;
         int pv_max=111;

/*

// Find left and right edges of warped symbol:

         vector<int> column_sum;
         for (int pu=0; pu<width; pu++)
         {
            int curr_column_sum=0;
            for (int pv=0; pv<height; pv++)
            {
               int R,G,B;
               warped_texture_rectangle_ptr->get_pixel_RGB_values(
                  pu,pv,R,G,B);
               curr_column_sum += R+G+B;
            } // loop over pv index
            column_sum.push_back(curr_column_sum);
//         cout << "pu = " << pu
//              << " column_sum = " << column_sum.back() << endl;
         } // loop over pu index

         int pu_min=0;
         for (int c=0; c<column_sum.size(); c++)
         {
            if (column_sum[c] > 0)
            {
               pu_min=c;
               break;
            }
         }
         cout << "pu_min = " << pu_min << endl;

         int pu_max=width-1;
         for (int c=column_sum.size()-1; c >= 0; c--)
         {
            if (column_sum[c] > 0)
            {
               pu_max=c;
               break;
            }
         }
         cout << "pu_max = " << pu_max << endl;

// Find top and bottom edges of warped symbol:

         vector<int> row_sum;
         for (int pv=0; pv<height; pv++)
         {
            int curr_row_sum=0;
            for (int pu=0; pu<width; pu++)
            {
               int R,G,B;
               warped_texture_rectangle_ptr->get_pixel_RGB_values(
                  pu,pv,R,G,B);
               curr_row_sum += R+G+B;
            } // loop over pv index
            row_sum.push_back(curr_row_sum);
//         cout << "pv = " << pv
//              << " row_sum = " << row_sum.back() << endl;
         } // loop over pu index

         int pv_min=0;
         for (int r=0; r<row_sum.size(); r++)
         {
            if (row_sum[r] > 0)
            {
               pv_min=r;
               break;
            }
         }
      cout << "pv_min = " << pv_min << endl;

         int pv_max=height-1;
         for (int r=row_sum.size()-1; r >= 0; r--)
         {
            if (row_sum[r] > 0)
            {
               pv_max=r;
               break;
            }
         }
      cout << "pv_max = " << pv_max << endl;
*/

         string orig_filename="orig.jpg";
         texture_rectangle_ptr->write_curr_frame(orig_filename);

         string warped_filename="warped.jpg";
         warped_texture_rectangle_ptr->write_curr_frame(warped_filename);

// Crop warped character image file to remove unnecessary black sea
// surrounding colored character region:
      
         int cropped_udim=pu_max-pu_min+1;
         int cropped_vdim=pv_max-pv_min+1;
         string cropped_filename="cropped.jpg";

         warped_texture_rectangle_ptr->write_curr_frame(
            pu_min,pu_max,height-pv_max,height-pv_min,cropped_filename);

// Rescale cropped image so that new height equals 32 pixels in size:

         int new_vdim=32;
         int new_udim=cropped_udim*new_vdim/double(cropped_vdim);

         string resized_filename=synthetic_output_subdir+"synthetic_char_"+
            stringfunc::integer_to_string(synthetic_char_counter++,5)+".png";
      
         videofunc::resize_image(
            cropped_filename,cropped_udim,cropped_vdim,new_udim,new_vdim,
            resized_filename);

// Perform gaussian blurring of final image with variable sigma
// values:

         if (sigma_blur > 0)
         {
            string unix_cmd="convert -blur "+stringfunc::number_to_string(
               3*sigma_blur)+
               "x"+stringfunc::number_to_string(sigma_blur)+" "+
               resized_filename+" "+resized_filename;
            sysfunc::unix_command(unix_cmd);
         }

      } // loop over iter index

      delete texture_rectangle_ptr;
      delete random_texture_rectangle_ptr;
      delete warped_texture_rectangle_ptr;

      string banner="Exported "+stringfunc::number_to_string(
         synthetic_char_counter)+" synthetic symbols to "+
         synthetic_output_subdir;
      outputfunc::write_big_banner(banner);

   } // loop over index s labeling symbol names
   
} 

