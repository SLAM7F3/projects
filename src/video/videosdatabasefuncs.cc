// ==========================================================================
// Videosdatabasefuncs namespace method definitions
// ==========================================================================
// Last modified on 10/31/13; 11/1/13; 11/4/13
// ==========================================================================

#include <iostream>
#include "postgres/gis_database.h"
#include "general/stringfuncs.h"
#include "video/videosdatabasefuncs.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::map;
using std::string;
using std::vector;

namespace videosdatabasefunc
{

// ==========================================================================
// Videos database insertion methods
// ==========================================================================

// Method insert_video_metadata() takes in metadata for a new entry
// within the videos table of the IMAGERY database.  It
// inserts this metadata into *gis_database_ptr.

   bool insert_video_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int mission_ID,int image_ID,
      int clip_ID,int frame_ID,string video_URL,string transcript_URL)
   {
// cout << "inside imagesdatabasefunc::insert_video_metadata()" << endl;

      string SQL_cmd=
         videosdatabasefunc::generate_insert_video_metadata_SQL_command(
            campaign_ID,mission_ID,image_ID,clip_ID,frame_ID,
            video_URL,transcript_URL);
      cout << SQL_cmd << endl;
    
      vector<string> insert_commands;
      insert_commands.push_back(SQL_cmd);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }

// ---------------------------------------------------------------------   
   string generate_insert_video_metadata_SQL_command(
      int campaign_ID,int mission_ID,int image_ID,
      int clip_ID,int frame_ID,string video_URL,string transcript_URL)
   {
//   cout << "inside imagesdatabasefunc::generate_insert_video_metadata_SQL_command()" << endl;

      string SQL_command="insert into videos ";
      SQL_command += 
         "(campaign_ID,mission_ID,image_ID,clip_ID,frame_ID,video_URL,transcript_URL) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(campaign_ID)+",";
      SQL_command += stringfunc::number_to_string(mission_ID)+",";
      SQL_command += stringfunc::number_to_string(image_ID)+",";
      SQL_command += stringfunc::number_to_string(clip_ID)+",";
      SQL_command += stringfunc::number_to_string(frame_ID)+",";
      SQL_command += "'"+video_URL+"',";
      SQL_command += "'"+transcript_URL+"'";
      SQL_command += ");";

      return SQL_command;
   }

// ---------------------------------------------------------------------   
// Method update_keyframe_ID() 

   bool update_keyframe_ID(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,int keyframe_ID)
   {
//      cout << "inside videosdatabasefunc::update_keyframe_ID()" << endl;
//      cout << "gis_database_ptr = " << gis_database_ptr << endl;

      string SQL_command=generate_update_keyframe_SQL_command(
         campaign_ID,mission_ID,image_ID,keyframe_ID);

      vector<string> update_commands;
      update_commands.push_back(SQL_command);
      gis_database_ptr->set_SQL_commands(update_commands);
      bool flag=gis_database_ptr->execute_SQL_commands();

      return flag;
   }

// ---------------------------------------------------------------------   
// Method generate_update_keyframe_SQL_command()

   string generate_update_keyframe_SQL_command(
      int campaign_ID,int mission_ID,int image_ID,int keyframe_ID)
   {
//      cout << "inside videosdatabasefunc::generate_update_keyframe_SQL_command()" << endl;

      string SQL_command="update videos ";
      SQL_command += "SET keyframe_ID="
         +stringfunc::number_to_string(keyframe_ID);
      SQL_command += " WHERE image_id="+stringfunc::number_to_string(
         image_ID);
      SQL_command += " AND campaign_ID="+stringfunc::number_to_string(
         campaign_ID);
      SQL_command += " AND mission_ID="+stringfunc::number_to_string(
         mission_ID)+";";
      return SQL_command;
   }

// ==========================================================================
// Videos database retrieval methods
// ==========================================================================

   void get_clip_and_frame_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int image_ID,int& clip_ID,int& frame_ID)
   {
//      cout << "inside videosdatabasefunc::get_clip_and_frame_IDs()" << endl;
      
      string SQL_cmd="SELECT clip_ID,frame_ID FROM videos ";
      SQL_cmd += "WHERE campaign_id="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_id="+
         stringfunc::number_to_string(mission_ID);
      SQL_cmd += " AND image_id="+
         stringfunc::number_to_string(image_ID)+";";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      clip_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      frame_ID=stringfunc::string_to_number(field_array_ptr->get(0,1));
   }


