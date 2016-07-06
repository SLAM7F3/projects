// ==========================================================================
// Descriptorfuncs namespace method definitions
// ==========================================================================
// Last modified on 3/24/14; 3/28/14; 5/10/14; 6/7/14
// ==========================================================================

#include <iostream>
#include <vector>


#include "datastructures/descriptor.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "video/descriptorfuncs.h"
#include "image/imagefuncs.h"
#include "templates/mytemplates.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

#include "math/prob_distribution.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

namespace descriptorfunc
{

// ==========================================================================
// GIST descriptor methods
// ==========================================================================

// Method compute_gist_descriptor() takes an input image and first
// rescales/crops it down to output_width x output_height (256x256).
// It next converts the resized image to PPM format.  The Lear GIST
// descriptor is calculated and exported to output text file
// gist_filename.
   
   bool compute_gist_descriptor(string image_filename,string gist_filename)
   {
//      cout << "inside descriptorfunc::compute_gist_descriptor()" << endl;
//      cout << "image_filename = " << image_filename << endl;
      unsigned int input_width,input_height;
      imagefunc::get_image_width_height(
         image_filename,input_width,input_height);
//      cout << "  input_width = " << input_width 
//           << " input_height = " << input_height << endl;
   
      unsigned int output_width=256;
      unsigned int output_height=256;

      string downsized_filename="downsized.jpg";
      string unix_cmd="convert "+image_filename+
         " -resize "+stringfunc::number_to_string(output_width)+"x"+
         stringfunc::number_to_string(output_height)+"^ "+
         downsized_filename;
      sysfunc::unix_command(unix_cmd);

      unsigned int downsized_width,downsized_height;
      imagefunc::get_image_width_height(
         downsized_filename,downsized_width,downsized_height);
//      cout << "  downsized_width = " << downsized_width
//           << " downsized_height = " << downsized_height << endl;

      int delta_width=downsized_width-output_width;
      int delta_height=downsized_height-output_height;
//      cout << "  delta_width = " << delta_width
//           << " delta_height = " << delta_height << endl;

      string output_filename="output.jpg";
      if (delta_width==0 && delta_height==0)
      {
         unix_cmd="mv "+downsized_filename+" "+output_filename;
         sysfunc::unix_command(unix_cmd);
      }
      else if (fabs(delta_width) > fabs(delta_height))
      {
         // Crop excess horizontal pixels

         int xoffset=delta_width/2;
         int yoffset=0;
         imagefunc::extract_subimage(
            downsized_filename,output_filename,
            output_width,output_height,xoffset,yoffset);
      }
      else
      {
         // Crop excess vertical pixels
         int xoffset=0;
         int yoffset=delta_height/2;
         imagefunc::extract_subimage(
            downsized_filename,output_filename,
            output_width,output_height,xoffset,yoffset);
      }

      imagefunc::get_image_width_height(
         output_filename,output_width,output_height);
//      cout << "Exported "+output_filename << endl;
//      cout << "output width = " << output_width
//           << " output height = " << output_height << endl;

// Convert resized image to PPM format 

      string prefix=stringfunc::prefix(output_filename);
      string suffix=stringfunc::suffix(output_filename);
      string ppm_filename=prefix+".ppm";
      string pgm_filename=prefix+".pgm";

// In August 2013, we empirically found that ImageMagick's CONVERT
// utility fails a non-negligible number of times to generate a PPM
// file which LEAR GIST can import.  So we use the jpegtopnm utility
// instead to convert JPEG to PPM files:

//      unix_cmd="convert "+output_filename+" "+ppm_filename;
      unix_cmd="jpegtopnm "+output_filename+" > "+ppm_filename;

//      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

// Compute Lear GIST descriptor for resized PPM image:

//         string gist_filename=gist_subdir+image_filename_prefix+".gist";
      unix_cmd="lear_gist "+ppm_filename+" > "+gist_filename;
//      unix_cmd="lear_gist "+pgm_filename+" > "+gist_filename;
//      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

// For reasons we don't understand as of 3/31/13, LEAR gist sometimes
// fails to generate descriptors for seemingly reasonable .ppm files.
// So delete any small-sized gist output files:

      long long filesize=filefunc::size_of_file_in_bytes(gist_filename);
      if (filesize < 10)
      {
         filefunc::deletefile(gist_filename);
         return false;
      }
      else
      {
         string banner="Exported "+gist_filename;
         outputfunc::write_banner(banner);
         return true;
      }
   }

// ------------------------------------------------------------------------
// Method GIST_descriptor_matrix() reads in a set of text files
// containing GIST descriptors.  It instantiates and returns a
// genmatrix whose rows hold GIST descriptors.  If a descriptor's
// magnitude is nonzero, it is renormalized to unity.

   genmatrix* GIST_descriptor_matrix(
      const std::vector<std::string>& gist_filenames,
      int n_descriptors)
   {
      cout << "Filling GIST descriptor matrix:" << endl;

      if (n_descriptors < 0) n_descriptors=gist_filenames.size();
      int n_fields=3*512;

      genmatrix* GIST_matrix_ptr=new genmatrix(n_descriptors,n_fields);
      for (int i=0; i<n_descriptors; i++)
      {
         outputfunc::update_progress_fraction(i,100,n_descriptors);
   
//         cout << "i = " << i 
//              << " gist_filename = " << gist_filenames[i] << endl;

         vector<double> column_values=filefunc::ReadInNumbers(
            gist_filenames[i]);

// Renormalize non-zero valued GIST descriptors so that they have unit
// magnitudes:

         double denom=0;
         for (unsigned int c=0; c<column_values.size(); c++)
         {
            denom += sqr(column_values[c]);
         }
         denom=sqrt(denom);

         bool zero_descriptor_flag=false;
         if (fabs(denom) < 1E-7) zero_descriptor_flag=true;

         double sum=0;
         for (unsigned int c=0; c<column_values.size(); c++)
         {
            double curr_component=0;
            if (!zero_descriptor_flag)
            {
               curr_component=column_values[c]/denom;
            }
            sum += sqr(curr_component);
            GIST_matrix_ptr->put(i,c,curr_component);
         }
      }
      cout << endl;

      return GIST_matrix_ptr;
   }

// ------------------------------------------------------------------------
// This overloaded version of GIST_descriptor_matrix() takes in a
// subdirectory which is assumed to contain a set of GIST text files
// corresponding to the images within input STL vector
// image_filenames.  It instantiates and returns *GIST_matrix_ptr
// whose rows are filled with GIST descriptors.
   
