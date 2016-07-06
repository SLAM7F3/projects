// ==========================================================================
// Rpy class member function definitions
// ==========================================================================
// Last modified on 4/27/06; 10/9/11; 5/4/12
// =========================================================================

#include "math/threevector.h"
#include "math/rpy.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void rpy::allocate_member_objects()
{
}

void rpy::initialize_member_objects()
{
   roll=pitch=yaw=0;
}

rpy::rpy()
{
   initialize_member_objects();
}

rpy::rpy(double r,double p,double y)
{
   roll=r;
   pitch=p;
   yaw=y;
}

rpy::rpy(const threevector& RollPitchYaw)
{
   roll=RollPitchYaw.get(0);
   pitch=RollPitchYaw.get(1);
   yaw=RollPitchYaw.get(2);
}

// ---------------------------------------------------------------------
void rpy::docopy(const rpy & m)
{
   roll=m.roll;
   pitch=m.pitch;
   yaw=m.yaw;
}

// Overload = operator:

rpy& rpy::operator= (const rpy& m)
{
   if (this==&m) return *this;
   docopy(m);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const rpy& m)
{
   outstream << "roll = " << m.get_roll() 
             << " pitch = " << m.get_pitch()
             << " yaw = " << m.get_yaw() << endl;
   return(outstream);
}

// ==========================================================================
// Note: Keyword friend should appear in class declaration file and
// not within class member function definition file.  Friendly
// functions should not be declared as member functions of a class.
// So their definition syntax is not

// returntype classname::memberfunctioname(argument list)

// but rather

// returntype friendlyfunctionname(argument list) 

// We learned from Vadim on 5/28/02 that there is no inheritance for
// friend functions.  So the following section of code is lifted
// almost verbatim from the genvector class.
// ==========================================================================

// Overload + operator:

rpy operator+ (const rpy& X,const rpy& Y)
{
   return rpy(X.get_roll()+Y.get_roll(),
   X.get_pitch()+Y.get_pitch(),X.get_yaw()+Y.get_yaw());
}

// Overload - operator:

rpy operator- (const rpy& X,const rpy& Y)
{
   return rpy(X.get_roll()-Y.get_roll(),
   X.get_pitch()-Y.get_pitch(),X.get_yaw()-Y.get_yaw());
}

// Overload * operator:

rpy operator* (int a,const rpy& X)
{
   return rpy(a*X.get_roll(),a*X.get_pitch(),a*X.get_yaw());
}

rpy operator* (double a,const rpy& X)
{
   return rpy(a*X.get_roll(),a*X.get_pitch(),a*X.get_yaw());
}

rpy operator* (const rpy& X,double a)
{
   return a*X;
}
