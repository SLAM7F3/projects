// ==========================================================================
// Header file for POLYLINEPICKHANDLER class
// ==========================================================================
// Last modfied on 2/8/11; 2/9/11; 3/20/11; 1/22/16
// ==========================================================================

#ifndef POLYLINE_PICK_HANDLER_H
#define POLYLINE_PICK_HANDLER_H

#include "color/colorfuncs.h"
#include "osg/osgGeometry/GeometricalPickHandler.h"

class PolyLine;
class PolyLinesGroup;
class ModeController;
class WindowManager;

class PolyLinePickHandler : public GeometricalPickHandler
{

  public: 

   PolyLinePickHandler(
      const int p_ndims,Pass* PI_ptr,
      osgGA::CustomManipulator* CM_ptr,PolyLinesGroup* CG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr);

// Set and get methods:

   void set_PolyLine_rather_than_Line_mode(bool flag);
   bool get_PolyLine_rather_than_Line_mode() const;
   void set_generate_polyline_kdtree_flag(bool flag);
   void set_form_polygon_flag(bool flag);
   void set_form_polyhedron_skeleton_flag(bool flag);
   void set_fix_PolyLine_altitudes_flag(bool flag);
   void set_min_points_picked(int n);
   void set_Polygon_color(colorfunc::Color color, double alpha=1);
   void set_Polyhedron_color(colorfunc::Color color, double alpha=1);
   void set_regular_vertex_spacing(double ds);
   double get_regular_vertex_spacing() const;
   void set_z_offset(double z);
   void set_pnt_on_Zplane_flag(bool flag);
   void set_approx_range_to_polyline(double range);
   double get_approx_range_to_polyline() const;
   void set_permanent_color(const colorfunc::Color& c,double alpha=1.0);
   void set_permanent_color(const osg::Vec4& color);

// PolyLine generation, manipulation and annihilation methods:

  protected:

   bool PolyLine_continuing_flag;
   bool pnt_on_Zplane_flag;

   virtual ~PolyLinePickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool release();

   virtual bool scale(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);
   virtual bool rotate(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);

  private:

   bool PolyLine_rather_than_Line_mode;
   bool form_polygon_flag,form_polyhedron_skeleton_flag;
   bool fix_PolyLine_altitudes_flag,generate_polyline_kdtree_flag;
   int curr_PolyLine_ID,min_points_picked;
   int close_polyline_counter;
   double z_offset;
   double approx_range_to_polyline,regular_vertex_spacing;
   PolyLinesGroup* PolyLinesGroup_ptr;
   PolyLine* curr_PolyLine_ptr;
   osg::Vec4 permanent_color,selected_color,Polygon_color,Polyhedron_color;
   std::vector<threevector> V;
   std::vector<osg::Vec4> colors;

   void allocate_member_objects();
   void initialize_member_objects();

   bool instantiate_PolyLine(double X,double Y);
   bool continue_PolyLine(double X,double Y);
   bool select_PolyLine(double X,double Y);

   virtual bool move_Graphical();
   virtual bool move_PolyLine(PolyLine* PolyLine_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PolyLinePickHandler::set_PolyLine_rather_than_Line_mode(bool flag)
{
   PolyLine_rather_than_Line_mode=flag;
}

inline bool PolyLinePickHandler::get_PolyLine_rather_than_Line_mode() const
{
   return PolyLine_rather_than_Line_mode;
}

inline void PolyLinePickHandler::set_generate_polyline_kdtree_flag(bool flag)
{
   generate_polyline_kdtree_flag=flag;
}

inline void PolyLinePickHandler::set_form_polygon_flag(bool flag)
{
   form_polygon_flag=flag;
}

inline void PolyLinePickHandler::set_form_polyhedron_skeleton_flag(bool flag)
{
   form_polyhedron_skeleton_flag=flag;
}

inline void PolyLinePickHandler::set_fix_PolyLine_altitudes_flag(bool flag)
{
   fix_PolyLine_altitudes_flag=flag;
}

inline void PolyLinePickHandler::set_min_points_picked(int n)
{
   min_points_picked=n;
}

inline void PolyLinePickHandler::set_Polygon_color(
   colorfunc::Color color, double alpha)
{
   Polygon_color=colorfunc::get_OSG_color(color, alpha);
}

inline void PolyLinePickHandler::set_Polyhedron_color(
   colorfunc::Color color, double alpha)
{
   Polyhedron_color=colorfunc::get_OSG_color(color, alpha);
}

inline void PolyLinePickHandler::set_regular_vertex_spacing(double ds)
{
   regular_vertex_spacing=ds;
}

inline double PolyLinePickHandler::get_regular_vertex_spacing() const
{
   return regular_vertex_spacing;
}

inline void PolyLinePickHandler::set_z_offset(double z)
{
   z_offset=z;
}

inline void PolyLinePickHandler::set_pnt_on_Zplane_flag(bool flag)
{
   pnt_on_Zplane_flag=flag;
}

inline void PolyLinePickHandler::set_approx_range_to_polyline(double range)
{
   approx_range_to_polyline=range;
}

inline double PolyLinePickHandler::get_approx_range_to_polyline() const
{
   return approx_range_to_polyline;
}

inline void PolyLinePickHandler::set_permanent_color(
   const colorfunc::Color& c,double alpha)
{
   set_permanent_color(colorfunc::get_OSG_color(c,alpha));
}

inline void PolyLinePickHandler::set_permanent_color(const osg::Vec4& color)
{
   permanent_color=color;
}


#endif // PolyLinePickHandler.h



