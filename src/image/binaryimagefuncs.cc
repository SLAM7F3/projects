// ==========================================================================
// BINARYIMAGEFUNCS stand-alone methods
// ==========================================================================
// Last modified on 4/15/14; 4/17/14; 4/28/14; 5/12/15
// ==========================================================================

#include <iostream>

#include "math/basic_math.h"
#include "image/binaryimagefuncs.h"
#include "math/Genarray.h"
#include "geometry/geometry_funcs.h"
#include "image/graphicsfuncs.h"
#include "datastructures/Linkedlist.h"
#include "general/outputfuncs.h"
#include "geometry/polygon.h"
#include "image/TwoDarray.h"

#include "math/lttwovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::pair;
using std::vector;

namespace binaryimagefunc
{

// ==========================================================================
// Binary image processing methods
// ==========================================================================

// Method locate_first_nonzero_pixel takes in a binary image within
// *zbinary_twoDarray_ptr.  It scans over the entire image and returns
// the pixel coordinates of the first non-zero valued pixel which it
// encounters.

   void locate_first_nonzero_pixel(
      twoDarray const *zbinary_twoDarray_ptr,int& px_first,int& py_first)
   {
      for (unsigned int py=0; py<zbinary_twoDarray_ptr->get_ndim(); py++)
      {
         for (unsigned int px=0; px<zbinary_twoDarray_ptr->get_mdim(); px++)
         {
            if (zbinary_twoDarray_ptr->get(px,py) > 0)
            {
               px_first=px;
               py_first=py;
               return;
            }
         } // loop over py index
      } // loop over px index
   }
   
   
// ---------------------------------------------------------------------
// Method binary_image_pixel_bbox takes in a binary image within
// *zbinary_twoDarray_ptr.  It returns the pixel coordinates
// (px_min,py_min) and (px_max,py_max) of a bounding box enclosing the
// non-zero valued content of *zbinary_twoDarray_ptr.

   void binary_image_pixel_bbox(
      twoDarray const *zbinary_twoDarray_ptr,
      int& px_min,int& px_max,int& py_min,int& py_max)
      {
         unsigned int mdim=zbinary_twoDarray_ptr->get_mdim();
         unsigned int ndim=zbinary_twoDarray_ptr->get_ndim();
         
         bool px_min_found=false;
         for (unsigned int px=0; px<mdim && !px_min_found; px++)
         {
            for (unsigned int py=0; py<ndim && !px_min_found; py++)
            {
               if (zbinary_twoDarray_ptr->get(px,py) > 0)
               {
                  px_min=px;
                  px_min_found=true;
               }
            } // loop over py index
         } // loop over px index

         bool px_max_found=false;
         for (unsigned int px=mdim-1; px>=0 && !px_max_found; px--)
         {
            for (unsigned int py=0; py<ndim && !px_max_found; py++)
            {
               if (zbinary_twoDarray_ptr->get(px,py) > 0)
               {
                  px_max=px;
                  px_max_found=true;
               }
            } // loop over py index
         } // loop over px index

         bool py_min_found=false;
         for (unsigned int py=0; py<ndim && !py_min_found; py++)
         {
            for (unsigned int px=0; px<mdim && !py_min_found; px++)
            {
               if (zbinary_twoDarray_ptr->get(px,py) > 0)
               {
                  py_min=py;
                  py_min_found=true;
               }
            } // loop over px index
         } // loop over py index

         bool py_max_found=false;
         for (unsigned int py=ndim-1; py>=0 && !py_max_found; py--)
         {
            for (unsigned int px=0; px<mdim && !py_max_found; px++)
            {
               if (zbinary_twoDarray_ptr->get(px,py) > 0)
               {
                  py_max=py;
                  py_max_found=true;
               }
            } // loop over px index
         } // loop over py index
      }

// ---------------------------------------------------------------------
// Method binary_threshold scans through a twoDarray and sets all
// pixel intensity values below (above) some specified threshold value
// equal to znull (zfill).

   void binary_threshold(double z_threshold,twoDarray* ztwoDarray_ptr,
   double znull,double zfill)
      {
         twoDarray* zbinary_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         binary_threshold(
            z_threshold,ztwoDarray_ptr,zbinary_twoDarray_ptr,znull,zfill);
         zbinary_twoDarray_ptr->copy(ztwoDarray_ptr);
         delete zbinary_twoDarray_ptr;
      }

   void binary_threshold(
      double z_threshold,const twoDarray* ztwoDarray_ptr,
      twoDarray* zbinary_twoDarray_ptr,double znull,double zfill)
      {
         binary_threshold(
            z_threshold,Unsigned_Zero,ztwoDarray_ptr->get_mdim(),
            Unsigned_Zero,ztwoDarray_ptr->get_ndim(),ztwoDarray_ptr,
            zbinary_twoDarray_ptr,znull,zfill);
      }

