// ========================================================================
// Program RGBS2VID converts an input sequence of RGB images into a
// Group 99 video which can be viewed and manipulated
// using programs mains/video/VIDEO and mains/osg/VIDEO3D.
// ========================================================================
// Last updated on 3/1/06; 4/4/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "video/VidFile.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
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
   cout << "Enter base RGB filename:" << endl;
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
   string rgb_filename[n_images];
   cout << "n_images = " << n_images << endl;

   const int n_digits=4;
   int counter=0;
   for (int i=start_image; i<=stop_image; i += image_skip)
   {
      string filename=basefilename
         +stringfunc::integer_to_string(i,n_digits)+".rgb";
      rgb_filename[counter]=imagedir+filename;
      cout << "counter = " << counter << " rgb_filename = " 
           << rgb_filename[counter] << endl;
      counter++;
   }
   
//   int width=1196;
//   int height=1800;
   int width,height;
 cout << "Enter image width in pixels:" << endl;
 cin >> width;
 cout << "Enter image height in pixels:" << endl;
 cin >> height;

   unsigned int dot_posn=rgb_filename[0].rfind(".rgb");
   string vid_filename=rgb_filename[0].substr(0,dot_posn)+".vid";
   cout << "vid_filename = " << vid_filename << endl;

// Loop over all input RGB images and store their byte data within STL
// vector charstar_vector:

   vector<char*> charstar_vector;

   for (int n=0; n<n_images; n++)
   {
      cout << endl;
      cout << "Parsing RGB file # " << n << endl;

      long long n_bytes;
      ifstream instream;
      if (filefunc::open_binaryfile(rgb_filename[n],instream,n_bytes))
      {
         char* charstar_array_ptr=new char[n_bytes];
         for (int b=0; b<n_bytes; b++)
         {
            filefunc::readobject(instream,charstar_array_ptr[b]);
         } // loop over b index labeling bytes in current RGB file
         charstar_vector.push_back(charstar_array_ptr);
         filefunc::closefile(rgb_filename[n],instream);
      }
      
   } // loop over index n labeling RGB images

// Instantiate a VidFile and copy byte data within charstar_vector to
// it:

   VidFile vid_out;
   vid_out.New_8U(vid_filename.c_str(),width,height,n_images,3);
   for (unsigned int i=0; i<charstar_vector.size(); i++)
   {
      vid_out.WriteFrame(charstar_vector[i], width*3);
      delete [] charstar_vector[i];
   }

}
