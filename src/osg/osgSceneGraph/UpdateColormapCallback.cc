// ========================================================================
// Ross' UpdateColormapCallback class member function definitions
// ========================================================================
// Last updated on 3/23/09; 12/2/11; 12/29/11
// ========================================================================

#include <iostream>
#include <osg/Geode>
#include <osg/Geometry>
#include "osg/osgSceneGraph/MyNodeInfo.h"
#include "osg/osgSceneGraph/scenegraphfuncs.h"
#include "general/stringfuncs.h"
#include "osg/osgSceneGraph/UpdateColormapCallback.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void UpdateColormapCallback::allocate_member_objects()
{
   ParentVisitor_refptr=new ParentVisitor();
}

void UpdateColormapCallback::initialize_member_objects()
{
   Callback_name="UpdateColormapCallback";
   ColorGeodeVisitor_ptr=NULL;
}
		
UpdateColormapCallback::UpdateColormapCallback(ColorMap* cm_ptr) 
{ 
//   cout << "inside UpdateColormapCallback constructor, this = "
//        << this << endl;
   
   allocate_member_objects();
   initialize_member_objects();
   
   ColorMap_ptr=cm_ptr;
}

UpdateColormapCallback::~UpdateColormapCallback()
{
}

// ------------------------------------------------------------------------
// If the given vertex array and metadata object are valid, set value
// to that of the given element and axis, where axes 0-2 are x, y, z,
// respectively, and > 2 are metadata axes.  Returns false if the
// element or axis given is out of range.

bool UpdateColormapCallback::getValue( 
   float& value, const osg::Vec3f& vertex, const model::Metadata* metadata,
   const unsigned int element, const int axis )
{
   if ( axis < 0 ) return false;
			
   // 0, 1, 2 - geometric axes
   if ( axis < 3 ) 
   {
      value = vertex[axis];
      return true;
   }
			
   // 3+ - metadata axes
   if ( metadata && (unsigned)axis < metadata->dims() + 3 ) 
   {
      if ( !metadata ) return NAN;
				
      value = metadata->get( element, axis-3 );
      return true;
   }
			
   return false;
}

// ------------------------------------------------------------------------
void UpdateColormapCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
//   cout << "inside UpdateColormapCallback::operator()" << endl;
//    cout << " node = "  << node << " nodevisitor = " << nv << endl;
   
   if (node==NULL) 
   {
      traverse(node,nv);
      return;
   }

   osg::Geode* geode_ptr = dynamic_cast<osg::Geode*>(node);

   model::MyNodeInfo* info = model::getOrCreateMyInfoForNode( *node );

   if ( ! info->colormapNeedsUpdate( 
      ColorMap_ptr->getCurrentUpdateIndex()) ) 
   {
//      cout << "colormap does NOT need update" << endl;
      traverse(node,nv);
      return;
   }

//   cout << "inside UpdateColormapCallback::operator()" << endl;
//   cout << "colormap DOES need update" << endl;

// For point coloring purposes, we do NOT want to set LocalToWorld
// matrix equal to entire cumulative MatrixTransform computed from
// *node all the way up to top of scenegraph.  Instead, we just want
// to use the MatrixTransform embedded at the top of the datagraph for
// point cloud coloring and ignore any higher matrix transforms in the
// total scenegraph that reposition the point cloud onto the surface
// of the earth.  So we set LocalToWorld equal to the bottommost
// MatrixTransform which is first encountered by the ParentVisitor as
// it ascends the scenegraph from the PagedNode:

   ParentVisitor_refptr->reset_matrix_transform();
   ParentVisitor_refptr->set_pointcloud_flag(false);
   node->accept(*(ParentVisitor_refptr.get()));

//   bool pointcloud_flag=ParentVisitor_refptr->get_pointcloud_flag();
//   cout << "pointcloud_flag = " << pointcloud_flag << endl;
   ColorGeodeVisitor_ptr->set_LocalToWorld(
      ParentVisitor_refptr->get_bottom_Matrix());

   osg::Geometry* curr_Geometry_ptr=scenegraphfunc::get_geometry(geode_ptr);
   if (curr_Geometry_ptr != NULL)
   {

// As of 9/9/07, we can perform two-way conversion of fused height/EO
// colors <---> grey colormap colors for NYC viewing purposes:

      if (curr_Geometry_ptr->getName() != 
          scenegraphfunc::get_mutable_colors_label() ||
          ColorGeodeVisitor_ptr->get_mutable_to_fixed_colors_flag())
      {
         osg::Vec4ubArray* colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
            curr_Geometry_ptr->getColorArray());
         osg::Vec4ubArray* secondary_colors_ptr=
            dynamic_cast<osg::Vec4ubArray*>(
               curr_Geometry_ptr->getSecondaryColorArray());
         if (colors_ptr != NULL && secondary_colors_ptr != NULL)
         {
            *colors_ptr=*secondary_colors_ptr;
         }
      }

// If fixed RGBA colors have been read in from .osga files, we want to
// store their values within the SecondaryColorArray of
// *curr_Geometry_ptr so that they can later be retrieved if they are
// overwritten by a call to color_geometry_vertices().  
      
      scenegraphfunc::save_fixed_colors(curr_Geometry_ptr);

      if (curr_Geometry_ptr->getName()==
          scenegraphfunc::get_mutable_colors_label() ||
          ColorGeodeVisitor_ptr->get_fixed_to_mutable_colors_flag())
      {
         ColorGeodeVisitor_ptr->color_geometry_vertices(curr_Geometry_ptr);
//         curr_Geometry_ptr->dirtyDisplayList();
      }
   } 
		
   traverse(node,nv);
}
