// ==========================================================================
// Program ISAR_OPT_FUSION reads in 3D ISAR and optical XYZRGBA files.
// RGB values are converted to HSV.  We set the fused image's hue
// equal to h_ISAR and its intensity value equal to v_optical.  We
// take the fused saturation to equal a linear combination of s_ISAR
// and s_optical.  The fused XYZRGBA is written out to
// "fused_image.xyzrgba".
// ==========================================================================
// Last updated on 3/16/06; 12/20/07
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
#include "geometry/projective.h"
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

   string isar_xyzrgba_filename="isar.xyzrgba";
   string optical_xyzrgba_filename="optical.xyzrgba";
//   cout << "Enter name of composite ISAR XYZRBA file:" << endl;
//   cin >> isar_xyzrgba_filename;
//   cout << "Enter name of composite optical XYZRBA file:" << endl;
//   cin >> optical_xyzrgba_filename;

   vector<threevector>* xyz_pnt_ptr=new vector<threevector>;
   vector<colorfunc::RGB>* isar_rgb_pnt_ptr=new vector<colorfunc::RGB>;
   vector<threevector>* xyz_opt_pnt_ptr=new vector<threevector>;
   vector<colorfunc::RGB>* opt_rgb_pnt_ptr=new vector<colorfunc::RGB>;

   xyzpfunc::read_xyzrgba_data(isar_xyzrgba_filename,xyz_pnt_ptr,
                               isar_rgb_pnt_ptr);
   xyzpfunc::read_xyzrgba_data(optical_xyzrgba_filename,xyz_opt_pnt_ptr,
                               opt_rgb_pnt_ptr);

   string xyzrgba_filename="fused_image.xyzrgba";
   ofstream outstream;
   filefunc::deletefile(xyzrgba_filename);
   filefunc::openfile(xyzrgba_filename,outstream);

   const double s_weight=0.1;

   for (unsigned int i=0; i<xyz_pnt_ptr->size(); i++)
   {
      threevector curr_XYZ( (*xyz_pnt_ptr)[i] );
      colorfunc::RGB isar_RGB( (*isar_rgb_pnt_ptr)[i] );
      colorfunc::HSV isar_HSV=colorfunc::RGB_to_hsv(isar_RGB,false);

      colorfunc::RGB opt_RGB( (*opt_rgb_pnt_ptr)[i] );
      colorfunc::HSV opt_HSV=colorfunc::RGB_to_hsv(opt_RGB,false);

      double s_avg=s_weight*opt_HSV.second+(1-s_weight)*isar_HSV.second;

      double fused_r,fused_g,fused_b;
      colorfunc::hsv_to_RGB(isar_HSV.first,s_avg,opt_HSV.third,
                            fused_r,fused_g,fused_b);

      colorfunc::RGB curr_RGB(fused_r,fused_g,fused_b);

//  Note: RGB values range over interval [0,256) rather than from
//  [0,1].  So they are NOT normalized!

      bool normalized_input_RGB_values=false;
      xyzpfunc::write_single_xyzrgba_point(
         outstream,curr_XYZ,curr_RGB,normalized_input_RGB_values);

   } // loop over index i labeling points in cloud

   delete xyz_pnt_ptr;
   delete isar_rgb_pnt_ptr;
   delete xyz_opt_pnt_ptr;
   delete opt_rgb_pnt_ptr;
}
