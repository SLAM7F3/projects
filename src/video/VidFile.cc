// Note: The starting position for unsigned char* data_ptr pixels is
// in the upper left corner corresponding to px=py=0.  So the correct
// counter for *data_ptr is p=py*getWidth()+px and NOT
// p=(getHeight()-1-py)*getWidth()+px.  

// ========================================================================
// Group 99 RGB video file parser class
// ========================================================================
// Last updated on 9/18/07; 10/22/07; 7/25/10
// ========================================================================

#include <iostream>
#include <vector>
#include "math/adv_mathfuncs.h"
#include "color/colorfuncs.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "video/VidFile.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ----------------------------------------------------------------
void VidFile::allocate_member_objects()
{
}		       

void VidFile::initialize_member_objects()
{
   intensity_twoDarray_ptr=NULL;
}

// ----------------------------------------------------------------
VidFile::VidFile()
{
   allocate_member_objects();
   initialize_member_objects();
   fp=NULL;
}

VidFile::VidFile(string filename)
{
   allocate_member_objects();
   initialize_member_objects();
   Open(filename.c_str());
   byte_counter=sizeof(header);
}

VidFile::~VidFile()
{
   fclose(fp);
}

// ---------------------------------------------------------------------
// Member function Open

int VidFile::Open(const char* name)
{
   if ( NULL == (fp = fopen(name, "rb")) ) return 0;
   if(fread(&header, sizeof(header), 1, fp) == 0)	//Read short header
   {
      cout << "No elements read in VidFile::Open()" << endl;
   }
   
   return 1;
}

// ---------------------------------------------------------------------
// Member function New_8U

int VidFile::New_8U(string filename, int w, int h, int N, int C)
{
   filefunc::deletefile(filename);
   if ( (fp = fopen(filename.c_str(), "wb")) == NULL) return 0;

   header.endian = 1;
   header.width = w;
   header.height = h;
   header.numframes = N;
   header.num_channels = C;

   header.data_type = VID_8U;
   header.bytes_per_pixel = 1;
   header.data_precision = 8;
   header.bytes_in_header = sizeof(header);
   header.imageFooterBytes = 0;
   header.imageHeaderBytes = 0;

   cout << "sizeof(header) = " << sizeof(header) << endl;

   fwrite(&header, sizeof(header), 1, fp);
   return 1;
}

// ---------------------------------------------------------------------
// Member function WriteFrame

void VidFile::WriteFrame(void* data,int stepbytes, int bytes_to_skip)
{
   char* char_data=static_cast<char*>(data);

//   int nbytes=header.width*header.height*header.num_channels;
//   char data_flipped[nbytes];
//   flip_data(static_cast<char*>(data),data_flipped);

   for (int i = 0; i < header.height; i++)
   {
//      fwrite(data_flipped+i*stepbytes,header.bytes_per_pixel,
//             header.width*header.num_channels,fp);
      fwrite(char_data+bytes_to_skip+i*stepbytes, header.bytes_per_pixel, 
             header.width*header.num_channels, fp);
   }
}

// ---------------------------------------------------------------------
// Member function flip_data flips the rows within input char array
// data[] and returns the flipped output within data_flipped[].  As of
// 6/23/05, we do NOT believe that there is any need to call this
// method prior to de-mosaicing.

void VidFile::flip_data(char data[],char data_flipped[])
{
   for (int r=0; r<header.height; r++)
   {
      for (int c=0; c<header.width; c++)
      {
         data_flipped[(header.height-1-r)*header.width+c]=
            data[r*header.width+c];
      }
   }
}

// ------------------------------------------------------------------------
int VidFile::image_size_in_bytes() 
{
   int ImageSize = (header.width*header.height*header.num_channels) 
      + header.imageHeaderBytes + header.imageFooterBytes;
   return ImageSize;
}

// ------------------------------------------------------------------------
void VidFile::query_structure_values() 
{
   cout << endl;
   cout << "==========================================================="
        << endl;
   cout << "Number of images in video file = " << getNumFrames() << endl;
   cout << "Number of color channels = " << getNumChannels() << endl;
   cout << "Number of bytes in header = " << header.bytes_in_header << endl;
//   cout << "Data type = " << getDataType() << endl;
   cout << "Image height in pixels = " << getHeight() << endl;
   cout << "Image width in pixels = " << getWidth() << endl;
   cout << "Bytes per pixel = " << getBytesPerPixel() << endl;
//   cout << "Precision = " << getPrecision() << endl << endl;
   cout << "==========================================================="
        << endl << endl;

/*
  cout << "Image header bytes = " << header.imageHeaderBytes << endl;
  cout << "Image footer bytes = " << header.imageFooterBytes << endl;
  cout << "Version ID = " << header.versionID << endl;
  cout << "month = " << header.month << endl;
  cout << "day = " << header.day << endl;
  cout << "year = " << header.year << endl;
  cout << "sec since midnight = " << header.secSinceMidnight << endl;
  cout << "micro seconds = " << header.uSeconds << endl;
*/

}

