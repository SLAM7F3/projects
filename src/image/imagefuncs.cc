// ==========================================================================
// IMAGEFUNCS stand-alone methods
// ==========================================================================
// Last modified on 3/31/16; 4/8/16; 6/1/16; 8/2/16
// ==========================================================================

#include <algorithm>	
#include <iostream>
#include <unistd.h>	// Needed for sleep command
#include "math/adv_mathfuncs.h"
#include "color/colorfuncs.h"
#include "image/compositefuncs.h"
#include "geometry/convexhull.h"
#include "datastructures/dataarray.h"
#include "delaunay/delaunay.h"
#include "image/drawfuncs.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "geometry/frustum.h"
#include "math/genmatrix.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "geometry/linesegment.h"
#include "math/mathfuncs.h"
#include "image/myimage.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "geometry/parallelepiped.h"
#include "plot/plotfuncs.h"
#include "geometry/polygon.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "math/threevector.h"
#include "time/timefuncs.h"
#include "image/TwoDarray.h"
#include "geometry/voronoifuncs.h"

#include "numrec/nrfuncs.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

using std::ostringstream;

namespace imagefunc
{

// ==========================================================================
// Metafile manipulation methods
// ==========================================================================

// Method maximize_pixel_density "oversamples" the contents of the
// input twoDarray *ztwoDarray_ptr.  It first determines the maximum
// number of integer times mx and my the densities in the cross range
// and range directions could be increased without exceeding the
// limits set input parameters nx_max and ny_max.  Member variables
// nxbins(_new) and nybins(_new) are then multiplied by mx and my, and
// a new, enlarged twoDarray *ztwoDarray_new_ptr is dynamically
// generated.  The method copies the contents of *ztwoDarray_ptr onto
// *ztwoDarray_new_ptr.  Every single pixel of *ztwoDarray_ptr is
// mapped into mx X my pixels in *ztwoDarray_new_ptr.  The
// *ztwoDarray_ptr array is then deleted, while ztwoDarray_new_ptr is
// returned by this method.

// Radar imagery data looks identical when it is displayed at higher
// pixel resolution.  But obviously, line drawings superimposed on top
// of the data look much finer and nicer to the eye when the
// resolution is increased.

   void maximize_pixel_density(
      int nx_max,int ny_max,twoDarray*& ztwoDarray_ptr) 
      {
         ztwoDarray_ptr=maximize_pixel_density(ztwoDarray_ptr,nx_max,ny_max);
      }

   twoDarray* maximize_pixel_density(
      twoDarray* ztwoDarray_ptr,int nx_max,int ny_max)
      {
// integer horizontal density increase:
         int mx=basic_math::mytruncate(nx_max/ztwoDarray_ptr->get_mdim());  
// integer vertical density increase
         int my=basic_math::mytruncate(ny_max/ztwoDarray_ptr->get_ndim());   

//   cout << "mx = " << mx << endl;
//   cout << "my = " << my << endl;

         ztwoDarray_ptr->set_mdim_tmp(ztwoDarray_ptr->get_mdim_tmp()*mx);
         ztwoDarray_ptr->set_ndim_tmp(ztwoDarray_ptr->get_ndim_tmp()*my);

// Dynamically create a new,enlarged twoDarray:

         twoDarray* ztwoDarray_new_ptr=new twoDarray(
            ztwoDarray_ptr->get_mdim_tmp(),ztwoDarray_ptr->get_ndim_tmp());
         ztwoDarray_new_ptr->set_xlo(ztwoDarray_ptr->get_xlo());
         ztwoDarray_new_ptr->set_xhi(ztwoDarray_ptr->get_xhi());
         ztwoDarray_new_ptr->set_deltax(
            (ztwoDarray_ptr->get_xhi()-ztwoDarray_ptr->get_xlo())/
            (ztwoDarray_ptr->get_mdim_tmp()-1));
         ztwoDarray_new_ptr->set_ylo(ztwoDarray_ptr->get_ylo());
         ztwoDarray_new_ptr->set_yhi(ztwoDarray_ptr->get_yhi());
         ztwoDarray_new_ptr->set_deltay(
            (ztwoDarray_ptr->get_yhi()-ztwoDarray_ptr->get_ylo())/
            (ztwoDarray_ptr->get_ndim_tmp()-1));

// Place output of oversampling ztwoDarray into ztwoDarray_new:

         for (unsigned int i=0; i<ztwoDarray_new_ptr->get_mdim(); i++)
         {
            for (unsigned int j=0; j<ztwoDarray_new_ptr->get_ndim(); j++)
            {
               ztwoDarray_new_ptr->put(i,j,ztwoDarray_ptr->get(
                  basic_math::mytruncate(i/mx),
                  basic_math::mytruncate(j/my)));
            }
         }

// Delete original ztwoDarray object.  Then return new, enlarged
// version of ztwoDarray which now resides within ztwoDarray_new:

         delete ztwoDarray_ptr;
         return ztwoDarray_new_ptr;
      }

// ---------------------------------------------------------------------
   void add_secret_labels(
      ofstream& imagestream,twoDarray const *ztwoDarray_ptr)
      {
         imagestream << "textcolor 'red'" << endl;
         imagestream << "textsize 2" << endl;
         imagestream << "text "+stringfunc::number_to_string(
            1.1*ztwoDarray_ptr->get_xhi(),2)+" "
            +stringfunc::number_to_string(0.91*ztwoDarray_ptr->get_ylo(),2)
            +" 'SECRET'" << endl;
         imagestream << "text "+stringfunc::number_to_string(
            1.45*ztwoDarray_ptr->get_xlo(),2)+" "
            +stringfunc::number_to_string(0.85*ztwoDarray_ptr->get_yhi(),2)
            +" 'SECRET'" << endl;
         imagestream << endl;
      }

// ==========================================================================
// Image cropping methods:
// ==========================================================================

// Method crop_image queries the user to specify the values of xlo,
// xhi, ylo and yhi which are used as bounds on the current image.
// This function should be called immediately before an image is
// written to metafile output.  We implemented this method on 7/24/01
// primarily for viewgraph generation purposes.

   void crop_image(twoDarray *ztwoDarray_ptr)
      {
         double xlo,xhi,ylo,yhi;
         ztwoDarray_ptr->set_xhi_orig(ztwoDarray_ptr->get_xhi());
         ztwoDarray_ptr->set_xlo_orig(ztwoDarray_ptr->get_xlo());
         ztwoDarray_ptr->set_yhi_orig(ztwoDarray_ptr->get_yhi());
         ztwoDarray_ptr->set_ylo_orig(ztwoDarray_ptr->get_ylo());

         cout << "Enter cropping bound xlo:" << endl;
         cin >> xlo;
         cout << "Enter cropping bound xhi:" << endl;
         cin >> xhi;
         cout << "Enter cropping bound ylo:" << endl;
         cin >> ylo;
         cout << "Enter cropping bound yhi:" << endl;
         cin >> yhi;

         ztwoDarray_ptr->set_xlo(xlo);
         ztwoDarray_ptr->set_xhi(xhi);
         ztwoDarray_ptr->set_ylo(ylo);
         ztwoDarray_ptr->set_yhi(yhi);

         ztwoDarray_ptr->set_mdim_tmp(basic_math::round((
            ztwoDarray_ptr->get_xhi()-ztwoDarray_ptr->get_xlo())/
            ztwoDarray_ptr->get_deltax())+1);

         if (is_odd(ztwoDarray_ptr->get_mdim()
                    -ztwoDarray_ptr->get_mdim_tmp())) 
         {
            ztwoDarray_ptr->set_mdim_tmp(ztwoDarray_ptr->get_mdim_tmp()-1);
         }

         ztwoDarray_ptr->set_ndim_tmp(
            basic_math::round((ztwoDarray_ptr->get_yhi()-
                               ztwoDarray_ptr->get_ylo())/
                              ztwoDarray_ptr->get_deltay())+1);
         if (is_odd(ztwoDarray_ptr->get_ndim()-
                    ztwoDarray_ptr->get_ndim_tmp()))
         {
            ztwoDarray_ptr->set_ndim_tmp(ztwoDarray_ptr->get_ndim_tmp()-1);
         }
      }

// ---------------------------------------------------------------------

   void crop_image(string filename,int width,int height)
      {
         crop_image(filename, width, height, 0, 0);
      }
   
   void crop_image(
      string filename,int width,int height,int xoffset,int yoffset)
   {
      crop_image(filename, filename, width, height, xoffset, yoffset);
   }
   
   void crop_image(
      string input_filename, string output_filename,
      int width, int height, int xoffset, int yoffset)
   {
      Magick::Image image;
      image.read(input_filename);
      Magick::Geometry crop_geometry(width, height, xoffset, yoffset);
      image.crop(crop_geometry);
      image.write(output_filename);
   }

// ---------------------------------------------------------------------
// Method extract_subimage() invokes ImageMagick's convert -extract
// command in order to excise a specified region from an input image
// to an output file.

   void extract_subimage(
      string input_filename,string output_filename,
      int width,int height,int xoffset,int yoffset)
      {
//         cout << "inside imagefunc::extract_subimage()" << endl;
         string unixcommandstr="convert -quality 100 -extract "
            +stringfunc::number_to_string(width)+"x"
            +stringfunc::number_to_string(height)+"+"
            +stringfunc::number_to_string(xoffset)+"+"
            +stringfunc::number_to_string(yoffset)+" "
            +input_filename+" "+output_filename;
//         cout << "unix command = " << unixcommandstr << endl;
         sysfunc::unix_command(unixcommandstr);
      }

// This overloaded version of extract_subimage() takes in a bounding
// box defined in UV space.  It crops that region out from the
// specified input image file.

   void extract_subimage(
      double Ulo,double Uhi,double Vlo,double Vhi,
      string input_filename,string output_filename)
      {
         unsigned int xdim,ydim;
         get_image_width_height(input_filename,xdim,ydim);
         int height=(Vhi-Vlo)*ydim;
         int width=(Uhi-Ulo)*ydim;
         int xoffset=Ulo*ydim;
         int yoffset=(1-Vhi)*ydim;
         extract_subimage(
            input_filename,output_filename,width,height,xoffset,yoffset);
      }

// ---------------------------------------------------------------------
// Method recolor_image utilizes ImageMagick's recolor matrix in order
// to alter the relative RGB values within an input image.  Every RGB
// values is multipled by 3x3 genmatrix *RGB_transform_ptr before the
// the output file is rewritten on top of the input file.  We wrote
// this method in Dec 2008 in order to increase red and decrease blue
// content in OSG screen captures.  Empirically we found that
// RGB_transform = diag (1.25 , 1 , 0.85) appears to yield reasonable
// coloring results.

   void recolor_image(
      string filename,genmatrix* RGB_transform_ptr)
      {
         string unixcommandstr="convert -recolor '";
         for (unsigned int i=0; i<RGB_transform_ptr->get_mdim(); i++)
         {
            for (unsigned int j=0; j<RGB_transform_ptr->get_ndim(); j++)
            {
               unixcommandstr += stringfunc::number_to_string(
                  RGB_transform_ptr->get(i,j))+" ";
            }
         }
         unixcommandstr += "' "+filename+" "+filename;
         sysfunc::unix_command(unixcommandstr);
      }

   void crop_and_recolor_image(
      string filename,int width,int height,int xoffset,int yoffset,
      genmatrix* RGB_transform_ptr)
      {
         string unixcommandstr="convert -crop "
            +stringfunc::number_to_string(width)+"x"
            +stringfunc::number_to_string(height)+"+"
            +stringfunc::number_to_string(xoffset)+"+"
            +stringfunc::number_to_string(yoffset)+" ";
         unixcommandstr += "-quality 100 ";
         unixcommandstr += "-recolor '";
         for (unsigned int i=0; i<RGB_transform_ptr->get_mdim(); i++)
         {
            for (unsigned int j=0; j<RGB_transform_ptr->get_ndim(); j++)
            {
               unixcommandstr += stringfunc::number_to_string(
                  RGB_transform_ptr->get(i,j))+" ";
            }
         }
         unixcommandstr += "' "+filename+" "+filename;
         cout << "unixcommandstr = " << unixcommandstr << endl;
         sysfunc::unix_command(unixcommandstr);
      }

// ---------------------------------------------------------------------
// Method convert_image_to_pgm() takes in some standard image file,
// strips off its suffix, and adds a new pgm suffix.  After issuing a
// unix system call to ImageMagick's Convert utility, this method
// returns the name of the pgm image filename.  We wrote this method
// in Oct 2009 in order to experiment with Lowe's binary executable of
// his SIFT feature extraction.

   string convert_image_to_pgm(string filename)
      {
//         cout << "inside convert_image_to_pgm(), filename = " << filename
//              << endl;
         string filename_prefix=stringfunc::prefix(filename);
         string output_filename=filename_prefix+".pgm";
//         cout << "output_filename = " << output_filename << endl;

         string unixcommandstr="convert "+filename+" "+output_filename;
//         cout << "unixcommandstr = " << unixcommandstr << endl;
         sysfunc::unix_command(unixcommandstr);

         return output_filename;
      }

// ==========================================================================
// Geometrical primitives manipulation methods
// ==========================================================================

// Method find_null_border_along_ray takes in a line segment l along
// with an image within *ztwoDarray_ptr which is assumed to contain
// some well defined and isolated hot pixel cluster.  This method
// first computes the integrated intensity along with entire ray.  It
// then locates the point along the ray for which the partial line
// integral equals some fixed large fraction (e.g. 95%) of the total
// line integral.  We take this point to mark the border between the
// hot pixel's interior and its cold, dark exterior along the ray.

   void find_null_border_along_ray(
      double intensity_integral_frac,twoDarray const *ztwoDarray_ptr,
      const linesegment& l,threevector& svec,bool integrate_binary_image)
      {
         double ds=0.5*basic_math::min(ztwoDarray_ptr->get_deltax(),
                           ztwoDarray_ptr->get_deltay());

         const double min_z=1;
         bool point_inside_image;
         double intensity_integral=0;
         int n=0;
         do
         {
            svec=l.get_v1()+n*ds*l.get_ehat();
            double interpolated_intensity_value;
            point_inside_image=ztwoDarray_ptr->point_to_interpolated_value(
               svec.get(0),svec.get(1),interpolated_intensity_value);
            if (integrate_binary_image && 
                interpolated_intensity_value > min_z)
            {
               interpolated_intensity_value=1;
            }
            if (point_inside_image)
               intensity_integral += interpolated_intensity_value;
            n++;
         } 
         while (point_inside_image);
         
         n=0;
//         const double intensity_integral_frac=0.95;
         double partial_intensity_integral=0;
         do
         {
            svec=l.get_v1()+n*ds*l.get_ehat();
            double interpolated_intensity_value;
            point_inside_image=ztwoDarray_ptr->point_to_interpolated_value(
               svec.get(0),svec.get(1),interpolated_intensity_value);
            if (integrate_binary_image && 
                interpolated_intensity_value > min_z)
            {
               interpolated_intensity_value=1;
            }
            if (point_inside_image) 
               partial_intensity_integral += interpolated_intensity_value;
            n++;
         } 
         while (point_inside_image && 
                partial_intensity_integral < intensity_integral_frac*
                intensity_integral);
      }

// ---------------------------------------------------------------------
// Method line_integral_along_segment integrates the image data stored
// within input *ztwoDarray_ptr along the input line segment l using
// Simpson's rule.  We interpolate the intensity values entering into
// the line integral sum, for the points at which the function is
// evaluated generally do not coincide with pixel centers:

   double line_integral_along_segment(
      twoDarray const *ztwoDarray_ptr,const linesegment& l)
      {
         const unsigned int max_nsteps=5000;

         double ds=0.5*basic_math::min(ztwoDarray_ptr->get_deltax(),
                           ztwoDarray_ptr->get_deltay());
         double true_nsteps=l.get_length()/ds;
         unsigned int nsteps=basic_math::mytruncate(true_nsteps);
         if (nsteps > max_nsteps)
         {
            cout << "Error inside line_integral_along_segment()!" << endl;
            cout << "nsteps = " << nsteps << " exceeds max_nsteps = "
                 << max_nsteps << endl;
         }
         double frac=true_nsteps-nsteps;

         double interpolated_intensity_value;
         double currz[max_nsteps];
         for (unsigned int i=0; i<=nsteps; i++)
         {
            threevector svec(l.get_v1()+i*ds*l.get_ehat());
            if (ztwoDarray_ptr->point_to_interpolated_value(
               svec.get(0),svec.get(1),interpolated_intensity_value))
            {
               currz[i]=interpolated_intensity_value;
            }
            else
            {
               currz[i]=0;
            }
         } // loop over index i

// True arc length between points v1 and v2 is generically not an
// integer number of steps.  So we set the line integral between v1
// and v2 equal to a weighted average of the Simpson sums with nsteps
// bins plus a fractional remainder of the integral within the nsteps+1st
// bin:

         double line_integral=mathfunc::simpsonsum(currz,0,nsteps)*ds;
         line_integral += frac*currz[nsteps]*ds;
         return line_integral;
      }

// ---------------------------------------------------------------------
// This variant of the line_integral_along_segment method uses the
// "midpoint lie algorithm" in order to efficiently compute intensity
// line integrals.  (See the "draw_line()" method for comparison.)
// While it is not quite as accurate as the previous
// line_integral_along_segment method, it is more economical.

   double fast_line_integral_along_segment(
      twoDarray const *ztwoDarray_ptr,const linesegment& l)
      {
         unsigned int px_start,py_start,px_stop,py_stop;
         ztwoDarray_ptr->point_to_pixel(
            l.get_v1().get(0),l.get_v1().get(1),px_start,py_start);
         ztwoDarray_ptr->point_to_pixel(
            l.get_v2().get(0),l.get_v2().get(1),px_stop,py_stop);

         unsigned int px=px_start;
         unsigned int py=py_start;
         int dx=px_stop-px_start;
         int dy=-(py_stop-py_start);
         int counter=0;
         int abs_dx=abs(dx);
         int abs_dy=abs(dy);
         int sgn_dx=sgn(dx);
         int sgn_dy=sgn(dy);
         double intensity_sum=0;

         if (abs_dx > abs_dy)
         {
            int d=2*abs_dy-abs_dx;
            int increE=2*abs_dy;
            int increNE=2*(abs_dy-abs_dx);
            while (sgn_dx*px < sgn_dx*px_stop)
            {
               px += sgn_dx;
               if (d <= 0)
               {
                  d += increE;
               }
               else
               {
                  d += increNE;
                  py -= sgn_dy;
               }
               if (ztwoDarray_ptr->pixel_inside_working_region(px,py))
               {
                  intensity_sum += ztwoDarray_ptr->get(px,py);
               }
               counter++;
            } // while loop over px
         }
         else
         {
            int d=abs_dy-2*abs_dx;
            int increN=-2*abs_dx;
            int increNE=2*(abs_dy-abs_dx);
            while (sgn_dy*py > sgn_dy*py_stop)
            {
               py -= sgn_dy;
               if (d >= 0)
               {
                  d += increN;
               }
               else
               {
                  d += increNE;
		  px += sgn_dx;
               }
               if (ztwoDarray_ptr->pixel_inside_working_region(px,py))
               {
                  intensity_sum += ztwoDarray_ptr->get(px,py);
               }
               counter++;
            } // while loop over py
         } // abs(dx) > abs(dy) conditional

         double delta_s=0;
         if (counter > 0)
         {
            delta_s=l.get_length()/counter;
         }
         return (intensity_sum*delta_s);
      }

// ---------------------------------------------------------------------
// This variant of the line_integral_along_segment method uses the
// "midpoint lie algorithm" in order to efficiently compute intensity
// line integrals.  (See the "draw_line()" method for comparison.)
// While it is not quite as accurate as the previous
// line_integral_along_segment method, it is more economical.  This
// method ignores all pixels within input *ztwoDarray_ptr whose value
// equal input parameter null_value.  It returns the number of pixels
// contained within the thick line segment as well as the intensity
// integral within the output STL pair.

   pair<int,double> fast_thick_line_integral_along_segment(
      double line_thickness,const double null_value,
      twoDarray const *ztwoDarray_ptr,const linesegment& l)
      {
         int line_thickness_in_pixels=line_thickness/
            basic_math::min(ztwoDarray_ptr->get_deltax(),
            ztwoDarray_ptr->get_deltay());
         if (is_even(line_thickness_in_pixels)) line_thickness_in_pixels++;
         int npixels_inside_thick_line=0;

// Compute tangent of input line segment's direction vector.  If
// tangent's absolute value is greater than unity, line segment
// basically runs more vertically than horizontally.  In this case, we
// should evaluate multiple line integrals along segments which are
// horizontally offset from each other.  Otherwise, we evaluate
// multiple line integrals along segments which are vertically offset
// from each other:

         int px_offset=0;
         int py_offset=1;
         if (nearly_equal(l.get_ehat().get(0),0))
         {
            px_offset=1;
            py_offset=0;
         }
         else if (fabs(l.get_ehat().get(1)/l.get_ehat().get(0)) > 1)
         {
            px_offset=1;
            py_offset=0;
         }
         unsigned int px_start,py_start,px_stop,py_stop;
         ztwoDarray_ptr->point_to_pixel(
            l.get_v1().get(0),l.get_v1().get(1),px_start,py_start);
         ztwoDarray_ptr->point_to_pixel(
            l.get_v2().get(0),l.get_v2().get(1),px_stop,py_stop);

         unsigned int px=px_start;
         unsigned int py=py_start;
         int dx=px_stop-px_start;
         int dy=-(py_stop-py_start);
         int abs_dx=abs(dx);
         int abs_dy=abs(dy);
         int sgn_dx=sgn(dx);
         int sgn_dy=sgn(dy);
         double intensity_sum=0;

         if (abs_dx > abs_dy)
         {
            int d=2*abs_dy-abs_dx;
            int increE=2*abs_dy;
            int increNE=2*(abs_dy-abs_dx);
            while (sgn_dx*px < sgn_dx*px_stop)
            {
               px += sgn_dx;
               if (d <= 0)
               {
                  d += increE;
               }
               else
               {
                  d += increNE;
                  py -= sgn_dy;
               }

               for (int i=-line_thickness_in_pixels/2; 
                    i<line_thickness_in_pixels/2; i++)
               {
                  unsigned int qx=px+i*px_offset;
                  unsigned int qy=py+i*py_offset;
                  if (ztwoDarray_ptr->pixel_inside_working_region(qx,qy))
                  {
                     double curr_z=ztwoDarray_ptr->get(qx,qy);
                     if (!nearly_equal(curr_z,null_value))
                     {
                        npixels_inside_thick_line++;
                        intensity_sum += ztwoDarray_ptr->get(qx,qy);
                     }
                  }
               }
            } // while loop over px
         }
         else
         {
            int d=abs_dy-2*abs_dx;
            int increN=-2*abs_dx;
            int increNE=2*(abs_dy-abs_dx);
            while (sgn_dy*py > sgn_dy*py_stop)
            {
               py -= sgn_dy;
               if (d >= 0)
               {
                  d += increN;
               }
               else
               {
                  d += increNE;
		  px += sgn_dx;
               }

               for (int i=-line_thickness_in_pixels/2; 
                    i<line_thickness_in_pixels/2; i++)
               {
                  unsigned int qx=px+i*px_offset;
                  unsigned int qy=py+i*py_offset;
                  if (ztwoDarray_ptr->pixel_inside_working_region(qx,qy))
                  {
                     double curr_z=ztwoDarray_ptr->get(qx,qy);
                     if (!nearly_equal(curr_z,null_value))
                     {
                        npixels_inside_thick_line++;
                        intensity_sum += ztwoDarray_ptr->get(qx,qy);
                     }
                  }
               }
            } // while loop over py
         } // abs(dx) > abs(dy) conditional

         return pair<int,double>(npixels_inside_thick_line,intensity_sum);
      }

// ---------------------------------------------------------------------
// Method count_bright_and_dark_pixels_along_segment returns the
// number of pixels along input linesegment l within the image
// contained in *ztwoDarray_ptr whose intensities lie above and below
// the input threshold intensity_value:

   void count_bright_and_dark_pixels_along_segment(
      double threshold,int& nbright_pixels,int& ndark_pixels,
      linesegment& l,twoDarray const *ztwoDarray_ptr)
      {
         double ds=0.5*basic_math::min(ztwoDarray_ptr->get_deltax(),
                           ztwoDarray_ptr->get_deltay());
         double interpolated_intensity_value,currz;

         nbright_pixels=ndark_pixels=0;
         unsigned int nsteps=basic_math::round(l.get_length()/ds);
         for (unsigned int i=0; i<=nsteps; i++)
         {
            threevector svec(l.get_v1()+i*ds*l.get_ehat());
            if (ztwoDarray_ptr->point_to_interpolated_value(
               svec.get(0),svec.get(1),interpolated_intensity_value))
            {
               currz=interpolated_intensity_value;
            }
            else
            {
               currz=0;
            }

            if (currz > threshold)
            {
               nbright_pixels++;
            }
            else
            {
               ndark_pixels++;
            }
         } // loop over index i labeling point along line segment l
      }

// ---------------------------------------------------------------------
// Method surface_integral_inside_polygon takes in a polygon p_3D and
// projects it down onto the x-y plane.  The vertices of a bounding
// box surrounding the polygon's 2D projection p_2D are next
// determined.  The overlap of each pixel within this bounding box and
// p_2D and the differential area 0 < dA < dx*dy of their intersection
// are calculated.  This method returns the area of p_2D along with
// the integrated flux of zarray through p_2D.

   double surface_integral_inside_polygon(
      twoDarray const *ztwoDarray_ptr,const polygon& p_3D)
      {
         twoDarray* zbinary_twoDarray_ptr=NULL;
         double area_integral;
         return surface_integral_inside_polygon(
            ztwoDarray_ptr,zbinary_twoDarray_ptr,p_3D,0,area_integral);
      }

