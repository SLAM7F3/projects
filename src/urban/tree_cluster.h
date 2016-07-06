// ==========================================================================
// Header file for TREE_CLUSTER class
// ==========================================================================
// Last modified on 4/17/05; 4/23/06; 6/14/06; 7/29/06
// ==========================================================================

#ifndef TREE_CLUSTER_H
#define TREE_CLUSTER_H

#include "geometry/contour_element.h"
class threevector;

class tree_cluster: public contour_element
{

  public:

// Initialization, constructor and destructor functions:

   tree_cluster();
   tree_cluster(int identification);
   tree_cluster(int identification,const threevector& p);
   tree_cluster(const tree_cluster& t);
   virtual ~tree_cluster();
   tree_cluster& operator= (const tree_cluster& t);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const tree_cluster& t);


  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const tree_cluster& t);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif // tree_cluster.h



