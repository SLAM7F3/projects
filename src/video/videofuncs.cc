// Note added on 3/2/16:  See note below !!!!

// On 10/13/13, we empirically observed that it's slightly faster for
// us to outer-loop over pv and inner-loop over pu than to instead
// outer-loop over pu and inner-loop over pv:

// ==========================================================================
// Videofuncs namespace method definitions
// ==========================================================================
// Last modified on 7/23/16; 7/30/16; 8/1/16; 8/19/16
// ==========================================================================

#include <iostream>
#include <Magick++.h>
#include <stdlib.h>

#include "osg/osgGraphicals/AnimationController.h"
#include "math/basic_math.h"
#include "astro_geo/Clock.h"
#include "geometry/contour.h"
#include "image/drawfuncs.h"
#include "image/extremal_region.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "postgres/gis_database.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "geometry/homography.h"
#include "image/imagefuncs.h"
#include "image/lsd.h"
#include "math/mathfuncs.h"
#include "messenger/Messenger.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "video/pa_struct.h"
#include "image/pngfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "math/twovector.h"
#include "video/videofuncs.h"

#include "numrec/nrfuncs.h"

using std::cerr;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

namespace videofunc
{

  genmatrix R_camera(3,3);
   
  XYZ_MAP* xyz_map_ptr=new XYZ_MAP;
  CAMERAID_XYZ_MAP* cameraID_xyz_map_ptr=new CAMERAID_XYZ_MAP;

  // ==========================================================================
  // RGB methods
  // ==========================================================================

  // Method demosaic implement's Mike Braun's edge sensing algorithm:

  void demosaic(
    unsigned char* grayImg, unsigned char* rgbImg, int width, int height)
  {
    const int Thresh = 100;
    int index, vDiff, hDiff;
    int tmp1, tmp2, tmp3;

    // Assumes CFA pattern:
    // G B
    // R G
	
    /********************** GREEN *******************/
    // Fill in data
    for (int i = 0; i < height; i++)
    {
      tmp1 = i*width;
      for (int j = i%2; j < width; j+=2)
      {
        index = tmp1 + j;
        rgbImg[(3*index)+1] = grayImg[index];
      }
    }

    //interior holes
    for (int i = 1; i < height-1; i++)
    {
      tmp1 = i*width;
      for (int j = (i%2) + 1; j < width-1; j+=2)
      {
        index = tmp1 + j;
        vDiff = abs(grayImg[index - width] - grayImg[index + width]);
        hDiff = abs(grayImg[index - 1] - grayImg[index + 1]);
        if (vDiff < hDiff && vDiff < Thresh)
        {
          rgbImg[(3*index)+1] = (grayImg[index - width] + 
                                 grayImg[index + width])/2;
        }
        else if (hDiff < vDiff && hDiff < Thresh)
        {
          rgbImg[(3*index)+1] = 
            (grayImg[index - 1] + grayImg[index + 1])/2;
        } 
        else 
        {
          rgbImg[(3*index)+1] = 
            (grayImg[index - 1] + grayImg[index + 1] + 
             grayImg[index - width] + grayImg[index + width])/4;
        }
      }
    }

    //top edge
    for (int j = 1; j < width; j+=2)
    { 
      rgbImg[(3*j) + 1] = grayImg[width+j];
    }

    //bottom edge
    for (int j = height%2; j < width; j+=2)
    {
      index = (width*(height-1)) + j;
      rgbImg[(3*index) + 1] = grayImg[index - width];
    }

    //left edge
    for (int j = 1; j < height; j+=2)
    {
      index = width*j;
      rgbImg[(3*index) + 1] = grayImg[index + 1];
    }

    //right edge
    for (int j = width%2; j < height; j+=2)
    {
      index = width*(j+1) - 1;
      rgbImg[(3*index) + 1] = grayImg[index - 1];
    }

    /********************** end GREEN *******************/

    /********************** BLUE *******************/

    //values
    for (int i = 0; i < height; i+=2)
    {
      tmp1 = i*width;
      for (int j = 1; j < width; j+=2)
      {
        index = tmp1 + j;
        rgbImg[3*index + 2] = grayImg[index];
      }
    }

    //horizontal green holes
    for (int i = 0; i < height; i+=2)
    {
      tmp1 = i*width;
      for (int j = 2; j < width-1; j+=2)
      {
        index = tmp1 + j;
        tmp2 = (rgbImg[3*(index-1) + 2] + rgbImg[3*(index+1) + 2])/2;
        tmp3 = rgbImg[(3*index)+1] - (rgbImg[(3*(index-1))+1] 
                                      + rgbImg[(3*(index+1))+1])/2;
        rgbImg[3*index + 2] = CLIP(tmp2 + tmp3, 0, 255);
      }
    }

    //vertical green holes
    for (int i = 1; i < height-1; i+=2)
    {
      tmp1 = i*width;
      for (int j = 1; j < width; j+=2)
      {
        index = tmp1 + j;
        tmp2 = (rgbImg[3*(index-width) + 2] 
                + rgbImg[3*(index+width) + 2])/2;
        tmp3 = rgbImg[(3*index)+1] - (rgbImg[(3*(index-width))+1] 
                                      + rgbImg[(3*(index+width))+1])/2;
        rgbImg[3*index + 2] = CLIP(tmp2 + tmp3, 0, 255);
      }
    }

    //interior holes
    for (int i = 1; i < height-1; i+=2)
    {
      tmp1 = i*width;
      for (int j = 2; j < width-1; j+=2)
      {
        index = tmp1 + j;
			
        vDiff = abs(grayImg[index - width - 1] 
                    - grayImg[index + width + 1]);
        hDiff = abs(grayImg[index - width + 1] 
                    - grayImg[index + width - 1]);
        if (vDiff < hDiff && vDiff < Thresh)
        {
          rgbImg[3*index + 2] = (grayImg[index - width - 1] 
                                 + grayImg[index + width + 1])/2;
        }
        else if (hDiff < vDiff && hDiff < Thresh)
        {
          rgbImg[3*index + 2] = (grayImg[index - width + 1] 
                                 + grayImg[index + width - 1])/2;
        } 
        else 
        {
          rgbImg[3*index + 2] = 
            (grayImg[index - width - 1] 
             + grayImg[index + width + 1] + 
             grayImg[index - width + 1] + 
             grayImg[index + width - 1])/4;
        }
      }
    }

    //first column
    index = 0;
    for (int i = 0; i < height; i++)
    {
      rgbImg[3*index + 2] = rgbImg[3*(index+1) + 2];
      index += width;
    }

    //last column
    if (width%2)
    {
      index = width-1;
      for (int i = 0; i < height; i++)
      {
        rgbImg[3*index + 2] = rgbImg[3*(index-1) + 2];
        index += width;
      }
    }

    //last row
    if ((height+1)%2)
    {
      index = width*(height-1);
      for (int i = 0; i < width; i++)
      {
        rgbImg[3*index + 2] = rgbImg[3*(index-width) + 2];
        index++;
      }
    }

    /********************** end BLUE *******************/

    /********************** RED *******************/

    //values
    for (int i = 1; i < height; i+=2)
    {
      tmp1 = i*width;
      for (int j = 0; j < width; j+=2)
      {
        index = tmp1 + j;
        rgbImg[3*index] = grayImg[index];
      }
    }

    //horizontal green holes
    for (int i = 1; i < height; i+=2)
    {
      tmp1 = i*width;
      for (int j = 1; j < width-1; j+=2)
      {
        index = tmp1 + j;
        tmp2 = (rgbImg[3*(index-1)] + rgbImg[3*(index+1)])/2;
        tmp3 = rgbImg[(3*index)+1] - (rgbImg[(3*(index-1))+1] 
                                      + rgbImg[(3*(index+1))+1])/2;
        rgbImg[3*index] = CLIP(tmp2 + tmp3, 0, 255);
      }
    }

    //vertical green holes
    for (int i = 2; i < height-1; i+=2)
    {
      tmp1 = i*width;
      for (int j = 0; j < width; j+=2)
      {
        index = tmp1 + j;
        tmp2 = (rgbImg[3*(index-width)] + rgbImg[3*(index+width)])/2;
        tmp3 = rgbImg[(3*index)+1] - (rgbImg[(3*(index-width))+1] 
                                      + rgbImg[(3*(index+width))+1])/2;
        rgbImg[3*index] = CLIP(tmp2 + tmp3, 0, 255);
      }
    }

    //interior holes
    for (int i = 2; i < height-1; i+=2)
    {
      tmp1 = i*width;
      for (int j = 1; j < width-1; j+=2)
      {
        index = tmp1 + j;
			
        vDiff = abs(grayImg[index - width - 1] 
                    - grayImg[index + width + 1]);
        hDiff = abs(grayImg[index - width + 1] 
                    - grayImg[index + width - 1]);
        if (vDiff < hDiff && vDiff < Thresh)
        {
          rgbImg[3*index] = (grayImg[index - width - 1] 
                             + grayImg[index + width + 1])/2;
        }
        else if (hDiff < vDiff && hDiff < Thresh)
        {
          rgbImg[3*index] = (grayImg[index - width + 1] 
                             + grayImg[index + width - 1])/2;
        } 
        else 
        {
          rgbImg[3*index] = 
            (grayImg[index - width - 1] 
             + grayImg[index + width + 1] 
             + grayImg[index - width + 1] 
             + grayImg[index + width - 1])/4;
        }
      }
    }

    //first row
    for (int i = 0; i < width; i++)
    {
      rgbImg[3*i] = rgbImg[3*(i+width)];
    }

    //last row
    if (height%2)
    {
      index = width*(height-1);
      for (int i = 0; i < width; i++)
      {
        rgbImg[3*index] = rgbImg[3*(index-width)];
        index++;
      }
    }

    //last column
    if ((width+1)%2)
    {
      index = width-1;
      for (int i = 0; i < height; i++)
      {
        rgbImg[3*index] = rgbImg[3*(index-1)];
        index += width;
      }
    }

    /********************** end RED *******************/

    /*
    // Check how many pixels in raw greyscale image are saturated with
    // values = 255:

    for (int n=0; n<width*height; n++)
    {
    int grey=stringfunc::unsigned_char_to_ascii_integer(grayImg[n]);
    if (grey >= 255)
    {
    rgbImg[3*n+0]=255;
    rgbImg[3*n+1]=0;
    rgbImg[3*n+2]=0;
    }
    }
    */

  }

  // ------------------------------------------------------------------------
  // Method gain was once-upon-a-time used by Braun to individually
  // amplify R, G and B values.  But he abandoned it since this method
  // induces RGB value wrap-around (i.e. 257 -> 2, etc).

  void gain(unsigned char* rgbImg, int width, int height)
  {
    const double redGain = 1.0;
    const double greenGain = 1.0;
    const double blueGain = 1.0;
    for (int i = 0; i < width*height; i++)
    {
      rgbImg[i*3] = CLIP(redGain*rgbImg[i*3],0.0,255.0);
      rgbImg[i*3+1] = CLIP(greenGain*rgbImg[i*3 + 1],0.0,255.0);
      rgbImg[i*3+2] = CLIP(blueGain*rgbImg[i*3 + 2],0.0,255.0);
    }
  }

  // ------------------------------------------------------------------------
  // Method RGB_to_grey takes RGB array *rgbImg, applies fixed weights
  // to each of its RGB elements, and returns the greyscale results
  // within *greyImg.

  void RGB_to_grey(
    unsigned char* rgbImg, unsigned char* greyImg, int width, int height)
  {

    // Weight factors to convert RGB to grey taken from function
    // GetGreyScale by Marco Bellinaso:

    const double w_red=0.3;
    const double w_green=0.59;
    const double w_blue=0.11;
         
    for (int i=0; i<height*width; i++)
    {
      int r=stringfunc::unsigned_char_to_ascii_integer(rgbImg[3*i+0]);
      int g=stringfunc::unsigned_char_to_ascii_integer(rgbImg[3*i+1]);
      int b=stringfunc::unsigned_char_to_ascii_integer(rgbImg[3*i+2]);
      int grey=basic_math::min(255,basic_math::round(
                                 w_red*r+w_green*g+w_blue*b));
      greyImg[i]=stringfunc::ascii_integer_to_unsigned_char(grey);
    }
  }

  // ------------------------------------------------------------------------
  void RGB_to_newRGB(
    unsigned char* rgbImg, unsigned char* newrgbImg, int width, int height)
  {
    const double w_red=255.0/140.0;
    const double w_green=1.0;
    const double w_blue=255.0/174.0;
         
    for (int i=0; i<height*width; i++)
    {
      int r=w_red*
        stringfunc::unsigned_char_to_ascii_integer(rgbImg[3*i+0]);
      int g=w_green*
        stringfunc::unsigned_char_to_ascii_integer(rgbImg[3*i+1]);
      int b=w_blue*
        stringfunc::unsigned_char_to_ascii_integer(rgbImg[3*i+2]);
      newrgbImg[3*i+0]=basic_math::min(255,r);
      newrgbImg[3*i+1]=basic_math::min(255,g);
      newrgbImg[3*i+2]=basic_math::min(255,b);
    }
  }

// -------------------------------------------------------------------------
// Method compute_average_subimage_RGB() loops over all pixels within
// a specified bounding box for input texture rectangle *tr_ptr.  It
// ignores any pixel whose alpha value = 0.  This method returns the
// average R, G, B values for the alpha != 0 pixels within the
// specified bbox.

   bool compute_average_subimage_RGB(
      texture_rectangle* tr_ptr, 
      int px_min, int px_max, int py_min, int py_max,
      colorfunc::RGB& average_RGB)
   {
//      cout << "inside videofunc::compute_average_subimage_RGB()" << endl;
      int xdim = tr_ptr->getWidth();
      int ydim = tr_ptr->getHeight();
      int n_channels = tr_ptr->getNchannels();

      if(n_channels < 3 || n_channels > 4) return false;
      
      px_min = basic_math::min(xdim-1, px_min);
      px_max = basic_math::max(0, px_max);
      py_min = basic_math::min(ydim-1, py_min);
      py_max = basic_math::max(0, py_max);

//      cout << "px_min = " << px_min << " px_max = " << px_max << endl;
//      cout << "py_min = " << py_min << " py_max = " << py_max << endl;
//      cout << "n_channels = " << n_channels << endl;

      int R,G,B,A;
      int Rsum, Gsum, Bsum;
      Rsum = Gsum = Bsum = 0;
      int pixel_counter = 0;
      for(int py = py_min; py <= py_max; py++)
      {
         for(int px = px_min; px <= px_max; px++)
         {
            if(n_channels == 4)
            {
               tr_ptr->get_pixel_RGBA_values(px,py,R,G,B,A);
               if(A == 0) continue;
            }
            else if (n_channels == 3)
            {
               tr_ptr->get_pixel_RGB_values(px,py,R,G,B);
            }
            
            if(R < 0 || G < 0 || B < 0) continue;

            Rsum += R;
            Gsum += G;
            Bsum += B;
            pixel_counter++;
         }
      }
      
      if(pixel_counter == 0)
      {
         return false;
      }
      else
      {
         average_RGB.first = double(Rsum) / pixel_counter;
         average_RGB.second = double(Gsum) / pixel_counter;
         average_RGB.third = double(Bsum) / pixel_counter;
         return true;
      }
   }

