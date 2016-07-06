// ========================================================================
// Program RGB2VID converts an input RGB image to a single-frame Group
// 99 video which can be viewed and manipulated using programs
// mains/video/VIDEO and mains/osg/VIDEO3D.
// ========================================================================
// Last updated on 3/1/06; 7/26/06; 4/4/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "video/VidFile.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;

// ========================================================================

int main(int argc, char* argv[])
{

// Parse RGB file:

   string rgb_filename="415179-03D.rgb";
   cout << "Enter rgb filename:" << endl;
   cin >> rgb_filename;

//   int width=1196;
//   int height=1800;
   int width,height,header_bytes_to_skip=0;
   cout << "Enter image width in pixels:" << endl;
   cin >> width;
   cout << "Enter image height in pixels:" << endl;
   cin >> height;
   cout << "Enter number of header bytes at beginning to skip:" << endl;
   cin >> header_bytes_to_skip;
   
   long long n_bytes;
   ifstream instream;
   if (filefunc::open_binaryfile(rgb_filename,instream,n_bytes))
   {
      char* charstar_array_ptr=new char[n_bytes];
      for (int n=0; n<n_bytes; n++)
      {
         filefunc::readobject(instream,charstar_array_ptr[n]);
      }
      filefunc::closefile(rgb_filename,instream);

      char* headerless_charstar_array_ptr=
         new char[n_bytes-header_bytes_to_skip];
      int counter=0;
      for (int n=header_bytes_to_skip; n<n_bytes; n++)
      {
         counter++;
         headerless_charstar_array_ptr[n-header_bytes_to_skip]=
            charstar_array_ptr[n];
      }
      cout << "Headerless array contains " << counter << " bytes " << endl;

      unsigned int dot_posn=rgb_filename.rfind(".rgb");
      string vid_filename=rgb_filename.substr(0,dot_posn)+".vid";
      int n_images=1;

      VidFile vid_out;
      vid_out.New_8U(vid_filename.c_str(), width, height, n_images, 3);

      vid_out.WriteFrame(headerless_charstar_array_ptr, width*3);
//      vid_out.WriteFrame(charstar_array_ptr, width*3, header_bytes_to_skip);
      delete [] charstar_array_ptr;

      filefunc::closefile(rgb_filename,instream);
   } // input rgb file successfully opened conditional
   else
   {
      cout << "Could not open RGB file " << rgb_filename << endl;
   }

}
