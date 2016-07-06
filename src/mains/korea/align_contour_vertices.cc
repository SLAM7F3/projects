// ========================================================================
// Program ALIGN_CONTOUR_VERTICES imports a set of TEL vertices
// generated via triangulating tiepoint pairs from 2 internet ground
// photos.  We initially work in a "TEL" coordinate system defined by
// n_hat, a_hat and z_hat direction vectors.  a_hat points in the
// TEL's forward direction.  z_hat points upwards.  n_hat = a_hat x
// z_hat points away from the TEL along its broadside direction.

// In this program, we first hardwire sets of 3D TEL vertices which
// should share n, a and/or z coordinates.  3D vertices n,a,z
// coordinates are replaced by median values.  We also allow for
// manual tweaking of TEL contours in the n,a,z coordinate system.
// Corrected TEL 3D vertices are exported to
// bundler_IO_subdir/corrected_rel_TEL_vertices.dat Corrected TEL
// contours are also exported as polylines in XYZ world coordinates
// and flipped about the a_hat - z_hat symmetry plane.

// 			align_contour_vertices

// ========================================================================
// Last updated on 9/20/13; 9/21/13; 9/25/13; 9/26/13
// ========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "models/Building.h"
#include "models/BuildingsGroup.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "geometry/plane.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "math/rotation.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "datastructures/union_find.h"
#include "osg/osgWindow/ViewerManager.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   string bundler_IO_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/korea/bundler/Korea/korea/";
   string contours_subdir=bundler_IO_subdir+"contours/";

   threevector star_center(60.1208, 49.2312,1.6156);

   typedef map<string,vector<int> > CONTOUR_VERTICES_MAP;
// independent string = contour label
// dependent vector<int> = feature_IDs corresponding to contour

   CONTOUR_VERTICES_MAP contour_vertices_map;
   CONTOUR_VERTICES_MAP::iterator contour_vertices_iter;

   typedef map<int,vector<threevector> > VERTEX_MAP;
// independent int = vertex ID
// dependent vector<threevector> = triangulated relative posns for vertex   
   VERTEX_MAP vertex_map;
   VERTEX_MAP::iterator vertex_iter;

// Store IDs for vertices which should share common relative n, a and
// z values within the following union_find objects:

   union_find N_union_find,A_union_find,Z_union_find;

   string rel_TEL_vertices_filename="rel_TEL_vertices.dat";

   filefunc::ReadInfile(rel_TEL_vertices_filename);
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      string contour_ID=substrings[0];
      int vertex_ID=stringfunc::string_to_number(substrings[1]);
      N_union_find.MakeSet(vertex_ID);
      A_union_find.MakeSet(vertex_ID);
      Z_union_find.MakeSet(vertex_ID);

//      cout << "i = " << i << " contour = " << contour_ID
//           << " vertex_ID = " << vertex_ID << endl;

      contour_vertices_iter=contour_vertices_map.find(contour_ID);
      if (contour_vertices_iter==contour_vertices_map.end())
      {
         vector<int> vertex_IDs;
         vertex_IDs.push_back(vertex_ID);
         contour_vertices_map[contour_ID]=vertex_IDs;
      }
      else
      {
         contour_vertices_iter->second.push_back(vertex_ID);
      }

      double rel_n=stringfunc::string_to_number(substrings[2]);
      double rel_a=stringfunc::string_to_number(substrings[3]);
      double rel_z=stringfunc::string_to_number(substrings[4]);
      threevector rel_vertex(rel_n,rel_a,rel_z);
//      cout << "rel_vertex = " << rel_vertex << endl;
      
      vertex_iter=vertex_map.find(vertex_ID);
      if (vertex_iter==vertex_map.end())
      {
         vector<threevector> rel_vertex_posns;
         rel_vertex_posns.push_back(rel_vertex);
         vertex_map[vertex_ID]=rel_vertex_posns;
      }
      else
      {
         vertex_iter->second.push_back(rel_vertex);
      }
      
//      cout << "At end of i for loop" << endl;
   } // loop over index i labeling text line

