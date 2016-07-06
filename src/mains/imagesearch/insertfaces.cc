// ========================================================================
// Program INSERTFACES queries the user to enter a hierarchy ID.  It
// also reads in human face circles extracted from a set of images
// which are assumed to corresponding to the specified hierarchy.
// This program loads the metadata from the human face circle metadata
// into table image_faces within the IMAGERY database.
// ========================================================================
// Last updated on 4/19/12; 5/4/12; 5/10/12
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   int hierarchy_ID=2;	// GrandCanyon
//   int hierarchy_ID=4;	// NewsWrap
   cout << "Enter hierarchy_ID" << endl;
   cin >> hierarchy_ID;

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
   imagesdatabasefunc::get_campaign_mission_IDs(
      postgis_db_ptr,hierarchy_ID,campaign_ID,mission_ID);

   cout << "campaign_ID = " << campaign_ID
        << " mission_ID = " << mission_ID << endl;
   if (campaign_ID < 0 || mission_ID < 0) 
   {
      cout << "campaign_ID = " << campaign_ID 
           << " mission_ID = " << mission_ID << endl;
      exit(-1);
   }

   typedef map<string,vector<threevector> > FACES_MAP;
   FACES_MAP* faces_map_ptr=new FACES_MAP();

   string face_detections_filename="face_detections.txt";
   filefunc::ReadInfile(face_detections_filename);
   
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();

   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      string image_filename=substrings[0];
      double center_u=stringfunc::string_to_number(substrings[1]);
      double center_v=stringfunc::string_to_number(substrings[2]);
      double radius=stringfunc::string_to_number(substrings[3]);

/*      
      texture_rectangle_ptr->import_photo_from_file(image_filename);
      int width=texture_rectangle_ptr->getWidth();
      int height=texture_rectangle_ptr->getHeight();
//      cout << "width = " << width << " height = " << height << endl;

      double center_u,center_v;
      texture_rectangle_ptr->get_uv_coords(
         center_x,center_y,center_u,center_v);
      radius /= height;
*/

      threevector normalized_circle(center_u,center_v,radius);

      FACES_MAP::iterator iter=faces_map_ptr->find(image_filename);
      if (iter==faces_map_ptr->end()) 
      {
         vector<threevector> face_circles;
         face_circles.push_back(normalized_circle);
         (*faces_map_ptr)[image_filename]=face_circles;
      }
      else
      {
         iter->second.push_back(normalized_circle);
      }

      int datum_ID=imagesdatabasefunc::get_datum_ID(
         postgis_db_ptr,hierarchy_ID,image_filename);
      int image_ID=imagesdatabasefunc::get_image_ID(
         postgis_db_ptr,datum_ID);

      if (datum_ID < 0 || image_ID < 0) continue;

      cout << "i = " << i
           << " image = " << image_filename
//           << " center = " << center_x << " " << center_y 
           << " center = " << center_u << " " << center_v 
           << " r = " << radius << endl;
      cout << "campaign_ID = " << campaign_ID
           << " mission_ID = " << mission_ID
           << " image_ID = " << image_ID
           << " datum_ID = " << datum_ID
           << endl;
      cout << endl;

      imagesdatabasefunc::insert_image_face(
         postgis_db_ptr,campaign_ID,mission_ID,image_ID,datum_ID,
         center_u,center_v,radius);

   } // loop over index i 
   
   cout << "Number of images with faces = " << faces_map_ptr->size()
        << endl;
}