   double surface_integral_inside_polygon(
      twoDarray const *ztwoDarray_ptr,twoDarray const *zbinary_twoDarray_ptr,
      const polygon& p_3D,double black_pixel_penalty,double& area_integral)
      {
         unsigned int min_px,min_py,max_px,max_py;
         polygon p_2D=p_3D.xy_projection();
         ztwoDarray_ptr->locate_extremal_xy_pixels(
            p_2D,min_px,min_py,max_px,max_py);

// Integrate intensity_values of pixels enclosed within p_2D:

         bool pixel_inside_poly,pixel_outside_poly,pixel_on_perimeter;
         area_integral=0;
         double intensity_surface_integral=0;
         threevector currpoint;
         for (unsigned int px=basic_math::max(Unsigned_Zero,min_px-1); 
              px<basic_math::min(max_px+1,ztwoDarray_ptr->
              get_mdim()); px++)
         {
            for (unsigned int py=basic_math::max(Unsigned_Zero,min_py-1); 
                 py<basic_math::min(max_py+1,ztwoDarray_ptr->get_ndim()); py++)
            {
               ztwoDarray_ptr->pixel_relative_to_poly(
                  px,py,currpoint,p_2D,pixel_inside_poly,pixel_outside_poly,
                  pixel_on_perimeter);

               double dA=0;
               if (pixel_outside_poly)
               {
                  dA=0;
               }
               else if (pixel_inside_poly)
               {
                  dA=ztwoDarray_ptr->get_deltax()*
                     ztwoDarray_ptr->get_deltay();
               }
               else if (pixel_on_perimeter)
               {
                  dA=0.5*ztwoDarray_ptr->get_deltax()*
                     ztwoDarray_ptr->get_deltay();
               }
         
               area_integral += dA;
               if (black_pixel_penalty==0)
               {
                  intensity_surface_integral += ztwoDarray_ptr->get(px,py)*dA;
               }
               else
               {
                  if (zbinary_twoDarray_ptr->get(px,py) > 0)
                  {
                     intensity_surface_integral += ztwoDarray_ptr->
                        get(px,py)*dA;
                  }
                  else
                  {
                     intensity_surface_integral -= black_pixel_penalty*dA;
                  }
               }
            }	   // py loop
         }	// px loop
         return intensity_surface_integral;
      }

// ---------------------------------------------------------------------
// Method surface_integral_inside_n_polgyons takes in an array of
// polygons p_3D and projects each member down onto the x-y plane.
// The vertices of a bounding box surrounding all of the polygons' 2D
// projections p_2D are next determined.  This method then scans
// through each pixel within the bounding box.  If the pixel lies
// inside either the intersection of of the polygons in p_2D, the
// contribution of that pixel is added to the integrated flux of
// zarray through the common intersection of all the polygons.

   double surface_integral_inside_n_polygons(
      bool poly_intersection,bool poly_union,unsigned int npolys,
      double zminimum,twoDarray const *ztwoDarray_ptr,const polygon p_3D[],
      double black_pixel_penalty,double& area_integral)
      {
         bool pixel_inside_all_polys,pixel_inside_at_least_one_poly;
         double intensity_surface_integral,dA;

         polygon p_2D[npolys];

// First project each 3D polygon inside p_3D down onto image plane:

         for (unsigned int n=0; n<npolys; n++)
         {
            p_2D[n]=p_3D[n].xy_projection();
         }

// Next locate extremal bounding box points which enclose either
// intersection or union of all polygons inside p_2D array:

         unsigned int min_px,min_py,max_px,max_py;
         ztwoDarray_ptr->locate_extremal_xy_pixels(
            poly_intersection,poly_union,npolys,
            p_2D,min_px,min_py,max_px,max_py);

// Scan through each pixel in bounding box.  If a given pixel lies
// inside all [at least one] of the polygons within the p_2D array,
// include its contribution to the total flux surface integral if
// poly_intersection [poly_union] flag equals true:


         area_integral=intensity_surface_integral=0;
         threevector currpoint;

         for (unsigned int px=min_px-1; px<max_px+1; px++)
         {
            for (unsigned int py=min_py-1; py<max_py+1; py++)
            {
               ztwoDarray_ptr->pixel_inside_n_polys(
                  px,py,currpoint,npolys,p_2D,
                  pixel_inside_all_polys,pixel_inside_at_least_one_poly);
               if ((poly_intersection &&  pixel_inside_all_polys) ||
                   (poly_union && pixel_inside_at_least_one_poly))
               {
                  dA=ztwoDarray_ptr->get_deltax()*ztwoDarray_ptr->
                     get_deltay();
               }
               else 
               {
                  dA=0;
               }

               area_integral += dA;
               if (black_pixel_penalty==0)
               {
                  intensity_surface_integral += ztwoDarray_ptr->get(px,py)*dA;
               }
               else
               {
                  if (ztwoDarray_ptr->get(px,py) > zminimum)
                  {
                     intensity_surface_integral += 
                        ztwoDarray_ptr->get(px,py)*dA;
                  }
                  else
                  {
                     intensity_surface_integral -= black_pixel_penalty*dA;
                  }
               }
            }	   // py loop
         }	// px loop
         return intensity_surface_integral;
      }

// ---------------------------------------------------------------------
// Method n_poly_moment_relative_to_line takes in an array of polygons
// which are assumed to lie within the xy plane along with a line
// segment.  For each pixel lying within these polygons, it calculates
// the product of their intensity times the moment-order-th power of
// their perpendicular distance from the line intensity.
// n_poly_moment_relative_to_line returns the integral of these
// moments.

   double n_poly_moment_relative_to_line(
      bool poly_intersection,bool poly_union,int npolys,double zminimum,
      twoDarray const *ztwoDarray_ptr,int moment_order,
      const linesegment& l,polygon p_2D[],
      double black_pixel_penalty,double& area_integral)
      {


//   polygon p_2D[npolys];

// First project each 3D polygon inside p_3D down onto image plane:

//   for (n=0; n<npolys; n++)
//   {
//      p_2D[n]=p_3D[n].xy_projection();
//   }

// Next locate extremal bounding box points which enclose either
// intersection or union of all polygons inside p_2D array:

         unsigned int min_px,min_py,max_px,max_py;
         ztwoDarray_ptr->locate_extremal_xy_pixels(
            poly_intersection,poly_union,npolys,
            p_2D,min_px,min_py,max_px,max_py);

// Scan through each pixel in bounding box.  If a given pixel lies
// inside all [at least one] of the polygons within the p_2D array,
// include its contribution to the total flux surface integral if
// poly_intersection [poly_union] flag equals true:

         double intensity_surface_integral=0;
         area_integral=0;
         threevector currpoint;
         for (unsigned int px=min_px-1; px<max_px+1; px++)
         {
            for (unsigned int py=min_py-1; py<max_py+1; py++)
            {
               if (ztwoDarray_ptr->pixel_inside_at_least_one_of_n_polys(
                  px,py,currpoint,npolys,p_2D))
               {
                  double dA=ztwoDarray_ptr->get_deltax()*ztwoDarray_ptr->
                     get_deltay();
                  area_integral += dA;
                  double bperp=l.point_to_line_distance(currpoint);
                  if (black_pixel_penalty==0)
                  {
                     intensity_surface_integral += 
                        ztwoDarray_ptr->get(px,py)*
                        mathfunc::real_power(bperp,moment_order)*dA;
                  }
                  else
                  {
                     if (ztwoDarray_ptr->get(px,py) > zminimum)
                     {
                        intensity_surface_integral += 
                           ztwoDarray_ptr->get(px,py)*
                           mathfunc::real_power(bperp,moment_order)*dA;
                     }
                     else
                     {
                        intensity_surface_integral -= 
                           black_pixel_penalty*
                           mathfunc::real_power(bperp,moment_order)*dA;
                     }
                  }
               }
            }	   // py loop
         }	// px loop

         return intensity_surface_integral;
      }

// ==========================================================================
// Intensity distribution methods
// ==========================================================================

// Method intensity_distribution_inside_bbox scans through the pixels
// located within a bounding box defined by pixel range min_px < px <
// max_px and min_py < py < max_py.  It computes a probability
// distribution for those pixels' intensities between some zmin
// intensity value and maxz.  This method is faster than
// intensity_distribution_inside_poly(), for no polygon interior
// checking is performed.  If no pixels inside the bounding box have
// intensities greater than zmin, this boolean method returns false.

   bool intensity_distribution_inside_bbox(
      double minimum_x,double minimum_y,
      double maximum_x,double maximum_y,
      twoDarray const *ztwoDarray_ptr,double zmin,prob_distribution& prob)
      {
         const int n_output_bins=30; // Number of bins in probability dist

         bool distribution_calculated_successfully=false;
         unsigned int min_px,max_px,min_py,max_py;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            int npixels_greater_than_zmin=0;
            double intensity[(max_px-min_px+2)*(max_py-min_py+2)];
            unsigned int pxlo=basic_math::max(Unsigned_Zero,min_px-1);
            unsigned int pxhi=basic_math::min(
               ztwoDarray_ptr->get_mdim(),max_px+1);
            unsigned int pylo=basic_math::max(Unsigned_Zero,min_py-1);
            unsigned int pyhi=basic_math::min(
               ztwoDarray_ptr->get_ndim(),max_py+1);
            for (unsigned int px=pxlo; px<pxhi; px++)
            {
               for (unsigned int py=pylo; py<pyhi; py++)
               {
                  if (ztwoDarray_ptr->get(px,py) > zmin)
                  {
                     intensity[npixels_greater_than_zmin]=ztwoDarray_ptr->
                        get(px,py);
                     npixels_greater_than_zmin++;
                  }
               }  // py loop
            }	// px loop
            if (npixels_greater_than_zmin > 0)
            {
               prob.fill_existing_distribution(
                  npixels_greater_than_zmin,intensity,n_output_bins);
               distribution_calculated_successfully=true;
            }
         }
//   cout << "npixels greater than zmin = " 
//        << npixels_greater_than_zmin << endl;
//   cout << "min_px = " << min_px << " max_px = " << max_px << endl;
//   cout << "min_py = " << min_py << " max_py = " << max_py << endl;

         return distribution_calculated_successfully;
      }

// ---------------------------------------------------------------------
// Method intensity_distribution_inside_convex_quadrilateral takes in
// a polygon which is assumed to be a convex quadrilateral.  It writes
// out a binary mask for this polygon to the memory location specified
// by zmask_twoDarray_ptr.  In order to save time (especially in
// loops), the necessary memory for *zmask_twoDarray_ptr is assumed to
// have already been pre-allocated prior to the call to this method.
// This specialized method then calls the more general
// intensity_distribution_inside_poly method.

   bool intensity_distribution_inside_convex_quadrilateral(
      double zmin,const polygon& quadrilateral,
      twoDarray const *ztwoDarray_ptr,twoDarray *zmask_twoDarray_ptr,
      prob_distribution& prob)
      {
         int npixels_inside_poly,npixels_above_zmin;
         return intensity_distribution_inside_convex_quadrilateral(
            zmin,quadrilateral,ztwoDarray_ptr,zmask_twoDarray_ptr,
            npixels_inside_poly,npixels_above_zmin,prob);
      }
   
   bool intensity_distribution_inside_convex_quadrilateral(
      double zmin,const polygon& quadrilateral,
      twoDarray const *ztwoDarray_ptr,twoDarray *zmask_twoDarray_ptr,
      int& npixels_inside_poly,int& npixels_above_zmin,
      prob_distribution& prob)
      {
         unsigned int min_px,max_px,min_py,max_py;
         zmask_twoDarray_ptr->locate_extremal_xy_pixels(
            quadrilateral,min_px,min_py,max_px,max_py);
         drawfunc::color_convex_quadrilateral_interior(
            quadrilateral,60,zmask_twoDarray_ptr);
         return intensity_distribution_inside_poly(
            min_px,min_py,max_px,max_py,
            zmin,ztwoDarray_ptr,zmask_twoDarray_ptr,
            npixels_inside_poly,npixels_above_zmin,prob);
      }

// ---------------------------------------------------------------------
// Method intensity_distribution_inside_polygon takes in a binary mask
// of some polygon within input *zmask_twoDarray_ptr.  It scans over
// the bounding box defined by input integers min_px, min_py, max_px
// and max_py which is assumed to enclose the polygon.  If the mask
// function is non-zero at a certain pixel location, the intensity
// value at the same pixel location within input *ztwoDarray_ptr is
// added to the intensity probability distribution if it exceeds zmin.
// If the resulting intensity distribution is empty, this boolean
// method returns false.

   bool intensity_distribution_inside_poly(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      double zmin,twoDarray const *ztwoDarray_ptr,
      twoDarray const *zmask_twoDarray_ptr,
      int& npixels_inside_poly,int& npixels_above_zmin,
      prob_distribution& prob)
      {
         npixels_inside_poly=0;
         npixels_above_zmin=0;
         double intensity[
            ztwoDarray_ptr->get_mdim()*ztwoDarray_ptr->get_ndim()];

         for (unsigned int px=min_px; px<max_px; px++)
         {
            for (unsigned int py=min_py; py<max_py; py++)
            {
               if (zmask_twoDarray_ptr->get(px,py) > 0)
               {
                  npixels_inside_poly++;
                  double currz=ztwoDarray_ptr->get(px,py);
                  if (currz > zmin)
                  {
                     intensity[npixels_above_zmin]=currz;
                     npixels_above_zmin++;
                  }
               }
            } // loop over index py
         }  // loop over index px

//         cout << "Number of pixels inside poly = " << npixels_inside_poly
//              << endl;
//         cout << "Number of nonzero pixels inside poly = "
//              << npixels_above_zmin << endl;
//         cout << "Integrated intensity inside poly = " << intensity_integral 
//              << endl;
//         if (npixels_above_zmin > 0)
//         {
//            cout << "Avg intensity inside poly = " 
//                 << intensity_integral/npixels_above_zmin << endl;
//         }

         bool distribution_calculated_successfully=false;
         if (npixels_above_zmin > 1)
         {
            const int n_output_bins=30;  // Number of bins in probability dist
            prob.fill_existing_distribution(
               npixels_above_zmin,intensity,n_output_bins);
//            prob.set_cumulativefilenamestr("poly_cum_intensity.meta");
//            prob.set_densityfilenamestr("poly_dens_intensity.meta");
//            prob.set_xlabel("Relative Normalized Intensity (dB)");
//            prob.xmin=0;
//            prob.writeprobdists();
            distribution_calculated_successfully=true;
         }
         return distribution_calculated_successfully;
      }

// ---------------------------------------------------------------------
// This overloaded version of method
// intensity_distribution_inside_polygon is generally much slower than
// the newer one above.  It scans through the pixels located within
// the input 2D xy projection p_2D of input polygon p_3D.  It computes
// a probability distribution for those pixels' intensities between
// some zmin intensity value and maxz.  If no pixels inside or on the
// perimeter of the projected polygon p_2D exist, this boolean method
// returns false.

   bool intensity_distribution_inside_poly(
      twoDarray const *ztwoDarray_ptr,double zmin,
      const polygon& p_3D,prob_distribution& prob)
      {
         const int n_output_bins=30;  // Number of bins in probability dist
   
         polygon p_2D=p_3D.xy_projection();
         unsigned int min_px,min_py,max_px,max_py;
         ztwoDarray_ptr->locate_extremal_xy_pixels(
            p_2D,min_px,min_py,max_px,max_py);

         bool pixel_inside_poly,pixel_outside_poly,pixel_on_perimeter;
         int npixels_inside_poly=0;
         double intensity[
            (max_px-min_px+1)*(max_py-min_py+1)];
         threevector currpoint;
         for (unsigned int px=basic_math::max(Unsigned_Zero,min_px-1); 
              px<basic_math::min(ztwoDarray_ptr->get_mdim(),
                                 max_px+1); px++)
         {
            for (unsigned int py=basic_math::max(Unsigned_Zero,min_py-1); 
                 py<basic_math::min(ztwoDarray_ptr->get_ndim(),
                                    max_py+1); py++)
            {
               ztwoDarray_ptr->pixel_relative_to_poly(
                  px,py,currpoint,p_2D,pixel_inside_poly,pixel_outside_poly,
                  pixel_on_perimeter);
               if (pixel_inside_poly && ztwoDarray_ptr->get(px,py) > zmin)
               {
                  intensity[npixels_inside_poly]=ztwoDarray_ptr->get(px,py);
                  npixels_inside_poly++;
               }
            }	   // py loop
         }	// px loop

// If npixels_inside_poly still equals 0, then 2D projection p_2D of
// the input 3D polygon p_3D is so thin that all interior pixels are
// classified as lying on the perimeter.  In this case, we redo the
// intensity computation taking into account pixels lying on the
// perimeter of p_2D:

         if (npixels_inside_poly==0)
         {
            for (unsigned int px=basic_math::max(Unsigned_Zero,min_px-1); 
                 px<basic_math::min(ztwoDarray_ptr->get_mdim(),
                                    max_px+1); px++)
            {
               for (unsigned int py=basic_math::max(Unsigned_Zero,min_py-1); 
                    py<basic_math::min(ztwoDarray_ptr->get_ndim(),
                                       max_py+1); py++)
               {
                  ztwoDarray_ptr->pixel_relative_to_poly(
                     px,py,currpoint,p_2D,pixel_inside_poly,
                     pixel_outside_poly,pixel_on_perimeter);

                  if (pixel_on_perimeter && ztwoDarray_ptr->get(px,py) > zmin)
                  {
                     intensity[npixels_inside_poly]=
                        ztwoDarray_ptr->get(px,py);
                     npixels_inside_poly++;
                  }
               } // py loop
            }	// px loop
         }

         bool distribution_calculated_successfully=false;
         if (npixels_inside_poly > 0)
         {
            prob.fill_existing_distribution(npixels_inside_poly,intensity,
                                            n_output_bins);
//            prob.cumulativefilenamestr=imagedir+"poly_intensity"
//               +stringfunc::number_to_string(imagenumber)+".meta";
            prob.set_xlabel("Relative Normalized Intensity (dB)");
            prob.set_xmin(0);
//            prob.writeprobdists();
            distribution_calculated_successfully=true;
         }
         return distribution_calculated_successfully;
      }

// ---------------------------------------------------------------------
// Method intensity_distribution_outside_polygon nulls out polygon
// p_3D within input twoDarray *ztwoDarray_ptr.  It then returns the
// intensity distribution corresponding to the altered image.

   void intensity_distribution_outside_poly(
      twoDarray const *ztwoDarray_ptr,double zmin,
      const polygon& p_3D,prob_distribution& prob)
      {
         polygon p_2D=p_3D.xy_projection();
         twoDarray* ztwoDarray_wo_poly_ptr=new twoDarray(*ztwoDarray_ptr);
         drawfunc::color_polygon_interior(p_2D,0,ztwoDarray_wo_poly_ptr);
         image_intensity_distribution(zmin,ztwoDarray_wo_poly_ptr,prob);
         delete ztwoDarray_wo_poly_ptr;
      }

// ---------------------------------------------------------------------
// Method intensity_distribution_inside_n_polys takes in an array of
// polygons p_2D which are assumed to be projected within the xy
// plane.  The vertices of a bounding box surrounding all of the
// projected polygons are next determined.  This method then scans
// through each pixel within the bounding box.  If the pixel lies
// inside either the intersection or union of the polygons in p_2D,
// the contribution of that pixel is added to the intensity
// probability distribution, provided the pixel's intensity exceeds
// some specified zmin intensity value.

   void intensity_distribution_inside_n_polys(
      bool poly_intersection,bool poly_union,
      int npolys,twoDarray const *ztwoDarray_ptr,double zmin,
      polygon p_2D[],prob_distribution& prob)
      {
         const int n_output_bins=30;	// Number of bins in probability dist
   
// Next locate extremal bounding box points which enclose either
// intersection or union of all polygons inside p_2D array:

         unsigned int min_px,min_py,max_px,max_py;
         ztwoDarray_ptr->locate_extremal_xy_pixels(
            poly_intersection,poly_union,npolys,
            p_2D,min_px,min_py,max_px,max_py);

// Scan through each pixel in bounding box.  If a given pixel lies
// inside all [at least one] of the polygons within the p_2D array,
// include its contribution to the intensity distribution if
// poly_intersection [poly_union] flag equals true:

         int npixels_inside_poly=0;
         threevector currpoint;
         double intensity[
            ztwoDarray_ptr->get_mdim()*ztwoDarray_ptr->get_ndim()];
         for (unsigned int px=basic_math::max(Unsigned_Zero,min_px-1); 
              px<basic_math::min(ztwoDarray_ptr->get_mdim(),
                                 max_px+1); px++)
         {
            for (unsigned int py=basic_math::max(Unsigned_Zero,min_py-1); 
                 py<basic_math::min(ztwoDarray_ptr->get_ndim(),
                                    max_py+1); py++)
            {
               if (ztwoDarray_ptr->get(px,py) > zmin)
               {
                  if (ztwoDarray_ptr->pixel_inside_at_least_one_of_n_polys(
                     px,py,currpoint,npolys,p_2D))
                  {
                     intensity[npixels_inside_poly++]=ztwoDarray_ptr->
                        get(px,py);
                  }
               }
            }	   // py loop
         }	// px loop

         prob.fill_existing_distribution(
            npixels_inside_poly,intensity,n_output_bins);
//   prob.set_densityfilenamestr(imagedir+"intensity_inside_dens"
//      +stringfunc::number_to_string(imagenumber)+".meta");
//   prob.set_cumulativefilenamestr(imagedir+"intensity_inside_cum"
//      +stringfunc::number_to_string(imagenumber)+".meta");
//   prob.set_xlabel("Relative Normalized Intensity (dB)");
//   prob.xmin=0;
//   prob.writeprobdists();
      }

// ---------------------------------------------------------------------
// Method intensity_distribution_outside_n_polys takes in an array of
// polygons p_3D and projects each member down onto the x-y plane.
// This method then scans through each pixel within the image.  If the
// pixel lies outside all of the polygons in p_2D, the contribution of
// that pixel is added to the intensity probability distribution,
// provided the pixel's intensity exceeds some specified zmin
// intensity value.

   void intensity_distribution_outside_n_polys(
      unsigned int npolys,twoDarray const *ztwoDarray_ptr,double zmin,
      const polygon p_3D[],prob_distribution& prob)
      {
         const bool poly_intersection=false;
         const bool poly_union=true;
         const int n_output_bins=30;	// Number of bins in probability dist
   
// First project each 3D polygon inside p_3D down onto image plane:

         polygon p_2D[npolys];
         for (unsigned int n=0; n<npolys; n++)
         {
            p_2D[n]=p_3D[n].xy_projection();
         }

// Next locate extremal bounding box points which enclose the
// intersection of all polygons inside p_2D array:

         unsigned int min_px,min_py,max_px,max_py;
         ztwoDarray_ptr->locate_extremal_xy_pixels(
            poly_intersection,poly_union,npolys,
            p_2D,min_px,min_py,max_px,max_py);

// Scan through each pixel in image.  If a given pixel lies outside
// all of the polygons within the p_2D array, include its contribution
// to the intensity distribution:

         bool pixel_inside_all_polys,pixel_inside_at_least_one_poly;
         int npixels_outside_polys=0;
         double intensity[(max_px-min_px+1)*(max_py-min_py+1)];
         threevector currpoint;
         for (unsigned int px=basic_math::max(Unsigned_Zero,min_px-1); 
              px<basic_math::min(ztwoDarray_ptr->get_mdim(),
                                 max_px+1); px++)
         {
            for (unsigned int py=basic_math::max(Unsigned_Zero,min_py-1); 
                 py<basic_math::min(ztwoDarray_ptr->get_ndim(),
                                    max_py+1); py++)
            {
               ztwoDarray_ptr->pixel_inside_n_polys(
                  px,py,currpoint,npolys,p_2D,
                  pixel_inside_all_polys,pixel_inside_at_least_one_poly);
               if (!pixel_inside_at_least_one_poly && ztwoDarray_ptr->
                   get(px,py) > zmin)
               {
                  intensity[npixels_outside_polys]=ztwoDarray_ptr->get(px,py);
                  npixels_outside_polys++;
               }
            }	   // py loop
         }	// px loop

         prob.fill_existing_distribution(
            npixels_outside_polys,intensity,n_output_bins);
//         prob.set_densityfilenamestr(imagedir+"intensity_outside_dens"
//            +stringfunc::number_to_string(imagenumber)+".meta");
//         prob.set_cumulativefilenamestr(imagedir+"intensity_outside_cum"
//            +stringfunc::number_to_string(imagenumber)+".meta");
         prob.set_xlabel("Relative Normalized Intensity (dB)");
//   prob.writeprobdists();
      }

// ---------------------------------------------------------------------
// Method isolated_region attempts to determine whether a certain
// location within an image specified by input threevector center
// represents an island of nonzero intensity surrounded by some ocean
// of black pixels.  (This method was invented for whip antenna dot
// extraction purposes.  In this application, we're specifically
// looking for a single hot dot to stand out against a cool
// background.)  This method draws a sequence of regular polygons
// surrounding the central location.  If the intensity line integral
// taken around any one of these polygons equals zero, the region
// centered upon center is declared to be isolated, and this boolean
// function returns true.

   bool isolated_region(
      const threevector& center,int min_nonzero_pixels,
      double rmin,double rmax,double& r_isolate,
      twoDarray const *ztwoDarray_ptr)
      {
         const int niters=5;
         bool region_isolated=false;

         double dr=(rmax-rmin)/(niters-1);
         int n=0;
         while (!region_isolated && n<niters)
         {
            double r=rmin+n*dr;
            regular_polygon poly(15,r);
            poly.translate(center);

// Make sure that region in question actually contains some nonzero
// intensity valued pixels!

            int n_nonzero_pixels=count_pixels_above_zmin_inside_poly(
               1,poly,ztwoDarray_ptr);
//      cout << "n_nonzero_pixels = " << n_nonzero_pixels << endl;
      
//      double interior_intensity_integral=surface_integral_inside_polygon(
//         ztwoDarray_ptr,poly);
//      if (interior_intensity_integral > 0)
            if (n_nonzero_pixels > min_nonzero_pixels)
            {
               bool poly_isolates_region=true;
               unsigned int i=0;
               while (poly_isolates_region && i < poly.get_nvertices())
               {
                  linesegment l(poly.get_vertex(i),poly.get_vertex(
                     (i+1)%poly.get_nvertices()));
                  if (line_integral_along_segment(ztwoDarray_ptr,l) != 0)
                     poly_isolates_region=false;
                  i++;
               }
               if (poly_isolates_region) 
               {
                  region_isolated=true;
                  r_isolate=r;
               }
            } // n_nonzero_pixels > min_nonzero_pixels conditional
            n++;
         } // !region_isolated && n<niters while loop
         return region_isolated;
      }
   
// ==========================================================================
// Global thresholding methods
// ==========================================================================

// Method min_intensity_above_floor scans through all pixel intensity
// values within the image and returns the smallest intensity value
// which exceeds intensity_floor:

   double min_intensity_above_floor(
      double intensity_floor,twoDarray const *ztwoDarray_ptr)
      {
//         cout << "inside imagefunc::min_intensity_above_floor()" << endl;

         double min_intensity=POSITIVEINFINITY;
         for (unsigned int i=0; i<ztwoDarray_ptr->get_mdim()*
                 ztwoDarray_ptr->get_ndim(); i++)
         {
            double curr_intensity=ztwoDarray_ptr->get(i);
            if (curr_intensity > intensity_floor &&
                curr_intensity < min_intensity) 
               min_intensity=curr_intensity;
         }
         return min_intensity;
      }

