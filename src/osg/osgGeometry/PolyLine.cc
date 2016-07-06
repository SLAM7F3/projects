// ==========================================================================
// PolyLine class member function definitions
// ==========================================================================
// Last updated on 2/7/11; 5/16/11; 4/6/14; 4/26/14
// ==========================================================================

#include <iostream>
#include <string>
#include <osg/Geode>
#include <osg/StateAttribute>
#include "math/constant_vectors.h"

#include "passes/Pass.h"
#include "osg/osgGeometry/ConesGroup.h"
#include "templates/mytemplates.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "geometry/polygon.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLine.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PolyLine::initialize_member_objects()
{
   Graphical_name="PolyLine";
   entry_finished_flag=shrunken_text_label_flag=false;
   polyline_ptr=NULL;
   polygon_ptr=NULL;
   CM_ptr=NULL;
   PointsGroup_ptr=NULL;
   ConesGroup_ptr=NULL;

// FAKE FAKE:  Sat Sep 15, 2012 at 9:07 am
// Change permanent color from white to red for chart purposes only

   set_permanent_color(colorfunc::red);

//   set_permanent_color(colorfunc::white);
   set_blinking_color(colorfunc::black);
   track_ptr=NULL;
   mover_ptr=NULL;
   objects_inside_PolyLine_map_ptr=NULL;
}

void PolyLine::allocate_member_objects()
{
   geom_refptr=new osg::Geometry;
}		       

PolyLine::PolyLine():
   Geometrical()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

PolyLine::PolyLine(
   const int p_ndims,Pass* PI_ptr,
   threevector* GO_ptr,osgText::Font* f_ptr,int n_text_messages,int id,
   osgGA::CustomManipulator* CM_ptr):
   Geometrical(p_ndims,id)
{	
   initialize_member_objects();
   allocate_member_objects();
   font_refptr=f_ptr;
   this->n_text_messages=n_text_messages;

   PointsGroup_ptr=new osgGeometry::PointsGroup(p_ndims,PI_ptr,GO_ptr);
   this->CM_ptr=CM_ptr;
 
   if (p_ndims==3)
   {
      ConesGroup_ptr=new ConesGroup(PI_ptr,GO_ptr);
   }
}		       

PolyLine::PolyLine(
   const int p_ndims,Pass* PI_ptr,
   threevector* GO_ptr,const threevector& reference_origin,
   const vector<threevector>& V,osgText::Font* f_ptr,
   int n_text_messages,int id,osgGA::CustomManipulator* CM_ptr):
   Geometrical(p_ndims,id)
{	
//   cout << "inside PolyLine constructor #2" << endl;
   
   initialize_member_objects();
   allocate_member_objects();
   font_refptr=f_ptr;
   this->n_text_messages=n_text_messages;

   set_reference_origin(reference_origin);
   set_relative_vertices(V);

   PointsGroup_ptr=new osgGeometry::PointsGroup(p_ndims,PI_ptr,GO_ptr);
   this->CM_ptr=CM_ptr;

   if (p_ndims==3)
   {
      ConesGroup_ptr=new ConesGroup(PI_ptr,GO_ptr);
   }
}		       

