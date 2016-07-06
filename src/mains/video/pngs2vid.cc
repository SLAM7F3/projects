// ========================================================================
// Program PNGS2VID converts an input sequence of PNG images into a
// Group 99 video which can be viewed and manipulated
// using programs mains/video/VIDEO and mains/osg/VIDEO3D.
// ========================================================================
// Last updated on 2/10/06
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "image/pngfuncs.h"
#include "general/stringfuncs.h"
#include "video/VidFile.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ========================================================================

int main(int argc, char* argv[])
{
   string imagedir;
   cout << "Enter image directory:" << endl;
   cin >> imagedir;
   imagedir += "/";

   string basefilename;
   cout << "Enter base PNG filename:" << endl;
   cin >> basefilename;

   int start_image,stop_image;
   cout << "Enter starting image number:" << endl;
   cin >> start_image;
   cout << "Enter stopping image number:" << endl;
   cin >> stop_image;

   int image_skip=1;
   cout << "Enter image skip:" << endl;
   cin >> image_skip;

   int n_images=(stop_image-start_image)/image_skip+1;
   string png_filename[n_images];
   cout << "n_images = " << n_images << endl;

//   const int n_digits=1;
//   const int n_digits=3;
//   const int n_digits=4;

   int n_digits=5;
   cout << "Enter number of maximum number of digits to zero pad:" << endl;
   cin >> n_digits;

   int counter=0;
   for (int i=start_image; i<=stop_image; i += image_skip)
   {
      string filename=basefilename
         +stringfunc::integer_to_string(i,n_digits)+".png";
      png_filename[counter]=imagedir+filename;
      cout << "counter = " << counter << " png_filename = " 
           << png_filename[counter] << endl;
      counter++;
   }
   
   unsigned int dot_posn=png_filename[0].rfind(".png");
   string vid_filename=png_filename[0].substr(0,dot_posn)+".vid";
   cout << "vid_filename = " << vid_filename << endl;

// Loop over all input PNG images and store their byte data within STL
// vector charstar_vector:

   vector<unsigned char*> charstar_vector;

   for (int n=0; n<n_images; n++)
   {
      cout << endl;
      cout << "Parsing PNG file # " << n << endl;
      bool png_opened_successfully=pngfunc::open_png_file(png_filename[n]);
      if (png_opened_successfully)
      {
         pngfunc::parse_png_file();
         charstar_vector.push_back(pngfunc::generate_charstar_array());
         pngfunc::close_png_file();
      }
   } // loop over index n labeling PNG images

// Instantiate a VidFile and copy byte data within charstar_vector to
// it:

   VidFile vid_out;
   vid_out.New_8U(vid_filename.c_str(),pngfunc::width, pngfunc::height, 
                  n_images, 3);
   for (int i=0; i<charstar_vector.size(); i++)
   {
      vid_out.WriteFrame(charstar_vector[i], pngfunc::width*3);
      delete [] charstar_vector[i];
   }

}
