// ==========================================================================
// Imagesdatabasefuncs namespace method definitions
// ==========================================================================
// Last modified on 4/3/14; 8/22/16; 9/5/16; 9/7/16
// ==========================================================================

#include <iostream>
#include "astro_geo/Clock.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "postgres/gis_database.h"
#include "graphs/graph.h"
#include "graphs/graph_edge.h"
#include "video/imagesdatabasefuncs.h"
#include "graphs/jsonfuncs.h"
#include "graphs/node.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "video/videosdatabasefuncs.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

namespace imagesdatabasefunc
{

// ==========================================================================
// Images database insertion methods
// ==========================================================================

// Method insert_image_metadata() takes in metadata for a new entry
// within the images table of the IMAGERY database.  It
// inserts this metadata into *gis_database_ptr.

   bool insert_image_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int importance,
      string URL,int npx,int npy,
      string thumbnail_URL,int thumbnail_npx,int thumbnail_npy)
   {
//      cout << "inside imagesdatabasefunc::insert_image_metadata()" << endl;

      string SQL_cmd=
         imagesdatabasefunc::generate_insert_image_metadata_SQL_command(
            campaign_ID,mission_ID,image_ID,importance,URL,npx,npy,
            thumbnail_URL,thumbnail_npx,thumbnail_npy);
//      cout << SQL_cmd << endl;
    
      vector<string> insert_commands;
      insert_commands.push_back(SQL_cmd);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------   
// Method generate_insert_image_metadata_SQL_command() takes in
// metadata associated with a single image.  It generates and returns
// a string containing a SQL insert command needed to populate a row
// within the images table of the IMAGERY postgis database.

   string generate_insert_image_metadata_SQL_command(
      int campaign_ID,int mission_ID,int image_ID,int importance,
      string URL,int npx,int npy,
      string thumbnail_URL,int thumbnail_npx,int thumbnail_npy)
   {
//   cout << "inside imagesdatabasefunc::generate_insert_image_metadata_SQL_command()" << endl;

      string SQL_command="insert into images ";
      SQL_command += 
         "(campaign_ID,mission_ID,image_ID,importance,URL,npx,npy,thumbnail_URL,thumbnail_npx,thumbnail_npy) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(campaign_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(image_ID)+",";
      SQL_command += stringfunc::number_to_string(importance)+",";
      SQL_command += "'"+URL+"',";
      SQL_command += stringfunc::number_to_string(npx)+",";
      SQL_command += stringfunc::number_to_string(npy)+",";
      SQL_command += "'"+thumbnail_URL+"',";
      SQL_command += stringfunc::number_to_string(thumbnail_npx)+",";
      SQL_command += stringfunc::number_to_string(thumbnail_npy);
      SQL_command += ");";

//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method generate_insert_image_metadata_SQL_command() OVERLOADED
// this version without image_ID
// It takes in metadata associated with a single image.  It generates
// and returns a string containing a SQL insert command needed to
// populate a row within the images table of the IMAGERY postgis
// database.

   string generate_insert_image_metadata_SQL_command(
      int campaign_ID,int mission_ID, 
      int importance, string URL,int npx,int npy,
      string thumbnail_URL,int thumbnail_npx,int thumbnail_npy)
   {
//   cout << "inside imagesdatabasefunc::generate_insert_image_metadata_SQL_command()" << endl;

      string SQL_command="insert into images ";
      SQL_command += 
         "(campaign_ID,mission_ID,importance,URL,npx,npy,thumbnail_URL,thumbnail_npx,thumbnail_npy) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(campaign_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(importance)+",";
      SQL_command += "'"+URL+"',";
      SQL_command += stringfunc::number_to_string(npx)+",";
      SQL_command += stringfunc::number_to_string(npy)+",";
      SQL_command += "'"+thumbnail_URL+"',";
      SQL_command += stringfunc::number_to_string(thumbnail_npx)+",";
      SQL_command += stringfunc::number_to_string(thumbnail_npy);
      SQL_command += ");";

//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method update_image_metadata() takes in metadata for an existing
// entry within the images table of the imagery database.  It
// inserts this metadata into *gis_database_ptr.

   bool update_image_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,string UTC,double epoch)
   {
//      cout << "inside imagesdatabasefunc::update_image_metadata()" << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      string curr_update_command=
         imagesdatabasefunc::generate_update_image_metadata_SQL_command(
            campaign_ID,mission_ID,image_ID,UTC,epoch);
//      cout << "update cmd = " << curr_update_command << endl;

      vector<string> update_commands;
      update_commands.push_back(curr_update_command);
      gis_database_ptr->set_SQL_commands(update_commands);
      bool flag=gis_database_ptr->execute_SQL_commands();

      return flag;
   }

// ---------------------------------------------------------------------   
// Method update_image_location() takes in metadata for an existing
// entry within the images table of the imagery database.  It
// inserts this metadata into *gis_database_ptr.

   bool update_image_location(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID)
   {
/*
//      cout << "inside imagesdatabasefunc::update_image_metadata()" << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      string curr_update_command=
         imagesdatabasefunc::generate_update_image_metadata_SQL_command(
            campaign_ID,mission_ID,image_ID,UTC,epoch);
//      cout << "update cmd = " << curr_update_command << endl;

      vector<string> update_commands;
      update_commands.push_back(curr_update_command);
      gis_database_ptr->set_SQL_commands(update_commands);
      bool flag=gis_database_ptr->execute_SQL_commands();

      return flag;
*/
      return false;
   }

// ---------------------------------------------------------------------   
// Method generate_update_image_metadata_SQL_command() 

   string generate_update_image_metadata_SQL_command(
      int campaign_ID,int mission_ID,int image_ID,
      string UTC,double epoch)
   {
//   cout << "inside imagesdatabasefunc::generate_update_image_metadata_SQL_command()" << endl;

      string SQL_command="update images ";
      SQL_command += "set UTC='"+UTC+"',";
      SQL_command += "epoch="+stringfunc::number_to_string(epoch)+" ";
      SQL_command += "where campaign_id="+stringfunc::number_to_string(
         campaign_ID)+" ";
      SQL_command += "AND mission_id="+stringfunc::number_to_string(
         mission_ID)+" ";
      SQL_command += "AND image_id="+stringfunc::number_to_string(
         image_ID)+";";
      
//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method generate_update_image_metadata_SQL_command_serialID() 
// same as above but uses serial id generated by the database

std::string generate_update_image_metadata_SQL_command_serialID(
      int campaign_ID,int mission_ID,int image_ID,
      std::string UTC,double epoch)
   {
//   cout << "inside imagesdatabasefunc::generate_update_image_metadata_SQL_command()" << endl;

      string SQL_command="update images ";
      SQL_command += "set UTC='"+UTC+"',";
      SQL_command += "epoch="+stringfunc::number_to_string(epoch)+" ";
      SQL_command += "where campaign_id="+stringfunc::number_to_string(
         campaign_ID)+" ";
      SQL_command += "AND mission_id="+stringfunc::number_to_string(
         mission_ID)+" ";
      SQL_command += "AND id="+stringfunc::number_to_string(
         image_ID)+";";
      
//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method generate_update_image_gps_SQL_command() 

   string generate_update_image_gps_SQL_command(
      int campaign_ID,int mission_ID,int image_ID, double lat, double lon)
   {
//   cout << "inside imagesdatabasefunc::generate_update_image_gps_SQL_command()" << endl;

      string SQL_command="update images ";
      SQL_command += "set the_geom= GeomFromText('POINT("
		+stringfunc::number_to_string(lon)+", "
		+stringfunc::number_to_string(lat)+")') ";
      
      SQL_command += "where campaign_id="+stringfunc::number_to_string(
         campaign_ID)+" ";
      SQL_command += "AND mission_id="+stringfunc::number_to_string(
         mission_ID)+" ";
      SQL_command += "AND image_id="+stringfunc::number_to_string(
         image_ID)+";";
      
//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method update_image_sensor_ID() takes in the sensor_ID for an existing
// entry within the images table of the imagery database.  It
// inserts this metadata into *gis_database_ptr.

   bool update_image_sensor_ID(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int sensor_ID)
   {
//      cout << "inside imagesdatabasefunc::update_image_sensor_ID()" << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      string SQL_command="update images ";
      SQL_command += "set sensor_metadata_ID="
         +stringfunc::number_to_string(sensor_ID)+" ";
      SQL_command += "where campaign_id="+stringfunc::number_to_string(
         campaign_ID)+" ";
      SQL_command += "AND mission_id="+stringfunc::number_to_string(
         mission_ID)+" ";
      SQL_command += "AND image_id="+stringfunc::number_to_string(
         image_ID)+";";

      vector<string> update_commands;
      update_commands.push_back(SQL_command);
      gis_database_ptr->set_SQL_commands(update_commands);
      bool flag=gis_database_ptr->execute_SQL_commands();

      return flag;
   }

// ==========================================================================
// Images database retrieval methods
// ==========================================================================

// Method images_in_database() queries the images table within
// *gis_database_ptr for the existence of row corresponding to
// specified campaign and mission IDs.  If no such entries are found,
// this boolean method returns false.
 
   bool images_in_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID)
   {
      string select_command="select * from images ";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
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
// Method image_in_database() queries the images table within
// *gis_database_ptr for the existence of a row corresponding to input
// datum_ID.  If no entry in the table matches datum_ID, this boolean
// method returns false.
 
   bool image_in_database(gis_database* gis_database_ptr,int datum_ID)
   {
      string select_command="select * from images ";
      select_command += " WHERE ID="+stringfunc::number_to_string(datum_ID);
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
// Method get_image_ID() takes in an image's datum ID and returns its
// image ID from the IMAGERY database.
 
   int get_image_ID(gis_database* gis_database_ptr,int datum_ID)
   {
      string select_command="select image_id from images ";
      select_command += " WHERE id="+stringfunc::number_to_string(datum_ID);
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int image_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "image_ID = "  << image_ID << endl;

      return image_ID;
   }

// ---------------------------------------------------------------------   
// Method get_image_ID() takes in an image's campaign and mission IDs
// along with its URL.  It returns the corresponding unique image ID
// from the IMAGERY database.
 
   int get_image_ID(
      gis_database* gis_database_ptr,int campaign_ID,
      int mission_ID,string URL)
   {
//      cout << "inside imagesdatabasefunc::get_image_ID()" << endl;
      
      string SQL_cmd="SELECT image_id FROM images ";
      SQL_cmd += "WHERE campaign_id="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_id="+
         stringfunc::number_to_string(mission_ID);
      SQL_cmd += " AND url='"+URL+"';";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int image_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "image_ID = "  << image_ID << endl;
//      outputfunc::enter_continue_char();
      
      return image_ID;
   }

// ---------------------------------------------------------------------   
// Method get_image__serial_ID() takes in an image's campaign and mission IDs
// along with its URL.  It returns the corresponding unique image 
// serial ID (the one generated by the database) 
// from the database referenced by gis_database_ptr.
 
   int get_image_serial_ID(
      gis_database* gis_database_ptr,int campaign_ID,
      int mission_ID, string URL)
   {
//      cout << "inside imagesdatabasefunc::get_image_ID()" << endl;
      
      string SQL_cmd="SELECT id FROM images ";
      SQL_cmd += "WHERE campaign_id="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_id="+
         stringfunc::number_to_string(mission_ID);
      SQL_cmd += " AND url='"+URL+"';";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int image_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "image_ID = "  << image_ID << endl;
//      outputfunc::enter_continue_char();
      
      return image_ID;
   }
// ---------------------------------------------------------------------   
// Method get_image_ID() takes in an image's graph hierarchy ID and
// URL.  It returns the corresponding unique image ID from the IMAGERY
// database.
 
   int get_image_ID(gis_database* gis_database_ptr,int Hierarchy_ID,string URL)
   {
//      cout << "inside imagesdatabasefunc::get_image_ID()" << endl;
      
      string SQL_cmd="SELECT DISTINCT images.image_id FROM images ";
      SQL_cmd += "INNER JOIN nodes on images.id=nodes.data_id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(Hierarchy_ID);
      SQL_cmd += " AND url='"+URL+"';";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int image_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "image_ID = "  << image_ID << endl;
      return image_ID;
   }

// ---------------------------------------------------------------------   
// Method get_datum_ID() takes in an image's graph hierarchy ID and
// URL.  It returns the corresponding unique datum ID from the IMAGERY
// database.
 
   int get_datum_ID(gis_database* gis_database_ptr,int Hierarchy_ID,string URL)
   {
//      cout << "inside imagesdatabasefunc::get_datum_ID()" << endl;
      
      string SQL_cmd="SELECT DISTINCT images.id FROM images ";
      SQL_cmd += "INNER JOIN nodes on images.id=nodes.data_id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(Hierarchy_ID);
      SQL_cmd += " AND url='"+URL+"';";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int datum_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "datum_ID = "  << datum_ID << endl;
      return datum_ID;
   }

// ---------------------------------------------------------------------   
// Method get_image_metadata_given_URL() takes in an image's URL.  It
// returns the corresponding unique integer IDs from the IMAGERY
// database.

   void get_image_metadata_given_URL(
      gis_database* gis_database_ptr,string URL,
      int& campaign_ID,int& mission_ID,int& image_ID,int& datum_ID)
   {
//      cout << "inside imagesdatabasefunc::get_image_metadata_given_URL()" 
//	     << endl;
      
      string SQL_cmd="SELECT id,campaign_ID,mission_ID,image_ID FROM images ";
      SQL_cmd += " WHERE url='"+URL+"';";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);

      campaign_ID=mission_ID=image_ID=datum_ID=-1;
      if (field_array_ptr==NULL) return;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      datum_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      campaign_ID=stringfunc::string_to_number(field_array_ptr->get(0,1));
      mission_ID=stringfunc::string_to_number(field_array_ptr->get(0,2));
      image_ID=stringfunc::string_to_number(field_array_ptr->get(0,3));
   }

// ---------------------------------------------------------------------   
// Method get_campaign_mission_image_IDs() takes in an image's datum
// ID and returns its corresponding campaign, mission and image IDs
// from the IMAGERY database.
 
   bool get_campaign_mission_image_IDs(
      gis_database* gis_database_ptr,int datum_ID,
      int& campaign_ID,int& mission_ID,int& image_ID)
   {
      string select_command=
         "select campaign_id,mission_ID,image_ID from images ";
      select_command += "WHERE ID="+stringfunc::number_to_string(datum_ID);
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      campaign_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      mission_ID=stringfunc::string_to_number(field_array_ptr->get(0,1));
      image_ID=stringfunc::string_to_number(field_array_ptr->get(0,2));
//      cout << "campaign_ID = "  << campaign_ID << endl;
//      cout << "mission_ID = "  << mission_ID << endl;
//      cout << "image_ID = "  << image_ID << endl;

      return true;
   }

// ---------------------------------------------------------------------   
// Method get_campaign_mission_IDs() takes in a Hierarchy ID
// and returns the corresponding campaign and mission IDs from the
// IMAGERY database.
 
   bool get_campaign_mission_IDs(
      gis_database* gis_database_ptr,int Hierarchy_ID,
      int& campaign_ID,int& mission_ID)
   {
      string SQL_cmd="SELECT campaign_ID,mission_ID FROM images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_ID=images.ID ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_ID="+
         stringfunc::number_to_string(Hierarchy_ID);
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      campaign_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      mission_ID=stringfunc::string_to_number(field_array_ptr->get(0,1));
//      cout << "campaign_ID = "  << campaign_ID << endl;
//      cout << "mission_ID = "  << mission_ID << endl;

      return true;
   }

// ---------------------------------------------------------------------   
// Method get_zeroth_image_npx_npy() takes in campaign and mission
// IDs for a set of images which are assumed to all have the same
// pixel width and height (e.g. cropped aerial FLIR video frames).  
// It returns npx and npy for the zeroth image in the data set.
 
   bool get_zeroth_image_npx_npy(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int& npx,int& npy)
   {
      string select_command="select npx,npy from images";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      select_command += " AND image_ID=0";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      npx=stringfunc::string_to_number(field_array_ptr->get(0,0));
      npy=stringfunc::string_to_number(field_array_ptr->get(0,1));
//      cout << "npx = "  << npx << endl;
//      cout << "npy = "  << npy << endl;

      return true;
   }

// ---------------------------------------------------------------------   
// Method get_image_URL() takes in an image's campaign, mission and
// image IDs and returns the corresponding URL stored within the
// images table of the IMAGERY database.
 
   string get_image_URL(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID)
   {
//      cout << "inside imagesdatabasefunc::get_image_URL()" << endl;
      
      string select_command="select url from images";
      select_command += " WHERE campaign_ID="+stringfunc::number_to_string(
         campaign_ID);
      select_command += " AND mission_ID="+stringfunc::number_to_string(
         mission_ID);
      select_command += " AND image_ID="+stringfunc::number_to_string(
         image_ID);
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      string URL=field_array_ptr->get(0,0);
//      cout << "URL = "  << URL << endl;

      return URL;
   }

// ---------------------------------------------------------------------   
// This overloaded version of method get_image_URL() takes in an
// image's datum ID and returns either its URL or thumbnail's URL
// stored within the images table of the IMAGERY database.

   string get_image_URL(
      gis_database* gis_database_ptr,int datum_ID,bool thumbnail_flag)
   {
//      cout << "inside imagesdatabasefunc::get_image_URL()" << endl;
//      cout << "datum_ID = " << datum_ID
//           << " thumbnail_flag = " << thumbnail_flag << endl;
      
      string select_command="select url from images ";
      if (thumbnail_flag)
      {
         select_command="select thumbnail_url from images ";
      }
      select_command += "WHERE ID="+stringfunc::number_to_string(
         datum_ID);
      
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
// This overloaded version of get_image_URL() takes in an image's
// hierarchy and node IDs and returns the corresponding URL stored
// within the images table of the IMAGERY database.
 
   string get_image_URL(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID)
   {
//      cout << "inside imagesdatabasefunc::get_image_URL()" << endl;
      string SQL_cmd="SELECT URL from images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      SQL_cmd += " AND nodes.node_ID="+stringfunc::number_to_string(
         node_ID);
//      cout << "SQL_cmd = " << SQL_cmd << endl;
      string URL="";

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
//      cout << "field_array_ptr = " << field_array_ptr << endl;
      if (field_array_ptr==NULL) return URL;
      else
      {
         return field_array_ptr->get(0,0);
      }
   }

// ---------------------------------------------------------------------   
// Method get_image_URLs() 

   bool get_image_URLs(
      int campaign_ID,int mission_ID,gis_database* gis_database_ptr,
      vector<string>& URLs)
   {
//      cout << "inside imagesdatabasefunc::get_image_URLs()" << endl;
      
      string SQL_cmd="SELECT url from images ";
      SQL_cmd += "WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_ID="+stringfunc::number_to_string(
         mission_ID);
      
      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         URLs.push_back(field_array_ptr->get(m,0));
      } // loop over index m labeling rows in *field_array_ptr

      return true;
   }
   
// ---------------------------------------------------------------------   
// Method get_image_URLs() 

   bool get_image_URLs(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID,
      vector<string>& URLs)
   {
//      cout << "inside imagesdatabasefunc::get_image_URLs()" << endl;
      
      string SQL_cmd="SELECT images.url from images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      SQL_cmd += " AND nodes.graph_ID="+stringfunc::number_to_string(
         graph_ID);
      
      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         URLs.push_back(field_array_ptr->get(m,0));
      } // loop over index m labeling rows in *field_array_ptr

      return true;
   }

// ---------------------------------------------------------------------   
   bool get_image_URLs(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,int graph_ID,
      vector<int>& campaign_IDs,vector<int>& mission_IDs,      
      vector<int>& image_IDs,vector<int>& datum_IDs,vector<string>& URLs)
   {
//      cout << "inside imagesdatabasefunc::get_image_URLs()" << endl;
      
      string SQL_cmd=
         "SELECT images.campaign_id,images.mission_id,images.image_id,images.id,images.url from images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      SQL_cmd += " AND nodes.graph_ID="+stringfunc::number_to_string(
         graph_ID);
      SQL_cmd += " ORDER BY images.image_id";
      
      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int campaign_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,0));
         int mission_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,1));
         int image_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,2));
         int datum_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,3));
         string URL=field_array_ptr->get(m,4);

         campaign_IDs.push_back(campaign_ID);
         mission_IDs.push_back(mission_ID);
         image_IDs.push_back(image_ID);
         datum_IDs.push_back(datum_ID);
         URLs.push_back(URL);
      } // loop over index m labeling rows in *field_array_ptr

      return true;
   }

