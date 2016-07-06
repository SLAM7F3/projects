// ==========================================================================
// Program SUBDIVIDE interpolates XYZP point clouds and their
// corresponding normal vectors onto finer meshes.  It first voxelizes
// all the points and stores nonempty voxel locations within a
// hashtable.  It next instantiates a KDtree with all the XYZP
// information and uses it to rapidly locate nearby neighbors to each
// XYZP point.  Working with both the hashtable and KDtree, SUBDIVIDE
// adds extra points into the cloud and takes their dependent p-values
// and normal vectors to equal the average of their progenitors'.  It
// writes the enlarged, interpolated point and normal clouds to two
// XYZP files.
// ==========================================================================
// Last updated on 3/5/06; 6/18/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "kdtree/kdtreefuncs.h"
#include "geometry/mybox.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "threeDgraphics/voxel_lattice.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string subdir="./xyzp_files/";
   string input_xyzp_filename="output1.xyzp";
   outputfunc::newline();
   cout << "Enter input XYZP point cloud file to subdivide:" << endl;
   cin >> input_xyzp_filename;
   input_xyzp_filename=subdir+input_xyzp_filename;
   vector<fourvector>* XYZP_ptr=xyzpfunc::read_xyzp_float_data(
      input_xyzp_filename);

// In order to generate subdivided normals file, we need to
// temporarily replace p values with integer indices labeling points
// in cloud:

   vector<double>* prob_ptr=new vector<double>;
   for (int i=0; i<XYZP_ptr->size(); i++)
   {
      fourvector curr_XYZP=(*XYZP_ptr)[i];
      double curr_prob=curr_XYZP.get(3);
      if (curr_prob < 0) curr_prob=-1;
//      prob_ptr->push_back( curr_XYZP.get(3) );
      prob_ptr->push_back( curr_prob );
      ((*XYZP_ptr)[i]).put(3,i);
   }

   string input_normals_filename="normals_isar0.xyzp";
   outputfunc::newline();
   cout << "Enter input normals file to subdivide:" << endl;
   cin >> input_normals_filename;
   input_normals_filename=subdir+input_normals_filename;
   vector<fourvector>* normals_ptr=xyzpfunc::read_xyzp_float_data(
      input_normals_filename);

// Instantiate voxel_lattice to keep track of occupied & empty voxels:

   outputfunc::write_banner("Instantiating voxel lattice:");

   voxel_lattice voxels;
   voxels.set_delta(threevector(0.001,0.001,0.001));
   voxels.initialize(XYZP_ptr);
   voxels.fill_lattice(XYZP_ptr);
   
   outputfunc::write_banner("Creating KDtree from original XYZP points:");

   KDTree::KDTree<3, fourvector>* xyz_kdtree_ptr=
      kdtreefunc::generate_3D_kdtree(*XYZP_ptr);
   cout << "Original KDtree size = " << xyz_kdtree_ptr->size() << endl;

   outputfunc::write_banner("Generating new XYZP points from old ones:");

   unsigned int n_closest_nodes=9;
   double rho=0.02;	// meter
   vector<fourvector> closest_node;

   vector<fourvector>* new_XYZP_ptr=new vector<fourvector>;
   vector<fourvector>* new_normals_ptr=new vector<fourvector>;

   for (int i=0; i<xyz_kdtree_ptr->size(); i++)
   {
      if (i%1000==0) cout << i << " " << flush;

      fourvector curr_xyzp=(*XYZP_ptr)[i];
      int curr_index=basic_math::round(curr_xyzp.get(3));
      fourvector curr_normal=(*normals_ptr)[curr_index];

      curr_xyzp.put(3,(*prob_ptr)[curr_index]);
      new_XYZP_ptr->push_back(curr_xyzp);
      new_normals_ptr->push_back(curr_normal);

      closest_node.clear();
      kdtreefunc::find_closest_nodes(
         xyz_kdtree_ptr,curr_xyzp,rho,n_closest_nodes,closest_node);
//      cout << "i = " << i << " curr_xyzp = " << curr_xyzp << endl;
      for (int j=0; j<n_closest_nodes; j++)
      {
         int closest_node_index=basic_math::round(closest_node[j].get(3));
         float closest_node_prob=(*prob_ptr)[closest_node_index];

         float curr_prob=curr_xyzp.get(3);
         float avg_prob=-1;
         if (closest_node_prob >= 0 && curr_prob >= 0)
         {
            avg_prob=0.5*(closest_node_prob+curr_xyzp.get(3));
         }
         else if (closest_node_prob < 0 && curr_prob >= 0)
         {
            avg_prob=curr_prob;
         }
         else if (closest_node_prob >= 0 && curr_prob < 0)
         {
            avg_prob=closest_node_prob;
         }
//         float avg_prob=0.5*(closest_node_prob+curr_xyzp.get(3));

         fourvector avg_xyzp=0.5*(closest_node[j]+curr_xyzp);
         avg_xyzp.put(3,avg_prob);

         if (voxels.empty_voxel(avg_xyzp))
         {
            new_XYZP_ptr->push_back(avg_xyzp);
            voxels.increment_voxel_counts(avg_xyzp);

            fourvector closest_node_normal=(*normals_ptr)[closest_node_index];
            fourvector avg_normal=0.5*(curr_normal+closest_node_normal);
            new_normals_ptr->push_back(avg_normal);
         }
      } // loop over index j labeling nearby neighbors of current XYZP point
   } // loop over index i labeling XYZP points from input ISAR file
   outputfunc::newline();
   delete XYZP_ptr;
   delete normals_ptr;

   string output_xyzp_filename="subdivided_cloud.xyzp";
   filefunc::deletefile(output_xyzp_filename);
   xyzpfunc::write_xyzp_data(output_xyzp_filename,new_XYZP_ptr,false);
   delete new_XYZP_ptr;

   string output_normals_filename="subdivided_normals.xyzp";
   filefunc::deletefile(output_normals_filename);
   xyzpfunc::write_xyzp_data(output_normals_filename,new_normals_ptr,false);
   delete new_normals_ptr;
}
