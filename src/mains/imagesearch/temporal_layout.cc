// ========================================================================
// Program TEMPORAL_LAYOUT reads in a set of node and image IDs as
// well as epoch times from the IMAGERY database for some
// user-specified graph hierarchy.  It generates a cosine layout for
// (gx2,gy2) graph node coordinates which can serve as a reasonable
// timeline display. TEMPORAL_LAYOUT exports gx2,gy2 coordinates to
// ascii output as well as updates gx2,gy2 columns in the nodes table
// of the IMAGERY database.  It also exports a SQL script which
// contains time stamp annotations that label the sinusoidal graph
// layout.

// 	Chant this to run program: temporal_layout --GIS_layer ./packages/imagery_metadata.pkg 

// ========================================================================
// Last updated on 8/27/12; 8/28/12; 2/24/13
// ========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   bool modify_IMAGERY_database_flag=true;
//   bool modify_IMAGERY_database_flag=false;
   if (modify_IMAGERY_database_flag)
   {
      cout << "modify_IMAGERY_database_flag = true" << endl;
   }
   else
   {
      cout << "modify_IMAGERY_database_flag = false" << endl;
   }
   outputfunc::enter_continue_char();

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

   int hierarchy_ID=-1;
   cout << "Enter ID for graph hierarchy:" << endl;
   cin >> hierarchy_ID;
   if (hierarchy_ID < 0) exit(-1);

   unsigned int n_graphs,n_levels,n_connected_components;
   string hierarchy_description;
   graphdbfunc::retrieve_graph_hierarchy_metadata_from_database(
      postgis_db_ptr,hierarchy_ID,hierarchy_description,n_graphs,n_levels,
      n_connected_components);
   cout << "n_connected_components = " << n_connected_components
        << endl;
   
   int campaign_ID,mission_ID;
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_db_ptr,hierarchy_ID,campaign_ID,mission_ID);
   cout << "campaign_ID = " << campaign_ID
        << " mission_ID = " << mission_ID << endl;

   cout.precision(12);
   int graph_ID=0;
   vector<int> image_IDs,node_IDs;
   vector<double> epoch_times;
   imagesdatabasefunc::retrieve_image_metadata_from_database(
      postgis_db_ptr,hierarchy_ID,graph_ID,
      node_IDs,image_IDs,epoch_times);

   int n_images=epoch_times.size();
   cout << "n_images = " << n_images << endl;

   templatefunc::Quicksort(epoch_times,node_IDs,image_IDs);

   double min_time=0;
   int i=0;
   while (min_time < 1000)
   {
      min_time=epoch_times[i++];
   }
//   double min_time=epoch_times.front();
   double max_time=epoch_times.back();

   Clock clock;
   clock.convert_elapsed_secs_to_date(min_time);
   string min_date_string=clock.YYYY_MM_DD_H_M_S();
   cout << "min_time = " << min_time << " = " << min_date_string
        << endl;

   clock.convert_elapsed_secs_to_date(max_time);
   string max_date_string=clock.YYYY_MM_DD_H_M_S();
   cout << "max_time = " << max_time << " = " << max_date_string 
        << endl;

   double time_interval=max_time-min_time;
   cout << "max_time - min_time = " << time_interval << " secs = "
        << time_interval/60.0 << " mins  = " 
        << time_interval/3600.0 << " hours = " 
        << time_interval/(24*3600.0) << " days" 
        << endl;

