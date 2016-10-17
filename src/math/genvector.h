// ==========================================================================
// Header file for genvector class 
// ==========================================================================
// Last modified on 5/31/13; 8/24/13; 2/8/16; 10/17/16
// ==========================================================================

#ifndef GENVECTOR_H
#define GENVECTOR_H

#include <string>
#include "math/genmatrix.h"

class descriptor;

class genvector:public genmatrix
{

  public:

// ---------------------------------------------------------------------
// Constructor function
// ---------------------------------------------------------------------

   genvector(int m);
   genvector(const descriptor& descript);
   genvector(const genmatrix& M);
   genvector(const tensor& T);
   genvector(const genvector& v);
   virtual ~genvector();
   void docopy(const genvector& v);
   genvector& operator= (const genvector& v);
   genvector& operator= (const genmatrix& m);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const genvector& X);
 
// ---------------------------------------------------------------------
// Member functions:
// ---------------------------------------------------------------------
   
// Vadim taught us in Dec 04 that there are subtle C++ compiler issues
// with overloading/overriding.  For reasons which even he didn't
// know, the get(int) method in this Genvector class can get confused
// with the get(int) method in Tensor.  So he told us to explicitly
// instruct the compiler that we will be using Tensor::get(int) method
// via the following using command:

   using tensor::put;
   using tensor::increment;
   using tensor::get;
 
   double dot(const genvector& X) const;
   double magnitude() const;
   double sqrd_magnitude() const;
   genvector unitvector() const;

   genvector hadamard_product(const genvector& Y) const;
   genvector hadamard_division(const genvector& Y) const;
   genvector hadamard_power(const genvector& Y, double alpha) const;
   genvector hadamard_power(double alpha) const;
   genmatrix outerproduct(const genvector& Y) const;
   void self_outerproduct(genmatrix& B) const;
   using tensor::outerproduct;

// Vector export member functions:

   void export_to_dense_text_format(std::string output_filename);
   void export_to_dense_binary_format(std::string output_filename);
   void export_to_sparse_text_format(std::string output_filename);
   void export_to_sparse_binary_format(std::string output_filename);

   void scale(const genvector& X);
   void operator+= (const genvector& X);
   void operator-= (const genvector& X);
   void operator*= (double a);
   void operator/= (double a);

// ---------------------------------------------------------------------
// Friend functions:
// ---------------------------------------------------------------------

   friend genvector operator+ (const genvector& X,const genvector& Y);
   friend genvector operator- (const genvector& X,const genvector& Y);
   friend genvector operator- (const genvector& X);
   friend genvector operator* (double a,const genvector& X);
   friend genvector operator* (const genvector& X,double a);
   friend genvector operator/ (const genvector& X,double a);
   friend genvector operator* (const genmatrix& A,const genvector& X);
   friend genvector operator* (const genvector& X,const genmatrix& A);

  private: 

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline double genvector::magnitude() const
{
   return sqrt(sqrd_magnitude());
}

#endif  // math/genvector.h





