// ==========================================================================
// Stand-alone moverfunc methods
// ==========================================================================
// Last updated on 10/22/11; 1/23/14; 4/5/14; 6/7/14
// ==========================================================================

#include <iostream>
#include <unistd.h> // for sleep() call 
#include "astro_geo/Clock.h"
#include "astro_geo/Ellipsoid_model.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "postgres/gis_database.h"
#include "track/mover_funcs.h"
#include "geometry/polygon.h"
#include "geometry/polyline.h"
#include "general/sysfuncs.h"
#include "track/track.h"
#include "track/tracks_group.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

namespace mover_func
{

   double compute_relative_ROI_size(polyline* polyline_ptr)
      {
//         cout << "inside mover_func::compute_relative_ROI_size()" << endl;

// Compute bounding box for bottom face and its area.  Then form
// relative size metric for ROI based upon sqrt(bbox_area) to typical
// ROI characterisic length scale of 65 meters:

         polygon bottom_face(*polyline_ptr);
         polygon* bbox_ptr=bottom_face.generate_bounding_box();
         double sqrt_bbox_area=sqrt(bbox_ptr->get_area());
         delete bbox_ptr;
         double relative_ROI_size=sqrt_bbox_area/65.0;	
//         cout << "relative_ROI_size = " << relative_ROI_size << endl;
         return relative_ROI_size;
      }

// --------------------------------------------------------------------------

   vector<threevector> extrude_bottom_ROI_face(
      polyline* polyline_ptr,double skeleton_height)
      {
//         cout << "inside mover_func::extrude_bottom_ROI_face()" << endl;

// First find maximum bottom face vertex altitude:
         
         double max_bottom_vertex_altitude=NEGATIVEINFINITY;
         for (unsigned int v=0; v<polyline_ptr->get_n_vertices(); v++)
         {
            threevector curr_bottom_face_vertex=polyline_ptr->get_vertex(v);
            max_bottom_vertex_altitude=
               basic_math::max(
                  max_bottom_vertex_altitude,curr_bottom_face_vertex.get(2));
         }
//         cout << "max_bottom_vertex_altitude = "
//              << max_bottom_vertex_altitude << endl;

// Extrude bottom face vertices upwards:

         vector<threevector> extruded_polyline_vertices;
         for (unsigned int v=0; v<polyline_ptr->get_n_vertices(); v++)
         {
            threevector curr_bottom_vertex=polyline_ptr->get_vertex(v);
            threevector curr_top_vertex=curr_bottom_vertex;
            curr_top_vertex.put(2,max_bottom_vertex_altitude+skeleton_height);

            extruded_polyline_vertices.push_back(curr_top_vertex);
            extruded_polyline_vertices.push_back(curr_bottom_vertex);
            extruded_polyline_vertices.push_back(curr_top_vertex);
         }

         return extruded_polyline_vertices;
      }

// ==========================================================================
// Flat retrieval of table metadata from TOC database member
// functions:
// ==========================================================================

// Method retrieve_fieldtest_metadata_from_database() queries the
// fieldtest table in the TOC database for fieldtest labels and IDs.

   void retrieve_fieldtest_metadata_from_database(
      gis_database* gis_database_ptr,
      vector<string>& fieldtest_label,vector<int>& fieldtest_ID,
      bool weekday_mon_day_flag)
   {
//      cout << "inside mover_func::retrieve_fieldtest_metadata_from_database()"
//           << endl;


/*
SELECT (
	label || ': ' || to_char(ftime, 'Dy Mon DD') ) AS fieldtest_label, 
	fieldtest_id
	
FROM (
	SELECT f.label,	f.id AS fieldtest_id, f.start_time_stamp AS ftime 
	FROM fieldtests AS f
	ORDER BY f.id
) as foo
*/

      string curr_select_cmd="SELECT ";

      if (weekday_mon_day_flag)
      {
         curr_select_cmd += 
            "(label || ': ' || to_char(ftime, 'Dy Mon DD') ) ";
         curr_select_cmd += "AS fieldtest_label,";
      
         curr_select_cmd += "fieldtest_id FROM (";
         curr_select_cmd += 
            "SELECT f.label,f.id AS fieldtest_id, f.start_time_stamp AS ftime ";
         curr_select_cmd += "FROM fieldtests AS f ";
         curr_select_cmd += "ORDER BY f.start_time_stamp";
//            "ORDER BY f.id";
         curr_select_cmd += ") as foo;";
      }
      else
      {
         curr_select_cmd += "start_time_stamp,id from fieldtests;";
      }

      cout << curr_select_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) 
      {
//         cout << "curr_select_cmd = " << endl;
//         cout << curr_select_cmd << endl;
//         cout << "No output returned from database!" << endl;
//         outputfunc::enter_continue_char();
         return;
      }
      
//   cout << "Field_array_ptr = " << field_array_ptr << endl;
//   cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//   cout << "mdim = " << field_array_ptr->get_mdim()
//        << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         fieldtest_label.push_back(field_array_ptr->get(i,0));
         fieldtest_ID.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,1)));

//         cout << "i = " << i
//              << " fieldtest_label = " << fieldtest_label.back()
//              << " fieldtest_ID = " << fieldtest_ID.back() << endl;
      } // loop over index i labeling database rows
   }

// ---------------------------------------------------------------------
// Method get_fieldtest_date() takes in some selected_fieltest_ID and
// returns a string containing the fieldtest's date in YYYY-MM-DD form.

   string get_fieldtest_date(int selected_fieldtest_ID,
      gis_database* gis_database_ptr)
   {
      vector<int> fieldtest_IDs;
      vector<string> fieldtest_labels;
   
      bool weekday_mon_day_flag=false;
      retrieve_fieldtest_metadata_from_database(
         gis_database_ptr,fieldtest_labels,fieldtest_IDs,weekday_mon_day_flag);
      int index=-1;
      for (unsigned int j=0; j<fieldtest_IDs.size(); j++)
      {
         if (fieldtest_IDs[j]==selected_fieldtest_ID)
         {
            index=j;
         }
      }
      string selected_fieldtest_label=fieldtest_labels[index];
//      cout << "selected_fieldtest_label = " << selected_fieldtest_label 
//           << endl;
      vector<string> substrings=
         stringfunc::decompose_string_into_substrings(
            selected_fieldtest_label);
      string selected_fieldtest_date=substrings[0];
      cout << "selected_fieldtest_date = " << selected_fieldtest_date << endl;

      return selected_fieldtest_date;
   }

// ---------------------------------------------------------------------
// Method retrieve_mission_metadata_from_database() queries the
// mission table in the TOC database for mission IDs returned in
// increasing order.

   void retrieve_mission_metadata_from_database(
      gis_database* gis_database_ptr,vector<int>& mission_ID)
   {
      bool just_mission_ID_flag=true;
      vector<int> fieldtest_ID,platform_ID;
      vector<string> mission_label,fieldtest_label,platform_label;
      retrieve_mission_metadata_from_database(
         gis_database_ptr,mission_label,mission_ID,
         fieldtest_label,fieldtest_ID,platform_label,platform_ID,
         just_mission_ID_flag);
   }

// ---------------------------------------------------------------------
// Method retrieve_platform_metadata_from_database() queries the
// platform table in the TOC database for platform labels and IDs.

   void retrieve_platform_metadata_from_database(
      gis_database* gis_database_ptr,
      vector<string>& platform_label,vector<int>& platform_ID)
   {
//      cout << "inside mover_func::retrieve_platform_metadata_from_database()"
//           << endl;

      string curr_select_cmd="SELECT description,id from platforms ";
      curr_select_cmd += "ORDER BY description;";
//      cout << curr_select_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) 
      {
//         cout << "curr_select_cmd = " << endl;
//         cout << curr_select_cmd << endl;
//         cout << "No output returned from database!" << endl;
//         outputfunc::enter_continue_char();
         return;
      }

//   cout << "Field_array_ptr = " << field_array_ptr << endl;
//   cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//   cout << "mdim = " << field_array_ptr->get_mdim()
//        << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         platform_label.push_back(field_array_ptr->get(i,0));
         platform_ID.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,1)));

