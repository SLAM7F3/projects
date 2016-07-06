// ==========================================================================
// GRAPHNODESGROUP class member function definitions
// ==========================================================================
// Last modified on 10/13/07; 12/2/11; 8/15/15; 9/9/15
// ==========================================================================

#include <iomanip>
#include <vector>
#include "osg/osgGraphicals/AnimationController.h"
#include "color/colorfuncs.h"
#include "osg/osgAnnotators/GraphNodesGroup.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"
#include "osg/osgSceneGraph/TreeVisitor.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GraphNodesGroup::allocate_member_objects()
{
}		       

void GraphNodesGroup::initialize_member_objects()
{
   GraphicalsGroup_name="GraphNodesGroup";

// Dimensions for Graph Nodes:

   width=40;
   height=15;

   depth=0.01;
   face_displacement=-0.5*depth;
   BoxesGroup::set_wlhd(width,height,depth,face_displacement);

   table_labels_map_ptr = NULL;
   TreeVisitor_ptr=NULL;
   tree_ptr=NULL;

   prev_update_time=NEGATIVEINFINITY;
   min_elapsed_time_interval=1.5;		// secs   

   prev_visited_node_number=curr_visited_node_number=0;
   display_scenegraph_flag = true;
   refresh_scenegraph_display_flag=true;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<GraphNodesGroup>(
         this, &GraphNodesGroup::update_display));
}		       