   genmatrix* GIST_descriptor_matrix(
      string gist_subdir,const vector<string>& image_filenames)
   {
      cout << "Filling GIST descriptor matrix:" << endl;
      int n_descriptors=image_filenames.size();
      int n_fields=3*512;
      genmatrix* GIST_matrix_ptr=new genmatrix(n_descriptors,n_fields);

      for (int i=0; i<n_descriptors; i++)
      {
         outputfunc::update_progress_fraction(i,100,n_descriptors);
   
         string curr_image_filename=image_filenames[i];
         string basename=filefunc::getbasename(curr_image_filename);
         string prefix=stringfunc::prefix(basename);
         string gist_filename=gist_subdir+prefix+".gist";

         vector<double> column_values;
         if (filefunc::fileexist(gist_filename))
         {
            filefunc::ReadInfile(gist_filename);
            for (unsigned int l=0; l<filefunc::text_line.size(); l++)
            {
               column_values=stringfunc::string_to_numbers(
                  filefunc::text_line[l]);
            }
         }
         else
         {
            cout << "GIST descriptor not found for image " << i << endl;
            outputfunc::enter_continue_char();
            for (int c=0; c<n_fields; c++)
            {
               column_values.push_back(-999);
            }
         }

         for (unsigned int c=0; c<column_values.size(); c++)
         {
            GIST_matrix_ptr->put(i,c,column_values[c]);
         }
      }

      return GIST_matrix_ptr;
   }

// ==========================================================================
// Color histogram methods 
// ==========================================================================

// Method compute_color_histogram() takes an input image and computes
// its color histogram.  It exports the results to the output text
// file specified by input argument color_histogram_filename.  The
// histogram is returned by this method.
   
   vector<double> compute_color_histogram(
      string image_filename,string color_histogram_filename,
      texture_rectangle* texture_rectangle_ptr,RGB_analyzer* RGB_analyzer_ptr)
   {
      vector<double> color_histogram;
      if (!texture_rectangle_ptr->reset_texture_content(image_filename))
      {
         return color_histogram;
      }

      color_histogram=RGB_analyzer_ptr->compute_color_histogram(
         texture_rectangle_ptr);

      ofstream colorstream;
      filefunc::openfile(color_histogram_filename,colorstream);

      for (unsigned int c=0; c<color_histogram.size(); c++)
      {
         colorstream << color_histogram[c] << "  ";
      }
      colorstream << endl;
      filefunc::closefile(color_histogram_filename,colorstream);         

//      string banner="Exported "+color_histogram_filename;
//      outputfunc::write_banner(banner);

      return color_histogram;
   }

/*
// ------------------------------------------------------------------------
// Method compute_sector_color_histogram() takes in integer indices
// row_ID and column_ID which must range over the interval [0,3].  It
// computes and returns the color histogram within the 1/16th sector
// of the input image specified by these input indices.

   vector<double> compute_sector_color_histogram(
      int row_ID,int column_ID,string image_filename,
      texture_rectangle* texture_rectangle_ptr,RGB_analyzer* RGB_analyzer_ptr)
   {
//      cout << "inside descriptorfunc::compute_sector_color_histogram()" << endl;

      vector<double> color_histogram;
      if (!texture_rectangle_ptr->reset_texture_content(image_filename))
      {
         return color_histogram;
      }

      int width=texture_rectangle_ptr->getWidth();
      int height=texture_rectangle_ptr->getHeight();
//      cout << "width = " << width << " height = " << height << endl;

      if (row_ID >= 4 || column_ID >= 4)
      {
         cout << "Error in descriptorfunc::compute_sector_color_histogram()" << endl;
         cout << "row_ID = " << row_ID << " and column_ID = " << column_ID
              << " must equal 0, 1, 2 or 3" << endl;
         exit(-1);
      }

      int left_pu=0.25*row_ID*width;
      int top_pv=0.25*column_ID*height;
      int right_pu=basic_math::min(width-1,int(left_pu+0.25*width));
      int bottom_pv=basic_math::min(height-1,int(top_pv+0.25*height));

//      cout << "left_pu = " << left_pu 
//           << " right_pu = " << right_pu << endl;
//      cout << "top_pv = " << top_pv << " bottom_pv = " << bottom_pv
//           << endl;

      string liberalized_color="";
      color_histogram=RGB_analyzer_ptr->compute_bbox_color_content(
         left_pu,top_pv,right_pu,bottom_pv,
         texture_rectangle_ptr,liberalized_color);
      return color_histogram;
   }
*/

// ------------------------------------------------------------------------
// Method compute_sector_color_histogram() takes in integer indices
// row_ID and column_ID which must range over the interval [0,3].  It
// computes and returns the color histogram within the 1/16th sector
// of the input image specified by these input indices.

   vector<double> compute_sector_color_histogram(
      int n_rows,int n_columns,int row_ID,int column_ID,string image_filename,
      texture_rectangle* texture_rectangle_ptr,RGB_analyzer* RGB_analyzer_ptr)
   {
//      cout << "inside descriptorfunc::compute_sector_color_histogram()" << endl;

      vector<double> color_histogram;
      if (!texture_rectangle_ptr->reset_texture_content(image_filename))
      {
         return color_histogram;
      }

      int width=texture_rectangle_ptr->getWidth();
      int height=texture_rectangle_ptr->getHeight();
//      cout << "width = " << width << " height = " << height << endl;

      if (row_ID >= n_rows || column_ID >= n_columns)
      {
         cout << "Error in descriptorfunc::compute_sector_color_histogram()" << endl;
         cout << "row_ID = " << row_ID << " >= n_rows = " << n_rows << endl;
         cout << "column_ID = " << column_ID << " >= n_columns = " 
              << n_columns << endl;
         exit(-1);
      }

      double row_frac=1.0/double(n_rows);
      double column_frac=1.0/double(n_columns);

      int left_pu=column_frac*row_ID*width;
      int top_pv=row_frac*column_ID*height;
      int right_pu=basic_math::min(width-1,int(left_pu+column_frac*width));
      int bottom_pv=basic_math::min(height-1,int(top_pv+row_frac*height));

//      cout << "left_pu = " << left_pu 
//           << " right_pu = " << right_pu << endl;
//      cout << "top_pv = " << top_pv << " bottom_pv = " << bottom_pv
//           << endl;

      color_histogram=RGB_analyzer_ptr->compute_bbox_color_content(
         left_pu,top_pv,right_pu,bottom_pv,texture_rectangle_ptr);
      return color_histogram;
   }

// ==========================================================================
// Edge histogram descriptor methods
// ==========================================================================

