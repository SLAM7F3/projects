// ==========================================================================
// CLIPPING class member function definitions
// ==========================================================================
// Last modified on 1/23/14; 3/28/14; 4/5/14; 4/6/14; 6/7/14
// ==========================================================================

#include <iostream>
#include "osg/AbstractOSGCallback.h"
#include "models/Building.h"
#include "geometry/bounding_box.h"
#include "osg/osgClipping/Clipping.h"
#include "color/colorfuncs.h"
#include "geometry/edge.h"
#include "geometry/homography.h"
#include "image/imagefuncs.h"
#include "osg/osgSceneGraph/PixelBuffer.h"
#include "geometry/polyhedron.h"
#include "math/threematrix.h"
#include "coincidence_processing/VolumetricCoincidenceProcessor.h"

using std::cin;
using std::cout;
using std::deque;
using std::endl;
using std::map;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Clipping::allocate_member_objects()
{
   OSGgroup_refptr=new osg::Group(); 
   OSGgroup_refptr->setName("Clipping");
//    PixelBuffer_ptr=new PixelBuffer();

   clipped_edges_map_ptr=new CLIPPED_EDGES_MAP;
   clipped_faces_map_ptr=new CLIPPED_FACES_MAP;
//    plane_polygons_map_ptr=new PLANE_POLYGONS_MAP;
   analyzed_OBSFRUSTA_IDs_map_ptr=new ANALYZED_OBSFRUSTA_IDS_MAP;
}		       

void Clipping::initialize_member_objects()
{
   prev_clipping_OBSFRUSTUM_ID=-1;
   curr_OBSFRUSTUM_index=-1;
   BuildingsGroup_ptr=NULL;
   MoviesGroup_ptr=NULL;
   PolygonsGroup_ptr=NULL;
   PolyhedraGroup_ptr=NULL;
   PolyLinesGroup_ptr=NULL;
   Clipped_PolygonsGroup_ptr=NULL;
   Clipped_PolyLinesGroup_ptr=NULL;
   texture_rectangle_ptr=NULL;
   WindowManager_ptr=NULL;
   sqrd_range_twoDarray_ptr=NULL;
   ztwoDarray_ptr=NULL;
   ptwoDarray_ptr=NULL;
   HtwoDarray_ptr=NULL;
   StwoDarray_ptr=NULL;
   VtwoDarray_ptr=NULL;

   color_map.set_mapnumber(0);	// jet
//   color_map.set_mapnumber(4);	// large hue value sans white

   color_map.set_min_value(2,0);
   color_map.set_max_value(2,1);
   color_map.set_min_threshold(0);
   color_map.set_max_threshold(1);

//   max_frustum_to_rectangle_distance=50;	// meters
   max_frustum_to_rectangle_distance=100;	// meters

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<Clipping>(
         this, &Clipping::update_display));
}		       

Clipping::Clipping(OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   
   this->OBSFRUSTAGROUP_ptr=OBSFRUSTAGROUP_ptr;
}

