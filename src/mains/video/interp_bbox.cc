// ==========================================================================
// Program INTERP_BBOX reads in bounding box information which was
// manually extracted from a few images in a video sequence indicating
// where ladar data overlaps.  INTERP_BBOX temporally interpolates the
// corners of the bounding boxes so that a bounding box can be placed
// onto every image in the video sequence. 

// We created this program to restrict the spatial area over which the
// KLT algorithm tries to track video features over time.
// ==========================================================================
// Last updated on 9/21/05
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "numerical/param_range.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"
#include "math/twovector.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ofstream;
   using std::setw;
   using std::string;
   using std::vector;

// First read in manually positioned bbox information:

   string subdir="./bbox_data/";
   string input_bbox_filename=
      subdir+"Rectangles_HAFB_overlap_corrected_grey.txt";
   filefunc::ReadInfile(input_bbox_filename);

// Open ascii file for interpolated bbox corner information:

   string output_filename="interp_bbox_corners.txt";
   ofstream outstream;
   filefunc::deletefile(output_filename);
   filefunc::openfile(output_filename,outstream);

   double X[3];
   vector<int> imagenumber;
   vector<twovector> bbox_corner[4];
   for (unsigned int i=0; i<filefunc::text_line.size(); i += 5)
   {
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i],X);
      imagenumber.push_back(X[0]);

      stringfunc::string_to_n_numbers(3,filefunc::text_line[i+1],X);
      bbox_corner[0].push_back(twovector(X[0],X[1]));
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i+2],X);
      bbox_corner[1].push_back(twovector(X[0],X[1]));
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i+3],X);
      bbox_corner[2].push_back(twovector(X[0],X[1]));
      stringfunc::string_to_n_numbers(3,filefunc::text_line[i+4],X);
      bbox_corner[3].push_back(twovector(X[0],X[1]));
//      cout << "imagenumber = " << imagenumber.back() << endl;
//      cout << "corner0 = " << bbox_corner[0].back() << endl;
//      cout << "corner1 = " << bbox_corner[1].back() << endl;
//      cout << "corner2 = " << bbox_corner[2].back() << endl;
//      cout << "corner3 = " << bbox_corner[3].back() << endl;
   } // loop over index i labeling filtered GPS/IMU information

// Linearly interpolate bbox corners manually determined at a few
// number of images to the entire image sequence:

   int n_images=289;	// HAFB_overlap_grey_corrected.vid
   twovector curr_corner[4];
   for (int i=0; i<n_images; i++)
   {
      int startbin=0;
      int stopbin=imagenumber.size()-1;
      int n=mathfunc::mylocate(imagenumber,i);
      int nstart=0;
      int nbins=1;
         
      twovector prev_corner,next_corner;
      for (int c=0; c<4; c++)
      {
         if (n < startbin)
         {
            prev_corner=(bbox_corner[c])[startbin];
            next_corner=(bbox_corner[c])[startbin];
         }
         else if (n >= stopbin)
         {
            prev_corner=(bbox_corner[c])[stopbin];
            next_corner=(bbox_corner[c])[stopbin];
         }
         else
         {
            prev_corner=(bbox_corner[c])[n];
            next_corner=(bbox_corner[c])[n+1];
            nstart=imagenumber[n];
            nbins=imagenumber[n+1]-nstart+1;
         }

         param_range U(prev_corner.get(0),next_corner.get(0),nbins);
         param_range V(prev_corner.get(1),next_corner.get(1),nbins);

         int rel_imagenumber=i-nstart;
         curr_corner[c]=twovector(U.compute_value(rel_imagenumber),
                                  V.compute_value(rel_imagenumber));
      } // loop over index c labeling bbox corners

      const int column_width=11;
      outstream << setw(column_width) << i 
                << setw(column_width) << "0"
                << setw(column_width) << "0" << endl;
      for (int c=0; c<4; c++)
      {
         outstream << setw(column_width) << curr_corner[c].get(0) 
                   << setw(column_width) << curr_corner[c].get(1) 
                   << setw(column_width) << "0" << endl;
      }
      outstream << endl;
      
   } // loop over index i labeling all image numbers   
}
