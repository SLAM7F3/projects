// ==========================================================================
// Treefuncs namespace method definitions
// ==========================================================================
// Last modified on 7/27/05; 6/18/06; 7/29/06; 4/5/14
// ==========================================================================

#include <set>
#include <vector>
#include "image/connectfuncs.h"
#include "ladar/featurefuncs.h"
#include "kdtree/kdtreefuncs.h"
#include "network/Networkfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "urban/tree_cluster.h"
#include "urban/treefuncs.h"
#include "image/TwoDarray.h"
#include "urban/urbanfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::istringstream;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

namespace treefunc
{
   
   Network<tree_cluster*>* trees_network_ptr=NULL;

// ==========================================================================

// Method generate_trees_network takes in a cleaned features map
// within *features_twoDarray_ptr.  This method first generates a
// connected binary components hashtable whose entries correspond to
// linked lists of individual treetop pixels.  It subsequently
// dynamically generates *trees_network_ptr whose entries correspond
// to tree clusters.  Each tree cluster is assigned a unique ID
// number, and its center-of-mass position is saved within its own
// members.  A copy of the linked list of treetop pixels is also saved
// within each of the tree sites.

Network<tree_cluster*>* generate_trees_network(
   string imagedir,double min_footprint_area,twoDarray const *ztwoDarray_ptr,
   twoDarray const *features_twoDarray_ptr)
{
   outputfunc::write_banner("Generating trees network:");
   string feature_type="tree";
   Hashtable<linkedlist*>* connected_components_hashtable_ptr=
      featurefunc::extract_feature_clusters(
         min_footprint_area,featurefunc::tree_sentinel_value,feature_type,
         imagedir,ztwoDarray_ptr,features_twoDarray_ptr);
   Network<tree_cluster*>* curr_trees_network_ptr=
      Networkfunc::generate_network<tree_cluster>(
      ztwoDarray_ptr,connected_components_hashtable_ptr);

   urbanfunc::generate_network_contours(
      ztwoDarray_ptr,curr_trees_network_ptr);
   connectfunc::delete_connected_hashtable(
      connected_components_hashtable_ptr);
   
   return curr_trees_network_ptr;
}

// ---------------------------------------------------------------------
// Method delete_trees_network loops over all dynamically allocated
// trees within input *curr_trees_network_ptr and deletes them.  It
// then deletes the input trees network itself.

   void delete_trees_network(
      Network<tree_cluster*>* curr_trees_network_ptr)
      {
         Networkfunc::delete_dynamically_allocated_objects_in_network(
            curr_trees_network_ptr);
         delete curr_trees_network_ptr;
      }

// ---------------------------------------------------------------------
// Method draw_tree_cluster_pixels loops over all sites within
// *trees_network_ptr.  It draws the pixels for each tree cluster onto
// output twoDarrays *ztree_twoDarray_ptr and *ftree_twoDarray_ptr.

   void draw_tree_cluster_pixels(
      Network<tree_cluster*> const *curr_trees_network_ptr,
      twoDarray const *ztwoDarray_ptr,twoDarray* ztree_twoDarray_ptr,
      twoDarray* ftree_twoDarray_ptr)
      {
         ztree_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
         ftree_twoDarray_ptr->initialize_values(xyzpfunc::null_value);
   
         for (const Mynode<int>* currnode_ptr=curr_trees_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int n=currnode_ptr->get_data();
            linkedlist* pixel_list_ptr=
               curr_trees_network_ptr->get_site_data_ptr(n)->
               get_pixel_list_ptr();

            if (pixel_list_ptr != NULL)
            {
               mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
         
               while (curr_pixel_ptr != NULL)
               {
                  int px=curr_pixel_ptr->get_data().get_var(0);
                  int py=curr_pixel_ptr->get_data().get_var(1);
                  ztree_twoDarray_ptr->put(px,py,ztwoDarray_ptr->get(px,py));
                  ftree_twoDarray_ptr->put(
                     px,py,featurefunc::tree_sentinel_value);
                  curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
               }
            } // pixel_list_ptr != NULL conditional
         } // loop over clusters in tree network
      }

// ==========================================================================
// Trees network text input/output methods:
// ==========================================================================

// Method output_trees_network_to_textfile loops over all nodes within
// the input *tree_network_ptr.

   void output_trees_network_to_textfile(
      Network<tree_cluster*> const *curr_tree_network_ptr,
      string output_filename)
      {
         outputfunc::write_banner(
            "Writing trees network to output textfile:");
         
         ofstream outstream;
         filefunc::deletefile(output_filename);
         filefunc::openfile(output_filename,outstream);

         for (Mynode<int> const *currnode_ptr=curr_tree_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // tree cluster number
            output_treecluster(curr_tree_network_ptr,r,outstream);
         }
         filefunc::closefile(output_filename,outstream);
      }

// ---------------------------------------------------------------------
// Method output_treecluster takes in ID label r for some entry in the
// trees network.  It writes to an output text file information about
// this tree cluster.