// Force Period to assume canonical time intervals:

   double time_interval_in_days=time_interval/(24*3600);
   double time_interval_in_months=time_interval/(24*3600*30);
   double time_interval_in_years=time_interval/(24*3600*365);
   cout << "time in days = " << time_interval_in_days << endl;
   cout << "time in months = " << time_interval_in_months << endl;
   cout << "time in years = " << time_interval_in_years << endl;

   double Period=-1;	// secs
   
	if (time_interval_in_years > 10){
		Period=24*3600*365*5; // 5 years
	} 
	else if (time_interval_in_years > 2)
	{
		Period=24*3600*365;	// 1 year
   	}
   	else if (time_interval_in_months > 2)
   		{
		Period=24*3600*30;	// 1 month
   	}	
   	else if (time_interval_in_days > 2)
   	{
   		Period=24*3600;		// 1 day
   	}
   	else
   	{
      double median_time=mathfunc::median_value(epoch_times);
      cout << "median_time = " << median_time << endl;
      int median_bin=mathfunc::binary_locate(
         epoch_times,0,n_images-1,median_time);
      cout << "median_bin = " << median_bin << endl;
   
      const int n_images_per_period=200;
      cout << "n_images_per_period = " << n_images_per_period << endl;
      int period_lo_bin=median_bin-n_images_per_period;
      cout << "period_lo_bin = " << period_lo_bin << endl;
      int period_hi_bin=median_bin+n_images_per_period;
      cout << "period_hi_bin = " << period_hi_bin << endl;
      double t_period_lo=epoch_times[period_lo_bin];
      cout << "t_period_lo = " << t_period_lo << endl;
      double t_period_hi=epoch_times[period_hi_bin];
      cout << "t_period_hi = " << t_period_hi << endl << endl;
      Period=0.5*(t_period_hi-t_period_lo);	// secs
   }
/* For Boston Bombing videos, 
// for Period = 1 minute:

   Period=60;	// secs = 1 minute

   cout << "Period = " << Period << " secs = "
        << Period/60.0 << " mins = " 
        << Period/3600.0 << " hours = " 
        << Period/(24*3600.0) << " days" << endl;
*/

   double omega=2*PI/Period;

   double n_Periods=(max_time-min_time)/Period;
   cout << "n_Periods = " << n_Periods << endl << endl;
   int n_complete_Periods=basic_math::mytruncate(n_Periods);

   double horizontal_stretch_factor=1;
   if (Period > 3600)	// 1 hours
   {
      horizontal_stretch_factor *= 6;
   }
   else if (Period > 10*3600)	// 10 hours
   {
      horizontal_stretch_factor *= 18;
   }
   else if (Period > 3*24*3600)	// 3 days
   {
      horizontal_stretch_factor *= 48;
   }
   else if (Period > 30*24*3600)  // 30 days
   {
      horizontal_stretch_factor *= 96;
   }

   const double delta_gx_per_connected_component=
      1.5*horizontal_stretch_factor;
//   double gx_extent=delta_gx_per_connected_component*n_connected_components;
   double gx_extent=delta_gx_per_connected_component*(5*n_Periods);
   double gy_extent=1;
   double delta_gx_per_Period=gx_extent/n_Periods;
   cout << "gx_extent = " << gx_extent 
        << " gy_extent = " << gy_extent << endl;

// Store node_IDs as function of time within STL map:

   typedef map<int,int> NODE_ID_INDEX_MAP;
// indepedent int = node_ID
// dependent int = node index (counter)   

   typedef map<double,NODE_ID_INDEX_MAP* > TIME_MULTIPLICITIES_MAP;

// independent double var = epoch time
// dependent var holds STL map of node indices vs node IDs

   TIME_MULTIPLICITIES_MAP time_multiplicities_map;
   TIME_MULTIPLICITIES_MAP::iterator iter;

   for (int i=0; i<n_images; i++)
   {
      double t=epoch_times[i];
      iter=time_multiplicities_map.find(t);
      if (iter==time_multiplicities_map.end())
      {
         NODE_ID_INDEX_MAP* node_id_index_map_ptr=new NODE_ID_INDEX_MAP;
         (*node_id_index_map_ptr)[node_IDs[i]]=0;
         time_multiplicities_map[t]=node_id_index_map_ptr;
      }
      else
      {
         NODE_ID_INDEX_MAP* node_id_index_map_ptr=iter->second;
         int node_index=node_id_index_map_ptr->size();
         (*node_id_index_map_ptr)[node_IDs[i]]=node_index;
      }
   }
//   cout << "time_multiplicities_map.size() = "
//        << time_multiplicities_map.size() << endl;

   int counter=0;
   int max_multiplicity=0;
   for (iter=time_multiplicities_map.begin(); iter != 
           time_multiplicities_map.end(); iter++)
   {
      double t=iter->first;
      NODE_ID_INDEX_MAP* node_id_index_map_ptr=iter->second;
      int curr_multiplicity=node_id_index_map_ptr->size();
      max_multiplicity=basic_math::max(max_multiplicity,curr_multiplicity);
      cout << "counter = " << counter++ << " t = " << t
           << " n_nodes = " << curr_multiplicity << endl;
   }
   cout << "max_multiplicity = " << max_multiplicity << endl;

   bool sinusoid_layout_flag=true;
   if (max_multiplicity > 10)
   {
      sinusoid_layout_flag=false;
   }
