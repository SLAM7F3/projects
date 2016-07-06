// ==========================================================================
// Header file for rpy class 
// ==========================================================================
// Last modified on 4/27/06; 10/9/11; 5/4/12
// ==========================================================================

#ifndef RPY_H
#define RPY_H

#include <iostream>
#include "math/basic_math.h"

class threevector;

class rpy
{

  public:

// Initialization, constructor and destructor functions:

   rpy();
   rpy(double r,double p,double y);
   rpy(const threevector& RollPitchYaw);
   rpy& operator= (const rpy& m);
   friend std::ostream& operator<< (std::ostream& outstream,const rpy& m);

// Set and get methods:

   void set_roll(double r);
   void set_pitch(double p);
   void set_yaw(double y);
   double get_roll() const;
   double get_pitch() const;
   double get_yaw() const;

   double magnitude() const;

// ---------------------------------------------------------------------
// Friend functions:
// ---------------------------------------------------------------------

   friend rpy operator+ (const rpy& X,const rpy& Y);
   friend rpy operator- (const rpy& X,const rpy& Y);
   friend rpy operator- (const rpy& X);
   friend rpy operator* (int a,const rpy& X);
   friend rpy operator* (double a,const rpy& X);

  private: 

   double roll,pitch,yaw;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const rpy& m);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void rpy::set_roll(double r)
{
   roll=r;
}

inline void rpy::set_pitch(double p)
{
   pitch=p;
}

inline void rpy::set_yaw(double y)
{
   yaw=y;
}

inline double rpy::get_roll() const
{
   return roll;
}

inline double rpy::get_pitch() const
{
   return pitch;
}

inline double rpy::get_yaw() const
{
   return yaw;
}

inline double rpy::magnitude() const
{
   return sqrt(sqr(roll)+sqr(pitch)+sqr(yaw));
}

#endif  // math/rpy.h




