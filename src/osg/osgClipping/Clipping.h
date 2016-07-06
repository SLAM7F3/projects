// ==========================================================================
// Header file for CLIPPING class
// ==========================================================================
// Last modified on 8/29/12; 8/30/12; 11/16/12; 4/6/14
// ==========================================================================

#ifndef CLIPPING_H
#define CLIPPING_H

#include <deque>
#include <map>
#include <string>
#include <vector>
#include <osg/Group>

#include "models/BuildingsGroup.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "video/texture_rectangle.h"
#include "math/twovector.h"

class edge;
class face;
class PixelBuffer;
class polyhedron;

class Clipping 
{

  public:

   typedef std::map<edge*,twovector> CLIPPED_EDGES_MAP;

   typedef std::map<polygon*,threevector> CLIPPED_FACES_MAP;

// Indep var for CLIPPED_FACES_MAP corresponds to clipped face polygon.
// Depend var for CLIPPED_FACES_MAP holds unclipped face, polyhedron
// and building IDs

   typedef std::map<fourvector,std::vector<int> > PLANE_POLYGONS_MAP;

   typedef std::map<int,bool> ANALYZED_OBSFRUSTA_IDS_MAP;

// Indep var = Clipped OBSFRUSTUM ID; depend var = dummy boolean

// Initialization, constructor and destructor functions:

   Clipping(OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr);
   virtual ~Clipping();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Clipping& C);

// Set & get methods:

   osg::Group* get_OSGgroup_ptr();
   const osg::Group* get_OSGgroup_ptr() const;

   void set_BuildingsGroup_ptr(BuildingsGroup* BG_ptr);
   void set_MoviesGroup_ptr(MoviesGroup* MG_ptr);
   void set_clipping_OBSFRUSTUM_ptr(int n);
   camera_frustum* get_clipping_camera_frustum_ptr();
   const camera_frustum* get_clipping_camera_frustum_ptr() const;

   void set_PolygonsGroup_ptr(osgGeometry::PolygonsGroup* P_ptr);   
   void set_Clipped_PolygonsGroup_ptr(osgGeometry::PolygonsGroup* P_ptr);
   void set_PolyhedraGroup_ptr(PolyhedraGroup* P_ptr);
   void set_PolyLinesGroup_ptr(PolyLinesGroup* P_ptr);
   void set_Clipped_PolyLinesGroup_ptr(PolyLinesGroup* P_ptr);
   void set_WindowManager_ptr(WindowManager* WM_ptr);
   camera* get_camera_ptr();
   const camera* get_camera_ptr() const;

   void set_max_frustum_to_rectangle_distance(double d);

// Clipping geometric object member functions:

   edge* clip_edge(edge& curr_edge,bool internal_edge_flag=false);
   std::vector<edge*> clip_face_edges(face* face_ptr);
   std::vector<edge*> clip_polyhedron_edges(polyhedron* polyhedron_ptr);
   polygon* clip_face(
      face* face_ptr,const threevector& FacePolyhedronBuildingIDs);
   edge* generate_clipped_edge(std::deque<fourvector>& clipped_posns);
   void clip_polyhedron(polyhedron* polyhedron_ptr,int Building_ID=-1);

   void clip_PolyLines();
   void destroy_all_PolyLines();

// Clipped polyhedron display member functions:
   void update_display();
   void project_Building_Polyhedra_into_imageplane(int clipping_OBSFRUSTUM_ID);
//   void project_Polyhedra_into_imageplane(int clipping_OBSFRUSTUM_ID);
   void draw_3D_clipped_face_polygons(
      const std::vector<unsigned int>& unoccluded_2D_clipped_face_polygon_IDs);
   void identify_texturable_rectangles(
      const std::vector<unsigned int>& unoccluded_2D_clipped_face_polygon_IDs);
   void generate_rectangle_face_decals();
   
//   void snap_clipped_face_polygons();

// Image plane projection member functions:

   void consolidate_clipped_polygon_planes();
   void project_clipped_polygons_into_imageplane();
   void classify_candidate_foliage_pixels();
   void convert_orig_RGBs_to_HSVs(RGBA_array& texture_RGBAs);
   void export_masks(RGBA_array& texture_RGBAs);
   std::vector<unsigned int> tally_unoccluded_clipped_polygons();

// Sky detection member functions:

   std::vector<threevector> generate_candidate_sky_colors(
      int bldg_value,int sky_value,
      const texture_rectangle* buildings_texture_rectangle_ptr,
      const texture_rectangle* buildings_mask_texture_rectangle_ptr);
   std::vector<std::pair<int,int> > find_candidate_sky_seed_pixels(
      int sky_value,unsigned int tallest_bldg_pv,
      const std::vector<threevector>& seed_sky_RGBs,
      texture_rectangle* buildings_texture_rectangle_ptr,
      const texture_rectangle* buildings_mask_texture_rectangle_ptr);

