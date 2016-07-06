// ==========================================================================
// LADARFUNCS stand-alone methods
// ==========================================================================
// Last modified on 4/16/07; 12/4/10; 2/5/11; 4/5/14
// ==========================================================================

#include <map>
#include <set>
#include "image/binaryimagefuncs.h"
#include "color/colorfuncs.h"
#include "image/connectfuncs.h"
#include "math/constants.h"
#include "geometry/convexhull.h"
#include "image/drawfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "astro_geo/geopoint.h"
#include "image/imagefuncs.h"
#include "datastructures/Hashtable.h"
#include "ladar/ladarfuncs.h"
#include "ladar/ladarimage.h"
#include "datastructures/Linkedlist.h"
#include "math/mathfuncs.h"
#include "datastructures/Mynode.h"
#include "math/mypolynomial.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "geometry/polygon.h"
#include "math/prob_distribution.h"
#include "image/recursivefuncs.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

namespace ladarfunc
{

// ==========================================================================
// Flight path methods
// ==========================================================================

// Method public_flight_path_filename takes in the name of a public
// xyz filename which we assume obeys the following strict naming
// convention: "ftdev_s7_03122003_001717_64-66.xyz".  This method
// parses this input string and returns the name of the corresponding
// flight path xyz file: "flight_ftdev_s7_03122003_0011717.xyz"

   string public_flight_path_filename(string& public_xyz_filenamestr)
      {
         int dash_posn[4];
         string substring[4];
         
         dash_posn[0]=public_xyz_filenamestr.find_first_of("_",0);
         substring[0]=public_xyz_filenamestr.substr(0,dash_posn[0]);
         string totalstr=substring[0];

         for (int i=0; i<3; i++)
         {
            dash_posn[i+1]=public_xyz_filenamestr.find_first_of
               ("_",dash_posn[i]+1);
            substring[i+1]=public_xyz_filenamestr.substr(
               dash_posn[i],dash_posn[i+1]-dash_posn[i]);
            totalstr += substring[i+1];
         }
         totalstr += ".xyzp";
         totalstr=filefunc::getdirname(totalstr)+"flight_"
            +filefunc::getbasename(totalstr);
         return totalstr;
      }

// ---------------------------------------------------------------------
// Method shift_Greenwich_to_HAFB_origin subtracts off the location of
// HAFB relative to the Greenwich origin from the input XYZ data.  The
// output xyz values are then effectively measured (in meters)
// relative to some canonical origin centered on HAFB rather than
// Greenwich.  The constant shift was extracted from the Group 94
// ali2xyz code which assumes that HAFB_lat = 42 degs + 27.905 secs,
// HAFB_long = -71 degs + 17.186 secs, HAFB_alt = 50 meters.  The
// Group 94 code then uses a particular ellipsoid earth model to
// convert these (lat,long,alt) coordinates to (x,y,z).

   void shift_HAFB_to_Greenwich_origin(vector<threevector>& XYZ)
      {
         const threevector HAFB_offset(
            359122.405813,4702838.576077,50);	 // meters
         
         for (unsigned int n=0; n<XYZ.size(); n++)
         {
            XYZ[n] += HAFB_offset;
         }
      }

// ---------------------------------------------------------------------
// Method rotate_xy_coords takes in a rotation angle theta along with
// a rotation origin.  It also takes in a set of raw x and y
// coordinates which it rotates about the origin through angle theta.
// This method is useful for aligning the x and y coordinates for raw
// ALIRT data with the natural flight path coordinate system.  It
// thereby helps reduce the size of twoDarrays which are used to store
// z and p images.  To avoid pixelization problems, this method should
// be called BEFORE data from a raw xyzp file is binned onto z2Darray
// and p2Darray!

   void rotate_xy_coords(
      double theta,const threevector& origin,vector<threevector>& XYZ)
      {
         double cos_theta=cos(theta);
         double sin_theta=sin(theta);
         for (unsigned int i=0; i<XYZ.size(); i++)
         {
            double dx=XYZ[i].get(0)-origin.get(0);
            double dy=XYZ[i].get(1)-origin.get(1);
            double dxnew=cos_theta*dx-sin_theta*dy;
            double dynew=sin_theta*dx+cos_theta*dy;
            XYZ[i].put(0,origin.get(0)+dxnew);
            XYZ[i].put(1,origin.get(1)+dynew);
         }
      }

// ==========================================================================
// Ladar data bounding box methods:
// ==========================================================================

// Method draw_data_bbox draws the approximate bounding box
// surrounding non-null data within the current ladar image in white
// onto *ztwoDarray_ptr.

   void draw_data_bbox(
      parallelogram const *data_bbox_ptr,twoDarray* ztwoDarray_ptr)
      {
         outputfunc::write_banner("Drawing data bounding box:");
         double x_extent=ztwoDarray_ptr->get_xhi()-ztwoDarray_ptr->get_xlo();
         double y_extent=ztwoDarray_ptr->get_yhi()-ztwoDarray_ptr->get_ylo();
         double radius=0.005*sqrt(sqr(x_extent)+sqr(y_extent));
         drawfunc::draw_thick_polygon(
            *data_bbox_ptr,colorfunc::white,radius,ztwoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method null_data_outside_bbox sets the z value of every point in
// twoDarray *ztwoDarray_ptr which lies outside data bounding box
// *data_bbox_ptr equal to xyzpfunc::null_value.

   void null_data_outside_bbox(
      parallelogram const *data_bbox_ptr,twoDarray* ztwoDarray_ptr)
      {
         outputfunc::write_banner("Nulling datapoints outside bounding box:");

         threevector currpoint;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
               if (!data_bbox_ptr->point_inside(currpoint))
               {
                  ztwoDarray_ptr->put(px,py,xyzpfunc::null_value);
               }
            } // loop over index py
         } // loop over index px
      }

// ---------------------------------------------------------------------
   void fake_compute_data_bbox(
      twoDarray const *ztwoDarray_ptr,parallelogram* data_bbox_ptr)
      {
         vector<threevector> vertex(4);
         vertex[0]=threevector(
            ztwoDarray_ptr->get_xlo(),ztwoDarray_ptr->get_ylo(),0);
         vertex[1]=threevector(
            ztwoDarray_ptr->get_xlo(),ztwoDarray_ptr->get_yhi(),0);
         vertex[2]=threevector(
            ztwoDarray_ptr->get_xhi(),ztwoDarray_ptr->get_yhi(),0);
         vertex[3]=threevector(
            ztwoDarray_ptr->get_xhi(),ztwoDarray_ptr->get_ylo(),0);
         data_bbox_ptr=new parallelogram(vertex);
      }

// ---------------------------------------------------------------------
// In order to minimize the impact of noisy pixels located nearby the
// edges of the data bounding box, method crop_data_inside_bbox crop
// away the outer elimination_frac of the image contained within
// *ftwoDarray_ptr.

   void crop_data_inside_bbox(
      double elimination_frac,parallelogram const *data_bbox_ptr,
      twoDarray* ftwoDarray_ptr)
      {
         parallelogram bbox(*data_bbox_ptr);
         bbox.scale(1-elimination_frac);
         null_data_outside_bbox(&bbox,ftwoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Method color_points_near_data_bbox takes in the ladar data bounding
// box as well as twoDarray *ftwoDarray_ptr.  It colors all points in
// *ftwoDarray_ptr which lie less than dist_from_bbox from the data
// bounding box.

   void color_points_near_data_bbox(
      double dist_from_bbox,polygon* data_bbox_ptr,twoDarray* ftwoDarray_ptr)
      {
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               threevector currpoint;
               if (ftwoDarray_ptr->pixel_to_point(px,py,currpoint))
               {
                  double distance_to_bbox=data_bbox_ptr->
                     point_dist_to_polygon(currpoint);
                  if (distance_to_bbox < dist_from_bbox)
                  {
                     ftwoDarray_ptr->put(px,py,1.0);
                  }
               }
            } // loop over py
         } // loop over px
      }

// ---------------------------------------------------------------------
// Method xy_data_bbox takes in twoDarray *ztwoDarray_ptr which holds
// a ladar height image.  It also takes in grid cell dimensions
// delta_x and delta_y.  This method dynamically generates a bounding
// box oriented along the x and y axes which contains an integer
// number of grid cells in both dimensions and whose center coincides
// with that of the height image.  This polygon is returned by this
// method and can be used for underlaying a coordinate system in ladar
// xyzp output.

