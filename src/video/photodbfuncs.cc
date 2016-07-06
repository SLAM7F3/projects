// ==========================================================================
// Photodbfuncs namespace method definitions
// ==========================================================================
// Last modified on 7/27/11; 10/20/11; 4/3/14; 4/5/14
// ==========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "postgres/gis_database.h"
#include "graphs/graph.h"
#include "graphs/graphdbfuncs.h"
#include "graphs/graph_edge.h"
#include "graphs/graph_hierarchy.h"
#include "graphs/jsonfuncs.h"
#include "math/mathfuncs.h"
#include "messenger/Messenger.h"
#include "templates/mytemplates.h"
#include "graphs/node.h"
#include "general/outputfuncs.h"
#include "video/photodbfuncs.h"
#include "video/photogroup.h"
#include "video/sift_feature.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "track/track.h"
#include "track/tracks_group.h"
#include "math/twovector.h"
#include "video/videofuncs.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

namespace photodbfunc
{

// ==========================================================================
// JSON file export methods
// ==========================================================================

// Method retrieve_photo_metadata_from_database() takes in
// *gis_database_ptr which is assumed to contain a "data_network"
// database with a "photo" table.  It extracts all rows and columns
// from this database table.  After sorting the extracted rows
// according to photo_ID, this method fills output STL vectors with
// photo metadata.

      void retrieve_photo_metadata_from_database(
         gis_database* gis_database_ptr,string bundler_IO_subdir,
         vector<int>& photo_ID,vector<int>& photo_importance,
         vector<int>& npx,vector<int>& npy,
         vector<int>& thumbnail_npx,vector<int>& thumbnail_npy,
         vector<string>& image_filenames,vector<string>& photo_timestamp,
         vector<string>& photo_URL,vector<string>& thumbnail_URL,
         vector<double>& zposn,vector<double>& azimuth,
         vector<double>& elevation,vector<double>& roll,
         vector<double>& focal_param,
         vector<double>& longitude,vector<double>& latitude)
         {
//            cout << "inside photodbfunc::retrieve_photo_metadata_from_database() #2" 
//                 << endl;

            string curr_select_command = 
               "SELECT id,importance,time_stamp,url,npx,npy,";
            curr_select_command += 
               "thumbnail_url,thumbnail_npx,thumbnail_npy,z_posn,";
            curr_select_command += 
               "azimuth,elevation,roll,focal_param,";
            curr_select_command += 
               "x(xy_posn) as longitude,y(xy_posn) as latitude ";
            curr_select_command += "from photo";

            Genarray<string>* field_array_ptr=gis_database_ptr->
               select_data(curr_select_command);
            if (field_array_ptr==NULL) return;

            vector<int> photo_ID_copy1,photo_ID_copy2;
            for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
            {
               int curr_ID=stringfunc::string_to_number(
                  field_array_ptr->get(i,0));
               int curr_importance=stringfunc::string_to_number(
                  field_array_ptr->get(i,1));
               string curr_timestamp=field_array_ptr->get(i,2);   
               string curr_URL=field_array_ptr->get(i,3);
               int curr_npx=stringfunc::string_to_number(
                  field_array_ptr->get(i,4));
               int curr_npy=stringfunc::string_to_number(
                  field_array_ptr->get(i,5));

               string curr_thumbnail_URL=field_array_ptr->get(i,6);
               int curr_thumbnail_npx=stringfunc::string_to_number(
                  field_array_ptr->get(i,7));
               int curr_thumbnail_npy=stringfunc::string_to_number(
                  field_array_ptr->get(i,8));

               double curr_z=stringfunc::string_to_number(
                  field_array_ptr->get(i,9));
               double curr_az=stringfunc::string_to_number(
                  field_array_ptr->get(i,10));
               double curr_el=stringfunc::string_to_number(
                  field_array_ptr->get(i,11));
               double curr_roll=
                  stringfunc::string_to_number(field_array_ptr->get(i,12));
               double curr_f=
                  stringfunc::string_to_number(field_array_ptr->get(i,13));
               double curr_lon=
                  stringfunc::string_to_number(field_array_ptr->get(i,14));
               double curr_lat=
                  stringfunc::string_to_number(field_array_ptr->get(i,15));

               photo_ID.push_back(curr_ID);
               photo_ID_copy1.push_back(curr_ID);
               photo_ID_copy2.push_back(curr_ID);
               photo_importance.push_back(curr_importance);
               npx.push_back(curr_npx);
               npy.push_back(curr_npy);
               photo_timestamp.push_back(curr_timestamp);
               photo_URL.push_back(curr_URL);
               thumbnail_URL.push_back(curr_thumbnail_URL);
               thumbnail_npx.push_back(curr_thumbnail_npx);
               thumbnail_npy.push_back(curr_thumbnail_npy);
      
               zposn.push_back(curr_z);
               azimuth.push_back(curr_az);
               elevation.push_back(curr_el);
               roll.push_back(curr_roll);
               focal_param.push_back(curr_f);
               longitude.push_back(curr_lon);
               latitude.push_back(curr_lat);

               string filename=filefunc::getbasename(curr_URL);
               image_filenames.push_back(bundler_IO_subdir+filename);

/*
               cout << "i = " << i << " ID = " << photo_ID.back()
                    << " importance = " << photo_importance.back()
                    << " npx = " << npx.back()
                    << " npy = " << npy.back() << endl;
               cout << "  filename = " << image_filenames.back() << endl;
               cout << "  timestamp = " << photo_timestamp.back()
                    << " URL = " << photo_URL.back() << endl;
               cout << "  thumbnail_URL = " << thumbnail_URL.back()
                    << " thumbnail_npx = " << thumbnail_npx.back()
                    << " thumbnail_npy = " << thumbnail_npy.back() << endl;
               cout << "  zposn = " << zposn.back()
                    << " az = " << azimuth.back()
                    << " el = " << elevation.back()
                    << " roll = " << roll.back()
                    << " f = " << focal_param.back() << endl;
               cout << "  lon = " << longitude.back()
                    << " lat = " << latitude.back() << endl;
               cout << endl;
*/

            } // loop over index i labeling database rows

// Recall database retrieval is NOT guaranteed to be sorted!  So in
// order to maintain consistency with Noah's original photo indexing
// conventions, we need to sort all values extracted from the database
// w.r.t photo_ID and its copies:
   
            templatefunc::Quicksort(
               photo_ID,photo_importance,npx,npy,image_filenames,
               photo_timestamp,photo_URL);
            templatefunc::Quicksort(
               photo_ID_copy1,thumbnail_URL,thumbnail_npx,thumbnail_npy,
               zposn);
            templatefunc::Quicksort(
               photo_ID_copy2,azimuth,elevation,roll,focal_param,
               longitude,latitude);
         }
      
// --------------------------------------------------------------------------
// Method generate_photogroup_from_database() takes in
// *gis_database_ptr which is assumed to contain a "data_network"
// database with a "photo" table.  It extracts all rows and columns
// from this database table.  After sorting the extracted rows
// according to photo_ID, this method dynamically generates a
// photogroup and fills it with image filenames and photo pixel sizes.

