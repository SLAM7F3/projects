// ==========================================================================
// Header file for ROADPOINT base class
// ==========================================================================
// Last modified on 3/12/05; 4/23/06; 6/14/06; 7/29/06
// ==========================================================================

#ifndef ROADPOINT_H
#define ROADPOINT_H

#include <vector>
#include "geometry/contour_element.h"
#include "datastructures/Triple.h"

template <class T> class Hashtable;
template <class T> class Linkedlist;
class threevector;

class roadpoint: public contour_element
{

  public:

// Initialization, constructor and destructor functions:

   roadpoint();
   roadpoint(const threevector& p);
   roadpoint(int identification,const threevector& p);
   roadpoint(const roadpoint& r);
   virtual ~roadpoint();
   roadpoint& operator= (const roadpoint& r);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const roadpoint& r);

// Set & get member functions:

   void set_intersection(bool inter_flag);
   void set_near_bldg_island(bool inter_flag);
   void set_at_infinity(bool inf_pnt);
   void set_in_front_of_bldg(bool in_front);
   void set_on_data_bbox(bool on_bbox);
   void set_data_bbox_corner(bool corner);
   void set_nearby_bldg_list_ptr(Linkedlist<int>* list_ptr);

   bool get_intersection() const;
   bool get_near_bldg_island() const;
   bool get_at_infinity() const;
   bool get_in_front_of_bldg() const;
   bool get_on_data_bbox() const;
   bool get_data_bbox_corner() const;
   Linkedlist<int>* get_nearby_bldg_list_ptr();   
   std::vector<Triple<int,double,double> >& get_adjacent_cityblock();

  private:

   bool intersection,near_bldg_island;
   bool at_infinity,in_front_of_bldg;
   bool on_data_bbox,data_bbox_corner;
   Linkedlist<int>* nearby_bldg_list_ptr;
   std::vector<Triple<int,double,double> > adjacent_cityblock;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const roadpoint& r);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void roadpoint::set_intersection(bool inter_flag)
{
   intersection=inter_flag;
}

inline void roadpoint::set_near_bldg_island(bool nbi_flag)
{
   near_bldg_island=nbi_flag;
}

inline void roadpoint::set_at_infinity(bool inf_pnt)
{
   at_infinity=inf_pnt;
}

inline void roadpoint::set_in_front_of_bldg(bool in_front)
{
   in_front_of_bldg=in_front;
}

inline void roadpoint::set_on_data_bbox(bool on_bbox)
{
   on_data_bbox=on_bbox;
}

inline void roadpoint::set_data_bbox_corner(bool corner)
{
   data_bbox_corner=corner;
}

inline void roadpoint::set_nearby_bldg_list_ptr(Linkedlist<int>* list_ptr)
{
   nearby_bldg_list_ptr=list_ptr;
}

inline bool roadpoint::get_intersection() const
{
   return intersection;
}

inline bool roadpoint::get_near_bldg_island() const
{
   return near_bldg_island;
}

inline bool roadpoint::get_at_infinity() const
{
   return at_infinity;
}

inline bool roadpoint::get_in_front_of_bldg() const
{
   return in_front_of_bldg;
}

inline bool roadpoint::get_on_data_bbox() const
{
   return on_data_bbox;
}

inline bool roadpoint::get_data_bbox_corner() const
{
   return data_bbox_corner;
}

inline Linkedlist<int>* roadpoint::get_nearby_bldg_list_ptr()
{
   return nearby_bldg_list_ptr;
}

inline std::vector<Triple<int,double,double> >& 
roadpoint::get_adjacent_cityblock()
{
   return adjacent_cityblock;
}

#endif // roadpoint.h



