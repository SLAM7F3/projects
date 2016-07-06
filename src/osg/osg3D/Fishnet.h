// ==========================================================================
// Header file for Fishnet class
// ==========================================================================
// Last updated on 12/6/11; 12/7/11; 12/10/11
// ==========================================================================

#ifndef FISHNET_H
#define FISHNET_H

#include <iostream>
#include <string>
#include <osg/Geode>
#include "osg/osgGeometry/Geometrical.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "image/TwoDarray.h"
#include "coincidence_processing/VolumetricCoincidenceProcessor.h"

class Pass;
class PolyLinesGroup;

class Fishnet : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   Fishnet(Pass* pass_ptr,int ID,bool fall_down_flag);
   virtual ~Fishnet();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Fishnet& p);
   void init_coord_system(
      double dx,double dy,double min_x,double max_x,double min_y,double max_y,
      double init_z);
   double refine_coord_system(double z_offset);

// Set & get methods:

   bool get_fall_downwards_flag() const;
   void set_min_z_points(double z);
   void set_max_z_points(double z);
   void set_ground_surface_thickness(double t);
   void set_Emin(double E);
   double get_Emin() const;
   void set_linewidth(double l);
   void set_weights_term_coeff(double c);
   void set_springs_term_coeff(double c);
   void set_pressure_term_coeff(double c);
   
   void set_VCP_ptr(VolumetricCoincidenceProcessor* vcp_ptr);
   twoDarray* get_ztwoDarray_ptr();
   const twoDarray* get_ztwoDarray_ptr() const;
   twoDarray* get_dz_twoDarray_ptr();
   const twoDarray* get_dz_twoDarray_ptr() const;
   twoDarray* get_ground_twoDarray_ptr();
   const twoDarray* get_ground_twoDarray_ptr() const;

// Scenegraph member functions:

   osg::Geode* generate_drawable_geode();
   void initialize();

// Drawing member functions:

   unsigned int regenerate_PolyLines();
   void update_display();
   void toggle_PolyLines();

// Ground surface modeling member functions:

   void generate_pressure_mask();
   double compute_total_current_energy();
   double compute_local_current_energy(unsigned int px,unsigned int py);
   double perturb_ground_surface_trial(
      unsigned int px,unsigned int py,double Zstop);
   double compute_local_ground_pressure_energy(
      unsigned int px,unsigned int py);
   void identify_masses_close_to_pointcloud();
   void export_inverted_ground_points();
   void export_inverted_ground_surface();

  protected:

  private:

   bool fall_downwards_flag;
   double Emin,Z_stop;
   double min_z_points,max_z_points,ground_surface_thickness;
   double linewidth;
   double weights_term_coeff,springs_term_coeff,pressure_term_coeff;
   twoDarray *ztwoDarray_ptr,*dz_twoDarray_ptr,*ground_twoDarray_ptr;
   twoDarray *pressure_twoDarray_ptr;
   std::vector<threevector> mass_posns;
   osgGeometry::PointsGroup* PointsGroup_ptr;
   PolyLinesGroup* PolyLinesGroup_ptr;
   VolumetricCoincidenceProcessor* VCP_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Fishnet& f);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline bool Fishnet::get_fall_downwards_flag() const
{
   return fall_downwards_flag;
}

inline void Fishnet::set_min_z_points(double z)
{
   min_z_points=z;
}

inline void Fishnet::set_max_z_points(double z)
{
   max_z_points=z;
}

inline void Fishnet::set_ground_surface_thickness(double t)
{
   ground_surface_thickness=t;
}

inline void Fishnet::set_Emin(double E)
{
   Emin=E;
}

inline double Fishnet::get_Emin() const
{
   return Emin;
}

inline void Fishnet::set_linewidth(double width)
{
   linewidth=width;
   regenerate_PolyLines();
}

inline void Fishnet::set_weights_term_coeff(double c)
{
   weights_term_coeff=c;
}

inline void Fishnet::set_springs_term_coeff(double c)
{
   springs_term_coeff=c;
}

inline void Fishnet::set_pressure_term_coeff(double c)
{
   pressure_term_coeff=c;
}


inline void Fishnet::set_VCP_ptr(VolumetricCoincidenceProcessor* vcp_ptr)
{
   VCP_ptr=vcp_ptr;
}

inline twoDarray* Fishnet::get_ztwoDarray_ptr()
{
   return ztwoDarray_ptr;
}

inline const twoDarray* Fishnet::get_ztwoDarray_ptr() const
{
   return ztwoDarray_ptr;
}

inline twoDarray* Fishnet::get_dz_twoDarray_ptr()
{
   return dz_twoDarray_ptr;
}

inline const twoDarray* Fishnet::get_dz_twoDarray_ptr() const
{
   return dz_twoDarray_ptr;
}

inline twoDarray* Fishnet::get_ground_twoDarray_ptr()
{
   return ground_twoDarray_ptr;
}

inline const twoDarray* Fishnet::get_ground_twoDarray_ptr() const
{
   return ground_twoDarray_ptr;
}

#endif // Fishnet.h



