// ==========================================================================
// Header file for node class
// ==========================================================================
// Last modified on 7/29/11; 2/27/12; 8/9/15; 12/1/15
// ==========================================================================

#ifndef NODE_H
#define NODE_H

#include <map>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "graphs/graph.h"
#include "graphs/stlastar.h"
#include "math/twovector.h"

class graph_edge;

class node
{

  public:

   enum VISITED_STATUS
      {
         current,unvisited,visited
      };

   typedef std::map<int,double> NEIGHBORS_MAP;
//    Independent var = node_ID
//    Dependent var = edge weight

   node();
   node(int ID,int level=0);
   node(const node& n);
   ~node();
   node& operator= (const node& n);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const node& n);

// Set and get member functions:

   void set_ID(int id);
   int get_ID() const;
   void set_data_ID(int id);
   int get_data_ID() const;
   void set_level(int l);
   int get_level() const;
   void set_source_ID(int id);
   int get_source_ID() const;
   void set_parent_ID(int id);
   int get_parent_ID() const;
   void set_representative_child_ID(int id);
   int get_representative_child_ID() const;
   void set_color_ID(int id);
   int get_color_ID() const;
   void set_hex_color(std::string hc);
   std::string get_hex_color() const;
   void set_label(std::string label);
   std::string get_label() const;

   void set_Uposn(double u);
   void set_Vposn(double u);
   double get_Uposn() const;
   double get_Vposn() const;

   void set_U2posn(double u);
   void set_V2posn(double u);
   double get_U2posn() const;
   double get_V2posn() const;

   void set_posn(const twovector& UV);
   twovector get_posn();
   const twovector get_posn() const;

   double get_degree() const;
   void set_centrality(double c);
   double get_centrality() const;
   void set_distance_from_start(double d);
   double get_distance_from_start() const;
   void set_relative_size(double r);
   double get_relative_size() const;
   void set_visited_status(VISITED_STATUS v);
   VISITED_STATUS get_visited_status() const;

   void set_path_predecessor_ptr(node* n_ptr);
   node* get_path_predecessor_ptr();
   const node* get_path_predecessor_ptr() const;

   std::vector<graph_edge*> get_graph_edge_ptrs();
   void set_node_RGB(colorfunc::RGB rgb);
   colorfunc::RGB get_node_RGB() const;

   int get_n_children() const;
   void add_child_node_ID(int ID);
   void set_children_node_IDs(const std::vector<int>& IDs);
   std::vector<int>& get_children_node_IDs(); 
   const std::vector<int>& get_children_node_IDs() const;
   int get_n_edges() const;

   void set_graph_ptr(graph* g_ptr);

// Node neighbor member functions:

   void pushback_edge_ptr(graph_edge* e_ptr);
   int get_neighbor_node_ID(graph_edge* e_ptr) const;
   bool is_neighboring_node(int node_ID);
   graph_edge* get_graph_edge_ptr(int neighboring_node_ID);
   std::vector<int> get_neighbor_node_IDs() const;
   std::vector<int> get_sorted_neighbor_node_IDs() const;
   int get_n_neighbors() const;

// Node coloring member functions:

   void set_pure_hue_color(double hue);

// A* search member functions:

   bool IsGoal( node& nodeGoal );
   bool IsSameState( node& rhs );
   double GetCost( node& successor );
   double GoalDistanceEstimate( node& nodeGoal );
   bool GetSuccessors( 
      AStarSearch<node>* astarsearch, node* parent_node_ptr );

  private: 

   int ID,data_ID,level;
   int source_ID,parent_ID,color_ID,representative_child_ID;
   double Uposn,Vposn;
   double U2posn,V2posn;
   double degree,centrality;
   double distance_from_start;
   double relative_size;
   VISITED_STATUS visited_status;
   std::string hex_color,label;
   colorfunc::RGB node_RGB;
   NEIGHBORS_MAP neighboring_node_IDs;
   std::vector<graph_edge*> graph_edge_ptrs;
   std::vector<int> children_node_IDs;
   node* path_predecessor_ptr;
   graph* graph_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const node& n);

   void update_degree();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void node::set_ID(int id)
{
   ID=id;
}

