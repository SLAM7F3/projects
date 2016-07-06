// ========================================================================
// Program ORTHOBLDGS imports a 2D building rectangular footprint
// corners manually derived from Google Earth.  It computes the
// lengths of the rectangle's short and long sides.
// ========================================================================
// Last updated on 8/5/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "geometry/parallelogram.h"
#include "geometry/polygon.h"
#include "general/sysfuncs.h"


// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
   string bldg_footprints_filename="./bldg_2D_footprints.txt";
   filefunc::ReadInfile(bldg_footprints_filename);

   vector<twovector> vertices;
   vector<threevector> corners;
   cout.precision(10);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      twovector EN(column_values[3],column_values[4]);
      threevector ENA(column_values[3],column_values[4],0);
      vertices.push_back(EN);
      corners.push_back(ENA);
   }

   polygon footprint(vertices);
   cout << "footprint = " << footprint << endl;
   footprint.initialize_edge_segments();
   threevector COM=footprint.compute_COM();
   cout << "COM = " << COM << endl;

   int n_edges=footprint.get_nvertices();

   vector<double> radii,phi,side_lengths;
   for (int e=0; e<n_edges; e++)
   {
//      cout << "e = " << e << endl;
      linesegment curr_edge=footprint.get_edge(e);
      side_lengths.push_back(curr_edge.get_length());
      cout << "e = " << e << " e_hat = " << curr_edge.get_ehat() << endl;
      threevector v1=curr_edge.get_v1();
      linesegment next_edge=footprint.get_edge(modulo(e+1,n_edges));
//      cout << "curr_edge = " << curr_edge
//           << " next_edge = " << next_edge << endl;
      double dotproduct=curr_edge.get_ehat().dot(next_edge.get_ehat());
      double theta=acos(dotproduct);

      threevector curr_radius=v1-COM;
      radii.push_back(curr_radius.magnitude());
      double curr_phi=atan2(curr_radius.get(1),curr_radius.get(0));
      curr_phi=basic_math::phase_to_canonical_interval(curr_phi,0,PI/2);
      phi.push_back(curr_phi);
      cout << "e = " << e 
           << " r = " << radii.back()
           << " phi = " << phi.back()*180/PI
           << " theta = " << theta*180/PI 
           << " s = " << side_lengths.back() << endl;
   }

   double side_short=0.5*(side_lengths[0]+side_lengths[2]);
   double side_long=0.5*(side_lengths[1]+side_lengths[3]);

   cout << "side_short = " << side_short
        << " side_long = " << side_long << endl;

   double r=mathfunc::mean(radii);
   cout << "r = " << r << endl;
	
   exit(-1);
   
   parallelogram* parallelogram_ptr=new parallelogram(corners);
   cout << "parallelogram = " << *parallelogram_ptr << endl;
   parallelogram_ptr->rectangle_within_quadrilateral(corners);
   cout << "rectangle = " << *parallelogram_ptr << endl;

}

