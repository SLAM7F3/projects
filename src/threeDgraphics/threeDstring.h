// ==========================================================================
// Header file for 3DSTRING class
// ==========================================================================
// Last modified on 4/23/06; 8/4/06; 1/29/12; 4/5/14
// ==========================================================================

#ifndef THREEDSTRING_H
#define THREEDSTRING_H

#include <string>
#include <vector>
#include "threeDgraphics/character.h"

class rotation;

class threeDstring
{

  public:

// Initialization, constructor and destructor functions:

   threeDstring();
   threeDstring(std::string inputstring);
   threeDstring(const threeDstring& s);
   ~threeDstring();
   threeDstring& operator= (const threeDstring& s);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const threeDstring& s);

// Set & get member functions:

   void set_origin(const threevector& o);
   void push_back_char(const character& c);
   void set_ascii_char(character* ascii_char_ptr);
   unsigned int get_nchars() const;
   const std::string& get_ascii_string() const;
   mypoint& get_origin();
   const mypoint& get_origin() const;
   linesegment& get_extent();
   const linesegment& get_extent() const;
   character get_char(int i);
   const character get_char(int i) const;
   std::vector<character>& get_charstring();
   const std::vector<character>& get_charstring() const;
   character& get_ascii_char(int i);

   void parse_string(std::string inputstring);

// Moving around member functions:

   void translate(const threevector& r_vec);
   void center_upon_location(const threevector& r_vec);
   void scale(const threevector& scale_origin,double scale_factor);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);
   void rotate(const threevector& rotation_origin,const rotation& R);

  private:

   unsigned int nchars;
   mypoint origin;
   linesegment extent;
   std::string ascii_string;
   std::vector<character> charstring;
   character* ascii_char;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const threeDstring& s);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void threeDstring::set_origin(const threevector& o)
{
   origin=mypoint(o);
}

inline void threeDstring::push_back_char(const character& c)
{
   nchars++;
   charstring.push_back(c);
   if (nchars > 1)
   {
      charstring[nchars-1].translate(threevector(
         charstring[nchars-1].get_origin().get_center()+
         charstring[nchars-2].get_extent().get_v2()));
   }
}

inline void threeDstring::set_ascii_char(character* ascii_char_ptr)
{
   ascii_char=ascii_char_ptr;
}

inline unsigned int threeDstring::get_nchars() const
{
   return nchars;
}

inline const std::string& threeDstring::get_ascii_string() const
{
   return ascii_string;
}

inline mypoint& threeDstring::get_origin()
{
   return origin;
}

inline const mypoint& threeDstring::get_origin() const
{
   return origin;
}

inline linesegment& threeDstring::get_extent()
{
   return extent;
}

inline const linesegment& threeDstring::get_extent() const
{
   return extent;
}

inline std::vector<character>& threeDstring::get_charstring() 
{
   return charstring;
}

inline const std::vector<character>& threeDstring::get_charstring() const 
{
   return charstring;
}

inline character threeDstring::get_char(int i) 
{
   return charstring[i];
}

inline const character threeDstring::get_char(int i) const
{
   return charstring[i];
}

inline character& threeDstring::get_ascii_char(int i)
{
   return ascii_char[i];
}


#endif // threeDstring.h



