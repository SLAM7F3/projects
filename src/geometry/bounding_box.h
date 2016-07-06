// ==========================================================================
// Header file for bounding_box class
// ==========================================================================
// Last modified on 6/24/16; 6/25/16; 7/3/16; 7/5/16
// ==========================================================================

#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <map>
#include <vector>
#include "color/colorfuncs.h"
#include "math/threevector.h"
#include "math/twovector.h"

class linesegment;
class polyline;

class bounding_box
{

  public:

   typedef std::map<std::string, std::string > ATTRIBUTES_MAP;
// independent string: attribute key
// dependent string: attribute value

   bounding_box();
   bounding_box(double xmin,double xmax,double ymin,double ymax);
   bounding_box(double xmin,double xmax,double ymin,double ymax,
                double zmin,double zmax);
   bounding_box(const polyline* polyline_ptr);
   bounding_box(const bounding_box& b);
   ~bounding_box();
   bounding_box& operator= (const bounding_box& b);
   friend std::ostream& operator<< 
      (std::ostream& outstream, const bounding_box& b);

// Set and get member functions:

   void set_ID(int id);
   int get_ID() const;
   void set_x_bounds(double xmin,double xmax);
   void set_y_bounds(double xmin,double xmax);
   void set_xy_bounds(double xmin,double xmax,double ymin,double ymax);
   void set_xyz_bounds(double xmin,double xmax,double ymin,double ymax,
                       double zmin,double zmax);

   void set_physical_deltaX(double phys_dX);
   void set_physical_deltaY(double phys_dY);
   double get_physical_deltaX() const;
   double get_physical_deltaY() const;

   double get_xmin() const;
   double get_xmax() const;
   double get_ymin() const;
   double get_ymax() const;
   double get_xcenter() const;
   double get_ycenter() const;
   double get_xextent() const;
   double get_yextent() const;
   double get_aspect_ratio() const;

   double get_zmin() const;
   double get_zmax() const;
   
   double* get_xmin_ptr();
   double* get_xmax_ptr();
   double* get_ymin_ptr();
   double* get_ymax_ptr();
   void set_UV_bounds(double Umin,double Umax,double Vmin,double Vmax);

   void set_color(colorfunc::Color c);
   colorfunc::Color get_color() const;
   void set_label(std::string l);
   std::string get_label() const;

   bounding_box::ATTRIBUTES_MAP& get_attributes_map();
   const bounding_box::ATTRIBUTES_MAP& get_attributes_map() const;
   bounding_box::ATTRIBUTES_MAP::iterator& get_attributes_map_iter();
   const bounding_box::ATTRIBUTES_MAP::iterator& get_attributes_map_iter() 
      const;

   void set_attribute_value(std::string attr_key, std::string attr_value);
   std::string get_attribute_value(std::string attr_key);

// Bounding box properties member functions:

   void recompute_bounding_box(const std::vector<threevector>& V);
   double get_area() const;
   threevector get_midpoint() const;
   std::vector<threevector> get_bbox_corners() const;
   std::vector<linesegment> get_bbox_diagonals() const;

// Fractional coordinate member functions:

   bool point_inside(double x,double y) const;
   void frac_XY_coords(double x,double y,double& xfrac,double& yfrac);
   void frac_UV_coords(double U,double V,double& ufrac,double& vfrac);
   void XY_frac_coords(double xfrac,double yfrac,double& x,double& y);
   void UV_frac_coords(double ufrac,double vfrac,double& u,double& v);

   twovector XY_to_UV_coords(double x,double y);
   twovector UV_to_XY_coords(double U,double V);

   void update_bounds(const bounding_box* bbox_ptr);
   void update_bounds(double x,double y);
   void update_bounds(const threevector& curr_XYZ);
   void update_bounds(const std::vector<threevector>& XYZ);
   void reset_bounds(const std::vector<threevector>& XYZ);

// Bounding box manipulation member functions

   void dilate(double alpha);
   void inflate(double alpha);
   void translate(double dx, double dy);
   void recenter(double x, double y);
   void inscribed_bbox(const std::vector<threevector>& corner);
   bool XY_inside_WL_bbox(double x,double y);

// Intersection & union member functions

   bool linesegment_inside_2D_bbox(
      const linesegment& l,threevector& p1,threevector& p2);
   bool overlap(const bounding_box& bbox) const;
   bool bbox_intersection(
      const bounding_box& bbox,bounding_box& intersection_bbox) const;
   void bbox_union(const bounding_box& bbox,bounding_box& union_bbox) const;
   double intersection_over_union(const bounding_box &bbox) const;
   bool encloses(const bounding_box& bbox) const;
   bool nearly_equal(const bounding_box& bbox2) const;

  private: 

