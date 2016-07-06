// ========================================================================
// ParentVisitor header file
// ========================================================================
// Last updated on 10/29/06; 11/19/06; 2/1/07; 6/27/07
// ========================================================================

#ifndef PARENTVISITOR_H
#define PARENTVISITOR_H

#include <osg/MatrixTransform>
#include <osg/NodeVisitor>

class ParentVisitor : public osg::NodeVisitor 
{
  public: 

   ParentVisitor(); 
   virtual ~ParentVisitor(); 

   osg::NodePath get_node_path() const;
   osg::Matrix& get_LocalToWorld();
   const osg::Matrix& get_LocalToWorld() const;
   osg::Matrix& get_bottom_Matrix();
   const osg::Matrix& get_bottom_Matrix() const;

   virtual void apply(osg::Node& currNode);
   virtual void apply(osg::MatrixTransform& currMT);
   void reset_matrix_transform();
   void set_pointcloud_flag(bool flag);
   bool get_pointcloud_flag() const;
   void set_earth_flag(bool flag);
   bool get_earth_flag() const;

  private:
   
   bool pointcloud_flag,earth_flag,bottom_matrix_identified_flag;
   osg::NodePath node_path;
   osg::Matrix LocalToWorld,bottom_Matrix;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline osg::NodePath ParentVisitor::get_node_path() const
{
   return node_path;
}

inline osg::Matrix& ParentVisitor::get_LocalToWorld()
{
   return LocalToWorld;
}

inline const osg::Matrix& ParentVisitor::get_LocalToWorld() const
{
   return LocalToWorld;
}

inline void ParentVisitor::set_pointcloud_flag(bool flag)
{
   pointcloud_flag=flag;
}

inline bool ParentVisitor::get_pointcloud_flag() const
{
   return pointcloud_flag;
}

inline void ParentVisitor::set_earth_flag(bool flag)
{
   earth_flag=flag;
}

inline bool ParentVisitor::get_earth_flag() const
{
   return earth_flag;
}

inline osg::Matrix& ParentVisitor::get_bottom_Matrix()
{
   return bottom_Matrix;
}

inline const osg::Matrix& ParentVisitor::get_bottom_Matrix() const
{
   return bottom_Matrix;
}


#endif 
