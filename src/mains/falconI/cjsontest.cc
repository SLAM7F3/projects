// ==========================================================================
// Program CJSONTEST
// ==========================================================================
// Last updated on 4/6/12; 4/10/12; 4/12/12; 5/27/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "graphs/cJSON.h"
#include "graphs/cppJSON.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;


int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);

//   string json_filename="json_demo.txt";
//   string json_filename="json2.txt";
//   string json_filename="test2.json";
   string json_filename="disks.json";
   filefunc::ReadInfile(json_filename);
   string json_string;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      json_string += filefunc::text_line[i];
   }
//   cout << "json_string = " << json_string << endl;



   json_string="[ ";
   json_string += "{\"num_files\":3,\"path\":\"/mnt/sortie14\",\"id\":\"558d110\"}, ";
   json_string += "{\"num_files\":2,\"path\":\"/media/alirt\",\"id\":\"cc2b03c\"} ";
   json_string += "]";

   cout << "json_string = " << json_string << endl;

   cppJSON* cppJSON_ptr=new cppJSON();

   cJSON* root_ptr=cppJSON_ptr->parse_json(json_string);
   cppJSON_ptr->generate_JSON_tree();
   cppJSON_ptr->extract_key_value_pairs(root_ptr);

   vector<int> num_files;
   vector<string> paths,ids;

   int n_JSON_objects=cppJSON_ptr->get_n_objects();
   cout << "Number JSON objects = " << n_JSON_objects << endl;
   for (int n=0; n<n_JSON_objects; n++)
   {
      vector<cppJSON::KEY_VALUE_PAIR> key_value_pairs=
         cppJSON_ptr->get_object_key_value_pairs(n);
      for (unsigned int k=0; k<key_value_pairs.size(); k++)
      {
         cppJSON::KEY_VALUE_PAIR curr_key_value_pair=key_value_pairs[k];
//         cout << "k = " << k 
//              << " key = " << curr_key_value_pair.first
//              << " value = " << curr_key_value_pair.second << endl;

         string key=curr_key_value_pair.first;
         string value=curr_key_value_pair.second;
         if (key=="num_files")
         {
            num_files.push_back(stringfunc::string_to_number(value));
         }
         else if (key=="path")
         {
            paths.push_back(value);
         }
         else if (key=="id")
         {
            ids.push_back(value);
         }
      } // loop over index k
   } // loop over index n 

   for (unsigned int i=0; i<num_files.size(); i++)
   {
      cout << "i = " << i
           << " num_files = " << num_files[i]
           << " path = " << paths[i]
           << " id = " << ids[i] << endl;
   }
   
   
/*
   cJSON* format = cJSON_GetObjectItem(root,"format");
   int framerate = cJSON_GetObjectItem(format,"frame rate")->valueint;
   cout << "framerate = " << framerate << endl;

   cJSON_GetObjectItem(format,"frame rate")->valueint=25;
   int framerate2 = cJSON_GetObjectItem(format,"frame rate")->valueint;
   cout << "framerate2 = " << framerate2 << endl;

   char* rendered=cJSON_Print(root);

   string new_json_string(rendered);
   cout << "new_json_string = " << new_json_string << endl;
*/

} 

