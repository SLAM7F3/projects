// ==========================================================================
// Permutation class member function definitions
// ==========================================================================
// Last modified on 2/13/06
// =========================================================================

#include <iostream>
#include <vector>
#include "math/Genarray.h"
#include "math/genmatrix.h"
#include "math/permutation.h"

using std::cout;
using std::endl;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void permutation::allocate_member_objects()
{
   R_ptr=new genmatrix(mdim,mdim);
   C_ptr=new genmatrix(ndim,ndim);
}		       

void permutation::initialize_member_objects()
{
   null_value=NEGATIVEINFINITY;
   R_ptr->identity();
   C_ptr->identity();
}

permutation::permutation(const genmatrix& M)
{
   Morig_ptr=new genmatrix(M);
   M_ptr=new genmatrix(M);
   mdim=M_ptr->get_mdim();
   ndim=M_ptr->get_ndim();
   allocate_member_objects();
   initialize_member_objects();

//   cout << "inside permutation constructor, *M_ptr = " << *M_ptr << endl;
}

// Copy constructor

permutation::~permutation()
{
   delete Morig_ptr;
   delete M_ptr;
   delete R_ptr;
   delete C_ptr;
}

// ---------------------------------------------------------------------
void permutation::docopy(const permutation& m)
{
}	

// ---------------------------------------------------------------------
// Member function row_with_least_nulls loops over every row within
// *M_ptr and returns the one which contains the fewest number of
// null_values.

int permutation::row_with_least_nulls(
   int m_min,int m_max,int n_min,int n_max)
{
   int rwln=-1;
   int min_n_nulls=m_max-m_min;

   for (int m=m_min; m<m_max; m++)
   {
      int n_nulls=0;
      for (int n=n_min; n<n_max; n++)
      {
         if (nearly_equal(M_ptr->get(m,n),null_value))
         {
            n_nulls++;
         }
      } // loop over index n labeling columns
      if (n_nulls < min_n_nulls)
      {
         min_n_nulls=n_nulls;
         rwln=m;
      }
   } // loop over index m labeling rows
   return rwln;
}

// ---------------------------------------------------------------------
// Member function row_with_first_null_value loops over the nth row
// within *M_ptr.  It returns the index m of the first row containing
// a null value within the nth column.  If no null value is
// encountered, this method returns -1.

int permutation::row_with_first_null_value(int n)
{
   int m_null=-1;
   for (int m=0; m<mdim; m++)
   {
      if (nearly_equal(M_ptr->get(m,n),null_value))
      {
         m_null=m;
         break;
      }
   } // loop over index m labeling rows
   return m_null;
}

// ---------------------------------------------------------------------
// Member function column_with_first_null_value loops over the mth row
// within *M_ptr.  It returns the index n of the first column
// containing a null value within the mth row.  If no null value is
// encountered, this method returns -1.

int permutation::column_with_first_null_value(int m)
{
   int n_null=-1;
   for (int n=0; n<ndim; n++)
   {
      if (nearly_equal(M_ptr->get(m,n),null_value))
      {
         n_null=n;
         break;
      }
   } // loop over index n labeling columns
   return n_null;
}

// ---------------------------------------------------------------------
// Member function swap_rows interchanges rows m1 and m2 within
// *M_ptr.  It also multiplies *R_ptr on the left by the permutation
// which implements this m1 <--> m2 swap.

