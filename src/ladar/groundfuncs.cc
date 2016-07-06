// ==========================================================================
// GROUNDFUNCS stand-alone methods
// ==========================================================================
// Last modified on 11/1/07; 11/5/07; 11/11/07; 12/4/10
// ==========================================================================

#include "image/binaryimagefuncs.h"
#include "image/drawfuncs.h"
#include "ladar/featurefuncs.h"
#include "filter/filterfuncs.h"
#include "image/graphicsfuncs.h"
#include "ladar/groundfuncs.h"
#include "image/imagefuncs.h"
#include "ladar/ladarimage.h"
#include "geometry/linesegment.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "math/prob_distribution.h"
#include "image/recursivefuncs.h"
#include "geometry/regular_polygon.h"
#include "image/TwoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

namespace groundfunc
{

// High level method extract_ground returns a dynamically generated
// twoDarray containing a height map for the ground surface
// corresponding to input height image *ztwoDarray_ptr.

   twoDarray* extract_ground(
      double xextent,double yextent,double correlation_distance,
      parallelogram const *data_bbox_ptr,double local_threshold_frac,
      twoDarray const *ztwoDarray_ptr)
      {
         outputfunc::write_banner("Extracting ground:");

         vector<double> threshold_intensity;
         vector<twovector> centers_posn;
         generate_threshold_centers( 
            xextent,yextent,correlation_distance,
            local_threshold_frac,ztwoDarray_ptr,
            threshold_intensity,centers_posn);

         for (unsigned int i=0; i<centers_posn.size(); i++)
         {
            cout << "i = " << i << " threshold_intensity = "
                 << threshold_intensity[i] << endl;
         }

         twoDarray* zground_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         generate_threshold_field( 
            correlation_distance,data_bbox_ptr,
            threshold_intensity,centers_posn,
            ztwoDarray_ptr,zground_twoDarray_ptr);
         return zground_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method generate_threshold_centers superposes a lattice onto input
// twoDarray *ztwoDarray_ptr whose spacing is set by input parameter
// correlation_distance.  If the local fraction of non-null
// intensities in the neighborhood of some lattice site exceeds some
// minimal cutoff, this method computes the intensity distribution
// within the lattice cell.  The lattice cell's location is taken as a
// threshold center, and the local threshold at the site is determined
// from the local intensity distribution via input parameter
// local_threshold_frac.

   void generate_threshold_centers( 
      double xextent,double yextent,double correlation_distance,
      double local_threshold_frac,twoDarray const *ztwoDarray_ptr,
      vector<double>& threshold_intensity,vector<twovector>& centers_posn)
      { 
         int mbins=basic_math::round(xextent/correlation_distance);
         int nbins=basic_math::round(yextent/correlation_distance);

         double zground_min=0;
         double minimal_fill_frac=0.33;
         generate_threshold_centers( 
            mbins,nbins,correlation_distance,
            local_threshold_frac,zground_min,minimal_fill_frac,
            ztwoDarray_ptr,threshold_intensity,centers_posn);
      }

   void generate_threshold_centers( 
      int mbins,int nbins,double correlation_distance,
      double local_threshold_frac,
      double zground_min,double minimal_fill_frac,
      twoDarray const *ztwoDarray_ptr,
      vector<double>& threshold_intensity,vector<twovector>& centers_posn)
      { 
         outputfunc::write_banner("Generating threshold centers:");
   
         double dx=(ztwoDarray_ptr->get_xhi()-
                    ztwoDarray_ptr->get_xlo())/(mbins-1);
         double dy=(ztwoDarray_ptr->get_yhi()-
                    ztwoDarray_ptr->get_ylo())/(nbins-1);

         cout << "correlation_distance = " << correlation_distance << endl;
         cout << "mbins = " << mbins << " nbins = " << nbins 
              << " dx = " << dx << " dy = " << dy << endl;
   
         prob_distribution prob;

         threshold_intensity.clear();
         centers_posn.clear();

         cout << "Computing threshold centers:" << endl;
         for (int m=0; m<mbins; m++)
         {
            double xcenter=ztwoDarray_ptr->get_xlo()+m*dx;
            double minimum_x=xcenter-correlation_distance;
            double maximum_x=xcenter+correlation_distance;

            for (int n=0; n<nbins; n++)
            {
               double ycenter=ztwoDarray_ptr->get_ylo()+n*dy;
               double minimum_y=ycenter-correlation_distance;
               double maximum_y=ycenter+correlation_distance;

               int npixels_inside_bbox;
               int nhot_pixels=imagefunc::count_pixels_above_zmin_inside_bbox(
                  minimum_x,minimum_y,maximum_x,maximum_y,
                  0.5*xyzpfunc::null_value,
                  ztwoDarray_ptr,npixels_inside_bbox);
               double fill_frac=double(nhot_pixels)/
                 double(npixels_inside_bbox);
               //               cout << "m = " << m << " n = " << n
               //     << " fill frac = " << fill_frac << endl;
               if (fill_frac > minimal_fill_frac && 
                   imagefunc::intensity_distribution_inside_bbox(
                      minimum_x,minimum_y,maximum_x,maximum_y,
                      ztwoDarray_ptr,zground_min,prob))
               {
//                  centers_posn.push_back(twovector(xcenter,ycenter));

                  twovector COM;
                  if (imagefunc::binary_COM_above_zmin(
                     minimum_x,maximum_x,minimum_y,maximum_y,
                     0.5*xyzpfunc::null_value,ztwoDarray_ptr,COM))
                  {
                     centers_posn.push_back(COM);
                     
                     threshold_intensity.push_back(
                        prob.find_x_corresponding_to_pcum(
                           local_threshold_frac));

                     //                     cout << "COM.x = " << COM.get(0) 
                     //     << " COM.y = " << COM.get(1)
                     //     << " threshold_intensity = "
                     //     << threshold_intensity.back() << endl;
                     
                     if (threshold_intensity.back() < -100)
                     {
                        cout << "Trouble in groundfunc::generate_threshold_centers!"
                             << endl;
                        cout << "ncenters = " << centers_posn.size()
                             << " threshold_intensity = " 
                             << threshold_intensity.back() << endl;
                     }
                  }
               } // fill_frac > 0.5 conditional
            } // loop over index n
         } // loop over index m
      }

// ---------------------------------------------------------------------
// Method generate_threshold_field takes in STL vectors
// threshold_intensity and centers_posn containing sampled intensity
// values from *ztwoDarray_ptr.  This method generates and returns a
// local threshold field based upon the intensity information
// collected at all the center positions.

   void generate_threshold_field( 
      double correlation_distance,parallelogram const *data_bbox_ptr,
      const vector<double>& threshold_intensity,
      const vector<twovector>& centers_posn,
      twoDarray const *ztwoDarray_ptr,twoDarray* zthreshold_twoDarray_ptr)
      { 
         outputfunc::write_banner("Generating threshold field:");

         cout << "ncenters = " << centers_posn.size() << endl;
         double sqr_correlation_length=sqr(correlation_distance);
         unsigned int mdim=zthreshold_twoDarray_ptr->get_mdim();
         unsigned int ndim=zthreshold_twoDarray_ptr->get_ndim();
         double curr_x,curr_y,curr_intensity;
         //         vector<pair<double,int> > rsq;
         for (unsigned int px=0; px<mdim; px++)
         {
            if (px%200==0) cout << "px = " << px << " of mdim = " << mdim 
                                << endl;
//      cout << "px = " << px << " mdim = "
//           << zthreshold_twoDarray_ptr->get_mdim() << endl;
            curr_x=zthreshold_twoDarray_ptr->fast_px_to_x(px);
            for (unsigned int py=0; py<ndim; py++)
            {
              curr_y=zthreshold_twoDarray_ptr->fast_py_to_y(py);

// FAKE FAKE:  Expt with ignoring data bbox...Fri Oct 26 at 1:10 pm...

//               if (data_bbox_ptr->point_inside(currposn))
               {
//                  if (zthreshold_twoDarray_ptr->get(px,py) > 
//                      xyzpfunc::null_value)
                  {
//                     double curr_intensity=imagefunc::threshold_field(
//                        true,ncenters,currposn,centers_posn,
//                        0.5*correlation_distance,threshold_intensity);
                    curr_intensity=imagefunc::fast_threshold_field(
                        sqr_correlation_length,curr_x,curr_y,
                        centers_posn,threshold_intensity);

//                     double curr_intensity=imagefunc::fast_threshold_field(
//                        py,sqr_correlation_length,&rsq,
//                        curr_x,curr_y,centers_posn,threshold_intensity);
                     zthreshold_twoDarray_ptr->put(px,py,curr_intensity);
                  }
               }
            } // loop over index py
         } // loop over index px
//         cout << "After loop call to imagefunc::threshold_field()" << endl;
      }

// ---------------------------------------------------------------------
// Method erode_strong_gradient_regions takes in a height image along
// with its gradient field.  It scans through the image and searches
// for points where the gradient magnitude exceeds some input critical
// threshold.  At such points, the method constructs a parallelogram
// around a test point located some radial distance delta_r outwards
// from the strong gradient field point along the direction specified
// by the gradient direction vector.  If no strong gradients exist
// within the parallelogram, this method replaces height values
// located along a radial line segment parallel to the gradient with
// the average height inside the parallelogram.

// We fondly refer to this erosion algorithm as "Mario bashing" :)

   bool erode_strong_gradient_regions(
      double grad_magnitude_threshold,
      twoDarray* ztwoDarray_ptr,twoDarray const *grad_mag_twoDarray_ptr,
      twoDarray const *grad_phase_twoDarray_ptr)
      {
         outputfunc::write_banner(
            "Eroding strong gradient regions in z-image:");

         const double delta_r=1;	// meters
//   const double delta_r=3;	// meters
//   const double delta_r=5;	// meters

         unsigned int min_px,min_py,max_px,max_py;
         threevector currpnt,point;
         vector<threevector> vertex(4);

         const unsigned int mdim=ztwoDarray_ptr->get_mdim();
         const unsigned int ndim=ztwoDarray_ptr->get_ndim();
         
         bool strong_gradient_region_found=false;
         for (unsigned int px=0; px<mdim; px++)
         {
            for (unsigned int py=0; py<ndim; py++)
            {
               int n_eff=px*ndim+py;
               double grad_mag=grad_mag_twoDarray_ptr->get(n_eff);
               if (grad_mag > grad_magnitude_threshold)
               {
                  strong_gradient_region_found=true;
                  ztwoDarray_ptr->pixel_to_point(px,py,currpnt);
                  double grad_phase=grad_phase_twoDarray_ptr->get(n_eff);

// Direction vector uhat points radially inwards toward increasing
// height values:

                  double cosine=cos(grad_phase);
                  double sine=sin(grad_phase);
                  threevector uhat(cosine,sine,0);  
                  threevector vhat(-sine,cosine,0);

                  threevector test_point=currpnt-delta_r*uhat;
                  vertex[0]=test_point+0.5*delta_r*uhat;
                  vertex[1]=test_point+0.5*delta_r*vhat;
                  vertex[2]=test_point-0.5*delta_r*uhat;
                  vertex[3]=test_point-0.5*delta_r*vhat;
                  parallelogram bbox(vertex);
                  ztwoDarray_ptr->locate_extremal_xy_pixels(
                     bbox,min_px,min_py,max_px,max_py);

// Loop over points inside candidate ground parallelogram.  Reject
// candidate if gradient magnitude is strong anywhere inside the
// parallelogram:

                  bool no_strong_gradients_in_bbox=true;
                  int npixels_in_bbox=0;
                  double height_sum=0;

                  unsigned int i=min_px;
                  while (i<=max_px && no_strong_gradients_in_bbox)
                  {
                     unsigned int j=min_py;
                     while (j<=max_py && no_strong_gradients_in_bbox)
                     {
                        ztwoDarray_ptr->pixel_to_point(i,j,point);
                        if (bbox.point_inside(point))
                        {
                           if (grad_mag_twoDarray_ptr->get(i,j) > 
                               grad_magnitude_threshold) 
                           {
                              no_strong_gradients_in_bbox=false;
                           }
                           else
                           {
                              double curr_z=ztwoDarray_ptr->get(n_eff);
                              if (curr_z > xyzpfunc::null_value)
                              {
                                 height_sum += curr_z;
                                 npixels_in_bbox++;
                              }
                           }
                        }
                        j++;
                     } // while loop over index j
                     i++;
                  } // while loop over index i
            
                  if (no_strong_gradients_in_bbox && npixels_in_bbox > 2)
                  {
//                     const double radial_dist=1;	// meter
                     const double radial_dist=2;	// meter
                     double avg_height_in_bbox=height_sum/npixels_in_bbox;

                     linesegment radial_segment(
                        currpnt,currpnt+radial_dist*uhat);
                     drawfunc::draw_line(
                        radial_segment,avg_height_in_bbox,ztwoDarray_ptr,
                        false,true);

/*
// Construct small rectangle which runs from current point radially
// inwards with a relatively small transverse extent:

                     const double transverse_dist=0.5*radial_dist; // meter
                     vertex[0]=currpnt+0.5*transverse_dist*vhat;
                     vertex[1]=vertex[0]+radial_dist*uhat;
                     vertex[2]=vertex[1]-transverse_dist*vhat;
                     vertex[3]=vertex[2]-radial_dist*uhat;
                     parallelogram rectangle(vertex);
                     drawfunc::color_convex_quadrilateral_interior(
                        rectangle,avg_height_in_bbox,ztwoDarray_ptr);
*/

                  }
               } // grad_mag > grad_magnitude_threshold conditional
            } // loop over py index
         } // loop over px index
         return strong_gradient_region_found;
      }
   
// ==========================================================================
// Oozing methods
// ==========================================================================

// Method identify_ground_seeds_pixels takes in a raw height image as
// well as a groundmask twoDarray.  It scans through the entire image
// and classifies "obvious relatively low" pixels.  The number of
// identified ground seed pixels is required to exceed a reasonable
// lower bound.

   void identify_ground_seed_pixels(
      twoDarray const *ztwoDarray_ptr,twoDarray* ground_seeds_twoDarray_ptr)
      {
         string banner="Identifying ground seed pixels";
         outputfunc::write_banner(banner);

         const unsigned int mdim=ztwoDarray_ptr->get_mdim();
         const unsigned int ndim=ztwoDarray_ptr->get_ndim();

         double intensity_floor=0.5*NEGATIVEINFINITY;
         double zmin=imagefunc::min_intensity_above_floor(
            intensity_floor,ztwoDarray_ptr);
         cout << "zmin = " << zmin << endl;
         
         prob_distribution prob;
         int n_output_bins=100;
         imagefunc::image_intensity_distribution(
            zmin,ztwoDarray_ptr,prob,n_output_bins);

         double ground_prob=0.005;
//         double ground_prob=0.01;
//         double ground_prob=0.025;
//         double ground_prob=0.05;
         const double max_ground_prob=0.50;

// Scan through entire image and classify "obviously" low pixels.
// These act as seeds for subsequent gradient tests:

         int n_seeds=0;
         int min_n_seeds=basic_math::min(5000,int(0.1*mdim*ndim));
         
         while (n_seeds < min_n_seeds && ground_prob < max_ground_prob)
         {
            double zlow_threshold=prob.find_x_corresponding_to_pcum(
               ground_prob);
            cout << "ground_prob = " << ground_prob
                 << " zlow_threshold = " << zlow_threshold << endl;

            ground_seeds_twoDarray_ptr->initialize_values(
               xyzpfunc::null_value);
            for (unsigned int px=0; px<mdim; px++)
            {
               for (unsigned int py=0; py<ndim; py++)
               {
                  if (ztwoDarray_ptr->get(px,py) <= zlow_threshold &&
                      ztwoDarray_ptr->get(px,py) > xyzpfunc::null_value)
                  {
                     ground_seeds_twoDarray_ptr->put(px,py,0);
//                  cout << "seed found at px = " << px
//                       << " py = " << py << endl;
                     n_seeds++;
                  }
               }
            }
            cout << "N seeds pixels = " << n_seeds << endl;
            ground_prob *= 2.0;
         } // n_seeds < min_n_seeds while loop
      }

// ---------------------------------------------------------------------
// Method find_low_local_pixels takes in a z-image as well as a
// groundmask containing seed pixels previously identified as ground.
// It scans over every unclassified pixel within the height image.  If
// it is adjacent to some pixel which has been classified as
// relatively low, this methods forms a crude height derivative
// between that neighbor and the current unclassified pixel.  If the
// derivative's magnitude is sufficiently small, it declares the
// current pixel to also be relatively low.  In this fashion,
// "relatively low" classification oozes throughout the entire image.

// This method returns a dynamically generated twoDarray which
// contains binary values indicating relatively low pixel locations
// within the input z-image.  We have intentionally tried to optimize
// this method for execution speed.

   void find_low_local_pixels(
      twoDarray const *ztwoDarray_ptr,twoDarray* groundmask_twoDarray_ptr,
      double max_gradient_magnitude_lo,int n_recursion_iters,
      double nonground_sentinel_value)
      {
         string banner="Finding low local pixels:";
         outputfunc::write_banner(banner);
//         cout << "inside groundfunc::find_low_local_pixels()" << endl;

// Scan over every unclassified pixel within the height image.  If it
// is adjacent to some pixel which has been classified as relatively
// low, form crude height derivative between that neighbor and the
// current unclassified pixel.  If the derivative's magnitude is
// sufficiently small, declare current pixel to also be relatively
// low.  In this fashion, "relatively low" classification oozes
// throughout the entire image.
   
         int nchanges,iter=0;

// Initialize entries within delta_s array:

         vector<double> delta_s=graphicsfunc::compute_delta_s_values(
            ztwoDarray_ptr->get_deltax(),ztwoDarray_ptr->get_deltay());

         vector<pair<int,int> >* pixel_posns_ptr=
            new vector<pair<int,int> >;
         vector<pair<int,int> >*  new_pixel_posns_ptr=
            new vector<pair<int,int> >;

         const unsigned int mdim=ztwoDarray_ptr->get_mdim();
         const unsigned int ndim=ztwoDarray_ptr->get_ndim();
         for (unsigned int px=0; px<mdim; px++)
         {
            for (unsigned int py=0; py<ndim; py++)
            {
               pixel_posns_ptr->push_back(pair<int,int>(px,py));
            }
         }

         do
         {
            nchanges=0;
            new_pixel_posns_ptr->clear();

            for (unsigned int pixel_counter=0; pixel_counter<
                    pixel_posns_ptr->size(); pixel_counter++)
            {
               unsigned int px=pixel_posns_ptr->at(pixel_counter).first;
               unsigned int py=pixel_posns_ptr->at(pixel_counter).second;
               local_ground_ooze(
                  px,py,ztwoDarray_ptr,groundmask_twoDarray_ptr,
                  max_gradient_magnitude_lo,delta_s,nchanges,
                  new_pixel_posns_ptr);
            } // loop over pixel counter index
            if (iter%1000==0)
            {
               cout << "iter = " << iter << " nchanges = " << nchanges 
                    << " new_pixel_posns.size() = " 
                    << new_pixel_posns_ptr->size()
                    << endl;
            }

            pixel_posns_ptr->clear();
            for (unsigned int pixel_counter=0; pixel_counter<
                    new_pixel_posns_ptr->size(); pixel_counter++)
            {
               pixel_posns_ptr->push_back(
                  new_pixel_posns_ptr->at(pixel_counter));
            }
            iter++;
         }

// FAKE FAKE:  for alg testing only...Fri Oct 26 at 9:30 pm...

//         while (nchanges > 0 && iter < 5000);
         while (nchanges > 0);

         delete pixel_posns_ptr;
         delete new_pixel_posns_ptr;

// At this point, we assume that all remaining null-valued entries
// within *groundmask_twoDarray_ptr which do NOT correspond to
// null-valued entries within *ztwoDarray_ptr correspond to local
// relative high points:

         for (unsigned int px=0; px<mdim; px++)
         {
            for (unsigned int py=0; py<ndim; py++)
            {
               int n_eff=ndim*px+py;
               if (groundmask_twoDarray_ptr->get(n_eff)==
                   xyzpfunc::null_value &&
                   ztwoDarray_ptr->get(n_eff) > xyzpfunc::null_value)
               {
                  groundmask_twoDarray_ptr->put(n_eff,1);
               }
            }
         }

// First recursively empty small ground islands surrounded by oceans
// of non-ground:

         cout << "Eliminating small ground islands:" << endl;
         binaryimagefunc::binary_reverse(groundmask_twoDarray_ptr);
         recursivefunc::recursive_empty(
            n_recursion_iters,groundmask_twoDarray_ptr,false);
         
// Next recursively empty small islands of non-ground surrounded by
// oceans of ground:

         cout << "Eliminating small non-ground islands:" << endl;
         binaryimagefunc::binary_reverse(groundmask_twoDarray_ptr);
         recursivefunc::recursive_empty(
            n_recursion_iters,groundmask_twoDarray_ptr,false);

// Re-null entries within *groundmask_twoDarray_ptr which correspond
// to null entries within *ztwoDarray_ptr:

         imagefunc::reset_values_using_another_image(
            ztwoDarray_ptr,groundmask_twoDarray_ptr,
            xyzpfunc::null_value,xyzpfunc::null_value);

// Reset final sentinel values used to mark non-ground pixels from
// xyzpfunc::null_value to nonground_sentinel_value:

         twoDarray* tmp_twoDarray_ptr=new twoDarray(groundmask_twoDarray_ptr);
         groundmask_twoDarray_ptr->copy(tmp_twoDarray_ptr);

         imagefunc::reset_values_using_another_image(
            groundmask_twoDarray_ptr,tmp_twoDarray_ptr,
            1.0,nonground_sentinel_value);

         tmp_twoDarray_ptr->copy(groundmask_twoDarray_ptr);
         delete tmp_twoDarray_ptr;
      }
   
// ---------------------------------------------------------------------
// Method local_ground_ooze takes in pixel coordinates (px,py) within
// input height array *ztwoDarray_ptr.  If the ground/non-ground
// classification of this pixel is unknown, this method searches for
// nearest neighbor pixels which are classified as ground.  It next
// forms simple height derivatives between those neighbors and
// (px,py).  If any height derivative's magnitude is less than
// max_gradient_magnitude_lo, (px,py) is classified as ground and its
// pixel coordinates are appended to *new_pixel_posns_ptr.

   void local_ground_ooze(
      unsigned int px,unsigned int py,twoDarray const *ztwoDarray_ptr,
      twoDarray* zhilo_twoDarray_ptr,
      double max_gradient_magnitude_lo,
      const vector<double>& delta_s,int& nchanges,
      vector<pair<int,int> >* new_pixel_posns_ptr)
      {
         int n_eff=ztwoDarray_ptr->get_ndim()*px+py;
         double curr_z=ztwoDarray_ptr->get(n_eff);
         if (curr_z > xyzpfunc::null_value &&
             zhilo_twoDarray_ptr->get(n_eff)==xyzpfunc::null_value)
         {
            int ilo=-1;
            int ihi=1;
            if (px==0) ilo=0;
            if (px==ztwoDarray_ptr->get_mdim()-1) ihi=0;

            int jlo=-1;
            int jhi=1;
            if (py==0) jlo=0;
            if (py==ztwoDarray_ptr->get_ndim()-1) jhi=0;
                        
            int counter=0;
            for (int i=ilo; i<=ihi; i++)
            {
               for (int j=jlo; j<=jhi; j++)
               {
                  int n2_eff=ztwoDarray_ptr->get_ndim()*(px+i)+(py+j);

                  if (zhilo_twoDarray_ptr->get(n2_eff)==0)
                  {
                     double abs_z_deriv=
                        fabs(ztwoDarray_ptr->get(n2_eff)-curr_z)/
                        delta_s[counter];
                     if (abs_z_deriv < max_gradient_magnitude_lo)
                     {
                        zhilo_twoDarray_ptr->put(n_eff,0);
                              
                        for (int inew=ilo; inew<=ihi; inew++)
                        {
                           int pxnew=px+inew;
                           for (int jnew=jlo; jnew<=jhi; jnew++)
                           {
                              int pynew=py+jnew;
                              new_pixel_posns_ptr->push_back(
                                 pair<int,int>(pxnew,pynew));
                           }
                        }
                        nchanges++;
                        break;
                     }
                  }

                  counter++;
               } // loop over j index
            } // loop over i index
         } // z(px,py)==null conditional
      }
   
// ---------------------------------------------------------------------
   twoDarray* find_low_local_pixels(
      const vector<threevector>& groundpoint_XYZ,
      twoDarray const *ztwoDarray_ptr,double max_gradient_magnitude_lo,
      int n_recursion_iters,parallelogram* data_bbox_ptr)
      {
         outputfunc::write_banner("Finding low local pixels:");

         const unsigned int mdim=ztwoDarray_ptr->get_mdim();
         const unsigned int ndim=ztwoDarray_ptr->get_ndim();
         const double dx=ztwoDarray_ptr->get_deltax();
         const double dy=ztwoDarray_ptr->get_deltay();
   
// First mark manually selected groundpoints within binary mask
// *zhilo_twoDarray_ptr:

         twoDarray* zhilo_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);   
         zhilo_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
         unsigned int px,py;
         for (unsigned int n=0; n<groundpoint_XYZ.size(); n++)
         {
            ztwoDarray_ptr->point_to_pixel(groundpoint_XYZ[n],px,py);
            zhilo_twoDarray_ptr->put(px,py,0);
         }

// Next initialize entries within delta_s array:

         vector<double> delta_s=graphicsfunc::compute_delta_s_values(dx,dy);

// Scan over every unclassified pixel within the height image.  If it
// is adjacent to some pixel which has been classified as relatively
// low, form crude height derivative between that neighbor and the
// current unclassified pixel.  If the derivative's magnitude is
// sufficiently small, declare current pixel to also be relatively
// low.  In this fashion, "relatively low" classification oozes
// throughout the entire image.
   
         int nchanges,iter=0;
         int max_iters=300;
         threevector currpoint;
         do
         {
            nchanges=0;
            for (unsigned int px=0; px<mdim; px++)
            {
               if (px%100==0) cout << px << " " << flush;
               for (unsigned int py=0; py<ndim; py++)
               {
                  int n_eff=ndim*px+py;
                  double curr_z=ztwoDarray_ptr->get(n_eff);
                  bool pixel_lies_inside_bbox_flag=true;

                  if (data_bbox_ptr != NULL)
                  {
                     ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
                     if (!data_bbox_ptr->point_inside(currpoint))
                     {
                        pixel_lies_inside_bbox_flag=false;
                     }
                  }

                  if (pixel_lies_inside_bbox_flag &&
                      curr_z > xyzpfunc::null_value &&
                      zhilo_twoDarray_ptr->get(n_eff)==xyzpfunc::null_value)
                  {
                     int counter=0;

                     int ilo=-1;
                     int ihi=1;
                     if (px==0) ilo=0;
                     if (px==mdim-1) ihi=0;

                     for (int i=ilo; i<=ihi; i++)
//                     for (int i=-1; i<=1; i++)
                     {

                        int jlo=-1;
                        int jhi=1;
                        if (py==0) jlo=0;
                        if (py==ndim-1) jhi=0;
                        
                        for (int j=jlo; j<=jhi; j++)
                        {
                           int n2_eff=ndim*(px+i)+(py+j);
                           if (zhilo_twoDarray_ptr->get(n2_eff)==0)
                           {
                              double abs_z_deriv=
                                 fabs(ztwoDarray_ptr->get(n2_eff)-curr_z)/
                                 delta_s[counter];
                              if (abs_z_deriv < max_gradient_magnitude_lo)
                              {
                                 zhilo_twoDarray_ptr->put(n_eff,0);
                                 nchanges++;
                                 break;
                              }
                           }
                           counter++;
                        } // loop over j index
                     } // loop over i index
                     
                  } // pixel_lies_inside_bbox && z(px,py)==null conditional
               } // loop over py index
            } // loop over px index
            outputfunc::newline();
//            if (iter%10==0)
            {
               cout << "iter = " << iter << " nchanges = " << nchanges 
                    << endl;
            }
            iter++;
         }
         while (nchanges > 0 && iter < max_iters);

// At this point, we assume that all remaining null-valued entries
// within the z_hilo matrix which do NOT correspond to null-valued
// entries within *ztwoDarray_ptr correspond to local relative high
// points:

         int n_nonground_pixels=0;
         for (unsigned int px=0; px<mdim; px++)
         {
            for (unsigned int py=0; py<ndim; py++)
            {
               int n_eff=ndim*px+py;
               if (zhilo_twoDarray_ptr->get(n_eff)==xyzpfunc::null_value &&
                   ztwoDarray_ptr->get(n_eff) > xyzpfunc::null_value)
               {
                  zhilo_twoDarray_ptr->put(n_eff,1);
                  n_nonground_pixels++;
               }
            }
         }

// Recursively empty z_hilo matrix to eliminate small noise islands:

         cout << "Recursively emptying *zhilo_twoDarray_ptr:" << endl;
         recursivefunc::recursive_empty(
            n_recursion_iters,zhilo_twoDarray_ptr,false);

/*   

// Re-null entries within z_hilo matrix which correspond to null
// entries within *ztwoDarray_ptr:

         imagefunc::threshold_intensities_using_another_image(
            zhilo_twoDarray_ptr,ztwoDarray_ptr,xyzpfunc::null_value);
*/

         cout << "Total number of pixels in *zhilo_twoDarray_ptr = "
              << zhilo_twoDarray_ptr->get_dimproduct() << endl;
         cout << "Number of ground pixels = "
              << zhilo_twoDarray_ptr->get_dimproduct()-n_nonground_pixels
              << endl;
         cout << "Number of non-ground pixels = "
              << n_nonground_pixels << endl;
         cout << "Fraction of non-ground pixels = "
              << double(n_nonground_pixels)/
            double(zhilo_twoDarray_ptr->get_dimproduct()-n_nonground_pixels)
              << endl;
         return zhilo_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// This overloaded version of method find_low_local_pixels applies the
// ground "oozing" procedure to just pixels classified as asphalt
// within input feature map *ftwoDarray_ptr.

   twoDarray* find_low_local_pixels(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ftwoDarray_ptr)
      {
         const unsigned int mdim=ztwoDarray_ptr->get_mdim();
         const unsigned int ndim=ztwoDarray_ptr->get_ndim();
         const double zlow_threshold=0.1;

// First scan through entire image and classify "obviously" low
// asphalt pixels.  These act as seeds for subsequent gradient tests:

         twoDarray* zlo_asphalt_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);  
         zlo_asphalt_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
         for (unsigned int px=0; px<mdim; px++)
         {
            for (unsigned int py=0; py<ndim; py++)
            {
               double curr_z=ztwoDarray_ptr->get(px,py);
               if (curr_z <= zlow_threshold && curr_z > xyzpfunc::null_value
                   && nearly_equal(ftwoDarray_ptr->get(px,py),
                                   featurefunc::road_sentinel_value))
               {
                  zlo_asphalt_twoDarray_ptr->put(px,py,curr_z);
               }
            }
         }

// Initialize entries within delta_s array:

         const double dx=ztwoDarray_ptr->get_deltax();
         const double dy=ztwoDarray_ptr->get_deltay();
//         double delta_s[9];
//         graphicsfunc::compute_delta_s_values(dx,dy,delta_s);
         vector<double> delta_s=graphicsfunc::compute_delta_s_values(dx,dy);

// Scan over every asphalt pixel within the height image.  If it is
// adjacent to some asphalt pixel which has been classified as
// relatively low, form crude height derivative between that neighbor
// and the current unclassified pixel.  If the derivative's magnitude
// is sufficiently small, declare current asphalt pixel to also be
// relatively low.  In this fashion, "relatively low" classification
// for asphalt oozes throughout the entire image.
   
         int nchanges,iter=0;
         const double max_gradient_magnitude_lo=0.25;
         do
         {
            nchanges=0;
            for (unsigned int px=1; px<mdim-1; px++)
            {
               for (unsigned int py=1; py<ndim-1; py++)
               {
                  int n_eff=ndim*px+py;
                  double curr_z=ztwoDarray_ptr->get(n_eff);
                  double curr_f=ftwoDarray_ptr->get(n_eff);
                  if (curr_z > xyzpfunc::null_value &&
                      nearly_equal(curr_f,featurefunc::road_sentinel_value) &&
                      zlo_asphalt_twoDarray_ptr->get(n_eff)==
                      xyzpfunc::null_value)
                  {
                     int counter=0;
                     for (int i=-1; i<=1; i++)
                     {
                        for (int j=-1; j<=1; j++)
                        {
                           int n2_eff=ndim*(px+i)+(py+j);
                           if (zlo_asphalt_twoDarray_ptr->get(n2_eff) !=
                               xyzpfunc::null_value)
                           {
                              double abs_z_deriv=
                                 fabs(ztwoDarray_ptr->get(n2_eff)-curr_z)/
                                 delta_s[counter];
                              if (abs_z_deriv < max_gradient_magnitude_lo)
                              {
                                 zlo_asphalt_twoDarray_ptr->put(n_eff,curr_z);
                                 nchanges++;
                                 break;
                              }
                           }
                           counter++;
                        } // loop over j index
                     } // loop over i index
                     
                  } // z(px,py)==null conditional
               } // loop over py index
            } // loop over px index
            if (iter%10==0)
            {
               cout << "iter = " << iter << " nchanges = " << nchanges 
                    << endl;
            }
            iter++;
         }
         while (nchanges > 0);

         return zlo_asphalt_twoDarray_ptr;
      }

// ==========================================================================
// Ground flattening methods
// ==========================================================================

// Method generate_zground_twoDarray instantiates a twoDarray with the
// same dimensions and spacing as input *ztwoDarray_ptr.  It then sets
// the value of non-ground regions within this new array to
// xyzpfunc::null_value based upon unit-valued pixels within input
// *groundmask_twoDarray_ptr.

