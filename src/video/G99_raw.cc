// ========================================================================
// Group 99 raw data file parser class
// ========================================================================
// Last updated on 8/24/05
// ========================================================================

#include <iostream>
#include <string> 	// needed for memset
#include "general/filefuncs.h"
#include "video/G99_raw.h"
#include "video/VidFile.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void RawFile::allocate_member_objects()
{
   hdrD10in_ptr=new tD10GrabberFileHdr;
}		       

void RawFile::initialize_member_objects()
{
   wholeImage=true;
   start_frame=0;
   stop_frame=10;
   nframes_to_skip=1;

   crop_x0=crop_y0=-1;
   crop_width=crop_height=0;
   scale=1;
   fpImg=NULL;
   hdrD10in_ptr=NULL;
}		       

RawFile::RawFile(string raw_filename)
{
   initialize_member_objects();
   allocate_member_objects();
   fpImg=fopen(raw_filename.c_str(), "rb" );
   if (!fpImg)
   {
      cout << "Input raw G99 video file is bad !!" << endl;
      exit(-1);
   }
}

RawFile::~RawFile()
{
   if (fpImg != NULL) fclose(fpImg);
   delete hdrD10in_ptr;
}

// ------------------------------------------------------------------------
// Member function parse_input_arguments strips off the ".raw" suffix
// from the input raw video filename entered on the command line and
// forms a corresponding ".vid" filename for the output file.  This
// method also parses the starting & stopping frame numbers as well as
// the number of frames to skip between each successive image in the
// output .vid file.

void RawFile::parse_input_arguments(int argc,char* argv[])
{
   string raw_filename=string(argv[1]);
//   cout << "raw_filename = " << raw_filename << endl;
   unsigned int dot_posn=raw_filename.rfind(".raw");
   output_filename=raw_filename.substr(0,dot_posn)+"_greyscale.vid";  

   int i = 2;
   if (argc > 2)
   {
      string candidate_output_filename=string(argv[2]);
      if (candidate_output_filename.substr(0,1) != "-")
      {
         output_filename=candidate_output_filename;
         i=3;
      }
   }
   cout << "Output filename = " << output_filename << endl;
   
   string suffix=stringfunc::suffix(output_filename);
   if (suffix=="vid")
   {
      output_fileformat=VID_FORMAT;
   }
   else if (suffix=="raw")
   {
      output_fileformat=RAW_FORMAT;
   }
   else
   {
      cout << "Error in RawFile::parse_input_arguments()" << endl;
      cout << "Cannot recognize suffix = " << suffix << endl;
      exit(-1);
   }

   cout << "i = " << i << endl;

   while (i < argc)
   {
      if (strcmp(argv[i],"-frames") == 0)
      {
         start_frame = atoi(argv[i+1]);
         stop_frame = atoi(argv[i+2]);
         i += 2;
      } 
      else if (strcmp(argv[i],"-crop") == 0)
      {
         wholeImage = false;
         crop_x0 = atoi(argv[i+1]);
         crop_y0 = atoi(argv[i+2]);
         crop_width = atoi(argv[i+3]);
         crop_height = atoi(argv[i+4]);
         i+=4;
      } 
      else if (strcmp(argv[i],"-skip") == 0)
      {
         nframes_to_skip = atoi(argv[i+1]);
         cout << "nframes_to_skip = " << nframes_to_skip << endl;
         i++;
      }
      else if (strcmp(argv[i],"-scale") == 0)
      {
         scale = atof(argv[i+1]);
         i++;
      }
      else cout << "Illegal argument " << argv[i] << endl;
      i++;
   } // i < argc while loop

   n_output_images=(stop_frame-start_frame)/nframes_to_skip+1;
   cout << "n_output_images = " << n_output_images << endl;
}

// ------------------------------------------------------------------------
void RawFile::read_file_header()
{
   unsigned int hdr_size=sizeof(*hdrD10in_ptr);
   memset(hdrD10in_ptr,0,hdr_size);
   if(fread(hdrD10in_ptr, 1, hdr_size, fpImg) == 0)
   {
      cout << "No elements read in RawFile::read_file_header()" << endl;
   }
   

   in_width=hdrD10in_ptr->row_length;
   in_height=hdrD10in_ptr->col_length;
   
   query_header_structure_values();

   total_bytes_per_raw_image=hdrD10in_ptr->dwBytesPerImgDiv10 
      + hdrD10in_ptr->wBytesPerImgExtraDiv10;
   cout << "# bytes in raw video file header = " << hdr_size << endl;
   cout << "# bytes in each image = " << hdrD10in_ptr->dwBytesPerImgDiv10
        << endl;
   cout << "# bytes in image footer = " 
        << hdrD10in_ptr->wBytesPerImgExtraDiv10 << endl;
   cout << "# total bytes per raw image = " 
        << total_bytes_per_raw_image << endl;
}

