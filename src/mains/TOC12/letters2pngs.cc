// ==========================================================================
// Program LETTERS2PNGS is a variant of CHARS2PNGS.  It imports
// PNG/JPG images of symbol "letters" generated via LOCATE_CHARS and
// connected_components::export_individual_connected_components()
// enabled.  The UV image is initially oriented in the YZ world-plane.
// It is rotated through az about the world z axis, el about the world
// y-axis and roll about the the world x-axis.  The az, el and roll
// angles are random variables selected from gaussian distributions
// with reasonable standard deviations to simulate camera views of
// TOC12 symbols in the wild.  

// The rotated synthetic character image is subsequently projected
// back into the YZ world-plane.  The projected image is cropped so
// that the rotated character fills most of the projection.  The
// cropped character is scaled so that its height equals 32 pixels
// in size.  Finally, the cropped character is blurred by some random
// amount.

//			       letters2pngs

// ==========================================================================
// Last updated on 8/15/12; 8/30/12; 8/31/12
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

#include "video/camerafuncs.h"
#include "geometry/plane.h"
#include "math/rotation.h"
#include "video/videofuncs.h"

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

   string char_type;
   cout << "Enter 'b' for bright chars against dark background " << endl;
   cout << " or 'd' for dark chars against bright background" << endl;
   cin >> char_type;

   bool bright_ccs_flag=true;
   if (char_type=="d")
   {
      bright_ccs_flag=false;
   }
   
   if (bright_ccs_flag)
   {
      cout << "Generating bright chars against dark background" << endl;
   }
   else
   {
      cout << "Generating dark chars against bright background" << endl;
   }

//   string alphabet_subdir="./images/final_signs/alphabet/";
   string alphabet_subdir="./images/ppt_signs/alphabet/";
   string ccs_subdir=alphabet_subdir;
   if (bright_ccs_flag)
   {
      ccs_subdir += "bright_connected_components/";
   }
   else
   {
      ccs_subdir += "dark_connected_components/";
   }
   string synthetic_subdir=ccs_subdir+"synthetic_letters/";

   string symbol_name;
   cout << "Enter symbol name (e.g. 'biohazard','eat','skull'):" << endl;
   cin >> symbol_name;

   string symbol_inputs_subdir=ccs_subdir+symbol_name+"/";

   string synthetic_output_subdir=synthetic_subdir+symbol_name+"/";
   filefunc::dircreate(synthetic_output_subdir);

   int fontsize=50;
   int npx=2.25*fontsize;
   int npy=2.25*fontsize;
   int width=npx;
   int height=npy;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* warped_texture_rectangle_ptr=new texture_rectangle();
   string blank_filename="blank.jpg";
   warped_texture_rectangle_ptr->generate_blank_image_file(
      width,height,blank_filename,0);

   vector<string> symbol_filenames=filefunc::image_files_in_subdir(
      symbol_inputs_subdir);

//   int n_iters=5;
//   int n_iters=500;
//   int n_iters=1000;
//   int n_iters=2000;
   int n_iters=3000;
   int synthetic_char_counter=0;
   for (int iter=0; iter<n_iters; iter++)
   {
      cout << "iter = " << iter << " of " << n_iters << endl;

// Generate rotation which turns face-on character image to non-nadir
// views:

      rotation R;
      double az=0+60*nrfunc::gasdev();
      double el=0+40*nrfunc::gasdev();
      double roll=0+40*nrfunc::gasdev();

      az=basic_math::max(az,-80.0);
      az=basic_math::min(az,80.0);
      el=basic_math::max(el,-70.0);
      el=basic_math::min(el,70.0);
      roll=basic_math::max(roll,-60.0);
      roll=basic_math::min(roll,60.0);
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

// Perform gaussian blurring of symbol letters with variable sigma
// values:

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

// Perform same rotation and blur for each "letter" within the symbol:

      for (int i=0; i<symbol_filenames.size(); i++)
      {
         string jpg_filename=symbol_filenames[i];
         texture_rectangle_ptr->import_photo_from_file(jpg_filename);
         warped_texture_rectangle_ptr->import_photo_from_file(blank_filename);

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

//            cout << "ray_basept = " << ray_basept << endl;
//            cout << "intersection_pt = " << intersection_pt << endl;
               warped_texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
            } // loop over pv index
         } // loop over pu index

// Find left and right edges of warped character:

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

/*
         if (column_sum[0] > 0 || column_sum[width-1] > 0)
         {
//         cout << "i = " << i << endl;
//         cout << "column_sum[0] = " << column_sum[0] << endl;
//         cout << "column_sum[1] = " << column_sum[1] << endl;
//         cout << "column_sum[width-1] = " << column_sum[width-1] << endl;
//         cout << "column_sum[width-2] = " << column_sum[width-2] << endl;
//         cout << endl;
            continue;
         }
         else
         {
//         cout << "i = " << i 
//              << " synthetic_char_counter = "
//              << synthetic_char_counter << endl;
//         outputfunc::enter_continue_char();
         }
*/

         int pu_min=0;
         for (int c=0; c<column_sum.size(); c++)
         {
            if (column_sum[c] > 0)
            {
               pu_min=c;
               break;
            }
         }
//      cout << "pu_min = " << pu_min << endl;

         int pu_max=width-1;
         for (int c=column_sum.size()-1; c >= 0; c--)
         {
            if (column_sum[c] > 0)
            {
               pu_max=c;
               break;
            }
         }
//      cout << "pu_max = " << pu_max << endl;

// Find top and bottom edges of warped character:

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

/*
         if (row_sum[0] > 0 || row_sum[1] > 0 || 
         row_sum[height-1] > 0 || row_sum[height-2] > 0) continue;
*/

         int pv_min=0;
         for (int r=0; r<row_sum.size(); r++)
         {
            if (row_sum[r] > 0)
            {
               pv_min=r;
               break;
            }
         }
//      cout << "pv_min = " << pv_min << endl;

         int pv_max=height-1;
         for (int r=row_sum.size()-1; r >= 0; r--)
         {
            if (row_sum[r] > 0)
            {
               pv_max=r;
               break;
            }
         }
//      cout << "pv_max = " << pv_max << endl;

//      string orig_filename="orig.jpg";
//      texture_rectangle_ptr->write_curr_frame(orig_filename);

//      string warped_filename="warped.jpg";
//      warped_texture_rectangle_ptr->write_curr_frame(warped_filename);

// Crop warped character image file to remove unnecessary black sea
// surrounding colored character region:
      
         int cropped_udim=pu_max-pu_min+1;
         int cropped_vdim=pv_max-pv_min+1;
         string cropped_filename="cropped.jpg";

//      imagefunc::crop_image(
//         warped_filename,cropped_filename,
//         cropped_udim,cropped_vdim,pu_min,pv_min);

// On 8/31/12, we learned the painful way that 8-bit greyscale rather
// than 24-bit RGB values are exported when R=G=B.  To prevent this
// compression, we explicitly introduce small color fluctuations
// before writing out the warped_texture_rectangle's contents:

         warped_texture_rectangle_ptr->minutely_perturb_RGB_values();

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

      } // loop over index i labeling symbol "letters"
   } // loop over iter index

   delete texture_rectangle_ptr;
   delete warped_texture_rectangle_ptr;

   string banner="Exported "+stringfunc::number_to_string(
      synthetic_char_counter)+" synthetic letters to "+
      synthetic_output_subdir;
   outputfunc::write_big_banner(banner);
   
} 

