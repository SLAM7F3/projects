// ==========================================================================
// PNGFUNCS stand-alone methods
// ==========================================================================
// Last modified on 7/30/13; 4/5/14; 6/7/14; 11/11/15
// ==========================================================================

#include <iostream>
#include <pngwriter.h>
#include <stdio.h>

#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "image/imagefuncs.h"
#include "image/myimage.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "image/pngfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"
#include "math/twovector.h"

using std::cout;
using std::cin;
using std::endl;
using std::flush;
using std::string;
using std::vector;

namespace pngfunc
{
   png_uint_32 width,height;
   RGB_array RGB_twoDarray;

// We treat the following structures and arrays as private objects
// within this pngfunc namespace:

   png_byte channels;
   png_structp png_ptr=NULL;
   png_infop info_ptr=NULL;
   png_infop end_info=NULL;
   png_bytep* row_pointers=NULL;

// ==========================================================================
// Set and get methods
// ==========================================================================

   void set_width(int w)
      {
         width=static_cast<png_uint_32>(w);
      }

   void set_height(int h)
      {
         height=static_cast<png_uint_32>(h);
      }

// ==========================================================================
// Memory management methods
// ==========================================================================

   void initialize_row_pointers(int nxbins,int nybins,int nchannels)
      {
         width=nxbins;
         height=nybins;
         channels=static_cast<png_byte>(nchannels);
         allocate_row_pointers();
      }

// ---------------------------------------------------------------------
// Method allocate_row_points dynamically allocates an array of PNG
// row pointers.  It subsequently allocates each row as a series of
// bytes (e.g. 3*width for RGB images).

   void allocate_row_pointers()
      {
         row_pointers=new png_bytep[height];
         for (unsigned int i=0; i<height; i++)
         {
            row_pointers[i]=new png_byte[width*channels];
         }
      }

// ---------------------------------------------------------------------
// Method generate_charstar_array dumps the contents of the current
// PNG image into a dynamically allocated unsigned char* array.

   unsigned char* generate_charstar_array()
      {
         unsigned int nxbins=width;
         unsigned int nybins=height;

         const int NBYTES_PER_PIXEL=3;
         unsigned char* data=new unsigned char[
            NBYTES_PER_PIXEL*nxbins*nybins];

         colorfunc::RGB curr_RGB;
         for (unsigned int py=0; py<nybins; py++)
         {
            for (unsigned int px=0; px<nxbins; px++)
            {
               Triple<int,int,int> rgb=get_pixel_RGB_values(px,py);
               curr_RGB.first=rgb.first;
               curr_RGB.second=rgb.second;
               curr_RGB.third=rgb.third;
               colorfunc::RGB_bytes curr_RGB_bytes=
                  colorfunc::RGB_to_bytes(curr_RGB,false);

               int p=py*nxbins+px;
               int i=NBYTES_PER_PIXEL*p;
               data[i+0]=curr_RGB_bytes.first;
               data[i+1]=curr_RGB_bytes.second;
               data[i+2]=curr_RGB_bytes.third;
            } // loop over px index
         } // loop over py index
         return data;
      }

// ---------------------------------------------------------------------
// Method allocate_RGB_twoDarrays dynamically creates the 3 twoDarrays
// within member object RGB_twoDarray to hold color information
// extracted from the current PNG image.  It initializes their entries
// to zero.

   void allocate_RGB_twoDarrays()
      {
         allocate_RGB_twoDarrays(static_cast<int>(width),
                                 static_cast<int>(height));
      }

   void allocate_RGB_twoDarrays(int nxbins,int nybins)
      {
         allocate_RGB_twoDarrays(nxbins,nybins,0,nxbins-1,0,nybins-1);
      }
   
   void allocate_RGB_twoDarrays(
      int nxbins,int nybins,double xlo,double xhi,double ylo,double yhi)
      {
         width=nxbins;
         height=nybins;

         twoDarray* RtwoDarray_ptr=new twoDarray(nxbins,nybins);
         RtwoDarray_ptr->set_xlo(xlo);
         RtwoDarray_ptr->set_xhi(xhi);
         RtwoDarray_ptr->set_ylo(ylo);
         RtwoDarray_ptr->set_yhi(yhi);
         RtwoDarray_ptr->set_deltax((xhi-xlo)/(nxbins-1));
         RtwoDarray_ptr->set_deltay((yhi-ylo)/(nybins-1));

         twoDarray* GtwoDarray_ptr=new twoDarray(RtwoDarray_ptr);
         twoDarray* BtwoDarray_ptr=new twoDarray(RtwoDarray_ptr);

         RGB_twoDarray.first=RtwoDarray_ptr;
         RGB_twoDarray.second=GtwoDarray_ptr;
         RGB_twoDarray.third=BtwoDarray_ptr;
         clear_RGB_twoDarrays();
      }

