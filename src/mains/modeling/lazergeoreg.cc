// ========================================================================
// Program LAZERGEOREG is a specialized program which attempts to find
// a global transformation that optimally georegisters Nick
// Armstrong-Crews' lazerkart point cloud with our 3D MIT building
// models.  It takes in a set of planar facets manually extracted from 
// building sides in the lazerkart point cloud.  It also has the
// unique building model ID associated with each lazerkart planar
// facet hardwired into STL vector bldg_ID_for_facet.  LAZERGEOREG
// iteratively performs a brute-force search for the azimuthal
// rotation angle, XY translation and XY scaling which minimizes the
// distances between the planar facets and their nearest building
// models.  

// After running this program multiple times on 6/29/12, we strongly
// suspect there is NO global transformation which can really align
// the spring 2012 lazerkart data with our models.  Instead, we
// believe cumulative "poor-man" SLAM solution errors would require a
// local XY warp in order to obtain reasonable agreement with the 3D
// models.

//				lazergeoreg

// ========================================================================
// Last updated on 6/28/12; 6/29/12; 7/6/12
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "models/Building.h"
#include "models/BuildingsGroup.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "numerical/param_range.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

// ==========================================================================


double compute_total_score(
   const threevector& rotation_origin,const rotation& R,
   const threevector& scalefactor,const threevector& trans,
   PolyLinesGroup* lazerkart_PolyLinesGroup_ptr,
   BuildingsGroup* BuildingsGroup_ptr,
   const vector<int>& bldg_ID_for_facet)
{
   vector<double> lazer_face_distances;

   double avg_lazer_face_distance=0;
   int n_lazerkart_polygons=lazerkart_PolyLinesGroup_ptr->get_n_Graphicals();
   for (int l=0; l<n_lazerkart_polygons; l++)
   {
      PolyLine* lazerkart_PolyLine_ptr=lazerkart_PolyLinesGroup_ptr->
         get_PolyLine_ptr(l);
      polyline* lazerkart_polyline_ptr=lazerkart_PolyLine_ptr->
         get_polyline_ptr();
      polygon* lazerkart_polygon_ptr=new polygon(
         *lazerkart_polyline_ptr);
//      cout << "l = " << l 
//           << " lazerkart poly = " << *lazerkart_polygon_ptr << endl;

      lazerkart_polygon_ptr->translate(-rotation_origin);
      lazerkart_polygon_ptr->rotate(R);      
      lazerkart_polygon_ptr->scale(rotation_origin,scalefactor);
      lazerkart_polygon_ptr->translate(rotation_origin);
      lazerkart_polygon_ptr->translate(trans);

      double min_lazer_face_distance=POSITIVEINFINITY;
      for (int b=0; b<BuildingsGroup_ptr->get_n_Buildings(); b++)
      {
         Building* Building_ptr=BuildingsGroup_ptr->get_Building_ptr(
            b);
         int Building_ID=Building_ptr->get_ID();
         if (Building_ID != bldg_ID_for_facet[l]) continue;

         vector<polyhedron*> polyhedra_ptrs=Building_ptr->
            get_polyhedra_ptrs();
         for (int p=0; p<polyhedra_ptrs.size(); p++)
         {
            polyhedron* polyhedron_ptr=polyhedra_ptrs[p];
            for (int f=0; f<polyhedron_ptr->get_n_faces(); f++)
            {
               face* face_ptr=polyhedron_ptr->get_face_ptr(f);
               threevector normal=face_ptr->get_normal();
               double abs_dotproduct=fabs(normal.dot(
                  lazerkart_polygon_ptr->get_normal()));

// cos (10 deg) = 0.984
// cos (15 deg) = 0.9659

               if (abs_dotproduct < 0.9659) continue;

// Compute distance from each vertex of *lazerkart_polygon_ptr to
// *face_ptr.  Ignore any face which lies more than some reasonable
// distance away from any polygon vertex:

               polygon face_poly(face_ptr->compute_face_poly());
               vector<threevector> face_vertices;
               for (int fv=0; fv<face_poly.get_nvertices(); fv++)
               {
                  face_vertices.push_back(face_poly.get_vertex(fv));
               } // loop over index fv labeling current face vertices
               plane* face_plane_ptr=NULL;
               if (face_vertices.size()==3)
               {
                  face_plane_ptr=new plane(
                     face_vertices[0],face_vertices[1],
                     face_vertices[2]);
               }
               else
               {
                  face_plane_ptr=new plane(face_vertices);
               }

               face_vertices.push_back(face_vertices.front());
               polyline face_polyline(face_vertices);

               bool lazerkart_polygon_close_to_face_flag=true;
               double mean_distance_to_plane=0;

               for (int v=0; v<lazerkart_polygon_ptr->
                       get_nvertices(); v++)
               {
                  threevector V=lazerkart_polygon_ptr->get_vertex(v);
                  double distance_to_plane=face_plane_ptr->
                     signed_distance_from_plane(V);
                  const double max_distance_to_plane=7;   // meters

                  double distance_to_face_edges=
                     face_polyline.min_distance_to_point(V);
                  const double max_distance_to_edges=20; // meters

//                  cout << "l = " << l 
//                       << " p = " << p << " f = " << f << " v = " << v 
//                       << " dist_to_plane = " << distance_to_plane
//                       << " dist_to_edges = " << distance_to_face_edges
//                       << endl;
                  if (fabs(distance_to_plane) > 
                  max_distance_to_plane ||
                  distance_to_face_edges > max_distance_to_edges)
                  {
                     lazerkart_polygon_close_to_face_flag=false;
                     break;
                  }
                  else
                  {
                     mean_distance_to_plane += 
                        fabs(distance_to_plane);
                  }
               } // loop over index v labeling lazerkart 
//				polygon vertices
               
               delete face_plane_ptr;
               
               if (lazerkart_polygon_close_to_face_flag)
               {
                  mean_distance_to_plane /= 
                     lazerkart_polygon_ptr->get_nvertices();

                  if (mean_distance_to_plane < 
                  0.999*min_lazer_face_distance)
                  {
                     min_lazer_face_distance=mean_distance_to_plane;
//                     cout << "lazerkart polygon ID = " << l << endl;
//                     cout << "Building ID = " << Building_ID << endl;
//                     cout << "Polyhedron ID = " 
//                          << polyhedron_ptr->get_ID() 
//                          << endl;
//                     cout << "face ID = " << face_ptr->get_ID() 
//                          << endl;
//                     cout << "min_lazer_face_distance = " 
//                          << min_lazer_face_distance << endl;
//                     cout << endl;
                  }
                  
               }
               else
               {
                  continue;
               }

            } // loop over index f labeling polyhedron faces
         } // loop over index p labeling building polyhedra
      } // loop over index b labeling Buildings
      avg_lazer_face_distance += min_lazer_face_distance;

      if (min_lazer_face_distance < 500)
         lazer_face_distances.push_back(min_lazer_face_distance);
      
   } // loop over index l labeling facets extracted from lazer data

   avg_lazer_face_distance /= n_lazerkart_polygons;
//   return avg_lazer_face_distance;

   if (lazer_face_distances.size()==n_lazerkart_polygons)
   {
      int n_output_bins=100;
      prob_distribution distance_prob(lazer_face_distances,n_output_bins);
      double x_50=distance_prob.median();
//      double x_75=distance_prob.find_x_corresponding_to_pcum(0.75);
//      double x_85=distance_prob.find_x_corresponding_to_pcum(0.85);

      cout << "avg_dist = " << avg_lazer_face_distance
           << " median dist = " << x_50
//           << " 75_dist = " << x_75 
//           << " 85_dist = " << x_85 
           << endl;

      return avg_lazer_face_distance;
//      return x_50;
//      return x_75;
//      return x_85;      
   }
   else
   {
      return POSITIVEINFINITY;
   }
}

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;

   WindowManager* window_mgr_3D_ptr=new ViewerManager();
   ModeController* ModeController_3D_ptr=new ModeController();
   window_mgr_3D_ptr->get_EventHandlers_ptr()->push_back( 
      new ModeKeyHandler(ModeController_3D_ptr) );