   int ID;
   double xmin,xmax,ymin,ymax,zmin,zmax;
   double Umin,Umax,Vmin,Vmax;
   double physical_deltaX,physical_deltaY;
   double wmin,wmax,lmin,lmax;
   threevector what,lhat,COM;
   colorfunc::Color bbox_color;
   std::string label;

   ATTRIBUTES_MAP attributes_map;
   ATTRIBUTES_MAP::iterator attributes_map_iter;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const bounding_box& b);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void bounding_box::set_ID(int id)
{
   ID = id;
}

inline int bounding_box::get_ID() const
{
   return ID;
}

inline void bounding_box::set_x_bounds(double xmin,double xmax)
{
   this->xmin=xmin;
   this->xmax=xmax;
}

inline void bounding_box::set_y_bounds(double ymin,double ymax)
{
   this->ymin=ymin;
   this->ymax=ymax;
}

inline void bounding_box::set_xy_bounds(
   double xmin,double xmax,double ymin,double ymax)
{
   set_x_bounds(xmin, xmax);
   set_y_bounds(ymin, ymax);
}

inline void bounding_box::set_xyz_bounds(
   double xmin,double xmax,double ymin,double ymax,double zmin,double zmax)
{
   set_xy_bounds(xmin,xmax,ymin,ymax);
   this->zmin=zmin;
   this->zmax=zmax;
}

inline void bounding_box::set_physical_deltaX(double phys_dX)
{
   physical_deltaX=phys_dX;
}

inline void bounding_box::set_physical_deltaY(double phys_dY)
{
   physical_deltaY=phys_dY;
}

inline double bounding_box::get_physical_deltaX() const
{
   return physical_deltaX;
}

inline double bounding_box::get_physical_deltaY() const
{
   return physical_deltaY;
}

inline double bounding_box::get_xmin() const
{
   return xmin;
}

inline double bounding_box::get_xmax() const
{
   return xmax;
}

inline double bounding_box::get_ymin() const
{
   return ymin;
}

inline double bounding_box::get_ymax() const
{
   return ymax;
}

inline double bounding_box::get_xcenter() const
{
   return 0.5*(xmin+xmax);
}

inline double bounding_box::get_ycenter() const
{
   return 0.5*(ymin+ymax);
}

inline double bounding_box::get_xextent() const
{
   return xmax - xmin;
}

inline double bounding_box::get_yextent() const
{
   return ymax - ymin;
}

inline double bounding_box::get_aspect_ratio() const
{
   return (xmax - xmin) / (ymax - ymin);
}

inline double bounding_box::get_zmin() const
{
   return zmin;
}

inline double bounding_box::get_zmax() const
{
   return zmax;
}

inline double* bounding_box::get_xmin_ptr() 
{
   return &xmin;
}

inline double* bounding_box::get_xmax_ptr() 
{
   return &xmax;
}

inline double* bounding_box::get_ymin_ptr() 
{
   return &ymin;
}

inline double* bounding_box::get_ymax_ptr() 
{
   return &ymax;
}

inline void bounding_box::set_UV_bounds(
   double Umin,double Umax,double Vmin,double Vmax)
{
   this->Umin=Umin;
   this->Umax=Umax;
   this->Vmin=Vmin;
   this->Vmax=Vmax;
}

// Boolean member function point_inside returns true if (x,y) lies
// within the current bounding_box object.

inline bool bounding_box::point_inside(double x,double y) const
{
   if (x > xmin && x < xmax && y > ymin && y < ymax)
   {
      return true;
   }
   else
   {
      return false;
   }
}

inline void bounding_box::set_color(colorfunc::Color c)
{
   bbox_color=c;
}

inline colorfunc::Color bounding_box::get_color() const
{
   return bbox_color;
}

inline void bounding_box::set_label(std::string l)
{
   label = l;
}

inline std::string bounding_box::get_label() const
{
   return label;
}

inline bounding_box::ATTRIBUTES_MAP& bounding_box::get_attributes_map()
{
   return attributes_map;
}

inline const bounding_box::ATTRIBUTES_MAP& bounding_box::get_attributes_map() const
{
   return attributes_map;
}

inline bounding_box::ATTRIBUTES_MAP::iterator& bounding_box::get_attributes_map_iter()
{
   return attributes_map_iter;
}

inline const bounding_box::ATTRIBUTES_MAP::iterator& bounding_box::get_attributes_map_iter() const
{
   return attributes_map_iter;
}

inline void bounding_box::set_attribute_value(
   std::string attr_key, std::string attr_value)
{
   attributes_map[attr_key] = attr_value;
}

inline std::string bounding_box::get_attribute_value(std::string attr_key)
{
   attributes_map_iter = attributes_map.find(attr_key);
   if(attributes_map_iter == attributes_map.end())
   {
      return "";
   }
   else
   {
      return attributes_map_iter->second;
   }
}



#endif  // bounding_box.h