// ------------------------------------------------------------------------
void VidFile::reset()
{
   byte_counter = sizeof(header);
}

// ------------------------------------------------------------------------
void VidFile::read_next_image(unsigned char* data_ptr)
{
//   cout << "byte_counter = " << byte_counter << endl;
//   cout << "frame number = " << (byte_counter-header.bytes_in_header)/
//      image_size_in_bytes() << endl;

   fseeko(fp,byte_counter,SEEK_SET);
   if(fread(reinterpret_cast<void*>(data_ptr), image_size_in_bytes(), 1, fp) ==
      0) cout << "No elements read wtihin VidFile::read_next_image()" << endl;
   byte_counter += image_size_in_bytes();
}

// ------------------------------------------------------------------------
void VidFile::read_image(
   const unsigned int p_imageNum,unsigned char* p_dataPtr)
{
   if (p_imageNum >= 0)
   {
      byte_counter = sizeof(header) + image_size_in_bytes() * p_imageNum;

      fseeko(fp,byte_counter,SEEK_SET);
      if(fread(reinterpret_cast<void*>(p_dataPtr), 
               image_size_in_bytes(), 1, fp) == 0)
      {
         cout << "No elements read within VidFile::read_image()" << endl;
      }
      
      byte_counter += image_size_in_bytes();
   }
}

// ========================================================================
// Greyscale methods:
// ========================================================================

// Member function pixel_greyscale_intensity_value takes in integer
// pixel coordinates (px,py) for an image which is assumed to have one
// greyscale channel.  It returns this pixel's intensity value which
// ranges from 0 to 255.

int VidFile::pixel_greyscale_intensity_value(
   int px,int py,const unsigned char* data_ptr) const
{

// See note at top of this file:

   int p=py*getWidth()+px;

   if (p <0 || p >= getWidth()*getHeight())
   {
      cout << "Error in VidFile::pixel_greyscale_intensity_value()!" << endl;
      cout << "p = " << p << " is >= getWidth()*getHeight() = "
           << getWidth()*getHeight() << endl;
      p=getWidth()*getHeight()-1;
   }

   return stringfunc::unsigned_char_to_ascii_integer(data_ptr[p]);
}

// ------------------------------------------------------------------------
// Member function generate_intensity_twoDarray dynamically allocates
// and initializes a twoDarray to hold intensity data read from Group
// 99 greyscale video files. 

void VidFile::generate_intensity_twoDarray()
{
   delete intensity_twoDarray_ptr;
   intensity_twoDarray_ptr=new twoDarray(getWidth(),getHeight());

   intensity_twoDarray_ptr->set_xlo(0);
   intensity_twoDarray_ptr->set_xhi(getWidth());
   intensity_twoDarray_ptr->set_ylo(0);
   intensity_twoDarray_ptr->set_yhi(getHeight());
   intensity_twoDarray_ptr->set_deltax(1);
   intensity_twoDarray_ptr->set_deltay(1);

//   cout << "*intensity_twoDarray_ptr = " << *intensity_twoDarray_ptr << endl;
}

void VidFile::delete_intensity_twoDarray()
{
   delete intensity_twoDarray_ptr;
}

// ------------------------------------------------------------------------
// Member function convert_charstar_array_to_intensity_twoDarray fills
// the *intensity_twoDarray_ptr member twoDarray with greyscale values
// extracted from input unsigned char array *data_ptr.

void VidFile::convert_charstar_array_to_intensity_twoDarray(
   unsigned char* data_ptr)
{
   for (unsigned int py=0; py<intensity_twoDarray_ptr->get_ndim(); py++)
   {
      for (unsigned int px=0; px <intensity_twoDarray_ptr->get_mdim(); px++)
      {
         int i=((getHeight()-1)-py)*getWidth()+px;
         double value=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i]);
         intensity_twoDarray_ptr->put(px,py,value);
      } // loop over px
   } // loop over py
}

// ---------------------------------------------------------------------
// Member function convert_intensity_twoDarray_to_charstar_array fills
// input an unsigned char* array with values from member twoDarray
// *intensity_twoDarray_ptr.

void VidFile::convert_intensity_twoDarray_to_charstar_array(
   unsigned char* data_ptr)
{
   unsigned int mdim=intensity_twoDarray_ptr->get_mdim();
   unsigned int ndim=intensity_twoDarray_ptr->get_ndim();

   for (unsigned int py=0; py<ndim; py++)
   {
      for (unsigned int px=0; px<mdim; px++)
      {
         unsigned int p=py*mdim+px;
         data_ptr[p]=stringfunc::ascii_integer_to_unsigned_char(
            intensity_twoDarray_ptr->get(px,py));
      } // loop over px index
   } // loop over py index
}
