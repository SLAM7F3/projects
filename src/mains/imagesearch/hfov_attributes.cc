// ========================================================================
// Program HFOV_ATTRIBUTES works with a set of FLIR video frames
// loaded into the IMAGERY database.  It first recovers all URLs
// corresponding to a specified hierarchy_ID and graph_ID=0 along with
// campaign, mission, image and datum IDs.  HORIZFOV then recovers the
// horizontal field-of-view values stored within the sensor_metadata
// table of the IMAGERY database.  It populates the image_attributes
// table with FOV key-value pairs.
// ========================================================================
// Last updated on 2/25/12; 2/26/12
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
   cout << "Enter graph hierarchy ID which contains FLIR video frames:"
        << endl;
   cin >> hierarchy_ID;

   int graph_ID=0;

   vector<int> campaign_IDs,mission_IDs,image_IDs,datum_IDs;
   vector<string> URLs;
   
   imagesdatabasefunc::get_image_URLs(
      postgis_db_ptr,hierarchy_ID,graph_ID,
      campaign_IDs,mission_IDs,image_IDs,datum_IDs,URLs);

   for (unsigned int i=0; i<URLs.size(); i++)
   {
      if (i%100==0) cout << i << " " << flush;
      
      string URL=URLs[i];
//      cout << "URL = " << URL << endl;

      double horiz_fov;
      imagesdatabasefunc::retrieve_particular_sensor_hfov_from_database(
         postgis_db_ptr,campaign_IDs[i],mission_IDs[i],image_IDs[i],horiz_fov);

      double trunc_horiz_fov;
      if (horiz_fov > 25)
      {
         trunc_horiz_fov=29;
      }
      else if (horiz_fov > 4.5)
      {
         trunc_horiz_fov=4.6;
      }
      else if (horiz_fov > 0.8)
      {
         trunc_horiz_fov=0.9;
      }
      else if (horiz_fov > 0.47)
      {
         trunc_horiz_fov=0.49;
      }
      else
      {
         trunc_horiz_fov=0.24;
      }

      string attribute_key="FLIR_horizontal_FOV_(deg)";
      string attribute_value=stringfunc::number_to_string(trunc_horiz_fov,2);

      cout << "image_ID = " << image_IDs[i]
//           << " datum_ID = " << datum_IDs[i]
           << " URL = " << filefunc::getbasename(URL)
           << " trunc(fov) = " << attribute_value << endl;

      imagesdatabasefunc::insert_image_attribute(
         postgis_db_ptr,campaign_IDs[i],mission_IDs[i],image_IDs[i],
         datum_IDs[i],attribute_key,attribute_value);

   } // loop over index i labeling input jpeg files
   cout << endl;

}