PolyLine::~PolyLine()
{
//   cout << "inside PolyLine destructor" << endl;

   delete polyline_ptr;
   delete polygon_ptr;

   delete PointsGroup_ptr;
   delete ConesGroup_ptr;
   delete objects_inside_PolyLine_map_ptr;

//   cout << "depth_off_refptr.get() = " << depth_off_refptr.get() << endl;
//   cout << "depth_on_refptr.get() = " << depth_on_refptr.get() << endl;
//   cout << "linewidth_refptr.get() = " << linewidth_refptr.get() << endl;
//   cout << "geom_refptr.get() = " << geom_refptr.get() << endl;
//   cout << "geode_refptr.get() = " << geode_refptr.get() << endl;
   for (unsigned int i=0; i<text_refptr.size(); i++)
   {
//      cout << "i = " << i 
//           << " text_refptr[i].get() = " << text_refptr[i].get()
//           << " text_refptr[i]->referenceCount() = "
//           << text_refptr[i]->referenceCount() << endl;
   }

/*   
   cout << "depth_off_refptr->referenceCount() = "
        << depth_off_refptr->referenceCount() << endl;
   cout << "depth_on_refptr->referenceCount() = "
        << depth_on_refptr->referenceCount() << endl;
   if (linewidth_refptr.valid())
   {
      cout << "linewidth_refptr->referenceCount() = "
           << linewidth_refptr->referenceCount() << endl;
   }
*/

//   cout << "geom_refptr->referenceCount() = "
//        << geom_refptr->referenceCount() << endl;
//   cout << "geode_refptr->referenceCount() = "
//       << geode_refptr->referenceCount() << endl;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PolyLine& l)
{
   outstream << "inside PolyLine::operator<<" << endl;
   outstream << "ID = " << l.get_ID() << endl;
   for (unsigned int i=0; i<l.vertices_refptr->size(); i++)
   {
      outstream << "i = " << i 
                << " V = " << l.vertices_refptr->at(i).x()
                << "," << l.vertices_refptr->at(i).y() 
                << "," << l.vertices_refptr->at(i).z() << endl;
   }
   
   threevector UVW;
   l.get_UVW_coords(0,0,UVW);
   cout << "UVW = " << UVW << endl;
//   outstream << static_cast<const Geometrical&>(l) << endl;
   outstream << "n_text_messages = " << l.get_n_text_messages() << endl;
   return(outstream);
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

osgGeometry::PointsGroup* PolyLine::get_PointsGroup_ptr()
{
//   cout << "inside PolyLine::get_PointsGroup_ptr() " << endl;
   if (PointsGroup_ptr==NULL)
   {
      cout << "Danger in PolyLine::get_PointsGroup_ptr, ptr = NULL" << endl;
   }
   return PointsGroup_ptr;
}

const osgGeometry::PointsGroup* PolyLine::get_PointsGroup_ptr() const
{
//   cout << "inside const PolyLine::get_PointsGroup_ptr() const " << endl;
   if (PointsGroup_ptr==NULL)
   {
      cout << "Danger in PolyLine::get_PointsGroup_ptr, ptr = NULL" << endl;
   }
   return PointsGroup_ptr;
}

ConesGroup* PolyLine::get_ConesGroup_ptr()
{
   return ConesGroup_ptr;
}

const ConesGroup* PolyLine::get_ConesGroup_ptr() const
{
   return ConesGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function set_relative_vertices fills *vertices_refptr with
// relative vertex information with respect to reference_origin.  In
// November 2006, Ross Anderson observed that floating point errors in
// very large absolute vertex positions can lead to noticeable
// polyline flickering.  To avoid roundoff problems, we simply need to
// incorporate the global reference_origin translation within the
// PolyLine's PAT.

void PolyLine::set_relative_vertices(const vector<threevector>& V)
{
//   cout << "inside PolyLine::set_relative_vertices()" << endl;
//   cout << "reference_origin = " << reference_origin << endl;
//   cout << "ndims = " << get_ndims() << endl;

// First either clear & reserve or dynamically allocate
// vertices_refptr.get():

   if (vertices_refptr.valid())
   {
      vertices_refptr->clear();
   }
   else
   {
      vertices_refptr = new osg::Vec3Array;
   }
   vertices_refptr->reserve(V.size());

//   cout << "n_vertices = " <<get_n_vertices() << endl;

   for (unsigned int n=0; n<V.size(); n++)
   {
      threevector relative_V=V[n]-reference_origin;
//      cout << "n = " << n << " V[n] = " << V[n] << endl;
//      cout << "  rel_V = " << relative_V << endl;
      if (get_ndims()==2)
      {
         vertices_refptr->push_back(
            osg::Vec3f(relative_V.get(0),relative_V.get(2),
                       relative_V.get(1)));
      }
      else if (get_ndims()==3)
      {
         vertices_refptr->push_back(
            osg::Vec3f(relative_V.get(0),relative_V.get(1),
                       relative_V.get(2)));
      }

//      cout << "n = " << n
//           << " vertices_refptr->back() = " 
//           << vertices_refptr->back().x() << " , "
//           << vertices_refptr->back().y() << " , "
//           << vertices_refptr->back().z() << endl;

   } // loop over index n labeling vertices
}

// ---------------------------------------------------------------------
// Member function get_relative_vertices()

vector<threevector> PolyLine::get_relative_vertices()
{
//   cout << "inside PolyLine::get_relative_vertices()" << endl;
 
   vector<threevector> relative_V;
   for (unsigned int n=0; n<vertices_refptr->size(); n++)
   {
      threevector curr_relative_V(vertices_refptr->at(n));
      relative_V.push_back(curr_relative_V);
   } // loop over index n labeling vertices
   return relative_V;
}

const vector<threevector> PolyLine::get_relative_vertices() const
{
//   cout << "inside PolyLine::get_relative_vertices()" << endl;
 
   vector<threevector> relative_V;
   for (unsigned int n=0; n<vertices_refptr->size(); n++)
   {
      threevector curr_relative_V(vertices_refptr->at(n));
      relative_V.push_back(curr_relative_V);
   } // loop over index n labeling vertices
   return relative_V;
}

// ---------------------------------------------------------------------
void PolyLine::set_color(const osg::Vec4& color)
{
//   cout << "inside PolyLine::set_color(), ID = " << get_ID() << endl;
//   cout << "color = " << color.r() << " " << color.g() << " "
//        << color.b() << endl;

//   cout << "this = " << this << endl;
   if (!color_array_refptr.valid())
   {
      color_array_refptr = new osg::Vec4Array(1);
   }
   color_array_refptr->at(0)=color;
   geom_refptr->setColorArray(color_array_refptr.get());
   geom_refptr->setColorBinding(osg::Geometry::BIND_OVERALL);
}

void PolyLine::set_colors(const vector<osg::Vec4>& colors)
{
//   cout << "inside PolyLine::set_colors()" << endl;
//   cout << "input colors.size() = " << colors.size() << endl;

// First either clear or dynamically allocate
// color_array_refptr.get().  Then reserve n_vertices slots within
// *(color_array_refptr.get()):

   if (color_array_refptr.valid())
   {
      color_array_refptr->clear();
   }
   else
   {
      color_array_refptr = new osg::Vec4Array;
   }
   color_array_refptr->reserve(basic_math::max(1,get_n_vertices()));


   for (unsigned int n=0; n<colors.size(); n++)
   {
      color_array_refptr->push_back(colors[n]);
   }
   
   geom_refptr->setColorArray(color_array_refptr.get());
   geom_refptr->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
}

// ---------------------------------------------------------------------
// Member function compute_color_fading loops over all PolyLine
// vertices within *this.  At each point, it multiplies the value V
// for the track's HSV triple by a piecewise-linear decreasing
// function of cumulative polyline length.  We wrote this specialized
// method in Feb 2009 in order to fade down dynamic UAV tracks
// computed by Luca Bertucelli's Consensus Based Bundle Algorithm
// which are more trustworthy near the track's beginning than at its
// end.

void PolyLine::compute_color_fading(const colorfunc::RGB& polyline_RGB)
{
//   cout << "inside PolyLine::compute_color_fading()" << endl;

   get_or_set_polyline_ptr();
   double total_length=polyline_ptr->get_total_length();
//   cout << "total_length = " << total_length << endl;

   const double reliable_distance=10000;	// meters
   const double unreliable_distance=20000;	// meters
   double f_start=reliable_distance/total_length;
   double f_stop=unreliable_distance/total_length;
   f_start=basic_math::min(f_start,1.0);
   f_stop=basic_math::min(f_stop,1.0);
//   cout << "f_start = " << f_start << " f_stop = " << f_stop << endl;

   local_colors.clear();

   const double TINY=0.00001;
   for (unsigned int n=0; n<polyline_ptr->get_n_vertices(); n++)
   {
      threevector curr_vertex(polyline_ptr->get_vertex(n));
      double f=polyline_ptr->frac_distance_along_polyline(curr_vertex);
      f -= TINY;

      const double v_start=1.0;
      const double v_stop=0.33;
      double v_frac;
      if (f <= f_start)
      {
         v_frac=v_start;
      }
      else if (f > f_stop)
      {
         v_frac=v_stop;
      }
      else
      {
         v_frac=v_start+(f-f_start)/(f_stop-f_start)*(v_stop-v_start);
      }
         
      colorfunc::RGB line_RGB=polyline_RGB;
      colorfunc::HSV line_HSV=colorfunc::RGB_to_hsv(line_RGB);
      line_HSV.third *= v_frac;
      line_RGB=colorfunc::hsv_to_RGB(line_HSV);

//      cout << "n = " << n 
//           << " cum l = " << cumulative_lengths[n]
//           << " f = " << f
//           << " v_frac = " << v_frac << endl;
//           << " h = " << line_HSV.first
//           << " s = " << line_HSV.second
//           << " vorig = " << orig_line_HSV.third
//           << " v = " << line_HSV.third 
//           << " r = " << line_RGB.first
//           << " g = " << line_RGB.second 
//           << " b = " << line_RGB.third
//           << endl;

      double alpha=1;
      osg::Vec4 curr_color(
         line_RGB.first,line_RGB.second,line_RGB.third,alpha);
      local_colors.push_back(curr_color);
   } // loop over index n labeling track points
}

// ==========================================================================
// PolyLine manipulation member functions
// ==========================================================================

// Member function add_vertex_points instantiates and inserts new
// Points into the current PolyLine's PointsGroup at the locations
// specified by input STL vector V.

void PolyLine::add_vertex_points(
   const vector<threevector>& V,const osg::Vec4& color)
{   
//   cout << "inside PolyLine::add_vertex_points()" << endl;
//   cout << "V.size() = " << V.size() << endl;

   for (unsigned int p=0; p<V.size(); p++)
   {
      osgGeometry::Point* curr_Point_ptr=PointsGroup_ptr->generate_new_Point(
         V[p],false);
      PointsGroup_ptr->initialize_Graphical(V[p],curr_Point_ptr);
      PointsGroup_ptr->insert_graphical_PAT_into_OSGsubPAT(curr_Point_ptr,0);
      curr_Point_ptr->set_crosshairs_color(color);
   } // loop over index p labeling points
}

// ---------------------------------------------------------------------
// Member function get_vertex_Point()

osgGeometry::Point* PolyLine::get_vertex_Point(int n)
{
//   cout << "inside PolyLine::get_vertex_Point()" << endl;
   return PointsGroup_ptr->get_Point_ptr(n);
}

// ==========================================================================
// Geometry object construction member functions
// ==========================================================================

// Member function construct_polyline instantiates a polyline (not
// PolyLine!) object for geometry book-keeping purposes.  This method
// passes absolute rather than relative vertices to the polyline
// constructor.

polyline* PolyLine::get_or_set_polyline_ptr()
{
//   cout << "inside PolyLine::get_or_set_polyline_ptr()" << endl;
   if (polyline_ptr==NULL)
   {
      polyline_ptr=construct_polyline();
   }
   return polyline_ptr;
}

polyline* PolyLine::construct_polyline()
{
//   cout << "inside PolyLine::construct_polyline()" << endl;

   vector<threevector> V;
   for (int n=0; n<get_n_vertices(); n++)
   {
      threevector curr_vertex(vertices_refptr->at(n));
      if (get_ndims()==2)
      {
         double x=curr_vertex.get(0);
         double y=curr_vertex.get(2);
         double z=curr_vertex.get(1);
         curr_vertex=threevector(x,y,z);
      }
      V.push_back(reference_origin+curr_vertex);
   } // loop over index n labeling vertices

   delete polyline_ptr;
   polyline_ptr=new polyline(V);

   return polyline_ptr;
}

// ---------------------------------------------------------------------
// Member function construct_polygon instantiates a polygon (not
// Polygon!) object for geometry book-keeping purposes.  Recall that
// if ndim==2, vertices are arranged as V=(Vx,Vz,Vy) rather than
// V=(Vx,Vy,Vz) !

polygon* PolyLine::construct_polygon()
{
//   cout << "inside PolyLine::construct_polygon()" << endl;

   if (get_n_vertices() < 3) return NULL;

   vector<threevector> V;
   for (int n=0; n<get_n_vertices()-1; n++)
   {
      threevector curr_V=
         reference_origin+threevector(vertices_refptr->at(n));
      if (get_ndims()==2)
      {
         curr_V.put(1,curr_V.get(2));
         curr_V.put(2,0);
      }
      V.push_back(curr_V);
   } // loop over index n labeling vertices

   threevector last_vertex(vertices_refptr->at(get_n_vertices()-1));
   if (!V[0].nearly_equal(last_vertex))
   {
      threevector last_V=reference_origin+last_vertex;
      if (get_ndims()==2)
      {
         last_V.put(1,last_V.get(2));
         last_V.put(2,0);
      }
      V.push_back(last_V);
   }

   delete polygon_ptr;
   polygon_ptr=new polygon(V);
   return polygon_ptr;
}

// ---------------------------------------------------------------------
polygon* PolyLine::construct_relative_polygon()
{
//   cout << "inside PolyLine::construct_relative_polygon()" << endl;

   if (get_n_vertices() < 3) return NULL;

   vector<threevector> V;
   for (int n=0; n<get_n_vertices(); n++)
   {
      threevector curr_V=threevector(vertices_refptr->at(n));
      if (get_ndims()==2)
      {
         curr_V.put(1,curr_V.get(2));
         curr_V.put(2,0);
      }
      V.push_back(curr_V);
   } // loop over index n labeling vertices

   delete polygon_ptr;
   polygon_ptr=new polygon(V);
//   cout << "relative polygon = " << *polygon_ptr << endl;

   return polygon_ptr;
}

// ---------------------------------------------------------------------
void PolyLine::assign_heights_using_pointcloud(PointFinder* PF_ptr)
{
//   cout << "inside PolyLine::assign_heights_using_pointcloud()" << endl;
   
   for (unsigned int i=0; i<vertices_refptr->size(); i++)
   {
      double curr_Z;

      if (PF_ptr->find_altitude_given_easting_and_northing(
         reference_origin.get(0)+vertices_refptr->at(i).x(),
         reference_origin.get(1)+vertices_refptr->at(i).y(),curr_Z))
      {
         cout << "i = " << i << " curr_Z = " << curr_Z << endl;
         vertices_refptr->at(i)=osg::Vec3(
            vertices_refptr->at(i).x(),
            vertices_refptr->at(i).y(),curr_Z-reference_origin.get(2));
      }
   } // loop over index n labeling vertices
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_geode instantiates an osg::Geode
// containing a single PolyLine drawable.

osg::Geode* PolyLine::generate_drawable_geode(bool force_display_flag)
{
//   cout << "inside PolyLine::generate_drawable_geode()" << endl;
   
   geode_refptr = new osg::Geode();
   fill_drawable_geode(geode_refptr.get(),force_display_flag,true);
   return geode_refptr.get();
}

void PolyLine::fill_drawable_geode(
   osg::Geode* geode_ptr,bool force_display_flag,
   bool generate_drawable_geom_flag)
{
//   cout << "inside PolyLine::fill_drawable_geode(), get_n_vertices() = " 
//        << get_n_vertices() << endl;
//   cout << "geode_ptr = " << geode_ptr << endl;

// As of 1/3/11, we believe the following conditional is unnecessary.
// So far as we can tell, generate_drawable_geom_flag always equals
// true...

   if (generate_drawable_geom_flag)
   {
      geode_ptr->addDrawable(generate_drawable_geom());
   }
   else
   {
      cout << "inside PolyLine::fill_drawable_geode()" << endl;
      cout << "generate_drawable_geom_flag=FALSE !!!" << endl;
      outputfunc::enter_continue_char();
      drawarrays_refptr->set(
         osg::PrimitiveSet::LINE_STRIP,0,get_n_vertices());
   }

   if (text_refptr.size() >= get_n_text_messages() &&
       get_n_text_messages() > 0)
   {
      cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
      cout << "Trouble in PolyLine::fill_drawable_geode()" << endl;
            cout << "text_refptr.size() = " << text_refptr.size() << endl;
      cout << "get_n_text_messages() = " << get_n_text_messages() << endl;
      cout << "Latter should almost certainly exceed former !!!!" << endl;
      cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
      outputfunc::enter_continue_char();
   }

//   cout << "n_text_messages = " << get_n_text_messages() << endl;
   for (unsigned int i=0; i<get_n_text_messages(); i++)
   {
      generate_text(i,geode_ptr);
   }

   adjust_depth_buffering(force_display_flag,geode_ptr);
}

// ---------------------------------------------------------------------
// Member function generate_drawable_geom instantiates Graphical
// member object *geom_refptr which contains an OpenGL LINE_STRIP
// drawable.

osg::Geometry* PolyLine::generate_drawable_geom()
{
   drawarrays_refptr=new osg::DrawArrays(
      osg::PrimitiveSet::LINE_STRIP,0,get_n_vertices());
   geom_refptr->addPrimitiveSet(drawarrays_refptr.get());
   geom_refptr->setVertexArray(vertices_refptr.get());

   return geom_refptr.get();
}

// ---------------------------------------------------------------------
void PolyLine::set_linewidth(double width)
{
//   cout << "inside PolyLine::set_linewidth(), width = " << width << endl;
   if (!linewidth_refptr.valid())
   {
      linewidth_refptr = new osg::LineWidth();
      osg::StateSet* stateset_ptr=geom_refptr->getOrCreateStateSet();
      stateset_ptr->setAttributeAndModes(
         linewidth_refptr.get(),osg::StateAttribute::ON);
   }
   linewidth_refptr->setWidth(width);
}

// ---------------------------------------------------------------------
// Member function add_flow_direction_arrows takes in an STL vector
// containing fractional locations of arrow heads along the current
// PolyLine object.  It instantiates new Cone members of the
// PolyLine's ConesGroup member.  The size of the arrow heads is
// proportional to the input linewidth parameter.

void PolyLine::add_flow_direction_arrows(
   const vector<double>& arrow_posn_fracs,double linewidth)
{
//   cout << "inside PolyLine::add_flow_direction_arrows()" << endl;
//   cout << "linewidth = " << linewidth << endl;

   if (ConesGroup_ptr==NULL) return;

//   if (get_polyline_ptr()==NULL) construct_polyline();
//   double polyline_length=get_polyline_ptr()->compute_total_length();
//   cout << "polyline_length = " << polyline_length << endl;
   double radius=linewidth;

   for (unsigned int f=0; f<arrow_posn_fracs.size(); f++)
   {
      double frac=arrow_posn_fracs[f];
//      cout << "f = " << f << " arrow_posn_frac = " << frac << endl;

      threevector grid_origin=ConesGroup_ptr->get_grid_world_origin();
      threevector arrow_center_posn=get_polyline_ptr()->edge_point(frac)-
         grid_origin;
     
      threevector arrow_dir=get_polyline_ptr()->edge_direction(frac);
      threevector arrow_base=arrow_center_posn-2*radius*arrow_dir;
      threevector arrow_tip=arrow_center_posn+2*radius*arrow_dir;

      Cone* curr_Cone_ptr=ConesGroup_ptr->generate_new_Cone(
         arrow_tip,arrow_base,radius);
      curr_Cone_ptr->set_permanent_color(get_permanent_color());
      curr_Cone_ptr->set_blinking_color(get_blinking_color());
      curr_Cone_ptr->set_color(curr_Cone_ptr->get_permanent_color());
   } // loop over index f labeling arrow position fractions
}

// ---------------------------------------------------------------------
// This overloaded (and higher-level) version of member function
// add_flow_direction_arrows takes in a distance_between_arrows
// parameter and computes a reasonable set of arrow position fractions
// along the current PolyLine object.  It then calls the preceding
// version of add_flow_direction_arrows.

void PolyLine::add_flow_direction_arrows(
   double distance_between_arrows,double linewidth)
{
//   cout << "inside PolyLine::add_flow_direction_arrows(dist_between_arrows)" 
//        << endl;

   if (ConesGroup_ptr==NULL) return;

//   if (get_polyline_ptr()==NULL) construct_polyline();
   double polyline_length=get_polyline_ptr()->compute_total_length();
//   cout << "polyline_length = " << polyline_length << endl;
//   cout << "distance_between_arrows = " << distance_between_arrows
//        << endl;

// Note added on 4/26/14: We discovered the hard and painful way that
// n_arrows defined below can go negative!  So it must not be set
// equal to an unsigned int!

   int n_arrows=
      basic_math::mytruncate(polyline_length/distance_between_arrows)-1;
//   cout << "n_arrows = " << n_arrows << endl;
   if (n_arrows < 1) return;
   
   double frac_spacing=1.0/(n_arrows+1.0);
   vector<double> arrow_posn_fracs;
   for (int n=1; n<=n_arrows; n++)
   {
      arrow_posn_fracs.push_back(n*frac_spacing);
//      cout << "n = " << n
//           << " arrow posn frac = " << arrow_posn_fracs.back()
//           << endl;
   }

   add_flow_direction_arrows(arrow_posn_fracs,linewidth);
}

// ==========================================================================
// Text methods
// ==========================================================================

// Member function generate_text()

void PolyLine::generate_text(int i,osg::Geode* geode_ptr)
{
//   cout << "inside PolyLine::generate_text(), i = " << i << endl;
//   cout << "get_n_text_messages() = " << get_n_text_messages() << endl;
//   cout << "get_ndims() = " << get_ndims() << endl;

   osg::ref_ptr<osgText::Text> curr_text_refptr=new osgText::Text;
   text_refptr.push_back(curr_text_refptr);

   Geometrical::initialize_text(i);
   text_refptr[i]->setAlignment(osgText::Text::CENTER_BOTTOM);

   if (get_ndims()==2)
      text_refptr[i]->setAxisAlignment(osgText::Text::SCREEN);

   geode_ptr->addDrawable(text_refptr[i].get());
}

// ---------------------------------------------------------------------
// Member function set_label is a high-level method which calls lower-
// level methods for setting text position and size.

void PolyLine::set_label(string label,const threevector& label_posn,
                         double text_size)
{
//   cout << "inside PolyLine::set_label()" << endl;
//   cout << "label = " << label << " label_posn = " << label_posn
//        << " text_size = " << text_size << endl;
 
   set_text_label(0,label);
   set_text_posn(0,label_posn);
   set_text_size(0,text_size);
}

void PolyLine::set_label(string label,const threevector& label_posn,
                         const threevector& label_dir,double text_size)
{
   set_label(label,label_posn,text_size);
   set_text_direction(0,label_dir);
}

// ---------------------------------------------------------------------
void PolyLine::set_text_posn(int i,const threevector& posn)
{
//   cout << "inside PolyLine::set_text_posn, reference_origin = "
//        << reference_origin << endl;

   if (get_ndims()==2)
   {
      threevector new_posn(
         posn.get(0)-reference_origin.get(0),-0.01,
         posn.get(1)-reference_origin.get(1));
      Geometrical::set_text_posn(i,new_posn);
   }
   else if (get_ndims()==3)
   {
      Geometrical::set_text_posn(i,posn-reference_origin);
   }
}

// ---------------------------------------------------------------------
// Member function set_COM_label takes in a label string along with a
// height parameter.  We assume the PolyLine corresponds to a closed
// contour.  The label is positioned at the contour's center-of-mass,
// and its size is scaled based upon its approximate mean radius.

void PolyLine::set_COM_label(string label,double height_above_polyline)
{
//   cout << "inside PolyLine::set_COM_label() " << endl;

//   if (get_polyline_ptr()==NULL) construct_polyline();
   polygon poly(*get_polyline_ptr());
   threevector polygon_COM(poly.compute_COM());
   
   double max_vertex_altitude=NEGATIVEINFINITY;
   for (unsigned int v=0; v<poly.get_nvertices(); v++)
   {
      max_vertex_altitude=basic_math::max(max_vertex_altitude,
                              poly.get_vertex(v).get(2));
   }

   threevector label_posn( polygon_COM );
   label_posn.put(2,max_vertex_altitude+height_above_polyline);
//   cout << "label_posn = " << label_posn << endl;
   
// Parameters within next line were empirically set to give reasonable
// results for labeling Regions of Interest within 2008 Bluegrass
// demo:

   double polygon_area=poly.compute_area();
   double text_size=8.0*sqrt(polygon_area)/100.0;
   set_label(label,label_posn,text_size);
}

// ---------------------------------------------------------------------
// Member function generate_PolyLine_label() adds a the input text
// label to the PolyLine located at its center.

void PolyLine::generate_PolyLine_label(std::string label)
{
//   cout << "inside PolyLine::generate_PolyLine_label()" << endl;
//   cout << "label = " << label << endl;

   const double edge_frac=0.5;
   threevector label_posn(get_polyline_ptr()->edge_point(edge_frac));
//   cout << "label_posn = " << label_posn << endl;

// If necessary, modify label directions so that they can be easily
// read from left to right:

   threevector label_dir(get_polyline_ptr()->edge_direction(edge_frac));
   if (label_dir.get(0) < 0) label_dir=-label_dir;

   double text_size_prefactor=5;
   if (get_ndims()==2)
   {
      text_size_prefactor=1.5;
   }

   double PolyLine_text_size=text_size_prefactor*get_length_sizefactor();
//   cout << "PolyLine_text_size = " << PolyLine_text_size << endl;
   set_label(label,label_posn,label_dir,PolyLine_text_size);
}

// ---------------------------------------------------------------------
// Member function generate_PolyLine_ID_label() adds a text label to
// the PolyLine located at its center.  The ID text label is returned by
// this method.

string PolyLine::generate_PolyLine_ID_label()
{
//   cout << "inside PolyLine::generate_PolyLine_ID_label()" << endl;

   string ID_label=stringfunc::number_to_string(get_ID());
//   cout << "ID_label = " << ID_label << endl;

   generate_PolyLine_label(ID_label);

/*   
   const double edge_frac=0.5;
   threevector label_posn(get_polyline_ptr()->edge_point(edge_frac));
//   cout << "label_posn = " << label_posn << endl;

// If necessary, modify label directions so that they can be easily
// read from left to right:

   threevector label_dir(get_polyline_ptr()->edge_direction(edge_frac));
   if (label_dir.get(0) < 0) label_dir=-label_dir;

   double text_size_prefactor=5;
   if (get_ndims()==2)
   {
      text_size_prefactor=1.5;
   }

   double PolyLine_text_size=text_size_prefactor*get_length_sizefactor();
//   cout << "PolyLine_text_size = " << PolyLine_text_size << endl;
   set_label(ID_label,label_posn,label_dir,PolyLine_text_size);
*/

   return ID_label;
}

// ---------------------------------------------------------------------
// Member function generate_PolyLine_length_label() computes the
// length of the current polyline.  This method adds a text label to
// the PolyLine located at its center.  The text label is returned by
// this method.

string PolyLine::generate_PolyLine_length_label()
{
//   cout << "inside PolyLine::generate_PolyLine_length_label()" << endl;

   bool kms_units=false;
   double polyline_length=get_polyline_ptr()->get_total_length();
   if (polyline_length > 2000)
   {
      polyline_length *= 0.001;
      kms_units=true;
   }

   length_label=stringfunc::number_to_string(polyline_length,2);

   if (get_ndims()==3)
   {
      if (kms_units)
      {
         length_label += " km";
      }
      else
      {
         length_label += " m";
      }
   }
//   cout << "length_label = " << length_label << endl;

   generate_PolyLine_label(length_label);

/*
   const double edge_frac=0.5;
   threevector label_posn(get_polyline_ptr()->edge_point(edge_frac));

// If necessary, modify label directions so that they can be easily
// read from left to right:

   threevector label_dir(get_polyline_ptr()->edge_direction(edge_frac));
   if (label_dir.get(0) < 0) label_dir=-label_dir;


// Find some direction which is orthogonal to label_dir:

   vector<threevector> crossproducts;
   crossproducts.push_back(label_dir.unitvector().cross(x_hat));
   crossproducts.push_back(label_dir.unitvector().cross(y_hat));
   crossproducts.push_back(label_dir.unitvector().cross(z_hat));
   vector<int> crossproduct_labels;
   crossproduct_labels.push_back(0);
   crossproduct_labels.push_back(1);
   crossproduct_labels.push_back(2);
   vector<double> crossproduct_mags;
   crossproduct_mags.push_back(crossproducts[0].magnitude());
   crossproduct_mags.push_back(crossproducts[1].magnitude());
   crossproduct_mags.push_back(crossproducts[2].magnitude());
   templatefunc::Quicksort_descending(crossproduct_mags,crossproduct_labels);
   cout << "Largest crossproduct label = " << crossproduct_labels[0] << endl;
   cout << "  Crossproduct dir =  " 
        << crossproducts[crossproduct_labels[0]].unitvector() 
        << endl;

   cout << "Middle crossproduct label = " << crossproduct_labels[1] << endl;
   cout << "  Crossproduct dir =  " 
        << crossproducts[crossproduct_labels[1]].unitvector() 
        << endl;

   cout << "Smallest crossproduct label = " << crossproduct_labels[2] << endl;
   cout << "  Crossproduct dir =  " 
        << crossproducts[crossproduct_labels[2]].unitvector() 
        << endl;

   double PolyLine_text_size=5*get_length_sizefactor();
//   cout << "PolyLine_text_size = " << PolyLine_text_size << endl;
   set_label(length_label,label_posn,label_dir,PolyLine_text_size);
//   set_text_color(0,colorfunc::red);
*/

   return length_label;
}

// ---------------------------------------------------------------------
// Member function get_length_sizefactor() computes log10 of the
// current polyline's total length (measured in meters).  It returns
// scale factor s = 4**(log10(length)-2).  On 12/30/10, we empirically
// found that this scale factor yields reasonably sized cross hairs
// and PolyLine length labels for Polylines within selected wtihin our
// lowell.osga and RASR S3-314 ladar point clouds.

double PolyLine::get_length_sizefactor()
{
//   cout << "inside PolyLine::get_length_sizefactor()" << endl;
   double polyline_length=get_polyline_ptr()->get_total_length();
//   cout << "polyline_length = " << polyline_length << endl;
   double PolyLine_sizefactor=1;
   if (!nearly_equal(polyline_length,0))
   {
      double loglength=log10(polyline_length);
      PolyLine_sizefactor=pow(4,loglength-2);
   }

   return PolyLine_sizefactor;
}

// ==========================================================================
// Bbox member functions
// ==========================================================================

// Member function compute_bbox() computes and saves the XYZ bounding
// box enclosing the current PolyLine object.

bounding_box& PolyLine::compute_bbox()
{
//   cout << "inside PolyLine::compute_bbox()" << endl;
//   cout << "this = " << this << endl;
//   polyline* polyline_ptr=get_polyline_ptr();
//   cout << "polyline_ptr = " << polyline_ptr << endl;
//   cout << "*polyline_ptr = " << *polyline_ptr << endl;
   bbox=bounding_box(get_polyline_ptr());
//   cout << "bbox = " << bbox << endl;
   return bbox;
}

// ---------------------------------------------------------------------
void PolyLine::instantiate_objects_inside_PolyLine_map()
{
//   cout << "inside PolyLine::instantiate_objects_inside_PolyLine_map()"
//        << endl;
   if (objects_inside_PolyLine_map_ptr==NULL)
      objects_inside_PolyLine_map_ptr=new OBJECTS_INSIDE_MAP;
}




