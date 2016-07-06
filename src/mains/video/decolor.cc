// ========================================================================
// Program DECOLOR converts 3-channel RGB .vid files to single channel
// grey .vid files.  The output from this program is NOT the same as
// that from RAW2VID, for the detector's non-uniform frequency
// response should be removed by the intervening DEMOSAIC program.  So
// the grey scale images output by this program should not exhibit the
// distinct dot pattern which is seen in the grey scale images output
// by RAW2VID.
// ========================================================================
// Last updated on 9/27/05
// ========================================================================

#include <iostream>
#include <string>
#include "video/VidFile.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;

// ========================================================================

int main(int argc, char* argv[])
{
   string input_filename=string(argv[1]);
   unsigned int dot_posn=input_filename.rfind("_RGB.vid");
   string output_filename=input_filename.substr(0,dot_posn)
      +"corrected_grey.vid";

   VidFile vid_in(input_filename);

   int startFrame=0;
   int endFrame=vid_in.getNumFrames()-1;
	
   VidFile vid_out;
   vid_out.New_8U(output_filename.c_str(),vid_in.getWidth(),
                  vid_in.getHeight(), (endFrame + 1 - startFrame), 1);

// We assume input represents 24-bit RGB:

   unsigned char* pbyImgIn = 
      new unsigned char[vid_in.getWidth()*vid_in.getHeight()*3];
   unsigned char* pbyImgOut = 
      new unsigned char[vid_in.getWidth()*vid_in.getHeight()];

   vid_in.read_image(startFrame-1,pbyImgIn);
   
   for (int n = startFrame; n <= endFrame; n++)
   {
      cout << n << " " << flush;
      vid_in.read_next_image(pbyImgIn);
      videofunc::RGB_to_grey(
         pbyImgIn, pbyImgOut, vid_in.getWidth(),vid_in.getHeight());
      vid_out.WriteFrame(pbyImgOut, vid_in.getWidth());
   }
   cout << endl;

   delete pbyImgIn;
   delete pbyImgOut;
}