// Compute averages of all vertex positions.  Then reset vertices to
// their averaged positions:

   for (vertex_iter=vertex_map.begin(); vertex_iter != vertex_map.end();
        vertex_iter++)
   {
      vector<threevector> curr_vertex_posns=vertex_iter->second;
      
      threevector avg_vertex_posn(0,0,0);
      for (int v=0; v<curr_vertex_posns.size(); v++)
      {
         avg_vertex_posn += curr_vertex_posns[v];
      }
      avg_vertex_posn /= curr_vertex_posns.size();
      
      vertex_iter->second.clear();
      vertex_iter->second.push_back(avg_vertex_posn);
   } // vertex_iter loop over vertex_map

// Identify 3D vertices with common N coordinates:

   N_union_find.Link(9,8);
   N_union_find.Link(8,24);
   N_union_find.Link(6,7);
   N_union_find.Link(14,16);
   N_union_find.Link(16,18);
   N_union_find.Link(13,12);
   N_union_find.Link(15,17);
   N_union_find.Link(17,19);
   N_union_find.Link(10,11);

   N_union_find.Link(80,81);
   N_union_find.Link(79,82);
   N_union_find.Link(83,84);

   N_union_find.Link(0,20);
   N_union_find.Link(9,21);
   N_union_find.Link(21,22);
   N_union_find.Link(22,23);
   N_union_find.Link(23,24);

   N_union_find.Link(30,29);
   N_union_find.Link(30,31);
   N_union_find.Link(29,32);
   N_union_find.Link(29,26);

   N_union_find.Link(26,25);
   N_union_find.Link(26,27);
   N_union_find.Link(27,28);
   N_union_find.Link(28,25);

   N_union_find.Link(32,31);
   N_union_find.Link(33,34);
   N_union_find.Link(34,42);
   N_union_find.Link(42,37);
   N_union_find.Link(38,39);
   N_union_find.Link(40,37);
   N_union_find.Link(43,42);
   N_union_find.Link(41,34);

   N_union_find.Link(61,62);
   N_union_find.Link(63,64);
   N_union_find.Link(90,93);
   N_union_find.Link(92,93);
   N_union_find.Link(91,92);

   N_union_find.Link(65,66);
   N_union_find.Link(67,74);
   N_union_find.Link(68,73);
   N_union_find.Link(69,72);
   N_union_find.Link(69,70);
   N_union_find.Link(72,71);

   N_union_find.Link(86,85);
   N_union_find.Link(89,88);

   N_union_find.Link(94,111);
   N_union_find.Link(95,112);
   N_union_find.Link(96,113);
   N_union_find.Link(97,114);
   N_union_find.Link(98,115);
   N_union_find.Link(99,116);
   N_union_find.Link(100,117);   
   N_union_find.Link(101,118);   

   N_union_find.Link(70,78);
   N_union_find.Link(78,77);
   N_union_find.Link(77,76);
   N_union_find.Link(76,75);   
   N_union_find.Link(75,71);   

   N_union_find.Link(44,45);
   N_union_find.Link(46,40);
   N_union_find.Link(40,43);
   N_union_find.Link(41,43);
   N_union_find.Link(40,46);
   N_union_find.Link(46,47);
   N_union_find.Link(47,41);

   N_union_find.Link(47,50);
   N_union_find.Link(46,49);
   N_union_find.Link(45,48);

   N_union_find.Link(48,51);
   N_union_find.Link(49,52);
   N_union_find.Link(50,53);

   N_union_find.Link(51,54);
   N_union_find.Link(52,55);
   N_union_find.Link(53,56);
   N_union_find.Link(52,53);
   N_union_find.Link(55,56);

   N_union_find.Link(54,57);
   N_union_find.Link(55,58);
   N_union_find.Link(56,35);
   N_union_find.Link(55,56);
   N_union_find.Link(35,58);
   N_union_find.Link(35,36);

   N_union_find.Link(129,130);
   N_union_find.Link(128,127);
   N_union_find.Link(128,129);
   N_union_find.Link(127,130);
   N_union_find.Link(124,125);

   N_union_find.Link(119,120);
   N_union_find.Link(121,122);

   N_union_find.Link(134,133);