   polygon* xy_data_bbox(
      twoDarray const *ztwoDarray_ptr,double delta_x,double delta_y)
      {
         double xlo=ztwoDarray_ptr->get_xlo();
         double xhi=ztwoDarray_ptr->get_xhi();
         double ylo=ztwoDarray_ptr->get_ylo();
         double yhi=ztwoDarray_ptr->get_yhi();
         polygon xybox(xlo,ylo,xhi,yhi);
   
         int mtics=basic_math::mytruncate((xhi-xlo)/delta_x)+1;
         int ntics=basic_math::mytruncate((yhi-ylo)/delta_y)+1;
         double xgrid_extent=mtics*delta_x;
         double ygrid_extent=ntics*delta_y;

//         cout << "xgrid_extent = " << xgrid_extent
//              << " ygrid_extent = " << ygrid_extent << endl;
         polygon* grid_poly_ptr=new polygon(0,0,xgrid_extent,ygrid_extent);
         grid_poly_ptr->translate(
            xybox.vertex_average()-grid_poly_ptr->vertex_average());
         return  grid_poly_ptr;
      }
   
// ==========================================================================
// Height image cleaning methods:
// ==========================================================================

// Method median_fill_image takes in a twoDarray containing raw
// imagery data and fills in holes where data is missing via median
// filtering.

   void median_fill_image(int niters,int nsize,twoDarray* ztwoDarray_ptr)
      {
         outputfunc::write_banner("Median filling z-image:");
         
         twoDarray* ztwoDarray_filled_ptr=new twoDarray(ztwoDarray_ptr);
         ztwoDarray_ptr->copy(ztwoDarray_filled_ptr);
         for (int iter=0; iter<niters; iter++)
         {
            cout << iter << " " << flush;
            int n_null_pixels_before=imagefunc::count_pixels_below_zmax(
               0.5*xyzpfunc::null_value,ztwoDarray_ptr);
            cout << "n_null_pixels_before = " 
                 << n_null_pixels_before << endl;
            imagefunc::median_fill(
               nsize,ztwoDarray_ptr,ztwoDarray_filled_ptr,
               xyzpfunc::null_value);
            cout << "Finished curr iter's median fill" << endl;
            ztwoDarray_filled_ptr->copy(ztwoDarray_ptr);
            int n_null_pixels_after=
               imagefunc::count_pixels_below_zmax(
                  0.5*xyzpfunc::null_value,ztwoDarray_ptr);
            cout << "iter = " << iter << " number of filled pixels = "
                 << (n_null_pixels_before-n_null_pixels_after) << endl;
         }
         outputfunc::newline();
         delete ztwoDarray_filled_ptr;
      }

   void median_fill_image(int niters,int nsize,parallelogram* bbox_ptr,
                          twoDarray* ztwoDarray_ptr)
      {
         outputfunc::write_banner("Median filling z-image:");
         
         twoDarray* ztwoDarray_filled_ptr=new twoDarray(ztwoDarray_ptr);
         ztwoDarray_ptr->copy(ztwoDarray_filled_ptr);
         for (int iter=0; iter<niters; iter++)
         {
            cout << iter << " " << flush;
            int n_null_pixels_before=
               imagefunc::count_pixels_below_zmax_inside_parallelogram(
                  0.5*xyzpfunc::null_value,*bbox_ptr,ztwoDarray_ptr);
            imagefunc::median_fill(
               nsize,*bbox_ptr,ztwoDarray_ptr,ztwoDarray_filled_ptr,
               xyzpfunc::null_value);
            ztwoDarray_filled_ptr->copy(ztwoDarray_ptr);
            int n_null_pixels_after=
               imagefunc::count_pixels_below_zmax_inside_parallelogram(
                  0.5*xyzpfunc::null_value,*bbox_ptr,ztwoDarray_ptr);
            cout << "iter = " << iter << " number of filled pixels = "
                 << (n_null_pixels_before-n_null_pixels_after) << endl;
         }
         outputfunc::newline();
         delete ztwoDarray_filled_ptr;
      }

   void median_fill_image(int nsize,parallelogram* bbox_ptr,
                          twoDarray* ztwoDarray_ptr)
      {
         outputfunc::write_banner("Median filling z-image:");

         twoDarray* ztwoDarray_filled_ptr=new twoDarray(ztwoDarray_ptr);
         
         const int niters_max=20;
         int iter=0;
         int n_null_pixels0=
            imagefunc::count_pixels_below_zmax_inside_parallelogram(
               0.5*xyzpfunc::null_value,*bbox_ptr,ztwoDarray_ptr);

         double frac_filled[2];
         frac_filled[0]=frac_filled[1]=0;
         do
         {
            imagefunc::median_fill(
               nsize,*bbox_ptr,ztwoDarray_ptr,ztwoDarray_filled_ptr,
               xyzpfunc::null_value);
            ztwoDarray_filled_ptr->copy(ztwoDarray_ptr);
            int n_null_pixels=
               imagefunc::count_pixels_below_zmax_inside_parallelogram(
                  0.5*xyzpfunc::null_value,*bbox_ptr,ztwoDarray_ptr);
            frac_filled[0]=frac_filled[1];
            frac_filled[1]=1-double(n_null_pixels)/double(n_null_pixels0);
            cout << "iter = " << iter 
                 << " n_null_pixels = " << n_null_pixels 
                 << " frac filled = " << frac_filled[1] << endl;
            iter++;
         }
         while(frac_filled[1] < 0.999 && frac_filled[1] != frac_filled[0]
               && iter < niters_max);
         
         outputfunc::newline();
         delete ztwoDarray_filled_ptr;
      }

// ---------------------------------------------------------------------
// Method remove_isolated_outliers scans through the height values
// within the median-filled twoDarray *ztwoDarray_ptr.  For each
// non-null valued pixel, this method computes the mean and variance
// of its neighbors.  If the pixel's value lies many standard
// deviations away from the mean of its neighbors, this method sets
// the pixel's value equal to the mean.

   int remove_isolated_outliers(
      parallelogram const *bbox_ptr,twoDarray* ztwoDarray_ptr)
      {
         outputfunc::write_banner("Removing isolated height outliers:");

         const int xsize=5;	 // pixel width of scanning bbox
         const int ysize=5;	 // pixel height of scanning bbox
         const int ntotal=xsize*ysize-1;
         const int min_nonnull_pnts=ntotal-2;
         const double max_variance_factor=sqr(10.0); 
         const double min_height_separation=7.5;	// meters
         threevector currpoint;
         double z[ntotal];

         int nzvalues_changed=0;
         for (unsigned int px=xsize/2; px<ztwoDarray_ptr->get_mdim()-xsize/2; 
              px++)
         {
            for (unsigned int py=ysize/2; py<ztwoDarray_ptr->
                    get_ndim()-ysize/2; py++)
            {
               ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
               if (bbox_ptr->point_inside(currpoint))
               {
                  double currz=ztwoDarray_ptr->get(px,py);
               
                  int n=0;
                  for (int i=-xsize/2; i<=xsize/2; i++)
                  {
                     for (int j=-ysize/2; j<=ysize/2; j++)
                     {
                        if (!(i==0 && j==0))
                        {
                           double candidate_z=ztwoDarray_ptr->get(px+i,py+j);
                           if (candidate_z > 0.99*xyzpfunc::null_value) 
                           {
                              z[n]=candidate_z;
                              n++;
                           }
                        }
                     } // loop over local index j
                  } // loop over local index i
                  if (n >= min_nonnull_pnts)
                  {
                     double mean=templatefunc::average(z,n);
                     double sigma_sqr=mathfunc::variance(z,n);
                     if (fabs(currz-mean) > min_height_separation &&
                         sqr(currz-mean) > max_variance_factor*sigma_sqr)
                     {
//                        outputfunc::newline();
//                        cout << "px = " << px << " py = " << py << endl;
//                        for (int m=0; m<n; m++)
//                        {
//                           cout << "m = " << m << " z[m] = " << z[m] 
//                                << endl;
//                        }
//                        cout << "x = " << currpoint.get(0) 
//                             << " y = " << currpoint.get(1) 
//                             << " currz = " << currz << endl;
//                        cout << "mean = " << mean 
//                             << " sigma = " << sqrt(sigma_sqr) << endl;
                        ztwoDarray_ptr->put(px,py,mean);
                        nzvalues_changed++;
                     }
                  } // n >= min_nonnull_pnts conditional
               } // point inside conditional
            } // loop over global py index
         } // loop over global px index

         cout << "Number of isolated outliers eliminated = " 
              << nzvalues_changed << endl;
         return nzvalues_changed;
      }

// ---------------------------------------------------------------------
// This refined version of remove_isolated_outliers uses feature
// classification as well as height information to identify outlier
// voxels.  It searches within the ladar data bounding box *bbox_ptr
// for all pixels whose feature value within the input feature map
// *ftwoDarray_ptr equals the specified input feature_value.  When it
// finds one such pixel, it forms a bounding box of pixel size xsize &
// ysize surrounding the classified pixel.  This method computes the
// height distribution of all similarly valued pixels within the
// smaller bounding box.  If the center pixel's height lies too many
// standard deviations away from the mean of those within the smaller
// bounding box, its height is reset to the mean.