//         cout << "i = " << i
//              << " platform_label = " << platform_label.back()
//              << " platform_ID = " << platform_ID.back() << endl;
      } // loop over index i labeling database rows
   }

// ---------------------------------------------------------------------
// Method retrieve_sensor_metadata_from_database() queries the
// sensor table in the TOC database for sensor labels and IDs.

   void retrieve_sensor_metadata_from_database(
      gis_database* gis_database_ptr,
      vector<string>& sensor_label,vector<int>& sensor_ID)
   {
//      cout << "inside mover_func::retrieve_sensor_metadata_from_database()"
//           << endl;

      string curr_select_cmd="SELECT description,id from sensors ";
      curr_select_cmd += "ORDER BY description;";
//      cout << curr_select_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) 
      {
//         cout << "curr_select_cmd = " << endl;
//         cout << curr_select_cmd << endl;
//         cout << "No output returned from database!" << endl;
//         outputfunc::enter_continue_char();
         return;
      }

//   cout << "Field_array_ptr = " << field_array_ptr << endl;
//   cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//   cout << "mdim = " << field_array_ptr->get_mdim()
//        << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         sensor_label.push_back(field_array_ptr->get(i,0));
         sensor_ID.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,1)));

//         cout << "i = " << i
//              << " sensor_label = " << sensor_label.back()
//              << " sensor_ID = " << sensor_ID.back() << endl;
      } // loop over index i labeling database rows
   }

// ==========================================================================
// Correlated retrieval of table metadata from TOC database member
// functions:
// ==========================================================================

// Method retrieve_fieldtest_mission_metadata_from_database() queries the
// fieldtest and mission tables in the TOC database for correlated
// fieldtest and mission labels and IDs returned in increasing
// chronological order.

   void retrieve_fieldtest_mission_metadata_from_database(
      gis_database* gis_database_ptr,
      vector<string>& fieldtest_label,vector<int>& fieldtest_ID,
      vector<string>& mission_label,vector<int>& mission_ID)
   {
//      cout << "inside mover_func::retrieve_fieldtest_mission_metadata_from_database()"
//           << endl;
      
      string curr_select_cmd="SELECT ";

      curr_select_cmd += 
         "(f.label || ': ' || to_char(f.start_time_stamp, 'Dy Mon DD') ) ";
      curr_select_cmd += "AS fieldtest_label, ";
      curr_select_cmd += "f.id AS fieldtest_id, ";
      curr_select_cmd += 
         "('#' || m.id || '  ' || to_char(m.start_time_stamp, 'HH:MI PM') )";
      curr_select_cmd += "AS mission_label,";
      curr_select_cmd += "m.id AS mission_id ";
      curr_select_cmd += "FROM missions AS m, fieldtests AS f ";
      curr_select_cmd += "WHERE m.fieldtest_id = f.id ";
      curr_select_cmd += "ORDER BY m.start_time_stamp;";
      cout << curr_select_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) 
      {
//         cout << "curr_select_cmd = " << endl;
//         cout << curr_select_cmd << endl;
//         cout << "No output returned from database!" << endl;
//         outputfunc::enter_continue_char();
         return;
      }

//   cout << "Field_array_ptr = " << field_array_ptr << endl;
//   cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//   cout << "mdim = " << field_array_ptr->get_mdim()
//        << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         fieldtest_label.push_back(field_array_ptr->get(i,0));
         fieldtest_ID.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,1)));
         mission_label.push_back(field_array_ptr->get(i,2));
         mission_ID.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,3)));

         cout << "i = " << i
              << " fieldtest_label = " << fieldtest_label.back()
              << " fieldtest_ID = " << fieldtest_ID.back() << endl;
         cout << "mission_label = " << mission_label.back()
              << " mission_ID = " << mission_ID.back() << endl << endl;
      } // loop over index i labeling database rows
   }

// ---------------------------------------------------------------------
// Method retrieve_fieldtest_mission_platform_metadata_from_database()
// queries the fieldtest, mission and platform tables in the TOC
// database for correlated labels and IDs returned in increasing
// chronological order.

   void retrieve_fieldtest_mission_platform_metadata_from_database(
      gis_database* gis_database_ptr,
      std::vector<std::string>& fieldtest_label,
      std::vector<int>& fieldtest_ID,
      std::vector<std::string>& mission_label,
      std::vector<int>& mission_ID,
      std::vector<std::string>& platform_label,
      std::vector<int>& platform_ID)
{

   string curr_select_cmd="SELECT ";
   curr_select_cmd += 
      "(f.label || ': ' || to_char(f.start_time_stamp, 'Dy Mon DD') ) ";
   curr_select_cmd += "AS fieldtest_label, ";
   curr_select_cmd += "f.id AS fieldtest_id, ";
   curr_select_cmd += 
      "('#' || m.id || '  ' || to_char(m.start_time_stamp, 'HH:MI PM') )";
   curr_select_cmd += "AS mission_label,";
   curr_select_cmd += "m.id AS mission_id, ";
   curr_select_cmd += "p.description AS platform_label, ";
   curr_select_cmd += "p.id AS platform_id ";
   curr_select_cmd += "FROM fieldtests AS f, missions AS m, platforms AS p ";
   curr_select_cmd += "WHERE m.fieldtest_id = f.id AND m.platform_id=p.id ";
   curr_select_cmd += "ORDER BY m.start_time_stamp;";
//   cout << curr_select_cmd << endl;

   Genarray<string>* field_array_ptr=gis_database_ptr->
      select_data(curr_select_cmd);
   if (field_array_ptr==NULL) 
   {
//      cout << "curr_select_cmd = " << endl;
//      cout << curr_select_cmd << endl;
//      cout << "No output returned from database!" << endl;
//      outputfunc::enter_continue_char();
      return;
   }

//   cout << "Field_array_ptr = " << field_array_ptr << endl;
//   cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//   cout << "mdim = " << field_array_ptr->get_mdim()
//        << " ndim = " << field_array_ptr->get_ndim() << endl;

   for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
   {
      fieldtest_label.push_back(field_array_ptr->get(i,0));
      fieldtest_ID.push_back(stringfunc::string_to_number(
         field_array_ptr->get(i,1)));
      mission_label.push_back(field_array_ptr->get(i,2));
      mission_ID.push_back(stringfunc::string_to_number(
         field_array_ptr->get(i,3)));
      platform_label.push_back(field_array_ptr->get(i,4));
      platform_ID.push_back(stringfunc::string_to_number(
         field_array_ptr->get(i,5)));

//      cout << "i = " << i
//           << " fieldtest_label = " << fieldtest_label.back()
//           << " fieldtest_ID = " << fieldtest_ID.back() << endl;
//      cout << "mission_label = " << mission_label.back()
//           << " mission_ID = " << mission_ID.back() << endl << endl;
//      cout << "platform_label = " << platform_label.back()
//           << " platform_ID = " << platform_ID.back() << endl << endl;
         
   } // loop over index i labeling database rows
}

