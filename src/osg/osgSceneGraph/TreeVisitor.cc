// ========================================================================
// TreeVisitor class member function definitions
// ========================================================================
// Last updated on 11/22/06; 1/3/07; 11/19/09; 4/6/14
// ========================================================================

#include <string>
#include <osg/PagedLOD>
#include <osg/Quat>
#include "math/fourvector.h"
#include "general/stringfuncs.h"
#include "osg/osgSceneGraph/TreeVisitor.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void TreeVisitor::allocate_member_objects()
{
   tree_ptr=new Tree<data_type>;
}

void TreeVisitor::initialize_member_objects()
{
   scenegraph_updated_flag=false;
}

TreeVisitor::TreeVisitor()
{ 
   allocate_member_objects();
   initialize_member_objects();
} 

TreeVisitor::~TreeVisitor()
{
   delete tree_ptr;
}

// ========================================================================

Tree<DATA_TYPE>* TreeVisitor::get_tree_ptr()
{
   return tree_ptr;
}

const Tree<DATA_TYPE>* TreeVisitor::get_tree_ptr() const
{
   return tree_ptr;
}

// ========================================================================
// Traversal member functions
// ========================================================================

void TreeVisitor::apply(osg::Node& currNode) 
{ 
//   cout << "Inside TreeVisitor::apply(Node)" << endl;
//   cout << "&currNode = " << &currNode << endl;
//   cout << "Custom name = " << currNode.getName() << endl;
//   cout << "classname = " << currNode.className() << endl;

// Check whether this is the very first node visited.  If so, purge
// existing tree and reset scenegraph_updated_flag from false to true:

   if (!scenegraph_updated_flag)
   {
      tree_ptr->purge_nodes();
      tree_ptr->generate_root_node();
      scenegraph_updated_flag=true;
   }
   reconstruct_total_indices();
   traversal_history.push_back(total_indices);

//   int level=total_indices.size()-1;
//   cout << "level = " << level  << endl;
//   cout << "total indices = " << endl;
//   templatefunc::printVector(total_indices);

   vector<string> label;

   string customized_name=currNode.getName();
   if (customized_name.size() > 0)
   {
      label.push_back(customized_name);
   }
      
   string OSGclass_name=currNode.className();
   label.push_back(OSGclass_name);
   if (OSGclass_name=="Geode")
   {
      if (geode_vertices_refptr.valid())
      {
         label.push_back(stringfunc::number_to_string(
            geode_vertices_refptr->size())+" vertices");
      }
   }
   else if (label.back()=="MatrixTransform")
   {
//      const int n_precision=2;
//      const int field_size=9;
      const int n_precision=3;
      const int field_size=11;

      for (unsigned int r=0; r<4; r++)
      {
         string curr_row;
         for (unsigned int c=0; c<4; c++)
         {

// Recall that curr_Matrix = individual matrix within current
// MatrixTransform.  On the other hand, LocalToWorld = cumulative
// matrix from top of path down to current scene graph level:

            curr_row += stringfunc::number_to_fixed_string_length(
               curr_Matrix(r,c),n_precision,field_size)+" ";
//               LocalToWorld(r,c),n_precision,field_size)+" ";
         }
         label.push_back(curr_row);
      }
   }
   else if (label.back()=="PositionAttitudeTransform")
   {
      const int n_precision=2;
      const int field_size=9;
      threevector curr_posn(curr_PAT_refptr->getPosition());
      
      string posn_row="posn = ";
      for (unsigned int c=0; c<3; c++)
      {
         posn_row += stringfunc::number_to_fixed_string_length(
            curr_posn.get(c),n_precision,field_size)+" ";
      }
      label.push_back(posn_row);

      threevector curr_scale(curr_PAT_refptr->getScale());
      string scale_row="scale = ";
      for (unsigned int c=0; c<3; c++)
      {
         scale_row += stringfunc::number_to_fixed_string_length(
            curr_scale.get(c),n_precision,field_size)+" ";
      }
      label.push_back(scale_row);

      osg::Quat curr_quat(curr_PAT_refptr->getAttitude());
      string quat_row="quat = ";
      for (unsigned int c=0; c<4; c++)
      {
         quat_row += stringfunc::number_to_fixed_string_length(
            curr_quat._v[c],n_precision,field_size)+" ";
      }
      label.push_back(quat_row);
   }
   else if (label.back()=="CoordinateSystemNode")
   {
      string curr_row="UTM zone "+stringfunc::number_to_string(UTM_zone);
      string hemisphere_label=" in northern hemisphere";
      if (!northern_hemisphere_flag)
      {
         hemisphere_label=" in southern hemisphere";
      }
      string total_row=curr_row+hemisphere_label;
//      cout << "total_row = " << total_row << endl;
      
      label.push_back(total_row);
   }
   else if (label.back()=="PagedLOD")
   {
      osg::PagedLOD* PagedLOD_ptr=dynamic_cast<osg::PagedLOD*>(&currNode);
//      cout << "PagedLOD n_filenames = " << PagedLOD_ptr->getNumFileNames()
//           << endl;
      for (unsigned int n=0; n<PagedLOD_ptr->getNumFileNames(); n++)
      {
         string pagedlod_filename="filename = "+PagedLOD_ptr->getFileName(n);
         label.push_back(pagedlod_filename);
//         cout << pagedlod_filename << endl;
      }
   }

   Treenode<data_type>* curr_treenode_ptr=tree_ptr->addChild(total_indices);
   curr_treenode_ptr->set_data(label);
   MyNodeVisitor::apply(currNode);
}
