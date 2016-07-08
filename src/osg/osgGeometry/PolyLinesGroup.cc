// ==========================================================================
// POLYLINESGROUP class member function definitions
// ==========================================================================
// Last modified on 1/22/16; 7/6/16; 7/7/16; 7/8/16
// ==========================================================================

#include <iomanip>
#include <vector>
#include <osg/Geode>
#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "osg/osgGeometry/ConesGroup.h"
#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "track/mover_funcs.h"
#include "numrec/nrfuncs.h"
#include "osg/osgGIS/postgis_database.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

#include "templates/mytemplates.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PolyLinesGroup::allocate_member_objects()
{
}		       

void PolyLinesGroup::initialize_member_objects()
{
   GraphicalsGroup_name="PolyLinesGroup";

   secs_since_Y2K = timefunc::secs_since_Y2K();
   altitude_dependent_labels_flag=true;
   variable_Point_size_flag=false;
   prev_framenumber=-1;
   currimage_PolyLine_index = 0;
   next_PolyLine_label="";
   next_PolyLine_mover_ptr=NULL;
   skeleton_height=350;	// meters (tuned for Bluegrass demo)
   constant_vertices_altitude=100;	// meters
   Pointsize_scalefactor=1;
   if (get_ndims()==2)
   {
      Pointsize_scalefactor=1;
//      Pointsize_scalefactor=10;
      ID_labels_flag=true;
   }
   else
   {
      ID_labels_flag=false;
   }
   
   textsize_scalefactor=1;
   Intersection_PointsGroup_ptr=NULL;
   photogroup_ptr=NULL;
   PolygonsGroup_ptr=NULL;
   PolyLinesGroup_3D_ptr=NULL;
   imageplane_PolyLinesGroup_ptr=NULL;
   PolyhedraGroup_ptr=NULL;
   babygis_database_ptr=NULL;
   movers_group_ptr=NULL;
   DataGraph_ptr=NULL;
   annotated_bboxes_map_ptr = NULL;
   osg_bboxes_map_ptr = NULL;
   max_image_width = max_image_height = -1;
   bbox_labels_filename="labeled_bboxes.txt";
   attribute_key = "";

   get_OSGgroup_ptr()->setUpdateCallback( 
        new AbstractOSGCallback<PolyLinesGroup>(
         this, &PolyLinesGroup::update_display));
}		       

