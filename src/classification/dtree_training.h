// ==========================================================================
// Header file for dtree_training class 
// ==========================================================================
// Last modified on 8/1/15
// ==========================================================================

#ifndef DTREE_TRAINING_H
#define DTREE_TRAINING_H

#include <string>
#include <vector>

class dtree_training
{   
  public:

// Initialization, constructor and destructor functions:

   dtree_training(int id);
//   dtree_training(const dtree_training& dt);
   ~dtree_training();
//   dtree_training operator= (const dtree_training& dt);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const dtree_training& dt);

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
   void docopy(const dtree_training& C);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline int dtree_training::get_ID() const
{
   return ID;
}

inline void dtree_training::append_feature_label(std::string label)
{
   feature_labels.push_back(label);
}

inline void dtree_training::append_feature_value(std::string f)
{
   feature_values.push_back(f);
}

inline void dtree_training::set_classification_value(std::string curr_value)
{
   classification_value = curr_value;
}

inline std::string dtree_training::get_feature_label(int f) const
{
   return feature_labels.at(f);
}

inline std::string dtree_training::get_feature_value(int f) const
{
   return feature_values.at(f);
}

inline std::string dtree_training::get_classification_value() const
{
   return classification_value;
}


#endif  // dtree_training.h