Clipping::~Clipping()
{
   delete clipped_edges_map_ptr;
   delete clipped_faces_map_ptr;
//    delete plane_polygons_map_ptr;
   delete analyzed_OBSFRUSTA_IDs_map_ptr;

//    delete PixelBuffer_ptr;
   delete sqrd_range_twoDarray_ptr;
   delete ztwoDarray_ptr;
   delete ptwoDarray_ptr;

   delete HtwoDarray_ptr;
   delete StwoDarray_ptr;
   delete VtwoDarray_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Clipping& C)
{
   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

camera* Clipping::get_camera_ptr()
{
   if (clipping_OBSFRUSTUM_ptr==NULL)
   {
      cout << "Error in Clipping::get_camera()" << endl;
      cout << "clipping_OBSFRUSTUM_ptr=NULL" << endl;
      exit(-1);
   }
   return clipping_OBSFRUSTUM_ptr->get_Movie_ptr()->get_camera_ptr();
}

const camera* Clipping::get_camera_ptr() const
{
   if (clipping_OBSFRUSTUM_ptr==NULL)
   {
      cout << "Error in Clipping::get_camera()" << endl;
      cout << "clipping_OBSFRUSTUM_ptr=NULL" << endl;
      exit(-1);
   }
   return clipping_OBSFRUSTUM_ptr->get_Movie_ptr()->get_camera_ptr();
}

// ==========================================================================
// Clipping geometric object member functions
// ==========================================================================

// Member function clip_edge()

edge* Clipping::clip_edge(edge& curr_edge,bool internal_edge_flag)
{
//   cout << "inside Clipping::clip_edge()" << endl;

   if (clipping_OBSFRUSTUM_ptr==NULL)
   {
      cout << "Error in Clipping::clip_edge()" << endl;
      cout << "clipping_OBSFRUSTUM_ptr=NULL" << endl;
      return NULL;
   }

   linesegment l(curr_edge.get_V1().get_posn(),curr_edge.get_V2().get_posn());

   linesegment* clipped_segment_ptr=
      get_clipping_camera_frustum_ptr()->SegmentFrustumIntersection(l);
   if (clipped_segment_ptr==NULL) return NULL;
//   cout << "*clipped_segment_ptr = " << *clipped_segment_ptr << endl;

   vertex V1prime(clipped_segment_ptr->get_v1(),curr_edge.get_V1().get_ID());
   vertex V2prime(clipped_segment_ptr->get_v2(),curr_edge.get_V2().get_ID());
   edge* clipped_edge_ptr=new edge(V1prime,V2prime);
//   cout << "*clipped_edge_ptr = " << *clipped_edge_ptr << endl;

   if (!internal_edge_flag)
   {

// On 3/2/12, Ross Anderson reminded us again that we should never
// attempt to render lines whose vertices have large numerical
// values (e.g. UTM coordinates).  Instead, we should always specify a
// single large offset for the line. After making this change, clipped
// edge flickering problems go away...

      threevector V1=clipped_segment_ptr->get_v1();
      threevector V2=clipped_segment_ptr->get_v2();
      threevector segment_COM=0.5*(V1+V2);

      vector<threevector> vertices;
      vertices.push_back(V1);
      vertices.push_back(V2);

      osg::Vec4 clipped_edge_color=colorfunc::get_OSG_color(colorfunc::yellow);
//       PolyLine* clipped_PolyLine_ptr=
         Clipped_PolyLinesGroup_ptr->generate_new_PolyLine(
            segment_COM,vertices,clipped_edge_color);
   }

   return clipped_edge_ptr;
}

// ---------------------------------------------------------------------
// Member function clip_face_edges() 

vector<edge*> Clipping::clip_face_edges(face* face_ptr)
{
//   cout << "inside Clipping::clip_face_edges()" << endl;

   vector<edge*> clipped_edge_ptrs;
   if (clipping_OBSFRUSTUM_ptr==NULL)
   {
      cout << "Error in Clipping::clip_face_edges()" << endl;
      cout << "clipping_OBSFRUSTUM_ptr=NULL" << endl;
      return clipped_edge_ptrs;
   }

   deque<fourvector> clipped_posns;

   bool prev_edge_inside_frustum_flag=false;
   threevector entering_clipped_vertex_posn,exiting_clipped_vertex_posn;
   for (unsigned int e=0; e<face_ptr->get_n_edges(); e++)
   {
      edge* clipped_edge_ptr=clip_edge(face_ptr->get_edge(e));

      if (clipped_edge_ptr != NULL)
      {
         threevector candidate_posn=clipped_edge_ptr->get_V2().get_posn();
         if (get_clipping_camera_frustum_ptr()->PointOnFrustum(candidate_posn))
         {
            exiting_clipped_vertex_posn=candidate_posn;
            clipped_posns.push_back(fourvector(exiting_clipped_vertex_posn,e));
            if (clipped_posns.size() >= 2)
            {
               edge* generated_edge_ptr=generate_clipped_edge(clipped_posns);
               if (generated_edge_ptr != NULL) clipped_edge_ptrs.push_back(
                  generated_edge_ptr);
            }
         }
         prev_edge_inside_frustum_flag=true;
      }

      if (clipped_edge_ptr != NULL && !prev_edge_inside_frustum_flag)
      {
         threevector candidate_posn=clipped_edge_ptr->get_V1().get_posn();
         if (get_clipping_camera_frustum_ptr()->PointOnFrustum(candidate_posn))
         {
            entering_clipped_vertex_posn=candidate_posn;
            clipped_posns.push_back(
               fourvector(entering_clipped_vertex_posn,e));
            if (clipped_posns.size() >= 2)
            {
               edge* generated_edge_ptr=generate_clipped_edge(clipped_posns);
               if (generated_edge_ptr != NULL) clipped_edge_ptrs.push_back(
                  generated_edge_ptr);
            }
         }
      }
      
      if (clipped_edge_ptr != NULL)
      {
         clipped_edge_ptrs.push_back(clipped_edge_ptr);
      }
   } // loop over index e labeling face edges

   return clipped_edge_ptrs;
}

// ---------------------------------------------------------------------
// Member function clip_polyhedron_edges() 

vector<edge*> Clipping::clip_polyhedron_edges(polyhedron* polyhedron_ptr)
{
//   cout << "inside Clipping::clip_polyhedron_edges()" << endl;

   vector<edge*> clipped_edge_ptrs;
   if (clipping_OBSFRUSTUM_ptr==NULL)
   {
      cout << "Error in Clipping::clip_polyhedron_edges()" << endl;
      cout << "clipping_OBSFRUSTUM_ptr=NULL" << endl;
      return clipped_edge_ptrs;
   }

   deque<fourvector> clipped_posns;

   bool prev_edge_inside_frustum_flag=false;
   threevector entering_clipped_vertex_posn,exiting_clipped_vertex_posn;

   for (unsigned int e=0; e<polyhedron_ptr->get_n_edges(); e++)
   {
      edge* edge_ptr=polyhedron_ptr->get_edge_ptr(e);
      if (polyhedron_ptr->get_internal_edge_flag(edge_ptr->get_ID()))
         continue;

      edge* clipped_edge_ptr=clip_edge(*edge_ptr);

      if (clipped_edge_ptr != NULL)
      {
         threevector candidate_posn=clipped_edge_ptr->get_V2().get_posn();
         if (get_clipping_camera_frustum_ptr()->PointOnFrustum(candidate_posn))
         {
            exiting_clipped_vertex_posn=candidate_posn;
            clipped_posns.push_back(fourvector(exiting_clipped_vertex_posn,e));
            if (clipped_posns.size() >= 2)
            {
               edge* generated_edge_ptr=generate_clipped_edge(clipped_posns);
               if (generated_edge_ptr != NULL) clipped_edge_ptrs.push_back(
                  generated_edge_ptr);
            }
         }
         prev_edge_inside_frustum_flag=true;
      }

      if (clipped_edge_ptr != NULL && !prev_edge_inside_frustum_flag)
      {
         threevector candidate_posn=clipped_edge_ptr->get_V1().get_posn();
         if (get_clipping_camera_frustum_ptr()->PointOnFrustum(candidate_posn))
         {
            entering_clipped_vertex_posn=candidate_posn;
            clipped_posns.push_back(
               fourvector(entering_clipped_vertex_posn,e));
            if (clipped_posns.size() >= 2)
            {
               edge* generated_edge_ptr=generate_clipped_edge(clipped_posns);
               if (generated_edge_ptr != NULL) clipped_edge_ptrs.push_back(
                  generated_edge_ptr);
            }
         }
      }
      
      if (clipped_edge_ptr != NULL)
      {
         clipped_edge_ptrs.push_back(clipped_edge_ptr);
      }
   } // loop over index e labeling face edges

   return clipped_edge_ptrs;
}

// ---------------------------------------------------------------------
// Member function generate_clipped_edge()

edge* Clipping::generate_clipped_edge(deque<fourvector>& clipped_posns)
{
   if (clipped_posns[0].get(3)==clipped_posns[1].get(3)) return NULL;

   vertex Vstart(threevector(clipped_posns[0]),clipped_posns[0].get(3));
   vertex Vstop(threevector(clipped_posns[1]),clipped_posns[1].get(3));
   clipped_posns.pop_front();
   clipped_posns.pop_front();
   
   edge* generated_edge_ptr=new edge(Vstart,Vstop);
   return generated_edge_ptr;
}

// ---------------------------------------------------------------------
// Member function clip_face() takes in 3D face *face_ptr.  It returns
// a pointer to a dynamically instantiated polygon which represents the
// part of the face that lies within the camera's view frustum.  It
// also associates the clipped 3D polygon with its progenitor face and
// polyhedron IDs within STL map member *clipped_faces_map_ptr.

polygon* Clipping::clip_face(
   face* face_ptr,const threevector& FacePolyhedronBuildingIDs)
{
//   cout << "inside Clipping::clip_face()" << endl;
   
   polygon* clipped_face_poly_ptr=get_clipping_camera_frustum_ptr()->
      FaceFrustumIntersection(*face_ptr);

   if (clipped_face_poly_ptr != NULL)
   {
//      cout << "clipped_face_poly = " << *clipped_face_poly_ptr << endl;
      (*clipped_faces_map_ptr)[clipped_face_poly_ptr]=
         FacePolyhedronBuildingIDs;
   }
//   cout << "clipped_faces_map_ptr->size() = "
//        << clipped_faces_map_ptr->size() << endl;
   return clipped_face_poly_ptr;
}

// ---------------------------------------------------------------------
// Member function clip_polyhedron() takes in polyhedron
// *polyhedron_ptr.  It loops over all of the input polyhedron's faces
// and ignores any which are oriented too far away from the clipping
// OBSFRUSTUM's camera.  Clipped polyhedron face polygons which point
// towards the camera are added to member STL vector
// clipped_face_polygon_ptrs.

void Clipping::clip_polyhedron(polyhedron* polyhedron_ptr,int Building_ID)
{
//   cout << "inside Clipping::clip_polyhedron()" << endl;

   threevector pointing_dir=get_camera_ptr()->get_pointing_dir();

   const double SMALL_NEG=cos((90+20)*PI/180.0);
//   cout << "SMALL_NEG = " << SMALL_NEG << endl;

   for (unsigned int f=0; f<polyhedron_ptr->get_n_faces(); f++)
   {
      face* face_ptr=polyhedron_ptr->get_face_ptr(f);
      double curr_dotproduct=pointing_dir.dot(-face_ptr->get_normal());
//      cout << "f = " << f
//           << " dotproduct = " << curr_dotproduct << endl;

// Ignore any face whose normal direction vector points far away from
// *camera_ptr:

      if (curr_dotproduct < SMALL_NEG) continue;

      threevector FacePolyhedronBuildingIDs(
         face_ptr->get_ID(),polyhedron_ptr->get_ID(),Building_ID);

      polygon* clipped_face_poly_ptr=clip_face(
         face_ptr,FacePolyhedronBuildingIDs);
      if (clipped_face_poly_ptr != NULL)
      {
         unsigned int curr_clipped_face_polygon_ID=
            clipped_face_polygon_ptrs.size();
         clipped_face_poly_ptr->set_ID(curr_clipped_face_polygon_ID);
         clipped_face_polygon_ptrs.push_back(clipped_face_poly_ptr);

         threevector clipped_polygon_COM=clipped_face_poly_ptr->compute_COM();
         double clipped_polygon_range=(clipped_polygon_COM-get_camera_ptr()->
         get_world_posn()).magnitude();
         clipped_face_polygon_ranges.push_back(clipped_polygon_range);
      } // clipped_face_poly_ptr != NULL conditional
   } // loop over index f labeling polyhedron faces

// NOTE ADDED ON SUN MAR 4, 2012:
// WE NEED A WAY TO ASSOCIATE ONE OR MORE FACES WITH A SPECIFIED
// POLYHEDRON EDGE.  CLIPPED EDGES ASSOCIATED WITH FACES WHICH ALL
// LOOK AWAY FROM THE CAMERA SHOULD NOT BE DISPLAYED IN THE 3D MAP...

/*
   vector<edge*> curr_clipped_edge_ptrs=clip_polyhedron_edges(polyhedron_ptr);

   for (unsigned int e=0; e<curr_clipped_edge_ptrs.size(); e++)
   {

// Perform brute force search over all edges stored within
// *clipped_edges_map_ptr.  If any existing edge is close to candidate
// edge, don't incorporate candidate into STL map:

      bool edge_already_exists_flag=false;
      for (CLIPPED_EDGES_MAP::iterator iter=clipped_edges_map_ptr->begin();
           iter != clipped_edges_map_ptr->end(); iter++)
      {
         edge* existing_edge_ptr=iter->first;
         if (existing_edge_ptr->nearly_equal_posn(
            *curr_clipped_edge_ptrs[e]))
         {
            edge_already_exists_flag=true;
            break;
         }
      }
      if (!edge_already_exists_flag)
      {
         (*clipped_edges_map_ptr)[curr_clipped_edge_ptrs[e]]=
            FacePolyhedronIDs;
      }
   } // loop over index e labeling clipped edges
*/
}

// ---------------------------------------------------------------------
// Member function clip_PolyLines() loops over all Polylines within
// *PolyLinesGroup_ptr.  For each PolyLine, it clips each line segment
// joining neighboring vertices against the currently selected
// OBSFRUSTUM.

void Clipping::clip_PolyLines()
{
   cout << "inside Clipping::clip_PolyLines()" << endl;

   if (clipping_OBSFRUSTUM_ptr==NULL)
   {
      cout << "Error in Clipping::clip_PolyLines()" << endl;
      cout << "clipping_OBSFRUSTUM_ptr=NULL" << endl;
      return;
   }

   if (PolyLinesGroup_ptr==NULL)
   {
      cout << "Error in Clipping::clip_PolyLines()!" << endl;
      cout << "PolyLinesGroup_ptr=NULL" << endl;
      return;
   }
   
   osg::Vec4 clipped_line_color=colorfunc::get_OSG_color(colorfunc::purple);
   Clipped_PolyLinesGroup_ptr->destroy_all_PolyLines();

   for (unsigned int n=0; n<PolyLinesGroup_ptr->get_n_Graphicals(); n++)
   {
      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(n);
      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
      
      for (unsigned int v=0; v<polyline_ptr->get_n_vertices()-1; v++)
      {
         threevector V1=polyline_ptr->get_vertex(v);
         threevector V2=polyline_ptr->get_vertex(v+1);
         linesegment l(V1,V2);

         linesegment* clipped_segment_ptr=
            get_clipping_camera_frustum_ptr()->SegmentFrustumIntersection(l);
         if (clipped_segment_ptr==NULL) continue;

         threevector V1prime=clipped_segment_ptr->get_v1();
         threevector V2prime=clipped_segment_ptr->get_v2();

         vector<threevector> V;
         V.push_back(V1prime);
         V.push_back(V2prime);

//          PolyLine* clipped_PolyLine_ptr=
            Clipped_PolyLinesGroup_ptr->generate_new_PolyLine(
               V,clipped_line_color);   

      } // loop over index v labeling polyline verticees
   } // loop over index n labeling PolyLines

}

// ---------------------------------------------------------------------
void Clipping::destroy_all_PolyLines()
{
//   cout << "inside Clipping::destroy_all_PolyLines()" << endl;
   if (PolyLinesGroup_ptr != NULL)
      PolyLinesGroup_ptr->destroy_all_PolyLines();
   if (Clipped_PolyLinesGroup_ptr != NULL)
      Clipped_PolyLinesGroup_ptr->destroy_all_PolyLines();
}

// ==========================================================================
// Clipped polyhedron display member functions
// ==========================================================================

// Member function update_display()

void Clipping::update_display()
{   
//   cout << "inside Clipping::update_display()" << endl;

   bool analyze_all_OBSFRUSTA_flag=false;
//   bool analyze_all_OBSFRUSTA_flag=true;
   if (analyze_all_OBSFRUSTA_flag)
   {
      curr_OBSFRUSTUM_index++;
      cout << "Current OBSFRUSTUM index = " << curr_OBSFRUSTUM_index << endl;

      unsigned int n_OBSFRUSTA=OBSFRUSTAGROUP_ptr->get_n_Graphicals();
      if (curr_OBSFRUSTUM_index==n_OBSFRUSTA)
      {
         cout << "Finished processing all OBSFRUSTA" << endl;
         exit(-1);
      }
      clipping_OBSFRUSTUM_ptr=static_cast<OBSFRUSTUM*>(
         OBSFRUSTAGROUP_ptr->get_Graphical_ptr(curr_OBSFRUSTUM_index));

      OBSFRUSTAGROUP_ptr->load_thumbnail_photo();

      int selected_OBSFRUSTUM_ID=clipping_OBSFRUSTUM_ptr->get_ID();
      OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(selected_OBSFRUSTUM_ID);
      OBSFRUSTAGROUP_ptr->load_hires_photo();
   }
   else
   {
      clipping_OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
         get_selected_OBSFRUSTUM_ptr();
   }

   if (clipping_OBSFRUSTUM_ptr==NULL) return;
   
   int clipping_OBSFRUSTUM_ID=clipping_OBSFRUSTUM_ptr->get_ID();
   if (clipping_OBSFRUSTUM_ID==prev_clipping_OBSFRUSTUM_ID) return;
   prev_clipping_OBSFRUSTUM_ID=clipping_OBSFRUSTUM_ID;

   ANALYZED_OBSFRUSTA_IDS_MAP::iterator iter=analyzed_OBSFRUSTA_IDs_map_ptr->
      find(clipping_OBSFRUSTUM_ID);
   if (iter==analyzed_OBSFRUSTA_IDs_map_ptr->end()) 
   {
      (*analyzed_OBSFRUSTA_IDs_map_ptr)[clipping_OBSFRUSTUM_ID]=true;
   }

   project_Building_Polyhedra_into_imageplane(clipping_OBSFRUSTUM_ID);

   vector<unsigned int> unoccluded_2D_clipped_face_polygon_IDs=
      tally_unoccluded_clipped_polygons();
   cout << "unoccluded_2D_clipped_face_polygon_IDs.size() = "
        << unoccluded_2D_clipped_face_polygon_IDs.size() << endl;

   draw_3D_clipped_face_polygons(unoccluded_2D_clipped_face_polygon_IDs);

//   identify_texturable_rectangles(unoccluded_2D_clipped_face_polygon_IDs);
//   generate_rectangle_face_decals();

//   backproject_Building_colors();
//   reset_Building_colors();
//   export_Building_colors();

   classify_candidate_foliage_pixels();
}

// ---------------------------------------------------------------------
// Member function project_Building_Polyhedra_into_imageplane() loops
// over all polyhedra within *Building_polyhedron_map_ptr.  Each
// polyhedron face is tested to determine if it faces towards the
// current OBSFRUSTUM.  If so, a clipped version of the face is added
// to STL vector member clipped_face_polygon_ptrs.  All clipped
// polygons lying in the same 3D plane are forced to have
// identical pi fourvectors.  Member twoDarrays
// *sqrd_range_twoDarray_ptr, *ptwoDarray_ptr and *ztwoDarray_ptr are
// instantiated and filled with minimal ranges to planar intersection
// points, polygon IDs and polygon altitudes.

void Clipping::project_Building_Polyhedra_into_imageplane(
   int clipping_OBSFRUSTUM_ID)
{
   cout << "inside Clipping::project_Building_Polyhedra_into_imageplane()" 
        << endl;
   cout << "clipping_OBSFRUSTUM_ID = " << clipping_OBSFRUSTUM_ID
        << endl;

   if (BuildingsGroup_ptr==NULL) return;

// First purge contents of *clipped_edges_map_ptr, 
// *clipped_faces_map_ptr, *Clipped_PolygonsGroup_ptr and
// *Clipped_PolyLinesGroup_ptr: 
   
   clipped_edges_map_ptr->clear();
   clipped_faces_map_ptr->clear();
   clipped_face_polygon_ranges.clear();
   for (unsigned int p=0; p<clipped_face_polygon_ptrs.size(); p++)
   {
      delete clipped_face_polygon_ptrs[p];
   }
   clipped_face_polygon_ptrs.clear();

   Clipped_PolygonsGroup_ptr->destroy_all_Polygons();
   Clipped_PolyLinesGroup_ptr->destroy_all_PolyLines();

   BuildingsGroup::BUILDING_POLYHEDRON_MAP* Building_polyhedron_map_ptr=
      BuildingsGroup_ptr->get_Building_polyhedron_map_ptr();
   for (BuildingsGroup::BUILDING_POLYHEDRON_MAP::iterator iter=
           Building_polyhedron_map_ptr->begin(); iter != 
           Building_polyhedron_map_ptr->end(); iter++)
   {
      twovector building_polyhedron_ids(iter->first);
      int Building_ID=building_polyhedron_ids.get(0);
//      int polyhedron_ID=building_polyhedron_ids.get(1);
      polyhedron* polyhedron_ptr=iter->second;
      clip_polyhedron(polyhedron_ptr,Building_ID);
   } // loop over iter

   consolidate_clipped_polygon_planes();

// On 4/16/12, we discovered that the following Quicksort line was
// responsible for seriously fouling up drawing of 3D clipped face
// polygons.  But we don't understand why...

// Sort clipped face polygons by their ranges to the camera:

//   templatefunc::Quicksort(
//      clipped_face_polygon_ranges,clipped_face_polygon_ptrs);

   project_clipped_polygons_into_imageplane();
   
   cout << "At end of Clipping::project_Building_Polyhedra_into_imageplane()"
        << endl;
}

/*
// ---------------------------------------------------------------------
// Member function project_Polyhedra_into_imageplane()

void Clipping::project_Polyhedra_into_imageplane(int clipping_OBSFRUSTUM_ID)
{
   cout << "inside Clipping::project_Polyhedra_into_imageplane()" << endl;
//   cout << "clipping_OBSFRUSTUM_ID = " << clipping_OBSFRUSTUM_ID
//        << endl;

// First purge contents of *clipped_edges_map_ptr, 
// *clipped_faces_map_ptr, *Clipped_PolygonsGroup_ptr and
// *Clipped_PolyLinesGroup_ptr: 
   
   clipped_edges_map_ptr->clear();
   clipped_faces_map_ptr->clear();
   clipped_face_polygon_ranges.clear();
   for (unsigned int p=0; p<clipped_face_polygon_ptrs.size(); p++)
   {
      delete clipped_face_polygon_ptrs[p];
   }
   clipped_face_polygon_ptrs.clear();

   Clipped_PolygonsGroup_ptr->destroy_all_Polygons();
   Clipped_PolyLinesGroup_ptr->destroy_all_PolyLines();

   for (unsigned int p=0; p<PolyhedraGroup_ptr->get_n_Graphicals(); p++)
   {
//      cout << "Clipping polyhedron p = " << p << endl;
      Polyhedron* Polyhedron_ptr=PolyhedraGroup_ptr->get_Polyhedron_ptr(p);
      polyhedron* polyhedron_ptr=Polyhedron_ptr->get_polyhedron_ptr();
      clip_polyhedron(polyhedron_ptr);
   } // loop over index p labeling Polyhedra

// Sort clipped face polygons by their ranges to the camera:

   templatefunc::Quicksort(
      clipped_face_polygon_ranges,clipped_face_polygon_ptrs);

   consolidate_clipped_polygon_planes();
   vector<int> unoccluded_clipped_face_polygon_IDs=
      project_clipped_polygons_into_imageplane();
   draw_clipped_face_polygons(unoccluded_clipped_face_polygon_IDs);
}
*/

// ---------------------------------------------------------------------
// Member function draw_3D_clipped_face_polygons() loops over all
// polygons within member STL vector clipped_face_polygon_ptrs.  It
// ignores any polygon whose ID does not appear within input STL
// vector unoccluded_clipped_face_polygon_IDs.
// Unoccluded 3D polygons are added to *Clipped_PolygonsGroup_ptr and
// colored so that they match their 2D imageplane projections.

void Clipping::draw_3D_clipped_face_polygons(
   const vector<unsigned int>& unoccluded_clipped_face_polygon_IDs)
{
   cout << "inside Clipping::draw_3D_clipped_face_polygons()" << endl;

   unsigned int n_polygons=clipped_face_polygon_ptrs.size();
   for (unsigned int p=0; p<n_polygons; p++)
   {

// Perform brute force search for index p within input STL vector.  If
// not found, skip current polygon:

      bool unoccluded_polygon_flag=false;
      for (unsigned int q=0; q<unoccluded_clipped_face_polygon_IDs.size(); q++)
      {
         if (unoccluded_clipped_face_polygon_IDs[q]==p)
         {
            unoccluded_polygon_flag=true;
            break;
         }
      }
      if (!unoccluded_polygon_flag) continue;

      polygon* polygon_ptr=clipped_face_polygon_ptrs[p];

// On 3/2/12, Ross Anderson reminded us again that we should never
// attempt to render Polygons whose vertices have large numerical
// values (e.g. UTM coordinates).  Instead, we should always specify a
// single large offset and relative vertex coordinates for the polygon.  
// After making this change, clipped face polygon flickering problems
// go away...


      threevector polygon_COM(polygon_ptr->compute_COM());
      vector<threevector> relative_vertices;
      for (unsigned int v=0; v<polygon_ptr->get_nvertices(); v++)
      {
         relative_vertices.push_back(polygon_ptr->get_vertex(v)-polygon_COM);
         if (relative_vertices.back().nearly_equal(Zero_vector))
         {
            cout << "*polygon_ptr = " << *polygon_ptr << endl;
            outputfunc::enter_continue_char();
         }
      } // loop over index v labeling clipped face polygon vertices
      polygon relative_poly(relative_vertices);

      osgGeometry::Polygon* clipped_Polygon_ptr=
         Clipped_PolygonsGroup_ptr->generate_new_Polygon(
            polygon_COM,relative_poly);

      double ID_frac=double(p)/double(n_polygons);
      colorfunc::RGBA curr_RGBA=color_map.retrieve_curr_RGBA(ID_frac);

      clipped_Polygon_ptr->set_permanent_color(
         osg::Vec4(curr_RGBA.first,curr_RGBA.second,curr_RGBA.third,1));
      clipped_Polygon_ptr->set_curr_color(
         osg::Vec4(curr_RGBA.first,curr_RGBA.second,curr_RGBA.third,1));

   } // loop over index p labeling clipped face polygons
}

// ---------------------------------------------------------------------
// Member function identify_texturable_rectangles() loops over all
// clipped face triangles.  For each triangle, the rectangle side-face
// progenitor is found.  The rectangle's ID is associated with
// counterpart building and polyhedron IDs.  The dotproduct between
// the rectangle side-face and the current camera's pointing direction
// is also calculated and saved in STL map
// *BuildingsGroup::ClippedPolygon_building_polyhedron_rectangle_map_ptr.
// Unoccluded rectangle side faces are used to generate Polygons
// within *Clipped_PolygonsGroup_ptr. 

void Clipping::identify_texturable_rectangles(
   const vector<unsigned int>& unoccluded_clipped_face_polygon_IDs)
{
   cout << "inside Clipping::identify_texturable_rectangles()" << endl;
   cout << "unoccluded_clipped_face_polygon_IDs.size() = "
        << unoccluded_clipped_face_polygon_IDs.size() << endl;

   BuildingsGroup::CLIPPEDPOLYGON_BUILDING_POLYHEDRON_RECTANGLE_MAP* 
      ClippedPolygon_building_polyhedron_rectangle_map_ptr=
      BuildingsGroup_ptr->
      get_ClippedPolygon_building_polyhedron_rectangle_map_ptr();

   vector<polygon*> rectangle_ptrs;

   unsigned int n_polygons=clipped_face_polygon_ptrs.size();
   cout << "n_polygons=clipped_face_polygon_ptrs.size() = "
        << n_polygons << endl;
   
   for (unsigned int p=0; p<n_polygons; p++)
   {

// Perform brute force search for index p within input STL vector.  If
// not found, skip current polygon:

      bool unoccluded_polygon_flag=false;
      for (unsigned int q=0; q<unoccluded_clipped_face_polygon_IDs.size(); q++)
      {
         if (unoccluded_clipped_face_polygon_IDs[q]==p)
         {
            unoccluded_polygon_flag=true;
            break;
         }
      }
      if (!unoccluded_polygon_flag) continue;

      polygon* clipped_face_poly_ptr=clipped_face_polygon_ptrs[p];
//      cout << "p = " << p << endl;

// Retrieve rectangle side face progenitor for current clipped face
// polygon:

      fourvector bldg_polyhedron_rectangle_IDs_weight(-1,-1,-1,0);
      polygon* rectangle_ptr=NULL;
      CLIPPED_FACES_MAP::iterator cf_iter=clipped_faces_map_ptr->find(
         clipped_face_poly_ptr);
      if (cf_iter != clipped_faces_map_ptr->end()) 
      {
         threevector FacePolyhedronBuildingIDs=cf_iter->second;
         int face_ID=FacePolyhedronBuildingIDs.get(0);
         int polyhedron_ID=FacePolyhedronBuildingIDs.get(1);
         int building_ID=FacePolyhedronBuildingIDs.get(2);

         Building* Building_ptr=BuildingsGroup_ptr->get_Building_ptr(
            building_ID);
         vector<polyhedron*> polyhedra_ptrs=Building_ptr->get_polyhedra_ptrs();
         polyhedron* polyhedron_ptr=NULL;
         for (unsigned int p=0; p<polyhedra_ptrs.size(); p++)
         {
            if (polyhedra_ptrs[p]->get_ID()==polyhedron_ID)
            {
               polyhedron_ptr=polyhedra_ptrs[p];
               break;
            }
         } // loop over index p
         polyhedron::TRIANGLEFACE_RECTANGLE_MAP* 
            triangleface_rectangle_map_ptr=polyhedron_ptr->
            get_triangleface_rectangle_map_ptr();
         polyhedron::TRIANGLEFACE_RECTANGLE_MAP::iterator tr_iter=
            triangleface_rectangle_map_ptr->find(face_ID);

         if (tr_iter != triangleface_rectangle_map_ptr->end()) 
         {
            int rectangle_ID=tr_iter->second;
//            cout << "building_ID = " << building_ID
//                 << " polyhedron_ID = " << polyhedron_ID
//                 << " face_ID = " << face_ID << endl;
//            cout << "   rectangle_ID = " << rectangle_ID << endl;
            bldg_polyhedron_rectangle_IDs_weight=fourvector(
               building_ID,polyhedron_ID,rectangle_ID,0);

            twovector rectangle_polyhedron_IDs(
               rectangle_ID,polyhedron_ID);
            Building::RECTANGLE_SIDEFACES_MAP* rectangle_sidefaces_map_ptr=
               Building_ptr->get_rectangle_sidefaces_map_ptr();

            Building::RECTANGLE_SIDEFACES_MAP::iterator rs_iter=
               rectangle_sidefaces_map_ptr->find(rectangle_polyhedron_IDs);
            if (rs_iter != rectangle_sidefaces_map_ptr->end())
            {
               rectangle_ptr=rs_iter->second;
               rectangle_ptr->set_ID(rectangle_ID);
            }
         } // tr_iter != triangleface_rectangle_map_ptr->end() conditional
      } // cf_iter != clipped_faces_map_ptr->end() conditional
//       cout << "rectangle_ptr = " << rectangle_ptr << endl;
      
      if (rectangle_ptr==NULL) continue;

// Ignore any rectangle side face which is completely occluded by
// other faces within the current building:

      if (rectangle_ptr->get_occluded_flag()) continue;

// Ignore any rectangle side face which is not basically oriented
// "face-on" to current the OBSFRUSTUM:

      threevector rectangle_normal(rectangle_ptr->get_normal());
      double dotproduct=-rectangle_normal.dot(
         get_camera_ptr()->get_pointing_dir());
//      cout << "dotproduct = " << dotproduct << endl;
      bldg_polyhedron_rectangle_IDs_weight.put(3,dotproduct);
      if (dotproduct < 0.3) continue;

// Ignore any rectangle side face which lies far away from camera
// position:

      double curr_distance=rectangle_ptr->point_dist_to_polygon(
         get_camera_ptr()->get_world_posn());
//      cout << "curr_distance = " << curr_distance << endl;
      if (curr_distance > max_frustum_to_rectangle_distance) continue;

// Ignore any rectangle which has previously been processed:

      bool continue_flag=false;
      for (unsigned int r=0; r<rectangle_ptrs.size(); r++)
      {
         if (rectangle_ptr==rectangle_ptrs[r]) 
         {
            continue_flag=true;
            break;
         }
      }
      if (continue_flag)
      {
         continue;
      }
      else
      {
         rectangle_ptrs.push_back(rectangle_ptr);
      }

// On 3/2/12, Ross Anderson reminded us again that we should never
// attempt to render Polygons whose vertices have large numerical
// values (e.g. UTM coordinates).  Instead, we should always specify a
// single large offset and relative vertex coordinates for the polygon.  
// After making this change, clipped face polygon flickering problems
// go away...

      threevector rectangle_COM(rectangle_ptr->compute_COM());
      vector<threevector> relative_vertices;
      for (unsigned int v=0; v<rectangle_ptr->get_nvertices(); v++)
      {
         relative_vertices.push_back(
            rectangle_ptr->get_vertex(v)-rectangle_COM);
         if (relative_vertices.back().nearly_equal(Zero_vector))
         {
            cout << "*rectangle_ptr = " << *rectangle_ptr << endl;
            outputfunc::enter_continue_char();
         }
      } // loop over index v labeling clipped face polygon vertices
      polygon relative_rectangle(relative_vertices);

      osgGeometry::Polygon* clipped_Polygon_ptr=
         Clipped_PolygonsGroup_ptr->generate_new_Polygon(
            rectangle_COM,relative_rectangle);

      double ID_frac=double(p)/double(n_polygons);
      colorfunc::RGBA curr_RGBA=color_map.retrieve_curr_RGBA(ID_frac);

      clipped_Polygon_ptr->set_permanent_color(
         osg::Vec4(curr_RGBA.first,curr_RGBA.second,curr_RGBA.third,1));
      clipped_Polygon_ptr->set_curr_color(
         osg::Vec4(curr_RGBA.first,curr_RGBA.second,curr_RGBA.third,1));

      cout << "bldg_polyhedron_rectangle_IDs_weight = "
           << bldg_polyhedron_rectangle_IDs_weight << endl;

      (*ClippedPolygon_building_polyhedron_rectangle_map_ptr)[
         clipped_Polygon_ptr->get_ID()]=bldg_polyhedron_rectangle_IDs_weight;

   } // loop over index p labeling clipped face polygons
}

// ---------------------------------------------------------------------
// Member function generate_rectangle_face_decals() loops over all
// rectangles within *Clipped_PolygonsGroup_ptr generated within
// identify_textureable_rectangles().  For each rectangle, this method
// first converts its 3D world-space XYZ corners to planar ABN
// coordinates.  It also projects the XYZ corners into 2D image plane
// UV coordinates.  After renormalizing A and B so that 0 <= B <= 1, 
// generate_rectangle_face_decals() computes the homography H which
// maps UV -> AB.  It then maps colored pixels within the UV image
// plane onto an AB "decal" via H.  The decal image is exported with
// building, polyhedron and rectangle IDs and normal dotproduct weight
// information encoded into the output filename.

void Clipping::generate_rectangle_face_decals()
{
   cout << "inside Clipping::generate_rectangle_face_decals()" << endl;

   BuildingsGroup::CLIPPEDPOLYGON_BUILDING_POLYHEDRON_RECTANGLE_MAP* 
      ClippedPolygon_building_polyhedron_rectangle_map_ptr=
      BuildingsGroup_ptr->
      get_ClippedPolygon_building_polyhedron_rectangle_map_ptr();

   for (unsigned int r=0; r<Clipped_PolygonsGroup_ptr->get_n_Graphicals(); r++)
   {
      osgGeometry::Polygon* Rectangle_ptr=
         Clipped_PolygonsGroup_ptr->get_Polygon_ptr(r);
      polygon* relative_rectangle_ptr=Rectangle_ptr->get_relative_poly_ptr();
      threevector rectangle_COM=Rectangle_ptr->get_reference_origin();

// Convert 3D rectangle corners from world XYZ to planar abn
// coordinates:

      vector<threevector> relative_vertices,absolute_vertices;
      vector<twovector> UV;
      for (unsigned int v=0; v<relative_rectangle_ptr->get_nvertices(); v++)
      {
         relative_vertices.push_back(relative_rectangle_ptr->get_vertex(v));

         threevector absolute_vertex=rectangle_COM+relative_vertices.back();
         absolute_vertices.push_back(absolute_vertex);
         threevector projected_vertex;
         get_camera_ptr()->project_XYZ_to_UV_coordinates(
            absolute_vertex,projected_vertex);
//         cout << "v = " << v 
//              << " rel vertex = " << relative_vertices.back() 
//              << "projected_vertex = " << projected_vertex
//              << endl;
         UV.push_back(twovector(projected_vertex));
      } // loop over index v labeling relative rectangle vertices

// Construct plane using relative rectangle's known normal direction
// vector and its COM:

      threevector n_hat=(relative_vertices[1]-relative_vertices[0]).cross(
         relative_vertices[2]-relative_vertices[1]);
      threevector relative_COM=0.25*(
         relative_vertices[0]+relative_vertices[1]+
         relative_vertices[2]+relative_vertices[3]);
      plane curr_plane(n_hat,relative_COM);
//      cout << "curr_plane = " << curr_plane << endl;

      vector<threevector>* planar_vertices_ptr=curr_plane.coords_wrt_plane(
         relative_vertices);
      vector<twovector> AB;
      for (unsigned int c=0; c<planar_vertices_ptr->size(); c++)
      {
         int c_eff=modulo(c+2,4);

// Renormalize planar coordinates to a,b coords where 0 <= b <= 1 and
// 0 <= a <= AB_aspect_ratio:

         twovector curr_AB=planar_vertices_ptr->at(c_eff)-
            planar_vertices_ptr->at(2);
         curr_AB /= (2*planar_vertices_ptr->at(0).get(1));
         AB.push_back(curr_AB);
//         cout << "r = " << r 
//              << " c = " << c 
//              << " U = " << UV[c].get(0)
//              << " V = " << UV[c].get(1)
//              << " rel X = " << relative_vertices[c].get(0)
//              << " rel Y = " << relative_vertices[c].get(1)
//              << " rel Z = " << relative_vertices[c].get(2)
//              << " A = " << AB.back().get(0)
//              << " B = " << AB.back().get(1) 
//              << endl;
      }	// loop over index c labeling planar vertices
      delete planar_vertices_ptr;

      homography H;
      H.parse_homography_inputs(UV,AB);
      H.compute_homography_matrix();
      H.compute_homography_inverse();

//      double RMS_residual=H.check_homography_matrix(UV,AB);
//      cout << "RMS_residual = " << RMS_residual << endl;
//      cout << "H = " << H << endl;

      string UV_image_filename=clipping_OBSFRUSTUM_ptr->get_Movie_ptr()->
         get_video_filename();
      texture_rectangle* UV_texture_rectangle_ptr=
         clipping_OBSFRUSTUM_ptr->get_Movie_ptr()->
         get_texture_rectangle_ptr();
      unsigned int UV_width,UV_height;
      imagefunc::get_image_width_height(
         UV_image_filename,UV_width,UV_height);
      double umax=double(UV_width)/double(UV_height);

// Instantiate new "decal" image in AB planar coordinates:

      double AB_aspect_ratio=AB[2].get(0)/AB[2].get(1);
//      cout << "AB_aspect_ratio = " << AB_aspect_ratio << endl;
      unsigned int AB_height=600;
      unsigned int AB_width=AB_aspect_ratio*AB_height;
//      cout << "width = " << width << " height = " << height << endl;
      texture_rectangle* AB_texture_rectangle_ptr=new texture_rectangle(
         AB_width,AB_height,1,3,NULL);
//       double amax=double(AB_width)/double(AB_height);

      string blank_filename="blank.jpg";
      AB_texture_rectangle_ptr->generate_blank_image_file(
         AB_width,AB_height,blank_filename,0);
      AB_texture_rectangle_ptr->import_photo_from_file(blank_filename);

// Map colored pixels in UV image plane onto AB decal via homography H:

      double u,v;
      for (unsigned int pa=0; pa<AB_width; pa++)
      {
         outputfunc::update_progress_fraction(pa,100,AB_width);
         double a_frac=double(pa)/(AB_width-1);
         double a=AB_aspect_ratio*a_frac;
         for (unsigned int pb=0; pb<AB_height; pb++)
         {
            double b_frac=1-double(pb)/(AB_height-1);
            double b=b_frac;
            H.project_image_plane_to_world_plane(a,b,u,v);

            if (u < 0 || u > umax || v < 0 || v > 1) continue;

            int R,G,B;
            UV_texture_rectangle_ptr->get_RGB_values(u,v,R,G,B);
//         cout << "R = " << R << " G = " << G << " B = " << B << endl;
            AB_texture_rectangle_ptr->set_pixel_RGB_values(pa,pb,R,G,B);
         } // loop over pb index
      } // loop over pa index
      cout << endl;

      string rectified_views_subdir="./rectified_views/";
      filefunc::dircreate(rectified_views_subdir);

// Export rectified "decal" images with building, polyhedron
// and rectangle ID labels.  Also include integer weight which is
// function of rectangle normal dotproduct with current camera viewing
// direction within output decal filename:

      BuildingsGroup::CLIPPEDPOLYGON_BUILDING_POLYHEDRON_RECTANGLE_MAP::
         iterator iter=
         ClippedPolygon_building_polyhedron_rectangle_map_ptr->find(
            Rectangle_ptr->get_ID());

      fourvector bldg_polyhedron_rectangle_IDs_weight(-1,-1,-1,0);
      if (iter != ClippedPolygon_building_polyhedron_rectangle_map_ptr->end()) 
      {
         bldg_polyhedron_rectangle_IDs_weight=iter->second;
      }
      int building_ID=bldg_polyhedron_rectangle_IDs_weight.get(0);
      int polyhedron_ID=bldg_polyhedron_rectangle_IDs_weight.get(1);
      int rectangle_ID=bldg_polyhedron_rectangle_IDs_weight.get(2);
      int weight=basic_math::round(
         100*bldg_polyhedron_rectangle_IDs_weight.get(3));

      string building_polyhedron_subdir=rectified_views_subdir+
         "bldg_"+stringfunc::number_to_string(building_ID)+
         "_polyhedron_"+stringfunc::number_to_string(polyhedron_ID)+"/";
      filefunc::dircreate(building_polyhedron_subdir);

      string image_filename=clipping_OBSFRUSTUM_ptr->get_Movie_ptr()->
         get_video_filename(); 
//      cout << "image_filename = " << image_filename << endl;
      string basename=filefunc::getbasename(image_filename);
      string prefix=stringfunc::prefix(basename);
      string ID_labels=
         stringfunc::number_to_string(building_ID)+"_"+
         stringfunc::number_to_string(polyhedron_ID)+"_"+
         stringfunc::number_to_string(rectangle_ID)+"_";
//      string rectified_image_filename=rectified_views_subdir+
      string rectified_image_filename=building_polyhedron_subdir+
         ID_labels+stringfunc::number_to_string(weight)+"_"
         +prefix+".jpg";
      cout << "Rectified image filename = " << rectified_image_filename 
           << endl;

      AB_texture_rectangle_ptr->write_curr_frame(rectified_image_filename);
      delete AB_texture_rectangle_ptr;

      string output_filename=building_polyhedron_subdir+ID_labels+prefix+
         "geom.metadata";
      ofstream outstream;
      outstream.precision(15);
      filefunc::openfile(output_filename,outstream);
      
      outstream << "# UV coordinates (dimensionless) for facet corners in original image plane:" << endl << endl;
      
      outstream << UV[1].get(0) << " " << UV[1].get(1) 
                << " # lower right corner" << endl;
      outstream << UV[0].get(0) << " " << UV[0].get(1) 
                << " # lower left corner" << endl;
      outstream << UV[3].get(0) << " " << UV[3].get(1)
                << " # upper left corner" << endl;
      outstream << UV[2].get(0) << " " << UV[2].get(1) 
                << " # upper right corner" << endl << endl;

      outstream << "# UV coordinates (dimensionless) for facet corners in rectified image plane:" << endl << endl;
      outstream << AB[0].get(0) << " " << AB[0].get(1) 
                << " # lower left corner" << endl;
      outstream << AB[1].get(0) << " " << AB[1].get(1) 
                << " # lower right corner" << endl;
      outstream << AB[2].get(0) << " " << AB[2].get(1) 
                << " # upper right corner" << endl;
      outstream << AB[3].get(0) << " " << AB[3].get(1) 
                << " # upper left corner" << endl;
      outstream << endl;

      outstream << "# Easting, Northing, Altitude UTM coordinates (meters) for center of rectified image" << endl << endl;
      outstream << rectangle_COM.get(0) << " "
                << rectangle_COM.get(1) << " "
                << rectangle_COM.get(2) << endl;
      outstream << endl;

      outstream << "# UTM offset coordinates (meters) for corners of rectified image:" << endl << endl;
      
      outstream << relative_vertices[1].get(0) << " "
                << relative_vertices[1].get(1) << " "
                << relative_vertices[1].get(2) 
                << " # lower right corner" << endl;
      outstream << relative_vertices[0].get(0) << " "
                << relative_vertices[0].get(1) << " "
                << relative_vertices[0].get(2) 
                << " # lower left corner" << endl;
      outstream << relative_vertices[3].get(0) << " "
                << relative_vertices[3].get(1) << " "
                << relative_vertices[3].get(2) 
                << " # upper left corner" << endl;
      outstream << relative_vertices[2].get(0) << " "
                << relative_vertices[2].get(1) << " "
                << relative_vertices[2].get(2) 
                << " # upper right corner" << endl << endl;

      outstream << "# Absolute UTM coordinates (meters) for corners of rectified image:" << endl << endl;
      
      outstream << absolute_vertices[1].get(0) << " "
                << absolute_vertices[1].get(1) << " "
                << absolute_vertices[1].get(2) 
                << " # lower right corner" << endl;
      outstream << absolute_vertices[0].get(0) << " "
                << absolute_vertices[0].get(1) << " "
                << absolute_vertices[0].get(2) 
                << " # lower left corner" << endl;
      outstream << absolute_vertices[3].get(0) << " "
                << absolute_vertices[3].get(1) << " "
                << absolute_vertices[3].get(2) 
                << " # upper left corner" << endl;
      outstream << absolute_vertices[2].get(0) << " "
                << absolute_vertices[2].get(1) << " "
                << absolute_vertices[2].get(2) 
                << " # upper right corner" << endl;

      filefunc::closefile(output_filename,outstream);

   } // loop over index r labeling texturable rectangles

// Do not show both Clipped Polygons and Movie textures!

   Clipped_PolygonsGroup_ptr->set_OSGgroup_nodemask(0);
}

/*   
// ---------------------------------------------------------------------
// Member function snap_clipped_face_polygons()

void Clipping::snap_clipped_face_polygons()
{
   cout << "inside Clipping::snap_clipped_face_polygons()" << endl;

   if (WindowManager_ptr==NULL)
   {
      cout << "Error in Clipping::snap_clipped_face_polygons()" << endl;
      cout << "WindowManager_ptr=NULL" << endl;
   }
   for (unsigned int i=0; i<100; i++)
   {
      WindowManager_ptr->process();
   }

   if (clipping_OBSFRUSTUM_ptr==NULL)
   {
      cout << "Error in Clipping::snap_clipped_face_polygons()" << endl;
      cout << "clipping_OBSFRUSTUM_ptr=NULL" << endl;
      return;
   }

//   PixelBuffer_ptr->set_root_ptr(
//      Clipped_PolygonsGroup_ptr->get_OSGgroup_ptr());
   PixelBuffer_ptr->set_root_ptr(
      Clipped_PolyLinesGroup_ptr->get_OSGgroup_ptr());

   Movie* Movie_ptr=clipping_OBSFRUSTUM_ptr->get_Movie_ptr();
   string photo_filename=Movie_ptr->get_video_filename();

//   cout << "photo_filename = " << photo_filename << endl;
//   cout << "width = " << Movie_ptr->getWidth() << endl;
//   cout << "height = " << Movie_ptr->getHeight() << endl;
   PixelBuffer_ptr->set_WindowRectangle(
      Movie_ptr->getWidth(),Movie_ptr->getHeight());

   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   PixelBuffer_ptr->set_camera_posn_and_pointing(
      camera_ptr->get_world_posn(),camera_ptr->get_Uhat(),
      camera_ptr->get_Vhat());
   PixelBuffer_ptr->set_camera_FOVs(
      camera_ptr->get_FOV_u()*180/PI,camera_ptr->get_FOV_v()*180/PI);

   string output_filename="mask.png";
   PixelBuffer_ptr->TakeSnapshot(output_filename);
}
*/

// ==========================================================================
// Image plane projection member functions
// ==========================================================================

// Member function consolidate_clipped_polygon_planes() loops over all
// polygons within STL vector member clipped_face_polygon_ptrs.  It
// forces all polygons which lie within the same plane to have the
// same fourvector pi representing that plane.

void Clipping::consolidate_clipped_polygon_planes()
{
   cout << "inside Clipping::consolidate_clipped_polygon_planes()" 
        << endl;

   for (unsigned int p=0; p<clipped_face_polygon_ptrs.size(); p++)
   {
//      cout << "p = " << p << endl;
      polygon* curr_polygon_ptr=clipped_face_polygon_ptrs[p];
      plane* curr_plane_ptr=curr_polygon_ptr->recompute_plane();

      for (unsigned int q=p+1; q<clipped_face_polygon_ptrs.size(); q++)
      {
//         cout << q << " " << flush;
         polygon* next_polygon_ptr=clipped_face_polygon_ptrs[q];

         if (next_polygon_ptr->lies_on_plane(*curr_plane_ptr))
         {
            next_polygon_ptr->set_plane(curr_plane_ptr->get_pi());
         }
      } // loop over index q labeling next polygon
//      cout << endl;
   } // loop over index p labeling curr polygon
}

// ---------------------------------------------------------------------
// Member function project_clipped_polygons_into_imageplane()
// instantiates TwoDarrays *sqrd_range_twoDarray_ptr, *ztwoDarray_ptr
// and *ptwoDarray_ptr whose pixel dimensions match those of the
// image corresponding to *clipping_OBSFRUSTUM_ptr.  Looping over all
// clipped 3D polygons, it finds 2D UV bounding boxes for their image
// plane projections.  Iterating over pu,pv pixels within each
// bounding box, this method traces rays back to 3D polygon planes.
// It stores minimal ranges to planar intersection points
// within member *sqrd_range_twoDarray_ptr, 3D progenitor polygon ID
// within member *ptwoDarray_ptr and 3D polygon altitudes within
// member *ztwoDarray_ptr.  

void Clipping::project_clipped_polygons_into_imageplane()
{
   cout << "inside Clipping::project_clipped_polygons_into_imageplane()" 
        << endl;

   Movie* Movie_ptr=clipping_OBSFRUSTUM_ptr->get_Movie_ptr();
   string photo_filename=Movie_ptr->get_video_filename();
   cout << "photo_filename = " << photo_filename << endl;

   delete texture_rectangle_ptr;
   texture_rectangle_ptr=new texture_rectangle(photo_filename,NULL);

   unsigned int mdim=Movie_ptr->getWidth();
   unsigned int ndim=Movie_ptr->getHeight();
//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

   delete sqrd_range_twoDarray_ptr;
   sqrd_range_twoDarray_ptr=new twoDarray(mdim,ndim);
   sqrd_range_twoDarray_ptr->initialize_values(-1);

   delete ztwoDarray_ptr;
   ztwoDarray_ptr=new twoDarray(mdim,ndim);
   ztwoDarray_ptr->initialize_values(-1);

   delete ptwoDarray_ptr;
   ptwoDarray_ptr=new twoDarray(mdim,ndim);
   ptwoDarray_ptr->initialize_values(-1);

   RGBA_array texture_RGBAs=texture_rectangle_ptr->get_RGBA_twoDarrays();
   convert_orig_RGBs_to_HSVs(texture_RGBAs);

   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   threevector camera_posn=camera_ptr->get_world_posn();

   cout << "Calculating clipped face polygon squared ranges" << endl;
   cout << "clipped_face_polygon_ptrs.size() = " 
        << clipped_face_polygon_ptrs.size() << endl;

   const double TINY=1E-3;
   threevector projected_vertex,prev_projected_vertex;
   vector<threevector> projected_vertices;
   bounding_box UV_bbox;
   for (unsigned int p=0; p<clipped_face_polygon_ptrs.size(); p++)
   {
      cout << p << " " << flush;
      polygon* polygon_ptr=clipped_face_polygon_ptrs[p];

      projected_vertices.clear();
      for (unsigned int v=0; v<polygon_ptr->get_nvertices(); v++)
      {
         camera_ptr->project_XYZ_to_UV_coordinates(
            polygon_ptr->get_vertex(v),projected_vertex);

         if (v==0)
         {
            projected_vertices.push_back(twovector(projected_vertex));
         }

// Ignore any projected vertex which lies too close to the zeroth
// and/or previously projected vertex:

         else if (v > 0 && 
         !projected_vertex.nearly_equal(prev_projected_vertex,TINY) &&
         !projected_vertex.nearly_equal(projected_vertices[0],TINY) )
         {
            projected_vertices.push_back(twovector(projected_vertex));
         }
         prev_projected_vertex=projected_vertices.back();
      } // loop over index v labeling polygon vertices

      if (projected_vertices.size() <= 2) continue;

      polygon projected_polygon(projected_vertices);
//      cout << "projected_polygon = " << projected_polygon << endl;

      UV_bbox.recompute_bounding_box(projected_vertices);
//      cout << "UV_bbox = " << UV_bbox << endl;

      unsigned int pu_min,pv_min,pu_max,pv_max;
      texture_rectangle_ptr->get_pixel_coords(
         UV_bbox.get_xmin(),UV_bbox.get_ymin(),pu_min,pv_max);
      texture_rectangle_ptr->get_pixel_coords(
         UV_bbox.get_xmax(),UV_bbox.get_ymax(),pu_max,pv_min);

      pu_min=basic_math::max(Unsigned_Zero,pu_min);
      pv_min=basic_math::max(Unsigned_Zero,pv_min);
      pu_max=basic_math::min(mdim-1,pu_max);
      pv_max=basic_math::min(ndim-1,pv_max);
//      cout << "pu_min = " << pu_min << " pu_max = " << pu_max << endl;
//      cout << "pv_min = " << pv_min << " pv_max = " << pv_max << endl;

// Loop over bounding box pixels.  Identify those which lie inside
// projected polygon:

      double u,v;
//       int n_points_inside=0;
//       int n_total_points=0;
      int Clipped_Polygon_ID=polygon_ptr->get_ID();
      threevector intersection_pt;
      plane* plane_ptr=polygon_ptr->get_plane_ptr();
      for (unsigned int pu=pu_min; pu <= pu_max; pu++)
      {
         for (unsigned int pv=pv_min; pv <= pv_max; pv++)
         {
//             n_total_points++;
            texture_rectangle_ptr->get_uv_coords(pu,pv,u,v);
            if (!projected_polygon.point_inside_polygon(twovector(u,v))) 
               continue;

            if (!plane_ptr->ray_intersection(
               camera_posn,camera_ptr->pixel_ray_direction(u,v),
               intersection_pt))
            {
               cout << "Ray does not intersect polygon's plane!" << endl;
               outputfunc::enter_continue_char();
               continue;
            }

            double curr_sqrd_range=(intersection_pt-camera_posn).
               sqrd_magnitude();
            double prev_sqrd_range=sqrd_range_twoDarray_ptr->get(pu,pv);
            if (prev_sqrd_range < 0 || curr_sqrd_range < prev_sqrd_range)
            {
               sqrd_range_twoDarray_ptr->put(pu,pv,curr_sqrd_range);
               ztwoDarray_ptr->put(pu,pv,intersection_pt.get(2));
               ptwoDarray_ptr->put(pu,pv,Clipped_Polygon_ID);
            }

//             n_points_inside++;
         } // loop over pv index
      } // loop over pu index

//      double frac_bbox=double(n_points_inside)/double(n_total_points);
//      cout << "frac bbox = " << frac_bbox << endl;

   } // loop over index p labeling Clipped Polygons
   cout << endl;

   export_masks(texture_RGBAs);

// On 8/29/12, we realized that since texture_rectangle_ptr is deleted
// near the start of this member function, we do NOT need to delete
// its RGBA twoDarrays at the end of this method...

}

// ---------------------------------------------------------------------
// Member function classify_candidate_foliage_pixels() marks all green
// and black colored pixels within *texture_rectangle_ptr.

void Clipping::classify_candidate_foliage_pixels()
{
   cout << "inside Clipping::classify_candidate_foliage_pixels()" 
        << endl;

   vector<int> masked_color_bin_numbers;
   masked_color_bin_numbers.push_back(3);	// green
   masked_color_bin_numbers.push_back(6);	// black
   string filtered_image_filename="color_filtered_image.jpg";
   videofunc::filter_color_image(
      HtwoDarray_ptr,StwoDarray_ptr,VtwoDarray_ptr,
      masked_color_bin_numbers,texture_rectangle_ptr,filtered_image_filename);
}

// ---------------------------------------------------------------------
// Member function convert_orig_RGBs_to_HSVs() resets and fills member
// twoDarrays *HtwoDarray_ptr, *StwoDarray_ptr and *VtwoDarray_ptr
// with hue, saturation and value data extracted from an input image's
// original RGB values.

void Clipping::convert_orig_RGBs_to_HSVs(RGBA_array& texture_RGBAs)
{
   cout << "inside Clipping::convert_orig_RGBs_to_HSVs()" << endl;

   unsigned int mdim=sqrd_range_twoDarray_ptr->get_mdim();
   unsigned int ndim=sqrd_range_twoDarray_ptr->get_ndim();
   
   delete HtwoDarray_ptr;
   delete StwoDarray_ptr;
   delete VtwoDarray_ptr;
   HtwoDarray_ptr=new twoDarray(mdim,ndim);
   StwoDarray_ptr=new twoDarray(mdim,ndim);
   VtwoDarray_ptr=new twoDarray(mdim,ndim);

   twoDarray* RtwoDarray_ptr=texture_RGBAs.first;
   twoDarray* GtwoDarray_ptr=texture_RGBAs.second;
   twoDarray* BtwoDarray_ptr=texture_RGBAs.third;

   for (unsigned int pu=0; pu<mdim; pu++)
   {
      for (unsigned int pv=0; pv<ndim; pv++)
      {
         double r_orig=RtwoDarray_ptr->get(pu,pv);
         double g_orig=GtwoDarray_ptr->get(pu,pv);
         double b_orig=BtwoDarray_ptr->get(pu,pv);
         
         double h_orig,s_orig,v_orig;
         colorfunc::RGB_to_hsv(
            r_orig,g_orig,b_orig,h_orig,s_orig,v_orig);
         HtwoDarray_ptr->put(pu,pv,h_orig);
         StwoDarray_ptr->put(pu,pv,s_orig);
         VtwoDarray_ptr->put(pu,pv,v_orig);
      } // loop over pv
   } // loop over pu
}

// ---------------------------------------------------------------------
// Member function export_masks() writes visible clipped polygon
// range information to an output JPEG file.  It exports another
// JPEG file which colors projected imageplane polygons according to
// their progenitor world-space polygon IDs.  Finally, this method
// writes out a mask which maps image plane pixels to Building IDs.
// It doesn't alter non-building pixels' colors.

void Clipping::export_masks(RGBA_array& texture_RGBAs)
{
   cout << "inside Clipping::export_masks()" << endl;

// Export image plane mask:

   int n_channels=3;

   texture_rectangle_ptr->initialize_twoDarray_image(
      sqrd_range_twoDarray_ptr,n_channels);

   unsigned int mdim=sqrd_range_twoDarray_ptr->get_mdim();
   unsigned int ndim=sqrd_range_twoDarray_ptr->get_ndim();
   double min_sqrd_range=POSITIVEINFINITY;
   double max_sqrd_range=0;
   double min_z=POSITIVEINFINITY;
   double max_z=NEGATIVEINFINITY;
   for (unsigned int pu=0; pu<mdim; pu++)
   {
      for (unsigned int pv=0; pv<ndim; pv++)
      {
         double curr_sqrd_range=sqrd_range_twoDarray_ptr->get(pu,pv);
         if (curr_sqrd_range < 0) continue;
         min_sqrd_range=basic_math::min(min_sqrd_range,curr_sqrd_range);
         max_sqrd_range=basic_math::max(max_sqrd_range,curr_sqrd_range);
         double z=ztwoDarray_ptr->get(pu,pv);
         min_z=basic_math::min(min_z,z);
         max_z=basic_math::max(max_z,z);
      } // loop over pv index
   } // loop over pu index

// Generate and export range mask:

   double min_range=sqrt(min_sqrd_range);
   double max_range=sqrt(max_sqrd_range);
   for (unsigned int pu=0; pu<mdim; pu++)
   {
      for (unsigned int pv=0; pv<ndim; pv++)
      {
         colorfunc::RGBA curr_RGBA(0,0,0,0);

         double curr_sqrd_range=sqrd_range_twoDarray_ptr->get(pu,pv);
         if (curr_sqrd_range > 0)
         {
            double curr_range=sqrt(curr_sqrd_range);
            double range_frac=(curr_range-min_range)/(max_range-min_range);
            curr_RGBA=color_map.retrieve_curr_RGBA(range_frac);
         }
         int R=255*curr_RGBA.first;
         int G=255*curr_RGBA.second;
         int B=255*curr_RGBA.third;
         texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
      } // loop over pv index
   } // loop over pu index

   string mask_filename="range_mask.jpg";
   texture_rectangle_ptr->write_curr_frame(mask_filename);

   string banner="Wrote unoccluded clipped polygon ranges to "+
      mask_filename;
   outputfunc::write_big_banner(banner);

// Generate and export height mask:

   for (unsigned int pu=0; pu<mdim; pu++)
   {
      for (unsigned int pv=0; pv<ndim; pv++)
      {
         colorfunc::RGBA curr_RGBA(0,0,0,0);

         double z=ztwoDarray_ptr->get(pu,pv);
         if (z > 0)
         {
            double z_frac=(z-min_z)/(max_z-min_z);
            curr_RGBA=color_map.retrieve_curr_RGBA(z_frac);
         }
         int R=255*curr_RGBA.first;
         int G=255*curr_RGBA.second;
         int B=255*curr_RGBA.third;
         texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
      } // loop over pv index
   } // loop over pu index

   mask_filename="z_mask.jpg";
   texture_rectangle_ptr->write_curr_frame(mask_filename);

   banner="Wrote unoccluded clipped polygon heights to "+mask_filename;
   outputfunc::write_big_banner(banner);

// Generate and export clipped polygon ID mask:

   unsigned int n_polygons=clipped_face_polygon_ptrs.size();
   for (unsigned int pu=0; pu<mdim; pu++)
   {
      for (unsigned int pv=0; pv<ndim; pv++)
      {
         colorfunc::RGBA curr_RGBA(0,0,0,0);

         int curr_Polygon_ID=ptwoDarray_ptr->get(pu,pv);
         if (curr_Polygon_ID >= 0)
         {
            double ID_frac=double(curr_Polygon_ID)/double(n_polygons);
            curr_RGBA=color_map.retrieve_curr_RGBA(ID_frac);
         }
         int R=255*curr_RGBA.first;
         int G=255*curr_RGBA.second;
         int B=255*curr_RGBA.third;
//            cout << "R = " << R << " G = " << G << " B = " << B << endl;
         texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);

      } // loop over pv index
   } // loop over pu index

   mask_filename="clipped_poly_ID.jpg";
   texture_rectangle_ptr->write_curr_frame(mask_filename);

   banner="Wrote unoccluded clipped polygon IDs to "+mask_filename;
   outputfunc::write_big_banner(banner);

// Generate and export building ID mask:

   Movie* Movie_ptr=clipping_OBSFRUSTUM_ptr->get_Movie_ptr();
   string photo_filename=Movie_ptr->get_video_filename();
//   cout << "photo_filename = " << photo_filename << endl;
   texture_rectangle* buildings_texture_rectangle_ptr=
      new texture_rectangle(photo_filename,NULL);

   texture_rectangle* buildings_mask_texture_rectangle_ptr=
      new texture_rectangle(photo_filename,NULL);
   buildings_mask_texture_rectangle_ptr->clear_all_RGB_values();
   
   bool segment_sky_pixels_flag=false;
//   bool segment_sky_pixels_flag=true;

   const int bldg_value=128;
   const int sky_value=255;
   int R,G,B;
   unsigned int tallest_bldg_pv=ndim;

   typedef map<int,int> BLDG_ID_COLOR_MAP;
   BLDG_ID_COLOR_MAP bldg_id_color_map;
   BLDG_ID_COLOR_MAP::iterator iter;
   
   int curr_color_index,color_index=0;
   for (unsigned int pu=0; pu<mdim; pu++)
   {
      for (unsigned int pv=0; pv<ndim; pv++)
      {
         colorfunc::RGBA curr_RGBA(0,0,0,0);

         threevector Face_Polyhedron_Building_IDs=
            get_Face_Polyhedron_Building_IDs(pu,pv);

         int Building_ID=Face_Polyhedron_Building_IDs.get(2);
         if (Building_ID >= 0)
         {
            iter=bldg_id_color_map.find(Building_ID);
            if (iter==bldg_id_color_map.end())
            {
               bldg_id_color_map[Building_ID]=color_index;
               curr_color_index=color_index;
               color_index++;
            }
            else
            {
               curr_color_index=iter->second;
            }

            colorfunc::Color bldg_color=colorfunc::get_color(curr_color_index);
//            if (bldg_color==colorfunc::purple) bldg_color=colorfunc::cream;
            if (bldg_color==colorfunc::pink) bldg_color=colorfunc::red;
            if (bldg_color==colorfunc::gold) bldg_color=colorfunc::yellow;

// Force ground-plane polyhedron to be tinted pink:

            if (Building_ID==29)
            {
               bldg_color=colorfunc::pink;
            }

            colorfunc::RGB bldg_RGB=colorfunc::get_RGB_values(bldg_color);
//            curr_RGBA=color_map.retrieve_curr_RGBA(ID_frac);
            
            double h,s,v;
            colorfunc::RGB_to_hsv(
               bldg_RGB.first,bldg_RGB.second,bldg_RGB.third,h,s,v);
//            colorfunc::RGB_to_hsv(
//               curr_RGBA.first,curr_RGBA.second,curr_RGBA.third,h,s,v);
            
//            double s_orig=StwoDarray_ptr->get(pu,pv);
            double v_orig=VtwoDarray_ptr->get(pu,pv);
         
            double r_new,g_new,b_new;
            colorfunc::hsv_to_RGB(h,s,v_orig,r_new,g_new,b_new);

            R=255*r_new;
            G=255*g_new;
            B=255*b_new;
//            cout << "R = " << R << " G = " << G << " B = " << B << endl;
            buildings_texture_rectangle_ptr->set_pixel_RGB_values(
               pu,pv,R,G,B);
            buildings_mask_texture_rectangle_ptr->set_pixel_RGB_values(
               pu,pv,bldg_value,bldg_value,bldg_value);
         }
//          cout << "R = " << R << " G = " << G << " B = " << B << endl;
      } // loop over pv index

      if (!segment_sky_pixels_flag) continue;

// Scan current pixel column for any pixels classified as building:

      bool column_contains_bldg_pixels_flag=false;
      for (unsigned int pv=0; pv<ndim; pv++)
      {
         buildings_mask_texture_rectangle_ptr->get_pixel_RGB_values(
            pu,pv,R,G,B);
         if (R==bldg_value && G==bldg_value && B==bldg_value)
         {
            column_contains_bldg_pixels_flag=true;
            tallest_bldg_pv=basic_math::min(tallest_bldg_pv,pv);
            break;
         }
      } // loop over pv index

// Reclassify all pixels located above building rooftop within current
// image plane as sky:

      if (column_contains_bldg_pixels_flag)
      {
         for (unsigned int pv=0; pv<ndim; pv++)
         {
            buildings_mask_texture_rectangle_ptr->get_pixel_RGB_values(
               pu,pv,R,G,B);
            if (R==bldg_value && G==bldg_value && B==bldg_value)
            {
               break;
            }
            buildings_mask_texture_rectangle_ptr->set_pixel_RGB_values(
               pu,pv,sky_value,sky_value,sky_value);
         } // loop over pv index
      }
   } // loop over pu index

   mask_filename="Building_IDs.jpg";
   buildings_texture_rectangle_ptr->write_curr_frame(mask_filename);

   banner="Wrote Building IDs to "+mask_filename;
   outputfunc::write_big_banner(banner);

   mask_filename="Building_mask.jpg";
   buildings_mask_texture_rectangle_ptr->write_curr_frame(mask_filename);

   if (segment_sky_pixels_flag)
   {
      banner="Wrote sky & building masks to "+mask_filename;
      outputfunc::write_big_banner(banner);

      vector<threevector> seed_sky_RGBs=generate_candidate_sky_colors(
         bldg_value,sky_value,buildings_texture_rectangle_ptr,
         buildings_mask_texture_rectangle_ptr);

      vector<pair<int,int> > sky_seed_pixels=find_candidate_sky_seed_pixels(
         sky_value,tallest_bldg_pv,
         seed_sky_RGBs,buildings_texture_rectangle_ptr,
         buildings_mask_texture_rectangle_ptr);

      mask_filename="sky_seeds.jpg";
      buildings_texture_rectangle_ptr->write_curr_frame(mask_filename);

      banner="Wrote sky seeds to "+mask_filename;
      outputfunc::write_big_banner(banner);

      buildings_texture_rectangle_ptr->reset_texture_content(photo_filename);
   
// "Ooze" sky classification from seed pixels to neighbors whose RGB
// values locally and globally close in value:

      int sky_R=255;
      int sky_G=0;
      int sky_B=255;

      double local_threshold=25;
//   cout << "Enter local_threshold:" << endl;
//   cin >> local_threshold;

      double global_threshold=70;
//   cout << "Enter global_threshold:" << endl;
//   cin >> global_threshold;

      vector<pair<int,int> > filled_pixels;
      vector<threevector> encountered_RGBs;

      cout << "Flooding sky pixels:" << endl;
      for (unsigned int p=0; p<sky_seed_pixels.size(); p++)
      {
         int pu=sky_seed_pixels[p].first;
         int pv=sky_seed_pixels[p].second;
         buildings_texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
      
         buildings_texture_rectangle_ptr->floodFill(
            pu,pv,sky_R,sky_G,sky_B,R,G,B,
            local_threshold,global_threshold,
            filled_pixels,encountered_RGBs);
      }

// To minimize floodFill errors, reset set any pixel within
// *buildings_texture_rectangle_ptr to its original RGB colors if the
// corresponding entry in *buildings_mask_texture_rectangle_ptr equals
// bldg_value:

      texture_rectangle* orig_photo_texture_rectangle_ptr=
         new texture_rectangle(photo_filename,NULL);
      string basename=filefunc::getbasename(photo_filename);
      orig_photo_texture_rectangle_ptr->write_curr_frame(basename);

      for (unsigned int pu=0; pu<mdim; pu++)
      {
         for (unsigned int pv=0; pv<ndim; pv++)
         {
            buildings_mask_texture_rectangle_ptr->get_pixel_RGB_values(
               pu,pv,R,G,B);
            if (R==bldg_value && G==bldg_value && B==bldg_value)
            {
               orig_photo_texture_rectangle_ptr->get_pixel_RGB_values(
                  pu,pv,R,G,B);
               buildings_texture_rectangle_ptr->set_pixel_RGB_values(
                  pu,pv,R,G,B);
            }
         
         } // loop over pv index
      } // loop over pu index

      mask_filename="sky_pixels_"+basename;
      buildings_texture_rectangle_ptr->write_curr_frame(mask_filename);

      banner="Wrote flooded sky pixels to "+mask_filename;
      outputfunc::write_big_banner(banner);
   } // segment_sky_pixels_flag conditional
   
   delete buildings_texture_rectangle_ptr;
   delete buildings_mask_texture_rectangle_ptr;
}