   int remove_isolated_outliers(
      double feature_value,parallelogram const *bbox_ptr,
      twoDarray* ztwoDarray_ptr,twoDarray const *ftwoDarray_ptr,
      int xsize,int ysize,int min_nonnull_pnts,
      double min_height_separation,double max_variance_factor)
      {
         outputfunc::write_banner("Removing isolated height outliers:");

         const int ntotal=xsize*ysize-1;
         threevector currpoint;
         double z[ntotal];

         int nzvalues_changed=0;
         for (unsigned int px=xsize/2; px<ztwoDarray_ptr->get_mdim()-xsize/2; 
              px++)
         {
            for (unsigned int py=ysize/2; py<ztwoDarray_ptr->
                    get_ndim()-ysize/2; py++)
            {
               ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
               double curr_feature_value=ftwoDarray_ptr->get(px,py);

               if (bbox_ptr->point_inside(currpoint) &&
                   nearly_equal(curr_feature_value,feature_value))
               {
                  double currz=ztwoDarray_ptr->get(px,py);
               
                  int n=0;
                  for (int i=-xsize/2; i<=xsize/2; i++)
                  {
                     for (int j=-ysize/2; j<=ysize/2; j++)
                     {
                        if (!(i==0 && j==0))
                        {
                           double candidate_z=ztwoDarray_ptr->get(px+i,py+j);
                           double candidate_f=ftwoDarray_ptr->get(px+i,py+j);
                           
                           if (candidate_z > 0.99*xyzpfunc::null_value &&
                               nearly_equal(candidate_f,feature_value))
                           {
                              z[n]=candidate_z;
                              n++;
                           }
                        }
                     } // loop over local index j
                  } // loop over local index i
                  if (n >= min_nonnull_pnts)
                  {
                     double mean=templatefunc::average(z,n);
                     double sigma_sqr=mathfunc::variance(z,n);
                     if (fabs(currz-mean) > min_height_separation &&
                         sqr(currz-mean) > max_variance_factor*sigma_sqr)
                     {
//                        outputfunc::newline();
//                        cout << "px = " << px << " py = " << py << endl;
//                        for (int m=0; m<n; m++)
//                        {
//                           cout << "m = " << m << " z[m] = " << z[m] 
//                                << endl;
//                        }
//                        cout << "x = " << currpoint.get(0) 
//                             << " y = " << currpoint.get(1) 
//                             << " currz = " << currz << endl;
//                        cout << "mean = " << mean 
//                             << " sigma = " << sqrt(sigma_sqr) << endl;
                        ztwoDarray_ptr->put(px,py,mean);
                        nzvalues_changed++;
                     }
                  } // n >= min_nonnull_pnts conditional
               } // point inside conditional
            } // loop over global py index
         } // loop over global px index

         cout << "Number of isolated outliers eliminated = " 
              << nzvalues_changed << endl;
         return nzvalues_changed;
      }

// ---------------------------------------------------------------------
// Method threshold_zimage takes in a z-image within input twoDarray
// *ztwoDarray_ptr.  It sets to null all pixels whose heights lie
// outside the ranges z < negativez_threshold and zmin < z < zmax.

   void threshold_zimage(
      double zmin,double zmax,twoDarray* ztwoDarray_ptr)
      {
         outputfunc::write_banner("Thresholding zimage:");

// Keep points within flattened height image which are 5 or more
// meters less than local ground in order to retain canals:

//          double negativez_threshold=-5;	// meters

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double currz=ztwoDarray_ptr->get(px,py);
               if (!(currz > zmin && currz < zmax))
//               if (!(currz < negativez_threshold ||
//                     (currz > zmin && currz < zmax)))
               {
                  ztwoDarray_ptr->put(px,py,xyzpfunc::null_value);
               }
            }
         }
      }

// ---------------------------------------------------------------------
// Method null_imagetwo_bright_pixels_in_imageone takes in 2
// twoDarrays as well as threshold value zthreshold.  If the intensity
// at some pixel within the 2nd twoDarray exceeds zthreshold, this
// method sets that pixel's value within the 1st twoDarray equal to
// xyzpfunc::null_value.

   void null_imagetwo_bright_pixels_in_imageone(
      double zthreshold,twoDarray const *ztwoDarray2_ptr,
      twoDarray* ztwoDarray1_ptr)
      {
         outputfunc::write_banner("Nulling bright pixels:");

         int npixels_nulled=0;
         for (unsigned int px=0; px<ztwoDarray2_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray2_ptr->get_ndim(); py++)
            {
               double currz=ztwoDarray2_ptr->get(px,py);
               if (currz > zthreshold)
               {
                  ztwoDarray1_ptr->put(px,py,xyzpfunc::null_value);
                  npixels_nulled++;
               }
            } // loop over py index
         } // loop over px index
         cout << "Number of pixels nulled = " << npixels_nulled << endl;
      }

// ---------------------------------------------------------------------
// Method compute_xyzp_distributions generates probability
// distributions for x, y, z and p data extracted from Group 94 binary
// xyzp files.  These distributions can be used to determine the
// regularity of deltax and deltay binning.  On 6/9/03, we found that
// deltax and deltay only approximately equal 30 cm.  They fluctuate
// slightly about this central value.  

   void compute_xyzp_distributions(
      ladarimage* xyzimage_ptr,const vector<threevector>& XYZ,
      const vector<double>& p)
      {
//         vector<double> x,y,z;
//         for (unsigned int n=0; n<XYZ.size(); n++)
//         {
//            x.push_back(XYZ[n].get(0));
//            y.push_back(XYZ[n].get(1));
//            z.push_back(XYZ[n].get(2));
//         }
         
//         prob_distribution prob_x(x,50);
//         prob_x.set_densityfilenamestr("prob_x.meta");
//         prob_x.set_xlabel("X (meters)");
//         prob_x.write_density_dist();

//         prob_distribution prob_y(y,50);
//         prob_y.set_densityfilenamestr("prob_y.meta");
//         prob_y.set_xlabel("Y (meters)");
//         prob_y.write_density_dist();

//         int n_zbins=500;
//         prob_distribution prob_z(z,n_zbins);
//         prob_z.compute_cumulative_distribution();
//         prob_z.compute_density_distribution();

/*
         prob_z.smooth_density_distribution();
         const double x_interval=2;	// meters
         vector<pair<int,mypolynomial> >* peak_bin_ptr=
            prob_z.locate_density_peaks(x_interval,2,10);
         prob_z.fit_gaussians_to_density_peaks(peak_bin_ptr,1);
         delete peak_bin_ptr;
*/

//         prob_z.set_densityfilenamestr("prob_z.meta");
//         prob_z.set_xlabel("Z (meters)");
//         prob_z.write_density_dist();

// Compute density and cumulative distributions for probability
// values:

//         int npoints=500;
//         double p_sanitized[xyzimage_ptr->get_npoints()];
//         int ngood=0;
//         for (int n=0; n<xyzimage_ptr->get_npoints(); n++)
//         {
//            if (p[n] >= 0) p_sanitized[ngood++]=p[n];
//         }
//         prob_distribution prob_p(ngood,p_sanitized,npoints);
//         prob_p.set_densityfilenamestr(xyzimage_ptr->imagedir+"raw_density.meta");
//         prob_p.set_cumulativefilenamestr(xyzimage_ptr->imagedir+"raw_cum.meta");
//         prob_p.set_xlabel("Relative intensity");
//         prob_p.writeprobdists();
      }
   