// Method max_intensity_below_ceiling scans through all pixel
// intensity values within the image and returns the largest one which
// lies below intensity_ceiling:

   double max_intensity_below_ceiling(
      double intensity_ceiling,twoDarray const *ztwoDarray_ptr)
      {
         double max_intensity=NEGATIVEINFINITY;
         for (unsigned int i=0; i<ztwoDarray_ptr->get_mdim()*
                 ztwoDarray_ptr->get_ndim(); i++)
         {
            double curr_intensity=ztwoDarray_ptr->get(i);
            if (curr_intensity < intensity_ceiling &&
                curr_intensity > max_intensity) 
               max_intensity=curr_intensity;
         }
         return max_intensity;
      }

// ---------------------------------------------------------------------
// Method threshold_intensities_below[above]_cutoff takes in an input
// twoDarray.  It scans through and sets any intensity less [greater]
// than some specified cutoff_intensity equal to znull.

   void threshold_intensities_below_cutoff(
      twoDarray* ztwoDarray_ptr,double cutoff_intensity,double znull)
      {
         for (unsigned int i=0; i<ztwoDarray_ptr->get_mdim()*
                 ztwoDarray_ptr->get_ndim(); i++)
         {
            double curr_intensity=ztwoDarray_ptr->get(i);
            if (curr_intensity < cutoff_intensity)
            {
               ztwoDarray_ptr->put(i,znull);
            }
         }
      }

   void threshold_intensities_above_cutoff(
      twoDarray* ztwoDarray_ptr,double cutoff_intensity,double znull)
      {
         for (unsigned int i=0; i<ztwoDarray_ptr->get_mdim()*
                 ztwoDarray_ptr->get_ndim(); i++)
         {
            double curr_intensity=ztwoDarray_ptr->get(i);
            if (curr_intensity > cutoff_intensity)
            {
               ztwoDarray_ptr->put(i,znull);
            }
         }
      }

// ---------------------------------------------------------------------
// Method threshold_intensities_below[above]_cutoff_frac first
// computes the intensity distribution for all non-null pixels within
// input *ztwoDarray_ptr.  It then sets to zero all pixels whose
// intensities lie below [above] the cutoff_frac percentile of the
// intensity distribution.

   void threshold_intensities_below_cutoff_frac(
      twoDarray* ztwoDarray_ptr,double cutoff_frac)
      {
         prob_distribution prob;
         image_intensity_distribution(1,ztwoDarray_ptr,prob);
         double threshold=prob.find_x_corresponding_to_pcum(cutoff_frac);
         threshold_intensities_below_cutoff(ztwoDarray_ptr,threshold);
      }

   void threshold_intensities_above_cutoff_frac(
      double zmin,twoDarray* ztwoDarray_ptr,
      double cutoff_frac,double znull)
      {
         prob_distribution prob;
         image_intensity_distribution(zmin,ztwoDarray_ptr,prob);
         double threshold=prob.find_x_corresponding_to_pcum(cutoff_frac);
         cout << "threshold = " << threshold << endl;
         threshold_intensities_above_cutoff(
            ztwoDarray_ptr,threshold,znull);
      }

// ---------------------------------------------------------------------
// Method particular_cutoff_threshold takes in twoDarray
// *ztwoDarray_ptr.  It sets to znull all pixel values which do not
// equal input intensity z_threshold.

   void particular_cutoff_threshold(
      double z_threshold,twoDarray* ztwoDarray_ptr,double znull)
      {
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (!nearly_equal(ztwoDarray_ptr->get(px,py),z_threshold))
               {
                  ztwoDarray_ptr->put(px,py,znull);
               }
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method threshold_intensities_inside_bbox takes in the coordinates
// of a rectangular bounding box and an input twoDarray.  It nulls out
// any pixel inside the bounding box whose intensity value is less
// than the input threshold level set by hot_threshold:

   void threshold_intensities_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double hot_threshold,twoDarray* ztwoDarray_ptr)
      {
         unsigned int min_px,max_px,min_py,max_py;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            for (unsigned int px=min_px; px<max_px; px++)
            {
               for (unsigned int py=min_py; py<max_py; py++)
               {
                  if (ztwoDarray_ptr->get(px,py) < hot_threshold) 
                  {
                     ztwoDarray_ptr->put(px,py,0);
                  }
               }	  
            }	
         } // bbox_corners_to_pixels conditional
      }

// ---------------------------------------------------------------------
// Method threshold_intensities_inside_poly takes in a polygon and an
// image array zarray.  It nulls out any pixel inside the polygon
// whose intensity value is less than the input threshold level set by
// hot_threshold:

   void threshold_intensities_inside_poly(
      polygon& poly,double hot_threshold,twoDarray* ztwoDarray_ptr)
      {
         bool inside_poly,outside_poly,on_perimeter;
         unsigned int min_px,min_py,max_px,max_py;
//         int n_cold_pixels=0;
//         int n_nonzero_pixels=0;
         threevector currpoint;

         ztwoDarray_ptr->locate_extremal_xy_pixels(
            poly,min_px,min_py,max_px,max_py);
         for (unsigned int i=min_px; i<max_px; i++)
         {
            for (unsigned int j=min_py; j<max_py; j++)
            {
               ztwoDarray_ptr->pixel_relative_to_poly(
                  i,j,currpoint,poly,inside_poly,outside_poly,on_perimeter);
               if (inside_poly)
               {
                  double currz=ztwoDarray_ptr->get(i,j);
//            if (currz > 0) n_nonzero_pixels++;
                  if (currz < hot_threshold)
                  {
//               if (currz > 0) n_cold_pixels++;
                     ztwoDarray_ptr->put(i,j,0);
                  }
               }
            } // loop over index i
         }  // loop over index j

//   cout << "At end of myimage::threshold_zarray_inside_poly()" << endl;
//   cout << "ztwoDarray_ptr->get_mdim() = " << ztwoDarray_ptr->get_mdim() 
//        << " ztwoDarray_ptr->get_ndim() = " << ztwoDarray_ptr->get_ndim() << endl;
//   cout << "min_px = " << min_px << " max_px = " << max_px << endl;
//   cout << "min_py = " << min_py << " max_py = " << max_py << endl;
//   cout << "Number of pixels inside bbox = "
//        << (max_px-min_px)*(max_py-min_py) << endl;
//   cout << "n_cold_pixels = " << n_cold_pixels
//        << " n_nonzero_pixels = " << n_nonzero_pixels
//        << " ratio = " << double(n_cold_pixels)/double(n_nonzero_pixels)
//        << endl;
//   outputfunc::newline();
      }

// ---------------------------------------------------------------------
// Method reset_values_using_another_image takes in 2 twoDarrays
// *ftwoDarray_ptr and *ztwoDarray_ptr.  For every pixel within
// *ftwoDarray_ptr whose value equals input_f_value, the corresponding

   void reset_values_using_another_image(
      const twoDarray* ftwoDarray_ptr,twoDarray* ztwoDarray_ptr,
      double input_f_value,double output_z_value)
      {
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               if (nearly_equal(ftwoDarray_ptr->get(px,py),input_f_value))
               {
                  ztwoDarray_ptr->put(px,py,output_z_value);
               }
            }
         }
      }

// ==========================================================================
// Local thresholding methods
// ==========================================================================

// Method compute_local_thresholds takes in an array of threshold
// centers along with an input twoDarray.  Around each center, it
// constructs a regular polygon whose radius is set by input parameter
// local_threshold_radius.  This method computes the intensity
// distribution within the circle, and it sets the local threshold
// based upon the threshold_fraction percentile of the cumulative
// distribution.

   void compute_local_thresholds(
      unsigned int n_threshold_centers,double local_threshold_radius,
      double threshold_frac,threevector threshold_center[],
      double local_threshold[],twoDarray const *ztwoDarray_ptr)
      {
         if (n_threshold_centers >= 1)
         {
            double threshold_radius[n_threshold_centers];
            double threshold_fraction[n_threshold_centers];
            for (unsigned int n=0; n<n_threshold_centers; n++)
            {
               threshold_radius[n]=local_threshold_radius;
               threshold_fraction[n]=threshold_frac;
            }
            compute_local_thresholds(n_threshold_centers,threshold_radius,
                                     threshold_fraction,threshold_center,
                                     local_threshold,ztwoDarray_ptr);
         }
         else
         {
            cout << "Error in imagefunc::compute_local_thresholds()" << endl;
            cout << "n_threshold_centers = " << n_threshold_centers
                 << endl;
         } // n_threshold_centers >= 1 conditional
      }

   void compute_local_thresholds(
      unsigned int n_threshold_centers,double local_threshold_radius[],
      double threshold_fraction[],threevector threshold_center[],
      double local_threshold[],twoDarray const *ztwoDarray_ptr)
      {
         if (n_threshold_centers >= 1)
         {
            const double zmin=1;
            prob_distribution prob;

            for (unsigned int n=0; n<n_threshold_centers; n++)
            {
               regular_polygon circle(10,local_threshold_radius[n]);
               circle.translate(threshold_center[n]);

               if (imagefunc::intensity_distribution_inside_poly(
                  ztwoDarray_ptr,zmin,circle,prob))
               {
                  local_threshold[n]=prob.find_x_corresponding_to_pcum(
                     threshold_fraction[n]);
               }
               else
               {
                  local_threshold[n]=zmin;
               }
            } // loop over index n labeling threshold center
         }
         else
         {
            cout << "Error in imagefunc::compute_local_thresholds()" << endl;
            cout << "n_threshold_centers = " << n_threshold_centers
                 << endl;
         } // n_threshold_centers >= 1 conditional
      }

// ---------------------------------------------------------------------
// Method differentially_threshold applies multiple local thresholds
// to the current image.  It takes in the local threshold centers and
// threshold values corresponding to these centers as input
// parameters.  It then scans through the entire image and computes
// the weighted threshold average at each point within the image.  If
// the intensity value at a given pixel within input array z_thresh
// exceeds its local threshold, the pixel's intensity is reset to
// zero.

   void differentially_threshold(
      bool exp_flag,int n_threshold_centers,double correlation_length,
      double threshold[],threevector threshold_center[],
      twoDarray* ztwoDarray_ptr)
      {
         threevector currpoint;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double currz=ztwoDarray_ptr->get(px,py);
               if (currz > 0 && 
                   ztwoDarray_ptr->pixel_to_point(px,py,currpoint))
               {
                  double interpolated_threshold=threshold_field(
                     exp_flag,n_threshold_centers,currpoint,
                     threshold_center,correlation_length,threshold);
                  if (currz < interpolated_threshold)
                  {
                     ztwoDarray_ptr->put(px,py,0);
                  }
               }
            } // loop over px index
         } // loop over py index
      }

   void differentially_threshold(
      bool exp_flag,int n_threshold_centers,double correlation_length,
      double threshold[],threevector threshold_center[],
      twoDarray* zthreshold_twoDarray_ptr,twoDarray* ztwoDarray_ptr)
      {
         threevector currpoint;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double currz=ztwoDarray_ptr->get(px,py);
               ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
//               if (currz > 0 && 
//                   ztwoDarray_ptr->pixel_to_point(px,py,currpoint))
               {
                  double interpolated_threshold=threshold_field(
                     exp_flag,n_threshold_centers,currpoint,
                     threshold_center,correlation_length,threshold);
                  zthreshold_twoDarray_ptr->put(px,py,interpolated_threshold);
                  if (currz < interpolated_threshold)
                  {
                     ztwoDarray_ptr->put(px,py,0);
                  }
               }
            } // loop over px index
         } // loop over py index
      }

// ---------------------------------------------------------------------
// Method threshold_field takes in a set of threshold values
// (calculated for example based upon intensity distributions within
// the localized regions of an image) along with their associated
// center positions.  It also takes in the current position where an
// interpolated threshold value is desired.  This method computes a
// weighted average of the threshold values.  If boolean
// exp_flag==true, the weights are proportional to exp(-r_i) where r_i
// = distance between current position and ith threshold value.
// Otherwise, the weights are proportional to the (faster) function
// 1/(1+r_i**2). 

   double fast_threshold_field(
      double sqr_correlation_length,double curr_x,double curr_y,
      const vector<twovector>& centers_posn,const vector<double>& threshold)
      {
         double numer=0;
         double denom=0;
         for (unsigned int i=0; i<centers_posn.size(); i++)
         {
            double xterm=centers_posn[i].get(0)-curr_x;
            double yterm=centers_posn[i].get(1)-curr_y;
            //            double normalized_rsq=
            //   (sqr(xterm)+sqr(yterm))/sqr_correlation_length;
            //double alpha=1/(1+normalized_rsq);
            double alpha=sqr_correlation_length/
                 (sqr_correlation_length+xterm*xterm+yterm*yterm);
            numer += alpha*threshold[i];
            denom += alpha;
         } 
         return numer/denom;
      }

   double fast_threshold_field(
      int counter,double sqr_correlation_length,
      vector<pair<double,int> >* rsq_ptr,
      double curr_x,double curr_y,const vector<twovector>& centers_posn,
      const vector<double>& threshold)
      {
         pair<double,int> p;
         if (counter%5==0)
         {
            rsq_ptr->clear();
            for (unsigned int i=0; i<centers_posn.size(); i++)
            {
               double xterm=centers_posn[i].get(0)-curr_x;
               double yterm=centers_posn[i].get(1)-curr_y;
               p.first=(sqr(xterm)+sqr(yterm))/sqr_correlation_length;
               p.second=i;
               rsq_ptr->push_back(p);
            }
            std::sort(rsq_ptr->begin(),rsq_ptr->end());
         }
         
//         const unsigned int n_closest_centers=5;
         const unsigned int n_closest_centers=20;
         double numer=0;
         double denom=0;
         double alpha=0;
         for (unsigned int i=0; i<n_closest_centers; i++)
         {
            alpha=1/(1+rsq_ptr->at(i).first);
            numer += alpha*threshold[rsq_ptr->at(i).second];
            denom += alpha;
         }
         return numer/denom;
      }

// ---------------------------------------------------------------------
   double threshold_field(
      bool exp_flag,unsigned int ncenters,
      const threevector& curr_posn,const threevector centers_posn[],
      double correlation_length,double threshold[])
      {
         double alpha,sum=0;
//         cout << "Inside imagefunc::threshold_field()" << endl;
//         cout << "ncenters = " << ncenters << " curr_posn = " 
//              << curr_posn << endl;

         double interpolated_threshold=0;
         for (unsigned int i=0; i<ncenters; i++)
         {
//            cout << "i = " << i << " threshold = " << threshold[i] << endl;
//            cout << "centers_posn = " << centers_posn[i]
//                 << endl;
            double xterm=centers_posn[i].get(0)-curr_posn.get(0);
            double yterm=centers_posn[i].get(1)-curr_posn.get(1);

            if (exp_flag)
            {
               double r=sqrt(sqr(xterm)+sqr(yterm));
//      cout << "r = " << r << endl;
               alpha=exp(-r/correlation_length);
            }
            else
            {
               double rsq=(sqr(xterm)+sqr(yterm))/sqr(correlation_length);
               alpha=1/(1+rsq);
            }

            interpolated_threshold += alpha*threshold[i];
            sum += alpha;
         }
         interpolated_threshold /= sum;
//         cout << "interpolated threshold = " 
//              << interpolated_threshold << endl;
         return interpolated_threshold;
      }

// ---------------------------------------------------------------------
// Method differentially_threshold_inside_bbox applies multiple local
// thresholds to the current image inside the bounding box specified
// by input parameters minimum_x, minimum_y, maximum_x and maximum_y:

   void differentially_threshold_inside_bbox(
      bool exp_flag,int n_threshold_centers,double correlation_length,
      double threshold[],threevector threshold_center[],
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      twoDarray* zthresh_twoDarray_ptr)
      {
         unsigned int min_px,max_px,min_py,max_py;
         if (zthresh_twoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            threevector currpoint;
//            twoDarray zvarthresh_twoDarray(
//               zthresh_twoDarray_ptr->get_mdim(),
//               zthresh_twoDarray_ptr->get_ndim());
            for (unsigned int px=min_px; px<max_px; px++)
            {
               for (unsigned int py=min_py; py<max_py; py++)
               {
                  if (zthresh_twoDarray_ptr->pixel_to_point(px,py,currpoint))
                  {
                     double interpolated_threshold=threshold_field(
                        exp_flag,n_threshold_centers-1,currpoint,
                        threshold_center,correlation_length,threshold);
//                     zvarthresh_twoDarray.put(px,py,interpolated_threshold);
                     if (zthresh_twoDarray_ptr->get(px,py) < 
                         interpolated_threshold)
                     {
                        zthresh_twoDarray_ptr->put(px,py,0);
                     }
                  }
               } // loop over py index
            } // loop over px index
//   writeimage("diff_thresh","Differential Threshold",zvarthresh_twoDarray);
         } // bbox_corners_to_pixels conditional
      }

// =====================================================================
// Image size methods
// =====================================================================

// Method valid_image_file() calls linux built-in "file" command to
// test the specified input image file.  If JPEG, PNG or TIFF keywords
// are found in the output, this boolean method returns true.

   bool valid_image_file(string image_filename)
   {
      string metadata_filename="/tmp/file.dat";
      filefunc::deletefile(metadata_filename);
      
// First resolve image_filename if it is a symbolic link:

      if (filefunc::symboliclink_exist(image_filename))
      {
//         cout << "image_filename = " << image_filename << " is a symbolic link"
//              << endl;
         image_filename=filefunc::resolve_linked_filename(image_filename);
//         cout << "resolved filename = "
//              << image_filename << endl;
      }
      
      string unix_cmd="file "+image_filename +" > "+metadata_filename;
//      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      int counter=0;
      while (!filefunc::fileexist(metadata_filename) && counter < 10)
      {
         usleep(100);
      }
      if (counter >= 10) return false;

      filefunc::ReadInfile(metadata_filename);
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(filefunc::text_line[0]);
      bool valid_image_flag=false;
      if (substrings[1]=="JPEG" ||substrings[1]=="PNG" ||
      substrings[1]=="TIFF")
      {
         valid_image_flag=true;
      }

      return valid_image_flag;
   }

// ---------------------------------------------------------------------
// Method corrupted_jpg_file() calls utility jpeginfo with a "check"
// flag which looks for input file errors.  If any are found, this
// boolean method returns true.

   bool corrupted_jpg_file(string jpg_filename)
   {
      bool corrupted_file = true;

      string jpg_logfilename="./jpg.log";
      string unix_cmd = "jpeginfo --check "+jpg_filename+" > "+jpg_logfilename;
      sysfunc::unix_command(unix_cmd);
      filefunc::ReadInfile(jpg_logfilename);
//      cout << filefunc::text_line[0] << endl;
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         filefunc::text_line[0]);
//      cout << substrings.back() << endl;
      
      if(substrings.back() == "[OK]")
      {
         corrupted_file = false;
      }
      return corrupted_file;
   }

// ---------------------------------------------------------------------
// Static method GetImageSize() comes from
// http://www.wischik.com/lu/programmer/get-image-size.html

   static bool GetImageSize(const char *fn, int *x,int *y)
   { 
//      cout << "inside imagefunc::GetImageSize()" << endl;
      FILE *f=fopen(fn,"rb"); 
      if (f==0) 
      {
         cout << "Couldn't open " << fn << endl;
         return false;
      }

      fseek(f,0,SEEK_END); 
      long len=ftell(f); 
      fseek(f,0,SEEK_SET); 
      if (len < 24) 
      {
         fclose(f); 
         cout << "len = " << len << endl;
         return false;
      }

      // Strategy:
      // reading GIF dimensions requires the first 10 bytes of the file
      // reading PNG dimensions requires the first 24 bytes of the file
      // reading JPEG dimensions requires scanning through jpeg chunks

      // In all formats, the file is at least 24 bytes big, so we'll read
      // that always

      unsigned char buf[24]; fread(buf,1,24,f);

      // For JPEGs, we need to read the first 12 bytes of each chunk.
      // We'll read those 12 bytes at buf+2...buf+14, i.e. overwriting
      // the existing buf.

      if (buf[0]==0xFF && buf[1]==0xD8 && buf[2]==0xFF && buf[3]==0xE0 && buf[6]=='J' && buf[7]=='F' && buf[8]=='I' && buf[9]=='F')
      { 
         long pos=2;
         while (buf[2]==0xFF)
         { 
            if (buf[3]==0xC0 || buf[3]==0xC1 || buf[3]==0xC2 || buf[3]==0xC3 || buf[3]==0xC9 || buf[3]==0xCA || buf[3]==0xCB) break;
            pos += 2+(buf[4]<<8)+buf[5];
            if (pos+12>len) break;
            fseek(f,pos,SEEK_SET); fread(buf+2,1,12,f);    
         }
      }

      fclose(f);

      // JPEG: (first two bytes of buf are first two bytes of the jpeg file; rest of buf is the DCT frame
      if (buf[0]==0xFF && buf[1]==0xD8 && buf[2]==0xFF)
      { 
         *y = (buf[7]<<8) + buf[8];
         *x = (buf[9]<<8) + buf[10];
         return true;
      }

      // GIF: first three bytes say "GIF", next three give version number. Then dimensions
      if (buf[0]=='G' && buf[1]=='I' && buf[2]=='F')
      { 
         *x = buf[6] + (buf[7]<<8);
         *y = buf[8] + (buf[9]<<8);
         return true;
      }
   
      // PNG: the first frame is by definition an IHDR frame, which gives dimensions
      if ( buf[0]==0x89 && buf[1]=='P' && buf[2]=='N' && buf[3]=='G' && buf[4]==0x0D && buf[5]==0x0A && buf[6]==0x1A && buf[7]==0x0A
           && buf[12]=='I' && buf[13]=='H' && buf[14]=='D' && buf[15]=='R')
      { 
         *x = (buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) + (buf[19]<<0);
         *y = (buf[20]<<24) + (buf[21]<<16) + (buf[22]<<8) + (buf[23]<<0);
         return true;
      }

      return false;
   }

// ---------------------------------------------------------------------
   bool get_image_width_height(string image_filename,int& width,int& height)
   {
      unsigned int w, h;
      bool flag = get_image_width_height(image_filename, w, h);
      width = w;
      height = h;
      return flag;
   }

   bool get_image_width_height(string image_filename,
                               unsigned int& width,unsigned int& height)
   {
      string format;
      unsigned int filesize;
      return get_image_properties(
         image_filename,width,height,format,filesize);
   }

// ---------------------------------------------------------------------
// Method get_image_properties() uses ImageMagick's ping() command
// followed by columns(), rows(), format() and fileSize() calls to
// rapidly return basic image property information without expensive
// pixel importing.  If the input file does not exist, this boolean
// method returns false.

   bool get_image_properties(
      string image_filename,unsigned int& width,unsigned int& height,
      string& format,unsigned int& bytesize)
   {
      if(filefunc::fileexist(image_filename))
      {
         Magick::Image curr_image;
//         cout << "   img=" << filefunc::getbasename(image_filename) << endl;
         curr_image.ping(image_filename);

         width=curr_image.columns();
         height=curr_image.rows();
         format=curr_image.format();
         bytesize=curr_image.fileSize();
         return true;
      }
      else
      {
         return false;
      }
   }

// =====================================================================
// Image properties methods:
// =====================================================================

   int count_pixels_below_zmax(
      double zmax,twoDarray const *ztwoDarray_ptr) 
      {
//         cout << "inside imagefunc::count_pixels_below_zmax()" << endl;
         int npixels_below_zmax=0;
         for (unsigned int i=0; i<ztwoDarray_ptr->get_mdim()*
                 ztwoDarray_ptr->get_ndim(); i++)
         {
            if (ztwoDarray_ptr->get(i) < zmax) npixels_below_zmax++;
         }
         return npixels_below_zmax;
      }

   int count_pixels_above_zmin(
      double zmin,twoDarray const *ztwoDarray_ptr) 
      {
         int npixels_above_zmin=0;
         for (unsigned int i=0; i<ztwoDarray_ptr->get_mdim()*
                 ztwoDarray_ptr->get_ndim(); i++)
         {
            if (ztwoDarray_ptr->get(i) > zmin) npixels_above_zmin++;
         }
         return npixels_above_zmin;
      }

   int count_pixels_in_z_interval(
      double zmin,double zmax,twoDarray const *ztwoDarray_ptr) 
      {
         int npixels_in_zinterval=0;
         for (unsigned int i=0; i<ztwoDarray_ptr->get_mdim()*
                 ztwoDarray_ptr->get_ndim(); i++)
         {
            double curr_z=ztwoDarray_ptr->get(i);
            if (curr_z > zmin && curr_z < zmax) npixels_in_zinterval++;
         }
         return npixels_in_zinterval;
      }

   int count_pixels_above_zmin_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmin,twoDarray const *ztwoDarray_ptr) 
      {
         int npixels_inside_bbox;
         return count_pixels_above_zmin_inside_bbox(
            minimum_x,minimum_y,maximum_x,maximum_y,zmin,ztwoDarray_ptr,
            npixels_inside_bbox);
      }
   
   int count_pixels_above_zmin_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmin,twoDarray const *ztwoDarray_ptr,int& npixels_inside_bbox) 
      {
         npixels_inside_bbox=0;
         int npixels_above_zmin=0;

         unsigned int min_px,max_px,min_py,max_py;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            for (unsigned int px=basic_math::max(Unsigned_Zero,min_px-1); 
                 px<basic_math::min(ztwoDarray_ptr->get_mdim(),
                                    max_px+1); px++)
            {
               for (unsigned int py=basic_math::max(Unsigned_Zero,min_py-1); 
                    py<basic_math::min(ztwoDarray_ptr->get_ndim(),
                                       max_py+1); py++)
               {
                  npixels_inside_bbox++;
                  if (ztwoDarray_ptr->get(px,py) > zmin) npixels_above_zmin++;
               }
            }
         } // bbox_corners_to_pixels conditional
         return npixels_above_zmin;
      }
   
