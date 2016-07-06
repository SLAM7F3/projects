// ==========================================================================
// Header file for templatized Site base class
// ==========================================================================
// Last modified on 3/16/05; 8/3/06
// ==========================================================================

#ifndef T_SITE_H
#define T_SITE_H

#include <vector>
#include "network/netlink.h"

template <class T> class Linkedlist;

template <class T>
class Site
{

  public:

// Initialization, constructor and destructor functions:

   Site();
   Site(T& d);
   Site(const Site& s);
   virtual ~Site();
   Site<T>& operator= (const Site<T>& s);

   template <class T1>
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Site<T1>& s);
   void display_dereferenced_data(std::ostream& sitestream) const;

// Set & get member functions:

   void set_RHS_neighbor(int r);
   void set_LHS_neighbor(int l);
   void set_data(T const & d);

   void set_netlink_list_ptr(Linkedlist<netlink>* list_ptr);

   int get_n_neighbors() const;
   int get_RHS_neighbor() const;
   int get_LHS_neighbor() const;
   T& get_data();
   const T& get_data() const;
   Linkedlist<netlink>* get_netlink_list_ptr();   
   const Linkedlist<netlink>* get_netlink_list_ptr() const;   
   netlink* get_netlink_ptr(int neighbor_ID);
   std::vector<int> get_neighbors() const;

   void set_n_neighbors(int n);
   void increment_n_neighbors();
   void decrement_n_neighbors();
   void display_neighbor_list();

  private:

   int n_neighbors;

// Right & left handed neighbors are only well-defined when a definite
// ordering can be imposed upon network sites:

   int RHS_neighbor;
   int LHS_neighbor;

   T data;		// Arbitrary data object
   Linkedlist<netlink>* netlink_list_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Site<T>& s);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

template <class T> inline void Site<T>::set_RHS_neighbor(int r)
{
   RHS_neighbor=r;
}

template <class T> inline void Site<T>::set_LHS_neighbor(int l)
{
   LHS_neighbor=l;
}

template <class T> inline void Site<T>::set_data(T const & d)
{
   data=d;
}

template <class T> inline void Site<T>::set_netlink_list_ptr(
   Linkedlist<netlink>* list_ptr)
{
   netlink_list_ptr=list_ptr;
}

template <class T> inline int Site<T>::get_RHS_neighbor() const
{
   return RHS_neighbor;
}

template <class T> inline int Site<T>::get_LHS_neighbor() const
{
   return LHS_neighbor;
}

template <class T> inline int Site<T>::get_n_neighbors() const
{
   return n_neighbors;
}

template <class T> inline T& Site<T>::get_data() 
{
   return data;
}

template <class T> inline const T& Site<T>::get_data() const
{
   return data;
}

template <class T> inline Linkedlist<netlink>* Site<T>::get_netlink_list_ptr()
{
   return netlink_list_ptr;
}

template <class T> inline const Linkedlist<netlink>* 
Site<T>::get_netlink_list_ptr() const
{
   return netlink_list_ptr;
}

template <class T> inline void Site<T>::set_n_neighbors(int n)
{
   n_neighbors=n;
}

template <class T> inline void Site<T>::increment_n_neighbors() 
{
   n_neighbors++;
}

template <class T> inline void Site<T>::decrement_n_neighbors() 
{
   n_neighbors--;
}

#include "Site.cc"

#endif // T_Site.h