  // ==========================================================================
  // Jason Cardema's (Group 104) IRIG timing methods which he kindly
  // supplied to us in August 2005
  // ==========================================================================

  double SinceMidnight(pa_struct& x)
  {
    double d = x.irig_useconds_20_bit;
    d /= 1e6;
    d += x.irig_seconds_17_bit;
    return d;
  }

  // ------------------------------------------------------------------------
  // Method irig2utc(s) converts from IRIG time in seconds to UTC.  It
  // returns the time in a string of the form "hr:mn:sc".

  void irig2utc(char timestr[], int irig_sec, int irig_usec)
  {
    int 	hr, min;
    float	sec;
    char	hr_str[16], min_str[16], sec_str[16];
	
    const int HR_IN_SEC = 3600;
    const int MIN_IN_SEC = 60;
	
    hr = 0;
    min = 0;
    sec = 0;
	
    while (irig_sec > 0) {
      if (irig_sec >= HR_IN_SEC) {
        irig_sec -= HR_IN_SEC;
        hr++;
      } else if (irig_sec >= MIN_IN_SEC) {
        irig_sec -= MIN_IN_SEC;
        min++;
      } else {
        irig_sec--;
        sec++;
      }
    }
    sec += irig_usec*1e-6;
	

    sprintf(hr_str, "%02d", hr);
    sprintf(min_str, "%02d", min);
    sprintf(sec_str, "%09.6f", sec);

    sprintf(timestr, "%s%s%s", hr_str, min_str, sec_str);
	
    return;
  }

  // ------------------------------------------------------------------------
  void irig2date(char datestr[], int irig_day)
  {
    int			i, day, month, year;
    int			days_per_month[12];
    const int 	CURRENT_YEAR = 04;
	
    day = 0;
    month = 1;
    year = CURRENT_YEAR;
	
    days_per_month[0] = 31;
    days_per_month[1] = 28;
    days_per_month[2] = 31;
    days_per_month[3] = 30;
    days_per_month[4] = 31;
    days_per_month[5] = 30;
    days_per_month[6] = 31;
    days_per_month[7] = 31;
    days_per_month[8] = 30;
    days_per_month[9] = 31;
    days_per_month[10] = 30;
    days_per_month[11] = 31;
	
    // Adjust for a leap year
    if (year % 4 == 0)
      days_per_month[1]++;

    i = 0;
    while (irig_day > days_per_month[i]) {
      irig_day -= days_per_month[i];
      month++;
      i++;
    }
    day = irig_day;
	
    sprintf(datestr, "%02d%02d%02d", day, month, year);
	
    return;
  }

  // ==========================================================================
  // ALIRT imagery generation methods
  // ==========================================================================

  /*
    void set_gross_sensor_cartesian_offsets(threevector& sensor_offset)
    {
    const double FIXED_OFFSET_LONG=-71+17.186/60.;
    const double FIXED_OFFSET_LAT=42+27.905/60.;
    const double FIXED_OFFSET_ALT=50;

    double offset_long = FIXED_OFFSET_LONG;
    double offset_lat = FIXED_OFFSET_LAT;
    double offset_alt = FIXED_OFFSET_ALT;

    string UTM_zone;
    double UTMNorthing,UTMEasting;
    latlongfunc::LLtoUTM(
    offset_lat,offset_long,UTM_zone,UTMNorthing,UTMEasting);
    sensor_offset=threevector(UTMEasting,UTMNorthing,offset_alt);
    cout << "Sensor's gross origin = " << sensor_offset << endl;
    }
  */

  // ---------------------------------------------------------------------
  // Method filter_raw_sensor_positions performs a brute force
  // convolution of raw sensor data within input STL vector
  // sensor_position with a variable sized gaussian filter.  At the
  // beginning of the raw data set, the gaussian's width linearly ramps
  // up from zero to time constant tau.  After plateauing at this
  // maximum value, the filter's width linearly ramps back down to zero
  // at the end of the data set.  This method overwrites the raw
  // positions within sensor_position with their filtered counterparts.

  void filter_raw_sensor_positions(
    int n_frames,double dt,double tau,
    vector<threevector>& sensor_position)
  {
    outputfunc::write_banner("Filtering raw sensor positions:");

    const double e_folding_distance=2.5;
    int max_n_size=filterfunc::gaussian_filter_size(
      tau,dt,e_folding_distance);

    double *filter=new double[max_n_size];
    vector<threevector> filtered_sensor_position;

    int frame_number=0;
    int prev_n_size=-1;
    while (frame_number < n_frames)
    {

      // To avoid divizion-by-zero problems, do not filter the first and
      // last few raw orientations:

      if (frame_number < 10 || n_frames-frame_number < 10)
      {
        filtered_sensor_position.push_back(sensor_position[
                                             frame_number]);
      }
      else
      {
        double filter_sum=0;
        threevector posn(0,0,0);
            
        double temporal_resolution=basic_math::min(
          tau,(frame_number-1)*dt/(e_folding_distance*SQRT_TWO),
          (n_frames-frame_number-1)*dt/(
            e_folding_distance*SQRT_TWO));
        int n_size=filterfunc::gaussian_filter_size(
          temporal_resolution,dt,e_folding_distance);
        int w=n_size/2;

        // Don't waste time recomputing gaussian filter unless its width has
        // changed:

        if (n_size != prev_n_size)
        {
          filterfunc::gaussian_filter(
            n_size,0,temporal_resolution,dt,filter);
          prev_n_size=n_size;
        }
               
        //               cout << "frame_number=" << frame_number
        //                    << " n_size=" << n_size 
        //                    << " max_n_size=" << max_n_size
        //                    << " sigma=" << temporal_resolution 
        //                    << " max_sigma=" << tau
        //                    << endl;
                            
        for (int i=0; i<n_size; i++)
        {
          int j=frame_number-w+i;
          if (j >= 0 && j < n_frames)
          {
            posn += sensor_position[j]*filter[i]*dt;
            filter_sum += filter[i]*dt;
          }
        } 

        posn /= filter_sum;
        filtered_sensor_position.push_back(posn);

      } // frame_number < 10 || n_frames-frame_number < 10
      //  conditional
      frame_number++;
    } // frame_number < n_frames while loop
    cout << endl;
    delete [] filter;

    // Overwrite raw sensor positions within sensor_position STL vector
    // with filtered ones:

    sensor_position.clear();
    for (frame_number=0; frame_number < n_frames; frame_number++)
    {
      sensor_position.push_back(
        filtered_sensor_position[frame_number]);
    }
  }

  // ==========================================================================
  // KLT testing methods
  // ==========================================================================

  void translate_image(
    unsigned char* inputImg, unsigned char* outputImg,
    int width, int height,int horiz_pixel_translation)
  {
    for (int i=0; i<height*width; i++)
    {
      outputImg[i]=stringfunc::ascii_integer_to_unsigned_char(0);  
    }

    for (int px=0; px < width; px++)
    {
      for (int py=0; py < height; py++)
      {
        int i=py*width+px;
        int grey=stringfunc::unsigned_char_to_ascii_integer(
          inputImg[i]);
        if (grey > 0)
          outputImg[i+horiz_pixel_translation]=
            inputImg[i];
      } // loop over py index
    } // loop over px index
  }

  // ==========================================================================
  // Thumbnail generation methods
  // ==========================================================================
      
  // Method get_thumbnail_filename() appends thumbnails/ to the current
  // photo's subdirectory and preprends thumbnail_prefix+"thumbnail_" to
  // the photo's filename.

  string get_thumbnail_filename(
    string input_filename,string thumbnail_prefix) 
  {
    string dirname=filefunc::getdirname(input_filename);


    // Note added on 11/20/13: Michael Yee's graph viewer occasionally has
    // catastrophic problems handling thumbnails generated from JPG
    // internet images.  So as a bandaid fix, we experiment with
    // generating all thumbnails in PNG format:

    //            string prefix=filefunc::getprefix(input_filename);
    //            string basename=prefix+".png";

    string basename=filefunc::getbasename(input_filename);

    string thumbnail_dirname=dirname+thumbnail_prefix+"thumbnails/";
    filefunc::dircreate(thumbnail_dirname);

    int posn=stringfunc::first_substring_location(
      basename,"thumbnail_");

    string thumbnail_filename;
    if (posn >= 0)
    {
      thumbnail_filename=thumbnail_dirname+thumbnail_prefix+basename;
    }
    else
    {
      thumbnail_filename=thumbnail_dirname+
        thumbnail_prefix+"thumbnail_"+basename;
    }
            
    return thumbnail_filename;
  }

  // -------------------------------------------------------------------------
  // Member function get_thumbnail_dims()

  void get_thumbnail_dims(
     unsigned int xdim,unsigned int ydim,
     unsigned int& thumbnail_xdim,unsigned int& thumbnail_ydim)
  {
     //   cout << "inside videofunc::get_thumbnail_dims()" << endl;

     const unsigned int max_thumbnail_pixel_dim=200;
//     const unsigned int max_thumbnail_pixel_dim=256;
     double aspect_ratio=double(xdim)/double(ydim);

     if (xdim <= max_thumbnail_pixel_dim && ydim <= max_thumbnail_pixel_dim)
     {
        thumbnail_xdim = xdim;
        thumbnail_ydim = ydim;
     }
     else if (xdim > ydim && xdim > max_thumbnail_pixel_dim)
     {
        thumbnail_xdim=max_thumbnail_pixel_dim;
        thumbnail_ydim=thumbnail_xdim/aspect_ratio;
     }
     else if (ydim > max_thumbnail_pixel_dim)
     {
        thumbnail_ydim=max_thumbnail_pixel_dim;
        thumbnail_xdim=thumbnail_ydim*aspect_ratio;
     }

/*
     // As of Nov 2015, we set min_thumbnail_pixel_dim = 230 for caffe
     // feature extraction purposes:

  const unsigned int min_thumbnail_pixel_dim=230;

  double aspect_ratio=double(xdim)/double(ydim);
  if (xdim <= min_thumbnail_pixel_dim || ydim <= min_thumbnail_pixel_dim)
  {
  thumbnail_xdim = xdim;
  thumbnail_ydim = ydim;
  }
  else 
  {
  if (xdim < ydim)
  {
  thumbnail_xdim=min_thumbnail_pixel_dim;
  thumbnail_ydim=thumbnail_xdim/aspect_ratio;
  }
  else
  {
  thumbnail_ydim=min_thumbnail_pixel_dim;
  thumbnail_xdim=thumbnail_ydim*aspect_ratio;
  }
  }
*/

  }

  // -------------------------------------------------------------------------
  // Method function generate_thumbnail() takes in the filename for some
  // image and uses ImageMagick++ to subsample it so that it takes up
  // less memory.

  string generate_thumbnail(
    string input_image_filename,double zoom_factor,
    string thumbnail_prefix)
  {
    //            cout << "inside videofunc::generate_thumbnail()" << endl;

    Magick::Image curr_image;
            
    curr_image.ping(input_image_filename);
    unsigned int n_columns=curr_image.columns();
    unsigned int n_rows=curr_image.rows();

    unsigned int thumbnail_columns=basic_math::round(
      n_columns*zoom_factor);
    unsigned int thumbnail_rows=basic_math::round(n_rows*zoom_factor);

    return generate_thumbnail(
      input_image_filename,n_columns,n_rows,
      thumbnail_columns,thumbnail_rows);
  }

  // -------------------------------------------------------------------------
  string generate_thumbnail(
    string input_image_filename,
    unsigned int orig_xdim,unsigned int orig_ydim,
    unsigned int new_xdim,unsigned int new_ydim)
  {
    string thumbnail_filename=videofunc::get_thumbnail_filename(
      input_image_filename);
    if(new_xdim == orig_xdim && new_ydim == orig_ydim)
    {
      string unix_cmd="cp "+input_image_filename+" "+thumbnail_filename;
      sysfunc::unix_command(unix_cmd);
    }
    else
    {
      resize_image(
        input_image_filename,orig_xdim,orig_ydim,new_xdim,new_ydim,
        thumbnail_filename);
    }
    return thumbnail_filename;
  }

  // -------------------------------------------------------------------------
  // Method function resize_image() takes in the filename for some
  // image along with original and new x and y pixel dimensions.  It
  // uses ImageMagick++ to resample the input image and return the
  // result within the specified output image file.

  bool resize_image(
    string image_filename,
    unsigned int orig_xdim,unsigned int orig_ydim,
    unsigned int new_xdim,unsigned int new_ydim)
  {
     string resized_image_filename = image_filename;
     return resize_image(image_filename, orig_xdim, orig_ydim, 
                         new_xdim, new_ydim, image_filename);
  }

  bool resize_image(
    string input_image_filename,
    unsigned int orig_xdim,unsigned int orig_ydim,
    unsigned int new_xdim,unsigned int new_ydim,
    string resized_image_filename)
  {
    //            cout << "inside videofunc::resize_image()" << endl;
    //            cout << "input_image_filename = " << input_image_filename
    //                 << endl;
    //            cout << "new_xdim = " << new_xdim << endl;
    //            cout << " new_ydim = " << new_ydim << endl;

    if (!filefunc::fileexist(input_image_filename)) return false;
    if (!imagefunc::valid_image_file(input_image_filename)) return false;

    Magick::Image curr_image;
    if(!import_IM_image(input_image_filename, curr_image))
    {
       cout << "Cannot resize input image " << input_image_filename
           << endl;
       return false;
    }

    resize_image(curr_image,new_xdim,new_ydim);
    export_IM_image(resized_image_filename, curr_image);
//     curr_image.write(resized_image_filename);
    return true;
  }
      
  // -------------------------------------------------------------------------
  // Method function downsize_image() takes in the filename for some
  // image along with maximum x and y pixel dimensions.  It uses
  // ImageMagick++ to resample the input image and return the result
  // within the specified output image file.

  void downsize_image(
    string image_filename,unsigned int max_xdim,unsigned int max_ydim,
    string downsized_image_filename)
  {
    unsigned int new_xdim,new_ydim;
    downsize_image(image_filename,max_xdim,max_ydim,
                   downsized_image_filename,new_xdim,new_ydim);
  }
      