   vector<double> compute_edge_histogram(
      string image_filename,texture_rectangle* texture_rectangle_ptr)
   {
      cout << "inside descriptorfunc::compute_edge_histogram()" << endl;
      vector<double> edge_histogram;
      if (!texture_rectangle_ptr->reset_texture_content(image_filename))
      {
         return edge_histogram;
      }

// Convert color RGB values into luminosity greyscale values:

      texture_rectangle_ptr->convert_color_image_to_luminosity();
      
// Set edge filter values:

      vector<double> fv,fh,f45,f135,fnd;

// Vertical:

      fv.push_back(1);
      fv.push_back(-1);
      fv.push_back(1);
      fv.push_back(-1);

// Horizontal:

      fh.push_back(1);
      fh.push_back(1);
      fh.push_back(-1);
      fh.push_back(-1);

// 45 degree diagonal

      f45.push_back(sqrt(2));
      f45.push_back(0);
      f45.push_back(0);
      f45.push_back(-sqrt(2));

// 135 degree diagonal

      f135.push_back(0);
      f135.push_back(sqrt(2));
      f135.push_back(-sqrt(2));
      f135.push_back(0);

// Non-directional

      fnd.push_back(2);
      fnd.push_back(-2);
      fnd.push_back(-2);
      fnd.push_back(2);

      int width=texture_rectangle_ptr->getWidth();
      int height=texture_rectangle_ptr->getHeight();
      int n_image_blocks=1100;	// Total number of image blocks
//      int n_image_blocks=4096;	// Total number of image blocks
//      int n_image_blocks=16384;	// Total number of image blocks

// image_block_size = size in pixels of square image blocks.
// image_block_size must be even.

      int image_block_size=
         basic_math::mytruncate(sqrt(width*height/double(n_image_blocks)));
      if (is_odd(image_block_size)) image_block_size--;

// Try to fill original width*height image with n_horiz_blocks x
// n_vert_blocks square image blocks where n_horiz_blocks and
// n_vert_blocks are multiples of 4:

      int n_horiz_blocks=width/image_block_size;
      int n_vert_blocks=height/image_block_size;
      n_horiz_blocks=(n_horiz_blocks/4)*4;
      n_vert_blocks=(n_vert_blocks/4)*4;
      cout << "n_horiz_blocks = " << n_horiz_blocks << endl;
      cout << "n_vert_blocks = " << n_vert_blocks << endl;

      int n_subimage_horiz_blocks=n_horiz_blocks/4;
      int n_subimage_vert_blocks=n_vert_blocks/4;
      int n_image_blocks_per_subimage=n_subimage_horiz_blocks*
         n_subimage_vert_blocks;

// Size in pixels of each subimage:
      
      int subimage_width=n_subimage_horiz_blocks*image_block_size;
      int subimage_height=n_subimage_vert_blocks*image_block_size;

      bool monotone_flag;
      const double Tedge=11;

      vector<double> subimage_edge_histogram;
      for (int index=0; index<16; index++)
      {
         int row=index/4;
         int column=index%4;
         cout << "index = " << index << " row = " << row
              << " column = " << column << endl;
         int subimage_pu_start=row*subimage_width;
//          int subimage_pu_stop=(row+1)*subimage_width-1;
         int subimage_pv_start=column*subimage_height;
//          int subimage_pv_stop=(column+1)*subimage_height-1;
         
         subimage_edge_histogram.clear();
         for (int s=0; s<5; s++)
         {
            subimage_edge_histogram.push_back(0);
         }
         
         for (int i=0; i<n_subimage_horiz_blocks; i++)
         {
            int pu_start=subimage_pu_start+i*image_block_size;
            int pu_stop=pu_start+subimage_width-1;
            for (int j=0; j<n_subimage_vert_blocks; j++)
            {
               int pv_start=subimage_pv_start+j*image_block_size;
               int pv_stop=pv_start+subimage_height-1;
               
               pair<double,int> P=filter_image_block(
                  monotone_flag,texture_rectangle_ptr,
                  fv,fh,f45,f135,fnd,
                  pu_start,pu_stop,pv_start,pv_stop);

//               cout << "monotone_flag = " << monotone_flag << endl;
               if (monotone_flag) continue;
               double m=P.first;
               int edge_ID=P.second;
               if (m < Tedge)
               {
                  edge_ID=4;	// non-directional
               }
               cout << "m = " << m << " edge_ID = " << edge_ID << endl;

               subimage_edge_histogram[edge_ID]=
                  subimage_edge_histogram[edge_ID]+
                  1.0/n_image_blocks_per_subimage;
            } // loop over index j labeling subimage vert blocks
         } // loop over index i labeling subimage horiz blocks

         for (int s=0; s<5; s++)
         {
            edge_histogram.push_back(subimage_edge_histogram[s]);
//            cout << "s = " << s << " edge_histogram[s] = " 
//                 << edge_histogram[s] << endl;
         }

         outputfunc::enter_continue_char();

      } // loop over index labeing 4x4 subimage regions

      return edge_histogram;
   }

// ------------------------------------------------------------------------
   pair<double,int> filter_image_block(
      bool& monotone_flag,texture_rectangle* texture_rectangle_ptr,
      const vector<double>& fv,const vector<double>& fh,
      const vector<double>& f45,const vector<double>& f135,
      const vector<double>& fnd,
      int pu_start,int pu_stop,int pv_start,int pv_stop)
   {
      cout << "inside descriptorfunc::filter_image_block()" << endl;
      
      pair<double,int> P;

// First assess whether current image block is "monotone":

      int R,G,B;
      vector<double> luminosities;
      for (int pu=pu_start; pu<=pu_stop; pu++)
      {
         for (int pv=pv_start; pv<=pv_stop; pv++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
            luminosities.push_back(R);
         }
      }

      const double monotone_threshold=10;
      double sigma=mathfunc::std_dev(luminosities);
//      cout << "sigma = " << sigma << endl;
      if (sigma < monotone_threshold)
      {
         monotone_flag=true;
         return P;
      }
      else
      {
         monotone_flag=false;
      }
      

// Compute average intensities within 4 sub-blocks of current image
// block:

      double a0,a1,a2,a3;
      a0=a1=a2=a3=0;
      vector<double> a;

      int pu_mid=0.5*(pu_start+pu_stop);
      int pv_mid=0.5*(pv_start+pv_stop);

      int pixel_counter=0;
      for (int pu=pu_start; pu<= pu_mid; pu++)
      {
         for (int pv=pv_start; pv<= pv_mid; pv++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
            a0 += R;
            pixel_counter++;
         }
      }
      a.push_back(a0/pixel_counter);

      pixel_counter=0;
      for (int pu=pu_mid+1; pu<= pu_stop; pu++)
      {
         for (int pv=pv_start; pv<= pv_mid; pv++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
            a1 += R;
            pixel_counter++;
         }
      }
      a.push_back(a1/pixel_counter);

      pixel_counter=0;
      for (int pu=pu_start; pu<= pu_mid; pu++)
      {
         for (int pv=pv_mid+1; pv<= pv_stop; pv++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
            a2 += R;
            pixel_counter++;
         }
      }
      a.push_back(a2/pixel_counter);

      pixel_counter=0;
      for (int pu=pu_mid+1; pu<= pu_stop; pu++)
      {
         for (int pv=pv_mid+1; pv<= pv_stop; pv++)
         {
            texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
            a3 += R;
            pixel_counter++;
         }
      }
      a.push_back(a3/pixel_counter);

      double mv,mh,m45,m135,mnd;
      mv=mh=m45=m135=mnd=0;
      for (int k=0; k<4; k++)
      {
         mv += a[k]*fv[k];
         mh += a[k]*fh[k];
         m45 += a[k]*f45[k];
         m135 += a[k]*f135[k];
         mnd += a[k]*fnd[k];
//         cout << "k = " << k << " a[k] = " << a[k] << endl;
      }
      cout << "mv = " << mv << endl;
      cout << "mh = " << mh << endl;
      cout << "m45 = " << m45 << endl;
      cout << "m135 = " << m135 << endl;

      mv=fabs(mv);
      mh=fabs(mh);
      m45=fabs(m45);
      m135=fabs(m135);
      mnd=fabs(mnd);
      
      vector<double> m;
      m.push_back(mv);
      m.push_back(mh);
      m.push_back(m45);
      m.push_back(m135);
//      m.push_back(mnd);

      vector<int> edge_ID;
      edge_ID.push_back(0);
      edge_ID.push_back(1);
      edge_ID.push_back(2);
      edge_ID.push_back(3);
//      edge_ID.push_back(4);
      
      templatefunc::Quicksort_descending(m,edge_ID);

      P.first=m.front();
      P.second=edge_ID.front();
      
      cout << "max m = " << P.first 
           << " edgeID = " << P.second << endl;

      return P;
   }
   
// ==========================================================================
// Line segments descriptor methods
// ==========================================================================

// Method compute_linesegments_histogram() first extracts line
// segments from the specified input image file via the LSD
// algorithm/codes by R.G. von Gioi (Nov 2011).  Each line segment is
// assigned an angle theta ranging over [0,180 degs], a fractional
// length with respect to the image's main diagonal, and quantized
// sub-image position indices 0 <= i,j <= 3.  The theta values are
// quantized into ten 18-degree bins, while the fractional length
// values are quantized into 8 logarithmic bins.  This method returns
// a 4x4x8x10=1280 dimensional histogram which characterizes the input
// image's line segment content.
   