      photogroup* generate_photogroup_from_database(
         gis_database* gis_database_ptr,string bundler_IO_subdir)
         {
            vector<int> photo_ID,photo_ID_copy1,photo_ID_copy2;
            vector<int> photo_importance,npx,npy,thumbnail_npx,thumbnail_npy;
            vector<string> image_filenames,photo_timestamp,photo_URL,
               thumbnail_URL;
            vector<double> zposn,azimuth,elevation,roll,focal_param,
               longitude,latitude;

            retrieve_photo_metadata_from_database(
               gis_database_ptr,bundler_IO_subdir,
               photo_ID,photo_importance,npx,npy,thumbnail_npx,thumbnail_npy,
               image_filenames,photo_timestamp,photo_URL,thumbnail_URL,
               zposn,azimuth,elevation,roll,focal_param,
               longitude,latitude);

            photogroup* photogroup_ptr=new photogroup();
            photogroup_ptr->generate_bundler_photographs(
               image_filenames,npx,npy);
            return photogroup_ptr;
         }

// --------------------------------------------------------------------------
      void export_JSON_files(
         graph_hierarchy& graphs_pyramid,photogroup* photogroup_ptr,
         string bundler_IO_subdir)
         {
            cout << "inside photodbfunc::export_JSON_files()" << endl;

// In order to run Michael Yee's GraphExplorer, we need to copy output
// JSON files into the appropriate subdirectory of
// apache-tomcat/webapps/photo/graphs:

            string tomcat_subdir=
               "/usr/local/apache-tomcat/webapps/photo/graphs/";
            tomcat_subdir += filefunc::getbasename(bundler_IO_subdir);
            filefunc::dircreate(tomcat_subdir);

// Write all graphs within hierarchy to output JSON files:

            for (unsigned int l=0; l<graphs_pyramid.get_n_levels(); l++)
            {
               string json_filename=bundler_IO_subdir+
                  "graph_level_"+stringfunc::number_to_string(l)+".json";
//               cout << "json_filename = " << json_filename << endl;

               write_graph_json_file(
                  graphs_pyramid.get_graph_ptr(l),photogroup_ptr,
                  json_filename);
               string unix_cmd="cp "+json_filename+" "+tomcat_subdir;
//               cout << "unix_cmd = " << unix_cmd << endl;
               sysfunc::unix_command(unix_cmd);

// Copy level-0 JSON file onto graph.json within webapp/photo/graphs
// subdir so that it appears by default when Michael Yee's viewer is
// run via call to run_graphjson:

               if (l==0)
               {
                  unix_cmd="cp "+json_filename+" "
                     +tomcat_subdir+"/graph.json";
                  sysfunc::unix_command(unix_cmd);
               }
            } // loop over index l labeling graph levels
         }

// --------------------------------------------------------------------------
      void write_graph_json_file(graph* graph_ptr,photogroup* photogroup_ptr,
                                 string json_filename)
         {
//            cout << "inside photodbfunc::write_graph_json_file()" << endl;
//            cout << "photogroup_ptr = " << photogroup_ptr << endl;

            string graph_ID="Graph";
            string edge_default="undirected";

            string value="\n";
            value="{ \n";
            value += "  \"graph\": { \n";

            value +=  jsonfunc::output_GraphML_key_value_pair(
               5,"id",graph_ID);
   
            value += "     \"edgedefault\": \"";
            value += edge_default;
            value += "\", \n";

// Write out nodes:

            cout << "Writing out nodes:" << endl;
            value += "     \"node\": [ \n";

            bool terminal_node_flag=false;
            unsigned int n_nodes=graph_ptr->get_n_nodes();
//            cout << "n_nodes = " << n_nodes << endl;
            for (unsigned int n=0; n<n_nodes; n++)
            {
               node* node_ptr=graph_ptr->get_ordered_node_ptr(n);
               if (n==graph_ptr->get_n_nodes()-1)
                  terminal_node_flag=true;

               int data_ID=node_ptr->get_data_ID();
//               cout << "Data_ID = " << data_ID << endl;
               photograph* photograph_ptr=photogroup_ptr->
                  get_photograph_ptr(data_ID);
               value += output_node_GraphML(
                  9,node_ptr,photograph_ptr,terminal_node_flag);
            } // loop over index n labeling graph nodes
            value += "    ], \n";

// Write out edges:

            cout << "Writing out edges:" << endl;
            graph_ptr->compute_edge_weights_distribution();

            value += "    \"edge\": [ \n";

            bool terminal_edge_flag=false;
            int n_edges=graph_ptr->get_n_graph_edges();
            int edge_counter=0;

            for (unsigned int e=0; e<graph_ptr->get_n_graph_edges(); e++)
            {
               graph_edge* graph_edge_ptr=
                  graph_ptr->get_ordered_graph_edge_ptr(e);
               int curr_matches=graph_edge_ptr->get_weight();

               edge_counter++;
               if (edge_counter==n_edges) terminal_edge_flag=true;
               if (curr_matches > 0)
               {
                  node* node1_ptr=graph_edge_ptr->get_node1_ptr();
                  node* node2_ptr=graph_edge_ptr->get_node2_ptr();
         
                  colorfunc::RGB edge_RGB=graph_ptr->
                     compute_edge_color(curr_matches);

                  double relative_edge_thickness=1;
                  if (graph_ptr->get_level()==1)
                  {
                     relative_edge_thickness=2;
                  }
                  else if (graph_ptr->get_level() >= 2)
                  {
                     relative_edge_thickness=3;
                  }
                  value += jsonfunc::output_edge_GraphML(
                     9,node1_ptr->get_ID(),node2_ptr->get_ID(),
                     curr_matches,edge_RGB.first,edge_RGB.second,
                     edge_RGB.third,
                     relative_edge_thickness,terminal_edge_flag);
               }
            } // loop over index e labeling graph edges

            value += "    ] \n";

            value += "  } \n";
            value += "} \n";

            jsonfunc::write_json_file(json_filename,value);
         }

// ---------------------------------------------------------------------
// Method output_node_GraphML()

      string output_node_GraphML(
         int n_indent,node* node_ptr,photograph* photograph_ptr,
         bool terminal_node_flag)
         {
//            cout << "inside photodbfunc::output_node_GraphML()" << endl;
//            cout << "photograph_ptr = " << photograph_ptr << endl;

            string node_value=jsonfunc::indent_spaces(n_indent-2);
            node_value += "{ \n";

            node_value += jsonfunc::output_GraphML_key_value_pair(
               n_indent,"id",stringfunc::number_to_string(
                  node_ptr->get_ID() ));

            int time_stamp=0;
            string photo_URL="";
            unsigned int xdim=0;
            unsigned int ydim=0;
            string thumbnail_URL="";
            unsigned int thumbnail_xdim=0;
            unsigned int thumbnail_ydim=0;

            if (photograph_ptr != NULL)
            {
               time_stamp=photograph_ptr->get_clock().
                  secs_elapsed_since_reference_date();
               string photo_filename=photograph_ptr->get_filename();
               if (photo_filename.size() > 0)
               {
                  photo_URL = photograph_ptr->get_URL();
                  thumbnail_URL = 
                     filefunc::getdirname(photograph_ptr->get_URL())
                     +"thumbnails/"+filefunc::getbasename(
                        videofunc::get_thumbnail_filename(
                           photograph_ptr->get_filename()));
//                  cout << "photo_URL = " << photo_URL << endl;
//                  cout << "thumbnail_URL = " << thumbnail_URL << endl;
//                  outputfunc::enter_continue_char();
               }

               xdim=photograph_ptr->get_xdim();
               ydim=photograph_ptr->get_ydim();
               videofunc::get_thumbnail_dims(
                  xdim,ydim,thumbnail_xdim,thumbnail_ydim);
            } // photograph_ptr != NULL conditional

            int parent_ID=node_ptr->get_parent_ID();
            colorfunc::RGB node_RGB=node_ptr->get_node_RGB();
//            cout << "parent_ID = " << parent_ID << endl;
//            cout << "node_RGB = " << node_RGB << endl;

            node_value += jsonfunc::output_data_GraphML(
               n_indent,"NODE",time_stamp,photo_URL,xdim,ydim,
               thumbnail_URL,thumbnail_xdim,thumbnail_ydim,
               -1,parent_ID,node_ptr->get_children_node_IDs(),
               node_ptr->get_Uposn(),node_ptr->get_Vposn(),
               node_RGB.first,node_RGB.second,node_RGB.third,
               node_ptr->get_relative_size());

            node_value += jsonfunc::indent_spaces(n_indent-2);
            node_value += "}";
            if (!terminal_node_flag) node_value += ",";
            node_value += "\n";

//            cout << "node_value = " << node_value << endl;
            return node_value;
         }

// --------------------------------------------------------------------------
      void write_geolocation_JSON_file(
         vector<int>& photo_ID,vector<double>& longitude,
         vector<double>& latitude,string json_filename)
         {
            string value=generate_geolocation_JSON_string(
               photo_ID,longitude,latitude);
            jsonfunc::write_json_file(json_filename,value);
         }

// --------------------------------------------------------------------------
// Method generate_geolocation_JSON_string() takes in photo IDs along
// with reconstructed lon-lat geocoords extracted to STL vectors via a
// gis database call.  It generates a JSON string containing this
// geocoordinate information in the form of key-value pairs.
      
      string generate_geolocation_JSON_string(
         vector<int>& photo_ID,vector<double>& longitude,
         vector<double>& latitude)
         {
//            cout << "inside photodbfunc::generate_geolocation_JSON_string()" << endl;

            string value="\n";
            value="{ \n";
            value += "  \"Node_Geolocations\": { \n";

// Write out nodes:

            cout << "Writing out nodes:" << endl;
            value += "     \"node\": [ \n";

            bool terminal_node_flag=false;
            unsigned int n_nodes=photo_ID.size();
//            cout << "n_nodes = " << n_nodes << endl;
            for (unsigned int n=0; n<n_nodes; n++)
            {
               if (n==n_nodes-1) terminal_node_flag=true;
               value += output_geolocation_GraphML(
                  9,photo_ID[n],longitude[n],latitude[n],terminal_node_flag);
            } // loop over index n labeling graph nodes

            value += "    ] \n";

            value += "  } \n";
            value += "} \n";

            return value;
         }

// --------------------------------------------------------------------------
// Method output_geolocation_GraphML()

      string output_geolocation_GraphML(
         unsigned int n_indent,int photo_ID,double longitude,double latitude,
         bool terminal_node_flag)
         {
//            cout << "inside photodbfunc::output_geolocation_GraphML()" << endl;

            string node_value=jsonfunc::indent_spaces(n_indent-2);
            node_value += "{ \n";

            node_value += jsonfunc::output_GraphML_key_value_pair(
               n_indent,"id",stringfunc::number_to_string(photo_ID));
            node_value += jsonfunc::output_geolocation_GraphML(
               n_indent,longitude,latitude);
            node_value += jsonfunc::indent_spaces(n_indent-2);
            node_value += "}";
            if (!terminal_node_flag) node_value += ",";
            node_value += "\n";

//            cout << "node_value = " << node_value << endl;
            return node_value;
         }

// --------------------------------------------------------------------------

      void write_metadata_JSON_file(
         int requested_photo_ID,
         vector<int>& photo_ID,vector<int>& npx,vector<int>& npy,
         vector<string>& image_filenames,vector<string>& image_timestamps,
         vector<double>& longitude,vector<double>& latitude,
         vector<double>& zposn,vector<double>& azimuth,
         vector<double>& elevation,vector<double>& roll,string json_filename)
         {
//            cout << "inside photodbfunc::write_meta_JSON_file()" << endl;
            string value=generate_metadata_JSON_string(
               requested_photo_ID,
               photo_ID,npx,npy,image_filenames,image_timestamps,
               longitude,latitude,zposn,azimuth,elevation,roll);
            jsonfunc::write_json_file(json_filename,value);
         }
      
// --------------------------------------------------------------------------
// Method generate_metadata_JSON_string() takes in some requested
// photo ID along with photo metadata extracted to several STL vectors
// via a gis database call.  It generates a JSON string containing
// metadata for the requested photo in the form of key-value pairs.

