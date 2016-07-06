// ==========================================================================
// Header file for ROADSEGMENT base class
// ==========================================================================
// Last modified on 3/13/05; 4/23/06; 6/14/06
// ==========================================================================

#ifndef ROADSEGMENT_H
#define ROADSEGMENT_H

#include "geometry/contour_element.h"
#include "geometry/linesegment.h"
#include "network/Network.h"
#include "urban/roadpoint.h"

class roadsegment: public contour_element
{

  public:

// Initialization, constructor and destructor functions:

   roadsegment();
   roadsegment(int identification,int begin_label,int end_label,
               Network<roadpoint*> const *roadpoints_network_ptr);
   roadsegment(const roadsegment& r);
   virtual ~roadsegment();
   roadsegment& operator= (const roadsegment& r);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const roadsegment& r);

// Set & get member functions:

   int get_start_label() const;
   int get_stop_label() const;
   roadpoint* get_start_roadpoint() const;
   roadpoint* get_stop_roadpoint() const;
   const linesegment& get_segment() const;

  private:

   roadpoint* start_roadpoint_ptr;
   roadpoint* stop_roadpoint_ptr;
   linesegment l;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const roadsegment& r);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline int roadsegment::get_start_label() const
{
   return start_roadpoint_ptr->get_ID();
}

inline int roadsegment::get_stop_label() const
{
   return stop_roadpoint_ptr->get_ID();
}

inline roadpoint* roadsegment::get_start_roadpoint() const
{
   return start_roadpoint_ptr;
}

inline roadpoint* roadsegment::get_stop_roadpoint() const
{
   return stop_roadpoint_ptr;
}

inline const linesegment& roadsegment::get_segment() const
{
   return l;
}

#endif // roadsegment.h