// ---------------------------------------------------------------------
// Member function tally_unoccluded_clipped_polygons() loops over
// *ptwoDarray_ptr and extracts clipped polygon IDs.  It returns a
// distinct set of ordered polygon IDs within an output STL vector.

vector<unsigned int> Clipping::tally_unoccluded_clipped_polygons()
{
   cout << "inside Clipping::tally_unoccluded_clipped_polygons()" << endl;

   typedef std::map<unsigned int,int> UNOCCLUDED_POLYGON_IDS_MAP;
   UNOCCLUDED_POLYGON_IDS_MAP unoccluded_polygon_ids_map;

   for (unsigned int pu=0; pu<ptwoDarray_ptr->get_mdim(); pu++)
   {
      for (unsigned int pv=0; pv<ptwoDarray_ptr->get_ndim(); pv++)
      {
         unsigned int curr_polygon_ID=ptwoDarray_ptr->get(pu,pv);
         if (curr_polygon_ID < 0) continue;

         UNOCCLUDED_POLYGON_IDS_MAP::iterator iter=unoccluded_polygon_ids_map.
            find(curr_polygon_ID);
         if (iter==unoccluded_polygon_ids_map.end()) 
         {
            unoccluded_polygon_ids_map[curr_polygon_ID]=1;
         }
         else
         {
            iter->second=iter->second+1;
         }
         
      } // loop over pv index
   } // loop over pu index

   vector<unsigned int> unoccluded_polygon_IDs,unoccluded_polygon_npixels;
   for (UNOCCLUDED_POLYGON_IDS_MAP::iterator iter=unoccluded_polygon_ids_map.
           begin(); iter != unoccluded_polygon_ids_map.end(); iter++)
   {
      unoccluded_polygon_IDs.push_back(iter->first);
      unoccluded_polygon_npixels.push_back(iter->second);
   }
   templatefunc::Quicksort(unoccluded_polygon_npixels,unoccluded_polygon_IDs);

//   cout << "unoccluded_polygon_IDs.size() = "
//        << unoccluded_polygon_IDs.size() << endl;

// Ignore any polygons whose unoccluded pixel content is negligibly
// small:

   vector<unsigned int> insignificant_unoccluded_polygon_IDs;
   vector<unsigned int> significantly_unoccluded_polygon_IDs;
   for (unsigned int p=0; p<unoccluded_polygon_IDs.size(); p++)
   {
//      int unoccluded_polygon_ID=unoccluded_polygon_IDs[p];
      int n_pixels=unoccluded_polygon_npixels[p];
//      cout << "p = " << p 
//           << " unoccluded_polygon_ID = " << unoccluded_polygon_ID
//           << " n_pixels = " << n_pixels
//           << endl;
      const int min_unoccluded_pixels=100;
      if (n_pixels <=min_unoccluded_pixels)
      {
         insignificant_unoccluded_polygon_IDs.push_back(
            unoccluded_polygon_IDs[p]);
      }
      else
      {
         significantly_unoccluded_polygon_IDs.push_back(
            unoccluded_polygon_IDs[p]);
      }
   }
    
   return significantly_unoccluded_polygon_IDs;
}