PolyLinesGroup::PolyLinesGroup(
   const int p_ndims,Pass* PI_ptr,threevector* GO_ptr):
   GeometricalsGroup(p_ndims,PI_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

PolyLinesGroup::PolyLinesGroup(
   const int p_ndims, Pass* PI_ptr, AnimationController* AC_ptr,
   threevector* GO_ptr):
GeometricalsGroup(p_ndims, PI_ptr, AC_ptr, GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

PolyLinesGroup::PolyLinesGroup(
   const int p_ndims,Pass* PI_ptr,osgGeometry::PolygonsGroup* PG_ptr,
   AnimationController* AC_ptr, threevector* GO_ptr):GeometricalsGroup(
      p_ndims, PI_ptr, AC_ptr, GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   PolygonsGroup_ptr=PG_ptr;
   if (PolygonsGroup_ptr != NULL)
   {
      PolygonsGroup_ptr->set_PolyLinesGroup_ptr(this);
   }
}		       

PolyLinesGroup::PolyLinesGroup(
   const int p_ndims,Pass* PI_ptr,osgGeometry::PolygonsGroup* PG_ptr,
   PolyhedraGroup* PHG_ptr,AnimationController* AC_ptr, threevector* GO_ptr):
   GeometricalsGroup(p_ndims,PI_ptr,AC_ptr, GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   PolygonsGroup_ptr=PG_ptr;
   if (PolygonsGroup_ptr != NULL)
   {
      PolygonsGroup_ptr->set_PolyLinesGroup_ptr(this);
   }
   PolyhedraGroup_ptr=PHG_ptr;
}		       

PolyLinesGroup::PolyLinesGroup(
   const int p_ndims,Pass* PI_ptr,postgis_database* bgdb_ptr,
   threevector* GO_ptr):
   GeometricalsGroup(p_ndims,PI_ptr,GO_ptr)
{
   initialize_member_objects();
   allocate_member_objects();
   babygis_database_ptr=bgdb_ptr;
}

PolyLinesGroup::~PolyLinesGroup()
{
//   cout << "inside PolyLinesGroup destructor" << endl;
   
   if (Intersection_PointsGroup_ptr != NULL)
   {
      remove_OSGgroup_from_OSGsubPAT(
         Intersection_PointsGroup_ptr->get_OSGgroup_ptr());
      delete Intersection_PointsGroup_ptr;
   }

//   cout << "At end of PolyLinesGroup destructor, OSGgroup_refptr->refcount="
//        << get_OSGgroup_refptr_count() << endl;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PolyLinesGroup& P)
{
   for (unsigned int n=0; n<P.get_n_Graphicals(); n++)
   {
      PolyLine* PolyLine_ptr=P.get_PolyLine_ptr(n);
      outstream << "PolyLine node # " << n << endl;
      outstream << "PolyLine = " << *PolyLine_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

vector<PolyLine*> PolyLinesGroup::get_all_PolyLine_ptrs() const
{
//   cout << "inside PolyLinesGroup::get_all_PolyLine_ptrs()" << endl;
   vector<PolyLine*> PolyLine_ptrs;
   vector<Graphical*> Graphical_ptrs=get_all_Graphical_ptrs();
//   cout << "Graphical_ptrs.size() = " << Graphical_ptrs.size() << endl;
   for (unsigned int g=0; g<Graphical_ptrs.size(); g++)
   {
      PolyLine_ptrs.push_back(static_cast<PolyLine*>(Graphical_ptrs[g]));
   }
   return PolyLine_ptrs;
}

// ==========================================================================
// PolyLine generation member functions
// ==========================================================================

// This first version of member function generate_new_PolyLine
// dynamically instantiates an empty PolyLine object.  Its vertices
// are intended to be sequentially added later as the PolyLine is
// built up....

PolyLine* PolyLinesGroup::generate_new_PolyLine(
   bool force_display_flag,bool single_polyline_per_geode_flag,
   int n_text_messages,int ID,int OSGsubPAT_number)
{
//   cout << "inside PolyLinesGroup::generate_new_PolyLine() #1" << endl;
   if (ID==-1) ID=get_next_unused_ID();

   PolyLine* curr_PolyLine_ptr=new PolyLine(
      get_ndims(),get_pass_ptr(),get_grid_world_origin_ptr(),
      font_refptr.get(),n_text_messages,ID,get_CM_ptr());

   initialize_new_PolyLine(
      curr_PolyLine_ptr,single_polyline_per_geode_flag,force_display_flag,
      OSGsubPAT_number);
   reset_colors();
   return curr_PolyLine_ptr;
}

// ---------------------------------------------------------------------
PolyLine* PolyLinesGroup::generate_new_PolyLine(
   const threevector& reference_origin,
   bool force_display_flag,bool single_polyline_per_geode_flag,
   int n_text_messages,int ID,int OSGsubPAT_number)
{
//    cout << "inside PLG::generate_new_PolyLine() #2" << endl;
   PolyLine* curr_PolyLine_ptr=generate_new_PolyLine(
      force_display_flag,single_polyline_per_geode_flag,
      n_text_messages,ID);
   curr_PolyLine_ptr->set_reference_origin(reference_origin);
   return curr_PolyLine_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_new_PolyLine returns a dynamically
// instantiated polyline running through the vertices contained within
// input vector V.

PolyLine* PolyLinesGroup::generate_new_PolyLine(
   const vector<threevector>& V,
   bool force_display_flag,bool single_polyline_per_geode_flag,
   int n_text_messages,int ID,int OSGsubPAT_number)
{
//   cout << "inside PolyLinesGroup::generate_new_PolyLine() #3" << endl;
//   cout << "inside PolyLinesGroup::generate_new_PolyLine()" << endl;
//   cout << "single_polyline_per_geode_flag = " 
//        << single_polyline_per_geode_flag << endl;
   threevector reference_origin;
   if (single_polyline_per_geode_flag)
   {
      reference_origin=V[0];
   }
   else
   {
      if (get_ndims()==2)
      {
         reference_origin=Zero_vector;
      }
      else
      {
         reference_origin=get_grid_world_origin();
      }
   }
//   cout << "reference_origin = " << reference_origin << endl;
//   cout << "get_grid_world_origin() = "
//        << get_grid_world_origin() << endl;

   return generate_new_PolyLine(
      reference_origin,V,force_display_flag,
      single_polyline_per_geode_flag,n_text_messages,ID);
}

// ---------------------------------------------------------------------
PolyLine* PolyLinesGroup::generate_new_PolyLine(
   const threevector& reference_origin,const vector<threevector>& V,
   bool force_display_flag,bool single_polyline_per_geode_flag,
   int n_text_messages,int ID,int OSGsubPAT_number)
{
//   cout << "inside PolyLinesGroup::generate_new_PolyLine() #4" << endl;
   if (ID==-1) ID=get_next_unused_ID();
//   cout << "ID = " << ID << endl;

   PolyLine* curr_PolyLine_ptr=new PolyLine(
      get_ndims(),get_pass_ptr(),
      get_grid_world_origin_ptr(),reference_origin,V,
      font_refptr.get(),n_text_messages,ID,get_CM_ptr());

   initialize_new_PolyLine(
      curr_PolyLine_ptr,single_polyline_per_geode_flag,force_display_flag,
      OSGsubPAT_number);

// On Aug 22, 2007, we learned the painful way that we should NOT
// issue a call to reset_colors() from within this overloaded version
// of generate_new_PolyLine() which is called by
// postgis_database::parse_linestring_geometry().  If we do, program
// NYROADS slows down to a crawl...

   return curr_PolyLine_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_new_PolyLine returns a dynamically
// instantiated polyline running through the vertices contained within
// input vector V.

PolyLine* PolyLinesGroup::generate_new_PolyLine(
   const vector<threevector>& V,const osg::Vec4& uniform_color,
   bool force_display_flag,bool single_polyline_per_geode_flag,
   int n_text_messages,int ID,int OSGsubPAT_number)
{
//   cout << "inside PolyLinesGroup::generate_new_PolyLine() #5" << endl;
   PolyLine* curr_PolyLine_ptr=generate_new_PolyLine(
      V,force_display_flag,single_polyline_per_geode_flag,n_text_messages,ID);

   curr_PolyLine_ptr->set_permanent_color(uniform_color);
   
   reset_colors();
   return curr_PolyLine_ptr;
}

// ---------------------------------------------------------------------
PolyLine* PolyLinesGroup::generate_new_PolyLine(
   const threevector& reference_origin,const vector<threevector>& V,
   const osg::Vec4& uniform_color,bool force_display_flag,
   bool single_polyline_per_geode_flag,int n_text_messages,int ID,
   int OSGsubPAT_number)
{
//   cout << "inside PolyLinesGroup::generate_new_PolyLine() #6" << endl;
   PolyLine* curr_PolyLine_ptr=generate_new_PolyLine(
      reference_origin,V,force_display_flag,single_polyline_per_geode_flag,
      n_text_messages,ID);
   curr_PolyLine_ptr->set_permanent_color(uniform_color);
   reset_colors();
   return curr_PolyLine_ptr;
}

// ---------------------------------------------------------------------
PolyLine* PolyLinesGroup::generate_new_PolyLine(
   const threevector& reference_origin,const vector<threevector>& V,
   const vector<osg::Vec4>& colors,
   bool force_display_flag,bool single_polyline_per_geode_flag,
   int n_text_messages,int ID,int OSGsubPAT_number)
{
//   cout << "inside PolyLinesGroup::generate_new_PolyLine() #7" << endl;
   PolyLine* curr_PolyLine_ptr=generate_new_PolyLine(
      reference_origin,V,force_display_flag,single_polyline_per_geode_flag,
      n_text_messages,ID);
   curr_PolyLine_ptr->set_local_colors(colors);
   curr_PolyLine_ptr->set_colors(colors);
   curr_PolyLine_ptr->set_multicolor_flag(true);
   reset_colors();
   return curr_PolyLine_ptr;
}

// ---------------------------------------------------------------------
void PolyLinesGroup::initialize_new_PolyLine(
   PolyLine* curr_PolyLine_ptr,bool single_polyline_per_geode_flag,
   bool force_display_flag,int OSGsubPAT_number)
{
//   cout << "inside PolyLinesGroup::initialize_new_PolyLine()" << endl;
//   cout << "curr_PolyLine_ptr = " << curr_PolyLine_ptr << endl;

   GraphicalsGroup::insert_Graphical_into_list(curr_PolyLine_ptr);
   initialize_Graphical(curr_PolyLine_ptr);

// Recall that as of Nov 2006, we store relative vertex information
// with respect to the average of all vertices in STL vector V within
// *curr_PolyLine_ptr to avoid floating point problems.  So we need to
// translate the polyline by its average vertex in order to globally
// position it:

   curr_PolyLine_ptr->set_UVW_coords(
      get_curr_t(),get_passnumber(),
      curr_PolyLine_ptr->get_reference_origin());

//   cout << "single_polyline_per_geode_flag = "
//        << single_polyline_per_geode_flag << endl;
   if (single_polyline_per_geode_flag)
   {
      osg::Geode* geode_ptr=curr_PolyLine_ptr->generate_drawable_geode(
         force_display_flag);
      curr_PolyLine_ptr->get_PAT_ptr()->addChild(geode_ptr);
      insert_graphical_PAT_into_OSGsubPAT(curr_PolyLine_ptr,OSGsubPAT_number);

// Add PointsGroup containing PolyLine vertices to OSGsubPAT:
      
      osgGeometry::PointsGroup* PointsGroup_ptr=
         curr_PolyLine_ptr->get_PointsGroup_ptr();
      if (PointsGroup_ptr != NULL)
      {
         insert_OSGgroup_into_OSGsubPAT(
            PointsGroup_ptr->get_OSGgroup_ptr(),OSGsubPAT_number);
      }

// Add ConesGroup indicating local PolyLine flow directions to
// OSGsubPAT:
      
      ConesGroup* ConesGroup_ptr=
         curr_PolyLine_ptr->get_ConesGroup_ptr();
      if (ConesGroup_ptr != NULL)
      {
         insert_OSGgroup_into_OSGsubPAT(
            ConesGroup_ptr->get_OSGgroup_ptr(),OSGsubPAT_number);
      }

   } // single_polyline_per_geode_flag
}

// ---------------------------------------------------------------------
// Member function generate_UV_Polyline() forms a 2D line based upon
// input homogeneous threevector l representing an infinite line in UV
// space.

PolyLine* PolyLinesGroup::generate_UV_PolyLine(
   const threevector& l,int n_text_messages,int ID,int OSGsubPAT_number)
{
//   cout << "inside PolyLinesGroup::generate_UV_PolyLine()" << endl;

   if (get_ndims() != 2)
   {
      cout << "Error in PolyLinesGroup::generate_UV_PolyLine(l)!" << endl;
      cout << "Ndims must equal 2" << endl;
      return NULL;
   }

   if (ID==-1) ID=get_next_unused_ID();
//   cout << "ID = " << ID << endl;

   polyline UV_line(l);
   vector<threevector> vertices;
   vertices.push_back(UV_line.get_vertex(0));
   vertices.push_back(UV_line.get_vertex(1));

   bool force_display_flag=false;
   bool single_polyline_per_geode_flag=true;
   
   return generate_new_PolyLine(
      vertices,force_display_flag,single_polyline_per_geode_flag,
      n_text_messages,ID,OSGsubPAT_number);
}

// ==========================================================================
// PolyLine destruction member functions
// ==========================================================================

// Member function destroy_all_PolyLines() first fills an STL vector
// with PolyLine pointers.  It then iterates over each vector entry
// and calls destroy_PolyLine for each PolyLine pointer.  On 5/3/08,
// we learned the hard and painful way that this two-step process is
// necessary in order to correctly purge all PolyLines.

void PolyLinesGroup::destroy_all_PolyLines()
{   
//   cout << "inside PolyLinesGroup::destroy_all_PolyLines()" << endl;
   unsigned int n_PolyLines=get_n_Graphicals();
//   cout << "n_PolyLines = " << n_PolyLines << endl;

   vector<PolyLine*> PolyLines_to_destroy;
   for (unsigned int p=0; p<n_PolyLines; p++)
   {
      PolyLine* PolyLine_ptr=get_PolyLine_ptr(p);
//      cout << "p = " << p << " PolyLine_ptr = " << PolyLine_ptr << endl;
      PolyLines_to_destroy.push_back(PolyLine_ptr);
   }

   for (unsigned int p=0; p<n_PolyLines; p++)
   {
      destroy_PolyLine(PolyLines_to_destroy[p]);
   }
}

// --------------------------------------------------------------------------
// Member function destroy_PolyLine removes the selected PolyLine from
// the PolyLinelist and the OSG PolyLines group. 

bool PolyLinesGroup::destroy_PolyLine()
{   
//   cout << "inside PolyLinesGroup::destroy_PolyLine()" << endl;
   return destroy_PolyLine(get_selected_Graphical_ID());
}

// --------------------------------------------------------------------------
bool PolyLinesGroup::destroy_PolyLine(int ID)
{   
//   cout << "inside PolyLinesGroup::destroy_PolyLine(int ID)" << endl;
//   cout << "int ID = " << ID << endl;
   if (ID >= 0)
   {
      return destroy_PolyLine(get_ID_labeled_PolyLine_ptr(ID));
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
bool PolyLinesGroup::destroy_PolyLine(PolyLine* curr_PolyLine_ptr)
{
//   cout << "inside PolyLinesGroup::destroy_PolyLine(curr_PolyLine_ptr)" 
//        << endl;
//   cout << "curr_PolyLine_ptr = " << curr_PolyLine_ptr << endl;
//   cout << "PolyLinesGroup this = " << this << endl;

   if (curr_PolyLine_ptr==NULL) return false;

// Recall that PointsGroup is added as a child to OSGsubPAT.  So we
// must explicitly remove it from the scenegraph before the PolyLine
// is destroyed:

   osgGeometry::PointsGroup* PointsGroup_ptr=
      curr_PolyLine_ptr->get_PointsGroup_ptr();
   if (PointsGroup_ptr != NULL)
   {
      remove_OSGgroup_from_OSGsubPAT(PointsGroup_ptr->get_OSGgroup_ptr());
   }

   ConesGroup* ConesGroup_ptr=
      curr_PolyLine_ptr->get_ConesGroup_ptr();
   if (ConesGroup_ptr != NULL)
   {
      remove_OSGgroup_from_OSGsubPAT(ConesGroup_ptr->get_OSGgroup_ptr());
   }

//   cout << "Before call to destroy_Graphical, get_n_Graphicals() = "
//        << get_n_Graphicals() << endl;
   bool flag=destroy_Graphical(curr_PolyLine_ptr);
//   cout << "After call to destroy_Graphical, get_n_Graphicals() = "
//        << get_n_Graphicals() << endl;

//   for (unsigned int n=0; n<get_n_Graphicals(); n++)
//   {
//      Geometrical* Geometrical_ptr=get_Geometrical_ptr(n);
//      cout << "n = " << n 
//           << " Geometrical_ptr = " << Geometrical_ptr 
//           << " Geometrical_ptr->get_ID() = " << Geometrical_ptr->get_ID()
//           << endl;
//   }
   
   return flag;
}

// ==========================================================================
// PolyLine manipulation member functions
// ==========================================================================

// Member function regnenerate_PolyLine destroys the contents within
// *orig_PolyLine_ptr and returns a new PolyLine generated from
// information passed via STL vector vertices.

PolyLine* PolyLinesGroup::regenerate_PolyLine(
   const vector<threevector>& vertices,PolyLine* orig_PolyLine_ptr,
   osg::Vec4 permanent_color,osg::Vec4 selected_color,
   bool display_vertices_flag,int ID)
{   
//   cout << "inside PolyLinesGroup::regenerate_PolyLine(vertices,orig_PolyLine_ptr)" 
//        << endl;
//   cout << "ndims = " << get_ndims() << endl;
//   int selected_PolyLine_ID=get_selected_Graphical_ID();
//   cout << "selected_PolyLine_ID = " << selected_PolyLine_ID << endl;
   
//   cout << "vertices = " << endl;
//   templatefunc::printVector(vertices);

   int new_PolyLine_ID=orig_PolyLine_ptr->get_ID();
   if (ID != -1)
   {
      new_PolyLine_ID=ID;
   }
//   int orig_PolyLine_ID=orig_PolyLine_ptr->get_ID();
//   cout << "orig_PolyLine_ID = " << orig_PolyLine_ID << endl;

   bool orig_entry_finished_flag=orig_PolyLine_ptr->get_entry_finished_flag();
   bool orig_shrunken_text_label_flag=orig_PolyLine_ptr->
      get_shrunken_text_label_flag();
   osg::Vec4 orig_permanent_text_color=orig_PolyLine_ptr->
      get_permanent_text_color();
   mover* orig_mover_ptr=orig_PolyLine_ptr->get_mover_ptr();
//   cout << "orig_mover_ptr = " << orig_mover_ptr << endl;

   destroy_PolyLine(orig_PolyLine_ptr);

   bool force_display_flag=false;
   bool single_polyline_per_geode_flag=true;

   PolyLine* new_PolyLine_ptr=generate_new_PolyLine(
      vertices,permanent_color,force_display_flag,
      single_polyline_per_geode_flag,
      get_n_text_messages(),new_PolyLine_ID);

   new_PolyLine_ptr->set_entry_finished_flag(orig_entry_finished_flag);
   new_PolyLine_ptr->set_shrunken_text_label_flag(
      orig_shrunken_text_label_flag);
   new_PolyLine_ptr->set_permanent_text_color(orig_permanent_text_color);
   new_PolyLine_ptr->set_mover_ptr(orig_mover_ptr);
   new_PolyLine_ptr->dirtyDisplay();

// Add vertex points to new PolyLine:

   if (display_vertices_flag)
   {
      double PolyLine_sizefactor=1;
      if (get_ndims()==2)
      {
         PolyLine_sizefactor=1.0/8.0;
      }

      if (get_variable_Point_size_flag())
      {
         PolyLine_sizefactor=new_PolyLine_ptr->get_length_sizefactor();
      }

      osgGeometry::PointsGroup* PointsGroup_ptr=new_PolyLine_ptr->
         get_PointsGroup_ptr();
      PointsGroup_ptr->set_crosshairs_size(
         PolyLine_sizefactor*Pointsize_scalefactor*
         PointsGroup_ptr->get_crosshairs_size());
      PointsGroup_ptr->set_crosshairs_text_size(
         PolyLine_sizefactor*textsize_scalefactor*
         PointsGroup_ptr->get_crosshairs_text_size());

      new_PolyLine_ptr->add_vertex_points(vertices,selected_color);
   } // display_vertices_flag

//   selected_PolyLine_ID=get_selected_Graphical_ID();
//   cout << "selected_PolyLine_ID = " << selected_PolyLine_ID << endl;

   return new_PolyLine_ptr;
}

// --------------------------------------------------------------------------
// Member function form_polyhedron_skeleton takes the input PolyLine
// as the base outline for a polyhedron skeleton.  It extrudes the
// base face's vertices upward to a constant height set by member
// skeleton_height.  This method then instantiates a second PolyLine
// which traces out both the top and sides of the polyhedron skeleton.
// Once both the bottom and top PolyLines forming the polyhedron
// skeleton have been instantiated, an add vertex message for the last
// mover within the movers_group queue is issued.

void PolyLinesGroup::form_polyhedron_skeleton(
   PolyLine* bottom_PolyLine_ptr,
   osg::Vec4 permanent_color,osg::Vec4 selected_color,
   bool display_vertices_flag)
{   
//   cout << "inside PolyLinesGroup::form_polyhedron_skeleton()" << endl;
//   cout << "permanent_color = " << endl;
//   osgfunc::print_Vec4(permanent_color);
//   cout << "selected_color = " << endl;
//   osgfunc::print_Vec4(selected_color);

// First form closed contour from entered polyline vertices.  Compute
// its central location for future fixed track generation purposes:

   polyline* polyline_ptr=bottom_PolyLine_ptr->get_or_set_polyline_ptr();

   polygon ROI_bottom_poly(*polyline_ptr);
   threevector ROI_bottom_COM=ROI_bottom_poly.compute_COM();

// Compute bounding box for bottom face and its area.  Then form
// relative size metric for ROI based upon sqrt(bbox_area) to typical
// ROI characterisic length scale of 65 meters:

//   double relative_ROI_size=mover_func::compute_relative_ROI_size(
//      polyline_ptr);

// Form second closed contour extruded upwards in height.  Add
// vertical spines to connect original and extruded polylines.
// Extrude bottom face vertices upwards so that they all have
// skeleton_height altitudes:

//   cout << "skeleton_height = " << skeleton_height << endl;
   vector<threevector> extruded_polyline_vertices=
      mover_func::extrude_bottom_ROI_face(polyline_ptr,skeleton_height);

   bool force_display_flag=false;
   bool single_polyline_per_geode_flag=true;
   set_n_text_messages(1);
   PolyLine* top_PolyLine_ptr=generate_new_PolyLine(
      bottom_PolyLine_ptr->get_reference_origin(),
      extruded_polyline_vertices,force_display_flag,
      single_polyline_per_geode_flag,get_n_text_messages());
   extruded_polyline_vertices.push_back(
      extruded_polyline_vertices.front());

// Add mover pointers into top and bottom PolyLines defining
// Polyhedron skeleton.  Set PolyLines' IDs based upon mover's ID:

   unsigned int curr_ROI_ID=get_n_Graphicals()/2;
   mover* next_mover_ptr=get_next_PolyLine_mover_ptr();
   if (next_mover_ptr != NULL)
   {
      track* ROI_track_ptr=next_mover_ptr->get_track_ptr();
      ROI_track_ptr->set_posn_velocity(0,ROI_bottom_COM,Zero_vector);

      bottom_PolyLine_ptr->set_mover_ptr(next_mover_ptr);
      top_PolyLine_ptr->set_mover_ptr(next_mover_ptr);
      set_next_PolyLine_mover_ptr(NULL);

      curr_ROI_ID=next_mover_ptr->get_ID();
      cout << "curr_ROI_ID = " << curr_ROI_ID << endl;
   }
         
   int bottom_PolyLine_ID=2*curr_ROI_ID;
   int top_PolyLine_ID=bottom_PolyLine_ID+1;
         
// Regenerate top and bottom PolyLines after having fixed their IDs:

   top_PolyLine_ptr=regenerate_PolyLine(
         extruded_polyline_vertices,top_PolyLine_ptr,
         permanent_color,selected_color,
         display_vertices_flag,top_PolyLine_ID);

// Add text label:

   string next_label=get_next_PolyLine_label();
//   cout << "next_label = " << next_label << endl;
   if (next_label.size() > 0)
   {
      top_PolyLine_ptr->set_COM_label(next_label);
      set_next_PolyLine_label("");
   }
         
   vector<threevector> bottom_vertices;
   for (unsigned int v=0; v<polyline_ptr->get_n_vertices(); v++)
   {
      bottom_vertices.push_back(polyline_ptr->get_vertex(v));
   }
   bottom_vertices.push_back(bottom_vertices.front());

   bottom_PolyLine_ptr=regenerate_PolyLine(
      bottom_vertices,bottom_PolyLine_ptr,
      permanent_color,selected_color,display_vertices_flag,
      bottom_PolyLine_ID);

   bottom_PolyLine_ptr->set_permanent_color(permanent_color);
   top_PolyLine_ptr->set_permanent_color(permanent_color);
}

// --------------------------------------------------------------------------
// Member function lowpass_filter_polyline performs a brute-force
// spatial filtering of the input PolyLine's vertices.

void PolyLinesGroup::lowpass_filter_polyline(PolyLine* curr_PolyLine_ptr)
{   
//   cout << "inside PolyLinesGroup::lowpass_filter_polyline()" << endl;
   
   polyline* polyline_ptr=curr_PolyLine_ptr->get_or_set_polyline_ptr();

   double ds=500;	// meters
//   double ds=1000;	// meters
//   cout << "********************************************************" << endl;
//   cout << "Before call to smooth_raw_vertices, polyline total length ="
//        << polyline_ptr->get_total_length() << endl;
//   outputfunc::enter_continue_char();
   polyline_ptr->smooth_raw_vertices(ds);
//   cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//   cout << "After call to smooth_raw_vertices, polyline total length ="
//        << polyline_ptr->get_total_length() << endl;
//   outputfunc::enter_continue_char();
}

// --------------------------------------------------------------------------
// Member function reset_PolyLine_altitudes resets all of the input
// PolyLine's vertices' z-values to member constant_vertices_altitude.

void PolyLinesGroup::reset_PolyLine_altitudes(PolyLine* curr_PolyLine_ptr)
{   
//   cout << "inside PolyLinesGroup::reset_PolyLine_altitude()" << endl;
   
   if (get_ndims() != 3) return;

   polyline* polyline_ptr=curr_PolyLine_ptr->get_or_set_polyline_ptr();

   double vertices_altitude=constant_vertices_altitude;
//   cout << "constant_vertices_altitude = " 
//        << constant_vertices_altitude << endl;

// If current PolyLine is not located close to world grid (e.g. when
// altering a UAV flight path), then offset its altitude by
// PolyLine.ID*100 meters so that multiple UAVs do not fly at exactly
// the same heights:

   if (vertices_altitude > get_grid_world_origin().get(2)+500)
   {
      vertices_altitude +=100*curr_PolyLine_ptr->get_ID();
   }
//   cout << "final vertices alt = " << vertices_altitude << endl;
   polyline_ptr->reset_vertex_altitudes(vertices_altitude);

   vector<threevector> V;
   for (unsigned int v=0; v<polyline_ptr->get_n_vertices(); v++)
   {
      V.push_back(polyline_ptr->get_vertex(v));
   }

   curr_PolyLine_ptr=regenerate_PolyLine(
      V,curr_PolyLine_ptr,curr_PolyLine_ptr->get_permanent_color(),
      curr_PolyLine_ptr->get_selected_color());
}

// --------------------------------------------------------------------------
// Member function merge_PolyLines() takes in an original PolyLine and
// retrieves the most recently added PolyLine.  It first computes the
// points along the original PolyLine that lie closest to the
// correction PolyLine's endpoints.  It next partitions the original
// PolyLine's vertices into those lying before & after the new points.
// This method forms a new set of vertices from the before & after
// vertices plus those from the most recently added PolyLine.  After
// destroying the original PolyLine and the most recently added one,
// it instantiates a new PolyLine from the new set of vertices and
// assigns it the original PolyLine's ID.

PolyLine* PolyLinesGroup::merge_PolyLines(PolyLine* original_PolyLine_ptr)
{
//   cout << "inside PolyLinesGroup::merge_PolyLines()" << endl;

   polyline* original_polyline_ptr=original_PolyLine_ptr->
      get_or_set_polyline_ptr();

   PolyLine* correction_PolyLine_ptr=static_cast<PolyLine*>(
      get_most_recently_added_Graphical_ptr());
   polyline* correction_polyline_ptr=correction_PolyLine_ptr->
      get_or_set_polyline_ptr();
//   cout << "orig polyline = " << *original_polyline_ptr << endl;
//   cout << "correction_polyline = " << *correction_polyline_ptr << endl;

// Find points along original PolyLine closest to the endpoints of the
// correction PolyLine.  Then compute those points' fractional
// distances along the original PolyLine:

   threevector new_start_vertex=correction_polyline_ptr->get_vertex(0);
   threevector new_stop_vertex=correction_polyline_ptr->get_last_vertex();
   
   threevector pnt_on_orig_polyline_closest_to_new_start_vertex;
   threevector pnt_on_orig_polyline_closest_to_new_stop_vertex;
   original_polyline_ptr->min_distance_to_point(
      new_start_vertex,pnt_on_orig_polyline_closest_to_new_start_vertex);
   original_polyline_ptr->min_distance_to_point(
      new_stop_vertex,pnt_on_orig_polyline_closest_to_new_stop_vertex);

//   int l1=original_polyline_ptr->find_edge_containing_point(
//      pnt_on_orig_polyline_closest_to_new_start_vertex);
//   int l2=original_polyline_ptr->find_edge_containing_point(
//      pnt_on_orig_polyline_closest_to_new_stop_vertex);

   double frac1=original_polyline_ptr->frac_distance_along_polyline(
      pnt_on_orig_polyline_closest_to_new_start_vertex);
   double frac2=original_polyline_ptr->frac_distance_along_polyline(
      pnt_on_orig_polyline_closest_to_new_stop_vertex);
   
//   cout << "new_start_vertex = " << new_start_vertex << endl;
//   cout << "pnt_on_orig_polyline_closest_to_new_start_vertex = "
//        << pnt_on_orig_polyline_closest_to_new_start_vertex << endl;
//   cout << "l1 = " << l1 << endl;
//   cout << "frac1 = " << frac1 << endl;

//   cout << "new_stop_vertex = " << new_stop_vertex << endl;
//   cout << "pnt_on_orig_polyline_closest_to_new_stop_vertex = "
//        << pnt_on_orig_polyline_closest_to_new_stop_vertex << endl;
//   cout << "l2 = " << l2 << endl;
//   cout << "frac2 = " << frac2 << endl;

// Partition original PolyLine's vertices into 3 categories:  

// 1.  Vertices lying in fraction interval [0,frac1]
// 2.  Vertices lying in fraction interval [frac1,frac2]
// 3.  Vertices lying in fraction interval [frac2,1]

   const double TINY=1E-6;
   vector<threevector> start_vertices=
      original_polyline_ptr->vertices_in_frac_interval(0-TINY,frac1);
   vector<threevector> excluded_vertices=
      original_polyline_ptr->vertices_in_frac_interval(frac1,frac2);
   vector<threevector> stop_vertices=
      original_polyline_ptr->vertices_in_frac_interval(frac2,1+TINY);

//   cout << "start_vertices = " << endl;
//   templatefunc::printVector(start_vertices);

//   cout << "stop_vertices = " << endl;
//   templatefunc::printVector(stop_vertices);

//   cout << "excluded_vertices = " << endl;
//   templatefunc::printVector(excluded_vertices);

// Generate new set of merged polyline vertices from those in
// categories 1 and 3 as well as the points on the original PolyLine
// closest to the correction PolyLine's start and stop vertices plus
// the correcton PolyLine's vertices:

   vector<threevector> new_vertices;
   for (unsigned int v=0; v<start_vertices.size(); v++)
   {
      new_vertices.push_back(start_vertices[v]);
   }
   new_vertices.push_back(pnt_on_orig_polyline_closest_to_new_start_vertex);

   for (unsigned int v=0; v<correction_polyline_ptr->get_n_vertices(); v++)
   {
      new_vertices.push_back(
         correction_polyline_ptr->get_vertex(v));
   }

   new_vertices.push_back(pnt_on_orig_polyline_closest_to_new_stop_vertex);
   for (unsigned int v=0; v<stop_vertices.size(); v++)
   {
      new_vertices.push_back(stop_vertices[v]);
   }

//   cout << "new_vertices = " << endl;
//   templatefunc::printVector(new_vertices);
   polyline new_polyline(new_vertices);

// Low-pass filter entire set of new vertices before using them to
// generate merged PolyLine:

   const double ds=250;	// meters
   vector<threevector> smoothed_vertices=new_polyline.smooth_raw_vertices(
      ds,new_vertices);

// Temporarily store original PolyLine's ID prior to destroying both
// the original PolyLine as well as the correction PolyLine:

   int original_PolyLine_ID=original_PolyLine_ptr->get_ID();
//   cout << "original_PolyLine_ID = " << original_PolyLine_ID << endl;
   osg::Vec4 uniform_color=original_PolyLine_ptr->get_permanent_color();

   destroy_PolyLine(original_PolyLine_ptr);
   destroy_PolyLine(correction_PolyLine_ptr);

// Generate new PolyLine based upon new set of merged PolyLine
// vertices.  Assign it the same ID as the original PolyLine:

   threevector reference_origin=get_grid_world_origin();

   bool force_display_flag=false;
   bool single_polyline_per_geode_flag=true;
   PolyLine* merged_PolyLine_ptr=generate_new_PolyLine(
      reference_origin,new_vertices,uniform_color,
//      reference_origin,smoothed_vertices,uniform_color,
      force_display_flag,single_polyline_per_geode_flag,
      get_n_text_messages(),original_PolyLine_ID);
//   merged_PolyLine_ptr->set_linewidth(10);

   merged_PolyLine_ptr->set_ID(original_PolyLine_ID);
//   cout << "get_ID_labeled_PolyLine_ptr(original_PolyLine_ID) = "
//        << get_ID_labeled_PolyLine_ptr(original_PolyLine_ID) << endl;
   return merged_PolyLine_ptr;
}

// --------------------------------------------------------------------------
// Member function move_z()

void PolyLinesGroup::move_z(double dz)
{   
//   cout << "inside PolyLinesGroup::move_z(), dz="
//      +stringfunc::number_to_string(dz) << endl;
   
   PolyLine* PolyLine_ptr=get_ID_labeled_PolyLine_ptr(
      get_selected_Graphical_ID());
   if (PolyLine_ptr==NULL) return;

   polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
   unsigned int n_vertices=polyline_ptr->get_n_vertices();
   vector<threevector> new_vertices;
   for (unsigned int n=0; n<n_vertices; n++)
   {
      threevector curr_vertex(polyline_ptr->get_vertex(n));
      curr_vertex.put(2,curr_vertex.get(2)+dz);
      new_vertices.push_back(curr_vertex);
   }

   PolyLine_ptr=regenerate_PolyLine(
      new_vertices,PolyLine_ptr,
      PolyLine_ptr->get_permanent_color(),
      PolyLine_ptr->get_selected_color());
}


// ---------------------------------------------------------------------
void PolyLinesGroup::rescale(PolyLine* PolyLine_ptr,double dy)
{
//   cout << "inside PolyLinesGroup::rescale(), dy = " << dy << endl;

   double scalefactor=1-dy;

   polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
   threevector COM=polyline_ptr->compute_vertices_COM();
   
   vector<threevector> rescaled_vertices;
   for (unsigned int i=0; i<polyline_ptr->get_n_vertices(); i++)
   {
      threevector vertex=polyline_ptr->get_vertex(i);
      double r=(vertex-COM).magnitude();
      threevector r_hat=(vertex-COM).unitvector();
      rescaled_vertices.push_back(COM+scalefactor*r*r_hat);
   }
   
   bool display_vertices_flag=true;
   PolyLine_ptr=regenerate_PolyLine(
      rescaled_vertices,PolyLine_ptr,
      PolyLine_ptr->get_permanent_color(),
      PolyLine_ptr->get_selected_color(),display_vertices_flag,
      PolyLine_ptr->get_ID());
   set_selected_Graphical_ID(PolyLine_ptr->get_ID());
}

// ---------------------------------------------------------------------
void PolyLinesGroup::rotate(PolyLine* PolyLine_ptr,double dy)
{
//   cout << "inside PolyLinesGroup::rotate(), dy = " << dy << endl;

   const double conversion_factor=0.5;
   double theta=conversion_factor*dy;

   polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
   threevector COM=polyline_ptr->compute_vertices_COM();
   
   vector<threevector> rotated_vertices;
   for (unsigned int i=0; i<polyline_ptr->get_n_vertices(); i++)
   {
      threevector vertex=polyline_ptr->get_vertex(i);
      double r=(vertex-COM).magnitude();
      threevector r_hat=(vertex-COM).unitvector();
      threevector r_hat_new(
         cos(theta)*r_hat.get(0)-sin(theta)*r_hat.get(1),
         sin(theta)*r_hat.get(0)+cos(theta)*r_hat.get(1),
         r_hat.get(2));
      rotated_vertices.push_back(COM+r*r_hat_new);
   }
   
   bool display_vertices_flag=true;
   PolyLine_ptr=regenerate_PolyLine(
      rotated_vertices,PolyLine_ptr,
      PolyLine_ptr->get_permanent_color(),
      PolyLine_ptr->get_selected_color(),display_vertices_flag,
      PolyLine_ptr->get_ID());
   set_selected_Graphical_ID(PolyLine_ptr->get_ID());
}

// ==========================================================================
// PolyLine properties member functions
// ==========================================================================

// Member function set_width resets the width of all polylines within
// the current PolyLinesGroup.  This method may be called before or
// after the polylines have been instantiated.

void PolyLinesGroup::set_width(double width)
{   
//   cout << "inside PolyLinesGroup::set_width(), width = " << width << endl;
   linewidth_refptr = new osg::LineWidth();
   linewidth_refptr->setWidth(width);
   osg::StateSet* stateset_ptr=get_OSGgroup_ptr()->getOrCreateStateSet();
   stateset_ptr->setAttributeAndModes(linewidth_refptr.get(),
                                      osg::StateAttribute::ON);
}

double PolyLinesGroup::get_width() const
{
   if (linewidth_refptr.valid())
   {
      return linewidth_refptr->getWidth();
   }
   else
   {
      return 1.0;
   }
}

// --------------------------------------------------------------------------
// Member function adjust_depth_buffering loops over all PolyLines
// within the Group and resets their depth buffering based upon input
// boolean force_display_flag.

// Note: We could not get this method to work reliably when we wrote
// it on 12/30/06.  We definitely do not understand enough about
// StateSets in order to use them robustly...

void PolyLinesGroup::adjust_depth_buffering(bool force_display_flag)
{   
   cout << "inside PolyLinesGroup::adjust_depth_buffering()" << endl;
   cout << "force_display_flag = " << force_display_flag << endl;
   
/*
   osg::StateSet* stateset_ptr=get_OSGgroup_ptr()->getOrCreateStateSet();
   if (force_display_flag)
   {
      cout << "depth_off_refptr = " << depth_on_refptr << endl;
      stateset_ptr->setAttributeAndModes(
         depth_off_refptr,osg::StateAttribute::ON);
   }
   else
   {
      stateset_ptr->setAttributeAndModes(
         depth_off_refptr.get(),osg::StateAttribute::OFF);
//      stateset_ptr->setAttributeAndModes(
//         depth_on_refptr.get(),osg::StateAttribute::ON);
   }
*/

/*
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      PolyLine* PolyLine_ptr=get_PolyLine_ptr(n);
      PolyLine_ptr->adjust_depth_buffering(
         force_display_flag,PolyLine_ptr->get_geode_ptr());
   } // loop over index n labeling PolyLines in Group

   cout << "at end of PolyLinesGroup::adjust_depth_buffering()" << endl;
*/

}

// ---------------------------------------------------------------------
void PolyLinesGroup::set_uniform_color(colorfunc::Color uniform_color)
{
   set_uniform_color(colorfunc::get_OSG_color(uniform_color));
}

void PolyLinesGroup::set_uniform_color(const osg::Vec4& uniform_color)
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      PolyLine* curr_PolyLine_ptr=get_PolyLine_ptr(n);
      curr_PolyLine_ptr->set_permanent_color(uniform_color);
      curr_PolyLine_ptr->set_color(uniform_color);
      for (unsigned int t=0; t<curr_PolyLine_ptr->get_n_text_messages(); t++)
      {
         curr_PolyLine_ptr->set_text_color(t,uniform_color);
      } // loop over index t labeling text messages
   } // loop over index n labeling PolyLines;
}

// ---------------------------------------------------------------------
// This next overloaded version of set_uniform_color only colors those
// PolyLines belonging to the OSGsubPAT corresponding to the input
// OSGsubPAT_number argument.  This previous member function is NOT
// trivially related to this one in the case where multiple PolyLines
// are attached as bunched geometries to a single OSGsubPAT.

void PolyLinesGroup::set_uniform_color(
   const osg::Vec4& uniform_color,int OSGsubPAT_number)
{
//   cout << "inside PolyLinesGroup::set_uniform_color, uniform_color = " 
//        << endl;
//   cout << "get_n_OSGsubPATs() = " << get_n_OSGsubPATs() << endl;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      PolyLine* curr_PolyLine_ptr=get_PolyLine_ptr(n);
      if (OSGsubPAT_parent_of_Graphical(curr_PolyLine_ptr)==OSGsubPAT_number)
      {
         curr_PolyLine_ptr->set_color(uniform_color);
         for (unsigned int t=0; t<curr_PolyLine_ptr->get_n_text_messages(); 
              t++)
         {
            curr_PolyLine_ptr->set_text_color(t,uniform_color);
         } // loop over index t labeling text messages
      }
      else
      {
         cout << "OSGsubPAT_parent_of_Graphical() = "
              << OSGsubPAT_parent_of_Graphical(curr_PolyLine_ptr) << endl;
         cout << "OSGsubPAT_number = " << OSGsubPAT_number << endl;
         outputfunc::enter_continue_char();
      }
   } // loop over index n labeling PolyLines;
}

// --------------------------------------------------------------------------
// Member function recolor_based_upon_zcolormap_dependent_var() resets
// the PolyLinesGroup uniform coloring based upon the current
// dependent variable for *z_colorMap_ptr.  We wrote this auxilliary
// method in May 2009 in order to recolor national borders for the
// line-of-sight program to orange whenever the grey-scale map is
// displayed.

void PolyLinesGroup::recolor_based_on_zcolormap_dependent_var()
{
   if (DataGraph_ptr==NULL) return;
   ColorMap* z_ColorMap_ptr=DataGraph_ptr->get_z_ColorMap_ptr();
   if (z_ColorMap_ptr==NULL) return;
   
   int n_depend_var=z_ColorMap_ptr->get_dependent_var();

   colorfunc::Color original_color=colorfunc::white;
//   colorfunc::Color contrasting_color=colorfunc::pink;
   colorfunc::Color contrasting_color=colorfunc::blue;
//   colorfunc::Color contrasting_color=colorfunc::orange;

   if (n_depend_var==3)
   {
      set_uniform_color(colorfunc::get_OSG_color(contrasting_color));
   }
   else
   {
      set_uniform_color(colorfunc::get_OSG_color(original_color));
   }
}

// ==========================================================================
// Update member functions
// ==========================================================================

// Member function compute_altitude_dependent_text_label_sizes
// introduces virtual camera altitude dependence into the display of
// Region of Interest (ROI) text labels for the Bluegrass demo.  In
// particular, we do not want moving vehicles within the CH video to
// be obscured by text labels when the user zooms in towards the
// bottom of a ROI polyhedron.  So if the virtual camera's altitude is
// less than some min_eye_altitude, we drastically shrink the size of
// all ROI PolyLine text labels so that they effectively disappear.
// When the virtual camera rises above the min_eye_altitude ceiling,
// we restore the text labels' sizes to their original values.

void PolyLinesGroup::compute_altitude_dependent_text_label_sizes()
{
//   cout << "inside PolyLinesGroup::compute_altitude_dependent_text_label_sizes()" << endl;
   
   if (!altitude_dependent_labels_flag) return;
   if (get_CM_3D_ptr()==NULL) return;
   
   double curr_altitude=
      get_CM_3D_ptr()->get_eye_world_posn().get(2)
      -get_grid_world_origin_ptr()->get(2);
//   cout << "curr alt = " << curr_altitude << endl;

   const double min_eye_altitude=750;  // meters
//   const double min_eye_altitude=1000;  // meters
   double mag_factor=1000.0;
   if (curr_altitude < min_eye_altitude)
   {
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         PolyLine* curr_PolyLine_ptr=get_PolyLine_ptr(n);
         if (curr_PolyLine_ptr->get_text_refptr_valid(0) &&
             curr_PolyLine_ptr->get_n_text_messages() > 0 &&
             !curr_PolyLine_ptr->get_shrunken_text_label_flag())
         {
            curr_PolyLine_ptr->set_text_size(
               0,curr_PolyLine_ptr->get_text_size(0)/mag_factor);
            curr_PolyLine_ptr->set_shrunken_text_label_flag(true);
         }
      }
   } 
   else if (curr_altitude >= min_eye_altitude)
   {
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         PolyLine* curr_PolyLine_ptr=get_PolyLine_ptr(n);

         if (curr_PolyLine_ptr->get_text_refptr_valid(0) &&
             curr_PolyLine_ptr->get_n_text_messages() > 0 &&
             !curr_PolyLine_ptr->get_shrunken_text_label_flag())
         {
            curr_PolyLine_ptr->set_text_size(
               0,curr_PolyLine_ptr->get_text_size(0)*mag_factor);
            curr_PolyLine_ptr->set_shrunken_text_label_flag(false);
         }
      }
   }
}

// --------------------------------------------------------------------------
// Member function update_display()

void PolyLinesGroup::update_display()
{
//   cout << "inside PolyLinesGroup::update_display()" << endl;
//   cout << "n_PolyLines = " << get_n_Graphicals() << endl;

   parse_latest_messages();

// Call reset_colors to enable PolyLine blinking:

   recolor_based_on_zcolormap_dependent_var();
   reset_colors();

   vector<PolyLine*> PolyLine_ptrs=get_all_PolyLine_ptrs();
   for (unsigned int n=0; n<PolyLine_ptrs.size(); n++)
   {
      osgGeometry::PointsGroup* PointsGroup_ptr=
         PolyLine_ptrs[n]->get_PointsGroup_ptr();
//      cout << "n = " << n << " PointsGroup_ptr = " << PointsGroup_ptr << endl;
      if (PointsGroup_ptr != NULL)
      {
         PointsGroup_ptr->update_display();
      }
      
      ConesGroup* ConesGroup_ptr=PolyLine_ptrs[n]->get_ConesGroup_ptr();
      if (ConesGroup_ptr != NULL)
      {
         ConesGroup_ptr->update_display();
      }
   }

// Display intersection points if calculated:

   if (Intersection_PointsGroup_ptr != NULL)
   {
      Intersection_PointsGroup_ptr->update_display();
   }

   compute_altitude_dependent_text_label_sizes();
   
/*
   if (drape_PolyLines_across_terrain_flag)
   {

   }
*/
 

   GraphicalsGroup::update_display();
}

// ==========================================================================
// PolyLine properties member functions
// ==========================================================================

// Member function compute_vertices_average computes the average of
// all vertices within input STL vector V.

threevector PolyLinesGroup::compute_vertices_average(
   const vector<threevector>& V)
{
//   cout << "inside PolyLinesGroup::compute_vertices_average()" << endl;
   
   threevector vertices_average(0,0,0);
   for (unsigned int n=0; n<V.size(); n++)
   {
//      cout << "n = " << n << " V[n] = " << V[n] << endl;
      vertices_average += V[n];
   }
   if (V.size() < 1)
   {
      cout << "Error in PolyLinesGroup::set_vertices_average()" << endl;
      cout << "V.size() = " << V.size() << endl;
      exit(-1);
   }
   
   vertices_average /= double(V.size());
   return vertices_average;
}

// ==========================================================================
// PolyLine intersection member functions
// ==========================================================================

// Member function find_intersection_points() performs a brute-force
// search over all pairs of PolyLines for points of intersection.  It
// then instantiates member *Intersection_PointsGroup_ptr if it
// doesn't already exist.  Finally, this method generates an
// intersection Point for display.

vector<threevector> PolyLinesGroup::find_intersection_points(
   bool northern_hemisphere_flag,int UTM_zone,
   double endpoint_distance_threshold,double max_dotproduct)
{
   cout << "inside PolyLinesGroup::find_intersection_points()" << endl;
   vector<threevector> intersection_points;

   string intersections_filename="intersections.dat";
   ofstream outstream;
   outstream.precision(12);
   filefunc::openfile(intersections_filename,outstream);

   int n_edges=0;
   for (unsigned int i=0; i<get_n_Graphicals(); i++)
   {
      cout << i << " " << flush;
//      cout << "i = " << i << endl;
      PolyLine* this_PolyLine_ptr=get_PolyLine_ptr(i);
      polyline* this_polyline_ptr=
         this_PolyLine_ptr->get_or_set_polyline_ptr();
      n_edges += this_polyline_ptr->get_n_edges();
//      cout << "*this_polyline_ptr = " << *this_polyline_ptr << endl;
      for (unsigned int j=i+1; j<get_n_Graphicals(); j++)
      {
//         cout << "j = " << j << endl;
         PolyLine* that_PolyLine_ptr=get_PolyLine_ptr(j);
         polyline* that_polyline_ptr=
            that_PolyLine_ptr->get_or_set_polyline_ptr();
//         cout << "*that_polyline_ptr = " << *that_polyline_ptr << endl;

         vector<threevector> curr_intersection_points=
            this_polyline_ptr->intersection_points_with_another_polyline(
               *that_polyline_ptr,max_dotproduct,endpoint_distance_threshold);
         for (unsigned int v=0; v<curr_intersection_points.size(); v++)
         {
            intersection_points.push_back(curr_intersection_points[v]);
            geopoint curr_geopoint(
               northern_hemisphere_flag,UTM_zone,
               intersection_points.back().get(0),
               intersection_points.back().get(1));

            outstream << curr_geopoint.get_longitude() << " , "
                      << curr_geopoint.get_latitude() << endl;
            
//            outstream << intersection_points.size() << " , "
//                      << intersection_points.back().get(0) << " , "
//                      << intersection_points.back().get(1) << " , "
//                      << endl;
         }
      } // loop over index j labeling PolyLines
   } // loop over index i labeling PolyLines
   cout << endl;

   filefunc::closefile(intersections_filename,outstream);

//   cout << "intersection points = " << endl;
//   templatefunc::printVector(intersection_points);
   cout << "Total # polyline edges = " << n_edges << endl;
   cout << "Total # polyline intersection points = " 
        << intersection_points.size() << endl;

   if (Intersection_PointsGroup_ptr==NULL)
   {
      Intersection_PointsGroup_ptr=new osgGeometry::PointsGroup(
         get_ndims(),get_pass_ptr(),get_grid_world_origin_ptr());

// Add Intersection_PointsGroup to OSGsubPAT:
      
      int OSGsubPAT_number=0;
      insert_OSGgroup_into_OSGsubPAT(
         Intersection_PointsGroup_ptr->get_OSGgroup_ptr(),OSGsubPAT_number);
   }

   for (unsigned int i=0; i<intersection_points.size(); i++)
   {
      osgGeometry::Point* Intersection_Point_ptr=
         Intersection_PointsGroup_ptr->generate_new_Point(
            intersection_points[i],false);
      Intersection_Point_ptr->set_crosshairs_color(
         colorfunc::get_OSG_color(colorfunc::red));
   }

   return intersection_points;
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

bool PolyLinesGroup::parse_next_message_in_queue(message& curr_message)
{
//   cout << "inside PolyLinesGroup::parse_next_message_in_queue()" << endl;
//   cout << "curr_message.get_text_message() = "
//        << curr_message.get_text_message() << endl;

   bool message_handled_flag=false;
   if (curr_message.get_text_message()=="SELECT_VERTEX")
   {
//      cout << "Received SELECT_VERTEX message from ActiveMQ" << endl;
      curr_message.extract_and_store_property_keys_and_values();
      string type=curr_message.get_property_value("TYPE");
//      cout << "type = " << type << endl;

      if (type=="VEHICLE")
      {
         string ID=curr_message.get_property_value("ID");  
         int track_label_ID=stringfunc::string_to_integer(ID);
         int PolyLine_ID=find_PolyLine_ID_given_track_label(track_label_ID);
         PolyLine* PolyLine_ptr=get_ID_labeled_PolyLine_ptr(PolyLine_ID);
         if (PolyLine_ptr != NULL)
         {
            mover* mover_ptr=PolyLine_ptr->get_mover_ptr();
            if (mover_ptr != NULL)
            {
               if (mover_ptr->get_MoverType()==mover::VEHICLE)
               {
                  blink_Geometrical(PolyLine_ID);
               }
            } // mover_ptr != NULL conditional
         } // PolyLine_ptr != NULL conditional
      }
      else if (type=="ROI")
      {
         string ID=curr_message.get_property_value("ID");  
         int ROI_label_ID=stringfunc::string_to_integer(ID);
         int PolyLine_ID=2*ROI_label_ID;

         PolyLine* PolyLine_ptr=get_ID_labeled_PolyLine_ptr(PolyLine_ID);
         if (PolyLine_ptr != NULL)
         {
            mover* mover_ptr=PolyLine_ptr->get_mover_ptr();
            if (mover_ptr != NULL)
            {
               if (mover_ptr->get_MoverType()==mover::ROI)
               {
                  vector<int> PolyLineIDs;
                  PolyLineIDs.push_back(PolyLine_ID);
                  PolyLineIDs.push_back(PolyLine_ID+1);

                  double ROI_max_blink_period=5;	// secs
                  if (blink_Geometricals(PolyLineIDs,ROI_max_blink_period))
                  {
                     get_ID_labeled_PolyLine_ptr(PolyLineIDs[0])->
                        set_blinking_color(colorfunc::black);
                     get_ID_labeled_PolyLine_ptr(PolyLineIDs[1])->
                        set_blinking_color(colorfunc::black);
                  } // blink_Geometricals conditional
               } // mover==ROI conditional
            } // mover_ptr != NULL conditional
         } // PolyLine_ptr != NULl conditional
      } // type conditional
      message_handled_flag=true;
   }

// Note: As of early Aug 2008, Michael Yee's GraphExplorer program
// seems to be broadcasting DELETE_EDGE rather than DELETE_VERTEX when
// the DELETE_NODE button on his tool is pressed.  So the following
// section no longer works and needs to be significantly reworked...

   else if (curr_message.get_text_message()=="DELETE_VERTEX")
   {
//      cout << "Received DELETE_VERTEX message from ActiveMQ" << endl;
      curr_message.extract_and_store_property_keys_and_values();
      string type=curr_message.get_property_value("TYPE");
      if (type=="VEHICLE")
      {
         string ID=curr_message.get_property_value("ID");  
         int track_label_ID=stringfunc::string_to_integer(ID);
         int PolyLine_ID=find_PolyLine_ID_given_track_label(track_label_ID);
//         cout << "track_label_ID = " << track_label_ID
//              << " PolyLine_ID = " << PolyLine_ID << endl;
         PolyLine* PolyLine_ptr=get_ID_labeled_PolyLine_ptr(PolyLine_ID);
         if (PolyLine_ptr != NULL)
         {
            mover* mover_ptr=PolyLine_ptr->get_mover_ptr();
            if (mover_ptr != NULL)
            {
               if (mover_ptr->get_MoverType()==mover::VEHICLE)
               {
                  destroy_PolyLine(PolyLine_ID);
                  
               }
            } // mover_ptr != NULL conditional
         } // PolyLine_ptr != NULL conditional
      }
      else if (type=="ROI")
      {
         string ID=curr_message.get_property_value("ID");  
         int ROI_ID=stringfunc::string_to_integer(ID);
         int PolyLine_ID=2*ROI_ID;
//         cout << "ROI_ID = " << ROI_ID
//              << " PolyLine_ID = " << PolyLine_ID << endl;

         PolyLine* PolyLine_ptr=get_ID_labeled_PolyLine_ptr(PolyLine_ID);
//         cout << "get_ID_labeled_PolyLine_ptr(PolyLine_ID) = "
//              << get_ID_labeled_PolyLine_ptr(PolyLine_ID) << endl;
         
         if (PolyLine_ptr != NULL)
         {
            mover* mover_ptr=PolyLine_ptr->get_mover_ptr();
//            cout << "mover_ptr = " << mover_ptr << endl;
            if (mover_ptr != NULL)
            {
//               cout << "mover type = " << mover_ptr->get_MoverType()
//                    << " mover::ROI = " << mover::ROI << endl;
               if (mover_ptr->get_MoverType()==mover::ROI)
               {
//                  cout << "Before call to destroy_PolyLine(PolyLine_ID)"
//                       << endl;
                  destroy_PolyLine(PolyLine_ID);
                  destroy_PolyLine(PolyLine_ID+1);
               }
            } // mover_ptr != NULL conditional
         } // PolyLine_ptr != NULL conditional
      } // type conditional
      message_handled_flag=true;
//   }
/*
   else if (curr_message.get_text_message()=="ADD_UAV")
   {
      curr_message.extract_and_store_property_keys_and_values();
      string longitude_str=curr_message.get_property_value("LONGITUDE");
      string latitude_str=curr_message.get_property_value("LATITUDE");
      string altitude_str=curr_message.get_property_value("ALTITUDE");
      double UAV_longitude=stringfunc::string_to_number(longitude_str);
      double UAV_latitude=stringfunc::string_to_number(latitude_str);
      geopoint UAV_geopoint(UAV_longitude,UAV_latitude);
      double UAV_easting=UAV_geopoint.get_UTM_easting();
      double UAV_northing=UAV_geopoint.get_UTM_northing();

      cout << "UAV_easting = " << UAV_easting 
           << " UAV_northing = " << UAV_northing << endl;
      message_handled_flag=true;
*/

   } // curr_message.get_text_message() conditional

   return message_handled_flag;
}

// --------------------------------------------------------------------------
// Member function find_PolyLine_ID_given_track_label takes in integer
// track_label.  It performs a brute force search over all PolyLines
// associated with tracks.  If some PolyLine's track label matches the
// input track_label, this method returns the PolyLine's ID.
// Otherwise, it returns -1.

int PolyLinesGroup::find_PolyLine_ID_given_track_label(int track_label)
{
//   cout << "inside PolyLinesGroup::find_PolyLine_ID_given_track_label()" 
//        << endl;

   for (unsigned int p=0; p<get_n_Graphicals(); p++)
   {
      PolyLine* curr_PolyLine_ptr=get_PolyLine_ptr(p);
      track* curr_track_ptr=curr_PolyLine_ptr->get_track_ptr();
      if (curr_track_ptr != NULL)
      {
         string curr_track_label=curr_track_ptr->get_label();
         int curr_track_ID=stringfunc::string_to_integer(
            stringfunc::suffix(curr_track_label,"V"));
         if (curr_track_ID==track_label)
         {
            return curr_PolyLine_ptr->get_ID();
         }
      }
   } // loop over index p labeling PolyLines
   return -1;
}

// --------------------------------------------------------------------------
// Member function issue_add_vertex_message() retrieves the last mover
// placed onto the movers_group queue.  After sending out an ActiveMQ
// message for that mover, this method pops the mover off the
// movers_group queue.

void PolyLinesGroup::issue_add_vertex_message(
   PolyLine* bottom_PolyLine_ptr,colorfunc::Color mover_color,
   string annotation_label)
{   
//   cout << "inside PolyLinesGroup::issue_add_vertex_message()" << endl;
//   cout << "movers_group_ptr = " << movers_group_ptr << endl;
   if (movers_group_ptr != NULL)
   {
      vector<mover*> movers_queue=movers_group_ptr->get_movers_queue();
//      cout << "movers_queue.size() = " << movers_queue.size() << endl;
      if (movers_queue.size() > 0)
      {
         mover* last_mover_ptr=movers_queue.back();
         last_mover_ptr->set_RGB_color(
            colorfunc::get_RGB_values(mover_color));
//         cout << "annotation_label = " << annotation_label << endl;
         if (annotation_label.size() > 0)
         {
            last_mover_ptr->set_annotation_label(annotation_label);
         }

// Compute bounding box for bottom face and its area.  Then form
// relative size metric for ROI based upon sqrt(bbox_area) to typical
// ROI characterisic length scale of 65 meters:

//         polyline* polyline_ptr=bottom_PolyLine_ptr->
//            get_or_set_polyline_ptr();
//         double relative_ROI_size=mover_func::compute_relative_ROI_size(
//            polyline_ptr);

// FAKE FAKE:  Thurs, July 24 at 5 pm...
// Comment out relative ROI size based upon geometrical size of ROI...

//         last_mover_ptr->set_relative_size(relative_ROI_size);

         movers_group_ptr->issue_add_vertex_message(last_mover_ptr);
         movers_group_ptr->get_movers_queue().pop_back();
      }
   } // movers_group_ptr != NULL conditional
}

// ==========================================================================
// Text label member functions
// ==========================================================================

// Member function reset_labels() loops over all PolyLines.  It
// regenerates either their ID or length labels based upon
// get_ID_labels_flag().

void PolyLinesGroup::reset_labels()
{
//   cout << "inside PolyLinesGroup::reset_labels()" << endl;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      PolyLine* PolyLine_ptr=get_PolyLine_ptr(n);

      if (get_ID_labels_flag())
      {
         PolyLine_ptr->generate_PolyLine_ID_label();
      }
      else
      {
         PolyLine_ptr->generate_PolyLine_length_label();
      }
   } // loop over index n labeling PolyLines in Group
}

// ==========================================================================
// Ascii feature file I/O methods
// ==========================================================================

// Member function save_info_to_file() generates an ascii text file
// with PolyLine ID, passnumber, vertex and color information.  It
// returns the name of the generated text file.

bool PolyLinesGroup::export_info_to_file()
{
   unsigned int n_PolyLines=get_n_Graphicals();
   if (n_PolyLines==0) 
   {
      return false;
   }
   else
   {
      save_info_to_file();
      return true;
   }
}

string PolyLinesGroup::save_info_to_file()
{
   string output_filename="polylines_"+stringfunc::number_to_string(
      get_ndims())+"D.txt";
   return save_info_to_file(output_filename);
}

string PolyLinesGroup::save_info_to_file(string output_filename)
{
   cout << "inside PolyLinesGroup::save_info_to_file()" << endl;
//   cout << "this = " << this << endl;
//   cout << "n_PolyLines = " << get_n_Graphicals() << endl;

   ofstream outstream;

   filefunc::openfile(output_filename,outstream);

   if (get_ndims()==2)
   {
      outstream << "# Time   PolyLine_ID   Passnumber   X Y R G B A"
                << endl << endl;
      outstream.precision(6);
   }
   else if (get_ndims()==3)
   {
      outstream << "# Time   PolyLine_ID   Passnumber   X Y Z R G B A"
                << endl << endl;
      outstream.precision(12);
   }

   for (unsigned int p=0; p<get_n_Graphicals(); p++)
   {
      PolyLine* PolyLine_ptr=get_PolyLine_ptr(p);
      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
//      cout << "*polyline_ptr = " << *polyline_ptr << endl;

      for (unsigned int t=get_first_framenumber(); t<=get_last_framenumber(); 
           t++)
      {
         if(PolyLine_ptr->get_mask(t,get_passnumber())) continue;

         for (unsigned int i=0; i<polyline_ptr->get_n_vertices(); i++)
         {
            threevector curr_vertex=polyline_ptr->get_vertex(i);
            outstream << t << "  ";
            outstream << PolyLine_ptr->get_ID() << "  ";
            outstream << get_passnumber() << "  ";

            if (get_ndims()==2)
            {
               outstream << curr_vertex.get(0) << "  "
                         << curr_vertex.get(1) << "  ";
            }
            else if (get_ndims()==3)
            {
               outstream << curr_vertex.get(0) << "  "
                         << curr_vertex.get(1) << "  "
                         << curr_vertex.get(2) << "  ";
            }
            osg::Vec4 permanent_color=PolyLine_ptr->get_permanent_color();
            outstream << permanent_color.r() << "  "
                      << permanent_color.g() << "  "
                      << permanent_color. b() << "  "
                      << permanent_color.a() << endl;
         } // loop over index i labeling polyline vertices
      } // loop index t labeling frame numbers
   } // loop over index p labeling PolyLines
   filefunc::closefile(output_filename,outstream);

   string PolyLine_export_filename=filefunc::get_pwd()+output_filename;
   cout << "PolyLine_export_filename = " << PolyLine_export_filename
        << endl;

   string banner="PolyLine exported to "+PolyLine_export_filename;
   outputfunc::write_banner(banner);
   return PolyLine_export_filename;
}

// --------------------------------------------------------------------------
// Member function read_info_from_file parses the ascii text file
// generated by member function save_info_to_file().  After purging
// the PolyLineslist, this method regenerates the PolyLines within the
// list based upon the ascii text file information.  This boolean
// member function returns false if it cannot successfully parse the
// input ascii file.

bool PolyLinesGroup::read_info_from_file(
   string polylines_filename,vector<double>& curr_time,
   vector<int>& polyline_ID,vector<int>& pass_number,
   vector<threevector>& V,vector<osg::Vec4>& color)
{
   cout << "inside PolyLinesGroup::read_info_from_file()" << endl;
   
   if (!filefunc::ReadInfile(polylines_filename))
   {
      cout << "Trouble in PolyLinesGroup::read_info_from_file()"
           << endl;
      cout << "Couldn't open polylines_filename = " << polylines_filename
           << endl;
      return false;
   }

   int nlines=filefunc::text_line.size();
   curr_time.reserve(nlines);
   pass_number.reserve(nlines);
   V.reserve(nlines);
   color.reserve(nlines);

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << "i = " << i
//           << " text_line = " << filefunc::text_line[i] << endl;
      vector<double> X=stringfunc::string_to_numbers(filefunc::text_line[i]);
      curr_time.push_back(X[0]);
      polyline_ID.push_back(basic_math::round(X[1]));
      pass_number.push_back(basic_math::round(X[2]));

      if (get_ndims()==2)
      {
         V.push_back(threevector(X[3],X[4]));
         color.push_back(osg::Vec4(X[5],X[6],X[7],X[8]));
      }
      else if (get_ndims()==3)
      {
         threevector curr_vertex(X[3],X[4],X[5]);
         V.push_back(curr_vertex);
         color.push_back(osg::Vec4(X[6],X[7],X[8],X[9]));
      }
   } // loop over index i labeling ascii file line number
   
   for (unsigned int i=0; i<curr_time.size(); i++)
   {
      cout << "time = " << curr_time[i]
           << " ID = " << polyline_ID[i]
           << " pass = " << pass_number[i]
           << " V = " << V[i].get(0) << "," << V[i].get(1) 
           << "," << V[i].get(2)
           << endl;
   }
   return true;
}

// --------------------------------------------------------------------------
// Member function reconstruct_polylines_from_file_info parses the
// polyline information written to an ascii file by member function
// save_info_to_file().  It then destroys all existing polylines and
// instantiates a new set using the input information.  This boolean
// method returns false if it cannot successfully reconstruct
// polylines from the input file information.

string PolyLinesGroup::reconstruct_polylines_from_file_info()
{
   cout << "inside PolyLinesGroup::reconstruct_polylines_from_file_info()"
        << endl;
   string polylines_filename="polylines_"+stringfunc::number_to_string(
      get_ndims())+"D.txt";
   return reconstruct_polylines_from_file_info(polylines_filename);
}

string PolyLinesGroup::reconstruct_polylines_from_file_info(
   string polylines_filename)
{
   cout << "inside PolyLinesGroup::reconstruct_polylines_from_file_info()"
        << endl;
   cout << "polylines_filename = " << polylines_filename << endl;

   vector<double> curr_time;
   vector<int> polyline_ID,pass_number;
   vector<threevector> V,curr_V;
   vector<osg::Vec4> color,curr_color;

   if (!read_info_from_file(
      polylines_filename,curr_time,polyline_ID,pass_number,V,color))
   {
      return "";
   }
   
   threevector global_vertex_avg=Zero_vector;
   if (get_ndims()==3) global_vertex_avg=compute_vertices_average(V);
//      cout << "global_vertex_avg = " << global_vertex_avg << endl;

// Destroy all existing PolyLines before creating a new PolyLine list
// from the input ascii file:

   destroy_all_Graphicals();

   int curr_polyline_ID=polyline_ID[0];
   int curr_t = curr_time[0];
   for (unsigned int i=0; i<polyline_ID.size(); i++)
   {
      cout << "i = " << i 
           << " curr_time = " << curr_time[i]
           << " polyline_ID[i] = " << polyline_ID[i] 
           << " curr_polyline_ID = " << curr_polyline_ID << endl;
      
      if (polyline_ID[i] == curr_polyline_ID)
      {
         curr_V.push_back(V[i]);
         curr_color.push_back(color[i]);
      }

      if (polyline_ID[i] != curr_polyline_ID || i==polyline_ID.size()-1 ||
          curr_time[i] != curr_t)
      {
         get_AnimationController_ptr()->set_curr_framenumber(curr_t);

// First instantiate current PolyLine corresponding to curr_polyline_ID:
            
         PolyLine* curr_PolyLine_ptr=NULL;
         
         if (curr_V.size() >= 2)
         {
            bool force_display_flag=false;
            bool single_polyline_per_geode_flag=false;
            int n_text_messages=1;
            curr_PolyLine_ptr=generate_new_PolyLine(
               global_vertex_avg,curr_V,curr_color,force_display_flag,
               single_polyline_per_geode_flag,n_text_messages,
               curr_polyline_ID);

// Note added on 10/3/12: We shouldn't need to add the following line.
// But the previous line is NOT correctly setting PolyLine permanent
// colors!

            curr_PolyLine_ptr->set_permanent_color(curr_color.back());

//            bool display_vertices_flag=true;               
            bool display_vertices_flag=false;
            curr_PolyLine_ptr=regenerate_PolyLine(
               curr_V,curr_PolyLine_ptr,
               curr_PolyLine_ptr->get_permanent_color(),
               curr_PolyLine_ptr->get_selected_color(),
               display_vertices_flag);

/*
// For viewgraph generation purposes, turn off vertices and change
// line segment colors:

bool display_vertices_flag=false;
curr_PolyLine_ptr=regenerate_PolyLine(
curr_V,curr_PolyLine_ptr,
colorfunc::get_OSG_color(colorfunc::red),
colorfunc::get_OSG_color(colorfunc::red),
display_vertices_flag);
*/

         } // curr_V.size() >= 2 conditional

         if (ID_labels_flag && curr_PolyLine_ptr != NULL)
         {
            curr_PolyLine_ptr->generate_PolyLine_ID_label();
         }

         if (get_ndims()==2)
         {
            curr_PolyLine_ptr->get_polyline_ptr()->
               compute_first_edge_2D_line_coeffs();
         }
            
// Then clear out vertex and color STL vectors.  Refresh them with
// latest PolyLineID, vertex and color information read in from text
// file:

         curr_V.clear();
         curr_color.clear();

         curr_polyline_ID=polyline_ID[i];
         curr_t = curr_time[i];
         curr_V.push_back(V[i]);
         curr_color.push_back(color[i]);

         cout << "polyline_ID = " << curr_polyline_ID
              << " i = " << i 
              << " curr_V = " << curr_V.back() << endl;
      }
      
   } // loop over index i labeling entries in polyline_ID STL vector
   cout << endl;

// Bunch together multiple geometries within GeometricalsGroup's single
// geode.  Then attach that single geode to GeometricalGroup's single
// PAT.  Finally, add GeometricalGroup's single PAT to PolyLinesGroup's
// OSGgroup:

   get_PAT_ptr()->addChild(get_geode_ptr());

   get_PAT_ptr()->setPosition(osg::Vec3d(
      global_vertex_avg.get(0),
      global_vertex_avg.get(1),
      global_vertex_avg.get(2)));

   get_OSGsubPAT_ptr(0)->addChild(get_PAT_ptr());
   get_OSGsubPAT_ptr(0)->setNodeMask(1);

   cout << endl;
   return filefunc::get_pwd()+polylines_filename;
}

// --------------------------------------------------------------------------
// Member function assign_heights_using_pointcloud

void PolyLinesGroup::assign_heights_using_pointcloud(PointFinder* PF_ptr)
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      PolyLine* PolyLine_ptr=get_PolyLine_ptr(n);
      PolyLine_ptr->assign_heights_using_pointcloud(PF_ptr);
   } // loop over index n labeling PolyLines in Group
}

// ==========================================================================
// PolyLine video plane projection member functions
// ==========================================================================
  
// Member function
// project_PolyLines_into_selected_aerial_video_frame() generates a 2D
// imageplane PolyLine if it doesn't already exists as a counterpart
// for any 3D PolyLine within *this.  It then loops over every 3D
// PolyLine and projects it into the 2D imageplane for the selected
// aerial video frame.  If the UV coords for the
// projected PolyLine's tip lies inside the movie's allowed UV range,
// this method sets the imageplane PolyLines's UV coordinates so that it 
// appears within the movie's viewport.  Otherwise, the imageplane
// PolyLine is masked.

bool PolyLinesGroup::project_PolyLines_into_selected_aerial_video_frame(
   double minU,double maxU,double minV,double maxV)
{
//   cout << "inside PolyLinesGroup::project_PolyLines_into_selected_aerial_video_frame()"
//        << endl;

   if (imageplane_PolyLinesGroup_ptr==NULL || photogroup_ptr==NULL) 
      return false;

   int curr_framenumber=AnimationController_ptr->get_curr_framenumber();
   if (curr_framenumber==prev_framenumber)
   {
      return false;
   }
   else
   {
      prev_framenumber=curr_framenumber;
   }
   
   int photo_ID=photogroup_ptr->get_selected_photo_ID();
   if (photo_ID < 0) photo_ID=0;
//   cout << "photo_ID = " << photo_ID << endl;

   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(photo_ID);

   unsigned int n_3D_PolyLines=get_n_Graphicals();
   if (n_3D_PolyLines==0) return false;

   camera* camera_ptr=photograph_ptr->get_camera_ptr();
//   cout << "*camera_ptr = " << *camera_ptr << endl;

   imageplane_PolyLinesGroup_ptr->erase_all_Graphicals();

   double curr_t=imageplane_PolyLinesGroup_ptr->get_curr_t();
   int pass_number=imageplane_PolyLinesGroup_ptr->get_passnumber();

// Loop over every 3D PolyLine and project it into the 2D imageplane
// for *Movie_ptr.  If the UV coords for the PolyLine's tip lies
// inside the movie's allowed UV range, set UV coordinates for
// corresponding imageplane PolyLine.

   unsigned int n_visible_PolyLines=0;
   for (unsigned int s=0; s<n_3D_PolyLines; s++)
   {
//      cout << s << " " << flush;
      PolyLine* PolyLine_ptr=get_PolyLine_ptr(s);
      int PolyLine_ID=PolyLine_ptr->get_ID();

// Make sure a 2D imageplane PolyLine with the same ID corresponds to
// every 3D PolyLine:

      PolyLine* imageplane_PolyLine_ptr=imageplane_PolyLinesGroup_ptr->
         get_ID_labeled_PolyLine_ptr(PolyLine_ID);
      if (imageplane_PolyLine_ptr==NULL)
      {
         bool force_display_flag=false;
         bool single_polyline_per_geode_flag=false;
         int n_text_messages=1;
         imageplane_PolyLine_ptr=imageplane_PolyLinesGroup_ptr->
            generate_new_PolyLine(
               force_display_flag,single_polyline_per_geode_flag,
               n_text_messages,PolyLine_ID);
         imageplane_PolyLine_ptr->set_stationary_Graphical_flag(true);
      } // imageplane_PolyLine_ptr==NULL conditional

      vector<bool> vertex_visibility_flags;
      vector<threevector> imageplane_vertices;
      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();

      for (unsigned int j=0; j<polyline_ptr->get_n_vertices(); j++)
      {
         threevector UVW=*(camera_ptr->get_P_ptr()) * 
            fourvector(polyline_ptr->get_vertex(j),1);
         double U=UVW.get(0)/UVW.get(2);
         double V=UVW.get(1)/UVW.get(2);
         imageplane_vertices.push_back(threevector(U,V,0));

         bool visible_region_flag=false;
         if (U >= minU && U <= maxU && V >= minV && V <= maxV)
         {
            visible_region_flag=true;
         }
         vertex_visibility_flags.push_back(visible_region_flag);
//         cout << "j = " << j << " u = " << U << " v = " << V << endl;
      } // loop over index v labeling polyline vertices

      vector<threevector> visible_imageplane_vertices;
      for (unsigned int j=0; j<imageplane_vertices.size(); j++)
      {
         bool curr_vertex_visible_flag=false;
         if (vertex_visibility_flags[j]) curr_vertex_visible_flag=true;

         if (j >= 1 && vertex_visibility_flags[j-1])
            curr_vertex_visible_flag=true;

         if (j <= imageplane_vertices.size()-2 && 
             vertex_visibility_flags[j+1])  curr_vertex_visible_flag=true;

         if (curr_vertex_visible_flag) 
            visible_imageplane_vertices.push_back(imageplane_vertices[j]);
      }

      if (visible_imageplane_vertices.size() < 2)
      {
         continue;
      }
      else
      {
         bool display_vertices_flag=false;
         imageplane_PolyLine_ptr=imageplane_PolyLinesGroup_ptr->
            regenerate_PolyLine(
               visible_imageplane_vertices,imageplane_PolyLine_ptr,
               colorfunc::get_OSG_color(colorfunc::red),
               colorfunc::get_OSG_color(colorfunc::purple),
               display_vertices_flag);
         imageplane_PolyLine_ptr->generate_PolyLine_label(
            PolyLine_ptr->get_text_label());
         imageplane_PolyLine_ptr->set_text_size(0,0.01);
         imageplane_PolyLine_ptr->set_mask(curr_t,pass_number,false);
         n_visible_PolyLines++;
      }
      
   } // loop over index s labeling PolyLines
   cout << "n_visible_PolyLines = " << n_visible_PolyLines << endl;
   
   return true;
} 

// --------------------------------------------------------------------------
// Member function convert_polylines_to_polygons()

void PolyLinesGroup::convert_polylines_to_polygons()
{
   cout << "inside PolyLinesGrop::convert_polylines_to_polygons()" << endl;
   unsigned int n_PolyLines=get_n_Graphicals();
//   cout << "n_PolyLines = " << n_PolyLines << endl;

   double length_scalefactor = 20 / 0.0861676;  // ft per dimensionless unit
//   double length_scalefactor = 20.5 / 0.0861676;  // ft per dimensionless unit

   double total_area_in_sqr_ft = 0;
   double total_area_in_sqr_yds = 0;
   for (unsigned int p=0; p<n_PolyLines; p++)
   {
      PolyLine* PolyLine_ptr=get_PolyLine_ptr(p);
      PolyLine_ptr->set_linewidth(5);
//      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
//      double total_length = length_scalefactor * 
//         polyline_ptr->get_total_length();
//      cout << "p = " << p 
//           << " polyline length = " << total_length << endl;

      polygon* polygon_ptr=PolyLine_ptr->construct_polygon();
      double area=sqr(length_scalefactor) * polygon_ptr->get_area();
      double area_in_sqr_yds = area / 9.0;
      total_area_in_sqr_ft += area;
      total_area_in_sqr_yds += area_in_sqr_yds;
      double perim=length_scalefactor * polygon_ptr->compute_perimeter();
      cout << "Bed: " << p 
           << " Area/ft**2 = " << area 
           << " Perimeter length / ft = " << perim
           << endl;
   } // loop over index p labeling PolyLines

   cout << "Total bed area in sqr ft = " << total_area_in_sqr_ft << endl;
   cout << "Total bedarea in sqr yds = " << total_area_in_sqr_yds << endl;

// Assume 1/2" thickness for mulch:

   double delta_z = 0.5 / 36;	// yd
   double total_mulch_vol = total_area_in_sqr_yds * delta_z;	// yd**3
   cout 
      << "Total mulch volume in cubic yards (assuming 1/2 inch thickness) = " 
      << total_mulch_vol << endl;

// Garage door opening = 20.5'   
   }

// --------------------------------------------------------------------------
string PolyLinesGroup::get_image_ID_str()
{
//   cout << "inside PolyLinesGroup::get_image_ID_str()" << endl;
   
   string currimage_filename = get_AnimationController_ptr()->
      get_curr_image_filename();
//   cout << "currimage_filename = " << currimage_filename << endl;

   vector<string> substrings = stringfunc::decompose_string_into_substrings(
      currimage_filename,"_.");
   string image_ID_str = substrings[1];
//   cout << "image_ID_str = " << image_ID_str << endl;
   return image_ID_str;
}

// --------------------------------------------------------------------------
void PolyLinesGroup::set_selected_bbox()
{
//   cout << "inside PolyLinesGroup::set_selected_bbox()" << endl;
//   cout << "image_ID_str = " << get_image_ID_str() << endl;

   annotated_bboxes_iter = annotated_bboxes_map_ptr->find(get_image_ID_str());
   if(annotated_bboxes_iter == annotated_bboxes_map_ptr->end())
   {
      return;
   }
   
   vector<bounding_box>* curr_bboxes_ptr = &annotated_bboxes_iter->second;
   if(curr_bboxes_ptr == NULL) return;
   if(curr_bboxes_ptr->size() == 0) return;

   int selected_PolyLine_ID = 
      curr_bboxes_ptr->at(currimage_PolyLine_index).get_ID();
   set_selected_Graphical_ID(selected_PolyLine_ID);
}

// --------------------------------------------------------------------------
bounding_box* PolyLinesGroup::get_selected_bbox()
{
   annotated_bboxes_iter = annotated_bboxes_map_ptr->find(get_image_ID_str());
   vector<bounding_box>* curr_bboxes_ptr = &annotated_bboxes_iter->second;
   bounding_box* selected_bbox_ptr = &curr_bboxes_ptr->at(
      currimage_PolyLine_index);
   return selected_bbox_ptr;
}

// --------------------------------------------------------------------------
double PolyLinesGroup::get_currimage_frame_diag()
{
   image_sizes_iter = image_sizes_map_ptr->find(get_image_ID_str());
   int curr_xdim = image_sizes_iter->second.first;
   int curr_ydim = image_sizes_iter->second.second;
   double curr_diag = sqrt(curr_xdim * curr_ydim);
   return curr_diag;
}

// --------------------------------------------------------------------------
// Member function increment_frame()

void PolyLinesGroup::increment_frame()
{
//  cout << "inside PolyLinesGrop::increment_frame()" << endl;

   double curr_diag = get_currimage_frame_diag();
   get_AnimationController_ptr()->increment_frame_counter();
   double next_diag = get_currimage_frame_diag();

   generate_image_bboxes(get_image_ID_str());
   currimage_PolyLine_index = 0;
   set_selected_bbox();

   double scalefactor = next_diag / curr_diag;
   get_CM_2D_ptr()->set_eye_to_center_distance(
      get_CM_2D_ptr()->get_eye_to_center_distance() * scalefactor);
}

// --------------------------------------------------------------------------
// Member function decrement_frame()

void PolyLinesGroup::decrement_frame()
{
//   cout << "inside PolyLinesGrop::decrement_frame()" << endl;

   double curr_diag = get_currimage_frame_diag();
   get_AnimationController_ptr()->decrement_frame_counter();
   double next_diag = get_currimage_frame_diag();

   generate_image_bboxes(get_image_ID_str());
   currimage_PolyLine_index = 0;
   set_selected_bbox();

   double scalefactor = next_diag / curr_diag;
   get_CM_2D_ptr()->set_eye_to_center_distance(
      get_CM_2D_ptr()->get_eye_to_center_distance() * scalefactor);
}

// --------------------------------------------------------------------------
// Member function jump_forward_frame()

void PolyLinesGroup::jump_forward_frame(int jump)
{
//   cout << "inside PolyLinesGrop::jump_forward_frame()" << endl;
   int orig_frame_skip = get_AnimationController_ptr()->get_frame_skip();
   get_AnimationController_ptr()->set_frame_skip(jump);
   get_AnimationController_ptr()->increment_frame_counter();
   get_AnimationController_ptr()->set_frame_skip(orig_frame_skip);
   generate_image_bboxes(get_image_ID_str());
   currimage_PolyLine_index = 0;
   set_selected_bbox();
}

// --------------------------------------------------------------------------
// Member function jump_backward_frame()

void PolyLinesGroup::jump_backward_frame(int jump)
{
//   cout << "inside PolyLinesGrop::jump_backward_frame()" << endl;
   int orig_frame_skip = get_AnimationController_ptr()->get_frame_skip();
   get_AnimationController_ptr()->set_frame_skip(jump);
   get_AnimationController_ptr()->decrement_frame_counter();
   get_AnimationController_ptr()->set_frame_skip(orig_frame_skip);
   generate_image_bboxes(get_image_ID_str());
   currimage_PolyLine_index = 0;
   set_selected_bbox();
}

// --------------------------------------------------------------------------
// Member function increment_currimage_PolyLine()

void PolyLinesGroup::increment_currimage_PolyLine()
{
//   cout << "inside PolyLinesGrop::increment_currimage_PolyLine()" << endl;
   currimage_PolyLine_index++;

   annotated_bboxes_iter = annotated_bboxes_map_ptr->find(
      get_image_ID_str());
   vector<bounding_box> curr_bboxes = annotated_bboxes_iter->second;
   if(currimage_PolyLine_index >= int(curr_bboxes.size()))
   {
      currimage_PolyLine_index -= curr_bboxes.size();
   }
   set_selected_bbox();
   bounding_box selected_bbox = curr_bboxes[currimage_PolyLine_index];
   cout << attribute_key << " = "  
        << selected_bbox.get_attribute_value(attribute_key) << endl;
}

// --------------------------------------------------------------------------
// Member function decrement_currimage_PolyLine()

void PolyLinesGroup::decrement_currimage_PolyLine()
{
//   cout << "inside PolyLinesGrop::decrement_currimage_PolyLine()" << endl;
   currimage_PolyLine_index--;

   annotated_bboxes_iter = annotated_bboxes_map_ptr->find(
      get_image_ID_str());
   vector<bounding_box> curr_bboxes = annotated_bboxes_iter->second;
   if(currimage_PolyLine_index < 0)
   {
      currimage_PolyLine_index += curr_bboxes.size();
   }
   set_selected_bbox();
   bounding_box selected_bbox = curr_bboxes[currimage_PolyLine_index];
   cout << attribute_key << " = "  
        << selected_bbox.get_attribute_value(attribute_key) << endl;
}

// --------------------------------------------------------------------------
// Member function set_PolyLine_attribute()

void PolyLinesGroup::set_PolyLine_attribute(int attribute_ID)
{
//   cout << "inside set_PolyLine_attribute(), attribute_ID = " << attribute_ID
//        << endl;

   int selected_PolyLine_ID = get_selected_Graphical_ID();
   osg_bboxes_iter = osg_bboxes_map_ptr->find(selected_PolyLine_ID);
   string image_ID_str = osg_bboxes_iter->second.first;
   currimage_PolyLine_index = osg_bboxes_iter->second.second;
   
   annotated_bboxes_iter = annotated_bboxes_map_ptr->find(get_image_ID_str());
   vector<bounding_box>* curr_bboxes_ptr = &annotated_bboxes_iter->second;
   bounding_box* selected_bbox_ptr = &curr_bboxes_ptr->at(
      currimage_PolyLine_index);

   if(selected_bbox_ptr->get_label() != "face") return;

   attribute_key = "gender";
   string attribute_value = "unset";
   if(attribute_ID == 0)
   {
      attribute_value = "unknown";
   }
   else if(attribute_ID == 1)
   {
      attribute_value = "male";
   }
   else if(attribute_ID == 2)
   {
      attribute_value = "female";
   }

   if(attribute_value != "unset")
   {
//      cout << "Attribute: key = " << attribute_key 
//           << "  value = " << attribute_value << endl;
      selected_bbox_ptr->set_attribute_value(attribute_key, attribute_value);
      display_PolyLine_attribute(selected_PolyLine_ID, attribute_value);
   }
   write_bboxes_to_file();
}

// --------------------------------------------------------------------------
// Member function set_all_PolyLine_attributes()

void PolyLinesGroup::set_all_PolyLine_attributes(int attribute_ID)
{
//   cout << "inside PG::set_all_PolyLine_attributes()" << endl;

   annotated_bboxes_iter = annotated_bboxes_map_ptr->find(get_image_ID_str());
   vector<bounding_box>* curr_bboxes_ptr = &annotated_bboxes_iter->second;
   
   for(unsigned int b = 0; b < curr_bboxes_ptr->size(); b++)
   {
      bounding_box* bbox_ptr = &curr_bboxes_ptr->at(b);
      if(bbox_ptr->get_label() != "face") return;

      attribute_key = "gender";
      string attribute_value = "unset";
      if(attribute_ID == 0)
      {
         attribute_value = "unknown";
      }
      else if(attribute_ID == 1)
      {
         attribute_value = "male";
      }
      else if(attribute_ID == 2)
      {
         attribute_value = "female";
      }
      
      if(attribute_value != "unset")
      {
         bbox_ptr->set_attribute_value(attribute_key, attribute_value);
         display_PolyLine_attribute(bbox_ptr->get_ID(), attribute_value);
      }
   } // loop over index b labeling bboxes for curr image

   write_bboxes_to_file();
}

// --------------------------------------------------------------------------
// Member function display_PolyLine_attribute()

void PolyLinesGroup::display_PolyLine_attribute(
   int PolyLine_ID, string attribute_value)
{
   osg_bboxes_iter = osg_bboxes_map_ptr->find(PolyLine_ID);
   if(osg_bboxes_iter == osg_bboxes_map_ptr->end()) return;

   string image_ID_str = osg_bboxes_iter->second.first;
   currimage_PolyLine_index = osg_bboxes_iter->second.second;
   
   annotated_bboxes_iter = annotated_bboxes_map_ptr->find(get_image_ID_str());
   vector<bounding_box>* curr_bboxes_ptr = &annotated_bboxes_iter->second;
   bounding_box* selected_bbox_ptr = &curr_bboxes_ptr->at(
      currimage_PolyLine_index);

   if(selected_bbox_ptr->get_label() != "face") return;

   PolyLine* PolyLine_ptr = get_PolyLine_ptr(PolyLine_ID);
   polygon poly(*PolyLine_ptr->get_polyline_ptr());
   double polygon_area=poly.compute_area();
   double text_size=20.0*sqrt(polygon_area)/100.0;
   
   threevector text_posn = 
      PolyLine_ptr->get_polyline_ptr()->get_vertex(0);
   PolyLine_ptr->set_label(attribute_value, text_posn, text_size);
}

// --------------------------------------------------------------------------
// Member function generate_image_bboxes() loops over all bboxes for
// image specified by its image_ID_str.  It converts dimensionful bbox
// pixel coordinates into dimensionless (u,v) coordinates.  It then
// convert (u,v) coordinates into corresponding (U,V) coordinates
// corresponding to maximal bbox width and height.

void PolyLinesGroup::generate_image_bboxes(string image_ID_str)
{
//   cout << "inside PolyLinesGroup::generate_image_bboxes()" << endl;
//   cout << "image_ID_str = " << image_ID_str << endl;

   int curr_width, curr_height;
   image_sizes_iter = image_sizes_map_ptr->find(image_ID_str);
   curr_width = image_sizes_iter->second.first;
   curr_height = image_sizes_iter->second.second;

   annotated_bboxes_iter = annotated_bboxes_map_ptr->find(image_ID_str);
   vector<bounding_box>* curr_bboxes_ptr = &annotated_bboxes_iter->second;

   destroy_all_PolyLines();

   for(unsigned int b = 0; b < curr_bboxes_ptr->size(); b++)
   {

// First check if a PolyLine corresponding to curr_bbox[b] has already
// been generated. If so, curr_bbox should have non-negative ID:

//       if(curr_bboxes_ptr->at(b).get_ID() >= 0) continue;
      
      double ulo=curr_bboxes_ptr->at(b).get_xmin()/curr_height;
      double uhi=curr_bboxes_ptr->at(b).get_xmax()/curr_height;
      double vlo=1 - curr_bboxes_ptr->at(b).get_ymin()/curr_height;
      double vhi=1 - curr_bboxes_ptr->at(b).get_ymax()/curr_height;
      
      double alpha = 0.5 * double(max_image_width-curr_width)/
         max_image_height;
      double beta = 0.5 * double(max_image_height-curr_height)/
         max_image_height;
      double gamma = double(curr_height)/max_image_height;
         
      double Ulo = alpha + gamma * ulo;
      double Uhi = alpha + gamma * uhi;
      double Vlo = beta + gamma * vlo;
      double Vhi = beta + gamma * vhi;

      vector<threevector> bbox_vertices;
      bbox_vertices.push_back(threevector(Ulo,Vlo));
      bbox_vertices.push_back(threevector(Uhi,Vlo));
      bbox_vertices.push_back(threevector(Uhi,Vhi));
      bbox_vertices.push_back(threevector(Ulo,Vhi));
      bbox_vertices.push_back(threevector(Ulo,Vlo));
      osg::Vec4 uniform_color=colorfunc::get_OSG_color(
         curr_bboxes_ptr->at(b).get_color());

      bool force_display_flag = false;
      bool single_polyline_per_geode_flag = true;
      int n_text_messages = 1;

      PolyLine* bbox_PolyLine_ptr = 
         generate_new_PolyLine(
            bbox_vertices, uniform_color, force_display_flag, 
            single_polyline_per_geode_flag, n_text_messages);
      int PolyLine_ID = bbox_PolyLine_ptr->get_ID();

// Tie together IDs for bounding boxes and their corresponding
// OSG PolyLines:

      curr_bboxes_ptr->at(b).set_ID(PolyLine_ID);

// Store association between OSG PolyLine ID and (image_ID_str, b)
// pair:

      pair<string, int> P;
      P.first = image_ID_str;
      P.second = b;
      (*osg_bboxes_map_ptr)[PolyLine_ID] = P;

      attribute_key = "gender";
      string attribute_value = curr_bboxes_ptr->at(b).
         get_attribute_value(attribute_key);
         
      if(attribute_value.size() > 0)
      {
         display_PolyLine_attribute(PolyLine_ID, attribute_value);
      }
   } // loop over index b labeling bboxes for curr_image
}


// --------------------------------------------------------------------------
// Member function write_bboxes_to_file()

void PolyLinesGroup::write_bboxes_to_file()
{
//    cout << "inside write_bboxes_to_file()" << endl;
   
   int curr_framenumber = get_AnimationController_ptr()->
      get_curr_framenumber();
   
// Export face and hand bounding boxes to output text file:

   string output_filename="./labeled_bboxes_"+
      stringfunc::number_to_string(secs_since_Y2K)+".txt";
   ofstream outstream;
//   cout << "output_filename = " << output_filename << endl;
   filefunc::openfile(output_filename, outstream);
   outstream << "# " << timefunc::getcurrdate() << endl;
   outstream << "# Image: index  ID_str " << endl;
   outstream << "# Bbox_ID  label  xmin  xmax  ymin ymax (attr_key attr_val)"
             << endl << endl;

   for(int i = get_AnimationController_ptr()->get_first_framenumber(); 
       i <= get_AnimationController_ptr()->get_last_framenumber(); i++)
   {
      get_AnimationController_ptr()->set_curr_framenumber(i);

      outstream << "Image: index = " << i 
                << " ID_str = " << get_image_ID_str() << endl;

      annotated_bboxes_iter = annotated_bboxes_map_ptr->find(
         get_image_ID_str());
      if(annotated_bboxes_iter == annotated_bboxes_map_ptr->end())
      {
         continue;
      }
      
      vector<bounding_box> curr_image_bboxes = annotated_bboxes_iter->second;

      for(unsigned int b = 0; b < curr_image_bboxes.size(); b++)
      {
         bounding_box curr_bbox = curr_image_bboxes[b];

         outstream << curr_bbox.get_ID() << "  "
                   << curr_bbox.get_label() << "   "
                   << curr_bbox.get_xmin() << "  "
                   << curr_bbox.get_xmax() << "  "
                   << curr_bbox.get_ymin() << "  "
                   << curr_bbox.get_ymax() << "  ";

         string attr_key, attr_value;
         for(curr_bbox.get_attributes_map_iter() = 
                curr_bbox.get_attributes_map().begin(); 
             curr_bbox.get_attributes_map_iter() != 
                curr_bbox.get_attributes_map().end();
             curr_bbox.get_attributes_map_iter()++)
         {
            attr_key = curr_bbox.get_attributes_map_iter()->first;
            attr_value = curr_bbox.get_attributes_map_iter()->second;
            
            outstream << attr_key << "  "  << attr_value << "  ";
         }

         outstream << endl;
      } // loop over index b labeling bboxes for current image
   } // loop over index i labeling all input images

// Reset AC's current frame number to its original value:

   get_AnimationController_ptr()->set_curr_framenumber(curr_framenumber);

   string unix_cmd="cp "+output_filename+" "+bbox_labels_filename;
   sysfunc::unix_command(unix_cmd);
}

// --------------------------------------------------------------------------
// Member function change_label_size()

void PolyLinesGroup::change_label_size(double factor)
{
//   cout << "inside PolyLinesGroup::change_label_size(), factor = "
//        << factor << endl;

   for(unsigned int i = 0; i < get_n_Graphicals(); i++)
   {
      PolyLine* PolyLine_ptr = get_PolyLine_ptr(i);
      PolyLine_ptr->change_text_size(0, factor);
   }
}

   
