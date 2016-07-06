// ==========================================================================
// Program GENERATE_MOVIE is a playground for developing video
// generation on demand.

//			run_generate_movie

// ==========================================================================
// Last updated on 10/19/11; 10/20/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "postgres/databasefuncs.h"
#include "general/filefuncs.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"


using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

// Instantiate gis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_database_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(passes_group,GISlayer_IDs);

   double start_time=1305162754;
   double stop_time=start_time+20;

// Retrieve FLIR imagery times and filename prefixes from Tstorm
// database corresponding to specified time interval:

   vector<double> epoch_time;
   vector<string> filename_stem;

   databasefunc::retrieve_aircraft_metadata_from_database(
      postgis_database_ptr,start_time,stop_time,
      epoch_time,filename_stem);

   string input_imagery_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/thunderstorm/video_data/20110511/flight1/";
   string image_suffix="jpg";
   int AVI_movie_counter=0;
   string output_movie_filename_prefix="movie";
   string finished_movie_subdir="./";

   videofunc::generate_FLIR_AVI_movie(
      input_imagery_subdir,image_suffix,AVI_movie_counter,
      output_movie_filename_prefix,finished_movie_subdir,
      start_time,stop_time,epoch_time,filename_stem);


} 

