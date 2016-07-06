// ========================================================================
// Program IMAGE2IMAGE uses OSG plugins to parse an input image.  Its
// color content can then be altered.  For example, we used this
// program in September 2006 to brighten the ocean regions within the
// two large land_shallow_topo_east.tif and land_shallow_topo_west.tif
// files which texture the OSG bluemarble.  Finally, OSG plugins are
// used to write the altered image to binary file output.
// ========================================================================
// Last updated on 9/2/06; 9/3/06; 9/14/06
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osg/Image>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ========================================================================

int main(int argc, char* argv[])
{

// Parse image file:

   string image_filename="earth.tif";
   cout << "Enter image filename:" << endl;
   cin >> image_filename;

   string banner="Reading input image:";
   outputfunc::write_banner(banner);

   osg::Image* image_ptr = osgDB::readImageFile(image_filename);

// We need to flip the image vertically before reading out its bytes
// in the unsigned char* array:

//   image_ptr->flipVertical();

   banner="Altering image coloring:";
   outputfunc::write_banner(banner);
   
   vector<unsigned char*> charstar_vector;
   charstar_vector.push_back(image_ptr->data());

   int width=image_ptr->s();
   int height=image_ptr->t();
//   int nbytes_per_pixel=3;
   int nbytes_per_pixel=4;
   unsigned char* data=new unsigned char[width*height*nbytes_per_pixel];
   memcpy(data, image_ptr->data(), nbytes_per_pixel*width*height);

   int n_pixels_changed=0;
   for (int i=0; i<width*height; i++)
   {
//      if (i%1000==0) cout << i/1000 << " " << flush;
      int R=stringfunc::unsigned_char_to_ascii_integer(
         data[nbytes_per_pixel*i+0]);
      int G=stringfunc::unsigned_char_to_ascii_integer(
         data[nbytes_per_pixel*i+1]);
      int B=stringfunc::unsigned_char_to_ascii_integer(
         data[nbytes_per_pixel*i+2]);
      if (nbytes_per_pixel==3)
      {
         cout << "i = " << i << " w*h = " << width*height
              << " R = " << R << " G = " << G << " B = " << B << endl;

// Original blue ocean coloring for land_shallow_topo.tif files was
// R=G=10, B=51.  This yields an earth.osga whose oceans are much too
// dark relative to background black space.  So we need to lighten the
// ocean coloring:

// hue = 240, saturation = 0.8039, value = 0.2  (R=G=10, B=51)
// hue = 240, saturation = 0.8039, value = 0.5  (R=G=24, B=127)
// hue = 240, saturation = 0.8039, value = 0.6  (R=G=29, B=152)

         const unsigned char R_new=
            stringfunc::ascii_integer_to_unsigned_char(29);
         const unsigned char G_new=
            stringfunc::ascii_integer_to_unsigned_char(29);
         const unsigned char B_new=
            stringfunc::ascii_integer_to_unsigned_char(152);
         if (R==10 && G==10 && B==51)
         {
            n_pixels_changed++;
            data[3*i+0]=R_new;
            data[3*i+1]=G_new;
            data[3*i+2]=B_new;
         }
      }
      else if (nbytes_per_pixel==4)
      {
         int A=stringfunc::unsigned_char_to_ascii_integer(
            data[nbytes_per_pixel*i+3]);
         cout << "i = " << i << " w*h = " << width*height
              << " R = " << R << " G = " << G 
              << " B = " << B << " A = " << A << endl;
         if (A==0)
         {
            int R_new=240;
            int G_new=240;
            int B_new=240;
            int A_new=255;
            data[4*i+0]=R_new;
            data[4*i+1]=G_new;
            data[4*i+2]=B_new;
            data[4*i+3]=A_new;
         }
      }


      
   } // loop over index i labeling pixels

   cout << "n_pixels_changed = " << n_pixels_changed << endl;

// Write out modified charstar data to new image file:

   banner="Writing output image:";
   outputfunc::write_banner(banner);

   osg::Image* new_image_ptr=new osg::Image();

   int GLimageDepth=1;
   GLint internalTextureFormat=GL_RGB;
   GLenum pixelFormat=GL_RGB;
   if (nbytes_per_pixel==4)
   {
      internalTextureFormat=GL_RGBA;
      pixelFormat=GL_RGBA;
   }
   GLenum GLtype=GL_UNSIGNED_BYTE;
   osg::Image::AllocationMode allocation_mode=osg::Image::NO_DELETE;
   int GLpacking=1;

   new_image_ptr->setImage( image_ptr->s(),image_ptr->t(),GLimageDepth,
                            internalTextureFormat,
                            pixelFormat,GLtype,
                            data, allocation_mode, GLpacking );

   string prefix=stringfunc::prefix(image_filename);
//   string suffix=".tif";
   string suffix=".png";
   string new_image_filename=prefix+"_new"+suffix;
   osgDB::writeImageFile(*new_image_ptr,new_image_filename);

}
