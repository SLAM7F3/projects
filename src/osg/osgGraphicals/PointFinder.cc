// ==========================================================================
// POINTFINDER class member function definitions
// ==========================================================================
// Last modified on 2/22/09; 9/16/09; 10/16/13; 4/6/14
// ==========================================================================

#include <iostream>
#include <set>
#include <vector>
#include "math/constant_vectors.h"
#include "osg/Custom3DManipulator.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "geometry/linesegment.h"
#include "general/outputfuncs.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/Transformer.h"

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PointFinder::allocate_member_objects()
{
}		       

void PointFinder::initialize_member_objects()
{
   minimal_allowed_range=0;
   maximal_rho=10000;	// meters
   max_tan_theta=0.36;    // = tan(20 degs)
   nearest_worldspace_point=threevector(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
}

PointFinder::PointFinder()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

PointFinder::PointFinder(DataGraphsGroup* DGG_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   pushback_DataGraphsGroup_ptr(DGG_ptr);
}		       

PointFinder::~PointFinder()
{	
}

// ==========================================================================
// Member function find_closest_world_point takes in screen
// coordinates X and Y for some point picked by the mouse.  It loops
// over all geodes within all DataGraphs and ignores those lying far
// away from the current line-of-sight ray.  It then searches for
// vertices lying within the cone whose tip lies at the mouse
// selection and whose opening angle is set by local variable
// tan_theta.  The ranges to all vertices lying within this cone are
// computed, and the point with the smallest range is returned by this
// method.  If no points are found to lie within the cone, then the
// cone's opening angle is enlarged, and the minimum range search is
// performed again.  This boolean method returns false if no valid
// nearby world point is found.

// We have made reasonable efforts to minimize execution time of this
// method.

bool PointFinder::find_closest_world_point(
   osgGA::Custom3DManipulator* CM_3D_ptr,
   double X,double Y,threevector& closest_worldspace_point)
{
//   cout << "inside PointFinder::find_closest_world_point()" << endl;
//   timefunc::initialize_timeofday_clock();

// Following Ross Anderson's approach, first compute semi-infinite ray
// along current line-of-sight.  Geodes lying much too far from this
// ray cannot contain the closest world point:

   threevector r_hat(CM_3D_ptr->get_Transformer_ptr()->
                     compute_ray_into_screen(X,Y));

// Camera's world position provides ray's basepoint:

   return find_closest_world_point(
      CM_3D_ptr->get_eye_world_posn(),r_hat,
      closest_worldspace_point);
}

// ---------------------------------------------------------------------
// This next overloaded version of find_closest_world_point() is meant
// to be picking points within essentially nadir-oriented aerial ladar
// or EO imagery.

bool PointFinder::find_closest_world_point(
   const threevector& ray_basepoint,const threevector& ray_ehat,
   threevector& closest_worldspace_point,double tan_theta)
{
//   cout << "inside PointFinder::find_closest_world_point()" << endl;
//   cout << "ray_basepoint = " << ray_basepoint << endl;
//   cout << "ray_ehat = " << ray_ehat << endl;

//   timefunc::initialize_timeofday_clock();

   closest_worldspace_point=threevector(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);

   if (DataGraphsGroup_ptrs.size()==0)
   {
//      cout << "Error in PointFinder::find_closest_world_point()!" << endl;
//      cout << "DataGraphsGroup_ptr = " << DataGraphsGroup_ptr << endl;
      return false;
   }

   int n_nearby_candidates=0;
   double min_range=POSITIVEINFINITY;
   osg::Vec3 viewpoint(ray_basepoint.get(0),ray_basepoint.get(1),
                       ray_basepoint.get(2));

   for (unsigned int dgg=0; dgg<DataGraphsGroup_ptrs.size(); dgg++)
   {
      DataGraphsGroup* curr_DataGraphsGroup_ptr=DataGraphsGroup_ptrs[dgg];

      const double max_sphere_to_ray_frac_dist=1.0;
      vector<pair<osg::Geometry*,osg::Matrix> > geoms_along_LOS=
         curr_DataGraphsGroup_ptr->geometries_along_ray(
            ray_basepoint,ray_ehat,max_sphere_to_ray_frac_dist);

//      cout << "geoms_along_LOS.size() = " << geoms_along_LOS.size() << endl;
//      double tan_theta=0.25*PI/180; // tan of initial cone opening angle 
//      double tan_theta=1.0*PI/180; // tan of initial cone opening angle 

      const double max_tan_theta=0.36;    // = tan(20 degs)
  
      while (n_nearby_candidates==0 && tan_theta < max_tan_theta)
      {
         for (unsigned int g=0; g<geoms_along_LOS.size(); g++)
         {
            osg::Geometry* curr_Geometry_ptr(geoms_along_LOS[g].first);
            osg::Matrix MatrixTransform(geoms_along_LOS[g].second);
            osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
               curr_Geometry_ptr->getVertexArray());

// We must either transform every vertex within the current Geometry
// by MatrixTransform.  Or we can inverse transform just the viewpoint
// once.  To minimize matrix multiplications, we choose the latter
// option...

            osg::Vec3 transformed_viewpoint(viewpoint*MatrixTransform.
                                            inverse(MatrixTransform));
//            cout << "g = " << g 
//                 << " # vertices = " << curr_vertices_ptr->size() << endl;
            
            for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
            {
               threevector rel_pnt(
                  curr_vertices_ptr->at(i)-transformed_viewpoint);            
               
//               threevector curr_point(
//                  curr_vertices_ptr->at(i)*MatrixTransform);
//               cout << "i = " << i 
//                    << " x=" << curr_point.get(0)
//                    << " y=" << curr_point.get(1)
//                    << " z=" << curr_point.get(2) << endl;
            
               double curr_range=ray_ehat.dot(rel_pnt);
               if (curr_range > 0 && curr_range < min_range)
               {
                  double sqrd_rho=(
                     rel_pnt-curr_range*ray_ehat).sqrd_magnitude();
                  if (sqrd_rho < sqr(curr_range*tan_theta))
                  {
                     min_range=curr_range;
                     closest_worldspace_point=threevector(
                        curr_vertices_ptr->at(i)*MatrixTransform);
                     n_nearby_candidates++;
                  }
               } // curr_range < min_range conditional
            } // loop over index i labeling vertices within *curr_Geometry_ptr
         } // loop over index g labeling geodes in *curr_DataGraphsGroup_ptr
         
         tan_theta *= 2;
         if (n_nearby_candidates==0)
         {
            cout << "tan_theta increased to " << tan_theta << endl;
         }
      } // while n_nearby_candidates==0 loop
   } // loop over dgg index labeling DataGraphsGroups

//   cout << "n_nearby_candidates = " << n_nearby_candidates << endl;
//   cout << "Closest point = " << closest_worldspace_point << endl;
//   cout << "Point's range from camera = " << min_range << endl;
//   cout << "Range * tan_theta = " << min_range*tan_theta << endl;

//   double elapsed_time=timefunc::elapsed_timeofday_time();   
//   cout << "elapsed time = " << elapsed_time << endl;

   return (closest_worldspace_point.magnitude() < POSITIVEINFINITY);
}

