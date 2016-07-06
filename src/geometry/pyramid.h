// ==========================================================================
// Header file for pyramid class
// ==========================================================================
// Last modified on 7/20/08; 1/13/09; 9/30/11; 1/29/12
// ==========================================================================

#ifndef PYRAMID_H
#define PYRAMID_H

#include <iostream>
#include <string>
#include <vector>
#include "geometry/polyhedron.h"

class rotation;

class pyramid: public polyhedron
{

  public:

// Initialization, constructor and destructor functions:

   pyramid();
   pyramid(const std::vector<threevector>& V);
   pyramid(const threevector& apex,const std::vector<threevector>& 
      base_vertices);
   pyramid(const threevector& apex,const std::vector<vertex>& V,
           const std::vector<edge>& E,const std::vector<face>& F);
   pyramid(const pyramid& p);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~pyramid();
   pyramid& operator= (const pyramid& p);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const pyramid& p);

// Set and get member functions:

   vertex& get_apex();
   const vertex& get_apex() const;

   void set_base(const face& f);
   face* get_base_ptr();
   const face* get_base_ptr() const;

   void set_zplane_face(const face& f);
   face* get_zplane_face_ptr();
   const face* get_zplane_face_ptr() const;
   
   threevector get_center_axis_direction() const;

// Building member functions:

   void generate_scaled_rotated_translated_square_pyramid(
      const threevector& apex_posn,
      const std::vector<threevector>& UV_corner_dir,double altitude,
      double& Uscale,double& Vscale,threevector& Uhat,threevector& Vhat);
   void generate_scaled_rotated_translated_square_pyramid(
      const threevector& apex_posn,
      double Uscale,double Vscale,double altitude,
      const threevector& Uhat,const threevector& Vhat);
   void generate_scaled_rotated_translated_square_pyramid(
      const threevector& scale_factors,const genmatrix& R,
      const threevector& apex_posn);
   void generate_square_pyramid(const std::vector<threevector>& V);

   void generate_or_reset_square_pyramid(
      const threevector& apex_posn,
      const std::vector<threevector>& base_vertices);
   void generate_square_pyramid(
      const threevector& apex_posn,
      const std::vector<threevector>& base_vertices);
   void reset_square_pyramid_vertices(
      const threevector& new_apex,
      const std::vector<threevector>& new_base_vertices);

   void ensure_faces_handedness(face::HandednessType desired_handedness);

// Search member functions:

   void relabel_IDs_for_all_face_edges_and_vertices();

// Above Z-plane member functions:

   bool lies_above_Zplane_check(double z);
   void extract_parts_above_Zplane(
      double z,std::vector<vertex>& vertices_above_Zplane,
      std::vector<edge>& edges_above_Zplane,
      std::vector<face>& triangles_above_Zplane);
   bool form_zplane_face(
      double z,const std::vector<vertex>& vertices_above_Zplane,
      std::vector<edge>& edges_above_Zplane,
      std::vector<face>& faces_above_Zplane);

// Moving around vertex member functions:

   void translate(const threevector& rvec);
   void absolute_position(const threevector& rvec);
   void scale(const threevector& scale_origin,const threevector& scalefactor);
   void rotate(const rotation& R);
   void rotate(const threevector& rotation_origin,const rotation& R);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);

  private: 

   face *base_ptr,*zplane_face_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const pyramid& p);

   void generate_canonical_square_pyramid();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline vertex& pyramid::get_apex()
{
   return *origin_vertex_ptr;
}

inline const vertex& pyramid::get_apex() const
{
   return *origin_vertex_ptr;
}

inline void pyramid::set_base(const face& f)
{
   if (base_ptr==NULL)
   {
      base_ptr=new face(f);
   }
   else
   {
      *base_ptr=f;
   }
}

inline face* pyramid::get_base_ptr()
{
   return base_ptr;
}

inline const face* pyramid::get_base_ptr() const
{
   return base_ptr;
}

inline void pyramid::set_zplane_face(const face& f)
{
   if (zplane_face_ptr==NULL)
   {
      zplane_face_ptr=new face(f);
   }
   else
   {
      *zplane_face_ptr=f;
   }
   
}

inline face* pyramid::get_zplane_face_ptr()
{
   return zplane_face_ptr;
}

inline const face* pyramid::get_zplane_face_ptr() const
{
   return zplane_face_ptr;
}


#endif  // pyramid.h