   vector<double> compute_linesegments_histogram(string image_filename)
   {
      vector<double> linesegments_histogram;
         
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
      if (!texture_rectangle_ptr->reset_texture_content(image_filename))
      {
         return linesegments_histogram;
      }

      double Umax=texture_rectangle_ptr->get_maxU();
      int n_subimage_bins=4;
      double subimage_dU=Umax/n_subimage_bins;
      double subimage_dV=1.0/n_subimage_bins;

      double Diag=sqrt(1+Umax*Umax);
//      cout << "Diag = " << Diag << endl;

      vector<linesegment> linesegments=videofunc::detect_line_segments(
         texture_rectangle_ptr);

      int n_frac_mag_bins=8;
      int n_theta_bins=10;
      double dtheta=PI/n_theta_bins;

      int n_bins=n_subimage_bins*n_subimage_bins*n_frac_mag_bins*n_theta_bins;
      linesegments_histogram.reserve(n_bins);
      for (int b=0; b<n_bins; b++)
      {
         linesegments_histogram.push_back(0);
      }

      for (unsigned int e=0; e<linesegments.size(); e++)
      {
         linesegment curr_segment(linesegments[e]);
         threevector V1=curr_segment.get_v1();
         threevector V2=curr_segment.get_v2();
         threevector COM=0.5*(V1+V2);
         double U_COM=COM.get(0);
         double V_COM=COM.get(1);
      
         int i=basic_math::mytruncate(U_COM/subimage_dU);
         int j=basic_math::mytruncate(V_COM/subimage_dV);
      
         double m=(V2.get(1)-V1.get(1))/(V2.get(0)-V1.get(0));
         double curr_theta=atan2(1,m);
         curr_theta=basic_math::phase_to_canonical_interval(
            curr_theta,0,0.99999*PI);
         int theta_bin=curr_theta/dtheta;

         double curr_mag=(V2-V1).magnitude();
         double curr_frac_mag=curr_mag/Diag;
         double log2_frac_mag=log(curr_frac_mag)/log(2.0);
         int log_bin=basic_math::mytruncate(log2_frac_mag);
         int frac_mag_bin=n_frac_mag_bins+basic_math::max(
            log_bin,-n_frac_mag_bins);

         int bin=
            i*n_subimage_bins*n_frac_mag_bins*n_theta_bins+
            j*n_frac_mag_bins*n_theta_bins+
            frac_mag_bin*n_theta_bins+
            theta_bin;
         linesegments_histogram[bin]=linesegments_histogram[bin]+1;

//         cout << "bin = " << bin
//              << " i = " << i << " j = " <<  j
//              << " theta = " << curr_theta*180/PI 
//              << " theta_bin = " << theta_bin 
//              << " frac_mag = " << curr_frac_mag 
//              << " log2_frac_mag = " << log2_frac_mag 
//              << " log_bin = " << log_bin
//              << " frac_mag_bin = " << frac_mag_bin
//              << endl;
      }

      delete texture_rectangle_ptr;
      return linesegments_histogram;
   }
   
// ------------------------------------------------------------------------
// This overloaded version of compute_linesegments_histogram() takes
// n_frac_mag_bins and n_theta_bins as input parameters.  It generates
// and returns a TWO dimensional histogram which bins line segments by
// their angle and fractional distance but NOT by their center
// position within the image plane.
   
   vector<double> compute_linesegments_histogram(
      int n_frac_mag_bins,int n_theta_bins,string image_filename)
   {
      vector<double> linesegments_histogram;
         
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
      if (!texture_rectangle_ptr->reset_texture_content(image_filename))
      {
         return linesegments_histogram;
      }

      double Umax=texture_rectangle_ptr->get_maxU();
      double Diag=sqrt(1+Umax*Umax);
//      cout << "Diag = " << Diag << endl;

      vector<linesegment> linesegments=videofunc::detect_line_segments(
         texture_rectangle_ptr);

      double dtheta=PI/n_theta_bins;

      int n_bins=n_frac_mag_bins*n_theta_bins;
      linesegments_histogram.reserve(n_bins);
      for (int b=0; b<n_bins; b++)
      {
         linesegments_histogram.push_back(0);
      }

      for (unsigned int e=0; e<linesegments.size(); e++)
      {
         linesegment curr_segment(linesegments[e]);
         threevector V1=curr_segment.get_v1();
         threevector V2=curr_segment.get_v2();
         threevector COM=0.5*(V1+V2);
      
         double m=(V2.get(1)-V1.get(1))/(V2.get(0)-V1.get(0));
         double curr_theta=atan2(1,m);
         curr_theta=basic_math::phase_to_canonical_interval(
            curr_theta,0,0.99999*PI);
         int theta_bin=curr_theta/dtheta;

         double curr_mag=(V2-V1).magnitude();
         double curr_frac_mag=curr_mag/Diag;
         double log2_frac_mag=log(curr_frac_mag)/log(2.0);
         int log_bin=basic_math::mytruncate(log2_frac_mag);
         int frac_mag_bin=n_frac_mag_bins+
            basic_math::max(log_bin,-n_frac_mag_bins);

         int bin=
            frac_mag_bin*n_theta_bins+theta_bin;
         linesegments_histogram[bin]=linesegments_histogram[bin]+1;

//         cout << "bin = " << bin
//              << " i = " << i << " j = " <<  j
//              << " theta = " << curr_theta*180/PI 
//              << " theta_bin = " << theta_bin 
//              << " frac_mag = " << curr_frac_mag 
//              << " log2_frac_mag = " << log2_frac_mag 
//              << " log_bin = " << log_bin
//              << " frac_mag_bin = " << frac_mag_bin
//              << endl;
      }

      delete texture_rectangle_ptr;
      return linesegments_histogram;
   }

// ------------------------------------------------------------------------
   