/*
// ---------------------------------------------------------------------
// Method locate_strong_edge_lumps searches for strong, significant
// and connected edge regions in the gradient image contained within
// *gradient_mag_twoDarray_ptr.  It returns the number of such lumps
// which it finds along with their COM locations and median threshold
// intensity values.

   void locate_strong_edge_lumps(
      int& nlumps,double edgelump_threshold[],
      vector<threevector>& edgelump_COM,
      vector<threevector>& edgelump_dir,const polygon& body_bbox,
      twoDarray* gradient_mag_twoDarray_ptr,
      twoDarray const *xderiv_twoDarray_ptr,
      twoDarray const *yderiv_twoDarray_ptr)
      {
         const bool merge_close_lumps_together=false;
         const int max_nlumps=1000;

         nlumps=0;
         linkedlist* lumplist_ptr[max_nlumps];
         recursivefunc::locate_hot_pixel_clusters(
            max_nlumps,nlumps,edgelump_COM,lumplist_ptr,
            gradient_mag_twoDarray_ptr,0,0,merge_close_lumps_together,0);
         const double intensity_frac=0.5;
         recursivefunc::compute_cluster_thresholds(
            nlumps,lumplist_ptr,gradient_mag_twoDarray_ptr,
            intensity_frac,edgelump_threshold);

// Compute averaged gradient direction vector for each hot lump within
// linkedlist:

         for (int l=0; l<nlumps; l++)
         {
            double xnumer=0;
            double ynumer=0;
            double denom=0;
            mynode* currnode_ptr=lumplist_ptr[l]->get_start_ptr();
            while (currnode_ptr != NULL)
            {
               int px=basic_math::round(currnode_ptr->get_data().get_var(0));
               int py=basic_math::round(currnode_ptr->get_data().get_var(1));
               double w=gradient_mag_twoDarray_ptr->get(px,py);
               xnumer += w*xderiv_twoDarray_ptr->get(px,py);
               ynumer += w*yderiv_twoDarray_ptr->get(px,py);
               denom += w;
               currnode_ptr=currnode_ptr->get_nextptr();
            }
            if (denom > 0)
            {
               edgelump_dir[l]=threevector(xnumer/denom,ynumer/denom,0);
            }
            else
            {
               edgelump_dir[l]=Zero_vector;
            }
//            cout << "l = " << l 
//                 << " edgelump_COM.x = " << edgelump_COM[l].get(0)
//                 << " edgelump_COM.y = " << edgelump_COM[l].get(1)
//                 << " edgelump_dir.x = " << edgelump_dir[l].get(0)
//                 << " edgelump_dir.y = " << edgelump_dir[l].get(1)
//                 << " t = " << edgelump_threshold[l] << endl;
         }

// Scan through edgelump array.  Remove any edge lumps whose averaged
// gradient is nearly horizontal or vertical:

         int nlumps_to_delete=0;
         double crit_angle=7.5;	// degs
         int lump_number_to_remove[nlumps];
         for (int l=0; l<nlumps; l++)
         {
            double theta=atan2(
               edgelump_dir[l].get(1),edgelump_dir[l].get(0))*180/PI;
            theta=basic_math::phase_to_canonical_interval(theta,-180,180);

            if ((fabs(theta-0) < crit_angle) 
                || (fabs(theta-180) < crit_angle) 
                || (fabs(theta+180) < crit_angle) 
                || (fabs(theta-90) < crit_angle) 
                || (fabs(theta+90) < crit_angle))
            {
               lump_number_to_remove[nlumps_to_delete]=l;
               nlumps_to_delete++;
            }
            
// We more liberally attempt to remove horizontal and vertical
// oriented edge lumps corresponding to main body range and side
// lobes.  Such lobes must be located within horizontal and vertical
// corridors centered upon the main body bounding box:

            double crit_angle2=15; // degs
            double curr_x=edgelump_COM[l].get(0);
            double curr_y=edgelump_COM[l].get(1);
            double tolerance=1.0;	// meter
            if ((curr_x > body_bbox.get_vertex(0).get(0)+tolerance &&
                 curr_x < body_bbox.get_vertex(2).get(0)-tolerance) ||
                (curr_y > body_bbox.get_vertex(0).get(1)+tolerance &&
                 curr_y < body_bbox.get_vertex(2).get(1)-tolerance))
            {
               if ((fabs(theta-0) < crit_angle2) 
                   || (fabs(theta-180) < crit_angle2) 
                   || (fabs(theta+180) < crit_angle2) 
                   || (fabs(theta-90) < crit_angle2) 
                   || (fabs(theta+90) < crit_angle2))
               {
                  lump_number_to_remove[nlumps_to_delete]=l;
                  nlumps_to_delete++;
               }
            }
         } // loop over index l labeling lump number

         for (int l=0; l<nlumps; l++) delete lumplist_ptr[l];

// Move information about lumps whose average directions are too close
// to horizontal or vertical to end of lump arrays.  Then simply reset
// nlumps to nlumps-nlumps_to_delete in order to ignore these
// horiz/vert directed lumps.

         for (int l=0; l<nlumps_to_delete; l++)
         {
            move_element_to_array_end(
               lump_number_to_remove[l],nlumps,edgelump_threshold);
            move_element_to_array_end(
               lump_number_to_remove[l],nlumps,edgelump_COM);
            move_element_to_array_end(
               lump_number_to_remove[l],nlumps,edgelump_dir);
         }
         nlumps -= nlumps_to_delete;
         nlumps=max(2,nlumps);
      }
*/


// ---------------------------------------------------------------------
// Method mark_tall_clusters takes in a raw z-image within input
// twoDarray *ztwoDarray_ptr along with another z-image containing
// whose non-null valued pixels correspond to only tall clustered
// objects (e.g. building rooftops).  It returns within twoDarray
// *ftwoDarray_ptr a new image for which the intensity values of
// pixels corresponding to the tall objects are set equal to
// zthreshold, while the intensities of all other pixels are kept at
// their original z values.  This method is useful for displaying tall
// objects in white.

// We add some large constant to all of the pixels associated with
// each tall object.  The Group 94 dataviewer will then assign
// qualitatively different colors to the tall objects which helps make
// them stand out against other tall objects which are not marked.

   void mark_tall_clusters(
      twoDarray const *ztwoDarray_ptr,twoDarray const *zcluster_twoDarray_ptr,
      twoDarray* ftwoDarray_ptr,double tall_object_null_value)
      {
         outputfunc::write_banner("Marking tall clusters:");

         double min_z,max_z;
         ztwoDarray_ptr->minmax_values(min_z,max_z);
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double currz=ztwoDarray_ptr->get(px,py);
               double curr_zcluster=zcluster_twoDarray_ptr->get(px,py);
               if (curr_zcluster > tall_object_null_value)
               {
//            ftwoDarray_ptr->put(px,py,50);
                  ftwoDarray_ptr->put(px,py,currz+50);
               }
               else
               {
                  ftwoDarray_ptr->put(px,py,currz);
               }
            } // loop over py
         } // loop over px
      }

