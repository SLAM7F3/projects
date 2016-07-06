// ==========================================================================
// NETWORK_ELEMENT base class member function definitions
// ==========================================================================
// Last modified on 1/27/05; 4/13/06; 6/14/06; 7/29/06
// ==========================================================================

#include <iostream>
#include "math/constant_vectors.h"
#include "datastructures/Linkedlist.h"
#include "datastructures/Mynode.h"
#include "network/network_element.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void network_element::allocate_member_objects()
{
}		       

void network_element::initialize_member_objects()
{
   ID=-1;
   posn=center=Zero_vector;
   pixel_list_ptr=NULL;
   translated_pixel_list_ptr=NULL;
}		       

network_element::network_element()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

network_element::network_element(int identification)
{	
   initialize_member_objects();
   allocate_member_objects();
   ID=identification;
}		       

network_element::network_element(const threevector& p)
{	
   initialize_member_objects();
   allocate_member_objects();
   posn=p;
}		       

network_element::network_element(int identification,const threevector& p)
{	
   initialize_member_objects();
   allocate_member_objects();
   ID=identification;
   posn=p;
}		       

// Copy constructor:

network_element::network_element(const network_element& n)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(n);
}

network_element::~network_element()
{
   delete pixel_list_ptr;
   delete translated_pixel_list_ptr;
   pixel_list_ptr=NULL;
   translated_pixel_list_ptr=NULL;
}

// ---------------------------------------------------------------------
void network_element::docopy(const network_element& n)
{
   ID=n.ID;
   posn=n.posn;
   center=n.center;
   *pixel_list_ptr=*(n.pixel_list_ptr);
   *translated_pixel_list_ptr=*(n.translated_pixel_list_ptr);
}

// ---------------------------------------------------------------------
// Overload = operator:

network_element& network_element::operator= (const network_element& n)
{
   if (this==&n) return *this;
   docopy(n);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const network_element& n)
{
   outstream << endl;
   outstream << "Network element ID = " << n.ID << endl;
   outstream << "posn = " << n.posn << endl;
   
   return outstream;
}

// ---------------------------------------------------------------------
// Member function generate_translated_pixel_list takes in linked list
// *plist_ptr of pixels along with their center-of-mass.  This method
// translates each pixel position so that the collective
// center-of-mass moves to the origin (0,0).  This method dynamically
// generates linked list member *translated_pixel_list_ptr containing
// the translated pixels.

linkedlist* network_element::generate_translated_pixel_list(
   linkedlist const *plist_ptr,const threevector& COM)
{
   if (plist_ptr != NULL)
   {
      const int n_node_indep_vars=4;
      const int n_node_depend_vars=2;
      double var[n_node_indep_vars];
      double func_value[n_node_depend_vars];

      delete translated_pixel_list_ptr;
      translated_pixel_list_ptr=new linkedlist;
            
// Translate rooftop pixels' positions (but not pixels' coordinates!)
// so that COM is reset to origin:

      for (mynode const *curr_pixel_ptr=plist_ptr->get_start_ptr();
           curr_pixel_ptr != NULL; curr_pixel_ptr=
              curr_pixel_ptr->get_nextptr())
      {
         var[0]=curr_pixel_ptr->get_data().get_var(0);
         var[1]=curr_pixel_ptr->get_data().get_var(1);
         var[2]=curr_pixel_ptr->get_data().get_var(2)-COM.get(0);
         var[3]=curr_pixel_ptr->get_data().get_var(3)-COM.get(1);
         func_value[0]=curr_pixel_ptr->get_data().get_func(0);
         func_value[1]=curr_pixel_ptr->get_data().get_func(1);

         translated_pixel_list_ptr->append_node(
            datapoint(n_node_indep_vars,n_node_depend_vars,
                      var,func_value));
      }
   } // plist_ptr != NULL conditional
   return translated_pixel_list_ptr;
}