// ---------------------------------------------------------------------
// Method
// retrieve_correlated_fieldtest_mission_platform_sensor_metadata_()
// executes Delsey's SQL command which queries the track_points table
// from the TOC database for correlated fieldtest, mission, platform
// and sensor
// labels and IDs.

   void retrieve_correlated_fieldtest_mission_platform_sensor_metadata(
         gis_database* gis_database_ptr,string database_table_name,
         vector<string>& fieldtest_label,vector<int>& fieldtest_ID,
         vector<string>& mission_label,vector<int>& mission_ID,
         vector<string>& platform_label,vector<int>& platform_ID,
         vector<string>& sensor_label,vector<int>& sensor_ID)
   {
//      cout << "inside mover_func::retrieve_correlated_fieldtest_mission_platform_sensor_metadata()"
//           << endl;

/*
// Delsey's SELECT cmd modified on 10/10/10

SELECT (label || ': ' || to_char(ftime, 'Dy Mon DD') ) AS fieldtest_label,
	fieldtest_id,
	('#' || mission_id || ' ' || to_char(mtime,'HH:MI PM') ) AS mission_label,
	mission_id,platform_label,platform_id,sensor_label,sensor_id
FROM (
	SELECT f.label,t.fieldtest_id, 
		f.start_time_stamp AS ftime,
		t.mission_id, m.start_time_stamp AS mtime, 
		p.description AS platform_label, t.platform_id, 
		s.description AS sensor_label, t.sensor_id 
	FROM fieldtests AS f, missions AS m, platforms AS p, sensors AS s,
		( SELECT DISTINCT fieldtest_id, mission_id, platform_id, sensor_id FROM photos ) AS t
	WHERE f.id = t.fieldtest_id 
	AND m.id = t.mission_id 
	AND p.id = t.platform_id 
	AND s.id = t.sensor_id 
	GROUP BY f.label, t.fieldtest_id, f.start_time_stamp, t.mission_id, 
		m.start_time_stamp,p.description, t.platform_id, s.description, 
		t.sensor_id 
	ORDER BY f.start_time_stamp, t.mission_id, p.description, s.description
) as foo;
*/


      string curr_select_cmd="SELECT ";
      curr_select_cmd += 
         "(label || ': ' || to_char(ftime, 'Dy Mon DD') ) AS fieldtest_label,";
      curr_select_cmd += "fieldtest_id,";
      curr_select_cmd +=
         "('#' || mission_id || '  ' || to_char(mtime, 'HH:MI PM') ) ";
      curr_select_cmd += "AS mission_label,";
      curr_select_cmd += "mission_id,";
      curr_select_cmd += 
         "platform_label,platform_id,sensor_label,sensor_id ";
      curr_select_cmd += "FROM (";
      curr_select_cmd += 
         "SELECT f.label,t.fieldtest_id, f.start_time_stamp AS ftime,";
      curr_select_cmd += "t.mission_id, m.start_time_stamp AS mtime, ";
      curr_select_cmd += "p.description AS platform_label, t.platform_id, ";
      curr_select_cmd += "s.description AS sensor_label, t.sensor_id ";
      curr_select_cmd += "FROM fieldtests AS f, missions AS m, platforms AS p, sensors AS s, ";
      curr_select_cmd += 
         "( SELECT DISTINCT fieldtest_id, mission_id, platform_id, sensor_id FROM ";
      curr_select_cmd += database_table_name+"  ) AS t ";
      curr_select_cmd += "WHERE f.id = t.fieldtest_id ";
      curr_select_cmd += "AND m.id = t.mission_id ";
      curr_select_cmd += "AND p.id = t.platform_id ";
      curr_select_cmd += "AND s.id = t.sensor_id ";
      curr_select_cmd += 
         "GROUP BY f.label, t.fieldtest_id, f.start_time_stamp, ";
      
      curr_select_cmd += "t.mission_id, m.start_time_stamp,";
      curr_select_cmd += "p.description, t.platform_id, ";
      curr_select_cmd += "s.description, t.sensor_id ";
      curr_select_cmd += 
         "ORDER BY f.start_time_stamp, t.mission_id, p.description, s.description";
      curr_select_cmd += ") as foo;";

      cout << "curr_select_cmd = " << endl;
      cout << curr_select_cmd << endl;

//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
         
      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) 
      {
//         cout << "curr_select_cmd = " << endl;
//         cout << curr_select_cmd << endl;
//         cout << "No output returned from database!" << endl;
//         outputfunc::enter_continue_char();
         return;
      }

//   cout << "Field_array_ptr = " << field_array_ptr << endl;
//   cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//   cout << "mdim = " << field_array_ptr->get_mdim()
//        << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         fieldtest_label.push_back(field_array_ptr->get(i,0));
         fieldtest_ID.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,1)));
         mission_label.push_back(field_array_ptr->get(i,2));
         mission_ID.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,3)));
         platform_label.push_back(field_array_ptr->get(i,4));
         platform_ID.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,5)));
         sensor_label.push_back(field_array_ptr->get(i,6));
         sensor_ID.push_back(stringfunc::string_to_number(
            field_array_ptr->get(i,7)));
      } // loop over index i labeling database rows
   }

// ---------------------------------------------------------------------
   void retrieve_mission_metadata_from_database(
      gis_database* gis_database_ptr,
      vector<string>& mission_label,vector<int>& mission_ID,
      vector<string>& fieldtest_label,vector<int>& fieldtest_ID,
      vector<string>& platform_label,vector<int>& platform_ID,
      bool just_mission_ID_flag)
   {
//      cout << "inside mover_func::retrieve_mission_metadata_from_database()"
//           << endl;
//      cout << "just_mission_ID_flag = " << just_mission_ID_flag << endl;

// First retrieve relationships between labels and IDs for fieldtests and
// platforms from their TOC database tables:

      vector<string> TOC_fieldtest_labels,TOC_platform_labels;
      vector<int> TOC_fieldtest_IDs,TOC_platform_IDs;
      if (!just_mission_ID_flag)
      {
         bool weekday_mon_day_flag=true;
         retrieve_fieldtest_metadata_from_database(
            gis_database_ptr,TOC_fieldtest_labels,TOC_fieldtest_IDs,
            weekday_mon_day_flag);
         retrieve_platform_metadata_from_database(
            gis_database_ptr,TOC_platform_labels,TOC_platform_IDs);
      }

      string curr_select_cmd;
      if (just_mission_ID_flag)
      {
         curr_select_cmd="SELECT id from missions ORDER by id;";
      }
      else
      {
         curr_select_cmd="SELECT(";
         curr_select_cmd +="'#' || id || '  ' || to_char(start_time_stamp, ";
         curr_select_cmd += "'HH:MI PM') ) ,id,fieldtest_id,platform_id ";
         curr_select_cmd += "FROM missions;";
      }
      
//      cout << curr_select_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) 
      {
//         cout << "curr_select_cmd = " << endl;
//         cout << curr_select_cmd << endl;
//         cout << "No output returned from database!" << endl;
//         outputfunc::enter_continue_char();
         return;
      }

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;
      
      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         if (just_mission_ID_flag)
         {
            mission_ID.push_back(stringfunc::string_to_number(
               field_array_ptr->get(i,0)));
            mission_label.push_back("");
            fieldtest_ID.push_back(-1);
            fieldtest_label.push_back("");
            platform_ID.push_back(-1);
	    platform_label.push_back("");
         }
         else
         {
            mission_label.push_back(field_array_ptr->get(i,0));
            mission_ID.push_back(stringfunc::string_to_number(
               field_array_ptr->get(i,1)));

            int curr_fieldtest_ID=stringfunc::string_to_number(
               field_array_ptr->get(i,2));
            int curr_platform_ID=stringfunc::string_to_number(
               field_array_ptr->get(i,3));
            
            for (unsigned int j=0; j<TOC_fieldtest_IDs.size(); j++)
            {
               if (curr_fieldtest_ID==TOC_fieldtest_IDs[j])
               {
                  fieldtest_ID.push_back(curr_fieldtest_ID);
                  fieldtest_label.push_back(TOC_fieldtest_labels[j]);
               }
            }

            for (unsigned int k=0; k<TOC_platform_IDs.size(); k++)
            {
               if (curr_platform_ID==TOC_platform_IDs[k])
               {
                  platform_ID.push_back(curr_platform_ID);
                  platform_label.push_back(TOC_platform_labels[k]);
               }
            }
         } // just_mission_ID_flag conditional
         
//         cout << "i = " << i
//              << " mission_ID = " << mission_ID.back() 
//              << " mission_label = " << mission_label.back()
//              << " fieldtest_ID = " << fieldtest_ID.back()
//              << " fieldtest_label = " << fieldtest_label.back()
//              << " platform_ID = " << platform_ID.back()
//              << " platform_label = " << platform_label.back()
//              << endl;
      } // loop over index i labeling database rows
   }

// ==========================================================================
// GPS track methods
// ==========================================================================

