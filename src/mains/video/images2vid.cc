// ========================================================================
// Program IMAGES2VID uses OSG plugins to parse and convert input
// images to a multi-frame Group 99 video which can be viewed and
// manipulated using programs mains/video/VIDEO and mains/osg/VIDEO3D.
// This program generalizes our earlier PNGS2VID program.
// ========================================================================
// Last updated on 12/6/06; 11/23/07
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
//   string imagedir="/home/cho/movies/clip20_stills";
//   string imagedir="/data3/video/NewYork/empire_cars_stills";
//   string imagedir="/data3/video/Lubbock/constant_hawk";
   string imagedir="/media/usbdisk/Lobby7/Lobby7_640x360";
//   cout << "Enter image directory:" << endl;
//   cin >> imagedir;
   imagedir += "/";

//   string basefilename="stabilized_clip1_";
//   string basefilename="ch_";
   string basefilename="img";
//   cout << "Enter base image filename:" << endl;
//   cin >> basefilename;

   int start_image=10001;
//   int stop_image=10100;
   int stop_image=11058;
//   cout << "Enter starting image number:" << endl;
//   cin >> start_image;
//   cout << "Enter stopping image number:" << endl;
//   cin >> stop_image;

   int image_skip=1;
//   cout << "Enter image skip:" << endl;
//   cin >> image_skip;

   int n_images=(stop_image-start_image)/image_skip+1;
   string image_filename[n_images];
   cout << "n_images = " << n_images << endl;

//   int n_digits=3;
//   int n_digits=4;
   int n_digits=5;
//   int n_digits=6;
//   cout << "Enter number of maximum number of digits to zero pad:" << endl;
//   cin >> n_digits;

   string image_suffix=".png";
//   string image_suffix=".tif";

   int counter=0;
   for (int i=start_image; i<=stop_image; i += image_skip)
   {
      string filename=basefilename
         +stringfunc::integer_to_string(i,n_digits)+image_suffix;
      image_filename[counter]=imagedir+filename;
      cout << "counter = " << counter << " filename = " 
           << image_filename[counter] << endl;
      counter++;
   }
   
   string vid_filename=basefilename+".vid";
   cout << "Output video filename = " << vid_filename << endl;

// Loop over all input images and store their byte data within STL
// vector charstar_vector:

   osg::Image* image_ptr;
   vector<unsigned char*> charstar_vector;
   for (int n=0; n<n_images; n++)
   {
      cout << endl;
      cout << "Parsing image file # " << n << endl;

      image_ptr = osgDB::readImageFile(image_filename[n]);

// We need to flip the image vertically before reading out its bytes
// in the unsigned char* array:

      image_ptr->flipVertical();
      charstar_vector.push_back(image_ptr->data());
   } // loop over index n labeling PNG images

// Instantiate animation controller:

   AnimationController* AnimationController_ptr=new AnimationController();

// Instantiate a VidFile and copy byte data within charstar_vector to
// it:

//   int n_channels=1;
   int n_channels=3;
   G99VideoDisplay* VD_ptr=new G99VideoDisplay(
      image_ptr->s(),image_ptr->t(),n_images,n_channels,
      AnimationController_ptr);
   VD_ptr->get_texture_rectangle_ptr()->write_dotVidfile(
      vid_filename,charstar_vector);

   delete VD_ptr;
}
