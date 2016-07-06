// ==========================================================================
// Header file for regular_polygon class
// ==========================================================================
// Last modified on 4/14/06; 6/10/06; 8/6/06; 1/29/12
// ==========================================================================

#ifndef REGULAR_POLYGON_H
#define REGULAR_POLYGON_H

#include "polygon.h"

class rotation;

class regular_polygon:public polygon
{

  public:

// Initialization, constructor and destructor functions:

   regular_polygon(void);
   regular_polygon(int number_of_sides,double r);
   regular_polygon(int number_of_sides,double a,double b);
   regular_polygon(const regular_polygon& p);
   virtual ~regular_polygon();

   regular_polygon& operator= (const regular_polygon& p);

// Moving around regular polygon methods:

   virtual void translate(const threevector& rvec);
   virtual void absolute_position(const threevector& rvec);
   virtual void rotate(const rotation& R);
   virtual void rotate(const threevector& rotation_origin,const rotation& R);
   virtual void rotate(const threevector& rotation_origin,
                       double thetax,double thetay,double thetaz);

  private: 

   int nsides;
   double radius;
   threevector center;

   void docopy(const regular_polygon& p);
   void allocate_member_objects();
   void initialize_member_objects();

};

#endif  // regular_polygon.h


