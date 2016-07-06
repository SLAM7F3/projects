// ========================================================================
// Program SPATIAL_LAYOUT
// ========================================================================
// Last updated on 2/27/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int hierarchy_ID=5;	// Tstorm
//   cout << "Enter graph hierarchy ID which contains FLIR video frames:"
//        << endl;
//   cin >> hierarchy_ID;

   int graph_ID=0;

   int n_connected_components=8;   // Tstorm
//   cout << "Enter number of connected graph components:" << endl;
//   cin >> n_connected_components;
   

   string backprojected_image_posns_filename="backprojected_image_posns.dat";
   filefunc::ReadInfile(backprojected_image_posns_filename);

   cout.precision(12);
   vector<int> node_IDs,datum_IDs;
   vector<double> backprojected_eastings,backprojected_northings;

   int i_max=filefunc::text_line.size();
//   i_max=500;
   
   for (int i=0; i<i_max; i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      int campaign_ID=column_values[0];
      int mission_ID=column_values[1];
      int image_ID=column_values[2];
      int datum_ID=imagesdatabasefunc::get_datum_ID(
         postgis_db_ptr,campaign_ID,mission_ID,image_ID);
      
      double easting=column_values[3];
      double northing=column_values[4];

      int node_ID;
      if (!graphdbfunc::retrieve_node_ID_for_particular_hierarchy_datum(
         postgis_db_ptr,hierarchy_ID,graph_ID,datum_ID,node_ID))
      {
//         cout << "Node doesn't exist" << endl;
         continue;
      }

      cout << "i = " << i
           << " datumID = " << datum_ID
           << " nodeID = " << node_ID
           << " easting = " << easting
           << " northing = " << northing 
           << endl;

      datum_IDs.push_back(datum_ID);
      node_IDs.push_back(node_ID);
      backprojected_eastings.push_back(easting);
      backprojected_northings.push_back(northing);
      
   } // loop over index i labeling lines in backprojected_image_posns file
   
   double min_easting=mathfunc::minimal_value(backprojected_eastings);
//   double mean_easting=mathfunc::mean(backprojected_eastings);
   double max_easting=mathfunc::maximal_value(backprojected_eastings);
   double easting_extent=max_easting-min_easting;

   double min_northing=mathfunc::minimal_value(backprojected_northings);
//   double mean_northing=mathfunc::mean(backprojected_northings);
   double max_northing=mathfunc::maximal_value(backprojected_northings);
   double northing_extent=max_northing-min_northing;

   double gx_extent=1.5*n_connected_components;
   double gy_extent=gx_extent*northing_extent/easting_extent;

   string spatial_filename="spatial_layout.dat";
   ofstream spatial_stream;
   filefunc::openfile(spatial_filename,spatial_stream);
   spatial_stream << "# DatumID  NodeID  gx'  gy'" << endl;
   spatial_stream << endl;
   
   for (unsigned int i=0; i<backprojected_eastings.size(); i++)
   {
      double easting=backprojected_eastings[i];
      double northing=backprojected_northings[i];

      double new_gx=
         0+(easting-min_easting)/(max_easting-min_easting)*gx_extent;
      double new_gy=
         (0+(northing-min_northing)/(max_northing-min_northing)*gy_extent)
         -0.5*gy_extent;

      spatial_stream 
         << datum_IDs[i] << "  "
         << node_IDs[i] << "  "
         << new_gx << "  "
         << new_gy << endl;

      graphdbfunc::update_nodes_table(
         postgis_db_ptr,hierarchy_ID,graph_ID,node_IDs[i],new_gx,new_gy);
      
   }
   filefunc::closefile(spatial_filename,spatial_stream);

}