   void binary_threshold(
      double z_threshold,unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      const twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr,
      double znull,double zfill)
      {
//         cout << "inside binaryimagefunc::binary_threshold()" << endl;
//         cout << "znull = " << znull << endl;
//         cout << "zfill = " << zfill << endl;
//         cout << "ztwoDarray_ptr = " << ztwoDarray_ptr
//              << " zbinary_twoDarray_ptr = " << zbinary_twoDarray_ptr
//              << endl;
//         cout << "px_min = " << px_min << " px_max = " << px_max << endl;
//         cout << "py_min = " << py_min << " py_max = " << py_max << endl;

// In mid-July 2003, we learned (the extremely painful & hard way!)
// that optimized vs unoptimized codes yield slightly different values
// for "zero".  So in order for results to be independent of
// optimization setting, we reset any "zero" threshold to some small
// positive value...

         const double TINY=1E-5;
         if (fabs(z_threshold) < TINY) z_threshold=TINY;

         int n_ones=0;
         int n_nulls=0;
         for (unsigned int px=px_min; px<px_max; px++)
         {
            for (unsigned int py=py_min; py<py_max; py++)
            {
//               cout << "px = " << px << " py = " << py << " z = "
//                    << ztwoDarray_ptr->get(px,py) << " zthreshold = " 
//                    << z_threshold << endl;
               if (ztwoDarray_ptr->get(px,py) > z_threshold)
               {
                  zbinary_twoDarray_ptr->put(px,py,zfill);
                  n_ones++;
               }
               else
               {
                  zbinary_twoDarray_ptr->put(px,py,znull);
                  n_nulls++;
               }
            } // loop over py index
         } // loop over px index

//         cout << "At end of binaryimagefunc::binary_threshold()" << endl;
//         cout << "n_nulls = " << n_nulls << " n_ones = " << n_ones << endl;
      }

   void binary_threshold_inside_bbox(
      double z_threshold,
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      const twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr,
      double znull)
      {
         unsigned int px_min,px_max,py_min,py_max;
         ztwoDarray_ptr->point_to_pixel(minimum_x,minimum_y,px_min,py_max);
         ztwoDarray_ptr->point_to_pixel(maximum_x,maximum_y,px_max,py_min);

         binary_threshold(z_threshold,px_min,px_max,py_min,py_max,
                          ztwoDarray_ptr,zbinary_twoDarray_ptr,znull);
      }

// ---------------------------------------------------------------------
 void binary_threshold_above_cutoff(
      double z_threshold,const twoDarray* ztwoDarray_ptr,
      twoDarray* zbinary_twoDarray_ptr,double znull)
      {
         binary_threshold_above_cutoff(
            z_threshold,0,ztwoDarray_ptr->get_mdim(),
            0,ztwoDarray_ptr->get_ndim(),ztwoDarray_ptr,
            zbinary_twoDarray_ptr,znull);
      }

