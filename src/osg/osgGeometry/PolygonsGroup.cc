// ==========================================================================
// POLYGONSGROUP class member function definitions
// ==========================================================================
// Last modified on 10/4/09; 1/6/11; 6/28/12
// ==========================================================================

#include <iomanip>
#include <string>
#include <vector>
#include <osg/Geode>
#include "osg/osgGraphicals/AnimationController.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "osg/osgGeometry/PointsGroup.h"
#include "osg/osgGeometry/Polygon.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLine.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::setw;
using std::string;
using std::vector;

namespace osgGeometry
{

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

   void PolygonsGroup::allocate_member_objects()
      {
      }		       

   void PolygonsGroup::initialize_member_objects()
      {
         GraphicalsGroup_name="PolygonsGroup";
         size[2]=1.0;
//   size[3]=1.0;
         PolyLinesGroup_ptr=NULL;

         get_OSGgroup_ptr()->setUpdateCallback( 
            new AbstractOSGCallback<PolygonsGroup>(
               this, &PolygonsGroup::update_display));
      }		       

   PolygonsGroup::PolygonsGroup(
      const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
      threevector* GO_ptr):
      GeometricalsGroup(p_ndims,PI_ptr,AC_ptr,GO_ptr)
      {	
         initialize_member_objects();
         allocate_member_objects();
      }		       

   PolygonsGroup::~PolygonsGroup()
      {
      }

// ---------------------------------------------------------------------
// Overload << operator

   ostream& operator<< (ostream& outstream,const PolygonsGroup& P)
      {
         int node_counter=0;
         for (unsigned int n=0; n<P.get_n_Graphicals(); n++)
         {
            Polygon* Polygon_ptr=P.get_Polygon_ptr(n);
            outstream << "Polygon node # " << node_counter++ << endl;
            outstream << "Polygon = " << *Polygon_ptr << endl;
         }
         return(outstream);
      }

// ==========================================================================
// Polygon creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Polygon from all other graphical insertion
// and manipulation methods...

   Polygon* PolygonsGroup::generate_new_Polygon(
      const threevector& reference_origin,const polygon& poly,int ID)
      {
//         cout << "inside PolygonsGroup::generate_new_Polygon() #1" << endl;
//         cout << "Reference_origin = " << reference_origin << endl;

         if (ID==-1) ID=get_next_unused_ID();

         Polygon* curr_Polygon_ptr=new Polygon(
            get_ndims(),get_pass_ptr(),get_grid_world_origin_ptr(),
            reference_origin,poly,ID);

         GraphicalsGroup::insert_Graphical_into_list(curr_Polygon_ptr);
         initialize_Graphical(curr_Polygon_ptr);
         insert_graphical_PAT_into_OSGsubPAT(curr_Polygon_ptr,Unsigned_Zero);

         curr_Polygon_ptr->set_UVW_coords(
            get_curr_t(),get_passnumber(),reference_origin);

         osg::Geode* geode_ptr=curr_Polygon_ptr->generate_drawable_geode();
         curr_Polygon_ptr->set_relative_vertices();
         curr_Polygon_ptr->get_PAT_ptr()->addChild(geode_ptr);

         reset_colors();
         return curr_Polygon_ptr;
      }

// --------------------------------------------------------------------------
   Polygon* PolygonsGroup::generate_new_Polygon(PolyLine* PolyLine_ptr,int ID)
      {
//         cout << "inside PolygonsGroup::generate_new_Polygon(PolyLine_ptr) #2" 
//              << endl;

         polygon* relative_polygon_ptr=
            PolyLine_ptr->construct_relative_polygon();
         if (relative_polygon_ptr != NULL)
         {
            Polygon* Polygon_ptr=generate_new_Polygon(
               PolyLine_ptr->get_reference_origin(),*relative_polygon_ptr);
            Polygon_ptr->set_PolyLine_ptr(PolyLine_ptr);

            threevector UVW_out;
            Polygon_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),UVW_out);
            return Polygon_ptr;
         }
         else
         {
            return NULL;
         }
      }

// ==========================================================================
// Polygon destruction member functions
// ==========================================================================

// Member function destroy_all_Polygons() first fills an STL vector
// with Polygon pointers.  It then iterates over each vector entry and
// calls destroy_Polygon for each pointer. 