// Add custom manipulators to the event handler list:

   osgGA::Terrain_Manipulator* CM_3D_ptr=new osgGA::Terrain_Manipulator(
      ModeController_3D_ptr,window_mgr_3D_ptr);
   window_mgr_3D_ptr->set_CameraManipulator(CM_3D_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations_3D(
      window_mgr_3D_ptr,ModeController_3D_ptr,CM_3D_ptr);

// Instantiate BuildingsGroup object:

   BuildingsGroup* BuildingsGroup_ptr=new BuildingsGroup();
   string OFF_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/modeling/OFF/";
   BuildingsGroup_ptr->import_from_OFF_files(OFF_subdir);

// Import (manually extracted) planar facets from lazerkart data along
// a few building walls:

   PolyLinesGroup* lazerkart_PolyLinesGroup_ptr=decorations_3D.add_PolyLines(
      3,passes_group.get_pass_ptr(cloudpass_ID));
   lazerkart_PolyLinesGroup_ptr->set_width(5);
   
//   string facet_polylines_filename="all_lazerkart_facet_polylines.dat";
//   string facet_polylines_filename="health_center_street_polylines.dat";
   string facet_polylines_filename="rebeccas_polylines.dat";
   lazerkart_PolyLinesGroup_ptr->reconstruct_polylines_from_file_info(
      facet_polylines_filename);

   vector<int> bldg_ID_for_facet;

// health_center_street_polylines.dat

   bldg_ID_for_facet.push_back(8);
   bldg_ID_for_facet.push_back(9);
   bldg_ID_for_facet.push_back(7);
   bldg_ID_for_facet.push_back(7);
   bldg_ID_for_facet.push_back(7);
   bldg_ID_for_facet.push_back(7);
   bldg_ID_for_facet.push_back(9);

/*
// health_center_street_polylines.dat

   bldg_ID_for_facet.push_back(0);
   bldg_ID_for_facet.push_back(0);
   bldg_ID_for_facet.push_back(2);
   bldg_ID_for_facet.push_back(2);
   bldg_ID_for_facet.push_back(4);
   bldg_ID_for_facet.push_back(3);
   bldg_ID_for_facet.push_back(3);
*/

/*
// all_lazerkart_facet_polylines.dat

   bldg_ID_for_facet.push_back(8);
   bldg_ID_for_facet.push_back(19);
   bldg_ID_for_facet.push_back(19);
   bldg_ID_for_facet.push_back(0);
   bldg_ID_for_facet.push_back(0);
   bldg_ID_for_facet.push_back(9);
   bldg_ID_for_facet.push_back(7);

   bldg_ID_for_facet.push_back(2);
   bldg_ID_for_facet.push_back(2);
   bldg_ID_for_facet.push_back(21);
   bldg_ID_for_facet.push_back(27);
   bldg_ID_for_facet.push_back(27);
   bldg_ID_for_facet.push_back(4);
   bldg_ID_for_facet.push_back(3);
   bldg_ID_for_facet.push_back(7);

   bldg_ID_for_facet.push_back(7);
   bldg_ID_for_facet.push_back(7);
   bldg_ID_for_facet.push_back(9);
   bldg_ID_for_facet.push_back(3);
*/

/*
   param_range az(0,0,1);
   param_range el(0,0,1);
   param_range roll(0,0,1);
   param_range scalefactor(1,1,1);
   param_range x_trans(0,0,1);
   param_range y_trans(0,0,1);

Best az value = 0
Best el value = 0
Best roll value = 0
Best scale value = 1
Best x_trans value = 0
Best y_trans value = 0
Average lazer-face distance = 4.02691
n_iters = 1

*/

   param_range az(-8,8,5);
//   param_range el(-0.01,0.01,3);
//   param_range roll(-0.01,0.01,3);
   param_range el(0,0,1);
   param_range roll(0,0,1);
   param_range x_scalefactor(0.999 , 1.001, 3);
   param_range y_scalefactor(0.999 , 1.001, 3);
   param_range x_trans(-10,10,5);
   param_range y_trans(-10,10,5);

/*

  Best az value = -2.28556
  Best el value = 0
  Best roll value = 0
  Best scale value = 1
  Best x_trans value = 3.14715
  Best y_trans value = -2.13541
  Average lazer-face distance = 1.64949
  n_lazerkart_polygons = 15
  n_iters = 10

Best az value = -2.28481
Best el value = 0.0181756
Best roll value = 0.0181756
Best x_scale value = 1
Best y_scale value = 1
Best x_trans value = 3.14981
Best y_trans value = -2.12896
Average lazer-face distance = 1.6488
n_lazerkart_polygons = 15
n_iters = 10


Best az value = -2.36471
Best el value = 0
Best roll value = 0
Best x_scale value = 1
Best y_scale value = 1
Best x_trans value = 3.08188
Best y_trans value = -3.00164
50% lazer-face distance = 0.780646
n_lazerkart_polygons = 19
n_iters = 10


Best az value = -1.57011
Best el value = 0
Best roll value = 0
Best x_scale value = 1
Best y_scale value = 1
Best x_trans value = 4.27679
Best y_trans value = -0.0778611
Avg lazer-face distance = 1.65165
n_lazerkart_polygons = 19
n_iters = 10
*/

   rotation R;   
   const threevector rotation_origin(328192,4692011,0);

//   int n_iters=1;
//   int n_iters=5;
//   int n_iters=7;
   int n_iters=10;
   double min_score=1000;
   for (int iter=0; iter<n_iters; iter++)
   {
      cout << "iter = " << iter+1 << " of " << n_iters << endl;

// ========================================================================
// Begin while loop over global transformation parameters
// ========================================================================

      while (az.prepare_next_value())
      {
         cout << "curr_az = " << az.get_value() << endl;
         double curr_az=az.get_value()*PI/180;
         
         while (el.prepare_next_value())
         {
            cout << "   curr_el = " << el.get_value() << endl;
            double curr_el=el.get_value()*PI/180;
            while (roll.prepare_next_value())
            {
               double curr_roll=roll.get_value()*PI/180;
               R=R.rotation_from_az_el_roll(curr_az,curr_el,curr_roll);
               
               while (x_scalefactor.prepare_next_value())
               {
                  double curr_x_scalefactor=x_scalefactor.get_value();
                  while (y_scalefactor.prepare_next_value())
                  {
                     double curr_y_scalefactor=y_scalefactor.get_value();
                     threevector scalefactor(
                        curr_x_scalefactor,curr_y_scalefactor,1);

                     while (x_trans.prepare_next_value())
                     {
                        while (y_trans.prepare_next_value())
                        {
                           threevector trans(
                              x_trans.get_value(),y_trans.get_value());

                           double avg_lazer_face_distance=compute_total_score(
                              rotation_origin,R,scalefactor,trans,
                              lazerkart_PolyLinesGroup_ptr,BuildingsGroup_ptr,
                              bldg_ID_for_facet);

                           if (avg_lazer_face_distance < min_score)
                           {
                              min_score=avg_lazer_face_distance;
                              cout << "          Curr lazer-face distance = " 
                                   << min_score << endl;
                              az.set_best_value();
                              el.set_best_value();
                              roll.set_best_value();
                              x_scalefactor.set_best_value();
                              y_scalefactor.set_best_value();
                              x_trans.set_best_value();
                              y_trans.set_best_value();
                           }
            
                        } // y_trans while loop
                     } // x_trans while loop
                  } // y_scalefactor while loop
               } // x_scalefactor while loop
            } // roll while loop
         } // el while loop
      } // az while loop
      
// ========================================================================
// End while loop over global transformation parameters
// ========================================================================

//      double frac=0.45;
      double frac=0.55;
      az.shrink_search_interval(az.get_best_value(),frac);
      el.shrink_search_interval(el.get_best_value(),frac);
      roll.shrink_search_interval(roll.get_best_value(),frac);
      x_scalefactor.shrink_search_interval(
         x_scalefactor.get_best_value(),frac);
      y_scalefactor.shrink_search_interval(
         y_scalefactor.get_best_value(),frac);
      x_trans.shrink_search_interval(x_trans.get_best_value(),frac);
      y_trans.shrink_search_interval(y_trans.get_best_value(),frac);

      cout << endl;
      cout << "=================================================" << endl;
      cout << "Best az value = " << az.get_best_value() << endl;
      cout << "Best el value = " << el.get_best_value() << endl;
      cout << "Best roll value = " << roll.get_best_value() << endl;
      cout << "Best x_scale value = " << x_scalefactor.get_best_value() 
           << endl;
      cout << "Best y_scale value = " << y_scalefactor.get_best_value() 
           << endl;
      cout << "Best x_trans value = " << x_trans.get_best_value() << endl;
      cout << "Best y_trans value = " << y_trans.get_best_value() << endl;
      cout << "lazer-face distance = " << min_score << endl;
      cout << "n_lazerkart_polygons = " 
           << lazerkart_PolyLinesGroup_ptr->get_n_Graphicals() << endl;
      cout << "n_iters = " << n_iters << endl;
      cout << "=================================================" << endl;
      cout << endl;

   } // loop over iter index

   string banner="Import these values into PLY2TDP in order to georegister lazerkart point cloud";
   outputfunc::write_big_banner(banner);

}
