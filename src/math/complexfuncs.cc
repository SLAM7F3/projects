// ==========================================================================
// Stand-alone complex variable methods
// ==========================================================================
// Last updated on 3/31/12; 4/2/12; 4/8/12
// ==========================================================================

#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include "math/complexfuncs.h"
#include "math/genmatrix.h"
#include "math/mypolynomial.h"                 

using std::cin;
using std::cout;
using std::endl;
using std::pair;
using std::vector;
using Eigen::MatrixXd;

namespace complexfunc
{

// Method find_real_polynomial_roots() takes in N-th order polynomial
// poly.  It computes all of its N complex roots.  But this method
// returns only those roots which are real within an STL vector.

   vector<double> find_real_polynomial_roots(const mypolynomial& poly)
   {
      vector<complex> poly_roots=find_polynomial_roots(poly);

      vector<double> real_poly_roots;
      for (unsigned int i=0; i<poly_roots.size(); i++)
      {
         if (poly_roots[i].is_real()) 
            real_poly_roots.push_back(poly_roots[i].get_real());
      }
      
      return real_poly_roots;
   }

// --------------------------------------------------------------------------
// Method find_polynomial_roots() returns N complex roots for the
// input Nth-order polynomial with an STL vector.

   vector<complex> find_polynomial_roots(const mypolynomial& poly)
   {
      genmatrix* companion_matrix_ptr=poly.construct_companion_matrix();
      vector<complex> poly_roots=compute_eigenvalues(companion_matrix_ptr);
      delete companion_matrix_ptr;
      return poly_roots;
   }

// --------------------------------------------------------------------------
// Method compute_eigenvalues() takes in an nxn matrix *M_ptr.  It
// returns the matrix eigenvalues within an STL vector of complex
// numbers.

   vector<complex> compute_eigenvalues(genmatrix* M_ptr)
   {
//      cout << "inside complexfunc::compute_eigenvalues()" << endl;
      unsigned int mdim=M_ptr->get_mdim();
      MatrixXd M(mdim,mdim);

      for (unsigned int i=0; i<M_ptr->get_mdim(); i++)
      {
         for (unsigned int j=0; j<M_ptr->get_ndim(); j++)
         {
            M(i,j)=M_ptr->get(i,j);
         } // loop over index j 
      } // loop over index i 
    
      Eigen::EigenSolver<MatrixXd> es(M);

      vector<complex> eigenvalues;
      for (unsigned int l=0; l<mdim; l++)
      {
         eigenvalues.push_back(es.eigenvalues()[l]);
//         cout << "l = " << l 
//              << " eigenvalue = " << eigenvalues[l] << endl;
      }
      return eigenvalues;
   }

// --------------------------------------------------------------------------
// Method eigen_system() decomposes input matrix *M_ptr into its
// eigenvalues and eigenvectors.  If the eigenvectors are all real,
// they are returned within the columns of an output genmatrix.
// Otherwise, the pointer to the output genmatrix is set to NULL.

   pair<vector<complex>,genmatrix*> eigen_system(genmatrix* M_ptr)
   {
      cout << "inside complexfunc::eigen_system()" << endl;
      unsigned int mdim=M_ptr->get_mdim();
      MatrixXd M(mdim,mdim);

      for (unsigned int i=0; i<M_ptr->get_mdim(); i++)
      {
         for (unsigned int j=0; j<M_ptr->get_ndim(); j++)
         {
            M(i,j)=M_ptr->get(i,j);
         } // loop over index j 
      } // loop over index i 
    
      Eigen::EigenSolver<MatrixXd> es(M);
//      cout << "The eigenvalues of M are:" << endl << es.eigenvalues() << endl;
//      cout << "The matrix of eigenvectors, V, is:" << endl 
//           << es.eigenvectors() << endl << endl;

//      Eigen::VectorXcd v = es.eigenvectors().col(0);

//      Eigen::MatrixXcd D = es.eigenvalues().asDiagonal();
//      Eigen::MatrixXcd V = es.eigenvectors();
//      Eigen::MatrixXcd Vinverse = V.inverse();

      vector<complex> eigenvalues;
      for (unsigned int l=0; l<mdim; l++)
      {
         eigenvalues.push_back(es.eigenvalues()[l]);
      }

      genmatrix* eigenvectors_ptr=new genmatrix(mdim,mdim);
      for (unsigned int i=0; i<mdim; i++)
      {
         Eigen::VectorXcd curr_v = es.eigenvectors().col(i);
         for (unsigned int j=0; j<mdim; j++)
         {
            complex z(curr_v(j));
            if (!z.is_real())
            {
               delete eigenvectors_ptr;
               eigenvectors_ptr=NULL;
               break;
            }
            else
            {
               eigenvectors_ptr->put(j,i,z.get_real());
            }
         } // loop over index j
      } // loop over index i

      cout << "eigenvectors_ptr = " << eigenvectors_ptr << endl;
      pair<vector<complex>,genmatrix*> P(eigenvalues,eigenvectors_ptr);
      return P;
   }
   
// --------------------------------------------------------------------------
// Method polynomial_value() takes in the (complex) coefficient for
// the higest order term in a polynomial along with all its (complex)
// roots.  This method returns the (complex) value of the polynomial
// evaluated at (complex) z.

   complex polynomial_value(
      double x,double c_order,const vector<complex>& poly_roots)
   {
      return polynomial_value(complex(x),complex(c_order),poly_roots);
   }

   complex polynomial_value(
      complex z,complex c_order,const vector<complex>& poly_roots)
   {
//      cout << "inside complexfunc::polynomial_value()" << endl;
//      cout << "Input z = " << z << endl;
//      cout << "poly_roots.size() = " << poly_roots.size() << endl;

      complex poly_value(c_order);
      for (unsigned int i=0; i<poly_roots.size(); i++)
      {
         poly_value *= (z-poly_roots[i]);
      }
      return poly_value;
   }
   

} // complexfunc namespace