// ---------------------------------------------------------------------   
// Method get_datum_ID() takes in a image's campaign, mission and image IDs.
// It returns the corresponding unique serial ID assigned by the
// images table within the IMAGERY database.
 
   int get_datum_ID(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID)
   {
      string select_command="select id from images ";
      select_command += " WHERE campaign_ID="+stringfunc::number_to_string(
         campaign_ID);
      select_command += " AND mission_ID="+stringfunc::number_to_string(
         mission_ID);
      select_command += " AND image_ID="+stringfunc::number_to_string(
         image_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int datum_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "datum_ID = "  << datum_ID << endl;

      return datum_ID;
   }

// ---------------------------------------------------------------------   
   bool retrieve_image_ID_URL_given_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      int& image_ID,string& image_URL,string& thumbnail_URL)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_ID_URL_given_node_ID()" 
//           << endl;

      string curr_select_cmd="select data_id,url,thumbnail_url from images ";
      curr_select_cmd += "inner join nodes on images.image_id=nodes.data_id ";
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

      image_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      image_URL=field_array_ptr->get(0,1);
      thumbnail_URL=field_array_ptr->get(0,2);
      return true;
   }

// ---------------------------------------------------------------------   
// Method get_node_ID() takes in an image's campaign, mission and
// image IDs.  It returns the node ID corresponding to graph_ID within
// the nodes table of the IMAGERY database.

    int get_node_ID(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,int graph_ID)
   {
      string SQL_cmd="SELECT node_id FROM nodes ";
      SQL_cmd += "INNER JOIN images ON nodes.data_id=images.id ";
      SQL_cmd += " WHERE images.campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND images.mission_ID="+stringfunc::number_to_string(
         mission_ID);
      SQL_cmd += " AND images.image_ID="+stringfunc::number_to_string(
         image_ID);
      SQL_cmd += " AND nodes.graph_ID="+stringfunc::number_to_string(
         graph_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int node_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "node_ID = "  << node_ID << endl;

      return node_ID;
   }

// ---------------------------------------------------------------------   
   bool retrieve_particular_image_metadata(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      int& campaign_ID,int& mission_ID,int& image_ID,double& epoch)
   {
      string SQL_cmd=
         "SELECT campaign_id,mission_ID,image_ID,epoch FROM images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      SQL_cmd += " AND nodes.node_id="+
         stringfunc::number_to_string(node_ID);
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      campaign_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      mission_ID=stringfunc::string_to_number(field_array_ptr->get(0,1));
      image_ID=stringfunc::string_to_number(field_array_ptr->get(0,2));
      epoch=stringfunc::string_to_number(field_array_ptr->get(0,3));
//      cout << "campaign_ID = "  << campaign_ID << endl;
//      cout << "mission_ID = "  << mission_ID << endl;
//      cout << "image_ID = "  << image_ID << endl;

      return true;
   }

// ---------------------------------------------------------------------   
   bool retrieve_particular_image_metadata_from_database(
      gis_database* gis_database_ptr,int datum_ID,int& image_ID,
      int& importance,string& image_timestamp,double& image_epoch,
      string& image_URL,int& npx,int& npy,
      string& thumbnail_URL,int& thumbnail_npx,int& thumbnail_npy)
   {
//      cout << "inside imagesdatabasefunc::retrieve_particular_image_metadata_from_database() #1" 
//           << endl;

      string curr_select_cmd=generate_retrieve_particular_image_SQL_command(
         datum_ID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      image_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      importance=stringfunc::string_to_number(field_array_ptr->get(0,1));
      image_timestamp=field_array_ptr->get(0,2);
      image_epoch=stringfunc::string_to_number(field_array_ptr->get(0,3));
      image_URL=field_array_ptr->get(0,4);
      npx=stringfunc::string_to_number(field_array_ptr->get(0,5));
      npy=stringfunc::string_to_number(field_array_ptr->get(0,6));
      thumbnail_URL=field_array_ptr->get(0,7);
      thumbnail_npx=stringfunc::string_to_number(field_array_ptr->get(0,8));
      thumbnail_npy=stringfunc::string_to_number(field_array_ptr->get(0,9));
      return true;
   }

// ---------------------------------------------------------------------   
// Method generate_retrieve_particular_image_SQL_command()

   string generate_retrieve_particular_image_SQL_command(int datum_ID)
   {
//      cout << "inside imagesdatabasefunc::generate_retrieve_particular_image_SQL_command()" 
//           << endl;

      string SQL_command="SELECT image_ID,importance,utc,epoch,url,npx,npy,thumbnail_url,thumbnail_npx,thumbnail_npy from images ";
      SQL_command += "WHERE id="
         +stringfunc::number_to_string(datum_ID)+";";
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method retrieve_particular_image_metadata_from_database()

   bool retrieve_particular_image_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      string image_URL,int& datum_ID,int& image_ID,int& npx,int& npy)
   {
//      cout << "inside imagesdatabasefunc::retrieve_particular_image_metadata_from_database()" 
//           << endl;

      string SQL_command="SELECT ID,image_ID,npx,npy from images ";
      SQL_command += "WHERE campaign_ID="
         +stringfunc::number_to_string(campaign_ID);
      SQL_command += " AND mission_ID="
         +stringfunc::number_to_string(mission_ID);
      SQL_command += " AND URL='"+image_URL+"'";

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_command);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      datum_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      image_ID=stringfunc::string_to_number(field_array_ptr->get(0,1));
      npx=stringfunc::string_to_number(field_array_ptr->get(0,2));
      npy=stringfunc::string_to_number(field_array_ptr->get(0,3));
      return true;
   }

// ---------------------------------------------------------------------   
   bool retrieve_image_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      vector<int>& image_IDs,vector<int>& datum_IDs,
      vector<string>& URLs)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_metadata_from_database #1()" 
//           << endl;

      string curr_select_cmd="SELECT image_ID,ID,URL from images ";
      curr_select_cmd += "WHERE campaign_ID="+stringfunc::number_to_string(
         campaign_ID);
      curr_select_cmd += " AND mission_ID="+stringfunc::number_to_string(
         mission_ID);
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
         int curr_image_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,0));
         int curr_datum_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,1));
         string curr_URL=field_array_ptr->get(m,2);
         image_IDs.push_back(curr_image_ID);
         datum_IDs.push_back(curr_datum_ID);
         URLs.push_back(curr_URL);
      } // loop over index m labeling rows in *field_array_ptr

      return true;
   }

// ---------------------------------------------------------------------   
   bool retrieve_image_metadata_from_database(
      gis_database* gis_database_ptr,int HierarchyID,int GraphID,
      vector<int>& datum_IDs,vector<int>& image_IDs,
      vector<double>& epoch_times,vector<string>& thumbnail_URLs)
   {
      cout << "inside imagesdatabasefunc::retrieve_image_metadata_from_database() #2" 
           << endl;

      string curr_select_cmd=
         "SELECT data_ID,image_ID,epoch,thumbnail_URL from images ";
      curr_select_cmd += "INNER JOIN nodes ON images.id=nodes.data_id ";
      curr_select_cmd += "WHERE nodes.graph_hierarchy_ID="+
         stringfunc::number_to_string(HierarchyID);
      curr_select_cmd += " AND nodes.graph_ID="+stringfunc::number_to_string(
         GraphID);
      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
      unsigned int mdim=field_array_ptr->get_mdim();
      cout << "mdim = " << mdim
           << " ndim = " << field_array_ptr->get_ndim() << endl;

      datum_IDs.clear();
      image_IDs.clear();
      epoch_times.clear();
      thumbnail_URLs.clear();
      
      datum_IDs.reserve(mdim);
      image_IDs.reserve(mdim);
      epoch_times.reserve(mdim);
      thumbnail_URLs.reserve(mdim);

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int curr_datum_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,0));
         int curr_image_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,1));
         double curr_epoch=
            stringfunc::string_to_number(field_array_ptr->get(m,2));
         string curr_thumbnail_URL=field_array_ptr->get(m,3);
         datum_IDs.push_back(curr_datum_ID);
         image_IDs.push_back(curr_image_ID);
//         cout << "image_ID = " << image_IDs.back() << endl;
         
         epoch_times.push_back(curr_epoch);
         thumbnail_URLs.push_back(curr_thumbnail_URL);
      } // loop over index m labeling rows in *field_array_ptr

      cout << "epoch_times.size() = " << epoch_times.size() << endl;

      return true;
   }

// ---------------------------------------------------------------------   
   bool retrieve_image_metadata_from_database(
      gis_database* gis_database_ptr,int HierarchyID,int GraphID,
      vector<int>& node_IDs,vector<int>& image_IDs,
      vector<double>& epoch_times)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_metadata_from_database() #3" << endl;

      string curr_select_cmd=
         "SELECT nodes.node_ID,image_ID,epoch from images ";
      curr_select_cmd += "INNER JOIN nodes ON images.id=nodes.data_id ";
      curr_select_cmd += "WHERE nodes.graph_hierarchy_ID="+
         stringfunc::number_to_string(HierarchyID);
      curr_select_cmd += " AND nodes.graph_ID="+stringfunc::number_to_string(
         GraphID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
      unsigned int mdim=field_array_ptr->get_mdim();

      node_IDs.reserve(mdim);
      image_IDs.reserve(mdim);
      epoch_times.reserve(mdim);

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int curr_node_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,0));
         int curr_image_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,1));
         double curr_epoch=
            stringfunc::string_to_number(field_array_ptr->get(m,2));
         node_IDs.push_back(curr_node_ID);
         image_IDs.push_back(curr_image_ID);