// Method strict_count_pixels_above_zmin_inside_bbox is a minor
// variant of the preceding count_pixels_above_zmin_inside_bbox.  Only
// those pixels which lie strictly within the interior of the input
// bounding box dimensions are counted by this method.

   int strict_count_pixels_above_zmin_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmin,twoDarray const *ztwoDarray_ptr,int& npixels_inside_bbox)
      {
         npixels_inside_bbox=0;
         int npixels_above_zmin=0;
         
         unsigned int min_px,max_px,min_py,max_py;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            for (unsigned int px=min_px+1; px<=max_px; px++)
            {
               for (unsigned int py=min_py+1; py<=max_py; py++)
               {
                  npixels_inside_bbox++;
                  if (ztwoDarray_ptr->get(px,py) > zmin) npixels_above_zmin++;
               }
            }
         } // bbox_corners_to_pixels conditional
         return npixels_above_zmin;
      }

   int count_pixels_above_zmin_outside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmin,twoDarray const *ztwoDarray_ptr) 
      {
         int npixels_above_zmin=
            count_pixels_above_zmin(zmin,ztwoDarray_ptr);
         int npixels_above_zmin_inside_bbox=
            count_pixels_above_zmin_inside_bbox(
               minimum_x,minimum_y,maximum_x,maximum_y,zmin,ztwoDarray_ptr);
         return (npixels_above_zmin-npixels_above_zmin_inside_bbox);
      }

   int count_pixels_below_zmax_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      double zmax,twoDarray const *ztwoDarray_ptr,int& npixels_inside_bbox) 
      {
         npixels_inside_bbox=0;
         int npixels_below_zmax=0;

         unsigned int min_px,max_px,min_py,max_py;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            for (unsigned int px=basic_math::max(Unsigned_Zero,min_px-1); 
                 px<basic_math::min(ztwoDarray_ptr->get_mdim(),
                                    max_px+1); px++)
            {
               for (unsigned int py=basic_math::max(Unsigned_Zero,min_py-1); 
                    py<basic_math::min(ztwoDarray_ptr->get_ndim(),
                                       max_py+1); py++)
               {
                  npixels_inside_bbox++;
                  if (ztwoDarray_ptr->get(px,py) < zmax) npixels_below_zmax++;
               }
            }
         } // bbox_corners_to_pixels conditional
         return npixels_below_zmax;
      }

   int count_pixels_below_zmax_inside_parallelogram(
      double zmax,parallelogram& parallelogram,
      twoDarray const *ztwoDarray_ptr) 
      {
         unsigned int min_px,min_py,max_px,max_py;
         ztwoDarray_ptr->locate_extremal_xy_pixels(
            parallelogram,min_px,min_py,max_px,max_py);

         int npixels_below_zmax=0;
         threevector currpoint;
         for (unsigned int i=min_px; i<max_px; i++)
         {
            for (unsigned int j=min_py; j<max_py; j++)
            {
               if (ztwoDarray_ptr->pixel_to_point(i,j,currpoint))
               {
                  if (parallelogram.point_inside(currpoint))
                  {
                     if (ztwoDarray_ptr->get(i,j) < zmax) 
                        npixels_below_zmax++;
                  }
               }
            }  // loop over index j               
         } // loop over index i
         return npixels_below_zmax;
      }

// ---------------------------------------------------------------------
// Method count_pixels_above_zmin_within_halfplane generalizes method
// count_pixels_above_zmin.  It takes in a line defined by a basepoint
// and a vector nhat which is normal to the line's direction vector
// and which points in the direction of the desired halfplane.

   int count_pixels_above_zmin_within_halfplane(
      const threevector& basepoint,const threevector& nhat,
      double zmin,twoDarray const *ztwoDarray_ptr) 
      {
         int npixels_above_zmin=0;

//	Input line's direction vector is given by 
//		uhat = threevector(-nhat.get(1),nhat.get(0),0);

         if (nhat.get(1)==0)
         {
         }
         else if (nhat.get(1) > 0)
         {
            unsigned int py_hi;
            double x;
            double slope=-nhat.get(0)/nhat.get(1);
            for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
            {
               ztwoDarray_ptr->px_to_x(px,x);
               double ylo=mathfunc::linefit(
                  x,basepoint.get(0),basepoint.get(1),slope);
               ztwoDarray_ptr->y_to_py(ylo,py_hi);
               py_hi=basic_math::min(py_hi,ztwoDarray_ptr->get_ndim());
               for (unsigned int py=0; py<py_hi; py++)
               {
                  if (ztwoDarray_ptr->get(px,py) > zmin) npixels_above_zmin++;
//                  ztwoDarray_ptr->put(px,py,30);
               }
            } // px loop
         }
         else if (nhat.get(1) < 0)
         {
            unsigned int py_lo;
            double x;
            double slope=-nhat.get(0)/nhat.get(1);
            for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
            {
               ztwoDarray_ptr->px_to_x(px,x);
               double yhi=mathfunc::linefit(
                  x,basepoint.get(0),basepoint.get(1),slope);
               ztwoDarray_ptr->y_to_py(yhi,py_lo);
               py_lo=basic_math::max(Unsigned_Zero,py_lo);
               for (unsigned int py=py_lo; py<ztwoDarray_ptr->get_ndim(); py++)
               {
                  if (ztwoDarray_ptr->get(px,py) > zmin) npixels_above_zmin++;
//                  ztwoDarray_ptr->put(px,py,30);
               }
            }
         }
         return npixels_above_zmin;
      }

// --------------------------------------------------------------------
   int count_pixels_above_zmin_inside_poly(
      double zmin,polygon& poly,twoDarray const *ztwoDarray_ptr) 
      {
         unsigned int min_px,min_py,max_px,max_py;
         ztwoDarray_ptr->locate_extremal_xy_pixels(
            poly,min_px,min_py,max_px,max_py);

         bool inside_poly,outside_poly,on_perimeter;
         int npixels_above_zmin=0;
         threevector currpoint;
         for (unsigned int i=min_px; i<max_px; i++)
         {
            for (unsigned int j=min_py; j<max_py; j++)
            {
               ztwoDarray_ptr->pixel_relative_to_poly(
                  i,j,currpoint,poly,inside_poly,outside_poly,on_perimeter);
               if (inside_poly)
               {
                  if (ztwoDarray_ptr->get(i,j) > zmin) npixels_above_zmin++;
               }
            } // loop over index i
         }  // loop over index j
         return npixels_above_zmin;
      }

   int count_pixels_below_zmax_inside_poly(
      double zmax,polygon& poly,twoDarray const *ztwoDarray_ptr) 
      {
         unsigned int min_px,min_py,max_px,max_py;
         ztwoDarray_ptr->locate_extremal_xy_pixels(
            poly,min_px,min_py,max_px,max_py);

         bool inside_poly,outside_poly,on_perimeter;
         int npixels_below_zmax=0;
         threevector currpoint;
         for (unsigned int i=min_px; i<max_px; i++)
         {
            for (unsigned int j=min_py; j<max_py; j++)
            {
               ztwoDarray_ptr->pixel_relative_to_poly(
                  i,j,currpoint,poly,inside_poly,outside_poly,on_perimeter);
               if (inside_poly)
               {
                  if (ztwoDarray_ptr->get(i,j) < zmax) npixels_below_zmax++;
               }
            } // loop over index i
         }  // loop over index j
         return npixels_below_zmax;
      }

// ---------------------------------------------------------------------
// Method energy returns the integral of squared pixel intensity
// within input *ztwoDarray_ptr:

   double energy(twoDarray const *ztwoDarray_ptr)
      {
         const double dA=ztwoDarray_ptr->get_deltax()*
            ztwoDarray_ptr->get_deltay();
         double energy=0;
         for (unsigned int i=0; i<ztwoDarray_ptr->get_dimproduct(); i++)
         {
            energy += sqr(ztwoDarray_ptr->Tensor<double>::get(i))*dA;
         }
         return energy;
      }

   double integrated_intensity(twoDarray const *ztwoDarray_ptr)
      {
         return integrated_intensity(-1,ztwoDarray_ptr);
      }

   double integrated_intensity(double zmin,twoDarray const *ztwoDarray_ptr)
      {
         double integral=0;
         for (unsigned int i=0; i<ztwoDarray_ptr->get_dimproduct(); i++)
         {
            double currz=ztwoDarray_ptr->Tensor<double>::get(i);
            if (currz > zmin) integral += currz;
         }
         return integral;
      }

   double integrated_intensity_inside_bbox(
      double minimum_x,double minimum_y,double maximum_x,double maximum_y,
      twoDarray const *ztwoDarray_ptr)
      {
         double integrated_intensity=0;

         unsigned int min_px,max_px,min_py,max_py;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            for (unsigned int px=basic_math::max(Unsigned_Zero,min_px-1); 
                 px<basic_math::min(ztwoDarray_ptr->get_mdim(),
                                    max_px+1); px++)
            {
               for (unsigned int py=basic_math::max(Unsigned_Zero,min_py-1); 
                    py<basic_math::min(ztwoDarray_ptr->get_ndim(),
                                       max_py+1); py++)
               {
                  integrated_intensity += ztwoDarray_ptr->get(px,py);
               }
            }
         } // bbox_corners_to_pixels conditional
         return integrated_intensity;
      }

// ---------------------------------------------------------------------
// Method integrated_intensity_inside_poly takes in a twoDarray
// containing a binary mask for some polygon.  It also takes
// *ztwoDarray_ptr which contains some image intensity distribution.
// This method further takes in bounding box corner pixel limits.
// Scanning over the bounding box, this method sums up intensities
// within *ztwoDarray_ptr if their corresponding locations within the
// mask is nonzero.  This method returns the total number of pixels
// inside the nonzero part of the mask, the number of non-null
// intensity values it found as well as the integral of those
// intensities.

   void integrated_intensity_inside_poly(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      double zmin,twoDarray const *ztwoDarray_ptr,
      twoDarray const *zmask_twoDarray_ptr,
      int& npixels_inside_poly,int& npixels_above_zmin,
      double& intensity_integral)
      {
         npixels_inside_poly=0;
         npixels_above_zmin=0;
         intensity_integral=0;
         for (unsigned int px=min_px; px<max_px; px++)
         {
            for (unsigned int py=min_py; py<max_py; py++)
            {
               if (zmask_twoDarray_ptr->get(px,py) > 0)
               {
                  npixels_inside_poly++;
                  double currz=ztwoDarray_ptr->get(px,py);
                  if (currz > zmin) npixels_above_zmin++;
                  intensity_integral += currz;
               }
            } // loop over index py
         }  // loop over index px

//         cout << "Number of pixels inside poly = " << npixels_inside_poly 
//              << endl;
//         cout << "Number of nonzero pixels inside poly = "
//              << npixels_above_zmin << endl;
//         cout << "Integrated intensity inside poly = " << intensity_integral 
//              << endl;
//         if (npixels_above_zmin > 0)
//         {
//            cout << "Avg intensity inside poly = " 
//                 << intensity_integral/npixels_above_zmin << endl;
//         }
      }


// ---------------------------------------------------------------------
// Method count_nonzero_pixels_inside_triangle is essentially the
// inverse of the drawfunc::color_triangle_interior method which is
// based upon the "midpoint line algorithm".  It returns the number of
// pixels contained within input triangle t as well as the number of
// pixels whose intensities exceed input threshold zmin.  It also
// returns the triangle's surface intensity integral.

// We have consciously tried to optimize this method for
// speed.

   double count_nonzero_pixels_inside_triangle(
      double zmin,const threevector& vertex0,const threevector& vertex1,
      const threevector& vertex2,const twoDarray* ztwoDarray_ptr,
      int& npixels_in_triangle,int& npixels_above_zmin_in_triangle)
      {
// Sort vertices by their x components:

         const unsigned int nsides=3;
         double vx[nsides],vy[nsides];
         vx[0]=vertex0.get(0);
         vx[1]=vertex1.get(0);
         vx[2]=vertex2.get(0);
         vy[0]=vertex0.get(1);
         vy[1]=vertex1.get(1);
         vy[2]=vertex2.get(1);
         
         Quicksort(vx,vy,nsides);

// Calculate slopes for triangle's sides:

         double m[nsides];
         m[0]=(vy[1]-vy[0])/(vx[1]-vx[0]);
         m[1]=(vy[2]-vy[1])/(vx[2]-vx[1]);
         m[2]=(vy[0]-vy[2])/(vx[0]-vx[2]);

// Convert vertex coordinates from 2D space to pixel space:

         unsigned int px[nsides],py[nsides];
         for (unsigned int n=0; n<nsides; n++)
         {
            ztwoDarray_ptr->x_to_px(vx[n],px[n]);
            ztwoDarray_ptr->y_to_py(vy[n],py[n]);
         }

// Horizontally scan across first section of triangle.  For each px
// pixel value, compute corresponding py_lo and py_hi locations on the
// triangle's two sides.  Then fill in vertical line connecting
// (px,py_hi) to (px,py_lo).

         npixels_in_triangle=0;
         npixels_above_zmin_in_triangle=0;
         double intensity_integral=0;

         double ztwoDarray_ylo=ztwoDarray_ptr->get_ylo();
         double ztwoDarray_yhi=ztwoDarray_ptr->get_yhi();
         
         unsigned int py_lo,py_hi;
         unsigned int px_lo=basic_math::max(px[0],Unsigned_Zero);
         unsigned int px_hi=basic_math::min(px[1],ztwoDarray_ptr->get_mdim());
         double x,ylo,yhi;
         for (unsigned int currpx=px_lo; currpx < px_hi; currpx++)
         {
            ztwoDarray_ptr->px_to_x(currpx,x);
            double y_01=mathfunc::linefit(x,vx[0],vy[0],m[0]);
            double y_20=mathfunc::linefit(x,vx[2],vy[2],m[2]);
            mathfunc::minmax(y_01,y_20,ylo,yhi);

// Don't process current vertical line segment any further if it lies
// completely outside the twoDarray's domain:

            if ((ylo < ztwoDarray_ylo && yhi < ztwoDarray_ylo) ||
                (ylo > ztwoDarray_yhi && yhi > ztwoDarray_yhi))
            {
            }
            else
            {
               if (!ztwoDarray_ptr->y_to_py(ylo,py_hi))
               {
                  py_hi=basic_math::min(py_hi,ztwoDarray_ptr->get_ndim());
               }
               if (!ztwoDarray_ptr->y_to_py(yhi,py_lo))
               {
                  py_lo=basic_math::max(Unsigned_Zero,py_lo);
               }

               double* e_ptr=ztwoDarray_ptr->get_ptr(currpx,py_lo);
               for (unsigned int py=py_lo; py<py_hi; py++)
               {
                  npixels_in_triangle++;
                  double currz=*e_ptr;
                  e_ptr++;
                  intensity_integral += currz;
                  if (currz > zmin)
                  {
                     npixels_above_zmin_in_triangle++;
                  }
               }
            } // vertical line segment sanity check conditional
         } // currpx loop

// Horizontally scan across 2nd section of triangle.  For each px
// pixel value, compute corresponding py_lo and py_hi locations on the
// triangle's two sides.  Then fill in vertical line connecting
// (px,py_hi) to (px,py_lo).

         px_lo=basic_math::max(px[1],Unsigned_Zero);
         px_hi=basic_math::min(px[2],ztwoDarray_ptr->get_mdim());
         for (unsigned int currpx=px_lo; currpx < px_hi; currpx++)
         {
            ztwoDarray_ptr->px_to_x(currpx,x);
            double y_12=mathfunc::linefit(x,vx[1],vy[1],m[1]);
            double y_20=mathfunc::linefit(x,vx[2],vy[2],m[2]);
            mathfunc::minmax(y_12,y_20,ylo,yhi);

// Don't process current vertical line segment any further if it lies
// completely outside the twoDarray's domain:

            if ((ylo < ztwoDarray_ylo && yhi < ztwoDarray_ylo) ||
                (ylo > ztwoDarray_yhi && yhi > ztwoDarray_yhi))
            {
            }
            else
            {
               if (!ztwoDarray_ptr->y_to_py(ylo,py_hi))
               {
                  py_hi=basic_math::min(py_hi,ztwoDarray_ptr->get_ndim());
               }
               if (!ztwoDarray_ptr->y_to_py(yhi,py_lo))
               {
                  py_lo=basic_math::max(Unsigned_Zero,py_lo);
               }

               double* e_ptr=ztwoDarray_ptr->get_ptr(currpx,py_lo);
               for (unsigned int py=py_lo; py<py_hi; py++)
               {
                  npixels_in_triangle++;
                  double currz=*e_ptr;
                  e_ptr++;
                  intensity_integral += currz;
                  if (currz > zmin)
                  {
                     npixels_above_zmin_in_triangle++;
                  }
               }
            } // vertical line segment sanity check conditional
         } // currpx loop
         return intensity_integral;
      }

   double count_nonzero_pixels_inside_triangle(
      double zmin,const polygon& t,const twoDarray* ztwoDarray_ptr,
      int& npixels_in_triangle,int& npixels_above_zmin_in_triangle)
      {
// Sort vertices by their x components:

         const unsigned int nsides=3;
         double vx[nsides],vy[nsides];
         for (unsigned int n=0; n<nsides; n++)
         {
            vx[n]=t.get_vertex(n).get(0);
            vy[n]=t.get_vertex(n).get(1);
         }
         Quicksort(vx,vy,nsides);

// Convert vertex coordinates from 2D space to pixel space:

         unsigned int px[nsides],py[nsides];
         for (unsigned int n=0; n<nsides; n++)
         {
            ztwoDarray_ptr->x_to_px(vx[n],px[n]);
            ztwoDarray_ptr->y_to_py(vy[n],py[n]);
         }

// Calculate slopes for triangle's sides:

         double m[nsides];
         m[0]=(vy[1]-vy[0])/(vx[1]-vx[0]);
         m[1]=(vy[2]-vy[1])/(vx[2]-vx[1]);
         m[2]=(vy[0]-vy[2])/(vx[0]-vx[2]);

// Horizontally scan across first section of triangle.  For each px
// pixel value, compute corresponding py_lo and py_hi locations on the
// triangle's two sides.  Then fill in vertical line connecting
// (px,py_hi) to (px,py_lo).

         npixels_in_triangle=0;
         npixels_above_zmin_in_triangle=0;
         double intensity_integral=0;

         double ztwoDarray_ylo=ztwoDarray_ptr->get_ylo();
         double ztwoDarray_yhi=ztwoDarray_ptr->get_yhi();
         
         unsigned int py_lo,py_hi;
         unsigned int px_lo=basic_math::max(px[0],Unsigned_Zero);
         unsigned int px_hi=basic_math::min(px[1],ztwoDarray_ptr->get_mdim());
         double x,ylo,yhi;
         for (unsigned int currpx=px_lo; currpx < px_hi; currpx++)
         {
            ztwoDarray_ptr->px_to_x(currpx,x);
            double y_01=mathfunc::linefit(x,vx[0],vy[0],m[0]);
            double y_20=mathfunc::linefit(x,vx[2],vy[2],m[2]);
            mathfunc::minmax(y_01,y_20,ylo,yhi);

// Don't process current vertical line segment any further if it lies
// completely outside the twoDarray's domain:

            if ((ylo < ztwoDarray_ylo && yhi < ztwoDarray_ylo) ||
                (ylo > ztwoDarray_yhi && yhi > ztwoDarray_yhi))
            {
            }
            else
            {
               if (!ztwoDarray_ptr->y_to_py(ylo,py_hi))
               {
                  py_hi=basic_math::min(py_hi,ztwoDarray_ptr->get_ndim());
               }
               if (!ztwoDarray_ptr->y_to_py(yhi,py_lo))
               {
                  py_lo=basic_math::max(Unsigned_Zero,py_lo);
               }

               for (unsigned int py=py_lo; py<py_hi; py++)
               {
                  npixels_in_triangle++;
                  double currz=ztwoDarray_ptr->get(currpx,py);
                  intensity_integral += currz;
                  if (currz > zmin)
                  {
                     npixels_above_zmin_in_triangle++;
                  }
               }
            } // vertical line segment sanity check conditional
         } // currpx loop

// Horizontally scan across 2nd section of triangle.  For each px
// pixel value, compute corresponding py_lo and py_hi locations on the
// triangle's two sides.  Then fill in vertical line connecting
// (px,py_hi) to (px,py_lo).

         px_lo=basic_math::max(px[1],Unsigned_Zero);
         px_hi=basic_math::min(px[2],ztwoDarray_ptr->get_mdim());
         for (unsigned int currpx=px_lo; currpx < px_hi; currpx++)
         {
            ztwoDarray_ptr->px_to_x(currpx,x);
            double y_12=mathfunc::linefit(x,vx[1],vy[1],m[1]);
            double y_20=mathfunc::linefit(x,vx[2],vy[2],m[2]);
            mathfunc::minmax(y_12,y_20,ylo,yhi);

// Don't process current vertical line segment any further if it lies
// completely outside the twoDarray's domain:

            if ((ylo < ztwoDarray_ylo && yhi < ztwoDarray_ylo) ||
                (ylo > ztwoDarray_yhi && yhi > ztwoDarray_yhi))
            {
            }
            else
            {
               if (!ztwoDarray_ptr->y_to_py(ylo,py_hi))
               {
                  py_hi=basic_math::min(py_hi,ztwoDarray_ptr->get_ndim());
               }
               if (!ztwoDarray_ptr->y_to_py(yhi,py_lo))
               {
                  py_lo=basic_math::max(Unsigned_Zero,py_lo);
               }

               for (unsigned int py=py_lo; py<py_hi; py++)
               {
                  npixels_in_triangle++;
                  double currz=ztwoDarray_ptr->get(currpx,py);
                  intensity_integral += currz;
                  if (currz > zmin)
                  {
                     npixels_above_zmin_in_triangle++;
                  }
               }
            } // vertical line segment sanity check conditional
         } // currpx loop
         return intensity_integral;
      }

// ---------------------------------------------------------------------
// method count_nonzero_pixels_inside_convex_quadrilateral is
// essentially the inverse of the color_convex_quadrilaterial_interior
// method which is based upon the "midpoint line algorithm".  It
// returns the number of pixels contained within input quadrilateral q
// as well as the number of pixels whose intensities exceed input
// threshold zmin.  It also returns the quadrilateral's surface
// intensity integral.

   double count_nonzero_pixels_inside_convex_quadrilateral(
      double zmin,const polygon& q,twoDarray const *ztwoDarray_ptr,
      int& npixels_in_quad,int& npixels_above_zmin_in_quad)
      {
         int npixels_in_tri1,npixels_above_zmin_in_tri1;
         double intensity_integral1=count_nonzero_pixels_inside_triangle(
            zmin,q.get_vertex(0),q.get_vertex(1),q.get_vertex(2),
            ztwoDarray_ptr,
            npixels_in_tri1,npixels_above_zmin_in_tri1);

         int npixels_in_tri2,npixels_above_zmin_in_tri2;
         double intensity_integral2=count_nonzero_pixels_inside_triangle(
            zmin,q.get_vertex(2),q.get_vertex(3),q.get_vertex(0),
            ztwoDarray_ptr,npixels_in_tri2,npixels_above_zmin_in_tri2);

         npixels_in_quad=npixels_in_tri1+npixels_in_tri2;
         npixels_above_zmin_in_quad=npixels_above_zmin_in_tri1
            +npixels_above_zmin_in_tri2;
         return (intensity_integral1+intensity_integral2);
      }

// ---------------------------------------------------------------------
// Method nonzero_pixels_inside_poly_zones represents a generalization
// of integrated_intensity_inside_poly.  It takes in a twoDarray
// containing a multi-valued mask for some polygon which is assumed to
// be divided into nzones zones.  It also takes *ztwoDarray_ptr which
// contains some image intensity distribution.  This method further
// takes in bounding box corner pixel limits.  Scanning over the
// bounding box, this method sums up the numbers of non-null intensity
// valued pixels in *ztwoDarray_ptr in each zone defined by the mask.
// This method returns the total number of pixels within each zone and
// the number of non-null intensity values in each zone.

   void nonzero_pixels_inside_poly_zones(
      unsigned int min_px,unsigned int min_py,
      unsigned int max_px,unsigned int max_py,
      unsigned int nzones,double zone_intensity_value[],double zmin,
      int delta_px,int delta_py,
      twoDarray const *zmask_twoDarray_ptr,twoDarray const *ztwoDarray_ptr,
      int npixels_in_zone[],int npixels_above_zmin_in_zone[])
      {
         for (unsigned int n=0; n<nzones; n++)
         {
            npixels_in_zone[n]=0;
            npixels_above_zmin_in_zone[n]=0;
         }

         unsigned int px_lo=basic_math::max(
            Unsigned_Zero,min_px-abs(delta_px));
         unsigned int px_hi=basic_math::min(
            ztwoDarray_ptr->get_mdim(),(unsigned int) max_px+abs(delta_px));
         for (unsigned int px=px_lo; px<px_hi; px++)
         {
            int px_shifted=px-delta_px;
            if (px_shifted >= int(px_lo) && px_shifted < int(px_hi))
            {
               unsigned int py_lo=basic_math::max(
                  Unsigned_Zero,min_py-abs(delta_py));
               unsigned int py_hi=basic_math::min(
                  ztwoDarray_ptr->get_ndim(),(unsigned int) 
                  max_py+abs(delta_py));
               for (unsigned int py=py_lo; py<py_hi; py++)
               {
                  int py_shifted=py-delta_py;
                  if (py_shifted >= int(py_lo) && py_shifted < int(py_hi))
                  {
                     double zmask=zmask_twoDarray_ptr->
                        get(px_shifted,py_shifted);
                     if (zmask > 0)
                     {
                        for (unsigned int n=0; n<nzones; n++)
                        {
                           if (zmask==zone_intensity_value[n])
                           {
                              (npixels_in_zone[n])++;
                              double currz=ztwoDarray_ptr->get(px,py);
                              if (currz > zmin) 
                                 (npixels_above_zmin_in_zone[n])++;
                           }
                        } // loop over index n labeling polygon zone
                     }
                  } // py_shifted conditional
               } // loop over index py
            } // px_shifted conditional
         }  // loop over index px
//         cout << "Number of pixels inside poly = " << npixels_inside_poly 
//              << endl;
//         cout << "Number of nonzero pixels inside poly = "
//              << npixels_above_zmin << endl;
//         cout << "Integrated intensity inside poly = " << intensity_integral 
//              << endl;
//         if (npixels_above_zmin > 0)
//         {
//            cout << "Avg intensity inside poly = " 
//                 << intensity_integral/npixels_above_zmin << endl;
//         }
      }

// ---------------------------------------------------------------------
   void light_nonzero_pixels_inside_poly(
      double zmin,twoDarray *ztwoDarray_ptr,
      twoDarray const *zmask_twoDarray_ptr)
      {
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (zmask_twoDarray_ptr->get(px,py) > 0)
               {
                  double currz=ztwoDarray_ptr->get(px,py);
                  if (currz > zmin) 
                  {
                     ztwoDarray_ptr->put(px,py,POSITIVEINFINITY);
                  }
                  else
                  {
                     ztwoDarray_ptr->put(px,py,60);
                  }
               }
            } // loop over index py
         }  // loop over index px
      }