// ==========================================================================
// Building coloring member functions
// ==========================================================================

// Member function get_Face_Polyhedron_Building_IDs() takes in some
// pixel location within the current image plane.  It retrieves and
// returns the face, polyhedron and Building IDs corresponding to the
// specified pixel if the clipping OBSFRUSTUM's ray intersects some
// clipped polygon.  Otherwise, this method returns (-1,-1,-1).

threevector Clipping::get_Face_Polyhedron_Building_IDs(int pu,int pv)
{
//   cout << "inside Clipping::get_Face_Polyhedron_Building_IDs()" << endl;

   threevector FacePolyhedronBuildingIDs(-1,-1,-1);

   int polygon_ID=ptwoDarray_ptr->get(pu,pv);
   for (unsigned int p=0; p<clipped_face_polygon_ptrs.size(); p++)
   {
      polygon* clipped_face_polygon_ptr=clipped_face_polygon_ptrs[p];
      if (clipped_face_polygon_ptr->get_ID()==polygon_ID)
      {
         CLIPPED_FACES_MAP::iterator iter=clipped_faces_map_ptr->find(
            clipped_face_polygon_ptr);
         if (iter != clipped_faces_map_ptr->end()) 
         {
            FacePolyhedronBuildingIDs=iter->second;
            break;
         }
      }
   } // loop over index p labeling all clipped face polygons

   return FacePolyhedronBuildingIDs;
}