GraphNodesGroup::GraphNodesGroup(Pass* PI_ptr,threevector* GO_ptr):
   BoxesGroup(PI_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

GraphNodesGroup::GraphNodesGroup(
   Pass* PI_ptr,threevector* GO_ptr,TreeVisitor* TV_ptr):
   BoxesGroup(PI_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   TreeVisitor_ptr=TV_ptr;
   tree_ptr=TreeVisitor_ptr->get_tree_ptr();
}		       

GraphNodesGroup::GraphNodesGroup(
   Pass* PI_ptr,threevector* GO_ptr,TreeVisitor* TV_ptr,
   AnimationController* AC_ptr):
   BoxesGroup(PI_ptr,GO_ptr,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   TreeVisitor_ptr=TV_ptr;
   tree_ptr=TreeVisitor_ptr->get_tree_ptr();
}		       

GraphNodesGroup::~GraphNodesGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const GraphNodesGroup& G)
{
   int node_counter=0;
   for (unsigned int n=0; n<G.get_n_Graphicals(); n++)
   {
      GraphNode* GraphNode_ptr=G.get_GraphNode_ptr(n);
      outstream << "GraphNode node # " << node_counter++ << endl;
      outstream << "GraphNode = " << *GraphNode_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// GraphNode creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_GraphNode from all other graphical insertion
// and manipulation methods...

GraphNode* GraphNodesGroup::generate_new_GraphNode(
   const threevector& V,int ID)
{
//   cout << "inside GNG::generate_new_GraphNode()" << endl;
   GraphNode* curr_GraphNode_ptr=generate_new_canonical_GraphNode(ID);

// Note added on 1/17/07: We must change this next line sometime in
// the future so that LSG->OSGgroup_ptr is not attached to
// GraphNodesGroup->OSGgroup_ptr but rather to
// GraphNodesGroup->OSGsubPAT(0).  See note below dated 1/17/07

   initialize_Graphical(
      V,curr_GraphNode_ptr,
      curr_GraphNode_ptr->get_LineSegmentsGroup_ptr()->get_OSGgroup_ptr());

   insert_graphical_PAT_into_OSGsubPAT(curr_GraphNode_ptr,0);

   curr_GraphNode_ptr->set_posn(get_curr_t(),get_passnumber(),V);
   update_mybox(static_cast<Box*>(curr_GraphNode_ptr));

   return curr_GraphNode_ptr;
}

GraphNode* GraphNodesGroup::generate_new_canonical_GraphNode(int ID)
{
//   cout << "inside GNG::generate_new_canonical_GraphNode()" << endl;

   if (ID==-1) ID=get_next_unused_ID();
   GraphNode* curr_GraphNode_ptr=new GraphNode(
      ID,width,height,depth,face_displacement,pass_ptr);
   GraphicalsGroup::insert_Graphical_into_list(curr_GraphNode_ptr);
   setup_canonical_GraphNode(ID,curr_GraphNode_ptr);
   update_mybox(static_cast<Box*>(curr_GraphNode_ptr));
   return curr_GraphNode_ptr;
}

void GraphNodesGroup::setup_canonical_GraphNode(
   int ID,GraphNode* curr_GraphNode_ptr)
{
//   cout << "inside GNG::setup_canonical_GraphNode()" << endl;
   osg::Group* drawable_group_ptr=
      curr_GraphNode_ptr->generate_drawable_group();
   curr_GraphNode_ptr->get_PAT_ptr()->addChild(drawable_group_ptr);
   reset_colors();
}

// --------------------------------------------------------------------------
// Member function reset_colors loops over all entries within the
// GraphNodesgroup.

void GraphNodesGroup::reset_colors()
{   
//   cout << "inside GNG::reset_colors()" << endl;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      GraphNode* GraphNode_ptr=get_GraphNode_ptr(n);
//      GraphNode_ptr->set_color(GraphNode_ptr->get_permanent_color());
      GraphNode_ptr->set_color(GraphNode_ptr->get_curr_color());
   } // loop over GraphNodes 
}

// ==========================================================================
// Table creation and display methods:
// ==========================================================================

void GraphNodesGroup::set_table_labels_map_ptr(
   TABLE_LABELS_MAP* table_labels_map_ptr)
{
   this->table_labels_map_ptr = table_labels_map_ptr;
}

// --------------------------------------------------------------------------
// Member function generate_Graph_from_table() takes in the maximum
// number of rows and columns for a 2D table.  If table_labels_map_ptr
// has not been set, this boolean method returns false.  Otherwise, it
// retrieves string labels from *table_labels_map_ptr correponding to
// table cells.  Each cell is displayed as a text box within the 2D
// table matrix.  Empty table cells are not displayed.

bool GraphNodesGroup::generate_Graph_from_table(int n_rows, int n_columns)
{
   cout << "inside GraphNodesGroup::generate_Graph_from_table()" << endl;
   
   if(table_labels_map_ptr == NULL) return false;

   int node_ID = 0;
   for(int r = 0; r < n_rows; r++)
   {
      double y = 2 * r * height;
      for(int c = 0; c < n_columns; c++)
      {
         double x = 2 * c * width;
         threevector node_posn(x,y);

         pair<int,int> P;
         P.first = r;
         P.second = c;
         TABLE_LABELS_MAP::iterator table_labels_iter = 
            table_labels_map_ptr->find(P);

         vector<string> input_labels;
         if(table_labels_iter != table_labels_map_ptr->end())
         {
            for(unsigned int l = 0; l < table_labels_iter->second.size(); l++)
            {
               input_labels.push_back(table_labels_iter->second[l]);
            }
         }

         if(input_labels.size() == 0) continue;
//         cout << "node: ID = " << node_ID << " posn = " << node_posn << endl;

// Instantiate GraphNode corresponding to table entry:

         GraphNode* curr_GraphNode_ptr=generate_new_GraphNode(
            node_posn, node_ID);
         node_ID++;

// Set curr_GraphNode's text label to that coming from table entry:
         
         set_table_entry_labels_and_color(curr_GraphNode_ptr, input_labels);

      } // loop over index c labeling table columns
   } // loop over index r labeling table rows

   return true;
}		       

// --------------------------------------------------------------------------
// Member function set_table_entry_labels_and_color() sets input
// *curr_GraphNode_ptr's text label(s) and color. 

void GraphNodesGroup::set_table_entry_labels_and_color(
   GraphNode* curr_GraphNode_ptr, vector<string>& input_labels)
{   
//  cout << "inside GraphNodesGroup::set_table_entry_labels_and_color()" << endl;
   
   unsigned int max_nlines=4;
   unsigned int nlines=input_labels.size();

   vector<unsigned int> linenumber;
   switch (nlines)
   {
      case 1:
         linenumber.push_back(2);
         break;
      case 2:
         linenumber.push_back(1);
         linenumber.push_back(3);
         break;
      case 3:
         linenumber.push_back(1);
         linenumber.push_back(2);
         linenumber.push_back(3);
         break;
      case 4:
         linenumber.push_back(1);
         linenumber.push_back(2);
         linenumber.push_back(3);
         linenumber.push_back(4);
         break;
   }
         
   for (unsigned int i=0; i<=max_nlines; i++)
   {
//      curr_GraphNode_ptr->change_text_size(i,0.8);

      string label="";
      string ID_label=" "+stringfunc::number_to_string(
         curr_GraphNode_ptr->get_ID());
      for (unsigned int j=0; j<nlines; j++)
      {
         if (i==linenumber[j])
         {
            label=input_labels[j];
//            cout << label << endl;
         } 
      } // loop over index j labeling table entry label lines

      curr_GraphNode_ptr->set_text_label(i,label);
   } // loop over index i labeling string labels

   curr_GraphNode_ptr->set_permanent_color(
      colorfunc::get_OSG_color(colorfunc::cream));
//   if(input_labels.size() > 1)
//   {
//      curr_GraphNode_ptr->set_permanent_color(
//         colorfunc::get_OSG_color(colorfunc::pink));
//   }
   curr_GraphNode_ptr->set_curr_color(
      curr_GraphNode_ptr->get_permanent_color());
}


// ==========================================================================
// Tree creation and display methods:
// ==========================================================================

void GraphNodesGroup::generate_Graph_from_tree()
{
   cout << "inside GraphNodesGroup::generate_Graph_from_tree()" << endl;
   
   if (tree_ptr==NULL) return;

// First compute maximum x extent of entire tree graph:

   cout << "tree_ptr->n_levels = " << tree_ptr->get_n_levels() << endl;

   double max_x_extent = 0;
   for (int l=0; l<=tree_ptr->get_n_levels(); l++)
   {
      int c_max=tree_ptr->number_nodes_on_level(l);
      int c_top=c_max-1;
      double x_extent = 2*c_top*width;
      max_x_extent=basic_math::max(max_x_extent, x_extent);
   }

   for (int l=0; l<=tree_ptr->get_n_levels(); l++)
   {
      double vert_separation = max_x_extent / (tree_ptr->get_n_levels() + 1);
      double y=0.0-vert_separation*l;

      tree_ptr->compute_columns_for_nodes_on_level(l);
      int c_max=tree_ptr->number_nodes_on_level(l);
      int c_top=c_max-1;
      int c_bottom=-c_top;

      for (int c=0; c<c_max; c++)
      {
         double x=(c_bottom+2*c)*width;
         string banner="Level = "+stringfunc::number_to_string(l)
            +" Column = "+stringfunc::number_to_string(c);
//         outputfunc::write_banner(banner);
         Treenode<data_type>* curr_Treenode_ptr=tree_ptr->get_node(l,c);
//         cout << "curr_Treenode = " << *curr_Treenode_ptr << endl;

         threevector node_posn(x,y);
//         cout << "node_posn = " << node_posn << endl;
         curr_Treenode_ptr->set_posn(node_posn);

// Instantiate GraphNode corresponding to current Treenode, and equate
// both nodes' IDs:

         GraphNode* curr_GraphNode_ptr=generate_new_GraphNode(
            curr_Treenode_ptr->get_posn(),
            curr_Treenode_ptr->get_ID());

// Set curr_GraphNode's text label to that coming from current
// Treenode:

         if(display_scenegraph_flag)
         {
            set_scenegraph_Treenode_labels_and_color(
               curr_GraphNode_ptr, curr_Treenode_ptr);
         }
         else
         {
            set_Treenode_labels_and_color(
               curr_GraphNode_ptr, curr_Treenode_ptr);
         }
        
// Draw relative links between curr_GraphNode and its parent(s) node:

         form_child_parent_links(curr_GraphNode_ptr,curr_Treenode_ptr);

      } // loop over index c labeling columns
   } // loop over index l labeling tree levels

}		       

// --------------------------------------------------------------------------
// Member function set_Treenode_labels_and_color sets input
// *curr_GraphNode_ptr's text label to that coming from
// *curr_Treenode_ptr.  

void GraphNodesGroup::set_Treenode_labels_and_color(
   GraphNode* curr_GraphNode_ptr,Treenode<data_type>* curr_Treenode_ptr)
{   
//  cout << "inside GraphNodesGroup::set_Treenode_labels_and_color()" << endl;
   
//   unsigned int max_nlines=curr_GraphNode_ptr->get_n_text_messages();
   unsigned int max_nlines=4;
   unsigned int nlines=curr_Treenode_ptr->get_data().size();

   vector<unsigned int> linenumber;
   switch (nlines)
   {
      case 1:
         linenumber.push_back(2);
         break;
      case 2:
         linenumber.push_back(1);
         linenumber.push_back(3);
         break;
      case 3:
         linenumber.push_back(1);
         linenumber.push_back(2);
         linenumber.push_back(3);
         break;
      case 4:
         linenumber.push_back(1);
         linenumber.push_back(2);
         linenumber.push_back(3);
         linenumber.push_back(4);
         break;
   }

   int string_counter=0;
   curr_GraphNode_ptr->set_permanent_color(
      colorfunc::get_OSG_color(colorfunc::cream));
         
   for (unsigned int i=0; i<=max_nlines; i++)
   {
//      curr_GraphNode_ptr->change_text_size(i,0.8);

      string label="";
      string ID_label=" "+stringfunc::number_to_string(
         curr_Treenode_ptr->get_ID());
      for (unsigned int j=0; j<nlines; j++)
      {
         if (i==linenumber[j])
         {
            label=curr_Treenode_ptr->get_data().at(string_counter++);
            if (label.substr(0,14)=="Classification")
            {
               curr_GraphNode_ptr->set_permanent_color(
                  colorfunc::get_OSG_color(colorfunc::pink));
            }
         } 
      } // loop over index j labeling Treenode label lines

      curr_GraphNode_ptr->set_curr_color(
         curr_GraphNode_ptr->get_permanent_color());

      curr_GraphNode_ptr->set_text_label(i,label);
   } // loop over index i labeling string labels
}

// --------------------------------------------------------------------------
// Member function set_scenegraph_Treenode_labels_and_color sets input
// *curr_GraphNode_ptr's text label to that coming from
// *curr_Treenode_ptr.  It also sets the GraphNode's permanent color
// based upon the OSG node type (e.g. PagedLODs are yellow,
// MatrixTransforms are pink, etc)

void GraphNodesGroup::set_scenegraph_Treenode_labels_and_color(
   GraphNode* curr_GraphNode_ptr,Treenode<data_type>* curr_Treenode_ptr)
{   
//   cout << "inside GraphNodesGroup::set_scenegraph_Treenode_labels_and_color()" << endl;
   
   unsigned int max_nlines=curr_GraphNode_ptr->get_n_text_messages();
   unsigned int nlines=curr_Treenode_ptr->get_data().size();
//   cout << "nlines = " << nlines << endl;

   vector<unsigned int> linenumber;
   switch (nlines)
   {
      case 1:
         linenumber.push_back(2);
         break;
      case 2:
         linenumber.push_back(1);
         linenumber.push_back(3);
         break;
      case 3:
         linenumber.push_back(1);
         linenumber.push_back(2);
         linenumber.push_back(3);
         break;
      case 4:
         linenumber.push_back(1);
         linenumber.push_back(2);
         linenumber.push_back(3);
         linenumber.push_back(4);
         break;
      case 5:
         linenumber.push_back(0);
         linenumber.push_back(1);
         linenumber.push_back(2);
         linenumber.push_back(3);
         linenumber.push_back(4);
         break;
      case 6:
         linenumber.push_back(0);
         linenumber.push_back(1);
         linenumber.push_back(2);
         linenumber.push_back(3);
         linenumber.push_back(4);
         linenumber.push_back(5);
         break;
   }

   int string_counter=0;
   curr_GraphNode_ptr->set_permanent_color(
      colorfunc::get_OSG_color(colorfunc::cream));
         
   bool reset_text_font_and_size_flag=false;
   for (unsigned int i=0; i<max_nlines; i++)
   {
      if (reset_text_font_and_size_flag)
         curr_GraphNode_ptr->reset_text_font_and_size(i,0.33);
//         curr_GraphNode_ptr->reset_text_font_and_size(i,0.18);

      string label="";
      string ID_label=" "+stringfunc::number_to_string(
         curr_Treenode_ptr->get_ID());
      for (unsigned int j=0; j<nlines; j++)
      {
         if (i==linenumber[j])
         {
            label=curr_Treenode_ptr->get_data().at(string_counter++);

// Color current GraphNode based upon its text label:

            if (label.substr(0,20)=="CoordinateSystemNode")
            {
               curr_GraphNode_ptr->set_OSGnode_type(
                  GraphNode::CoordinateSystemNode);
               label += ID_label;
               reset_text_font_and_size_flag=true;
               curr_GraphNode_ptr->set_permanent_color(
                  colorfunc::get_OSG_color(colorfunc::cyan));
            }
            else if (label.substr(0,10)=="Projection")
            {
               curr_GraphNode_ptr->set_OSGnode_type(GraphNode::Projection);
               label += ID_label;
               curr_GraphNode_ptr->set_permanent_color(
                  colorfunc::get_OSG_color(colorfunc::magenta));
            }
            else if (label.substr(0,25)=="PositionAttitudeTransform")
            {
               curr_GraphNode_ptr->set_OSGnode_type(
                  GraphNode::PositionAttitudeTransform);
               label="PAT "+ID_label;
               reset_text_font_and_size_flag=true;
               curr_GraphNode_ptr->set_permanent_color(
                  colorfunc::get_OSG_color(colorfunc::yegr));
            }
            else if (label.substr(0,15)=="MatrixTransform")
            {
               curr_GraphNode_ptr->set_OSGnode_type(
                  GraphNode::MatrixTransform);
               label += ID_label;
               reset_text_font_and_size_flag=true;
               curr_GraphNode_ptr->set_permanent_color(
                  colorfunc::get_OSG_color(colorfunc::pink));
            }
            else if (label.substr(0,8)=="PagedLOD")
            {
               curr_GraphNode_ptr->set_OSGnode_type(GraphNode::PagedLOD);
               label += ID_label;
               curr_GraphNode_ptr->set_permanent_color(
                  colorfunc::get_OSG_color(colorfunc::yellow));
            }
            else if (label.substr(0,3)=="LOD")
            {
               curr_GraphNode_ptr->set_OSGnode_type(GraphNode::LOD);
               label += ID_label;
               curr_GraphNode_ptr->set_permanent_color(
                  colorfunc::get_OSG_color(colorfunc::gold));
            }
            else if (label.substr(0,5)=="Group")
            {
               curr_GraphNode_ptr->set_OSGnode_type(GraphNode::Group);
               label += ID_label;
               curr_GraphNode_ptr->set_permanent_color(
                  colorfunc::get_OSG_color(colorfunc::orange));
            }
            else if (label.substr(0,5)=="Geode")
            {
               curr_GraphNode_ptr->set_OSGnode_type(GraphNode::Geode);
               label += ID_label;
               curr_GraphNode_ptr->set_permanent_color(
                  colorfunc::get_OSG_color(colorfunc::green));
            }
         } // i==linenumber[j] conditional
      } // loop over index j labeling Treenode label lines

      curr_GraphNode_ptr->set_curr_color(
         curr_GraphNode_ptr->get_permanent_color());

      curr_GraphNode_ptr->set_text_label(i,label);
   } // loop over index i labeling string labels
}

// --------------------------------------------------------------------------
// Member function form_child_parent_links instantiates relative
// LineSegments between input *curr_GraphNode_ptr and its parent(s)
// node(s).

void GraphNodesGroup::form_child_parent_links(
   GraphNode* curr_GraphNode_ptr,Treenode<data_type>* curr_Treenode_ptr)
{   
   Linkedlist< Treenode<data_type>* >* Parents_list_ptr=
      curr_Treenode_ptr->get_Parents_list_ptr();

   LineSegmentsGroup* LineSegmentsGroup_ptr=
      curr_GraphNode_ptr->get_LineSegmentsGroup_ptr();

   for (Mynode<Treenode<data_type>* >* parent_node_ptr=
           Parents_list_ptr->get_start_ptr(); parent_node_ptr != NULL;
        parent_node_ptr=parent_node_ptr->get_nextptr())
   {
      threevector parent_posn=parent_node_ptr->get_data()->get_posn();
//            cout << "parent_posn = " << parent_posn << endl;

// Draw RELATIVE link from curr_GraphNode posn to its parent posn.
// PAT translation of curr_GraphNode will then carry link to its
// final, absolute position within the output tree display:

      threevector origin(0,0,0);
      LineSegment* curr_link_ptr=LineSegmentsGroup_ptr->
         generate_new_segment(origin,parent_posn-
                              curr_Treenode_ptr->get_posn());

      curr_GraphNode_ptr->get_drawable_group_ptr()->
         addChild(curr_link_ptr->get_geode_ptr());

// Base coloring of parent/child links upon parent's column value to
// emphasize sibling relationships

      int parent_column=parent_node_ptr->get_data()->get_column();

      const int n_link_colors=15;
      int color_index=modulo(parent_column,n_link_colors);
      colorfunc::Color curr_color=colorfunc::get_color(color_index);

/*
      colorfunc::Color curr_color=colorfunc::blue;
      switch(color_index)
      {
         case 0:
            curr_color=colorfunc::blue;
            break;
         case 1:
            curr_color=colorfunc::grey;
            break;
      }
*/

// Geode's always appear to have unique links to their parents.  So
// correlate their link and node colors:

      if (curr_GraphNode_ptr->get_OSGnode_type()==GraphNode::Geode)
      {
         curr_color=colorfunc::blgr;
      }
      
      curr_link_ptr->set_permanent_color(curr_color);

   }
   LineSegmentsGroup_ptr->set_width(3);
   LineSegmentsGroup_ptr->reset_colors();
}

// --------------------------------------------------------------------------
// Member function update_display

void GraphNodesGroup::update_display()
{   
   if(display_scenegraph_flag)
   {
      double curr_update_time=timefunc::elapsed_timeofday_time();
      if (curr_update_time-prev_update_time > min_elapsed_time_interval &&
          refresh_scenegraph_display_flag)
      {
         prev_update_time=curr_update_time;

         if (curr_update_time > min_elapsed_time_interval) 
            min_elapsed_time_interval=60; // secs

         TreeVisitor_ptr->purge_traversal_history();
         TreeVisitor_ptr->get_DataNode_ptr()->accept(*TreeVisitor_ptr);
//      TreeVisitor_ptr->print_traversal_history();
         TreeVisitor_ptr->set_scenegraph_updated_flag(false);

         destroy_all_Graphicals();
         generate_Graph_from_tree();
         string banner="At t = "+stringfunc::number_to_string(curr_update_time)+
            " secs, scene graph contains "+
            stringfunc::number_to_string(get_n_Graphicals())+" nodes";
         outputfunc::write_banner(banner);

// FAKE FAKE: For earth debugging purposes, we do not want to wait
// while scenegraph is updated.  So update just once and never again
// as of Friday, Nov 17 at 12:34 pm...

//      refresh_scenegraph_display_flag=false;
      }

      display_curr_visited_node();
   } // display_scenegraph_flag conditional
   
   GraphicalsGroup::update_display();
}

// --------------------------------------------------------------------------
// Member function destroy_GraphNode removes the selected GraphNode
// from the GraphNodeslist and the OSG group.  If the GraphNode is
// successfully destroyed, its number is returned by this method.
// Otherwise, -1 is returned.

int GraphNodesGroup::destroy_GraphNode()
{   
   cout << "inside GraphNodesGroup::destroy_GraphNode()" << endl;

// FAKE FAKE: Fri Nov 10 at 8:45 am...hardwire in last GraphNode to be
// destroyed...

   int n=get_n_Graphicals()-1;
   GraphNode* curr_GraphNode_ptr=get_GraphNode_ptr(n);
   LineSegmentsGroup* LineSegmentsGroup_ptr=
      curr_GraphNode_ptr->get_LineSegmentsGroup_ptr();

// Note added on 1/17/07: We must change this next line sometime in
// the future so that LSG->OSGgroup_ptr is deleted from 
// GraphNodesgroup->OSGsubPAT(0) rather than from 
// GraphNodesGroup->OSGgroup_ptr.  See note above dataed 1/17/07...

   get_OSGgroup_ptr()->removeChild(LineSegmentsGroup_ptr->get_OSGgroup_ptr());

   int destroyed_GraphNode_number=destroy_Graphical(n);
   return destroyed_GraphNode_number;
}

// ==========================================================================
// Animation methods
// ==========================================================================

// Member function display_curr_visited_node temporarily colors red
// the node indexed by member integer curr_visited_node_number.  It
// also resets the node indexed by member integer
// prev_visited_node_number back to its permanent color.

void GraphNodesGroup::display_curr_visited_node()
{   
   vector<int> prev_node_total_indices=
      TreeVisitor_ptr->get_traversal_history().at(prev_visited_node_number);
   vector<int> curr_node_total_indices=
      TreeVisitor_ptr->get_traversal_history().at(curr_visited_node_number);

   Treenode<data_type>* prev_Treenode_ptr=
      tree_ptr->get_total_indices_labeled_Treenode_ptr(
         prev_node_total_indices);
   int prev_ID=prev_Treenode_ptr->get_ID();

   Treenode<data_type>* curr_Treenode_ptr=
      tree_ptr->get_total_indices_labeled_Treenode_ptr(
         curr_node_total_indices);
   int curr_ID=curr_Treenode_ptr->get_ID();

   GraphNode* prev_GraphNode_ptr=get_ID_labeled_GraphNode_ptr(prev_ID);
   GraphNode* curr_GraphNode_ptr=get_ID_labeled_GraphNode_ptr(curr_ID);
   prev_GraphNode_ptr->set_curr_color(
      prev_GraphNode_ptr->get_permanent_color());
   curr_GraphNode_ptr->set_curr_color(
      colorfunc::get_OSG_color(colorfunc::red));
   reset_colors();

   prev_visited_node_number=curr_visited_node_number;
   curr_visited_node_number++;
   if (curr_visited_node_number==
       TreeVisitor_ptr->get_traversal_history().size())
   {
      curr_visited_node_number=0;
   }
}
