// =========================================================================
// CPPJSON class member function definitions
// =========================================================================
// Last modified on 2/20/13; 4/5/14; 5/27/14; 1/18/16
// =========================================================================

#include <iostream>
#include <string>

#include "general/filefuncs.h"
#include "graphs/cppJSON.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void cppJSON::allocate_member_objects()
{
}

void cppJSON::initialize_member_objects()
{
   root_ptr=NULL;
   next_node_ptr=NULL;
   child_node_ptr=NULL;

   key_value_pairs_ptr=NULL;
   node_counter=0;
}		 

// ---------------------------------------------------------------------
cppJSON::cppJSON()
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

cppJSON::cppJSON(const cppJSON& c)
{
   docopy(c);
}

cppJSON::~cppJSON()
{
   cJSON_Delete(root_ptr);
   delete key_value_pairs_ptr;
}

// ---------------------------------------------------------------------
void cppJSON::docopy(const cppJSON& c)
{
}

// Overload = operator:

cppJSON& cppJSON::operator= (const cppJSON& c)
{
   if (this==&c) return *this;
   docopy(c);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const cppJSON& c)
{
   outstream << endl;
   return outstream;
}

// =========================================================================
// Tree generation & traversing member functions
// =========================================================================

// Member function assign_node_ID() takes in a cJSON node.  If the
// input node isn't NULL and doesn't already have an ID, this
// method assigns it the next available ID.

void cppJSON::assign_node_ID(cJSON* node_ptr)
{
   if (node_ptr != NULL)
   {
      if (node_ptr->ID == -1)
      {
         node_ptr->ID = node_counter++;
      }
   }
}

// ---------------------------------------------------------------------   
// Member function get_JSON_string_from_JSON_file()

string cppJSON::get_JSON_string_from_JSON_file(string json_filename)
{
   filefunc::ReadInfile(json_filename);
   string json_string;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      json_string += filefunc::text_line[i];
   }
//   cout << "json_string = " << json_string << endl;
   return json_string;
}

// ---------------------------------------------------------------------
cJSON* cppJSON::parse_json(string json_string)
{
//   cout << "inside cppJSON::parse_json()" << endl;
   cout << "Parsing input JSON string" << endl;
   root_ptr=cJSON_Parse(json_string.c_str());
   root_ptr->ID = node_counter++;
   root_ptr->level = 0;

   return root_ptr;
}

// ---------------------------------------------------------------------
void cppJSON::print_node(cJSON* node_ptr)
{
   cout << endl;
   cout << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << endl << endl;
   cout << "inside cppJSON::print_node()" << endl;
   cout << "node_ID = " << node_ptr->ID << endl;
   cout << "node_level = " << node_ptr->level << endl;
   cout << "node_ptr = " << node_ptr << endl;
   cout << "node name = " << get_name(node_ptr) << endl;
   cout << "node = " << string(cJSON_Print(node_ptr)) << endl;
//   cout << "node_ptr = " << node_ptr << endl;

   int node_type=get_node_type(node_ptr);
   if (node_type==0)
   {
      cout << "node type = FALSE" << endl;
   }
   else if (node_type==1)
   {
      cout << "node type = TRUE" << endl;
   }
   else if (node_type==2)
   {
      cout << "node type = NULL" << endl;
   }
   else if (node_type==3)
   {
      cout << "node type = NUMBER" << endl;
   }
   else if (node_type==4)
   {
      cout << "node type = STRING" << endl;
   }
   else if (node_type==5)
   {
      cout << "node type = ARRAY" << endl;
   }
   else // if (node_type==6)
   {
      cout << "node type = OBJECT" << endl;
   }

//   cout << "next_node_ptr = " << get_next_node_ptr(node_ptr) << endl;
//   cout << "child_node_ptr = " << get_child_node_ptr(node_ptr) << endl;
//   cout << "parent_node_ptr = " << get_parent_node_ptr(node_ptr) << endl;
//   cout << "at end of cppJSON::print_node()" << endl;
   cout << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << endl << endl;
}

// ---------------------------------------------------------------------
int cppJSON::get_node_type(cJSON* node_ptr)
{
   int node_type=node_ptr->type;
   if (node_type==0)
   {
      return cJSON_false;
   }
   else if (node_type==1)
   {
      return cJSON_true;
   }
   else if (node_type==2)
   {
      return cJSON_null;
   }
   else if (node_type==3)
   {
      return cJSON_number;
   }
   else if (node_type==4)
   {
      return cJSON_string;
   }
   else if (node_type==5)
   {
      return cJSON_array;
   }
   else // if (node_type==6)
   {
      return cJSON_object;
   }
}

// ---------------------------------------------------------------------
// Member function generate_JSON_tree() recursively generates the N-tree
// from the input JSON string.  Each new node added to the JSON tree
// is assigned a unique ID, and its level relative to the starting
// node is recorded.  Children nodes are also assigned pointers to
// their parents.  

void cppJSON::generate_JSON_tree()
{
   cout << "Generating JSON tree starting from root node" << endl;
   generate_JSON_tree(root_ptr);
}