// ---------------------------------------------------------------------
// Method generate_xyzpfile_with_tall_object_info fills an ftwoDarray
// object with modified height information.  If a particular pixel
// within input twoDarray *ztwoDarray_ptr (which we assume corresponds
// to a cleaned & flattened zimage) belongs to a cluster of tall
// pixels within input twoDaray *zcluster_twoDarray_ptr (which we
// assume was previously computed via the find_large_hot_lumps()
// member function of this class), this method either sets its
// corresponding "p" value to equal to z+zoffset.  (The value of the
// zoffset constant is hardwired within
// ladarfunc::mark_tall_clusters().)  Otherwise, its "p" value is set
// equal to its cleaned, flattened z value.  An xyzp file with these
// fictitious "probabilities" is written out by this method.  When
// read in by our modified version of the Group 94 3D dataviewer and
// viewed using one of our modified colormaps which color the tallest
// pixels in white, the output generated by this method effectively
// colors tall pixel clusters in white.

   void generate_xyzpfile_with_tall_object_info(
      string tall_objects_filenamestr,parallelogram const *data_bbox_ptr,
      double zmax,twoDarray* ztwoDarray_ptr,
      twoDarray const *zcluster_twoDarray_ptr,double tall_object_null_value)
      {
         outputfunc::write_banner(
            "Generating xyzp file with tall object info:");

         twoDarray* ftwoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         threshold_zimage(xyzpfunc::null_value,zmax,ztwoDarray_ptr);

// In order to cover up spurious edge effects, we simply null all
// pixels located outside some large fraction of the data bounding box
// before writing out the tall object xyzp file:

//   parallelogram shrunken_bbox=*data_bbox_ptr;
//   shrunken_bbox.scale(0.985);
//   drawfunc::null_pixels_outside_parallelogram(
//      xyzpfunc::null_value,&shrunken_bbox,ztwoDarray_ptr);
         null_data_outside_bbox(data_bbox_ptr,ztwoDarray_ptr);

         mark_tall_clusters(
            ztwoDarray_ptr,zcluster_twoDarray_ptr,ftwoDarray_ptr,
            tall_object_null_value);

         xyzpfunc::write_xyzp_data(
            ztwoDarray_ptr,ftwoDarray_ptr,tall_objects_filenamestr,false);
         delete ftwoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method compute_voxel_median_height takes in a linked list of voxels
// and returns the cluster's median height.

   double compute_voxel_median_height(
      linkedlist* voxel_list_ptr,twoDarray const *ztwoDarray_ptr)
      {
         const int n_nodes=voxel_list_ptr->size();
         double *z_value=new double[n_nodes];
         int n_currvoxel=0;
         mynode* curr_voxel_ptr=voxel_list_ptr->get_start_ptr();
         while (curr_voxel_ptr != NULL)
         {
            int px=basic_math::round(curr_voxel_ptr->get_data().get_var(0));
            int py=basic_math::round(curr_voxel_ptr->get_data().get_var(1));
            z_value[n_currvoxel++]=ztwoDarray_ptr->get(px,py);
            curr_voxel_ptr=curr_voxel_ptr->get_nextptr();
         }
         prob_distribution prob(n_nodes,z_value,30);
         delete [] z_value;
         
         return prob.median();
      }
   
// ---------------------------------------------------------------------
// Method compute_z_distribution dynamically plots the probability
// density distribution for height data contained within input
// twoDarray *ztwoDarray_ptr.

   double compute_z_distribution(
      string imagedir,twoDarray const *ztwoDarray_ptr)
      {
         outputfunc::write_banner("Plotting height distribution:");
         int n_output_bins=600;
   
         double zmin=0.5*xyzpfunc::null_value;
         prob_distribution prob_z;
         imagefunc::image_intensity_distribution(
            zmin,ztwoDarray_ptr,prob_z,n_output_bins);
   
         prob_z.set_densityfilenamestr(imagedir+"z_density.meta");
         prob_z.set_cumulativefilenamestr(imagedir+"z_cum.meta");
         prob_z.set_xlabel("Height z (meters)");
//         prob_z.write_density_dist();
         prob_z.writeprobdists();

         double zlo=prob_z.get_x(0);
         double zhi=prob_z.get_x(prob_z.get_nbins()-1);
         vector<pair<int,mypolynomial> >* peak_bin_ptr=
            prob_z.locate_density_peaks(zhi-zlo,zlo,zhi);
         int max_bin=(*peak_bin_ptr)[0].first;
//         cout << "max_bin = " << max_bin << endl;
         cout << "Height for which z density distribution is maximal = " 
              << prob_z.get_x(max_bin) << endl;
//         cout << "p max = " << prob_z.get_p(max_bin) << endl;
         return prob_z.get_x(max_bin);
      }

// ---------------------------------------------------------------------
// Method mark_snowflake_points takes in an STL vector of XYZ triples
// which have only had their offset values renormalized.  This method
// computes the height distribution corresponding to the Z values and
// finds the median raw height value.  It then performs a search in
// z-space starting at the median z value for reasonable ends of the
// height distribution.  We define the onset of "snowflake" regions
// within the raw ladar data to correspond to those height values for
// which the height distribution density falls below some small
// min_density fraction.  Z values for 3D cloud points lying within
// these snowflake regions are set to xyzpfunc::null_value by this
// method.

   void mark_snowflake_points(
      double min_z,double max_z,double delta_z,vector<threevector>& XYZ)
      {
         const int nzbins=basic_math::round(floor((max_z-min_z)/delta_z));

         vector<double> Zvec;
         Zvec.reserve(XYZ.size());
         for (unsigned int n=0; n<XYZ.size(); n++)
         {
            Zvec.push_back(XYZ[n].get(2));
         }

         prob_distribution prob(Zvec,nzbins);
         prob.set_densityfilenamestr("raw_z_dist.meta");
//         prob.set_xlabel("Z (meters)");
//         prob.write_density_dist();

         double min_zdensity=1E-5;
         int n=prob.get_bin_number(prob.median());
         double curr_p=0.5;
         do
         {
            curr_p=prob.get_p(n);
            n++;
         }
         while(curr_p > min_zdensity && n < prob.get_nbins()-1);
         double z_hi=prob.get_x(n-1);

         min_zdensity=1E-4;
         n=prob.get_bin_number(prob.median());
         do
         {
            curr_p=prob.get_p(n);
            n--;
         }
         while(curr_p > min_zdensity && n >= 0);
         double z_lo=prob.get_x(n+1);

         cout << "z_lo = " << z_lo << " z_hi = " << z_hi << endl;

// Reset all z values for "snow" points within the raw 3D point cloud
// to a sentinel xyzpfunc::null_value.  Such noise points should NOT
// be included in the ladarimage object.

         for (unsigned int nz=0; nz<Zvec.size(); nz++)
         {
            if (Zvec[nz] < z_lo || Zvec[nz] > z_hi)
            {
               XYZ[nz].put(2,xyzpfunc::null_value);
            }
            else
            {
               XYZ[nz].put(2,XYZ[nz].get(2)-z_lo);
            }
         }
      }

// ---------------------------------------------------------------------
// This next variant of mark_snowflake_points only attempts to
// statistically identify spurious noise points lying below the main
// ground surface.

   void mark_belowground_snowflake_points(
      double min_zdensity,double min_z,double max_z,
      double delta_z,osg::Vec3Array* vertices_ptr,double ceiling_min_z)
      {
         const int nzbins=basic_math::round(floor((max_z-min_z)/delta_z));

         vector<double> Zvec;
         Zvec.reserve(vertices_ptr->size());
         for (unsigned int n=0; n<vertices_ptr->size(); n++)
         {
            Zvec.push_back(vertices_ptr->at(n).z());
         }

         prob_distribution prob(Zvec,nzbins);
//         prob.set_densityfilenamestr("raw_z_dist.meta");
//         prob.set_xlabel("Z (meters)");
//         prob.write_density_dist();

         int n=prob.get_bin_number(prob.median());
         double curr_p;
         do
         {
            curr_p=prob.get_p(n);
            n--;
         }
         while(curr_p > min_zdensity && n >= 0);
         double z_lo=prob.get_x(n+1);
         cout << "z_lo = " << z_lo << endl;

// Allow user to manually set a ceiling upon z_lo:

         z_lo=basic_math::min(z_lo,ceiling_min_z);

// Reset all z values for "snow" points within the raw 3D point cloud
// to a sentinel xyzpfunc::null_value.  Such noise points should NOT
// be included in the ladarimage object.

         for (unsigned int nz=0; nz<Zvec.size(); nz++)
         {
            if (Zvec[nz] < z_lo)
            {
               vertices_ptr->at(nz).set(
                  vertices_ptr->at(nz).x(),vertices_ptr->at(n).x(),
                  xyzpfunc::null_value);
            }
         }
      }

// ==========================================================================
// Height gradient computation methods:
// ==========================================================================

// Method compute_x_y_deriv_fields takes in a twoDarray.  It generates
// 1D gaussian derivative filters proportional to x exp(-x**2) and y
// exp(-y**2).  This method performs a brute force convolution of
// these smooth derivative filters with the median filtered data.
// Artificial edge effects due to image borders are eliminated.

   void compute_x_y_deriv_fields(
      double spatial_resolution,parallelogram* bbox_ptr,
      twoDarray const *ztwoDarray_ptr,
      twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
      double min_distance_to_border,double null_value)
      {
         unsigned int nx_size=filterfunc::gaussian_filter_size(
            spatial_resolution,ztwoDarray_ptr->get_deltax());
         unsigned int ny_size=filterfunc::gaussian_filter_size(
            spatial_resolution,ztwoDarray_ptr->get_deltay());

         double *xfilter=new double[nx_size];
         filterfunc::gaussian_filter(
            nx_size,1,spatial_resolution,ztwoDarray_ptr->get_deltax(),
            xfilter);
         imagefunc::horiz_derivative_filter(
            nx_size,xfilter,ztwoDarray_ptr,xderiv_twoDarray_ptr,null_value,
            ztwoDarray_ptr->get_deltax());
         delete xfilter;

         double *yfilter=new double[ny_size];
         filterfunc::gaussian_filter(
            ny_size,1,spatial_resolution,ztwoDarray_ptr->get_deltay(),
            yfilter);
         imagefunc::vert_derivative_filter(
            ny_size,yfilter,ztwoDarray_ptr,yderiv_twoDarray_ptr,null_value,
            ztwoDarray_ptr->get_deltay());
         delete yfilter;

// Try to eliminate artificial image boundary effects from gradient
// arrays:
         
         double xlo=ztwoDarray_ptr->get_xlo();
         double xhi=ztwoDarray_ptr->get_xhi();
         double ylo=ztwoDarray_ptr->get_ylo();
         double yhi=ztwoDarray_ptr->get_yhi();
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
                     xderiv_twoDarray_ptr->put(px,py,null_value);
                     yderiv_twoDarray_ptr->put(px,py,null_value);
                  }
               }
            } // loop over index py
         } // loop over index px

