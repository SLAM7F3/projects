// ==========================================================================
// Message class member function definitions
// ==========================================================================
// Last modified on 5/30/08; 6/1/08; 6/9/08; 5/11/10
// ==========================================================================

#include <iostream>
#include <vector>
#include "messenger/message.h"

using std::cout;
using std::endl;
using std::ostream;
using std::map;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void message::allocate_member_objects()
{
   key_value_map_ptr=new std::map<std::string,std::string>;
}		       

void message::initialize_member_objects()
{
}

message::message()
{
   allocate_member_objects();
   initialize_member_objects();
}

message::message(string text_message,string producer_ID)
{
   allocate_member_objects();
   initialize_member_objects();

   this->text_message=text_message;
   this->producer_ID=producer_ID;
}

message::message(string text_message,string producer_ID,
                 const vector<Property>& props)
{
//   cout << "inside message constructor" << endl;
   
   allocate_member_objects();
   initialize_member_objects();

   this->text_message=text_message;
   this->producer_ID=producer_ID;

   for (unsigned int p=0; p<props.size(); p++)
   {
//      cout << "message property # = " << p << endl;
//      cout << "properties[p].first = " << props[p].first
//           << " properties[p].second = " << props[p].second
//           << endl;
      pushback_property(props[p]);
   } // loop over index p labeling message properties
}

// Copy constructor:

message::message(const message& m)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(m);
}

message::~message()
{
   delete key_value_map_ptr;
}

// ---------------------------------------------------------------------
void message::docopy(const message& m)
{
   text_message=m.text_message;
   producer_ID=m.producer_ID;

   properties.clear();
   for (unsigned int p=0; p<m.get_n_properties(); p++)
   {
      Property P(m.get_property(p).first,m.get_property(p).second);
      pushback_property(P);
   }

   extract_and_store_property_keys_and_values();
}

// Overload = operator:

message& message::operator= (const message& m)
{
   if (this==&m) return *this;
   docopy(m);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const message& m)
{
   outstream.precision(12);

   outstream << endl;
   outstream << "text_command = " << m.text_message << endl;
//   outstream << "producer_ID = " << m.producer_ID << endl;

   for (unsigned int p=0; p<m.get_n_properties(); p++)
   {
      outstream << "Property " << p << endl;
      message::Property P=m.get_property(p);
      outstream << "  key = " << P.first << endl;
      outstream << " value = " << P.second << endl;
   }
   outstream << endl;

   return outstream;
}

// =====================================================================
// Property retrieval member functions
// =====================================================================

// Member function extract_and_store_property_keys_and_values loops
// over the current messages properties.  It extracts all key and
// value strings and stores them within STL map member
// *key_value_map_ptr.

void message::extract_and_store_property_keys_and_values()
{
   key_value_map_ptr->clear();
   for (unsigned int n=0; n<get_n_properties(); n++)
   {
      (*key_value_map_ptr)[get_property(n).first]=get_property(n).second;
   }
}

// ---------------------------------------------------------------------
// Member function get_property_value

string message::get_property_value(string property_key)
{
   map<string,string>::iterator key_value_iter=
      key_value_map_ptr->find(property_key);

   if (key_value_iter != key_value_map_ptr->end())
   {
      return key_value_iter->second;
   } 
   else
   {
      return "";
   }
}