      string generate_metadata_JSON_string(
         int requested_photo_ID,
         vector<int>& photo_ID,vector<int>& npx,vector<int>& npy,
         vector<string>& image_filenames,vector<string>& image_timestamps,
         vector<double>& longitude,vector<double>& latitude,
         vector<double>& zposn,vector<double>& azimuth,
         vector<double>& elevation,vector<double>& roll)
         {
//            cout << "inside photodbfunc::generate_metadata_JSON_string()" << endl;
            string value;
            if (requested_photo_ID < 0) return value;

            value="\n";
            value="{ \n";
            value += "  \"Node_Metadata\": { \n";

// Write out metadata for requested node:

            value += "     \"node\": [ \n";

            int n=mathfunc::mylocate(photo_ID,requested_photo_ID);
            bool terminal_node_flag=true;

            value += output_metadata_GraphML(
               9,photo_ID[n],npx[n],npy[n],image_filenames[n],
               image_timestamps[n],longitude[n],latitude[n],zposn[n],
               azimuth[n],elevation[n],roll[n],terminal_node_flag);
            
            value += "    ] \n";
            value += "  } \n";
            value += "} \n";

            return value;
         }

// ---------------------------------------------------------------------
// Method output_metadata_GraphML()

      string output_metadata_GraphML(
         int n_indent,int photo_ID,int npx,int npy,
         string image_filename,string photo_timestamp,
         double longitude,double latitude,double zposn,
         double azimuth,double elevation,double roll,
         bool terminal_node_flag)
         {
//            cout << "inside photodbfunc::output_metadata_GraphML()" << endl;

            string node_value=jsonfunc::indent_spaces(n_indent-2);
            node_value += "{ \n";

            node_value += jsonfunc::output_GraphML_key_value_pair(
               n_indent,"id",stringfunc::number_to_string(photo_ID));

            node_value += jsonfunc::output_metadata_GraphML(
               n_indent,npx,npy,image_filename,photo_timestamp,
               longitude,latitude,zposn,azimuth,elevation,roll);
            node_value += jsonfunc::indent_spaces(n_indent-2);
            node_value += "}";
            if (!terminal_node_flag) node_value += ",";
            node_value += "\n";

//            cout << "node_value = " << node_value << endl;
            return node_value;
         }

// ==========================================================================
// Database metadata insertion methods
// ==========================================================================

// Method generate_insert_photo_SQL_command() takes in metadata
// associated with a single photo.  It generates and returns a string
// containing a SQL insert command needed to populate a row within the
// photos table of the TOC database.

   string generate_insert_photo_SQL_command(
      int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID,
      int photo_counter,bool genuine_timestamp_flag,double secs_since_epoch,
      bool genuine_geolocation_flag,const geopoint& geolocation,
      string URL,int npx,int npy,int importance)
   {
//      cout << "inside photodbfunc::generate_insert_photo_SQL_command()" << endl;
//      cout << "genuine_timestamp_flag = " << genuine_timestamp_flag << endl;

      Clock clock;
      clock.convert_elapsed_secs_to_date(secs_since_epoch);
      string date_str=clock.YYYY_MM_DD_H_M_S();
      
      string SQL_command="insert into photos ";
      SQL_command += "(fieldtest_ID,mission_ID,platform_ID,sensor_ID,";
      SQL_command += "photo_counter,";
      if (genuine_timestamp_flag)
      {
         SQL_command += "time_stamp,";
      }
      if (genuine_geolocation_flag)
      {
         SQL_command += "z_posn,xy_posn,";
      }

      SQL_command += "url,npx,npy,importance) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(fieldtest_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(platform_ID)+",";
      SQL_command += stringfunc::number_to_string(sensor_ID)+",";
      SQL_command += stringfunc::number_to_string(photo_counter)+",";
      if (genuine_timestamp_flag)
      {
         SQL_command += "'"+date_str+"',";
      }
      if (genuine_geolocation_flag)
      {
         SQL_command += stringfunc::number_to_string(
            geolocation.get_altitude())+",";
         SQL_command += "'SRID=4326; POINT("
            +stringfunc::number_to_string(
               geolocation.get_longitude(),9)
            +" "+stringfunc::number_to_string(
               geolocation.get_latitude(),9)+")',";
      }
      SQL_command += "'"+URL+"',";

      SQL_command += stringfunc::number_to_string(npx)+",";
      SQL_command += stringfunc::number_to_string(npy)+",";
      SQL_command += stringfunc::number_to_string(importance);
      SQL_command += ");";

//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------
// This overloaded version of generate_insert_photo_SQL_command()
// takes in ID, time stamp, filename, size, thumbnail, camera position
// and pointing information for a particular photograph.  It generates
// and returns a string containing a SQL insert command needed to
// populate the columns of the photo table within the TOC database.

string generate_insert_photo_SQL_command(
   int photo_ID,int time_stamp,string photo_URL,int xdim,int ydim,
   string thumbnail_URL,int thumbnail_xdim,int thumbnail_ydim,
   double longitude,double latitude,double altitude,
   double az,double el,double roll,double focal_param)
{
//   cout << "inside photodbfunc::generate_insert_SQL_command()" << endl;

   int importance=0;

   string SQL_command="insert into photo ";
   SQL_command += "(photo_counter,importance,url,npx,npy,";
   SQL_command += "thumbnail_url,thumbnail_npx,thumbnail_npy,";
   SQL_command += "z_posn,azimuth,elevation,roll,focal_param,xy_posn";
   SQL_command += ") ";
   SQL_command += "values(";
   SQL_command += stringfunc::number_to_string(photo_ID)+",";
   SQL_command += stringfunc::number_to_string(importance)+",";
   SQL_command += "'"+photo_URL+"',";
   SQL_command += stringfunc::number_to_string(xdim)+",";
   SQL_command += stringfunc::number_to_string(ydim)+",'";
   SQL_command += thumbnail_URL+"',"+stringfunc::number_to_string(
      thumbnail_xdim)+",";
   SQL_command += stringfunc::number_to_string(thumbnail_ydim)+",";
   SQL_command += stringfunc::number_to_string(altitude)+",";
   SQL_command += stringfunc::number_to_string(az)+",";
   SQL_command += stringfunc::number_to_string(el)+",";
   SQL_command += stringfunc::number_to_string(roll)+",";
   SQL_command += stringfunc::number_to_string(focal_param)+",";
   SQL_command += "'SRID=4326; POINT("
      +stringfunc::number_to_string(longitude,9)
      +" "+stringfunc::number_to_string(latitude,9)+")'";
   SQL_command += ");";

   return SQL_command;
}

// ---------------------------------------------------------------------   
// Method insert_photo_metadata_into_database() takes in an already
// opened GIS database along with metadata for multiple photos within
// input STL vectors.  It retrieves an STL vector filled with SQL
// insert commands.  This method then has the GIS database execute the
// insert commands to populate the photos table of the TOC database
// with imagery metadata information.

   bool insert_photo_metadata_into_database(
      gis_database* gis_database_ptr,
      int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID,
      Messenger* messenger_ptr,string progress_type,
      bool genuine_timestamp_flag,const vector<double>& secs_elapsed,
      bool genuine_geolocation_flag,const vector<geopoint>& geolocations,
      const vector<string>& photo_filenames,
      const vector<int>& xdim,const vector<int>& ydim)
   {
//      cout << "inside photodbfunc::insert_photo_metadata_into_database()" 
//           << endl;
//      cout << "photo_filenames.size() = " << photo_filenames.size() << endl;

      int default_importance=1;
      vector<string> insert_commands;
      for (unsigned int i=0; i<photo_filenames.size(); i++)
      {
         string curr_insert_cmd=generate_insert_photo_SQL_command(
            fieldtest_ID,mission_ID,platform_ID,sensor_ID,i,
            genuine_timestamp_flag,secs_elapsed[i],
            genuine_geolocation_flag,geolocations[i],
            photo_filenames[i],
            xdim[i],ydim[i],default_importance);
//         cout << "i = " << i
//              << " curr_insert_cmd = " << curr_insert_cmd << endl;
         insert_commands.push_back(curr_insert_cmd);
      } // loop over index i labeling photo filenames

//      cout << "insert_commands.size() = " << insert_commands.size() << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      gis_database_ptr->set_SQL_commands(insert_commands);

      double dataupload_progress=0.5;
      messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

      bool exec_flag=gis_database_ptr->execute_SQL_commands();
      exec_flag=true;

      dataupload_progress=0.95;
      messenger_ptr->broadcast_progress(dataupload_progress,progress_type);

//      cout << "exec_flag = " << exec_flag << endl;
      return exec_flag;
   }

// ==========================================================================
// Database metadata retrieval methods
// ==========================================================================

// Method generate_retrieve_photos_SQL_command()

   std::string generate_retrieve_photos_SQL_command(int mission_ID)
   {
//      cout << "inside photodbfunc::generate_retrieve_photos_SQL_command()" 
//           << endl;

      string SQL_command="SELECT id,time_stamp,url,npx,npy,x(xy_posn),y(xy_posn),photo_counter ";
      SQL_command += "from photos ";
      SQL_command += "WHERE mission_ID="+stringfunc::number_to_string(
         mission_ID);
      SQL_command += " ORDER BY id";
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method generate_retrieve_photos_SQL_command()

   std::string generate_retrieve_photos_SQL_command(
      int mission_ID,int sensor_ID)
   {
//      cout << "inside photodbfunc::generate_retrieve_photos_SQL_command()" 
//           << endl;

      string SQL_command="SELECT id,url,time_stamp from photos ";
      SQL_command += "WHERE mission_ID="+stringfunc::number_to_string(
         mission_ID);
      SQL_command += " AND sensor_ID="+stringfunc::number_to_string(
         sensor_ID);
      SQL_command += " ORDER BY id";
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method retrieve_photo_metadata_from_database() takes in mission
// and sensor IDs.  It queries the photos table of the TOC database
// for entries with matching mission and sensor IDs.  This method
// returns STL vectors containing output photo IDs and filenames which
// match the input parameter specifications.

   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,int sensor_ID,
      vector<int>& photo_IDs,vector<string>& photo_filenames,
      vector<int>& photo_framenumbers)
   {
      vector<string> photo_timestamps;
      retrieve_photo_metadata_from_database(
         gis_database_ptr,mission_ID,sensor_ID,photo_IDs,photo_filenames,
         photo_framenumbers,photo_timestamps);
   }

   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,int sensor_ID,
      vector<int>& photo_IDs,vector<string>& photo_filenames,
      vector<int>& photo_framenumbers,vector<string>& photo_timestamps)
   {
//      cout << "inside photodbfunc::retrieve_photo_metadata_from_database()" 
//           << endl;

      string curr_select_cmd=generate_retrieve_photos_SQL_command(
         mission_ID,sensor_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL)  return;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      const string separator_chars="-_.";   
      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         photo_IDs.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,0)));
         photo_filenames.push_back(field_array_ptr->get(i,1));

         string curr_photo_filename=photo_filenames.back();
         string curr_basename=filefunc::getbasename(curr_photo_filename);

         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               curr_basename,separator_chars);
         int n_substrings=substrings.size();