   vector<double> compute_image_segments_histogram(
      int n_horiz_subimage_bins,int n_vert_subimage_bins,
      int n_frac_mag_bins,int n_theta_bins,string image_filename)
   {
      vector<double> linesegments_histogram;
         
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
      if (!texture_rectangle_ptr->reset_texture_content(image_filename))
      {
         return linesegments_histogram;
      }

      double Umax=texture_rectangle_ptr->get_maxU();
      double subimage_dU=Umax/n_horiz_subimage_bins;
      double subimage_dV=1.0/n_vert_subimage_bins;

      double Diag=sqrt(1+Umax*Umax);
//      cout << "Diag = " << Diag << endl;
      double dtheta=PI/n_theta_bins;

      vector<linesegment> linesegments=videofunc::detect_line_segments(
         texture_rectangle_ptr);

      int n_bins=n_horiz_subimage_bins*n_vert_subimage_bins*n_frac_mag_bins*
         n_theta_bins;
      linesegments_histogram.reserve(n_bins);
      for (int b=0; b<n_bins; b++)
      {
         linesegments_histogram.push_back(0);
      }

      for (unsigned int e=0; e<linesegments.size(); e++)
      {
         linesegment curr_segment(linesegments[e]);
         threevector V1=curr_segment.get_v1();
         threevector V2=curr_segment.get_v2();
         threevector COM=0.5*(V1+V2);
         double U_COM=COM.get(0);
         double V_COM=COM.get(1);
      
         int i=basic_math::mytruncate(U_COM/subimage_dU);
         int j=basic_math::mytruncate(V_COM/subimage_dV);

         double curr_mag=(V2-V1).magnitude();
         double curr_frac_mag=curr_mag/Diag;
//         cout << "curr_frac_mag = " << curr_frac_mag << endl;
         double log_frac_mag=log(curr_frac_mag)/log(2);
//         double log_frac_mag=log(curr_frac_mag)/log(2.5);
         int log_bin=basic_math::mytruncate(log_frac_mag);
//         cout << "log_bin = " << log_bin << endl;

         int frac_mag_bin=2*n_frac_mag_bins+
            basic_math::max(log_bin,-2*n_frac_mag_bins);
         frac_mag_bin=basic_math::min(frac_mag_bin,n_frac_mag_bins-1);
//         cout << "frac_mag_bin = " << frac_mag_bin << endl;

         double m=(V2.get(1)-V1.get(1))/(V2.get(0)-V1.get(0));
         double curr_theta=atan2(1,m);
         curr_theta=basic_math::phase_to_canonical_interval(
            curr_theta,0,0.99999*PI);
         int theta_bin=curr_theta/dtheta+0.5;
         theta_bin=theta_bin%n_theta_bins;

//         cout << "curr_theta = " << curr_theta*180/PI 
//              << " dtheta = " << dtheta*180/PI
//              << " theta_bin = " << theta_bin << endl;
//         outputfunc::enter_continue_char();


//         int bin=i*n_vert_subimage_bins*n_theta_bins+j*n_theta_bins+theta_bin;

//         int bin=
//            i*n_subimage_bins*n_frac_mag_bins+j*n_frac_mag_bins+
//            frac_mag_bin;

         int bin=
            i*n_vert_subimage_bins*n_frac_mag_bins*n_theta_bins+
            j*n_frac_mag_bins*n_theta_bins+
            frac_mag_bin*n_theta_bins+
            theta_bin;
//         cout << "bin = " << bin << endl;

//         int bin=frac_mag_bin;
//         cout << "bin = " << bin << endl;

         linesegments_histogram[bin]=linesegments_histogram[bin]+1;

//         cout << "bin = " << bin
//              << " i = " << i << " j = " <<  j
//              << " theta = " << curr_theta*180/PI 
//              << " theta_bin = " << theta_bin 
//              << " frac_mag = " << curr_frac_mag 
//              << " log2_frac_mag = " << log2_frac_mag 
//              << " log_bin = " << log_bin
//              << " frac_mag_bin = " << frac_mag_bin
//              << endl;
      }

      delete texture_rectangle_ptr;
      return linesegments_histogram;
   }


/*
// ------------------------------------------------------------------------
   
   vector<double> compute_image_segments_histogram(
      int n_horiz_subimage_bins,int n_vert_subimage_bins,
      int n_frac_mag_bins,int n_theta_bins,string image_filename)
   {
      vector<double> linesegments_histogram;
         
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
      if (!texture_rectangle_ptr->reset_texture_content(image_filename))
      {
         return linesegments_histogram;
      }

      double Umax=texture_rectangle_ptr->get_maxU();
      double subimage_dU=Umax/n_horiz_subimage_bins;
      double subimage_dV=1.0/n_vert_subimage_bins;

      double Diag=sqrt(1+Umax*Umax);
//      cout << "Diag = " << Diag << endl;
      double dtheta=PI/n_theta_bins;

      vector<linesegment> linesegments=videofunc::detect_line_segments(
         texture_rectangle_ptr);

      int n_bins=n_horiz_subimage_bins*n_vert_subimage_bins*n_frac_mag_bins*
         n_theta_bins;
      linesegments_histogram.reserve(n_bins);
      for (int b=0; b<n_bins; b++)
      {
         linesegments_histogram.push_back(0);
      }

      for (unsigned int e=0; e<linesegments.size(); e++)
      {
         linesegment curr_segment(linesegments[e]);
         threevector V1=curr_segment.get_v1();
         threevector V2=curr_segment.get_v2();
         threevector COM=0.5*(V1+V2);
         double U_COM=COM.get(0);
         double V_COM=COM.get(1);
      
         int i=basic_math::mytruncate(U_COM/subimage_dU);
         int j=basic_math::mytruncate(V_COM/subimage_dV);

         double curr_mag=(V2-V1).magnitude();
         double curr_frac_mag=curr_mag/Diag;
//         cout << "curr_frac_mag = " << curr_frac_mag << endl;
         double log_frac_mag=log(curr_frac_mag)/log(2);
//         double log_frac_mag=log(curr_frac_mag)/log(2.5);
         int log_bin=basic_math::mytruncate(log_frac_mag);
         cout << "log_bin = " << log_bin << endl;
         int frac_mag_bin=n_frac_mag_bins+
            basic_math::max(log_bin,-n_frac_mag_bins);
//         int frac_mag_bin=1+n_frac_mag_bins+
//            basic_math::max(log_bin,-n_frac_mag_bins);
//         frac_mag_bin=basic_math::min(frac_mag_bin,n_frac_mag_bins);
//         cout << "frac_mag_bin = " << frac_mag_bin << endl;

         double m=(V2.get(1)-V1.get(1))/(V2.get(0)-V1.get(0));
         double curr_theta=atan2(1,m);
         curr_theta=basic_math::phase_to_canonical_interval(
            curr_theta,0,0.99999*PI);
         int theta_bin=curr_theta/dtheta+0.5;
         theta_bin=theta_bin%n_theta_bins;

//         cout << "curr_theta = " << curr_theta*180/PI 
//              << " dtheta = " << dtheta*180/PI
//              << " theta_bin = " << theta_bin << endl;
//         outputfunc::enter_continue_char();


//         int bin=i*n_vert_subimage_bins*n_theta_bins+j*n_theta_bins+theta_bin;

//         int bin=
//            i*n_subimage_bins*n_frac_mag_bins+j*n_frac_mag_bins+
//            frac_mag_bin;

         int bin=
            i*n_vert_subimage_bins*n_frac_mag_bins*n_theta_bins+
            j*n_frac_mag_bins*n_theta_bins+
            frac_mag_bin*n_theta_bins+
            theta_bin;
//         cout << "bin = " << bin << endl;

         linesegments_histogram[bin]=linesegments_histogram[bin]+1;

//         cout << "bin = " << bin
//              << " i = " << i << " j = " <<  j
//              << " theta = " << curr_theta*180/PI 
//              << " theta_bin = " << theta_bin 
//              << " frac_mag = " << curr_frac_mag 
//              << " log2_frac_mag = " << log2_frac_mag 
//              << " log_bin = " << log_bin
//              << " frac_mag_bin = " << frac_mag_bin
//              << endl;
      }

      delete texture_rectangle_ptr;
   }
*/

// ==========================================================================
// Texture descriptor methods
// ==========================================================================