   void output_treecluster(
      Network<tree_cluster*> const *curr_tree_network_ptr,int r,
      ofstream& outstream)
      {
         tree_cluster* curr_treecluster_ptr=
            curr_tree_network_ptr->get_site_data_ptr(r);

         outstream << r << endl;
         threevector treecluster_posn(curr_treecluster_ptr->get_posn());
         outstream << treecluster_posn.get(0) << " "
                   << treecluster_posn.get(1) << " "
                   << treecluster_posn.get(2) << " " << endl;
//         curr_treecluster_ptr->get_posn().write_to_textstream(outstream);
         contour* curr_contour_ptr=curr_treecluster_ptr->get_contour_ptr();
         curr_contour_ptr->write_to_textstream(outstream);

         outstream << endl;
      }

// ---------------------------------------------------------------------
// Method readin_trees_network_from_textfile performs the inverse
// operation of method output_tree_network_to_textfile.  It
// dynamically generates a tree network based upon the information
// read in from a text file generated by
// output_trees_network_to_textfile.

   Network<tree_cluster*>* readin_trees_network_from_textfile(
      string input_filename,bool vertices_lie_in_plane)
      {
         outputfunc::write_banner("Reading in trees network from text file:");
         vector<string> line;
         filefunc::ReadInfile(input_filename,line);
         stringfunc::comment_strip(line);

         Network<tree_cluster*>* curr_tree_network_ptr=
            new Network<tree_cluster*>(10*line.size());

         unsigned int i=0;
         threevector r_posn;
         while (i<line.size())
         {
            int r=stringfunc::string_to_number(line[i++]);
            istringstream input_string_stream(line[i++]);
            double x,y,z;
            input_string_stream >> x;
            input_string_stream >> y;
            input_string_stream >> z;
            r_posn=threevector(x,y,z);
//            r_posn.read_from_text_lines(i,line);
            tree_cluster* curr_treecluster_ptr=new tree_cluster(r,r_posn);

// For neighbor drawing purposes, we set each treepoint's "center"
// equal to its "position":

            curr_treecluster_ptr->set_center(r_posn);
            curr_tree_network_ptr->insert_site(
               r,Site<tree_cluster*>(curr_treecluster_ptr));

            contour* curr_contour_ptr=new contour;
            curr_contour_ptr->read_from_text_lines(
               i,line,vertices_lie_in_plane);
            curr_tree_network_ptr->get_site_data_ptr(r)->set_contour_ptr(
               curr_contour_ptr);

         } // i < line.size() conditional
         return curr_tree_network_ptr;
      }

// ---------------------------------------------------------------------
// Method generate_tree_network_kdtree takes in trees network
// *curr_tree_network_ptr.  It generates a KDtree holding contour
// vertex information for each tree cluster within the network.  This
// method returns a pointer to the dynamically generated KDtree.

   KDTree::KDTree<3, threevector>* generate_tree_network_kdtree(
      Network<tree_cluster*> const *curr_tree_network_ptr)
      {
         outputfunc::write_banner("Generating KDtree for tree network:");
         
         vector<threevector> V;
         for (Mynode<int> const *currnode_ptr=curr_tree_network_ptr->
                 get_entries_list_ptr()->get_start_ptr(); 
              currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
         {
            int r=currnode_ptr->get_data(); // tree cluster number
            tree_cluster* curr_treecluster_ptr=
               curr_tree_network_ptr->get_site_data_ptr(r);
            contour* curr_contour_ptr=curr_treecluster_ptr->get_contour_ptr();
            for (unsigned int n=0; n<curr_contour_ptr->get_nvertices(); n++)
            {
               V.push_back(curr_contour_ptr->get_vertex(n).first);
            }
//            cout << "cluster r = " << r 
//                 << " V.size() = " << V.size() << endl;
         } // loop over nodes in tree network
         KDTree::KDTree<3, threevector>* kdtree_ptr=
            kdtreefunc::generate_3D_kdtree(V);

         cout << "kdtree.size = " << kdtree_ptr->size() << endl;
//         cout << "KD tree = " << *kdtree_ptr << endl;
         
         return kdtree_ptr;
      }

// ---------------------------------------------------------------------
// Method tree_points_near_input_location takes in some point xy_pnt
// within the XY plane as well as 2D radius rho.  It loops over all
// tree contour points within input *kdtree_ptr.  This method fills
// and returns STL vector nearby_tree_points with those tree contour
// points that lie within a 2D bounding box centered upon xy_pnt of
// size +/- rho.

   void tree_points_near_input_location(
      const threevector& xy_pnt,double rho,
      KDTree::KDTree<3, threevector> const *kdtree_ptr,
      vector<threevector>& nearby_tree_points)
      {
         nearby_tree_points.clear();

         const int upper_dims_to_ignore=1;
         kdtree_ptr->find_within_range(
            xy_pnt,rho,std::back_inserter(nearby_tree_points),
            upper_dims_to_ignore);
      }

} // treefunc namespace

