// ==========================================================================
// Header file for GRAPHNODESGROUP class
// ==========================================================================
// Last modified on 1/26/07; 10/13/07; 3/22/14; 9/9/15
// ==========================================================================

#ifndef GRAPHNODESGROUP_H
#define GRAPHNODESGROUP_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <osg/Group>
#include <osg/Node>
#include "osg/osgGeometry/BoxesGroup.h"
#include "osg/osgAnnotators/GraphNode.h"
#include "datastructures/Tree.h"
#include "osg/osgSceneGraph/TreeVisitor.h"

typedef std::map<std::pair<int, int>, std::vector<std::string> > 
   TABLE_LABELS_MAP;
// independent pair var: (row, column)
// dependent vector<string>: table entry labels

class AnimationController;
class GraphNodesGroup : public BoxesGroup
{

  public:

   typedef TreeVisitor::data_type data_type;

// Initialization, constructor and destructor functions:

   GraphNodesGroup(Pass* PI_ptr,threevector* GO_ptr);
   GraphNodesGroup(Pass* PI_ptr,threevector* GO_ptr,TreeVisitor* TV_ptr);
   GraphNodesGroup(Pass* PI_ptr,threevector* GO_ptr,TreeVisitor* TV_ptr,
                   AnimationController* AC_ptr);
   virtual ~GraphNodesGroup();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const GraphNodesGroup& G);

// Set & get methods:

   void set_display_scenegraph_flag(bool flag);
   GraphNode* get_GraphNode_ptr(int n) const;
   GraphNode* get_ID_labeled_GraphNode_ptr(int ID) const;

// GraphNode creation and manipulation methods:

   GraphNode* generate_new_GraphNode(const threevector& V,int ID=-1);

   void reset_colors();
   
// Table creation and display methods:

   void set_table_labels_map_ptr(TABLE_LABELS_MAP* table_labels_map_ptr);
   bool generate_Graph_from_table(int n_rows, int n_columns);
   void set_table_entry_labels_and_color(
      GraphNode* curr_GraphNode_ptr, std::vector<std::string>& input_labels);

// Tree creation and display methods:

   void generate_Graph_from_tree();
   void set_Treenode_labels_and_color(
      GraphNode* curr_GraphNode_ptr,Treenode<data_type>* curr_Treenode_ptr);
   void set_scenegraph_Treenode_labels_and_color(
      GraphNode* curr_GraphNode_ptr,Treenode<data_type>* curr_Treenode_ptr);
   void form_child_parent_links(
      GraphNode* curr_GraphNode_ptr,Treenode<data_type>* curr_Treenode_ptr);
   void update_display();
   int destroy_GraphNode();

// Animation methods:

   void display_curr_visited_node();

  protected:

  private:

   bool display_scenegraph_flag;
   bool refresh_scenegraph_display_flag;

   unsigned int curr_visited_node_number,prev_visited_node_number;
   double prev_update_time,min_elapsed_time_interval;
   double width,height,depth,face_displacement;
   TABLE_LABELS_MAP* table_labels_map_ptr; // Just pointer to external hashmap
   Tree<data_type>* tree_ptr;
   TreeVisitor* TreeVisitor_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const GraphNodesGroup& G);

   GraphNode* generate_new_canonical_GraphNode(int ID=-1);
   void setup_canonical_GraphNode(int ID,GraphNode* curr_GraphNode_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:


inline void GraphNodesGroup::set_display_scenegraph_flag(bool flag)
{
   display_scenegraph_flag = flag;
}

inline GraphNode* GraphNodesGroup::get_GraphNode_ptr(int n) const
{
   return dynamic_cast<GraphNode*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline GraphNode* GraphNodesGroup::get_ID_labeled_GraphNode_ptr(int ID) const
{
   return dynamic_cast<GraphNode*>(get_ID_labeled_Graphical_ptr(ID));
}

#endif // GraphNodesGroup.h



