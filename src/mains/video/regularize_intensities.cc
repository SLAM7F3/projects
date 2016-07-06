// ========================================================================
// Program REGULARIZE_INTENSITIES reads in a Group 99 greyscale [RGB]
// video which we assume has previously been generated via program
// DECOLOR [DEMOSAIC].  In Sept 2005, we empirically observed that the
// overall intensity distribution in consecutive images sometimes
// changes signficantly.  This looks bad to the eye, and we have seen
// it foul up the KLT feature tracking algorithm.

// So this program first computes the intensity distribution for each
// image in the input video.  Intensity values below some minimal
// threshold cutoff are ignored.  Intensities above the threshold are
// remapped onto a constant gaussian with mean 50% and some fixed
// standard deviation.  

// The renormalized sequence is written to a Group 99 .vid file with a
// "_renorm" in its output filename.
// ========================================================================
// Last updated on 10/19/05; 12/18/05; 11/6/06; 2/16/07
// ========================================================================

#include <set>
#include "osg/osgGraphicals/AnimationController.h"
#include "video/G99VideoDisplay.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::string;

// ========================================================================
int main( int argc, char** argv )
{
   string input_video_filename(argv[1]);

// Instantiate animation controller:

   AnimationController* AnimationController_ptr=new AnimationController();

// Instantiate a video display and add a movie controller to it:

   G99VideoDisplay* vidDisplay_ptr = 
      new G99VideoDisplay(input_video_filename,AnimationController_ptr);

// Globally renormalize every image's intensity distribution so that
// they match a gaussian with a fixed mean and standard deviation:

   double intensity_threshold=2;
   cout << "Enter intensity_threshold:" << endl;
   cin >> intensity_threshold;

//   pair<double,double> p=
//      vidDisplay_ptr->compute_median_greyscale_image_intensity(
//         intensity_threshold);
//   vidDisplay_ptr->regularize_greyscale_image_intensities(
//      input_video_filename,intensity_threshold,p);

//   vidDisplay_ptr->equalize_greyscale_intensity_histograms(
//      input_video_filename,intensity_threshold);
   vidDisplay_ptr->equalize_RGB_intensity_histograms(
      input_video_filename,intensity_threshold);
}