// Search backwards among substrings for last one which represents a
// genuine number.  As of 10/3/10, we assume that this last such
// substring corresponds to the frame's number:

         bool genuine_number_flag=false;
         int curr_framenumber=0;
         for (unsigned int n=n_substrings-1; n>=0 && !genuine_number_flag; n--)
         {
            string curr_substring=substrings[n];
            if (stringfunc::is_number(curr_substring))
            {
               genuine_number_flag=true;
               curr_framenumber=stringfunc::string_to_number(
                  curr_substring);
            }
         }

//         int curr_framenumber=stringfunc::string_to_number(substrings[1]);
         photo_framenumbers.push_back(curr_framenumber);

//         cout << "i = " << i
//              << " photo_ID = " << photo_IDs.back()
//              << " photo_filename = " << curr_basename 
//              << " framenumber = " << curr_framenumber;

         string curr_timestamp=field_array_ptr->get(i,2);
         if (curr_timestamp != "NULL")
         {
            photo_timestamps.push_back(curr_timestamp);
//            cout << " timestamp = " << photo_timestamps.back();
         }
         cout << endl;
         
      } // loop over index i labeling database rows
   }

// ---------------------------------------------------------------------   
   void retrieve_photo_IDs_from_database(
      gis_database* gis_database_ptr,int mission_ID,vector<int>& photo_IDs)
   {
      vector<string> photo_timestamps;
      retrieve_photo_metadata_from_database(
         gis_database_ptr,mission_ID,photo_IDs,photo_timestamps);
   }

   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,
      vector<int>& photo_IDs,vector<string>& photo_timestamps)
   {
      vector<int> photo_framenumbers;
      vector<string> photo_urls;
      vector<twovector> photo_npxnpys,photo_lonlats;
      retrieve_photo_metadata_from_database(
         gis_database_ptr,mission_ID,photo_IDs,photo_timestamps,
         photo_urls,photo_npxnpys,photo_lonlats,photo_framenumbers);
   }

   void retrieve_photo_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,
      vector<int>& photo_IDs,vector<string>& photo_timestamps,
      vector<string>& photo_urls,vector<twovector>& photo_npxnpys,
      vector<twovector>& photo_lonlats,vector<int>& photo_framenumbers)
   {
//      cout << "inside photodbfunc::retrieve_photo_metadata_from_database()" 
//           << endl;

      string curr_select_cmd=generate_retrieve_photos_SQL_command(
         mission_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;

//      unsigned int mdim=field_array_ptr->get_mdim();
//      unsigned int ndim=field_array_ptr->get_ndim();
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         photo_IDs.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,0)));
//         cout << "i = " << i
//              << " mdim = " << mdim 
//              << " ndim = " << ndim 
//              << " photo_ID = " << photo_IDs.back() << endl;

         string curr_timestamp=field_array_ptr->get(i,1);
         photo_timestamps.push_back(curr_timestamp);
//         cout << " timestamp = " << photo_timestamps.back() << endl;

         string curr_url=field_array_ptr->get(i,2);
         photo_urls.push_back(curr_url);

         int npx=stringfunc::string_to_number(field_array_ptr->get(i,3));
         int npy=stringfunc::string_to_number(field_array_ptr->get(i,4));
         twovector npxnpy(npx,npy);
         photo_npxnpys.push_back(npxnpy);

         double curr_long=stringfunc::string_to_number(
            field_array_ptr->get(i,5));
         double curr_lat=stringfunc::string_to_number(
            field_array_ptr->get(i,6));
//         cout << "curr_lon = " << curr_long << " curr_lat = " << curr_lat
//              << endl;
         twovector curr_lonlat(curr_long,curr_lat);
         photo_lonlats.push_back(curr_lonlat);

         int photo_counter=stringfunc::string_to_number(
            field_array_ptr->get(i,7));
         photo_framenumbers.push_back(photo_counter);

      } // loop over index i labeling database rows
   }

// ---------------------------------------------------------------------   
// Method retrieve_photo_metadata_for_graph() takes in the ID for
// some graph.  It returns a dynamically instantiated STL MAP 
// containing strings metadata versus photo IDs for all photos within
// the specified graph.

   PHOTO_IDS_METADATA_MAP* retrieve_photo_metadata_for_graph(
      gis_database* gis_database_ptr,int graph_ID)
   {
//      cout << "inside photodbfunc::retrieve_photo_metadata_for_graph()" 
//           << endl;

      PHOTO_IDS_METADATA_MAP* photo_ids_metadata_map_ptr=
         new PHOTO_IDS_METADATA_MAP;

      string select_cmd="select data_id,time_stamp,url,npx,npy,";
      select_cmd += "thumbnail_url,thumbnail_npx,thumbnail_npx,";
      select_cmd += "importance,photo_counter from photos ";
      select_cmd += "inner join nodes ";
      select_cmd += "on photos.id=nodes.data_id ";
      select_cmd += "where nodes.graph_id= "
         +stringfunc::number_to_string(graph_ID);
      select_cmd += "order by data_id";
//      cout << "select_cmd = " << select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_cmd);
      unsigned int mdim=field_array_ptr->get_mdim();
      unsigned int ndim=field_array_ptr->get_ndim();

      vector<string> field_strings;
      for (unsigned int m=0; m<mdim; m++)
      {
         int photo_ID=stringfunc::string_to_number(field_array_ptr->get(m,0));

         field_strings.clear();
         for (unsigned int n=1; n<ndim; n++)
         {
//            string photo_timestamp=field_array_ptr->get(m,1);
//            string photo_URL=field_array_ptr->get(m,2);
//            string npx=field_array_ptr->get(m,3);
//            string npy=field_array_ptr->get(m,4);
//            string thumbnail_URL=field_array_ptr->get(m,5);
//            string thumbnail_npx=field_array_ptr->get(m,6);
//            string thumbnail_npy=field_array_ptr->get(m,7);
//            string importance=field_array_ptr->get(m,8);      
//            string photo_counter=field_array_ptr->get(m,9);

            field_strings.push_back(field_array_ptr->get(m,n));

         } // loop over index n labeling rows in *field_array_ptr
         (*photo_ids_metadata_map_ptr)[photo_ID]=field_strings;
      } // loop over index m labeling rows in *field_array_ptr

      return photo_ids_metadata_map_ptr;
   }

// ---------------------------------------------------------------------   
// Method retrieve_photo_URLs_vs_node_IDs() takes in the ID for
// some graph.  It returns a dynamically instantiated STL MAP 
// containing string URL metadata versus node IDs for all photos within
// the specified graph.

   PHOTO_IDS_METADATA_MAP* retrieve_photo_URLs_vs_node_IDs(
      gis_database* gis_database_ptr,int graph_ID)
   {
//      cout << "inside photodbfunc::retrieve_photo_URLs_vs_node_IDs()" 
//           << endl;

      PHOTO_IDS_METADATA_MAP* photo_ids_metadata_map_ptr=
         new PHOTO_IDS_METADATA_MAP;

      string select_cmd="select node_id,url from photos ";
      select_cmd += "inner join nodes ";
      select_cmd += "on photos.id=nodes.data_id ";
      select_cmd += "where nodes.graph_id= "
         +stringfunc::number_to_string(graph_ID);
      select_cmd += "order by node_id";
//      cout << "select_cmd = " << select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_cmd);
//      cout << "field_array_ptr = " << field_array_ptr << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      unsigned int ndim=field_array_ptr->get_ndim();
//      cout << "mdim = " << mdim << " ndim = " << mdim << endl;

      vector<string> field_strings;
      for (unsigned int m=0; m<mdim; m++)
      {
         int node_ID=stringfunc::string_to_number(field_array_ptr->get(m,0));

         field_strings.clear();
         for (unsigned int n=1; n<ndim; n++)
         {
//            string photo_timestamp=field_array_ptr->get(m,1);
//            string photo_URL=field_array_ptr->get(m,2);
//            string npx=field_array_ptr->get(m,3);
//            string npy=field_array_ptr->get(m,4);
//            string thumbnail_URL=field_array_ptr->get(m,5);
//            string thumbnail_npx=field_array_ptr->get(m,6);
//            string thumbnail_npy=field_array_ptr->get(m,7);
//            string importance=field_array_ptr->get(m,8);      
//            string photo_counter=field_array_ptr->get(m,9);

            field_strings.push_back(field_array_ptr->get(m,n));

         } // loop over index n labeling rows in *field_array_ptr
         (*photo_ids_metadata_map_ptr)[node_ID]=field_strings;
      } // loop over index m labeling rows in *field_array_ptr

      return photo_ids_metadata_map_ptr;
   }

