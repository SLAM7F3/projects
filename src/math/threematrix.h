// ==========================================================================
// Header file for threematrix class 
// ==========================================================================
// Last modified on 1/29/12; 5/8/13
// ==========================================================================

#ifndef THREEMATRIX_H
#define THREEMATRIX_H

#include "math/genmatrix.h"

class rotation;
class threevector;

class threematrix:public genmatrix
{

  public:

// Initialization, allocation and construction functions

   threematrix();
   threematrix(const rotation& R);
   threematrix(const threematrix& m);
   ~threematrix();

   threematrix& operator= (const threematrix& m);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const threematrix& A); 

// Friend methods:

//   friend threematrix operator+ (const threematrix& A,const threematrix& B);
   friend threematrix operator- (const threematrix& A);
   friend threevector operator* (const threematrix& A,const threevector& X);
   friend threevector operator* (const threevector& X,const threematrix& A);
   friend threematrix operator* (const threematrix& A,const threematrix& B);


  protected:

   void docopy(const threematrix& m);

  private: 

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif  // math/threematrix.h




