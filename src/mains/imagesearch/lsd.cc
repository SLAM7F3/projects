// ==========================================================================
// Program LSD runs the line segment detector algorithm/code of von
// Gioi (version 1.6 Nov 2011) on some specified input image.  It bins
// the detected line segments into (segment azimuthal orientation
// angle,impact parameter) cells.  Within each cell, LSD tries to
// concatenate line segments into longer "super" segments.  Super
// segments whose lengths exceed some minimal threshold are randomly
// colored within an output copy of the original input image.
// ==========================================================================
// Last updated on 5/13/12; 5/14/12; 8/29/12; 11/21/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "geometry/linesegment.h"
#include "math/lttwovector.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::string;
   using std::vector;

// Import image:

   string subdir="./";
//   string subdir="./mini_India/";
//   string subdir="/data/ImageEngine/south_India/";
   string image_filename="/data/ImageEngine/kermit/kermit000.jpg";
   cout << "Enter image filename:" << endl;
   cin >> image_filename;
//   image_filename="restaurant.jpg";

   image_filename=subdir+image_filename;
   
//   subdir="/data/ImageEngine/MIT2317_search/";
//   image_filename=subdir+"DCFC0935.JPG";

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      image_filename,NULL);
   int npx=texture_rectangle_ptr->getWidth();
   int npy=texture_rectangle_ptr->getHeight();
   double Umax=double(npx)/double(npy);
   double Vmax=1;
   cout << "Umax = " << Umax << " Vmax = " << Vmax << endl;

// Run von Gioi's line segment detector algorithm and generate initial
// raw set of line segment candidates for input image:

   vector<linesegment> line_segments=
      videofunc::detect_line_segments(texture_rectangle_ptr);
   cout << "n_segments = " << line_segments.size() << endl;

// Instantiate STL map *phi_r_segments_map_ptr and
// *parallel_segments_map_ptr which hold vectors of linesegments
// functions of quantized integer indices for impact parameter r from
// origin (U,V)=(0,0) and azimuthal linesegment orientation angle phi:

   double phi_start=0;
   double phi_stop=PI;
   double d_phi=1.5*PI/180;
//   double d_phi=3*PI/180;
   int n_phi=(phi_stop-phi_start)/d_phi;

   double r_start=0;
   double r_stop=basic_math::max(Umax,Vmax);
   double d_r=5.0/basic_math::min(npx,npy);
   int n_r=(r_stop-r_start)/d_r;

   twoDarray* r_phi_twoDarray_ptr=new twoDarray(n_phi,n_r);
   r_phi_twoDarray_ptr->init_coord_system(
      phi_start,phi_stop,r_start,r_stop);

   typedef std::map<twovector,vector<linesegment>,lttwovector > 
      PHI_R_SEGMENTS_MAP;
   PHI_R_SEGMENTS_MAP* phi_r_segments_map_ptr=new PHI_R_SEGMENTS_MAP;
   // *phi_r_segment_map_ptr independent var = (px_phi,py_r)
   // dependent var = vector<linesegment>

   vector<linesegment> orig_line_segments,long_orig_segments;
   for (unsigned int l=0; l<line_segments.size(); l++)
   {
      linesegment curr_l(line_segments[l]);
      threevector e_hat=curr_l.get_ehat();
      vector<double> abc=curr_l.compute_2D_line_coeffs();
      double a=abc[0];
      double b=abc[1];
      double c=abc[2];
      
      if (sgn(c) < 0) 
      {
         a *= -1;
         b *= -1;
         c *= -1;
      }

      double phi=atan2(e_hat.get(1),e_hat.get(0));
      double r=1/sqrt(a*a+b*b);

//      cout << "e_hat = " << e_hat << endl;
//      cout << "phi = " << phi*180/PI << endl;

      if (phi < 0) phi += PI;
//      cout << " restricted phi = " << phi*180/PI << endl;
//      cout << "r = " << r << endl;where

      unsigned int px,py;
//      cout << "r_phi_twoDarray = " << *r_phi_twoDarray_ptr << endl;
      
      if (r_phi_twoDarray_ptr->point_to_pixel(phi,r,px,py))
      {
//         cout << "px = " << px << " py = " << py << endl;
         twovector phi_r_indices(px,py);
         PHI_R_SEGMENTS_MAP::iterator iter=phi_r_segments_map_ptr->find(
            phi_r_indices);
         if (iter==phi_r_segments_map_ptr->end()) 
         {
            vector<linesegment> V;
            V.push_back(curr_l);
            (*phi_r_segments_map_ptr)[phi_r_indices]=V;
         }
         else
         {
            iter->second.push_back(curr_l);
         }
      }
      else
      {
         cout << "Point lies outside twoDarray bounds!" << endl;
      }

//      cout << "l = " << l << " phi_r_segments_map_ptr->size() = "
//           << phi_r_segments_map_ptr->size() << endl;
//      outputfunc::enter_continue_char();

      orig_line_segments.push_back(curr_l);

      const double min_length=0.01;
      if (curr_l.get_length() > min_length)
      {
         long_orig_segments.push_back(curr_l);
      }
   }
   delete r_phi_twoDarray_ptr;