   twoDarray* generate_zground_twoDarray(
     int iter,twoDarray const *ztwoDarray_ptr,
     twoDarray const *groundmask_twoDarray_ptr)
     {
//        cout << "inside groundfunc::generate_zground_twoDarray()" << endl;
        
        twoDarray* zground_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
        ztwoDarray_ptr->copy(zground_twoDarray_ptr);

        double z_threshold=1.0;
        double z_null=xyzpfunc::null_value;
        bool nearly_equal_flag=true;
        recursivefunc::binary_null(
           z_threshold,zground_twoDarray_ptr,groundmask_twoDarray_ptr,
           z_null,nearly_equal_flag);
        return zground_twoDarray_ptr;
     }

// ---------------------------------------------------------------------
// Method eliminate_ground_outliers takes in the original height map,
// the estimated bald earth ground surface and the binary ground mask.
// Scanning over every pixel within the binary mask, it examines the
// difference in height between the original height map and the bald
// earth surface for those pixels classified as ground.  If the height
// differential exceeds input parameter max_delta_height, this method
// resets the pixel's classification from ground to non-ground.  

// We wrote this method as a sanity check to eliminate obvious errors
// introduced by recursive emptying of small non-ground islands
// surrounded by oceans of ground.  Recall that recursive emptying
// works exclusively with binary classification information and does
// not take height into account.  As a result, it definitely
// introduces small errors wherein voxels on buildings are
// misclassified as ground.  This method removes the worst of such
// obvious mistakes.

