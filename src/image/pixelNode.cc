// =========================================================================
// PixelNode class member function definitions
// =========================================================================
// Last modified on 7/22/12
// =========================================================================

#include <iostream>
#include "image/pixelNode.h"
#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ostream;
using std::pair;
using std::vector;


// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void pixelNode::allocate_member_objects()
{
   child_nodes_map_ptr=new PIXELNODES_MAP;
}

void pixelNode::initialize_member_objects()
{
   px=py=ID=-1;
   rank=0;
   intensity=-1;
   parent_node_ptr=NULL;
}		 

// ---------------------------------------------------------------------
pixelNode::pixelNode()
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
pixelNode::pixelNode(int px,int py,int ID)
{
   allocate_member_objects();
   initialize_member_objects();
   this->px=px;
   this->py=py;
   this->ID=ID;
}

// ---------------------------------------------------------------------
// Copy constructor:

pixelNode::pixelNode(const pixelNode& pN)
{
   docopy(pN);
}

pixelNode::~pixelNode()
{
//   cout << "inside pixelNode destructor" << endl;

   delete child_nodes_map_ptr;
   parent_node_ptr=NULL;
}

// ---------------------------------------------------------------------
void pixelNode::docopy(const pixelNode& pN)
{
}

// Overload = operator:

pixelNode& pixelNode::operator= (const pixelNode& pN)
{
   if (this==&pN) return *this;
   docopy(pN);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,pixelNode& pN)
{
   outstream << "px = " << pN.get_px() 
             << " py = " << pN.get_py() 
             << " ID = " << pN.get_ID() 
             << " rank = " << pN.rank << endl;

   if (pN.get_rootnode_ptr()==&pN)
   {
      outstream << "ROOTNODE" << endl;
   }
   if (pN.get_parent_node_ptr() != NULL)
   {
      outstream << "parent node ID = " << pN.get_parent_node_ptr()->get_ID()
                << endl;
   }
   if (pN.getNumChildren() > 0)
   {
      outstream << "children node IDs:" << endl;

      pixelNode* child_node_ptr=pN.reset_child_pixelNode_ptr();
      while (child_node_ptr != NULL)
      {
         outstream << child_node_ptr->get_ID() << " ";
         child_node_ptr=pN.get_next_child_pixelNode_ptr();
         
      }
      outstream << endl;
   }
   
   return outstream;
}

// ==========================================================================
// Parent member functions:
// ==========================================================================

pixelNode* pixelNode::get_parent_node_ptr() 
{
   return parent_node_ptr;
}

// ---------------------------------------------------------------------
const pixelNode* pixelNode::get_parent_node_ptr()  const
{
   return parent_node_ptr;
}

// ---------------------------------------------------------------------
void pixelNode::set_parent_node_ptr(pixelNode* parent_node_ptr)
{
//   cout << "inside pixelNode::set_parent_node_ptr()" << endl;
//   cout << "this->ID = " << get_ID()
//        << " parent_node_ptr->get_ID() = " 
//        << parent_node_ptr->get_ID() << endl;
   this->parent_node_ptr=parent_node_ptr;
   parent_node_ptr->addChild(this);
}

// ==========================================================================
// Children member functions:
// ==========================================================================

bool pixelNode::isChild(pixelNode* child_node_ptr)
{
   int child_ID=child_node_ptr->get_ID();
   child_pixelNode_iter=child_nodes_map_ptr->find(child_ID);
   if (child_pixelNode_iter==child_nodes_map_ptr->end())
   {
      return false;
   }
   else
   {
      return true;
   }
}

// ---------------------------------------------------------------------
void pixelNode::addChild(pixelNode* child_node_ptr)
{
//   cout << "inside pixelNode::addChild()" << endl;
   int child_ID=child_node_ptr->get_ID();
//   cout << "this->ID = " << get_ID()
//             << " child_ID = " << child_ID << endl;

/*
   if (isChild(child_node_ptr))
   {
//      std::cout << "Warning in pixelNode::addChild()" << std::endl;
//      std::cout << "Node with ID = " << child_ID
//                << " is already a child of node with ID = " << get_ID()
//                << std::endl;
      return;
   }
*/
   (*child_nodes_map_ptr)[child_ID]=child_node_ptr;

   if (child_node_ptr->get_parent_node_ptr() != this)
   {
      child_node_ptr->set_parent_node_ptr(this);
   }
}

// ---------------------------------------------------------------------
void pixelNode::deleteChild(pixelNode* child_node_ptr)
{
//   cout << "inside pixelNode::deleteChild()" << endl;
   int child_ID=child_node_ptr->get_ID();

   child_pixelNode_iter=child_nodes_map_ptr->find(child_ID);
   if (child_pixelNode_iter==child_nodes_map_ptr->end())
   {
      return;
   }
   else
   {
      child_nodes_map_ptr->erase(child_pixelNode_iter);
   }
}

// ---------------------------------------------------------------------
int pixelNode::getNumChildren() const
{
   return child_nodes_map_ptr->size();
}

// ---------------------------------------------------------------------
pixelNode* pixelNode::reset_child_pixelNode_ptr()
{
   child_pixelNode_iter=child_nodes_map_ptr->begin();
   if (child_pixelNode_iter==child_nodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return child_pixelNode_iter->second;
   }
}

// ---------------------------------------------------------------------
pixelNode* pixelNode::get_next_child_pixelNode_ptr()
{
   child_pixelNode_iter++;
   if (child_pixelNode_iter==child_nodes_map_ptr->end())
   {
      return NULL;
   }
   else
   {
      return child_pixelNode_iter->second;
   }
}

// ---------------------------------------------------------------------
// Member function get_rootnode_ptr() recursively climbs from the
// current pixel Node to the top of its tree.  It returns the rootnode
// which either has no parent or equals its own parent.

pixelNode* pixelNode::get_rootnode_ptr()
{
   pixelNode* parent_node_ptr=get_parent_node_ptr();
   if (parent_node_ptr==NULL || parent_node_ptr==this) 
   {
      return this;
   }
   else
   {
      return parent_node_ptr->get_rootnode_ptr();
   }
}