//   outputfunc::enter_continue_char();

   string temporal_filename="temporal_layout.dat";
   ofstream temporal_stream;
   temporal_stream.precision(12);
   filefunc::openfile(temporal_filename,temporal_stream);
   temporal_stream << "# index  NodeID  t_renorm gx'  gy'" << endl;
   temporal_stream << endl;

   string SQL_timestamp_annotation_filename="insert_timestamp_annotations.sql";
   ofstream SQL_timestamp_annotation_stream;
   filefunc::openfile(
      SQL_timestamp_annotation_filename,SQL_timestamp_annotation_stream);

   vector<string> update_SQL_commands;
   for (int i=0; i<n_images; i++)
   {
      if (i%100==0) cout << i << " " << flush;

      if (node_IDs[i] < 0) continue;

      double t=epoch_times[i];
      double t_renorm=t-min_time;

      double new_gx=
         0+(t-min_time)/(max_time-min_time)*gx_extent;

      double new_gy;
      if (sinusoid_layout_flag)
      {
         const double A=1;
         new_gy=A*cos(omega*t_renorm);
      }
      else
      {
         iter=time_multiplicities_map.find(t);
         NODE_ID_INDEX_MAP* node_id_index_map_ptr=iter->second;
         NODE_ID_INDEX_MAP::iterator node_iter=node_id_index_map_ptr->
            find(node_IDs[i]);
         int node_index=node_iter->second;
         new_gy=node_index;
      }

// Reset temporal graph coordinates for any node whose epoch time
// is less than min_time!

      if (t < min_time) 
      {
         new_gx=new_gy=-1;
      }

      temporal_stream 
         << i << "  "
         << node_IDs[i] << "  "
         << t_renorm << "  "
         << new_gx << "  "
         << new_gy << endl;

      if (modify_IMAGERY_database_flag)
      {
         update_SQL_commands.push_back(
            graphdbfunc::generate_update_node_SQL_command(
               hierarchy_ID,graph_ID,node_IDs[i],new_gx,new_gy));
      }
   } // loop over index i labeling images

   if (modify_IMAGERY_database_flag)
   {
      postgis_db_ptr->set_SQL_commands(update_SQL_commands);
      postgis_db_ptr->execute_SQL_commands();
   }

// Periodically add time-stamp label annotations into timeline graph:

   for (int p=0; p<n_complete_Periods+1; p++)
   {
      double gx=p*delta_gx_per_Period;
      double gy=-1;
      double t=min_time+p*Period;
      clock.convert_elapsed_secs_to_date(t);
      string date_string=clock.YYYY_MM_DD_H_M_S();

      if (time_interval_in_days > 2)
      {
         date_string=date_string.substr(0,10);	// yyyy-mm-dd
      }
      else
      {
         date_string += " UTC";
      }
    
// Display full date and time information within every 4th label.
// Otherwise, display just UTC time:

//
//      if (p%4 != 0)
//      {
//         vector<string> substrings=
//            stringfunc::decompose_string_into_substrings(date_string);
//         date_string=substrings.back();
//      }
//

      cout << "p = " << p
           << " date_string = " << date_string
           << endl;

      int level=graph_ID;
      int layout=1;	// timeline graph display
      string color="white";
      double annotation_size=1;
      string SQL_command=
         graphdbfunc::generate_insert_graph_annotation_SQL_command(
            hierarchy_ID,graph_ID,level,layout,
            gx,gy,date_string,color,annotation_size);
      SQL_timestamp_annotation_stream << SQL_command << endl;
   } // look over index p labeling complete Periods

   cout << endl;
   filefunc::closefile(temporal_filename,temporal_stream);
   filefunc::closefile(
      SQL_timestamp_annotation_filename,SQL_timestamp_annotation_stream);

   string banner="Exported temporal layout to "+temporal_filename;
   outputfunc::write_big_banner(banner);

   banner="Exported timestamp annotations to SQL script "
      +SQL_timestamp_annotation_filename;
   outputfunc::write_big_banner(banner);
}
