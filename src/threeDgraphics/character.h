// ==========================================================================
// Header file for CHARACTER class
// ==========================================================================
// Last modified on 7/25/04; 8/4/06; 1/29/12
// ==========================================================================

#ifndef CHARACTER_H
#define CHARACTER_H

#include <vector>
#include "geometry/linesegment.h"
#include "geometry/mypoint.h"

class rotation;

class character
{

  public:

// Initialization, constructor and destructor functions:

   character();
   character(const character& c);
   ~character();
   character& operator= (const character& c);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const character& c);

// Set & get member functions:

   void set_ascii_value(int i);
   void set_origin(const threevector& c);
   void set_extent(const linesegment& e);
   void push_back_segment(const linesegment& l);
   int get_ascii_value() const;
   mypoint& get_origin();
   const mypoint& get_origin() const;
   linesegment& get_extent();
   const linesegment& get_extent() const;
   std::vector<linesegment>& get_segment();
   const std::vector<linesegment>& get_segment() const;

// Moving around member functions:

   void translate(const threevector& r_vec);
   void scale(const threevector& scale_origin,double scale_factor);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);
   void rotate(const threevector& rotation_origin,const rotation& R);

  private:

   int ascii_value;
   mypoint origin;
   linesegment extent;
   std::vector<linesegment> segment;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const character& c);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void character::set_ascii_value(int i)
{
   ascii_value=i;
}

inline void character::set_origin(const threevector& c)
{
   origin=mypoint(c);
}

inline void character::set_extent(const linesegment& e)
{
   extent=e;
}

inline void character::push_back_segment(const linesegment& l)
{
   segment.push_back(l);
}

inline mypoint& character::get_origin()
{
   return origin;
}

inline int character::get_ascii_value() const
{
   return ascii_value;
}

inline const mypoint& character::get_origin() const
{
   return origin;
}

inline linesegment& character::get_extent()
{
   return extent;
}

inline const linesegment& character::get_extent() const
{
   return extent;
}

inline std::vector<linesegment>& character::get_segment() 
{
   return segment;
}

inline const std::vector<linesegment>& character::get_segment() const
{
   return segment;
}

#endif // character.h



