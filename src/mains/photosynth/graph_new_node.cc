// ========================================================================
// Program GRAPH_NEW_NODE displays the topology of an n+1st node
// within the graph for n previously matched images.  It first
// generates the photogroup for the n images from the contents of the
// photo table wtihin the data_network database.  GRAPH_NEW_NODE then
// reads in the name and pixel size of the n+1st photo from files in
// bundler/individual_photo/.  After reconstructing the previously
// calculated graph hierarchy from the contents of node and link
// tables within data_network database, this program adds the n+1st
// node to the child graph.  The new node's position in the child
// graph is based upon those of the nodes with which it has SIFT
// overlap.  Finally, GRAPH_NEW_NODE generates a JSON file for the
// modified child graph which can be displayed via Michael Yee's graph
// viewers.


/*

./graph_new_node \
--region_filename ./bundler/MIT2317/packages/peter_inputs.pkg \
--GIS_layer ./bundler/MIT2317/packages/graph_nodes.pkg


*/

// ========================================================================
// Last updated on 5/31/10; 6/8/10; 1/18/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "graphs/graph.h"
#include "graphs/graph_edge.h"
#include "graphs/graphfuncs.h"
#include "graphs/graph_hierarchy.h"
#include "templates/mytemplates.h"
#include "passes/PassesGroup.h"
#include "video/photodbfuncs.h"
#include "video/photogroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   
   string new_bundler_IO_subdir="./bundler/individual_photo/";

   string base_dirname=filefunc::getbasename(bundler_IO_subdir);
   string base_URL="http://127.0.0.1:8080/photo/images/";
   base_URL += base_dirname+"images/";
//   cout << "base_URL = " << base_URL << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_database_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_database_ptr << endl;

// First regenerate bundler photogroup from contents of photo table in
// data_network database:

   photogroup* bundler_photogroup_ptr=
      photodbfunc::generate_photogroup_from_database(
         postgis_database_ptr,bundler_IO_subdir);
   bundler_photogroup_ptr->set_base_URL(base_URL);

// Read in name of n+1st photo from image_list_filename:

   string new_image_list_filename=new_bundler_IO_subdir+"image_list.dat";
//   cout << "new_image_list_filename = " << new_image_list_filename << endl;
   filefunc::ReadInfile(new_image_list_filename);
   string new_image_filename=new_bundler_IO_subdir+filefunc::text_line[0];
//   cout << "new_image_filename = " << new_image_filename << endl;
   string new_image_basename=filefunc::getbasename(new_image_filename);
   string new_thumbnail_filename=new_bundler_IO_subdir+
      "images/thumbnails/thumbnail_"+new_image_basename;
//   cout << "new_thumbnail_filename = " << new_thumbnail_filename << endl;

// In order to run Michael Yee's GraphExplorer, we need to copy the
// new image and its thumbnail into appropriate subdirectories of
// apache-tomcat/webapps/photo/images:

   string tomcat_subdir="/usr/local/apache-tomcat/webapps/photo/images/";
   tomcat_subdir += filefunc::getbasename(new_bundler_IO_subdir);
   tomcat_subdir += "images/";
   filefunc::dircreate(tomcat_subdir);

   string unix_cmd="cp "+new_image_filename+" "+tomcat_subdir;
   sysfunc::unix_command(unix_cmd);

   string tomcat_thumbnails_subdir = tomcat_subdir+"thumbnails/";
   filefunc::dircreate(tomcat_thumbnails_subdir);

// For SIGMA demo purposes, we prefer to see the full rather than
// thumbnail version of the n+1st photo in Michael Yee's GraphExplorer
// tool.  So simply copy the former onto the latter in the appropriate
// tomcat subdirectory:

   unix_cmd="cp "+new_image_filename+" "
      +tomcat_thumbnails_subdir+"/thumbnail_"+new_image_basename;
   sysfunc::unix_command(unix_cmd);

//   unix_cmd="cp "+new_thumbnail_filename+" "+tomcat_thumbnails_subdir;
//   sysfunc::unix_command(unix_cmd);

// Read in size of n+1st photo from image_sizes_filename:

   string new_image_sizes_filename=new_bundler_IO_subdir+"image_sizes.dat";
