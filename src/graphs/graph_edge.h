// ==========================================================================
// Header file for graph_edge class
// ==========================================================================
// Last modified on 5/29/10; 2/16/11; 7/28/11
// ==========================================================================

#ifndef GRAPH_EDGE_H
#define GRAPH_EDGE_H

#include "color/colorfuncs.h"

class node;

class graph_edge
{

  public:

   graph_edge(node* n1_ptr,node* n2_ptr,int ID);
   graph_edge(const graph_edge& e);
   ~graph_edge();
   graph_edge& operator= (const graph_edge& e);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const graph_edge& e);

// Set and get methods:

   void set_directed_flag(bool flag);
   bool get_directed_flag() const;
   void set_ID(int id);
   int get_ID() const;
   void set_weight(double w);
   double get_weight() const;
   void set_relative_size(double s);
   double get_relative_size() const;
   void set_hex_color(std::string hc);
   std::string get_hex_color() const;
   void set_edge_RGB(colorfunc::RGB rgb);
   colorfunc::RGB get_edge_RGB() const;

   void set_node_ptrs(node* n1_ptr,node* n2_ptr);
   node* get_node1_ptr();
   const node* get_node1_ptr() const;
   node* get_node2_ptr();
   const node* get_node2_ptr() const;

   double get_centrality() const;

  private: 

   bool directed_flag;
   int ID;
   double weight,centrality,relative_size;
   std::string hex_color;
   colorfunc::RGB edge_RGB;
   node *node1_ptr, *node2_ptr;
   
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const graph_edge& e);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void graph_edge::set_directed_flag(bool flag)
{
   directed_flag=flag;
}

inline bool graph_edge::get_directed_flag() const
{
   return directed_flag;
}

inline void graph_edge::set_ID(int id)
{
   ID=id;
}

inline int graph_edge::get_ID() const
{
   return ID;
}

inline void graph_edge::set_weight(double w)
{
   weight=w;
}

inline double graph_edge::get_weight() const
{
   return weight;
}

inline void graph_edge::set_relative_size(double s)
{
   relative_size=s;
}

inline double graph_edge::get_relative_size() const
{
   return relative_size;
}

inline void graph_edge::set_hex_color(std::string hc)
{
   hex_color=hc;
}

inline std::string graph_edge::get_hex_color() const
{
   return hex_color;
}

inline void graph_edge::set_edge_RGB(colorfunc::RGB rgb)
{
   edge_RGB=rgb;
}

inline colorfunc::RGB graph_edge::get_edge_RGB() const
{
   return edge_RGB;
}


#endif  // graph_edge.h