// ------------------------------------------------------------------------
void RawFile::compute_n_raw_images()
{
   off_t hdr_size=sizeof(*hdrD10in_ptr);
   fseeko(fpImg,0,SEEK_END);
   off_t total_raw_file_size=ftello(fpImg);
   n_raw_images=(total_raw_file_size-hdr_size)/total_bytes_per_raw_image;
//   cout << "hdr_size = " << hdr_size << endl;
   cout << "Total raw file size = " << total_raw_file_size << endl;
   cout << "Number of raw images = " << n_raw_images << endl;
}

// ------------------------------------------------------------------------
void RawFile::query_header_structure_values() 
{
   cout << endl;
   cout << "==========================================================="
        << endl;

   cout << "Major version = " << hdrD10in_ptr->major_version << endl;
   cout << "Minor version = " << hdrD10in_ptr->minor_version << endl;
   cout << "Bits per pixel = " << hdrD10in_ptr->bits_per_pixel << endl;
   cout << "Start time = " << hdrD10in_ptr->start_time << endl;
   cout << "Stop time = " << hdrD10in_ptr->end_time << endl;
   cout << "Starting row = "<< hdrD10in_ptr->row_start << endl;
   cout << "Row length = " << hdrD10in_ptr->row_length << endl;
   cout << "Starting column = " << hdrD10in_ptr->col_start << endl;
   cout << "Column length = " << hdrD10in_ptr->col_length << endl;
   cout << "Frames requested = " << hdrD10in_ptr->frames_requested << endl;
   cout << "Good frames = " << hdrD10in_ptr->frames_good << endl;
   cout << "big_little = " << hdrD10in_ptr->big_little << endl;
   cout << "Frame type = " << hdrD10in_ptr->frame_type << endl;
   cout << "dwBytesPerImgDiv10 = " << hdrD10in_ptr->dwBytesPerImgDiv10 
        << endl;
   cout << "wBytesPerImgExtraDiv10 = "
        << hdrD10in_ptr->wBytesPerImgExtraDiv10 << endl;
   cout << "==========================================================="
        << endl << endl;
}

// ------------------------------------------------------------------------
void RawFile::compute_output_video_image_params()
{
   n_output_images=(stop_frame-start_frame)/nframes_to_skip+1;
   if (crop_x0 != -1)	// crop
   {	       
      out_width = scale*crop_width;
      out_height = scale*crop_height;
   }
   else 		// don't crop
   {				       
      crop_x0 = 0;
      crop_y0 = 0;
      crop_width = in_width;
      crop_height = in_height;
      
      out_width=scale*in_width;
      out_height=scale*in_height;
   }
}

// ------------------------------------------------------------------------
void RawFile::write_output_header()
{
   output_image_ptr=new VidFile();
   output_image_ptr->New_8U(
      output_filename,out_width,out_height,n_output_images, 1);
}

// ------------------------------------------------------------------------
void RawFile::advance_raw_file_counter()
{

// First skip over raw file's header section:

   off_t hdr_size=sizeof(*hdrD10in_ptr);
   fseeko(fpImg,hdr_size,SEEK_SET);

// Next skip over frames 0 through start_frame-1:

   fseeko(fpImg,start_frame*total_bytes_per_raw_image,SEEK_CUR);

//   off_t curr_counter=ftello(fpImg);
//   cout << "curr_counter = " << curr_counter << endl;
//   cout << "fpImg = " << fpImg << endl << endl;
}

// ------------------------------------------------------------------------
void RawFile::write_output_video_images()
{
   const int total_bytes_per_output_image = hdrD10in_ptr->dwBytesPerImgDiv10 
      + hdrD10in_ptr->wBytesPerImgExtraDiv10;

   unsigned char* pbyImgIn = new unsigned char[total_bytes_per_raw_image];
   unsigned char* pbyImgIn2 = pbyImgIn + 4;
   unsigned char* pbyImgOut = new unsigned char[total_bytes_per_output_image];

   cout << "Processing frames " << start_frame << " to " << stop_frame 
        << endl;
   cout << "nframes_to_skip = " << nframes_to_skip << endl;

   for (int i = start_frame; i <= stop_frame; i+= nframes_to_skip)
   {
      cout << i << " " << flush;

// Read in next raw image:      

      if(fread( pbyImgIn, 1, total_bytes_per_raw_image, fpImg ) == 0)
      {
         cout << "No elements read in RawFile::write_output_video_images()"
              << endl;
      }
      

// Skip over nframes_to_skip-1 images to get to next raw image:

      fseeko( fpImg, total_bytes_per_raw_image*(nframes_to_skip - 1), 
              SEEK_CUR );

      if (wholeImage) 
      { //just copy the whole image
         memcpy(pbyImgOut, pbyImgIn2, out_width*out_height);
      } 
      else 
      {		// copy cropped region
         unsigned char* pby_tmp = pbyImgIn2 + (crop_y0*in_width + crop_x0);
         unsigned char* pby_tmp2 = pbyImgOut;
         for (int jj = 0; jj < crop_height; jj++)
         {
            memcpy(pby_tmp2, pby_tmp, out_width);
            pby_tmp2 += out_width;
            pby_tmp += in_width;
         }
      }

      output_image_ptr->WriteFrame(pbyImgOut, out_width);
   } // loop over index i labeling output images in .vid file
   cout << endl;
}
