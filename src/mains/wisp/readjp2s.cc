// ==========================================================================
// Program READJP2S
// ==========================================================================
// Last updated on 4/11/11; 4/16/11; 4/17/11; 5/29/11
// ==========================================================================

#include <iomanip>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "kakadu/kakadufuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string subdir="/media/10E8-0ED2/wisp/";
   string header_filename=subdir+"junk.index";
   string panos_filename=subdir+"junk.jp2s";

   int n_frames,JP2_size_in_bytes;
   int frame_width,frame_height;
   kakadufunc::ParseWISPHeaderFile(
      header_filename,n_frames,JP2_size_in_bytes,frame_width,frame_height);

   cout << "n_frames = " << n_frames << endl;
   cout << "frame_width = " << frame_width 
        << " frame_height = " << frame_height << endl;

   twoDarray* ptwoDarray_ptr=new twoDarray(frame_width,frame_height);

// Loop over all frames and extract their intensities from the JP2S
// file to a short integer buffer.  Perform histogram equalization on
// the extracted intensities.  Write out equalized intensities to PNG
// files:

   const int n_channels=3;
//   n_frames=1;
//   n_frames=3;
   for (int framenumber=0; framenumber<n_frames; framenumber++)
   {
      string banner="Processing frame "+stringfunc::number_to_string(
         framenumber);
      outputfunc::write_big_banner(banner);

      short* intensities_buffer=kakadufunc::ExtractWISPFrameData(
         panos_filename,framenumber,JP2_size_in_bytes);

      int start_index=0;
      int stop_index=frame_width*frame_height;

// Compute minimal and maximal intensity values for current frame:

      double xmin=POSITIVEINFINITY;
      double xmax=NEGATIVEINFINITY;
      for (int i=start_index; i<stop_index; i++)
      {
         double raw_value=intensities_buffer[i];
         xmin=basic_math::min(xmin,raw_value);
         xmax=basic_math::max(xmax,raw_value);
      } // loop over py index
      cout << "Min raw intensity value = " << xmin << endl;
      cout << "Max raw intensity value = " << xmax << endl;

// Renormalize intensities so that min & max are mapped to 0 and 1:

      cout << "Renormalizing raw intensity values:" << endl;
      vector<double> ren_values;
      for (int i=start_index; i<stop_index; i++)
      {
         double ren_value=(intensities_buffer[i]-xmin)/(xmax-xmin);
         if (ren_value < 0) ren_value=0;
         if (ren_value > 1) ren_value=1;
         ren_values.push_back(ren_value);
      }

      int n_output_bins=10000;
      prob_distribution ren_prob(ren_values,n_output_bins,0);

// Perform histogram equalization on raw WISP intensity values:

      cout << "Performing histogram equalization on renormalized intensities:"
           << endl;

      int counter=start_index;
      for (int py=0; py<frame_height; py++)
      {
         for (int px=0; px<frame_width; px++)
         {
            double raw_intensity=intensities_buffer[counter++];
            double renorm_intensity=(raw_intensity-xmin)/(xmax-xmin);   
            int n=ren_prob.get_bin_number(renorm_intensity);
            double pcum=ren_prob.get_pcum(n);
            ptwoDarray_ptr->put(px,py,pcum);

/*
            if (px==20000 && py > 500 && py < 510)
            {
               cout << "px = " << px << " py = " << py << endl;
               cout << "raw_intensity = " << raw_intensity
                    << " renorm_intensity = " << renorm_intensity << endl;
               cout << "n = " << n 
                    << " pcum = " << pcum << " counter = " << counter << endl;
            }
*/

         } // loop over px index
      } // loop over py index

      delete [] intensities_buffer;

      cout << "Writing out PNG file for frame " 
           << stringfunc::number_to_string(framenumber) << endl;
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
      texture_rectangle_ptr->initialize_twoDarray_image(
         ptwoDarray_ptr,n_channels);   
      string image_filename="frame_"+
         stringfunc::integer_to_string(framenumber,4)+".png";
      texture_rectangle_ptr->write_curr_frame(image_filename);
      delete texture_rectangle_ptr;

   } // loop over framenumber index

   delete ptwoDarray_ptr;

}

