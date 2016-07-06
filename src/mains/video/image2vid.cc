// ========================================================================
// Program IMAGE2VID uses OSG plugins to parse and convert input
// images to a single-frame Group 99 video which can be viewed and
// manipulated using programs mains/video/VIDEO and mains/osg/VIDEO3D.
// This program generalizes our earlier PNG2VID program.
// ========================================================================
// Last updated on 11/6/06; 1/30/07; 8/12/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osg/Image>
#include <osgDB/ReadFile>
#include "osg/osgGraphicals/AnimationController.h"
#include "video/G99VideoDisplay.h"
#include "video/VidFile.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ========================================================================

int main(int argc, char* argv[])
{

// Parse image file:

   string image_filename;
   cout << "Enter image filename:" << endl;
   cin >> image_filename;
   
   osg::Image* image_ptr = osgDB::readImageFile(image_filename);

// We need to flip the image vertically before reading out its bytes
// in the unsigned char* array:

   cout << "Flipping pixels vertically" << endl;
   image_ptr->flipVertical();

   vector<unsigned char*> charstar_vector;
   charstar_vector.push_back(image_ptr->data());

   unsigned int dot_posn=image_filename.rfind(".");
   string vid_filename=image_filename.substr(0,dot_posn)+".vid";
   cout << "Output video filename = " << vid_filename << endl;

// Instantiate animation controller:

   AnimationController* AnimationController_ptr=new AnimationController();

   int n_images=1;
//   int n_channels=1;
   int n_channels=3;

   string input_string;
   cout << "Input image colored (y/n)?" << endl;
   cin >> input_string;
   if (input_string != "y")
   {
      n_channels=1;
   }

   cout << "Instantiating new G99VideoDisplay object" << endl;
   G99VideoDisplay* VD_ptr=new G99VideoDisplay(
      image_ptr->s(),image_ptr->t(),n_images,n_channels,
      AnimationController_ptr);

   cout << "Writing .vid file" << endl;
   VD_ptr->get_texture_rectangle_ptr()->
      write_dotVidfile(vid_filename,charstar_vector);

   delete VD_ptr;
}
