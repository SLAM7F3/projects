// ==========================================================================
// Program ISAR_ISAR_FUSION reads in 3D averaged and instantaneous
// ISAR XYZRGBA files.  RGB values are converted to HSV.  We set the
// fused image's hue equal to h_instantaneous and its intensity value
// equal to v_composite.  We take the fused saturation to equal unity.
// The fused XYZRGBA is written out to "fused_instant_composite.xyzrgba".
// ==========================================================================
// Last updated on 4/3/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "math/adv_mathfuncs.h"
#include "general/filefuncs.h"
#include "geometry/mybox.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string subdir="./xyzp_files/";
   string composite_xyzrgba_filename=
      subdir+"composite_intensities.xyzrgba";
//   cout << "Enter name of composite COMPOSITE XYZRBA file:" << endl;
//   cin >> composite_xyzrgba_filename;

// First read in time-averaged composite ISAR image:

   vector<threevector>* xyz_pnt_ptr=new vector<threevector>;
   vector<colorfunc::RGB>* composite_rgb_pnt_ptr=new vector<colorfunc::RGB>;
   xyzpfunc::read_xyzrgba_data(
      composite_xyzrgba_filename,xyz_pnt_ptr,composite_rgb_pnt_ptr);

   int starting_imagenumber=0;
   cout << "Enter starting imagenumber to be fused:" << endl;
   cin >> starting_imagenumber;
   int stopping_imagenumber=1;
   cout << "Enter stopping imagenumber to be fused:" << endl;
   cin >> stopping_imagenumber;
   int imagenumber_skip=1;
   cout << "Enter imagenumber skip:" << endl;
   cin >> imagenumber_skip;
   
   vector<threevector>* xyz_instant_pnt_ptr=new vector<threevector>;
   vector<colorfunc::RGB>* instant_rgb_pnt_ptr=new vector<colorfunc::RGB>;
   for (int imagenumber=starting_imagenumber; imagenumber < 
           stopping_imagenumber; imagenumber += imagenumber_skip)
   {
      string number_str=stringfunc::integer_to_string(imagenumber,2);

      string instantaneous_xyzrgba_filename=subdir+
         "instant"+number_str+"_pure_hues.xyzrgba";
//   cout << "Enter name of composite instantaneous XYZRBA file:" << endl;
//   cin >> instantaneous_xyzrgba_filename;

      xyz_instant_pnt_ptr->clear();
      instant_rgb_pnt_ptr->clear();
      xyzpfunc::read_xyzrgba_data(
         instantaneous_xyzrgba_filename,xyz_instant_pnt_ptr,
         instant_rgb_pnt_ptr);

      string xyzrgba_filename=subdir+"fused_instant_composite_"+number_str+
         ".xyzrgba";
      ofstream outstream;
      filefunc::deletefile(xyzrgba_filename);
      filefunc::openfile(xyzrgba_filename,outstream);

      const double saturation=1;
      for (unsigned int i=0; i<xyz_pnt_ptr->size(); i++)
      {
         threevector curr_XYZ( (*xyz_pnt_ptr)[i] );
         colorfunc::RGB composite_RGB( (*composite_rgb_pnt_ptr)[i] );
         colorfunc::HSV composite_HSV=colorfunc::RGB_to_hsv(
            composite_RGB,false);

         colorfunc::RGB instant_RGB( (*instant_rgb_pnt_ptr)[i] );
         colorfunc::HSV instant_HSV=colorfunc::RGB_to_hsv(instant_RGB,false);

// Don't fuse any instantaneous hues if their corresponding
// intensities lie below some minimal threshold:

         const double min_intensity=0.5;
         colorfunc::RGB curr_RGB=composite_RGB;
         if (instant_HSV.third > min_intensity)
         {
            double fused_r,fused_g,fused_b;
            colorfunc::hsv_to_RGB(
               instant_HSV.first,saturation,composite_HSV.third,
               fused_r,fused_g,fused_b);
            curr_RGB=colorfunc::RGB(fused_r,fused_g,fused_b);
         }

         xyzpfunc::write_single_xyzrgba_point(
            outstream,curr_XYZ,curr_RGB,true);
      } // loop over index i labeling points in cloud

   } // loop over imagenumber index

   delete xyz_pnt_ptr;
   delete composite_rgb_pnt_ptr;
   delete xyz_instant_pnt_ptr;
   delete instant_rgb_pnt_ptr;
}
