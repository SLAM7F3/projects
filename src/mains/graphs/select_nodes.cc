// ========================================================================
// Program SELECT_NODES takes in some text file containing node IDs
// for some particular photo set of interest (e.g. nodes selected via Michael
// Yee's graph tool).  It queries the user to enter a graph ID.
// SELECT_NODES then retrieves the URLs corresponding to the input
// node IDs.  It outputs a unix command file filled with link commands
// from an unknown subdirectory containing all input photo filenames
// to the current subdirectory.

// We wrote SELECT_NODES in order to generate input lists of images
// for bundler derived from manual node selection via Michael's graph
// tool.

/*

select_nodes --region_filename ./bundler/MIT2317/packages/peter_inputs.pkg --GIS_layer ./Qt/packages/TOC_metadata_remote_LLAN.pkg

*/

// ========================================================================
// Last updated on 7/27/11; 7/28/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "graphs/graph_hierarchy.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "video/photodbfuncs.h"
#include "video/photogroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate gis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_database_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

// Read in text file containing node IDs for particular photo sets:

   string nodes_filename="MIT32K_nodes1";
   cout << "Enter input filename containing node IDs:" << endl;
   cin >> nodes_filename;
   filefunc::ReadInfile(nodes_filename);

   vector<int> node_IDs;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      node_IDs.push_back(stringfunc::string_to_number(filefunc::text_line[i]));
   }

   int graph_ID=-1;
   cout << "Enter graph ID:" << endl;
   cin >> graph_ID;

   photodbfunc::PHOTO_IDS_METADATA_MAP* photo_ids_metadata_map_ptr=
      photodbfunc::retrieve_photo_URLs_vs_node_IDs(
         postgis_database_ptr,graph_ID);

   string output_filename="link_"+nodes_filename;
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   
   int n_missing_photos=0;
   for (unsigned int i=0; i<node_IDs.size(); i++)
   {
      int node_ID=node_IDs[i];
      photodbfunc::PHOTO_IDS_METADATA_MAP::iterator photo_iter=
         photo_ids_metadata_map_ptr->find(node_ID);
      if (photo_iter==photo_ids_metadata_map_ptr->end()) 
      {
         n_missing_photos++;
         continue;
      }

      string URL=photo_iter->second.at(0);
//      cout << "node_ID = " << node_ID
//           << " URL = " << URL << endl;
      string photo_basename=filefunc::getbasename(URL);
      string ln_cmd="ln -s ../subdir/"+photo_basename+" .";
      outstream << ln_cmd << endl;
   }
   cout << "n_missing_photos = " << n_missing_photos << endl;
   
   filefunc::closefile(output_filename,outstream);
}