   void eliminate_ground_outliers(
      twoDarray const *ztwoDarray_ptr,
      twoDarray const *zthreshold_twoDarray_ptr,
      twoDarray* groundmask_twoDarray_ptr,double max_delta_height)
      {
         string banner="Eliminating ground outliers:";
         outputfunc::write_banner(banner);

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double curr_z=ztwoDarray_ptr->get(px,py);
               double zthreshold=zthreshold_twoDarray_ptr->get(px,py);
               if (nearly_equal(groundmask_twoDarray_ptr->get(px,py),0) &&
                   curr_z-zthreshold > max_delta_height)
               {
                  groundmask_twoDarray_ptr->put(px,py,1);
               }
            } // loop over py index
         } // loop over px index
      }

// On 11/11/07, we realized that rejecting candidate ground points
// based upon their proximity to the estimated bald earth surface
// badly fails for complicated, multi-valued ground regions such as
// bridges and cloverleaf on-ramps.  So the previous version of
// eliminate_ground_outliers is deprecated compared to this next one
// where we simply compare each candidate groundpoint to its nearest
// neighbors.  If any neighboring groundpoint lies at a significantly
// different height than the candidate center point, we declare the
// center point to be an outlier and reset its ground mask value to
// unity (indicating that it is NOT a ground point).

   void eliminate_ground_outliers(
      int n_filter_size,twoDarray const *ztwoDarray_ptr,
      twoDarray* groundmask_twoDarray_ptr,double max_delta_height)
      {
         string banner="Eliminating ground outliers:";
         outputfunc::write_banner(banner);

         twoDarray* orig_groundmask_twoDarray_ptr=
            new twoDarray(groundmask_twoDarray_ptr);
         groundmask_twoDarray_ptr->copy(orig_groundmask_twoDarray_ptr);

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (nearly_equal(orig_groundmask_twoDarray_ptr->get(px,py),0))
               {
                  double curr_z=ztwoDarray_ptr->get(px,py);
                  
                  int ihi=n_filter_size;
                  int ilo=-ihi;
                  if (px+ilo < 0) ilo=-px;
                  if (px+ihi > ztwoDarray_ptr->get_mdim()-1) 
                  {
                     ihi=ztwoDarray_ptr->get_mdim()-1-px;
                  }
                  
                  int jhi=n_filter_size;
                  int jlo=-jhi;
                  if (py+jlo < 0) jlo=-py;
                  if (py+jhi > ztwoDarray_ptr->get_ndim()-1)
                  {
                     jhi=ztwoDarray_ptr->get_ndim()-1-py;
                  }

                  for (int i=ilo; i<=ihi; i++)
                  {
                     for (int j=jlo; j<=jhi; j++)
                     {
                        int n_eff=ztwoDarray_ptr->get_ndim()*(px+i)+(py+j);
                        
                        if (orig_groundmask_twoDarray_ptr->get(n_eff)==0)
                        {
                           double abs_deltaz=fabs(ztwoDarray_ptr->get(n_eff)-
                                                  curr_z);
                           if (abs_deltaz > max_delta_height)
                           {
                              groundmask_twoDarray_ptr->put(px,py,1);
                              continue;
                           }
                        } // groundmask(neff)==0 conditional
                     } // loop over j index
                  } // loop over i index
               } // groundmask(px,py)==0 conditional
            } // loop over py index
         } // loop over px index
         
         delete orig_groundmask_twoDarray_ptr;
      }
   
