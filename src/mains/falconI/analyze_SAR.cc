// ========================================================================
// Program ANALYZE_SAR works with synthetic SAR images exported by
// program GENERATE_SAR.  It searches for hot-spots and wraps bounding
// boxes around them.  Bbox centroids are then backprojected into
// frusta corresponding to the synthetic SAR images.  ANALYZE SAR
// exports a text file containing rays from frusta vertices to the Z=0
// ground plane which can be imported by program SAR_PROPAGATOR.  Rays
// should generally intersect around 3D locations corresponding to
// genuine SAR signal locations.
// ========================================================================
// Last updated on 10/3/12; 10/4/12; 11/26/12; 3/12/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "image/binaryimagefuncs.h"
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "video/connected_components.h"
#include "image/extremal_region.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "datastructures/tree.h"
#include "datastructures/union_find.h"
#include "video/videofuncs.h"

#include "general/outputfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);

//   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
//   int texturepass_ID=passes_group.get_curr_texturepass_ID();

   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string image_sizes_filename=passes_group.get_image_sizes_filename();
   cout << "image_sizes_filename = " << image_sizes_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group,image_sizes_filename);
   photogroup_ptr->set_bundler_IO_subdir(bundler_IO_subdir);
   int n_photos=photogroup_ptr->get_n_photos();
   cout << "n_photos = " << n_photos << endl;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   connected_components* connected_components_ptr=NULL;

   string output_subdir="./SAR_images/";
   filefunc::dircreate(output_subdir);

// Instantiate STL vector to hold 3D linesegments from frusta vertices
// to backprojections of SAR hot-spots into Z=0 plane:

   vector<linesegment> backprojected_rays;

   int n_start=0;
   int n_stop=n_photos;
//   int n_start=100;
//   int n_stop=101;
//   int delta_n=1;
   int delta_n=2;
//   int delta_n=3;
//   int delta_n=5;
//   int delta_n=10;

// Loop over SAR images starts here:

   for (int n=n_start; n<n_stop; n += delta_n)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      string image_filename=photo_ptr->get_filename();
      cout << "n = " << n << " image_filename = " << image_filename << endl;
      camera* camera_ptr=photo_ptr->get_camera_ptr();

      texture_rectangle_ptr->reset_texture_content(image_filename);
//      unsigned int width=texture_rectangle_ptr->getWidth();
//      unsigned int height=texture_rectangle_ptr->getHeight();

// Convert colored SAR images to greyscale based upon their hue
// content:


      texture_rectangle_ptr->convert_color_image_to_h_s_or_v(2);
//      string greyscale_filename="./greyscale.jpg";
//      texture_rectangle_ptr->write_curr_frame(greyscale_filename);

      twoDarray* ptwoDarray_ptr=texture_rectangle_ptr->get_ptwoDarray_ptr();

//      double z_threshold=0.85*255;
//      double z_threshold=0.9*255;
      double z_threshold=0.925*255;
      binaryimagefunc::binary_threshold(z_threshold,ptwoDarray_ptr);
      texture_rectangle_ptr->initialize_twoDarray_image(ptwoDarray_ptr,3);
      
      string thresholded_filename=output_subdir+"thresholded_"+
         stringfunc::integer_to_string(n,3)+".jpg";
      texture_rectangle_ptr->write_curr_frame(thresholded_filename);

      delete connected_components_ptr;   
      connected_components_ptr=new connected_components();

      int color_channel_ID=-1;	// value
      connected_components_ptr->reset_image(
         thresholded_filename,color_channel_ID,n);
      connected_components::TREE_PTR tree_ptr=connected_components_ptr->
         get_tree_ptr();

      int index=0;
      int threshold=128;
      int level=threshold;
      bool RLE_flag=true;
      bool invert_binary_values_flag=false;
      bool export_connected_regions_flag=false;

      connected_components_ptr->compute_connected_components(
         index,threshold,level,RLE_flag,invert_binary_values_flag,
         export_connected_regions_flag);
      cout << "tree.size() = " << tree_ptr->size() << endl;

      treenode<extremal_region*>* treenode_ptr=
         tree_ptr->reset_curr_treenodes_map_ptr(level);