// ---------------------------------------------------------------------   
// Method generate_retrieve_particular_photo_SQL_command()

   string generate_retrieve_particular_photo_SQL_command(int photo_ID)
   {
//      cout << "inside photodbfunc::generate_retrieve_particular_photo_SQL_command()" 
//           << endl;

      string SQL_command="SELECT time_stamp,url,npx,npy,thumbnail_url,thumbnail_npx,thumbnail_npy,importance,photo_counter from photos ";
      SQL_command += "WHERE id="+stringfunc::number_to_string(photo_ID)+";";
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
   bool retrieve_particular_photo_metadata_from_database(
      gis_database* gis_database_ptr,int photo_ID,string& caption,
      string& photo_timestamp,string& photo_URL,
      int& photo_counter,int& importance)
   {
//      cout << "inside photodbfunc::retrieve_particular_photo_metadata_from_database() #1" 
//           << endl;

      string curr_select_cmd=generate_retrieve_particular_photo_SQL_command(
         photo_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      photo_timestamp=field_array_ptr->get(0,0);
      photo_URL=field_array_ptr->get(0,1);
      importance=stringfunc::string_to_number(field_array_ptr->get(0,7));
      photo_counter=stringfunc::string_to_number(field_array_ptr->get(0,8));
      return true;
   }

// ---------------------------------------------------------------------   
   bool retrieve_particular_photo_metadata_from_database(
      gis_database* gis_database_ptr,int photo_ID,
      string& photo_timestamp,string& photo_URL,int& npx,int& npy,
      string& thumbnail_URL,int& thumbnail_npx,int& thumbnail_npy,
      int& importance,int& photo_counter)
   {
//      cout << "inside photodbfunc::retrieve_particular_photo_metadata_from_database() #2" 
//           << endl;

      string curr_select_cmd=generate_retrieve_particular_photo_SQL_command(
         photo_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      photo_timestamp=field_array_ptr->get(0,0);
      photo_URL=field_array_ptr->get(0,1);
      npx=stringfunc::string_to_number(field_array_ptr->get(0,2));
      npy=stringfunc::string_to_number(field_array_ptr->get(0,3));
      thumbnail_URL=field_array_ptr->get(0,4);
      thumbnail_npx=stringfunc::string_to_number(field_array_ptr->get(0,5));
      thumbnail_npy=stringfunc::string_to_number(field_array_ptr->get(0,6));
      importance=stringfunc::string_to_number(field_array_ptr->get(0,7));      
      photo_counter=stringfunc::string_to_number(field_array_ptr->get(0,8));

      return true;
   }

// ---------------------------------------------------------------------   
   bool retrieve_photo_ID_URL_given_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      int& photo_ID,string& photo_URL,string& thumbnail_URL)
   {
//      cout << "inside photodbfunc::retrieve_photo_ID_URL_given_node_ID()" 
//           << endl;

      string curr_select_cmd="select data_id,url,thumbnail_url from photos ";
      curr_select_cmd += "inner join nodes on photos.id=nodes.data_id ";
      curr_select_cmd += "where nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      curr_select_cmd += " and nodes.node_id="+
         stringfunc::number_to_string(node_ID)+" order by data_id";
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      photo_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      photo_URL=field_array_ptr->get(0,1);
      thumbnail_URL=field_array_ptr->get(0,2);
      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_photo_subdir_from_database() takes in mission
// and sensor IDs.  It queries the photos table of the TOC database
// for photo filename entries with matching mission and sensor IDs.
// This method the name of the subdirectory containing the queried
// photos.  If no photo files are found matching the specified input
// IDs, the returned string has zero size.

   string retrieve_photo_subdir_from_database(
      gis_database* gis_database_ptr,int mission_ID,int sensor_ID)
   {
//      cout << "inside photodbfunc::retrieve_photo_subdir_from_database()"
//           << endl;

      vector<int> photo_IDs,photo_framenumbers;
      vector<string> photo_filenames,photo_timestamps;
      retrieve_photo_metadata_from_database(
         gis_database_ptr,mission_ID,sensor_ID,photo_IDs,photo_filenames,
         photo_framenumbers,photo_timestamps);

//      cout << "photo_filenames.size() = " << photo_filenames.size() << endl;

      string movie_frames_subdir;
      if (photo_filenames.size() > 0)
      {
         movie_frames_subdir=filefunc::getdirname(photo_filenames.back());
      }

//      cout << "movie_frames_subdir = " << movie_frames_subdir << endl;
      return movie_frames_subdir;
   }

// ==========================================================================
// Database metadata update methods
// ==========================================================================

// Method update_photo_timestamps_in_database() takes in a calculated
// set of photo timestamps and their corresponding photo IDs as well as 
// mission and sensor IDs.  It generates a set of SQL UPDATE commands
// which modifies the timestamp column of the photos table in the TOC
// database.

   bool update_photo_timestamps_in_database(
      gis_database* gis_database_ptr,int mission_ID,int sensor_ID,
      const std::vector<int>& photo_IDs,
      const std::vector<std::string>& photo_timestamps)
   {
//      cout << "inside photodbfunc::insert_photo_timestamps_into_database()" 
//           << endl;

      vector<string> update_commands;
      for (unsigned int i=0; i<photo_IDs.size(); i++)
      {
         string curr_update_cmd="UPDATE photos ";
         curr_update_cmd += "SET time_stamp='"+photo_timestamps[i]+"' ";
         curr_update_cmd += "WHERE id="+stringfunc::number_to_string(
            photo_IDs[i])+" AND mission_id = "+
            stringfunc::number_to_string(mission_ID)+" AND sensor_id = "+
            stringfunc::number_to_string(sensor_ID);

//         cout << "i = " << i
//              << " curr_update_cmd = " << curr_update_cmd << endl;
         update_commands.push_back(curr_update_cmd);
      } // loop over index i labeling photo filenames

//      cout << "update_commands.size() = " << update_commands.size() << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      gis_database_ptr->set_SQL_commands(update_commands);
      bool exec_flag=gis_database_ptr->execute_SQL_commands();
      exec_flag=true;
      return exec_flag;
   }

// ---------------------------------------------------------------------   
// Method update_thumbnail_metadata_in_database() takes in an already
// opened GIS database along with metadata for multiple photo
// thumbnails within input STL vectors.  It forms an STL vector
// filled with SQL update commands.  This method then has the GIS
// database execute the update commands to populate the photos table
// of the TOC database with thumbnail metadata information.

   bool update_thumbnail_metadata_in_database(
      gis_database* gis_database_ptr,const vector<int>& photo_IDs,
      const vector<string>& thumbnail_filenames,
      const vector<int>& xdim,const vector<int>& ydim)
   {
//      cout << "inside photodbfunc::update_thumnbnail_metadata_in_database()" 
//           << endl;
//      cout << "photo_IDs.size() = " << photo_IDs.size() << endl;
//      cout << "thumbnail_filenames.size() = "
//           << thumbnail_filenames.size() << endl;
//      cout << "xdim.size() = " << xdim.size()
//           << " ydim.size() = " << ydim.size() << endl;

// First check to make sure photo_IDs.size()==thumbnail_filenames.size()==
// xdim.size()==ydim.size:

      if (photo_IDs.size()==thumbnail_filenames.size() &&
          thumbnail_filenames.size()==xdim.size() &&
          xdim.size()==ydim.size())
      {
      }
      else
      {
         cout << "Trouble detected in photodbfunc::update_thumbnail_metadata_in_database()!" << endl;
         cout << "photo_IDs.size() = " << photo_IDs.size() << endl;
         cout << "thumbnail_filenames.size() = "
              << thumbnail_filenames.size() << endl;
         cout << "xdim.size() = " << xdim.size() << endl;
         cout << "ydim.size() = " << ydim.size() << endl;
         cout << "These STL vectors should all have the same size!!!" << endl;
         exit(-1);
      }

      vector<string> update_commands;
      for (unsigned int i=0; i<photo_IDs.size(); i++)
      {
         string curr_update_cmd=generate_photo_thumbnail_SQL_command(
            photo_IDs[i],thumbnail_filenames[i],xdim[i],ydim[i]);
         update_commands.push_back(curr_update_cmd);
      } // loop over index i labeling photo filenames

//      cout << "update_commands.size() = " << update_commands.size() << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      gis_database_ptr->set_SQL_commands(update_commands);
      bool exec_flag=gis_database_ptr->execute_SQL_commands();
      exec_flag=true;
      return exec_flag;
   }

// ---------------------------------------------------------------------   
// Method generate_photo_thumbnail_SQL_command() takes in thumbnail
// metadata for a single photo.  It generates and returns a string
// containing a SQL update command needed to modify a row within the
// photos table of the TOC database.

   string generate_photo_thumbnail_SQL_command(
      int photo_ID,string thumbnail_URL,int thumbnail_npx,int thumbnail_npy)
   {
//      cout << "inside photodbfunc::generate_photo_thumbnail_SQL_command()" << endl;

      string SQL_command="UPDATE photos ";
      SQL_command += "set thumbnail_url='"+thumbnail_URL+"',";
      SQL_command += "thumbnail_npx="+
         stringfunc::number_to_string(thumbnail_npx)+",";      
      SQL_command += "thumbnail_npy="+
         stringfunc::number_to_string(thumbnail_npy)+" ";      
      SQL_command += "where id="+stringfunc::number_to_string(photo_ID)+";";
//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method update_photo_importance_in_database() takes in
// the URL for a particular photo.  It generates and executes a SQL
// update command that resets the importance value for the row within
// the photos table of the TOC database corresponding to the input
// URL.

   bool update_photo_importance_in_database(
      gis_database* gis_database_ptr,string URL,int importance)
   {
//      cout << "inside photodbfunc::update_photo_importance_in_database()" 
//           << endl;

      vector<string> update_commands;

      string SQL_command="UPDATE photos SET importance=";
      SQL_command += stringfunc::number_to_string(importance);
      SQL_command += " WHERE url='"+URL+"';";
      update_commands.push_back(SQL_command);
//      cout << "SQL_command = " << SQL_command << endl;

//      cout << "update_commands.size() = " << update_commands.size() << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      gis_database_ptr->set_SQL_commands(update_commands);
      bool exec_flag=gis_database_ptr->execute_SQL_commands();
      exec_flag=true;
      return exec_flag;
   }

// ---------------------------------------------------------------------   
// Method compute_photo_importance_intervals()

   vector<fourvector> compute_photo_importance_intervals(
      gis_database* gis_database_ptr,int mission_ID)
   {
//      cout << "inside photodbfunc::compute_photo_importance_intervals()" << endl;

      string SQL_command="SELECT id,importance from photos ";
      SQL_command += " WHERE mission_id="+stringfunc::number_to_string(
         mission_ID)+" ORDER BY id;";
//      cout << "SQL_command = " << SQL_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_command);
//      cout << "*field_array_ptr = " << *field_array_ptr << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
//      unsigned int ndim=field_array_ptr->get_ndim();
//      cout << "mdim = " << mdim << " ndim = " << ndim << endl;

      vector<int> photo_IDs,photo_importances;
      for (unsigned int i=0; i<mdim; i++)
      {
         photo_IDs.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,0)));
         photo_importances.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,1)));
      }

