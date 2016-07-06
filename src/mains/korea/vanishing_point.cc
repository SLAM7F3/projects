// ========================================================================
// Program VANISHING_POINT imports polylines from text files generated
// by program IMAGELINES.  These polylines are assumed to correspond
// to parallel lines in 3D world-space.  It computes the 2D imageplane
// intersection point of the parallel lines.  VANISHING_POINT then
// exports a new set of polylines which replace the initial input set.
// The vanishing points may be visualized via program VIDEO.
// ========================================================================
// Last updated on 9/17/13; 9/18/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Import manually-selected points lying along imageplane lines from
// file exported by program IMAGELINES:

   string input_filename="polylines_2D.txt";
   vector< vector<double> > row_numbers=filefunc::ReadInRowNumbers(
      input_filename);

   int n_lines=row_numbers.size()/2;
   vector<linesegment> lines;
   for (int l=0; l<n_lines; l++)
   {
      int r=2*l+0;
      double U0=row_numbers[r].at(3);
      double V0=row_numbers[r].at(4);
      threevector UVW0(U0,V0);

      r++;
      double U1=row_numbers[r].at(3);
      double V1=row_numbers[r].at(4);
      threevector UVW1(U1,V1);
      linesegment curr_segment(UVW0,UVW1);

      cout << "l = " << l << " curr_segment = " << curr_segment << endl;
      lines.push_back(curr_segment);
   } // loop over index l labeling 2D line segments

// Compute U,V coordinates of vanishing point where parallel lines
// intersect within 2D image plane:

   threevector vanishing_point;
   if (geometry_func::multi_line_intersection_point(
      lines,vanishing_point))
   {
      cout << "vanishing_point = " << vanishing_point << endl;
   }

// In order to visualize vanishing point within program VIDEO, export
// new set of line segments which all have the vanishing point as one
// of the end points:

   string output_filename="vanishinglines_2D.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# Time   PolyLine_ID   Passnumber   X Y R G B A" 
             << endl << endl;
   
   for (int l=0; l<n_lines; l++)
   {
      int r=2*l+0;
      double U0=row_numbers[r].at(3);
      double V0=row_numbers[r].at(4);
      threevector UVW0(U0,V0);
      linesegment segment0(vanishing_point,UVW0);

      r++;
      double U1=row_numbers[r].at(3);
      double V1=row_numbers[r].at(4);
      threevector UVW1(U1,V1);
      linesegment segment1(vanishing_point,UVW1);

      linesegment vanishing_segment;
      if (segment0.get_length() > segment1.get_length())
      {
         vanishing_segment=segment0;
      }
      else
      {
         vanishing_segment=segment1;
      }

      double time=0;
      int polyline_ID=n_lines+l;
      int passnumber=0;

      double Ustart=vanishing_segment.get_v1().get(0);
      double Vstart=vanishing_segment.get_v1().get(1);
      double Ustop=vanishing_segment.get_v2().get(0);
      double Vstop=vanishing_segment.get_v2().get(1);
      
      double red=1;
      double green=0;
      double blue=1;
      double alpha=1;
         
      outstream << time << "  "
                << polyline_ID << "  "
                << passnumber << "  "
                << Ustart << "   "
                << Vstart << "   "
                << red << "  "
                << green << "  "
                << blue << "  "
                << alpha << endl;
      outstream << time << "  "
                << polyline_ID << "  "
                << passnumber << "  "
                << Ustop << "   "
                << Vstop << "   "
                << red << "  "
                << green << "  "
                << blue << "  "
                << alpha << endl;
      outstream << endl;
   }
   filefunc::closefile(output_filename,outstream);

}

