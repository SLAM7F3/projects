// ==========================================================================
// Header file for cppJSON class
// ==========================================================================
// Last modified on 4/11/12; 4/12/12; 5/27/14; 1/18/16
// ==========================================================================

#ifndef CPPJSON_H
#define CPPJSON_H

#include <map>
#include <vector>
#include "graphs/cJSON.h"

class cppJSON
{

  public:

   typedef std::pair<std::string,std::string> KEY_VALUE_PAIR;
   typedef std::vector<std::vector<KEY_VALUE_PAIR> > KEY_VALUE_PAIRS;

   typedef std::map<int, cJSON*> NODE_IDS_MAP;
// Independent int: node ID
// Dependent CJSON pointer: node pointer

   cppJSON();
   cppJSON(const cppJSON& c);
   ~cppJSON();
   cppJSON& operator= (const cppJSON& c);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const cppJSON& c);

// Set and get methods:

   cJSON* get_root_ptr();
   const cJSON* get_root_ptr() const;
   cJSON* get_next_node_ptr(cJSON* node_ptr);
   const cJSON* get_next_node_ptr(cJSON* node_ptr) const;
   cJSON* get_child_node_ptr(cJSON* node_ptr);
   const cJSON* get_child_node_ptr(cJSON* node_ptr) const;
   cJSON* get_parent_node_ptr(cJSON* node_ptr);
   const cJSON* get_parent_node_ptr(cJSON* node_ptr) const;

   std::string get_name(cJSON* node_ptr) const;
   NODE_IDS_MAP& get_node_ids_map();
   const NODE_IDS_MAP& get_node_ids_map() const;

   unsigned int get_n_nodes() const;

// Tree generation and traversing member functions:

//    std::string get_JSON_string_from_JSON_file(std::string json_filename);
   cJSON* parse_json(std::string json_string);
   void print_node(cJSON* node_ptr);
   int get_node_type(cJSON* node_ptr);

   void generate_JSON_tree();
   void generate_JSON_tree(cJSON* node_ptr);

   KEY_VALUE_PAIRS* extract_key_value_pairs(cJSON* node_ptr);

// Object key-value pair parsing member functions:

   int get_n_objects() const;
   std::vector<KEY_VALUE_PAIR>& get_object_key_value_pairs(int n);

  protected:

  private: 
   
   enum JSON_node_type 
   {
      cJSON_false=0, cJSON_true=1, cJSON_null=2, cJSON_number=3,
      cJSON_string=4, cJSON_array=5, cJSON_object=6
   };

   cJSON *root_ptr,*next_node_ptr,*child_node_ptr,*parent_node_ptr;
   unsigned int node_counter;

   NODE_IDS_MAP node_ids_map;

   std::vector<KEY_VALUE_PAIR> curr_key_value_pairs;
   KEY_VALUE_PAIRS* key_value_pairs_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const cppJSON& c);

   void assign_node_ID(cJSON* node_ptr);
   void retrieve_subtree_key_value_pairs(int start_level, cJSON* node_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline cJSON* cppJSON::get_root_ptr()
{
   return root_ptr;
}

inline const cJSON* cppJSON::get_root_ptr() const
{
   return root_ptr;
}

inline cJSON* cppJSON::get_next_node_ptr(cJSON* node_ptr) 
{
   return node_ptr->next;
}

inline const cJSON* cppJSON::get_next_node_ptr(cJSON* node_ptr) const
{
   return node_ptr->next;
}

inline cJSON* cppJSON::get_child_node_ptr(cJSON* node_ptr)
{
   return node_ptr->child;
}

inline const cJSON* cppJSON::get_child_node_ptr(cJSON* node_ptr) const
{
   return node_ptr->child;
}

inline cJSON* cppJSON::get_parent_node_ptr(cJSON* node_ptr)
{
   return node_ptr->parent;
}

inline const cJSON* cppJSON::get_parent_node_ptr(cJSON* node_ptr) const
{
   return node_ptr->parent;
}


inline std::string cppJSON::get_name(cJSON* node_ptr) const
{
   std::string name_string;
   char* string_ptr=node_ptr->string;

   if (string_ptr != NULL)
   {
      name_string=std::string(string_ptr);
   }
   return name_string;
}

inline cppJSON::NODE_IDS_MAP& cppJSON::get_node_ids_map()
{
  return node_ids_map;
}

inline const cppJSON::NODE_IDS_MAP& cppJSON::get_node_ids_map() const
{
  return node_ids_map;
}


inline unsigned int cppJSON::get_n_nodes() const
{
  return node_counter;
}


#endif  // cppJSON.h
