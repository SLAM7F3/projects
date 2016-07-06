// =========================================================================
// Header file for stand-alone OSG GIS functions.
// =========================================================================
// Last modified on 5/13/10
// =========================================================================

#ifndef GISFUNCS_H
#define GISFUNCS_H

#include <string>
#include "color/colorfuncs.h"

class Cylinder;
class CylindersGroup;
class threevector;

namespace GISfunc
{
   Cylinder* generate_GIS_Cylinder(
      const threevector& curr_point,const threevector& r_hat,
      CylindersGroup* CylindersGroup_ptr,colorfunc::Color cylinder_color,
      double text_displacement,double text_size,
      bool text_screen_axis_alignment_flag,std::string label);

//   void parse_worldpoint_geometry(
//      bool flat_grid_flag,
//      OGRGeometry* OGRGeometry_ptr,CylindersGroup* CylindersGroup_ptr,
//      Ellipsoid_model* Ellipsoid_model_ptr,
//      colorfunc::Color cylinder_color,double text_displacement,
//      double text_size,bool text_screen_axis_alignment_flag);
   
}

#endif // GISfuncs.h



