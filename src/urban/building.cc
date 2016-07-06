// ==========================================================================
// BUILDING base class member function definitions
// ==========================================================================
// Last modified on 5/22/06; 6/14/06; 6/15/06; 7/29/06
// ==========================================================================

#include <iostream>
#include <set>
#include "urban/building.h"
#include "math/constants.h"
#include "math/constant_vectors.h"
#include "geometry/contour.h"
#include "geometry/linesegment.h"
#include "datastructures/Linkedlist.h"
#include "urban/oriented_box.h"
#include "geometry/parallelogram.h"
#include "geometry/polygon.h"
#include "urban/rooftop.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::vector;

const int building::max_front_dirs=3;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void building::allocate_member_objects()
{
}		       

void building::initialize_member_objects()
{
   on_street=true;
   on_street_corner=is_street_island=is_street_peninsula=false;
   cityblock_ID=-1;
   roadsegment_ID.first=roadsegment_ID.second=-1;
   rooftop_COM=Zero_vector;
   front_dir[0]=front_dir[1]=front_dir[2]=Zero_vector;
   footprint_area=max_height=-1;
   bbox_ptr=NULL;
   tallest_box_ptr=NULL;
   box_list_ptr=NULL;
   nearby_roadpoint_list_ptr=NULL;
}		       

building::building():
   contour_element()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

building::building(int identification):
   contour_element(identification)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

// Copy constructor:

building::building(const building& b):
   contour_element(b)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(b);
}

building::~building()
{
   cout << "inside building destructor" << endl;
   delete bbox_ptr;
   cout << "Before deleting nearby_roadpoint_list_ptr" << endl;
   delete nearby_roadpoint_list_ptr;
   cout << "Before deleting box_list_ptr" << endl;
   delete box_list_ptr;

/*
   if (box_list_ptr != NULL && box_list_ptr->get_start_ptr() != NULL)
   {
      for (Mynode<oriented_box*>* currnode_ptr=box_list_ptr->get_start_ptr();
           currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
      {
         delete currnode_ptr->get_data();
      }
      delete box_list_ptr;
   }
*/

   bbox_ptr=NULL;
   tallest_box_ptr=NULL;
   box_list_ptr=NULL;
   nearby_roadpoint_list_ptr=NULL;
   cout << "At end of building destructor" << endl;
}

// ---------------------------------------------------------------------
void building::docopy(const building& b)
{
   on_street=b.on_street;
   on_street_corner=b.on_street_corner;
   is_street_island=b.is_street_island;
   is_street_peninsula=b.is_street_peninsula;
   cityblock_ID=b.cityblock_ID;
   roadsegment_ID=b.roadsegment_ID;
   rooftop_COM=b.rooftop_COM;
   footprint_area=b.footprint_area;
   max_height=b.max_height;
   Imin=b.Imin;
   Imax=b.Imax;
   Imin_hat=b.Imin_hat;
   Imax_hat=b.Imax_hat;
   *bbox_ptr=*(b.bbox_ptr);
//   tallest_box_ptr=b.tallest_box_ptr;
   for (int i=0; i<max_front_dirs; i++)
   {
      front_dir[i]=b.front_dir[i];
   }

   if (b.box_list_ptr != NULL)
   {
      if (box_list_ptr==NULL)
      {
         box_list_ptr=new Linkedlist<oriented_box*>(
            *(b.box_list_ptr));
      }
   }
   if (b.nearby_roadpoint_list_ptr != NULL)
   {
      if (nearby_roadpoint_list_ptr==NULL) 
         nearby_roadpoint_list_ptr=new Linkedlist<int>(
            *(b.nearby_roadpoint_list_ptr));
   }
}

// ---------------------------------------------------------------------
// Overload = operator:

building& building::operator= (const building& b)
{
   if (this==&b) return *this;
   contour_element::operator=(b);
   docopy(b);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (std::ostream& outstream,const building& b)
{
   outstream << std::endl;
//   outstream << (contour_element&)b << endl;
//   outstream << "footprint area = " << b.footprint_area << endl;
//   outstream << "max height = " << b.max_height << endl;

   if (b.box_list_ptr != NULL)
   {
      cout << "Current building has " << b.box_list_ptr->size()
           << " boxes" << endl;
      int box_number=1;
      for (Mynode<oriented_box*>* boxnode_ptr=b.box_list_ptr->
              get_start_ptr();
           boxnode_ptr != NULL; boxnode_ptr=boxnode_ptr->get_nextptr())
      {
         cout << "Box " << box_number << ":" << endl;
         oriented_box* oriented_box_ptr=boxnode_ptr->get_data();
         cout << *oriented_box_ptr << endl;
         box_number++;
      }
   } // box_list_ptr != NULL conditional

   return outstream;
}

// ---------------------------------------------------------------------
// Member function find_max_height loops over all oriented boxes
// within the current building object.  It computes and sets member
// variable max_height equal to the maximum of each box's rooftop
// z-value.

double building::find_max_height()
{
   if (max_height < 0)
   {
      for (Mynode<oriented_box*>* boxnode_ptr=box_list_ptr->get_start_ptr();
           boxnode_ptr != NULL; boxnode_ptr=boxnode_ptr->get_nextptr())
      {
         oriented_box* oriented_box_ptr=boxnode_ptr->get_data();
         rooftop* roof_ptr=oriented_box_ptr->get_roof_ptr();

         double curr_height=-1;
//      if (!roof_ptr->get_none_flag())
         {
            if (roof_ptr->get_none_flag() || roof_ptr->get_flat_flag())
            {
               curr_height=(oriented_box_ptr->get_topface()).
                  get_vertex(0).get(2);
            }
            else if (roof_ptr->get_pyramid_flag())
            {
               curr_height=roof_ptr->get_COM().get(2);
            }
            else if (roof_ptr->get_spine_ptr() != NULL)
            {
               curr_height=roof_ptr->get_spine_ptr()->get_v1().get(2);
            }
         }
//      cout << "curr_height = " << curr_height << endl;
         if (curr_height > max_height)
         {
            max_height=curr_height;
            tallest_box_ptr=oriented_box_ptr;
         }
      } // loop over current building object's oriented boxes 
//   cout << "max_height = " << max_height << endl;
   }
   return max_height;
}

// ---------------------------------------------------------------------
// Member function find_closest_point loops over all oriented boxes
// comprising the current building object.  It returns the point along
// the bottom faces of these oriented boxes which lies closest to the
// specified external point's position.

threevector building::find_closest_point(const threevector& ext_posn)
{
   double min_distance=POSITIVEINFINITY;
   threevector closest_point;
   for (Mynode<oriented_box*>* boxnode_ptr=box_list_ptr->get_start_ptr();
        boxnode_ptr != NULL; boxnode_ptr=boxnode_ptr->get_nextptr())
   {
      oriented_box* oriented_box_ptr=boxnode_ptr->get_data();
      polygon curr_bface(oriented_box_ptr->get_bottomface());

      pair<double,threevector> D=curr_bface.closest_polygon_perimeter_point(
         ext_posn);

      if (D.first < min_distance)
      {
         min_distance=D.first;
         closest_point=D.second;
      }
   } // loop over oriented boxes in *box_list_ptr
   
   return closest_point;
}

// ---------------------------------------------------------------------
// Member function generate_footprint_vertices_list loops over all
// oriented boxes within the current building object.  It appends (w/o
// duplication) the vertex of each oriented box's bottom face to a
// dynamically generated linked list.  The list is returned by this
// method.

Linkedlist<threevector>* building::generate_footprint_vertices_list(
   bool return_edge_midpoints) const
{
   Linkedlist<threevector>* vertex_list_ptr=new Linkedlist<threevector>;
   
   for (Mynode<oriented_box*>* boxnode_ptr=box_list_ptr->get_start_ptr();
        boxnode_ptr != NULL; boxnode_ptr=boxnode_ptr->get_nextptr())
   {
      oriented_box* oriented_box_ptr=boxnode_ptr->get_data();
      polygon curr_bface(oriented_box_ptr->get_bottomface().xy_projection());

      int nvertices=curr_bface.get_nvertices();
      for (int i=0; i<nvertices; i++)
      {
         threevector curr_vertex(curr_bface.get_vertex(i));

         threevector v(curr_vertex);
         if (return_edge_midpoints)
         {
            threevector next_vertex(
               curr_bface.get_vertex(modulo(i+1,nvertices)));
            threevector midpoint(0.5*(curr_vertex+next_vertex));
            v=midpoint;
         }
         
         if (vertex_list_ptr->data_in_list(v)==NULL)
         {
            vertex_list_ptr->append_node(v);
         }
      } // loop over bottom face vertices
   } // loop over oriented boxes in *box_list_ptr
   
   return vertex_list_ptr;
}

// ---------------------------------------------------------------------
// Member function footprint_COM computes and returns the average of
// each oriented box's bottom face COM weighted by its area.

threevector building::footprint_COM() const
{
   vector<double> area;
   vector<threevector> COM;
   
   for (Mynode<oriented_box*>* boxnode_ptr=box_list_ptr->get_start_ptr();
        boxnode_ptr != NULL; boxnode_ptr=boxnode_ptr->get_nextptr())
   {
      oriented_box* oriented_box_ptr=boxnode_ptr->get_data();
      polygon curr_bface(oriented_box_ptr->get_bottomface().xy_projection());
      area.push_back(curr_bface.compute_area());
      COM.push_back(curr_bface.compute_COM());
   } // loop over oriented boxes

   double denom=0;
   threevector numer(Zero_vector);
   for (unsigned int i=0; i<COM.size(); i++)
   {
      numer += area[i]*COM[i];
      denom += area[i];
   }
   
   return numer/denom;
}

// ---------------------------------------------------------------------
linesegment* building::get_tallest_spine_ptr() const
{
   rooftop* tallest_rooftop_ptr=tallest_box_ptr->get_roof_ptr();
   return tallest_rooftop_ptr->get_spine_ptr();
}
