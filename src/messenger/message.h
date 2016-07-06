// ==========================================================================
// Header file for message class
// ==========================================================================
// Last modified on 3/6/08; 5/30/08; 6/1/08; 4/5/14
// ==========================================================================

#ifndef MESSAGE_H
#define MESSAGE_H

#include <map>
#include <set>
#include <string>
#include <vector>

class message
{

  public:

   typedef std::pair<std::string,std::string> Property;
   
// Initialization, constructor and destructor functions:

   message();
   message(std::string text_message,std::string producer_ID);
   message(std::string text_message,std::string producer_ID,
           const std::vector<Property>& props);
   message(const message& m);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~message();
   message& operator= (const message& p);
   friend std::ostream& operator<< (std::ostream& outstream,const message& m);

// Set and get member functions:

   std::string get_text_message() const;
   std::string get_producer_ID() const;

   unsigned int get_n_properties() const;
   void pushback_property(const Property& P);
   Property get_property(int n);
   const Property get_property(int n) const;
   std::map<std::string,std::string>* get_key_value_map_ptr() const;

// Property retrieval member functions

   void extract_and_store_property_keys_and_values();
   std::string get_property_value(std::string property_key);

  private: 

   std::string text_message;
   std::string producer_ID;
   std::vector<Property> properties;
   std::map<std::string,std::string>* key_value_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const message& m);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline std::string message::get_text_message() const
{
   return text_message;
}

inline std::string message::get_producer_ID() const
{
   return producer_ID;
}

inline unsigned int message::get_n_properties() const
{
   return properties.size();
}

inline void message::pushback_property(const Property& P)
{
   properties.push_back(P);
}

inline message::Property message::get_property(int n) 
{
   return properties[n];
}

inline const message::Property message::get_property(int n) 
   const
{
   return properties[n];
}

inline std::map<std::string,std::string>* message::get_key_value_map_ptr() 
   const
{
   return key_value_map_ptr;
}


#endif  // message.h



