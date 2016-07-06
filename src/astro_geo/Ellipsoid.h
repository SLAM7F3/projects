// ==========================================================================
// Self-contained header file for Ellipsoid class
// ==========================================================================
// Last modified on 2/26/04; 6/19/06
// ==========================================================================

#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <string>

class Ellipsoid
{
  public:

   Ellipsoid(){};
   Ellipsoid(int Id,std::string name,double radius,double ecc)
      {
         id = Id; 
         ellipsoidName = name;
         EquatorialRadius = radius; 
         eccentricitySquared = ecc;
      }

   double get_EquatorialRadius() const;
   double get_eccentricitySquared() const;

  private:

   int id;
   std::string ellipsoidName;
   double EquatorialRadius;
   double eccentricitySquared;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline double Ellipsoid::get_EquatorialRadius() const
{
   return EquatorialRadius;
}

inline double Ellipsoid::get_eccentricitySquared() const
{
   return eccentricitySquared;
}

#endif // ELLIPSOID.h