// ---------------------------------------------------------------------
// Method image_intensity_distribution scans through an image array
// and computes the cumulative probability distribution for pixel
// intensities.  This method returns the number of pixels whose
// intensities exceed input parameter z_minimum.

   int image_intensity_distribution(
      double z_minimum,twoDarray const *ztwoDarray_ptr,
      prob_distribution& prob,int n_output_bins)
      {
         int npixels_above_zmin=0;

// Dynamically rather than statically declare next 1D array to avoid
// running out of stack space:

         double *intensity=new double[ztwoDarray_ptr->get_dimproduct()];
         for (unsigned int i=0; i<ztwoDarray_ptr->get_dimproduct(); i++)
         {
            double currz=ztwoDarray_ptr->Tensor<double>::get(i);
            if (currz > z_minimum) intensity[npixels_above_zmin++]=currz;
         }	
         prob.fill_existing_distribution(
            npixels_above_zmin,intensity,n_output_bins);
         delete [] intensity;
//   prob.cumulativefilenamestr=imagedir+"intensity"
//      +stringfunc::number_to_string(imagenumber)+".meta";
//   prob.xmin=0;
//   prob.set_xlabel("Relative Normalized Intensity (dB)");
//   prob.writeprobdists();
         return npixels_above_zmin;
      }
   
   prob_distribution* image_intensity_distribution(
      double z_minimum,twoDarray const *ztwoDarray_ptr,
      int& npixels_above_zmin,int n_output_bins)
      {
         npixels_above_zmin=0;

// Dynamically rather than statically declare next 1D array to avoid
// running out of stack space:

//       double intensity[ztwoDarray_ptr->get_dimproduct()];
         double *intensity=new double[ztwoDarray_ptr->get_dimproduct()];
         for (unsigned int i=0; i<ztwoDarray_ptr->get_dimproduct(); i++)
         {
            double currz=ztwoDarray_ptr->Tensor<double>::get(i);
            if (currz > z_minimum) intensity[npixels_above_zmin++]=currz;
         }	

         prob_distribution* prob_ptr=new prob_distribution(
            npixels_above_zmin,intensity,n_output_bins);
         delete [] intensity;
//   prob.cumulativefilenamestr=imagedir+"intensity"
//      +stringfunc::number_to_string(imagenumber)+".meta";
//   prob.xmin=0;
//   prob.set_xlabel("Relative Normalized Intensity (dB)");
//   prob.writeprobdists();
         return prob_ptr;
      }

// ==========================================================================
// Center of mass and moment of inertia methods
// ==========================================================================

// Method center_of_mass computes the x and y coordinates of the input
// *ztwoDarray_ptr's COM.  If the twoDarray contains positive
// intensity content, this boolean method returns true.

   bool center_of_mass(twoDarray const *ztwoDarray_ptr,threevector& COM) 
      {
         return center_of_mass(
            0,ztwoDarray_ptr->get_mdim(),
            0,ztwoDarray_ptr->get_ndim(),ztwoDarray_ptr,COM);
      }

   bool center_of_mass(
      double minimum_x,double maximum_x,double minimum_y,double maximum_y,
      twoDarray const *ztwoDarray_ptr,threevector& COM) 
      {
         unsigned int min_px,max_px,min_py,max_py;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            return center_of_mass(
               min_px,max_px,min_py,max_py,ztwoDarray_ptr,COM);
         } // bbox_corners_to_pixels conditional
         else
         {
            return false;
         }
      }

   bool center_of_mass(
      unsigned int min_px,unsigned int max_px,
      unsigned int min_py,unsigned int max_py,
      twoDarray const *ztwoDarray_ptr,threevector& COM) 
      {
         double currx,curry;
         double xsum=0;
         double ysum=0;
         double zsum=0;
         for (unsigned int i=min_px; i<max_px; i++)
         {
            for (unsigned int j=min_py; j<max_py; j++)
            {
               ztwoDarray_ptr->pixel_to_point(i,j,currx,curry);
               double currz=ztwoDarray_ptr->get(i,j);
               xsum += currz*currx;
               ysum += currz*curry;
               zsum += currz;
            }
         }

         bool COM_calculated_successfully=true;
         if (zsum > 0)
         {
            COM.put(0,xsum/zsum);
            COM.put(1,ysum/zsum);
            COM.put(2,0);
         }
         else
         {
            outputfunc::newline();
            cout << "Error in imagefunc::center_of_mass()!" << endl;
            cout << "zsum = " << zsum << " is <= 0 !!!" << endl << endl;
            COM_calculated_successfully=false;
         }
         return COM_calculated_successfully;
      }

// ---------------------------------------------------------------------
// Method binary_COM_above_zmin computes the 2D center-of-mass for
// binary thresholded pixels within the bounding box of
// *ztwoDarray_ptr specified by the input extremal X and Y values.  

   bool binary_COM_above_zmin(
      double minimum_x,double maximum_x,double minimum_y,double maximum_y,
      double z_min,twoDarray const *ztwoDarray_ptr,twovector& COM) 
      {
         unsigned int min_px,max_px,min_py,max_py;
         if (ztwoDarray_ptr->bbox_corners_to_pixels(
            minimum_x,minimum_y,maximum_x,maximum_y,
            min_px,min_py,max_px,max_py))
         {
            return binary_COM_above_zmin(
               min_px,max_px,min_py,max_py,z_min,
               ztwoDarray_ptr,COM);
         }
         else
         {
            return false;
         }
      }

   bool binary_COM_above_zmin(
      unsigned int min_px,unsigned int max_px,
      unsigned int min_py,unsigned int max_py,double z_min,
      twoDarray const *ztwoDarray_ptr,twovector& COM) 
      {
         double currx,curry;
         double x_numer=0;
         double y_numer=0;
         double denom=0;
         for (unsigned int i=min_px; i<max_px; i++)
         {
            for (unsigned int j=min_py; j<max_py; j++)
            {
               if (ztwoDarray_ptr->get(i,j) > z_min &&
                   ztwoDarray_ptr->pixel_to_point(i,j,currx,curry) )
               {
                  x_numer += 1*currx;
                  y_numer += 1*curry;
                  denom += 1;
               }
            }
         }

         bool COM_calculated_successfully=true;
         if (denom > 0)
         {
            COM.put(0,x_numer/denom);
            COM.put(1,y_numer/denom);
         }
         else
         {
            outputfunc::newline();
            cout << "Error in imagefunc::binary_COM_above_zmin()!" << endl;
            cout << "denom = " << denom << " is <= 0 !!!" << endl << endl;
            COM_calculated_successfully=false;
         }
         return COM_calculated_successfully;
      }

// ---------------------------------------------------------------------
// Method moment_of_inertia first constructs the image's 2D
// "moment-of-inertia" tensor relative to some specified input origin
// point.  It then computes the eigenalues and eigenvectors of this
// matrix which correspond to the principle axes within the image.

   void moment_of_inertia(
      const threevector& origin,double& Imin,double& Imax,
      twoDarray const *ztwoDarray_ptr)
      {
         double Ixx,Iyy,Ixy;
         moment_of_inertia(origin,Ixx,Iyy,Ixy,Imin,Imax,ztwoDarray_ptr);
      }
   
   void moment_of_inertia(
      const threevector& origin,
      double& Ixx,double& Iyy,double& Ixy,double& Imin,double& Imax,
      twoDarray const *ztwoDarray_ptr)
      {
         const double TINY=1E-8;
   
         double currx,curry;
         double xsqsum=0;
         double ysqsum=0;
         double xysum=0;
         double zsum=0;
         for (unsigned int i=0; i<ztwoDarray_ptr->get_mdim(); i++)
         {
            for (unsigned int j=0; j<ztwoDarray_ptr->get_ndim(); j++)
            {
               ztwoDarray_ptr->pixel_to_point(i,j,currx,curry);
               double dx=currx-origin.get(0);
               double dy=curry-origin.get(1);

               double currz=ztwoDarray_ptr->get(i,j);
               if (currz < -TINY)
               {
                  cout << "Error in moment_of_inertia()!" << endl;
                  cout << "i = " << i << " j = " << j 
                       << " currx = " << currx << " curry = " << curry
                       << " currz = " << currz << endl;
                  sleep(2);
               }
               else if (currz >= -TINY && currz < 0)
               {
                  currz=0;
               }

               xsqsum += currz*sqr(dx);
               ysqsum += currz*sqr(dy);
               xysum += currz*(dx*dy);
               zsum += currz;
            } // loop over index j
         } // loop over index i
         Ixx=ysqsum/zsum;
         Iyy=xsqsum/zsum;
         Ixy=-xysum/zsum;

//   cout << "Ixx = " << Ixx << " Iyy = " << Iyy << " Ixy = " << Ixy << endl;

         double I1=(Ixx+Iyy+sqrt(sqr(Ixx-Iyy)+4*sqr(Ixy)))/2;
         double I2=(Ixx+Iyy-sqrt(sqr(Ixx-Iyy)+4*sqr(Ixy)))/2;
         Imin=basic_math::min(I1,I2);
         Imax=basic_math::max(I1,I2);

/*
// If Ixy vanishes, then the principle axes lie in the x and y directions:

         double theta_min,theta_max,theta_min2,theta_max2;
         if (abs(Ixy) < TINY)
         {
            if (Ixx < Iyy)
            {
               theta_min=0;
               theta_max=PI/2;
            }
            else
            {
               theta_min=PI/2;
               theta_max=0;
            }
         }
         else
         {
            theta_min=-atan2(Ixy,Iyy-Imin);
            theta_max=-atan2(Ixy,Iyy-Imax);
            theta_min2=-atan2(Ixx-Imin,Ixy);
            theta_max2=-atan2(Ixx-Imax,Ixy);
         }
         Imin=threevector(cos(theta_min),sin(theta_min),0);
         Imax_hat=threevector(-Imin_hat.get(1),Imin_hat.get(0),0);
*/

//   cout << "Imin = " << Imin << " Imax = " << Imax << endl;
//   cout << "theta_min = " << theta_min*180/PI 
//        << " theta_max = " << theta_max*180/PI << endl;
//   cout << "theta_min2 = " << theta_min2*180/PI 
//        << " theta_max2 = " << theta_max2*180/PI << endl;
//   cout << "Imax_hat = " << Imax_hat << endl;
//   cout << "Imin_hat = " << Imin_hat << endl;
      }

   void moment_of_inertia(
      const threevector& origin,double& Imin,double& Imax,
      threevector& Imin_hat,threevector& Imax_hat,
      twoDarray const *ztwoDarray_ptr)
      {
         double Ixx,Iyy,Ixy;
         moment_of_inertia(origin,Ixx,Iyy,Ixy,Imin,Imax,ztwoDarray_ptr);

// If Ixy vanishes, then the principle axes lie in the x and y directions:

         const double TINY=1E-8;
         double theta_min;
//         double theta_max,theta_min2,theta_max2;
         if (fabs(Ixy) < TINY)
         {
            if (Ixx < Iyy)
            {
               theta_min=0;
//               theta_max=PI/2;
            }
            else
            {
               theta_min=PI/2;
//               theta_max=0;
            }
         }
         else
         {
            theta_min=-atan2(Ixy,Iyy-Imin);
//            theta_max=-atan2(Ixy,Iyy-Imax);
//            theta_min2=-atan2(Ixx-Imin,Ixy);
//            theta_max2=-atan2(Ixx-Imax,Ixy);
         }

         Imin_hat=threevector(cos(theta_min),sin(theta_min));
         Imax_hat=threevector(-Imin_hat.get(1),Imin_hat.get(0));


//   cout << "Imin = " << Imin << " Imax = " << Imax << endl;
//   cout << "theta_min = " << theta_min*180/PI 
//        << " theta_max = " << theta_max*180/PI << endl;
//   cout << "theta_min2 = " << theta_min2*180/PI 
//        << " theta_max2 = " << theta_max2*180/PI << endl;
//   cout << "Imax_hat = " << Imax_hat << endl;
//   cout << "Imin_hat = " << Imin_hat << endl;
      }

// ---------------------------------------------------------------------
// This overloaded version of method moment_of_inertia essentially
// computes the characteristic spread of intensities about some
// specified axis which runs through some specified origin.  If the
// moment is not successfully calculated, this boolean method returns
// false.

   bool moment_of_inertia(
      const threevector& center_pnt,const threevector& axis,
      twoDarray const *ztwoDarray_ptr,double& I)
      {
         const double min_intensity=1;	// dBsm
         double currx,curry;
         double bsqsum=0;
         double zsum=0;
         threevector a_hat(axis.unitvector());
         for (unsigned int i=0; i<ztwoDarray_ptr->get_mdim(); i++)
         {
            for (unsigned int j=0; j<ztwoDarray_ptr->get_ndim(); j++)
            {
               double currz=ztwoDarray_ptr->get(i,j);
               if (currz > min_intensity)
               {
                  ztwoDarray_ptr->pixel_to_point(i,j,currx,curry);
                  threevector v(currx,curry,0);
                  threevector d(v-center_pnt);
                  bsqsum += currz*(sqr(d.magnitude())-sqr(d.dot(a_hat)));
               }
               zsum += currz;
            }
         }

         bool moi_successfully_calculated=false;
         const double TINY=1E-13;
         if (zsum > 0 && bsqsum > TINY)
         {
            I=bsqsum/zsum;
            moi_successfully_calculated=true;
//            cout << "Inside myimage::moment_of_inertia()" << endl;
//            cout << "I = " << I << " sqrt(I) = " << sqrt(I) << endl;
//            cout << "bsqsum = " << bsqsum << " zsum = " << zsum << endl;
         }
         return moi_successfully_calculated;
      }

// ==========================================================================
// Image processing methods
// ==========================================================================

// Method compute_pixel_borders performs a minimal amount of
// vertical/horizontal profiling in order to establish the pixel
// boundaries of an image outside of which all intensities are nearly
// null:

   void compute_pixel_borders
      (twoDarray const *ztwoDarray_ptr,
       unsigned int& px_min,unsigned int& px_max,
       unsigned int& py_min,unsigned int& py_max)
      {
         unsigned int mdim=ztwoDarray_ptr->get_mdim();
         unsigned int ndim=ztwoDarray_ptr->get_ndim();
         unsigned int px,py;
         
         const double TINY_intensity=0.01;
         const double small_intensity=5;	// dBsm
         
         px=0;
         double intensity_sum=0;
         int n_nonzero_pixels=0;
         double avg_intensity=0;
         double currz;
         do
         {
            for (py=0; py<ndim; py++)
            {
               currz=ztwoDarray_ptr->get(px,py);
               if (currz > TINY_intensity)
               {
                  intensity_sum += currz;
                  n_nonzero_pixels++;
                  avg_intensity=intensity_sum/double(n_nonzero_pixels);
               }
            }
            if (avg_intensity < small_intensity) px++;
         }
         while (avg_intensity < small_intensity && px < mdim);
         px_min=px;
         
         px=mdim-1;
         intensity_sum=avg_intensity=0;
         n_nonzero_pixels=0;
         do
         {
            for (py=0; py<ndim; py++)
            {
               currz=ztwoDarray_ptr->get(px,py);
               if (currz > TINY_intensity)
               {
                  intensity_sum += currz;
                  n_nonzero_pixels++;
                  avg_intensity=intensity_sum/double(n_nonzero_pixels);
               }
            }
            if (avg_intensity < small_intensity) px--;
         }
         while (avg_intensity < small_intensity && px >= 0);
         px_max=px;
         
         py=0;
         intensity_sum=avg_intensity=0;
         n_nonzero_pixels=0;
         do
         {
            for (px=0; px<mdim; px++)
            {
               currz=ztwoDarray_ptr->get(px,py);
               if (currz > TINY_intensity)
               {
                  intensity_sum += currz;
                  n_nonzero_pixels++;
                  avg_intensity=intensity_sum/double(n_nonzero_pixels);
               }
            }
            if (avg_intensity < small_intensity) py++;
         }
         while (avg_intensity < small_intensity && py < ndim);
         py_min=py;
         
         py=ndim-1;
         intensity_sum=avg_intensity=0;
         n_nonzero_pixels=0;
         do
         {
            for (px=0; px<mdim; px++)
            {
               currz=ztwoDarray_ptr->get(px,py);
               if (currz > TINY_intensity)
               {
                  intensity_sum += currz;
                  n_nonzero_pixels++;
                  avg_intensity=intensity_sum/double(n_nonzero_pixels);
               }
            }
            if (avg_intensity < small_intensity) py--;
         }
         while (avg_intensity < small_intensity && py >= 0);
         py_max=py;

// Perform sanity check on pixel border values:

         if (px_min >= mdim) px_min=0;
         if (px_max < 0) px_max=mdim-1;
         if (py_min >= ndim) py_min=0;
         if (py_max < 0) py_max=ndim-1;

//         cout << "Inside imagefunc::compute_pixel_borders()" << endl;
//         cout << "px_min = " << px_min << " px_max = " << px_max << endl;
//         cout << "py_min = " << py_min << " py_max = " << py_max << endl;
//         double x_min,x_max,y_min,y_max;
//         ztwoDarray_ptr->pixel_to_point(px_min,py_max,x_min,y_min);
//         ztwoDarray_ptr->pixel_to_point(px_max,py_min,x_max,y_max);
//         cout << "x_min = " << x_min << " x_max = " << x_max << endl;
//         cout << "y_min = " << y_min << " y_max = " << y_max << endl;
//         draw_bbox(
//            x_min,y_min,x_max,y_max,colorfunc::white,ztwoDarray_ptr);
      }
      
// ---------------------------------------------------------------------
// Method integrate_absolute_difference_image takes in two image
// twoDarrays *ztwoDarray1_ptr and *ztwoDarray2_ptr which are assumed
// to have the same dimensions and bin sizes.  It subtracts the latter
// from the former.  This method then returns the integral of the
// absolute difference.  We have consciously attempted to optimize
// this method for speed.

   double integrate_absolute_difference_image(
      double z_threshold,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr)
      {
         return integrate_absolute_difference_image(
            z_threshold,ztwoDarray1_ptr,ztwoDarray2_ptr,
            0,0);
      }
   
   double integrate_absolute_difference_image(
      double z_threshold,
      int bbox_px_min,int bbox_px_max,int bbox_py_min,int bbox_py_max,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr)
      {
         return integrate_absolute_difference_image(
            true,z_threshold,bbox_px_min,bbox_px_max,bbox_py_min,bbox_py_max,
            ztwoDarray1_ptr,ztwoDarray2_ptr,0,0);
      }
   
// In this overloaded version of integrate_absolute_difference_image,
// *ztwoDarray2_ptr is shifted relative to *ztwoDarray1_ptr by pixel
// displacements delta_px and delta_py before the integral is
// calculated:

   double integrate_absolute_difference_image(
      double z_threshold,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr,
      int delta_px,int delta_py)
      {
         return integrate_absolute_difference_image(
            z_threshold,0,ztwoDarray2_ptr->get_mdim()-1,
            0,ztwoDarray2_ptr->get_ndim()-1,ztwoDarray1_ptr,ztwoDarray2_ptr,
            delta_px,delta_py);
      }
   
   double integrate_absolute_difference_image(
      double z_threshold,
      unsigned int bbox_px_min,unsigned int bbox_px_max,
      unsigned int bbox_py_min,unsigned int bbox_py_max,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr,
      int delta_px,int delta_py)
      {
         return integrate_absolute_difference_image(
            false,z_threshold,bbox_px_min,bbox_px_max,bbox_py_min,bbox_py_max,
            ztwoDarray1_ptr,ztwoDarray2_ptr,delta_px,delta_py);
      }
   
   double integrate_absolute_difference_image(
      bool binary_threshold_ztwoDarray1,double z_threshold,
      unsigned int bbox_px_min,unsigned int bbox_px_max,
      unsigned int bbox_py_min,unsigned int bbox_py_max,
      twoDarray const *ztwoDarray1_ptr,twoDarray const *ztwoDarray2_ptr,
      int delta_px,int delta_py)
      {
         double abs_diff_integral=0;
         unsigned int px_lo=basic_math::max(
            (unsigned int) 0,bbox_px_min-abs(delta_px));
         unsigned int px_hi=basic_math::min(ztwoDarray2_ptr->get_mdim(),
                                            bbox_px_max+abs(delta_px));

         for (unsigned int px=px_lo; px<px_hi; px++)
         {
            int px_shifted=px-delta_px;
            if (px_shifted >= int(px_lo) && px_shifted < int(px_hi))
            {
               unsigned int py_lo=
                  basic_math::max((unsigned int) 0,bbox_py_min-abs(delta_py));
               unsigned int py_hi=basic_math::min(ztwoDarray2_ptr->get_ndim(),
                                                  bbox_py_max+abs(delta_py));
               for (unsigned int py=py_lo; py<py_hi; py++)
               {
                  int py_shifted=py-delta_py;
                  if (py_shifted >= int(py_lo) && py_shifted < int(py_hi))
                  {
                     double z1=ztwoDarray1_ptr->get(px,py);
                     if (binary_threshold_ztwoDarray1)
                     {
                        if (z1 > z_threshold)
                        {
                           z1=1;
                        }
                        else
                        {
                           z1=0;
                        }
                     }

                     double z2;
                     if (ztwoDarray2_ptr->get(px_shifted,py_shifted) 
                         > z_threshold)
                     {
                        z2=1;
                     }
                     else
                     {
                        z2=0;
                     }
                     abs_diff_integral += fabs(z1-z2);
                  } // py_shifted conditional
               } // py loop
            } // px_shifted conditional
         } // px loop

         const double dA=ztwoDarray1_ptr->get_deltax()*
            ztwoDarray1_ptr->get_deltay();
         abs_diff_integral *= dA;
         return abs_diff_integral;
      }

// ==========================================================================
// Median filtering methods
// ==========================================================================

// Method median_filter takes in an image within *ztwoDarray_ptr along
// with(odd!) integers nx_size and ny_size.  It returns within output
// twoDarray *ztwoDarray_filtered_ptr the median values computed
// within a moving nx_size x ny_size square window.

   void median_filter(unsigned int nsize,twoDarray *ztwoDarray_ptr)
      {
         median_filter(nsize,nsize,ztwoDarray_ptr);
      }

   void median_filter(
      unsigned int nx_size,unsigned int ny_size,twoDarray *ztwoDarray_ptr)
      {
         twoDarray* ztwoDarray_filtered_ptr=new twoDarray(ztwoDarray_ptr);
         median_filter(nx_size,ny_size,ztwoDarray_ptr,
                       ztwoDarray_filtered_ptr);
         ztwoDarray_filtered_ptr->copy(ztwoDarray_ptr);
         delete ztwoDarray_filtered_ptr;
      }

   void median_filter(unsigned int nsize,twoDarray const *ztwoDarray_ptr,
                      twoDarray *ztwoDarray_filtered_ptr)
      {
         median_filter(nsize,nsize,ztwoDarray_ptr,ztwoDarray_filtered_ptr);
      }


   void median_filter(
      unsigned int nx_size,unsigned int ny_size,
      twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,
      double irrelevant_intensity,
      bool ignore_z_greater_than_irrelevant_intensity)
      {
         ztwoDarray_ptr->copy(ztwoDarray_filtered_ptr);
   
         if (is_even(nx_size)) nx_size++;
         if (is_even(ny_size)) ny_size++;
         unsigned int npixels=nx_size*ny_size;
         double intensity[npixels];

         unsigned int wx=(nx_size-1)/2;
         unsigned int wy=(ny_size-1)/2;
         for (unsigned int py=wy; py<ztwoDarray_ptr->get_ndim()-wy; py++)
         {
            for (unsigned int px=wx; px<ztwoDarray_ptr->get_mdim()-wx; px++)
            {
               int n=0;
               for (unsigned int j=0; j<ny_size; j++)
               {
                  for (unsigned int i=0; i<nx_size; i++)
                  {

                     double curr_intensity=
                        ztwoDarray_ptr->get(px-wx+i,py-wy+j);
                     if ((ignore_z_greater_than_irrelevant_intensity &&
                          curr_intensity < irrelevant_intensity) ||
                         (!ignore_z_greater_than_irrelevant_intensity &&
                          curr_intensity > irrelevant_intensity))
                     {
                        intensity[n++]=curr_intensity;
                     }
                  } // loop over index i 
               } // loop over index j 
               Quicksort(intensity,n);
               ztwoDarray_filtered_ptr->put(px,py,intensity[npixels/2]);
            } // loop over index px
         } // loop over index py
      }

// ---------------------------------------------------------------------
   bool median_filter(
      unsigned int px,unsigned int py,unsigned int nsize,
      twoDarray const *ztwoDarray_ptr,
      twoDarray* ztwoDarray_filtered_ptr,double minimum_z_value)
      {
         if (is_even(nsize)) nsize++;
         vector<double> intensity;

         unsigned int w=(nsize-1)/2;
         for (unsigned int qx=px-w; qx <= px+w; qx++)
         {
            for (unsigned int qy=py-w; qy <= py+w; qy++)
            {
               if (!ztwoDarray_ptr->pixel_inside_working_region(qx,qy))
                  continue;
               double curr_Z=ztwoDarray_ptr->get(qx,qy);
               if (curr_Z > minimum_z_value) intensity.push_back(curr_Z);
            }
         }
         
         if (intensity.size() >=3 )
         {
            std::sort(intensity.begin(),intensity.end());
            double median_value=intensity[intensity.size()/2];
            if (median_value < minimum_z_value)
            {
               cout << "Error: min_z_value = " << minimum_z_value
                    << " median_value = " << median_value << endl;
               exit(-1);
            }
            
            ztwoDarray_filtered_ptr->put(px,py,median_value);
            return true;
         }
         else
         {
            return false;
         }
      }

// ---------------------------------------------------------------------
// Method fast_percentile_filter() first computes the minimum and maximum 
// intensity values within input *ztwoDarray_ptr.  It creates a set of
// intensity bins which range evenly from min_intensity to
// max_intensity.  Looping over (most) pixels (px,py) within
// *ztwoDarray_ptr, this method extracts a wx x wy patch of
// intensities around (px,py).  If px == wx this method generates a
// new histogram from the patch intensities.  The intensity histogram
// is then converted into a cumulative probability distribution from
// which the requested percentile intensity is recovered.  For px >
// wx, the previous patch's left-most pixel intensities are removed
// from the histogram, while the current patch's right-most pixel
// intensities are added to the histogram.  