// ---------------------------------------------------------------------
// Member function backproject_Building_colors()

void Clipping::backproject_Building_colors()
{
   cout << "inside Clipping::backproject_Building_colors()" << endl;

   unsigned int mdim=HtwoDarray_ptr->get_mdim();
   unsigned int ndim=HtwoDarray_ptr->get_ndim();

   const double min_height_above_ground=4;	// meters
   for (unsigned int pu=0; pu<mdim; pu++)
   {
      for (unsigned int pv=0; pv<ndim; pv++)
      {

// Ignore any black-colored pixels within input filter
// *texture_rectangle_ptr:

         int Rfiltered,Gfiltered,Bfiltered;
         texture_rectangle_ptr->get_pixel_RGB_values(
            pu,pv,Rfiltered,Gfiltered,Bfiltered);
         if (Rfiltered < 1 && Gfiltered < 1 && Bfiltered < 1) continue;

         int Building_ID=get_Face_Polyhedron_Building_IDs(pu,pv).get(2);
//         cout << "pu = " << pu << " pv = " << pv
//              << " BldgID = " << Building_ID << endl;
         
         if (Building_ID < 0) continue;
         
         double v=VtwoDarray_ptr->get(pu,pv);
         if (v < 0.01) continue;

         double h=HtwoDarray_ptr->get(pu,pv);
         double s=StwoDarray_ptr->get(pu,pv);

         Building* Building_ptr=BuildingsGroup_ptr->get_Building_ptr(
            Building_ID);
         double ground_z=Building_ptr->get_ground_z();
         double height_above_ground=ztwoDarray_ptr->get(pu,pv)-ground_z;
         if (height_above_ground < min_height_above_ground) continue;

         Building_ptr->push_back_hsv(h,s,v);
      } // loop over pv index
   } // loop over pu index

   BuildingsGroup_ptr->compute_primary_Building_colors();
}