//      int min_pixel_area=sqr(10);
      int min_pixel_area=sqr(20);
//      int min_pixel_area=sqr(25);
      vector<pair<int,int> > center_pixels;

      while (treenode_ptr != NULL)
      {
         extremal_region* extremal_region_ptr=treenode_ptr->get_data_ptr();
//         cout << "*extremal_region_ptr = " << *extremal_region_ptr << endl;
         int pixel_area=extremal_region_ptr->get_pixel_area();
         if (pixel_area < min_pixel_area) 
         {
            treenode_ptr=tree_ptr->get_next_treenode_ptr();
            continue;
         }
//         cout << "treenode counter = " << treenode_counter++ << endl;

         unsigned int min_px,min_py,max_px,max_py;
         extremal_region_ptr->get_bbox(min_px,min_py,max_px,max_py);
         int center_px=0.5*(min_px+max_px);
         int center_py=0.5*(min_py+max_py);
         pair<int,int> P(center_px,center_py);
         center_pixels.push_back(P);
         
         double u_center,v_center;
         texture_rectangle_ptr->get_uv_coords(
            center_px,center_py,u_center,v_center);
         
         threevector V1=camera_ptr->get_world_posn();
         threevector ray_hat=camera_ptr->pixel_ray_direction(
            u_center,v_center);

// Solve for range at which V2.z=0:

         double range=V1.get(2)/fabs(ray_hat.get(2));
         threevector V2=V1+range*ray_hat;

         linesegment curr_ray(V1,V2);
         backprojected_rays.push_back(curr_ray);

         treenode_ptr=tree_ptr->get_next_treenode_ptr();
      } // loop over treenodes

      int radius=10;
      texture_rectangle_ptr->reset_texture_content(image_filename);
      for (unsigned int c=0; c<center_pixels.size(); c++)
      {
         texture_rectangle_ptr->fill_circle(
            center_pixels[c].first,center_pixels[c].second,
            radius,colorfunc::pink);
      }
      
      string centers_filename=output_subdir+"centers_"+
         stringfunc::integer_to_string(n,3)+".jpg";
      texture_rectangle_ptr->write_curr_frame(centers_filename);
      cout << "Exported " << centers_filename << endl;

   } // loop over index n labeling aerial SAR images

// Instantiate union_find object to hold 3D rays which are identified
// as originating from same ground sources:

   union_find* union_find_ptr=new union_find();
   for (unsigned int i=0; i<backprojected_rays.size(); i++)
   {
      union_find_ptr->MakeSet(i);
   }

   double max_ground_separation_dist=75; // meters
   cout << "Enter max separation distance in Z=0 plane to associate two rays:"
        << endl;
   cin >> max_ground_separation_dist;

//   const double max_ground_separation_dist=50;		// meters
//   const double max_ground_separation_dist=66;		// meters
//   const double max_ground_separation_dist=75;		// meters
//   const double max_ground_separation_dist=100;	// meters
//   const double max_ground_separation_dist=150;	// meters
   for (unsigned int i=0; i<backprojected_rays.size(); i++)
   {
      if (i%1000==0) cout << i/1000 << " " << flush;
      linesegment curr_ray=backprojected_rays[i];
      threevector curr_V2=curr_ray.get_v2();

      for (unsigned int j=i+1; j<backprojected_rays.size(); j++)
      {
         linesegment next_ray=backprojected_rays[j];
         threevector next_V2=next_ray.get_v2();

         double ground_points_separation=(curr_V2-next_V2).magnitude();
         if (ground_points_separation < max_ground_separation_dist)
         {
            union_find_ptr->Link(i,j);
         }
      } // loop over index j labeling 3D rays
   } // loop over index i labeling 3D rays
   cout << endl;