//         cout << "image_ID = " << image_IDs.back() << endl;
         epoch_times.push_back(curr_epoch);
      } // loop over index m labeling rows in *field_array_ptr

      return true;
   }

// ---------------------------------------------------------------------   
   bool retrieve_image_metadata_from_database(
      gis_database* gis_database_ptr,int HierarchyID,int GraphID,
      vector<int>& node_IDs,vector<double>& epoch_times,
      vector<string>& image_URLs)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_metadata_from_database() #3" << endl;

      string curr_select_cmd=
         "SELECT nodes.node_ID,image_ID,epoch,URL from images ";
      curr_select_cmd += "INNER JOIN nodes ON images.id=nodes.data_id ";
      curr_select_cmd += "WHERE nodes.graph_hierarchy_ID="+
         stringfunc::number_to_string(HierarchyID);
      curr_select_cmd += " AND nodes.graph_ID="+stringfunc::number_to_string(
         GraphID);
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
      unsigned int mdim=field_array_ptr->get_mdim();

      node_IDs.reserve(mdim);
      epoch_times.reserve(mdim);

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int curr_node_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,0));
//         int curr_image_ID=
//            stringfunc::string_to_number(field_array_ptr->get(m,1));
         double curr_epoch=
            stringfunc::string_to_number(field_array_ptr->get(m,2));
         string curr_URL=field_array_ptr->get(m,3);

         node_IDs.push_back(curr_node_ID);
         epoch_times.push_back(curr_epoch);
         image_URLs.push_back(curr_URL);
      } // loop over index m labeling rows in *field_array_ptr

      return true;
   }

// ==========================================================================
// Temporal neighbor retrieval methods
// ==========================================================================

   int retrieve_sensor_metadata_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      int node_ID)
   {
      cout << "inside imagesdatabasefunc::retrieve_sensor_metadata_ID()"
           << endl;
      
      string SQL_cmd="SELECT sensor_metadata_id FROM images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      SQL_cmd += " AND nodes.graph_id="+
         stringfunc::number_to_string(graph_ID);
      SQL_cmd += " AND nodes.node_id= "+
         stringfunc::number_to_string(node_ID);
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int sensor_metadata_ID=stringfunc::string_to_number(
         field_array_ptr->get(0,0));
      return sensor_metadata_ID;
   }

// ---------------------------------------------------------------------   
   int retrieve_beginning_temporal_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      double& new_epoch)
   {
      cout << "inside imagesdatabasefunc::retrieve_beginning_temporal_node_ID()"
           << endl;
      
      double curr_epoch;
      if (!retrieve_particular_image_epoch(
         gis_database_ptr,hierarchy_ID,node_ID,curr_epoch))
      {
         return -1;
      }
      
      string SQL_cmd="SELECT nodes.node_ID,epoch FROM images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      SQL_cmd += " AND nodes.graph_id=0 ";
      SQL_cmd += " ORDER BY epoch";
      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int beginning_node_ID=stringfunc::string_to_number(
         field_array_ptr->get(0,0));
      new_epoch=stringfunc::string_to_number(field_array_ptr->get(0,1));
      return beginning_node_ID;
   }

// ---------------------------------------------------------------------   
   int retrieve_prev_temporal_neighbor_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      int sensor_ID,double& new_epoch)
   {
      cout << endl;
      cout << "inside imagesdatabasefunc::retrieve_prev_temporal_neighbor_node_ID()" << endl;
      cout << "sensor_ID = " << sensor_ID << endl;
      
      double curr_epoch;
      if (!retrieve_particular_image_epoch(
         gis_database_ptr,hierarchy_ID,node_ID,curr_epoch))
      {
         return -1;
      }
      
      string SQL_cmd="SELECT nodes.node_ID,epoch,sensor_metadata_ID ";
      SQL_cmd += "FROM images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      SQL_cmd += " AND nodes.graph_id=0 ";
      SQL_cmd += " AND epoch < "+stringfunc::number_to_string(curr_epoch);
      SQL_cmd += " ORDER BY epoch";
      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return -1;

      unsigned int mdim=field_array_ptr->get_mdim();
      unsigned int ndim=field_array_ptr->get_ndim();

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
      cout << "*field_array_ptr = " << *field_array_ptr << endl;
      cout << "mdim = " << mdim
           << " ndim = " << ndim << endl;

      new_epoch=stringfunc::string_to_number(field_array_ptr->get(mdim-1,1));
      double curr_new_epoch;

      cout.precision(12);
      cout << "new_epoch = " << new_epoch << endl;
      int m=mdim-1;
      bool sensor_OK_flag;
      do 
      {
         cout << "m = " << m << endl;
         curr_new_epoch=
            stringfunc::string_to_number(field_array_ptr->get(m,1));
         cout << "curr_new_epoch = " << curr_new_epoch << endl;

         if (ndim <= 2) 
         {
            sensor_OK_flag=true;
         }
         else
         {
            int sensor_metadata_ID=
               stringfunc::string_to_number(field_array_ptr->get(m,2));
            cout << "sensor_metadata_ID = " << sensor_metadata_ID
                 << " sensor_ID = " << sensor_ID << endl;
            if (sensor_metadata_ID==sensor_ID) 
            {
               sensor_OK_flag=true;
            }
            else
            {
               sensor_OK_flag=false;
            }
         }
         m--;
      }
      while (m >= 0 && nearly_equal(new_epoch,curr_new_epoch) &&
      !sensor_OK_flag);

      cout << "m = " << m << endl;

      int prev_node_ID=stringfunc::string_to_number(
         field_array_ptr->get(m+1,0));
      cout << "prev_node_ID = " << prev_node_ID << endl;
      return prev_node_ID;
   }

// ---------------------------------------------------------------------   
   int retrieve_next_temporal_neighbor_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      int sensor_ID,double& new_epoch)
   {
      cout << endl;
      cout << "inside imagesdatabasefunc::retrieve_next_temporal_neighbor_node_ID()" << endl;
      cout << "sensor_ID = " << sensor_ID << endl;
      
      double curr_epoch;
      if (!retrieve_particular_image_epoch(
         gis_database_ptr,hierarchy_ID,node_ID,curr_epoch))
      {
         return -1;
      }
      
      string SQL_cmd="SELECT nodes.node_ID,epoch,sensor_metadata_id ";
      SQL_cmd += "FROM images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      SQL_cmd += " AND nodes.graph_id=0 ";
      SQL_cmd += " AND epoch > "+stringfunc::number_to_string(curr_epoch);
      SQL_cmd += " ORDER BY epoch";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
      unsigned int mdim=field_array_ptr->get_mdim();
      unsigned int ndim=field_array_ptr->get_ndim();
      cout << "mdim = " << mdim
           << " ndim = " << ndim << endl;

      new_epoch=stringfunc::string_to_number(field_array_ptr->get(0,1));
      double curr_new_epoch;

      unsigned int m=0;
      bool sensor_OK_flag;
      do 
      {
         curr_new_epoch=
            stringfunc::string_to_number(field_array_ptr->get(m,1));
         if (ndim <= 2) 
         {
            sensor_OK_flag=true;
         }
         else
         {
            int sensor_metadata_ID=
               stringfunc::string_to_number(field_array_ptr->get(m,2));
            if (sensor_metadata_ID==sensor_ID) 
            {
               sensor_OK_flag=true;
            }
            else
            {
               sensor_OK_flag=false;
            }
         }
         m++;
      }
      while (m < mdim && nearly_equal(new_epoch,curr_new_epoch) &&
      !sensor_OK_flag);

      int next_node_ID=stringfunc::string_to_number(
         field_array_ptr->get(m-1,0));
      cout << "next_node_ID = " << next_node_ID << endl;
      return next_node_ID;
   }

// ---------------------------------------------------------------------   
   int retrieve_ending_temporal_node_ID(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      double& new_epoch)
   {
//      cout << "inside imagesdatabasefunc::retrieve_ending_temporal_node_ID()"
//           << endl;

      double curr_epoch;
      if (!retrieve_particular_image_epoch(
         gis_database_ptr,hierarchy_ID,node_ID,curr_epoch))
      {
         return -1;
      }
      
      string SQL_cmd="SELECT nodes.node_ID,epoch FROM images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      SQL_cmd += " AND nodes.graph_id=0 ";
      SQL_cmd += " ORDER BY epoch";
      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return -1;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      unsigned int ending_node_ID=stringfunc::string_to_number(
         field_array_ptr->get(mdim-1,0));
      new_epoch=stringfunc::string_to_number(field_array_ptr->get(mdim-1,1));
      return ending_node_ID;
   }

// ---------------------------------------------------------------------   
   bool retrieve_particular_image_epoch(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      double& epoch)
   {
      string SQL_cmd="SELECT epoch FROM images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      SQL_cmd += " AND nodes.node_id="+
         stringfunc::number_to_string(node_ID);
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      epoch=stringfunc::string_to_number(field_array_ptr->get(0,0));
      return true;
   }

// ==========================================================================
// SIFT matching database insertion methods
// ==========================================================================

// Method generate_insert_sift_match_SQL_command() takes in IDs for two
// matching SIFT features within two separate images.  It generates
// and returns a SQL insert command string needed to populate a row in
// the sift_matches table of the IMAGERY database.

   string generate_insert_sift_match_SQL_command(
      int campaign_ID1,int mission_ID1,int image_ID1,int feature_ID1,
      int campaign_ID2,int mission_ID2,int image_ID2,int feature_ID2)
   {
//      cout << "inside imagesdatabasefunc::generate_insert_sift_match_SQL_command()" << endl;

      string SQL_command="insert into sift_matches ";
      SQL_command += "(campaign_ID1,mission_ID1,image_ID1,feature_ID1,campaign_ID2,mission_ID2,image_ID2,feature_ID2) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(campaign_ID1)+",";
      SQL_command += stringfunc::number_to_string(mission_ID1)+",";
      SQL_command += stringfunc::number_to_string(image_ID1)+",";
      SQL_command += stringfunc::number_to_string(feature_ID1)+",";
      SQL_command += stringfunc::number_to_string(campaign_ID2)+",";
      SQL_command += stringfunc::number_to_string(mission_ID2)+",";
      SQL_command += stringfunc::number_to_string(image_ID2)+",";
      SQL_command += stringfunc::number_to_string(feature_ID2);
      SQL_command += ");";
//      cout << SQL_command << endl;
      return SQL_command;
   }

// ==========================================================================
// Image graph database querying methods
// ==========================================================================

// Method write_graph_json_string() takes in *graph_ptr along with
// boolean flag get_nodes_flag and get_edges_flag.  It outputs a JSON
// string containing node and edge information depending upon these
// flags' values.  If STL vector incident_node_IDs is non-empty, only
// edges adjacent to input nodes are written to output JSON string.

   string write_graph_json_string(
      gis_database* gis_database_ptr,int hierarchy_ID,graph* graph_ptr,
      bool get_nodes_flag,bool get_edges_flag,bool get_annotations_flag,
      const vector<int>& incident_node_IDs)
   {
      cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
      cout << "inside imagesdatabasefunc::write_graph_json_string()" << endl;
      cout << "incident_node_IDs.size() = "
           << incident_node_IDs.size() << endl;
      cout << "graph_ptr = " << graph_ptr << endl;
      cout << "graph_ptr->get_ID() = " << graph_ptr->get_ID() << endl;

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
         json_string += write_nodes_json_string(
            gis_database_ptr,hierarchy_ID,graph_ptr->get_ID());
         
         if (get_edges_flag || get_annotations_flag)
         {
            json_string += ",";
         }
         json_string += "\n";

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

            double max_weight = 100;
            double min_weight = 0;
            int edge_weight = curr_matches - (max_weight+min_weight)/2; 
            int min_abs_weight = 5;

            if (e==graph_edge_ptrs.size()-1) terminal_edge_flag=true;

            if (curr_matches > 0)
//            if (curr_matches > 0 && abs(edge_weight) > min_abs_weight)
            {
               node* node1_ptr=graph_edge_ptr->get_node1_ptr();
               node* node2_ptr=graph_edge_ptr->get_node2_ptr();
         
// FAKE FAKE:  Mon Aug 22 at 4:30 am

               colorfunc::RGB edge_RGB=graph_ptr->
                  compute_edge_color(curr_matches, max_weight, min_weight);
//                  compute_edge_color(curr_matches);

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
                  curr_matches,
//                  edge_weight,
                  edge_RGB.first,edge_RGB.second,edge_RGB.third,
                  relative_edge_thickness,terminal_edge_flag);
            }
         } // loop over index e labeling graph edges

         json_string += "    ]";

         if (get_annotations_flag)
         {
            json_string += ",";
         }
         json_string += "\n";

      } // get_edges_flag conditional

// Write out graph annotations:
      
      if (get_annotations_flag)
      {
         cout << "Writing out graph annotations:" << endl;

         vector<int> layouts;
         vector<double> gxs,gys,sizes;
         vector<string> labels,colors;
         graphdbfunc::retrieve_graph_annotations_from_database(
            gis_database_ptr,hierarchy_ID,graph_ptr->get_ID(),
            layouts,gxs,gys,labels,colors,sizes);

         json_string += "    \"annotation\": [ \n";
         for (unsigned int a=0; a<labels.size(); a++)
         {
            json_string += "         { \n";
            json_string += "           \"ID\": "+
               stringfunc::number_to_string(a)+", \n";
            json_string += "           \"data\": { \n";
            json_string += "               \"layout\":  "+
               stringfunc::number_to_string(layouts[a])+", \n";
            json_string += "               \"gx\":  "+
               stringfunc::number_to_string(gxs[a])+", \n";
            json_string += "               \"gy\":  "+
               stringfunc::number_to_string(gys[a])+", \n";
            json_string += "               \"label\":  \""+
               labels[a]+"\", \n";
            json_string += "               \"size\":  \""+
               stringfunc::number_to_string(sizes[a])+"\", \n";

            colorfunc::Color c=colorfunc::string_to_color(colors[a]);
            colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(c);
            json_string += "               \"rgbColor\": ["+
               stringfunc::number_to_string(curr_RGB.first)+","+
               stringfunc::number_to_string(curr_RGB.second)+","+
               stringfunc::number_to_string(curr_RGB.third)+"]\n";
            json_string += "            } \n";

            json_string += "         }";
            if (a<labels.size()-1) json_string += ",";
            json_string += "\n";

         } // loop over index a labeling graph annotations
         json_string += "    ]\n";
      }

      json_string += "  } \n";
      json_string += "} \n";