// Identify clusters of photo importances:

      int prev_importance=photo_importances[0];
      int start_frame=0;
      int stop_frame=0;

      vector<fourvector> importance_intervals;
      for (unsigned int i=0; i<photo_importances.size(); i++)
      {
         int starting_photo_ID=photo_IDs[start_frame];
         int curr_importance=photo_importances[i];
         if (curr_importance==prev_importance && i<photo_importances.size()-1) 
            continue;

         if (i==photo_importances.size()-1)
         {
            stop_frame=i;
         }
         else
         {
            stop_frame=i-1;
         }
         fourvector ID_start_stop_importance(
            starting_photo_ID,start_frame,stop_frame,prev_importance);
//         cout << "ID_start_stop_importance = " 
//              << ID_start_stop_importance << endl;
         importance_intervals.push_back(ID_start_stop_importance);

         start_frame=i;
         prev_importance=curr_importance;

      } // loop over index i labeling photo importances

//      cout << "importance_intervals.size() = "
//           << importance_intervals.size() << endl;
      
      return importance_intervals;
   }

// ---------------------------------------------------------------------   
// Method fuse_photo_and_gps_metadata() takes in a mission ID along
// with a GPS tracks group for some fieldtest assumed to correspond to
// the selected mission.  It loops over every entry within the photos
// table of the TOC database corresponding to the input mission ID.  If a
// GPS track from the same mission also exists within
// *tracks_group_ptr, this method interpolates its position values in
// order to assign a lon,lat,alt triple to as many of the mission's
// photos as possible.  The number of photos which inherit GPS
// metadata is returned by this method.

   int fuse_photo_and_gps_metadata(
      gis_database* gis_database_ptr,int selected_mission_ID,
      tracks_group* tracks_group_ptr)
   {
//      cout << "inside fuse_photo_and_gps_metadata()" << endl;
//      cout << "tracks_group_ptr->get_n_tracks() = "
//           << tracks_group_ptr->get_n_tracks() << endl;
      
      vector<int> photo_IDs;
      vector<string> photo_timestamps;
      photodbfunc::retrieve_photo_metadata_from_database(
         gis_database_ptr,selected_mission_ID,photo_IDs,photo_timestamps);

      vector<track*> raw_GPS_track_ptrs=
         tracks_group_ptr->get_all_track_ptrs();

      Clock clock;
      vector<int> image_IDs;
      vector<double> image_lons,image_lats,image_alts;

      for (unsigned int i=0; i<photo_IDs.size(); i++)
      {
         string curr_timestamp=photo_timestamps[i];
         if (curr_timestamp == "NULL") continue;

         int timestamp_length=curr_timestamp.size();
         curr_timestamp=curr_timestamp.substr(0,timestamp_length-3);

         bool UTC_flag=true;
         double elapsed_secs=clock.timestamp_string_to_elapsed_secs(
            curr_timestamp,UTC_flag);
//      string recovered_timestamp=clock.YYYY_MM_DD_H_M_S(" ",":",false,4);
               
// Loop over each raw GPS track:	

         for (unsigned int r=0; r<raw_GPS_track_ptrs.size(); r++)
         {
            track* track_ptr=raw_GPS_track_ptrs[r];

//            cout << "r = " << r << " track_ptr = " << track_ptr << endl;

            int curr_mission_ID=track_ptr->get_label_ID();
            if (curr_mission_ID != selected_mission_ID) continue;

            threevector interpolated_posn;
            if (track_ptr->get_interpolated_posn(
               elapsed_secs,interpolated_posn))
            {
//               cout << "interp_posn = " << interpolated_posn << endl;

               double longitude=interpolated_posn.get(0);
               double latitude=interpolated_posn.get(1);
               double altitude=interpolated_posn.get(2);

               image_IDs.push_back(photo_IDs[i]);
               image_lons.push_back(longitude);
               image_lats.push_back(latitude);
               image_alts.push_back(altitude);
            
//               cout << "i = " << i
//                    << " photo_ID = " << photo_IDs[i]
//                    << " time = " << curr_timestamp
//                    << " lon = " << longitude
//                    << " lat = " << latitude
//                    << " alt = " << altitude << endl;
            }

         } // loop over index r labeling raw GPS tracks
      } // loop over index i labeling photos for selected mission and fieldtest

      photodbfunc::update_photo_geometries_in_database(
         gis_database_ptr,image_IDs,image_lons,image_lats,image_alts);

      return image_IDs.size();
   }

// ---------------------------------------------------------------------   
// Method update_photo_geometries_in_database() takes in photo
// lon, lat and alt triples and their corresponding photo
// IDs. It generates and executes a set of SQL UPDATE commands which
// modify the xy_posn and z_posn columns of the photos table in the
// TOC database.

   bool update_photo_geometries_in_database(
      gis_database* gis_database_ptr,
      const vector<int>& photo_IDs,const vector<double>& photo_lons,
      const vector<double>& photo_lats,const vector<double>& photo_alts)
   {
//      cout << "inside photodbfunc::update_photo_geometries_in_database()" 
//           << endl;

      vector<string> update_commands;
      for (unsigned int i=0; i<photo_IDs.size(); i++)
      {
         string curr_update_cmd="UPDATE photos ";
         curr_update_cmd += "SET z_posn="
            +stringfunc::number_to_string(photo_alts[i])+",";
         curr_update_cmd += " xy_posn=";
         curr_update_cmd += "'SRID=4326; POINT("
            +stringfunc::number_to_string(photo_lons[i],9)
            +" "+stringfunc::number_to_string(photo_lats[i],9)+")'";
         curr_update_cmd += " WHERE id="+stringfunc::number_to_string(
            photo_IDs[i]);

//         cout << "i = " << i
//              << " curr_update_cmd = " << curr_update_cmd << endl;
         update_commands.push_back(curr_update_cmd);
      } // loop over index i labeling photo filenames

//      cout << "update_commands.size() = " << update_commands.size() << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      gis_database_ptr->set_SQL_commands(update_commands);
      bool exec_flag=gis_database_ptr->execute_SQL_commands();
      exec_flag=true;
      return exec_flag;
   }
      
// ==========================================================================
// TOC specific methods
// ==========================================================================

