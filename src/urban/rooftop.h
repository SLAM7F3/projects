// ==========================================================================
// Header file for ROOFTOP class
// ==========================================================================
// Last modified on 6/21/05; 4/23/06; 7/29/06
// ==========================================================================

#ifndef ROOFTOP_H
#define ROOFTOP_H

#include <iostream>
#include <string>
#include <vector>
#include "math/threevector.h"
class linesegment;

class rooftop
{

  public:

// Initialization, constructor and destructor functions:

   rooftop();
   rooftop(const rooftop& r);
   ~rooftop();
   rooftop& operator= (const rooftop& r);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const rooftop& r);

// Set & get member functions:

   void set_none_flag(const bool flag);
   void set_flat_flag(const bool flag);
   void set_pyramid_flag(const bool flag);
   void set_COM(const threevector& v);
   void set_spine_ptr(linesegment* s_ptr);

   bool get_none_flag() const;
   bool get_flat_flag() const;
   bool get_pyramid_flag() const;
   const threevector& get_COM() const;
   linesegment* get_spine_ptr();
   const linesegment* get_spine_ptr() const;

// Text file I/O:

   std::ostream& write_to_textstream(std::ostream& textstream);
   void read_from_text_lines(unsigned int& i,std::vector<std::string>& line);

  private:

   bool none_flag,flat_flag,pyramid_flag;
   threevector COM;
   linesegment* spine_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const rooftop& b);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void rooftop::set_none_flag(const bool flag)
{
   none_flag=flag;
}

inline void rooftop::set_flat_flag(const bool flag)
{
   flat_flag=flag;
}

inline void rooftop::set_pyramid_flag(const bool flag)
{
   pyramid_flag=flag;
}

inline void rooftop::set_COM(const threevector& v)
{
   COM=v;
}

inline void rooftop::set_spine_ptr(linesegment* s_ptr)
{
   spine_ptr=s_ptr;
}

inline bool rooftop::get_none_flag() const
{
   return none_flag;
}

inline bool rooftop::get_flat_flag() const
{
   return flat_flag;
}

inline bool rooftop::get_pyramid_flag() const
{
   return pyramid_flag;
}

inline const threevector& rooftop::get_COM() const
{
   return COM;
}

inline linesegment* rooftop::get_spine_ptr() 
{
   return spine_ptr;
}

inline const linesegment* rooftop::get_spine_ptr() const
{
   return spine_ptr;
}

#endif // rooftop.h