//      cout << "Finished constructing JSON string for graph = " 
//           << json_string << endl;
      return json_string;
   }

// ---------------------------------------------------------------------
// Method reset_nodes_metadata() queries the user to enter an index
// for some set of Facenet node activations corresponding to a
// particular test image.  For each facenet node, this method imports
// a text file containing Pcum, R, G, B as a function of old node ID
// into an STL map.  Looping over all node IDs, this method then
// resets the relative size and color of nodes with nonzero
// activations.  If a node has zero activation, its thumbnail URL is
// reset to a dark grey image chip.

// We wrote this highly specialized method on 9/5/16 for facenet
// visualization purposes.

   void reset_nodes_metadata(
      vector<int>& node_ID, vector<double>& relative_size, 
      vector<string>& color, vector<string>& thumbnail_URL)
   {
      string projects_subdir = "/home/pcho/programs/c++/git/projects/";
      string caffe_subdir=projects_subdir+"src/mains/machine_learning/caffe/";
      string activations_subdir=caffe_subdir+
         "vis_facenet/node_images/activations/";
      string renorm_subdir=activations_subdir+"images/renormalized/";
      
      int image_activations_index;
      cout << "Enter image activations index:" << endl;
      cin >> image_activations_index;
      string index_str = stringfunc::integer_to_string(
         image_activations_index,4);

      string image_activations_filename=renorm_subdir+
         "image_activations_"+index_str+".dat";
      filefunc::ReadInfile(image_activations_filename);
      string image_filename = filefunc::text_line[0];
      
      typedef std::map<int, fourvector> NODE_ACTIVATIONS_MAP;
      NODE_ACTIVATIONS_MAP node_activations_map;
      NODE_ACTIVATIONS_MAP::iterator node_activations_iter;

// Independent int contains old global node ID
// Dependent fourvector contains Pcum, r, g, b

      for(unsigned int i = 1; i < filefunc::text_line.size(); i++)
      {
         vector<double> curr_vals = stringfunc::string_to_numbers(
            filefunc::text_line[i]);
         int layer_ID = curr_vals[0];
         int old_local_node_ID = curr_vals[1];
         int old_global_node_ID = curr_vals[2];
         int new_local_node_ID = curr_vals[3];
         int new_global_node_ID = curr_vals[4];

         double Pcum = curr_vals[5];
         double r = curr_vals[6] / 255.0;
         double g = curr_vals[7] / 255.0;
         double b = curr_vals[8] / 255.0;
         fourvector f(Pcum, r, g, b);
         node_activations_map[old_global_node_ID] = f;
//         node_activations_map[new_global_node_ID] = f;
      } // loop over index i 

      for(unsigned int i = 0; i < node_ID.size(); i++)
      {
         node_activations_iter = node_activations_map.find(node_ID[i]);
         if(node_activations_iter == node_activations_map.end())
         {
            thumbnail_URL[i] = 
               "/data/ImageEngine/facenet/thumbnails/thumbnail_dark_grey.jpg";
            continue;
         }

         fourvector f = node_activations_iter->second;
         double Pcum = f.get(0);
         double r = f.get(1);
         double g = f.get(2);
         double b = f.get(3);
         
         relative_size[i] = 12;
         color[i] = colorfunc::RGB_to_RRGGBB_hex(r,g,b);
      } // loop over index i 
   }

/*

// ---------------------------------------------------------------------
// This overloaded version of reset_nodes_metadata() was written for
// visualizing overall stimulation frequencies and nonzero median
// activations for every node within Facenet 1.

   void reset_nodes_metadata(
      vector<int>& node_ID, vector<double>& relative_size, 
      vector<string>& color, vector<string>& thumbnail_URL)
   {
      string projects_subdir = "/home/pcho/programs/c++/git/projects/";
      string caffe_subdir=projects_subdir+"src/mains/machine_learning/caffe/";
      string activations_subdir=caffe_subdir+
         "vis_facenet/node_images/activations/";
      string ordered_activations_filename=activations_subdir+
         "ordered_activations.dat";

      vector< vector<double> > row_values = filefunc::ReadInRowNumbers(
         ordered_activations_filename);

      typedef std::map<int, fourvector> NODE_ACTIVATIONS_MAP;
      NODE_ACTIVATIONS_MAP node_activations_map;
      NODE_ACTIVATIONS_MAP::iterator node_activations_iter;

// Independent int contains old global node ID
// Dependent fourvector contains stimul_freq/median nonzero activation frac, 
// r, g, b

      for(unsigned int i = 0; i < row_values.size(); i++)
      {
         vector<double> curr_vals = row_values[i];
         int layer_ID = curr_vals[0];
         int old_local_node_ID = curr_vals[1];
         int old_global_node_ID = curr_vals[2];
         int new_local_node_ID = curr_vals[3];
         int new_global_node_ID = curr_vals[4];

         double stimul_freq = curr_vals[5];
         double median_nonzero_activation_frac = curr_vals[7];
//         double frac = stimul_freq;
         double frac = median_nonzero_activation_frac;

         double h = 250 * (1 - frac);
         double s = 1;
         double v = 0.4 * (1 + frac);
         
         double r, g, b;
         colorfunc::hsv_to_RGB(h, s, v, r, g, b);

//         const double TINY = 1E-5;
         const double TINY = -1E-5;
         if(frac < TINY)
         {
            r = g = b = 0.2;
         }
         
         fourvector f(stimul_freq, r, g, b);
         node_activations_map[old_global_node_ID] = f;
      } // loop over index i 

      for(unsigned int i = 0; i < node_ID.size(); i++)
      {
         node_activations_iter = node_activations_map.find(node_ID[i]);
         if(node_activations_iter == node_activations_map.end())
         {
            thumbnail_URL[i] = 
               "/data/ImageEngine/facenet/thumbnails/thumbnail_dark_grey.jpg";
            continue;
         }

         fourvector f = node_activations_iter->second;
         double stimul_freq = f.get(0);
         double r = f.get(1);
         double g = f.get(2);
         double b = f.get(3);
         
//         relative_size[i] = 12;
         color[i] = colorfunc::RGB_to_RRGGBB_hex(r,g,b);
      } // loop over index i 
   }

*/

   
// ---------------------------------------------------------------------   
// Method write_nodes_json_string() takes in hierarchy and graph IDs.
// It performs a single database call to retrieve all node metadata
// for the specified graph and another single database call to
// retrieve all image attributes for the graph.  This method
// explicitly constructs the JSON string which needs to be sent to
// Michael Yee's GraphViewer.  We have tried to streamline this method
// so that image server responses to Get_Graph calls issued by the
// GraphViewer execute fairly quickly.

   string write_nodes_json_string(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID)
   {
//      cout << "inside imagesdatabasefunc::write_nodes_json_string()" << endl;

      int campaign_ID,mission_ID;
      imagesdatabasefunc::get_campaign_mission_IDs(
         gis_database_ptr,hierarchy_ID,campaign_ID,mission_ID);

      cout << "hierarchy_ID = " << hierarchy_ID
           << " graph_ID = " << graph_ID << endl;
      cout << "campaign_ID = " << campaign_ID
           << " mission_ID = " << mission_ID << endl;

      cout << "Writing out nodes:" << endl;

      vector<int> node_ID,parent_node_ID,npx,npy,thumbnail_npx,thumbnail_npy;
      vector<double> epoch,gx,gy,gx2,gy2,relative_size;
      vector<string> URL,thumbnail_URL,color,label;

      retrieve_nodes_metadata(
         gis_database_ptr,hierarchy_ID,graph_ID,
         node_ID,epoch,URL,npx,npy,thumbnail_URL,thumbnail_npx,thumbnail_npy,
         parent_node_ID,gx,gy,gx2,gy2,relative_size,color,label);

// FAKE FAKE Mon Sep 5 at 10:46 am 

// For facenet visualization purposes, reset nodes' colors, sizes and
// thumbnail URLs based upon their activation responses to a
// particular input test image:

      if(hierarchy_ID == 47)
      {
         reset_nodes_metadata(node_ID, relative_size, color, thumbnail_URL);
      }

      ATTRIBUTES_METADATA_MAP* image_annotations_map_ptr=
         retrieve_all_image_annotations(
            gis_database_ptr,campaign_ID,mission_ID);

      ATTRIBUTES_METADATA_MAP* attributes_metadata_map_ptr=
         retrieve_all_attributes(gis_database_ptr,campaign_ID,mission_ID);

      ATTRIBUTES_METADATA_MAP* color_histograms_map_ptr=
         retrieve_all_image_color_histograms(
         gis_database_ptr,campaign_ID,mission_ID);

      ATTRIBUTES_METADATA_MAP* human_faces_map_ptr=
         retrieve_all_human_faces(gis_database_ptr,campaign_ID,mission_ID);

      ATTRIBUTES_METADATA_MAP* video_keyframes_map_ptr=
         videosdatabasefunc::retrieve_all_video_keyframes(
            gis_database_ptr,hierarchy_ID,campaign_ID,mission_ID);

      string json_string = "     'node': [ \n";
      for (unsigned int n=0; n<node_ID.size(); n++)
      {
         string epoch_milliseconds;
         if (epoch[n] > 0)
         {

// As of 2/16/2012, Michael Yee requests that epoch time values be
// reported as integers specifying millseconds since midnight 1 Jan 1970:
               
            epoch_milliseconds=stringfunc::number_to_string(
               1000*epoch[n],0);
         }
         double r,g,b;
         colorfunc::RRGGBB_hex_to_rgb(color[n],r,g,b);

         json_string += "     { \n";
         json_string += "       'id': "+stringfunc::number_to_string(
            node_ID[n])+", \n";
         json_string += "       'data': { \n";
         json_string += "         'type': 'NODE',\n";
         if (epoch_milliseconds.size() > 0)
         {
            json_string += "         'epoch_millisecs': '"
               +epoch_milliseconds+"',\n";
         }
         json_string += "         'photoURL': '"+URL[n]+"',\n";
         json_string += "         'photo_Npx': "+
            stringfunc::number_to_string(npx[n])+",\n";
         json_string += "         'photo_Npy': "+
            stringfunc::number_to_string(npy[n])+",\n";
         json_string += "         'thumbnailURL': '"+thumbnail_URL[n]+"',\n";
         json_string += "         'thumbnail_Npx': "+
            stringfunc::number_to_string(thumbnail_npx[n])+",\n";
         json_string += "         'thumbnail_Npy': "+
            stringfunc::number_to_string(thumbnail_npy[n])+",\n";
         json_string += "         'parent_ID': "+
            stringfunc::number_to_string(parent_node_ID[n])+",\n";
         json_string += "         'gx': "+
            stringfunc::number_to_string(gx[n])+",\n";
         json_string += "         'gy': "+
            stringfunc::number_to_string(gy[n])+",\n";
         if (gx2[n] > 0.5*NEGATIVEINFINITY)
         {
            json_string += "         'gx2': "+
               stringfunc::number_to_string(gx2[n])+",\n";
            json_string += "         'gy2': "+
               stringfunc::number_to_string(gy2[n])+",\n";
         }

         json_string += "         'relativeSize': "+
            stringfunc::number_to_string(relative_size[n])+",\n";
  
         if (label[n].size() > 0)
         {
            json_string += "         'label': '"+label[n]+"',\n";
         }

         add_node_attributes_to_json_string(
            node_ID[n],json_string,
            image_annotations_map_ptr,attributes_metadata_map_ptr,
            color_histograms_map_ptr,human_faces_map_ptr,
            video_keyframes_map_ptr);

         json_string += "         'rgbColor': ["+
            stringfunc::number_to_string(r)+","+
            stringfunc::number_to_string(g)+","+
            stringfunc::number_to_string(b)+"]\n";
         json_string += "        }\n";
         json_string += "     }";

         if (n < node_ID.size()-1) 
         {
            json_string += ",";
         }
         json_string += "\n";
      } // loop over index n labeling nodes
      json_string += "    ]";      

      delete image_annotations_map_ptr;
      delete attributes_metadata_map_ptr;
      delete color_histograms_map_ptr;
      delete human_faces_map_ptr;
      delete video_keyframes_map_ptr;

//      cout << "json_string.size() = " << json_string.size() << endl;
//      cout << "json_string = " << json_string << endl;
      return json_string;
   }

// ---------------------------------------------------------------------   
// Method add_node_attributes_to_json_string() takes in multiple
// attributes_metadata_maps along with a current node ID and json
// string.  Attribute information for the current node existing
// within the metadata maps is extracted and added to the json string.

   void add_node_attributes_to_json_string(
      int curr_node_ID,string& json_string,
      ATTRIBUTES_METADATA_MAP* image_annotations_map_ptr,
      ATTRIBUTES_METADATA_MAP* attributes_metadata_map_ptr,
      ATTRIBUTES_METADATA_MAP* color_histograms_map_ptr,
      ATTRIBUTES_METADATA_MAP* human_faces_map_ptr,
      ATTRIBUTES_METADATA_MAP* video_keyframes_map_ptr)
      {
//         cout << "inside imagesdatabasefunc::add_node_attributes_to_json_string()" << endl;
         
         ATTRIBUTES_METADATA_MAP::iterator iter=
            image_annotations_map_ptr->find(curr_node_ID);
         vector<STRING_PAIR> image_annotations_key_value_pairs;
         if (iter != image_annotations_map_ptr->end()) 
         {
            image_annotations_key_value_pairs=iter->second;
         }

         iter=attributes_metadata_map_ptr->find(curr_node_ID);
         vector<STRING_PAIR> key_value_pairs;
         if (iter != attributes_metadata_map_ptr->end()) 
         {
            key_value_pairs=iter->second;
         }

         iter=color_histograms_map_ptr->find(curr_node_ID);
         vector<STRING_PAIR> color_key_value_pairs;
         if (iter != color_histograms_map_ptr->end()) 
         {
            color_key_value_pairs=iter->second;
         }

         iter=human_faces_map_ptr->find(curr_node_ID);
         vector<STRING_PAIR> human_faces_key_value_pairs;
         if (iter != human_faces_map_ptr->end()) 
         {
            human_faces_key_value_pairs=iter->second;
         }

         iter=video_keyframes_map_ptr->find(curr_node_ID);
         vector<STRING_PAIR> video_keyframes_key_value_pairs;
         if (iter != video_keyframes_map_ptr->end()) 
         {
            video_keyframes_key_value_pairs=iter->second;
         }

         for (unsigned int a=0; a<image_annotations_key_value_pairs.size(); 
              a++)
         {
            string key=image_annotations_key_value_pairs[a].first;
            string value=image_annotations_key_value_pairs[a].second;
            json_string += "         '"+key+"': '"+value+"',\n";
         }

         for (unsigned int a=0; a<key_value_pairs.size(); a++)
         {
            string key=key_value_pairs[a].first;
            string value=key_value_pairs[a].second;
            json_string += "         '"+key+"': '"+value+"',\n";
         }

         for (unsigned int c=0; c<color_key_value_pairs.size(); c++)
         {
            string key=color_key_value_pairs[c].first;
            string value=color_key_value_pairs[c].second;
            json_string += "         '"+key+"': '"+value+"',\n";
         }

         for (unsigned int h=0; h<human_faces_key_value_pairs.size(); h++)
         {
            string key=human_faces_key_value_pairs[h].first;
            string value=human_faces_key_value_pairs[h].second;
            json_string += "         '"+key+"': '"+value+"',\n";
         }

         for (unsigned int v=0; v<video_keyframes_key_value_pairs.size(); v++)
         {
            string key=video_keyframes_key_value_pairs[v].first;
            string value=video_keyframes_key_value_pairs[v].second;
//            string delta_json_string="         '"+key+"': '"+value+"',\n";
            json_string += "         '"+key+"': '"+value+"',\n";
         }

//         cout << "json_string = " << json_string << endl;
//         outputfunc::enter_continue_char();
      }
      
