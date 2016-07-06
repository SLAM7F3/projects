// ========================================================================
// Program IMAGE_OVERLAP_GRAPH loops over all aerial video frames
// specified by input campaign and mission IDs.  It first defines a
// bounding box based upon the aerial camera's track through UTM
// space.  For each video frame, IMAGE_OVERLAP_GRAPH reconstructs the
// camera based upon parameters retrieved from the sensor_metadata
// table of the IMAGERY database.  It loops over a 10x10 pixel lattice
// for each video frame and backprojects the sampled pixels down onto
// a ground z-plane.  If the backprojected pixel area is less than a
// ground voxel's XY area, the image ID and backprojected pixel's
// inverse area are added to the voxel's STL map.  Two different image
// entries within a ground voxel's STL map define an edge between two
// image nodes.  The edge weight is set equal to the minimum of the
// image weights.

// Once all aerial video frames have been processed, a node-node-edge
// STL map is instantiated and filled with data extracted from all
// ground voxel STL maps.  Finally, an edge-list is exported
// to an output text file for subsequent image graph processing.

// ========================================================================
// Last updated on 2/24/12; 2/25/12; 2/27/12
// ========================================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "geometry/bounding_box.h"
#include "general/filefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::map;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Constants declarations:

   const double z_groundplane=1220; // meters = 4000 ft 
				    // (reasonable for Tucson border)

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int campaign_ID,mission_ID;
//   cout << "Enter campaign_ID:" << endl;
//   cin >> campaign_ID;
   campaign_ID=3;	// Tstorm

//   cout << "Enter mission_ID:" << endl;
//   cin >> mission_ID;
   mission_ID=0;	// Tstorm

// Set bounding box in ground UTM space based upon aerial camera's
// flight path:

   vector<threevector> camera_posns;
   imagesdatabasefunc::retrieve_all_sensor_posns_from_database(
      postgis_db_ptr,campaign_ID,mission_ID,camera_posns);

   bounding_box camera_bbox;
   camera_bbox.recompute_bounding_box(camera_posns);
//   cout << "camera_bbox = " << camera_bbox << endl;

   double xlo=camera_bbox.get_xmin()-5000;
   double xhi=camera_bbox.get_xmax()+5000;
   const double dx=3;	// meters
   int npx=(xhi-xlo)/dx+1;
   xhi=xlo+npx*dx;

   double ylo=camera_bbox.get_ymin()-5000;
   double yhi=camera_bbox.get_ymax()+5000;
   const double dy=3;	// meters
   int npy=(yhi-ylo)/dy+1;
   yhi=ylo+npy*dy;

   double ground_cell_area=dx*dy;

   cout.precision(12);
   cout << "xlo = " << xlo << " xhi = " << xhi 
        << " xhi-xlo = " << xhi-xlo 
        << " npx = " << npx << endl;
   cout << "ylo = " << ylo << " yhi = " << yhi 
        << " yhi-ylo = " << yhi-ylo
        << " npy = " << npy << endl;

// Instantiate STL map *ground_map_ptr
// Dependent vars: px,py = integer labels for ground map voxel
// Independent vars: image_ID (int) and weight (double)

   typedef map<int,double> INT_DOUBLE_MAP;
   typedef map<pair<int,int> , INT_DOUBLE_MAP* > GROUND_MAP;
   GROUND_MAP* ground_map_ptr=new GROUND_MAP;

// Set up 10x10 grid of image plane pixels.  We will backproject each
// of these sample pixels into UTM ground space:

   int npu,npv;
   imagesdatabasefunc::get_zeroth_image_npx_npy(
      postgis_db_ptr,campaign_ID,mission_ID,npu,npv);
//   cout << "npu = " << npu << " npv = " << npv << endl;

//   double umin=0;
   double umax=double(npu)/double(npv);
//   double vmin=0;
//   double vmax=1;

   int dpu=npu/12;
   int dpv=npv/12;
   int pu_start=0+dpu;
   int pv_start=0+dpv;
   int pu_stop=npu-dpu;
   int pv_stop=npv-dpv;

   int n_images=camera_posns.size();
   threevector posn,az_el_roll,f_u0_v0;
   camera* camera_ptr=new camera();
   vector<threevector> ground_pts;

// Open text file for recording backprojected image locations:

   string backprojected_image_location_filename=
      "backprojected_image_posns.dat";
   ofstream backprojected_image_location_stream;
   backprojected_image_location_stream.precision(12);
   
   filefunc::openfile(
      backprojected_image_location_filename,
      backprojected_image_location_stream);

   backprojected_image_location_stream 
      << "# campaign_ID" << " "
      << "mission_ID"  << " "
      << "image_ID" << " "
      << "backprojected easting" << " "
      << "backprojected northing" << " "
      << endl << endl;

   int n_total_pixels=0;