// Identify 3D vertices with common A coordinates:

   A_union_find.Link(0,5);
   A_union_find.Link(1,4);
   A_union_find.Link(6,9);
   A_union_find.Link(2,3);
   A_union_find.Link(7,8);
   A_union_find.Link(9,8);
   A_union_find.Link(8,24);

//   A_union_find.Link(0,1);
//   A_union_find.Link(1,2);
//   A_union_find.Link(2,3);
//   A_union_find.Link(3,4);
//   A_union_find.Link(4,5);
//   A_union_find.Link(5,6);
//   A_union_find.Link(6,7);
//   A_union_find.Link(7,8);
//   A_union_find.Link(8,9);
//   A_union_find.Link(8,24);

   A_union_find.Link(10,13);
   A_union_find.Link(12,13);
   A_union_find.Link(11,12);
   A_union_find.Link(10,11);

   A_union_find.Link(14,15);
   A_union_find.Link(16,17);
   A_union_find.Link(18,19);
   A_union_find.Link(14,16);
   A_union_find.Link(16,18);

   A_union_find.Link(79,80);
   A_union_find.Link(79,82);
   A_union_find.Link(80,81);
   A_union_find.Link(81,82);
   A_union_find.Link(83,84);
   A_union_find.Link(135,136);
   A_union_find.Link(136,137);
   A_union_find.Link(137,138);
   A_union_find.Link(138,139);
   A_union_find.Link(139,135);

   A_union_find.Link(20,21);
   A_union_find.Link(21,22);
   A_union_find.Link(61,62);

   A_union_find.Link(29,30);
   A_union_find.Link(25,26);
   A_union_find.Link(26,29);
   A_union_find.Link(27,28);
   A_union_find.Link(31,32);
   A_union_find.Link(33,34);
   A_union_find.Link(34,42);
   A_union_find.Link(37,42);
   A_union_find.Link(37,38);

   A_union_find.Link(39,44);
   A_union_find.Link(40,44);
   A_union_find.Link(40,43);
   A_union_find.Link(41,43);

   A_union_find.Link(87,88);
   
   A_union_find.Link(92,93);
   A_union_find.Link(90,91);
   A_union_find.Link(65,66);
   A_union_find.Link(63,64);
   A_union_find.Link(67,74);
   A_union_find.Link(73,68);
   A_union_find.Link(69,72);
   A_union_find.Link(70,71);
   A_union_find.Link(75,76);

   A_union_find.Link(94,95);
   A_union_find.Link(95,96);
   A_union_find.Link(96,97);
   A_union_find.Link(97,98);
   A_union_find.Link(98,99);
   A_union_find.Link(99,100);
   A_union_find.Link(100,101);

   A_union_find.Link(102,103);
   A_union_find.Link(103,104);
   A_union_find.Link(104,105);

   A_union_find.Link(106,107);
   A_union_find.Link(107,108);
   A_union_find.Link(108,109);
   A_union_find.Link(109,110);

   A_union_find.Link(111,112);
   A_union_find.Link(112,113);
   A_union_find.Link(113,114);
   A_union_find.Link(114,115);
   A_union_find.Link(115,116);
   A_union_find.Link(116,117);
   A_union_find.Link(117,118);

   A_union_find.Link(45,46);
   A_union_find.Link(46,47);
   A_union_find.Link(48,49);
   A_union_find.Link(49,50);

   A_union_find.Link(51,52);
   A_union_find.Link(52,53);
   A_union_find.Link(54,55);
   A_union_find.Link(55,56);
   A_union_find.Link(57,58);
   A_union_find.Link(58,35);
   A_union_find.Link(35,36);

   A_union_find.Link(131,132);
   A_union_find.Link(132,133);

   A_union_find.Link(123,124);
   A_union_find.Link(124,125);