// ---------------------------------------------------------------------
// Member function reset_Building_colors()

void Clipping::reset_Building_colors()
{
   cout << "inside Clipping::reset_Building_colors()" << endl;

   BuildingsGroup::BUILDING_POLYHEDRON_MAP* Building_polyhedron_map_ptr=
      BuildingsGroup_ptr->get_Building_polyhedron_map_ptr();

   PolyhedraGroup::POLYHEDRON_MAP* Polyhedron_map_ptr=
      PolyhedraGroup_ptr->get_Polyhedron_map_ptr();

   for (unsigned int b=0; b<BuildingsGroup_ptr->get_n_Buildings(); b++)
   {
      Building* Building_ptr=BuildingsGroup_ptr->get_Building_ptr(b);
      colorfunc::RGB primary_RGB=Building_ptr->get_primary_color();

      if (primary_RGB.first < 0) continue;
      osg::Vec4 white_color(1,1,1,1);
      osg::Vec4 primary_color=colorfunc::get_OSG_color(primary_RGB);

      for (BuildingsGroup::BUILDING_POLYHEDRON_MAP::iterator iter=
              Building_polyhedron_map_ptr->begin();
           iter != Building_polyhedron_map_ptr->end(); iter++)
      {
         twovector building_polyhedron_ids=iter->first;
         int curr_Building_ID=
            basic_math::round(building_polyhedron_ids.get(0));
         if (curr_Building_ID != Building_ptr->get_ID()) continue;
         
         polyhedron* polyhedron_ptr=iter->second;

         PolyhedraGroup::POLYHEDRON_MAP::iterator PM_iter=
            Polyhedron_map_ptr->find(polyhedron_ptr);
         if (PM_iter != Polyhedron_map_ptr->end()) 
         {
            Polyhedron* Polyhedron_ptr=PM_iter->second;
            Polyhedron_ptr->set_color(white_color,primary_color);
         }
      } // loop over iterator
      
   } // loop over index b labeling Buildings
}

