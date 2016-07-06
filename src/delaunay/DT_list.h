// ==========================================================================
// Header file for DT_LIST class
// ==========================================================================
// Last modified on 12/14/05
// ==========================================================================

#ifndef DT_LIST_H
#define DT_LIST_H

class DT_node;

class DT_list
{
   public :

      DT_list(DT_list* l, DT_node* k);
      ~DT_list();

//      friend class DT_node;
//      friend class Delaunay_tree;

      DT_list*		next;
      DT_node*		key;

   private :
};

#endif  // DT_LIST_H