// ---------------------------------------------------------------------
// Member function find_smallest_relative_angle_world_point() is meant
// to be called for point picking within photos shot by ground
// cameras.  It selects candidate 3D points within a ladar cloud whose
// angles relative to input ray_ehat are minimal.  This method
// subsequently sorts the 3D candidates according to their ranges
// relative to input ray_basepoint weighted by their angles wrt
// ray_ehat.  The candidate 3D point with the smallest weighted range
// is returned within closest_worldspace_point.

bool PointFinder::find_smallest_relative_angle_world_point(
   const threevector& ray_basepoint,const threevector& ray_ehat,
   threevector& closest_worldspace_point)
{
//   cout << "inside PointFinder::find_smallest_relative_angle_world_point()" 
//	  << endl;
//   cout << "ray_basepoint = " << ray_basepoint << endl;
//   cout << "ray_ehat = " << ray_ehat << endl;

//   timefunc::initialize_timeofday_clock();

   closest_worldspace_point=threevector(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);

   if (DataGraphsGroup_ptrs.size()==0)
   {
//      cout << "Error in PointFinder::find_closest_world_point()!" << endl;
//      cout << "DataGraphsGroup_ptr = " << DataGraphsGroup_ptr << endl;
      return false;
   }

   int n_nearby_candidates=0;
   double max_dotproduct=0;
   vector<double> close_worldspace_point_angles;
   vector<double> close_worldspace_point_ranges;
   vector<threevector> close_worldspace_points;

   osg::Vec3 viewpoint(ray_basepoint.get(0),ray_basepoint.get(1),
                       ray_basepoint.get(2));

   for (unsigned int dgg=0; dgg<DataGraphsGroup_ptrs.size(); dgg++)
   {
      DataGraphsGroup* curr_DataGraphsGroup_ptr=DataGraphsGroup_ptrs[dgg];

      const double max_sphere_to_ray_frac_dist=0.2;
      vector<pair<osg::Geometry*,osg::Matrix> > geoms_along_LOS=
         curr_DataGraphsGroup_ptr->geometries_along_ray(
            ray_basepoint,ray_ehat,max_sphere_to_ray_frac_dist);
//      cout << "geoms_along_LOS.size() = " << geoms_along_LOS.size() << endl;

      for (unsigned int g=0; g<geoms_along_LOS.size(); g++)
      {
         osg::Geometry* curr_Geometry_ptr(geoms_along_LOS[g].first);
         osg::Matrix MatrixTransform(geoms_along_LOS[g].second);
         osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
            curr_Geometry_ptr->getVertexArray());

// We must either transform every vertex within the current Geometry
// by MatrixTransform.  Or we can inverse transform just the viewpoint
// once.  To minimize matrix multiplications, we choose the latter
// option...

         osg::Vec3 transformed_viewpoint(viewpoint*MatrixTransform.
                                         inverse(MatrixTransform));
//            cout << "g = " << g 
//                 << " # vertices = " << curr_vertices_ptr->size() << endl;
            
         for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
         {
            threevector rel_pnt(
               curr_vertices_ptr->at(i)-transformed_viewpoint);            
               
//               threevector curr_point(
//                  curr_vertices_ptr->at(i)*MatrixTransform);
//               cout << "i = " << i 
//                    << " x=" << curr_point.get(0)
//                    << " y=" << curr_point.get(1)
//                    << " z=" << curr_point.get(2) << endl;
            
// Do not select points which lie too close to camera's position:

            double curr_range=ray_ehat.dot(rel_pnt);
            if (curr_range > minimal_allowed_range)
            {
               double curr_dotproduct=ray_ehat.dot(rel_pnt.unitvector());

// Add points whose dot product with ray_ehat are maximal to
// close_worldspace_points STL vector:

               if (curr_dotproduct > max_dotproduct)
               {
                  max_dotproduct=curr_dotproduct;
                  threevector curr_worldspace_point=threevector(
                     curr_vertices_ptr->at(i)*MatrixTransform);

                  close_worldspace_points.push_back(curr_worldspace_point);
                  close_worldspace_point_ranges.push_back(curr_range);
                  close_worldspace_point_angles.push_back(
                     acos(max_dotproduct)*180/PI);

                  n_nearby_candidates++;
//                  cout << "max_dotproduct = " << max_dotproduct 
//                       << " acos = " << acos(max_dotproduct)*180/PI 
//                       << endl;
               }
            } // curr_range < min_range conditional
         } // loop over index i labeling vertices within *curr_Geometry_ptr
      } // loop over index g labeling geodes in *curr_DataGraphsGroup_ptr
   } // loop over dgg index labeling DataGraphsGroups