// ---------------------------------------------------------------------
// Member function export_Building_colors()

void Clipping::export_Building_colors()
{
   cout << "inside Clipping::export_Building_colors()" << endl;

   string building_colors_filename="building_colors.dat";
   ofstream colors_stream;
   filefunc::appendfile(building_colors_filename,colors_stream);

   vector<int> analyzed_OBSFRUSTA_IDs;
   for (ANALYZED_OBSFRUSTA_IDS_MAP::iterator iter=
           analyzed_OBSFRUSTA_IDs_map_ptr->begin();
        iter != analyzed_OBSFRUSTA_IDs_map_ptr->end(); iter++)
   {
      analyzed_OBSFRUSTA_IDs.push_back(iter->first);
   }
   std::sort(analyzed_OBSFRUSTA_IDs.begin(),analyzed_OBSFRUSTA_IDs.end());

   colors_stream << "# Number of analyzed OBSFRUSTA = "
                 << analyzed_OBSFRUSTA_IDs.size() << endl;
   colors_stream << "# Analyzed OBSFRUSTA IDS : ";
   for (unsigned int i=0; i<analyzed_OBSFRUSTA_IDs.size(); i++)
   {
      colors_stream << analyzed_OBSFRUSTA_IDs[i] << " ";
   }
   colors_stream << endl << endl;
   colors_stream << "# Building ID   R   G   B" << endl << endl;
   
   for (unsigned int b=0; b<BuildingsGroup_ptr->get_n_Buildings(); b++)
   {
      Building* Building_ptr=BuildingsGroup_ptr->get_Building_ptr(b);
      colorfunc::RGB primary_RGB=Building_ptr->get_primary_color();
      double red=primary_RGB.first;
      double green=primary_RGB.second;
      double blue=primary_RGB.third;
      if (red < 0 || green < 0 || blue < 0) continue;
      colors_stream << Building_ptr->get_ID() << "  "
                    << red << "  "
                    << green << "  "
                    << blue << endl;
   } // loop over index b labeling buildings
   colors_stream << endl;

   filefunc::closefile(building_colors_filename,colors_stream);   
}