// Method generate_insert_track_point_SQL_command() takes in
// metadata associated with a single point in the current track.  
// It generates and returns a string containing a SQL insert command
// needed to populate a row within the track_points table of the
// TOC postgis database.

   string generate_insert_track_point_SQL_command(
      int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID,
      double secs_since_epoch,
      int fix_quality,int n_satellites,double horizontal_dilution,
      double longitude,double latitude,double altitude,
      double roll,double pitch,double yaw)
   {
//   cout << "inside mover_func::generate_insert_track_point_SQL_command()" << endl;

      Clock clock;
      clock.convert_elapsed_secs_to_date(secs_since_epoch);
      string date_str=clock.YYYY_MM_DD_H_M_S();
      
      string SQL_command="insert into track_points ";
      SQL_command += "(fieldtest_ID,mission_ID,platform_ID,sensor_ID,";
      SQL_command += "time_stamp,fix_quality,n_satellites,";
      SQL_command += "horizontal_dilution,z_posn,roll,pitch,yaw,xy_posn) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(fieldtest_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(platform_ID)+",";
      SQL_command += stringfunc::number_to_string(sensor_ID)+",";
      SQL_command += "'"+date_str+"',";

      SQL_command += stringfunc::number_to_string(fix_quality)+",";
      SQL_command += stringfunc::number_to_string(n_satellites)+",";
      SQL_command += stringfunc::number_to_string(horizontal_dilution)+",";

      SQL_command += stringfunc::number_to_string(altitude)+",";
      SQL_command += stringfunc::number_to_string(roll)+",";
      SQL_command += stringfunc::number_to_string(pitch)+",";
      SQL_command += stringfunc::number_to_string(yaw)+",";
      SQL_command += "'SRID=4326; POINT("
         +stringfunc::number_to_string(longitude,9)
         +" "+stringfunc::number_to_string(latitude,9)+")'";
      SQL_command += ");";

//   cout << SQL_command << endl;
      return SQL_command;
   }

// ---------------------------------------------------------------------
// Method retrieve_track_points_metadata_from_database() takes in
// *gis_database_ptr which is assumed to contain a "TOC" database with
// a "track_points" table.  It extracts all rows and columns from this
// database table.  After sorting the extracted rows according to
// photo_ID, this method fills output STL vectors with track metadata.

   void retrieve_track_points_metadata_from_database(
      bool daylight_savings_flag,gis_database* gis_database_ptr,
      int mission_ID,int sensor_ID,
      vector<int>& trackpoint_ID,vector<double>& elapsed_secs,
      vector<int>& fix_quality,vector<int>& n_satellites, 
      vector<double>& horiz_dilution,vector<double>& longitude,
      vector<double>& latitude,vector<double>& altitude,
      vector<double>& roll,vector<double>& pitch,vector<double>& yaw)
   {
      vector<string> time_stamp;
      mover_func::retrieve_track_points_metadata_from_database(
         gis_database_ptr,mission_ID,sensor_ID,
         trackpoint_ID,time_stamp,fix_quality,n_satellites,horiz_dilution,
         altitude,roll,pitch,yaw,longitude,latitude);
      
      elapsed_secs=mover_func::UTC_timestamp_to_secs_since_epoch(
         daylight_savings_flag,time_stamp,longitude,latitude,altitude);
   }

   void retrieve_track_points_metadata_from_database(
      gis_database* gis_database_ptr,int mission_ID,int sensor_ID,
      vector<int>& trackpoint_ID,vector<string>& time_stamp,
      vector<int>& fix_quality,vector<int>& n_satellites, 
      vector<double>& horiz_dilution,vector<double>& zposn,
      vector<double>& roll,vector<double>& pitch,vector<double>& yaw,
      vector<double>& longitude,vector<double>& latitude)
   {
//      cout << "inside mover_func::retrieve_track_points_metadata_from_database()"
//           << endl;

      trackpoint_ID.clear();
      time_stamp.clear();
      fix_quality.clear();
      n_satellites.clear();
      horiz_dilution.clear();
      zposn.clear();
      roll.clear();
      pitch.clear();
      yaw.clear();
      longitude.clear();
      latitude.clear();
   
      string curr_select_cmd = 
         "SELECT id,time_stamp,fix_quality,n_satellites,horizontal_dilution,";
      curr_select_cmd += "z_posn,roll,pitch,yaw,";
      curr_select_cmd += 
         "x(xy_posn) as longitude,y(xy_posn) as latitude ";
      curr_select_cmd += "from track_points ";
      curr_select_cmd += " where mission_id="+
         stringfunc::number_to_string(mission_ID);
      curr_select_cmd += " AND sensor_id="+
         stringfunc::number_to_string(sensor_ID);
      curr_select_cmd += ";";
//      cout << "curr_select_cmd = " << curr_select_cmd << endl;
         
      Genarray<string>* field_array_ptr=gis_database_ptr->
         select_data(curr_select_cmd);
      if (field_array_ptr==NULL) 
      {
//         cout << "curr_select_cmd = " << endl;
//         cout << curr_select_cmd << endl;
//         cout << "No output returned from database!" << endl;
//         outputfunc::enter_continue_char();
         return;
      }

//   cout << "Field_array_ptr = " << field_array_ptr << endl;
//   cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//   cout << "mdim = " << field_array_ptr->get_mdim()
//        << " ndim = " << field_array_ptr->get_ndim() << endl;

      vector<int> trackpoint_ID_copy;
      for (unsigned int i=0; i<field_array_ptr->get_mdim(); i++)
      {
         int curr_trackpoint_ID=stringfunc::string_to_number(
            field_array_ptr->get(i,0));
         string curr_time_stamp=field_array_ptr->get(i,1);   
         int curr_fix_quality=stringfunc::string_to_number(
            field_array_ptr->get(i,2));
         int curr_n_satellites=stringfunc::string_to_number(
            field_array_ptr->get(i,3));
         double curr_horiz_dilution=stringfunc::string_to_number(
            field_array_ptr->get(i,4));
         double curr_z=stringfunc::string_to_number(
            field_array_ptr->get(i,5));
         double curr_roll=stringfunc::string_to_number(
            field_array_ptr->get(i,6));
         double curr_pitch=stringfunc::string_to_number(
            field_array_ptr->get(i,7));
         double curr_yaw=
            stringfunc::string_to_number(field_array_ptr->get(i,8));
         double curr_lon=
            stringfunc::string_to_number(field_array_ptr->get(i,9));
         double curr_lat=
            stringfunc::string_to_number(field_array_ptr->get(i,10));

         trackpoint_ID.push_back(curr_trackpoint_ID);
         trackpoint_ID_copy.push_back(curr_trackpoint_ID);
         time_stamp.push_back(curr_time_stamp);
         fix_quality.push_back(curr_fix_quality);
         n_satellites.push_back(curr_n_satellites);
         horiz_dilution.push_back(curr_horiz_dilution);
         zposn.push_back(curr_z);
         roll.push_back(curr_roll);
         pitch.push_back(curr_pitch);
         yaw.push_back(curr_yaw);
         longitude.push_back(curr_lon);
         latitude.push_back(curr_lat);

      } // loop over index i labeling database rows

// Recall database retrieval is NOT guaranteed to be sorted!  So in
// order to maintain consistency with Noah's original photo indexing
// conventions, we need to sort all values extracted from the database
// w.r.t photo_ID and its copies:
   
      templatefunc::Quicksort(
         trackpoint_ID,time_stamp,zposn,fix_quality,n_satellites,
         horiz_dilution);
      templatefunc::Quicksort(
         trackpoint_ID_copy,zposn,roll,pitch,yaw,longitude,latitude);
   }