// Identify 3D vertices with common Z coordinates:

   Z_union_find.Link(0,1);
   Z_union_find.Link(0,4);
   Z_union_find.Link(0,5);
   Z_union_find.Link(9,6);
   Z_union_find.Link(2,3);
   Z_union_find.Link(7,8);
   Z_union_find.Link(13,10);
   Z_union_find.Link(14,15);
   Z_union_find.Link(16,17);
   Z_union_find.Link(18,19);
   Z_union_find.Link(12,11);
   Z_union_find.Link(79,80);
   Z_union_find.Link(82,81);
   Z_union_find.Link(60,59);

   Z_union_find.Link(0,20);
   Z_union_find.Link(9,21);
   Z_union_find.Link(23,24);
   Z_union_find.Link(30,31);
   Z_union_find.Link(29,32);
   Z_union_find.Link(27,26);
   Z_union_find.Link(28,25);

   Z_union_find.Link(60,61);
   Z_union_find.Link(61,63);

   Z_union_find.Link(90,93);
   Z_union_find.Link(91,92);

   Z_union_find.Link(66,67);
   Z_union_find.Link(67,68);
   Z_union_find.Link(68,69);
   Z_union_find.Link(69,70);
   Z_union_find.Link(71,75);
//   Z_union_find.Link(76,77);
   
   Z_union_find.Link(65,74);

   Z_union_find.Link(38,39);
   Z_union_find.Link(40,37);
   Z_union_find.Link(43,42);
   Z_union_find.Link(41,34);
   Z_union_find.Link(33,36);
   Z_union_find.Link(34,35);

   Z_union_find.Link(44,45);
   Z_union_find.Link(40,46);
   Z_union_find.Link(47,41);
   Z_union_find.Link(45,48);
   Z_union_find.Link(46,49);
   Z_union_find.Link(50,47);

   Z_union_find.Link(85,86);
   Z_union_find.Link(100,117);
   Z_union_find.Link(115,98);
   Z_union_find.Link(114,97);
   Z_union_find.Link(113,96);
   Z_union_find.Link(112,95);
//   Z_union_find.Link(78,70);

   Z_union_find.Link(94,111);
   Z_union_find.Link(116,99);

   Z_union_find.Link(131,132);
   Z_union_find.Link(51,48);
   Z_union_find.Link(52,49);
   Z_union_find.Link(53,50);

   Z_union_find.Link(54,51);
   Z_union_find.Link(55,52);
   Z_union_find.Link(56,53);

   Z_union_find.Link(57,54);
   Z_union_find.Link(58,55);
   Z_union_find.Link(35,56);

   Z_union_find.Link(123,124);
   Z_union_find.Link(128,127);
   Z_union_find.Link(129,130);
   Z_union_find.Link(131,132);
   
   Z_union_find.Link(121,120);
   Z_union_find.Link(122,119);

   Z_union_find.Link(136,139);
   Z_union_find.Link(137,138);
   
   vector<int> Nvertex_IDs,Avertex_IDs,Zvertex_IDs;
   vector<int> Nvertex_parent_IDs,Avertex_parent_IDs,Zvertex_parent_IDs;
   N_union_find.get_all_nodes(Nvertex_IDs,Nvertex_parent_IDs);
   A_union_find.get_all_nodes(Avertex_IDs,Avertex_parent_IDs);
   Z_union_find.get_all_nodes(Zvertex_IDs,Zvertex_parent_IDs);

   typedef map<int,vector<double> > PARENT_VERTEX_COORDS_MAP;
// independent int = parent vertex ID
// dependent vector<double> = children vertices' coordinate values

   PARENT_VERTEX_COORDS_MAP parent_vertex_N_map,parent_vertex_A_map,
      parent_vertex_Z_map;
   PARENT_VERTEX_COORDS_MAP::iterator parent_Niter,parent_Aiter,parent_Ziter;