// Iterate over all coarse (phi,r) cells within
// *phi_r_segments_map_ptr.  Ignore any which contain only a single
// line segment.  Try to concatenate remaining multiple quasi-parallel
// line segment candidates into longer parallel line segments.  Two
// line segments are declared to be parallel if endpoints of one lie
// within some tolerance to infinite line defined by other:

//   double toler=0.1;
   double toler=0.2;
//   cout << "Enter tolerance:" << endl;
//   cin >> toler;

   PHI_R_SEGMENTS_MAP* parallel_segments_map_ptr=new PHI_R_SEGMENTS_MAP;

   for (PHI_R_SEGMENTS_MAP::iterator iter=phi_r_segments_map_ptr->begin();
        iter != phi_r_segments_map_ptr->end(); iter++)
   {
      twovector phi_r_indices=iter->first;
      vector<linesegment> V=iter->second;
//      cout << "phi_r_indices = " << phi_r_indices
//           << " V.size() = " << V.size() << endl;

      if (V.size() == 1)
      {
         (*parallel_segments_map_ptr)[phi_r_indices]=V;
         continue;
      }

      vector<linesegment> parallel_segments;
      for (unsigned int v=0; v<V.size(); v++)
      {
         linesegment curr_l=V[v];
         
         PHI_R_SEGMENTS_MAP::iterator parallel_iter=
            parallel_segments_map_ptr->find(phi_r_indices);

         if (parallel_iter==parallel_segments_map_ptr->end()) 
         {
            vector<linesegment> V;
            V.push_back(curr_l);
            (*parallel_segments_map_ptr)[phi_r_indices]=V;
         }
         else
         {
            vector<linesegment> parallel_segments=parallel_iter->second;

// Loop over all existing parallel segments within current (phi,r)
// cell.  Check if curr_l segment is parallel to any of the existing
// segments.  If so, concatenate curr_l and existing parallel segment
// into single "super segment":

            bool curr_segment_parallel_flag=false;
            int parallel_segment_index=-1;
            linesegment existing_parallel_segment;
            for (unsigned int p=0; p<parallel_segments.size(); p++)
            {
               existing_parallel_segment=parallel_segments[p];
               double d1=existing_parallel_segment.point_to_line_distance(
                  curr_l.get_v1());
               double d2=existing_parallel_segment.point_to_line_distance(
                  curr_l.get_v2());

               if (d1 < toler*d_r && d2 < toler*d_r)
               {
                  curr_segment_parallel_flag=true;
                  parallel_segment_index=p;
                  break;
               }
            } // loop over index p labeling existing parallel segments

// Compute and sort endpoint fractions for existing parallel segment
// and curr_l.  Then use extremal endpoint fractions to define new
// endpoints for new "super segment":

            vector<double> fracs;
            vector<threevector> endpoints;
            if (curr_segment_parallel_flag)
            {
               endpoints.push_back(existing_parallel_segment.get_v1());
               fracs.push_back(
                  curr_l.fraction_distance_along_segment(endpoints.back()));

               endpoints.push_back(existing_parallel_segment.get_v2());
               fracs.push_back(
                  curr_l.fraction_distance_along_segment(endpoints.back()));

               endpoints.push_back(curr_l.get_v1());
               fracs.push_back(
                  curr_l.fraction_distance_along_segment(endpoints.back()));

               endpoints.push_back(curr_l.get_v2());
               fracs.push_back(
                  curr_l.fraction_distance_along_segment(endpoints.back()));

               templatefunc::Quicksort(fracs,endpoints);
               for (unsigned int f=0; f<fracs.size(); f++)
               {
//                  cout << "f = " << f << " frac = " << fracs[f] << endl;
               }
               double delta_f=(fracs.back()-fracs.front());
   
// On 5/13/12, we observed that linesegments completely unrelated in 3D space
// sometimes by chance nearly lie along the same infinite line within
// 2D image planes.  To minimize the likelihood that such unrelated
// line segments are concatenated, we do not join curr_l onto
// existing_parallel_segment if delta_f exceeds some reasonable
// maximal value:
            
               linesegment new_parallel_segment=existing_parallel_segment;

               const double max_delta_f=10;
               if (delta_f < max_delta_f)
               {
                  threevector v1_new(endpoints.front());
                  threevector v2_new(endpoints.back());
                  new_parallel_segment=linesegment(v1_new,v2_new);
               }

               parallel_iter->second.at(parallel_segment_index)=
                  new_parallel_segment;

//                outputfunc::enter_continue_char();

            }
            else
            {
               parallel_iter->second.push_back(curr_l);
            }
         } // parallel segments map conditional 

      } // loop over index v labeling candidate parallel segments

   } // iteration loop over *phi_r_segments_map_ptr

   delete phi_r_segments_map_ptr;