// ---------------------------------------------------------------------   
// Method retrieve_all_tracks_in_TOC_database() takes in a selected
// fieldtest ID.  It extracts from the track_points table of the TOC
// database all tracks corresponding to the specified fieldtest.  This
// method instantiates and returns a tracks_group containing the
// extracted tracks.

   tracks_group* retrieve_all_tracks_in_TOC_database(
      gis_database* gis_database_ptr,int selected_fieldtest_ID)
   {
//      cout << "inside mover_func::retrieve_all_tracks_in_TOC_database()"
//           << endl;
      
      vector<int> fieldtest_IDs,mission_IDs,platform_IDs,sensor_IDs;
      vector<string> fieldtest_labels,mission_labels,platform_labels,
         sensor_labels;

      bool just_mission_ID_flag=false;
      mover_func::retrieve_mission_metadata_from_database(
         gis_database_ptr,mission_labels,mission_IDs,
         fieldtest_labels,fieldtest_IDs,platform_labels,platform_IDs,
         just_mission_ID_flag);

// Retrieve ALL sensor IDs from TOC database:

      mover_func::retrieve_sensor_metadata_from_database(
         gis_database_ptr,sensor_labels,sensor_IDs);

// Instantiate group for ALL raw GPS tracks.  Fill their contents with
// data retrieved from track_points table of TOC database:

      tracks_group* tracks_group_ptr=new tracks_group();
      
      for (unsigned int m=0; m<mission_IDs.size(); m++)
      {
//      cout << "m = " << m << endl;
         int curr_mission_ID=mission_IDs[m];
         int fieldtest_ID=fieldtest_IDs[m];
         if (fieldtest_ID != selected_fieldtest_ID) continue;

         for (unsigned int s=0; s<sensor_IDs.size(); s++)
         {
            int curr_sensor_ID=sensor_IDs[s];
      
            bool daylight_savings_flag=true;
            vector<int> trackpoint_ID,fix_quality,n_satellites;
            vector<double> GPS_elapsed_secs,horiz_dilution;
            vector<double> longitude,latitude,altitude,roll,pitch,yaw;

            mover_func::retrieve_track_points_metadata_from_database(
               daylight_savings_flag,gis_database_ptr,
               curr_mission_ID,curr_sensor_ID,
               trackpoint_ID,GPS_elapsed_secs,
               fix_quality,n_satellites,horiz_dilution,
               longitude,latitude,altitude,roll,pitch,yaw);

            if (longitude.size()==0) continue;

// Store curr_mission_ID as raw track's label ID.  Store
// modified GPStrack_ID within raw track's plain ID:

            int GPStrack_ID=1000*curr_mission_ID+curr_sensor_ID;
            track* raw_GPS_track_ptr=tracks_group_ptr->generate_new_track(
               GPStrack_ID);
            raw_GPS_track_ptr->set_label_ID(curr_mission_ID);

            cout << "Generating new track with total ID = " << GPStrack_ID
                 << ", missionID = " << curr_mission_ID
                 << " & sensorID = " << curr_sensor_ID << endl;

            for (unsigned int i=0; i<trackpoint_ID.size(); i++)
            {
               threevector lla_posn(longitude[i],latitude[i],altitude[i]);
               threevector velocity(0,0,0);
               threevector quality(fix_quality[i],n_satellites[i],
               horiz_dilution[i]);
               raw_GPS_track_ptr->set_posn_velocity_GPSquality(
                  GPS_elapsed_secs[i],lla_posn,velocity,quality);
            } // loop over index i labeling waypoints within current track

         } // loop over index s labeling sensor IDs 
      } // loop over index m labeling Mission IDs & raw GPS tracks

//      cout << "tracks_group_ptr->get_n_tracks() = "
//           << tracks_group_ptr->get_n_tracks() << endl;

      return tracks_group_ptr;
   }

// ---------------------------------------------------------------------
// Method UTC_timestamp_to_secs_since_epoch() converts UTC time stamp
// strings retrieved from the track_points table in the TOC_metadata
// database into elapsed secs since reference epoch.

   vector<double> UTC_timestamp_to_secs_since_epoch(
      bool daylight_savings_flag,const vector<string>& time_stamp,
      const vector<double>& longitude,const vector<double>& latitude,
      const vector<double>& altitude)
  {
//      cout << "inside mover_func::UTC_timestamp_to_secs_since_epoch()"
//           << endl;
   
      Clock clock;
      clock.set_daylight_savings_flag(daylight_savings_flag);

      bool UTC_flag=true;
      vector<double> gps_elapsed_secs;

      cout.precision(12);
      for (unsigned int i=0; i<time_stamp.size(); i++)
      {
         gps_elapsed_secs.push_back(clock.timestamp_string_to_elapsed_secs(
            time_stamp[i],UTC_flag));
         geopoint lla(longitude[i],latitude[i],altitude[i]);
//         int UTM_zone_time_offset=
           clock.compute_UTM_zone_time_offset(lla.get_UTM_zonenumber());
      
//      cout << "i = " << i
//           << " UTM_zone_time_offset = " << UTM_zone_time_offset << endl;
//      cout << " gps_elapsed_secs = " << gps_elapsed_secs.back()
//           << " localtime = " << clock.YYYY_MM_DD_H_M_S(" ",":",false)
//           << endl << endl;
      }
      return gps_elapsed_secs;
   }

// ==========================================================================
// Peter's GPS, MIDG and Quad GPS log file parsing
// ==========================================================================

// Method alpha_filter_raw_GPS_data() smooths noisy GPS track with
// simple alpha filter.  We let alpha slowly vary from unity down to
// some fraction less than one.

// Recall alpha=1 implies filtered value = raw noisy input value,
// while alpha=0 implies filtered value = prev filtered value.

   void alpha_filter_raw_GPS_data(
      int curr_index,int starting_index,
      double& prev_filtered_longitude,double& prev_filtered_latitude,
      double raw_longitude,double raw_latitude,
      double& filtered_longitude,double& filtered_latitude)
   {
//      cout << "inside mover_func::alpha_filter_raw_GPS_data()" << endl;

      const double sigma_iter=5;
      double mu_iter=starting_index+5*sigma_iter;
      double arg_iter=(curr_index-mu_iter)/(SQRT_TWO*sigma_iter);
//      cout << "arg_iter = " << arg_iter << endl;
//      cout << "errorfunc::error_function(arg_iter)) = "
//           << mathfunc::error_function(arg_iter) << endl;
      
//      const double min_alpha=0.001;
//      const double min_alpha=0.1;
//      const double min_alpha=0.5;
      const double min_alpha=0.8;
      double alpha=1.0+(min_alpha-1.0)*0.5*(
         1+mathfunc::error_function(arg_iter));
//      cout << "alpha = " << alpha << endl;

      filtered_longitude=filterfunc::alpha_filter(
         raw_longitude,prev_filtered_longitude,alpha);
      filtered_latitude=filterfunc::alpha_filter(
         raw_latitude,prev_filtered_latitude,alpha);
      prev_filtered_longitude=filtered_longitude;
      prev_filtered_latitude=filtered_latitude;

//      cout.precision(12);
//      cout << "raw_lon = " << raw_longitude
//           << " filtered_lon = " << filtered_longitude << endl;
//      cout << "raw_lat = " << raw_latitude
//           << " filtered_lat = " << filtered_latitude << endl;
   }

