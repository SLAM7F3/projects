// ==========================================================================
// Header file for dtree_node_data class 
// ==========================================================================
// Last modified on 8/9/15
// ==========================================================================

#ifndef DTREE_NODE_DATA_H
#define DTREE_NODE_DATA_H

#include <string>
#include <vector>

class dtree_node_data
{   
  public:

// Initialization, constructor and destructor functions:

   dtree_node_data(int id);
//   dtree_node_data(const dtree_node_data& dt);
   ~dtree_node_data();
//   dtree_node_data operator= (const dtree_node_data& dt);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const dtree_node_data& dt);

// Set and get member functions

   int get_ID() const;
   void set_prev_decision_feature_value(std::string value);
   std::string get_prev_decision_feature_value() const;
   void set_decision_feature_label(std::string label);
   std::string get_decision_feature_label() const;
   void set_classification_value(std::string value);
   std::string get_classification_value() const;
   void set_training_example_IDs(const std::vector<int>& IDs);
   void set_active_feature_IDs(const std::vector<int>& IDs);

   std::vector<int>& get_training_example_IDs();
   const std::vector<int>& get_training_example_IDs() const;
   std::vector<int>& get_active_feature_IDs();
   const std::vector<int>& get_active_feature_IDs() const;

  private: 

   int ID;
   std::string prev_decision_feature_value;
   std::string decision_feature_label;
   std::vector<int> training_example_IDs;
   std::vector<int> active_feature_IDs;
   std::string classification_value;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const dtree_node_data& C);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline int dtree_node_data::get_ID() const
{
   return ID;
}

inline void dtree_node_data::set_prev_decision_feature_value(std::string value)
{
   prev_decision_feature_value = value;
}

inline std::string dtree_node_data::get_prev_decision_feature_value() const
{
   return prev_decision_feature_value;
}

inline void dtree_node_data::set_decision_feature_label(std::string label)
{
   decision_feature_label = label;
}

inline std::string dtree_node_data::get_decision_feature_label() const
{
   return decision_feature_label;
}

inline void dtree_node_data::set_classification_value(std::string value)
{
   classification_value = value;
}

inline std::string dtree_node_data::get_classification_value() const
{
   return classification_value;
}

inline std::vector<int>& dtree_node_data::get_training_example_IDs() 
{
   return training_example_IDs;
}

inline const std::vector<int>& dtree_node_data::get_training_example_IDs() const
{
   return training_example_IDs;
}

inline std::vector<int>& dtree_node_data::get_active_feature_IDs() 
{
   return active_feature_IDs;
}

inline const std::vector<int>& dtree_node_data::get_active_feature_IDs() const
{
   return active_feature_IDs;
}


#endif  // dtree_node_data.h