// ---------------------------------------------------------------------   
// Method retrieve_nodes_metadata() issues a single SQL select command
// for all node metadata corresponding to the graph specified by the
// input hierarchy and graph IDs.  The node metadata is returned via a
// set of STL vectors.

   bool retrieve_nodes_metadata(
      gis_database* gis_database_ptr,int hierarchy_ID,int graph_ID,
      vector<int>& node_ID,vector<double>& epoch,
      vector<string>& URL,vector<int>& npx,vector<int>& npy,
      vector<string>& thumbnail_URL,vector<int>& thumbnail_npx,
      vector<int>& thumbnail_npy,vector<int>& parent_node_ID,
      vector<double>& gx,vector<double>& gy,
      vector<double>& gx2,vector<double>& gy2,
      vector<double>& relative_size,vector<string>& color,
      vector<string>& label)
   {
//      cout << "inside imagesdatabasefunc::retrieve_nodes_metadata()" 
//           << endl;

      string SQL_cmd=
         "SELECT nodes.node_id,images.epoch,images.url,images.npx,images.npy,";
      SQL_cmd += 
         "images.thumbnail_URL,images.thumbnail_npx,images.thumbnail_npy,";
      SQL_cmd += "nodes.parent_node_id,nodes.gx,nodes.gy,nodes.gx2,nodes.gy2,";
      SQL_cmd += "nodes.relative_size,nodes.color,nodes.label FROM images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_ID="+
         stringfunc::number_to_string(hierarchy_ID);
      SQL_cmd += " AND nodes.graph_ID="+stringfunc::number_to_string(graph_ID);
      SQL_cmd += " ORDER BY nodes.node_ID";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         node_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,0)));
         epoch.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,1)));
         URL.push_back(field_array_ptr->get(m,2));
         npx.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,3)));
         npy.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,4)));
         thumbnail_URL.push_back(field_array_ptr->get(m,5));
         thumbnail_npx.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,6)));
         thumbnail_npy.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,7)));
         parent_node_ID.push_back(stringfunc::string_to_number(
            field_array_ptr->get(m,8)));
         gx.push_back(stringfunc::string_to_number(field_array_ptr->get(m,9)));
         gy.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,10)));

         double curr_gx2=NEGATIVEINFINITY;
         double curr_gy2=NEGATIVEINFINITY;
         if (field_array_ptr->get(m,11) != "NULL")
         {
            curr_gx2=stringfunc::string_to_number(field_array_ptr->get(m,11));
            curr_gy2=stringfunc::string_to_number(field_array_ptr->get(m,12));
         }
         gx2.push_back(curr_gx2);
         gy2.push_back(curr_gy2);
      
         relative_size.push_back(stringfunc::string_to_number(
            field_array_ptr->get(m,13)));
         color.push_back(field_array_ptr->get(m,14));

         string curr_label="";
         if (field_array_ptr->get(m,15) != "NULL") 
            curr_label=field_array_ptr->get(m,15);
         label.push_back(curr_label);
         
      } // loop over index m 

      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_all_attributes() issues a single SQL select command
// for all image attributes corresponding to the the input campaign
// and mission IDs.  Each node within the image graph may have an
// arbitrary number of attributes.  So this method returns attribute
// information within an ATTRIBUTES_METADATA_MAP where the independent
// variable is node ID and the dependent variable is an STL vector of
// key-value string pairs.

   ATTRIBUTES_METADATA_MAP* retrieve_all_attributes(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID)
   {
//      cout << "inside imagesdatabasefunc::retrieve_all_attributes()" 
//           << endl;

      ATTRIBUTES_METADATA_MAP* attributes_metadata_map_ptr=
         new ATTRIBUTES_METADATA_MAP;
      
      string SQL_cmd=
         "SELECT nodes.node_id,key,value FROM image_attributes ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=image_attributes.datum_id";
      SQL_cmd += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_ID="+stringfunc::number_to_string(mission_ID);
      SQL_cmd += " ORDER BY nodes.node_ID";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);
      if (field_array_ptr==NULL) return attributes_metadata_map_ptr;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int prev_node_ID=-1;
      vector<STRING_PAIR> attribute_pairs;
      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int curr_node_ID=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         if (curr_node_ID != prev_node_ID)
         {
            if (prev_node_ID >= 0)
            {
               (*attributes_metadata_map_ptr)[prev_node_ID]=attribute_pairs;
            }
            prev_node_ID=curr_node_ID;
            attribute_pairs.clear();
         }
         string key=field_array_ptr->get(m,1);
         string value=field_array_ptr->get(m,2);
//         cout << "key = " << key << " value = " << value << endl;
         STRING_PAIR curr_pair(key,value);
         attribute_pairs.push_back(curr_pair);
      } // loop over index m 

      return attributes_metadata_map_ptr;
   }

// ==========================================================================
// Image time querying methods
// ==========================================================================

// Method retrieve_campaign_UTM_zonenumber() queries the world_regions
// table within *gis_database_ptr for the UTM zonenumber corresponding
// to the input campaign_ID.

   int retrieve_campaign_UTM_zonenumber(
      gis_database* gis_database_ptr,int campaign_ID)
   {
//      cout << "inside imagesdatabasefunc::retrieve_campaign_UTM_zonenumber()" << endl;

      string SQL_cmd="SELECT utm_zonenumber from world_regions ";
      SQL_cmd += "INNER JOIN campaigns ";
      SQL_cmd += "ON world_regions.world_region_ID=campaigns.world_region_ID ";
      SQL_cmd += "WHERE campaigns.campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);

      if (field_array_ptr==NULL) return 0;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      int UTM_zonenumber=stringfunc::string_to_number(
         field_array_ptr->get(0,0));
      return UTM_zonenumber;
   }

// ---------------------------------------------------------------------   
// Method retrieve_median_image_time() queries the images table within
// *gis_database_ptr for UTC times measured in epoch seconds
// corresponding to the input campaign and mission IDs.  It returns
// their median value.
 
   double retrieve_median_image_time(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID)
   {
//      cout << "inside imagesdatabasefunc::retrieve_median_image_time()" << endl;

      string select_command="select epoch from images ";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      select_command += " ORDER by epoch";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      if (field_array_ptr==NULL) return 0;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      vector<double> epoch_times;
      for (unsigned int m=0; m<mdim; m++)
      {
         epoch_times.push_back(stringfunc::string_to_number(
            field_array_ptr->get(m,0)));
      }
      return mathfunc::median_value(epoch_times);
   }

// ---------------------------------------------------------------------   
// Method retrieve_image_time() queries the images table within
// *gis_database_ptr for the UTC time measured in epoch seconds
// corresponding to the input campaign, mission and image IDs.  
 
   double retrieve_image_time(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_time()" << endl;

      string select_command="select epoch from images ";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      select_command += " AND image_ID="+
         stringfunc::number_to_string(image_ID);
      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      if (field_array_ptr==NULL) return 0;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      double image_epoch=stringfunc::string_to_number(
         field_array_ptr->get(0,0));
      return image_epoch;
   }

// ---------------------------------------------------------------------   
// Method retrieve_extremal_image_times() queries the images table
// within *gis_database_ptr for UTC times measured in epoch seconds
// corresponding to the input campaign and mission IDs.  If none
// are found this method returns starting_epoch=stopping_epoch=-1.
// Otherwise, it returns the extremal times stored within the images
// table of the IMAGERY database.
 
   void retrieve_extremal_image_times(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double& starting_epoch,double& stopping_epoch)
   {
//      cout << "inside imagesdatabasefunc::retrieve_extremal_times()"
//           << endl;

      starting_epoch=stopping_epoch=-1;

      string select_command="select epoch from images ";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      select_command += " ORDER by epoch";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      if (field_array_ptr==NULL) return;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      starting_epoch=stringfunc::string_to_number(
         field_array_ptr->get(0,0));
      stopping_epoch=stringfunc::string_to_number(
         field_array_ptr->get(mdim-1,0));

//      cout.precision(12);
//      cout << "starting_epoch = " << starting_epoch
//           << " stopping_epoch = " << stopping_epoch << endl;
   }

// ---------------------------------------------------------------------   
// Method retrieve_image_times() queries the images table within
// *gis_database_ptr for UTC times measured in epoch seconds
// corresponding to the input campaign and mission IDs.  It fills
// output STL vector epoch_IDs with correlated epoch times and image
// IDs.
 
   void retrieve_image_times(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      vector<twovector>& epoch_IDs)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_times()" << endl;

      epoch_IDs.clear();
      
      string select_command="select epoch,image_ID from images ";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      select_command += " ORDER by epoch";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      if (field_array_ptr==NULL) return;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      for (unsigned int m=0; m<mdim; m++)
      {
         double epoch_time=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         int image_ID=stringfunc::string_to_number(
            field_array_ptr->get(m,1));
         twovector curr_epoch_ID(epoch_time,image_ID);
         epoch_IDs.push_back(curr_epoch_ID);
      }
   }

// ---------------------------------------------------------------------   
// Method retrieve_images_within_time_interval() queries the images
// table within *gis_database_ptr for entires corresponding to the
// input campaign and mission IDs and lying within the specified time
// interval.  It fills output STL vectors with correlated image IDs
// epoch times and thumbnail_URLs.
 
   bool retrieve_images_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      vector<int>& datum_IDs,vector<int>& image_IDs,
      vector<double>& epoch_times,vector<string>& thumbnail_URLs)
   {
//      cout << "inside imagesdatabasefunc::retrieve_images_within_time_interval()" << endl;

      string select_command=
         "select nodes.data_id,image_ID,epoch,thumbnail_URL from images ";
      select_command += "INNER JOIN nodes ON nodes.data_id=images.id ";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      select_command += " AND epoch > "+
         stringfunc::number_to_string(start_epoch_time);
      select_command += " AND epoch < "+
         stringfunc::number_to_string(stop_epoch_time);
      select_command += " AND nodes.graph_id=0 ";
      select_command += " ORDER by epoch";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      for (unsigned int m=0; m<mdim; m++)
      {
         int datum_ID=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         int image_ID=stringfunc::string_to_number(
            field_array_ptr->get(m,1));
         double epoch_time=stringfunc::string_to_number(
            field_array_ptr->get(m,2));

// Add random fraction of a second to each epoch time to minimize
// likelihood of two images having precisely the same timestamp:

//         epoch_time += nrfunc::ran1()*1.0;
         
         string thumbnail_URL=field_array_ptr->get(m,3);
         datum_IDs.push_back(datum_ID);
         image_IDs.push_back(image_ID);
         epoch_times.push_back(epoch_time);
         thumbnail_URLs.push_back(thumbnail_URL);

//         cout << "m = " << m
//              << " URL = " << thumbnail_URL
//              << " datum ID = " << datum_ID
//              << " image_ID = " << image_ID << endl;

//         Clock clock;
//         clock.convert_elapsed_secs_to_date(epoch_time);
//         cout << "m = " << m 
//              << " t = " << clock.YYYY_MM_DD_H_M_S()
//              << " image_ID = " << image_ID 
//              << endl;
      }

      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_image_minutes_within_time_interval() queries the images
// table within *gis_database_ptr for distinct, quantized minutes
// corresponding to the input campaign and mission IDs and lying
// within the specified time interval.  It fills and returns output
// STL vector distinct_epoch_minutes.
 
   bool retrieve_image_minutes_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      vector<double>& distinct_epoch_minutes)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_minutes_within_time_interval()" << endl;

      string select_command=
         "select distinct 60*floor(epoch/60.0) as epoch_minute from images ";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      select_command += " AND epoch > "+
         stringfunc::number_to_string(start_epoch_time);
      select_command += " AND epoch < "+
         stringfunc::number_to_string(stop_epoch_time);
      select_command += " ORDER by epoch_minute";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      for (unsigned int m=0; m<mdim; m++)
      {
         double epoch_minute=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         distinct_epoch_minutes.push_back(epoch_minute);
//         cout << "m = " << m << " distinct_epoch_minute = "
//              << distinct_epoch_minutes.back() << endl;
      }

      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_image_hours_within_time_interval() queries the images
// table within *gis_database_ptr for distinct, quantized hours
// corresponding to the input campaign and mission IDs and lying
// within the specified time interval.  It fills and returns output
// STL vector distinct_epoch_hours.
 
   bool retrieve_image_hours_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      vector<double>& distinct_epoch_hours)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_hours_within_time_interval()" << endl;

      string select_command=
         "select distinct 3600*floor(epoch/3600.0) as epoch_hour from images ";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      select_command += " AND epoch > "+
         stringfunc::number_to_string(start_epoch_time);
      select_command += " AND epoch < "+
         stringfunc::number_to_string(stop_epoch_time);
      select_command += " ORDER by epoch_hour";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      for (unsigned int m=0; m<mdim; m++)
      {
         double epoch_hour=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         distinct_epoch_hours.push_back(epoch_hour);
//         cout << "m = " << m << " distinct_epoch_hour = "
//              << distinct_epoch_hours.back() << endl;
      }

      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_image_days_within_time_interval() queries the images
// table within *gis_database_ptr for distinct, quantized days
// corresponding to the input campaign and mission IDs and lying
// within the specified time interval.  It fills and returns output
// STL vector distinct_epoch_days.
 
   bool retrieve_image_days_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      vector<double>& distinct_epoch_days)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_days_within_time_interval()" << endl;

      string select_command=
         "select distinct 24*3600*floor(epoch/(24*3600.0)) as epoch_day from images ";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      select_command += " AND epoch > "+
         stringfunc::number_to_string(start_epoch_time);
      select_command += " AND epoch < "+
         stringfunc::number_to_string(stop_epoch_time);
      select_command += " ORDER by epoch_day";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      for (unsigned int m=0; m<mdim; m++)
      {
         double epoch_day=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         distinct_epoch_days.push_back(epoch_day);
//         cout << "m = " << m << " distinct_epoch_day = "
//              << distinct_epoch_days.back() << endl;
      }

      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_image_months_within_time_interval() queries the images