// ---------------------------------------------------------------------
// Method generate_flattened_height_map instantiates and fills
// *zflattened_twoDarray_ptr with the difference between height map
// *ztwoDarray_ptr and threshold field *zthreshold_twoDarray_ptr.  It
// then thresholds the contents of *zflattened_twoDarray_ptr so that
// only a narrow band of pixels lying within the height interval
// -cutoff_height < Z < +cutoff_height survive.

   twoDarray* generate_thresholded_flattened_height_map(
      twoDarray const *ztwoDarray_ptr,twoDarray* zthreshold_twoDarray_ptr,
      double cutoff_height)
      {
         string banner="Generating flattened height map:";
         outputfunc::write_banner(banner);

         twoDarray* zflattened_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         *zflattened_twoDarray_ptr = 
            *ztwoDarray_ptr - *zthreshold_twoDarray_ptr;
         ztwoDarray_ptr->copy_metric_data(zflattened_twoDarray_ptr);
   
// Discard points within the flattened height map which lie outside
// the interval [ -cutoff_height , +cutoff_height ] about z=0:

         imagefunc::threshold_intensities_above_cutoff(
            zflattened_twoDarray_ptr,cutoff_height,xyzpfunc::null_value);
         imagefunc::threshold_intensities_below_cutoff(
            zflattened_twoDarray_ptr,-cutoff_height,xyzpfunc::null_value);
         return zflattened_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method refine_ground_mask_seeds converts all non-null valued pixels
// within the flattened and thresholded twoDarray
// *zflattened_twoDarray_ptr into zero-valued ground sentinels.  The
// result yields an improved set of ground mask seeds and is returned
// within *groundmask_twoDarray_ptr.

   void refine_ground_mask_seeds(
      double cutoff_height,twoDarray* zflattened_twoDarray_ptr,
      twoDarray* groundmask_twoDarray_ptr)
      {
         double z_threshold=-100*cutoff_height;	// meters
         double z_null=0;
         bool nearly_equal_flag=false;
         bool greater_than_flag=true;

// FAKE FAKE: November 11 at 12:30 pm...Experiment with not
// obliterating previous iteration's ground ID info already stored
// within *groundmask_twoDarray_ptr:

// Reset any unit-valued pixels within *groundmask_twoDarray_ptr to
// xyzpfunc::null_value:

         imagefunc::threshold_intensities_above_cutoff(
            groundmask_twoDarray_ptr,0.5,xyzpfunc::null_value);
//         groundmask_twoDarray_ptr->initialize_values(xyzpfunc::null_value);

         recursivefunc::binary_null(
            z_threshold,groundmask_twoDarray_ptr,zflattened_twoDarray_ptr,
            z_null,nearly_equal_flag,greater_than_flag);
      }

// ---------------------------------------------------------------------
// Method completely_flatten_ground takes in twoDarray *ztwoDarray_ptr
// which we assume already contains height information corresponding
// to a cleaned, filled and fairly-well flattened ladar image.  It
// also takes in twoDarray *zhilo_twoDarray_ptr in which relatively
// high (low) pixels are indicated with one (zero) binary values.
// This method returns a new, dynamically generated twoDarray in which
// the z-values of all relatively low pixels are set precisely equal
// to zero.  

   twoDarray* completely_flatten_ground(
      const twoDarray* ztwoDarray_ptr,const twoDarray* zhilo_twoDarray_ptr)
      {
         twoDarray* zflat_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         zflat_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double curr_zhilo=zhilo_twoDarray_ptr->get(px,py);
               if (curr_zhilo > xyzpfunc::null_value)
               {
                  if (curr_zhilo < 0.5)
                  {
                     zflat_twoDarray_ptr->put(px,py,0);
                  }
                  else
                  {
                     zflat_twoDarray_ptr->put(
                        px,py,ztwoDarray_ptr->get(px,py));
                  }
               }
            } // loop over py index
         } // loop over px index
         return zflat_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method generate_gaussian_weights_filter returns a dynamically