// ---------------------------------------------------------------------   
// Method get_image_ID() takes in an video image's campaign and
// mission IDs along with its clip and frame IDs.  It returns the
// corresponding unique image ID from the videos table in the IMAGERY
// database.
 
   int get_image_ID(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int clip_ID,int frame_ID)
   {
//      cout << "inside videosdatabasefunc::get_image_ID()" << endl;
      
      string SQL_cmd="SELECT image_id FROM videos ";
      SQL_cmd += "WHERE campaign_id="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_id="+
         stringfunc::number_to_string(mission_ID);
      SQL_cmd += " AND clip_id="+
         stringfunc::number_to_string(clip_ID);
      SQL_cmd += " AND frame_id="+
         stringfunc::number_to_string(frame_ID)+";";
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
// Method get_image_and_keyframe_IDs() takes in an video image's
// campaign and mission IDs along with its clip and frame IDs.  It
// returns the corresponding unique image ID and keyframe ID from the
// videos table in the IMAGERY database.
 
   void get_image_and_keyframe_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      int clip_ID,int frame_ID,int& image_ID,int& keyframe_ID)
   {
//      cout << "inside videosdatabasefunc::get_image_and_keyframe_IDs()" << endl;
      
      string SQL_cmd="SELECT image_id,keyframe_ID FROM videos ";
      SQL_cmd += "WHERE campaign_id="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_id="+
         stringfunc::number_to_string(mission_ID);
      SQL_cmd += " AND clip_id="+
         stringfunc::number_to_string(clip_ID);
      SQL_cmd += " AND frame_id="+
         stringfunc::number_to_string(frame_ID)+";";
//      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);

      image_ID=keyframe_ID=-1;
      if (field_array_ptr==NULL) return;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_arraay_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      image_ID=stringfunc::string_to_number(field_array_ptr->get(0,0));
      keyframe_ID=stringfunc::string_to_number(field_array_ptr->get(0,1));
//      cout << "image_ID = "  << image_ID << endl;
//      outputfunc::enter_continue_char();
   }

// ---------------------------------------------------------------------   
// Method get_keyframe_IDs() interrogates the videos table within the
// IMAGERY database for entries which have non-null valued keyframe
// IDs.  It returns the keyframe IDs along with their corresponding
// image IDs in STL vectors.

   bool get_keyframe_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      vector<int>& image_IDs,vector<int>& keyframe_IDs)
   {
//      cout << "inside videosdatabasefunc::get_keyframe_IDs()" << endl;
      
      string SQL_cmd="SELECT image_id,keyframe_id FROM videos ";
      SQL_cmd += "WHERE campaign_id="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_id="+
         stringfunc::number_to_string(mission_ID)+";";
      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int image_ID=
            stringfunc::string_to_number(field_array_ptr->get(m,0));

         int keyframe_ID=-1;
         string keyframe_str=field_array_ptr->get(m,1);
//         cout << "keyframe_str = " << keyframe_str
//              << " keyframe_str.size() = " << keyframe_str.size() << endl;
//         outputfunc::enter_continue_char();
         if (keyframe_str != "NULL")
         {
            keyframe_ID=stringfunc::string_to_number(keyframe_str);
            image_IDs.push_back(image_ID);
            keyframe_IDs.push_back(keyframe_ID);
         }
      } // loop over index m labeling rows in *field_array_ptr
      
      return true;
   }