// table within *gis_database_ptr for distinct, quantized months
// corresponding to the input campaign and mission IDs and lying
// within the specified time interval.  It fills and returns output
// STL vector distinct_epoch_months.
 
   bool retrieve_image_months_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      vector<double>& distinct_epoch_months)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_months_within_time_interval()" << endl;

      string select_command=
         "select distinct 30*24*3600*floor(epoch/(30*24*3600.0)) as epoch_month from images ";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      select_command += " AND epoch > "+
         stringfunc::number_to_string(start_epoch_time);
      select_command += " AND epoch < "+
         stringfunc::number_to_string(stop_epoch_time);
      select_command += " ORDER by epoch_month";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      for (unsigned int m=0; m<mdim; m++)
      {
         double epoch_month=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         distinct_epoch_months.push_back(epoch_month);
//         cout << "m = " << m << " distinct_epoch_month = "
//              << distinct_epoch_months.back() << endl;
      }

      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_image_years_within_time_interval() queries the images
// table within *gis_database_ptr for distinct, quantized years
// corresponding to the input campaign and mission IDs and lying
// within the specified time interval.  It fills and returns output
// STL vector distinct_epoch_years.
 
   bool retrieve_image_years_within_time_interval(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double start_epoch_time,double stop_epoch_time,
      vector<double>& distinct_epoch_years)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_years_within_time_interval()" << endl;

      string select_command=
         "select distinct 365*24*3600*floor(epoch/(365*24*3600.0)) as epoch_year from images ";
      select_command += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      select_command += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      select_command += " AND epoch > "+
         stringfunc::number_to_string(start_epoch_time);
      select_command += " AND epoch < "+
         stringfunc::number_to_string(stop_epoch_time);
      select_command += " ORDER by epoch_year";
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      unsigned int mdim=field_array_ptr->get_mdim();
      for (unsigned int m=0; m<mdim; m++)
      {
         double epoch_year=stringfunc::string_to_number(
            field_array_ptr->get(m,0));
         distinct_epoch_years.push_back(epoch_year);
//         cout << "m = " << m << " distinct_epoch_year = "
//              << distinct_epoch_years.back() << endl;
      }

      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_closest_time_image() queries the images
// table within *gis_database_ptr for the row entry whose epoch time
// lies closest in absolute value to input_epoch_time.  It returns
// that image's datum_ID, image_ID, epoch and URL.
 
   bool retrieve_closest_time_image(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      double input_epoch_time,int& datum_ID,int& image_ID,
      double& epoch,string& thumbnail_URL)
   {
//      cout << "inside imagesdatabasefunc::retrieve_closest_time_image()" << endl;

      string SQL_cmd="select nodes.data_id,image_id,epoch,thumbnail_URL,";
      SQL_cmd += "abs(epoch-"+stringfunc::number_to_string(input_epoch_time)+
         ") as hour_difference FROM images ";
      SQL_cmd += "INNER JOIN nodes ON nodes.data_id=images.id ";
      SQL_cmd += "WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
      SQL_cmd += " AND nodes.graph_id=0 ";
      SQL_cmd += " ORDER by hour_difference ";
      SQL_cmd += " ASC LIMIT 1 ";
//      cout << "SQL_command = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);

      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      datum_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
//      cout << "datum_ID = " << datum_ID << endl;
      image_ID=stringfunc::string_to_number(field_array_ptr->get(0,1));
//      cout << "image_ID = " << image_ID << endl;
      epoch=stringfunc::string_to_number(field_array_ptr->get(0,2));
      thumbnail_URL=field_array_ptr->get(0,3);
//      cout << "thumbnail_URL = " << thumbnail_URL << endl;

      return true;
   }

// ==========================================================================
// Image attributes methods
// ==========================================================================

// Method insert_image_attribute() takes in metadata for a new entry
// within the image attributes table of the IMAGERY database. 

   bool insert_image_attribute(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      string key,string value)
   {
//      cout << "inside imagesdatabasefunc::insert_image_attribute()" << endl;

      string SQL_cmd=
         generate_insert_image_attribute_SQL_command(
            campaign_ID,mission_ID,image_ID,datum_ID,key,value);
//      cout << SQL_cmd << endl;
    
      vector<string> insert_commands;
      insert_commands.push_back(SQL_cmd);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------   
// Method generate_insert_image_attribute_SQL_command() takes in
// metadata associated with a single image attribute.  It generates
// and returns a string containing a SQL insert command needed to
// populate a row within the image attributes table of the IMAGERY
// postgis database.

   string generate_insert_image_attribute_SQL_command(
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      string key,string value)
   {
//   cout << "inside imagesdatabasefunc::generate_insert_image_metadata_SQL_command()" << endl;

      string SQL_command="insert into image_attributes ";
      SQL_command += 
         "(campaign_ID,mission_ID,image_ID,datum_ID,key,value) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(campaign_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(image_ID)+",";
      SQL_command += stringfunc::number_to_string(datum_ID)+",";
      SQL_command += "'"+key+"',";
      SQL_command += "'"+value+"'";
      SQL_command += ");";

//      cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
   bool update_image_attribute(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      string key,string value)
   {
//      cout << "inside imagesdatabasefunc::update_image_attribute()" << endl;

      string SQL_cmd="UPDATE image_attributes";
      SQL_cmd += " SET value='"+value+"'";
      SQL_cmd += " WHERE key='"+key+"'";
      SQL_cmd += " AND campaign_id="+stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_id="+stringfunc::number_to_string(mission_ID);
      SQL_cmd += " AND image_id="+stringfunc::number_to_string(image_ID);
      SQL_cmd += " AND datum_id="+stringfunc::number_to_string(datum_ID);
//      cout << SQL_cmd << endl;
    
      vector<string> update_commands;
      update_commands.push_back(SQL_cmd);
      gis_database_ptr->set_SQL_commands(update_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------   
   bool retrieve_image_attributes_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      string key,vector<int>& image_IDs,vector<int>& datum_IDs,
      vector<string>& values)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_attributes_database() #1" 
//           << endl;

      string curr_select_cmd=generate_retrieve_image_attributes_SQL_command(
         campaign_ID,mission_ID,key);
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

         int curr_image_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,0));
         int curr_datum_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,1));
         string curr_value=field_array_ptr->get(m,2);
         image_IDs.push_back(curr_image_ID);
         datum_IDs.push_back(curr_datum_ID);
         values.push_back(curr_value);
      } // loop over index m labeling rows in *field_array_ptr

      return true;
   }

// ---------------------------------------------------------------------   
// Method generate_retrieve_image_attributes_SQL_command() 

   string generate_retrieve_image_attributes_SQL_command(
      int campaign_ID,int mission_ID,string key)
   {
//      cout << "inside imagesdatabasefunc::generate_retrieve_image_attributes_SQL_command()" 
//           << endl;

      string SQL_command="SELECT image_ID,datum_ID,value,value_type from image_attributes ";
      SQL_command += "WHERE campaign_ID="
         +stringfunc::number_to_string(campaign_ID);
      SQL_command += " AND mission_ID="
         +stringfunc::number_to_string(mission_ID);
      SQL_command += " AND key="+key;
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method retrieve_particular_image_attributes_from_database()

   bool retrieve_particular_image_attributes_from_database(
      gis_database* gis_database_ptr,int datum_ID,
      vector<string>& keys,vector<string>& values)
   {
//      cout << "inside imagesdatabasefunc::retrieve_particular_image_attributes_from_database()" << endl;
//      cout << "datum_ID = " << datum_ID << endl;

      string curr_select_cmd="select key,value from image_attributes ";
      curr_select_cmd += "WHERE datum_ID="+
         stringfunc::number_to_string(datum_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         keys.push_back(field_array_ptr->get(m,0));
         values.push_back(field_array_ptr->get(m,1));
      }

      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_image_attribute_keys_values_from_database()

   bool retrieve_image_attribute_keys_values_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      vector<string>& keys,vector<vector<string> >& key_values)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_attribute_keys_values_from_database()" << endl;
      
      string SQL_cmd=
         "SELECT DISTINCT key FROM image_attributes";
      SQL_cmd += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID);
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         keys.push_back(field_array_ptr->get(m,0));
//         cout << "m = " << m << " key = " << keys.back() << endl;
      }

      for (unsigned int k=0; k<keys.size(); k++)
      {
         SQL_cmd="SELECT value,value_type,color FROM image_attribute_metadata";
         SQL_cmd += " WHERE key='"+keys[k]+"'";
         SQL_cmd += " ORDER by value";
         field_array_ptr=gis_database_ptr->select_data(SQL_cmd);

         string value_type;
         vector<string> curr_key_values;
         for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
         {
            curr_key_values.push_back(field_array_ptr->get(m,0));
            value_type=field_array_ptr->get(m,1);
         }
         cout << "value_type = " << value_type << endl;

// Sort numerical curr_key_values from smallest to largest:

         if (value_type=="INT" || value_type=="DOUBLE")
         {
            vector<double> values;
            for (unsigned int v=0; v<curr_key_values.size(); v++)
            {
               values.push_back(stringfunc::string_to_number(
                  curr_key_values[v]));
            }
            std::sort(values.begin(),values.end());
            curr_key_values.clear();
            for (unsigned int v=0; v<values.size(); v++)
            {
               if (value_type=="DOUBLE") 
               {
                  curr_key_values.push_back(stringfunc::number_to_string(
                     values[v],2));
               }
               else if (value_type=="INT")
               {
                  curr_key_values.push_back(stringfunc::number_to_string(
                     values[v]));
               }
            } // loop over index v labeling values
         }

         key_values.push_back(curr_key_values);
      } // loop over index k labeling attribute keys
      
      return true;
   }

// ==========================================================================
// Image attributes metadata methods
// ==========================================================================

   bool image_attribute_key_exists(
      gis_database* gis_database_ptr,string attribute_key)
   {
//      cout << "inside imagesdatabasefunc::image_attribute_key_exists()" << endl;
      
      string SQL_cmd="SELECT id FROM image_attribute_metadata ";
      SQL_cmd += "WHERE key='"+attribute_key+"'";

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;
      return true;
   }

// ---------------------------------------------------------------------   
   bool image_attribute_value_exists(
      gis_database* gis_database_ptr,string attribute_value)
   {
//      cout << "inside imagesdatabasefunc::image_attribute_value_exists()" << endl;
      
      string SQL_cmd="SELECT id FROM image_attribute_metadata ";
      SQL_cmd += "WHERE value='"+attribute_value+"'";

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;
      return true;
   }

// ==========================================================================
// Sensor metadata methods
// ==========================================================================

// Method insert_sensor_metadata() takes in metadata for a new entry
// within the sensor_metadata table of the IMAGERY database.  It
// inserts this metadata into *gis_database_ptr.

   bool insert_sensor_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int sensor_ID,
      int metadata_ID,int image_ID,int datum_ID,int status,
      double X,double Y,double Z,double az,double el,double roll,
      double FOV_U,double FOV_V,double f,double U0,double V0)
   {
      string SQL_command="insert into sensor_metadata ";
      SQL_command += 
         "(campaign_ID,mission_ID,sensor_ID,metadata_ID,image_ID,data_ID,status,x_posn,y_posn,z_posn,az,el,roll,horiz_fov,vert_fov,focal_param,u0,v0) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(campaign_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(sensor_ID)+",";
      SQL_command += stringfunc::number_to_string(metadata_ID)+",";
      SQL_command += stringfunc::number_to_string(image_ID)+",";
      SQL_command += stringfunc::number_to_string(datum_ID)+",";
      SQL_command += stringfunc::number_to_string(status)+",";
      SQL_command += stringfunc::number_to_string(X)+",";
      SQL_command += stringfunc::number_to_string(Y)+",";
      SQL_command += stringfunc::number_to_string(Z)+",";
      SQL_command += stringfunc::number_to_string(az)+",";
      SQL_command += stringfunc::number_to_string(el)+",";
      SQL_command += stringfunc::number_to_string(roll)+",";
      SQL_command += stringfunc::number_to_string(FOV_U)+",";
      SQL_command += stringfunc::number_to_string(FOV_V)+",";
      SQL_command += stringfunc::number_to_string(f)+",";
      SQL_command += stringfunc::number_to_string(U0)+",";
      SQL_command += stringfunc::number_to_string(V0);
      SQL_command += ");";
//      cout << "SQL_command = " << SQL_command << endl;

      vector<string> insert_commands;
      insert_commands.push_back(SQL_command);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------   
// Method retrieve_particular_sensor_posn_from_database()

   bool retrieve_particular_sensor_posn_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      string image_URL,threevector& XYZ_posn)
   {
//      cout << "inside imagesdatabasefunc::retrieve_particular_sensor_posn_from_database()" << endl;
      
      string SQL_cmd="SELECT x_posn,y_posn,z_posn from images ";
      SQL_cmd += 
         "INNER JOIN sensor_metadata on images.id=sensor_metadata.data_id ";
      SQL_cmd += 
         "WHERE sensor_metadata.campaign_ID="+stringfunc::number_to_string(
            campaign_ID);
      SQL_cmd += 
         " AND sensor_metadata.mission_ID="+stringfunc::number_to_string(
            mission_ID);
      SQL_cmd += " AND URL='"+image_URL+"';";
      
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
      double X=stringfunc::string_to_number(field_array_ptr->get(0,0));
      double Y=stringfunc::string_to_number(field_array_ptr->get(0,1));
      double Z=stringfunc::string_to_number(field_array_ptr->get(0,2));
      XYZ_posn=threevector(X,Y,Z);

      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_all_sensor_posns_from_database()

   bool retrieve_all_sensor_posns_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      vector<threevector>& XYZ_posns)
   {
      vector<int> datum_IDs;
      return retrieve_all_sensor_posns_from_database(
         gis_database_ptr,campaign_ID,mission_ID,datum_IDs,XYZ_posns);
   }

   bool retrieve_all_sensor_posns_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      vector<int>& datum_IDs,vector<threevector>& XYZ_posns)
   {
//      cout << "inside imagesdatabasefunc::retrieve_all_sensor_posns_from_database()" << endl;
      
      string SQL_cmd="SELECT data_ID,x_posn,y_posn,z_posn from sensor_metadata ";
      SQL_cmd += 
         "WHERE sensor_metadata.campaign_ID="+stringfunc::number_to_string(
            campaign_ID);
      SQL_cmd += 
         " AND sensor_metadata.mission_ID="+stringfunc::number_to_string(
            mission_ID);
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         datum_IDs.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,0)));
         double X=stringfunc::string_to_number(field_array_ptr->get(m,1));
         double Y=stringfunc::string_to_number(field_array_ptr->get(m,2));
         double Z=stringfunc::string_to_number(field_array_ptr->get(m,3));
         XYZ_posns.push_back(threevector(X,Y,Z));
      }
      
      return true;
   }
      