void permutation::swap_rows(int m1,int m2)
{
   genmatrix R_latest(mdim,mdim);
   for (int m=0; m<mdim; m++)
   {
      if (m==m1)
      {
         R_latest.put(m,m2,1);
      }
      else if (m==m2)
      {
         R_latest.put(m,m1,1);
      }
      else
      {
         R_latest.put(m,m,1);
      }
   } // loop over index m
   *M_ptr = R_latest * (*M_ptr);
   *R_ptr = R_latest * (*R_ptr);

//   cout << "After swapping rows, *M_ptr = " << *M_ptr << endl;
//   cout << "*R_ptr = " << *R_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function swap_columns interchanges columns n1 and n2 m2
// within *M_ptr.  It also multiplies *C_ptr on the right by the
// permutation which implements this n1 <--> n2 swap.

void permutation::swap_columns(int n1,int n2)
{
   genmatrix C_latest(ndim,ndim);
   for (int n=0; n<ndim; n++)
   {
      if (n==n1)
      {
         C_latest.put(n2,n,1);
      }
      else if (n==n2)
      {
         C_latest.put(n1,n,1);
      }
      else
      {
         C_latest.put(n,n,1);
      }
   } // loop over index n
   *M_ptr = (*M_ptr) * C_latest;
   *C_ptr = (*C_ptr) * C_latest;

//   cout << "After swapping columns, *M_ptr = " << *M_ptr << endl;
//   cout << "*C_ptr = " << *C_ptr << endl;

}

// ---------------------------------------------------------------------
// Member function sort_rows_by_nth_column takes in integer n labeling
// some column within *M_ptr.  It sorts the entires within the nth
// column and generates a row permutation matrix R_latest
// corresponding to the sorted values.  This method applies R_latest
// to *M_ptr and left multiplies *R_ptr by R_latest.

void permutation::sort_rows_by_nth_column(int n,bool descending_flag)
{
   vector<double> column=M_ptr->Genarray<double>::get_column(n);

   vector<int> row_label;
   for (int m=0; m<mdim; m++)
   {
      row_label.push_back(m);
   }
   
   if (descending_flag)
   {
      templatefunc::Quicksort_descending(column,row_label);
   }
   else
   {
      templatefunc::Quicksort(column,row_label);
   }
   
//   templatefunc::printVector(column);
//   cout << endl;
//   templatefunc::printVector(row_label);
   
   genmatrix R_latest(mdim,mdim);
   for (int m=0; m<mdim; m++)
   {
      R_latest.put(m,row_label[m],1);
   }

   *M_ptr = R_latest * (*M_ptr);
   *R_ptr = R_latest * (*R_ptr);
   
//   cout << "After sorting rows, *M_ptr = " << *M_ptr << endl;
//   cout << "*R_ptr = " << *R_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function sort_columns_by_mth_row takes in integer m labeling
// some row within *M_ptr.  It sorts the entries within the mth row
// and generates a column permutation matrix C_latest corresponding to
// the sorted values.  This method applies C_latest to *M_ptr and
// right multiplies cumulative *C_ptr by C_latest.

void permutation::sort_columns_by_mth_row(int m,bool descending_flag)
{
   vector<double> row=M_ptr->Genarray<double>::get_row(m);

   vector<int> column_label;
   for (int n=0; n<ndim; n++)
   {
      column_label.push_back(n);
   }
   
   if (descending_flag)
   {
      templatefunc::Quicksort_descending(row,column_label);
   }
   else
   {
      templatefunc::Quicksort(row,column_label);
   }
   
//   templatefunc::printVector(row);
//   cout << endl;
//   templatefunc::printVector(column_label);
   
   genmatrix C_latest(ndim,ndim);
   for (int n=0; n<ndim; n++)
   {
      C_latest.put(column_label[n],n,1);
   }

   *M_ptr = (*M_ptr) * C_latest;
   *C_ptr = (*C_ptr) * C_latest;

//   cout << "After sorting columns, *M_ptr = " << *M_ptr << endl;
//   cout << "*C_ptr = " << *C_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function restore_original_ordering multiplies the current
// matrix within *M_ptr by the inverses ( = transposes) of the
// cumulative row and column permutation matrices.  The result is
// stored within *Morig_ptr.

void permutation::restore_original_ordering()
{
   *Morig_ptr= R_ptr->transpose() * (*M_ptr) * C_ptr->transpose();
}