   vector<double> compute_RGB_texture_histogram(
      string image_filename,
      int n_horiz_subimage_bins,int n_vert_subimage_bins,
      int n_frac_mag_bins,int n_theta_bins)
   {
//      cout << "inside descriptorfunc::compute_RGB_texture_histogram()"
//           << endl;
      
      vector<double> RGB_texture_histogram;

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
      if (!texture_rectangle_ptr->import_photo_from_file(image_filename))
      {
         return RGB_texture_histogram;
      }

/*
      texture_rectangle_ptr->convert_color_image_to_luminosity(); 
      twoDarray* ptwoDarray_ptr=texture_rectangle_ptr->
         refresh_ptwoDarray_ptr();

      twoDarray* xderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
      twoDarray* yderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
      twoDarray* gradient_mag_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
      twoDarray* gradient_phase_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
*/

      bool include_alpha_channel_flag=false;
      RGBA_array curr_RGBA_array=texture_rectangle_ptr->get_RGBA_twoDarrays(
         include_alpha_channel_flag);

      twoDarray* RtwoDarray_ptr=curr_RGBA_array.first;
      twoDarray* GtwoDarray_ptr=curr_RGBA_array.second;
      twoDarray* BtwoDarray_ptr=curr_RGBA_array.third;

      twoDarray* xderiv_twoDarray_ptr=new twoDarray(RtwoDarray_ptr);
      twoDarray* yderiv_twoDarray_ptr=new twoDarray(RtwoDarray_ptr);
      twoDarray* gradient_mag_twoDarray_ptr=new twoDarray(RtwoDarray_ptr);
      twoDarray* gradient_phase_twoDarray_ptr=new twoDarray(RtwoDarray_ptr);

      vector<double> red_texture_histogram,green_texture_histogram,
         blue_texture_histogram;

//      cout << "Compute red channel histogram:" << endl;
      descriptorfunc::compute_texture_channel_histogram(
         n_horiz_subimage_bins,n_vert_subimage_bins,
         n_frac_mag_bins,n_theta_bins,
         RtwoDarray_ptr,xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
         gradient_mag_twoDarray_ptr,gradient_phase_twoDarray_ptr,
         red_texture_histogram);

//      cout << "Compute green channel histogram:" << endl;
      descriptorfunc::compute_texture_channel_histogram(
         n_horiz_subimage_bins,n_vert_subimage_bins,
         n_frac_mag_bins,n_theta_bins,
         GtwoDarray_ptr,xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
         gradient_mag_twoDarray_ptr,gradient_phase_twoDarray_ptr,
         green_texture_histogram);

//    cout << "Compute blue channel histogram:" << endl;
      descriptorfunc::compute_texture_channel_histogram(
         n_horiz_subimage_bins,n_vert_subimage_bins,
         n_frac_mag_bins,n_theta_bins,
         BtwoDarray_ptr,xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
         gradient_mag_twoDarray_ptr,gradient_phase_twoDarray_ptr,
         blue_texture_histogram);

      delete gradient_mag_twoDarray_ptr;
      delete gradient_phase_twoDarray_ptr;
      delete xderiv_twoDarray_ptr;
      delete yderiv_twoDarray_ptr;
      delete texture_rectangle_ptr;

      for (unsigned int b=0; b<red_texture_histogram.size(); b++)
      {
         RGB_texture_histogram.push_back(red_texture_histogram[b]);
         RGB_texture_histogram.push_back(green_texture_histogram[b]);
         RGB_texture_histogram.push_back(blue_texture_histogram[b]);
      }

//      cout << "At end of compute_RGB_texture_histogram()" << endl;

      return RGB_texture_histogram;
   }

// ------------------------------------------------------------------------
   void compute_texture_channel_histogram(
      int n_horiz_subimage_bins,int n_vert_subimage_bins,
      int n_frac_mag_bins,int n_theta_bins,
      twoDarray* ptwoDarray_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      twoDarray* gradient_mag_twoDarray_ptr,
      twoDarray* gradient_phase_twoDarray_ptr,
      vector<double>& texture_channel_histogram)
   {
//      cout << "inside descriptorfunc::compute_texture_channel_histogram() #1"
//           << endl;

      imagefunc::compute_sobel_gradients(
         ptwoDarray_ptr,xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
         gradient_mag_twoDarray_ptr);

      double zmag_min_threshold=1E-8;
      double zmag_max_threshold=1E8;
      imagefunc::compute_gradient_phase_field(
         zmag_min_threshold,zmag_max_threshold,
         xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
         gradient_mag_twoDarray_ptr,gradient_phase_twoDarray_ptr);

      int n_bins=n_horiz_subimage_bins*n_vert_subimage_bins*n_frac_mag_bins*
         n_theta_bins;
      texture_channel_histogram.reserve(n_bins);
      for (int b=0; b<n_bins; b++)
      {
         texture_channel_histogram.push_back(0);
      }
      
      descriptorfunc::compute_texture_channel_histogram(
         n_horiz_subimage_bins,n_vert_subimage_bins,
         n_frac_mag_bins,n_theta_bins,
         gradient_mag_twoDarray_ptr,gradient_phase_twoDarray_ptr,
         texture_channel_histogram);


//      for (int b=0; b<n_bins; b++)
//      {
//         cout << texture_channel_histogram[b] << " " << flush;
//      }
//      cout << endl;

   }

// ------------------------------------------------------------------------   
   void compute_texture_channel_histogram(
      int n_horiz_subimage_bins,int n_vert_subimage_bins,
      int n_frac_mag_bins,int n_theta_bins,
      twoDarray const *gradient_mag_twoDarray_ptr,
      twoDarray const *gradient_phase_twoDarray_ptr,
      vector<double>& texture_channel_histogram)
   {
//      cout << "inside compute_texture_channel_histogram()" << endl;

      int width=gradient_mag_twoDarray_ptr->get_mdim();
      int height=gradient_mag_twoDarray_ptr->get_ndim();
      double subimage_width=width/n_horiz_subimage_bins;
      double subimage_height=height/n_vert_subimage_bins;
      const double max_magnitude=640;
      double dtheta=PI/n_theta_bins;
      const double log_two=0.693147;

//      cout << "width = " << width << " height = " << height << endl;
      for (int px=0; px<width; px++)
      {
         int i=basic_math::mytruncate(px/subimage_width);
         i=basic_math::min(i,n_horiz_subimage_bins-1);

         for (int py=0; py<height; py++)
         {
            int j=basic_math::mytruncate(py/subimage_height);
            j=basic_math::min(j,n_vert_subimage_bins-1);

//            double curr_mag=gradient_mag_twoDarray_ptr->get(px,py);
//            double curr_frac_mag=basic_math::min(curr_mag/max_magnitude,1.0);
            double curr_frac_mag=basic_math::min(
               gradient_mag_twoDarray_ptr->get(px,py)/max_magnitude,1.0);

//            cout << curr_frac_mag << " " << flush;
            
            int frac_mag_bin=0;
            if (curr_frac_mag > 1E-10)
            {
               double log_frac_mag=log(curr_frac_mag)/log_two;
               int log_bin=basic_math::mytruncate(log_frac_mag);
               frac_mag_bin=log_bin+16;
               frac_mag_bin=basic_math::max(0,frac_mag_bin);
               frac_mag_bin=basic_math::min(n_frac_mag_bins-1,frac_mag_bin);
  
//               frac_mag_bin=n_frac_mag_bins+
//                  basic_math::max(log_bin,-n_frac_mag_bins);
            }

//            if (frac_mag_bin >= n_frac_mag_bins || log_bin < -100)
//            {
//               cout << "curr_mag = " << curr_mag << endl;
//               cout << "curr_frac_mag = " << curr_frac_mag << endl;
//               cout << "log_frac_mag = " << log_frac_mag << endl;
//               cout << "log_bin = " << log_bin << endl;
//               cout << "frac_mag_bin = " << frac_mag_bin << endl;
//               exit(-1);
//            }

            double curr_phase=gradient_phase_twoDarray_ptr->get(px,py);
            int theta_bin=0;
            if (curr_phase > 0.5*NEGATIVEINFINITY)
            {
               curr_phase=basic_math::phase_to_canonical_interval(
                  curr_phase,0,PI);
               theta_bin=curr_phase/dtheta+0.5;
               theta_bin=theta_bin%n_theta_bins;
            }

//            if (theta_bin >= n_theta_bins)
//            {
//               cout << "theta_bin = " << theta_bin << endl;
//               exit(-1);
//            }
            
            int bin=
               i*n_vert_subimage_bins*n_frac_mag_bins*n_theta_bins+
               j*n_frac_mag_bins*n_theta_bins+
               frac_mag_bin*n_theta_bins+
               theta_bin;

            texture_channel_histogram[bin]=texture_channel_histogram[bin]+1;

         } // loop over py
      } // loop over px
   }

// ==========================================================================
// Self similarity descriptor methods
// ==========================================================================

// Method compute_SSIM_descriptor()

