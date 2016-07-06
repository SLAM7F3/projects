// Note added on 1/23/08: VALGRIND indicates there's a clash between
// object pooling and OSG reference counting within this class !!!

// ==========================================================================
// Header file for PolyLine class
// ==========================================================================
// Last updated on 1/15/11; 2/7/11; 2/9/11; 5/16/11
// ==========================================================================

#ifndef PolyLine_H
#define PolyLine_H

#include <map>
#include <vector>
#include <osg/Array>
#include <osg/LineWidth>
#include "geometry/bounding_box.h"
#include "osg/osgGeometry/Geometrical.h"
#include "track/mover.h"
#include "pool/objpool.h"
#include "track/track.h"

class ConesGroup;
class Pass;
class PointFinder;
class polygon;
class polyline;

// Forward declaration of PointsGroup and Polygon classes which sit
// inside osgGeometry namespace:

namespace osgGA
{
   class CustomManipulator;
}

namespace osgGeometry
{
   class Point;
   class PointsGroup;
   class Polygon;
}

class PolyLine : public Geometrical, public ObjectPool<PolyLine>
{

  public:

   typedef osg::TemplateArray<osg::Vec3,osg::Array::Vec3ArrayType,3,GL_DOUBLE>
      Vec3DArray;

   typedef std::map<int,int> OBJECTS_INSIDE_MAP;

// Initialization, constructor and destructor functions:

   PolyLine();
   PolyLine(const int p_ndim,Pass* PI_ptr,
      threevector* GO_ptr,osgText::Font* f_ptr,
      int n_text_messages,int id,
      osgGA::CustomManipulator* CM_ptr=NULL);
   PolyLine(const int p_ndim,Pass* PI_ptr,
      threevector* GO_ptr,const threevector& reference_origin,
      const std::vector<threevector>& V,osgText::Font* f_ptr,
      int n_text_messages,int id,
      osgGA::CustomManipulator* CM_ptr=NULL);
   
   virtual ~PolyLine();
   friend std::ostream& operator<< (
      std::ostream& outstream,const PolyLine& l);

// Set & get methods:

   void set_entry_finished_flag(bool flag);
   bool get_entry_finished_flag() const;
   std::string get_length_label() const;
   osgGeometry::PointsGroup* get_PointsGroup_ptr();
   const osgGeometry::PointsGroup* get_PointsGroup_ptr() const;

   ConesGroup* get_ConesGroup_ptr();
   const ConesGroup* get_ConesGroup_ptr() const;
   
   void set_linewidth(double width);
   
   polyline* get_polyline_ptr();
   polygon* get_polygon_ptr();
   void set_track_ptr(track* t_ptr);
   track* get_track_ptr();
   const track* get_track_ptr() const;
   void set_mover_ptr(mover* m_ptr);
   mover* get_mover_ptr();
   const mover* get_mover_ptr() const;

   void set_relative_vertices(const std::vector<threevector>& V);
   std::vector<threevector> get_relative_vertices();
   const std::vector<threevector> get_relative_vertices() const;
   
   virtual void set_color(const osg::Vec4& color);
   virtual void set_colors(const std::vector<osg::Vec4>& colors);

// PolyLine manipulation member functions:

   void add_vertex_points(
      const std::vector<threevector>& V,const osg::Vec4& color);
   osgGeometry::Point* get_vertex_Point(int n);
   void rescale(double ds);

// Geometry object construction member functions:

   polyline* get_or_set_polyline_ptr();
   polyline* construct_polyline();
   polygon* construct_polygon();
   polygon* construct_relative_polygon();
   void assign_heights_using_pointcloud(PointFinder* PF_ptr);

// Drawing methods:

   osg::Geode* generate_drawable_geode(bool force_display_flag=false);
   void fill_drawable_geode(
      osg::Geode* geode_ptr,bool force_display_flag=false,
      bool generate_drawable_geom_flag=true);
   void add_flow_direction_arrows(
      const std::vector<double>& arrow_posn_fracs,double linewidth);
   void add_flow_direction_arrows(
      double distance_between_arrows,double linewidth);
   void compute_color_fading(const colorfunc::RGB& polyline_RGB);

// Text methods:

   void set_text_posn(int i,const threevector& posn);
   void set_label(std::string label,const threevector& label_posn,
                  double text_size);
   void set_label(std::string label,const threevector& label_posn,
                  const threevector& label_dir,double text_size);
   void set_COM_label(std::string label,double height_above_polyline=0);
   void set_shrunken_text_label_flag(bool flag);
   bool get_shrunken_text_label_flag() const;

   void generate_PolyLine_label(std::string label);
   std::string generate_PolyLine_ID_label();
   std::string generate_PolyLine_length_label();
   double get_length_sizefactor();

// Bbox member functions:

   bounding_box& get_bbox();
   const bounding_box& get_bbox() const;
   bounding_box& compute_bbox();
   void instantiate_objects_inside_PolyLine_map();
   OBJECTS_INSIDE_MAP* get_objects_inside_PolyLine_map_ptr();
   const OBJECTS_INSIDE_MAP* get_objects_inside_PolyLine_map_ptr() const;

  protected:

  private:

   bool entry_finished_flag,shrunken_text_label_flag;
   std::string length_label;
   polyline* polyline_ptr;
   polygon* polygon_ptr;
   osgGA::CustomManipulator* CM_ptr;
   osgGeometry::PointsGroup* PointsGroup_ptr;
   ConesGroup* ConesGroup_ptr;
   osgGeometry::Polygon* Polygon_ptr;
   osg::ref_ptr<osg::Geometry> geom_refptr;
   osg::ref_ptr<osg::LineWidth> linewidth_refptr;
   osg::ref_ptr<osg::DrawArrays> drawarrays_refptr;
   track* track_ptr;
   mover* mover_ptr;
   bounding_box bbox;
   OBJECTS_INSIDE_MAP* objects_inside_PolyLine_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
//   void docopy(const PolyLine& p);

   osg::Geometry* generate_drawable_geom();
   void generate_text(int i,osg::Geode* geode_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PolyLine::set_entry_finished_flag(bool flag)
{
   entry_finished_flag=flag;
}

inline bool PolyLine::get_entry_finished_flag() const
{
   return entry_finished_flag;
}

inline std::string PolyLine::get_length_label() const
{
   return length_label;
}

inline void PolyLine::set_shrunken_text_label_flag(bool flag)
{
   shrunken_text_label_flag=flag;
}

inline bool PolyLine::get_shrunken_text_label_flag() const
{
   return shrunken_text_label_flag;
}

inline polyline* PolyLine::get_polyline_ptr()
{
   return get_or_set_polyline_ptr();
}

inline polygon* PolyLine::get_polygon_ptr()
{
   return polygon_ptr;
}

inline void PolyLine::set_track_ptr(track* t_ptr)
{
   track_ptr=t_ptr;
}

inline track* PolyLine::get_track_ptr()
{
   return track_ptr;
}

inline const track* PolyLine::get_track_ptr() const
{
   return track_ptr;
}

inline void PolyLine::set_mover_ptr(mover* m_ptr)
{
   mover_ptr=m_ptr;
}

inline mover* PolyLine::get_mover_ptr()
{
   return mover_ptr;
}

inline const mover* PolyLine::get_mover_ptr() const
{
   return mover_ptr;
}

inline bounding_box& PolyLine::get_bbox()
{
   return bbox;
}

inline const bounding_box& PolyLine::get_bbox() const
{
   return bbox;
}

inline PolyLine::OBJECTS_INSIDE_MAP* 
PolyLine::get_objects_inside_PolyLine_map_ptr()
{
   return objects_inside_PolyLine_map_ptr;
}

inline const PolyLine::OBJECTS_INSIDE_MAP* 
PolyLine::get_objects_inside_PolyLine_map_ptr() const
{
   return objects_inside_PolyLine_map_ptr;
}


#endif // PolyLine.h