// Sort candidate 3D points according to their angles wrt ray_ehat:

   templatefunc::Quicksort(close_worldspace_point_angles,
                           close_worldspace_point_ranges,
                           close_worldspace_points);

// Weight ranges of candidate 3D points by their angles wrt ray_ehat.
// Take candidate whose weighted range is minimal as most likely
// ray-trace match:

//   unsigned int n_nearby_points=1;
   unsigned int n_nearby_points=3;
   vector<double> nearby_worldspace_point_ranges;
   vector<double> weighted_ranges;
   vector<threevector> nearby_worldspace_points;
   for (unsigned int n=0; n<n_nearby_points; n++)
   {
      nearby_worldspace_point_ranges.push_back(
         close_worldspace_point_ranges[n]);
      weighted_ranges.push_back(
         close_worldspace_point_ranges[n]*
         close_worldspace_point_angles[n]);
      nearby_worldspace_points.push_back(close_worldspace_points[n]);
//      cout << "n = " << n 
//           << " nearby_worldspace_point_range = "
//           << nearby_worldspace_point_ranges.back() 
//           << " nearby_worldspace_point = "
//           << nearby_worldspace_points.back() << endl;
   }
   
//   templatefunc::Quicksort(nearby_worldspace_point_ranges,
   templatefunc::Quicksort(weighted_ranges,
                           nearby_worldspace_points);
   closest_worldspace_point=nearby_worldspace_points.front();

