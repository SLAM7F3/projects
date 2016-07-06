// ========================================================================
// Mike Braun's demosaicing program which converts single channel
// greyscale .vid files to 3-channel RGB .vid files.  Input files are
// assumed to be named "foo_greyscale.vid".  Output files are named
// "foo.vid".
// ========================================================================
// Last updated on 10/6/05; 10/22/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "math/adv_mathfuncs.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "math/prob_distribution.h"
#include "video/G99VideoDisplay.h"
#include "video/VidFile.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ========================================================================

int main(int argc, char* argv[])
{
   string input_filename=string(argv[1]);
   unsigned int dot_posn=input_filename.rfind("_greyscale.vid");
   string output_filename=input_filename.substr(0,dot_posn)+"_RGB.vid";

   AnimationController* AnimationController_ptr=new AnimationController();
   G99VideoDisplay vid_in(input_filename,AnimationController_ptr);
   G99VideoDisplay vid_out(output_filename,AnimationController_ptr);

//   VidFile vid_in(input_filename);

   int startFrame,endFrame,skip;
   cout << "Total number frames within input greyscale file = " 
        << vid_in.get_Nimages() << endl;
   cout << "Enter starting frame number for colored video sequence:" << endl;
   cin >> startFrame;
   cout << "Enter ending frame number for colored video sequence:" << endl;
   cin >> endFrame;
   cout << "Enter frame skip value:" << endl;
   cin >> skip;
   int n_images=(endFrame+1-startFrame)/skip;
   if ((endFrame+1-startFrame)%skip > 0) n_images++;
   cout << "Number of RGB images = " << n_images << endl;

//   VidFile vid_out;
   vid_out.get_VidFile_ptr()->New_8U(
      output_filename.c_str(),vid_in.getWidth(),
      vid_in.getHeight(), n_images, 3);

// We assume input represents 8-bit greyscale:

//   vid_in.read_and_set_image();

//   unsigned char* pbyImgIn = 
//      new unsigned char[vid_in.getWidth()*vid_in.getHeight()];
   unsigned char* pbyImgOut = 
      new unsigned char[vid_in.getWidth()*vid_in.getHeight()*3];

//   vid_in.read_image(max(0,startFrame-1),pbyImgIn);
   

// Generate final, desired gaussian distribution for colored pixels'
// intensities.  We convert from RGB to HSV and then map the V
// distribution onto this gaussian:

   const int nbins=75;
   double mu=0.575;
   double sigma=0.2;
   prob_distribution p_gaussian=advmath::generate_gaussian_density(
      nbins,mu,sigma);
   mathfunc::errorfunc::initialize_fast_error_function();

   const int n_pixels=vid_in.getHeight()*vid_in.getWidth();
   vector<double> h,s,v,non_negligible_intensities;
   h.reserve(n_pixels);
   s.reserve(n_pixels);
   v.reserve(n_pixels);
   non_negligible_intensities.reserve(n_pixels);

   for (int n = startFrame; n <= endFrame; n += skip)
   {
      cout << n << " " << flush;
//      vid_in.read_image(n,pbyImgIn);
      vid_in.displayFrame(n);
      videofunc::demosaic(
         vid_in.get_m_image_ptr(),
         pbyImgOut, vid_in.getWidth(),vid_in.getHeight());

      h.clear();
      s.clear();
      v.clear();
      non_negligible_intensities.clear();
      vid_out.convert_to_RGB(pbyImgOut,2,p_gaussian,h,s,v,
                             non_negligible_intensities);
      vid_out.get_VidFile_ptr()->WriteFrame(pbyImgOut, vid_in.getWidth()*3);
   }
   cout << endl;

   delete pbyImgOut;
}