// Copy linesegments from STL map *parallel_segments_map_ptr which
// exceed min_UV_length into STL vector parallel_segments for drawing
// purposes:

   const double min_UV_length=0.05;
//   const double min_UV_length=0.1;

   vector<linesegment> parallel_segments;
   for (PHI_R_SEGMENTS_MAP::iterator iter=parallel_segments_map_ptr->begin();
        iter != parallel_segments_map_ptr->end(); iter++)
   {
      twovector phi_r_indices=iter->first;
      vector<linesegment> V=iter->second;

      for (unsigned int v=0; v<V.size(); v++)
      {
         linesegment curr_parallel_segment(V[v]);
         if (curr_parallel_segment.get_length() < min_UV_length) continue;
         parallel_segments.push_back(V[v]);
      }
   } // iterator over *parallel_segments_map_ptr

// Generate histogram in azimuthal angle phi for concatenated super
// segments:

   phi_start=0;
   phi_stop=PI;
   d_phi=2*PI/180;
   n_phi=(phi_stop-phi_start)/d_phi;
   cout << "n_phi = " << n_phi << endl;

   typedef std::map<double,vector<linesegment> > PHI_SEGMENTS_MAP;
   PHI_SEGMENTS_MAP* phi_segments_map_ptr=new PHI_SEGMENTS_MAP;
   // *phi_segment_map_ptr independent var = phi_index
   // dependent var = vector<linesegment>

   for (unsigned int p=0; p<parallel_segments.size(); p++)
   {
      linesegment curr_l=parallel_segments[p];
      threevector e_hat=curr_l.get_ehat();
      double phi=atan2(e_hat.get(1),e_hat.get(0));
      if (phi < 0) phi += PI;
      int phi_index=phi/d_phi;
//      cout << "p = " << p << " phi_index = " << phi_index << endl;

      PHI_SEGMENTS_MAP::iterator iter=phi_segments_map_ptr->find(
         phi_index);
      if (iter==phi_segments_map_ptr->end()) 
      {
         vector<linesegment> V;
         V.push_back(curr_l);
         (*phi_segments_map_ptr)[phi_index]=V;
      }
      else
      {
         iter->second.push_back(curr_l);
      }
   }

// Superpose randomly colored parallel super segment on copy of
// starting input image:

   subdir="./line_results/";
   filefunc::dircreate(subdir);
   string basename=filefunc::getbasename(image_filename);

   texture_rectangle_ptr->import_photo_from_file(image_filename);


   cout << "Number original line segments = " << orig_line_segments.size()
        << endl;

   int segment_color_index=-1;	// random segment coloring
   videofunc::draw_line_segments(
      orig_line_segments,texture_rectangle_ptr,segment_color_index);
   string orig_segments_filename=subdir+"orig_lines_"+basename;
   texture_rectangle_ptr->write_curr_frame(orig_segments_filename);

   texture_rectangle_ptr->import_photo_from_file(image_filename);

   videofunc::draw_line_segments(
      long_orig_segments,texture_rectangle_ptr,segment_color_index);
   string long_segments_filename=subdir+"long_lines_"+basename;
   texture_rectangle_ptr->write_curr_frame(long_segments_filename);

   cout << "Number long original segments = " << long_orig_segments.size()
        << endl;

// Count number of super segments that belong to nontrivial parallel
// line pairs:

   unsigned int min_parallel_cluster_size=2;
   cout << "Enter minimum number of parallel segments in clusters:" << endl;
   cin >> min_parallel_cluster_size;

   texture_rectangle_ptr->import_photo_from_file(image_filename);
   string parallel_segments_filename=subdir+"parallel_segments_"+basename;

   int color_counter=0;
   int parallel_lines_sum=0;
   for (PHI_SEGMENTS_MAP::iterator iter=phi_segments_map_ptr->begin();
        iter != phi_segments_map_ptr->end(); iter++)
   {
      vector<linesegment> V=iter->second;
      if (V.size() < min_parallel_cluster_size) continue;
      parallel_lines_sum += V.size();

      videofunc::draw_line_segments(
         V,texture_rectangle_ptr,color_counter%12);
      texture_rectangle_ptr->write_curr_frame(parallel_segments_filename);
      color_counter++;
   }

   delete texture_rectangle_ptr;

   cout << "Number concatenated parallel segments = " 
        << parallel_segments.size() << endl;
   cout << "Parallel lines sum = " << parallel_lines_sum << endl;
}