//   double selected_point_range=nearby_worldspace_point_ranges.front();
//   double selected_point_rho=
//      ((closest_worldspace_point-ray_basepoint)
//       -selected_point_range*ray_ehat).magnitude();

//   cout << "n_nearby_candidates = " << n_nearby_candidates << endl;
//   cout << "Closest point = " << closest_worldspace_point << endl;
//   cout << "Selected point range from camera = " 
//        << selected_point_range << endl;
//   cout << "Selected point rho = " << selected_point_rho << endl;

//   double elapsed_time=timefunc::elapsed_timeofday_time();   
//   cout << "elapsed time = " << elapsed_time << endl;

   return (n_nearby_candidates > 0);
}

// ---------------------------------------------------------------------
bool PointFinder::find_altitude_given_longitude_and_latitude(
   double longitude,double latitude,double& altitude,double Z1)
{
   int UTM_zonenumber;
   bool northern_hemisphere_flag;
   double UTM_easting,UTM_northing;
   latlongfunc::LLtoUTM(
      latitude,longitude,UTM_zonenumber,northern_hemisphere_flag,
      UTM_northing,UTM_easting);

   return find_altitude_given_easting_and_northing(
      UTM_easting,UTM_northing,Z1,altitude);
}

// ---------------------------------------------------------------------
bool PointFinder::find_altitude_given_easting_and_northing(
   double UTM_easting,double UTM_northing,double& altitude,double Z1)
{
   const double tan_theta=5*PI/180;	// rads

// Note added on 9/16/09: Need to change +z_hat to -z_hat if Z1
// changes from negative to large positive value!

   threevector ray_basepoint(UTM_easting,UTM_northing,Z1);
   if (find_closest_world_point(
      ray_basepoint,z_hat,nearest_worldspace_point,tan_theta))
   {
      altitude=nearest_worldspace_point.get(2);
      return true;
   }
   else
   {
      return false;
   }
}