// ---------------------------------------------------------------------   
// Method parse_GPS_logfile() extracts instantaneous time stamp,
// longitude, latitude, altitude, fix_quality, n_satellites,
// horizontal dilution values from the GPS logfile generated by
// Peter's GPSDEVICE program.  After alpha-filtering the raw
// geocoordinates, this method fills input *gps_track_ptr with GPS
// metadata.

   void parse_GPS_logfile(string logfilename,Clock& clock,
		          track* gps_track_ptr)
   {
//      cout << "inside mover_func::parse_GPS_logfile()" << endl;

      filefunc::ReadInfile(logfilename);

      int istart=0;
      double prev_time=-1;
      double prev_filtered_longitude=0;
      double prev_filtered_latitude=0;
      for (unsigned int i=istart; i<filefunc::text_line.size(); i++)
      {
         string curr_line=filefunc::text_line[i];
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(curr_line);

// GPS log might occasionally contain spurious metadata.  So make
// sure that log file line has at least 10 entries:

         int n_fields=substrings.size();
         if (n_fields < 10) continue;

         int column=0;
         if (n_fields==11) column=1;

         double curr_time=stringfunc::string_to_number(substrings[column++]);
         if (curr_time <= prev_time) continue;
         prev_time=curr_time;

         clock.convert_elapsed_secs_to_date(curr_time);

// Ignore any result with year < 2010;

         int year=clock.get_year();
         if (year < 2010) continue;

         cout.precision(12);
         string separator_char="_";
//         bool display_UTC_time_flag=false;
//         cout << "time = " << clock.YYYY_MM_DD_H_M_S(
//            separator_char,separator_char,display_UTC_time_flag) << endl;

// Search for GPS trackpoint uncertainty metadata:

         int fix_quality=0;
         int n_satellites=0;
         double horizontal_dilution=0;

         if (n_fields >= 10)
         {
            fix_quality=stringfunc::string_to_number(substrings[column++]);
            n_satellites=stringfunc::string_to_number(substrings[column++]);
            horizontal_dilution=stringfunc::string_to_number(
               substrings[column++]);
         }

// Ignore any GPS data whose horizontal dilution exceeds
// max_horiz_dilution.  Such data is highly inaccurate:
      
         if (n_satellites > 0)
         {
            const double max_horiz_dilution=5;
            if (horizontal_dilution > max_horiz_dilution)
            {
               cout << "horizontal_dilution = " << horizontal_dilution << endl;
               continue;
            }
         }

         double raw_longitude=stringfunc::string_to_number(
            substrings[column++]);
         double raw_latitude=stringfunc::string_to_number(
            substrings[column++]);
         double raw_altitude=stringfunc::string_to_number(
            substrings[column++]);

//         cout << "altitude = " << raw_altitude 
//              << " latitude = " << raw_latitude 
//              << " longitude = " << raw_longitude << endl;

         double filtered_longitude,filtered_latitude;
         alpha_filter_raw_GPS_data(
            i,istart,prev_filtered_longitude,prev_filtered_latitude,
            raw_longitude,raw_latitude,
            filtered_longitude,filtered_latitude);

/*
// Smooth noisy GPS track with simple alpha filter.  But let alpha
// slowly vary from unity down to some fraction less than one.
// Recall alpha=1 implies filtered value = raw noisy input value,
// while alpha=0 implies filtered value = prev filtered value.

         double sigma_iter=5;
         double mu_iter=i_flying_start+7*sigma_iter;
         double arg_iter=(i-mu_iter)/(SQRT_TWO*sigma_iter);
//      cout << "arg_iter = " << arg_iter << endl;
//      cout << "errorfunc::error_function(arg_iter)) = "
//           << mathfunc::error_function(arg_iter) << endl;
      
         double alpha=1.0+(0.85-1.0)*0.5*(
            1+mathfunc::error_function(arg_iter));
//      cout << "alpha = " << alpha << endl;

         double filtered_longitude=filterfunc::alpha_filter(
            raw_longitude,prev_filtered_longitude,alpha);
         double filtered_latitude=filterfunc::alpha_filter(
            raw_latitude,prev_filtered_latitude,alpha);
         prev_filtered_longitude=filtered_longitude;
         prev_filtered_latitude=filtered_latitude;
*/

         threevector lla_posn(
            filtered_longitude,filtered_latitude,raw_altitude);
         threevector velocity(0,0,0);
         threevector quality(fix_quality,n_satellites,horizontal_dilution);
         gps_track_ptr->set_posn_velocity_GPSquality(
            curr_time,lla_posn,velocity,quality);
      } // loop over index i labeling lines within  gps log file
   }

// ---------------------------------------------------------------------   
// Method insert_track_points() takes in an already opened GIS
// database along with a GPS track object.  It retrieves an STL vector
// filled with SQL insert commands.  This method then has the GIS
// database execute the insert commands to populate the track_points
// table of the TOC database with GPS track information.

   bool insert_track_points(
      gis_database* gis_database_ptr,track* gps_track_ptr,
      int fieldtest_ID,int mission_ID,int platform_ID,int sensor_ID)
   {
      cout << "inside mover_func::insert_track_points()" << endl;

      vector<string> insert_commands=
         gps_track_ptr->generate_SQL_insert_track_commands(
            fieldtest_ID,mission_ID,platform_ID,sensor_ID);
      
      cout << "insert_commands.size() = " << insert_commands.size() << endl;
      cout << "gis_database_ptr = " << gis_database_ptr << endl;
      for (unsigned int i=0; i<4; i++)
      {
         cout << insert_commands[i] << endl;
      }
      
      gis_database_ptr->set_SQL_commands(insert_commands);
      gis_database_ptr->execute_SQL_commands();

      bool exec_flag=true;
      return exec_flag;
   }

// ---------------------------------------------------------------------   
// Method parse_insparse_output() takes in a 42-column text file 
// generated by INSPARSE.EXE from MIDG binary output.  It extracts the MIDG's
// GPS timestamp, geoposition and geo-orientation.  This MIDG metadata
// is saved into *MIDG_track_ptr.

   void parse_insparse_output(string insparse_output_filename,Clock& clock,
   track* MIDG_track_ptr)
   {
//      cout << "inside mover_func::parse_insparse_output()" << endl;
      
      Ellipsoid_model ellipsoid_model;

      int n_bad_points=0;
      filefunc::ReadInfile(insparse_output_filename);
      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         if (i%1000==0) cout << i/1000 << " " << flush;
         vector<double> curr_row_fields=
            stringfunc::string_to_numbers(filefunc::text_line[i]);

// In early Aug 2010, Darin Marriott indicated that small status
// values indicate the MIDG wasn't reporting valid data:

         const double min_MIDG_status=100;
         double status=curr_row_fields[0];
         if (status < min_MIDG_status) continue;

         double secs_into_GPS_week=0.001*curr_row_fields[30];
         int GPS_week_number=curr_row_fields[31];

         double curr_time=clock.GPS_time_to_elapsed_secs(
            GPS_week_number,secs_into_GPS_week);

         double direction1=0.01*curr_row_fields[33];
         double direction2=0.01*curr_row_fields[34];
         double direction3=0.01*curr_row_fields[35];
         threevector surface_posn(direction1,direction2,direction3);

//         double posn_DOP=0.01*curr_row_fields[40];
         double GPS_posn_error=0.01*curr_row_fields[41];

         const double max_tolerable_GPS_posn_error=15;	// meters

         if (GPS_posn_error > max_tolerable_GPS_posn_error)
         {
            n_bad_points++;
            continue;
         }
      
         int GPS_updated_flag=curr_row_fields[29];
         if (GPS_updated_flag==0) continue;
      
         double raw_longitude,raw_latitude,raw_altitude;
         ellipsoid_model.ConvertXYZToLongLatAlt(
            surface_posn,raw_longitude,raw_latitude,raw_altitude);

         int fix_quality=0;
         int n_satellites=0;
         double horizontal_dilution=0;

         double yaw=0.01*curr_row_fields[9];
         double pitch=0.01*curr_row_fields[10];
         double roll=0.01*curr_row_fields[11];
         rpy curr_rpy(roll,pitch,yaw);

         threevector lla_posn(raw_longitude,raw_latitude,raw_altitude);
         threevector velocity(0,0,0);
         threevector quality(fix_quality,n_satellites,horizontal_dilution);
         MIDG_track_ptr->set_posn_velocity_GPSquality_rpy(
            curr_time,lla_posn,velocity,quality,curr_rpy);
      } // loop over index i labeling lines within MIDG GPS log file
      cout << endl;

      cout << "Number of rejected points = " << n_bad_points << endl;
      cout << "MIDG track size = " << MIDG_track_ptr->size() << endl;
   }

