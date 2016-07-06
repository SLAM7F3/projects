// ==========================================================================
// Header file for CONTOUR_ELEMENT base class (which should be pure virtual)
// ==========================================================================
// Last modified on 8/9/04; 6/14/06; 7/29/06
// ==========================================================================

#ifndef CONTOUR_ELEMENT_H
#define CONTOUR_ELEMENT_H

#include "network/network_element.h"

class contour;
template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;
class polygon;
class threevector;

class contour_element: public network_element
{

  public:

// Initialization, constructor and destructor functions:

   contour_element();
   contour_element(int identification);
   contour_element(const threevector& p);
   contour_element(int identification,const threevector& p);
   contour_element(const contour_element& n);
   virtual ~contour_element();
   contour_element& operator= (const contour_element& n);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const contour_element& n);

// Set & get member functions:

   void set_voronoi_region_ptr(polygon* poly_ptr);
   void set_contour_ptr(contour* contour_ptr);
   void set_subcontour_list_ptr(Linkedlist<contour*>* list_ptr);

   contour* get_contour_ptr();
   const contour* get_contour_ptr() const;
   Linkedlist<contour*>* get_subcontour_list_ptr();
   const Linkedlist<contour*>* get_subcontour_list_ptr() const;
   polygon* get_voronoi_region_ptr();

  private:

   contour* contour_ptr;
   Linkedlist<contour*>* subcontour_list_ptr;
   polygon* voronoi_region_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const contour_element& n);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void contour_element::set_contour_ptr(contour* c_ptr)
{
   contour_ptr=c_ptr;
}

inline void contour_element::set_subcontour_list_ptr(
   Linkedlist<contour*>* list_ptr)
{
   subcontour_list_ptr=list_ptr;
}

inline void contour_element::set_voronoi_region_ptr(polygon* poly_ptr)
{
   voronoi_region_ptr=poly_ptr;
}

inline contour* contour_element::get_contour_ptr() 
{
   return contour_ptr;
}

inline const contour* contour_element::get_contour_ptr() const
{
   return contour_ptr;
}

inline Linkedlist<contour*>* contour_element::get_subcontour_list_ptr() 
{
   return subcontour_list_ptr;
}

inline const Linkedlist<contour*>* contour_element::get_subcontour_list_ptr() 
   const
{
   return subcontour_list_ptr;
}

inline polygon* contour_element::get_voronoi_region_ptr()
{
   return voronoi_region_ptr;
}

#endif // contour_element.h



