// ==========================================================================
// Header file for Pyramid class
// ==========================================================================
// Last updated on 10/13/07; 12/15/07; 12/16/07; 12/17/07
// ==========================================================================

#ifndef Pyramid_H
#define Pyramid_H

#include <iostream>
#include <vector>
#include "osg/osgGeometry/Polyhedron.h"
#include "geometry/pyramid.h"

class Pyramid : public Polyhedron
{

  public:
    
// Initialization, constructor and destructor functions:

   Pyramid(Pass* PI_ptr,const threevector& grid_world_origin,
           pyramid* p_ptr,osgText::Font* f_ptr,
           int id,AnimationController* AC_ptr);
   virtual ~Pyramid();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Pyramid& P);

// Set & get methods:

   void set_pyramid_ptr(pyramid* p_ptr);
   pyramid* get_pyramid_ptr();
   const pyramid* get_pyramid_ptr() const;

// Pyramid generation methods:

   void build_canonical_pyramid(double curr_t,int pass_number);
   void build_current_pyramid(
      double curr_t,int pass_number,pyramid* curr_pyramid_ptr);
   void store_apex_and_zplane_vertices(
      double curr_t,int pass_number,pyramid* curr_pyramid_ptr);

// Drawing methods:

   void set_color(
      pyramid* curr_pyramid_ptr,const osg::Vec4& side_edge_color,
      const osg::Vec4& base_edge_color);
   void set_color(
      pyramid* curr_pyramid_ptr,const osg::Vec4& side_edge_color,
      const osg::Vec4& zplane_edge_color,const osg::Vec4& base_edge_color,
      const osg::Vec4& volume_color);
   void set_edge_widths(
      pyramid* pyramid_ptr,double side_edge_width,
      double base_edge_width,double zplane_edge_width);
   void set_edge_masks(
      double t,int pass_number,pyramid* curr_pyramid_ptr,bool side_edge_mask,
      bool zplane_edge_mask,bool base_edge_mask);

// Animation methods:

   void update_square_pyramid_triangle_mesh(
      const std::vector<threevector>& rel_vertices);

  protected:

  private:

   bool pyramid_allocated_flag;
   pyramid* pyramid_ptr;  // just a pointer to an outside pyramid (?)
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Pyramid& P);

   void set_edge_color(int& e,const osg::Vec4& external_edge_color);
   void set_edge_mask(double t,int pass_number,int& e,bool edge_mask);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void Pyramid::set_pyramid_ptr(pyramid* p_ptr)
{
   pyramid_ptr=p_ptr;
   polyhedron_ptr=static_cast<polyhedron*>(pyramid_ptr);
}

inline pyramid* Pyramid::get_pyramid_ptr()
{
   return pyramid_ptr;
}

inline const pyramid* Pyramid::get_pyramid_ptr() const
{
   return pyramid_ptr;
}

#endif // Pyramid.h



