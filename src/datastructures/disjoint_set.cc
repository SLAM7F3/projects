// =========================================================================
// disjoint_set class member function definitions
// =========================================================================
// Last modified on 6/27/14
// =========================================================================

#include <cstdlib>
#include <iostream>
#include "datastructures/disjoint_set.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void disjoint_set::allocate_member_objects()
{
}

void disjoint_set::initialize_member_objects()
{
}		 

// ---------------------------------------------------------------------
disjoint_set::disjoint_set(unsigned int n)
{
   allocate_member_objects();
   initialize_member_objects();

   for (unsigned int i=0; i<n; i++)
   {
      ds_node_t curr_node;
      curr_node.parent=i;
      ds.push_back(curr_node);
   }
}

// ---------------------------------------------------------------------
// Copy constructor:

disjoint_set::disjoint_set(const disjoint_set& ds)
{
   docopy(ds);
}

disjoint_set::~disjoint_set()
{
   ds.clear();
}

// ---------------------------------------------------------------------
void disjoint_set::docopy(const disjoint_set& ds)
{
}

// Overload = operator:

disjoint_set& disjoint_set::operator= (const disjoint_set& ds)
{
   if (this==&ds) return *this;
   docopy(ds);
   return *this;
}

// ---------------------------------------------------------------------
unsigned int disjoint_set::size() const
{
   return ds.size();
}

// ---------------------------------------------------------------------

unsigned int disjoint_set::find(unsigned int i)
{
   if (i >= size())
   {
      cout << "Error in disjoint_set::find()!" << endl;
      cout << "i = " << i << " should be less than size = " << size() << endl;
      exit(-1);
   }

   ds_node_t curr_node = ds[i];

   if(curr_node.parent != i)
      curr_node.parent = find(curr_node.parent);

   return curr_node.parent;
}


// ---------------------------------------------------------------------

unsigned int disjoint_set::link(unsigned int i,unsigned int j)
{
   if (i >= size())
   {
      cout << "Error in disjoint_set::;link()!" << endl;
      cout << "i = " << i << " should be less than size = " << size() << endl;
      exit(-1);
   }

   if (j >= size())
   {
      cout << "Error in disjoint_set::link()!" << endl;
      cout << "j = " << j << " should be less than size = " << size() << endl;
      exit(-1);
   }


   unsigned int iroot = find(i);
   unsigned int jroot = find(j);

   if (iroot == jroot) return iroot;

   ds_node_t* iroot_node = &ds[iroot];
   ds_node_t* jroot_node = &ds[jroot];

   if (iroot_node->rank < jroot_node->rank)
   {
      iroot_node->parent = jroot;
      return jroot;
   }
   else if (iroot_node->rank > jroot_node->rank)
   {
      jroot_node->parent = iroot;
      return iroot;
   }
   else{
      jroot_node->parent = iroot;
      ++iroot_node->rank;
      return iroot;
   }
}

// ---------------------------------------------------------------------

void disjoint_set::initialize_CCs()
{
   for (unsigned int c=0; c<CCs.size(); c++)
   {
      CCs[c].clear();
   }
}

// ---------------------------------------------------------------------
// Member function form_CCs() groups together individual elements into
// equivalence classes

vector<vector<unsigned int> >& disjoint_set::form_CCs()
{
   initialize_CCs();

   unsigned int n = size();
   vector<unsigned int> mark;
   for (unsigned int i=0; i<n; i++)
   {
      mark.push_back(0);
   }
    
   for (unsigned int i = 0; i < n; i++)
   {
      if (mark[i]==0)
      {
         vector<unsigned int> curr_list;
         curr_list.push_back(i);
         mark[i] = 1;

         for (unsigned int j=i+1; j<n; j++)
         {
            if (mark[j]==0 && find(i)==find(j))
            {
               curr_list.push_back(j);
               mark[j] = 1;
            }
         }

         CCs.push_back(curr_list);

      } // mark[i] == 0 conditional
   } // loop over index i 

   return CCs;
}

// ---------------------------------------------------------------------
void disjoint_set::print_CCs()
{
   for (unsigned int c=0; c<CCs.size(); c++)
   {
      cout << "CC = " << c << " : " << flush;
      for (unsigned int i=0; i<CCs[c].size(); i++)
      {
         cout << CCs[c].at(i) << " " << flush;
      }
      cout << endl;
   }
}