// ------------------------------------------------------------------------
// Loop over aerial video frames starts here
// ------------------------------------------------------------------------

   for (int image_ID=0; image_ID<n_images; image_ID++)
   {
      if (image_ID%100==0) cout << image_ID << " " << flush;
      
      imagesdatabasefunc::retrieve_particular_sensor_metadata_from_database(
         postgis_db_ptr,campaign_ID,mission_ID,image_ID,
         posn,az_el_roll,f_u0_v0);
      
      double az=az_el_roll.get(0)*PI/180;
      double el=az_el_roll.get(1)*PI/180;
      double roll=az_el_roll.get(2)*PI/180;
      
      camera_ptr->set_world_posn(posn);
      camera_ptr->set_Rcamera(az,el,roll);
      camera_ptr->set_f(f_u0_v0.get(0));
      camera_ptr->set_u0(f_u0_v0.get(1));
      camera_ptr->set_v0(f_u0_v0.get(2));
      camera_ptr->construct_projection_matrix();

//      cout << "camera = " << *camera_ptr << endl;
//      outputfunc::enter_continue_char();

      double avg_backprojected_pixel_area=0;
      int n_backprojected_pixels=0;
      twovector avg_backprojected_XY(0,0);

      for (int pu=pu_start; pu<pu_stop; pu += dpu)
      {
         double u=double(pu)/double(npu)*umax;
         double up=double(pu+1)/double(npu)*umax;
         for (int pv=pv_start; pv<pv_stop; pv += dpv)
         {
            n_total_pixels++;
            
            double v=double(pv)/double(npv);
            double vp=double(pv+1)/double(npv);
            threevector r_hat=camera_ptr->pixel_ray_direction(u,v);

// Ignore any ray whose direction vector is pointed upwards towards sky:

            if (r_hat.get(2) > 0) continue;

// Backproject pixel's 4 corners towards ground.  Compute
// backprojected quadrilateral's area within ground z-plane.  Take
// pixel's weight to equal 1/quadrilateral area:

            ground_pts.clear();
            ground_pts.push_back(camera_ptr->backproject_imagepoint_to_zplane(
               u,v,z_groundplane));
            ground_pts.push_back(camera_ptr->backproject_imagepoint_to_zplane(
               up,v,z_groundplane));
            ground_pts.push_back(camera_ptr->backproject_imagepoint_to_zplane(
               up,vp,z_groundplane));
            ground_pts.push_back(camera_ptr->backproject_imagepoint_to_zplane(
               u,vp,z_groundplane));
               
//            cout << "ground_pts[0] = " << ground_pts[0] << endl;
//            cout << "ground_pts[1] = " << ground_pts[1] << endl;
//            cout << "ground_pts[2] = " << ground_pts[2] << endl;
//            cout << "ground_pts[3] = " << ground_pts[3] << endl;

            double tri1_area=geometry_func::compute_triangle_area(
               ground_pts[0],ground_pts[1],ground_pts[2]);
            double tri2_area=geometry_func::compute_triangle_area(
               ground_pts[2],ground_pts[3],ground_pts[0]);
            double backprojected_pixel_area=tri1_area+tri2_area;
//            cout << "backprojected_pixel_area = "
//                 << backprojected_pixel_area << endl;

            if (backprojected_pixel_area > ground_cell_area) continue;
            double weight=1.0/backprojected_pixel_area;

// Find averaged px,py integer labels for backprojected pixel:

            threevector avg_ground_pt=
               0.25*(ground_pts[0]+ground_pts[1]+ground_pts[2]+ground_pts[3]);
            int avg_px=(avg_ground_pt.get(0)-xlo)/dx;
            int avg_py=(avg_ground_pt.get(1)-ylo)/dy;

            double curr_backprojected_x=xlo+avg_px*dx;
            double curr_backprojected_y=ylo+avg_py*dy;

            pair<int,int> indep_var(avg_px,avg_py);
            GROUND_MAP::iterator iter=ground_map_ptr->find(indep_var);

// Store image ID and pixel weight within STL map
// *image_IDs_weights_map_ptr corresponding to avg_px,avg_py:

            INT_DOUBLE_MAP* image_IDs_weights_map_ptr=NULL;

            if (iter==ground_map_ptr->end())
            {
               image_IDs_weights_map_ptr=new INT_DOUBLE_MAP;
               (*ground_map_ptr)[indep_var]=image_IDs_weights_map_ptr;
            }
            else
            {
               image_IDs_weights_map_ptr=iter->second;
            }

            INT_DOUBLE_MAP::iterator dependent_iter=
               image_IDs_weights_map_ptr->find(image_ID);
            if (dependent_iter==image_IDs_weights_map_ptr->end())
            {
               (*image_IDs_weights_map_ptr)[image_ID]=weight;
            }
            else
            {

// Compare current image pixel weight with existing weight already
// stored in ground voxel location.  If latter is larger than former,
// replace ground voxel weight value:

               double existing_weight=dependent_iter->second;
               if (existing_weight < weight)
               {
                  dependent_iter->second=weight;
               }
            }
            
            avg_backprojected_pixel_area += backprojected_pixel_area;
            avg_backprojected_XY += twovector(
               curr_backprojected_x,curr_backprojected_y);
            

            n_backprojected_pixels++;

         } // loop over pv index
      } // loop over pu index

      if (n_backprojected_pixels==0) continue;

      avg_backprojected_pixel_area /= n_backprojected_pixels;

      avg_backprojected_XY /= n_backprojected_pixels;
      backprojected_image_location_stream 
         << campaign_ID << "  "
         << mission_ID  << "  "
         << image_ID << "  "
         << avg_backprojected_XY.get(0) << "  "
         << avg_backprojected_XY.get(1) << "  "
         << endl;
      
      if (image_ID%100==0) 
      {
         string image_URL=imagesdatabasefunc::get_image_URL(
            postgis_db_ptr,campaign_ID,mission_ID,image_ID);

         cout << "image = " << image_ID
              << " URL = " << filefunc::getbasename(image_URL) << endl;
         cout << " sqrt(Avg thresholded backprojected pixel area) = "
              << sqrt(avg_backprojected_pixel_area) << endl << endl;
      }
   } // loop over image_ID index

   filefunc::closefile(
      backprojected_image_location_filename,
      backprojected_image_location_stream);

