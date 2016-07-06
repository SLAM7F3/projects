// ========================================================================
// Program DOUBLES2VID converts an input binary file containing double
// intensity values to a Group 99 video which can be viewed and
// manipulated using programs mains/video/VIDEO.  We wrote this
// specialized utility to pair up with the CDFDUMP code which Hyrum
// Anderson wrote to circumvent XELIAS.  His dump routine parses
// imagecdf files and writes out their contents as binary files
// containing strings of doubles.  This program lets us read his
// output and convert satellite ISAR imagery into G99 video file
// format for subsequent processing.
// ========================================================================
// Last updated on 3/28/06; 12/4/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "video/VidFile.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

// ========================================================================

int main(int argc, char* argv[])
{

// Parse RGB file:

//   string doubles_filename="spase.doubles";
   string doubles_filename="htot.doubles";
//   cout << "Enter doubles intensity filename:" << endl;
//   cin >> doubles_filename;

   int width=1067;
   int height=800;
//   cout << "Enter image width in pixels:" << endl;
//   cin >> width;
//   cout << "Enter image height in pixels:" << endl;
//   cin >> height;

   vector<double>* pdata_ptr=new vector<double>;
   xyzpfunc::read_p_data(doubles_filename,pdata_ptr);

   int n_pixels=pdata_ptr->size();
   cout << "n_pixels = " << n_pixels << endl;
   int n_images=n_pixels/(width*height);
   cout << "n_images = " << n_pixels/double(width*height) << endl;

   const double p_max=1;
   const double p_min=0;

// Next two lines should be used for XELIAS ISAR imagery:

//   const double p_max=0;	// dB
//   const double p_min=-60;	// dB

// In order to cut down on excessively large cross range windows for
// ISAR images, we allow the user to specify a smaller width fraction:

   double min_width_frac=0;
   double max_width_frac=1.0;
   cout << "Enter output video imagery minimum width fraction:" << endl;
   cin >> min_width_frac;
   cout << "Enter output video imagery maximum width fraction:" << endl;
   cin >> max_width_frac;

   int px_min=basic_math::round(min_width_frac*width);
   int px_max=basic_math::round(max_width_frac*width);
   int reduced_width=px_max-px_min;
   cout << "px_min = " << px_min << " px_max = " << px_max 
        << " reduced width = " << reduced_width
        << " width = " << width << endl;

   char* charstar_array_ptr=new char[n_images*reduced_width*height];
   
   int n=0;
   int pixel_counter=0;
   for (int imagenumber=0; imagenumber<n_images; imagenumber++)
   {
      for (int py=0; py<height; py++)
      {
         for (int px=0; px<width; px++)
         {
            if (px >= px_min && px < px_max)
            {
               double prob=(*pdata_ptr)[pixel_counter];
               double frac=(prob-p_min)/(p_max-p_min);
               frac=basic_math::max(0.0,frac);
               frac=basic_math::min(1.0,frac);
               int intensity=255*frac;
               *(charstar_array_ptr+n)=static_cast<char>( intensity );
               n++;
            }
            pixel_counter++;
         } // loop over px index 
      } // loop over py index
   } // loop over imagenumber index

   unsigned int dot_posn=doubles_filename.rfind(".doubles");
   string vid_filename=doubles_filename.substr(0,dot_posn)+".vid";

   VidFile vid_out;
   vid_out.New_8U(vid_filename.c_str(), reduced_width, height, n_images, 1);
   for (int i=0; i<n_images; i++)
   {
      vid_out.WriteFrame(charstar_array_ptr+i*reduced_width*height, 
                         reduced_width*1);
   }
   
   delete [] charstar_array_ptr;
}
