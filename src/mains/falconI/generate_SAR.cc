// ========================================================================
// Program GENERATE_SAR is a playground for simulating SAR imagery
// which can be inserted as 3D frusta in the HAFB ALIRT map.
// ========================================================================
// Last updated on 10/1/12; 10/2/12; 10/3/12
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "geometry/bounding_box.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "numrec/nrfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

#include "general/outputfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

//   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
//   int texturepass_ID=passes_group.get_curr_texturepass_ID();

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   photogroup_ptr->set_bundler_IO_subdir(bundler_IO_subdir);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   vector<fourvector> SAR_tgts;
   SAR_tgts.push_back(fourvector(
      311211.420898, 4702559.60039, 38.6437454224, 1));
   SAR_tgts.push_back(fourvector(
      311526.080928, 4702278.34791, 57.6246490479, 1));
   SAR_tgts.push_back(fourvector(
      311120.400302, 4702888.42887, 38.9729156494, 0.8));
   SAR_tgts.push_back(fourvector(
      310857.55762, 4701824.7056, 49.0043830872, 0.8));
   SAR_tgts.push_back(fourvector(
      312091.862572, 4702964.69371, 23.7440109253, 0.6));
   SAR_tgts.push_back(fourvector(
      312829.173717, 4702421.57967, 42.8321037292, 0.6));

   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      string image_filename=photo_ptr->get_filename();
      texture_rectangle_ptr->reset_texture_content(image_filename);
      int width=texture_rectangle_ptr->getWidth();
      int height=texture_rectangle_ptr->getHeight();
      
      camera* camera_ptr=photo_ptr->get_camera_ptr();
//      cout << "n = " << n
//           << " camera = " << *camera_ptr 
//           << " camera posn = " << camera_ptr->get_world_posn()
//           << endl;
      texture_rectangle_ptr->clear_all_RGB_values();
      
      vector<threevector> UVSignal;
      for (unsigned int t=0; t<SAR_tgts.size(); t++)
      {
         threevector SAR_tgt_posn(SAR_tgts[t]);
         if (!camera_ptr->XYZ_in_front_of_camera(SAR_tgt_posn)) continue;
         threevector UVW;
         camera_ptr->project_XYZ_to_UV_coordinates(SAR_tgt_posn,UVW);

// Add random scintillation to SAR target strength:

         double ran_number=2*(nrfunc::ran1()-0.5);
         double SAR_tgt_strength=SAR_tgts[t].get(3)+0.35*ran_number;
         SAR_tgt_strength=basic_math::max(SAR_tgt_strength,0.5);
         SAR_tgt_strength=basic_math::min(SAR_tgt_strength,1.0);
         UVW.put(2,SAR_tgt_strength);

         UVSignal.push_back(UVW);
//         cout << "t = " << t << " UVW = " << UVW << endl;
      }

// Add low-level random noise background:

      int n_noise_blobs=nrfunc::ran1()*120;
      for (int b=0; b<n_noise_blobs; b++)
      {
         double u_noise=nrfunc::ran1()*texture_rectangle_ptr->get_maxU();
         double v_noise=nrfunc::ran1()*texture_rectangle_ptr->get_maxV();
         double noise_Amp=nrfunc::ran1()*0.5;
         UVSignal.push_back(threevector(u_noise,v_noise,noise_Amp));
      } // loop over index b labeling noise blobs

      for (unsigned int t=0; t<UVSignal.size(); t++)
      {
         double U=UVSignal[t].get(0);
         double V=UVSignal[t].get(1);
         double SAR_signal=UVSignal[t].get(2);
         
         if (U < texture_rectangle_ptr->get_minU()) continue;
         if (U > texture_rectangle_ptr->get_maxU()) continue;
         if (V < texture_rectangle_ptr->get_minV()) continue;
         if (V > texture_rectangle_ptr->get_maxV()) continue;

         unsigned int pu,pv;
         texture_rectangle_ptr->get_pixel_coords(U,V,pu,pv);
         double sigma=10+10*nrfunc::ran1();
         unsigned int radius=4*sigma;
         for (unsigned int py=pv-radius; py<pv+radius; py++)
         {
            if (py < 0)  continue;
            if (py >= height) continue;
            for (int px=pu-radius; px<pu+radius; px++)
            {
               if (px < 0) continue;
               if (px >= width) continue;
               double rsq=sqr(pu-px)+sqr(pv-py);
               double decay_factor=exp(-rsq/(2*sqr(sigma)));
               double Amp=SAR_signal*decay_factor;

               double hue=(1-Amp)*300;
               double sat=1;
               double value=2*Amp;
               if (value > 1) value=1;
               double r,g,b;
               colorfunc::hsv_to_RGB(hue,sat,value,r,g,b);
               int R=255*r;
               int G=255*g;
               int B=255*b;
               int R_curr,B_curr,G_curr;
               texture_rectangle_ptr->get_pixel_RGB_values(
                  px,py,R_curr,G_curr,B_curr);
               R_curr += R;
               G_curr += G;
               B_curr += B;
               if (R_curr > 255) R_curr=255;
               if (G_curr > 255) G_curr=255;
               if (B_curr > 255) B_curr=255;
               texture_rectangle_ptr->set_pixel_RGB_values(
                  px,py,R_curr,G_curr,B_curr);
            } // loop over px index
         } // loop over py index

      } // loop over index t labeling SAR tgts

      string SAR_subdir="./SAR_images/";
      filefunc::dircreate(SAR_subdir);
      string SAR_output_filename=SAR_subdir+
         "SAR_"+stringfunc::integer_to_string(n,3)+".jpg";
      texture_rectangle_ptr->write_curr_frame(SAR_output_filename);
      string banner="Exported "+SAR_output_filename;
      outputfunc::write_big_banner(banner);

   } // loop over index n labeling aerial images

}