// Remap root IDs to unique, sequential color indices:

   map<int,pair<int,int> > color_map;
   map<int,pair<int,int> >::iterator iter;

// Independent var: root_ID
// Dependent var #1: color_index
// Dependent var #2: root ID multiplicity

   for (unsigned int node_ID=0; node_ID<union_find_ptr->get_n_nodes(); 
        node_ID++)
   {
//      int parent_ID=union_find_ptr->get_parent_ID(node_ID);
      int root_ID=union_find_ptr->Find(node_ID);

      iter=color_map.find(root_ID);
      if (iter==color_map.end())
      {
         pair<int,int> P(-1,1);
         color_map[root_ID]=P;
      }
      else
      {
         iter->second.second=iter->second.second+1;
      }
      
//      cout << "node_ID = " << node_ID
//           << " parent_ID = " << parent_ID 
//           << " root_ID = " << root_ID
//           << endl;
   } // loop over node_ID index

// Export polylines text file containing 3D rays colored to indicate
// common SAR ground targets:

   string polylines_filename=output_subdir+"SAR_LOS_polylines_3D.txt";
   ofstream outstream;
   filefunc::openfile(polylines_filename,outstream);
   outstream << "# Time   PolyLine_ID   Passnumber   X Y Z R G B A"
             << endl << endl;
   outstream.precision(12);

   int color_index=0;
   int polyline_ID=0;
//   const int min_root_ID_multiplicity=4;
   int min_root_ID_multiplicity=30;
//   const int min_root_ID_multiplicity=30;
//   const int min_root_ID_multiplicity=50;

   cout << "Min root ID multiplicity to define a ray cluster:" << endl;
   cin >> min_root_ID_multiplicity;
   
   for (unsigned int i=0; i<backprojected_rays.size(); i++)
   {
      int root_ID=union_find_ptr->Find(i);
      iter=color_map.find(root_ID);

// Reject any rays which do not belong to a sufficiently large ground
// cluster:

      int root_ID_multiplicity=iter->second.second;
      cout << "i = " << i << " root_ID_multiplicity = "
           << root_ID_multiplicity << endl;
      
      if (root_ID_multiplicity < min_root_ID_multiplicity) continue;

      linesegment curr_ray=backprojected_rays[i];      
      int curr_color_index=iter->second.first;
      if (curr_color_index < 0)
      {
         curr_color_index=color_index;
         iter->second.first=color_index;
         color_index++;
      }

      osg::Vec4 ray_color=colorfunc::get_OSG_color(
         colorfunc::get_color(curr_color_index));

      outstream << "0   " << polyline_ID << "  0 "
                << curr_ray.get_v1().get(0) << "  "
                << curr_ray.get_v1().get(1) << "  "
                << curr_ray.get_v1().get(2) << "  "
                << ray_color.r() << "  "
                << ray_color.g() << "  "
                << ray_color.b() << "  "
                << ray_color.a() << endl;

      outstream << "0   " << polyline_ID << "  0 "
                << curr_ray.get_v2().get(0) << "  "
                << curr_ray.get_v2().get(1) << "  "
                << curr_ray.get_v2().get(2) << "  "
                << ray_color.r() << "  "
                << ray_color.g() << "  "
                << ray_color.b() << "  "
                << ray_color.a() << endl;

      polyline_ID++;
   }
   filefunc::closefile(polylines_filename,outstream);

   string banner="Exported "+polylines_filename;
   outputfunc::write_big_banner(banner);

   delete union_find_ptr;

   cout << "INPUTS:" << endl << endl;
   cout << "Max ground separation distance = " << max_ground_separation_dist
        << endl;
   cout << "Min number of rays per cluster = " << min_root_ID_multiplicity
        << endl;

   cout << "OUTPUTS:" << endl << endl;
   cout << "Number of exported rays = " << polyline_ID << endl;
   cout << "Number of ray clusters = " << color_index << endl;
}

