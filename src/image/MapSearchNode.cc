// =========================================================================
// MapSearchNode class member function definitions
// =========================================================================
// Last modified on 12/4/10; 12/5/10; 6/19/11; 6/29/11
// =========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "image/MapSearchNode.h"
#include "general/outputfuncs.h"


using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void MapSearchNode::allocate_member_objects()
{
}

void MapSearchNode::initialize_member_objects()
{
   skip=1;
   set_px(0);
   set_py(0);
   set_vertical_displacement_term_weight(20);
   set_zfrac_term_weight(0);
   zmin=0;
   zmax=1;
   z=NEGATIVEINFINITY;
   ztwoDarray_ptr=NULL;
}

MapSearchNode::MapSearchNode()
{ 
   allocate_member_objects();
   initialize_member_objects();
}

MapSearchNode::MapSearchNode(int skip,twoDarray* ztwoDarray_ptr)
{ 
   allocate_member_objects();
   initialize_member_objects();

   this->skip=skip;
   this->ztwoDarray_ptr=ztwoDarray_ptr;
   mdim=ztwoDarray_ptr->get_mdim();
   ndim=ztwoDarray_ptr->get_ndim();
   delta_x=skip*ztwoDarray_ptr->get_deltax();
}

// ---------------------------------------------------------------------
MapSearchNode::MapSearchNode(
   int px,int py,int skip,
   double alpha_term_weight,double beta_term_weight,
   double zmin,double zmax,twoDarray* ztwoDarray_ptr)
{ 
   allocate_member_objects();
   initialize_member_objects();

   this->skip=skip;
   set_vertical_displacement_term_weight(alpha_term_weight);
   set_zfrac_term_weight(beta_term_weight);
   this->zmin=zmin;
   this->zmax=zmax;
   this->ztwoDarray_ptr=ztwoDarray_ptr;
   mdim=ztwoDarray_ptr->get_mdim();
   ndim=ztwoDarray_ptr->get_ndim();
   delta_x=skip*ztwoDarray_ptr->get_deltax();

   if (ztwoDarray_ptr->pixel_inside_working_region(px,py))
   {
      set_px(px);
      set_py(py);
      this->ztwoDarray_ptr=ztwoDarray_ptr;
   }
   else
   {
      cout << "Error in MapSearchNode constructor #2!" << endl;
      cout << "mdim = " << mdim << " ndim = " << ndim << endl;
      cout << "px = " << px << " py = " << py << " does NOT lie inside DTED map"
           << endl;
      outputfunc::enter_continue_char();
   }
}

// ---------------------------------------------------------------------
void MapSearchNode::PrintNodeInfo()
{
   if (ztwoDarray_ptr->pixel_to_point(px,py,x,y))
   {
      z=ztwoDarray_ptr->get(px,py);
   }

   cout << "x = " << get_x() 
        << " y = " << get_y() 
        << " z = " << get_z() << endl;
}

// ---------------------------------------------------------------------
// This generates the successors to the given Node. It uses a helper
// function called AddSuccessor to give the successors to the AStar
// class. The A* specific initialisation is done for each node
// internally, so here you just set the state information that is
// specific to the application

bool MapSearchNode::GetSuccessors( 
   AStarSearch<MapSearchNode>* astarsearch, MapSearchNode* parent_node )
{
   int parent_px = -1; 
   int parent_py = -1; 

   if ( parent_node != NULL )
   {
      parent_px = parent_node->get_px();
      parent_py = parent_node->get_py();
   }
	
   MapSearchNode NewNode;

// Push each possible move except allowing the search to go backwards

   if (px-skip > 0 && py-skip > 0 && 
	!(parent_px==px-skip && parent_py==py-skip) )
   {
      NewNode = MapSearchNode( 
         px-skip,py-skip,skip,
         vertical_displacement_term_weight,zfrac_term_weight,
         zmin, zmax, ztwoDarray_ptr );
      delta_s=SQRT_TWO*delta_x;
      astarsearch->AddSuccessor( NewNode );
   }

   if (px-skip > 0 && 
        !(parent_px==px-skip && parent_py==py) )
   {
      NewNode = MapSearchNode( 
         px-skip,py,skip,
         vertical_displacement_term_weight,zfrac_term_weight,
         zmin, zmax, ztwoDarray_ptr );
      delta_s=delta_x;
      astarsearch->AddSuccessor( NewNode );
   }

   if (px-skip > 0 && py+skip < ndim && 
	!(parent_px==px-skip && parent_py==py+skip) )
   {
      NewNode = MapSearchNode( 
         px-skip,py+skip,skip,
         vertical_displacement_term_weight,zfrac_term_weight,
         zmin, zmax, ztwoDarray_ptr );
      delta_s=SQRT_TWO*delta_x;
      astarsearch->AddSuccessor( NewNode );
   }
      
   if (py-skip > 0 && 
	!(parent_px==px && parent_py==py-skip) )
   {
      NewNode = MapSearchNode( 
         px,py-skip,skip,
         vertical_displacement_term_weight,zfrac_term_weight,
         zmin, zmax, ztwoDarray_ptr );
      delta_s=delta_x;
      astarsearch->AddSuccessor( NewNode );
   }

   if (py+skip < ndim && 
	!(parent_px==px && parent_py==py+skip) )
   {
      NewNode = MapSearchNode( 
         px,py+skip,skip,
         vertical_displacement_term_weight,zfrac_term_weight,
         zmin, zmax, ztwoDarray_ptr );
      delta_s=delta_x;
      astarsearch->AddSuccessor( NewNode );
   }

   if (px+skip < mdim && py-skip > 0 && 
	!(parent_px==px+skip && parent_py==py-skip) )
   {
      NewNode = MapSearchNode( 
         px+skip,py-skip,skip,
         vertical_displacement_term_weight,zfrac_term_weight,
         zmin, zmax, ztwoDarray_ptr );
      delta_s=SQRT_TWO*delta_x;
      astarsearch->AddSuccessor( NewNode );
   }

   if (px+skip < mdim && 
	!(parent_px==px+skip && parent_py==py) )
   {
      NewNode = MapSearchNode( 
         px+skip,py,skip,
         vertical_displacement_term_weight,zfrac_term_weight,
         zmin, zmax, ztwoDarray_ptr );
      delta_s=delta_x;
      astarsearch->AddSuccessor( NewNode );
   }

   if (px+skip < mdim && py+skip < ndim && 
	!(parent_px==px+skip && parent_py==py+skip) )
   {
      NewNode = MapSearchNode( 
         px+skip,py+skip,skip,
         vertical_displacement_term_weight,zfrac_term_weight,
         zmin, zmax, ztwoDarray_ptr );
      delta_s=SQRT_TWO*delta_x;
      astarsearch->AddSuccessor( NewNode );
   }

   return true;
}