// Load parent_vertex_N[A](Z)_map with children vertices coordinate
// values:

   for (int i=0; i<Zvertex_IDs.size(); i++)
   {
      vertex_iter=vertex_map.find(Zvertex_IDs[i]);
      double curr_N=vertex_iter->second.back().get(0);
      double curr_A=vertex_iter->second.back().get(1);
      double curr_Z=vertex_iter->second.back().get(2);

      int curr_Nparent_ID=Nvertex_parent_IDs[i];
      int curr_Aparent_ID=Avertex_parent_IDs[i];
      int curr_Zparent_ID=Zvertex_parent_IDs[i];

//      cout << "i = " << i 
//           << " Z = " << curr_Z
//           << " curr_Zvertex_ID = " << Zvertex_IDs[i]
//           << " curr_Zparent_ID = " << curr_Zparent_ID << endl;
      
      parent_Niter=parent_vertex_N_map.find(curr_Nparent_ID);
      parent_Aiter=parent_vertex_A_map.find(curr_Aparent_ID);
      parent_Ziter=parent_vertex_Z_map.find(curr_Zparent_ID);

      if (parent_Niter==parent_vertex_N_map.end())
      {
         vector<double> children_N;
         children_N.push_back(curr_N);
         parent_vertex_N_map[curr_Nparent_ID]=children_N;
//         cout << "i = " << i << " curr_N = " << curr_N << endl;
      }
      else
      {
         parent_Niter->second.push_back(curr_N);
      }

      if (parent_Aiter==parent_vertex_A_map.end())
      {
         vector<double> children_A;
         children_A.push_back(curr_A);
         parent_vertex_A_map[curr_Aparent_ID]=children_A;
      }
      else
      {
         parent_Aiter->second.push_back(curr_A);
      }

      if (parent_Ziter==parent_vertex_Z_map.end())
      {
         vector<double> children_Z;
         children_Z.push_back(curr_Z);
         parent_vertex_Z_map[curr_Zparent_ID]=children_Z;
      }
      else
      {
         parent_Ziter->second.push_back(curr_Z);
      }
   } // loop over index i labeling vertex IDs

// Compute medians/averages of children vertex coordinate values for
// each parent vertex.  Then clear all children vertex coordinate STL
// vectors within parent_vertex_N[A](Z)_map and replace with single
// averaged coordinate value:

   for (parent_Niter=parent_vertex_N_map.begin(); 
        parent_Niter != parent_vertex_N_map.end(); parent_Niter++)
   {
      double parent_median_coord=mathfunc::median_value(parent_Niter->second);
//      double parent_average_coord=mathfunc::mean(parent_Niter->second);
      parent_Niter->second.clear();
      parent_Niter->second.push_back(parent_median_coord);
//      parent_Niter->second.push_back(parent_average_coord);
   }

   for (parent_Aiter=parent_vertex_A_map.begin(); 
        parent_Aiter != parent_vertex_A_map.end(); parent_Aiter++)
   {
      double parent_median_coord=mathfunc::median_value(parent_Aiter->second);
//      double parent_average_coord=mathfunc::mean(parent_Aiter->second);
      parent_Aiter->second.clear();
      parent_Aiter->second.push_back(parent_median_coord);
//      parent_Aiter->second.push_back(parent_average_coord);
   }

   for (parent_Ziter=parent_vertex_Z_map.begin(); 
        parent_Ziter != parent_vertex_Z_map.end(); parent_Ziter++)
   {
      double parent_median_coord=mathfunc::median_value(parent_Ziter->second);
//      double parent_average_coord=mathfunc::mean(parent_Ziter->second);
      parent_Ziter->second.clear();
      parent_Ziter->second.push_back(parent_median_coord);
//      parent_Ziter->second.push_back(parent_average_coord);
   }

