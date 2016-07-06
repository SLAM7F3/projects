// ========================================================================
// Program VIDS2VID concatenates multiple .vid files into a single
// .vid file which can be viewed and manipulated using program
// mains/video/VIDEO.

//				vids2vid

// User is queried to enter name of image directory containing
// individual video files, base name for each individual video file,
// starting and stopping numbers for individual video files and name
// of output, concatenated video file.

// ========================================================================
// Last updated on 1/2/08; 1/24/08
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
   string imagedir="/data3/video/Lubbock/constant_hawk";
   cout << "Enter image directory:" << endl;
   cin >> imagedir;
   imagedir += "/";

   string basefilename="ch_";
   cout << "Enter base image filename:" << endl;
   cin >> basefilename;

   int start_image=1;
   int stop_image=10;
   cout << "Enter starting image number:" << endl;
   cin >> start_image;
   cout << "Enter stopping image number:" << endl;
   cin >> stop_image;

   int image_skip=1;
//   cout << "Enter image skip:" << endl;
//   cin >> image_skip;

   int n_images=(stop_image-start_image)/image_skip+1;
   string image_filename[n_images];
   cout << "n_images = " << n_images << endl;

//   int n_digits=2;
//   int n_digits=3;
//   int n_digits=4;
   int n_digits=6;
//   cout << "Enter number of maximum number of digits to zero pad:" << endl;
//   cin >> n_digits;

   string image_suffix=".vid";

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
   cout << "Enter output concatenated  video filename = " 
        << vid_filename << endl;
   cin >> vid_filename;
   vid_filename += ".vid";

// Loop over all input images and store their byte data within STL
// vector charstar_vector:

   int width,height,n_channels;
   vector<unsigned char*> charstar_vector;
   for (int n=0; n<n_images;n++)
   {
      cout << "Parsing image file # " << n << endl;

      VidFile* m_g99Video_ptr=new VidFile(image_filename[n]);

      width=m_g99Video_ptr->getWidth();
      height=m_g99Video_ptr->getHeight();
      n_channels=m_g99Video_ptr->getNumChannels();
      
      int image_size_in_bytes=width*height*n_channels;
      unsigned char* m_image=new unsigned char[image_size_in_bytes];
      m_g99Video_ptr->read_image(0,m_image);
      charstar_vector.push_back(m_image);
      
      delete m_g99Video_ptr;

   } // loop over index n labeling input .vid files
   
// Instantiate animation controller:

   AnimationController* AnimationController_ptr=new AnimationController(
      n_images);
   
// Instantiate a VidFile and copy byte data within charstar_vector to
// it:

   G99VideoDisplay* VD_ptr=new G99VideoDisplay(
      width,height,n_images,n_channels,AnimationController_ptr);
   VD_ptr->get_texture_rectangle_ptr()->write_dotVidfile(
      vid_filename,charstar_vector);

   delete VD_ptr;
}
