// ==========================================================================
// Header file for BUILDING class
// ==========================================================================
// Last modified on 4/17/05; 6/14/06; 7/29/06
// ==========================================================================

#ifndef BUILDING_H
#define BUILDING_H

#include <vector>
#include "geometry/contour_element.h"
#include "datastructures/datapoint.h"
#include "math/threevector.h"

class contour;
class linesegment;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;
class oriented_box;
class parallelogram;
class polygon;

class building: public contour_element
{
  public:

   static const int max_front_dirs;
   
// Initialization, constructor and destructor functions:

   building();
   building(int identification);
   building(const building& b);
   virtual ~building();
   building& operator= (const building& b);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const building& b);

// Set & get member functions:

   void set_on_street(const bool on_street_flag);
   void set_on_street_corner(const bool corner_flag);
   void set_is_street_island(const bool island_flag);
   void set_is_street_peninsula(const bool peninsula_flag);
   void set_cityblock_ID(int id);
   void set_roadsegment_ID(int id);
   void set_rooftop_COM(const threevector& COM);
   void set_footprint_area(double area);
   void set_max_height(double height);
   void set_Imin(double imin);
   void set_Imax(double imax);
   void set_Imin_hat(const threevector& imin_hat);
   void set_Imax_hat(const threevector& imax_hat);
   void set_front_dir(int n,const threevector& dir);
   void set_bbox_ptr(parallelogram* bbox_ptr);
   void set_box_list_ptr(Linkedlist<oriented_box*>* list_ptr);
   void set_nearby_roadpoint_list_ptr(Linkedlist<int>* list_ptr);

   bool get_on_street() const;
   bool get_on_street_corner() const;
   bool get_is_street_island() const;
   bool get_is_street_peninsula() const;
   int get_cityblock_ID() const;
   std::pair<int,int> get_roadsegment_ID() const;
   const threevector& get_rooftop_COM() const;
   double get_footprint_area() const;
   double get_max_height() const;
   double get_Imin() const;
   double get_Imax() const;
   threevector& get_Imin_hat();
   threevector& get_Imax_hat();
   const threevector& get_front_dir(int n) const;
   parallelogram* get_bbox_ptr();
   oriented_box* get_tallest_box_ptr();
   Linkedlist<oriented_box*>* get_box_list_ptr();
   const Linkedlist<oriented_box*>* get_box_list_ptr() const;
   Linkedlist<int>* get_nearby_roadpoint_list_ptr();   
   const Linkedlist<int>* get_nearby_roadpoint_list_ptr() const;   

   double find_max_height();
   threevector find_closest_point(const threevector& ext_posn);
   Linkedlist<threevector>* generate_footprint_vertices_list(
      bool return_edge_midpoints=true) const;
   threevector footprint_COM() const;
   linesegment* get_tallest_spine_ptr() const;

  private:

   bool on_street;
   bool on_street_corner,is_street_island,is_street_peninsula;
   int cityblock_ID;
   std::pair<int,int> roadsegment_ID;
   threevector rooftop_COM;
   double footprint_area,max_height;
   double Imin,Imax;
   threevector Imin_hat,Imax_hat;
   threevector front_dir[3];
   parallelogram* bbox_ptr;
   oriented_box* tallest_box_ptr;
   Linkedlist<oriented_box*>* box_list_ptr;
   Linkedlist<int>* nearby_roadpoint_list_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const building& b);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void building::set_on_street(const bool on_street_flag)
{
   on_street=on_street_flag;
}

inline void building::set_on_street_corner(const bool corner_flag)
{
   on_street_corner=corner_flag;
}

inline void building::set_is_street_island(const bool island_flag)
{
   is_street_island=island_flag;
}

inline void building::set_is_street_peninsula(const bool peninsula_flag)
{
   is_street_peninsula=peninsula_flag;
}

inline void building::set_cityblock_ID(const int id)
{
   cityblock_ID=id;
}

// Corner buildings are generally associated with two different
// cityblock road segments.  Most other buildings face a single
// roadsegment.

inline void building::set_roadsegment_ID(int id)
{
   if (roadsegment_ID.first==-1)
   {
      roadsegment_ID.first=id;
   }
   else
   {
      roadsegment_ID.second=id;
   }
}

inline void building::set_rooftop_COM(const threevector& COM)
{
   rooftop_COM=COM;
}

inline void building::set_footprint_area(double area)
{
   footprint_area=area;
}

inline void building::set_max_height(double height)
{
   max_height=height;
}

inline void building::set_Imin(double imin)
{
   Imin=imin;
}

inline void building::set_Imax(double imax)
{
   Imax=imax;
}

inline void building::set_Imin_hat(const threevector& imin_hat)
{
   Imin_hat=imin_hat;
}

inline void building::set_Imax_hat(const threevector& imax_hat)
{
   Imax_hat=imax_hat;
}

inline void building::set_front_dir(int n,const threevector& dir)
{
   if (n > 2)
   {
      std::cout << "Error in building::set_front_dir()" << std::endl;
      std::cout << "Maximum number of allowed front directions = 3"
                << std::endl;
      std::cout << "n = " << n << std::endl;
      exit(-1);
   }
   else
   {
      front_dir[n]=dir;
   }
}

inline void building::set_bbox_ptr(parallelogram* poly_ptr)
{
   bbox_ptr=poly_ptr;
}

inline void building::set_box_list_ptr(Linkedlist<oriented_box*>* list_ptr)
{
   box_list_ptr=list_ptr;
}

inline void building::set_nearby_roadpoint_list_ptr(Linkedlist<int>* list_ptr)
{
   nearby_roadpoint_list_ptr=list_ptr;
}

inline bool building::get_on_street() const
{
   return on_street;
}

inline bool building::get_on_street_corner() const
{
   return on_street_corner;
}

inline bool building::get_is_street_island() const
{
   return is_street_island;
}

inline bool building::get_is_street_peninsula() const
{
   return is_street_peninsula;
}

inline int building::get_cityblock_ID() const
{
   return cityblock_ID;
}

inline std::pair<int,int> building::get_roadsegment_ID() const
{
   return roadsegment_ID;
}

inline const threevector& building::get_rooftop_COM() const
{
   return rooftop_COM;
}

inline double building::get_footprint_area() const
{
   return footprint_area;
}

inline double building::get_max_height() const
{
   return max_height;
}

inline double building::get_Imin() const
{
   return Imin;
}

inline double building::get_Imax() const
{
   return Imax;
}

inline threevector& building::get_Imin_hat()
{
   return Imin_hat;
}

inline threevector& building::get_Imax_hat()
{
   return Imax_hat;
}

inline const threevector& building::get_front_dir(int n) const
{
   return front_dir[n];
}

inline parallelogram* building::get_bbox_ptr()
{
   return bbox_ptr;
}

inline oriented_box* building::get_tallest_box_ptr()
{
   return tallest_box_ptr;
}

inline Linkedlist<oriented_box*>* building::get_box_list_ptr()
{
   return box_list_ptr;
}

inline const Linkedlist<oriented_box*>* building::get_box_list_ptr() const
{
   return box_list_ptr;
}

inline Linkedlist<int>* building::get_nearby_roadpoint_list_ptr()
{
   return nearby_roadpoint_list_ptr;
}

inline const Linkedlist<int>* building::get_nearby_roadpoint_list_ptr() const
{
   return nearby_roadpoint_list_ptr;
}

#endif // building.h