// allocated twoDarray containing values for a 2D centered gaussian.

   twoDarray* generate_gaussian_weights_filter(double ds)
      {
         const int filter_length=5;	// meters
         unsigned int wbins=basic_math::round(filter_length/ds);
         if (is_even(wbins)) wbins++;

         twoDarray* weight_twoDarray_ptr=new twoDarray(wbins,wbins);
         weight_twoDarray_ptr->set_deltax(ds);
         weight_twoDarray_ptr->set_deltay(ds);
         weight_twoDarray_ptr->init_coord_system();

         double mu=0;
         double sigma=1;	// meters
         for (unsigned int px=0; px<wbins; px++)
         {
            double curr_x=weight_twoDarray_ptr->get_xlo()+px*ds;
            for (unsigned int py=0; py<wbins; py++)
            {
               double curr_y=weight_twoDarray_ptr->get_ylo()+py*ds;
               double curr_r=sqrt(sqr(curr_x)+sqr(curr_y));
               weight_twoDarray_ptr->put(
                  px,py,mathfunc::gaussian(curr_r,mu,sigma));
            } // loop over py index
         } // loop over px index

         return weight_twoDarray_ptr;
      }
   
// ---------------------------------------------------------------------
// Method interpolate_lo_asphalt_heights takes in a cleaned, flattened
// height image within *ztwoDarray_ptr, a feature map within
// *ftwoDarray_ptr and seed height values for genuinely low height
// asphalt pixels within *zlo_asphalt_twoDarray_ptr.  It attempts to
// apply an "oozing" procedure in order to determine the true ground
// heights for all pixels which have been classified as asphalt,
// including those which actually correspond to vehicles.  It
// essentially convolves a 2D gaussian weight filter with the contents
// of *zlo_asphalt_twoDarray_ptr.  It returns filtered asphalt height
// information within a dynamically generated twoDarray which
// hopefully corresponds to true ground and lets one search for
// vehicle height fluctuations within *ztwoDarray_ptr.

   twoDarray* interpolate_lo_asphalt_heights(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ftwoDarray_ptr,
      twoDarray const *zlo_asphalt_twoDarray_ptr)
      {
         twoDarray* z_ooze_asphalt_twoDarray_ptr=new twoDarray(
            zlo_asphalt_twoDarray_ptr);
         zlo_asphalt_twoDarray_ptr->copy(z_ooze_asphalt_twoDarray_ptr);

         const unsigned int mdim=ztwoDarray_ptr->get_mdim();
         const unsigned int ndim=ztwoDarray_ptr->get_ndim();
         const double dx=ztwoDarray_ptr->get_deltax();
         const double dy=ztwoDarray_ptr->get_deltay();

         twoDarray* weights_twoDarray_ptr=generate_gaussian_weights_filter(
            basic_math::min(dx,dy)); 
         int wbins=weights_twoDarray_ptr->get_mdim();

         int n_changes;
         do
         {
            n_changes=0;
            for (unsigned int px=0; px<mdim; px++)
            {
               for (unsigned int py=0; py<ndim; py++)
               {
                  double curr_z=ztwoDarray_ptr->get(px,py);
                  double curr_f=ftwoDarray_ptr->get(px,py);
                  double curr_zlo=z_ooze_asphalt_twoDarray_ptr->get(px,py);

                  if (curr_z > 0.99*xyzpfunc::null_value &&
                      nearly_equal(curr_f,featurefunc::road_sentinel_value) &&
                      curr_zlo < 0.99*xyzpfunc::null_value)
                  {
                     double numer=0;
                     double denom=0;
                     for (int i=-wbins/2; i<=wbins/2; i++)
                     {
                        for (int j=-wbins/2; j<=wbins/2; j++)
                        {
                           if (z_ooze_asphalt_twoDarray_ptr->
                               pixel_inside_working_region(px+i,py+j))
                           {
                              double curr_z_ooze_asphalt=
                                 z_ooze_asphalt_twoDarray_ptr->get(px+i,py+j);
                              if (curr_z_ooze_asphalt != 
                                  xyzpfunc::null_value)
                              {
                                 double curr_w=weights_twoDarray_ptr->get(
                                    i+wbins/2,j+wbins/2);
                                 numer += curr_w*curr_z_ooze_asphalt;
                                 denom += curr_w;
                              }
                           } // pixel inside working region conditional
                        } // loop over j index
                     } // loop over i index
                     if (denom > 0)
                     {
                        z_ooze_asphalt_twoDarray_ptr->put(px,py,numer/denom);
                        n_changes++;
                     }
                  }
               } // loop over py index
            } // loop over px index
            cout << "n_changes = " << n_changes << endl;
         }
         while (n_changes > 0);
         
         delete weights_twoDarray_ptr;
         return z_ooze_asphalt_twoDarray_ptr;
      }