inline int node::get_ID() const
{
   return ID;
}

inline void node::set_data_ID(int id)
{
   data_ID=id;
}

inline int node::get_data_ID() const
{
   return data_ID;
}

inline void node::set_level(int l)
{
   level=l;
}

inline int node::get_level() const
{
   return level;
}

inline void node::set_source_ID(int id)
{
   source_ID=id;
}

inline int node::get_source_ID() const
{
   return source_ID;
}

inline void node::set_parent_ID(int id)
{
   parent_ID=id;
}

inline int node::get_parent_ID() const
{
   return parent_ID;
}

inline void node::set_representative_child_ID(int id)
{
   representative_child_ID=id;
}

inline int node::get_representative_child_ID() const
{
   return representative_child_ID;
}

inline void node::set_color_ID(int id)
{
   color_ID=id;
}

inline int node::get_color_ID() const
{
   return color_ID;
}

inline void node::set_Uposn(double u)
{
   Uposn=u;
}

inline void node::set_Vposn(double v)
{
   Vposn=v;
}

inline double node::get_Uposn() const
{
   return Uposn;
}

inline double node::get_Vposn() const
{
   return Vposn;
}

inline void node::set_U2posn(double u)
{
   U2posn=u;
}

inline void node::set_V2posn(double v)
{
   V2posn=v;
}

inline double node::get_U2posn() const
{
   return U2posn;
}

inline double node::get_V2posn() const
{
   return V2posn;
}

inline void node::set_posn(const twovector& UV)
{
   Uposn=UV.get(0);
   Vposn=UV.get(1);
}

inline twovector node::get_posn()
{
   return twovector(Uposn,Vposn);
}

inline const twovector node::get_posn() const
{
   return twovector(Uposn,Vposn);
}

inline double node::get_degree() const
{
   return degree;
}

inline void node::set_centrality(double c)
{
   centrality=c;
}

inline double node::get_centrality() const
{
   return centrality;
}

inline void node::set_distance_from_start(double d)
{
   distance_from_start=d;
}

inline double node::get_distance_from_start() const
{
   return distance_from_start;
}

inline void node::set_relative_size(double r)
{
   relative_size=r;
}

inline double node::get_relative_size() const
{
   return relative_size;
}

inline void node::set_visited_status(node::VISITED_STATUS v)
{
   visited_status=v;
}

inline node::VISITED_STATUS node::get_visited_status() const
{
   return visited_status;
}

inline void node::set_path_predecessor_ptr(node* n_ptr)
{
   path_predecessor_ptr=n_ptr;
}

inline node* node::get_path_predecessor_ptr() 
{
   return path_predecessor_ptr;
}

inline const node* node::get_path_predecessor_ptr() const
{
   return path_predecessor_ptr;
}


inline int node::get_n_edges() const
{
   return graph_edge_ptrs.size();
}

inline std::vector<graph_edge*> node::get_graph_edge_ptrs()
{
   return graph_edge_ptrs;
}

inline int node::get_n_children() const
{
   return children_node_IDs.size();
}

inline void node::add_child_node_ID(int ID)
{
   return children_node_IDs.push_back(ID);
}

inline void node::set_children_node_IDs(const std::vector<int>& IDs)
{
   children_node_IDs=IDs;
}

inline std::vector<int>& node::get_children_node_IDs() 
{
   return children_node_IDs;
}

inline const std::vector<int>& node::get_children_node_IDs() const
{
   return children_node_IDs;
}

inline void node::set_node_RGB(colorfunc::RGB rgb)
{
   node_RGB=rgb;
}

inline colorfunc::RGB node::get_node_RGB() const
{
   return node_RGB;
}

inline void node::set_graph_ptr(graph* g_ptr)
{
   graph_ptr=g_ptr;
}


#endif  // node.h
