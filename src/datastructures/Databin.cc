// ==========================================================================
// Templatized Databin class member function definitions
// ==========================================================================
// Last updated on 7/29/05; 7/2/07
// ==========================================================================

#include <iostream>
#include "templates/mytemplates.h"

// Initialization, constructor and destructor methods:
   
template <class T> Databin<T>::Databin(int id,const threevector& p)
{
   ID=id;
   posn=p;
}

template <class T> Databin<T>::Databin(int id,const threevector& p,const T& d)
{
   ID=id;
   posn=p;
   data=d;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,const Databin<T>& D)
{
   outstream << "Databin ID = " << D.get_ID() << std::endl;
   outstream << "posn = " << D.get_posn() << std::endl;
   outstream << "data = " << std::endl;
//   templatefunc::printVector(D.get_data());
   return outstream;
}