// ---------------------------------------------------------------------   
// Method retrieve_particular_sensor_metadata_from_database()

   bool retrieve_particular_sensor_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,threevector& posn,threevector& az_el_roll,
      threevector& f_u0_v0)
   {
//      cout << "inside imagesdatabasefunc::retrieve_particular_sensor_metadata_from_database()" << endl;
      
      string SQL_cmd="SELECT x_posn,y_posn,z_posn,az,el,roll,focal_param,u0,v0 from sensor_metadata ";
      SQL_cmd += 
         "WHERE campaign_ID="+stringfunc::number_to_string(campaign_ID);
      SQL_cmd += 
         " AND mission_ID="+stringfunc::number_to_string(mission_ID);
      SQL_cmd += 
         " AND image_ID="+stringfunc::number_to_string(image_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
      double X=stringfunc::string_to_number(field_array_ptr->get(0,0));
      double Y=stringfunc::string_to_number(field_array_ptr->get(0,1));
      double Z=stringfunc::string_to_number(field_array_ptr->get(0,2));
      posn=threevector(X,Y,Z);

      double az=stringfunc::string_to_number(field_array_ptr->get(0,3));
      double el=stringfunc::string_to_number(field_array_ptr->get(0,4));
      double roll=stringfunc::string_to_number(field_array_ptr->get(0,5));
      az_el_roll=threevector(az,el,roll);

      double f=stringfunc::string_to_number(field_array_ptr->get(0,6));
      double u0=stringfunc::string_to_number(field_array_ptr->get(0,7));
      double v0=stringfunc::string_to_number(field_array_ptr->get(0,8));
      f_u0_v0=threevector(f,u0,v0);

      return true;
   }
      
// ---------------------------------------------------------------------   
// Method retrieve_particular_sensor_hfov_from_database()

   bool retrieve_particular_sensor_hfov_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,double& horiz_fov)
   {
//      cout << "inside imagesdatabasefunc::retrieve_particular_sensor_hfov_from_database()" << endl;
      
      string SQL_cmd="SELECT horiz_fov from sensor_metadata ";
      SQL_cmd += 
         "WHERE campaign_ID="+stringfunc::number_to_string(campaign_ID);
      SQL_cmd += 
         " AND mission_ID="+stringfunc::number_to_string(mission_ID);
      SQL_cmd += 
         " AND image_ID="+stringfunc::number_to_string(image_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
      horiz_fov=stringfunc::string_to_number(field_array_ptr->get(0,0));
      return true;
   }

// ---------------------------------------------------------------------   
// Method threeD_sensor_metadata_in_database() takes in campaign
// and mission IDs.  It returns true if any of the corresponding
// entries within the sensor_metadata table have non-null azimuth
// values.

   bool threeD_sensor_metadata_in_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID)
   {
      string SQL_cmd="SELECT count(*) from sensor_metadata ";
      SQL_cmd += "WHERE az IS NOT NULL ";
      SQL_cmd += 
         "AND campaign_ID="+stringfunc::number_to_string(campaign_ID);
      SQL_cmd += 
         " AND mission_ID="+stringfunc::number_to_string(mission_ID);
      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);
      int n_nonnull_az_entries=
         stringfunc::string_to_number(field_array_ptr->get(0,0));
      return (n_nonnull_az_entries > 0);
   }

// ---------------------------------------------------------------------   
// Method retrieve_sensor_metadata_from_database() takes in campaign
// and mission IDs.  It retrieves all camera parameters for all images
// corresponding to the input parameters.  

   bool retrieve_sensor_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      vector<int>& image_ID,vector<string>& URL,
      vector<double>& FOV_u,vector<double>& FOV_v,
      vector<double>& U0,vector<double>& V0,
      vector<double>& az,vector<double>& el,
      vector<double>& roll,vector<double>& camera_lon,
      vector<double>& camera_lat,vector<double>& camera_alt,
      vector<double>& frustum_sidelength)
   {
//      cout << "inside imagesdatabasefunc::retrieve_sensor_metadata_from_database()" << endl;

      string SQL_cmd="SELECT sensor_metadata.image_id,images.URL,horiz_fov,vert_fov,U0,V0,az,el,roll,x_posn,y_posn,z_posn ";
      SQL_cmd += "FROM sensor_metadata INNER JOIN images ";
      
      SQL_cmd += "ON sensor_metadata.image_id=images.image_id ";
      
      SQL_cmd += "AND sensor_metadata.campaign_id=images.campaign_ID ";
      SQL_cmd += "AND sensor_metadata.mission_id=images.mission_id ";
      SQL_cmd += "WHERE sensor_metadata.campaign_id= "+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += "AND sensor_metadata.mission_id= "+
         stringfunc::number_to_string(mission_ID);
      SQL_cmd += "ORDER BY images.image_id";

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

// FAKE FAKE:  Tues Aug 13, 2013 at 4 pm
// Hardwire UTM zone, northern hemisphere flag, curr_frustum_sidelength

      bool northern_hemisphere_flag=true;
      int UTM_zonenumber=19;
      double curr_frustum_sidelength=5;	 // meters

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         image_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,0)));
         URL.push_back(field_array_ptr->get(m,1));
         FOV_u.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,2)));
         FOV_v.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,3)));
         U0.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,4)));
         V0.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,5)));
         az.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,6)));
         el.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,7)));
         roll.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,8)));
         
         double easting=
            stringfunc::string_to_number(field_array_ptr->get(m,9));
         double northing=
            stringfunc::string_to_number(field_array_ptr->get(m,10));
         geopoint camera_geoposn(
            northern_hemisphere_flag,UTM_zonenumber,easting,northing);
         camera_lon.push_back(camera_geoposn.get_longitude());
         camera_lat.push_back(camera_geoposn.get_latitude());
         camera_alt.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,11)));
         frustum_sidelength.push_back(curr_frustum_sidelength);
      }
      
      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_sensor_metadata_from_database() takes in campaign
// and mission IDs.  It retrieves limited "2D" camera parameters for
// all images corresponding to the input parameters.  

   bool retrieve_sensor_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      vector<int>& image_ID,vector<string>& URL,
      vector<double>& U0,vector<double>& V0,
      vector<double>& camera_lon,vector<double>& camera_lat)
   {
//      cout << "inside imagesdatabasefunc::retrieve_sensor_metadata_from_database()" << endl;

      string SQL_cmd="SELECT sensor_metadata.image_id,images.URL,U0,V0,x_posn,y_posn ";
      SQL_cmd += "FROM sensor_metadata INNER JOIN images ";
      
      SQL_cmd += "ON sensor_metadata.image_id=images.image_id ";
      
      SQL_cmd += "AND sensor_metadata.campaign_id=images.campaign_ID ";
      SQL_cmd += "AND sensor_metadata.mission_id=images.mission_id ";
      SQL_cmd += "WHERE sensor_metadata.campaign_id= "+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += "AND sensor_metadata.mission_id= "+
         stringfunc::number_to_string(mission_ID);
      SQL_cmd += "ORDER BY images.image_id";

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

// FAKE FAKE:  Tues Aug 13, 2013 at 4 pm
// Hardwire UTM zone, northern hemisphere flag, curr_frustum_sidelength

      bool northern_hemisphere_flag=true;
      int UTM_zonenumber=19;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         image_ID.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,0)));
         URL.push_back(field_array_ptr->get(m,1));
         U0.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,2)));
         V0.push_back(
            stringfunc::string_to_number(field_array_ptr->get(m,3)));
         double easting=
            stringfunc::string_to_number(field_array_ptr->get(m,4));
         double northing=
            stringfunc::string_to_number(field_array_ptr->get(m,5));
         geopoint camera_geoposn(
            northern_hemisphere_flag,UTM_zonenumber,easting,northing);
         camera_lon.push_back(camera_geoposn.get_longitude());
         camera_lat.push_back(camera_geoposn.get_latitude());
      }
      
      return true;
   }

// ==========================================================================
// Platform metadata methods
// ==========================================================================

// Method insert_platform_metadata() takes in metadata for a new entry
// within the platform_metadata table of the IMAGERY database.  It
// inserts this metadata into *gis_database_ptr.

   bool insert_platform_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int platform_ID,
      int metadata_ID,int image_ID,
      string image_prefix,string UTC,double epoch,
      double lon,double lat,double alt,
      double yaw,double pitch,double roll)
   {
      string SQL_command="insert into platform_metadata ";
      SQL_command += 
         "(campaign_ID,mission_ID,platform_ID,metadata_ID,image_ID,image_prefix,UTC,epoch,longitude,latitude,altitude,yaw,pitch,roll) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(campaign_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(platform_ID)+",";
      SQL_command += stringfunc::number_to_string(metadata_ID)+",";
      SQL_command += stringfunc::number_to_string(image_ID)+",";
      SQL_command += "'"+image_prefix+"',";
      SQL_command += "'"+UTC+"',";
      SQL_command += stringfunc::number_to_string(epoch)+",";
      SQL_command += stringfunc::number_to_string(lon)+",";
      SQL_command += stringfunc::number_to_string(lat)+",";
      SQL_command += stringfunc::number_to_string(alt)+",";
      SQL_command += stringfunc::number_to_string(yaw)+",";
      SQL_command += stringfunc::number_to_string(pitch)+",";
      SQL_command += stringfunc::number_to_string(roll);
      SQL_command += ");";
//      cout << "SQL_command = " << SQL_command << endl;

      vector<string> insert_commands;
      insert_commands.push_back(SQL_command);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }
     
// ---------------------------------------------------------------------   
// Method retrieve_particular_platform_metadata_from_database()

   bool retrieve_particular_platform_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,threevector& platform_lla,threevector& platform_rpy)
   {
//      cout << "inside imagesdatabasefunc::retrieve_particular_platform_metadata_from_database()" << endl;
      
      string SQL_cmd="SELECT longitude,latitude,altitude,yaw,pitch,roll from platform_metadata ";
      SQL_cmd += 
         "WHERE campaign_ID="+stringfunc::number_to_string(campaign_ID);
      SQL_cmd += 
         " AND mission_ID="+stringfunc::number_to_string(mission_ID);
      SQL_cmd += 
         " AND image_ID="+stringfunc::number_to_string(image_ID);

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
      double lon=stringfunc::string_to_number(field_array_ptr->get(0,0));
      double lat=stringfunc::string_to_number(field_array_ptr->get(0,1));
      double alt=stringfunc::string_to_number(field_array_ptr->get(0,2));
      platform_lla=threevector(lon,lat,alt);

      double yaw=stringfunc::string_to_number(field_array_ptr->get(0,3));
      double pitch=stringfunc::string_to_number(field_array_ptr->get(0,4));
      double roll=stringfunc::string_to_number(field_array_ptr->get(0,5));
      platform_rpy=threevector(roll,pitch,yaw);

      return true;
   }

// ==========================================================================
// World region metadata methods
// ==========================================================================

// Method retrieve_world_region_metadata_from_database()

   bool retrieve_world_region_metadata_from_database(
      gis_database* gis_database_ptr,int campaign_ID,
      int& UTM_zonenumber,string& northern_hemisphere_flag)
   {
//      cout << "inside imagesdatabasefunc::retrieve_world_region_metadata_from_database()" 
//           << endl;

      string SQL_cmd="SELECT campaigns.world_region_id,world_regions.UTM_zonenumber,";
      SQL_cmd += "world_regions.northern_hemisphere_flag FROM campaigns ";
      SQL_cmd += "INNER JOIN world_regions ";
      SQL_cmd += "ON world_regions.world_region_id=campaigns.world_region_id ";
      SQL_cmd += "WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
//      cout << "SQL_cmd = " << SQL_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      UTM_zonenumber=stringfunc::string_to_number(field_array_ptr->get(0,1));
      int flag=stringfunc::string_to_number(field_array_ptr->get(0,2));
      if (flag==1)
      {
         northern_hemisphere_flag="TRUE";
      }
      else
      {
         northern_hemisphere_flag="FALSE";
      }
      return true;
   }

// ==========================================================================
// Color histogram metadata methods
// ==========================================================================

// Method insert_color_histogram() takes in metadata for a
// new entry within the image_color_histograms table of the IMAGERY
// database. It inserts this metadata into *gis_database_ptr.

   bool insert_image_color_histogram(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      double red_frac,double orange_frac,double yellow_frac,
      double green_frac,double blue_frac,double purple_frac,
      double black_frac,double white_frac,double grey_frac,double brown_frac,
      string primary_color,string secondary_color,string tertiary_color)
   {
      string SQL_command="insert into image_color_histograms ";
      SQL_command += 
         "(campaign_ID,mission_ID,image_ID,datum_ID,red_frac,orange_frac,yellow_frac,green_frac,blue_frac,purple_frac,black_frac,white_frac,grey_frac,brown_frac,primary_color,secondary_color,tertiary_color) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(campaign_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(image_ID)+",";
      SQL_command += stringfunc::number_to_string(datum_ID)+",";
      SQL_command += stringfunc::number_to_string(red_frac)+",";
      SQL_command += stringfunc::number_to_string(orange_frac)+",";
      SQL_command += stringfunc::number_to_string(yellow_frac)+",";
      SQL_command += stringfunc::number_to_string(green_frac)+",";
      SQL_command += stringfunc::number_to_string(blue_frac)+",";
      SQL_command += stringfunc::number_to_string(purple_frac)+",";
      SQL_command += stringfunc::number_to_string(black_frac)+",";
      SQL_command += stringfunc::number_to_string(white_frac)+",";
      SQL_command += stringfunc::number_to_string(grey_frac)+",";
      SQL_command += stringfunc::number_to_string(brown_frac)+",";
      SQL_command += "'"+primary_color+"',";
      SQL_command += "'"+secondary_color+"',";
      SQL_command += "'"+tertiary_color+"'";
      SQL_command += ");";
//      cout << "SQL_command = " << SQL_command << endl;

      vector<string> insert_commands;
      insert_commands.push_back(SQL_command);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------   
// Method retrieve_all_image_color_histograms() issues a single SQL
// select command for all image attributes corresponding to the 
// input campaign and mission IDs.  

   ATTRIBUTES_METADATA_MAP* retrieve_all_image_color_histograms(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID)
   {
//      cout << "inside imagesdatabasefunc::retrieve_all_image_color_histograms()" 
//           << endl;

      ATTRIBUTES_METADATA_MAP* attributes_metadata_map_ptr=
         new ATTRIBUTES_METADATA_MAP;

      string SQL_cmd=
         "SELECT nodes.node_id,primary_color,secondary_color,tertiary_color ";
      SQL_cmd += "FROM image_color_histograms ";
      SQL_cmd += 
         "INNER JOIN nodes ON nodes.data_id=image_color_histograms.datum_id";
      SQL_cmd += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_ID="+stringfunc::number_to_string(mission_ID);
      SQL_cmd += " ORDER BY nodes.node_ID";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return attributes_metadata_map_ptr;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      string key,value;
      STRING_PAIR curr_pair;
      vector<STRING_PAIR> attribute_pairs;
      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         attribute_pairs.clear();
         
         int node_ID=stringfunc::string_to_number(field_array_ptr->get(m,0));

         key="primary_color";
         value=field_array_ptr->get(m,1);
         curr_pair=STRING_PAIR(key,value);
         attribute_pairs.push_back(curr_pair);

         key="secondary_color";
         value=field_array_ptr->get(m,2);
         curr_pair=STRING_PAIR(key,value);
         attribute_pairs.push_back(curr_pair);

         key="tertiary_color";
         value=field_array_ptr->get(m,3);
         curr_pair=STRING_PAIR(key,value);
         attribute_pairs.push_back(curr_pair);

         (*attributes_metadata_map_ptr)[node_ID]=attribute_pairs;
      } // loop over index m 

      return attributes_metadata_map_ptr;
   }