  void downsize_image(
    string image_filename,unsigned int max_xdim,unsigned int max_ydim,
    string downsized_image_filename,
    unsigned int& new_xdim,unsigned int& new_ydim)
  {
    //            cout << "inside videofunc::downsize_image()" << endl;
    //            cout << "image_filename = " << image_filename << endl;

    unsigned int xdim,ydim;
    imagefunc::get_image_width_height(image_filename,xdim,ydim);
    downsize_image(image_filename, max_xdim, max_ydim, xdim, ydim,
                   downsized_image_filename, new_xdim, new_ydim);
  }

  void downsize_image(
    string image_filename,unsigned int max_xdim,unsigned int max_ydim,
    unsigned int xdim, unsigned int ydim, string downsized_image_filename,
    unsigned int& new_xdim,unsigned int& new_ydim)
  {
//    cout << "inside videofunc::downsize_image()" << endl;
//    cout << "image_filename = " << image_filename << endl;
//    cout << "xdim = " << xdim << " max_xdim = " << max_xdim << endl;
//    cout << "ydim = " << ydim << " max_ydim = " << max_ydim << endl;

    if (xdim < max_xdim && ydim < max_ydim) 
    {
      if(image_filename != downsized_image_filename)
      {
         string unix_cmd="cp "+image_filename+" "+
            downsized_image_filename;
         sysfunc::unix_command(unix_cmd);
      }
      return;
    }
            
    double aspect_ratio=double(xdim)/double(ydim);
    double xratio=double(xdim)/double(max_xdim);
    double yratio=double(ydim)/double(max_ydim);

    if (xratio > yratio)
    {
      new_xdim=max_xdim;
      new_ydim=new_xdim/aspect_ratio;
    }
    else
    {
      new_ydim=max_ydim;
      new_xdim=aspect_ratio*new_ydim;
    }

//    cout << "xdim = " << xdim << " new_xdim = " << new_xdim << endl;
//    cout << "ydim = " << ydim << " new_ydim = " << new_ydim << endl;

    resize_image(
      image_filename,xdim,ydim,new_xdim,new_ydim,
      downsized_image_filename);
  }

  void downsize_image(
    string image_filename,unsigned int max_xdim,unsigned int max_ydim)
  {
    downsize_image(image_filename,max_xdim,max_ydim,image_filename);
  }

  // -------------------------------------------------------------------------
  // Method function force_size_image() takes in the filename for some
  // image along with output x and y pixel dimensions.  It executes an
  // ImageMagick command which rescales and crops the image so that the
  // output image has precisely the specified pixel dimensions.

  void force_size_image(
    string image_filename,unsigned int output_xdim,unsigned int output_ydim,
    string resized_image_filename)
  {
    string output_xdim_str=stringfunc::number_to_string(output_xdim);
    string output_ydim_str=stringfunc::number_to_string(output_ydim);
      
    string unix_cmd="convert "+image_filename+
      " -resize "+output_xdim_str+"x"+output_ydim_str+
      "^ -gravity center -extent "+
      output_xdim_str+"x"+output_ydim_str+" "+
      resized_image_filename;
    sysfunc::unix_command(unix_cmd);
  }

  // -------------------------------------------------------------------------
  // Method function highpass_filter_image() 

  void highpass_filter_image(
    string image_filename,string filtered_image_filename)
  {
    string unix_cmd="convert "+image_filename+
      " -unsharp 1.5x1+0.7+0.02 "+filtered_image_filename;
    sysfunc::unix_command(unix_cmd);
  }

  // ==========================================================================
  // Bundler methods
  // ==========================================================================

  // Method import_photoID_XYZID_UV_bundler_pairs() takes in the name
  // for a text file containing photoID,XYZID,U,V information generated
  // by BUNDLER_CONVERT.  It parses this file and populates STL vectors
  // photo_ID, XYZ_ID and UV_sift with the imported data.

  void import_photoID_XYZID_UV_bundler_pairs(
    string camera_views_filename,vector<int>& photo_ID,
    vector<int>& XYZ_ID,vector<twovector>& UV_sift)
  {
    cout << "inside videofunc::import_photoID_XYZ_UV_bundler_pairs()" 
         << endl;
    cout << "camera_views_filename = " << camera_views_filename
         << endl;

    photo_ID.clear();
    XYZ_ID.clear();
    UV_sift.clear();
   
    filefunc::ReadInfile(camera_views_filename);
    for (unsigned int i=0; i<filefunc::text_line.size(); i++)
    {
      //      cout << "i = " << i << " line = " << filefunc::text_line[i] << endl;
      vector<double> fields=
        stringfunc::string_to_numbers(filefunc::text_line[i]);
      photo_ID.push_back(fields[0]);
      XYZ_ID.push_back(fields[1]);
      UV_sift.push_back(twovector(fields[2],fields[3]));
      //      cout << "photo=" << photo_ID[i]
      //           << " XYZ_ID = " << XYZ_ID[i]
      //           << " UV_sift = " << UV_sift[i] << endl;
    } // loop over index i labeling lines in camera_views_filename

    cout << "At end of videofunc::import_photoID_XYZ_UV_bundler_pairs()" << endl;
    cout << "photo_ID.size() = " << photo_ID.size() << endl;
    cout << "XYZ_ID.size() = " << XYZ_ID.size() << endl;
    cout << "UV_sift.size() = " << UV_sift.size() << endl;
  }

  // -------------------------------------------------------------------------
  // Method import_reconstructed_XYZ_points() takes in the name for a
  // text file containing XYZ points generated by BUNDLER_CONVERT.  It
  // parses this file and populates STL map *xyz_map_ptr with the
  // imported data.

  void import_reconstructed_XYZ_points(string xyz_points_filename)
  {
    //            cout << "inside videofunc::import_reconstructed_XYZ_points()" 
    //                 << endl;
    //            cout << "xyz_points_filename = " << xyz_points_filename << endl;
    //            cout << "xyz_map_ptr = " << xyz_map_ptr << endl;
    xyz_map_ptr->clear();

    filefunc::ReadInfile(xyz_points_filename);
    for (unsigned int i=0; i<filefunc::text_line.size(); i++)
    {
      vector<double> fields=
        stringfunc::string_to_numbers(filefunc::text_line[i]);
      int curr_xyz_id=fields[0];
      threevector curr_xyz_point(fields[1],fields[2],fields[3]);

      vector<int> camera_IDs;
      for (unsigned int j=4; j<fields.size(); j++)
      {
        int curr_camera_ID=fields[j];
        camera_IDs.push_back(curr_camera_ID);
      }
      //               cout << "curr_xyz_point = " << curr_xyz_point << endl;
               
      pair<threevector,vector<int> > P(curr_xyz_point,camera_IDs);
      (*xyz_map_ptr)[curr_xyz_id]=P;

    } // loop over index i labeling lines in xyz_points_filename

    //            cout << "xyz_map_ptr->size() = " << xyz_map_ptr->size() 
    //                 << endl;
    //            cout << "At end of videofunc::import_reconstructed_XYZ_points()"
    //                 << endl;
  }
      
  // --------------------------------------------------------------------------
  // Method sort_XYZ_points_by_camera_ID()

  CAMERAID_XYZ_MAP* sort_XYZ_points_by_camera_ID()
  {
    //            cout << "inside videofunc::sort_XYZ_points_by_camera_ID()" 
    //                 << endl;

    cameraID_xyz_map_ptr->clear();

    for (XYZ_MAP::iterator iter=xyz_map_ptr->begin();
         iter != xyz_map_ptr->end(); iter++)
    {
      threevector curr_XYZ((iter->second).first);
      vector<int> visible_cameras=(iter->second).second;

      for (unsigned int c=0; c<visible_cameras.size(); c++)
      {
        int curr_camera_ID=visible_cameras[c];
        CAMERAID_XYZ_MAP::iterator camid_xyz_iter=
          cameraID_xyz_map_ptr->find(curr_camera_ID);
        if (camid_xyz_iter==cameraID_xyz_map_ptr->end())
        {

          // As of 5/11/12, VIEWBUNDLER stalls when calling this next line.  We
          // don't know why...

          (*cameraID_xyz_map_ptr)[curr_camera_ID].push_back(
            curr_XYZ);
        }
        else
        {
          (camid_xyz_iter->second).push_back(curr_XYZ);
        }
      } // loop over index c labeling cameras which can view curr_XYZ
    } // iterator over all reconstructed XYZ points

    //            cout << "cameraID_xyz_map_ptr->size() = "
    //                 << cameraID_xyz_map_ptr->size() << endl;
    return cameraID_xyz_map_ptr;
  }

  // ==========================================================================
  // Photograph input methods
  // ==========================================================================

  // Method get_next_photo_filename() takes in the next
  // image number.  This method returns a still image filename of the
  // form XXXX-NNNN.suffix or XXXX_NNNN.suffix.  If input bool
  // prev_number_flag==true [false], this method returns a new filename
  // where the index number is decreased [increased] by one.

  string get_next_photo_filename(
    const vector<string>& photo_filename_substrings,string separator_char,
    int next_imagenumber)
  {
    //         cout << "inside videofunc::get_next_photo_filename()" << endl;

    int n_substrings=photo_filename_substrings.size();
    //         cout << "n_substrings = " << n_substrings << endl;

    int n_digits=photo_filename_substrings[n_substrings-2].size();
    //         cout << "n_digits = " << n_digits << endl;
    string next_imagenumber_str=stringfunc::integer_to_string(
      next_imagenumber,n_digits);
    //         cout << "next_imagenumber_str = " << next_imagenumber_str << endl;

    string next_photo_filename;
    for (int n=0; n<n_substrings-2; n++)
    {
      //            cout << "n = " << n << " photo_filename_substrings[n] = "
      //                 << photo_filename_substrings[n] << endl;
      next_photo_filename += photo_filename_substrings[n];
      //            cout << "next_photo_filename = " << next_photo_filename << endl;
    }
    next_photo_filename += separator_char+next_imagenumber_str+"."
      +photo_filename_substrings[n_substrings-1];

    //         string next_photo_filename=photo_filename_substrings[0]+"-"
    //            +next_imagenumber_str+"."+photo_filename_substrings[2];
    //         cout << "next_photo_filename = " << next_photo_filename << endl;
    return next_photo_filename;
  }

  // ---------------------------------------------------------------------
  // Method find_min_max_photo_numbers() takes in a subdirectory which
  // is assumed to hold video frames whose basenames are of the form
  // XXXX-NNNNN.png or XXXXX_NNNNN.JPG.  It scans over all integer NNNNN
  // values and returns the minimal and maximal number values for all
  // the video frames.  This method also returns the full pathname for
  // the image corresponding to the minimal photo number.

  string find_min_max_photo_numbers(
    string subdir,int& min_photo_number,int& max_photo_number)
  {
    //         cout << "inside videofunc::find_min_max_photo_numbers()" << endl;

    vector<string> filenames=find_numbered_image_filenames(subdir);

    string min_photo_filename;
    min_photo_number=POSITIVEINFINITY;
    max_photo_number=NEGATIVEINFINITY;
    for (unsigned int f=0; f<filenames.size(); f++)
    {
      string base_filename=filefunc::getbasename(filenames[f]);

      vector<string> substrings=
        stringfunc::decompose_string_into_substrings(
          base_filename,"-_.");
      //            cout << "f = " << f << " filenames[f] = " << filenames[f] << endl;
            
      if (substrings.size() < 3) continue;

      // As of January 2011, we assume true imagenumber occurs within
      // next-to-last substring.  If this substring is NOT numeric, search
      // for genuinely numeric substring in next-to-next-to-last substring, 
      // next-to-next-to-next-to-last substring, etc:

      string curr_substring;
      for (int s=int(substrings.size()-2); s>=0; s--)
      {
        curr_substring=substrings[s];
        //               cout << "s = " << s << " curr_substring = " 
        //		      << curr_substring << endl;
        if (stringfunc::is_number(curr_substring)) break;
      }
      int curr_imagenumber=stringfunc::string_to_number(curr_substring);
      //            cout << "curr_imagenumber = " << curr_imagenumber << endl;

      min_photo_number=basic_math::min(
        min_photo_number,curr_imagenumber);

      if (curr_imagenumber==min_photo_number)
      {
        min_photo_filename=filenames[f];
      }
      max_photo_number=basic_math::max(
        max_photo_number,curr_imagenumber);

    } // loop over index f labeling filenames

    //         cout << "min_photo_number = " << min_photo_number
    //              << " max_photo_number = " << max_photo_number << endl;

    return min_photo_filename;
  }

  // ---------------------------------------------------------------------
  // Method find_numbered_image_filenames() takes in a subdirectory which
  // is assumed to hold a temporally ordered sequence of image files.
  // It returns an STL string with the names of the image files it finds
  // within the specified subdir.