// ---------------------------------------------------------------------   
// Method parse_QuadGPS_logfile() parses the GPS/IMG file generated
// by the Pelican quad-rotor computers.  It fills input
// *Quad_track_ptr with this Quad GPS information.

   void parse_QuadGPS_logfile(string quad_log_filename,
       Clock& clock,track* Quad_track_ptr)
   {
      cout << "inside mover_func::parse_Quad_GPS_log()" << endl;
      
      int n_bad_points=0;
      filefunc::ReadInfile(quad_log_filename);
      string separator_chars=";";

      int istart=0;
      double prev_filtered_longitude,prev_filtered_latitude;
      prev_filtered_longitude=prev_filtered_latitude=0;
      for (unsigned int i=istart; i<filefunc::text_line.size(); i++)
      {
         if (i%1000==0) cout << i/1000 << " " << flush;

// Ignore very first line within Quad GPS log file which contains
// column heading labels rather than actual data:

         if (i==0) continue;

         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               filefunc::text_line[i],separator_chars);

//         for (unsigned int j=0; j<substrings.size(); j++)
//         {
//            cout << "i = " << i << " j = " << j << " substrings[j] = "
//                 << substrings[j] << endl;
//         }
         
         string curr_date=substrings[0];
         vector<string> subsubstrings=
            stringfunc::decompose_string_into_substrings(curr_date,".");
//         for (unsigned int j=0; j<subsubstrings.size(); j++)
//         {
//            cout << "j = " << j << " subsubstrings[j] = "
//                 << subsubstrings[j] << endl;
//         }

         double horiz_dilution=stringfunc::string_to_number(substrings[28]);
//         cout << "horiz_dilution = " << horiz_dilution << endl;

// Ignore any highly inaccurate GPS data whose horizontal dilution
// exceeds max_horiz_dilution:
      
         const double max_horiz_dilution=5;
         if (horiz_dilution > max_horiz_dilution)
         {
            cout << "horizontal_dilution = " << horiz_dilution << endl;
            n_bad_points++;
            continue;
         }
            
         int year=stringfunc::string_to_number(subsubstrings[2]);
         int month=stringfunc::string_to_number(subsubstrings[1]);
         int day=stringfunc::string_to_number(subsubstrings[0]);
//         cout << "year = " << year << " month = " << month
//              << " day = " << day << endl;

// Ignore any result with year < 2010;

         if (year < 2010) continue;

         string curr_time=substrings[1];
         subsubstrings=
            stringfunc::decompose_string_into_substrings(curr_time,":");

//         for (unsigned int j=0; j<subsubstrings.size(); j++)
//         {
//            cout << "j = " << j << " subsubstrings[j] = "
//                 << subsubstrings[j] << endl;
//         }

         int hour=stringfunc::string_to_number(subsubstrings[0]);
         int minutes=stringfunc::string_to_number(subsubstrings[1]);
         double secs=stringfunc::string_to_number(subsubstrings[2]);
//         cout << "hour = " << hour << " mins = " << minutes
//              << " secs = " << secs << endl;

         clock.set_local_time(year,month,day,hour,minutes,secs);

         double raw_latitude=stringfunc::string_to_number(substrings[23]);
         double raw_longitude=stringfunc::string_to_number(substrings[24]);
//         double altitude=stringfunc::string_to_number(substrings[19]);
         double altitude2=stringfunc::string_to_number(substrings[25]);

         double filtered_longitude,filtered_latitude;
         alpha_filter_raw_GPS_data(
            i,istart,prev_filtered_longitude,prev_filtered_latitude,
            raw_longitude,raw_latitude,
            filtered_longitude,filtered_latitude);

//         cout.precision(12);
//         cout << "lon = " << filtered_longitude 
//              << " lat = " << filtered_latitude << endl;
//         cout << " alt = " << altitude << " alt2 = " << altitude2 << endl;

//         double pitch=stringfunc::string_to_number(substrings[3]);
//         double roll=stringfunc::string_to_number(substrings[4]);
//         double yaw=stringfunc::string_to_number(substrings[5]);

//         double pitch3=stringfunc::string_to_number(substrings[6]);
//         double roll3=stringfunc::string_to_number(substrings[7]);
//         double yaw3=stringfunc::string_to_number(substrings[8]);

//	double pitch2=stringfunc::string_to_number(substrings[37]);
//	double roll2=stringfunc::string_to_number(substrings[38]);
//	double yaw2=stringfunc::string_to_number(substrings[39]);

//         cout << "pitch = " << pitch 
//              << " pitch3 = " << pitch3 << endl;
//         cout << "roll = " << roll 
//              << " roll3 = " << roll3 << endl;
//         cout << "yaw = " << yaw 
//              << " yaw3 = " << yaw3 << endl;
//         cout << "-----------------------------------------------" << endl;

         threevector lla_posn(
            filtered_longitude,filtered_latitude,altitude2);
         threevector velocity(0,0,0);

         int fix_quality=1;
         int n_satellites=100;
         threevector quality(fix_quality,n_satellites,horiz_dilution);
         Quad_track_ptr->set_posn_velocity_GPSquality(
            clock.secs_elapsed_since_reference_date(),
            lla_posn,velocity,quality);
      } // loop over index i labeling lines within Quad gps log file
      cout << endl;

      cout << "Number of rejected Quad GPS points = " << n_bad_points << endl;
      cout << "Quad track size = " << Quad_track_ptr->size() << endl;
   }

// ---------------------------------------------------------------------   
// Method parse_DroidGPS_logfile() extracts time stamp,
// longitude, latitude and altitude values from the GPS
// logfile generated by Paul Briemyer's program running on the droid
// phones.  After alpha-filtering the raw geocoordinates, this method
// fills input *gps_track_ptr with GPS metadata.

   void parse_DroidGPS_logfile(string logfilename,Clock& clock,
   				track* gps_track_ptr)
   {
      cout << "inside mover_func::parse_DroidGPS_logfile()" << endl;
      cout << "logfilename = " << logfilename << endl;

      filefunc::ReadInfile(logfilename);

      int istart=1;	// Skip over column labels written to very first line
			// of droid log file
      double prev_time=-1;
      double prev_filtered_longitude=0;
      double prev_filtered_latitude=0;
      for (unsigned int i=istart; i<filefunc::text_line.size(); i++)
      {
         string curr_line=filefunc::text_line[i];
//         cout << curr_line << endl;
         
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(curr_line,",");

/*

As of 8/23/10, Paul Breimyer's droid gps logfile has the following columns
which are separated by commas:

	timestamp,      0
	systemtime,	1
	accuracy,	2
	altitude,	3
	bearing,	4
	latitude,	5
	longitude,	6
	speed,		7
	gpstime,	8
*/

// Droid GPS log occasionally contains spurious metadata.  So make
// sure that log file line has at least 9 entries:

         int n_fields=substrings.size();
         if (n_fields < 9) continue;

         int index=0;
         string timestamp=substrings[index++];
         vector<string> time_substrings=
            stringfunc::decompose_string_into_substrings(timestamp,"_");
         string ymd=time_substrings[0];
         string hms=time_substrings[1];
         string frac_sec=time_substrings[2];

         int year=stringfunc::string_to_number(ymd.substr(0,4));
         int month=stringfunc::string_to_number(ymd.substr(4,2));
         int day=stringfunc::string_to_number(ymd.substr(6,2));
         int hour=stringfunc::string_to_number(hms.substr(0,2));
         int minute=stringfunc::string_to_number(hms.substr(2,2));
         double second=stringfunc::string_to_number(hms.substr(4,2));
         double fractional_sec=0.001*stringfunc::string_to_number(frac_sec);
         second += fractional_sec;

         clock.set_local_time(year,month,day,hour,minute,second);

         double curr_time=clock.secs_elapsed_since_reference_date(); 
         if (curr_time <= prev_time) continue;
         prev_time=curr_time;

//         double systemtime=stringfunc::string_to_number(substrings[index++]);
         double accuracy=stringfunc::string_to_number(substrings[index++]);
         double raw_altitude=stringfunc::string_to_number(substrings[index++]);
         double bearing=stringfunc::string_to_number(substrings[index++]);
         double raw_latitude=stringfunc::string_to_number(substrings[index++]);
         double raw_longitude=stringfunc::string_to_number(
            substrings[index++]);
         double speed=stringfunc::string_to_number(substrings[index++]);
//         double gpstime=stringfunc::string_to_number(substrings[index++]);

// Search for GPS trackpoint uncertainty metadata:

         int fix_quality=0;
         int n_satellites=0;
         double horizontal_dilution=0;

         cout.precision(12);
         cout << "iter = " << i << endl;
         cout << "timestamp = " << timestamp 
              << " ymd = " << ymd
              << " hms = " << hms
              << " frac_sec = " << frac_sec << endl;
         cout << "year = " << year
              << " month = " << month 
              << " day = " << day
              << endl;
         cout << "hour = " << hour
              << " minute = " << minute
              << " second = " << second << endl;
         cout << "time = " << clock.YYYY_MM_DD_H_M_S() << endl;
      
         cout << " accuracy = " << accuracy 
              << " bearing = " << bearing << endl;
         cout << "altitude = " << raw_altitude 
              << " latitude = " << raw_latitude 
              << " longitude = " << raw_longitude << endl;
         cout << "speed = " << speed << endl << endl;

         double filtered_longitude,filtered_latitude;
         alpha_filter_raw_GPS_data(
            i,istart,prev_filtered_longitude,prev_filtered_latitude,
            raw_longitude,raw_latitude,
            filtered_longitude,filtered_latitude);

         threevector lla_posn(
            filtered_longitude,filtered_latitude,raw_altitude);
         threevector velocity(0,0,0);
         threevector quality(fix_quality,n_satellites,horizontal_dilution);
         gps_track_ptr->set_posn_velocity_GPSquality(
            curr_time,lla_posn,velocity,quality);
      } // loop over index i labeling lines within gps log file
   }

