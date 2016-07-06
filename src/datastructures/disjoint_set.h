// ==========================================================================
// Header file for disjoint_set class
// ==========================================================================
// Last modified on 6/27/14
// ==========================================================================

#ifndef DISJOINT_SET
#define DISJOINT_SET

#include <vector>

// disjoint_set node

typedef struct {
   unsigned int rank;
   unsigned int parent;
} ds_node_t;

class disjoint_set
{
  
  public:

   disjoint_set(unsigned int n);
   disjoint_set(const disjoint_set& ds);
   ~disjoint_set();
   disjoint_set& operator= (const disjoint_set& ds);

   unsigned int size() const;
   unsigned int find(unsigned int i);
   unsigned int link(unsigned int i, unsigned int j);

   std::vector<std::vector<unsigned int> >& form_CCs();
   void print_CCs();

  private:

   std::vector<ds_node_t> ds;

// Connected components of nodes which belong to same equivalence class:

   std::vector<std::vector<unsigned int> > CCs;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const disjoint_set& ds);

   void initialize_CCs();
};



#endif /* DISJOINT_SET_H_ */