// ==========================================================================
// Ground planarity testing methods
// ==========================================================================

// Method compute_local_planarity takes in a polygon and runs it over
// the input image contained within *ztwoDarray_ptr.  For each pixel
// (px,py) location within the image, this method first determines
// which neighboring pixels lie within the polygon when it is centered
// upon (px,py).  It then computes the dotproducts between the unit
// normal vectors of the center and neighboring pixels.  The average
// of these dotproducts is saved within output image
// *zplanar_twoDarray_ptr.

   void compute_local_planarity(
      polygon& poly,TwoDarray<threevector> const *normal_twoDarray_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray* zplanar_twoDarray_ptr)
      {
         twoDarray* zmask_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         threevector curr_center;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            if (px%100==0)
            {
               cout << "px = " << px << " mdim = " 
                    << ztwoDarray_ptr->get_mdim() << endl;
            }
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               threevector curr_normal=normal_twoDarray_ptr->get(px,py);  
               if (curr_normal.get(0) > xyzpfunc::null_value &&
                   curr_normal.get(1) > xyzpfunc::null_value &&
                   curr_normal.get(2) > xyzpfunc::null_value)
               {

// Center regular poly at pixel location (px,py).  Then compute
// pixel bounding box enclosing poly:

                  ztwoDarray_ptr->pixel_to_point(px,py,curr_center);
                  poly.absolute_position(curr_center);
                  unsigned int min_px,min_py,max_px,max_py;
                  ztwoDarray_ptr->locate_extremal_xy_pixels(
                     poly,min_px,min_py,max_px,max_py);

// Store poly's binary mask within *zmask_twoDarray_ptr:

//                  drawfunc::color_regular_hexagon_interior(
//                     poly,60,zmask_twoDarray_ptr);
                  drawfunc::color_convex_quadrilateral_interior(
                     poly,60,zmask_twoDarray_ptr);

                  int n_neighboring_normals=0;
                  double dotproduct_sum=0;
                  for (unsigned int i=min_px; i<max_px; i++)
                  {
                     for (unsigned int j=min_py; j<max_py; j++)
                     {
                        if (!(zmask_twoDarray_ptr->get(px,py) < 1 || 
                              (i==px && j==py)))
                        {
                           threevector neighbor_normal=
                              normal_twoDarray_ptr->get(i,j);
                           if (neighbor_normal.get(0) > xyzpfunc::null_value 
                               &&
                               neighbor_normal.get(1) > xyzpfunc::null_value 
                               &&
                               neighbor_normal.get(2) > xyzpfunc::null_value)
                           {
                              dotproduct_sum += neighbor_normal.dot(
                                 curr_normal);
                              n_neighboring_normals++;
                              zmask_twoDarray_ptr->put(i,j,0);
                           }
                        }
                     } // loop over j index
                  } // loop over i index
                  zmask_twoDarray_ptr->put(px,py,0);

                  double avg_dotproduct=dotproduct_sum/double(
                     n_neighboring_normals);
                  zplanar_twoDarray_ptr->put(px,py,avg_dotproduct);
               } // components of curr_normal > null_value conditional
            } // loop over py index
         } // loop over px index
         delete zmask_twoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method compute_planarity_in_bbox has been intentionally streamlined
