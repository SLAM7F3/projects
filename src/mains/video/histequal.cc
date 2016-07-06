// ========================================================================
// Program HISTEQUAL takes in a greyscale video file as a command line
// argument.  It ignores all pixels whose intensities lie below some
// minimal threshold.  The remaining pixels' intensities are subjected
// to histogram equalization in order to amplify contrast.  The
// modified imagery is written to an output .vid file.
// ========================================================================
// Last updated on 12/7/06; 12/19/06; 12/20/06; 2/4/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "general/filefuncs.h"
#include "osg/osg2D/MoviesGroup.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "video/VidFile.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ========================================================================
int main(int argc, char* argv[])
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();

// Instantiate animation controller & key handler:

   AnimationController* AnimationController_ptr=new AnimationController();

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);
   string banner=
      "Enter minimal intensity for pixels to be included in histogram equalization:";
   outputfunc::write_banner(banner);

   vector<double> intensity_thresholds;
   intensity_thresholds.push_back(50);
   intensity_thresholds.push_back(30);
   intensity_thresholds.push_back(25);
   intensity_thresholds.push_back(50);
   intensity_thresholds.push_back(50);
   intensity_thresholds.push_back(45);
   intensity_thresholds.push_back(35);

   string input_video_filename=passes_group.get_pass_ptr(
      videopass_ID)->get_first_filename();
   movie_ptr->equalize_greyscale_intensity_histograms(
      input_video_filename,intensity_thresholds);
}
