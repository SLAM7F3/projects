// =========================================================================
// pixelForest class member function definitions
// =========================================================================
// Last modified on 7/22/12
// =========================================================================

#include <iostream>
#include "image/graphicsfuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "image/pixelForest.h"


using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::vector;


// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void pixelForest::allocate_member_objects()
{
   pixelNodes_map_ptr=new PIXELNODES_MAP;
   rootNodes_map_ptr=new PIXELNODES_MAP;
}

void pixelForest::initialize_member_objects()
{
}		 

// ---------------------------------------------------------------------
pixelForest::pixelForest(int xdim,int ydim)
{
   allocate_member_objects();
   initialize_member_objects();

   this->xdim=xdim;
   this->ydim=ydim;
}

// ---------------------------------------------------------------------
// Copy constructor:

pixelForest::pixelForest(const pixelForest& pF)
{
   docopy(pF);
}

pixelForest::~pixelForest()
{
//   cout << "inside pixelForest destructor" << endl;

   purge_pixelNodes();
   delete pixelNodes_map_ptr;
   delete rootNodes_map_ptr;
}

// ---------------------------------------------------------------------
// Member function purge_pixelNodes()

void pixelForest::purge_pixelNodes() 
{
//   cout << "inside pixelForest::purge_pixelNodes()" << endl;

   vector<pixelNode*> pixelNodes_to_delete;

   pixelNode* pixelNode_ptr=reset_pixelNodes_map_ptr();
   while (pixelNode_ptr != NULL)
   {
      pixelNodes_to_delete.push_back(pixelNode_ptr);
      pixelNode_ptr=get_next_pixelNode_ptr();
   }

   int n_pixelNodes=pixelNodes_to_delete.size();
//   cout << "n_pixelNodes to delete = " << n_pixelNodes << endl;
   for (int n=0; n<n_pixelNodes; n++)
   {
      delete pixelNodes_to_delete[n];
   }
   pixelNodes_map_ptr->clear();
}

// ---------------------------------------------------------------------
void pixelForest::docopy(const pixelForest& pF)
{
}

// Overload = operator:

pixelForest& pixelForest::operator= (const pixelForest& pF)
{
   if (this==&pF) return *this;
   docopy(pF);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,pixelForest& pF)
{
   outstream << "xdim = " << pF.xdim
             << " ydim= " << pF.ydim
             << endl;
   outstream << "n_trees = " << pF.get_n_trees() << endl;
   outstream << "n_pixelNodes = " << pF.get_n_pixelNodes() << endl;
   
   pixelNode* rootNode_ptr=pF.reset_rootNodes_map_ptr();
   while (rootNode_ptr != NULL)
   {
      outstream << "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT" << endl;
      pF.recursively_print_pixelNodes(outstream,rootNode_ptr);
      rootNode_ptr=pF.get_next_rootNode_ptr();
   }

   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

int pixelForest::get_n_trees() const
{
   return rootNodes_map_ptr->size();
}

int pixelForest::get_n_pixelNodes() const
{
   return pixelNodes_map_ptr->size();
}

// ---------------------------------------------------------------------
void pixelForest::compute_four_neighbor_pixel_IDs(int px,int py)
{
   four_neighbor_pixel_IDs.clear();
   
   if (py >= 1)
   {
      four_neighbor_pixel_IDs.push_back(
         graphicsfunc::get_pixel_ID(px,py-1,xdim));
   }
   if (py <= ydim-2)
   {
      four_neighbor_pixel_IDs.push_back(
         graphicsfunc::get_pixel_ID(px,py+1,xdim));
   }
   if (px >= 1)
   {
      four_neighbor_pixel_IDs.push_back(
         graphicsfunc::get_pixel_ID(px-1,py,xdim));
   }
   if (px <= xdim-2)
   {
      four_neighbor_pixel_IDs.push_back(
         graphicsfunc::get_pixel_ID(px+1,py,xdim));
   }
}

// ==========================================================================
// RootNode iteration member functions
// ==========================================================================

void pixelForest::add_rootNode_ptr(pixelNode* rootNode_ptr)
{
   (*rootNodes_map_ptr)[rootNode_ptr->get_ID()]=rootNode_ptr;
   add_pixelNode_ptr(rootNode_ptr);
}

// ---------------------------------------------------------------------
pixelNode* pixelForest::reset_rootNodes_map_ptr()
{
   rootNode_iter=rootNodes_map_ptr->begin();
   if (rootNode_iter==rootNodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return rootNode_iter->second;
   }
}

// ---------------------------------------------------------------------
pixelNode* pixelForest::get_next_rootNode_ptr()
{
   rootNode_iter++;
   if (rootNode_iter==rootNodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return rootNode_iter->second;
   }
}

// ==========================================================================
// PixelNode retrieval member functions
// ==========================================================================

void pixelForest::add_pixelNode_ptr(pixelNode* pixelNode_ptr)
{
   (*pixelNodes_map_ptr)[pixelNode_ptr->get_ID()]=pixelNode_ptr;
}

// ---------------------------------------------------------------------
pixelNode* pixelForest::get_pixelNode_ptr(int ID)
{
   PIXELNODES_MAP::iterator pixel_node_iter=pixelNodes_map_ptr->find(ID);
   if (pixel_node_iter == pixelNodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return pixel_node_iter->second;
   }
}

// ---------------------------------------------------------------------
pixelNode* pixelForest::reset_pixelNodes_map_ptr()
{
   pixelNode_iter=pixelNodes_map_ptr->begin();
   if (pixelNode_iter==pixelNodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return pixelNode_iter->second;
   }
}

// ---------------------------------------------------------------------
pixelNode* pixelForest::get_next_pixelNode_ptr()
{
//   cout << "inside pixelForest::get_next_pixelNode_ptr()" << endl;

   pixelNode_iter++;
   if (pixelNode_iter==pixelNodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
//      cout << "pixelNode ID = " << pixelNode_iter->first << endl;
      return pixelNode_iter->second;
   }
}
      
// ==========================================================================
// Union-Find algorithm member functions
// ==========================================================================

// Member function MakeSet()

void pixelForest::MakeSet(pixelNode* pixelNode_ptr)
{
//   cout << "inside pixelForest::MakeSet(), pixelNode_ptr = " 
//        << pixelNode_ptr << endl;
   
   pixelNode_ptr->set_parent_node_ptr(pixelNode_ptr);
   pixelNode_ptr->set_rank(0);

   add_rootNode_ptr(pixelNode_ptr);
}

// ---------------------------------------------------------------------
// Member function Find()

pixelNode* pixelForest::Find(pixelNode* pixelNode_ptr)
{
//   cout << "inside pixelForest::Find()" << endl;
   
   pixelNode* parent_node_ptr=pixelNode_ptr->get_parent_node_ptr();
   if (parent_node_ptr != pixelNode_ptr)
   {
      parent_node_ptr=Find(parent_node_ptr);
   }
   return parent_node_ptr;
}

// ---------------------------------------------------------------------
// Member function Link()

pixelNode* pixelForest::Link(
   pixelNode* pixelNode1_ptr,pixelNode* pixelNode2_ptr)
{
//   std::cout << "inside pixelForest::Link()" << std::endl;
//   std::cout << "n_trees = " << get_n_trees() << std::endl;

//   int rank1=pixelNode1_ptr->get_rank();
//   int rank2=pixelNode2_ptr->get_rank();

   if (pixelNode1_ptr->get_rank() > pixelNode2_ptr->get_rank())
   {
      templatefunc::swap(pixelNode1_ptr,pixelNode2_ptr);
   }

   if (pixelNode1_ptr->get_rank()==pixelNode2_ptr->get_rank())
   {
      pixelNode2_ptr->set_rank(pixelNode2_ptr->get_rank()+1);
   }

// Erase pixelNode1_ptr from *rootNodes_map_ptr if it exists within
// this STL map:

   erase_rootNode(pixelNode1_ptr);
//   erase_rootNode(pixelNode2_ptr);

// Set pixelNode2 as parent for pixelNode1:

   pixelNode1_ptr->set_parent_node_ptr(pixelNode2_ptr);

// As of 7/18/12, we *think* that pixelNode1_ptr should not have itself
// as a child once pixelNode2_ptr becomes its parent...

   pixelNode1_ptr->deleteChild(pixelNode1_ptr);

//   (*rootNodes_map_ptr)[pixelNode2_ptr->get_ID()]=pixelNode2_ptr;   

//   std::cout << "n_trees = " << get_n_trees() << std::endl;
//   outputfunc::enter_continue_char();

   return pixelNode2_ptr;
}

// ==========================================================================
// Recursive tree member functions
// ==========================================================================

// Member function recursively_count_nodes()

int pixelForest::recursively_count_nodes(pixelNode* rootNode_ptr)
{
//   std::cout << "inside pixelForest::recursively_count_nodes()" << std::endl;

   int n_nodes=0;
   recursively_count_nodes(rootNode_ptr,n_nodes);
   return n_nodes;
}

// ---------------------------------------------------------------------
void pixelForest::recursively_count_nodes(
   pixelNode* input_pixelNode_ptr,int& n_nodes)
{
//   cout << "inside pixelForest::recursively_count_nodes() #2" << endl;

   pixelNode* child_pixelNode_ptr=
      input_pixelNode_ptr->reset_child_pixelNode_ptr();
   
// Call recursively_count_nodes() on all children nodes of
// *input_pixelNode_ptr (which do NOT equal input_pixelNode_ptr!):

   while (child_pixelNode_ptr != NULL)
   {
      if (child_pixelNode_ptr != input_pixelNode_ptr)
      {
         recursively_count_nodes(child_pixelNode_ptr,n_nodes);
      }
      child_pixelNode_ptr=input_pixelNode_ptr->get_next_child_pixelNode_ptr();
   }
   n_nodes=n_nodes+1;
}

// ---------------------------------------------------------------------
// Member function erase_rootNode() 

void pixelForest::erase_rootNode(pixelNode* rootNode_ptr)
{
//   cout << "inside pixelForest::erase_rootNode()" 
//        << endl;
   
   rootNode_iter=rootNodes_map_ptr->find(rootNode_ptr->get_ID());
   if (rootNode_iter != rootNodes_map_ptr->end())
   {
      rootNodes_map_ptr->erase(rootNode_iter);
   }
}

// ---------------------------------------------------------------------
// Member function recursively_destroy_pixelNodes() 

void pixelForest::recursively_destroy_pixelNodes(pixelNode* rootNode_ptr)
{
//   cout << "inside pixelForest::recursively_destroy_pixelNodes()" 
//        << endl;

// First erase rootnode from *rootnodes_map_ptr:

   erase_rootNode(rootNode_ptr);

// Next recursively destroy tree with *rootNode_ptr as its root:

   recursively_destroy_pixel_nodes(rootNode_ptr);
}

// ---------------------------------------------------------------------
void pixelForest::recursively_destroy_pixel_nodes(
   pixelNode* input_pixelNode_ptr)
{
   pixelNode* child_pixelNode_ptr=
      input_pixelNode_ptr->reset_child_pixelNode_ptr();
   
// Call recursively_destroy_pixelNodes() on all children nodes of
// *input_pixelNode_ptr (which do NOT equal input_pixelNode_ptr!):

   while (child_pixelNode_ptr != NULL)
   {
      if (child_pixelNode_ptr != input_pixelNode_ptr)
      {
         recursively_destroy_pixel_nodes(child_pixelNode_ptr);
      }
      child_pixelNode_ptr=input_pixelNode_ptr->get_next_child_pixelNode_ptr();
   }
   
   delete input_pixelNode_ptr;
}

// ---------------------------------------------------------------------
void pixelForest::recursively_print_pixelNodes(
   ostream& outstream,pixelNode* input_pixelNode_ptr)
{
   pixelNode* child_pixelNode_ptr=
      input_pixelNode_ptr->reset_child_pixelNode_ptr();
   
// Call recursively_print_pixelNodes() on all children nodes of
// *input_pixelNode_ptr (which do NOT equal input_pixelNode_ptr!):

   while (child_pixelNode_ptr != NULL)
   {
      if (child_pixelNode_ptr != input_pixelNode_ptr)
      {
         recursively_print_pixelNodes(outstream,child_pixelNode_ptr);
      }
      child_pixelNode_ptr=input_pixelNode_ptr->get_next_child_pixelNode_ptr();
   }
   
   outstream << *input_pixelNode_ptr << endl;
}