// ---------------------------------------------------------------------   
// Method generate_retrieve_image_colorings_SQL_command() 

   string generate_retrieve_image_colorings_SQL_command(
      int campaign_ID,int mission_ID,string primary_color)
   {
//      cout << "inside imagesdatabasefunc::generate_retrieve_image_colorings_SQL_command()" 
//           << endl;

      string SQL_command=
         "SELECT nodes.node_id,nodes.data_id from image_color_histograms ";
      SQL_command += 
         "INNER JOIN nodes ON nodes.data_id=image_color_histograms.datum_id ";
      SQL_command += "WHERE campaign_ID="
         +stringfunc::number_to_string(campaign_ID);
      SQL_command += " AND mission_ID="
         +stringfunc::number_to_string(mission_ID);
      SQL_command += " AND nodes.graph_ID=0 ";
      SQL_command += " AND primary_color='"+primary_color+"' ";
      SQL_command += " ORDER BY nodes.data_ID";
//      cout << "SQL_command = " << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------   
   bool retrieve_dominant_image_colors_from_database(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      string primary_color,
      vector<int>& node_IDs,vector<int>& datum_IDs)
   {
//      cout << "inside imagesdatabasefunc::retrieve_dominant_image_colors_from_database()" 
//           << endl;

      string SQL_cmd=generate_retrieve_image_colorings_SQL_command(
         campaign_ID,mission_ID,primary_color);
//      cout << "SQL_cmd = " << SQL_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int curr_node_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,0));
         int curr_datum_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,1));
         node_IDs.push_back(curr_node_ID);
         datum_IDs.push_back(curr_datum_ID);
      } // loop over index m labeling rows in *field_array_ptr

      return true;
   }

// ==========================================================================
// Image faces metadata methods
// ==========================================================================

// Method insert_image_face() takes in metadata for a
// new entry within the image_faces table of the IMAGERY
// database. It inserts this metadata into *gis_database_ptr.

   bool insert_image_face(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      double center_u,double center_v,double radius)
   {
      string SQL_command="insert into image_faces ";
      SQL_command += 
         "(campaign_ID,mission_ID,image_ID,datum_ID,center_u,center_v,radius) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(campaign_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(image_ID)+",";
      SQL_command += stringfunc::number_to_string(datum_ID)+",";
      SQL_command += stringfunc::number_to_string(center_u)+",";
      SQL_command += stringfunc::number_to_string(center_v)+",";
      SQL_command += stringfunc::number_to_string(radius);
      SQL_command += ");";
//      cout << "SQL_command = " << SQL_command << endl;

      vector<string> insert_commands;
      insert_commands.push_back(SQL_command);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------   
// Method retrieve_all_human_faces() issues a single SQL select
// command for all images containing human faces corresponding
// to the input campaign and mission IDs.  

   ATTRIBUTES_METADATA_MAP* retrieve_all_human_faces(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID)
   {
//      cout << "inside imagesdatabasefunc::retrieve_all_human_faces()" 
//           << endl;

      ATTRIBUTES_METADATA_MAP* attributes_metadata_map_ptr=
         new ATTRIBUTES_METADATA_MAP;

      string SQL_cmd=
         "SELECT nodes.node_id,center_u,center_v,radius ";
      SQL_cmd += "FROM image_faces ";
      SQL_cmd += 
         "INNER JOIN nodes ON nodes.data_id=image_faces.datum_id";
      SQL_cmd += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_ID="+stringfunc::number_to_string(mission_ID);
      SQL_cmd += " ORDER BY nodes.node_ID";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
//      cout << "field_array_ptr = " << field_array_ptr << endl;
      
      if (field_array_ptr==NULL) return attributes_metadata_map_ptr;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      typedef map<int,vector<threevector> > FACES_MAP;
      FACES_MAP* faces_map_ptr=new FACES_MAP();

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int node_ID=stringfunc::string_to_number(field_array_ptr->get(m,0));
         double center_u=
            stringfunc::string_to_number(field_array_ptr->get(m,1));
         double center_v=
            stringfunc::string_to_number(field_array_ptr->get(m,2));
         double radius=
            stringfunc::string_to_number(field_array_ptr->get(m,3));
         threevector face_circle(center_u,center_v,radius);

         FACES_MAP::iterator iter=faces_map_ptr->find(node_ID);
         if (iter==faces_map_ptr->end()) 
         {
            vector<threevector> face_circles;
            face_circles.push_back(face_circle);
            (*faces_map_ptr)[node_ID]=face_circles;
         }
         else
         {
            iter->second.push_back(face_circle);
         }
      }
      
      string key,value;
      STRING_PAIR curr_pair;
      vector<STRING_PAIR> attribute_pairs;
      for (FACES_MAP::iterator iter=faces_map_ptr->begin();
           iter != faces_map_ptr->end(); ++iter)
      {
         attribute_pairs.clear();
         int node_ID=iter->first;
         int n_human_faces=iter->second.size();

// As of 4/19/12, we lump 5,6,7... human faces into one large category:

         n_human_faces=basic_math::min(n_human_faces,5);

         key="human_faces";
         value=stringfunc::number_to_string(n_human_faces);

         curr_pair=STRING_PAIR(key,value);
         attribute_pairs.push_back(curr_pair);

         (*attributes_metadata_map_ptr)[node_ID]=attribute_pairs;
      }
      delete faces_map_ptr;

      return attributes_metadata_map_ptr;
   }

// ---------------------------------------------------------------------   
   bool retrieve_detected_face_circles_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      vector<twovector>& center,vector<double>& radius)
   {
//      cout << "inside imagesdatabasefunc::retrieve_detected_face_circles_from_database()" 
//           << endl;

      string SQL_cmd="SELECT center_u,center_v,radius from image_faces ";
      SQL_cmd += 
         "INNER JOIN nodes ON nodes.data_id=image_faces.datum_id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_ID="
         +stringfunc::number_to_string(hierarchy_ID);
      SQL_cmd += " AND nodes.node_ID="
         +stringfunc::number_to_string(node_ID);
//      cout << "SQL_cmd = " << SQL_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         double center_u=
            stringfunc::string_to_number(field_array_ptr->get(m,0));
         double center_v=
            stringfunc::string_to_number(field_array_ptr->get(m,1));
         double curr_radius=
            stringfunc::string_to_number(field_array_ptr->get(m,2));
         center.push_back(twovector(center_u,center_v));
         radius.push_back(curr_radius);
      } // loop over index m labeling rows in *field_array_ptr

      return true;
   }

// ==========================================================================
// Image annotations metadata methods
// ==========================================================================

// Method insert_image_annotations() takes in metadata for set of 
// new entries in the image_annotations table of the IMAGERY
// database. It inserts these metadata into *gis_database_ptr.

   bool insert_image_annotations(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,int datum_ID,
      const vector<double>& U,const vector<double>& V,
      const vector<string>& label)
   {

// First delete all existing image annotations corresponding to input
// campaign_ID, missoin_Id and image_ID:

      string SQL_command="DELETE FROM image_annotations ";
      SQL_command += " WHERE campaign_id="+
         stringfunc::number_to_string(campaign_ID);
      SQL_command += " AND mission_id="+
         stringfunc::number_to_string(mission_ID);
      SQL_command += " AND image_id="+
         stringfunc::number_to_string(image_ID);

      vector<string> insert_commands;
      insert_commands.push_back(SQL_command);
      gis_database_ptr->set_SQL_commands(insert_commands);
      gis_database_ptr->execute_SQL_commands();

// Next insert all input image annotations:

      unsigned int n_annotations=U.size();
      
      for (unsigned int n=0; n<n_annotations; n++)
      {
         SQL_command="insert into image_annotations ";
         SQL_command += 
            "(campaign_ID,mission_ID,image_ID,datum_ID,u,v,label) ";
         SQL_command += "values( ";
         SQL_command += stringfunc::number_to_string(campaign_ID)+",";
         SQL_command += stringfunc::number_to_string(mission_ID)+",";
         SQL_command += stringfunc::number_to_string(image_ID)+",";
         SQL_command += stringfunc::number_to_string(datum_ID)+",";
         SQL_command += stringfunc::number_to_string(U[n])+",";
         SQL_command += stringfunc::number_to_string(V[n])+",";
         SQL_command += "'"+label[n]+"'";
         SQL_command += ");";
//         cout << "SQL_command = " << SQL_command << endl;

         insert_commands.clear();
         insert_commands.push_back(SQL_command);
         gis_database_ptr->set_SQL_commands(insert_commands);
         gis_database_ptr->execute_SQL_commands();
      } // loop over index n labeling annotations

      return true;
   }

// ---------------------------------------------------------------------   
   bool retrieve_image_annotations_from_database(
      gis_database* gis_database_ptr,int hierarchy_ID,int node_ID,
      vector<twovector>& UV,vector<string>& label)
   {
//      cout << "inside imagesdatabasefunc::retrieve_image_annotations_from_database()" 
//           << endl;

      string SQL_cmd=
         "SELECT u,v,image_annotations.label from image_annotations ";
      SQL_cmd += 
         "INNER JOIN nodes ON nodes.data_id=image_annotations.datum_id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_ID="
         +stringfunc::number_to_string(hierarchy_ID);
      SQL_cmd += " AND nodes.node_ID="
         +stringfunc::number_to_string(node_ID);
//      cout << "SQL_cmd = " << SQL_cmd << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         double u=stringfunc::string_to_number(field_array_ptr->get(m,0));
         double v=stringfunc::string_to_number(field_array_ptr->get(m,1));
         UV.push_back(twovector(u,v));
         label.push_back(field_array_ptr->get(m,2));

//         cout << "m = " << m 
//              << " UV = " << UV.back()
//              << " label = " << label.back() << endl;
         
      } // loop over index m labeling rows in *field_array_ptr

      return true;

   }
   
// ---------------------------------------------------------------------   
// Method retrieve_all_image_annotations() issues a single SQL select
// command for all images containing annotations corresponding
// to the input campaign and mission IDs.  

   ATTRIBUTES_METADATA_MAP* retrieve_all_image_annotations(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID)
   {
//      cout << "inside imagesdatabasefunc::retrieve_all_image_annotations()" 
//           << endl;

      ATTRIBUTES_METADATA_MAP* attributes_metadata_map_ptr=
         new ATTRIBUTES_METADATA_MAP;

      string SQL_cmd=
         "SELECT nodes.node_id,u,v,image_annotations.label ";
      SQL_cmd += "FROM image_annotations ";
      SQL_cmd += 
         "INNER JOIN nodes ON nodes.data_id=image_annotations.datum_id";
      SQL_cmd += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_ID="+stringfunc::number_to_string(mission_ID);
      SQL_cmd += " ORDER BY nodes.node_ID";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
//      cout << "field_array_ptr = " << field_array_ptr << endl;
      
      if (field_array_ptr==NULL) return attributes_metadata_map_ptr;

//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      typedef map<int,int > ANNOTATIONS_MAP;
      ANNOTATIONS_MAP* annotations_map_ptr=new ANNOTATIONS_MAP();

// Independent var for *annotations_map_ptr: node_ID
// Dependent var for *annotations_map_ptr: number of annotations

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int node_ID=stringfunc::string_to_number(field_array_ptr->get(m,0));
//         double u=
//            stringfunc::string_to_number(field_array_ptr->get(m,1));
//         double v=
//            stringfunc::string_to_number(field_array_ptr->get(m,2));
//         string label=field_array_ptr->get(m,3);
         
         ANNOTATIONS_MAP::iterator iter=annotations_map_ptr->find(node_ID);
         if (iter==annotations_map_ptr->end()) 
         {
            (*annotations_map_ptr)[node_ID]=1;
         }
         else
         {
            iter->second=iter->second+1;
         }
      }
      
      string key,value;
      STRING_PAIR curr_pair;
      vector<STRING_PAIR> attribute_pairs;
      for (ANNOTATIONS_MAP::iterator iter=annotations_map_ptr->begin();
           iter != annotations_map_ptr->end(); ++iter)
      {
         attribute_pairs.clear();
         int node_ID=iter->first;
         int n_annotations=iter->second;

// As of 5/16/12, we lump 5,6,7... annotations into one large category:

         n_annotations=basic_math::min(n_annotations,5);

         key="annotations";
         value=stringfunc::number_to_string(n_annotations);

         curr_pair=STRING_PAIR(key,value);
         attribute_pairs.push_back(curr_pair);

         (*attributes_metadata_map_ptr)[node_ID]=attribute_pairs;
      }
      delete annotations_map_ptr;

      return attributes_metadata_map_ptr;
   }

// ==========================================================================
// Connected component methods
// ==========================================================================

// Method get_connected_component_image_URLs() queries the images table
// within *gis_database_ptr for all URLs corresponding to the
// specified graph hierarchy and connected component.  It returns the
// retrieved URLs within STL vector image_URLs.
 
   bool get_connected_component_image_URLs(
      gis_database* gis_database_ptr,int hierarchy_ID,
      int connected_component_ID,vector<string>& image_URLs)
   {
      string select_command="SELECT images.url FROM nodes ";
      select_command += "INNER JOIN images ON images.id=nodes.data_id ";
      select_command += " WHERE graph_hierarchy_id="+
         stringfunc::number_to_string(hierarchy_ID);
      select_command += " AND graph_ID=0";
      select_command += " AND connected_component_ID="+
         stringfunc::number_to_string(connected_component_ID);
//      cout << "select_command = " << select_command << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(select_command);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      image_URLs.clear();
      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         image_URLs.push_back(field_array_ptr->get(m,0));
      }
      return true;
   }

} // imagesdatabasefunc namespace
