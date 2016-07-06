// ==========================================================================
// Header file for mybox class
// ==========================================================================
// Last modified on 11/20/05; 2/3/06; 8/6/06
// ==========================================================================

#ifndef MYBOX_H
#define MYBOX_H

#include <set>
#include <vector>
#include "geometry/parallelepiped.h"

class threevector;

class mybox:public parallelepiped
{

  public:

// Constructor functions

   mybox(void);
   mybox(double w,double l,double h);
   mybox(double xlo,double xhi,double ylo,double yhi,double zlo,double zhi);
   void reset_dimensions(double w,double l,double h);
   void reset_dimension_fractions(double w_frac,double l_frac,double h_frac);
   mybox(const polygon& bface,const threevector& uhat,double h);
   mybox(const mybox& b);
   ~mybox();

   mybox operator= (const mybox& b);
   friend std::ostream& operator<< (std::ostream& outstream,mybox& b);

// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------

   bool point_inside_xyprojected_mybox(const threevector& point);
   void locate_extremal_xy_points(double& min_x,double& min_y,
                                  double& max_x,double& max_y);
   void locate_extremal_xyz_corners(
      threevector& min_corner,threevector& max_corner);
   double calculate_illuminated_box_projected_area(
      const threevector& uhat,const threevector& nhat);
   std::vector<std::pair<int,polygon> > retrieve_illuminated_faces(
      const threevector& rhat);
   void retrieve_illuminated_faces(
      int& nlit_faces,const threevector& uhat,polygon lit_face[]);
   int calculate_box_shadow_regions(
      const threevector& uhat,parallelepiped shadow_volume[]);

   void print_box_corners(std::ostream& outstream);
   
// Face selection methods:
   
   int face_intercepted_by_ray(const std::pair<threevector,threevector>& p);
   
  private: 

   void docopy(const mybox& b);

};

#endif  // mybox.h






