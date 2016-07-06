// ==========================================================================
// Program POLY_DIFFERENCE reads in a set of OFF files derived from
// Geoff Brown's parking lot and road PLY files (which he created in
// April 2012).  It first extracts the footprint polygons for each
// MIT model building and each parking lot.  POLY_DIFFERENCE then
// subtracts all building footprint polygons from each parking lot
// polygon.  Polyhedra corresponding to trimmed parking lots are
// exported to OFF files.
// ==========================================================================
// Last updated on 6/12/12; 6/13/12
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "models/Building.h"
#include "models/BuildingsGroup.h"
#include "geometry/contour.h"
#include "geometry/face.h"
#include "geometry/geometry_funcs.h"
#include "models/ParkingLot.h"
#include "models/ParkingLotsGroup.h"
#include "geometry/polygon.h"
#include "geometry/polyhedron.h"
#include "models/RoadsGroup.h"
#include "general/sysfuncs.h"

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


// Instantiate BuildingsGroup object:

   BuildingsGroup* BuildingsGroup_ptr=new BuildingsGroup();
   string OFF_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/modeling/OFF/";
   BuildingsGroup_ptr->import_from_OFF_files(OFF_subdir);

// Instantiate ParkingLotsGroup object:

   ParkingLotsGroup* ParkingLotsGroup_ptr=new ParkingLotsGroup();
   ParkingLotsGroup_ptr->import_from_OFF_files(OFF_subdir);

// Instantiate RoadsGroup object:

   RoadsGroup* RoadsGroup_ptr=new RoadsGroup();
   RoadsGroup_ptr->import_from_OFF_files(OFF_subdir);

   int n_Buildings=BuildingsGroup_ptr->get_n_Buildings();
   int n_ParkingLots=ParkingLotsGroup_ptr->get_n_ParkingLots();
   int n_Roads=RoadsGroup_ptr->get_n_Roads();

   cout << "n_Buildings = " << n_Buildings << endl;
   cout << "n_ParkingLots = " << n_ParkingLots << endl;
   cout << "n_Roads = " << n_Roads << endl;

   cout.precision(12);

// Extract single polygon footprint for each building:

   vector<polygon> Building_polys;
   for (int b=0; b<n_Buildings; b++)
   {
      Building* Building_ptr=BuildingsGroup_ptr->get_Building_ptr(b);
      vector<polyhedron*> Building_polyhedra_ptrs=Building_ptr->
         get_polyhedra_ptrs();

      vector<polygon> lateral_faces;
      for (int p=0; p<Building_polyhedra_ptrs.size(); p++)
      {
         polyhedron* Building_polyhedron_ptr=
            Building_polyhedra_ptrs[p];
         int n_faces=Building_polyhedron_ptr->get_n_faces();

         for (int f=0; f<n_faces; f++)
         {
            face* face_ptr=Building_polyhedron_ptr->get_face_ptr(f);
            threevector normal=face_ptr->get_normal();
            if (normal.dot(z_hat) > 0.99)
            {
               polygon curr_poly(face_ptr->get_polygon());
               curr_poly.scale(curr_poly.compute_COM(),1.05);
               lateral_faces.push_back(curr_poly);
            }
         } // loop over index f labeling polyhedron faces
      } // loop over index p labeling Building polyhedra

      vector<polygon> face_union=geometry_func::polygon_union(lateral_faces);
//      cout << "b = " << b 
//           << " face_union.size() = " << face_union.size() << endl;

      for (int f=0; f<face_union.size(); f++)
      {
         Building_polys.push_back(face_union[f]);
      }

   } // loop over index b labeling Buildings

   cout << endl;
   cout << "Building_polys.size() = " << Building_polys.size() << endl;
   
// Extract single polygon footprint for each parking lot:

   vector<polygon> ParkingLot_polys;
   for (int p=0; p<n_ParkingLots; p++)
   {
      ParkingLot* ParkingLot_ptr=ParkingLotsGroup_ptr->get_ParkingLot_ptr(p);
      vector<polyhedron*> ParkingLot_polyhedra_ptrs=ParkingLot_ptr->
         get_polyhedra_ptrs();
//      cout << "p = " << p
//           << " ParkingLot_polyhedra_ptrs.size() = "
//           << ParkingLot_polyhedra_ptrs.size() << endl;

      polyhedron* ParkingLot_polyhedron_ptr=ParkingLot_polyhedra_ptrs.front();
      int n_faces=ParkingLot_polyhedron_ptr->get_n_faces();
//      cout << "n_faces = " << n_faces << endl;

      vector<polygon> lateral_faces;
      for (int f=0; f<n_faces; f++)
      {
         face* face_ptr=ParkingLot_polyhedron_ptr->get_face_ptr(f);
         threevector normal=face_ptr->get_normal();
         if (normal.dot(z_hat) > 0.99)
         {
            polygon curr_poly(face_ptr->get_polygon());
            curr_poly.scale(curr_poly.compute_COM(),1.001);
            lateral_faces.push_back(curr_poly);
         }
      } // loop over index f labeling polyhedron faces

      vector<polygon> face_union=geometry_func::polygon_union(lateral_faces);
      ParkingLot_polys.push_back(face_union.front());
   } // loop over index p labeling parking lots
   cout << "ParkingLot_polys.size() = " << ParkingLot_polys.size() 
        << endl;

   vector<polygon> trimmed_ParkingLot_polys=
      geometry_func::polygon_difference(ParkingLot_polys,Building_polys);

   double z_lo=2;	// meters
   double z_hi=4;	// meters
   double delta_z=z_hi-z_lo;

   for (int p=0; p<trimmed_ParkingLot_polys.size(); p++)
   {
//      cout << "p = " << p
//           << " trimmed_PL poly = " << trimmed_ParkingLot_polys[p]      
//           << endl;
      contour bottom_contour(&trimmed_ParkingLot_polys[p]);
      bottom_contour.translate(z_lo*z_hat);
      polyhedron* trimmed_ParkingLot_polyhedron_ptr=new polyhedron();
      trimmed_ParkingLot_polyhedron_ptr->
         generate_prism_with_rectangular_faces(bottom_contour,delta_z);

      string off_filename="parking_"+stringfunc::number_to_string(
         p)+".off";
//      string off_filename="TrimmedParkingLot_"+stringfunc::number_to_string(
//         p)+".off";
      trimmed_ParkingLot_polyhedron_ptr->write_OFF_file(off_filename);
   }
   
}
