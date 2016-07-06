// ==========================================================================
// Header file for vertex class
// ==========================================================================
// Last modified on 5/12/08; 1/29/12; 6/29/12
// ==========================================================================

#ifndef VERTEX_H
#define VERTEX_H

#include <vector>
#include "math/threevector.h"

class rotation;

class vertex
{

  public:

   vertex();
   vertex(const threevector& V,int id=-1);
   vertex(const vertex& v);
   ~vertex();
   vertex& operator= (const vertex& v);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const vertex& v);

// Set and get methods:

   void set_ID(int id);
   int get_ID() const;
   void set_posn(const threevector& posn);
   threevector& get_posn();
   const threevector& get_posn() const;

// Above Z-plane member functions:

   bool above_Zplane(double z);
   bool below_Zplane(double z);

// Moving around vertex member functions:

   void translate(const threevector& rvec);
   void absolute_position(const threevector& rvec);
   void scale(const threevector& scale_origin,double s);
   void scale(const threevector& scale_origin,const threevector& scalefactor);
   void rotate(const rotation& R);
   void rotate(const threevector& rotation_origin,const rotation& R);
   void rotate(const threevector& rotation_origin,
               double thetax,double thetay,double thetaz);

  private: 

   int ID;
   threevector V;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const vertex& v);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void vertex::set_ID(int id)
{
//   std::cout << "inside vertex::set_ID(), id = " << id << std::endl;
   ID=id;
}

inline int vertex::get_ID() const
{
   return ID;
}

inline void vertex::set_posn(const threevector& posn)
{
   V=posn;
}

inline threevector& vertex::get_posn() 
{
   return V;
}

inline const threevector& vertex::get_posn() const
{
   return V;
}

// Above Z-plane member functions

inline bool vertex::above_Zplane(double z)
{
   const double eps=1E-4;
   return (V.get(2) > z-eps);
}

inline bool vertex::below_Zplane(double z)
{
   const double eps=1E-4;
   return (V.get(2) < z+eps);
}

#endif  // vertex.h
