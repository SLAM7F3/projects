// ========================================================================
// Program LOAD_SINGLE_IMAGE_METADATA is to add +1 to the images table in 
// an imagery-like database. 

// preq: image has been added to a subdir of /data/bundler/subdir/images
// preq: image has been downsized
// preq: image's thumbnail is in /data/bundler/subdir/images/thumbnails
// (preq:) image has been renamed 
// parameter: database
// user input: missionID and campaignID 
//		image filename, thumbnail filename

// modifies: 
// 	- adds picture (url, size, importance, thumbnail url, thumbnail 
// 	size, missionID, campaignID) to database
//	- if EXIF GPS exists, updates the_geom in images table of database
//	- inserts picture time into images table 

// 	RUN COMMAND

// 	./load_single_image_metadata --GIS_layer ./packages/Tid_metadata.pkg

// ========================================================================
// Last updated on 8/1/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
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

   int campaign_ID,mission_ID;
   cout << "Enter campaign_ID:" << endl;
   cin >> campaign_ID;

   cout << "Enter mission_ID:" << endl;
   cin >> mission_ID;

	string image_filename, thumbnail_filename;

	while (!filefunc::fileexist(image_filename))
	{ 
		cout << "Enter full path to image file:" << endl;
		cin >> image_filename;
	}

	while (!filefunc::fileexist(thumbnail_filename))
	{
		cout << "Enter full path to image thumbnail file:" << endl;
		cin >> thumbnail_filename; 
	}
	cout << endl;
 
	vector<string> sql_load_commands;

// generate sql command to load image into database: 
	int importance=1;
	int npx,npy;
	imagefunc::get_image_width_height(image_filename,npx,npy);
	int thumbnail_npx,thumbnail_npy;
	imagefunc::get_image_width_height(thumbnail_filename,thumbnail_npx,thumbnail_npy);

	string load_sql_cmd = imagesdatabasefunc::generate_insert_image_metadata_SQL_command(
		campaign_ID, mission_ID, importance, image_filename, npx, npy,
		thumbnail_filename, thumbnail_npx, thumbnail_npy);

	cout << "load_sql_cmd: " << load_sql_cmd << endl;

// run sql command to load image
	sql_load_commands.push_back(load_sql_cmd);
	postgis_db_ptr->set_SQL_commands(sql_load_commands);
	postgis_db_ptr->execute_SQL_commands();
	
	vector<string> sql_commands;

// retrieve image ID
	int image_ID=imagesdatabasefunc::get_image_serial_ID(postgis_db_ptr,campaign_ID,mission_ID,image_filename);


// determine time 
	double epoch = 0.0;
	string UTC;
	int UTM_zonenumber=imagesdatabasefunc::retrieve_campaign_UTM_zonenumber(
      postgis_db_ptr,campaign_ID);

// adapted from update_image_times
	bool parse_exif_metadata_flag = true;
	
	photograph photo(image_filename,0,parse_exif_metadata_flag);

	photo.get_clock().compute_UTM_zone_time_offset(UTM_zonenumber);
	bool exif_timestamp=photo.parse_timestamp_exiftag();

	if (exif_timestamp)
	{
			epoch=photo.get_clock().secs_elapsed_since_reference_date();
	}
	if (epoch <=0.0 or !exif_timestamp) // can't use EXIF data, get epoch from file
	{ 
		epoch=filefunc::assign_file_epoch_time(image_filename);

	} // end of getting epoch from file


	photo.get_clock().convert_elapsed_secs_to_date(epoch);

	UTC=photo.get_clock().YYYY_MM_DD_H_M_S();

	string update_time_cmd=imagesdatabasefunc::generate_update_image_metadata_SQL_command_serialID(
            		campaign_ID,mission_ID,image_ID,UTC,epoch);

	cout << "update_time_cmd: " << update_time_cmd << endl;

	sql_commands.push_back(update_time_cmd);

// try to get GPS, already have photograph object from getting time

	geopoint loc=photo.get_geolocation();

	double lat=loc.get_latitude();
	double lon=loc.get_longitude();

	if (lat==0.0 and lon==0.0)
	{
		cout << "Couldn't extract nonzero GPS" << endl;
		
	}

	else
	{
		string gps_cmd = "update images set the_geom= \"st_geomfromtext\"('POINT("
		+stringfunc::number_to_string(lon)+" "
		+stringfunc::number_to_string(lat)+")', 4326) " // for now, hard-coding the srid integer, 4326 
		+ "where campaign_id="
		+stringfunc::number_to_string(campaign_ID)
		+ " AND mission_id="
		+ stringfunc::number_to_string(mission_ID)
		+ " AND id=" 
		+ stringfunc::number_to_string(image_ID); 

		sql_commands.push_back(gps_cmd);

		cout << "gps_cmd: " << gps_cmd << endl;
	}

// run sql commands 
	postgis_db_ptr->set_SQL_commands(sql_commands);
	postgis_db_ptr->execute_SQL_commands();

}
