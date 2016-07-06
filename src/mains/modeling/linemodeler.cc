// ==========================================================================
// Program LINEMODELER reads in 2D linesegments selected from at least
// 2 reconstructed and georegistered photos.  It backprojects these
// segments into 3D space to find their associated planes.
// LINEMODELER computes the infinite 3D lines corresponding to 2 or
// more matching line segments in images.  

// LINEMODELER next assumes the input 2D linesegments correspond to a
// 2D rectangle.  It intersects the infinite 3D lines and determines
// the finite corner locations.  LINEMODELER writes out a polyline
// which corresponds to a 3D rectangle.  The 3D rectangle may be
// viewed via QTVIEWLADAR superposed against bundler and/or ladar
// point clouds.

// ==========================================================================
// Last updated on 12/20/11; 4/4/12; 2/28/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/genvector.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"


using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   threevector camera_world_COM(Zero_vector);
   vector<camera*> camera_ptrs;
   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
//      cout << "n = " << n << " *photo_ptr = " << *photo_ptr << endl;
      camera* camera_ptr=photo_ptr->get_camera_ptr();
      camera_ptrs.push_back(camera_ptr);
      cout << "n = " << n
           << " camera = " << *camera_ptr << endl;
      camera_world_COM += camera_ptr->get_world_posn();
   }
   camera_world_COM /= n_photos;
   cout << "camera_world_COM = " << camera_world_COM << endl;

// Subtract off camera_world_COM from camera world positions in order
// to avoid working with numerically large values:

   for (int n=0; n<camera_ptrs.size(); n++)
   {
      camera* camera_ptr=camera_ptrs[n];
      threevector camera_world_posn=camera_ptr->get_world_posn();
      camera_world_posn -= camera_world_COM;
      camera_ptr->set_world_posn(camera_world_posn);
      camera_ptr->construct_projection_matrix(false);
      genmatrix* P_ptr=camera_ptr->get_P_ptr();
      cout << "n = " << n << " reset P = " << *P_ptr << endl;
   }

// Read in line segments manually extracted from reconstructed
// photos:

   vector<string> polyline_filenames;
   polyline_filenames.push_back("polylines_2D_5898.txt");
   polyline_filenames.push_back("polylines_2D_5902.txt");
   polyline_filenames.push_back("polylines_2D_5905.txt");

