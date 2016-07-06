// ==========================================================================
// Header file for oriented_box class
// ==========================================================================
// Last modified on 6/21/05; 4/23/06; 6/10/06
// ==========================================================================

#ifndef ORIENTED_BOX_H
#define ORIENTED_BOX_H

#include <string>
#include <vector>
#include "geometry/mybox.h"
class rooftop;

class oriented_box: public mybox
{
  public:

   enum sideface_type
   {
      front,side,back
   };

// Initialization, constructor and destructor functions:

   oriented_box(void);
   oriented_box(double w,double l,double h);
   oriented_box(
      double xlo,double xhi,double ylo,double yhi,double zlo,double zhi);
   oriented_box(const polygon& bface,const threevector& zhat,double h);
   oriented_box(const oriented_box& o);
   virtual ~oriented_box();
   oriented_box operator= (const oriented_box& o);
   friend std::ostream& operator<< 
      (std::ostream& outstream,oriented_box& o);
   void print_rooftop_information();
   std::ostream& write_to_textstream(std::ostream& textstream);
   void read_from_text_lines(unsigned int& i,std::vector<std::string>& line);

// Set and get member functions:

   void set_roof_ptr(rooftop* r_ptr);
   rooftop* get_roof_ptr();
   const rooftop* get_roof_ptr() const;

  protected:

   rooftop* roof_ptr;
   sideface_type* face_label;	// array of sideface types

  private: 

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const oriented_box& o);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void oriented_box::set_roof_ptr(rooftop* r_ptr)
{
   roof_ptr=r_ptr;
}

inline rooftop* oriented_box::get_roof_ptr()
{
   return roof_ptr;
}

inline const rooftop* oriented_box::get_roof_ptr() const
{
   return roof_ptr;
}


#endif  // oriented_box.h






