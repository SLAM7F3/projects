// ==========================================================================
// Header file for Polyhedron class
// ==========================================================================
// Last updated on 1/22/12; 1/23/12; 8/4/13
// ==========================================================================

#ifndef Polyhedron_H
#define Polyhedron_H

#include <iostream>
#include <vector>
#include <osg/Group>
#include <osg/ShapeDrawable>
#include "osg/osgGeometry/Geometrical.h"
#include "passes/PassesGroup.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "geometry/polyhedron.h"

class Cylinder;
class LineSegment;
class LineSegmentsGroup;

class Polyhedron : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   Polyhedron(Pass* PI_ptr,const threevector& grid_world_origin,
              osgText::Font* f_ptr,int id,AnimationController* AC_ptr=NULL);
   Polyhedron(Pass* PI_ptr,const threevector& grid_world_origin,
              polyhedron* p_ptr,osgText::Font* f_ptr,int id,
              AnimationController* AC_ptr=NULL);
   virtual ~Polyhedron();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Polyhedron& P);

// Set & get methods:

   void set_selected_vertex_ID(int ID);
   int get_selected_vertex_ID() const;
   void set_selected_edge_ID(int ID);
   int get_selected_edge_ID() const;
   void set_selected_face_ID(int ID);
   int get_selected_face_ID() const;

   LineSegmentsGroup* get_LineSegmentsGroup_ptr();
   const LineSegmentsGroup* get_LineSegmentsGroup_ptr() const;
   LineSegment* get_or_create_LineSegment_ptr(unsigned int n);
   osgGeometry::PointsGroup* get_PointsGroup_ptr();
   const osgGeometry::PointsGroup* get_PointsGroup_ptr() const;
   
   void set_polyhedron_ptr(polyhedron* p_ptr);
   polyhedron* get_polyhedron_ptr();
   const polyhedron* get_polyhedron_ptr() const;
   void set_Cylinder_ptr(Cylinder* Cylinder_ptr);
   Cylinder* get_Cylinder_ptr();
   const Cylinder* get_Cylinder_ptr() const;

// Polyhedron generation member functions:

   void generate_vertex_Points();
   void build_current_polyhedron(double curr_t,int pass_number);
   
// Drawing member functions:

   void reset_edges_width(double width);
   osg::Geode* generate_drawable_geode(
      bool text_screen_axis_alignment_flag=false);

   void set_color(const osg::Vec4& edge_color);
   void set_color(const osg::Vec4& edge_color,const osg::Vec4& volume_color);
   void set_edge_color(const osg::Vec4& external_edge_color);
   void reset_volume_alpha(double alpha);

// Text member functions:

   osgText::Text* generate_text(
      int i,const threevector& text_posn,colorfunc::Color text_color,
      bool text_screen_axis_alignment_flag);

// Manipulation member functions:
   
   void scale_rotate_and_then_translate(
      double curr_t,int pass_number,
      double theta,double phi,const threevector& scale,
      const threevector& trans);

  protected:

   int selected_vertex_ID,selected_edge_ID,selected_face_ID;
   threevector grid_world_origin;
   polyhedron* polyhedron_ptr;
   Cylinder* Cylinder_ptr;	// just ptr, not object!
   LineSegmentsGroup* LineSegmentsGroup_ptr;
   osgGeometry::PointsGroup* PointsGroup_ptr;
   osg::ref_ptr<osg::ShapeDrawable> shape_refptr;
   osg::ref_ptr<osg::TriangleMesh> mesh_refptr;

   void update_triangle_mesh();
//   void update_triangle_mesh(polyhedron* P_ptr);
//   void update_triangle_mesh(const std::vector<threevector>& rel_vertices);

  private:

   bool dynamically_instantiated_polyhedron_flag;
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Polyhedron& f);

   void initialize_linesegments();
   void generate_triangle_mesh();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void Polyhedron::set_selected_vertex_ID(int ID)
{
   selected_vertex_ID=ID;
}

inline int Polyhedron::get_selected_vertex_ID() const
{
   return selected_vertex_ID;
}

inline void Polyhedron::set_selected_edge_ID(int ID)
{
   selected_edge_ID=ID;
}

inline int Polyhedron::get_selected_edge_ID() const
{
   return selected_edge_ID;
}

inline void Polyhedron::set_selected_face_ID(int ID)
{
   selected_face_ID=ID;
}

inline int Polyhedron::get_selected_face_ID() const
{
   return selected_face_ID;
}


inline LineSegmentsGroup* Polyhedron::get_LineSegmentsGroup_ptr()
{
   return LineSegmentsGroup_ptr;
}

inline const LineSegmentsGroup* Polyhedron::get_LineSegmentsGroup_ptr() 
   const
{
   return LineSegmentsGroup_ptr;
}

inline osgGeometry::PointsGroup* Polyhedron::get_PointsGroup_ptr()
{
   return PointsGroup_ptr;
}

inline const osgGeometry::PointsGroup* Polyhedron::get_PointsGroup_ptr() const
{
   return PointsGroup_ptr;
}


inline void Polyhedron::set_polyhedron_ptr(polyhedron* p_ptr)
{
   polyhedron_ptr=p_ptr;
   generate_vertex_Points();
}

inline polyhedron* Polyhedron::get_polyhedron_ptr()
{
   return polyhedron_ptr;
}

inline const polyhedron* Polyhedron::get_polyhedron_ptr() const
{
   return polyhedron_ptr;
}



#endif // Polyhedron.h



