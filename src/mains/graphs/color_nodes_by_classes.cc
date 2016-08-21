// ==========================================================================
// Program COLOR_NODES_BY_CLASSES is a quick-n-dirty attempt to reset
// node colorings within the zeroth level image graph based upon image
// class assignments.  It parses text file image_labels.dat which
// contains (image ID, class ID) pairs.  COLOR_NODES_BY_CLASSES then
// issues SQL update commands to nodes within a specified graph
// hierarchy which recolors them according to their class labels.
// ==========================================================================
// Last updated on 11/15/15
// ==========================================================================

#include <stdint.h>
#include <byteswap.h>
#include <iostream>
#include <string>
#include <vector>

#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "graphs/graphdbfuncs.h"
#include "graphs/graph_hierarchy.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::ifstream;
using std::ofstream;

int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   string bundle_filename=passes_group.get_bundle_filename();
//   cout << " bundle_filename = " << bundle_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(bundle_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

// Import image labels:

   string image_labels_filename=bundler_IO_subdir+"image_labels.dat";
   cout << "image_labels_filename = " << image_labels_filename << endl;
   vector<int> image_labels;
   filefunc::ReadInfile(image_labels_filename);
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];
      int curr_image_id, curr_image_label;
      stringfunc::string_to_two_numbers(curr_line, curr_image_id, 
                                        curr_image_label);
      image_labels.push_back(curr_image_label);
   }
   
   vector<int> hierarchy_IDs;
   vector<string> hierarchy_descriptions;
   graphdbfunc::retrieve_hierarchy_IDs_from_database(
      postgis_db_ptr,hierarchy_IDs,hierarchy_descriptions);

   cout << "Existing graph hierarchies within IMAGERY database:" << endl;
   cout << endl;
   for (unsigned int h=0; h<hierarchy_IDs.size(); h++)
   {
      cout << "Hierarchy ID = " << hierarchy_IDs[h]
           << "  hierarchy description = " << hierarchy_descriptions[h] 
           << endl;
   }
   cout << endl;

   int hierarchy_ID=-1;
   cout << "Enter graph hierarchy ID:" << endl;
   cin >> hierarchy_ID;
   if (hierarchy_ID < 0) exit(-1);

   int levelzero_graph_ID=0;
   int n_images = image_labels.size();
   for(int n = 0; n < n_images; n++)
   {
      outputfunc::update_progress_fraction(n, 250, n_images);
      int curr_node_ID = n;
      colorfunc::Color curr_color = colorfunc::get_color(image_labels[n]);
      colorfunc::RGB curr_node_RGB = colorfunc::get_RGB_values(curr_color);
      graphdbfunc::update_nodes_table(
         postgis_db_ptr, hierarchy_ID, levelzero_graph_ID,
         curr_node_ID, curr_node_RGB);
   }
}