// ---------------------------------------------------------------------
// Member function import_Building_colors()

void Clipping::import_Building_colors()
{
   cout << "inside Clipping::import_Building_colors()" << endl;

   string building_colors_filename="building_colors.dat";
   filefunc::ReadInfile(building_colors_filename);
   
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=
         stringfunc::string_to_numbers(filefunc::text_line[i]);
      int Building_ID=column_values[0];
      Building* Building_ptr=BuildingsGroup_ptr->get_Building_ptr(
         Building_ID);
      double r=column_values[1];
      double g=column_values[2];
      double b=column_values[3];
//      cout << "BldgID = " << Building_ID << "r = " << r 
//           << " g = " << g << " b = " << b << endl;
      Building_ptr->set_primary_color(r,g,b);
   }
   reset_Building_colors();
}

// ---------------------------------------------------------------------
// Member function backproject_OBSFRUSTA_colorings()

void Clipping::backproject_OBSFRUSTA_colorings(int nskip)
{
   unsigned int n_OBSFRUSTA=OBSFRUSTAGROUP_ptr->get_n_Graphicals();
   for (unsigned int n=0; n<n_OBSFRUSTA; n += nskip)
   {
      cout << endl;
      cout << "------------------------------------------------------" << endl;
      cout << "n = " << n << " n_OBSFRUSTA = " << n_OBSFRUSTA << endl;
      OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(n);
      OBSFRUSTUM* selected_OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
         get_selected_OBSFRUSTUM_ptr();
      camera* camera_ptr=selected_OBSFRUSTUM_ptr->get_Movie_ptr()->
         get_camera_ptr();
      threevector camera_posn=camera_ptr->get_world_posn();


      OBSFRUSTAGROUP_ptr->load_hires_photo();
      update_display();
      OBSFRUSTAGROUP_ptr->load_thumbnail_photo(n);
   }

   string banner="Finished backprojecting OBSFRUSTA colorings";
   outputfunc::write_big_banner(banner);
}

// ==========================================================================
// Sky detection member functions
// ==========================================================================

// Member function generate_candidate_sky_colors() first forms an
// 8x8x8 VCP grid.  It then loops over all pixels within
// *buildings_texture_rectangle_ptr and extracts RGB values from
// pixels located above building rooftops.  Quantized RGB values are
// accumulated within *VCP_ptr.  After renormalizing the VCP's counts
// and converting them into probabilities, this method finds all
// voxels with probabilities exceeding a minimal threshold.  The RGB
// values of these candidate sky voxels are returned within an output
// STL vector.

vector<threevector> Clipping::generate_candidate_sky_colors(
   int bldg_value,int sky_value,
   const texture_rectangle* buildings_texture_rectangle_ptr,
   const texture_rectangle* buildings_mask_texture_rectangle_ptr)
{
   cout << "inside Clipping::generate_candidate_sky_colors()" << endl;

   VolumetricCoincidenceProcessor* VCP_ptr=
      new VolumetricCoincidenceProcessor();

   threevector XYZ_minimum(0,0,0);
   threevector XYZ_maximum(255,255,255);
   float binlength=8;
   VCP_ptr->initialize_coord_system(XYZ_minimum,XYZ_maximum,binlength);

   unsigned int mdim=buildings_texture_rectangle_ptr->getWidth();
   unsigned int ndim=buildings_texture_rectangle_ptr->getHeight();

   cout << "Accumulating RGB points in VCP" << endl;
   for (unsigned int pu=0; pu<mdim; pu++)
   {
      bool bldg_rooftop_encountered_flag=false;
      for (unsigned int pv=0; pv<ndim && !bldg_rooftop_encountered_flag; pv++)
      {
         int R,G,B;
         buildings_mask_texture_rectangle_ptr->get_pixel_RGB_values(
            pu,pv,R,G,B);
         if (R==bldg_value && G==bldg_value && B==bldg_value)
         {
            bldg_rooftop_encountered_flag=true;
         }
         else if (R==sky_value && G==sky_value && B==sky_value)
         {
            for (unsigned int py=0; py<pv; py++)
            {
               buildings_texture_rectangle_ptr->get_pixel_RGB_values(
                  pu,py,R,G,B);

// Perform sanity check on candidate sky pixel HSV values:

               double r=R/255.0;
               double g=G/255.0;
               double b=B/255.0;
               double h,s,v;
               colorfunc::RGB_to_hsv(r,g,b,h,s,v);

               const double max_s=0.5;
               const double min_v=0.5;
               if (s > max_s || v < min_v) continue;

               double R_quantized=(R/8)*8;
               double G_quantized=(G/8)*8;
               double B_quantized=(B/8)*8;
               VCP_ptr->accumulate_points(R_quantized,G_quantized,B_quantized);

            } // loop over py index
         } // R,G,B > 0 conditional
      } // loop over pv index
   } // loop over pu index

   cout << "Setting probs to renormalized counts" << endl;
   VCP_ptr->set_probs_to_renormalized_counts();

   const double min_sky_color_prob=0.0125;
//   const double min_sky_color_prob=0.025;
//   const double min_sky_color_prob=0.05;
//   const double min_sky_color_prob=0.1;

   int counts;
   double prob;
   vector<threevector> seed_sky_RGBs;
   for (unsigned int m=0; m<VCP_ptr->get_mdim(); m++)
   {
      for (unsigned int n=0; n<VCP_ptr->get_ndim(); n++)
      {
         for (unsigned int p=0; p<VCP_ptr->get_pdim(); p++)
         {
            VCP_ptr->mnp_to_voxel_counts_and_prob(m,n,p,counts,prob);
            if (prob > min_sky_color_prob)
            {
               int R=m*8;
               int G=n*8;
               int B=p*8;
               cout << "R = " << R << " G = " << G << " B = " << B
                    << " prob = " << prob << endl;
               seed_sky_RGBs.push_back(threevector(R,G,B));
            }
         } // loop over p index
      } // loop over n index
   } // loop over m index

   delete VCP_ptr;

   return seed_sky_RGBs;
}

// ---------------------------------------------------------------------
// Member function find_candidate_sky_seed_pixels() loops over all
// pixels within *buildings_texture_rectangle_ptr.  The RGB values for
// any pixel which is classified as 'sky' or whose pv value lies above
// tallest building in the image are compared against all RGBs in
// seed_sky_RGBs.  If the pixel's RGB is sufficiently close to some
// seed sky RGB, the pixel's coordinates are added to STL vector
// seed_pixels.  The seed_pixels vector is returned at the end of this
// method.

vector<pair<int,int> > Clipping::find_candidate_sky_seed_pixels(
   int sky_value,unsigned int tallest_bldg_pv,
   const vector<threevector>& seed_sky_RGBs,
   texture_rectangle* buildings_texture_rectangle_ptr,
   const texture_rectangle* buildings_mask_texture_rectangle_ptr)
{
   cout << "inside Clipping::find_candidate_sky_seed_pixels()" << endl;

   vector<pair<int,int> > seed_pixels;

   unsigned int mdim=buildings_texture_rectangle_ptr->getWidth();
   unsigned int ndim=buildings_texture_rectangle_ptr->getHeight();

   for (unsigned int pu=0; pu<mdim; pu++)
   {
      for (unsigned int pv=0; pv<ndim; pv++)
      {
         int R,G,B;
         buildings_mask_texture_rectangle_ptr->get_pixel_RGB_values(
            pu,pv,R,G,B);
         if ((R==sky_value && G==sky_value && B==sky_value) ||
              pv < tallest_bldg_pv)
         {
            buildings_texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
            for (unsigned int s=0; s<seed_sky_RGBs.size(); s++)
            {
               int sky_R=seed_sky_RGBs[s].get(0);
               int sky_G=seed_sky_RGBs[s].get(1);
               int sky_B=seed_sky_RGBs[s].get(2);

//            cout << "s = " << s 
//                 << " sky_R = " << sky_R << " R = " << R
//                 << " sky_G = " << sky_G << " G = " << G
//                 << " sky_B = " << sky_B << " B = " << B
//                 << endl;
         
               const int threshold=8;
//               const int threshold=12;
//               const int threshold=16;
               if (colorfunc::color_match(R,G,B,sky_R,sky_G,sky_B,threshold))
               {
                  buildings_texture_rectangle_ptr->set_pixel_RGB_values(
                     pu,pv,0,0,0);
                  pair<int,int> P(pu,pv);
                  seed_pixels.push_back(P);
               }
            } // loop over index s labeling seed_sky_RGBs

         } // pixel=sky_value or pv < tallest_bldg_pv conditional
         
      } // loop over pv index
   } // loop over pu index

   cout << "n_seed_pixels = " << seed_pixels.size() << endl;
   return seed_pixels;
}
