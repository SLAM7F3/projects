// ==========================================================================
// Header file for data_example class 
// ==========================================================================
// Last modified on 8/1/15; 8/30/15
// ==========================================================================

#ifndef DATA_EXAMPLE_H
#define DATA_EXAMPLE_H

#include <string>
#include <vector>

class data_example
{   
  public:

// Initialization, constructor and destructor functions:

   data_example(int id);
//   data_example(const data_example& dt);
   ~data_example();
//   data_example operator= (const data_example& dt);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const data_example& dt);

// Set and get member functions

   int get_ID() const;
   void append_feature_label(std::string label);
   void append_feature_value(std::string f);
   void set_classification_value(std::string curr_value);

   std::string get_feature_label(int f) const;
   std::string get_feature_value(int f) const;
   std::string get_classification_value() const;

  private: 

   int ID;
   std::vector<std::string> feature_labels;
   std::vector<std::string> feature_values;
   std::string classification_value;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const data_example& C);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline int data_example::get_ID() const
{
   return ID;
}

inline void data_example::append_feature_label(std::string label)
{
   feature_labels.push_back(label);
}

inline void data_example::append_feature_value(std::string f)
{
   feature_values.push_back(f);
}

inline void data_example::set_classification_value(std::string curr_value)
{
   classification_value = curr_value;
}

inline std::string data_example::get_feature_label(int f) const
{
   return feature_labels.at(f);
}

inline std::string data_example::get_feature_value(int f) const
{
   return feature_values.at(f);
}

inline std::string data_example::get_classification_value() const
{
   return classification_value;
}


#endif  // data_example.h