   vector<double> compute_SSIM_descriptor(
      string image_filename,
      int n_radial_bins,int n_theta_bins)
   {
      cout << "inside descriptorfunc::compute_SSIM_descriptor()" << endl;

//      double var_prefactor=1;
//      cout << "Enter variance prefactor:" << endl;
//      cin >> var_prefactor;
      
      vector<double> SSIM_descriptor;

      texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
      if (!texture_rectangle_ptr->import_photo_from_file(image_filename))
      {
         return SSIM_descriptor;
      }

      const int patch_length=5;
      const int neighborhood_radius=40;
      twoDarray* log_polar_twoDarray_ptr=
         log_polar_bins(patch_length,neighborhood_radius,
                        n_radial_bins,n_theta_bins);

// Initialize SSIM_descriptor to hold n_radial_bins*n_theta_bins zero
// values:

      for (int s=0; s<n_radial_bins*n_theta_bins; s++)
      {
         SSIM_descriptor.push_back(0);
      }

      int width=texture_rectangle_ptr->getWidth();
      int height=texture_rectangle_ptr->getHeight();
      for (int qx=neighborhood_radius; qx<width-neighborhood_radius; 
           qx += patch_length)
      {
         for (int qy=neighborhood_radius; qy<height-neighborhood_radius;
              qy += patch_length)
         {
         
// FAKE FAKE:  Sun Oct 6, 2013 at 6:12 pm

            cout << "Enter qx:" << endl;
            cin >> qx;
            cout << "Enter qy:" << endl;
            cin >> qy;

            int mdim=2*neighborhood_radius-patch_length+1;
            int ndim=2*neighborhood_radius-patch_length+1;
            texture_rectangle* tr_ptr=new texture_rectangle(
               mdim,ndim,1,3,NULL);
            tr_ptr->generate_blank_image_file(mdim,ndim,"blank.jpg",0.5);

// Compute maximal variance of difference of all patches within a
// neighborhood of q (radius 1) relative to the patch centered at q:

            int Rp,Rq,Gp,Gq,Bp,Bq;
            vector<double> patch_differences;
            for (int px=qx-1; px<qx+1; px++)
            {
               for (int py=qy-1; py<qy+1; py++)
               {
                  double SSD=0;
                  for (int i=-patch_length/2; i<=patch_length/2; i++)
                  {
                     for (int j=-patch_length/2; j<=patch_length/2; j++)
                     {
                        texture_rectangle_ptr->get_pixel_RGB_values(
                           px+i,py+j,Rp,Gp,Bp);
                        texture_rectangle_ptr->get_pixel_RGB_values(
                           qx+i,qy+j,Rq,Gq,Bq);
                        SSD += sqr(Rp-Rq)+sqr(Gp-Gq)+sqr(Bp-Bq);
                     } // loop over index j
                  } // loop over index i 
                  patch_differences.push_back(SSD);
               } // loop over py
            } // loop over px

            double sigma_auto=mathfunc::maximal_value(patch_differences);
            double sigma_noise=1;
            double sigma=sqrt(basic_math::max(sigma_auto,sigma_noise));
//            cout << "sigma = " << sigma << endl;

            for (int px=qx-neighborhood_radius+patch_length/2; 
                 px<qx+neighborhood_radius-patch_length/2; px++)
            {
               int nx=px-qx+neighborhood_radius-patch_length/2;
               
               for (int py=qy-neighborhood_radius+patch_length/2; 
                    py<qy+neighborhood_radius-patch_length/2; py++)
               {
                  int ny=py-qy+neighborhood_radius-patch_length/2;
//                  cout << "nx = " << nx << " ny = " << ny << endl;

                  double SSD=0;
                  int Rp,Rq,Gp,Gq,Bp,Bq;
                  for (int i=-patch_length/2; i<=patch_length/2; i++)
                  {
                     for (int j=-patch_length/2; j<=patch_length/2; j++)
                     {
//                        cout << "px+i = " << px+i
//                             << " py+j = " << py+j
//                             << " qx+i = " << qx+i
//                             << " qy+j = " << qy+j << endl;

                        texture_rectangle_ptr->get_pixel_RGB_values(
                           px+i,py+j,Rp,Gp,Bp);
                        texture_rectangle_ptr->get_pixel_RGB_values(
                           qx+i,qy+j,Rq,Gq,Bq);
                        SSD += sqr(Rp-Rq)+sqr(Gp-Gq)+sqr(Bp-Bq);
                     } // loop over index j
                  } // loop over index i 

//                  cout << "SSD = " << SSD << " SSD/sqr(sigma) = "
//                       << SSD/sqr(sigma) << endl;

//                  double S=exp(-SSD/var_prefactor*sqr(sigma));
                  double S=exp(-SSD/sqr(sigma));

                  int R=255*S;
                  int G=255*S;
                  int B=255*S;//                  cout << "R = " << R << " G = " << G 
//                       << " B = " << B << endl;
                  tr_ptr->set_pixel_RGB_values(nx,ny,R,G,B);

                  int SSIM_bin=log_polar_twoDarray_ptr->get(nx,ny);
                  SSIM_descriptor[SSIM_bin]=basic_math::max(
                     SSIM_descriptor[SSIM_bin],S);

               } // loop over py
            } // loop over px

// Linearly renormalize SSIM_descriptor values so that they range from
// 0 to 1:

            double SSIM_max=mathfunc::maximal_value(SSIM_descriptor);
            double SSIM_min=mathfunc::minimal_value(SSIM_descriptor);
            for (unsigned int s=0; s<SSIM_descriptor.size(); s++)
            {
               double renormalized_SSIM=
                  (SSIM_descriptor[s]-SSIM_min)/(SSIM_max-SSIM_min);
               SSIM_descriptor[s]=renormalized_SSIM;
            }

// For visualization purposes, replace all entries within
// *logpolar_twoDarray_ptr with their corresponding values in
// SSIM_descriptor:

            for (int px=qx-neighborhood_radius+patch_length/2; 
                 px<qx+neighborhood_radius-patch_length/2; px++)
            {
               int nx=px-qx+neighborhood_radius-patch_length/2;
               
               for (int py=qy-neighborhood_radius+patch_length/2; 
                    py<qy+neighborhood_radius-patch_length/2; py++)
               {
                  int ny=py-qy+neighborhood_radius-patch_length/2;
//                  cout << "nx = " << nx << " ny = " << ny << endl;
                  int SSIM_bin=log_polar_twoDarray_ptr->get(nx,ny);
                  double SSIM_value=SSIM_descriptor[SSIM_bin];
                  log_polar_twoDarray_ptr->put(nx,ny,SSIM_value);
               }
            }
            
            string ssim_filename="ssim.jpg";
            cout << "ssim_filename = " << ssim_filename << endl;
            tr_ptr->write_curr_frame(ssim_filename);
            delete tr_ptr;

            texture_rectangle* polar_tr_ptr=new texture_rectangle(
               log_polar_twoDarray_ptr->get_mdim(),
               log_polar_twoDarray_ptr->get_ndim(),1,3,NULL);
            polar_tr_ptr->initialize_twoDarray_image(
               log_polar_twoDarray_ptr,3);
            string polar_filename="polar.jpg";
            cout << "polar_filename = " << polar_filename << endl;
            polar_tr_ptr->write_curr_frame(polar_filename);
            delete polar_tr_ptr;

            outputfunc::enter_continue_char();

         } // loop over qy
      } // loop over qx

      delete texture_rectangle_ptr;
      delete log_polar_twoDarray_ptr;
      return SSIM_descriptor;
   }
   
// ------------------------------------------------------------------------   
// Method log_polar_bins()