// Method extract_droid_image_time_from_metadata_file() takes in the
// full path for an image captured via Paul Breimyer's codes operating
// on a droid cell phone.  It searches for a corresponding metadata
// text file also generated by Paul's codes.  If both files exist,
// this method extracts and returns the image's time from the text
// file.
 
   double extract_droid_image_time_from_metadata_file(
      string image_filename,Clock& clock)
  {
// cout << "inside photodbfunc::extract_droid_image_time_from_metadata_file()
//           << endl;

      string curr_timestamp=stringfunc::prefix(image_filename);
      string text_filename=curr_timestamp+".txt";

      if (!filefunc::fileexist(text_filename)) return -1;
//      cout << "i = " << i
//           << " image_filename = " << image_filename 
//           << " text filename = " << text_filename << endl;

      filefunc::ReadInfile(text_filename);
      vector<double> fields=stringfunc::string_to_numbers(
         filefunc::text_line[0],",");

// As of 9/2/10, we believe the first column entry in Paul Breimyer's
// output text files accompanying each droid JPG image corresponds to
// MILLIseconds since midnight Jan 1, 1970:

      double secs_elapsed=0.001*fields[0];
      return secs_elapsed;
   }

// ---------------------------------------------------------------------   
// Method append_counter_to_image_filenames() takes in the name for
// some image filename which does not initially explicitly contain
// image counter information (e.g. droid and Axis image filenames are
// in this category).  It generates a new image filename with an
// "_###" counter field preprended before the ".jpg" or ".png" suffix.
// The original image file is moved into an "orig_data/" subdir of the
// original image directory.  The new image file is left within
// the original image directory, and its new filename is returned by
// this method.
 
   string append_counter_to_image_filenames(
      string curr_image_filename,int& good_image_counter)
   {
//      cout << "inside photodbfunc::append_counter_to_image_filenames()"
//           << endl;
      
      string dirname=filefunc::getdirname(curr_image_filename);
      string orig_images_dirname=dirname+"orig_data/";
      filefunc::dircreate(orig_images_dirname);

      string prefix=stringfunc::prefix(curr_image_filename);
      string suffix=stringfunc::suffix(curr_image_filename);
      string new_prefix =prefix+"_"+stringfunc::number_to_string(
         good_image_counter++);
      string new_image_filename=new_prefix+"."+suffix;
//      cout << "new image filename = " << new_image_filename << endl;

      string unix_cmd="cp "+curr_image_filename+" "+
         new_image_filename;
//      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      unix_cmd="mv "+curr_image_filename+" "+orig_images_dirname;
//      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

// Paul Breimyer's droid codes generate an associated .txt file for
// each output image file.  So check whether such a file exists.  If
// so, move the text file into orig_images_dirname:

      string text_filename=prefix+".txt";
      if (filefunc::fileexist(text_filename))
      {
         unix_cmd="mv "+text_filename+" "+orig_images_dirname;
//         cout << "unix_cmd = " << unix_cmd << endl << endl;
         sysfunc::unix_command(unix_cmd);
      }

      return new_image_filename;
   }

// ---------------------------------------------------------------------   
// Method photo_in_database() queries the photos table within
// *gis_database_ptr for the existence of a row corresponding to input
// photo_ID.  If no entry in the table matches photo_ID, this boolean
// method returns false.
 
   bool photo_in_database(gis_database* gis_database_ptr,int photo_ID)
   {
      string select_command="select * from photos ";
      select_command += " WHERE ID="+stringfunc::number_to_string(photo_ID);
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);
      if (field_array_ptr==NULL) 
      {
         return false;
      }
      else
      {
         return true;
      }
   }
   
// ---------------------------------------------------------------------   
// Method get_photo_ID() takes in a photo's URL and returns its unique
// ID assigned by the TOC database.
 
   int get_photo_ID(gis_database* gis_database_ptr,string URL)
   {
      string select_command="select id from photos ";
      select_command += " WHERE url='"+URL+"';";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int photo_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "photo_ID = "  << photo_ID << endl;

      return photo_ID;
   }

// ---------------------------------------------------------------------   
// Method get_photo_URL() takes in a photo's ID and returns either its
// URL or thumbnail's URL stored within the photos table of the TOC
// database.
 
   string get_photo_URL(gis_database* gis_database_ptr,int photo_ID,
      bool thumbnail_flag)
   {
      string select_command="select url from photos ";
      if (thumbnail_flag)
      {
         select_command="select thumbnail_url from photos ";
      }
      select_command += "WHERE ID="+stringfunc::number_to_string(photo_ID);
      
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      string URL=field_array_ptr->get(0,0);
//      cout << "URL = "  << URL << endl;

      return URL;
   }

// ---------------------------------------------------------------------   
// Method get_photo_dims() takes in a photo's ID and returns its width
// and height in pixels.
 
   twovector get_photo_dims(gis_database* gis_database_ptr,int photo_ID)
   {
      string select_command="SELECT npx,npy from photos ";
      select_command += " WHERE id="+stringfunc::number_to_string(photo_ID)
         +";";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      int npx=stringfunc::string_to_number(
         field_array_ptr->get(0,0));
      int npy=stringfunc::string_to_number(
         field_array_ptr->get(0,1));
//      cout << "npx = " << npx << " npy = " << npy << endl;
      twovector photo_dims(npx,npy);
      
      return photo_dims;
   }

// ==========================================================================
// Graph database querying methods
// ==========================================================================

// Method write_graph_json_string() takes in *graph_ptr along with
// boolean flag get_nodes_flag and get_edges_flag.  It outputs a JSON
// string containing node and edge information depending upon these
// flags' values.  If STL vector incident_node_IDs is non-empty, only
// edges adjacent to input nodes are written to output JSON string.

   string write_graph_json_string(
      gis_database* gis_database_ptr,int hierarchy_ID,graph* graph_ptr,
      bool get_nodes_flag,bool get_edges_flag,
      const vector<int>& incident_node_IDs)
   {
      cout << "inside photodbfunc::write_graph_json_string()" << endl;
//      cout << "photogroup_ptr = " << photogroup_ptr << endl;
//      cout << "incident_node_IDs.size() = "
//           << incident_node_IDs.size() << endl;

      string edge_default="undirected";

      string json_string="\n";
      json_string="{ \n";
      json_string += "  \"graph\": { \n";

      json_string +=  jsonfunc::output_key_value_pair(
         5,"id",graph_ptr->get_ID());
      json_string +=  jsonfunc::output_key_value_pair(
         5,"edgedefault",edge_default);

// Write out nodes:

      if (get_nodes_flag)
      {
         cout << "Writing out nodes:" << endl;
         json_string += "     \"node\": [ \n";

         vector<node*> node_ptrs;
  
// If input STL vector incident_node_IDs is non-empty, only output
// nodes corresponding to input IDs.  Otherwise, output all
// nodes within input graph:
       
         if (incident_node_IDs.size() > 0)
         {
            for (unsigned int n=0; n<incident_node_IDs.size(); n++)
            {
               node_ptrs.push_back(graph_ptr->get_node_ptr(
                  incident_node_IDs[n]));
            }
         }
         else
         {
            for (unsigned int n=0; n<graph_ptr->get_n_nodes(); n++)
            {
               node* node_ptr=graph_ptr->get_ordered_node_ptr(n);
               node_ptrs.push_back(node_ptr);
            }
         }

// Execute single SQL call to retrieve all photo metadata for
// particular graph:

         PHOTO_IDS_METADATA_MAP* photo_ids_metadata_map_ptr=
            retrieve_photo_metadata_for_graph(
               gis_database_ptr,graph_ptr->get_ID());

// Execute single SQL call to retrieve all node labels for particular
// graph:

         graphdbfunc::NODE_IDS_LABELS_MAP* node_ids_labels_map_ptr=
            graphdbfunc::retrieve_node_labels_from_database(
               gis_database_ptr,graph_ptr->get_ID());

         bool terminal_node_flag=false;
         for (unsigned int n=0; n<node_ptrs.size(); n++)
         {
            node* node_ptr=node_ptrs[n];
//            cout << "n = " << n 
//                 << " node_ID = " << node_ptr->get_ID() << endl;
            
            if (n==node_ptrs.size()-1) terminal_node_flag=true;

            json_string += write_node_json_string(
               9,hierarchy_ID,node_ptr,gis_database_ptr,
               photo_ids_metadata_map_ptr,node_ids_labels_map_ptr,
               terminal_node_flag);
         } // loop over index n labeling graph nodes

         json_string += "    ]";
         if (get_edges_flag)
         {
            json_string += ",";
         }
         json_string += "\n";

         delete photo_ids_metadata_map_ptr;
         delete node_ids_labels_map_ptr;

      } // get_nodes_flag conditional
      
// Write out edges:

      if (get_edges_flag)
      {
         cout << "Writing out edges:" << endl;
         graph_ptr->compute_edge_weights_distribution();

         json_string += "    \"edge\": [ \n";

         vector<graph_edge*> graph_edge_ptrs;
  
// If input STL vector incident_node_IDs is non-empty, only output
// edges adjacent to specified input nodes.  Otherwise, output all
// edges within input graph:
       
         if (incident_node_IDs.size() > 0)
         {
            vector<int> adjacent_edge_IDs=graph_ptr->get_adjacent_edge_IDs(
               incident_node_IDs);
            for (unsigned int a=0; a<adjacent_edge_IDs.size(); a++)
            {
//               cout << "a = " << a << " edge ID = "
//                    << adjacent_edge_IDs[a] << endl;
               graph_edge_ptrs.push_back(graph_ptr->get_graph_edge_ptr(
                  adjacent_edge_IDs[a]));
            }
         }
         else
         {
            for (unsigned int e=0; e<graph_ptr->get_n_graph_edges(); e++)
            {
               graph_edge* graph_edge_ptr=
                  graph_ptr->get_ordered_graph_edge_ptr(e);
               graph_edge_ptrs.push_back(graph_edge_ptr);
            }
         }
         
         bool terminal_edge_flag=false;
         for (unsigned int e=0; e<graph_edge_ptrs.size(); e++)
         {
            graph_edge* graph_edge_ptr=graph_edge_ptrs[e];
            int curr_matches=graph_edge_ptr->get_weight();

            if (e==graph_edge_ptrs.size()-1) terminal_edge_flag=true;
            if (curr_matches > 0)
            {
               node* node1_ptr=graph_edge_ptr->get_node1_ptr();
               node* node2_ptr=graph_edge_ptr->get_node2_ptr();
         
               colorfunc::RGB edge_RGB=graph_ptr->
                  compute_edge_color(curr_matches);

               double relative_edge_thickness=1;
               if (graph_ptr->get_level()==1)
               {
                  relative_edge_thickness=2;
               }
               else if (graph_ptr->get_level() >= 2)
               {
                  relative_edge_thickness=3;
               }
               json_string += jsonfunc::write_edge_json_string(
                  9,node1_ptr->get_ID(),node2_ptr->get_ID(),
                  curr_matches,edge_RGB.first,edge_RGB.second,edge_RGB.third,
                  relative_edge_thickness,terminal_edge_flag);
            }
         } // loop over index e labeling graph edges

         json_string += "    ] \n";
      } // get_edges_flag conditional
      
      json_string += "  } \n";
      json_string += "} \n";

      return json_string;
   }

