// ========================================================================
// Program MSER_REGIONS queries the user to enter an image filename.
// It then calls the Oxford MSER binary to compute positive and
// negative maximally stable extremal regions within the input image.
// The positive and negative regions are represented by colored pixel blobs
// in two output versions of the original image.

//				  mser_regions

// ========================================================================
// Last updated on 6/7/12; 6/8/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "geometry/polygon.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{
   texture_rectangle* input_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* output_texture_rectangle_ptr=new texture_rectangle();

   string image_filename="Kstreet.png";
   cout << "Enter input image:" << endl;
   cin >> image_filename;
   
// Find MSERs within input image:

   for (int color_channel=0; color_channel<=3; color_channel++)
   {
      input_texture_rectangle_ptr->import_photo_from_file(image_filename);

      if (color_channel==0)
      {
         input_texture_rectangle_ptr->convert_color_image_to_greyscale();
      }
      else
      {
         bool generate_greyscale_image_flag=false;
         input_texture_rectangle_ptr->
            convert_color_image_to_single_color_channel(
               color_channel,generate_greyscale_image_flag);
      }

      if (color_channel==0 || color_channel==1)
      {
         output_texture_rectangle_ptr->import_photo_from_file(image_filename);
         output_texture_rectangle_ptr->convert_color_image_to_greyscale();
      }
         

      string curr_channel_filename="curr_channel.png";
      input_texture_rectangle_ptr->write_curr_frame(curr_channel_filename);

//   cout << "width = " << input_texture_rectangle_ptr->getWidth() 
//        << " height = " << input_texture_rectangle_ptr->getHeight() << endl;

      string unix_cmd="mser.ln -i "+curr_channel_filename+" -t 0";
//      string unix_cmd="mser.ln -i "+image_filename+" -t 0";
      sysfunc::unix_command(unix_cmd);

      string mser_filename=curr_channel_filename+".rle";
      filefunc::ReadInfile(mser_filename);

      int line_counter=0;

      int n_mser_plus=stringfunc::string_to_number(filefunc::text_line[
         line_counter++]);
      cout << "n_mser_plus = " << n_mser_plus << endl;
   
      int iskip=1;
      int region_counter=1;
      int MSER_pixel_counter=0;
      int R,G,B;
      for (int i=0; i<n_mser_plus; i += iskip)
      {
         int curr_color_index=(region_counter++)%12;
         colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(
            colorfunc::get_color(curr_color_index));
         R=255*curr_RGB.first;
         G=255*curr_RGB.second;
         B=255*curr_RGB.third;

//      cout << "line_counter = " << line_counter << endl;
         vector<double> column_values=
            stringfunc::string_to_numbers(filefunc::text_line[line_counter++]);

         int n_rle=column_values[0];
         for (int n=0; n<n_rle; n++)
         {
            int row=column_values[3*n+1];
            int col_start=column_values[3*n+2];
            int col_stop=column_values[3*n+3];

            int pv=row;
            for (int pu=col_start; pu<=col_stop; pu++)
            {
               output_texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
               MSER_pixel_counter++;
            }
         } // loop over index n labeing extended boundary extrema points
      } // loop over index i labeling positive MSERs

      int n_mser_minus=stringfunc::string_to_number(filefunc::text_line[
         line_counter++]);
      cout << "n_mser_minus = " << n_mser_minus << endl;

      for (int i=0; i<n_mser_minus; i += iskip)
      {
         int curr_color_index=(region_counter++)%12;
         colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(
            colorfunc::get_color(curr_color_index));
         R=255*curr_RGB.first;
         G=255*curr_RGB.second;
         B=255*curr_RGB.third;

//      cout << "line_counter = " << line_counter << endl;
         vector<double> column_values=
            stringfunc::string_to_numbers(filefunc::text_line[line_counter++]);

         int n_rle=column_values[0];
         for (int n=0; n<n_rle; n++)
         {
            int row=column_values[3*n+1];
            int col_start=column_values[3*n+2];
            int col_stop=column_values[3*n+3];

            int pv=row;
            for (int pu=col_start; pu<=col_stop; pu++)
            {
               output_texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
               MSER_pixel_counter++;
            }
         } // loop over index n labeing extended boundary extrema points
      } // loop over index i labeling negative MSERs

      filefunc::deletefile(mser_filename);
      
      if (color_channel==0 || color_channel==3)
      {
         string MSER_output_filename="MSER_channel_"+
            stringfunc::number_to_string(color_channel)+".png";
         output_texture_rectangle_ptr->write_curr_frame(MSER_output_filename);
      }
      
      int n_total_pixels=output_texture_rectangle_ptr->getWidth()*
         output_texture_rectangle_ptr->getHeight();
      double MSER_frac=double(MSER_pixel_counter)/double(n_total_pixels);
      cout << "MSER fraction = " << MSER_frac << endl;

   } // loop over color_channel index

}