void cppJSON::generate_JSON_tree(cJSON* node_ptr)
{
//   cout << "inside cppJSON::generate_JSON_tree()" << endl;

   while (node_ptr != NULL)
   {
//      cout << "node ID = " << node_ptr->ID
//           << " level = " << node_ptr->level << endl;
      
// Search for possible child node:
      
      cJSON* child_node_ptr=get_child_node_ptr(node_ptr);
      if (child_node_ptr != NULL) 
      {
         assign_node_ID(child_node_ptr);
         child_node_ptr->level = node_ptr->level+1;
	 child_node_ptr->parent = node_ptr;
         generate_JSON_tree(child_node_ptr);
      }

// Move to next sibling node:

      cJSON* next_node_ptr=get_next_node_ptr(node_ptr);
      if (next_node_ptr != NULL)
      {
         assign_node_ID(next_node_ptr);
         next_node_ptr->level = node_ptr->level;
      }

      node_ids_map[node_ptr->ID] = node_ptr;
      
      node_ptr=next_node_ptr;

   } // while node_ptr != NULL 
}

// ---------------------------------------------------------------------
// Member function retrieve_subtree_key_value_pairs() assembles all
// key-value pairs associated with all nodes within the JSON subtree
// defined by input node_ptr within member STL vector
// *key_value_pairs_ptr.  Member STL vectors curr_key_value_pairs and
// *key_value_pairs_ptr must be cleared before this recursive method
// is called!

void cppJSON::retrieve_subtree_key_value_pairs(
   int start_level, cJSON* node_ptr)
{
   string curr_key, curr_value;
   
   while (node_ptr != NULL)
   {
//      cout << "node_ID = " << node_ptr->ID << " ";

      int node_type=get_node_type(node_ptr);
      if (node_type==0)	// false
      {
         cout << "False encountered" << endl;
      }
      else if (node_type==1)	// true
      {
         cout << "True encountered" << endl;
      }
      else if (node_type==2)	// null
      {
         cout << "Null encountered" << endl;
      }
      else if (node_type==3)	// number
      {
         curr_key=get_name(node_ptr);
         curr_value=string(cJSON_Print(node_ptr));
         KEY_VALUE_PAIR P(curr_key,curr_value);
         curr_key_value_pairs.push_back(P);
      }
      else if (node_type==4)	// string
      {
         curr_key=get_name(node_ptr);
         curr_value=string(cJSON_Print(node_ptr));
         KEY_VALUE_PAIR P(curr_key,curr_value);
         curr_key_value_pairs.push_back(P);
      }
      else if (node_type==5)	// array
      {
         curr_key=get_name(node_ptr);
         curr_value="";
	 int array_size = cJSON_GetArraySize(node_ptr);

         cJSON* array_node_ptr = node_ptr->child;
         for (int a=0; a<array_size; a++)
         {
            int array_node_type=get_node_type(array_node_ptr);
            if (array_node_type==3)
            {
               curr_value += string(cJSON_Print(array_node_ptr))+" ";
               array_node_ptr = array_node_ptr->next;
            }
         }
         if (curr_value.size()==0) curr_value="array";

         KEY_VALUE_PAIR P(curr_key,curr_value);
         curr_key_value_pairs.push_back(P);
      }
      else if (node_type==6)	// object
      {
         curr_key=get_name(node_ptr);
         curr_value="object";
         KEY_VALUE_PAIR P(curr_key,curr_value);
         curr_key_value_pairs.push_back(P);
         
         if (curr_key_value_pairs.size() > 0)
         {
            key_value_pairs_ptr->push_back(curr_key_value_pairs);
            curr_key_value_pairs.clear();
         }
      }
      
// Search for possible child node

      cJSON* child_node_ptr=get_child_node_ptr(node_ptr);
      if (child_node_ptr != NULL) 
      {
         retrieve_subtree_key_value_pairs(start_level, child_node_ptr);
      }
      
// Move to next sibling node only if current level exceeds starting level:

      cJSON* next_node_ptr=get_next_node_ptr(node_ptr);
      if (next_node_ptr==NULL)
      {
         node_ptr=NULL;
      }
      else
      {
         if (next_node_ptr->level > start_level)
         {
            node_ptr=next_node_ptr;
         }
         else
         {
            node_ptr=NULL;
         }
      }
      
   } // while node_ptr != NULL 

   if (curr_key_value_pairs.size() > 0)
   {
      key_value_pairs_ptr->push_back(curr_key_value_pairs);
      curr_key_value_pairs.clear();
   }
}

// ---------------------------------------------------------------------
// Member function extract_key_value_pairs() retrieves key-value
// pairs for all nodes within the JSON subtree defined by *input
// node_ptr.  It returns the key-value pairs within member STL vector
// *key_value_pairs_ptr.

cppJSON::KEY_VALUE_PAIRS* cppJSON::extract_key_value_pairs(cJSON* node_ptr)
{
//  cout << "inside cppJSON::extract_key_value_pairs()" << endl;
//  cout << "node_ptr->level = " << node_ptr->level << endl;

   curr_key_value_pairs.clear();

   delete key_value_pairs_ptr;
   key_value_pairs_ptr=new KEY_VALUE_PAIRS;

   retrieve_subtree_key_value_pairs(node_ptr->level, node_ptr);

   return key_value_pairs_ptr;
}

// =========================================================================
// Object key-value pair parsing member functions
// =========================================================================

int cppJSON::get_n_objects() const
{
   return key_value_pairs_ptr->size();
}

vector<cppJSON::KEY_VALUE_PAIR>& cppJSON::get_object_key_value_pairs(int n)
{
   return key_value_pairs_ptr->at(n);
}
