// ==========================================================================
// Header file for TRANSFORMER class which converts between world xyz
// and screen XYZ coordinate systems.  This class obtains its
// Projection and View Matrices from a WindowManager object whose
// pointer is passed in after the transformer is constructed.
// ==========================================================================
// Last modified on 9/20/07; 9/21/07; 9/22/07; 6/19/08
// ==========================================================================

#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include <iostream>
#include <set>
#include <osg/Array>
#include "math/threevector.h"

class genmatrix;
class WindowManager;

class Transformer
{
  public:

// Initialization, constructor and destructor functions:

   Transformer(WindowManager* WM_ptr);
   ~Transformer();

// Transformations between world xyz and screen XYZ coordinate systems:

   threevector world_to_screen_transformation(const threevector& q);
   threevector screen_to_world_transformation(const threevector& q);

   threevector compute_ray_into_screen(double X,double Y);
   bool compute_screen_ray_intercept_with_zplane(
      double X,double Y,double Zplane,threevector& intercept_posn);
   threevector compute_screen_ray_forward_position(
      double X,double Y,double depth_range);

 private:

   genmatrix *PV_ptr,*PVinverse_ptr;
   WindowManager* WindowManager_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   void world_to_screen_transformation();
   void screen_to_world_transformation();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif // Transformer.h