//   int n_matching_segments=1;
   int n_matching_segments=4;
   vector<linesegment> infinite_lines;

   for (int segment_ID=0; segment_ID < n_matching_segments; segment_ID++)
   {
      vector<linesegment> twoD_linesegment;
//       vector<plane*> plane_ptrs;

      int n_planes=polyline_filenames.size();
      genmatrix A(n_planes,4);

      for (int n=0; n<n_planes; n++)

      for (int file_counter=0; file_counter<polyline_filenames.size(); 
           file_counter++)
      {
         filefunc::ReadInfile(polyline_filenames[file_counter]);

         vector<threevector> V;
         for (int i=0; i<filefunc::text_line.size(); i++)
         {
            vector<double> column_values=stringfunc::string_to_numbers(
               filefunc::text_line[i]);
            int polyline_ID=column_values[1];
            if (polyline_ID != segment_ID) continue;
            
            V.push_back(threevector(column_values[3],column_values[4]));
//            cout << "V = " << V.back() << endl;
         }

         linesegment l(V[0],V[1]);
         twoD_linesegment.push_back(l);

         vector<double> line_coeffs=l.compute_2D_line_coeffs();
         for (int i=0; i<line_coeffs.size(); i++)
         {
//            cout << "i = " << i << " line_coeff = " << line_coeffs[i] << endl;
         }
         cout << endl;

         genvector l2(3);
         l2.put(0,0,line_coeffs[0]);
         l2.put(0,1,line_coeffs[1]);
         l2.put(0,2,line_coeffs[2]);
         camera* camera_ptr=camera_ptrs[file_counter];
         genmatrix* P_ptr=camera_ptr->get_P_ptr();
         fourvector pi=P_ptr->transpose()*l2;
         A.put_row(file_counter,pi);

//         cout << "pi = " << pi << endl;
//         plane* plane_ptr=new plane(pi);
//      cout << "*plane_ptr = " << *plane_ptr << endl;
//         plane_ptrs.push_back(plane_ptr);

      } // loop over file_counter index labeling input polyline files

//   cout << "A = " << A << endl;

// Follow "Line reconstruction section 12.7 in "Multiple view geometry
// in computer vision" by Hartley and Zisserman, 2nd edition:

      genmatrix U(n_planes,4) , W(4,4) , V(4,4);
      A.sorted_singular_value_decomposition(U,W,V);
//   cout << "U = " << U << endl;
//   cout << "W = " << W << endl;
//   cout << "V = " << V << endl;
//   cout << "U*Utrans = " << U*U.transpose() << endl;
//   cout << "V*Vtrans = " << V*V.transpose() << endl;
//   cout << "Vtrans*V = " << V.transpose()*V << endl;

//   cout << "A-U*W*Vtrans = "
//        << A-U*W*V.transpose() << endl;
   
      fourvector V0,V1,V2;
      V.get_column(0,V0);
      V.get_column(1,V1);
      V.get_column(2,V2);
//      cout << "V0 = " << V0 
//           << " V1 = " << V1 
//           << " V2 = " << V2
//           << endl;

// On 12/20/11, we found that the fourth column in the 4x4 matrix V
// was all zeros.  So we need to explicitly project out a 4th
// orthonormal vector V3 from V:

      genmatrix Q(4,4);
      Q.identity();
      Q -= V0.outerproduct(V0);
      Q -= V1.outerproduct(V1);
      Q -= V2.outerproduct(V2);
//   cout << "Q = " << Q << endl;
   
//   cout << "Q.V0 = " << Q*V0 << endl;
//   cout << "Q.V1 = " << Q*V1 << endl;
//   cout << "Q.V2 = " << Q*V2 << endl;

      fourvector Qx=Q*fourvector(1,0,0,0);
      fourvector Qy=Q*fourvector(0,1,0,0);
      fourvector Qz=Q*fourvector(0,0,1,0);
      fourvector Qw=Q*fourvector(0,0,0,1);
      Qx=Qx.unitvector();
      Qy=Qy.unitvector();
      Qz=Qz.unitvector();
      Qw=Qw.unitvector();

      if (Qx.get(0) < 0) Qx *= -1;
      if (Qy.get(0) < 0) Qy *= -1;
      if (Qz.get(0) < 0) Qz *= -1;
      if (Qw.get(0) < 0) Qw *= -1;

//      cout << "Q*x_hat = " << Qx << endl;
//      cout << "Q*y_hat = " << Qy << endl;
//      cout << "Q*z_hat = " << Qz << endl;
//      cout << "Q*w_hat = " << Qw << endl;

      fourvector V3=0.25*(Qx+Qy+Qz+Qw);
//      cout << "V3 = " << V3 << endl;

      cout << "V0.V0 = " << V0.dot(V0) << endl;
      cout << "V0.V1 = " << V0.dot(V1) << endl;
      cout << "V0.V2 = " << V0.dot(V2) << endl;
      cout << "V0.V3 = " << V0.dot(V3) << endl;

      cout << "V1.V1 = " << V1.dot(V1) << endl;
      cout << "V1.V2 = " << V1.dot(V2) << endl;
      cout << "V1.V3 = " << V1.dot(V3) << endl;

      cout << "V2.V2 = " << V2.dot(V2) << endl;
      cout << "V2.V3 = " << V2.dot(V3) << endl;

      cout << "V3.V3 = " << V3.dot(V3) << endl << endl;

// Renormalize the threevector parts of V2 and V3.  The normalized
// vectors for a basis for the 2-dimensional null space of W*T = [V1 , V2]

      V2 /= V2.get(3);
      V3 /= V3.get(3);

      threevector a(V2);
      threevector b(V3);
      threevector n_hat=(a-b).unitvector();

// Infinite 3D line has direction vector n_hat.  Threevectors a and b
// lie along the 3D line:

//      string banner="Infinite 3D line ID = "+stringfunc::number_to_string(
//         segment_ID);
//      outputfunc::write_big_banner(banner);

//      cout << "a = " << a << endl;
//      cout << "b = " << b << endl;
//      cout << "n_hat = " << n_hat << endl;
//      cout << "b+10*n_hat = " << b+10*n_hat << endl;

// Reshift reconstructed infinite 3D lines by camera_world_COM in
// order to return to genuine UTM coordinates:

      threevector vstart=b+camera_world_COM;
      threevector vstop=b+10*n_hat+camera_world_COM;

      linesegment curr_infinite_line(vstart,vstop);
      infinite_lines.push_back(curr_infinite_line);
  
/*    
      cout << "0 " << segment_ID 
           << " 0 "
           << vstart.get(0) << "  "
           << vstart.get(1) << "  "
           << vstart.get(2) << "  " 
           << " 1  1  1  1" << endl;
      cout << "0 " << segment_ID 
           << " 0 "
           << vstop.get(0) << "  "
           << vstop.get(1) << "  "
           << vstop.get(2) << "  " 
           << " 1  1  1  1" << endl;
*/

   } // loop over segment_ID index

   vector<threevector> rectangle_corners;
   for (int c=0; c<4; c++)
   {
      vector<linesegment> orthogonal_lines;

      orthogonal_lines.push_back(infinite_lines[c]);
//      cout << "ortho line A = " << orthogonal_lines.back() << endl;

      orthogonal_lines.push_back(infinite_lines[modulo(c+1,4)]);
//      cout << "ortho line B = " << orthogonal_lines.back() << endl;
      
      threevector curr_corner;
      geometry_func::multi_line_intersection_point(
         orthogonal_lines,curr_corner);
      rectangle_corners.push_back(curr_corner);
//      cout << "c = " << c << " corner = " << curr_corner << endl;
//      outputfunc::enter_continue_char();

      cout << "0 " << c << " 0 "
           << curr_corner.get(0) << "  "
           << curr_corner.get(1) << "  "
           << curr_corner.get(2) << "  "
           << " 1  1  1  1 " << endl;
   } // loop over index c labeling 3D rectangle corners
   
      cout << "0 4 0 "
           << rectangle_corners[0].get(0) << "  "
           << rectangle_corners[0].get(1) << "  "
           << rectangle_corners[0].get(2) << "  "
           << " 1  1  1  1 " << endl;

}