// Set derivatives at pixels located outside bounding box to null_value:

         drawfunc::null_pixels_outside_parallelogram(
            null_value,bbox_ptr,xderiv_twoDarray_ptr,yderiv_twoDarray_ptr);
      }

// ---------------------------------------------------------------------
// Member function threshold_gradient_phase_field takes in phase
// interval [phi_min,phi_max] as well as a gradient phase information
// within twoDarray *gradient_phase_twoDarray_ptr.  It copies those
// pixels within this twoDarray which lie inside the phase interval
// onto output twoDarray *phase_threshold_twoDarray_ptr.

   void threshold_gradient_phase_field(
      double phi_min,double phi_max,
      twoDarray const *gradient_phase_twoDarray_ptr,
      twoDarray* phase_threshold_twoDarray_ptr)
      {
         outputfunc::write_banner("Thresholding gradient phase field:");

// phi_min's value sets canonical 2*PI interval.  Adjust phi_max so
// that it lies within 2*PI of phi_min:

         phi_max=basic_math::phase_to_canonical_interval(
            phi_max,phi_min-PI,phi_min+PI);
   
         phase_threshold_twoDarray_ptr->initialize_values(
            xyzpfunc::null_value);
         for (unsigned int px=0; px<gradient_phase_twoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<gradient_phase_twoDarray_ptr->get_ndim(); py++)
            {
               double orig_phase=gradient_phase_twoDarray_ptr->get(px,py);
               double curr_phase=basic_math::phase_to_canonical_interval(
                  orig_phase,phi_min-PI,phi_min+PI);
               if (curr_phase > phi_min && curr_phase < phi_max)
               {
                  phase_threshold_twoDarray_ptr->put(px,py,orig_phase);
               }
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method binary_subtract takes in two binary images.  If the a
// pixel's intensity in the first image equals 1 and its counterpart
// intensity in the second image equals 0, the output pixel's
// intensity is set equal to 1.  Otherwise, the output pixel's value
// is set equal to 0.  

   twoDarray* binary_subtract(
      twoDarray const *zsurviving_connected_binary_twoDarray_ptr,
      twoDarray const *zbinary_twoDarray_ptr)
      {
         outputfunc::write_banner(
            "Binary subtracting heights & phase images");
         twoDarray* zbinary_diff_twoDarray_ptr=new twoDarray(
            zsurviving_connected_binary_twoDarray_ptr);
         zbinary_diff_twoDarray_ptr->initialize_values(
            xyzpfunc::null_value);

         for (unsigned int px=0; px<zsurviving_connected_binary_twoDarray_ptr->
                 get_mdim(); px++)
         {
            for (unsigned int py=0; py<zsurviving_connected_binary_twoDarray_ptr->
                    get_ndim(); py++)
            {
               double binary_height=zsurviving_connected_binary_twoDarray_ptr
                  ->get(px,py);
               double binary_phase=zbinary_twoDarray_ptr->get(px,py);
               if (binary_height > 0.5 && binary_phase < 0.5)
               {
                  zbinary_diff_twoDarray_ptr->put(px,py,1);
               }
            } // loop over py index
         } // loop over px index
         return zbinary_diff_twoDarray_ptr;
      }
   
// ==========================================================================
// Connected components methods:
// ==========================================================================

// Method connect_height_components takes in a cleaned height image
// within input twoDarray *ztwoDarray_ptr.  It first generates a run
// length encoded list of pixels for this image.  It then connects
// together the runs into continguous components whose projected areas
// on the ground must exceed min_projected_area.  Connected height
// component information is saved within the dynamically generated
// output hashtable *connected_heights_hashtable_ptr.  

   Hashtable<linkedlist*>* connect_height_components(
      double abs_zthreshold,double min_projected_area,
      twoDarray const *ztwoDarray_ptr)
      {
         outputfunc::write_banner("Connecting height components:");
         twoDarray* zbinary_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         binaryimagefunc::binary_threshold(
            abs_zthreshold,ztwoDarray_ptr,zbinary_twoDarray_ptr);

//   dynamic_colortable=true;
//   dynamic_colortable_minz=-0.1;
//   dynamic_colortable_maxz=1.1;
//   writeimage("bin_thresh",zbinary_twoDarray_ptr);
//   dynamic_colortable=false;

         linkedlist* run_list_ptr=
            connectfunc::run_length_encode(zbinary_twoDarray_ptr);
         int n_connected_runs=connectfunc::connect_runs(
            run_list_ptr,zbinary_twoDarray_ptr);
         delete zbinary_twoDarray_ptr;
         outputfunc::newline();
         cout << "n_connected_runs = " << n_connected_runs << endl;
         outputfunc::newline();

         const double dA=ztwoDarray_ptr->get_deltax()*
            ztwoDarray_ptr->get_deltay();	// bin area on ground
         const unsigned int min_component_pixels=
            basic_math::round(min_projected_area/dA);

//         connectfunc::delete_connected_hashtable(
//            connected_heights_hashtable_ptr);
         Hashtable<linkedlist*>* connected_heights_hashtable_ptr=
            connectfunc::generate_connected_hashtable(
               n_connected_runs,min_component_pixels,run_list_ptr,
               ztwoDarray_ptr);
         delete run_list_ptr;
         return connected_heights_hashtable_ptr;
      }

// ---------------------------------------------------------------------
// Method compute_hulls_surrounding_components takes in a hashtable
// containing either connected heights or connected phase component
// information.  It traverses through the connected components,
// computes the convex hulls surrounding each connected component and
// draws the convex hulls onto a copy of the original input image.
// The annotated image is returned by this member function.

   twoDarray* compute_hulls_surrounding_components(
      Hashtable<linkedlist*>* connected_hashtable_ptr,
      twoDarray *ztwoDarray_ptr)
      {
         outputfunc::write_banner(
            "Computing convex hulls surrounding components:");

         twoDarray* zhulls_twoDarray_ptr=new twoDarray(*ztwoDarray_ptr);

         polygon* convex_hull_ptr=NULL;
         for (unsigned int n=0; n<connected_hashtable_ptr->size(); n++)
         {
            linkedlist* currlist_ptr=connected_hashtable_ptr->retrieve_key(n)
               ->get_data();

            if (currlist_ptr != NULL)
            {
               convex_hull_ptr=convexhull::convex_hull_poly(currlist_ptr);
            }
//      cout << "convex_hull = " << *convex_hull_ptr << endl;
            convex_hull_ptr->initialize_edge_segments();

// Shrink convex hulls around pixel clusters in order to eliminate
// empty black space between hull border and cluster periphery:

//      const int n_hullpnts=2000;
//      const double ds=basic_math::min(ztwoDarray_ptr->get_deltax(),
//                          ztwoDarray_ptr->get_deltay());
//      const double max_init_search_distance=50;	// meters
//      const int max_iters=basic_math::round(max_init_search_distance/ds);
//      threevector hull_point,start_point;
//      threevector r_hat;

//      for (int n=0; n<n_hullpnts; n++)
//      {
//         double theta_frac=double(n)/double(n_hullpnts);
//         convex_hull_ptr->edge_point(theta_frac,hull_point);
//         convex_hull_ptr->radial_direction_vector(theta_frac,r_hat);

//         int px,py;
//         int iter=0;
//         double curr_intensity=0;
//         do
//         {
//            start_point=hull_point-iter*ds*r_hat;
//            ztwoDarray_ptr->point_to_pixel(start_point,px,py);
//            curr_intensity=ztwoDarray_ptr->get(px,py);
//            iter++;
//         }
//         while(iter < max_iters && curr_intensity < 0.5*null_value);

            double x_extent=ztwoDarray_ptr->get_xhi()
               -ztwoDarray_ptr->get_xlo();
            double y_extent=ztwoDarray_ptr->get_yhi()
               -ztwoDarray_ptr->get_ylo();
            double radius=0.002*sqrt(sqr(x_extent)+sqr(y_extent));
            drawfunc::draw_thick_polygon(
               *convex_hull_ptr,colorfunc::white,radius,zhulls_twoDarray_ptr);
            delete convex_hull_ptr;
         } // loop over index n labeling connected components

         return zhulls_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method connect_binary_components takes in a binary image and
// returns a new, dynamically generated binary image containing
// connected components whose area footprints exceed
// min_projected_area:

   twoDarray* connect_binary_components(
      double min_projected_area,twoDarray const *zbinary_twoDarray_ptr)
      {
         Hashtable<linkedlist*>* connected_components_hashtable_ptr=
            generate_connected_binary_components_hashtable(
               min_projected_area,zbinary_twoDarray_ptr);

// Generate new twoDarray containing surviving binary connected
// components:

         twoDarray* zconnected_twoDarray_ptr=new twoDarray(
            zbinary_twoDarray_ptr);
         connectfunc::decode_connected_hashtable(
            connected_components_hashtable_ptr,zconnected_twoDarray_ptr);
         connectfunc::delete_connected_hashtable(
            connected_components_hashtable_ptr);
         return zconnected_twoDarray_ptr;
      }
   
   Hashtable<linkedlist*>* generate_connected_binary_components_hashtable(
      double min_projected_area,twoDarray const *zbinary_twoDarray_ptr)
      {
         outputfunc::write_banner("Connecting binary components:");

         linkedlist* run_list_ptr=
            connectfunc::run_length_encode(zbinary_twoDarray_ptr);
         int n_connected_runs=connectfunc::connect_runs(
            run_list_ptr,zbinary_twoDarray_ptr);

         const double dA=zbinary_twoDarray_ptr->get_deltax()*
            zbinary_twoDarray_ptr->get_deltay();	// bin area on ground
         const unsigned int min_component_pixels=
            basic_math::round(min_projected_area/dA);

         cout << "min_component_pixels = " << min_component_pixels << endl;

         Hashtable<linkedlist*>* connected_components_hashtable_ptr=
            connectfunc::generate_connected_hashtable(
               n_connected_runs,min_component_pixels,run_list_ptr,
               zbinary_twoDarray_ptr);
         delete run_list_ptr;
         return connected_components_hashtable_ptr;
      }

// ---------------------------------------------------------------------
// Method retrieve_connected_pixel_list takes an integer index which
// labels the nth connected list of pixels within input hashtable
// *connected_hashtable_ptr.  It returns a pointer to the linkedlist
// for this connected set of pixels.

   linkedlist* retrieve_connected_pixel_list(
      int n,Hashtable<linkedlist*> const *connected_hashtable_ptr)
      {
         linkedlist* currlist_ptr=NULL;
         const Mynode<linkedlist*>* keynode_ptr=connected_hashtable_ptr->
            retrieve_key(n);
         if (keynode_ptr != NULL)
         {
            currlist_ptr=keynode_ptr->get_data();
         }
         return currlist_ptr;
      }

// ==========================================================================
// Intensity image cleaning methods:
// ==========================================================================

// Method set_pixel_intensity_values_to_sentinel takes in a pair of
// height and intensity images.  It scans through the intensity image
// and sets all pixels whose corresponding height values are greater
// [less] than input parameter z_threshold equal to p_sentinel
// depending upon boolean input ignore_tall_objects.  This little
// utility method is useful for filtering ground [building & tree]
// intensity pixels without interference from building & tree [ground]
// pixels.

   void set_pixel_intensity_values_to_sentinel(
      bool ignore_tall_objects,double z_threshold,double p_sentinel,
      twoDarray const *ztwoDarray_ptr,twoDarray* ptwoDarray_ptr)
      {
         for (unsigned int px=0; px<ptwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ptwoDarray_ptr->get_ndim(); py++)
            {
               double curr_z=ztwoDarray_ptr->get(px,py);
               if ((ignore_tall_objects && curr_z > z_threshold) ||
                   (!ignore_tall_objects && curr_z < z_threshold))
               {
                  ptwoDarray_ptr->put(px,py,p_sentinel);
               }
            } // py loop
         } // px loop
      }

// ---------------------------------------------------------------------
// Method probability_filter performs local intensity contrast
// amplification which results in high and low intensity cluster
// formation.

   void probability_filter(
      int nsize,
      twoDarray const *ptwoDarray_ptr,twoDarray const *pmedian_twoDarray_ptr,
      twoDarray *pfiltered_twoDarray_ptr,double irrelevant_intensity)
      {
         ptwoDarray_ptr->copy(pfiltered_twoDarray_ptr);
   
         if (is_even(nsize)) nsize++;
         double intensity[sqr(nsize)];

         int w=(nsize-1)/2;
         double filtered_intensity,frac;
         for (unsigned int px=w; px<ptwoDarray_ptr->get_mdim()-w; px++)
         {
            for (unsigned int py=w; py<ptwoDarray_ptr->get_ndim()-w; py++)
            {

// Calculate probability fraction to use on a pixel-by-pixel basis for
// p-image filtering based upon median intensities within input
// *pmedian_twoDarray_ptr:

               double curr_median=pmedian_twoDarray_ptr->get(px,py);
               if (curr_median < 0)
               {
                  filtered_intensity=0;
               }
               else
               {
                  const double median_lo=0.2;
                  const double median_hi=0.3;
//                  const double frac_lo=0.3;
                  const double frac_lo=0.2;
                  const double frac_hi=0.8;

                  if (curr_median < median_lo)
                  {
                     frac=frac_lo;
                  }
                  else if (curr_median > median_hi)
                  {
                     frac=frac_hi;
                  }
                  else
                  {
                     frac=frac_lo+(frac_hi-frac_lo)/(median_hi-median_lo)*
                        (curr_median-median_lo);
                  }

                  int n=0;
                  for (int i=0; i<nsize; i++)
                  {
                     for (int j=0; j<nsize; j++)
                     {
                        double curr_intensity=
                           ptwoDarray_ptr->get(px-w+i,py-w+j);
                        if (curr_intensity < irrelevant_intensity)
                        {
                           intensity[n++]=curr_intensity;
                        }
                     }
                  } 
                  Quicksort(intensity,n);
                  filtered_intensity=intensity[
                     basic_math::round((n-1)*frac)];
               } // curr_median < 0 conditional
               pfiltered_twoDarray_ptr->put(px,py,filtered_intensity);
            } // loop over index py
         } // loop over index px
      }

// ---------------------------------------------------------------------
// Method merge_hi_lo_intensity_images takes in 2 intensity images
// within input twoDarrays *p_hi_twoDarray_ptr and
// *p_lo_twoDarray_ptr.  It also takes in their common height image
// within twoDarray *ztwoDarray_ptr.  If a pixel's height exceeds
// input height threshold z_threshold, the output intensity is set
// equal to the value within *p_hi_twoDarray_ptr.  Otherwise, the
// intensity is set equal to the value within p_lo_twoDarray_ptr.  A
// dynamically generated twoDarray containing the merged intensity
// values is returned by this method.

   twoDarray* merge_hi_lo_intensity_images(
      double z_threshold,twoDarray const *ztwoDarray_ptr,
      twoDarray* p_hi_twoDarray_ptr,twoDarray* p_lo_twoDarray_ptr)
      {
         twoDarray* pmerge_twoDarray_ptr=new twoDarray(p_hi_twoDarray_ptr);
         for (unsigned int px=0; px<pmerge_twoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<pmerge_twoDarray_ptr->get_ndim(); py++)
            {
               if (ztwoDarray_ptr->get(px,py) > z_threshold)
               {
                  pmerge_twoDarray_ptr->put(
                     px,py,p_hi_twoDarray_ptr->get(px,py));
               }
               else
               {
                  pmerge_twoDarray_ptr->put(
                     px,py,p_lo_twoDarray_ptr->get(px,py));
               }
            } // py loop
         } // px loop
         return pmerge_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method increase_intensities_for_large_height_fluctuations takes in
// intensity twoDarray *ptwoDarray_ptr along with normalized height
// fluctuation image *norm_zfluc_twoDarray_ptr.  If a pixel's
// intensity exceeds input threshold p_threshold and its normalized
// fluctuation exceeds input threshold norm_zfluc_threshold, that
// pixel's intensity value is reset to p_new_min.  

   void increase_intensities_for_large_height_fluctuations(
      double p_threshold,double norm_zfluc_threshold,double p_new_min,
      twoDarray const *ptwoDarray_ptr,
      twoDarray const *norm_zfluc_twoDarray_ptr,twoDarray* pnew_twoDarray_ptr)
      {
         ptwoDarray_ptr->copy(pnew_twoDarray_ptr);
         for (unsigned int px=0; px<pnew_twoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<pnew_twoDarray_ptr->get_ndim(); py++)
            {
               double curr_p=ptwoDarray_ptr->get(px,py);
               if (curr_p > p_threshold)
               {
                  if (norm_zfluc_twoDarray_ptr->get(px,py) > 
                      norm_zfluc_threshold)
                  {
                     pnew_twoDarray_ptr->put(px,py,basic_math::max(
                        curr_p,p_new_min));
                  }
               }
            } // py loop
         } // px loop
      }

// ==========================================================================
// Drawing & coloring methods
// ==========================================================================

// Method draw_xy_coordinate_system constructs an xy grid and
// underlays it beneath the xyzp data within the binary output file
// specified by xyzp_filename.

   void draw_xy_coordinate_system(
      string xyzp_filename,double annotation_value,
      twoDarray const *ztwoDarray_ptr,double z_coord_system)
      {
//         const double delta_x=200;	// x cell length in meters
//         const double delta_y=200;	// y cell length in meters
         const double delta_x=100;	// x cell length in meters
         const double delta_y=100;	// y cell length in meters
         
         polygon* grid_poly_ptr=xy_data_bbox(ztwoDarray_ptr,delta_x,delta_y);
         grid_poly_ptr->translate(threevector(0,0,z_coord_system));

         string x_axis_label="METERS EAST";
         string y_axis_label="METERS NORTH";
//         string x_axis_label="METERS ALONG TRACK";
//         string y_axis_label="METERS CROSS TRACK";
         draw3Dfunc::ds=0.025;	// meter
         draw3Dfunc::draw_coordinate_system(
            *grid_poly_ptr,xyzp_filename,annotation_value,
            delta_x,delta_y,x_axis_label,y_axis_label);
         delete grid_poly_ptr;
      }

// ---------------------------------------------------------------------
// Method recolor_feature_heights_for_RGB_colormap takes in features
// map *ftwoDarray_ptr which is assumed to have already been filled
// with values appropriate for group 94/106 dataviewer display using
// our "Hue + values" colormap.  This method dynamically generates a
// new features map whose values are appropriate for dataviewer
// display using our "RGB" colormap.  We created this method in Feb 05
// for ALIRT features/color photo fusion purposes.

   twoDarray* recolor_feature_heights_for_RGB_colormap(
      twoDarray const *ftwoDarray_ptr)
      {
         twoDarray* gtwoDarray_ptr=new twoDarray(ftwoDarray_ptr);
         gtwoDarray_ptr->initialize_values(xyzpfunc::null_value);

         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double curr_f=ftwoDarray_ptr->get(px,py);
               colorfunc::RGB curr_RGB=colorfunc::dataviewer_colormap_to_RGB(
                  curr_f);
               double f_new=colorfunc::rgb_colormap_value(curr_RGB);
               gtwoDarray_ptr->put(px,py,f_new);
            } // loop over py index
         } // loop over px index
         return gtwoDarray_ptr;
      }