// Replace vertices' N,A,Z coordinate values with their parents'
// median/averaged N,A,Z values:

   for (vertex_iter=vertex_map.begin(); vertex_iter != vertex_map.end();
        vertex_iter++)
   {
      int curr_vertex_ID=vertex_iter->first;
      int parent_vertex_ID=N_union_find.Find(curr_vertex_ID);

//      cout << "curr_vertex_ID = " << curr_vertex_ID
//           << " parent_vertex_ID = " << parent_vertex_ID
//           << endl;
      
      parent_Niter=parent_vertex_N_map.find(parent_vertex_ID);
      if (parent_Niter != parent_vertex_N_map.end())
      {
         double orig_vertex_N=vertex_iter->second.back().get(0);
         double avgd_vertex_N=parent_Niter->second.back();
         vertex_iter->second.back().put(0,avgd_vertex_N);
//         cout << "orig_vertex_N = " << orig_vertex_N 
//              << " avgd_vertex_N = " << avgd_vertex_N << endl;
      }
      
      parent_vertex_ID=A_union_find.Find(curr_vertex_ID);
      parent_Aiter=parent_vertex_A_map.find(parent_vertex_ID);
      if (parent_Aiter != parent_vertex_A_map.end())
      {
         double avgd_vertex_A=parent_Aiter->second.back();
         vertex_iter->second.back().put(1,avgd_vertex_A);
      }

      parent_vertex_ID=Z_union_find.Find(curr_vertex_ID);
      parent_Ziter=parent_vertex_Z_map.find(parent_vertex_ID);
      if (parent_Ziter != parent_vertex_Z_map.end())
      {
         double avgd_vertex_Z=parent_Ziter->second.back();
         vertex_iter->second.back().put(2,avgd_vertex_Z);
      }
   } // vertex_iter loop over vertex_map

   bool import_corrected_vertices_flag=true;
//   bool import_corrected_vertices_flag=false;

// Hardwired manual alterations for some contours:

   typedef map<string,threevector > CONTOUR_DELTAS_MAP;
// independent string = contour label
// dependent threevector = delta_n, delta_a, delta_z for contour

   CONTOUR_DELTAS_MAP contour_deltas_map;
   CONTOUR_DELTAS_MAP::iterator contour_deltas_iter;

   if (!import_corrected_vertices_flag)
   {
      contour_deltas_map["2"]=threevector(0,-0.25+1.04128,0);
      contour_deltas_map["3"]=threevector(0,-0.25+1.04128,0);
      contour_deltas_map["4"]=threevector(0,-0.25+1.04128,0);
   }
   
// Export corrected n,a,z vertex coordinates to output text file.
// Also export corrected TEL contours as polylines in XYZ coordinates.
// If flip_contours_flag==true, further export another set of
// polylines which have the same a and z but negative n vertex
// coordinates:

   string corrected_relative_vertices_filename=
      bundler_IO_subdir+"corrected_rel_TEL_vertices.dat";

   ofstream corrected_rel_vertices_stream;
   if (import_corrected_vertices_flag)
   {
      filefunc::ReadInfile(corrected_relative_vertices_filename);
      vector< vector<double> > row_column_values=filefunc::ReadInRowNumbers(
         corrected_relative_vertices_filename);

      vertex_map.clear();
      for (int r=0; r<row_column_values.size(); r++)
      {
         int contour_ID=row_column_values[r].at(0);
         int vertex_ID=row_column_values[r].at(1);
         double rel_n=row_column_values[r].at(2);
         double rel_a=row_column_values[r].at(3);
         double rel_z=row_column_values[r].at(4);

         threevector rel_vertex(rel_n,rel_a,rel_z);
//      cout << "rel_vertex = " << rel_vertex << endl;
      
         vertex_iter=vertex_map.find(vertex_ID);
         if (vertex_iter==vertex_map.end())
         {
            vector<threevector> rel_vertex_posns;
            rel_vertex_posns.push_back(rel_vertex);
            vertex_map[vertex_ID]=rel_vertex_posns;
         }
         else
         {
            vertex_iter->second.push_back(rel_vertex);
         }
      }
   }
   else
   {
      filefunc::openfile(corrected_relative_vertices_filename,
      corrected_rel_vertices_stream);
      corrected_rel_vertices_stream 
         << "# Contour_ID Feature ID	rel_n	rel_a 	rel_z" 
         << endl << endl;
   }
   
