// ==========================================================================
// GISFUNCS stand-alone methods
// ==========================================================================
// Last modified on 5/13/10
// ==========================================================================

#include <iostream>
#include <osg/Quat>

#include "osg/osgGeometry/CylindersGroup.h"
//#include "astro_geo/Ellipsoid_model.h"
//#include "ogrsf_frmts.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::string;

namespace GISfunc
{
   
// Method generate_GIS_Cylinder() instantiates an OSG Cylinder whose
// symmetry axis is radially aligned with the local earth radial
// direction.

   Cylinder* generate_GIS_Cylinder(
      const threevector& curr_point,const threevector& r_hat,
      CylindersGroup* CylindersGroup_ptr,colorfunc::Color cylinder_color,
      double text_displacement,double text_size,
      bool text_screen_axis_alignment_flag,string label)
      {
//         cout << "inside GISfunc::generate_GIS_Cylinder()" << endl;
         
         const osg::Vec3f Z_hat(0,0,1);
         osg::Quat q;
         q.makeRotate(
            Z_hat,osg::Vec3f(r_hat.get(0),r_hat.get(1),r_hat.get(2)));

         const int n_text_messages=1;
         Cylinder* curr_Cylinder_ptr=CylindersGroup_ptr->
            generate_new_Cylinder(
               curr_point,q,cylinder_color,
               n_text_messages,text_displacement,text_size,
               text_screen_axis_alignment_flag);
         curr_Cylinder_ptr->set_text_color(
            0,colorfunc::get_OSG_color(cylinder_color));
         curr_Cylinder_ptr->set_text_label(0,label);

         return curr_Cylinder_ptr;
         
      }

/*
// ---------------------------------------------------------------------
// Member function parse_worldpoint_geometry

   void parse_worldpoint_geometry(
      bool flat_grid_flag,bounding_box& gis_bbox,
      OGRGeometry* OGRGeometry_ptr,CylindersGroup* CylindersGroup_ptr,
      Ellipsoid_model* Ellipsoid_model_ptr,
      colorfunc::Color cylinder_color,double text_displacement,
      double text_size,bool text_screen_axis_alignment_flag)
      {
//   cout << "inside GISfunc::parse_worldpoint_geometry()" << endl;

         OGRPoint* poPoint_ptr=dynamic_cast<OGRPoint*>(OGRGeometry_ptr);

         const int n_text_messages=1;
//   const double text_displacement=50*1000;	// meters
//   const double text_size=20000;
   
         double longitude=poPoint_ptr->getX();
         double latitude=poPoint_ptr->getY();
//      cout << "p = " << p << " x = " << x << " y = " << y ;
         if (poCT_ptr != NULL) poCT_ptr->Transform(1,&longitude,&latitude);
//   cout << " longitude = " << x << " latitude = " << y << endl;

// Don't bother processing vertices if they lie outside specified
// boundingbox:

         if (longitude > get_gis_bbox().get_xmin() && 
             longitude < get_gis_bbox().get_xmax() && 
             latitude > get_gis_bbox().get_ymin() && 
             latitude < get_gis_bbox().get_ymax())
         {
            threevector curr_point,r_hat;
            const osg::Vec3f Z_hat(0,0,1);
            osg::Quat q;

            if (flat_grid_flag)
            {
               if (specified_UTM_zonenumber < 0)
               {
                  cout << "Error in postgis_database::parse_worldpoint_geometry()!"
                       << endl;
                  cout << "flat_grid_flag=true" << endl;
                  cout << "specified_UTM_zonenumber = "
                       << specified_UTM_zonenumber << endl;
                  exit(-1);
               }

               double point_altitude=2*1000;	// meters
               curr_point=geopoint(
                  longitude,latitude,point_altitude,specified_UTM_zonenumber).
                  get_UTM_posn();
               r_hat=z_hat;
            }
            else
            {
               curr_point=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
                  longitude,latitude,altitude);
               Ellipsoid_model_ptr->compute_east_north_radial_dirs(
                  latitude,longitude);
               r_hat=Ellipsoid_model_ptr->get_radial_hat();
            } // flat_grid_flag conditional

            q.makeRotate(
               Z_hat,osg::Vec3f(r_hat.get(0),r_hat.get(1),r_hat.get(2)));

            Cylinder* curr_Cylinder_ptr=CylindersGroup_ptr->
               generate_new_Cylinder(
                  curr_point,q,cylinder_color,
                  n_text_messages,text_displacement,text_size,
                  text_screen_axis_alignment_flag);

            curr_Cylinder_ptr->set_text_color(
               0,colorfunc::get_OSG_color(cylinder_color));

// Save name associated with current polyline within STL vector member
// geometry_names if a database column containing name information has
// been specified:

            if (geom_name.size() > 0)
            {
               string label=poFeature_ptr->GetFieldAsString(
                  geom_name.c_str());
               curr_Cylinder_ptr->set_text_label(0,label);

//         cout << "long = " << longitude
//              << " lat = " << latitude
//              << " label = " << label << endl;

               if (!text_screen_axis_alignment_flag && !flat_grid_flag)
               {
                  Ellipsoid_model_ptr->align_text_with_cardinal_dirs(
                     longitude,latitude,curr_Cylinder_ptr->get_text_ptr(0));
               }
         
            } // geom_name.size >= 0 conditional
         } // point inside bbox conditional
      }
*/

} // GISfunc namespace


   
