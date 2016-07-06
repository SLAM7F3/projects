// ========================================================================
// Program HOMOPROJECT reads in a G99 video file which contains some
// planar target.  It applies a homography (which may have been
// precalculated via mains/isds/HOMO) to the input still image and
// generates a new G99 video file containing the transformed output.
// In Jan 2008, we used this program to orthorectify an image
// downloaded from the web of the LACE satellite.

//			homoproject lace.vid

// ========================================================================
// Last updated on 11/7/05; 1/15/08
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "geometry/homography.h"
#include "video/G99VideoDisplay.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ========================================================================

int main(int argc, char* argv[])
{

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   string input_filename=string(argv[1]);
   unsigned int dot_posn=input_filename.rfind(".vid");
   string output_filename=input_filename.substr(0,dot_posn)
      +"_backproject.vid";

   AnimationController* AnimationController_ptr=new AnimationController();
   G99VideoDisplay vid_in(input_filename,AnimationController_ptr);

   int startFrame=0;
   int endFrame=1;
   int skip=1;
   int n_images=1;

   int n_channels=vid_in.getNchannels();
   VidFile vid_out;
   vid_out.New_8U(output_filename.c_str(),vid_in.getWidth(),
                  vid_in.getHeight(), n_images, n_channels);

// We assume input and output video streams are 24 bit RGB color:

   int n_bytes=vid_in.getWidth()*vid_in.getHeight()*n_channels;
   unsigned char* pbyImgOut = new unsigned char[n_bytes];

   cout << "width = " << vid_in.getWidth()
        << " height = " << vid_in.getHeight() << endl;

// Instantiate and fill homography projection matrix using values
// calculated by programs mains/isds/consolidate and mains/isds/homo
// on Jan 15, 2008 for LACE satellite:

   genmatrix* H_ptr=new genmatrix(3,3);

// H: world XY -> image plane UV

   H_ptr->put(0 , 0 , -0.0222174767622 );
   H_ptr->put(0 , 1 , -0.146734161896 );
   H_ptr->put(0 , 2 , -0.321540784219 );
   H_ptr->put(1 , 0 , -0.189478203044 );
   H_ptr->put(1 , 1 , 0.020793048288 );
   H_ptr->put(1 , 2 , -0.335296786292 );
   H_ptr->put(2 , 0 , -0.026564761902 );
   H_ptr->put(2 , 1 , 0.0421012893426 );
   H_ptr->put(2 , 2 , -0.850500154744 );

/*

// H: UV -> XY

   H_ptr->put(0 , 0 , -0.0160658380214 );
   H_ptr->put(0 , 1 , -0.623065834186 );
   H_ptr->put(0 , 2 , 0.25170488455 );
   H_ptr->put(1 , 0 , -0.685705524942 );
   H_ptr->put(1 , 1 , 0.0466409651371 );
   H_ptr->put(1 , 2 , 0.240852113486 );
   H_ptr->put(2 , 0 , -0.0334400373468 );
   H_ptr->put(2 , 1 , 0.021765455375 );
   H_ptr->put(2 , 2 , -0.127304155202 );
*/


   homography H(H_ptr);

   const double xmin=-4;
   const double xmax=4;
   const double ymin=-4;
   const double ymax=4;
   for (int n = startFrame; n < endFrame; n += skip)
   {
      cout << n << " " << flush;
      vid_in.planar_orthorectify(
         H,xmin,xmax,ymin,ymax,pbyImgOut);
      vid_out.WriteFrame(pbyImgOut, n_channels*vid_in.getWidth());
   }
   cout << endl;

   delete pbyImgOut;
   delete H_ptr;
}
