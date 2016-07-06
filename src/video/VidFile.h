// ========================================================================
// Header for Group 99 RGB video file parser class
// ========================================================================
// Last updated on 9/18/07; 10/22/07; 7/25/10
// ========================================================================

#ifndef VIDFILE_H
#define VIDFILE_H

// Note: Group 99 RGB video files have a 40 byte header followed by
// numframes images.  Each individual image is just a sequence of
// 3*image_size_in_bytes() RGB triples.

#include <string>
#include "datastructures/Quadruple.h"
#include "datastructures/Triple.h"
#include "image/TwoDarray.h"

class prob_distribution;

typedef unsigned short _uint16;
typedef unsigned int _uint32;

enum vid_dataTypes 
{ 
   VID_1B = 1, //bool
   VID_8U, // byte
   VID_8S, 
   VID_16U,
   VID_16S,
   VID_32U,
   VID_32S,
   VID_32F,
   VID_64U,
   VID_64S,
   VID_64F
};

typedef struct
{
      _uint16 endian;				//0  
	  	//will be 1, always... which byte tells you endian-ness
      _uint16 bytes_in_header;	//2  //should be 40 + size of comment
      _uint16 width;				//4
      _uint16 height;				//6
      _uint32 numframes;			//8
      _uint16 num_channels;			//12
      _uint16 bytes_per_pixel;			//14
      _uint16 data_type;			//16	
				//writer can optionally specify
      _uint16 data_precision;			//18	
				//writer can optionally specify
      _uint16 imageHeaderBytes;		   	//20
      _uint16 imageFooterBytes;			//22
      _uint16 versionID;			//24
      _uint16 month;				//26
      _uint16 day;				//28
      _uint16 year;				//30
      _uint32 secSinceMidnight;			//32
      _uint32 uSeconds;				//36
}VID_HEADER;

class VidFile
{

  public:

   VidFile();
   VidFile(std::string filename);
   virtual ~VidFile();

// Set & get member functions:
      
   int getNumFrames() { return header.numframes; }
   int getNumChannels() { return header.num_channels; }
   int getDataType() { return header.data_type; }
   int getHeight() const { return header.height; }
   int getWidth() const { return header.width; }
   int getBytesPerPixel() { return header.bytes_per_pixel; }
   int getPrecision() { return header.data_precision; }
   int get_bytes_in_header() {return header.bytes_in_header; }
   int get_imageHeaderBytes() {return header.imageHeaderBytes; }
   int get_imageFooterBytes() { return header.imageFooterBytes; }
      
   void set_intensity_twoDarray_ptr(twoDarray* ztwoDarray_ptr);
   twoDarray* get_intensity_twoDarray_ptr();
   const twoDarray* get_intensity_twoDarray_ptr() const;

// Initialization and parsing member functions:

   int Open(const char* name);
   int New_8U(std::string filename, int w, int h, int N, int C);
   void WriteFrame(void* data,int stepbytes,int bytes_to_skip=0);
   void flip_data(char data[],char data_flipped[]);

   int image_size_in_bytes();
   void query_structure_values();
   void reset();
   void read_next_image(unsigned char* data_ptr);
   void read_image(const unsigned int p_imageNum, unsigned char* p_dataPtr);

// Greyscale member functions:

   int pixel_greyscale_intensity_value(
      int px,int py,const unsigned char* data_ptr) const;
   void generate_intensity_twoDarray();
   void delete_intensity_twoDarray();
   void convert_charstar_array_to_intensity_twoDarray(
      unsigned char* data_ptr);
   void convert_intensity_twoDarray_to_charstar_array(
      unsigned char* data_ptr);

  private:

   unsigned int byte_counter;

// Note: As of Aug 2005, we do not know how to successfully read in
// files larger than 2 GByte in size using C++ ifstream calls.  So in
// order to accomodate very large video files, we are forced to work
// with C-style FILE* pointers instead...

   FILE* fp;	
   VID_HEADER header;
   twoDarray* intensity_twoDarray_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void VidFile::set_intensity_twoDarray_ptr(
   twoDarray* ztwoDarray_ptr)
{
   intensity_twoDarray_ptr=ztwoDarray_ptr;
}

inline twoDarray* VidFile::get_intensity_twoDarray_ptr()
{
   return intensity_twoDarray_ptr;
}

inline const twoDarray* VidFile::get_intensity_twoDarray_ptr() const
{
   return intensity_twoDarray_ptr;
}

#endif // VidFile.h
