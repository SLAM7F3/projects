// ==========================================================================
// Header file for MapSearchNode class
// ==========================================================================
// Last modified on 12/4/10; 12/5/10; 6/19/11; 6/29/11
// ==========================================================================

#ifndef MAPSEARCHNODE_H
#define MAPSEARCHNODE_H

#include "graphs/stlastar.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"

class MapSearchNode
{
  public:
	
   MapSearchNode();
   MapSearchNode(int skip,twoDarray* ztwoDarray_ptr);
   MapSearchNode(
      int px,int py,int skip,
      double alpha_term_weight,double beta_term_weight,
      double zmin,double zmax,
      twoDarray* ztwoDarray_ptr);

// Set & get methods:

   void set_skip(int skip);
   void set_px(int qx);
   void set_py(int qy);
   int get_px() const;
   int get_py() const;
   double get_x();
   double get_y();
   double get_z() const;
   threevector get_posn();
   void set_vertical_displacement_term_weight(double w);
   void set_zfrac_term_weight(double w);

   double GoalDistanceEstimate( MapSearchNode &nodeGoal );
   bool IsGoal( MapSearchNode &nodeGoal );
   bool GetSuccessors( AStarSearch<MapSearchNode>* astarsearch, 
   MapSearchNode* parent_node );
   double GetCost( MapSearchNode &successor );
   bool IsSameState( MapSearchNode &rhs );

   void PrintNodeInfo(); 

  private:

   int px,py,mdim,ndim,skip;
   double x,y,z;
   double zmin,zmax;
   double vertical_displacement_term_weight,zfrac_term_weight;
   double delta_x,delta_s;
   twoDarray* ztwoDarray_ptr;
   void allocate_member_objects();
   void initialize_member_objects();
};

// ---------------------------------------------------------------------
inline void MapSearchNode::set_skip(int skip)
{
   this->skip=skip;
}

inline void MapSearchNode::set_px(int qx)
{
   px=qx;
}

inline void MapSearchNode::set_py(int qy)
{
   py=qy;
}

inline int MapSearchNode::get_px() const
{
   return px;
}

inline int MapSearchNode::get_py() const
{
   return py;
}

inline double MapSearchNode::get_x() 
{
   if (ztwoDarray_ptr != NULL)
   {
      if (ztwoDarray_ptr->px_to_x(px,x))
      {
         return x;
      }
   }
   return NEGATIVEINFINITY;
}

inline double MapSearchNode::get_y() 
{
   if (ztwoDarray_ptr != NULL)
   {
      if (ztwoDarray_ptr->py_to_y(py,y))
      {
         return y;
      }
   }
   return NEGATIVEINFINITY;
}

inline double MapSearchNode::get_z() const 
{
   if (ztwoDarray_ptr != NULL)
   {
      if (ztwoDarray_ptr->pixel_inside_working_region(px,py))
      {
         return ztwoDarray_ptr->get(px,py);
      }
   }
   return NEGATIVEINFINITY;
}

inline threevector MapSearchNode::get_posn()
{
//   std::cout << "inside MapSearchNode::get_posn()" << std::endl;
//   std::cout << "x = " << get_x() << " y = " << get_y() 
//             << " z = " << get_z() << std::endl;
   threevector posn(get_x(),get_y(),get_z());
   return posn;
}

inline void MapSearchNode::set_vertical_displacement_term_weight(double w)
{
   vertical_displacement_term_weight=w;
}

inline void MapSearchNode::set_zfrac_term_weight(double w)
{
   zfrac_term_weight=w;
}

// ---------------------------------------------------------------------
inline bool MapSearchNode::IsSameState( MapSearchNode& rhs )
{
   // same state in a maze search is simply when (px,py) are the same

   return ( px==rhs.get_px() && py==rhs.get_py() );
}

// ---------------------------------------------------------------------
inline bool MapSearchNode::IsGoal( MapSearchNode &nodeGoal )
{
   return (px==nodeGoal.get_px() && py==nodeGoal.get_py());
}

#endif  // MapSearchNode.h
