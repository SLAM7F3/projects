// ========================================================================
// Program GREY2RGB reads in a greyscale G99 video file.  It writes
// out a new G99 video file to "colored_video.vid" which is colored
// according to a JET-like colormap.
// ========================================================================
// Last updated on 12/6/06; 12/19/06; 2/4/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
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
      ndims,passes_group.get_pass_ptr(videopass_ID),
      AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(passes_group);
   movie_ptr->change_color_map(2);

   const int bytes_per_pixel=3;
   string vid_filename="colored_video.vid";
   VidFile vid_out;
   vid_out.New_8U(vid_filename.c_str(),movie_ptr->getWidth(),
                  movie_ptr->getHeight(), movie_ptr->get_Nimages(), 
                  bytes_per_pixel);

   movie_ptr->get_texture_rectangle_ptr()->set_first_frame_to_display(0);
   for (int i=0; i<movie_ptr->get_Nimages(); i++)
   {
      cout << i << " " << flush;
      movie_ptr->displayFrame(i);
      vid_out.WriteFrame(
         movie_ptr->get_m_colorimage_ptr(), movie_ptr->getWidth()*
         bytes_per_pixel);
   }
   outputfunc::newline();

}