// ==========================================================================
// ALIRT tiling methods
// ==========================================================================

// Method AlirtTileLabelToLonLat() takes in an ALIRT tile label whose
// form is specified in Ross Anderson's "ALIRT Product Grid
// Specification" document dated March 2009.  It extracts longitude
// and latitude geocoordinates for tile's anchor point from the input
// hypenated tile label .

   void AlirtTileLabelToLonLat(string tile_label,double& lon,double& lat)
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               tile_label,"-");

         string s_horiz=substrings[0];
         string s_vert=substrings[1];
//         cout << "s_horiz = " << s_horiz << " s_vert = " << s_vert << endl;

         int i_horiz=mathfunc::decode_base32(s_horiz);
         int i_vert=mathfunc::decode_base32(s_vert);
//         cout << "i_horiz = " << i_horiz << " i_vert = " << i_vert << endl;
   
         const double M=15;
         const double N=15;

         lon=N/3600 * i_horiz-180;
         lat=M/3600 * i_vert-90;
      }

// --------------------------------------------------------------------- 
// Method LonLatToAlirtTileLabel() takes in lon,lat geocoordinates for
// an ALIRT tile's anchor point.  It returns the corresponding tile
// label specified in Ross Anderson's "ALIRT Product Grid
// Specification" dated March 2009.

   string LonLatToAlirtTileLabel(double& longitude,double& latitude)
      {
         const double M=15;
         const double N=15;

         int i_horiz=basic_math::mytruncate( (longitude+180) * 3600/N );
         int i_vert=basic_math::mytruncate( (latitude+90) * 3600/M );

         string s_horiz=mathfunc::encode_base32(i_horiz);
         string s_vert=mathfunc::encode_base32(i_vert);

// Add leading zeros to horizontal and vertical labels if their number
// of characters is less than 4:

         unsigned int n_horiz_chars=s_horiz.size();
         unsigned int n_vert_chars=s_vert.size();
         for (unsigned int j=0; j<4-n_horiz_chars; j++)
         {
            s_horiz = "0"+s_horiz;
         }
         for (unsigned int j=0; j<4-n_vert_chars; j++)
         {
            s_vert = "0"+s_vert;
         }

         string tile_label=s_horiz+"-"+s_vert;
         return tile_label;
      }