// ------------------------------------------------------------------------
// Loop over aerial video frames stops here
// ------------------------------------------------------------------------

   delete camera_ptr;
   cout << "n_total_pixels = " << n_total_pixels << endl;
   cout << "ground_map_ptr->size = " << ground_map_ptr->size() << endl;

// Instantiate node-node-edge map:

   typedef map<pair<int,int> , double> NODE_NODE_EDGE_MAP;
   NODE_NODE_EDGE_MAP* node_node_edge_map_ptr=new NODE_NODE_EDGE_MAP;

// Iterate over ground map voxels and consolidate dependent pairs
// s.t. each image ID appears only once within the dependent STL vector:

   for (GROUND_MAP::iterator iter=ground_map_ptr->begin();
        iter != ground_map_ptr->end(); ++iter)
   {
      INT_DOUBLE_MAP* image_IDs_weight_map_ptr=iter->second;

      vector<int> node_IDs;
      vector<double> node_weights;
      for (INT_DOUBLE_MAP::iterator dependent_iter=image_IDs_weight_map_ptr->
              begin(); dependent_iter != image_IDs_weight_map_ptr->end();
           ++dependent_iter)
      {
         int curr_node_ID=dependent_iter->first;
         double curr_node_weight=dependent_iter->second;
         node_IDs.push_back(curr_node_ID);
         node_weights.push_back(curr_node_weight);
      }

      for (unsigned int i=0; i<node_weights.size(); i++)
      {
         int curr_node_ID=node_IDs[i];
         double curr_node_weight=node_weights[i];
         for (unsigned int j=i+1; j<node_weights.size(); j++)
         {
            int next_node_ID=node_IDs[j];
            double next_node_weight=node_weights[j];
            double edge_weight=basic_math::min(
               curr_node_weight,next_node_weight);
            
// Search for curr_node_ID, next_node_ID, edge_weight triple within 
// *node_node_edge_map_ptr:
            
            int node_ID1=basic_math::min(curr_node_ID,next_node_ID);
            int node_ID2=basic_math::max(curr_node_ID,next_node_ID);
            pair<int,int> node_node_indep_var(node_ID1,node_ID2);
            
            NODE_NODE_EDGE_MAP::iterator iter=node_node_edge_map_ptr->find(
               node_node_indep_var);
            if (iter==node_node_edge_map_ptr->end())
            {
               (*node_node_edge_map_ptr)[node_node_indep_var]=edge_weight;
            }
            else
            {
               double existing_edge_weight=iter->second;
               if (edge_weight > existing_edge_weight)
               {
                  iter->second=edge_weight;
               }
            }
         } // loop over index j
      } // loop over index i
   } // iterator over ground map voxels
   delete ground_map_ptr;

// Iterate over node-node-edge map and exports its contents to an
// edge-list text file:

   string output_filename="geom_edgelist.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# Edge weight threshold = 0 " << endl;
   outstream << "# NodeID  NodeID'  Edge weight" << endl;
   outstream << endl;

   for (NODE_NODE_EDGE_MAP::iterator iter=node_node_edge_map_ptr->begin();
        iter != node_node_edge_map_ptr->end(); ++iter)
   {
      pair<int,int> node_node_IDs=iter->first;
      double edge_weight=iter->second;

      outstream << node_node_IDs.first << "   "
                << node_node_IDs.second << "   "
                << edge_weight << endl;
   }
   filefunc::closefile(output_filename,outstream);   
   delete node_node_edge_map_ptr;

   string banner="Wrote geometry-derived edge list to "+output_filename;
   outputfunc::write_big_banner(banner);
}

