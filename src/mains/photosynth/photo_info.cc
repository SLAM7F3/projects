// ========================================================================
// Program PHOTO_INFO is a testing grounds for querying the
// data_network database for MIT2317 photo metadata.  It generates 3
// JSON files: photo_geoposns.JSON lists lon-lat coords for each of
// the 2317 MIT photos; photo_metadata.JSON lists photo metadata for
// some specified input photo; photo_neighbors.JSON lists the node IDs
// for all sibling neighbors of some specified node.


/*

./photo_info \
--region_filename ./bundler/MIT2317/packages/peter_inputs.pkg \
--GIS_layer ./bundler/MIT2317/packages/graph_nodes.pkg


*/

// ========================================================================
// Last updated on 6/11/10; 6/19/10; 6/20/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "postgres/gis_databases_group.h"
#include "graphs/graph.h"
#include "graphs/graph_edge.h"
#include "graphs/graphfuncs.h"
#include "graphs/graph_hierarchy.h"
#include "graphs/jsonfuncs.h"
#include "templates/mytemplates.h"
#include "passes/PassesGroup.h"
#include "video/photodbfuncs.h"
#include "video/photogroup.h"
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

// Instantiate gis database objects to send data to and retrieve
// data from external Postgres database:

   gis_databases_group* gis_databases_group_ptr=new gis_databases_group;
   gis_database* gis_database_ptr=gis_databases_group_ptr->
      generate_gis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

   vector<int> photo_ID,photo_ID_copy1,photo_ID_copy2;
   vector<int> photo_importance,npx,npy,thumbnail_npx,thumbnail_npy;
   vector<string> image_filenames,photo_timestamp,photo_URL,thumbnail_URL;
   vector<double> zposn,azimuth,elevation,roll,focal_param,
      longitude,latitude;
   
   photodbfunc::retrieve_photo_metadata_from_database(
      gis_database_ptr,bundler_IO_subdir,
      photo_ID,photo_importance,npx,npy,thumbnail_npx,thumbnail_npy,
      image_filenames,photo_timestamp,photo_URL,thumbnail_URL,
      zposn,azimuth,elevation,roll,focal_param,longitude,latitude);

   string JSON_filename="photo_geoposns.JSON";
   photodbfunc::write_geolocation_JSON_file(
      photo_ID,longitude,latitude,JSON_filename);

   int requested_photo_ID=200;
   cout << "Enter requested photo ID:" << endl;
   cin >> requested_photo_ID;
   JSON_filename="photo_metadata.JSON";
   photodbfunc::write_metadata_JSON_file(
      requested_photo_ID,photo_ID,npx,npy,image_filenames,photo_timestamp,
      longitude,latitude,zposn,azimuth,elevation,roll,JSON_filename);

   vector<int> neighbor_IDs=jsonfunc::retrieve_node_neighbors_from_database(
      gis_database_ptr,requested_photo_ID);

   JSON_filename="photo_neighbors.JSON";
   jsonfunc::write_neighbors_JSON_file(
      requested_photo_ID,neighbor_IDs,JSON_filename);

}