// This approximate approach to image median filtering is
// significantly faster than the brute-force approach which performs
// expensive sorting for every pixel patch!

   void fast_median_filter(
      unsigned int n_intensity_bins, int nsize, 
      twoDarray const *ztwoDarray_ptr, twoDarray *ztwoDarray_filtered_ptr)
   {
      fast_percentile_filter(
         n_intensity_bins, nsize, nsize, 0.5, 
         ztwoDarray_ptr, ztwoDarray_filtered_ptr);
   }

   void fast_percentile_filter(
      unsigned int n_intensity_bins, 
      unsigned int nx_size, unsigned int ny_size, double cum_frac, 
      twoDarray const *ztwoDarray_ptr, twoDarray *ztwoDarray_filtered_ptr)
      {
//         cout << "inside imagefunc::fast_percentile_filter()" << endl;
	
         double min_intensity, max_intensity;
         ztwoDarray_ptr->minmax_values(min_intensity, max_intensity);
//	 cout << "min_intensity = " << min_intensity
//	      << "max_intensity = " << max_intensity << endl;
         if (nearly_equal(min_intensity,max_intensity))
         {
            ztwoDarray_filtered_ptr->initialize_values(min_intensity);
            return;
         }

         double d_intensity = (max_intensity - min_intensity)/(
            n_intensity_bins - 1);

// Establish intensity bins ranging from min_intensity to max_intensity:

         vector<double> X;
         X.reserve(n_intensity_bins);
         for (unsigned int k=0; k<n_intensity_bins; k++)
         {
            X.push_back(min_intensity + k*d_intensity);
         }

         vector<double> Xhistogram, Pcum;
         Xhistogram.reserve(n_intensity_bins);
         Pcum.reserve(n_intensity_bins);

         for (unsigned int k=0; k<n_intensity_bins; k++)
         {
            Pcum.push_back(0);
         }


         ztwoDarray_ptr->copy(ztwoDarray_filtered_ptr);
   
         if (is_even(nx_size)) nx_size++;
         if (is_even(ny_size)) ny_size++;
         int n_patch_pixels=nx_size*ny_size;

         unsigned int wx=(nx_size-1)/2;
         unsigned int wy=(ny_size-1)/2;
         for (unsigned int py=wy; py<ztwoDarray_ptr->get_ndim()-wy; py++)
         {
            for (unsigned int px=wx; px<ztwoDarray_ptr->get_mdim()-wx; px++)
            {

               if (px==wx)
               {

// Clear X histogram and then fill it with pixel intensities
// surrounding (px,py):

                  Xhistogram.clear();
                  for (unsigned int k=0; k<n_intensity_bins; k++)
                  {
                     Xhistogram.push_back(0);
                  }

                  for (unsigned int i=0; i<nx_size; i++)
                  {
                     for (unsigned int j=0; j<ny_size; j++)
                     {
                        double curr_intensity=
                           ztwoDarray_ptr->get(px-wx+i,py-wy+j);

                        int k=basic_math::round( 
                           (curr_intensity-min_intensity)/d_intensity);
                        Xhistogram[k] = Xhistogram[k]+1;

                     } // loop over index j
                  }  // loop over index i
               }
               else
               {
                  int i=-1;
                  for (unsigned int j=0; j<ny_size; j++)
                  {
                     double curr_intensity=
                        ztwoDarray_ptr->get(px-wx+i,py-wy+j);
                     
                     int k=basic_math::round( 
                        (curr_intensity-min_intensity)/d_intensity);
                     Xhistogram[k] = Xhistogram[k]-1;
                  } // loop over index j

                  i=nx_size-1;
                  for (unsigned int j=0; j<ny_size; j++)
                  {
                     double curr_intensity=
                        ztwoDarray_ptr->get(px-wx+i,py-wy+j);
                     
                     int k=basic_math::round( 
                        (curr_intensity-min_intensity)/d_intensity);
                     Xhistogram[k] = Xhistogram[k]+1;
                  } // loop over index j
               }
               
// Renormalize X histogram entries so that they become probability
// density values.  Then compute cumulative probability distribution:

               double pcum_prev = 0;
               for (unsigned int k=0; k<n_intensity_bins; k++)
               {
                  double curr_p = Xhistogram[k] / n_patch_pixels;
                  Pcum[k] = pcum_prev + curr_p;
                  pcum_prev += curr_p;
               }

               double percentile_p = mathfunc::find_x_corresponding_to_pcum(
                  X, Pcum, cum_frac);

               ztwoDarray_filtered_ptr->put(px,py,percentile_p);

            } // loop over index px
         } // loop over index py
      }

// ---------------------------------------------------------------------
// Method probability_filter takes in twoDarray *ztwoDarray_ptr and
// computes the intensity distribution within an nx_size x ny_size
// window about each pixel.  It returns within output twoDarray
// *ztwoDarray_filtered_ptr the intensities corresponding to the
// prob_frac percentile of the intensity distributions.  Only
// intensities less than the input irrelevant_intensity value are used
// within the intensity distribution computation.

   void probability_filter(
      unsigned int nx_size,unsigned int ny_size,
      twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double prob_frac,
      double irrelevant_intensity)
      {
         if (is_even(nx_size)) nx_size++;
         if (is_even(ny_size)) ny_size++;
         int npixels=nx_size*ny_size;
         double intensity[npixels];

         unsigned int wx=(nx_size-1)/2;
         unsigned int wy=(ny_size-1)/2;
         for (unsigned int px=wx; px<ztwoDarray_ptr->get_mdim()-wx; px++)
         {
            for (unsigned int py=wy; py<ztwoDarray_ptr->get_ndim()-wy; py++)
            {
               unsigned int n=0;
               for (unsigned int i=0; i<nx_size; i++)
               {
                  for (unsigned int j=0; j<ny_size; j++)
                  {
                     double curr_intensity=
                        ztwoDarray_ptr->get(px-wx+i,py-wy+j);
                     if (curr_intensity < irrelevant_intensity)
                     {
                        intensity[n++]=curr_intensity;
                     }
                  }
               } 
               Quicksort(intensity,n);
               ztwoDarray_filtered_ptr->put(
                  px,py,intensity[basic_math::round((n-1)*prob_frac)]);
            } // loop over index py
         } // loop over index px
      }

// ---------------------------------------------------------------------
// Method average_filter() takes in twoDarray *ztwoDarray_ptr and
// brute-force computes the average intensity within an nx_size x
// ny_size window about each pixel.  It returns the averaged results
// within output twoDarray *ztwoDarray_filtered_ptr.  Only intensities
// less than the input irrelevant_intensity value are used in the
// averaging computation.

   void average_filter(
      unsigned int nx_size,unsigned int ny_size,
      twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double irrelevant_intensity)
      {
         if (is_even(nx_size)) nx_size++;
         if (is_even(ny_size)) ny_size++;
         int npixels=nx_size*ny_size;
         vector<double> intensity;
         intensity.reserve(npixels);

         unsigned int wx=(nx_size-1)/2;
         unsigned int wy=(ny_size-1)/2;
         for (unsigned int px=wx; px<ztwoDarray_ptr->get_mdim()-wx; px++)
         {
            for (unsigned int py=wy; py<ztwoDarray_ptr->get_ndim()-wy; py++)
            {
               for (unsigned int i=0; i<nx_size; i++)
               {
                  for (unsigned int j=0; j<ny_size; j++)
                  {
                     double curr_intensity=
                        ztwoDarray_ptr->get(px-wx+i,py-wy+j);
                     if (curr_intensity < irrelevant_intensity)
                     {
                        intensity.push_back(curr_intensity);
                     }
                  }
               } 
               double intensity_sum=0;
               for (unsigned int n=0; n<intensity.size(); n++)
               {
                  intensity_sum += intensity[n];
               }
               ztwoDarray_filtered_ptr->put(
                  px,py,intensity_sum/intensity.size());
               intensity.clear();
            } // loop over index py
         } // loop over index px
      }

// Method average_nonnull_values() is a slightly different and
// improved version of average_filter().  It works with pixels
// within *ztwoDarray_ptr whose values exceed input null_value. 

   void average_nonnull_values(
      unsigned int nx_size,unsigned int ny_size,
      twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double null_value)
      {
//         cout << "inside imagefunc::average_nonnull_values()" << endl;
         
         if (is_even(nx_size)) nx_size++;
         if (is_even(ny_size)) ny_size++;
         int npixels=nx_size*ny_size;
         vector<double> intensity;
         intensity.reserve(npixels);

         unsigned int wx=(nx_size-1)/2;
         unsigned int wy=(ny_size-1)/2;
         for (unsigned int px=wx; px<ztwoDarray_ptr->get_mdim()-wx; px++)
         {
            for (unsigned int py=wy; py<ztwoDarray_ptr->get_ndim()-wy; py++)
            {
               for (unsigned int i=0; i<nx_size; i++)
               {
                  for (unsigned int j=0; j<ny_size; j++)
                  {
                     double curr_intensity=
                        ztwoDarray_ptr->get(px-wx+i,py-wy+j);
                     if (curr_intensity > null_value)
                     {
                        intensity.push_back(curr_intensity);
                     }
                  } // loop over index j
               } // loop over index i

               if (intensity.size()==0)
               {
                  ztwoDarray_filtered_ptr->put(px,py,null_value);
               }
               else
               {
                  double intensity_sum=0;
                  for (unsigned int n=0; n<intensity.size(); n++)
                  {
                     intensity_sum += intensity[n];
                  }
                  ztwoDarray_filtered_ptr->put(
                     px,py,intensity_sum/intensity.size());
               }
               intensity.clear();
            } // loop over index py
         } // loop over index px
      }

// ---------------------------------------------------------------------
// Method median_fill applies median filtering only to those points
// whose intensities = null_value.  Moreover, null_valued pixels are
// not used to determine median intensities.  This method consequently
// applies as little median filtering as possible to imagery in order
// to avoid unnecessarily smearing raw data.  In this 2011 version,
// null valued entries within input *ztwoDarray_ptr are replaced with
// their median filled analogs.

   void median_fill(
      unsigned int nsize,twoDarray *ztwoDarray_ptr,double null_value)
      {
//         cout << "inside imagefunc::median_fill() #1" << endl;
         
         twoDarray* ztwoDarray_filtered_ptr=new twoDarray(ztwoDarray_ptr);
         ztwoDarray_filtered_ptr->initialize_values(null_value);
         
         median_fill(nsize,ztwoDarray_ptr,ztwoDarray_filtered_ptr,
                     null_value);

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double curr_z=ztwoDarray_ptr->get(px,py);
               if (nearly_equal(curr_z,null_value))
               {
                  double filtered_z=ztwoDarray_filtered_ptr->get(px,py);
                  ztwoDarray_ptr->put(px,py,filtered_z);
               }
            } // loop over py index
         } // loop over px index
         
         delete ztwoDarray_filtered_ptr;
      }

   void old_median_fill(
      unsigned int nsize,twoDarray *ztwoDarray_ptr,double null_value)
      {
         twoDarray* ztwoDarray_filtered_ptr=new twoDarray(ztwoDarray_ptr);
         median_fill(nsize,ztwoDarray_ptr,ztwoDarray_filtered_ptr,
                     null_value);
         ztwoDarray_filtered_ptr->copy(ztwoDarray_ptr);
         delete ztwoDarray_filtered_ptr;
      }
 
   void median_fill(
      unsigned int nsize,polygon& bbox,twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double null_value)
      {
         if (is_even(nsize)) nsize++;
         double intensity[nsize*nsize];

         unsigned int w=(nsize-1)/2;
         threevector currpoint;
         for (unsigned int px=w; px<ztwoDarray_ptr->get_mdim()-w; px++)
         {
            for (unsigned int py=w; py<ztwoDarray_ptr->get_ndim()-w; py++)
            {
               ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
               double central_z=ztwoDarray_ptr->get(px,py);
               if (central_z==null_value)
               {
                  if (bbox.point_inside_polygon(currpoint))
                  {
                     int n=0;
                     for (unsigned int i=0; i<nsize; i++)
                     {
                        for (unsigned int j=0; j<nsize; j++)
                        {
                           double currz=ztwoDarray_ptr->get(px-w+i,py-w+j);
                           if (currz != null_value) intensity[n++]=currz;
                        }
                     } 

                     double median_value;
                     if (n==0)
                     {
                        median_value=null_value;
                     }
                     else
                     {
                        Quicksort(intensity,n);
                        median_value=intensity[n/2];
                     }
                     ztwoDarray_filtered_ptr->put(px,py,median_value);
                  } // point inside convex polygon conditional

               } // cental_z==null_value conditional
            } // loop over index py
         } // loop over index px
      }
   
   void median_fill(
      unsigned int nsize,twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double null_value)
      {
//         cout << "inside imagefunc::median_fill() #2" << endl;
//         cout << "null_value = " << null_value << endl;

         if (is_even(nsize)) nsize++;
         double intensity[nsize*nsize];

         unsigned int w=(nsize-1)/2;
         for (unsigned int px=w; px<ztwoDarray_ptr->get_mdim()-w; px++)
         {
            for (unsigned int py=w; py<ztwoDarray_ptr->get_ndim()-w; py++)
            {
               double central_z=ztwoDarray_ptr->get(px,py);
               if (nearly_equal(central_z,null_value))
               {
                  unsigned int n=0;
                  for (unsigned int i=0; i<nsize; i++)
                  {
                     for (unsigned int j=0; j<nsize; j++)
                     {
                        double currz=ztwoDarray_ptr->get(px-w+i,py-w+j);
                        if (!nearly_equal(currz,null_value)) 
                        {
                           intensity[n++]=currz;
                        }
                     }
                  } 

                  double median_value;
                  if (n==0)
                  {
                     median_value=null_value;
                  }
                  else
                  {
                     Quicksort(intensity,n);
                     median_value=intensity[n/2];
                  }

                  ztwoDarray_filtered_ptr->put(px,py,median_value);

               } // cental_z==null_value conditional
            } // loop over index py
         } // loop over index px
      }

// ---------------------------------------------------------------------   
// Method median_clean() iterates over all non-edge pixels within
// input *ztwoDarray_ptr.  Within an nsize x nsize neighborhood around
// each pixel, it computes the median value and quartile width for all
// Z values other than the center pixel's.  If the center pixel's Z
// deviates too much from the neighbor's median values (as measured in
// units of quartile width), this method resets the center pixel's Z
// value to the median.  This method is intended to eliminate isolated noise
// outliers with minimal data smearing.

   int median_clean(
      unsigned int nsize,twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double null_value,
      double max_deviation_ratio)
   {
      if (is_even(nsize)) nsize++;
      vector<double> intensity;

      unsigned int n_isolated_noise_pixels=0;
      
      unsigned int w=(nsize-1)/2;
      for (unsigned int px=w; px<ztwoDarray_ptr->get_mdim()-w; px++)
      {
         outputfunc::update_progress_fraction(
            px,1000,ztwoDarray_ptr->get_mdim()-w);

         for (unsigned int py=w; py<ztwoDarray_ptr->get_ndim()-w; py++)
         {
            double central_z=ztwoDarray_ptr->get(px,py);
            if (central_z <= null_value) continue;

//            cout << "px = " << px << " py = " << py 
//                 << " z = " << central_z << endl;

// Calculate median value for all neighboring pixels (not including
// current central_z value):

            intensity.clear();
            for (unsigned int i=0; i<nsize; i++)
            {
               for (unsigned int j=0; j<nsize; j++)
               {
                  if (i==w && j==w) continue;
                  
                  double currz=ztwoDarray_ptr->get(px-w+i,py-w+j);
                  if (currz > null_value) intensity.push_back(currz);
               }
            } 

            if (intensity.size()==0)
            {
               ztwoDarray_filtered_ptr->put(px,py,null_value);
            }
            else
            {
               double median_value,quartile_width;
               mathfunc::median_value_and_quartile_width(
                  intensity,median_value,quartile_width);
               double deviation=fabs(central_z-median_value)/quartile_width;
               if (deviation < max_deviation_ratio)
               {
                  ztwoDarray_filtered_ptr->put(px,py,central_z);
               }
               else
               {
                  ztwoDarray_filtered_ptr->put(px,py,median_value);
                  n_isolated_noise_pixels++;
               }
            }

         } // loop over index py
      } // loop over index px
      cout << endl;

      return n_isolated_noise_pixels;
   }

// ==========================================================================
// Numerical image intensity differentiation methods
// ==========================================================================

// Method brute_twoD_convolve takes in an image within *ztwoDarray_ptr
// along with an nsize x nsize filter.  It returns within output
// twoDarray *ztwoDarray_filtered_ptr the convolved values computed by
// sliding the filter across the input image.

   void brute_twoD_convolve(
      genmatrix* filter_ptr,twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr)
      {
         ztwoDarray_ptr->copy(ztwoDarray_filtered_ptr);

         unsigned int nsize=filter_ptr->get_mdim();
         unsigned int w=nsize/2;
         for (unsigned int px=w; px<ztwoDarray_ptr->get_mdim()-w; px++)
         {
            for (unsigned int py=w; py<ztwoDarray_ptr->get_ndim()-w; py++)
            {
               double intensity=0;
               for (unsigned int i=0; i<nsize; i++)
               {
                  for (unsigned int j=0; j<nsize; j++)
                  {
                     intensity += filter_ptr->get(i,j)*ztwoDarray_ptr->
                        get(px-w+i,py-w+j);
                  }
               } 
//               ztwoDarray_filtered_ptr->put(px,py,fabs(intensity));
               ztwoDarray_filtered_ptr->put(px,py,intensity);
            } // loop over index py
         } // loop over index px
      }

// This overloaded version of brute_twoD_convolve performs a brute
// force convolution only at the image location specified by input
// pixel pair (px,py):

   double brute_twoD_convolve(
      unsigned int px,unsigned int py,genmatrix* filter_ptr,
      twoDarray const *ztwoDarray_ptr)
      {
         unsigned int nsize=filter_ptr->get_mdim();
         unsigned int w=nsize/2;

         double convolution=0;
         if (px >= w && px <ztwoDarray_ptr->get_mdim()-w)
         {
            if (py >= w && py<ztwoDarray_ptr->get_ndim()-w)
            {
               for (unsigned int i=0; i<nsize; i++)
               {
                  for (unsigned int j=0; j<nsize; j++)
                  {
                     convolution += filter_ptr->get(i,j)*ztwoDarray_ptr->
                        get(px-w+i,py-w+j);
                  }
               } 
            } // py conditional
         } // px conditional
         return convolution;
      }

// ---------------------------------------------------------------------
// Methods horiz_derivative_filter and vert_derivative_filter take in
// an image within *ztwoDarray_ptr along with a 1D nsize filter.  It
// returns within output twoDarray *ztwoDarray_filtered_ptr the
// convolved values computed by sliding the filter across the input
// image.

   void horiz_derivative_filter(
      unsigned int nsize,const double filter[],twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double dx,double null_value)
      {
         ztwoDarray_filtered_ptr->clear_values();
         unsigned int w=nsize/2;
         for (unsigned int px=w; px<ztwoDarray_ptr->get_mdim()-w; px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               unsigned int n_nulls=0;
               double intensity=0;
               for (unsigned int i=0; i<nsize; i++)
               {
                  double currz=ztwoDarray_ptr->get(px-w+i,py);
                  if (currz <= null_value) 
                  {
                     n_nulls++;
                  }
                  else
                  {
                     intensity += filter[i]*currz;
                  }
               } 
               intensity *= dx;
               if (n_nulls==nsize)
               {
                  cout << " null_value = "
                       << null_value << endl;
                  intensity=null_value;
                  cout << "intensity = null!!" << endl;
                  outputfunc::enter_continue_char();
               }
               
               if (nearly_equal(intensity,0)) intensity=0;
               ztwoDarray_filtered_ptr->put(px,py,intensity);

/*
               if (intensity < 1E-5)
               {
                  cout << "nsize = " << nsize << endl;
                  cout << "px = " << px << " py = " << py
                       << " dx = " << dx 
                       << " intensity = " << intensity << endl;

                  for (unsigned int i=0; i<nsize; i++)
                  {
                     double currz=ztwoDarray_ptr->get(px-w+i,py);
                     cout << "i = " << i 
                          << " filter[i] = " << filter[i]
                          << " px-w+i = " << px-w+i << " py = " << py
                          << " currz = " << currz
                          << endl;
                  }
               }
*/

            } // loop over index py
         } // loop over index px
      }

   void vert_derivative_filter(
      unsigned int nsize,const double filter[],twoDarray const *ztwoDarray_ptr,
      twoDarray *ztwoDarray_filtered_ptr,double dy,double null_value)
      {
         ztwoDarray_filtered_ptr->clear_values();

         unsigned int w=nsize/2;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=w; py<ztwoDarray_ptr->get_ndim()-w; py++)
            {
               unsigned int n_nulls=0;
               double intensity=0;
               for (unsigned int j=0; j<nsize; j++)
               {
                  double currz=ztwoDarray_ptr->get(px,py-w+j);
                  if (currz <= null_value) 
                  {
                     n_nulls++;
                  }
                  else
                  {
                     intensity += filter[j]*currz;
                  }
               } 
               intensity *= dy;
               if (n_nulls==nsize) intensity=-null_value;

               if (nearly_equal(intensity,0)) intensity=0;

//               if (nearly_equal(intensity,0))
//               {
//                  cout << "px = " << px << " py = " << py 
//                       << " vert_deriv = " << intensity << endl;
//               }
               

               ztwoDarray_filtered_ptr->put(px,py,-intensity);
            } // loop over index py
         } // loop over index px
      }

// These next overloaded versions of horiz_derivative_filter and
// vert_derivative_filter perform a brute force convolution only at
// the image location specified by input pixel pair (px,py):

   double horiz_derivative_filter(
      unsigned int px,unsigned int py,unsigned int nsize,const double filter[],
      twoDarray const *ztwoDarray_ptr,double null_value,double dx)
      {
         double convolution=0;
         const unsigned int w=nsize/2;
         if (px >= w && px < ztwoDarray_ptr->get_mdim()-w)
         {
            if (py >= 0 && py < ztwoDarray_ptr->get_ndim())
            {
               unsigned int n_nulls=0;
               for (unsigned int i=0; i<nsize; i++)
               {
                  double currz=ztwoDarray_ptr->get(px-w+i,py);
                  if (currz <= null_value) 
                  {
                     n_nulls++;
                  }
                  else
                  {
                     convolution += filter[i]*currz;
                  }
               } 
               convolution *= dx;
               if (n_nulls==nsize) return null_value;
            } // py conditional
         } // px conditional
         return convolution;
      }

   double vert_derivative_filter(
      unsigned int px,unsigned int py,unsigned int nsize,const double filter[],
      twoDarray const *ztwoDarray_ptr,double null_value,double dy)
      {
         double convolution=0;
         const unsigned int w=nsize/2;
         if (px >= 0 && px < ztwoDarray_ptr->get_mdim())
         {
            if (py >= w && py < ztwoDarray_ptr->get_ndim()-w)
            {
               unsigned int n_nulls=0;
               for (unsigned int j=0; j<nsize; j++)
               {
                  double currz=ztwoDarray_ptr->get(px,py-w+j);
                  if (currz <= null_value) 
                  {
                     n_nulls++;
                  }
                  else
                  {
                     convolution += filter[j]*currz;
                  }
               } 
               convolution *= dy;
               if (n_nulls==nsize) return null_value;
            } // py conditional
         } // px conditional
         return -convolution;
      }

// ---------------------------------------------------------------------
// Method compute_x_y_deriv_fields takes in a twoDarray and first
// median filters its contents.  It next generates 1D gaussian 1st or
// 2nd derivative filters depending upon input parameter deriv_order.
// This method performs a brute force convolution of these smooth
// derivative filters with the median filtered data.  Artificial edge
// effects due to image borders are eliminated.

   void compute_x_y_deriv_fields(
      twoDarray const *ztwoDarray_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      double magnification_factor,int deriv_order)
      {
//         cout << "inside imagefunc::compute_x_y_deriv_fields()" << endl;
         
         double dx,dy,spatial_resolution;
         dx=dy=1;
         spatial_resolution=1;
         unsigned int nx_size=
            filterfunc::gaussian_filter_size(spatial_resolution,dx);
         unsigned int ny_size=nx_size;

         double* xfilter=new double[nx_size];
         filterfunc::gaussian_filter(
            nx_size,deriv_order,spatial_resolution,dx,xfilter);

         for (unsigned int f=0; f<nx_size; f++)
         {
            xfilter[f] *= magnification_factor;
         }

         horiz_derivative_filter(
            nx_size,xfilter,ztwoDarray_ptr,xderiv_twoDarray_ptr,dx);
         delete [] xfilter;

         double* yfilter=new double[ny_size];
         filterfunc::gaussian_filter(
            ny_size,deriv_order,spatial_resolution,dy,yfilter);

         for (unsigned int f=0; f<ny_size; f++)
         {
            yfilter[f] *= magnification_factor;
         }

         vert_derivative_filter(
            ny_size,yfilter,ztwoDarray_ptr,yderiv_twoDarray_ptr,dy);
         delete [] yfilter;
      }

// ---------------------------------------------------------------------
// Method compute_x_y_deriv_fields takes in a twoDarray and first
// median filters its contents.  It next generates 1D gaussian 1st or
// 2nd derivative filters depending upon input parameter deriv_order.
// This method performs a brute force convolution of these smooth
// derivative filters with the median filtered data.  Artificial edge
// effects due to image borders are eliminated.

   void compute_x_y_deriv_fields(
      double spatial_resolution,twoDarray const *ztwoDarray_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      int deriv_order)
      {
//         cout << "inside imagefunc::compute_x_y_deriv_fields()" << endl;
         
         twoDarray* zfiltered_twoDarray_ptr=new twoDarray(*ztwoDarray_ptr);

// As of June 03, Andrew Bradley recommends that we perform only 1
// round of median filtering with a large plaquette rather than
// multiple rounds with a smaller plaquette...

         int nx_size=filterfunc::gaussian_filter_size(
            spatial_resolution,ztwoDarray_ptr->get_deltax());
//         cout << "nx_size = " << nx_size << endl;
         int ny_size=filterfunc::gaussian_filter_size(
            spatial_resolution,ztwoDarray_ptr->get_deltay());
//         cout << "ny_size = " << ny_size << endl;
         if (nx_size <= 2 || ny_size <= 2)
         {
            cout << "Trouble in imagefunc::compute_x_y_deriv_fields()!"
                 << endl;
            cout << "nx_size = " << nx_size << " ny_size = " << ny_size 
                 << endl;
            outputfunc::enter_continue_char();
         }
         
         imagefunc::median_filter(nx_size,ny_size,zfiltered_twoDarray_ptr);

         double *xfilter=new double[nx_size];
         filterfunc::gaussian_filter(
            nx_size,deriv_order,spatial_resolution,
            ztwoDarray_ptr->get_deltax(),xfilter);
         horiz_derivative_filter(
            nx_size,xfilter,zfiltered_twoDarray_ptr,xderiv_twoDarray_ptr,
            ztwoDarray_ptr->get_deltax());
         delete [] xfilter;

         double *yfilter=new double[ny_size];
         filterfunc::gaussian_filter(
            ny_size,deriv_order,spatial_resolution,
            ztwoDarray_ptr->get_deltay(),yfilter);
         vert_derivative_filter(
            ny_size,yfilter,zfiltered_twoDarray_ptr,yderiv_twoDarray_ptr,
            ztwoDarray_ptr->get_deltay());
         delete [] yfilter;
         delete zfiltered_twoDarray_ptr;

// Try to eliminate artificial image boundary effects from gradient
// arrays:
         
         double xlo=ztwoDarray_ptr->get_xlo();
         double xhi=ztwoDarray_ptr->get_xhi();
         double ylo=ztwoDarray_ptr->get_ylo();
         double yhi=ztwoDarray_ptr->get_yhi();
         const double min_distance_to_border=0.5;	// meter
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double x,y;
               if (ztwoDarray_ptr->pixel_to_point(px,py,x,y))
               {
                  if (fabs(x-xlo) < min_distance_to_border ||
                      fabs(x-xhi) < min_distance_to_border ||
                      fabs(y-ylo) < min_distance_to_border ||
                      fabs(y-yhi) < min_distance_to_border)
                  {
                     xderiv_twoDarray_ptr->put(px,py,0);
                     yderiv_twoDarray_ptr->put(px,py,0);
                  }
               }
            } // loop over index py
         } // loop over index px
      }