   void binary_threshold_above_cutoff(
      double z_threshold,unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      const twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr,
      double znull)
      {

// In mid-July 2003, we learned (the extremely painful & hard way!)
// that optimized vs unoptimized codes yield slightly different values
// for "zero".  So in order for results to be independent of
// optimization setting, we reset any "zero" threshold to be some
// small positive value...

         const double TINY=1E-5;
         if (fabs(z_threshold) < TINY) z_threshold=TINY;
         
         for (unsigned int px=px_min; px<px_max; px++)
         {
            for (unsigned int py=py_min; py<py_max; py++)
            {
               if (ztwoDarray_ptr->get(px,py) < z_threshold)
               {
                  zbinary_twoDarray_ptr->put(px,py,1);
               }
               else
               {
                  zbinary_twoDarray_ptr->put(px,py,znull);
               }
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method binary_threshold_for_particular_cutoff takes in twoDarray
// *ztwoDarray_ptr.  It returns a binary image which equals znull
// everywhere except for those locations where z is very close to
// input value z_threshold.

   void binary_threshold_for_particular_cutoff(
      double z_threshold,const twoDarray* ztwoDarray_ptr,
      twoDarray* zbinary_twoDarray_ptr,double znull)
      {
         const double TINY=1E-5;
         int n_unit_valued_pixels=0;
         int n_null_valued_pixels=0;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (fabs(ztwoDarray_ptr->get(px,py)-z_threshold) < TINY)
               {
                  zbinary_twoDarray_ptr->put(px,py,1);
                  n_unit_valued_pixels++;
               }
               else
               {
                  zbinary_twoDarray_ptr->put(px,py,znull);
                  n_null_valued_pixels++;
               }
            } // loop over py index
         } // loop over px index
//         cout << "n_unit_valued_pixels = " << n_unit_valued_pixels << endl;
//         cout << "n_null_valued_pixels = " << n_null_valued_pixels << endl;
//         cout << "sum = " << n_unit_valued_pixels + n_null_valued_pixels
//              << endl;
//         cout << "mdim*ndim = " << ztwoDarray_ptr->get_mdim()*
//            ztwoDarray_ptr->get_ndim() << endl;
      }

// ---------------------------------------------------------------------
   void abs_binary_threshold(
      double abs_z_threshold,const twoDarray* ztwoDarray_ptr,
      twoDarray* zbinary_twoDarray_ptr,double znull)
      {
         abs_binary_threshold(abs_z_threshold,0,ztwoDarray_ptr->get_mdim(),
                              0,ztwoDarray_ptr->get_ndim(),ztwoDarray_ptr,
                              zbinary_twoDarray_ptr,znull);
      }

   void abs_binary_threshold(
      double abs_z_threshold,unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      const twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr,
      double znull)
      {
         const double TINY=1E-5;
         if (fabs(abs_z_threshold) < TINY) abs_z_threshold=TINY;
         
//         cout << "abs_z_threshold = " << abs_z_threshold << endl;
//         cout << "px_min = " << px_min << " px_max = " << px_max << endl;
//         cout << "py_min = " << py_min << " py_max = " << py_max << endl;

         for (unsigned int px=px_min; px<px_max; px++)
         {
            for (unsigned int py=py_min; py<py_max; py++)
            {
               if (fabs(ztwoDarray_ptr->get(px,py)) > abs_z_threshold)
               {
                  zbinary_twoDarray_ptr->put(px,py,1);
               }
               else
               {
                  zbinary_twoDarray_ptr->put(px,py,znull);
               }
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method binary_dilate()

// semi-minor axes equal to xdilate_dist and ydilate_dist.  Mask
// points lying inside the ellipse are set equal to unity, while those
// on the outside have zero value.  This method subsequently performs
// a brute force convolution of the mask with the input binary image
// within *zbinary_twoDarray_ptr.  Any non-zero value within the
// convolved output is set equal to unity.  The resulting dilated
// image is returned within a dynamically generated twoDarray.  This
// particular implementation of binary dilation smooths sharp object
// corners in the original image.

   twoDarray* binary_dilate(
      int n_size,double znull,twoDarray const *ztwoDarray_ptr)
      {
         cout << "inside binaryimagefunc::binary_dilate()" << endl;
//         cout << "n_size = " << n_size << " znull = " << znull << endl;
         
         twoDarray* zdilated_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         zdilated_twoDarray_ptr->initialize_values(znull);
         
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (!nearly_equal(ztwoDarray_ptr->get(px,py),znull))
               {
                  for (int i=-n_size; i<=n_size; i++)
                  {
                     for (int j=-n_size; j<=n_size; j++)
                     {
                        if (ztwoDarray_ptr->pixel_inside_working_region(
                           px+i,py+j))
                        {
                           zdilated_twoDarray_ptr->put(px+i,py+j,1);
                        }
                     } // loop over moving window index j
                  } // loop over moving window index i
               } // ztwoDarray_ptr(px,py) != znull conditional
            
            } // loop over index py
         } // loop over index px
         return zdilated_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method binary_dilate forms an elliptical mask with semi-major and
// semi-minor axes equal to xdilate_dist and ydilate_dist.  Mask
// points lying inside the ellipse are set equal to unity, while those
// on the outside have zero value.  This method subsequently performs
// a brute force convolution of the mask with the input binary image
// within *zbinary_twoDarray_ptr.  Any non-zero value within the
// convolved output is set equal to unity.  The resulting dilated
// image is returned within a dynamically generated twoDarray.  This
// particular implementation of binary dilation smooths sharp object
// corners in the original image.

   twoDarray* binary_dilate(
      double xdilate_dist,double ydilate_dist,double z_threshold,
      twoDarray const *ztwoDarray_ptr,double znull)
      {
         twoDarray* zdilated_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         zdilated_twoDarray_ptr->initialize_values(znull);
         
         int nx_size=basic_math::round(
            xdilate_dist/ztwoDarray_ptr->get_deltax());
         int ny_size=basic_math::round(
            ydilate_dist/ztwoDarray_ptr->get_deltay());
         xdilate_dist=nx_size*ztwoDarray_ptr->get_deltax();
         ydilate_dist=ny_size*ztwoDarray_ptr->get_deltay();

// Form elliptical dilation mask:

         genarray dilation_mask(2*nx_size+1,2*ny_size+1);
         for (int i=-nx_size; i<=nx_size; i++)
         {
            double x=i*ztwoDarray_ptr->get_deltax();
            for (int j=-ny_size; j<=ny_size; j++)
            {
               double y=j*ztwoDarray_ptr->get_deltay();
               double sqrd_dist=0;
               if (xdilate_dist > 0) sqrd_dist += sqr(x/xdilate_dist);
               if (ydilate_dist > 0) sqrd_dist += sqr(y/ydilate_dist);
               if (sqrd_dist <= 1)
               {
                  dilation_mask.put(i+nx_size,j+ny_size,1);
               }
//               cout << "i = " << i << " j = " << j 
//                    << " x=" << x << " y=" << y
//                    << " sqrd_dist = " << sqrd_dist 
//                    << " mask = " << dilation_mask.get(i+nx_size,j+ny_size)
//                    << endl;
            } // loop over j
         } // loop over i
         
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               bool pixel_near_some_border=false;
               for (int i=-nx_size; i<=nx_size && !pixel_near_some_border; 
                    i++)
               {
                  for (int j=-ny_size; j<=ny_size && 
                          !pixel_near_some_border; j++)
                  {
                     if (dilation_mask.get(i+nx_size,j+ny_size) > 0.5 &&
                         ztwoDarray_ptr->
                         pixel_inside_working_region(px+i,py+j))
                     {
                        if (ztwoDarray_ptr->get(px+i,py+j) > z_threshold)
                        {
                           pixel_near_some_border=true;
                           zdilated_twoDarray_ptr->put(px,py,1);
                        }
                     }
                  } // loop over moving window index j
               } // loop over moving window index i
            } // loop over index py
         } // loop over index px
         return zdilated_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method binary_reverse takes in a binary image and changes every
// zero (unity) value to unity (zero).

   void binary_reverse(
      twoDarray* zbinary_twoDarray_ptr,double znull,double zfill)
      {
         binary_reverse(
            0,zbinary_twoDarray_ptr->get_mdim(),
            0,zbinary_twoDarray_ptr->get_ndim(),
            zbinary_twoDarray_ptr,znull,zfill);
      }

   void binary_reverse(
      unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      twoDarray* zbinary_twoDarray_ptr,double znull,double zfill)
      {
         for (unsigned int px=px_min; px<px_max; px++)
         {
            for (unsigned int py=py_min; py<py_max; py++)
            {
               double curr_z=zbinary_twoDarray_ptr->get(px,py);
               if (curr_z >= znull && curr_z <= zfill)
               {
                  if (nearly_equal(curr_z,znull))
                  {
                     zbinary_twoDarray_ptr->put(px,py,zfill);
                  }
                  else if (nearly_equal(curr_z,zfill))
                  {
                     zbinary_twoDarray_ptr->put(px,py,znull);
                  }
               }
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method binary_filter() takes in binary twoDarray
// *zbinary_twoDarray_ptr and iterates over all of its pixels.  It
// counts the number of unit-valued neighbors within an nx_size x
// ny_size window around each pixel.  If the intensity sum ratio to
// the number of window pixels exceeds input intensity_frac_threshold,
// this method resets the central pixel's binary value to unity.

   int binary_filter(
      unsigned int nx_size,unsigned int ny_size,
      twoDarray const *zbinary_twoDarray_ptr,
      twoDarray *zbinary_twoDarray_filtered_ptr,
      double intensity_frac_threshold)
      {
         if (is_even(nx_size)) nx_size++;
         if (is_even(ny_size)) ny_size++;
         unsigned int npixels=nx_size*ny_size;

         unsigned int n_changes=0;
         unsigned int wx=(nx_size-1)/2;
         unsigned int wy=(ny_size-1)/2;
         for (unsigned int px=wx; px<zbinary_twoDarray_ptr->get_mdim()-wx;
              px++)
         {
            for (unsigned int py=wy; py<zbinary_twoDarray_ptr->get_ndim()-wy;
                 py++)
            {
               int intensity_sum=0;
               for (unsigned int i=0; i<nx_size; i++)
               {
                  for (unsigned int j=0; j<ny_size; j++)
                  {
                     intensity_sum += zbinary_twoDarray_ptr->
                        get(px-wx+i,py-wy+j);
                  }
               } 
               double intensity_frac=double(intensity_sum)/double(npixels);
               if (intensity_frac > intensity_frac_threshold)
               {
                  n_changes++;
                  zbinary_twoDarray_filtered_ptr->put(px,py,1);
               }
            } // loop over index py
         } // loop over index px

         cout << "Number of binary zeros changed to ones = "
              << n_changes << endl;
         return n_changes;
      }

// ---------------------------------------------------------------------
// Method binary_density_filter takes in a binary image within
// *zbinary_twoDarray_ptr.  It runs a quasi-circular filter whose
// diameter in meters is set by input parameter diameter over the
// non-zero elements in the binary image.  This method computes the
// integrated intensity within the filter and compares that to the
// filter's size in pixels.  If their ratio exceeds input parameter
// fill_frac_threshold and if boolean flag
// copy_all_pixels_in_filter==true, this method copies over all pixels
// inside the circular filter onto a new, dynamically generated binary
// image.  If copy_all_pixels_in_filter==false, only the filter's
// center pixel location is copied onto the new binary image.  The
// filtered binary image is returned by this method.

// This method is intended to help eliminate long, connected strands
// and filaments from a binary image whose total area may be large.
// It is designed to retain non-dark regions in a binary image whose
// perimeter/area ratios are relatively small...

   twoDarray* binary_density_filter(
      double diameter,double fill_frac_threshold,
      twoDarray const *zbinary_twoDarray_ptr,
      bool copy_all_pixels_in_filter)
      {
         outputfunc::write_banner("Density filtering binary image:");

         double dx=zbinary_twoDarray_ptr->get_deltax();
         double dy=zbinary_twoDarray_ptr->get_deltay();
         double ds=0.5*(dx+dy);
         unsigned int nrows=basic_math::round(diameter/ds);

         int* column=graphicsfunc::poor_mans_circle_array(nrows);
         int circle_pixel_count=0;
         for (unsigned int r=0; r<nrows; r++)
         {
            circle_pixel_count += column[r];
         }

         twoDarray* zbinary_filtered_twoDarray_ptr=new twoDarray(
            zbinary_twoDarray_ptr);

         for (unsigned int px=nrows/2+1; px<zbinary_twoDarray_ptr->get_mdim()
                 -(nrows/2+1); px++)
         {
            for (unsigned int py=nrows/2+1; py<zbinary_twoDarray_ptr->
                    get_ndim()-(nrows/2+1); py++)
            {
               double curr_z=zbinary_twoDarray_ptr->get(px,py);
               if (curr_z > 0.5)
               {
                  double intensity_sum=0;
                  for (unsigned int r=0; r<nrows; r++)
                  {
                     int j=py+nrows/2-r;
                     for (int c=0; c<column[r]; c++)
                     {
                        int i=px+column[r]/2-c;
                        intensity_sum += zbinary_twoDarray_ptr->get(i,j);
                     }
                  }

                  if (intensity_sum > fill_frac_threshold*circle_pixel_count)
                  {
                     if (copy_all_pixels_in_filter)
                     {
                        for (unsigned int r=0; r<nrows; r++)
                        {
                           int j=py+nrows/2-r;
                           for (int c=0; c<column[r]; c++)
                           {
                              int i=px+column[r]/2-c;
                              zbinary_filtered_twoDarray_ptr->put(
                                 i,j,zbinary_twoDarray_ptr->get(i,j));
                           }
                        }
                     }
                     else
                     {
                        zbinary_filtered_twoDarray_ptr->put(
                           px,py,zbinary_twoDarray_ptr->get(px,py));
                     } // copy_all_pixels_in_filter conditional
                  } // intensity_sum conditional
               } // zbinary(px,py) > 0.5 conditional
               
            } // loop over py index
         } // loop over px index

         delete [] column;
         return zbinary_filtered_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method binary_image_to_list takes in a binary image within
// *zbinary_twoDarray_ptr.  It returns the pixel coordinates of those
// pixels within the image which are non-zero valued and which lie
// within the pixel ranges px_min <= px < px_max, py_min <= py <
// py_max within the output linked list.

   Linkedlist<pair<int,int> >* binary_image_to_list(
      twoDarray const *zbinary_twoDarray_ptr)
      {
         return binary_image_to_list(
            0,zbinary_twoDarray_ptr->get_mdim(),
            0,zbinary_twoDarray_ptr->get_ndim(),zbinary_twoDarray_ptr);
      }
   
   Linkedlist<pair<int,int> >* binary_image_to_list(
      unsigned int px_min,unsigned int px_max,
      unsigned int py_min,unsigned int py_max,
      twoDarray const *zbinary_twoDarray_ptr)
      {
         Linkedlist<pair<int,int> >* pixel_list_ptr=
            new Linkedlist<pair<int,int> >;
         pair<int,int> pixel_pair;
         
         for (unsigned int px=px_min; px<px_max; px++)
         {
            for (unsigned int py=py_min; py<py_max; py++)
            {
               if (zbinary_twoDarray_ptr->get(px,py) > 0)
               {
                  pixel_pair.first=px;
                  pixel_pair.second=py;
                  pixel_list_ptr->append_node(pixel_pair);
               }
            } // loop over py index
         } // loop over px index

         if (pixel_list_ptr->size()==0)
         {
            delete pixel_list_ptr;
            pixel_list_ptr=NULL;
         }
         
         return pixel_list_ptr;
      }

// ==========================================================================
// Euler number methods
// ==========================================================================

// Method pixel_to_quad() takes in coordinates px,py for the upper
// left corner of some 2x2 quad along with binary image
// *pbinary_twoDarray_ptr.  It fills the top-left, top-right,
// bottom-left and bottom-right quad entries with appropriate
// intensity values.

   void pixel_to_quad(
      int px,int py,int pfill,twoDarray const *pbinary_twoDarray_ptr,
      int& tl, int& tr, int& bl, int& br)
   {
      int xdim=pbinary_twoDarray_ptr->get_xdim();
      int ydim=pbinary_twoDarray_ptr->get_ydim();
      
// Compute top-left, top-right, bottom-left & bottom-right quad
// values:

      tl=tr=bl=br=-1;

// Treat pixels lying outside legitimate bounds for
// *cc_twoDarray_ptr as zero-valued:
         
      if (px < 0)
      {
         tl=bl=0;
      }
         
      if (px+1 >= xdim) 
      {
         tr=br=0;
      }

      if (py < 0) 
      {
         tr=tl=0;
      }
         
      if (py+1 >= ydim) 
      {
         bl=br=0;
      }
      
      if (tl < 0) tl=pbinary_twoDarray_ptr->get(px,py);
      if (tr < 0) tr=pbinary_twoDarray_ptr->get(px+1,py);
      if (bl < 0) bl=pbinary_twoDarray_ptr->get(px,py+1);
      if (br < 0) br=pbinary_twoDarray_ptr->get(px+1,py+1);

// Ignore any quad entries which do not equal pfill:

      if (tl != pfill)
      {
         tl=0;
      }
            
      if (tr != pfill) 
      {
         tr=0;
      }

      if (bl != pfill)
      {
         bl=0;
      }

      if (br != pfill) 
      {
         br=0;
      }
   }

// ---------------------------------------------------------------------
// Method count_quads returns the number of "one" quads ( 10 , 01 , 00 , 00)
//							  00   00   10   01

// "two" quads ( 11  ,  10 , 00 , 01 ), "three quads ( 11 , 11 , 01 , 10 )
//		 00	10   11   01		       10   01   11   11

// and "diagonal quads  ( 10 , 01 )  
//			  01   10 

// contained within the input quad  tl tr
//				    bl br

   void count_quads(
      int tl,int tr,int bl,int br,int pfill,
      int& n_Q1, int& n_Q2, int& n_Q3, int& n_Q4, int& n_QD)
   {
//      cout << "inside binaryimagefunc::count_quads()" << endl;
      
      n_Q1=0;
      n_Q2=0;
      n_Q3=0;
      n_Q4=0;
      n_QD=0;
       
      int p_sum=tl+tr+bl+br;
      if (p_sum==1*pfill) n_Q1++;
      if (p_sum==3*pfill) n_Q3++;
      if (p_sum==4*pfill) n_Q4++;

      if (tl==pfill && tr==pfill && bl==0 && br==0)
      {
         n_Q2++;
      }
      else if (tl==pfill && tr==0 && bl==pfill && br==0)
      {
         n_Q2++;
      }
      else if (tl==0 && tr==0 && bl==pfill && br==pfill)
      {
         n_Q2++;
      }
      else if (tl==0 && tr==pfill && bl==0 && br==pfill)
      {
         n_Q2++;
      }

      if (tl==pfill && br==pfill && tr==0 && bl==0)
      {
         n_QD++;
      }
      if (bl==pfill && tr==pfill && tl==0 && br==0)
      {
         n_QD++;
      }

//      cout << "n_Q1 = " << n_Q1 << " n_Q3 = " << n_Q3 << " n_Q4 = " << n_Q4
//           << " n_QD = " << n_QD << endl;
   }

// ---------------------------------------------------------------------
// Section V entitled "The Square Lattice" in "Local properties of
// binary images in two dimensions" by Gray (IEEE Trans on Computers,
// Vol C-20, 1971) explains in detail multiple defns for Euler number.  
// In particular if "diagonal quads" 1 0   and   0 1
//				     0 1	 1 0
// are regarded as FULL connections, then

// 			E = 0.25 * (nQ1 - nQ3 - 2 * nQD)

   double quad_Euler_number_contribution(
      int tl,int tr,int bl,int br,int pfill)
   {
//      cout << "inside binaryimagefunc::quad_Euler_number_contribution()" 
//           << endl;

      int n_Q1=0;
      int n_Q2=0;
      int n_Q3=0;
      int n_Q4=0;
      int n_QD=0;
      count_quads(tl,tr,bl,br,pfill,n_Q1,n_Q2,n_Q3,n_Q4,n_QD);

      double quad_Euler_number=0.25*(n_Q1-n_Q3-2*n_QD); 
      return quad_Euler_number;
   }

// ---------------------------------------------------------------------
   double quad_perimeter_contribution(
      int tl,int tr,int bl,int br,int pfill)
   {
      int n_Q1=0;
      int n_Q2=0;
      int n_Q3=0;
      int n_Q4=0;
      int n_QD=0;
      count_quads(tl,tr,bl,br,pfill,n_Q1,n_Q2,n_Q3,n_Q4,n_QD);

      double quad_perimeter=n_Q1+n_Q2+n_Q3+2*n_QD;
      return quad_perimeter;
   }

// ---------------------------------------------------------------------
   double quad_area_contribution(
      int tl,int tr,int bl,int br,int pfill)
   {
      int n_Q1=0;
      int n_Q2=0;
      int n_Q3=0;
      int n_Q4=0;
      int n_QD=0;
      count_quads(tl,tr,bl,br,pfill,n_Q1,n_Q2,n_Q3,n_Q4,n_QD);

      double quad_area=0.25*(n_Q1+2*n_Q2+3*n_Q3+4*n_Q4+2*n_QD);
      return quad_area;
   }

// ---------------------------------------------------------------------
// Method image_Euler_number_perimeter_area() performs a brute-force
// loop over all quads (including those which partially lie outside
// the image by one pixel!) for the input binary image contained
// within *twoDarray_ptr.  It uses the quads to compute the image's
// Euler number, perimeter and area.

   void image_Euler_number_perimeter_area(
      int pfill, twoDarray* pbinary_twoDarray_ptr,double& Euler_number,
      double& perimeter, double& area)
   {
      Euler_number=perimeter=area=0;
      int width = pbinary_twoDarray_ptr->get_mdim();
      int height = pbinary_twoDarray_ptr->get_ndim();
      
      for (int py=-1; py<height; py++)
      {
         for (int px=-1; px<width; px++)
         {
            int tl,tr,bl,br;
            pixel_to_quad(px,py,pfill,pbinary_twoDarray_ptr,tl,tr,bl,br);
            Euler_number += quad_Euler_number_contribution(tl,tr,bl,br,pfill);
            perimeter += quad_perimeter_contribution(tl,tr,bl,br,pfill);
            area += quad_area_contribution(tl,tr,bl,br,pfill);
         } // loop over px
      } // loop over py
//      cout << "image Euler number = " << Euler_number << endl;
//      cout << "image perimeter = " << perimeter << endl;
//      cout << "image area = " << area << endl;
   }

// ---------------------------------------------------------------------
// Method quad_euler_number_contribution() takes in coordinates
// px,py for the upper left corner of some 2x2 quad along with
// binary image *pbinary_twoDarray_ptr.  It computes pfill valued
// pixel contributions to the quad's Q1, Q3 and Qdiag sums.  Any
// pixel whose value does not equal pfill is ignored.

// This method returns the linear combination 0.25[ N{Q1} - N{Q3} + 2N{QDiag}]
// which corresponds to the input quad's contribution to a binary
// image's Euler number.  See section 18.2.1 in Digital Image
// Processing: PIKS Inside, Third Edition. William K. Pratt Copyright
// © 2001 John Wiley & Sons, Inc.


   double quad_Euler_number_contribution(
      int px,int py,int pfill,twoDarray const *cc_twoDarray_ptr)
   {
//      cout << "inside binaryimagefunc::quad_Euler_number_contribution()" 
//           << endl;
//      cout << "px = " << px << " py = " << py << endl;
      
      int xdim=cc_twoDarray_ptr->get_xdim();
      int ydim=cc_twoDarray_ptr->get_ydim();
      
// Compute top-left, top-right, bottom-left & bottom-right quad
// values:

      int tl,tr,bl,br;  
      tl=tr=bl=br=-1;

// Treat pixels lying outside legitimate bounds for
// *cc_twoDarray_ptr as zero-valued:
         
      if (px < 0)
      {
         tl=bl=0;
      }
         
      if (px+1 >= xdim) 
      {
         tr=br=0;
      }

      if (py < 0) 
      {
         tr=tl=0;
      }
         
      if (py+1 >= ydim) 
      {
         bl=br=0;
      }
      
      if (tl < 0) tl=cc_twoDarray_ptr->get(px,py);
      if (tr < 0) tr=cc_twoDarray_ptr->get(px+1,py);
      if (bl < 0) bl=cc_twoDarray_ptr->get(px,py+1);
      if (br < 0) br=cc_twoDarray_ptr->get(px+1,py+1);

// Ignore any quad entries which do not equal pfill:

      if (tl != pfill)
      {
         tl=0;
      }
            
      if (tr != pfill) 
      {
         tr=0;
      }

      if (bl != pfill)
      {
         bl=0;
      }

      if (br != pfill) 
      {
         br=0;
      }

      return quad_Euler_number_contribution(tl,tr,bl,br,pfill);
   }

// ---------------------------------------------------------------------
   double Euler_number_contribution_from_single_pixel(
      int px,int py,int pfill,twoDarray* cc_twoDarray_ptr)
   {
//      cout << "inside Euler_number_contrib_from_single_pixel()" << endl;
//      cout << "px = " << px << " py = " << py << endl;

      double delta_Euler1=quad_Euler_number_contribution(
         px-1,py-1,pfill,cc_twoDarray_ptr);
      double delta_Euler2=quad_Euler_number_contribution(
         px,py-1,pfill,cc_twoDarray_ptr);
      double delta_Euler3=quad_Euler_number_contribution(
         px-1,py,pfill,cc_twoDarray_ptr);
      double delta_Euler4=quad_Euler_number_contribution(
         px,py,pfill,cc_twoDarray_ptr);
      double delta_Euler=delta_Euler1+delta_Euler2+delta_Euler3+delta_Euler4;
      return delta_Euler;
   }

// ---------------------------------------------------------------------
// Method delta_Euler_number_for_single_pixel() takes in 
// a binary image along with some pixel (px,py) whose value
// will change from zero to pfill.  It computes and returns the change
// in Euler number for an extremal region associated with this single
// pixel modification.  The value for pixel (px,py) is changed by this
// method from 0 to pfill.

   double delta_Euler_number_for_single_pixel(
      int px,int py,int pfill,twoDarray* cc_twoDarray_ptr)
   {
//      cout << "inside binaryimagefunc::delta_Euler_number_for_single_pixel()" 
//           << endl;

      cc_twoDarray_ptr->put(px,py,0);
      double delta_Euler_before=Euler_number_contribution_from_single_pixel(
         px,py,pfill,cc_twoDarray_ptr);

      cc_twoDarray_ptr->put(px,py,pfill);
      double delta_Euler_after=Euler_number_contribution_from_single_pixel(
         px,py,pfill,cc_twoDarray_ptr);

      double delta_Euler_number=delta_Euler_after - delta_Euler_before;
      return delta_Euler_number;
   }

// ---------------------------------------------------------------------
   double Euler_number_contribution_from_single_pixel(
      int px,int py,int pfill,vector<int>& neighbor_values)
   {
      double Euler_tl=
         binaryimagefunc::quad_Euler_number_contribution(
            neighbor_values[0],neighbor_values[1],
            neighbor_values[3],neighbor_values[4],pfill);
      double Euler_tr=
         binaryimagefunc::quad_Euler_number_contribution(
            neighbor_values[1],neighbor_values[2],
            neighbor_values[4],neighbor_values[5],pfill);
      double Euler_bl=
         binaryimagefunc::quad_Euler_number_contribution(
            neighbor_values[3],neighbor_values[4],
            neighbor_values[6],neighbor_values[7],pfill);
      double Euler_br=
         binaryimagefunc::quad_Euler_number_contribution(
            neighbor_values[4],neighbor_values[5],
            neighbor_values[7],neighbor_values[8],pfill);
      double Euler_pixel=Euler_tl+Euler_tr+Euler_bl+Euler_br;
      return Euler_pixel;
   }

// ---------------------------------------------------------------------
   double delta_Euler_number_for_single_pixel(
      int px,int py,int pfill,vector<int>& neighbor_values)
   {
//      cout << "inside binaryimagefunc::delta_Euler_number_for_single_pixel()//" //
      //        << endl;

      neighbor_values[4] = 0;
      double delta_Euler_before=Euler_number_contribution_from_single_pixel(
         px,py,pfill,neighbor_values);

      neighbor_values[4] = pfill;
      double delta_Euler_after=Euler_number_contribution_from_single_pixel(
         px,py,pfill,neighbor_values);

      double delta_Euler_number=delta_Euler_after - delta_Euler_before;
      return delta_Euler_number;
   }

// ---------------------------------------------------------------------
// Method compute_delta_Euler_number() takes in binary image
// *cc_twoDarray_ptr whose null pixels are assumed to have zero
// values.  It also takes in an STL vector of pixel coordinates
// which are assumed to have recently changed values from null to
// non-zero values.  This method returns the change in Euler number
// associated with all pixels in pixel_coords having turned on from
// zero to pfill.  Entries within pixel_IDs and *cc_twoDarray_ptr with
// pixel values not equal to pfill are ignored.

// Note: On 7/31/12, we explicitly confirmed that 2x2 quads
// extending beyond *cc_twoDarray_ptr into px=-1, py=-1, px=mdim and
// py=ndim must be included into Euler number computations!

// As of April 2014, this method is deprecated.  Calls should instead
// be made to Euler_number_contribution_from_single_pixel() and/or
// delta_Euler_number_for_single_pixel().

   double compute_delta_Euler_number(
      vector<pair<int,int> >& pixel_coords,int pfill,
      twoDarray* cc_twoDarray_ptr,twoDarray* visited_twoDarray_ptr)
   {
//      cout << "inside binaryimagefunc::new_compute_delta_Euler_number()"
//           << endl;
//      cout << "pfill = " << pfill << endl;

// First set values of all pixels corresponding to input pixel_IDs to
// zero within *cc_twoDarray_ptr:

      for (unsigned int i=0; i<pixel_coords.size(); i++)
      {
         int px=pixel_coords[i].first;
         int py=pixel_coords[i].second;
         if (int(cc_twoDarray_ptr->get(px,py)) == pfill) 
         {
            cc_twoDarray_ptr->put(px,py,0);
         }
      }

//      cout << "BEFORE" << endl;
      double delta_Euler_before=0;
      for (unsigned int i=0; i<pixel_coords.size(); i++)
      {
         int px=pixel_coords[i].first;
         int py=pixel_coords[i].second;
//         cout << "px = " << px << " py = " << py << endl;

         if (int(cc_twoDarray_ptr->get(px,py)) != 0) continue;      

// Mark upper left corner pixel for each 2x2 quad which is
// visited:

         int pxm1=(px-1)+1;
         int pym1=(py-1)+1;
         int pxz=(px)+1;
         int pyz=(py)+1;
         if (int(visited_twoDarray_ptr->get(pxm1,pym1)) != pfill)
         {
            delta_Euler_before += quad_Euler_number_contribution(
               px-1,py-1,pfill,cc_twoDarray_ptr);
            visited_twoDarray_ptr->put(pxm1,pym1,pfill);
         }
//         else
//         {
//            cout << "already visited" << endl;
//            cout << "px-1 = " << px-1 << " py-1 = " << py-1 << endl;
//         }

         if (int(visited_twoDarray_ptr->get(pxz,pym1)) != pfill)
         {
            delta_Euler_before += quad_Euler_number_contribution(
               px,py-1,pfill,cc_twoDarray_ptr);
            visited_twoDarray_ptr->put(pxz,pym1,pfill);
         }
//         else
//         {
//            cout << "already visited" << endl;
//            cout << "px = " << px << " py-1 = " << py-1 << endl;
//         }
         
         if (int(visited_twoDarray_ptr->get(pxm1,pyz)) != pfill)
         {
            delta_Euler_before += quad_Euler_number_contribution(
               px-1,py,pfill,cc_twoDarray_ptr);
            visited_twoDarray_ptr->put(pxm1,pyz,pfill);
         }
//         else
//         {
//            cout << "already visited" << endl;
//            cout << "px-1 = " << px-1 << " py = " << py << endl;
//         }
         
         if (int(visited_twoDarray_ptr->get(pxz,pyz)) != pfill)
         {
            delta_Euler_before += quad_Euler_number_contribution(
               px,py,pfill,cc_twoDarray_ptr);
            visited_twoDarray_ptr->put(pxz,pyz,pfill);
         }
//         else
//         {
//            cout << "already visited" << endl;
//            cout << "px = " << px << " py = " << py << endl;
//         }
         
      } // index i labeling changed pixels
//      cout << "delta_Euler_before = " << delta_Euler_before << endl;

// Next reset values of all pixels corresponding to input pixel_IDs
// back to pfill within *cc_twoDarray_ptr:

      for (unsigned int i=0; i<pixel_coords.size(); i++)
      {
         int px=pixel_coords[i].first;
         int py=pixel_coords[i].second;

         if (int(cc_twoDarray_ptr->get(px,py)) == 0) 
         {
            cc_twoDarray_ptr->put(px,py,pfill);
         }
      }

//      cout << "AFTER" << endl;
      double delta_Euler_after=0;
      for (unsigned int i=0; i<pixel_coords.size(); i++)
      {
         int px=pixel_coords[i].first;
         int py=pixel_coords[i].second;
         if (int(cc_twoDarray_ptr->get(px,py)) != pfill) continue;

         int pxm1=(px-1)+1;
         int pym1=(py-1)+1;
         int pxz=(px)+1;
         int pyz=(py)+1;
         if (int(visited_twoDarray_ptr->get(pxm1,pym1)) != -pfill)
         {
            delta_Euler_after += quad_Euler_number_contribution(
               px-1,py-1,pfill,cc_twoDarray_ptr);
            visited_twoDarray_ptr->put(pxm1,pym1,-pfill);
         }

         if (int(visited_twoDarray_ptr->get(pxz,pym1)) != -pfill)
         {
            delta_Euler_after += quad_Euler_number_contribution(
               px,py-1,pfill,cc_twoDarray_ptr);
            visited_twoDarray_ptr->put(pxz,pym1,-pfill);
         }

         if (int(visited_twoDarray_ptr->get(pxm1,pyz)) != -pfill)
         {
            delta_Euler_after += quad_Euler_number_contribution(
               px-1,py,pfill,cc_twoDarray_ptr);
            visited_twoDarray_ptr->put(pxm1,pyz,-pfill);
         }
         
         if (int(visited_twoDarray_ptr->get(pxz,pyz)) != -pfill)
         {
            delta_Euler_after += quad_Euler_number_contribution(
               px,py,pfill,cc_twoDarray_ptr);
            visited_twoDarray_ptr->put(pxz,pyz,-pfill);
         }
         
      } // index i labeling changed pixels
//      cout << "delta_Euler_after = " << delta_Euler_after << endl;

      double delta_Euler=delta_Euler_after-delta_Euler_before;

//      cout << "delta_Euler_after = " << delta_Euler_after
//           << " delta_Euler_before = " << delta_Euler_before 
//           << " delta_Euler = " << delta_Euler << endl;
      return delta_Euler;
   }

// ==========================================================================
// Convex hull methods
// ==========================================================================

// Method compute_convex_hull() takes in binary twoDarray
// *zbinary_twoDarray_ptr whose entries are assumed to equal
// either 0 or 1.  It extracts the unit-valued pixel locations to an
// STL vector.  The convex hull of these pixels is returned within
// a dynamically instantiated polygon.

   polygon* compute_convex_hull(const twoDarray* zbinary_twoDarray_ptr)
   {
      vector<twovector> V;
      for (unsigned int px=0; px<zbinary_twoDarray_ptr->get_mdim(); px++)
      {
         for (unsigned int py=0; py<zbinary_twoDarray_ptr->get_ndim(); py++)
         {
            if (zbinary_twoDarray_ptr->get(px,py) > 0.5)
            {
               V.push_back(twovector(px,py));
            }
         } // loop over py index
      } // loop over px index
      
      return geometry_func::compute_convex_hull(V);
   }


      
} // binaryimagefunc namespace