/*
// ---------------------------------------------------------------------
// Member function find_closest_world_point takes in screen
// coordinates X and Y for some point picked by the mouse.  It loops
// over all geodes within all DataGraphs and ignores those lying far
// away from the current line-of-sight ray.  It next uses the current
// 4x4 matrix that maps world to screen space to convert every
// candidate 3D point from world to screen space coordinates.  This
// method returns the transformed world point which lies closest in
// screen space to the input (X,Y) pair.

// We have made a reasonable effort to minimize execution time of this
// method.

threevector Transformer::find_closest_world_point(double X,double Y)
{
//   cout << "inside Transformer::find_closest_world_point()" << endl;

//   timefunc::initialize_timeofday_clock();

   world_to_screen_transformation();

// Following Ross Anderson's approach, first compute semi-infinite ray
// along current line-of-sight.  Geodes lying much too far from this
// ray cannot contain the closest world point:

   pair<threevector,threevector> R=compute_ray_into_screen(X,Y);
   linesegment LOS_ray(R.first,R.first+R.second);
   vector<osg::Geometry*> geoms_along_ray=
      DataGraphsGroup_ptr->geometries_along_ray(LOS_ray);

// For speed purposes, we do NOT work with genmatrix multiplication.
// Instead, we try to avoid as many C++ constructors and get calls and
// work with as many local variables as possible...

   double PV_00=PV_ptr->get(0,0);
   double PV_01=PV_ptr->get(0,1);
   double PV_02=PV_ptr->get(0,2);
   double PV_03=PV_ptr->get(0,3);

   double PV_10=PV_ptr->get(1,0);
   double PV_11=PV_ptr->get(1,1);
   double PV_12=PV_ptr->get(1,2);
   double PV_13=PV_ptr->get(1,3);
   
//   double PV_20=PV_ptr->get(2,0);
//   double PV_21=PV_ptr->get(2,1);
//   double PV_22=PV_ptr->get(2,2);
//   double PV_23=PV_ptr->get(2,3);
   
   double PV_30=PV_ptr->get(3,0);
   double PV_31=PV_ptr->get(3,1);
   double PV_32=PV_ptr->get(3,2);
   double PV_33=PV_ptr->get(3,3);

//   double w=1.0;
   double closest_x,closest_y,closest_z;
   double min_sqr_dist=POSITIVEINFINITY;

   if (DataGraphsGroup_ptr==NULL)
   {
      cout << "Error in Transformer::find_closest_world_point()!" << endl;
      cout << "DataGraphsGroup_ptr = " << DataGraphsGroup_ptr << endl;
      exit(-1);
   }

   double range;
   for (unsigned int g=0; g<geoms_along_LOS.size(); g++)
   {
      osg::Geometry* curr_Geometry_ptr=geoms_along_LOS[g];
      osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
         curr_Geometry_ptr->getVertexArray());

      for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
      {
         double x=curr_vertices_ptr->at(i).x();
         double y=curr_vertices_ptr->at(i).y();
         double z=curr_vertices_ptr->at(i).z();

         double new_w=PV_30*x+PV_31*y+PV_32*z+PV_33;
         double new_x=(PV_00*x+PV_01*y+PV_02*z+PV_03)/new_w;
         double new_y=(PV_10*x+PV_11*y+PV_12*z+PV_13)/new_w;
//       double new_z=(PV_20*x+PV_21*y+PV_22*z+PV_23*w)/new_w;

         double curr_sqr_dist=sqr(X-new_x)+sqr(Y-new_y);
         if (curr_sqr_dist < min_sqr_dist)
         {
            min_sqr_dist=curr_sqr_dist;
            closest_x=x;
            closest_y=y;
            closest_z=z;
            range=R.second.dot((threevector(x,y,z)-R.first));
         }
      } // loop over index i labeling vertices within *curr_Geometry_ptr
   } // loop over index g labeling geodes in *DataGraphsGroup_ptr

   threevector closest_worldspace_point(closest_x,closest_y,closest_z);
   cout << "Closest point = " << closest_worldspace_point << endl;
   cout << "range = " << range << endl;

//   double elapsed_time=timefunc::elapsed_timeofday_time();   
//   cout << "elapsed time = " << elapsed_time << endl;

//   cout << "at end of Transformer::find_closest_world_point()" << endl;
   return closest_worldspace_point;
}
*/
