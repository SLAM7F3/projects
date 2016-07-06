// ========================================================================
// Program FINDROUTE

// 				findroute

// ========================================================================
// Last updated on 12/1/10
// ========================================================================

#include <iostream>
#include <string>
#include "graphs/graph.h"
#include "graphs/graphfuncs.h"
#include "graphs/node.h"
#include "image/raster_parser.h"
#include "image/terrainfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{
   
   string geotif_filename="mountains_cropped.tif";
   
   raster_parser RasterParser;
   RasterParser.open_image_file(geotif_filename);

   int channel_ID=0;
   RasterParser.fetch_raster_band(channel_ID);

   twoDarray* ztwoDarray_ptr=RasterParser.get_ztwoDarray_ptr();
   cout << "ztwoDarray_ptr = " << ztwoDarray_ptr << endl;
   int mdim=ztwoDarray_ptr->get_mdim();
   int ndim=ztwoDarray_ptr->get_ndim();
   RasterParser.read_raster_data(ztwoDarray_ptr);

   int skip=20;
   double alpha=10;
   graph* graph_ptr=terrainfunc::generate_DTED_graph(
      ztwoDarray_ptr,alpha,skip);

   int px_start=0+2*skip;
   int py_start=0+2*skip;
   int ID_start=terrainfunc::px_py_to_node_ID(mdim,px_start,py_start);
   node* start_node_ptr=graph_ptr->get_node_ptr(ID_start);
   start_node_ptr->set_distance_from_start(0);
   cout << "start_node_ptr = " << start_node_ptr << endl;

   int px_stop=0+(mdim/skip-2)*skip;
   int py_stop=0+(ndim/skip-2)*skip;
   int ID_stop=terrainfunc::px_py_to_node_ID(mdim,px_stop,py_stop);
   node* stop_node_ptr=graph_ptr->get_node_ptr(ID_stop);
   cout << "stop_node_ptr = " << stop_node_ptr << endl;

   graph_ptr->Dijkstra(start_node_ptr);
   terrainfunc::print_shortest_path(stop_node_ptr,ztwoDarray_ptr);

   delete graph_ptr;

}