// as much as possible in order to minimize execution time.

   void compute_planarity_in_bbox(
      double bbox_width,double bbox_length,
      TwoDarray<threevector> const *normal_twoDarray_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray const *zhilo_twoDarray_ptr,
      twoDarray* zplanar_twoDarray_ptr)
      {
         unsigned int mdim=ztwoDarray_ptr->get_mdim();
         unsigned int ndim=ztwoDarray_ptr->get_ndim();

// For speed purposes, first generate C-style array containing normal
// vector field magnitude information:

         double normal_magnitude[mdim*ndim];
         for (unsigned int px=0; px<mdim; px++)
         {
            for (unsigned int py=0; py<ndim; py++)
            {
               double nx=normal_twoDarray_ptr->get(ndim*px+py).get(0);
               double ny=normal_twoDarray_ptr->get(ndim*px+py).get(1);
               double nz=normal_twoDarray_ptr->get(ndim*px+py).get(2);
               normal_magnitude[ndim*px+py]=nx*nx+ny*ny+nz*nz;
            }
         }

         parallelogram bbox(bbox_width,bbox_length);
         threevector curr_center;

         double n_neighboring_normals=0;
         for (unsigned int px=0; px<mdim; px++)
         {
            if (px%100==0)
            {
               cout << "px = " << px << " mdim = " 
                    << ztwoDarray_ptr->get_mdim() << endl;
            }
      
            for (unsigned int py=0; py<ndim; py++)
            {
               int n_eff=ndim*px+py;
               if (normal_magnitude[n_eff] > 0.5 &&
                   zhilo_twoDarray_ptr->get(px,py) < 0.5)
               {

// Center bbox at pixel location (px,py):

                  ztwoDarray_ptr->pixel_to_point(px,py,curr_center);
                  bbox.absolute_position(curr_center);
                  unsigned int min_px,min_py,max_px,max_py;
                  ztwoDarray_ptr->locate_extremal_xy_pixels(
                     bbox,min_px,min_py,max_px,max_py);

                  double dotproduct_sum=integrate_dotproducts(
                     min_px,max_px,min_py,max_py,
                     normal_twoDarray_ptr->get(n_eff).get(0),
                     normal_twoDarray_ptr->get(n_eff).get(1),
                     normal_twoDarray_ptr->get(n_eff).get(2),
                     normal_magnitude,n_neighboring_normals,
                     normal_twoDarray_ptr);
                  double avg_dotproduct=dotproduct_sum/n_neighboring_normals;
                  zplanar_twoDarray_ptr->put(n_eff,avg_dotproduct);
               } // normal_magnitude > 0.5 conditional
            } // loop over py index
         } // loop over px index
      }

