// ========================================================================
// Header for Group 99 raw data file parser class
// ========================================================================
// Last updated on 8/12/05
// ========================================================================

#ifndef G99_RAW_H
#define G99_RAW_H

#include <fstream>
#include <string>
#include "video/D10GrabberFileHdr.h"

class VidFile;

class RawFile
{

  public:

   RawFile(std::string filename);
   virtual ~RawFile();

   void set_start_frame(int frame);
   void set_stop_frame(int frame);
   void set_nframes_to_skip(int n);
   FILE* get_fpImg();
   tD10GrabberFileHdr* get_hdrD10in_ptr();
   const tD10GrabberFileHdr* get_hdrD10in_ptr() const;

   void parse_input_arguments(int argc,char* argv[]);
   void read_file_header();
   void query_header_structure_values();
   void compute_n_raw_images();
   void compute_output_video_image_params();
   void write_output_header();
   void advance_raw_file_counter();
   void write_output_video_images();

  private:

   enum FILEFORMAT {RAW_FORMAT=0, VID_FORMAT=1, IMG_FORMAT=2};

   bool wholeImage;
   FILEFORMAT output_fileformat;
   int n_raw_images,total_bytes_per_raw_image;
   int in_width,in_height,out_width,out_height;
   int start_frame,stop_frame,nframes_to_skip,n_output_images;
   int crop_x0,crop_y0,crop_width,crop_height;
   double scale;
   std::string output_filename;
   FILE* fpImg;
   tD10GrabberFileHdr* hdrD10in_ptr;
   VidFile* output_image_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void RawFile::set_start_frame(int frame)
{
   start_frame=frame;
}

inline void RawFile::set_stop_frame(int frame)
{
   stop_frame=frame;
}

inline void RawFile::set_nframes_to_skip(int n)
{
   nframes_to_skip=n;
}

inline FILE* RawFile::get_fpImg() 
{
   return fpImg;
}

inline tD10GrabberFileHdr* RawFile::get_hdrD10in_ptr() 
{
   return hdrD10in_ptr;
}

inline const tD10GrabberFileHdr* RawFile::get_hdrD10in_ptr() const
{
   return hdrD10in_ptr;
}

#endif // G99_RAW.h