// ---------------------------------------------------------------------
// Method compute_sobel_gradients() takes in a twoDarray and 
// calculates first order x and y derivatives via convolution with
// a 2D Sobel edge filter.  On 9/6/13, we empirically confirmed that
// the output xderiv_twoDarray_ptr and yderiv_twoDarray_ptr results
// are qualitiatively similar to those generated via member function
// compute_x_y_deriv_fields() #1.

   void compute_sobel_gradients(
      twoDarray const *ztwoDarray_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      twoDarray* gradient_mag_twoDarray_ptr)
      {
//         cout << "inside imagefunc::compute_sobel_gradients()" << endl;
         
         xderiv_twoDarray_ptr->clear_values();
         yderiv_twoDarray_ptr->clear_values();
         gradient_mag_twoDarray_ptr->clear_values();

	 for (unsigned int py=1; py<ztwoDarray_ptr->get_ndim()-1; py++)
	 {
	   for (unsigned int px=1; px<ztwoDarray_ptr->get_mdim()-1; px++)
	   {
               double z1=ztwoDarray_ptr->get(px-1,py-1);
               double z2=ztwoDarray_ptr->get(px,py-1);
               double z3=ztwoDarray_ptr->get(px+1,py-1);
               double z4=ztwoDarray_ptr->get(px+1,py);
               double z5=ztwoDarray_ptr->get(px+1,py+1);
               double z6=ztwoDarray_ptr->get(px,py+1);
               double z7=ztwoDarray_ptr->get(px-1,py+1);
               double z8=ztwoDarray_ptr->get(px-1,py);

               double gx=z3+2*z4+z5-z1-2*z8-z7;               
               double gy=z7+2*z6+z5-z1-2*z2-z3;
	       double grad_mag = sqrt(gx*gx+gy*gy);

               xderiv_twoDarray_ptr->put(px,py,gx);
               yderiv_twoDarray_ptr->put(px,py,-gy);
	       gradient_mag_twoDarray_ptr->put(px,py,grad_mag);

            } // loop over index py
         } // loop over index px
      }

// ---------------------------------------------------------------------
// Method compute_gradient_magnitude_field takes in x and y partial
// derivative information and returns the gradient magnitude field.

   void compute_gradient_magnitude_field(
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,
      twoDarray* gradient_mag_twoDarray_ptr)
      {
//          outputfunc::write_banner("Computing gradient magnitude field:");
         compositefunc::combine_identically_sized_twoDarrays_in_quadrature(
            xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
            gradient_mag_twoDarray_ptr);
      }
   
// ---------------------------------------------------------------------
// Method laplacian_field returns the laplacian of input image
// *ztwoDarray_ptr.

   void laplacian_field(
      double spatial_resolution,twoDarray const *ztwoDarray_ptr,
      twoDarray *zlaplacian_twoDarray_ptr)
      {
         twoDarray* x2deriv_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         twoDarray* y2deriv_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         compute_x_y_deriv_fields(
            spatial_resolution,ztwoDarray_ptr,
            x2deriv_twoDarray_ptr,y2deriv_twoDarray_ptr,2);
         
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double term1=x2deriv_twoDarray_ptr->get(px,py);
               double term2=y2deriv_twoDarray_ptr->get(px,py);
               zlaplacian_twoDarray_ptr->put(px,py,fabs(term1+term2));
//               zlaplacian_twoDarray_ptr->put(px,py,term1+term2);
            }
         }
         delete x2deriv_twoDarray_ptr;
         delete y2deriv_twoDarray_ptr;
      }
   
// ---------------------------------------------------------------------
// Method remove_horiz_vert_gradients attempts to eliminate horizontal
// and vertical streaks which are more likely to be radar image
// generation artifacts than genuine signal.  It takes in some small
// critical angle measured in degrees.  This method subsequently
// computes gradient two-vector angles wrt horizontal and vertical
// axes.  If a gradient's direction vector lies within the critical
// angle of either axis, both the x and y derivative values are set to
// zero.

   void remove_horiz_vert_gradients(
      double crit_angle,twoDarray* xderiv_twoDarray_ptr,
      twoDarray* yderiv_twoDarray_ptr)
      {
         remove_horiz_vert_gradients(
            crit_angle,
            xderiv_twoDarray_ptr->get_xlo(),xderiv_twoDarray_ptr->get_ylo(),
            xderiv_twoDarray_ptr->get_xhi(),xderiv_twoDarray_ptr->get_yhi(),
            xderiv_twoDarray_ptr,yderiv_twoDarray_ptr);
      }
   
   void remove_horiz_vert_gradients(
      double crit_angle,double minimum_x,double minimum_y,
      double maximum_x,double maximum_y,twoDarray* xderiv_twoDarray_ptr,
      twoDarray* yderiv_twoDarray_ptr)
      {
         unsigned int px_lo,px_hi,py_lo,py_hi;
         xderiv_twoDarray_ptr->point_to_pixel(
            minimum_x,minimum_y,px_lo,py_hi);
         yderiv_twoDarray_ptr->point_to_pixel(
            maximum_x,maximum_y,px_hi,py_lo);

         for (unsigned int px=px_lo; px<px_hi; px++)
         {
            for (unsigned int py=py_lo; py<py_hi; py++)
            {
               double curr_theta=atan2(
                  yderiv_twoDarray_ptr->get(px,py),
                  xderiv_twoDarray_ptr->get(px,py))*180.0/PI;
               curr_theta=basic_math::phase_to_canonical_interval(
                  curr_theta,-180,180);
               if ((fabs(curr_theta-0) < crit_angle) ||
                   (fabs(curr_theta-180) < crit_angle) ||
                   (fabs(curr_theta+180) < crit_angle) ||
                   (fabs(curr_theta-90) < crit_angle) ||
                   (fabs(curr_theta+90) < crit_angle))
               {
                  xderiv_twoDarray_ptr->put(px,py,0);
                  yderiv_twoDarray_ptr->put(px,py,0);
               }
            } // loop over index py
         } // loop over index px
      }

// ---------------------------------------------------------------------
// Method draw_gradient_dir_field takes in horizontal and vertical
// partial derivative fields.  It draws white unitvectors pointing in
// the direction of the gradient at all points within
// *gradient_dir_twoDarray_ptr where the gradient's magnitude exceeds
// zthreshold:

   void draw_gradient_dir_field(
      double zthreshold,twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,
      twoDarray const *gradient_mag_twoDarray_ptr,
      twoDarray* gradient_dir_twoDarray_ptr)
      {
         for (unsigned int px=0; px<gradient_mag_twoDarray_ptr->get_mdim(); 
              px++)
         {
            for (unsigned int py=0; py<gradient_mag_twoDarray_ptr->get_ndim();
                 py++)
            {
               if (gradient_mag_twoDarray_ptr->get(px,py) > zthreshold)
               {
                  threevector gradient(xderiv_twoDarray_ptr->get(px,py),
                                       yderiv_twoDarray_ptr->get(px,py),0);
                  threevector curr_posn;
                  gradient_mag_twoDarray_ptr->pixel_to_point(px,py,curr_posn);
                  threevector unitvector(25*gradient.unitvector());

//                   if (nrfunc::ran1() > 0.9)
                  {
                     drawfunc::draw_vector(
                        unitvector,curr_posn,colorfunc::white,
                        gradient_dir_twoDarray_ptr);
                  }
                  
               }
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method compute_gradient_steps() takes in horizontal and vertical
// partial derivative fields.  It computes the gradient at each pixel
// location and ignores any whose magnitude lies below
// grad_mag_threshold. This method then computes a "stepped" pixel
// location which resides step_distance away from the gradient's base
// position along its direction vector.  It returns an STL map
// containing pixel_ID vs stepped_pixel_ID.  Any stepped pixel which
// lies outside [0,mdim-1] x [0,ndim-1] has stepped_pixel_ID=-1.

   map<int,int> compute_gradient_steps(
      double grad_mag_threshold,double step_distance,
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,
      twoDarray const *gradient_mag_twoDarray_ptr)
      {
//         cout << "inside imagefunc::compute_gradient_steps()" << endl;
//         cout << "step_distance = " << step_distance << endl;

         map<int,int> step_map;

         unsigned int mdim=gradient_mag_twoDarray_ptr->get_mdim();
         unsigned int ndim=gradient_mag_twoDarray_ptr->get_ndim();
         for (unsigned int px=0; px<mdim; px++)
         {
            for (unsigned int py=0; py<ndim; py++)
            {

               if (gradient_mag_twoDarray_ptr->get(px,py) > grad_mag_threshold)
               {
                  int pixel_ID=graphicsfunc::get_pixel_ID(px,py,mdim);
                  twovector gradient(xderiv_twoDarray_ptr->get(px,py),
		                     yderiv_twoDarray_ptr->get(px,py));
                  twovector g_hat(gradient.unitvector());
                  unsigned int qx=px+step_distance*g_hat.get(0);
                  unsigned int qy=py+step_distance*g_hat.get(1);
                  
                  int stepped_pixel_ID=-1;
                  if (qx >= 0 && qx < mdim && qy >= 0 && qy < ndim)
                  {
                     stepped_pixel_ID=graphicsfunc::get_pixel_ID(qx,qy,mdim);
//                     double delta_pixel=sqrt(sqr(qx-px)+sqr(qy-py));
//                     cout << "qx = " << qx << " px = " << px
//                          << " qy = " << qy << " py = " << py << endl;
//                     cout << "delta_pixel = " << delta_pixel << endl;
                  }
                  step_map[pixel_ID]=stepped_pixel_ID;
               }
            } // loop over py index
         } // loop over px index
         return step_map;
      }

// ---------------------------------------------------------------------
// Method compute_gradient_phase_field takes in horizontal and
// vertical partial derivative fields.  For each point in the image
// where the magnitude of the gradient exceeds zthreshold, this method
// computes the phase angle defined by the arctangent of the y to x
// partial derivatives.  The results are returned within twoDarray
// *gradient_phase_twoDarray_ptr.

   void compute_gradient_phase_field(
      double zmag_min_threshold,double zmag_max_threshold,
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr,
      twoDarray const *gradient_mag_twoDarray_ptr,
      twoDarray* gradient_phase_twoDarray_ptr,double null_value)
      {
         for (unsigned int py=0; py<gradient_mag_twoDarray_ptr->get_ndim(); 
              py++)
         {
            for (unsigned int px=0; px<gradient_mag_twoDarray_ptr->get_mdim();
                 px++)
            {
               double curr_grad_mag=gradient_mag_twoDarray_ptr->get(px,py);
               if (curr_grad_mag > zmag_min_threshold && curr_grad_mag < 
                   zmag_max_threshold)
               {
                  double curr_phase=atan2(yderiv_twoDarray_ptr->get(px,py),
                                          xderiv_twoDarray_ptr->get(px,py));
//                  cout << "px = " << px << " py = " << py
//                       << " yderiv = " << yderiv_twoDarray_ptr->get(px,py)
//                       << " xderiv = " << xderiv_twoDarray_ptr->get(px,py)
//                       << " phase = " << curr_phase*180/PI << endl;

                  gradient_phase_twoDarray_ptr->put(px,py,curr_phase);
               }
               else
               {
                  gradient_phase_twoDarray_ptr->put(px,py,0);
//                  gradient_phase_twoDarray_ptr->put(px,py,null_value);
               }
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method compute_local_pixel_intensity_variation takes in twoDarrays
// *ztwoDarray_ptr and *zbinary_twoDarray_ptr containing the image and
// binary image containing the connected component.  For each non-null
// pixel within the binary image, this method computes the sum of the
// absolute differences between the pixel's intensity value and those
// of its nearest non-null neighbors.  The average of all these
// absolute differences is returned within output parameter
// avg_global_delta_f.
      
   void compute_local_pixel_intensity_variation(
      int filter_pixel_size,const twoDarray* ztwoDarray_ptr,
      const twoDarray* zbinary_twoDarray_ptr,
      twoDarray* pseudo_grad_twoDarray_ptr,
      bool convert_phase_to_canonical_intervals)
      {
         double delta_f[filter_pixel_size*filter_pixel_size];
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (zbinary_twoDarray_ptr->get(px,py) > 0.5)
               {
                  double curr_intensity=ztwoDarray_ptr->get(px,py);
                  int n_neighbors=0;
                  for (int i=-filter_pixel_size/2; i<=filter_pixel_size/2; 
                       i++)
                  {
                     for (int j=-filter_pixel_size/2; j<=filter_pixel_size/2; 
                          j++)
                     {
                        if (ztwoDarray_ptr->pixel_inside_working_region(
                           px+i,py+j) 
                            && zbinary_twoDarray_ptr->get(px+i,py+j) > 0.5)
                        {
                           double currz=ztwoDarray_ptr->get(px+i,py+j);

// If intensities within input twoDarray *ztwoDarray_ptr correspond to
// angular phases, we need to avoid artificially enlarging delta_f
// values due to 2*PI discontinuities.  So we first force the currz
// value to lie within the 2*PI interval centered about the
// curr_intensity phase value:

                           if (convert_phase_to_canonical_intervals)
                           {
                              currz=basic_math::phase_to_canonical_interval(
                                 currz,curr_intensity-PI,curr_intensity+PI);
                           }
                           delta_f[n_neighbors]=fabs(curr_intensity-currz);
                           n_neighbors++;
                        }
                     } // loop over index j
                  } // loop over index i

                  prob_distribution prob(n_neighbors,delta_f,30);
                  double delta_f_75=prob.find_x_corresponding_to_pcum(0.75);
//                  double delta_f_80=prob.find_x_corresponding_to_pcum(0.8);
//                  double delta_f_90=prob.find_x_corresponding_to_pcum(0.9);
                  pseudo_grad_twoDarray_ptr->put(px,py,delta_f_75);
//                  pseudo_grad_twoDarray_ptr->put(px,py,delta_f_80);
//                  pseudo_grad_twoDarray_ptr->put(px,py,delta_f_90);
//                  double max_delta_f=max_array_value(n_neighbors,delta_f);
//                  pseudo_grad_twoDarray_ptr->put(px,py,max_delta_f);

               } // zbinary > 0.5 conditional
            } // py loop
         } // px loop
      }

// ---------------------------------------------------------------------
// Method edge_gradient takes in a line segment.  It first rotates the
// line segment's direction vector through the specified angle theta
// in order to define "black" and "white" directions.  It next divides
// up the candidate line segment into bins.  From the center of each
// bin out to the distance set by the constant "correlation_length"
// parameter, method edge_gradient computes intensity line integrals B
// and W in the "black" and "white" directions.  The difference
// delta=W-B is then integrated up along the candidate line integral
// and returned by this routine.

// Edge_gradient attempts to compute a quantity which is essentially
// proportional to an averaged intensity gradient along a line set at
// some user specified angle relative to a candidate edge.

   double edge_gradient(
      double correlation_length,const linesegment& curr_edge,double theta,
      twoDarray const *ztwoDarray_ptr)
      {
         const int max_nsteps=5000;

         double costheta=cos(theta);
         double sintheta=sin(theta);
         threevector b_hat(
            costheta*curr_edge.get_ehat().get(0)-
            sintheta*curr_edge.get_ehat().get(1),
            sintheta*curr_edge.get_ehat().get(0)+
            costheta*curr_edge.get_ehat().get(1));
         threevector w_hat(-b_hat);

// Next line for debugging and viewgraph generation purposes only...

//   drawfunc::draw_vector(
//      correlation_length*b_hat,curr_edge.midpoint(),colorfunc::white,
// 	ztwoDarray_ptr);

         double ds=basic_math::min(ztwoDarray_ptr->get_deltax(),
                       ztwoDarray_ptr->get_deltay());
         double true_nsteps=curr_edge.get_length()/ds;
         unsigned int nsteps=basic_math::mytruncate(true_nsteps);
         double frac_step=true_nsteps-nsteps;

         double delta[max_nsteps];
         for (unsigned int i=0; i<=nsteps; i++)
         {
            threevector s_vec(curr_edge.get_v1()+i*ds*curr_edge.get_ehat());
            linesegment black_segment(s_vec,s_vec+correlation_length*b_hat);
            linesegment white_segment(s_vec,s_vec+correlation_length*w_hat);
            double B=imagefunc::fast_line_integral_along_segment(
               ztwoDarray_ptr,black_segment);
            double W=imagefunc::fast_line_integral_along_segment(
               ztwoDarray_ptr,white_segment);
            delta[i]=(W-B);
         }
 
// True arc length between points v1 and v2 is generically not an
// integer number of steps.  So we set the line integral between v1
// and v2 equal to a weighted average of the Simpson sums with nsteps
// bins plus a fractional remainder of the integral within the nsteps+1st
// bin:

         double line_integral=mathfunc::simpsonsum(delta,0,nsteps)*ds;
         line_integral += frac_step*delta[nsteps]*ds;

         return line_integral;
      }

// ==========================================================================
// Polar image methods
// ==========================================================================

// Method convert_intensities_to_polar_coords takes in an intensity
// pattern laid out on a Cartesian grid within input twoDarray
// *ztwoDarray_ptr.  This method dynamically generates a new twoDarray
// whose horizontal and vertical axes respectively correspond to
// radial and polar angle coordinates.  It then scans through every
// pixel in the Cartesian grid and determines its corresponding
// location in the polar coordinate system.  This method returns the
// polar version of the input twoDarray.

   twoDarray* convert_intensities_to_polar_coords(
      twoDarray const *ztwoDarray_ptr)
      {
//         double xlo=ztwoDarray_ptr->get_xlo();
//         double xhi=ztwoDarray_ptr->get_xhi();
         double deltax=ztwoDarray_ptr->get_deltax();
//         double ylo=ztwoDarray_ptr->get_ylo();
//         double yhi=ztwoDarray_ptr->get_yhi();
         double deltay=ztwoDarray_ptr->get_deltay();
         double deltar=basic_math::min(deltax,deltay);
//         double rhi=basic_math::max(
//            sqrt(sqr(xlo)+sqr(ylo)),sqrt(sqr(xlo)+sqr(yhi)),
//            sqrt(sqr(xhi)+sqr(ylo)),sqrt(sqr(xhi)+sqr(yhi)));
         double rlo=0;
         double rhi=20;
         double thetahi=180;
         double thetalo=-180;
         int m=basic_math::round(rhi/deltar);
         int n=360;
         twoDarray* ztwoDarray_polar_ptr=new twoDarray(m,n);
         ztwoDarray_polar_ptr->clear_values();
         
         ztwoDarray_polar_ptr->set_xlo(rlo);	
         ztwoDarray_polar_ptr->set_xhi(rhi);	
         ztwoDarray_polar_ptr->set_ylo(thetalo);	
         ztwoDarray_polar_ptr->set_yhi(thetahi);	
         ztwoDarray_polar_ptr->set_deltax((rhi-rlo)/double(m-1));
         ztwoDarray_polar_ptr->set_deltay((thetahi-thetalo)/double(n-1));

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double x,y;
               ztwoDarray_ptr->pixel_to_point(px,py,x,y);

//               cout << "px = " << px << " py = " << py << endl;
//               cout << "x = " << x << " y = " << y << endl;
               
               double r=sqrt(sqr(x)+sqr(y));
               double theta=atan2(y,x)*180/PI;
               unsigned int pr,ptheta;
               if (ztwoDarray_polar_ptr->point_to_pixel(r,theta,pr,ptheta))
               {
//               cout << "r = " << r << " theta = " << theta << endl;
//               cout << "pr = " << pr << " ptheta = " << ptheta << endl;
                  ztwoDarray_polar_ptr->increment(
                     pr,ptheta,ztwoDarray_ptr->get(px,py));
               }
               
            } // loop over py
         } // loop over px
         return ztwoDarray_polar_ptr;
      }

// ---------------------------------------------------------------------
// Method generate_polar_image takes in twoDarray
// *ztwoDarray_polar_ptr containing intensity information in polar
// coordinates and returns a corresponding dynamically generated
// myimage object.

   myimage* generate_polar_image(
      string imagedir,string colortablefilename,
      twoDarray *ztwoDarray_polar_ptr)
      {
         myimage* polarimage_ptr=new myimage(*ztwoDarray_polar_ptr);   
         polarimage_ptr->set_imagedir(imagedir);
         polarimage_ptr->set_colortable_filename(colortablefilename);
         polarimage_ptr->set_classified(false);
         polarimage_ptr->set_adjust_xscale(false);
         polarimage_ptr->set_xaxis_label("R (meters)");
         polarimage_ptr->set_yaxis_label("Theta (degs)");
         polarimage_ptr->set_xtic(2);	// meter
         polarimage_ptr->set_ytic(20);	// degs
//         polarimage_ptr->writeimage(
//            "polar","Image transformed to polar coordinates",
//            ztwoDarray_polar_ptr);
         return polarimage_ptr;
      }

// ==========================================================================
// Profiling methods
// ==========================================================================

// Method invert_vertical_profile takes in a vertical profile is
// ordered from the lowest pixel (top of screen) to highest pixel
// (bottom of screen).  It inverts the ordering of the profile so that
// lowest [highest] row integral appears first [last] within
// vert_profile.

   void invert_vertical_profile(unsigned int nbins,double vert_profile[])
      {
         double* vert_swap_profile=new double[nbins];
         for (unsigned int n=0; n<nbins; n++)
         {
            vert_swap_profile[n]=vert_profile[nbins-1-n];
         }
         for (unsigned int n=0; n<nbins; n++)
         {
            vert_profile[n]=vert_swap_profile[n];
         }
         delete [] vert_swap_profile;
      }
   
// ---------------------------------------------------------------------
// Method find_horiz_vert_profile_edges scan through horizontal and
// vertical profiles for edges of a bounding box.  It declares these
// edges to be found when profile values exceed
// edge_fraction_of_median*horiz_profile_median and
// edge_fraction_of_median*vert_profile_median.

   void find_horiz_vert_profile_edges(
      double edge_fraction_of_median,
      int jmax,int kmax,int px_min,int px_max,int py_min,int py_max,
      double horiz_profile_median,double vert_profile_median,
      double horiz_profile[],double vert_profile[],
      twoDarray const *ztwoDarray_ptr,double& minimum_x,double& maximum_x,
      double& minimum_y,double& maximum_y)
      {
         cout << "inside imagefunc::find_horiz_vert_profile_edges()" << endl;
         
         int k=kmax/2;
         bool edge_found=false;
         do
         {
            cout << "k = " << k << " horiz_profile = " << horiz_profile[k]
                 << endl;
            
            if (horiz_profile[k] <
                edge_fraction_of_median*horiz_profile_median)
            {
               edge_found=true;
               int p_edge=px_min+k;
               ztwoDarray_ptr->px_to_x(p_edge,minimum_x);
            }
            k--;
         }
         while (!edge_found && k>0);

         k=kmax/2;
         edge_found=false;
         do
         {
            cout << "k = " << k << " horiz_profile = " << horiz_profile[k]
                 << endl;

            if (horiz_profile[k] <
                edge_fraction_of_median*horiz_profile_median)
            {
               edge_found=true;
               int p_edge=px_min+k;
               ztwoDarray_ptr->px_to_x(p_edge,maximum_x);
            }
            k++;
         }
         while (!edge_found && k<kmax);

         int j=jmax/2;
         edge_found=false;
         do
         {
            if (vert_profile[j] < edge_fraction_of_median*
                vert_profile_median)
            {
               edge_found=true;
               int p_edge=py_min+j;
               ztwoDarray_ptr->py_to_y(p_edge,maximum_y);
            }
            j--;
         }
         while (!edge_found && j>0);

         j=jmax/2;
         edge_found=false;
         do
         {
            if (vert_profile[j] < edge_fraction_of_median*
                vert_profile_median)
            {
               edge_found=true;
               int p_edge=py_min+j;
               ztwoDarray_ptr->py_to_y(p_edge,minimum_y);
            }
            j++;
         }
         while (!edge_found && j<jmax);
      }

// ---------------------------------------------------------------------
// Method prepare_profile_twoDarray takes in an array of doubles along
// with some metric information.  It converts these inputs into a
// twoDarray which can be sent directly into
// imagefunc::plot_profile().

   twoDarray* prepare_profile_twoDarray(
      unsigned int nbins,double xmin,double xmax,const double profile_array[])
      {
         twoDarray* profile_twoDarray_ptr=new twoDarray(1,nbins);
         profile_twoDarray_ptr->set_xhi(xmax);
         profile_twoDarray_ptr->set_xlo(xmin);
         profile_twoDarray_ptr->set_deltax((xmax-xmin)/(nbins-1));
         for (unsigned int n=0; n<nbins; n++)
         {
            profile_twoDarray_ptr->put(0,n,profile_array[n]);
         }
         return profile_twoDarray_ptr;
      }

// This overloaded version of prepare_profile_twoDarray takes in a
// pointer to an already existing twoDarray along with a single array
// of data.  It fills up the curr_row row of the multi-rowed twoDarray
// with this input data.  

   void prepare_profile_twoDarray(
      int curr_row,unsigned int nbins,double xmin,double xmax,
      const double profile_row[],twoDarray* profile_twoDarray_ptr)
      {
         profile_twoDarray_ptr->set_xhi(xmax);
         profile_twoDarray_ptr->set_xlo(xmin);
         profile_twoDarray_ptr->set_deltax((xmax-xmin)/(nbins-1));
         for (unsigned int n=0; n<nbins; n++)
         {
            profile_twoDarray_ptr->put(curr_row,n,profile_row[n]);
         }
      }

// ---------------------------------------------------------------------
// Method prepare_profile_dataarray takes in a twoDarray along with
// some metafile information.  It converts these inputs into a
// datarray which can then be output as in metafile format by calling
// plot_profile() below.  These methods are useful for displaying
// horizontal and vertical profiles of images.

   dataarray* prepare_profile_dataarray(
      twoDarray const *profile_twoDarray_ptr,
      string title,string xlabel,string ylabel,string imagedir,double xtic)
      {
         const double SMALL=0.01;
         dataarray* profile_dataarray_ptr=new dataarray(
            profile_twoDarray_ptr->get_xlo(),
            profile_twoDarray_ptr->get_deltax(),*profile_twoDarray_ptr);
         profile_dataarray_ptr->title=title;
         profile_dataarray_ptr->xlabel=xlabel;
         profile_dataarray_ptr->ylabel=ylabel;
         profile_dataarray_ptr->datafilenamestr=imagedir+title;
         profile_dataarray_ptr->xmin=profile_twoDarray_ptr->get_xlo();
         profile_dataarray_ptr->xmax=profile_twoDarray_ptr->get_xhi();
         profile_dataarray_ptr->xtic=xtic;
         profile_dataarray_ptr->xsubtic=0.5*xtic;
         profile_dataarray_ptr->yplotminval=-SMALL;
         profile_dataarray_ptr->narrays=profile_twoDarray_ptr->get_mdim();
         profile_dataarray_ptr->npoints=profile_twoDarray_ptr->get_ndim();
         return profile_dataarray_ptr;
      }
   
   void plot_profile(dataarray *profile_dataarray_ptr)
      {
         profile_dataarray_ptr->opendatafile();
         profile_dataarray_ptr->writedataarray();
         filefunc::meta_to_jpeg(profile_dataarray_ptr->datafilenamestr);
         profile_dataarray_ptr->closedatafile();
         filefunc::gzip_file(profile_dataarray_ptr->datafilenamestr+".meta");
      }

   void plot_profile(unsigned int nextralines,string extraline[],
                     dataarray *profile_dataarray_ptr)
      {
         for (unsigned int n=0; n<nextralines; n++)
         {
            profile_dataarray_ptr->extraline[n]=extraline[n];
         }
         profile_dataarray_ptr->opendatafile();
         profile_dataarray_ptr->writedataarray();
         filefunc::meta_to_jpeg(profile_dataarray_ptr->datafilenamestr);
         profile_dataarray_ptr->closedatafile();
         filefunc::gzip_file(profile_dataarray_ptr->datafilenamestr+".meta");
      }

// ==========================================================================
// Erosion/dilation methods
// ==========================================================================

// Method erode takes in an image within *ztwoDarray_ptr along with
// two empty image buffers *zbuf_twoDarray_ptr and
// *zerode_twoDarray_ptr.  It scans over all pixels within the range
// defined by min_px <= px < max_px and min_py <= py < max_py with a
// nsize x nsize window.  (nsize is forced to be odd).  If the pixel's
// value at the center of this window within *ztwoDarray_ptr equals
// znull, the values for the neighbors within the window are also set
// equal to znull.  This erosion process takes place niters times.

   void erode(unsigned int niters,unsigned int nsize,double znull,
              twoDarray const *ztwoDarray_ptr,
              twoDarray* zbuf_twoDarray_ptr,twoDarray* zerode_twoDarray_ptr)
      {
         erode(niters,nsize,znull,
               0,0,ztwoDarray_ptr->get_mdim(),ztwoDarray_ptr->get_ndim(),
               ztwoDarray_ptr,zbuf_twoDarray_ptr,zerode_twoDarray_ptr);
      }

// Note added on Weds, Aug 11, 2004: This next algorithm looks
// extremely inefficient.  It essentially performs a brute force
// convolution of a "negative" mask with a positive function...
   
   void erode(unsigned int niters,unsigned int nsize,double znull,
              unsigned int min_px,unsigned int min_py,
              unsigned int max_px,unsigned int max_py,
              twoDarray const *ztwoDarray_ptr,twoDarray* zbuf_twoDarray_ptr,
              twoDarray* zerode_twoDarray_ptr)
      {
         if (is_even(nsize)) nsize++;

// First copy contents of *ztwoDarray_ptr into buffer *zbuf_twoDarray_ptr:

         ztwoDarray_ptr->copy(zbuf_twoDarray_ptr);

         for (unsigned int n=0; n<niters; n++)
         {
            cout << "n = " << n << " out of niters = " << niters << endl;
            zbuf_twoDarray_ptr->copy(zerode_twoDarray_ptr);
            for (unsigned int px=basic_math::max(Unsigned_Zero,min_px); 
                 px<basic_math::min(
                    zbuf_twoDarray_ptr->get_mdim(),max_px); px++)
            {
               for (unsigned int py=basic_math::max(Unsigned_Zero,min_py); 
                    py<basic_math::min(
                       zbuf_twoDarray_ptr->get_ndim(),max_py); py++)
               {
                  double currz=zbuf_twoDarray_ptr->get(px,py);
                  if (nearly_equal(currz,znull))
                  {
                     for (int qx=int(px-nsize/2); qx<= int(px+nsize/2); qx++)
                     {
                        for (int qy=int(py-nsize/2); qy<= int(py+nsize/2); 
                             qy++)
                        {
                           if (zbuf_twoDarray_ptr->
                               pixel_inside_working_region(qx,qy))
                           {
                              zerode_twoDarray_ptr->put(qx,qy,znull);
                           }
                        } // loop over qy index
                     } // loop over qx index
                  } // currz==znull conditional
               } // loop over py index
            } // loop over px index
            zerode_twoDarray_ptr->copy(zbuf_twoDarray_ptr);
         } // loop over index n labeling iteration number
      }

// ==========================================================================
// Histogram equalization methods
// ==========================================================================

   void equalize_intensity_histogram(twoDarray* ztwoDarray_ptr)
      {

         const double scale_factor=255;

// Generate final, desired gaussian intensity distribution:

         const int nbins=200;
         double mu=0.5*scale_factor;
         double sigma=0.2*scale_factor;
         prob_distribution p_gaussian=advmath::generate_gaussian_density(
            nbins,mu,sigma);

// Load probabilities greater than input threshold into STL vector,
// and then compute their cumulative distribution:
   
//         double intensity_threshold=-1;
         double intensity_threshold=5;
//         cout << "Enter threshold below which intensities will not be equalized:"
//                      << endl;
//         cin >> intensity_threshold;
   
         vector<double> intensities;
         intensities.reserve(ztwoDarray_ptr->get_dimproduct());
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double curr_intensity=ztwoDarray_ptr->get(px,py);
               if (curr_intensity > intensity_threshold)
                  intensities.push_back(ztwoDarray_ptr->get(px,py));
            }
         }
         prob_distribution prob(intensities,nbins);

// To perform "intensity histogram equalization", we reset each
// normalized intensity value 0 <= x <= 1 to Pcum(x).  To perform
// "intensity histogram specification" onto the desired gaussian
// distribution, we perform an inverse histogram equalization and map
// Pcum(x) onto the y value for which Pcum(x) = Pgaussian(y):

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double old_intensity=ztwoDarray_ptr->get(px,py);
               double new_intensity=old_intensity;
               if (old_intensity > intensity_threshold)
               {
                  int n=prob.get_bin_number(old_intensity);
                  double pcum=prob.get_pcum(n);
//                  new_intensity=scale_factor*pcum;		
					// Histogram equalization
 	          new_intensity=scale_factor*
                     p_gaussian.find_x_corresponding_to_pcum(pcum);

//               cout << "px = " << px << " py = " << py
//                    << " old_intensity = " << old_intensity
//                    << " new_intensity = " << new_intensity
//                    << endl;

               }
               ztwoDarray_ptr->put(px,py,new_intensity);
            }
         }
      }