// ---------------------------------------------------------------------
// Method integrate_dotproducts has been intentionally stripped down
// to its barest essentials in order to increase execution speed as
// much as possible:
   
   double integrate_dotproducts(
      int min_px,int max_px,int min_py,int max_py,
      double curr_normal_x,double curr_normal_y,double curr_normal_z,
      double normal_magnitude[],double& n_neighboring_normals,
      TwoDarray<threevector> const *normal_twoDarray_ptr)
      {
         int ndim=normal_twoDarray_ptr->get_ndim();
         double nx_sum=0;
         double ny_sum=0;
         double nz_sum=0;
         n_neighboring_normals=0;
         
         for (int i=min_px; i<max_px; i++)
         {
            for (int j=min_py; j<max_py; j++)
            {
               int n_eff=ndim*i+j;
               double nx=normal_twoDarray_ptr->get(n_eff).get(0);
               double ny=normal_twoDarray_ptr->get(n_eff).get(1);
               double nz=normal_twoDarray_ptr->get(n_eff).get(2);
               nx_sum += nx;
               ny_sum += ny;
               nz_sum += nz;
               n_neighboring_normals += normal_magnitude[n_eff];
            }
         }
         return curr_normal_x*nx_sum+curr_normal_y*ny_sum
            +curr_normal_z*nz_sum;
      }

// ==========================================================================
// Surface feature enhancement methods:
// ==========================================================================

// Method exaggerate_surface_image

   twoDarray* exaggerate_surface_image(
      double z1,double z2,twoDarray const *ztwoDarray_ptr)
      {
         twoDarray* ptwoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         ptwoDarray_ptr->initialize_values(xyzpfunc::null_value);

         const double enhancement_factor=20;
         const double suppression_factor=0.05;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double curr_z=ztwoDarray_ptr->get(px,py);
               if (curr_z > xyzpfunc::null_value)
               {
                  if (curr_z >= z1 && curr_z <= z2)
                  {
                     ptwoDarray_ptr->put(
                        px,py,enhancement_factor*(curr_z-0.5*(z1+z2)));
                  }
                  else if (curr_z  > z2)
                  {
                     ptwoDarray_ptr->put(
                        px,py,enhancement_factor*(
                           0.5*(z2-z1))+suppression_factor*(curr_z-z2));
                  }
                  else if (curr_z < z1)
                  {
                     ptwoDarray_ptr->put(
                        px,py,enhancement_factor*(
                           0.5*(z1-z2))+suppression_factor*(curr_z+z1));
                  }
               }
            } // loop over py index
         } // loop over px index
         return ptwoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method average_down_small_height_fluctuations

   void average_down_small_height_fluctuations(
      twoDarray const *ztwoDarray_ptr,twoDarray* zsmoothed_twoDarray_ptr)
      {
         cout << "inside groundfunc::average_away_small_height_fluctuations"
              << endl;
         ztwoDarray_ptr->copy(zsmoothed_twoDarray_ptr);
         
         const int wbins=3;
         const int wbins_over_two=wbins/2;
         const double small_height_fluctuation=0.3;	// meter

         int n_smoothed_pixels=0;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double curr_z=ztwoDarray_ptr->get(px,py);
               if (curr_z > 0.99*xyzpfunc::null_value)
               {
                  int n_nonnull_neighbors=0;
                  double neighbor_zsum=0;
                  double max_z=NEGATIVEINFINITY;
                  double min_z=POSITIVEINFINITY;
                  for (int i=-wbins_over_two; i<=wbins_over_two; i++)
                  {
                     int qx=px+i;
                     for (int j=-wbins_over_two; j<=wbins_over_two; j++)
                     {
                        int qy=py+j;
                        if (ztwoDarray_ptr->pixel_inside_working_region(
                           qx,qy))
                        {
                           double neighbor_z=ztwoDarray_ptr->get(qx,qy);
                           if (neighbor_z > 0.99*xyzpfunc::null_value)
                           {
                              n_nonnull_neighbors++;
                              neighbor_zsum += neighbor_z;
                              max_z=basic_math::max(max_z,neighbor_z);
                              min_z=basic_math::min(min_z,neighbor_z);
                           } // neighbor_z > null_value conditional
                        } // neighbor inside working region conditional
                     } // loop over j index
                  } // loop over i index
                  if (max_z-min_z < small_height_fluctuation)
                  {
                     double numer=curr_z+neighbor_zsum;
                     double denom=1+n_nonnull_neighbors;
                     zsmoothed_twoDarray_ptr->put(px,py,numer/denom);
                     n_smoothed_pixels++;
                  } // small height fluctation conditional
               } // curr_z > null_value conditional
            } // loop over py index
         } // loop over px index
         cout << "n_smoothed_pixels = " << n_smoothed_pixels << endl;
      }

// ---------------------------------------------------------------------
// Method remove_individual_height_outliers

   void remove_individual_height_outliers(
      twoDarray const *ztwoDarray_ptr,twoDarray* zsmoothed_twoDarray_ptr)
      {
         cout << "inside groundfunc::remove_individual_height_outliers" 
              << endl;
         ztwoDarray_ptr->copy(zsmoothed_twoDarray_ptr);
         
         const int wbins=3;
         const int wbins_over_two=wbins/2;

         int n_outliers_removed=0;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double curr_z=ztwoDarray_ptr->get(px,py);
               if (curr_z > 0.99*xyzpfunc::null_value)
               {
                  int n_lower_neighbors=0;
                  double neighbor_zsum=0;
                  for (int i=-wbins_over_two; i<=wbins_over_two; i++)
                  {
                     int qx=px+i;
                     for (int j=-wbins_over_two; j<=wbins_over_two; j++)
                     {
                        int qy=py+j;
                        if (ztwoDarray_ptr->pixel_inside_working_region(
                           qx,qy))
                        {
                           double neighbor_z=ztwoDarray_ptr->get(qx,qy);
                           if (neighbor_z > 0.99*xyzpfunc::null_value &&
                               neighbor_z < curr_z)
                           {
                              n_lower_neighbors++;
                              neighbor_zsum += neighbor_z;
                           } 
                        } // neighbor inside working region conditional
                     } // loop over j index
                  } // loop over i index
                  if (n_lower_neighbors==sqr(wbins)-1)
                  {
                     zsmoothed_twoDarray_ptr->
                        put(px,py,
                            double(neighbor_zsum)/double(n_lower_neighbors));
                     n_outliers_removed++;
                  } // small height fluctation conditional
               } // curr_z > null_value conditional
            } // loop over py index
         } // loop over px index
         cout << "n_outliers_removed = " << n_outliers_removed << endl;
      }

// ---------------------------------------------------------------------
// Method remove_isolated_pixel

   void remove_isolated_pixels(twoDarray* ztwoDarray_ptr)
      {
         cout << "inside groundfunc::remove_isolated_pixels" << endl;
         twoDarray* znew_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         ztwoDarray_ptr->copy(znew_twoDarray_ptr);
         
         const int wbins=3;
         const int wbins_over_two=wbins/2;

         int n_isolated_pixels_removed=0;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               double curr_z=ztwoDarray_ptr->get(px,py);
               if (curr_z > 0.99*xyzpfunc::null_value)
               {
                  int n_null_neighbors=0;
                  for (int i=-wbins_over_two; i<=wbins_over_two; i++)
                  {
                     int qx=px+i;
                     for (int j=-wbins_over_two; j<=wbins_over_two; j++)
                     {
                        int qy=py+j;
                        if (ztwoDarray_ptr->pixel_inside_working_region(
                           qx,qy))
                        {
                           double neighbor_z=ztwoDarray_ptr->get(qx,qy);
                           if (neighbor_z < 0.99*xyzpfunc::null_value)
                           {
                              n_null_neighbors++;
                           } 
                        } // neighbor inside working region conditional
                     } // loop over j index
                  } // loop over i index
                  if (n_null_neighbors >= sqr(wbins)-2)
                  {
                     znew_twoDarray_ptr->
                        put(px,py,xyzpfunc::null_value);
                     n_isolated_pixels_removed++;
                  } // small height fluctation conditional
               } // curr_z > null_value conditional
            } // loop over py index
         } // loop over px index
         cout << "n_isolated_pixels_removed = " 
              << n_isolated_pixels_removed << endl;
         znew_twoDarray_ptr->copy(ztwoDarray_ptr);
         delete znew_twoDarray_ptr;
      }

   
} // groundfunc namespace