void PolygonsGroup::destroy_all_Polygons()
{   
//   cout << "inside PolygonsGroup::destroy_all_Polygons()" << endl;
   unsigned int n_Polygons=get_n_Graphicals();
//   cout << "n_Polygons = " << n_Polygons << endl;

   vector<Polygon*> Polygons_to_destroy;
   for (unsigned int p=0; p<n_Polygons; p++)
   {
      Polygon* Polygon_ptr=get_Polygon_ptr(p);
//      cout << "p = " << p << " Polygon_ptr = " << Polygon_ptr << endl;
      Polygons_to_destroy.push_back(Polygon_ptr);
   }

   for (unsigned int p=0; p<n_Polygons; p++)
   {
      destroy_Polygon(Polygons_to_destroy[p]);
   }

   if (PolyLinesGroup_ptr != NULL) 
      PolyLinesGroup_ptr->destroy_all_PolyLines();
}

// --------------------------------------------------------------------------
// Member function destroy_Polygon removes the selected Polygon from
// the Polygonlist and the OSG Polygons group. 

   bool PolygonsGroup::destroy_Polygon()
      {   
//   cout << "inside PolygonsGroup::destroy_Polygon()" << endl;
         return destroy_Polygon(get_selected_Graphical_ID());
      }

// --------------------------------------------------------------------------
   bool PolygonsGroup::destroy_Polygon(int ID)
      {   
//         cout << "inside PolygonsGroup::destroy_Polygon(ID)" << endl;
//         cout << "ID = " << ID << endl;
//         cout << "n_Polygons = " << get_n_Graphicals() << endl;
         if (ID >= 0)
         {
            return destroy_Polygon(get_ID_labeled_Polygon_ptr(ID));
         }
         else
         {
            return false;
         }
      }

// ---------------------------------------------------------------------
   bool PolygonsGroup::destroy_Polygon(Polygon* curr_Polygon_ptr)
      {
//         cout << "inside PolygonsGroup::destroy_Polygon()" << endl;
//         cout << "curr_Polygon_ptr = " << curr_Polygon_ptr << endl;

         if (curr_Polygon_ptr==NULL) return false;

// Recall that PointsGroup is added as a child to OSGsubPAT.  So we
// must explicitly remove it from the scenegraph before the Polygon
// is destroyed:

         osgGeometry::PointsGroup* PointsGroup_ptr=
            curr_Polygon_ptr->get_PointsGroup_ptr();
//         cout << "PointsGroup_ptr = " << PointsGroup_ptr << endl;
         if (PointsGroup_ptr != NULL)
         {
            remove_OSGgroup_from_OSGsubPAT(
               PointsGroup_ptr->get_OSGgroup_ptr());
         }
         PolyLine* PolyLine_ptr=curr_Polygon_ptr->get_PolyLine_ptr();
//         cout << "PolyLine_ptr = " << PolyLine_ptr << endl;

         if (PolyLine_ptr != NULL && PolyLinesGroup_ptr != NULL)
         {
            PolyLinesGroup_ptr->destroy_PolyLine(PolyLine_ptr);
         }
         return destroy_Graphical(curr_Polygon_ptr);
      }

// --------------------------------------------------------------------------
// Member function erase_Polygon sets boolean entries within the
// member map coords_erased to true for the current Polygon.  When
// Polygon crosshairs are drawn within
// PolygonsGroup::reassign_PAT_ptrs(), entries within this STL map are
// first checked and their positions are set to large negative values
// to prevent them from appearing within the OSG data window.  Yet the
// Polygon itself continues to exist.

   bool PolygonsGroup::erase_Polygon()
      {   
         return erase_Graphical();
      }

// --------------------------------------------------------------------------
// Member function unerase_Polygon queries the user to enter the ID
// for some erased Polygon.  It then unerases that Polygon within the
// current image.

   bool PolygonsGroup::unerase_Polygon()
      {   
         bool Polygon_unerased_flag=unerase_Graphical();
         if (Polygon_unerased_flag) reset_colors();
         return Polygon_unerased_flag;
      }

// --------------------------------------------------------------------------
// Member function reset_colors loops over all entries
// within the current video image's Polygonlist.  It colors blue the
// polygon whose ID equals selected_Graphical_ID.  All other
// polygons are set to their permanent colors.

   void PolygonsGroup::reset_colors()
      {   
         for (unsigned int n=0; n<get_n_Graphicals(); n++)
         {
            Polygon* Polygon_ptr=get_Polygon_ptr(n);
            if (get_selected_Graphical_ID()==Polygon_ptr->get_ID())
            {
               Polygon_ptr->set_curr_color(colorfunc::blue);
            }
            else
            {
               Polygon_ptr->set_curr_color(
                  Polygon_ptr->get_permanent_color());
            }
         } // loop over Polygons in Polygonlist
      }