// ---------------------------------------------------------------------   
// This overloaded version get_keyframe_IDs() fills and returns an STL
// map whose independent variable equals keyframe ID and dependent
// variable holds an STL vector of associated image IDs.

   map<int,vector<int> >* get_keyframe_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID)
   {
//      cout << "inside videosdatabasefunc::get_keyframe_IDs()" << endl;
      
      vector<int> image_IDs,keyframe_IDs;
      if (!get_keyframe_IDs(
         gis_database_ptr,campaign_ID,mission_ID,image_IDs,keyframe_IDs))
      {
         return NULL;
      }

      typedef map<int,vector<int> > KEYFRAME_IMAGE_IDS_MAP;
      KEYFRAME_IMAGE_IDS_MAP* keyframe_image_IDs_map_ptr=new
         KEYFRAME_IMAGE_IDS_MAP;
      KEYFRAME_IMAGE_IDS_MAP::iterator iter;
      
      for (unsigned int k=0; k<keyframe_IDs.size(); k++)
      {
         int keyframe_ID=keyframe_IDs[k];
         int image_ID=image_IDs[k];

         iter=keyframe_image_IDs_map_ptr->find(keyframe_ID);
         if (iter==keyframe_image_IDs_map_ptr->end())
         {
            vector<int> V;
            V.push_back(image_ID);
            (*keyframe_image_IDs_map_ptr)[keyframe_ID]=V;
         }
         else
         {
            iter->second.push_back(image_ID);
         }
      } // loop over index k labeling keyframe IDs
      
      return keyframe_image_IDs_map_ptr;
   }
   

// ---------------------------------------------------------------------   
// Method get_keyframe_and_data_IDs() interrogates the videos table
// within the IMAGERY database for entries which have non-null valued
// keyframe IDs.  It returns the keyframe IDs along with their
// corresponding image primary keys in STL vectors.

   bool get_keyframe_and_data_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      vector<int>& keyframe_IDs,vector<int>& data_IDs)
   {
      cout << "inside videosdatabasefunc::get_keyframe_and_data_IDs()" << endl;
      
      string SQL_cmd="SELECT keyframe_id,images.id FROM videos ";
      SQL_cmd += "INNER JOIN images ";
      SQL_cmd += "ON images.image_id=videos.image_id ";
      SQL_cmd += "WHERE images.campaign_id="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND images.mission_id="+
         stringfunc::number_to_string(mission_ID)+";";
      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int keyframe_ID=-1;
         string keyframe_str=field_array_ptr->get(m,0);
         if (keyframe_str == "NULL") continue;

         keyframe_ID=stringfunc::string_to_number(keyframe_str);
         keyframe_IDs.push_back(keyframe_ID);

         int data_ID=stringfunc::string_to_number(field_array_ptr->get(m,1));
         data_IDs.push_back(data_ID);
      } // loop over index m labeling rows in *field_array_ptr
      
      return true;
   }

// ---------------------------------------------------------------------   
// Method get_keyframe_and_node_IDs() interrogates the videos table
// within the IMAGERY database for entries which have non-null valued
// keyframe IDs.  It returns the keyframe IDs along with the IDs for
// the corresponding level-0 nodes in STL vectors.

   bool get_keyframe_and_node_IDs(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int campaign_ID,int mission_ID,
      vector<int>& keyframe_IDs,vector<int>& node_IDs)
   {
      cout << "inside videosdatabasefunc::get_keyframe_and_node_IDs()" << endl;
      
      string SQL_cmd=
         "SELECT nodes.node_id,images.id,images.image_id,videos.image_id,videos.keyframe_id ";
      SQL_cmd += "FROM nodes INNER JOIN images ";
      SQL_cmd += "ON nodes.data_id=images.id ";
      SQL_cmd += "INNER JOIN videos ";
      SQL_cmd += "ON images.image_id=videos.image_id ";
      SQL_cmd += "WHERE nodes.graph_hierarchy_id="+
         stringfunc::number_to_string(graph_hierarchy_ID);
      SQL_cmd += " AND images.campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND images.mission_ID="+
         stringfunc::number_to_string(mission_ID);
      SQL_cmd += " AND videos.campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND videos.mission_ID="+
         stringfunc::number_to_string(mission_ID);
      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
//      cout << "mdim = " << field_array_ptr->get_mdim()
//           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int keyframe_ID=-1;
         string keyframe_str=field_array_ptr->get(m,4);
//         cout << "keyframe_str = " << keyframe_str << endl;
         if (keyframe_str == "NULL") continue;

//         int imageID=stringfunc::string_to_number(field_array_ptr->get(m,3));

         keyframe_ID=stringfunc::string_to_number(keyframe_str);
         keyframe_IDs.push_back(keyframe_ID);

         int node_ID=stringfunc::string_to_number(field_array_ptr->get(m,0));

//         cout << "imageID = " << imageID
//              << " node_ID = " << node_ID
//              << " keyframe_ID = " << keyframe_ID << endl;

         node_IDs.push_back(node_ID);
      } // loop over index m labeling rows in *field_array_ptr