// ---------------------------------------------------------------------
// Member function GetCost() evaluates a dimensionless cost function
// involving 3 terms for moving from the current (X,Y,Z) position
// within the world map to a candidate successor location.

double MapSearchNode::GetCost( MapSearchNode& successor )
{
//   cout << "inside MapSearchNode::GetCost()" << endl;
//   cout << "skip = " << skip << endl;
//   cout << "successor = " << endl;
//   successor.PrintNodeInfo();

// Set edge weight between node and neighbor_node using functional
// form suggested in eqn 2 of "Least-cost paths in mountainous
// terrain" Computers & Geosciences 30 (2004) 203-209 by W.G. Rees:

   double ds=sqrt(sqr(successor.get_x()-get_x())+
                  sqr(successor.get_y()-get_y()));
//   double ds=delta_s;
//   cout << "ds = " << ds << endl;
   double dz=successor.get_z()-get_z();

   double term1=ds/skip;
   double term2=sqr(dz)/(ds*skip);

   const double max_alt=3520;	// meters  (around FOB Blessing)
   const double min_alt=1150;	// meters  (around FOB Blessing)
   double avg_alt=0.5*(max_alt+min_alt);
   double ratio=(successor.get_z()-avg_alt)/250;
//   double ratio=(successor.get_z()-avg_alt)/500;

/*
   double zfrac=(successor.get_z()-min_alt)/(max_alt-min_alt);
   if (zfrac < 0.01) zfrac=0.01;
   if (zfrac > 0.99) zfrac=0.99;
*/

//   double zfrac=(successor.get_z()-zmin)/(zmax-zmin);
//   double term3=(1-(sqr(sqr(zfrac*(1-zfrac))))) * ds/skip;
//   double term3=sqr(sqr(zfrac)) * ds/skip;
   double term3=sqr(ratio) * ds/skip;

//   double term3=zfrac * ds/skip;
//   double term3=zfrac*(1-zfrac) * ds/skip;

//   double step_cost=term1+vertical_displacement_term_weight*term2;
   double step_cost=term1+vertical_displacement_term_weight*term2
      +zfrac_term_weight*term3;

//   cout << "ds/skip term1 = " << term1
//        << " dz**2/(ds skip) = " << term2 
//        << " zfrac = " << term3 << endl;
//   cout << " step cost = " << step_cost << endl;

   return step_cost;
}

// ---------------------------------------------------------------------
// Here's the heuristic function that supplies a lower bound for the
// path integral of the cost function from the current Node to the
// Goal.

double MapSearchNode::GoalDistanceEstimate( MapSearchNode &nodeGoal )
{
   double delta_x=fabs(get_x()-nodeGoal.get_x());
   double delta_y=fabs(get_y()-nodeGoal.get_y());
   double delta_z=fabs(get_z()-nodeGoal.get_z());
   double delta_rho=sqrt(sqr(delta_x)+sqr(delta_y));
//   double zfrac_start=(get_z()-zmin)/(zmax-zmin);
//   double zfrac_stop=(nodeGoal.get_z()-zmin)/(zmax-zmin);

   double term1=delta_rho/skip;
   double term2=sqr(delta_z)/(delta_rho*skip);
   double term3=0.5*delta_rho/skip;
//   double term3=0.5*fabs(zfrac_start-zfrac_stop)*delta_rho/skip;

//   double goal_cost=term1+vertical_displacement_term_weight*term2;
   double goal_cost=term1+vertical_displacement_term_weight*term2
      +zfrac_term_weight*term3;

//   double goal_cost=ds+vertical_displacement_term_weight*sqr(dz)/ds;
//   cout << "goal cost = " << goal_cost << endl;

   return goal_cost;
}