// --------------------------------------------------------------------------
// Member function update_display()

   void PolygonsGroup::update_display()
      {
//         cout << "***************************************************" 
//              << endl;
//         cout << "inside PolygonsGroup::update_display()" << endl;
//         cout << "n_polygons = " << get_n_Graphicals() << endl;

//         for (unsigned int n=0; n<get_n_Graphicals(); n++)
//         {
//            Polygon* Polygon_ptr=get_Polygon_ptr(n);
//            cout << "n = " << n << " *Polygon_ptr(n) = "
//                 << *(get_Polygon_ptr(n)) << endl;
//         }
         
         GraphicalsGroup::update_display();
//         cout << "***************************************************" 
//              << endl << endl;

      }

// ==========================================================================
// Video annotation member functions
// ==========================================================================

// Member function generate_translucent_Polygon() takes in STL vector
// vertices containing imageplane (U,V) pairs.  It instantiates a new
// PolyLine member of *PolyLinesGroup_ptr to demarcate the polygon's
// border.  It also instantiates a new Polygon member this class which
// holds a translucent polygon filling the border.  

// We wrote this method in Sep 2009 in order to display translucent
// polygons on video image planes.

   void PolygonsGroup::generate_translucent_Polygon(
      colorfunc::Color poly_color,const vector<threevector>& vertices,
      double alpha,bool draw_border_flag)
      {
//         cout << "inside PolygonsGroup::generate_translucent_Polygon()" 
//              << endl;
//         cout << "alpha = " << alpha << endl;

         threevector reference_origin=vertices.front();
         if (PolyLinesGroup_ptr != NULL && draw_border_flag)
         {
            vector<threevector> V;
            for (unsigned int v=0; v<vertices.size(); v++)
            {
               V.push_back(vertices[v]);
            }
            if (vertices.front().nearly_equal(vertices.back()))
            {
            }
            else
            {
               V.push_back(vertices.front());
            }
            
            PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
               reference_origin,V,colorfunc::get_OSG_color(poly_color));
//               Zero_vector,V,colorfunc::get_OSG_color(poly_color));

            PolyLine_ptr->set_linewidth(2);
         }
	
         polygon poly(vertices);
         osgGeometry::Polygon* Polygon_ptr=generate_new_Polygon(
            Zero_vector,poly);
         Polygon_ptr->enable_alpha_blending();
         Polygon_ptr->set_permanent_color(colorfunc::get_OSG_color(
            poly_color,alpha));
         reset_colors();
      }

// --------------------------------------------------------------------------
// Member function sample_translucent_Polygons() forms a lattice of
// U,V pairs inside imageplane_bbox.  It checks whether each lattice
// site overlaps any of the translucent Polygon members of *this.
// This method returns an STL vector of (U,V) lattice sites which do
// lie inside some translucent polygon.

   vector<threevector> PolygonsGroup::sample_translucent_Polygons(
      int mdim,int ndim,const bounding_box& imageplane_bbox)
      {
         double Ulo=imageplane_bbox.get_xmin();
         double Uhi=imageplane_bbox.get_xmax();
         double orig_U_separation=Uhi-Ulo;
         double Vlo=imageplane_bbox.get_ymin();
         double Vhi=imageplane_bbox.get_ymax();
         double orig_V_separation=Vhi-Vlo;
         double deltaU=0.9*(orig_U_separation)/(mdim-1);
         double deltaV=0.9*(orig_V_separation)/(ndim-1);
         Ulo += 0.05*(orig_U_separation);
         Uhi -= 0.05*(orig_U_separation);
         Vlo += 0.05*(orig_V_separation);
         Vhi -= 0.05*(orig_V_separation);

         cout << "Ulo = " << Ulo << " Uhi = " << Uhi << endl;
         cout << "Vlo = " << Vlo << " Vhi = " << Vhi << endl;

         vector<threevector> interior_UV_points;
         for (int px=0; px<mdim; px++)
         {
            double U=Ulo+px*deltaU;
            for (int py=0; py<ndim; py++)
            {
               double V=Vlo+py*deltaV;
               threevector UV(U,V);
               bool UV_inside_flag=false;
               for (unsigned int i=0; i<get_n_Graphicals(); i++)
               {
                  polygon* polygon_ptr=get_Polygon_ptr(i)->
                     get_relative_poly_ptr();
                  if (polygon_ptr->point_inside_polygon(UV))
                  {
                     UV_inside_flag=true;
                  }
               } // loop over index i labeling imageplane polygons
               if (UV_inside_flag) 
               {
                  interior_UV_points.push_back(UV);
               }
            } // loop over py
         } // loop over px
         cout << "interior_UV_points.size()/(mdim*ndim) = "
              << double(interior_UV_points.size())/(mdim*ndim) 
              << endl;
         
         return interior_UV_points;

      }
  

 
} // osgGeometry namespace