//      cout << "keyframe_IDs.size() = " << keyframe_IDs.size() << endl;
//      cout << "node_IDs.size() = " << node_IDs.size() << endl;
      
      return true;
   }

// ---------------------------------------------------------------------   
// Method get_distinct_keyframe_IDs() interrogates the videos table
// within the IMAGERY database for entries which have non-null valued
// keyframe IDs.  It returns the keyframe IDs in an STL vector.

   bool get_distinct_keyframe_IDs(
      gis_database* gis_database_ptr,int campaign_ID,int mission_ID,
      vector<int>& keyframe_IDs)
   {
      cout << "inside videosdatabasefunc::get_distinct_keyframe_IDs()" << endl;
      
      string SQL_cmd="SELECT DISTINCT keyframe_id FROM videos";
      SQL_cmd += " WHERE campaign_ID="+
         stringfunc::number_to_string(campaign_ID);
      SQL_cmd += " AND mission_ID="+
         stringfunc::number_to_string(mission_ID)+";";
      cout << "SQL_cmd = " << SQL_cmd << endl;

      Genarray<string>* field_array_ptr=gis_database_ptr->select_data(SQL_cmd);
      if (field_array_ptr==NULL) return false;

//      cout << "Field_array_ptr = " << field_array_ptr << endl;
//      cout << "*field_array_ptr = " << *field_array_ptr << endl;
      cout << "mdim = " << field_array_ptr->get_mdim()
           << " ndim = " << field_array_ptr->get_ndim() << endl;

      for (unsigned int m=0; m<field_array_ptr->get_mdim(); m++)
      {
         int keyframe_ID=-1;
         string keyframe_str=field_array_ptr->get(m,0);
//         cout << "keyframe_str = " << keyframe_str << endl;
         if (keyframe_str == "NULL") continue;
         keyframe_ID=stringfunc::string_to_number(keyframe_str);
         keyframe_IDs.push_back(keyframe_ID);
      } // loop over index m labeling rows in *field_array_ptr

      cout << "keyframe_IDs.size() = " << keyframe_IDs.size() << endl;
      
      return true;
   }

// ---------------------------------------------------------------------   
// Method retrieve_all_video_keyframes() instantiates and fills an
// ATTRIBUTES_METADATA_MAP with node IDs corresponding to video
// keyframe image IDs.

   imagesdatabasefunc::ATTRIBUTES_METADATA_MAP* retrieve_all_video_keyframes(
      gis_database* gis_database_ptr,int graph_hierarchy_ID,
      int campaign_ID,int mission_ID)
   {
//      cout << "inside videosdatabasefunc::retrieve_all_video_keyframes()" 
//           << endl;
//      cout << "graph_hierarchy_ID = " << graph_hierarchy_ID << endl;

      imagesdatabasefunc::ATTRIBUTES_METADATA_MAP* attributes_metadata_map_ptr=
         new imagesdatabasefunc::ATTRIBUTES_METADATA_MAP;

      vector<int> keyframe_IDs,node_IDs;
      if (videosdatabasefunc::get_keyframe_and_node_IDs(
         gis_database_ptr,graph_hierarchy_ID,campaign_ID,mission_ID,
         keyframe_IDs,node_IDs))
      {
//         cout << "keyframe_IDs.size() = " << keyframe_IDs.size() << endl;
//         cout << "node_IDs.size() = " << node_IDs.size() << endl;

         string key,value;
         vector<imagesdatabasefunc::STRING_PAIR> attribute_pairs;
         for (unsigned int i=0; i<node_IDs.size(); i++)
         {
            attribute_pairs.clear();

            key="video_keyframes";
            value=stringfunc::number_to_string(keyframe_IDs[i]);

            imagesdatabasefunc::STRING_PAIR curr_pair(key,value);
            attribute_pairs.push_back(curr_pair);

            (*attributes_metadata_map_ptr)[node_IDs[i]]=attribute_pairs;
         }
      }
//      cout << "attribute_metadata_map_ptr->size() = "
//           << attributes_metadata_map_ptr->size() << endl;

      return attributes_metadata_map_ptr;
   }


} // videosdatabasefunc namespace