   twoDarray* log_polar_bins(
      int patch_length,int neighborhood_radius,
      int n_radial_bins,int n_theta_bins)
   {
//      cout << "inside compute_texture_channel_histogram()" << endl;

      int mdim=2*neighborhood_radius-patch_length+1;
      int ndim=2*neighborhood_radius-patch_length+1;
      twoDarray* logpolar_twoDarray_ptr=new twoDarray(mdim,ndim);

      texture_rectangle* tr_ptr=new texture_rectangle(mdim,ndim,1,3,NULL);
      tr_ptr->generate_blank_image_file(mdim,ndim,"blank.jpg",0.5);

      double r=0;
//      double dr=3.5;
//      double dr=4;
      double dr=neighborhood_radius/10.0;
      vector<double> radii;
      for (int i=1; i<n_radial_bins; i++)
      {
         r += dr;
         radii.push_back(r);
         dr += dr;
//         cout << "i = " << i << " dr = " << dr 
//              << " radius = " << radii.back() << endl;
      }

      double d_theta=360.0/n_theta_bins*PI/180;

      for (int px=-neighborhood_radius+patch_length/2; 
           px<neighborhood_radius-patch_length/2; px++)
      {
         double nx=px+neighborhood_radius-patch_length/2;
         
         for (int py=-neighborhood_radius+patch_length/2; 
              py<neighborhood_radius-patch_length/2; py++)
         {
            double ny=py+neighborhood_radius-patch_length/2;
//            cout << "px = " << px << " py = " << py
//                 << " nx = " << nx << " ny = " << ny << endl;

            double curr_r=sqrt(px*px+py*py);
            int r=0;
            if (curr_r >= radii[0] && curr_r <radii[1])
            {
               r=1;
            }
            else if (curr_r >= radii[1] && curr_r <radii[2])            
            {
               r=2;
            }
            else if (curr_r >= radii[2])
            {
               r=3;
            }

            double theta=atan2(py,px);
            theta=basic_math::phase_to_canonical_interval(theta,0,2*PI);
            int t=theta/d_theta;

            int bin=r*n_theta_bins+t;
            colorfunc::RGB curr_rgb=colorfunc::get_RGB_values(
               colorfunc::get_color(bin%17));
            int R=255*curr_rgb.first;
            int G=255*curr_rgb.second;
            int B=255*curr_rgb.third;

            tr_ptr->set_pixel_RGB_values(nx,ny,R,G,B);
            logpolar_twoDarray_ptr->put(nx,ny,bin);

         } // loop over py
      } // loop over px

      string output_filename="log_polar.jpg";
      cout << "output_filename = " << output_filename << endl;
      tr_ptr->write_curr_frame(output_filename);
      delete tr_ptr;

      return logpolar_twoDarray_ptr;
   }

// ==========================================================================
// Local Binary Pattern descriptor methods
// ==========================================================================

/*

#include <opencv2/opencv.hpp>
#include "LBP.hpp"

// Method compute_LBP_descriptor()

   vector<double> compute_LBP_descriptor(string image_filename)
   {
      cout << "inside descriptorfunc::compute_LBP_descriptor()" << endl;

      // Read an (RGB) image and convert to monochrome
      cv::Mat img = imread( image_filename.c_str(), 0 );

      // convert to double precision
      img.convertTo( img, CV_64F );
      cout << "image w/h = " << img.rows << "/" << img.cols << " (" 
           << img.rows*img.cols << ")" << endl;

      // Create an LBP instance of type HF using 8 support points
      LBP lbp( 8, LBP_MAPPING_NONE );
      // Calculate the descriptor
      lbp.calcLBP( img );
      // Calculate Fourier tranformed histogram

      vector<double> LBP_descriptor = lbp.calcHist().getHist( false );

      double hist_sum = 0;
      for(unsigned int i = 0; i < LBP_descriptor.size(); i++ ) 
      {
         hist_sum += LBP_descriptor[i];
      }

      for(unsigned int i = 0; i < LBP_descriptor.size(); i++ ) 
      {
         LBP_descriptor[i] /= hist_sum;
      }
      return LBP_descriptor;
   }

*/
   

} // descriptorfunc namespace