// --------------------------------------------------------------------- 
// Method AlirtTilesInBbox() takes in the lower left and upper right
// geocoordinates for some bounding box on the earth.  It computes and
// the labels for all ALIRT tiles which cover the input bbox.  The
// labels are returned as an STL vector of strings.

   vector<string> AlirtTilesInBbox(
      const geopoint& lower_left,const geopoint& upper_right)
   {
      double d_lon=10.0/3600.0;	// 10 arc seconds
      double d_lat=10.0/3600.0;	// 10 arc seconds

      double lon_extent=upper_right.get_longitude()-lower_left.get_longitude();
      double lat_extent=upper_right.get_latitude()-lower_left.get_latitude();
      int n_lons=lon_extent/d_lon+1;
      int n_lats=lat_extent/d_lat+1;

      typedef map<string,int> TILE_LABELS_MAP;
      TILE_LABELS_MAP tile_labels_map;
      TILE_LABELS_MAP::iterator iter;

      int tile_counter=0;
      for (int i=0; i<n_lons; i++)
      {
         double curr_lon=lower_left.get_longitude()+i*d_lon;
         for (int j=0; j<n_lats; j++)
         {
            double curr_lat=lower_left.get_latitude()+j*d_lat;
            string curr_tile_label=ladarfunc::LonLatToAlirtTileLabel(
               curr_lon,curr_lat);
            iter=tile_labels_map.find(curr_tile_label);
            if (iter==tile_labels_map.end()) 
            {
               tile_labels_map[curr_tile_label]=tile_counter++;
            }
         } // loop over index j labeling latitude steps
      } // loop over index i labeling longitude steps

      vector<string> tile_labels;
      for (iter=tile_labels_map.begin(); iter != tile_labels_map.end(); iter++)
      {
         tile_labels.push_back(iter->first);
//         cout << "tile label = " << tile_labels.back() << endl;
      }

      return tile_labels;
   }
   

} // ladarfunc namespace