// ---------------------------------------------------------------------
// Method write_node_json_string()

      string write_node_json_string(
         int n_indent,int graph_hierarchy_ID,node* node_ptr,
         gis_database* gis_database_ptr,
         PHOTO_IDS_METADATA_MAP* photo_ids_metadata_map_ptr,
         graphdbfunc::NODE_IDS_LABELS_MAP* node_ids_labels_map_ptr,
         bool terminal_node_flag)
         {
//            cout << "inside photodbfunc::write_node_json_string()" << endl;
//            cout << "photograph_ptr = " << photograph_ptr << endl;

            string node_json_string=jsonfunc::indent_spaces(n_indent-2);
            node_json_string += "{ \n";

            node_json_string += jsonfunc::output_key_value_pair(
               n_indent,"id",node_ptr->get_ID() );

            int photo_ID=node_ptr->get_data_ID();
//            cout << "node_ID = " << node_ptr->get_ID()
//                 << " photo_ID = " << photo_ID << endl;
            
// Look up metadata for current photo within
// *photo_ids_metadata_map_ptr:

            PHOTO_IDS_METADATA_MAP::iterator photo_iter=
               photo_ids_metadata_map_ptr->find(photo_ID);
            if (photo_iter == photo_ids_metadata_map_ptr->end()) 
            {
               cout << "Error in photodbfunc::write_node_json_string()"
                    << endl;
               cout << "photo_iter = NULL !" << endl;
               exit(-1);
            }

            vector<string> photo_metadata_strings=photo_iter->second;
            string photo_timestamp=photo_metadata_strings[0];
            string photo_URL=photo_metadata_strings[1];
            int npx=stringfunc::string_to_number(photo_metadata_strings[2]);
            int npy=stringfunc::string_to_number(photo_metadata_strings[3]);
            string thumbnail_URL=photo_metadata_strings[4];
            int thumbnail_npx=
               stringfunc::string_to_number(photo_metadata_strings[5]);
            int thumbnail_npy=
               stringfunc::string_to_number(photo_metadata_strings[6]);
//            int importance=
//               stringfunc::string_to_number(photo_metadata_strings[7]);
//            int photo_counter=
//               stringfunc::string_to_number(photo_metadata_strings[8]);

            cout << "thumbnail_URL = " << thumbnail_URL << endl;

// Look up caption for current node within *node_ids_labels_map_ptr:

            graphdbfunc::NODE_IDS_LABELS_MAP::iterator iter=
               node_ids_labels_map_ptr->find(node_ptr->get_ID());
            string caption="";
            if (iter != node_ids_labels_map_ptr->end()) 
            {
               caption=iter->second;
            }
            if (caption=="NULL") caption="";

// Note added on 2/15/11: We eventually need to convert the contents
// of string photo_timestamp into int time_stamp...

            int time_stamp=0;

            int connected_component_ID=-1;
            int parent_ID=node_ptr->get_parent_ID();
            colorfunc::RGB node_RGB=node_ptr->get_node_RGB();
//            cout << "parent_ID = " << parent_ID << endl;
//            cout << "node_RGB = " << node_RGB << endl;

            node_json_string += jsonfunc::write_data_json_string(
               n_indent,"NODE",time_stamp,caption,photo_URL,npx,npy,
               thumbnail_URL,thumbnail_npx,thumbnail_npy,
               -1,connected_component_ID,
               parent_ID,node_ptr->get_children_node_IDs(),
               node_ptr->get_Uposn(),node_ptr->get_Vposn(),
               node_RGB.first,node_RGB.second,node_RGB.third,
               node_ptr->get_relative_size());

            node_json_string += jsonfunc::indent_spaces(n_indent-2);
            node_json_string += "}";
            if (!terminal_node_flag) node_json_string += ",";
            node_json_string += "\n";

//            cout << "node_json_string = " << node_json_string << endl;
//            outputfunc::enter_continue_char();
            return node_json_string;
         }

// ==========================================================================
// SIFT features and matches insertion methods
// ==========================================================================

// Method generate_insert_sift_feature_SQL_command() takes in
// *sift_feature_ptr containing metadata for a single SIFT feature.
// It generates and returns a SQL insert command string needed to
// populate a row in the sift_features table of the TOC database.

   string generate_insert_sift_feature_SQL_command(
      sift_feature* sift_feature_ptr)
   {
//      cout << "inside photodbfunc::generate_insert_sift_feature_SQL_command()" << endl;

      int photo_ID=sift_feature_ptr->get_photo_ID();
      int feature_ID=sift_feature_ptr->get_ID();
      double u,v,theta,scale;
      sift_feature_ptr->get_u_v_orientation_scale(u,v,theta,scale);
      string descriptor="'"+sift_feature_ptr->get_descriptor_as_string()+"'";
      string visual_word="";

      string SQL_command="insert into sift_features ";
      SQL_command += "(photo_ID,feature_ID,uv_posn,orientation,scale,";
      SQL_command += "descriptor) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(photo_ID)+",";
      SQL_command += stringfunc::number_to_string(feature_ID)+",";
      SQL_command += "'SRID=4326; POINT("
         +stringfunc::number_to_string(u)+" "
         +stringfunc::number_to_string(v)+")',";
      SQL_command += stringfunc::number_to_string(theta)+",";
      SQL_command += stringfunc::number_to_string(scale)+",";
      SQL_command += descriptor;
      SQL_command += ");";
//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------
// Method generate_insert_sift_match_command() takes in IDs for two
// matching SIFT features within two separate photos.  It generates
// and returns a SQL insert command string needed to populate a row in
// the sift_matches table of the TOC database.

   string generate_insert_sift_match_SQL_command(
      int photo_ID1,int feature_ID1,int photo_ID2,int feature_ID2)
   {
//      cout << "inside photodbfunc::generate_insert_sift_match_SQL_command()" << endl;

      string SQL_command="insert into sift_matches ";
      SQL_command += "(photo_ID1,feature_ID1,photo_ID2,feature_ID2) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(photo_ID1)+",";
      SQL_command += stringfunc::number_to_string(feature_ID1)+",";
      SQL_command += stringfunc::number_to_string(photo_ID2)+",";
      SQL_command += stringfunc::number_to_string(feature_ID2);
      SQL_command += ");";
//      cout << SQL_command << endl;
      return SQL_command;
   }

// ==========================================================================
// SIFT feature & matches retrieval methods
// ==========================================================================

// Method generate_retrieve_SIFT_matches_SQL_command()

   string generate_retrieve_SIFT_matches_SQL_command(
      int photo_ID1,int photo_ID2)
   {
//      cout << "inside photodbfunc::generate_retrieve_SIFT_matches_SQL_command()" 
//           << endl;

      string SQL_command="SELECT feature_id1,feature_id2 from sift_matches";
      SQL_command += " WHERE photo_id1="+stringfunc::number_to_string(
         photo_ID1);
      SQL_command += " AND photo_id2="+stringfunc::number_to_string(
         photo_ID2);
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method retrieve_SIFT_matches_from_database() takes in the IDs for
// two images within the photos table of the TOC database.  If
// photo_ID2 > photo_ID2, the IDs are swapped.  The method then
// returns all SIFT tiepoint pairs for the two photos within output
// STL vector feature_matches.

   bool retrieve_SIFT_matches_from_database(
      gis_database* gis_database_ptr,int photo_ID1,int photo_ID2,
      vector<twovector>& feature_matches)
   {
//      cout << "inside photodbfunc::retrieve_SIFT_matches_database()" 
//           << endl;

      bool swap_flag=false;
      if (photo_ID1 > photo_ID2)
      {
         templatefunc::swap(photo_ID1,photo_ID2);
         swap_flag=true;
      }

      string curr_select_cmd=generate_retrieve_SIFT_matches_SQL_command(
         photo_ID1,photo_ID2);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int feature_ID1=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         int feature_ID2=stringfunc::string_to_number(
            field_array_ptr->get(m,1));
         if (swap_flag) templatefunc::swap(feature_ID1,feature_ID2);
         twovector curr_tiepoint_pair(feature_ID1,feature_ID2);
         feature_matches.push_back(curr_tiepoint_pair);
      } // loop over index m labeling tiepoint pairs
      
      return true;
   }



} // photodbfunc namespace