// Building coloring member functions:

   threevector get_Face_Polyhedron_Building_IDs(int pu,int pv);
   void backproject_Building_colors();
   void reset_Building_colors();
   void export_Building_colors();
   void import_Building_colors();
   void backproject_OBSFRUSTA_colorings(int nskip);

  protected:

  private:

   int prev_clipping_OBSFRUSTUM_ID;
   unsigned int curr_OBSFRUSTUM_index;
   double max_frustum_to_rectangle_distance;

   ColorMap color_map;
   CLIPPED_EDGES_MAP* clipped_edges_map_ptr;
   CLIPPED_FACES_MAP* clipped_faces_map_ptr;
   PLANE_POLYGONS_MAP* plane_polygons_map_ptr;
   ANALYZED_OBSFRUSTA_IDS_MAP* analyzed_OBSFRUSTA_IDs_map_ptr;

   BuildingsGroup* BuildingsGroup_ptr;
   MoviesGroup* MoviesGroup_ptr;
   OBSFRUSTUM* clipping_OBSFRUSTUM_ptr;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;   
   PixelBuffer* PixelBuffer_ptr;
   osgGeometry::PolygonsGroup *PolygonsGroup_ptr,*Clipped_PolygonsGroup_ptr;
   PolyhedraGroup* PolyhedraGroup_ptr;
   PolyLinesGroup *PolyLinesGroup_ptr,*Clipped_PolyLinesGroup_ptr;
   texture_rectangle* texture_rectangle_ptr;
   WindowManager* WindowManager_ptr;

   osg::ref_ptr<osg::Group> OSGgroup_refptr;

   std::vector<double> clipped_face_polygon_ranges;
   std::vector<polygon*> clipped_face_polygon_ptrs;
   twoDarray *sqrd_range_twoDarray_ptr,*ztwoDarray_ptr,*ptwoDarray_ptr;
   twoDarray *HtwoDarray_ptr,*StwoDarray_ptr,*VtwoDarray_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Clipping& C);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline osg::Group* Clipping::get_OSGgroup_ptr() 
{
   return OSGgroup_refptr.get();
}

inline const osg::Group* Clipping::get_OSGgroup_ptr() const
{
   return OSGgroup_refptr.get();
}

inline void Clipping::set_BuildingsGroup_ptr(BuildingsGroup* BG_ptr)
{
   BuildingsGroup_ptr=BG_ptr;
}

inline void Clipping::set_MoviesGroup_ptr(MoviesGroup* MG_ptr)
{
   MoviesGroup_ptr=MG_ptr;
}

inline void Clipping::set_clipping_OBSFRUSTUM_ptr(int n)
{
   clipping_OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->get_OBSFRUSTUM_ptr(n);
}

inline camera_frustum* Clipping::get_clipping_camera_frustum_ptr()
{
   if (clipping_OBSFRUSTUM_ptr==NULL) return NULL;
   camera* camera_ptr=clipping_OBSFRUSTUM_ptr->get_Movie_ptr()->
      get_camera_ptr();
   return camera_ptr->get_camera_frustum_ptr();
}

inline const camera_frustum* Clipping::get_clipping_camera_frustum_ptr() const
{
   if (clipping_OBSFRUSTUM_ptr==NULL) return NULL;
   camera* camera_ptr=clipping_OBSFRUSTUM_ptr->get_Movie_ptr()->
      get_camera_ptr();
   return camera_ptr->get_camera_frustum_ptr();
}

inline void Clipping::set_PolygonsGroup_ptr(osgGeometry::PolygonsGroup* P_ptr)
{
   PolygonsGroup_ptr=P_ptr;
}

inline void Clipping::set_Clipped_PolygonsGroup_ptr(
   osgGeometry::PolygonsGroup* P_ptr)
{
   Clipped_PolygonsGroup_ptr=P_ptr;
}

inline void Clipping::set_PolyhedraGroup_ptr(PolyhedraGroup* P_ptr)
{
   PolyhedraGroup_ptr=P_ptr;
}

inline void Clipping::set_PolyLinesGroup_ptr(PolyLinesGroup* P_ptr)
{
   PolyLinesGroup_ptr=P_ptr;
}

inline void Clipping::set_Clipped_PolyLinesGroup_ptr(PolyLinesGroup* P_ptr)
{
   Clipped_PolyLinesGroup_ptr=P_ptr;
}

inline void Clipping::set_WindowManager_ptr(WindowManager* WM_ptr)
{
   WindowManager_ptr=WM_ptr;
}

inline void Clipping::set_max_frustum_to_rectangle_distance(double d)
{
   max_frustum_to_rectangle_distance=d;
}

#endif // Clipping.h