// ---------------------------------------------------------------------   
// Method parse_droid_pointing_logfile() extracts 

   void parse_droid_pointing_logfile(string logfilename,Clock& clock,
   				     track* gps_track_ptr)
   {
      cout << "inside mover_func::parse_droid_pointing_logfile()" << endl;
      cout << "logfilename = " << logfilename << endl;
   }
   
// ---------------------------------------------------------------------   
// Method parse_GarminGPS_kmlfile() extracts time stamp,
// longitude, latitude and altitude values from the GPS
// logfile generated by Jan Kansky's GARMIN GPS running on the
// sailplane, After alpha-filtering the raw geocoordinates, this
// method fills input *gps_track_ptr with GPS metadata.

   void parse_GarminGPS_kmlfile(string KML_filename,Clock& clock,
   				track* gps_track_ptr)
   {
      cout << "inside mover_func::parse_GarminGPS_logfile()" << endl;
      cout << "KML filename = " << KML_filename << endl;

      string filename_prefix=stringfunc::prefix(KML_filename);
      string text_filename=filename_prefix+".txt";
      string unix_cmd="gpsbabel -i kml -f "+KML_filename+
         " -o text -F "+text_filename;
//   cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);

      while (!filefunc::fileexist(text_filename))
      {
         sleep(1);
      }
   
      cout << "Input GARMIN KML file converted via GPSBABEL to text file " 
           << text_filename << endl;
      filefunc::ReadInfile(text_filename);

      int FixQual=-1;
      int Nsats=-1;
      double HDOP=-1;
      double Vx=0;
      double Vy=0;
//      double Vz=0;

      string first_timestamp;
      double longitude,latitude,altitude;
//      double heading,speed;
      longitude=latitude=altitude=-1;
      vector<string> output_lines;

      for (unsigned int i=0; i<filefunc::text_line.size(); i++)
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
            filefunc::text_line[i]);
         int n_substrings=substrings.size();
         if (n_substrings > 2)
         {
//         cout << filefunc::text_line[i] << endl;
            if (stringfunc::first_substring_location(
               substrings[0],"Longitude",0) > 0)
            {
               longitude=stringfunc::string_to_number(substrings[1]);
            }
            else if (stringfunc::first_substring_location(
               substrings[0],"Latitude",0) > 0)
            {
               latitude=stringfunc::string_to_number(substrings[1]);
            }
            else if (stringfunc::first_substring_location(
               substrings[0],"Altitude",0) > 0)
            {
               altitude=stringfunc::string_to_number(substrings[1]);
            }
            else if (stringfunc::first_substring_location(
               substrings[0],"Speed",0) > 0)
            {
//               speed=stringfunc::string_to_number(substrings[1]);
            }
            else if (stringfunc::first_substring_location(
               substrings[0],"Heading",0) > 0)
            {
//               heading=stringfunc::string_to_number(substrings[1]);
            }
            else if (stringfunc::first_substring_location(
               substrings[0],"Time",0) > 0)
            {
               string timestamp=substrings[1];
               if (first_timestamp.size()==0)
               {
                  first_timestamp=timestamp;
               }
            
               bool UTC_flag=true;
               double elapsed_secs=clock.timestamp_string_to_elapsed_secs(
                  timestamp,UTC_flag);
            
//            cout.precision(10);
//            cout << endl;
//            cout << "lon = " << longitude
//                 << " lat = " << latitude
//                 << " alt = " << altitude << endl;
//            cout << "speed = " << speed 
//                 << " heading = " << heading
//                 << " timestamp = " << timestamp 
               //                << " elapsed secs = " << elapsed_secs << endl;

               output_lines.push_back(
                  stringfunc::number_to_string(elapsed_secs) + " "
                  +stringfunc::number_to_string(FixQual) + " "
                  +stringfunc::number_to_string(Nsats) + " "
                  +stringfunc::number_to_string(HDOP) + " "
                  +stringfunc::number_to_string(longitude,8) + " "
                  +stringfunc::number_to_string(latitude,8) + " "
                  +stringfunc::number_to_string(altitude) + " "
                  +stringfunc::number_to_string(Vx) + " "
                  +stringfunc::number_to_string(Vy) + " "
                  +stringfunc::number_to_string(Vx));
            }
         }
      } // loop over index i labeling lines within text file converted from KML

// Write out GPS track file in same convention as that generated by
// GPSDEVICE program:

      string output_subdir=filefunc::getdirname(text_filename);
      string logfilename=output_subdir+"KML_gps_track_"+first_timestamp+".dat";

      ofstream outstream;
      filefunc::openfile(logfilename,outstream);
      outstream << "# Time FixQual Nsats HDOP    X	Y	Z	Vx	Vy 	Vz" << endl;
      outstream << endl;
      for (unsigned int i=0; i<output_lines.size(); i++)
      {
         outstream << output_lines[i] << endl;
      }
      filefunc::closefile(logfilename,outstream);

      string banner="GPS KML file converted to "+logfilename;
      outputfunc::write_big_banner(banner);

      parse_GPS_logfile(logfilename,clock,gps_track_ptr);
   }
   
// ---------------------------------------------------------------------   
// Method generate_GPScamera_track() takes in time stamp, longitude,
// latitude and altitude values from the GPS camera's EXIF tags.
// After alpha-filtering the raw geocoordinates, this method fills
// input *gps_track_ptr with GPS metadata.

   void generate_GPScamera_track(
      vector<double>& secs_elapsed,vector<geopoint>& geolocations,
      Clock& clock,std::string output_subdir,track* gps_track_ptr)
   {
      cout << "inside mover_func::generate_GPScamera_track()" << endl;
      cout << "secs_elapsed.size() = " << secs_elapsed.size()
           << " geolocations.size() = " << geolocations.size() << endl;

      int FixQual=-1;
      int Nsats=-1;
      double HDOP=-1;
      double Vx=0;
      double Vy=0;
//      double Vz=0;

      vector<string> output_lines;
      for (unsigned int i=0; i<geolocations.size(); i++)
      {
         double elapsed_secs=secs_elapsed[i];
         double longitude=geolocations[i].get_longitude();
         double latitude=geolocations[i].get_latitude();
         double altitude=geolocations[i].get_altitude();

//            cout.precision(10);
//            cout << endl;
//            cout << "lon = " << longitude
//                 << " lat = " << latitude
//                 << " alt = " << altitude << endl;
//                 << " elapsed secs = " << elapsed_secs << endl;

         output_lines.push_back(
            stringfunc::number_to_string(elapsed_secs) + " "
            +stringfunc::number_to_string(FixQual) + " "
            +stringfunc::number_to_string(Nsats) + " "
            +stringfunc::number_to_string(HDOP) + " "
            +stringfunc::number_to_string(longitude,8) + " "
            +stringfunc::number_to_string(latitude,8) + " "
            +stringfunc::number_to_string(altitude) + " "
            +stringfunc::number_to_string(Vx) + " "
            +stringfunc::number_to_string(Vy) + " "
            +stringfunc::number_to_string(Vx));
      } // loop over index i 

// Write out GPS track file in same convention as that generated by
// GPSDEVICE program:

      string logfilename=output_subdir+"GPScam_track.dat";

      ofstream outstream;
      filefunc::openfile(logfilename,outstream);
      outstream << "Time FixQual Nsats HDOP    X	Y	Z	Vx	Vy 	Vz" << endl;
      outstream << endl;
      for (unsigned int i=0; i<output_lines.size(); i++)
      {
         outstream << output_lines[i] << endl;
      }
      filefunc::closefile(logfilename,outstream);

      string banner="GPS camera metadata written to "+logfilename;
      outputfunc::write_big_banner(banner);

      parse_GPS_logfile(logfilename,clock,gps_track_ptr);
   }
      
} // mover_func namespace