//   cout << "new_image_sizes_filename = " << new_image_sizes_filename 
//        << endl;
   filefunc::ReadInfile(new_image_sizes_filename);
   vector<double> id_xdim_ydim=
      stringfunc::string_to_numbers(filefunc::text_line[0]);
   int new_xdim=id_xdim_ydim[1];
   int new_ydim=id_xdim_ydim[2];
//   cout << "new_xdim = " << new_xdim << " new_ydim = " << new_ydim << endl;

   int new_photo_ID=bundler_photogroup_ptr->get_n_photos();
   bool parse_exif_metadata_flag=true;
   photograph* new_photograph_ptr=new photograph(
      new_image_filename,new_xdim,new_ydim,new_photo_ID,
      parse_exif_metadata_flag); 
   bundler_photogroup_ptr->add_node(new_photograph_ptr);

   string new_base_dirname=filefunc::getbasename(new_bundler_IO_subdir);
   string new_photo_URL="http://127.0.0.1:8080/photo/images/";
   new_photo_URL += new_base_dirname+"images/"+new_image_basename;
   new_photograph_ptr->set_URL(new_photo_URL);
//   cout << "new_photo_URL = " << new_photo_URL << endl;

// Next reconstruct graph hierarchy from contents of node and link
// tables within data_network database:

   graph_hierarchy* graphs_pyramid_ptr=
      graphfunc::generate_graph_hierarchy_from_database(postgis_database_ptr);

// Extract level-0 "child graph" and compute its nodes'
// center-of-mass:

   graph* child_graph_ptr=graphs_pyramid_ptr->get_graph_ptr(0);
   twovector node_COM=child_graph_ptr->node_COM();
   twovector node_sigma=child_graph_ptr->node_std_dev_from_COM();
//   cout << "node_COM = " << node_COM << endl;
//   cout << "node_SIGMA = " << node_sigma << endl;

// Instantiate node for n+1st input photo & add to child graph:

   int new_node_ID=child_graph_ptr->get_max_node_ID()+1;
//   cout << "new_node_ID = " << new_node_ID << endl;
   node* new_node_ptr=new node(new_node_ID,0);
   child_graph_ptr->add_node(new_node_ptr);
//   cout << "child_graph_ptr->get_n_nodes() = "
//        << child_graph_ptr->get_n_nodes() << endl;

// Instantiate edges between n+1st node and n nodes.  Compute weighted
// average of (gx,gy) positions for nodes which are connected via
// edges to n+1st node relative to COM.  Set n+1st node's position
// equal to weighted average:

   double denom=0;
   twovector numer(0,0);
   
   string new_node_edgelist_filename=new_bundler_IO_subdir+"edgelist_n+1.dat";
   filefunc::ReadInfile(new_node_edgelist_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> nnw=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      int prev_node_ID=nnw.at(1);
      double weight=nnw.at(2);
      node* prev_node_ptr=child_graph_ptr->get_node_ptr(prev_node_ID);
      twovector prev_node_posn(prev_node_ptr->get_Uposn(),
                               prev_node_ptr->get_Vposn());
      twovector curr_nhat=(prev_node_posn-node_COM).unitvector();
      numer += weight*curr_nhat;
      denom += weight;

      child_graph_ptr->add_graph_edge(new_node_ptr,prev_node_ptr,weight);
   }

// Set new node's radius wrt node COM sufficiently large so that it
// stands out against previous "n" photo nodes:

   double radius=2.5*node_sigma.magnitude();
//   cout << "Radius = " << radius << endl;
   twovector new_node_posn=node_COM+radius*((numer/denom).unitvector());
//   cout << "new_node_posn = " << new_node_posn << endl;

   new_node_ptr->set_data_ID(new_photo_ID);
   new_node_ptr->set_posn(new_node_posn);
   new_node_ptr->set_relative_size(3);
   new_node_ptr->set_node_RGB(colorfunc::get_RGB_values(colorfunc::white));

   photodbfunc::export_JSON_files(
      *graphs_pyramid_ptr,bundler_photogroup_ptr,new_bundler_IO_subdir);
}
