// ==========================================================================
// Header file for permutation class 
// ==========================================================================
// Last modified on 2/13/06
// ==========================================================================

#ifndef PERMUTATION_H
#define PERMUTATION_H

class genmatrix;

class permutation
{

  public:

// Initialization, allocation and construction methods:

   permutation(const genmatrix& M);
   ~permutation();

   permutation& operator= (const permutation& m);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const permutation& A); 

// Set and get member functions:

   genmatrix* get_Morig_ptr();
   genmatrix* get_M_ptr();

// 

   int row_with_least_nulls(
      int m_min,int m_max,int n_min,int n_max);
   int row_with_first_null_value(int n);
   int column_with_first_null_value(int m);
   void swap_rows(int m1,int m2);
   void swap_columns(int n1,int n2);
   void sort_rows_by_nth_column(int n,bool descending_flag=true);
   void sort_columns_by_mth_row(int m,bool descending_flag=true);
   void restore_original_ordering();

  private: 

   int mdim,ndim;
   double null_value;
   genmatrix *M_ptr,*Morig_ptr;
   genmatrix *R_ptr,*C_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const permutation& m);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline genmatrix* permutation::get_Morig_ptr()
{
   return Morig_ptr;
}

inline genmatrix* permutation::get_M_ptr()
{
   return M_ptr;
}

#endif  // math/permutation.h