  vector<string> find_numbered_image_filenames(string subdir)
  {
    //         cout << "inside videofunc::extract_numbered_image_filenames()" << endl;
    vector<string> allowed_suffixes;
    allowed_suffixes.push_back("jpg");
    allowed_suffixes.push_back("JPG");
    allowed_suffixes.push_back("jpeg");
    allowed_suffixes.push_back("JPEG");
    allowed_suffixes.push_back("png");
    allowed_suffixes.push_back("PNG");
    allowed_suffixes.push_back("rgb");
    allowed_suffixes.push_back("tif");

    vector<string> ordered_image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
        allowed_suffixes,subdir);
    return ordered_image_filenames;
  }

  // ==========================================================================
  // ActiveMQ message methods
  // ==========================================================================

  // Method broadcast_video_params() takes in the starting & stopping
  // image numbers for some video clip as well as the total number of
  // frames.  It broadcasts this metadata to ActiveMQ messenger
  // *messenger_ptr.
      
  void broadcast_video_params(
    Messenger* messenger_ptr,int start_imagenumber,int stop_imagenumber,
    int n_images)
  {
    cout << "inside videofunc::broadcast_video_params()" << endl;
         
    string command,key,value;
    vector<Messenger::Property> properties;

    command="SEND_NIMAGES";
    cout << "command = " << command << endl;
         
    key="Start Image Number";
    value=stringfunc::number_to_string(start_imagenumber);
    properties.push_back(Messenger::Property(key,value));
    cout << "key = " << key << " value = " << value << endl;

    key="Stop Image Number";
    value=stringfunc::number_to_string(stop_imagenumber);
    properties.push_back(Messenger::Property(key,value));
    cout << "key = " << key << " value = " << value << endl;

    key="Nimages";
    value=stringfunc::number_to_string(n_images);
    properties.push_back(Messenger::Property(key,value));
    cout << "key = " << key << " value = " << value << endl;

    // FAKE FAKE:  Sat Feb 11, 2012 at 10:57 am
    // Comment out next line for debugging only...

    //         messenger_ptr->broadcast_subpacket(command,properties);
  }

  // ---------------------------------------------------------------------
  // Method broadcast_current_image_URL() broadcasts via ActiveMQ the
  // URL for the current image.

  void broadcast_current_image_URL(
    Messenger* messenger_ptr,int n_frames,string URL,int npx,int npy)
  {
    cout << "inside videofunc::broadcast_current_image()" << endl;
    cout << "n_frames = " << n_frames << endl;

    string command,key,value;
    vector<Messenger::Property> properties;

    command="UPDATE_IMAGE";
    //         cout << "command = " << command << endl;
         
    key="n_frames";
    value=stringfunc::number_to_string(n_frames);
    properties.push_back(Messenger::Property(key,value));

    key="URL";
    value=URL;
    properties.push_back(Messenger::Property(key,value));

    key="Npx";
    value=stringfunc::number_to_string(npx);
    properties.push_back(Messenger::Property(key,value));

    key="Npy";
    value=stringfunc::number_to_string(npy);
    properties.push_back(Messenger::Property(key,value));

    messenger_ptr->broadcast_subpacket(command,properties);
  }

  // ---------------------------------------------------------------------
  // Method broadcast_selected_image() broadcasts via ActiveMQ the
  // TOC-assigned ID and URL for an image selected by the user via the
  // "Annotate Current Frame" button.

  void broadcast_selected_image(
    Messenger* messenger_ptr,int image_ID,string URL,
    int npx,int npy)
  {
    //         cout << "inside videofunc::broadcast_selected_image()" << endl;
         
    string command,key,value;
    vector<Messenger::Property> properties;

    command="ANNOTATE_IMAGE";
    //         cout << "command = " << command << endl;
         
    key="Image ID";
    value=stringfunc::number_to_string(image_ID);
    properties.push_back(Messenger::Property(key,value));

    key="URL";
    value=URL;
    properties.push_back(Messenger::Property(key,value));

    key="Npx";
    value=stringfunc::number_to_string(npx);
    properties.push_back(Messenger::Property(key,value));

    key="Npy";
    value=stringfunc::number_to_string(npy);
    properties.push_back(Messenger::Property(key,value));

    messenger_ptr->broadcast_subpacket(command,properties);
  }

  // ==========================================================================
  // Orthorectification methods
  // ==========================================================================

  // Method compute_image_corner_world_coords() takes in a set of image plane
  // points and their corresponding lon-lat tiepoints in STL
  // vectors UV and XY respectively.  After converting XY from lon-lat to
  // UTM geocoordinates, this method computes the homography which maps
  // image plane onto the world plane.  It then projects the image's
  // corner points onto their world counterparts.
      
  void compute_image_corner_world_coords(
    vector<twovector>& XY,vector<twovector>& UV,double Umax,
    homography& H,twovector& lower_left_XY,twovector& lower_right_XY,
    twovector& upper_right_XY,twovector& upper_left_XY)
  {

    // Convert XY points from lon,lat to UTM geocoords:

    for (unsigned int i=0; i<XY.size(); i++)
    {
      cout << "i = " << i << " XY lon lat = " << XY[i] << endl;
      
      geopoint curr_tiepoint(XY[i].get(0),XY[i].get(1));
      XY[i].put(0,curr_tiepoint.get_UTM_easting());
      XY[i].put(1,curr_tiepoint.get_UTM_northing());
      cout << "XY UTM = " << XY[i] << endl;
    }

    H.parse_homography_inputs(XY,UV,XY.size());
    H.compute_homography_matrix();
    H.check_inverse_homography_matrix(UV,XY,UV.size());

    vector<twovector> XY_sorted,UV_sorted;
    XY_sorted=H.get_XY_sorted();
    UV_sorted=H.get_UV_sorted();

    // Recompute homography after eliminating worst tiepoint pairs from
    // initial homography computation:

    const double worst_frac=0.85;
    int n_best_inputs=(1-worst_frac)*XY.size();
    H.parse_homography_inputs(XY_sorted,UV_sorted,n_best_inputs);
    H.compute_homography_matrix();
    H.check_inverse_homography_matrix(
      UV_sorted,XY_sorted,n_best_inputs);

    //   cout << "H = " << *(H.get_H_ptr()) << endl;
    //   cout << "Hinv = " << *(H.get_Hinv_ptr()) << endl;

    double Umin=0;
    double Vmin=0;
    double Vmax=1;

    twovector lower_left_UV(Umin,Vmin);
    twovector lower_right_UV(Umax,Vmin);
    twovector upper_right_UV(Umax,Vmax);
    twovector upper_left_UV(Umin,Vmax);
   
    lower_left_XY=H.project_image_plane_to_world_plane(lower_left_UV);
    lower_right_XY=H.project_image_plane_to_world_plane(lower_right_UV);
    upper_right_XY=H.project_image_plane_to_world_plane(upper_right_UV);
    upper_left_XY=H.project_image_plane_to_world_plane(upper_left_UV);

    //         cout << "lower_left_XY = " << lower_left_XY << endl;
    //         cout << "lower_right_XY = " << lower_right_XY << endl;
    //         cout << "upper_right_XY = " << upper_right_XY << endl;
    //         cout << "upper_left_XY = " << upper_left_XY << endl;
  }

  // ---------------------------------------------------------------------
  // Method compute_extremal_easting_northing() takes in the geocoordinates
  // for an orthorectified image's corners.  It returns the extremal
  // easting and northing values among these 4 geocoords.  This method
  // also computes the width and height of a new image which encloses
  // the orthorectified image as a bounding box.  

  void compute_extremal_easting_northing(
    const twovector& lower_left_XY,const twovector& lower_right_XY,
    const twovector& upper_right_XY,const twovector& upper_left_XY,
    int xdim,int ydim,int& new_xdim,int& new_ydim,
    double& min_easting,double& max_easting,
    double& min_northing,double& max_northing)
  {
    //         cout << "inside videofunc::compute_extremal_easting_northing()"
    //	         << endl;

    double delta_x=(lower_right_XY-lower_left_XY).magnitude()/xdim;
    double delta_y=(upper_left_XY-lower_left_XY).magnitude()/ydim;
    cout << "delta_x = " << delta_x << " delta_y = " << delta_y << endl;
    double delta_s=basic_math::min(delta_x,delta_y);

    // Find extremal easting and northing values:

    min_easting=basic_math::min(
      lower_left_XY.get(0),lower_right_XY.get(0),
      upper_right_XY.get(0),upper_left_XY.get(0));
    min_northing=basic_math::min(
      lower_left_XY.get(1),lower_right_XY.get(1),
      upper_right_XY.get(1),upper_left_XY.get(1));
    max_easting=basic_math::max(
      lower_left_XY.get(0),lower_right_XY.get(0),
      upper_right_XY.get(0),upper_left_XY.get(0));
    max_northing=basic_math::max(
      lower_left_XY.get(1),lower_right_XY.get(1),
      upper_right_XY.get(1),upper_left_XY.get(1));
   
    cout << "min_easting = " << min_easting << endl;
    cout << "max_easting = " << max_easting << endl;
    cout << "min_northing = " << min_northing << endl;
    cout << "max_northing = " << max_northing << endl;

    double delta_easting = max_easting - min_easting;
    double delta_northing = max_northing - min_northing;
   
    cout << "Delta easting = " << delta_easting << endl;
    cout << "Delta easting / ds = " << delta_easting / delta_s << endl;
    cout << "Delta northing = " << delta_northing << endl;
    cout << "Delta northing / ds = " << delta_northing / delta_s << endl;

    new_xdim=(max_easting-min_easting)/delta_s+1;
    new_ydim=(max_northing-min_northing)/delta_s+1;
   
    cout << "new_xdim = " << new_xdim 
         << " new_ydim = " << new_ydim << endl;
  }

  // ==========================================================================
  // Blank image generation methods
  // ==========================================================================
   
  // Method generate_blank_imagefile() calls ImageMagick in order to
  // instantiate an image containing a pure grey background.

  string generate_blank_jpgfile(int n_horiz_pixels,int n_vertical_pixels)
  {
    //         cout << "inside videofunc::generate_blank_jpgfile()" << endl;

    string blank_filename="/tmp/blank.jpg";
    generate_blank_imagefile(
      n_horiz_pixels,n_vertical_pixels,blank_filename);
    return blank_filename;
  }

  string generate_blank_pngfile(int n_horiz_pixels,int n_vertical_pixels)
  {
    //         cout << "inside videofunc::generate_blank_jpgfile()" << endl;

    string blank_filename="/tmp/blank.png";
    generate_blank_imagefile(
      n_horiz_pixels,n_vertical_pixels,blank_filename);
    return blank_filename;
  }

  void generate_blank_imagefile(
    int n_horiz_pixels,int n_vertical_pixels,string blank_filename)
  {
    //         cout << "inside videofunc::generate_blank_imagefile()" << endl;
    //         cout << "n_horiz_pixels = " << n_horiz_pixels
    //              << " n_vertical_pixels = " << n_vertical_pixels << endl;
    //         cout << "blank_filename = " << blank_filename << endl;

    string geometry=stringfunc::number_to_string(n_horiz_pixels)+"x"+
      stringfunc::number_to_string(n_vertical_pixels);
    Magick::Image currimage(geometry.c_str(),"grey");
    currimage.write(blank_filename.c_str());
  }

  // ==========================================================================
  // Greyscale PNG export methods
  // ==========================================================================

  // Method write_8bit_greyscale_pngfile() imports a byte array.  It
  // opens various PNG datastructures, fills them with the input byte
  // information and exports an 8-bit greyscale png image.  
  // This method is adapted from code presented in
  // http://zarb.org/~gc/html/libpng.html and
  // http://www.libpng.org/pub/png/libpng-1.2.5-manual.html

  void write_8bit_greyscale_pngfile(
     vector<vector<unsigned char> >& byte_array, 
     string output_filename)
  {
    png_structp png_ptr = 
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (setjmp(png_jmpbuf(png_ptr)))
    {
      cout << "Error in videofunc::write_png_file() during init_io" << endl;
      exit(-1);
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
      cout << "Error in videofunc::write_png_file()" << endl;
      cout << "png_create_info_struct failed" << endl;
      exit(-1);
    }

    int height = byte_array.size();
    int width = byte_array.at(0).size();
    png_bytep* row_pointers = (png_bytep*) 
      png_malloc(png_ptr, height * sizeof(png_bytep));

    for (int i=0; i<height; i++)
    {
      row_pointers[i] = (png_bytep) png_malloc(png_ptr, width);
    }
   
    png_set_rows(png_ptr, info_ptr, row_pointers);

    for(int py = 0; py < height; py++)
    {
      for(int px = 0; px < width; px++)
      {
        row_pointers[py][px] = byte_array[py].at(px);
      }
    }

    FILE *fp = fopen(output_filename.c_str(), "wb");
    png_init_io(png_ptr, fp);

    // write header 
    if (setjmp(png_jmpbuf(png_ptr)))
    {
      cout << "Error in videofunc::write_png_file() during writing header" 
           << endl;
      exit(-1);
    }
   
    png_byte color_type = (png_byte) 0;
    png_byte bit_depth = (png_byte) 8;
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    // write bytes 
    if (setjmp(png_jmpbuf(png_ptr)))
    {
      cout << "Error in videofunc::write_png_file() during writing bytes" 
           << endl;
      exit(-1);
    }
   
    png_write_image(png_ptr, row_pointers);

    // end write 
    if (setjmp(png_jmpbuf(png_ptr)))
    {
      cout << "Error in videofunc::write_png_file() during end of write" 
           << endl;
      exit(-1);
    }

    png_write_end(png_ptr, NULL);

    // cleanup heap allocation 

    for (int py = 0; py < height; py++)
    {
      free(row_pointers[py]);
    }
    free(row_pointers);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
  }

  // ==========================================================================
  // AVI movie generation methods
  // ==========================================================================

  // Method generate_AVI_movie() runs the MENCODER program on a set
  // of images within a specified input subdirectory.  It uses the
  // specified codec for the output file.  

  string generate_AVI_movie(
    string video_codec,string input_imagery_subdir,string image_suffix,
    double fps,string output_movie_filename_prefix,
    string finished_movie_subdir)
  {
    //         cout << "inside videofunc::generate_AVI_movie()" << endl;
    //         cout << "video_codec = " << video_codec << endl;
    //         cout << "input_imagery_subdir = " << input_imagery_subdir << endl;

    string unix_command=
      "mencoder \"mf://"+input_imagery_subdir+"*."+image_suffix+"\" ";
    //   cout << "unix_command = " << unix_command << endl;
   
    unix_command += 
      //      "-ovc lavc -lavcopts vcodec=msmpeg4v2:vbitrate=24000000 -mf fps=5 ";
      "-ovc lavc -lavcopts ";
    unix_command += "vcodec="+video_codec+":";

    string output_suffix;

    // On 6/15/09, we empirically found that MPEG1 and MPEG2 codecs cannot
    // support frame rates smaller than 5 fps.  On the other hand, MPEG4
    // codec can support a 3 fps rate:

    if (video_codec=="mpeg1video")
    {
      output_suffix="_mp1.avi";
      unix_command += "vbitrate=24000000 -mf fps=5 ";
    }
    else if (video_codec=="mpeg2video")
    {
      output_suffix="_mp2.avi";
      unix_command += "vbitrate=24000000 -mf fps=5 ";
    }
    else if (video_codec=="msmpeg4v2")
    {
      output_suffix="_mp4.avi";
      unix_command += "vbitrate=24000000 -mf fps="
        + stringfunc::number_to_string(fps)+" ";
    }

    string output_movie_filename=output_movie_filename_prefix+
      output_suffix;

    unix_command += "-o "+output_movie_filename;
    cout << "unix_command = " << unix_command << endl;

    sysfunc::unix_command(unix_command);

    // Move movie into subdirectory on the computer's desktop:

    unix_command="mv "+output_movie_filename+" "+finished_movie_subdir;
    sysfunc::unix_command(unix_command);

    output_movie_filename=finished_movie_subdir+output_movie_filename;

    // Attempt to remain within this method until AVI movie has been
    // completely written to disk.  Keep checking size of output AVI movie
    // until it no longer changes...

    int movie_filesize=
      filefunc::size_of_file_in_bytes(output_movie_filename);
    cout << "movie_filesize = " << movie_filesize << endl;
         
    bool movie_writing_flag=true;
    while (movie_filesize < 10 || movie_writing_flag)
    {
      sleep(1);
      int curr_movie_filesize=
        filefunc::size_of_file_in_bytes(output_movie_filename);
      if (curr_movie_filesize == movie_filesize)
      {
        movie_writing_flag=false;
      }
      else
      {
        movie_filesize=curr_movie_filesize;
      }
      cout << "movie_filesize = " << movie_filesize << endl;
    }

    return output_movie_filename;
  }

  // ---------------------------------------------------------------------
  // Method generate_FLIR_AVI_movie() takes in start and stop times
  // (measured in secs since Jan 1, 1970) for some desired FLIR video
  // clip to be generated.  It creates soft links between FLIR video
  // frames within the specified input_imagery_subdir to a temporary
  // directory.  This method then calls generate_AVI_movie() which
  // creates an AVI movie using an MPEG4 codec from the FLIR frames.
  // Finally, this method destroys the temporary subdirectory holding
  // the soft linked frame filenames.

  string generate_FLIR_AVI_movie(
    string input_imagery_subdir,string image_suffix,
    int& AVI_movie_counter,
    string output_movie_filename_prefix,string finished_movie_subdir,
    double start_time,double stop_time,const vector<double>& epoch_time,
    const vector<string>& filename_stem)
  {
    int start_bin=mathfunc::mylocate(epoch_time,start_time);
    int stop_bin=mathfunc::mylocate(epoch_time,stop_time);
    start_bin=basic_math::max(0,start_bin);
    stop_bin=basic_math::min(int(epoch_time.size()-1),stop_bin);
    //         cout << "Start_bin = " << start_bin 
    //              << " stop_bin = " << stop_bin << endl;

    string tmpdirname="/tmp/AVI_movie"+
      stringfunc::number_to_string(AVI_movie_counter)+"/";
    //         cout << "tmpdirname = " << tmpdirname << endl;
    filefunc::dircreate(tmpdirname);
    AVI_movie_counter++;

    // Create soft links of FLIR video frames into temporary subdirectory:

    for (int bin=start_bin; bin<=stop_bin; bin++)
    {
      string curr_filename_stem=filename_stem[bin]; 
      //      cout << "b = " << bin << " filename = " << curr_filename_stem << endl;
      string curr_image_filename=input_imagery_subdir+
        curr_filename_stem+"."+image_suffix;
      string unix_cmd="ln -s "+curr_image_filename+" "+tmpdirname;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
    }

    string video_codec="msmpeg4v2";
    double fps=3;
    string AVI_movie_filename=videofunc::generate_AVI_movie(
      video_codec,tmpdirname,image_suffix,fps,
      output_movie_filename_prefix,finished_movie_subdir);

    // Purge temporary subdirectory holding soft links to FLIR video frames:

    string unix_cmd="/bin/rm -r -f "+tmpdirname;
    sysfunc::unix_command(unix_cmd);

    return AVI_movie_filename;
  }
      
  // ==========================================================================
  // Color histogram methods
  // ==========================================================================

  // Method generate_color_histogram() takes in *texture_rectangle_ptr
  // which is assumed to hold some input image.  It generates and returns
  // an STL vector containing integrated pixel count fractions for a
  // discrete set of colors.

  vector<double> generate_color_histogram(
    bool generate_quantized_color_image_flag,
    texture_rectangle* texture_rectangle_ptr,
    string& quantized_color_image_filename)
  {
    //         cout << "inside videofunc::generate_color_histogram()" << endl;

    vector<string> color_labels;
    color_labels.push_back("red");
    color_labels.push_back("orange");
    color_labels.push_back("yellow");
    color_labels.push_back("green");
    color_labels.push_back("blue");
    color_labels.push_back("purple");
    color_labels.push_back("black");
    color_labels.push_back("white");
    color_labels.push_back("grey");
    color_labels.push_back("brown");
    unsigned int n_color_bins=color_labels.size();
         
    vector<double> color_bins;
    for (unsigned int c=0; c<n_color_bins; c++)
    {
      color_bins.push_back(0);
    }

    int n_pixels=0;
    if (!quantize_image_coloring(
          generate_quantized_color_image_flag,
          texture_rectangle_ptr,color_bins,n_pixels,
          quantized_color_image_filename))
    {
      return color_bins;
    }
   
    double color_frac_integral=0;
    //         cout << "n_pixels = " << n_pixels << endl;
    for (unsigned int c=0; c<n_color_bins; c++)
    {
      double color_frac=color_bins[c]/n_pixels;
      color_bins[c]=color_frac;
      //            cout << "  color = " << color_labels[c]
      //                 << " fraction = " << color_bins[c] << endl;
      color_frac_integral += color_frac;
    }
    //         cout << "color frac integral = " << color_frac_integral << endl;

    return color_bins;
  }

  // ---------------------------------------------------------------------
  // Method quantize_image_coloring() takes in an image within
  // *texture_rectangle_ptr.  Looping over all pixels within the input
  // image, this method assigns each to a quantized color bin.  If input
  // boolean flag generate_quantized_color_image_flag==true, the pixels'
  // original RGB values are replaced with their quantized counterparts. 
  // If quantized_color_image_filename is an empty string, the
  // quantized image is exported to
  // /data/ImageEngine/quantized_colors.jpg.

  bool quantize_image_coloring(
    bool generate_quantized_color_image_flag,
    texture_rectangle* texture_rectangle_ptr,
    vector<double>& color_bins,int& n_pixels,
    string& quantized_color_image_filename)
  {
    //         cout << "inside videofunc::quantize_image_coloring()" << endl;

    // Perform color analysis only within central region of image:

    int width=texture_rectangle_ptr->getWidth();
    int height=texture_rectangle_ptr->getHeight();

    //         cout << "width = " << width
    //              << " height = " << height << endl;

    //       double crop_frac=0.05;
    double crop_frac=0.0;

    int pu_start=crop_frac*width;
    int pu_stop=(1-crop_frac)*width;
    int pv_start=(crop_frac)*height;
    int pv_stop=(1-crop_frac)*height;
    for (int pu=pu_start; pu<pu_stop; pu++)
    {
      for (int pv=pv_start; pv<pv_stop; pv++)
      {
        int R,G,B;
        texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);

        double r=R/255.0;
        double g=G/255.0;
        double b=B/255.0;
        double h,s,v;
        colorfunc::RGB_to_hsv(r,g,b,h,s,v);

        double h_quantized,s_quantized,v_quantized;
        int bin_number=colorfunc::assign_hsv_to_color_histogram_bin(
          h,s,v,h_quantized,s_quantized,v_quantized);
         
        //               cout << pu << " " << pv << "   " 
        //                    << R << " " << G << " " << B << "   "
        //                    << h << " " << s << " " << v << "   "
        //                    << endl;

        color_bins[bin_number]=color_bins[bin_number]+1;
        n_pixels++;

        if (generate_quantized_color_image_flag)
        {
          colorfunc::hsv_to_RGB(
            h_quantized,s_quantized,v_quantized,r,g,b);
          R=255*r;
          G=255*g;
          B=255*b;
          texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
        }
         
      } // loop over pv index
    } // loop over pu index

    // Export quantized color version of input image:

    if (quantized_color_image_filename.size()==0)
    {
      string subdir="/data/ImageEngine/";
      quantized_color_image_filename=subdir+"quantized_colors.jpg";
    }
         
    if (generate_quantized_color_image_flag)
    {
      texture_rectangle_ptr->write_curr_frame(
        quantized_color_image_filename);
    }

    return true;
  }
      
  // ---------------------------------------------------------------------
  // Method filter_color_image() takes in a raw, colored image within
  // twoDarrays *HtwoDarray_ptr, *StwoDarray_ptr and *VtwoDarray_ptr.
  // It computes a quantized color bin for each pixel within the image.
  // If the quantized bin number matches any of those within input STL
  // vector masked_color_bin_numbers, the pixel's color is reset to
  // black.  This method exports the filtered version of the input image
  // to filtered_image_filename.

  void filter_color_image(
    twoDarray* HtwoDarray_ptr,twoDarray* StwoDarray_ptr,
    twoDarray* VtwoDarray_ptr,
    const vector<int>& masked_color_bin_numbers,
    texture_rectangle* texture_rectangle_ptr,
    string filtered_image_filename)
  {
    //         cout << "inside videofunc::filter_color_image()" << endl;

    for (unsigned int pu=0; pu<HtwoDarray_ptr->get_mdim(); pu++)
    {
      for (unsigned int pv=0; pv<HtwoDarray_ptr->get_ndim(); pv++)
      {
        double h=HtwoDarray_ptr->get(pu,pv);
        double s=StwoDarray_ptr->get(pu,pv);
        double v=VtwoDarray_ptr->get(pu,pv);
               
        double h_quantized,s_quantized,v_quantized;
        int bin_number=colorfunc::assign_hsv_to_color_histogram_bin(
          h,s,v,h_quantized,s_quantized,v_quantized);

        for (unsigned int i=0; i<masked_color_bin_numbers.size(); i++)
        {
          if (bin_number==masked_color_bin_numbers[i])
          {
            v=0;
            VtwoDarray_ptr->put(pu,pv,0);
            break;
          }
        }

        double r,g,b;
        colorfunc::hsv_to_RGB(h,s,v,r,g,b);

        int R=255*r;
        int G=255*g;
        int B=255*b;
        texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
         
      } // loop over pv index
    } // loop over pu index

    // Export filtered version of input image:

    texture_rectangle_ptr->write_curr_frame(filtered_image_filename);
  }

  // ---------------------------------------------------------------------
  // Member function compute_RGB_fluctuations() converts the 1-pixel
  // runs within input STL vector member RLE_pixel_IDs into pixel
  // locations.  It retrieves RGB values for those pixels from
  // *texture_rectangle_ptr.  This method returns the quartile widths
  // of the pixels' RGB values.

  void compute_RGB_fluctuations(
    const vector<int>& RLE_pixel_IDs,
    const texture_rectangle* texture_rectangle_ptr,
    double& quartile_width_R,double& quartile_width_G,
    double& quartile_width_B)
    //         double& sigma_R,double& sigma_G,double& sigma_B)
  {
    //         cout << "inside videofunc::compute_RGB_fluctuations()" << endl;

    int xdim=texture_rectangle_ptr->getWidth();

    unsigned int px_start,px_stop,py_start,py_stop;
    int curr_R,curr_G,curr_B;
    vector<double> R,G,B;
    for (unsigned int i=0; i<RLE_pixel_IDs.size(); i += 2)
    {
      unsigned int start_pixel_ID=RLE_pixel_IDs[i];
      unsigned int stop_pixel_ID=RLE_pixel_IDs[i+1];
      graphicsfunc::get_pixel_px_py(
        start_pixel_ID,xdim,px_start,py_start);
      graphicsfunc::get_pixel_px_py(stop_pixel_ID,xdim,px_stop,py_stop);

      //      cout << "px_start = " << px_start << " px_stop = " << px_stop
      //           << " py_start = " << py_start << " py_stop = " << py_stop
      //           << endl;
      
      for (unsigned int px=px_start; px<=px_stop; px++)
      {
        texture_rectangle_ptr->get_pixel_RGB_values(
          px,py_start,curr_R,curr_G,curr_B);
        R.push_back(curr_R);
        G.push_back(curr_G);
        B.push_back(curr_B);
      }
    } // loop over index i labeling RLE runs

    double median_R,median_G,median_B;
    mathfunc::median_value_and_quartile_width(
      R,median_R,quartile_width_R);
    mathfunc::median_value_and_quartile_width(
      G,median_G,quartile_width_G);
    mathfunc::median_value_and_quartile_width(
      B,median_B,quartile_width_B);

    //         sigma_R=mathfunc::std_dev(R);
    //         sigma_G=mathfunc::std_dev(G);
    //         sigma_B=mathfunc::std_dev(B);
  }
    
  // ==========================================================================
  // Line segment detection methods
  // ==========================================================================

  // Method detect_line_segments() takes in a texture_rectangle which is
  // assumed to already contain an RGB image.  It runs the LSD
  // algorithm/codes by R.G. von Gioi (version 1.6 Nov 2011) which
  // locates line segments within the input image.  This method returns
  // an STL vector containing linesegments in UV image plane space
  // derived from the LSD code output.

  vector<linesegment> detect_line_segments(
    texture_rectangle* texture_rectangle_ptr)
  {
    //         cout << "inside videofunc::detect_line_segments()" << endl;

    texture_rectangle_ptr->convert_color_image_to_greyscale();

    int npx=texture_rectangle_ptr->getWidth();
    int npy=texture_rectangle_ptr->getHeight();
    double* image=new double[npx*npy*sizeof(double)];

    int R,G,B;
    for (int px=0; px<npx; px++)
    {
      for (int py=0; py<npy; py++)
      {
        texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
        image[px+py*npx]=R;
      }
    }

    // Call von Gioi's line segment detector:

    int n;
    double* out = lsd(&n,image,npx,npy);
    delete [] image;

    //         cout << "Number of detected line segments = " << n << endl;

    vector<linesegment> line_segments;
    twovector start_uv,stop_uv;
    for (int i=0; i<n; i++)
    {
      double start_pu=out[7*i+0];
      double start_pv=out[7*i+1];
      double stop_pu=out[7*i+2];
      double stop_pv=out[7*i+3];
      double width=out[7*i+4];
      //             double angle_precision=out[7*i+5];
      //            double log_nfa=-out[7*i+6];

      //      cout << "pu_0=" << start_pu
      //           << " pv_0=" << start_pv 
      //           << " pu_1=" << stop_pu
      //           << " pv_1=" << stop_pv << endl;

      double start_u=start_pu/npy;
      double start_v=start_pv/npy;
      double stop_u=stop_pu/npy;
      double stop_v=stop_pv/npy;
      start_v=1-start_v;
      stop_v=1-stop_v;
      width /= npy;

      threevector V1(start_u,start_v);
      threevector V2(stop_u,stop_v);
      linesegment curr_l(V1,V2);
      line_segments.push_back(curr_l);

    } // loop over index i labeling found line segments

    free( (void *) out );
    return line_segments;
  }
    
  // ==========================================================================
  // Texture rectangle drawing methods
  // ==========================================================================

  // Method draw_solid_squares() takes in a texture_rectangle along with
  // a set of square centers.  It recolors the pixels for each square
  // according to the specified color index.

  void draw_solid_squares(
    const std::vector<twovector>& square_centers,
    int square_length_in_pixels,
    texture_rectangle* texture_rectangle_ptr,int color_index)
  {
    //         cout << "inside videofunc::draw_solid_squares()" << endl;

    int width=texture_rectangle_ptr->getWidth();
    int height=texture_rectangle_ptr->getHeight();

    if (is_even(square_length_in_pixels))
      square_length_in_pixels++;
         
    int R=255;
    int G=0;
    int B=0;
    if (color_index >= 0)
    {
      colorfunc::Color curr_color=colorfunc::get_color(color_index);
      colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(curr_color);
      R=255*curr_RGB.first;
      G=255*curr_RGB.second;
      B=255*curr_RGB.third;
    }
         
    for (unsigned int s=0; s<square_centers.size(); s++)
    {
      twovector curr_center(square_centers[s]);
      unsigned int pu,pv;
      texture_rectangle_ptr->get_pixel_coords(
        curr_center.get(0),curr_center.get(1),pu,pv);

      int pu_start=pu-square_length_in_pixels/2;
      int pu_stop=pu+square_length_in_pixels/2;
      int pv_start=pv-square_length_in_pixels/2;
      int pv_stop=pv+square_length_in_pixels/2;
            
      pu_start=basic_math::max(pu_start,0);
      pu_stop=basic_math::min(pu_stop,width-1);
      pv_start=basic_math::max(pv_start,0);
      pv_stop=basic_math::min(pv_stop,height-1);
            
      for (int pu=pu_start; pu<=pu_stop; pu++)
      {
        for (int pv=pv_start; pv<=pv_stop; pv++)
        {
          texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
        } // loop over pv 
      } // loop over pu
    } // loop over index s labeling solid squares
  }
      
  // ---------------------------------------------------------------------      
  // Method draw_line_segments() takes in an STL vector of
  // linesegments in UV image plane space.  It also takes in a texture
  // rectangle which is assumed to have a clean copy of the image from
  // which the linesegments were derived.  This method colors the
  // linesegments according to input segment_color_index.

  void draw_line_segments(
    const vector<linesegment>& line_segments,
    texture_rectangle* texture_rectangle_ptr,int segment_color_index)
  {
    int npx=texture_rectangle_ptr->getWidth();
    int npy=texture_rectangle_ptr->getHeight();

    twoDarray* ztwoDarray_ptr=new twoDarray(npx,npy);
    ztwoDarray_ptr->initialize_values(-1);
         
    double min_u=0;
    double max_u=double(npx)/double(npy);
    double min_v=0;
    double max_v=1;
    ztwoDarray_ptr->init_coord_system(min_u,max_u,min_v,max_v);

    for (unsigned int l=0; l<line_segments.size(); l++)
    {
      linesegment curr_l(line_segments[l]);
      double curr_color_index=l%12;
      if (segment_color_index >= 0)
      {
        curr_color_index=segment_color_index;
      }
      drawfunc::draw_line(curr_l,curr_color_index,ztwoDarray_ptr);
    }
         
    for (int px=0; px<npx; px++)
    {
      for (int py=0; py<npy; py++)
      {
        int color_index=ztwoDarray_ptr->get(px,py);
               
        int R=255;
        int G=0;
        int B=255;

        if (color_index > -0.5)
        {
          colorfunc::Color curr_color=colorfunc::get_color(
            color_index);
          colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(
            curr_color);
                     
          R=255*curr_RGB.first;
          G=255*curr_RGB.second;
          B=255*curr_RGB.third;

          //                  const int dp=1;
          const int dp=2;
          for (int dpx=-dp; dpx <= dp; dpx++)
          {
            for (int dpy=-dp; dpy <= dp; dpy++)
            {
              texture_rectangle_ptr->set_pixel_RGB_values(
                px+dpx,py+dpy,R,G,B);
            } // loop over dpy index
          } // loop over dpx index

        } // color_index > -0.5 conditional
      } // loop over py index
    } // loop over px index
         
    delete ztwoDarray_ptr;
    //         texture_rectangle_ptr->write_curr_frame(output_filename);
  }

  // ---------------------------------------------------------------------      
  // Method display_circles() takes in a texture_rectangle which is
  // assumed to contain an image with at least one human face.  It also
  // takes in the UV coordinates for at least one circle which encloses
  // the face(s).  This method draws a purple circle around each face
  // and exports the annotated image to human_faces_image_filename.

  void display_circles(
    texture_rectangle* texture_rectangle_ptr,
    string human_faces_image_filename,
    const vector<twovector>& center,const vector<double>& radius)
  {
    //         cout << "inside videofunc::display_circles()" << endl;

    unsigned int width=texture_rectangle_ptr->getWidth();
    unsigned int height=texture_rectangle_ptr->getHeight();

    for (unsigned int c=0; c<center.size(); c++)
    {
      twovector curr_center(center[c]);
      double r=radius[c];
            
      double theta_start=0;
      double dtheta=1;	// deg
      int n_theta_bins=360;
      for (int t=0; t<n_theta_bins; t++)
      {
        double theta=theta_start+t*dtheta;
        theta *= PI/180;
        twovector currUV=curr_center+r*twovector(cos(theta),sin(theta));
        unsigned int pu,pv;
        texture_rectangle_ptr->get_pixel_coords(
          currUV.get(0),currUV.get(1),pu,pv);
        const int R=255;
        const int G=0;
        const int B=255;

        if (pu > 0 && pu < width && pv > 0 && pv < height)
        {
          texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
        }
        if (pu+1 > 0 && pu+1 < width && pv > 0 && pv < height)
        {
          texture_rectangle_ptr->set_pixel_RGB_values(pu+1,pv,R,G,B);
        }
        if (pu > 0 && pu < width && pv+1 > 0 && pv+1 < height)
        {
          texture_rectangle_ptr->set_pixel_RGB_values(pu,pv+1,R,G,B);
        }
        if (pu+1 > 0 && pu+1 < width && pv+1 > 0 && pv+1 < height)
        {
          texture_rectangle_ptr->set_pixel_RGB_values(
            pu+1,pv+1,R,G,B);
        }

      } // loop over index t labeling theta bins
    } // loop over index c labeling detection circles
    texture_rectangle_ptr->write_curr_frame(human_faces_image_filename);
  }

  // ---------------------------------------------------------------------
  // Method display_bboxes() takes in an STL vector of (Ulo,Uhi,Vlo,Vhi)
  // params.  It also takes in a texture rectangle which is assumed to
  // have a clean copy of the image from which the bboxes were derived.
  // This method colors bbox edges.

  void display_bboxes(
    const vector<bounding_box>& bboxes,
    texture_rectangle* texture_rectangle_ptr,
    int polygon_color_index,int line_thickness)
  {
    vector<fourvector> bbox_params;
    for (unsigned int b=0; b<bboxes.size(); b++)
    {
      bbox_params.push_back(
        fourvector(bboxes[b].get_xmin(),
                   bboxes[b].get_xmax(),
                   bboxes[b].get_ymin(),
                   bboxes[b].get_ymax()));
    }
    display_bboxes(bbox_params,texture_rectangle_ptr,polygon_color_index,
                   line_thickness);
  }
      
  void display_bboxes(
    const vector<fourvector>& bbox_params,
    texture_rectangle* texture_rectangle_ptr,
    int polygon_color_index,int line_thickness)
  {
    //         cout << "inside videofunc::display_bboxes()" << endl;
    //         cout << "texture_rectangle_ptr = " << texture_rectangle_ptr
    //              << endl;
    //         cout << "*texture_rectangle_ptr = " << *texture_rectangle_ptr
    //              << endl;
    //         cout << "texture_rectangle_ptr->get_video_filename() = "
    //              << texture_rectangle_ptr->get_video_filename() << endl;
         
    for (unsigned int c=0; c<bbox_params.size(); c++)
    {
      double Ulo=bbox_params[c].get(0);
      double Uhi=bbox_params[c].get(1);
      double Vlo=bbox_params[c].get(2);
      double Vhi=bbox_params[c].get(3);
            
      vector<twovector> bbox_vertices;
      bbox_vertices.push_back(twovector(Ulo,Vlo));
      bbox_vertices.push_back(twovector(Uhi,Vlo));
      bbox_vertices.push_back(twovector(Uhi,Vhi));
      bbox_vertices.push_back(twovector(Ulo,Vhi));
      polygon curr_polygon(bbox_vertices);
            
      double curr_color_index=1+c%6;
      if (polygon_color_index >= 0)
      {
        curr_color_index=polygon_color_index;
      }
      display_polygon(
        curr_polygon,texture_rectangle_ptr,curr_color_index,
        line_thickness);
    } // loop over index c
  }

  // ---------------------------------------------------------------------
  // Method display_polygon() takes in a polygon in UV image plane
  // space.  It also takes in a texture rectangle which is assumed to
  // have a clean copy of the image from which the polygons were
  // derived.  This method colors the polygon's edges within 
  // *texture_rectangle_ptr.

  void display_polygon(
    polygon& curr_polygon,texture_rectangle* texture_rectangle_ptr,
    int polygon_color_index,int line_thickness)
  {
    //         cout << "inside videofunc::display_polygon()" << endl;
    //         cout << "curr_polygon = " << curr_polygon << endl;
    //         cout << "texture_rectangle_ptr = "
    //              << texture_rectangle_ptr << endl;
    //         cout << "texture_rectangle_ptr->get_video_filename() = "
    //              << texture_rectangle_ptr->get_video_filename() << endl;

    int npx=texture_rectangle_ptr->getWidth();
    int npy=texture_rectangle_ptr->getHeight();
    //         cout << "npx = " << npx << " npy = " << npy << endl;

    twoDarray* ztwoDarray_ptr=new twoDarray(npx,npy);
    ztwoDarray_ptr->initialize_values(-1);
         
    double min_u=0;
    double max_u=double(npx)/double(npy);
    double min_v=0;
    double max_v=1;
    ztwoDarray_ptr->init_coord_system(min_u,max_u,min_v,max_v);

    curr_polygon.initialize_edge_segments();

    int n_edges=curr_polygon.get_nvertices();            
    for (int l=0; l<n_edges; l++)
    {
      linesegment curr_l=curr_polygon.get_edge(l);
      drawfunc::draw_line(curr_l,polygon_color_index,ztwoDarray_ptr);
    } // loop over index l labeling edges

    // Transfer colored pixels from *ztwoDarray_ptr to *texture_rectangle_ptr:
         
    for (int px=0; px<npx; px++)
    {
      for (int py=0; py<npy; py++)
      {
        int color_index=ztwoDarray_ptr->get(px,py);
               
        int R=255;
        int G=0;
        int B=255;

        if (color_index > -0.5)
        {
          colorfunc::Color curr_color=colorfunc::get_color(
            color_index);
          colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(
            curr_color);
                     
          R=255*curr_RGB.first;
          G=255*curr_RGB.second;
          B=255*curr_RGB.third;

          //                  cout << "line R = " << R << " G = " << G << " B = " << B
          //                       << endl;

          //                  const int dp=1;
          const int dp=line_thickness;
          for (int dpx=-dp; dpx <= dp; dpx++)
          {
            for (int dpy=-dp; dpy <= dp; dpy++)
            {
              texture_rectangle_ptr->set_pixel_RGB_values(
                px+dpx,py+dpy,R,G,B);
            } // loop over dpy index
          } // loop over dpx index

        } // color_index > -0.5 conditional
      } // loop over py index
    } // loop over px index

    delete ztwoDarray_ptr;
  }

  // ---------------------------------------------------------------------
  // Method display_polygons() takes in an STL vector of polygons
  // in UV image plane space.  It also takes in a texture rectangle
  // which is assumed to have a clean copy of the image from which the
  // polygons were derived.  This method colors polygon edges and
  // exports the annotated image to output_filename.

  void display_polygons(
    const vector<polygon>& polygons,
    texture_rectangle* texture_rectangle_ptr,
    int polygon_color_index,int line_thickness)
  {
    //         cout << "inside videofunc::display_polygons()" << endl;
    //         cout << "polygons.size() = " << polygons.size() << endl;
         
    for (unsigned int c=0; c<polygons.size(); c++)
    {
      polygon curr_polygon(polygons[c]);
      double curr_color_index=1+c%6;
      if (polygon_color_index >= 0)
      {
        curr_color_index=polygon_color_index;
      }
      display_polygon(
        curr_polygon,texture_rectangle_ptr,curr_color_index,
        line_thickness);
    } // loop over index c
    //         texture_rectangle_ptr->write_curr_frame(output_filename);
  }

  // ---------------------------------------------------------------------      
  // Method generate_ellipse_polygon() takes in 5 parameters
  // which define an ellipse.  It forms and returns a polygon
  // representing the ellipse within *texture_rectangle_ptr in UV
  // imageplane coordinates.

  polygon generate_ellipse_polygon(
    double a,double b,double c,double px_center,double py_center,
    texture_rectangle* texture_rectangle_ptr)
  {
    int n_points=36;
    vector<twovector> ellipse_pixels=geometry_func::ellipse_points(
      n_points,a,b,c,px_center,py_center);

    double prev_u=NEGATIVEINFINITY;
    double prev_v=NEGATIVEINFINITY;
    vector<twovector> vertices;
    for (unsigned int p=0; p<ellipse_pixels.size(); p++)
    {
      double curr_u,curr_v;
      texture_rectangle_ptr->get_uv_coords(
        ellipse_pixels[p].get(0),ellipse_pixels[p].get(1),
        curr_u,curr_v);

      if (nearly_equal(curr_u,prev_u) && nearly_equal(curr_v,prev_v))
      {
      }
      else
      {
        vertices.push_back(twovector(curr_u,curr_v));
      }
    } // loop over index p labeling ellipse pixels
          
    polygon curr_polygon(vertices);
    return curr_polygon;
  }

  // ---------------------------------------------------------------------      
  void draw_ellipse(
    double a,double b,double c,double px_center,double py_center,
    texture_rectangle* texture_rectangle_ptr,
    int color_index,int line_thickness)
  {
    //         cout << "inside videofunc::draw_ellipse()" << endl;

    polygon ellipse=generate_ellipse_polygon(
      a,b,c,px_center,py_center,texture_rectangle_ptr);
    videofunc::display_polygon(
      ellipse,texture_rectangle_ptr,color_index,line_thickness);
  }

  // ==========================================================================
  // Text annotation methods
  // ==========================================================================

  // Method annotate_image_with_text() takes in *texture_rectangle_ptr
  // which is assumed to already contain an image as well as
  // *text_texture_rectangle_ptr whose width and height must equal that
  // of *texture_rectangle_ptr.  Looping over all text entries within
  // STL vector text_lines, this method generates a png file with
  // colored text strings against a black background.  It subsequently
  // fills *text_texture_rectangle_ptr with the colored text strings
  // superposed against the original image.

  void annotate_image_with_text(
    texture_rectangle* texture_rectangle_ptr,
    texture_rectangle* text_texture_rectangle_ptr,
    int fontsize,
    vector<string>& text_lines,
    vector<twovector>& xy_start,
    vector<colorfunc::Color>& text_colors)
  {
    //         cout << "inside videofunc::annotate_image_with_text()" << endl;

    // First generate png image containing colored text strings against
    // black background:

    int width=texture_rectangle_ptr->getWidth();
    int height=texture_rectangle_ptr->getHeight();
    string text_png_filename="text.png";
    double background_greyscale_intensity=0;
    string font_path=sysfunc::get_projectsrootdir()+
      "data/OpenSceneGraph-Data/fonts/arial.ttf";

    vector<colorfunc::RGB> text_RGB;
    for (unsigned int i=0; i<text_colors.size(); i++)
    {
      colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(
        text_colors[i]);
            
      if (text_colors[i]==colorfunc::white)
      {
        curr_RGB=colorfunc::RGB(1,0.9,0.8);
      }
      else if (text_colors[i]==colorfunc::grey)
      {
        curr_RGB=colorfunc::RGB(0.5,0.6,0.4);
      }
      text_RGB.push_back(curr_RGB);
    }

    pngfunc::convert_textlines_to_PNG(
      width,height,text_png_filename,
      background_greyscale_intensity,font_path,fontsize,
      text_lines,xy_start,text_RGB);

    // Next convert png text image into jpg format:

    string text_jpg_filename="text.jpg";
    string unix_cmd="convert "+text_png_filename+" "+text_jpg_filename;
    sysfunc::unix_command(unix_cmd);
    filefunc::deletefile(text_png_filename);
    text_texture_rectangle_ptr->reset_texture_content(text_jpg_filename);
    filefunc::deletefile(text_jpg_filename);

    int R,G,B;
    double h,s,v;
    for (int py=0; py<height; py++)
    {
      for (int px=0; px<width; px++)
      {
        text_texture_rectangle_ptr->get_pixel_hsv_values(px,py,h,s,v);
        if (v < 0.5) 
        {
          texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
          text_texture_rectangle_ptr->set_pixel_RGB_values(
            px,py,R,G,B);
        }
      } // loop over px index
    } // loop over py index
  }

  // ==========================================================================
  // Cross correlation methods
  // ==========================================================================

  // Method image_pair_cross_correlation() takes in two texture
  // rectangles whose width and heights must be equal.  It computes and
  // returns their normalized cross correlation:

  // cross_correl=1/n_pixels sum_i,j { 
  // 	[I1(i,j)-<I1>] [I2(i,j)-<I2>] }/(sigma1*sigma2) }

  double image_pair_cross_correlation(
    texture_rectangle* texture_rectangle1_ptr,
    texture_rectangle* texture_rectangle2_ptr)
  {
    int width1=texture_rectangle1_ptr->getWidth();
    int height1=texture_rectangle1_ptr->getHeight();
    int width2=texture_rectangle2_ptr->getWidth();
    int height2=texture_rectangle2_ptr->getHeight();

    if ((width1 != width2) || (height1 != height2))
    {
      cout << "Trouble in videofunc::image_pair_cross_correlation()"
           << endl;
      cout << "width1 = " << width1 << " width2 = " << width2 << endl;
      cout << "height1 = " << height1 << " height2 = " << height2 << endl;
      return -1;
    }
    int n_pixels=width1*height1;

    vector<double> red1_values,green1_values,blue1_values;
    texture_rectangle1_ptr->get_all_RGB_values(
      red1_values,green1_values,blue1_values);

    double mu1_R,mu1_G,mu1_B,sigma1_R,sigma1_G,sigma1_B;
    mathfunc::mean_and_std_dev(red1_values,mu1_R,sigma1_R);
    mathfunc::mean_and_std_dev(green1_values,mu1_G,sigma1_G);
    mathfunc::mean_and_std_dev(blue1_values,mu1_B,sigma1_B);

    vector<double> red2_values,green2_values,blue2_values;
    texture_rectangle2_ptr->get_all_RGB_values(
      red2_values,green2_values,blue2_values);

    double mu2_R,mu2_G,mu2_B,sigma2_R,sigma2_G,sigma2_B;
    mathfunc::mean_and_std_dev(red2_values,mu2_R,sigma2_R);
    mathfunc::mean_and_std_dev(green2_values,mu2_G,sigma2_G);
    mathfunc::mean_and_std_dev(blue2_values,mu2_B,sigma2_B);

    double correl_R=0;
    double correl_G=0;
    double correl_B=0;
    for (int p=0; p<n_pixels; p++)
    {
      correl_R += (red1_values[p]-mu1_R)*(red2_values[p]-mu2_R);
      correl_G += (green1_values[p]-mu1_G)*(green2_values[p]-mu2_G);
      correl_B += (blue1_values[p]-mu1_B)*(blue2_values[p]-mu2_B);
    } // loop over index p labeling pixels
      
    correl_R /= (n_pixels*sigma1_R*sigma2_R);
    correl_G /= (n_pixels*sigma1_G*sigma2_G);
    correl_B /= (n_pixels*sigma1_B*sigma2_B);

    double avg_correl=(correl_R+correl_G+correl_B)/3.0;
    return avg_correl;
  }

  // ---------------------------------------------------------------------
  double image_pair_cross_correlation(
    const vector<double>& red1_values,
    const vector<double>& green1_values,
    const vector<double>& blue1_values,
    const vector<double>& red2_values,
    const vector<double>& green2_values,
    const vector<double>& blue2_values,
    double mu1_R,double mu1_G,double mu1_B,
    double sigma1_R,double sigma1_G,double sigma1_B,
    double mu2_R,double mu2_G,double mu2_B,
    double sigma2_R,double sigma2_G,double sigma2_B)
  {
    unsigned int n_pixels=red1_values.size();

    double correl_R=0;
    double correl_G=0;
    double correl_B=0;
    for (unsigned int p=0; p<n_pixels; p++)
    {
      correl_R += (red1_values[p]-mu1_R)*(red2_values[p]-mu2_R);
      correl_G += (green1_values[p]-mu1_G)*(green2_values[p]-mu2_G);
      correl_B += (blue1_values[p]-mu1_B)*(blue2_values[p]-mu2_B);
    } // loop over index p labeling pixels
      
    correl_R /= (n_pixels*sigma1_R*sigma2_R);
    correl_G /= (n_pixels*sigma1_G*sigma2_G);
    correl_B /= (n_pixels*sigma1_B*sigma2_B);

    double avg_correl=(correl_R+correl_G+correl_B)/3.0;
    return avg_correl;
  }

  // Method fill_YCbCr_arrays() imports a texture array and converts its
  // RGB content into YCbCr color space.  Mean values for Y, Cb and Cr
  // are calculated and removed from these color channels.  

  void fill_YCbCr_arrays(
    texture_rectangle* texture_rectangle_ptr, 
    double& mu_Y, double& mu_Cb, double& mu_Cr,
    flann::Matrix<float> *Y_matrix_ptr, 
    flann::Matrix<float> *Cb_matrix_ptr, flann::Matrix<float> *Cr_matrix_ptr)
  {
    int curr_R, curr_G, curr_B;
    double curr_Y, curr_Cb, curr_Cr;
    int width = texture_rectangle_ptr->getWidth();
    int height = texture_rectangle_ptr->getHeight();
    double mean_factor = 1.0 / double(width * height);

    mu_Y = 0, mu_Cb = 0, mu_Cr = 0;

    for (int py = 0 ; py < height; py++)
    {
      for (int px = 0; px < width; px++)
      {
        texture_rectangle_ptr->get_pixel_RGB_values(
          px, py, curr_R, curr_G, curr_B);

        // Convert from renormalized RGB to YCbCr color coordinates:

        colorfunc::RGB_to_YCbCr(
          curr_R, curr_G, curr_B, curr_Y, curr_Cb, curr_Cr);
        (*Y_matrix_ptr)[px][py]=curr_Y;
        (*Cb_matrix_ptr)[px][py]=curr_Cb;
        (*Cr_matrix_ptr)[px][py]=curr_Cr;

        mu_Y += mean_factor * curr_Y;
        mu_Cb += mean_factor * curr_Cb;
        mu_Cr += mean_factor * curr_Cr;

      } // loop over px
    } // loop over py

    for(int py = 0 ; py < height; py++)
    {
      for(int px = 0; px < width; px++)
      {
        (*Y_matrix_ptr)[px][py] -= mu_Y;
        (*Cb_matrix_ptr)[px][py] -= mu_Cb;
        (*Cr_matrix_ptr)[px][py] -= mu_Cr;
      }
    }
  }

  // ==========================================================================
  // Discrete cosine transform methods
  // As of 1/4/15, these slow methods are deprecated!  Use DCT and IDCT
  // methods within fourier_2D class which are orders of magnitude faster.
  // ==========================================================================

  void generate_cosine_array(int chip_width, int chip_height, 
                             vector<double>& cosine_array)
  {
    const int max_array_size = (2*chip_width+1) * chip_width;
    const double pi_factor = PI/(2*chip_height);

    cosine_array.reserve( max_array_size );
    for (int i=0; i<max_array_size; i++)
    {
      cosine_array.push_back(0);
    }

    for (int pu = 0; pu < chip_width; pu++)
    {
      for (int px = 0; px < chip_width; px++)
      {
        int curr_index = (2*px + 1) * pu;
        cosine_array[curr_index] = cos(curr_index * pi_factor);
      } // loop over px
    } // loop over pu
    cosine_array[0] = 1.0/sqrt(2.0);
  }

   
  // Method compute_DCT() calculates via brute-force the Discrete Cosine
  // Transform of the single-channel image contained within *matrix_ptr
  // and returns the result within *dct_matrix_ptr:

  void compute_DCT(int width, int height, const vector<double>& cosine_array,
                   const flann::Matrix<float> *matrix_ptr,
                   flann::Matrix<float> *dct_matrix_ptr)
  {
    double prefactor = 4.0/(width*height);
    double curr_dct;
    double x_factor, xy_factor;

    for (int pu = 0; pu < width; pu++)
    {
      for (int pv = 0; pv < height; pv++)
      {
        curr_dct = 0;

        for(int px = 0; px < width; px++)
        {
          x_factor = prefactor * cosine_array[(2*px + 1) * pu];

          for(int py = 0; py < height; py++)
          {
            xy_factor = x_factor * cosine_array[(2*py + 1) * pv];

            curr_dct += xy_factor * (*matrix_ptr)[px][py];
          } // loop over py
        } // loop over px

        (*dct_matrix_ptr)[pu][pv] = curr_dct;

      } // loop over pv
    } // loop over pu 
  }

  void compute_multi_DCT(
    int width, int height, const vector<double>& cosine_array,
    const flann::Matrix<float> *matrix1_ptr,
    const flann::Matrix<float> *matrix2_ptr,
    const flann::Matrix<float> *matrix3_ptr,
    flann::Matrix<float> *dct_matrix1_ptr,
    flann::Matrix<float> *dct_matrix2_ptr,
    flann::Matrix<float> *dct_matrix3_ptr)
  {
    double prefactor = 4.0/(width*height);
    double curr_dct1, curr_dct2, curr_dct3;
    double x_factor, xy_factor;

    for (int pu = 0; pu < width; pu++)
    {
      for (int pv = 0; pv < height; pv++)
      {
        curr_dct1 = curr_dct2 = curr_dct3 = 0;

        for(int px = 0; px < width; px++)
        {
          x_factor = prefactor * cosine_array[(2*px + 1) * pu];

          for(int py = 0; py < height; py++)
          {
            xy_factor = x_factor * cosine_array[(2*py + 1) * pv];

            curr_dct1 += xy_factor * (*matrix1_ptr)[px][py];
            curr_dct2 += xy_factor * (*matrix2_ptr)[px][py];
            curr_dct3 += xy_factor * (*matrix3_ptr)[px][py];
          } // loop over py
        } // loop over px

        (*dct_matrix1_ptr)[pu][pv] = curr_dct1;
        (*dct_matrix2_ptr)[pu][pv] = curr_dct2;
        (*dct_matrix3_ptr)[pu][pv] = curr_dct3;

      } // loop over pv
    } // loop over pu 
  }

  // Method compute_inverse_DCT() calculates via brute-force the inverse
  // Discrete Cosine Transform of the single-channel image contained
  // within *dct_matrix_ptr and returns the result within *dct_matrix_ptr:

  void compute_inverse_DCT(
    int width, int height, const vector<double>& cosine_array,
    double max_spatial_frequency,
    const flann::Matrix<float> *dct_matrix_ptr,
    flann::Matrix<float> *matrix_ptr)
  {
    double curr_intensity;
    double u_factor, uv_factor;

    for (int px = 0; px < width; px++)
    {
      for (int py = 0; py < height; py++)
      {
        curr_intensity = 0;

        for (int pu = 0; pu < width; pu++)
        {
          u_factor = cosine_array[(2*px + 1) * pu];

          for(int pv = 0; pv < height; pv++)
          {
            if (pu*pu+pv*pv > sqr(max_spatial_frequency)) continue;

            uv_factor = u_factor * cosine_array[(2*py + 1) * pv];

            curr_intensity += uv_factor * (*dct_matrix_ptr)[pu][pv];

          } // loop over pv index
        } // loop over pu index

        (*matrix_ptr)[px][py] = curr_intensity;

      } // loop over py index
    } // loop over px index
  }

  void compute_multi_inverse_DCT(
    int width, int height, const vector<double>& cosine_array,
    double max_spatial_frequency,
    const flann::Matrix<float> *dct_matrix2_ptr,
    const flann::Matrix<float> *dct_matrix3_ptr,
    flann::Matrix<float> *matrix2_ptr,
    flann::Matrix<float> *matrix3_ptr)
  {
    double curr_intensity2, curr_intensity3;
    double u_factor, uv_factor;

    for (int px = 0; px < width; px++)
    {
      for (int py = 0; py < height; py++)
      {
        curr_intensity2 = curr_intensity3 = 0;

        for (int pu = 0; pu < width; pu++)
        {
          u_factor = cosine_array[(2*px + 1) * pu];

          for(int pv = 0; pv < height; pv++)
          {
            if (pu*pu+pv*pv > sqr(max_spatial_frequency)) continue;

            uv_factor = u_factor * cosine_array[(2*py + 1) * pv];

            curr_intensity2 += uv_factor * (*dct_matrix2_ptr)[pu][pv];
            curr_intensity3 += uv_factor * (*dct_matrix3_ptr)[pu][pv];

          } // loop over pv index
        } // loop over pu index

        (*matrix2_ptr)[px][py] = curr_intensity2;
        (*matrix3_ptr)[px][py] = curr_intensity3;

      } // loop over py index
    } // loop over px index
  }

  // ---------------------------------------------------------------------
  // Method generate_byte_array() takes in RGB texture rectangle
  // mask_tr or ImageMagick mask IM_mask_ptr.  It loops over the
  // subimage defined by [px_start,px_stop] and [py_start,py_stop] and
  // writes the mask's red channel to an unsigned byte array.  The

  // Within this innner loop, py = 0 corresponds to the *bottom* row of
  // the full image.  Now 

  vector<vector<unsigned char> > generate_byte_array(
     const texture_rectangle* mask_tr_ptr, 
     const Magick::Image* IM_mask_ptr,
     unsigned int px_start, unsigned int px_stop,
     unsigned int py_start, unsigned int py_stop, 
     bool visualize_mask_flag)
  {
     vector<vector<unsigned char> > byte_array;
     vector<double> values;

     int mask_R, mask_G, mask_B;
     int img_height = 0;
     if(mask_tr_ptr != NULL)
     {
        img_height = mask_tr_ptr->getHeight();
     }

     for(unsigned int py = py_start; py <= py_stop; py++)
     {
        vector<unsigned char> curr_byte_row;
        for(unsigned int px = px_start; px <= px_stop; px++)
        {

// Set mask values for pixels lying outside borders of current image
// equal to background value 0:

           unsigned char curr_char = 0;

           if(mask_tr_ptr != NULL)
           {
              mask_tr_ptr->get_pixel_RGB_values(
                 px, img_height - 1 - py, mask_R, mask_G, mask_B);

// Recall get_pixel_RGB_values() returns a negative value if input
// pixel coordinates are invalid!

              if(visualize_mask_flag && mask_R > 0)
              {
                 curr_char = 255;
              }
              else if(mask_R >= 0)
              {
                 curr_char = mask_R;
              }
           }
           else
           {
              Magick::ColorRGB currRGB(IM_mask_ptr->pixelColor(px, py));
              if(visualize_mask_flag)
              {
                 curr_char = 255 * currRGB.red();
              }
              else
              {
                 curr_char = currRGB.red();
              }
           }
           curr_byte_row.push_back( curr_char );
        }
        byte_array.push_back(curr_byte_row);
     }

// Recall zeroth row fed into write_8bit_greyscale_pngfile() must
// correspond to TOP row of the image to be exported to output PNG
// file:

     if(mask_tr_ptr != NULL)
     {
        vector<vector<unsigned char > > flipped_byte_array;
        for(unsigned int py = 0; py < byte_array.size(); py++)
        {
//           vector<unsigned char> curr_byte_row = 
//              byte_array.at(byte_array.size() - 1 - py);
//           flipped_byte_array.push_back(curr_byte_row);
        flipped_byte_array.push_back(
           byte_array.at(byte_array.size() - 1 - py));
        }
        return flipped_byte_array;
     }
     else
     {
        return byte_array;
     }
  }

  // ---------------------------------------------------------------------
  vector<vector<unsigned char> > generate_byte_array(
     string png_filename,
     unsigned int px_start, unsigned int px_stop,
     unsigned int py_start, unsigned int py_stop, 
     bool visualize_mask_flag, int nontrivial_pixel_value)
  {
//     cout << "inside generate_byte_array #2" << endl;
//     cout << "visualize_mask_flag = " << visualize_mask_flag << endl;

     vector<vector<unsigned char> > byte_array;
     vector<double> values;

     texture_rectangle *mask_tr_ptr = new texture_rectangle(
        png_filename, NULL);
//     cout << "png_filename = " << png_filename << endl;
//     cout << "n_channels = " << mask_tr_ptr->getNchannels() << endl;

     int mask_R, mask_G, mask_B, mask_A;
     int img_height = mask_tr_ptr->getHeight();

     for(unsigned int py = py_start; py <= py_stop; py++)
     {
        vector<unsigned char> curr_byte_row;
        for(unsigned int px = px_start; px <= px_stop; px++)
        {

// Set mask values for pixels lying outside borders of current image
// equal to background value 0:

           unsigned char curr_char = 0;
           mask_tr_ptr->get_pixel_RGBA_values(
              px, img_height - 1 - py, mask_R, mask_G, mask_B, mask_A);

// Recall get_pixel_RGB_values() returns a negative value if input
// pixel coordinates are invalid!

           if(visualize_mask_flag && mask_R > 0 && mask_A > 0)
           {

// Display binary masks as white against black:

              curr_char = 255;

// Display multi-valued masks using 4-shades of grey:
              if(mask_R < 25)
              {
                 curr_char = 60 + 64 * (mask_R%4);
              }
           }
           else if(mask_R > 0 && mask_A > 0)
           {
              if(mask_R < 25)
              {
                 curr_char = mask_R;
              }
              else
              {
                 curr_char = nontrivial_pixel_value;
              }
           }
           curr_byte_row.push_back( curr_char );
        }
        byte_array.push_back(curr_byte_row);
     }

// Recall zeroth row fed into write_8bit_greyscale_pngfile() must
// correspond to TOP row of the image to be exported to output PNG
// file:

     delete mask_tr_ptr;
     vector<vector<unsigned char > > flipped_byte_array;
     for(unsigned int py = 0; py < byte_array.size(); py++)
     {
        flipped_byte_array.push_back(
           byte_array.at(byte_array.size() - 1 - py));
     }
     return flipped_byte_array;
  }

  // ==========================================================================
  // Focus measure methods
  // ==========================================================================

  // Method modified_laplacian() implements and RGB generalization of
  // eqn (13) within "Shape from Focus" by S.K. Nayar and Y. Nakagawa,
  // PAMI, Vol 16, Aug 1994, pg 824.

  double modified_laplacian(int px, int py, int pstep, 
                            texture_rectangle* tr_ptr)
  {
     int xdim = tr_ptr->getWidth();
     int ydim = tr_ptr->getHeight();
     double ml = 0;
     if(px >= pstep && px <= xdim - 1 - pstep &&
        py >= pstep && py < ydim - 1 -pstep)
     {
/*
        int Rcenter, Gcenter, Bcenter;
        int Rright, Gright, Bright;
        int Rleft, Gleft, Bleft;
        int Rup, Gup, Bup;
        int Rdown, Gdown, Bdown;
        
        tr_ptr->get_pixel_RGB_values(px, py, Rcenter, Gcenter, Bcenter);
        tr_ptr->get_pixel_RGB_values(px+pstep, py, Rright, Gright, Bright);
        tr_ptr->get_pixel_RGB_values(px-pstep, py, Rleft, Gleft, Bleft);
        tr_ptr->get_pixel_RGB_values(px, py-pstep, Rup, Gup, Bup);
        tr_ptr->get_pixel_RGB_values(px, py+pstep, Rdown, Gdown, Bdown);
        ml = fabs(2 * Rcenter - Rright - Rleft) +
           fabs(2 * Rcenter - Rup - Rdown) + 
           fabs(2 * Gcenter - Gright - Gleft) + 
           fabs(2 * Gcenter - Gup - Gdown) +
           fabs(2 * Bcenter - Bright - Bleft) + 
           fabs(2 * Bcenter - Bup - Bdown);
*/
        double hcenter, scenter, vcenter;
        double hright, sright, vright;
        double hleft, sleft, vleft;
        double hup, sup, vup;
        double hdown, sdown, vdown;
        
        tr_ptr->get_pixel_hsv_values(px, py, hcenter, scenter, vcenter);
        tr_ptr->get_pixel_hsv_values(px+pstep, py, hright, sright, vright);
        tr_ptr->get_pixel_hsv_values(px-pstep, py, hleft, sleft, vleft);
        tr_ptr->get_pixel_hsv_values(px, py-pstep, hup, sup, vup);
        tr_ptr->get_pixel_hsv_values(px, py+pstep, hdown, sdown, vdown);

        ml = fabs(2 * vcenter - vright - vleft) +
           fabs(2 * vcenter - vup - vdown);
     }
     return ml;
  }
  
  // ---------------------------------------------------------------------
  double sum_modified_laplacian(int px, int py, int pstep, 
                                texture_rectangle* tr_ptr)
  {
     double focus_measure = 0;

     int N = 1;
     for(int qy = py - N; qy <= py + N; qy++)
     {
        for (int qx = px - N; qx <= px + N; qx++)
        {
           focus_measure +=  modified_laplacian(qx, qy, pstep, tr_ptr);
        } // loop over px index
     } // loop over py index

//     cout << "px = " << px << " py = " << py 
//          << " focus_measure = " << focus_measure << endl;
     return focus_measure;
  }

  // ---------------------------------------------------------------------
  // Method avg_modified_laplacian() implements an RGB generalization
  // of the focus measure presented in "Shape from Focus" by
  // S.K. Nayar and Y. Nakagawa, PAMI, Vol 16, Aug 1994, pg 824.  
                                
  double avg_modified_laplacian(
     int px_start, int px_stop, int py_start, int py_stop,
     texture_rectangle* tr_ptr)
  {
//     int xdim = tr_ptr->getWidth();
//     int ydim = tr_ptr->getHeight();
     int pstep = 1;
     double avg_focus_measure = 0;
     double curr_local_focus_measure;
//     vector<double> local_focus_measures;

/*
     texture_rectangle* greyscale_tr_ptr = texture_rectangle_ptr(
        tr_ptr->getWidth(), tr_ptr->getHeight(), 1, tr_ptr->getNchannels(),
        NULL);
     greyscale_tr_ptr->copy_RGB_values(tr_ptr);
     greyscale_tr_ptr->convert_color_image_to_greyscale();
*/

     for(int py = py_start; py <= py_stop; py++)
     {
        for(int px = px_start; px <= px_stop; px++)
        {
           curr_local_focus_measure = 
              sum_modified_laplacian(px, py, pstep, tr_ptr);
           avg_focus_measure += curr_local_focus_measure;
//           local_focus_measures.push_back(sum_modified_laplacian(
//                                             px, py, pstep, tr_ptr));
        }
     }
//     double median_focus_measure = 
//        mathfunc::median_value(local_focus_measures);
//     return median_focus_measure;


//     delete greyscale_tr_ptr;
     

     avg_focus_measure /= ((px_stop-px_start+1)*(py_stop-py_start+1));
     return avg_focus_measure;
  }

  // ---------------------------------------------------------------------
  // Method RGB_variance()
  
  double RGB_variance(texture_rectangle* tr_ptr)
  {
     int xdim = tr_ptr->getWidth();
     int ydim = tr_ptr->getHeight();

     int R, G, B;
     double Rmean, Gmean, Bmean;
     Rmean = Gmean = Bmean = 0;
     for(int py = 0; py < ydim; py++)
     {
        for(int px = 0; px < xdim; px++)
        {
           tr_ptr->get_pixel_RGB_values(px, py, R, G, B);
           Rmean += R;
           Gmean += G;
           Bmean += B;
        } // loop over px
     } // loop over py 
     Rmean /= ( xdim * ydim);
     Gmean /= ( xdim * ydim);
     Bmean /= ( xdim * ydim);
     
     double Rvar, Gvar, Bvar;
     Rvar = Gvar = Bvar = 0;
     for(int py = 0; py < ydim; py++)
     {
        for(int px = 0; px < xdim; px++)
        {
           tr_ptr->get_pixel_RGB_values(px, py, R, G, B);
           Rvar += sqr(R - Rmean);
           Gvar += sqr(G - Gmean);
           Bvar += sqr(B - Bmean);
        } // loop over px
     } // loop over py 
     double avg_rgb_var = (Rvar + Gvar + Bvar) / (xdim * ydim);
     return avg_rgb_var;
  }
  
  // ==========================================================================
  // Image Magick methods
  // ==========================================================================

  // Method function import_IM_image()

  bool import_IM_image(
     string input_image_filename, Magick::Image& curr_image)
  {
    if (!filefunc::fileexist(input_image_filename)) return false;
    if (!imagefunc::valid_image_file(input_image_filename)) return false;

    // On 5/7/12, we discovered the painful way that flickr images whose
    // horizontal and vertical dimension metadata can be read may still
    // have corrupted pixel data.  So we include the following try-catch
    // exception handling to replace corrupted pixel data with blank pixel
    // data:

    try {
      // Try reading image file
      //      curr_image.read("\""+input_image_filename+"\"");
      curr_image.read(input_image_filename);
    }
    catch( Magick::ErrorBlob& error ) 
    { 
      // Process Magick++ file open error
      cout << "ErrorBlob: " << endl;
      cout << "Error message = " << error.what() << endl;
      return false;
    }
    catch( Magick::ErrorCoder& error ) 
    { 
      // Process Magick++ file open error
      cout << "ErrorCoder: " << endl;
      cout << "Error message = " << error.what() << endl;
      return false;
    }
    catch( Magick::ErrorCorruptImage& error ) 
    { 
      // Process Magick++ file open error
      cout << "ErrorCorruptImage: " << endl;
      cout << "Error message = " << error.what() << endl;
      return false;
    }
    return true;
  }

  // ---------------------------------------------------------------------
  void export_IM_image(
     string output_image_filename, Magick::Image& curr_image)
  {
     curr_image.write(output_image_filename);     
  }

  // ---------------------------------------------------------------------
  void resize_image(
    Magick::Image& curr_image,unsigned int new_xdim,unsigned int new_ydim)
  {
    //            cout << "inside videofunc::resize_image()" << endl;
    //            cout << "new_xdim = " << new_xdim << endl;
    //            cout << " new_ydim = " << new_ydim << endl;

    Magick::Geometry tNailSize(new_xdim,new_ydim);
    curr_image.zoom(tNailSize);
  }

  // ---------------------------------------------------------------------
  void gaussian_blur_image(Magick::Image& curr_image,double sigma)
  {
    curr_image.gaussianBlur(0,sigma);
  }

  // ---------------------------------------------------------------------
  // Method rotate_image() rotates the input image counter-clockwise
  // by angle theta measured in degrees.

  void rotate_image(Magick::Image& curr_image,double theta)
  {
    curr_image.rotate(theta);
  }

  // ---------------------------------------------------------------------
  // Method rotate_image() rotates the input image counter-clockwise
  // by angle theta measured in degrees.

  void crop_image(Magick::Image& curr_image, int width, int height, 
                  int xoffset, int yoffset)
  {
     curr_image.crop(Magick::Geometry(width, height, xoffset, yoffset));
  }

  // ---------------------------------------------------------------------
  // Method crop_rotate_image() rotates the input image
  // counter-clockwise by angle theta measured in degrees.  It then
  // crops the rotated image so that it has the same pixel size as the
  // original input image.

  void crop_rotate_image(Magick::Image& curr_image,double theta)
  {
     int xdim = curr_image.columns();
     int ydim = curr_image.rows();
     curr_image.rotate(theta);
     videofunc::crop_image(curr_image, xdim, ydim, 0, 0);
  }

  // ---------------------------------------------------------------------
  // Method get_pixel_RGB() uses ImageMagick to obtain the R, G, B
  // values for a specified pixel.  See
  // http://stackoverflow.com/questions/28151240/get-rgb-color-with-magick-using-c

  void get_pixel_RGB(Magick::Image& curr_image, 
                     int px, int py, int& R, int& G, int& B)
  {
     int width = curr_image.columns();
     int height = curr_image.rows();

// Calc what your range is. See
// http://www.imagemagick.org/Magick++/Color.html There's also other
// helpful macros, and definitions in ImageMagick's header files

     int range = pow(2, curr_image.modulusDepth());

// Get a "pixel cache" for the specified pixel:

     Magick::PixelPacket *pixels = curr_image.getPixels(px, py, 1, 1);
     Magick::Color color = pixels[0];
     R = color.redQuantum() / range;
     G = color.greenQuantum() / range;
     B = color.blueQuantum() / range;
  }

} // videofunc namespace