//   string contours_filename="aligned_TEL_contours.dat";
   string contours_filename=contours_subdir+
      "aligned_correctedNAZ_TEL_contours.dat";
   ofstream contour_stream;
   filefunc::openfile(contours_filename,contour_stream);
   contour_stream << "# Time   PolyLine_ID   Passnumber   X Y Z R G B A"
                  << endl << endl;

   threevector n_hat(-0.9798235497 , 0.1998644826 , 0);

   double phi_n=atan2(n_hat.get(1),n_hat.get(0));
   cout << "phi_n = " << phi_n*180/PI << endl;
   double delta_phi_n=0;
   phi_n += delta_phi_n;
   n_hat=threevector(cos(phi_n),sin(phi_n));

   threevector a_hat=z_hat.cross(n_hat);

   bool flip_contours_flag=true;
//   bool flip_contours_flag=false;
   int curr_time=0;
   int polyline_ID=0;
   int passnumber=0;
   for (contour_vertices_iter=contour_vertices_map.begin();
        contour_vertices_iter != contour_vertices_map.end();
        contour_vertices_iter++)
   {
      string contour_ID=contour_vertices_iter->first;

      threevector delta_naz=Zero_vector;
      contour_deltas_iter=contour_deltas_map.find(contour_ID);
      if (contour_deltas_iter != contour_deltas_map.end())
      {
         delta_naz=contour_deltas_iter->second;
         cout << "delta_naz = " << delta_naz << endl;
      }

      vector<int> vertex_IDs=contour_vertices_iter->second;

      vector<threevector> flipped_abs_vertex_posns;
      for (int v=0; v<=vertex_IDs.size(); v++)
      {
         int curr_vertex_ID=vertex_IDs[v%vertex_IDs.size()];
         vertex_iter=vertex_map.find(curr_vertex_ID);
         if (vertex_iter==vertex_map.end())
         {
            cout << "Error! vertex_iter=NULL" << endl;
            continue;
            exit(-1);
         }
         threevector rel_naz_vertex_posn=vertex_iter->second.back()+
            delta_naz;

         threevector abs_vertex_posn=
            rel_naz_vertex_posn.get(0)*n_hat+
            rel_naz_vertex_posn.get(1)*a_hat+
            rel_naz_vertex_posn.get(2)*z_hat+
            star_center;
         flipped_abs_vertex_posns.push_back(
            -rel_naz_vertex_posn.get(0)*n_hat+
            rel_naz_vertex_posn.get(1)*a_hat+
            rel_naz_vertex_posn.get(2)*z_hat+
            star_center);

         if (!import_corrected_vertices_flag)
         {
            corrected_rel_vertices_stream 
               << contour_ID << "   "
               << curr_vertex_ID << "   "
               << rel_naz_vertex_posn.get(0) << "  "
               << rel_naz_vertex_posn.get(1) << "  "
               << rel_naz_vertex_posn.get(2) 
               << endl;
         }
         
         contour_stream << curr_time << " "
                        << polyline_ID << " "
                        << passnumber << "   "
                        << abs_vertex_posn.get(0) << "  "
                        << abs_vertex_posn.get(1) << "  "
                        << abs_vertex_posn.get(2) << "   "
                        << "1 0 1 1" 
                        << endl;

      } // loop over index v labeling current contour's vertices

      if (!import_corrected_vertices_flag)
         corrected_rel_vertices_stream << endl;

      contour_stream << endl;

      if (flip_contours_flag)
      {
         for (int f=0; f<flipped_abs_vertex_posns.size(); f++)
         {
            contour_stream << curr_time << " "
                           << polyline_ID+10000 << " "
                           << passnumber << "   "
                           << flipped_abs_vertex_posns[f].get(0) << "  "
                           << flipped_abs_vertex_posns[f].get(1) << "  "
                           << flipped_abs_vertex_posns[f].get(2) << "   "
                           << "1 1 0 1" 
                           << endl;
         }
      }
      
      polyline_ID++;

   } // contour_vertices_iter loop over contour_vertices_map

   if (!import_corrected_vertices_flag)
      filefunc::closefile(corrected_relative_vertices_filename,
      corrected_rel_vertices_stream);

   filefunc::closefile(contours_filename,contour_stream);

   string banner="Exported "+corrected_relative_vertices_filename;
   outputfunc::write_banner(banner);
   banner="Exported "+contours_filename;
   outputfunc::write_banner(banner);
   

}
