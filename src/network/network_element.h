// ==========================================================================
// Header file for NETWORK_ELEMENT base class (which should be pure virtual)
// ==========================================================================
// Last modified on 8/9/04; 6/14/06; 7/29/06
// ==========================================================================

#ifndef NETWORK_ELEMENT_H
#define NETWORK_ELEMENT_H

#include "datastructures/datapoint.h"
#include "math/threevector.h"

template <class T> class Linkedlist;
typedef Linkedlist<datapoint> linkedlist;
class threevector;

class network_element
{

  public:

// Initialization, constructor and destructor functions:

   network_element();
   network_element(int identification);
   network_element(const threevector& p);
   network_element(int identification,const threevector& p);
   network_element(const network_element& n);
   virtual ~network_element();
   network_element& operator= (const network_element& n);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const network_element& n);

// Set & get member functions:

   void set_ID(const int id);
   void set_posn(const threevector& p);
   void set_center(const threevector& c);
   void set_pixel_list_ptr(linkedlist* list_ptr);

   int get_ID() const;
   threevector& get_posn();
   const threevector& get_posn() const;
   threevector& get_center();
   linkedlist* get_pixel_list_ptr();
   const linkedlist* get_pixel_list_ptr() const;
   linkedlist* get_translated_pixel_list_ptr();
   const linkedlist* get_translated_pixel_list_ptr() const;

   linkedlist* generate_translated_pixel_list(
      linkedlist const *plist_ptr,const threevector& COM);

  private:

   int ID;
   threevector posn,center;
   linkedlist *pixel_list_ptr,*translated_pixel_list_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const network_element& n);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void network_element::set_ID(const int id)
{
   ID=id;
}

inline void network_element::set_posn(const threevector& p)
{
   posn=p;
}

inline void network_element::set_center(const threevector& c)
{
   center=c;
}

inline void network_element::set_pixel_list_ptr(linkedlist* list_ptr)
{
   pixel_list_ptr=list_ptr;
}

inline int network_element::get_ID() const
{
   return ID;
}

inline threevector& network_element::get_posn() 
{
   return posn;
}

inline const threevector& network_element::get_posn() const
{
   return posn;
}

inline threevector& network_element::get_center() 
{
   return center;
}

inline linkedlist* network_element::get_pixel_list_ptr()
{
   return pixel_list_ptr;
}

inline const linkedlist* network_element::get_pixel_list_ptr() const
{
   return pixel_list_ptr;
}

inline linkedlist* network_element::get_translated_pixel_list_ptr()
{
   return translated_pixel_list_ptr;
}

inline const linkedlist* network_element::get_translated_pixel_list_ptr() 
   const
{
   return translated_pixel_list_ptr;
}

#endif // network_element.h