   void delete_RGB_twoDarrays()
      {
         delete RGB_twoDarray.first;
         delete RGB_twoDarray.second;
         delete RGB_twoDarray.third;
         RGB_twoDarray.first=NULL;
         RGB_twoDarray.second=NULL;
         RGB_twoDarray.third=NULL;
      }

// ==========================================================================
// PNG file I/O methods
// ==========================================================================

// Method open_png_file takes in the filename for some PNG file.  It
// opens up and reads header information for this file.  In
// particular, the width and height of the image are returned by this
// method as well as the number of color channels (e.g. 3 for RGB
// data).  A pointer to a PNG structure is also returned.  

// All of the PNG library calls made within this method are described
// in the libpng manual at
// http://www.libpng.org/pub/png/libpng-1.2.5-manual.html#section-5.
// It's also useful to look at the png.h header file to find out
// interface type information.

   bool open_png_file(string png_filename)
      {
         const bool ERROR=false;

         FILE *fp = fopen(png_filename.c_str(), "rb");
         if (!fp)
         {
            cout << "Could not open " << png_filename << endl;
            return ERROR;
         }

         size_t number=8;
         png_byte* header=new png_byte[number];
         fread(header, 1, number, fp);

// First check whether input file actually corresponds to a PNG image:

         bool is_png = !png_sig_cmp(header, 0, number);
         if (!is_png)
         {
            cout << "Image file is not PNG!" << endl;
            return ERROR;
         }

// Allocate and initialize png_struct, png_info and end_info
// structures:

         png_ptr = png_create_read_struct
            (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
         if (!png_ptr) return ERROR;

         info_ptr = png_create_info_struct(png_ptr);
         if (!info_ptr)
         {
            png_destroy_read_struct(
               &png_ptr,(png_infopp)NULL, (png_infopp)NULL);
            return ERROR;
         }

         end_info = png_create_info_struct(png_ptr);
         if (!end_info)
         {
            png_destroy_read_struct(&png_ptr, &info_ptr,(png_infopp)NULL);
            return ERROR;
         }

// When libpng encounters an error, it expects to longjmp back to your
// routine.  So we need to call setjmp below:

         if (setjmp(png_jmpbuf(png_ptr)))
         {
            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
            fclose(fp);
            return (ERROR);
         }

// Set up input code:

         png_init_io(png_ptr, fp);
         png_set_sig_bytes(png_ptr, number);

// Perform low-level reads of PNG header information: 

         png_read_info(png_ptr,info_ptr);

         width=png_get_image_width(png_ptr,info_ptr);
         height=png_get_image_height(png_ptr,info_ptr);
         channels=png_get_channels(png_ptr,info_ptr);
         png_byte bit_depth=png_get_bit_depth(png_ptr,info_ptr);
         png_byte color_type=png_get_color_type(png_ptr,info_ptr);
//         png_uint_32 rowbytes=png_get_rowbytes(png_ptr,info_ptr);

//         cout << "width = " << width << " height = " << height << endl;
         cout << "channels = " << stringfunc::byte_value(channels) << endl;
         cout << "bit_depth = " << stringfunc::byte_value(bit_depth) << endl;
         cout << "color_type = " << stringfunc::byte_value(color_type) 
              << endl;

         cout << endl;
         cout << "PNG_COLOR_TYPE_GRAY = " << PNG_COLOR_TYPE_GRAY << endl;
         cout << "PNG_COLOR_TYPE_GRAY_ALPHA = " 
              << PNG_COLOR_TYPE_GRAY << endl;
         cout << "PNG_COLOR_TYPE_PALETTE = " << PNG_COLOR_TYPE_PALETTE 
              << endl;
         cout << "PNG_COLOR_TYPE_RGB = " << PNG_COLOR_TYPE_RGB << endl;
         cout << "PNG_COLOR_TYPE_RGB_ALPHA = " 
              << PNG_COLOR_TYPE_RGB_ALPHA << endl;
         cout << "PNG_COLOR_MASK_PALETTE = " << PNG_COLOR_MASK_PALETTE
              << endl;
         cout << "PNG_COLOR_MASK_COLOR = " << PNG_COLOR_MASK_COLOR << endl;
         cout << "PNG_COLOR_MASK_ALPHA = " << PNG_COLOR_MASK_ALPHA << endl;
         cout << endl;
         
         if (color_type==PNG_COLOR_TYPE_PALETTE)
         {
            cout << "color type = PNG_COLOR_TYPE_PALETTE" << endl;
         }
         else if (color_type==PNG_COLOR_TYPE_RGB)
         {
            cout << "color type = PNG_COLOR_TYPE_RGB" << endl;
         }
         
//         cout << "color_type & PNG_COLOR_TYPE_PALETTE = "
//              << (color_type & PNG_COLOR_TYPE_PALETTE) << endl;
//         cout << "color_type & PNG_COLOR_TYPE_RGB = "
//              << (color_type & PNG_COLOR_TYPE_RGB) << endl;
//         cout << "color_type & PNG_COLOR_MASK_ALPHA = "
//              << (color_type & PNG_COLOR_MASK_ALPHA) << endl;
//         cout << "rowbytes = " << rowbytes << endl;

         if (color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png_ptr);

// We don't need the alpha channel on an image, and we want to remove
// it rather than combine it with the background:

         if (color_type & PNG_COLOR_MASK_ALPHA)
            png_set_strip_alpha(png_ptr);

         return !ERROR;
      }

// ---------------------------------------------------------------------
// Method parse_png_file dynamically allocates an array of PNG image
// row pointers.  and then reads the PNG image information into this
// double array.

   void parse_png_file()
      {
         allocate_row_pointers();
         png_read_image(png_ptr,row_pointers);
      }

// ---------------------------------------------------------------------
// Method draw_png_file takes in double array row_pointers[][] which
// is generated by parse_png_file().  This method converts the RGB
// values within this array into corresponding grey-scale values.  It
// subsequently writes the PNG picture to metafile output.

   void draw_png_file()
      {
         const double max_x=5;  // meters
         const double max_y=5;  // meters

         unsigned int nxbins=width;
         unsigned int nybins=height;
         cout << "nxbins = " << nxbins << " nybins = " << nybins << endl;
         myimage greyimage(nxbins,nybins);   
         greyimage.set_z2Darray_ptr(new twoDarray(nxbins,nybins));

// Initialize image parameters:

         greyimage.set_imagedir(filefunc::get_pwd()+"images/fitimage/");
         filefunc::dircreate(greyimage.get_imagedir());
         greyimage.set_classified(false);
         greyimage.set_title("Grey scale PNG image");
         greyimage.set_colortable_filename(filefunc::get_pwd()
            +"colortables/colortable.prob");
         greyimage.get_z2Darray_ptr()->init_coord_system(max_x,max_y);
         greyimage.get_z2Darray_ptr()->initialize_values(0);

         twoDarray* ztwoDarray_ptr=greyimage.get_z2Darray_ptr();

         for (unsigned int py=0; py<nybins; py++)
         {
            for (unsigned int px=0; px<nxbins; px++)
            {
               Triple<int,int,int> rgb=get_pixel_RGB_values(px,py);
               double grey=0.212671*rgb.first+0.715160*rgb.second
                  +0.072169*rgb.third;
//               cout << "px = " << px << " py = " << py 
//                    << " R = " << rgb.first << " G = " << rgb.second 
//                    << " B = " << rgb.third << " grey = " << grey << endl;
               ztwoDarray_ptr->put(px,py,grey/256.0);
//               ztwoDarray_ptr->put(
//                  px,py,RGB_twoDarray.first->get(px,py)/256.0);
//               ztwoDarray_ptr->put(
//                  px,py,RGB_twoDarray.second->get(px,py)/256.0);
//               ztwoDarray_ptr->put(
//                  px,py,RGB_twoDarray.third->get(px,py)/256.0);
            }
         }

         cout << "Before call to writeimage" << endl;
         greyimage.writeimage("png_image",ztwoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method close_png_file frees all memory allocated by libpng and
// frees the dynamically allocated double array row_pointers.

   void close_png_file()
      {
         if (png_ptr != NULL)
         {
            png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
         }
         
         for (unsigned int i=0; i<height; i++)
         {
            delete row_pointers[i];
         }
         delete row_pointers;
         row_pointers=NULL;
      }

// ---------------------------------------------------------------------
// Method write_output_png_file takes in the filename for some output
// PNG file.  It opens up this file and dumps out the PNG image
// data stored within row_pointers.

   bool write_output_png_file(string output_png_filename)
      {
         const bool ERROR=false;

         FILE* output_fp = fopen(output_png_filename.c_str(), "wb");
         if (!output_fp)
         {
            return (ERROR);
         }

// Allocate and initialize png_struct and png_info structures:

         png_structp output_png_ptr = png_create_write_struct
            (PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);
         if (!output_png_ptr) return (ERROR);

         if (info_ptr==NULL)
         {
            png_infop info_ptr = png_create_info_struct(output_png_ptr);
            if (!info_ptr)
            {
               png_destroy_write_struct(&output_png_ptr,(png_infopp)NULL);
               return (ERROR);
            }

            png_byte color_type=PNG_COLOR_TYPE_RGB;
            png_byte bit_depth=static_cast<png_byte>(8);
            png_set_IHDR(output_png_ptr, info_ptr, width, height,
                         bit_depth, color_type, PNG_INTERLACE_NONE,
                         PNG_COMPRESSION_TYPE_DEFAULT, 
                         PNG_FILTER_TYPE_DEFAULT);

         } // info_ptr==NULL conditional

// When libpng encounters an error, it expects to longjmp back to your
// routine.  So we need to call setjmp below:

         if (setjmp(png_jmpbuf(output_png_ptr)))
         {
            png_destroy_write_struct(&output_png_ptr, &info_ptr);
            fclose(output_fp);
            return (ERROR);
         }

// Set up output code:

         png_init_io(output_png_ptr, output_fp);
         png_write_info(output_png_ptr, info_ptr);

//         png_byte* row_pointers[height];
         png_write_image(output_png_ptr, row_pointers);
         png_write_end(output_png_ptr,info_ptr);
         png_destroy_write_struct(&output_png_ptr, &info_ptr);
         return !ERROR;
      }

// ==========================================================================
// RGB methods
// ==========================================================================

// Method enumerate_all_RGB_values loops over every pixel within the
// PNG image and prints it RGB values.

   void enumerate_all_RGB_values()
      {
         int counter=0;
         for (unsigned int py=0; py<height; py++)
         {
            for (unsigned int px=0; px<width; px++)
            {
               int R=stringfunc::byte_value(row_pointers[py][3*px+0]);
               int G=stringfunc::byte_value(row_pointers[py][3*px+1]);
               int B=stringfunc::byte_value(row_pointers[py][3*px+2]);
               if (!(R==255 && B==255 && G==255))
               {
                  counter++;
                  cout << "px = " << px << " py = " << py 
                       << " R = " << R << " G = " << G << " B = " << B
                       << endl;
               }
            } // loop over px index
         } // loop over py index
         cout << "Total number of non-null valued RGB pixels = "
              << counter << endl;
      }

// ---------------------------------------------------------------------
// Method get_RGB_values takes in an image plane point expressed in u
// and v coordinates.  This boolean method returns false if these
// coordinates lie outside the current PNG image.  Otherwise, it
// fetches the RGB values for the pixel which lies closest to (u,v)
// and returns those within an output Triple of integers.

   bool get_RGB_values(const twovector& image_point,Triple<int,int,int>& rgb)
      {
         double u=image_point.get(0);
         double v=image_point.get(1);

         if (u < 0 || v < 0)
         {
            return false;
         }
         else
         {
            unsigned int px=basic_math::round(u);
            unsigned int py=basic_math::round(v);
            if (px >= width || py >= height)
            {
               return false;
            }
            else
            {
               int R=stringfunc::byte_value(row_pointers[py][3*px+0]);
               int G=stringfunc::byte_value(row_pointers[py][3*px+1]);
               int B=stringfunc::byte_value(row_pointers[py][3*px+2]);
               rgb=Triple<int,int,int>(R,G,B);
               return true;
            }
         }
      }

// ---------------------------------------------------------------------
   Triple<int,int,int> get_pixel_RGB_values(int px,int py)
      {
         int R=stringfunc::byte_value(row_pointers[py][3*px+0]);
         int G=stringfunc::byte_value(row_pointers[py][3*px+1]);
         int B=stringfunc::byte_value(row_pointers[py][3*px+2]);
         return Triple<int,int,int>(R,G,B);
      }

   void put_pixel_RGB_values(
      int px,int py,const Triple<int,int,int>& rgb)
      {
         row_pointers[py][3*px+0]=static_cast<char>(rgb.first);
         row_pointers[py][3*px+1]=static_cast<char>(rgb.second);
         row_pointers[py][3*px+2]=static_cast<char>(rgb.third);
      }
   
   Triple<double,double,double> get_RGB_twoDarray_values(int px,int py)
      {
         double R=RGB_twoDarray.first->get(px,py);
         double G=RGB_twoDarray.second->get(px,py);
         double B=RGB_twoDarray.third->get(px,py);
         return Triple<double,double,double>(R,G,B);
      }

// ---------------------------------------------------------------------
// Method fill_RGB_twoDarrays loops over every pixel in the PNG image
// and transfers its RGB values to member RGB_twoDarray.

   void clear_RGB_twoDarrays()
      {
         RGB_twoDarray.first->clear_values();
         RGB_twoDarray.second->clear_values();
         RGB_twoDarray.third->clear_values();
      }
   
   void fill_RGB_twoDarrays()
      {
         for (unsigned int px=0; px<width; px++)
         {
            for (unsigned int py=0; py<height; py++)
            {
               Triple<int,int,int> rgb=get_pixel_RGB_values(px,py);
//               cout << "px = " << px << " py = " << py 
//                    << " R = " << rgb.first << " G = " << rgb.second 
//                    << " B = " << rgb.third << " grey = " << grey << endl;
               RGB_twoDarray.first->put(px,py,rgb.first);
               RGB_twoDarray.second->put(px,py,rgb.second);
               RGB_twoDarray.third->put(px,py,rgb.third);
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method copy_RGB_twoDarray takes in RGB_arrays RGB_orig_twoDarray
// and RGB_copy_twoDarray.  It copies the contents of former onto the
// latter.

   void copy_RGB_twoDarrays(
      const RGB_array& RGB_orig_twoDarray,RGB_array& RGB_copy_twoDarray)
      {
         unsigned int nxbins=width;
         unsigned int nybins=height;
         for (unsigned int px=0; px<nxbins; px++)
         {
            for (unsigned int py=0; py<nybins; py++)
            {
               RGB_copy_twoDarray.first->put(
                  px,py,RGB_orig_twoDarray.first->get(px,py));
               RGB_copy_twoDarray.second->put(
                  px,py,RGB_orig_twoDarray.second->get(px,py));
               RGB_copy_twoDarray.third->put(
                  px,py,RGB_orig_twoDarray.third->get(px,py));
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method antialias_RGB_twoDarrays loops over every pixel within a PNG
// file.  We assume that the picture has been loaded into "member
// object" RGB_twoDarray prior to this method being called.  Any
// non-black pixel within the image is left unchanged.  But on the
// first of two iterations, black pixels which are surrounded by 3
// non-black "corner" pixels are reset to a fraction of the minimal
// corner pixel value.  On the second iteration, remaining black
// pixels which are surrounded by 2 non-black "corner" pixels are
// reset to a smaller fraction of the minimal corner pixel value.  

// This method can be called multiple times in order to increase the
// smoothness of the final anti-aliased image.

   void antialias_RGB_twoDarrays(const vector<double>& filter_frac)
      {
         outputfunc::write_banner("Antialiasing RGB twoDarrays:");

         unsigned int nxbins=width;
         unsigned int nybins=height;
         RGB_array RGB_copy_twoDarray;
         RGB_copy_twoDarray.first=new twoDarray(nxbins,nybins);
         RGB_copy_twoDarray.second=new twoDarray(nxbins,nybins);
         RGB_copy_twoDarray.third=new twoDarray(nxbins,nybins);
         copy_RGB_twoDarrays(RGB_twoDarray,RGB_copy_twoDarray);

         const double THRESHOLD=1.0;
         vector<bool> neighbor_colored;
         vector<Triple<double,double,double> > neighbor_rgb;
         neighbor_colored.reserve(9);
         neighbor_rgb.reserve(9);

         const unsigned int n_iters=filter_frac.size();
         for (unsigned int iter=0; iter<n_iters; iter++)
         {
            for (unsigned int px=1; px<nxbins-1; px++)
            {
               for (unsigned int py=1; py<nybins-1; py++)
               {

// Check whether current pixel's color is black:

                  Triple<double,double,double> rgb=
                     get_RGB_twoDarray_values(px,py);
               
                  if (rgb.first < THRESHOLD && rgb.second < THRESHOLD &&
                      rgb.third < THRESHOLD)
                  {
                     neighbor_colored.clear();
                     neighbor_rgb.clear();

                     for (int qx=-1; qx<=1; qx++)
                     {
                        for (int qy=-1; qy<=1; qy++)
                        {
                           Triple<double,double,double> curr_RGB=
                              get_RGB_twoDarray_values(px+qx,py+qy);
                           neighbor_rgb.push_back(curr_RGB);
                           if (curr_RGB.first > 0 || curr_RGB.second > 0 ||
                               curr_RGB.third > 0)
                           {
                              neighbor_colored.push_back(true);
                           }
                           else
                           {
                              neighbor_colored.push_back(false);
                           }
                        } // loop over qy
                     } // loop over qx


                     if (iter==0)
                     {
                     
// First search for sets of 3 "corner neighbors" which are all
// non-zero valued.  Reset central black pixel to filter_frac of the
// minimal value of these corner neighbors:

                        bool central_pixel_black=true;
                        int i,j,k;
                        for (unsigned int c=0; c<4; c++)
                        {
                           switch(c)
                           {
                              case 0:
                                 i=3; j=0; k=1;
                                 break;
                              case 1:
                                 i=1; j=2; k=5;
                                 break;
                              case 2:
                                 i=5; j=8; k=7;
                                 break;
                              case 3:
                                 i=7; j=6; k=3;
                                 break;
                           }
                     
                           if (neighbor_colored[i] && neighbor_colored[j] &&
                               neighbor_colored[k] && central_pixel_black)
                           {
                              RGB_copy_twoDarray.first->
                                 put(px,py,filter_frac[iter]*basic_math::min(
                                 neighbor_rgb[i].first,
                                 neighbor_rgb[j].first,
                                 neighbor_rgb[k].first));
                              RGB_copy_twoDarray.second->
                                 put(px,py,filter_frac[iter]*basic_math::min(
                                 neighbor_rgb[i].second,
                                 neighbor_rgb[j].second,
                                 neighbor_rgb[k].second));
                              RGB_copy_twoDarray.third->
                                 put(px,py,filter_frac[iter]*basic_math::min(
                                 neighbor_rgb[i].third,
                                 neighbor_rgb[j].third,
                                 neighbor_rgb[k].third));
                              central_pixel_black=false;
                           }
                        } // loop over index c labeling 3x3
			  // plaquette's corners
                     }
                     else if (iter==1)
                     {
                     
// Next search for sets of 2 "off-diagonal neighbors" which are
// non-zero valued.  Reset central black pixel to filter_frac of the
// minimal value of these corner neighbors:

                        bool central_pixel_black=true;
                        int i,k;
                        for (unsigned int c=0; c<4; c++)
                        {
                           switch(c)
                           {
                              case 0:
                                 i=3; k=1;
                                 break;
                              case 1:
                                 i=1; k=5;
                                 break;
                              case 2:
                                 i=5; k=7;
                                 break;
                              case 3:
                                 i=7; k=3;
                                 break;
                           }
                     
                           if (neighbor_colored[i] &&
                               neighbor_colored[k] && central_pixel_black)
                           {
                              RGB_copy_twoDarray.first->
                                 put(px,py,filter_frac[iter]*basic_math::min(
                                 neighbor_rgb[i].first,
                                 neighbor_rgb[k].first));
                              RGB_copy_twoDarray.second->
                                 put(px,py,filter_frac[iter]*basic_math::min(
                                 neighbor_rgb[i].second,
                                 neighbor_rgb[k].second));
                              RGB_copy_twoDarray.third->
                                 put(px,py,filter_frac[iter]*basic_math::min(
                                 neighbor_rgb[i].third,
                                 neighbor_rgb[k].third));
                              central_pixel_black=false;
                           }
                        } // loop over index c labeling 3x3
			  // plaquette's corners
                     }
                     else if (iter==2)
                     {
                     
// Finally search for sets of 3 "above", "below", "left" or "right"
// neighbors which are non-zero valued.  Also look for sets of 3
// "below", above", "right" and "left" neighbors which are zero
// valued.  Reset central black pixel to filter_frac of the minimal
// value of the non-zero valued neighbors:

                        bool central_pixel_black=true;
                        int i,j,k;
                        for (unsigned int c=0; c<4; c++)
                        {
                           switch(c)
                           {
                              case 0:
                                 i=0; j=1; k=2;
                                 break;
                              case 1:
                                 i=2; j=5; k=8;
                                 break;
                              case 2:
                                 i=8; j=7; k=6;
                                 break;
                              case 3:
                                 i=6; j=3; k=0;
                                 break;
                           }
                     
                           if (neighbor_colored[i] && neighbor_colored[j] &&
                               neighbor_colored[k] && central_pixel_black)
                           {
                              RGB_copy_twoDarray.first->
                                 put(px,py,filter_frac[iter]*basic_math::min(
                                 neighbor_rgb[i].first,
                                 neighbor_rgb[k].first));
                              RGB_copy_twoDarray.second->
                                 put(px,py,filter_frac[iter]*basic_math::min(
                                 neighbor_rgb[i].second,
                                 neighbor_rgb[k].second));
                              RGB_copy_twoDarray.third->
                                 put(px,py,filter_frac[iter]*basic_math::min(
                                 neighbor_rgb[i].third,
                                 neighbor_rgb[k].third));
                              central_pixel_black=false;
                           }
                        } // loop over index c labeling 3x3
			  // plaquette's corners
                  
                     } // iter index conditional

                  } // current pixel is not black conditional
               } // loop over py index
            } // loop over px index
         
         } // loop over iter index
                  
         copy_RGB_twoDarrays(RGB_copy_twoDarray,RGB_twoDarray);         
         delete RGB_copy_twoDarray.first;
         delete RGB_copy_twoDarray.second;
         delete RGB_copy_twoDarray.third;
      }

// ---------------------------------------------------------------------
// Method transfer_RGB_twoDarrays_to_rowpointers copies the contents
// of "member object" RGB_twoDarray into "member object" rowpointers
// for eventual output as a png file.

   void transfer_RGB_twoDarrays_to_rowpointers()
      {
         unsigned int nxbins=width;
         unsigned int nybins=height;

         Triple<int,int,int> rgb;
         for (unsigned int px=0; px<nxbins; px++)
         {
            for (unsigned int py=0; py<nybins; py++)
            {
               rgb.first=RGB_twoDarray.first->get(px,py);
               rgb.second=RGB_twoDarray.second->get(px,py);
               rgb.third=RGB_twoDarray.third->get(px,py);
               put_pixel_RGB_values(px,py,rgb);
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method smear_RGB_twoDarrays performs a brute-force averaging of
// every pixel over an n_size x n_size plaquette for each of the 3
// twoDarrays within "member object" RGB_twoDarray.  

   void smear_RGB_twoDarrays()
      {
         outputfunc::write_banner("Smearing RGB twoDarrays:");

         unsigned int nxbins=width;
         unsigned int nybins=height;
         RGB_array RGB_copy_twoDarray;
         RGB_copy_twoDarray.first=new twoDarray(nxbins,nybins);
         RGB_copy_twoDarray.second=new twoDarray(nxbins,nybins);
         RGB_copy_twoDarray.third=new twoDarray(nxbins,nybins);
         copy_RGB_twoDarrays(RGB_twoDarray,RGB_copy_twoDarray);

         unsigned int n_size=3;
         const double irrelevant_intensity=POSITIVEINFINITY;
         imagefunc::average_filter(
            n_size,n_size,RGB_twoDarray.first,RGB_copy_twoDarray.first,
            irrelevant_intensity);
         imagefunc::average_filter(
            n_size,n_size,RGB_twoDarray.second,RGB_copy_twoDarray.second,
            irrelevant_intensity);
         imagefunc::average_filter(
            n_size,n_size,RGB_twoDarray.third,RGB_copy_twoDarray.third,
            irrelevant_intensity);
         copy_RGB_twoDarrays(RGB_copy_twoDarray,RGB_twoDarray);
      }

// ---------------------------------------------------------------------
// Methods RGB_to_int and int_to_RGB converts an input RGB triple to a
// unique integer and vice-versa.

   int RGB_to_int(const Triple<int,int,int>& rgb)
      {
         return 256*256*rgb.first + 256*rgb.second+rgb.third;
      }

   Triple<int,int,int> int_to_RGB(int flat)
      {
         Triple<int,int,int> rgb;
         rgb.first=flat/(256*256);
         flat -= rgb.first*256*256;
         rgb.second=flat/256;
         flat -= rgb.second*256;
         rgb.third=flat;
         return rgb;
      }

// ---------------------------------------------------------------------
   Triple<int,int,int> most_frequent_RGB()
      {
         unsigned int nxbins=width;
         unsigned int nybins=height;

         unsigned int n_colors=256*256*256;
         int* flattened_color;
         new_clear_array(flattened_color,n_colors);

// Don't count contribution of pixels colored with null_RGB_value:

         const int null_RGB_value=256*256*256-1;
         for (unsigned int py=0; py<nybins; py++)
         {
            for (unsigned int px=0; px<nxbins; px++)
            {
               int flat=RGB_to_int(get_pixel_RGB_values(px,py));
               if (flat != null_RGB_value)
               {
                  flattened_color[flat]=flattened_color[flat]+1;
               }
            } // loop over px index
         } // loop over py index

         int max_RGB_frequency=-1;
         int n_max=-1;
         for (unsigned int n=0; n<n_colors; n++)
         {
            if (flattened_color[n] > max_RGB_frequency)
            {
               n_max=n;
               max_RGB_frequency=flattened_color[n];
            }
         }
         delete [] flattened_color;

         Triple<int,int,int> rgb=int_to_RGB(n_max);
         cout << "Most frequent RGB = " 
              << rgb.first << "," << rgb.second << "," << rgb.third << endl;
         cout << "Most frequent RGB flattened value = "
              << n_max << endl;
         cout << "Number of pixels colored with most frequent RGB values = "
              << max_RGB_frequency << endl;
         cout << "Frequency fraction = " 
              << double(max_RGB_frequency)/(nxbins*nybins)
              << endl;

         return rgb;
      }

// ---------------------------------------------------------------------
   void recolor_RGB_pixels(
      const Triple<int,int,int>& initial_RGB,
      const Triple<int,int,int>& replacement_RGB)
      {
         unsigned int nxbins=width;
         unsigned int nybins=height;
         for (unsigned int py=0; py<nybins; py++)
         {
            for (unsigned int px=0; px<nxbins; px++)
            {
               Triple<int,int,int> curr_rgb=get_pixel_RGB_values(px,py);
               if (curr_rgb.first==initial_RGB.first &&
                   curr_rgb.second==initial_RGB.second &&
                   curr_rgb.third==initial_RGB.third)
               {
                  put_pixel_RGB_values(px,py,replacement_RGB);
               }
            } // loop over px index
         } // loop over py index
      }
   
// ==========================================================================
// PNGWriter methods
// ==========================================================================

// Method convert_textlines_to_PNG() calls pngwriter methods in order
// to convert multiple lines of input text (passed via STL vector
// text_lines) into a PNG image.

   void convert_textlines_to_PNG(
      const vector<string>& text_lines,string output_PNG_filename,
      colorfunc::Color text_color,double background_greyscale_intensity)
   {
      const int fontsize=50;
      int x_start=1*fontsize;

      unsigned int n_rows=text_lines.size();
      int n_columns=-1;
      for (unsigned int r=0; r<n_rows; r++)
      {
         n_columns=basic_math::max(n_columns,int(text_lines[r].size()));
      }
//      cout << "n_rows = " << n_rows
//           << " n_columns = " << n_columns << endl;

      int npx=fontsize*(n_columns+1);
      int npy=1.4*fontsize*n_rows;
      double angle=0;

//      cout << "npx = " << npx << " npy = " << npy << endl;
      colorfunc::RGB text_rgb=colorfunc::get_RGB_values(text_color);

      string projects_rootdir = getenv("PROJECTSROOT");
      string font_path=projects_rootdir + 
         "/data/OpenSceneGraph-Data/fonts/arial.ttf";

      convert_textlines_to_PNG(
         x_start,0,npx,npy,angle,text_lines,output_PNG_filename,
         text_rgb,background_greyscale_intensity,font_path,fontsize);
   }
   
// ---------------------------------------------------------------------
   void convert_textlines_to_PNG(
      int x_start,int y_offset,int npx,int npy,double angle,
      const vector<string>& text_lines,string output_PNG_filename,
      colorfunc::RGB text_rgb,double background_greyscale_intensity,
      string font_path,int fontsize)
   {
      pngwriter png(
         npx,npy,background_greyscale_intensity,
         output_PNG_filename.c_str());

      for (unsigned int r=0; r<text_lines.size(); r++)
      {
         int y_start=(0.33+(text_lines.size()-1-r)*1.3)*fontsize+y_offset;

         png.plot_text( 
            const_cast<char *>(font_path.c_str()),fontsize,
            x_start,y_start,angle,
            const_cast<char *>(text_lines[r].c_str()),
            text_rgb.first,text_rgb.second,text_rgb.third);
      } // loop over index r labeling text lines
      png.close();
   }
   
// ---------------------------------------------------------------------
   void convert_textlines_to_PNG(
      int npx,int npy,string output_PNG_filename,
      double background_greyscale_intensity,
      string font_path,int fontsize,
      const vector<string>& text_lines,
      const vector<twovector>& xy_start,
      const vector<colorfunc::RGB>& text_RGB)
   {
      pngwriter png(
         npx,npy,background_greyscale_intensity,
         output_PNG_filename.c_str());

      const double angle=0;
      for (unsigned int i=0; i<text_lines.size(); i++)
      {
         int px=xy_start[i].get(0);
         int py=xy_start[i].get(1);

         png.plot_text( 
            const_cast<char *>(font_path.c_str()),fontsize,
            px,py,angle,
            const_cast<char *>(text_lines[i].c_str()),
            text_RGB[i].first,text_RGB[i].second,text_RGB[i].third);
      } // loop over index i labeling text lines

      png.close();
   }
   
// ---------------------------------------------------------------------
// Method add_text_to_PNG_image() imports an existing PNG image.  It
// inserts the string text_line at the pixel location starting at
// px_start,py_start where (0,0) corresponds to the picture's upper
// left corner.  It also takes in angle measured in radians which
// specifies the text's orientation relative to the picture's
// horizontal axis.  Input text_rgb has red,green and blue values
// measured from 0 to 1.  

   void add_text_to_PNG_image(
      string input_PNG_filename,string output_PNG_filename,
      int px_start,int py_start,double angle,
      string text_line,colorfunc::RGB text_rgb,
      string font_path,int fontsize)
   {
      pngwriter image(1,1,0,output_PNG_filename.c_str());
      image.readfromfile(input_PNG_filename.c_str());

      unsigned int width,height;
      imagefunc::get_image_width_height(
         input_PNG_filename,width,height);

      py_start=height-py_start;

      double opacity=1.0;

      image.plot_text_utf8_blend(
         const_cast<char *>(font_path.c_str()),fontsize,
         px_start,py_start,angle,
         const_cast<char *>(text_line.c_str()),opacity,
         text_rgb.first,text_rgb.second,text_rgb.third);
      
      image.close();
   }

// ---------------------------------------------------------------------
// This overloaded version of add_text_to_PNG_image() incorporates
// multiple horizontal lines of text at specified starting pixel
// locations to an input PNG file.

   void add_text_to_PNG_image(
      string input_PNG_filename,string output_PNG_filename,
      int fontsize,
      const vector<string>& text_lines,
      const vector<twovector>& xy_start,
      const vector<colorfunc::RGB>& text_RGB)
   {
      double angle=0;
      string projects_rootdir = getenv("PROJECTSROOT");
      string arial_font_path=projects_rootdir+
         "/data/OpenSceneGraph-Data/fonts/";
      arial_font_path += "arial.ttf";

      pngwriter image(1,1,0,output_PNG_filename.c_str());
      image.readfromfile(input_PNG_filename.c_str());

      unsigned int width,height;
      imagefunc::get_image_width_height(
         input_PNG_filename,width,height);
      
      double opacity=1.0;
      for (unsigned int i=0; i<text_lines.size(); i++)
      {
         int px_start=xy_start[i].get(0);
         int py_start=xy_start[i].get(1);
         py_start=height-py_start;

         image.plot_text_utf8_blend(
            const_cast<char *>(arial_font_path.c_str()),fontsize,
            px_start,py_start,angle,
            const_cast<char *>(text_lines[i].c_str()),opacity,
            text_RGB[i].first,text_RGB[i].second,text_RGB[i].third);
      }
      image.close();      
   }

// ==========================================================================
// PNG conversion methods
// ==========================================================================

   string convert_image_to_PNG(string image_filename)
   {
      string suffix=stringfunc::suffix(image_filename);
      if (suffix == "png" || suffix == "PNG") return image_filename;

      string basename=stringfunc::prefix(image_filename);
      string png_filename=basename+".png";
      string unix_cmd="convert "+image_filename+" "+png_filename;
      sysfunc::unix_command(unix_cmd);
      return png_filename;
   }




} // pngfunc namespace