// ==========================================================================
// ImageMagick text annotation methods
// ==========================================================================

// Method get_gravity_location() returns a randomized gross text
// position within an image if input bool randomize_gravity_flag ==
// true.

   string get_gravity_location(bool randomize_gravity_flag)
   {
      string gravity_location = "Center";
      if(randomize_gravity_flag)
      {
         double g = nrfunc::ran1();
         double gthresh = 0.09;

         if(g < gthresh)
         {
            gravity_location = "NorthWest";
         }
         else if(g < 2*gthresh)
         {
            gravity_location = "North";
         }
         else if(g < 3*gthresh)
         {
            gravity_location = "NorthEast";
         }
         else if(g < 4*gthresh)
         {
            gravity_location = "West";
         }
         else if(g < 5*gthresh)
         {
            gravity_location = "East";
         }
         else if(g < 6*gthresh)
         {
            gravity_location = "SouthWest";
         }
         else if(g < 7*gthresh)
         {
            gravity_location = "South";
         }
         else if(g < 8*gthresh)
         {
            gravity_location = "SouthEast";
         }
      } // randomize_gravity_flag conditional
      return gravity_location;
   }

// ---------------------------------------------------------------------
// Method get_gravity_type() returns a randomized gross text
// position within an image if input bool randomize_gravity_flag ==
// true.

   Magick::GravityType get_gravity_type(bool randomize_gravity_flag)
   {
      Magick::GravityType gravity = Magick::CenterGravity;
      if(randomize_gravity_flag)
      {
         double g = nrfunc::ran1();
         double gthresh = 0.09;

         if(g < gthresh)
         {
            gravity = Magick::NorthWestGravity;
         }
         else if(g < 2*gthresh)
         {
            gravity = Magick::NorthGravity;
         }
         else if(g < 3*gthresh)
         {
            gravity = Magick::NorthEastGravity;
         }
         else if(g < 4*gthresh)
         {
            gravity = Magick::WestGravity;
         }
         else if(g < 5*gthresh)
         {
            gravity = Magick::EastGravity;
         }
         else if(g < 6*gthresh)
         {
            gravity = Magick::SouthWestGravity;
         }
         else if(g < 7*gthresh)
         {
            gravity = Magick::SouthGravity;
         }
         else if(g < 8*gthresh)
         {
            gravity = Magick::SouthEastGravity;
         }
      } // randomize_gravity_flag conditional
      return gravity;
   }

// ---------------------------------------------------------------------
// Method add_text_to_image() overlays a caption onto the specified
// input image and exports the annotated image to the specified output
// image filename.  The caption's gross location is specified by input
// parameters eastwest_location ("east", "west", "center") and
// northsouth_location ("north","south","center").  This method calls
// ImageMagick to perform the text annotation and is more general than
// our older, deprecated pngfunc::convert_textlines_to_PNG.

   void add_text_to_image(
      string text_color,string caption,
      string eastwest_location,string northsouth_location,
      string input_image_filename,string annotated_image_filename)
      {
         unsigned int width,height;
         get_image_width_height(
            input_image_filename,width,height);

         string unix_cmd="convert -background '#0000' ";
         unix_cmd += "-fill "+text_color+" -undercolor '#FFFFFF80' ";
         unix_cmd += "-gravity "+eastwest_location+" ";
         unix_cmd += "-size "+stringfunc::number_to_string(width)+"x";
         unix_cmd += stringfunc::number_to_string(height/10)+" ";
         unix_cmd += "caption:\""+caption+"\" ";
         unix_cmd += input_image_filename+
            " +swap -gravity "+northsouth_location+" -composite "+
            annotated_image_filename;
//         cout << "unix_cmd = " << unix_cmd << endl;
         sysfunc::unix_command(unix_cmd);
      }
  
// ---------------------------------------------------------------------
// Method generate_text_via_ImageMagick() calls ImageMagick in order
// to create an image of the input text label.  It avoids our older,
// deprecated pngfunc::convert_textlines_to_PNG().
 
   void generate_text_via_ImageMagick(
      colorfunc::RGB& foreground_RGB, colorfunc::RGBA& background_RGBA,
      string font_path,int point_size,int img_width,int img_height,
      string label,string output_image_filename)
   {
      bool debug_annotate_flag = false;
      bool randomize_gravity_flag = false;
      colorfunc::RGB stroke_RGB(0,0,0);
      double strokewidth = -1;
      bool underbox_flag = false;
      colorfunc::RGB undercolor_RGB;
      bool drop_shadow_flag = false;

      generate_text_via_ImageMagick(
         debug_annotate_flag, randomize_gravity_flag, 
         foreground_RGB, stroke_RGB, background_RGBA,
         underbox_flag, undercolor_RGB, strokewidth,
         font_path,point_size,img_width,img_height,label,
         output_image_filename, drop_shadow_flag);
   }

/*
   void generate_text_via_ImageMagick(
      bool randomize_gravity_flag,
      colorfunc::RGB& foreground_RGB, colorfunc::RGB& stroke_RGB,
      colorfunc::RGB& background_RGB,
      double strokewidth, string font_path,int point_size,int width,int height,
      string label,string output_image_filename)
   {
      Magick::Geometry image_size(width, height, 0, 0);
      Magick::ColorRGB background_color(
         background_RGB.first / 255.0,
         background_RGB.second / 255.0,
         background_RGB.third / 255.0);
      Magick::Image image(image_size, background_color);
      image.font(font_path);
      image.fontPointsize(point_size);

      Magick::ColorRGB foreground_color(
         foreground_RGB.first / 255.0,
         foreground_RGB.second / 255.0,
         foreground_RGB.third / 255.0);
      image.fillColor(foreground_color);

      if (strokewidth > 0)
      {
         image.strokeWidth(strokewidth);
         Magick::ColorRGB stroke_color(
            stroke_RGB.first / 255.0,
            stroke_RGB.second / 255.0,
            stroke_RGB.third / 255.0);
         image.strokeColor(stroke_color);
      }

      Magick::GravityType gravity = get_gravity_type(randomize_gravity_flag);
      image.annotate(label, image_size, gravity);

      image.write(output_image_filename);
   }
*/

// ---------------------------------------------------------------------
   void generate_text_via_ImageMagick(
      bool debug_annotate_flag, bool randomize_gravity_flag,
      colorfunc::RGB& foreground_RGB, colorfunc::RGB& stroke_RGB,
      colorfunc::RGBA& background_RGBA,
      bool underbox_flag, colorfunc::RGB& undercolor_RGB,
      double strokewidth, string font_path,int point_size,
      int img_width,int img_height,
      string label,string output_image_filename,
      bool drop_shadow_flag)
      {
//         cout << "inside imagefunc::generate_text_via_IM()" << endl;
//         cout << "strokewidth = " << strokewidth << endl;
//         cout << "font = " << font_path << endl;
//         cout << "point_size = " << point_size << endl;
//         cout << "img_width = " << img_width << " img_height = " << img_height
//              << endl;
//          cout << "label = " << label << endl;

         int foreground_R = foreground_RGB.first;
         int foreground_G = foreground_RGB.second;
         int foreground_B = foreground_RGB.third;

         int stroke_R = stroke_RGB.first;
         int stroke_G = stroke_RGB.second;
         int stroke_B = stroke_RGB.third;

         int background_R = background_RGBA.first;
         int background_G = background_RGBA.second;
         int background_B = background_RGBA.third;
         int background_A = background_RGBA.fourth;

         int undercolor_R = undercolor_RGB.first;
         int undercolor_G = undercolor_RGB.second;
         int undercolor_B = undercolor_RGB.third;

         string unix_cmd = generate_ImageMagick_text_convert_cmd(
            debug_annotate_flag, randomize_gravity_flag,
            foreground_R, foreground_G, foreground_B,
            stroke_R, stroke_G, stroke_B,
            background_R, background_G,background_B,background_A,
            underbox_flag, undercolor_R, undercolor_G, undercolor_B,
            strokewidth, font_path, point_size, img_width, img_height,
            label,drop_shadow_flag);

//         if(debug_annotate_flag)
//         {
//            unix_cmd += " null: > annot.out";
//            unix_cmd += "null: > annot.out";
//         }
//         else
         {
            string suffix=stringfunc::suffix(output_image_filename);
            if(suffix=="png" || suffix=="PNG")
            {
               unix_cmd += "png32:";
            }
            unix_cmd += output_image_filename;
         }

//         cout << endl << unix_cmd << endl;
         sysfunc::unix_command(unix_cmd);
      }

// ---------------------------------------------------------------------   

   string generate_ImageMagick_text_convert_cmd(
      bool debug_annotate_flag, bool randomize_gravity_flag,
      int foreground_R, int foreground_G, int foreground_B,
      int stroke_R, int stroke_G, int stroke_B,
      int background_R, int background_G, int background_B, int background_A,
      bool underbox_flag, 
      int undercolor_R, int undercolor_G, int undercolor_B,
      double strokewidth, string font_path,int point_size,
      int img_width,int img_height,string label, bool drop_shadow_flag)
      {
         string foreground_RGB_hex=colorfunc::RGB_to_RRGGBB_hex(
            foreground_R/255.0,
            foreground_G/255.0,
            foreground_B/255.0);
         string background_RGBA_hex=colorfunc::RGBA_to_RRGGBBAA_hex(
            background_R/255.0,
            background_G/255.0,
            background_B/255.0,
            background_A/255.0);

         string unix_cmd="convert ";
         if(drop_shadow_flag)
         {
            unix_cmd = "montage ";
         }

         if(debug_annotate_flag)
         {
            unix_cmd += "-debug annotate ";
         }
         unix_cmd += "-background '#"+background_RGBA_hex+"' ";
         unix_cmd += "-fill '#"+foreground_RGB_hex+"' ";
         if(strokewidth > 0)
         {
            string stroke_RGB_hex=colorfunc::RGB_to_RRGGBB_hex(
               stroke_R/255.0,
               stroke_G/255.0,
               stroke_B/255.0);
            unix_cmd += "-stroke '#"+stroke_RGB_hex+"' ";
            unix_cmd += "-strokewidth "+stringfunc::number_to_string(
               strokewidth)+" ";
         }

         unix_cmd += "-depth 8 "; // 8 bits per output channel
         unix_cmd += "-font '"+font_path+"' ";

         if(point_size > 0)
         {
            unix_cmd += "-pointsize "+stringfunc::number_to_string(point_size)
               +" ";
            unix_cmd += "-size "+stringfunc::number_to_string(img_width)+"x";
            if(img_height > 0)
            {
               unix_cmd += stringfunc::number_to_string(img_height);
            }
         }
         else
         {
            unix_cmd += "-size "+stringfunc::number_to_string(img_width)+"x";
            unix_cmd += stringfunc::number_to_string(img_height);
         }
         
         if(underbox_flag)
         {
            string undercolor_RGBA_hex=colorfunc::RGBA_to_RRGGBBAA_hex(
               undercolor_R/255.0,
               undercolor_G/255.0,
               undercolor_B/255.0,
               1.0);
            unix_cmd += " -undercolor '#"+undercolor_RGBA_hex+"'";   
         }

         unix_cmd += " -gravity "+get_gravity_location(
            randomize_gravity_flag)+" ";
         unix_cmd += "caption:\""+label+"\" ";
         if(drop_shadow_flag)
         {
            unix_cmd += " -shadow -geometry +1+1 ";
         }
         return unix_cmd;
      }

// ---------------------------------------------------------------------
   void generate_ImageMagick_3D_variegated_text(
      string input_image_filename, string output_image_filename,
      int shade_x, int shade_y)
      {
//         cout << "inside imagefunc::gen_IM_3D_var_text()" << endl;
//         cout << "input_image_filename = " << input_image_filename << endl;
//         cout << "output_image_filename = " << output_image_filename << endl;
         
         string unix_cmd = "convert "+input_image_filename+" -alpha extract ";
         unix_cmd += "-blur -0x3 -shade "+stringfunc::number_to_string(
            shade_x)+"x"+stringfunc::number_to_string(shade_y);
         unix_cmd += " -normalize "+input_image_filename+" ";
         unix_cmd += "-compose Overlay -composite "+input_image_filename;
         unix_cmd += " -alpha on -compose Dst_In -composite png32:"+
            output_image_filename;
//         cout << endl << unix_cmd << endl;
         sysfunc::unix_command(unix_cmd);
      }

// ---------------------------------------------------------------------
   void pad_image(unsigned int padded_width, unsigned int padded_height,
                  string input_image_filename, string padded_images_subdir)
   {
      string basename = filefunc::getbasename(input_image_filename);
      string padded_image_filename=padded_images_subdir
         +stringfunc::prefix(basename)+"_"
         +stringfunc::number_to_string(padded_width)+"x"
         +stringfunc::number_to_string(padded_height)+"."
         +stringfunc::suffix(basename);

      Magick::Geometry image_size(padded_width, padded_height, 0, 0);
      Magick::ColorRGB background_color(0, 0, 0);
      Magick::Image padded_image(image_size, background_color);

      Magick::Image input_image;
      input_image.read(input_image_filename);
      padded_image.composite(input_image, Magick::CenterGravity);

      padded_image.write(padded_image_filename);
   }

// ---------------------------------------------------------------------
   Magick::Image* pad_image(
      unsigned int padded_width, unsigned int padded_height,
      const Magick::Image& input_image)
   {
      Magick::Geometry padded_image_size(padded_width, padded_height, 0, 0);
      Magick::ColorRGB background_color(0, 0, 0);
      Magick::Image *padded_image_ptr = 
         new Magick::Image(padded_image_size, background_color);

      padded_image_ptr->composite(input_image, Magick::CenterGravity);
      return padded_image_ptr;
   }

// ---------------------------------------------------------------------
// Method generate_multicolored_chars_via_ImageMagick() takes in an
// STL vector of foreground character colors whose size must equal
// that of the input text label.  Each char within the text label is
// colored individually.

   void generate_multicolored_chars_via_ImageMagick(
      bool randomize_gravity_flag,string background_color,
      vector<string>& foreground_char_colors,
      double strokewidth, int stroke_R, int stroke_G, int stroke_B,
      double shade_az, double shade_el, 
      string font_path,int point_size,int width,int height,
      string label,string output_image_filename)
   {

// Make sure number of foreground char colors equals number of chars
// within input string label:

      if(foreground_char_colors.size() != label.size())
      {
         cout << "Error inside generate_multicolored_chars_via_ImageMagick()"
              << endl;
         cout << "foreground_char_colors.size() = "
              << foreground_char_colors.size() << endl;
         cout << "label.size() = " << label.size() << endl;
         exit(-1);
      }

      string unix_cmd = "convert -background "+background_color;
      string gravity_location = get_gravity_location(randomize_gravity_flag);
      unix_cmd += " -gravity " + gravity_location; 
      unix_cmd += " -font '"+font_path+"' ";
      unix_cmd += " -pointsize "+stringfunc::number_to_string(point_size);
      unix_cmd += " -size "+stringfunc::number_to_string(width)+"x";
      if (strokewidth > 0)
      {
         unix_cmd += "-strokewidth "+stringfunc::number_to_string(
            strokewidth);
         unix_cmd += " -stroke 'rgb("+stringfunc::number_to_string(
            stroke_R);
         unix_cmd += ","+stringfunc::number_to_string(stroke_G);
         unix_cmd += ","+stringfunc::number_to_string(stroke_B)+")' ";
      }
      if (shade_az > 0 && shade_el > 0)
      {
         unix_cmd += " -shade "+stringfunc::number_to_string(shade_az)+"x"+
            stringfunc::number_to_string(shade_el);
      }

      for(unsigned int i = 0; i < foreground_char_colors.size(); i++)
      {
         unix_cmd += " -fill "+foreground_char_colors[i]+" ";
         
         if(height > 0)
         {
            unix_cmd += stringfunc::number_to_string(height);
         }
         unix_cmd += " caption:'";
//         unix_cmd += " label:'";
         unix_cmd += label[i];
         unix_cmd += "' ";
      } // loop over index i labeling chars within input label string

      unix_cmd += " +append "+output_image_filename;
      cout << "unix_cmd = " << unix_cmd << endl << endl;

      sysfunc::unix_command(unix_cmd);
   }

// ==========================================================================
// Integral image methods
// ==========================================================================

// Method compute_integral_image()

   void compute_integral_image(
      const twoDarray* ztwoDarray_ptr, twoDarray* zinteg_twoDarray_ptr)
   {
      for (unsigned int py=0; py < ztwoDarray_ptr->get_ndim(); py++)
      {
         for (unsigned int px=0; px < ztwoDarray_ptr->get_mdim(); px++)
         {
            double z_integ = ztwoDarray_ptr->get(px,py);
            if(px >= 1)
            {
               z_integ += zinteg_twoDarray_ptr->get(px-1, py);
            }
            if(py >= 1)
            {
               z_integ += zinteg_twoDarray_ptr->get(px, py-1);
            }
            if(px >= 1 && py >= 1)
            {
               z_integ -= zinteg_twoDarray_ptr->get(px-1, py-1);
            }
            zinteg_twoDarray_ptr->put(px,py,z_integ);
         } // loop over px 
      } // loop over py
   }
   
// ---------------------------------------------------------------------
   double bbox_intensity_integral(
      int px_lo, int px_hi, int py_lo, int py_hi,
      const twoDarray* zinteg_twoDarray_ptr)
   {
      return zinteg_twoDarray_ptr->get(px_hi,py_hi) + 
         zinteg_twoDarray_ptr->get(px_lo, py_lo) - 
         zinteg_twoDarray_ptr->get(px_hi, py_lo) - 
         zinteg_twoDarray_ptr->get(px_lo, py_hi);
   }
      
// ---------------------------------------------------------------------

   void compute_gradients_integral_image(
      const twoDarray* ztwoDarray_ptr, twoDarray* integ_grads_twoDarray_ptr)
   {
      twoDarray* xderiv_twoDarray_ptr = new twoDarray(ztwoDarray_ptr);
      twoDarray* yderiv_twoDarray_ptr = new twoDarray(ztwoDarray_ptr);
      twoDarray* gradient_mag_twoDarray_ptr = new twoDarray(ztwoDarray_ptr);

      imagefunc::compute_sobel_gradients(
         ztwoDarray_ptr,xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
         gradient_mag_twoDarray_ptr);
      imagefunc::compute_integral_image(
         gradient_mag_twoDarray_ptr, integ_grads_twoDarray_ptr);

      delete xderiv_twoDarray_ptr;
      delete yderiv_twoDarray_ptr;
      delete gradient_mag_twoDarray_ptr;
   }
   


} // imagefunc namespace

