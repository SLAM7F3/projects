// ==========================================================================
// Header file for templatized genarray class 
// ==========================================================================
// Last modified on 1/11/13; 3/28/14; 10/20/16; 12/20/16
// ==========================================================================

#ifndef T_GENARRAY_H
#define T_GENARRAY_H

//#include "math/complex.h"
#include "math/Tensor.h"

template <class A>
class Genarray:public Tensor<A>
{

  public:

// Initialization, constructor and destructor functions:

   Genarray(int m);
   Genarray(int m,int n);
   Genarray(Genarray<A> const *g_ptr);
   Genarray(const Genarray<A>& g);
   virtual ~Genarray();
   void docopy(const Genarray<A>& g);
   Genarray<A>& operator= (const Genarray<A>& g);

   template <class B>
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Genarray<B>& g);

// Set and get member functions:

   void set_mdim(int m);
   void set_ndim(int n);
   unsigned int get_mdim() const;
   unsigned int get_ndim() const;

   bool check_indices_validity(int m,int n) const;
   int indices_to_index(int m,int n) const;
   void index_to_indices(int index,int& m,int& n) const; 

// Vadim taught us in Dec 04 that there are subtle C++ compiler issues
// with overloading/overriding.  For reasons which even he didn't
// know, the get(int,int) method in this Genarray class can get
// confused with the get(int) method in Tensor.  So he told us to
// explicitly instruct the compiler that we will be using
// Tensor::get(int) method via the following using command:

   using Tensor<A>::put;
   using Tensor<A>::increment;
   using Tensor<A>::get;

// Related templatization note:

// In June 2005, we learned that as of version 3.4 of GCC, unqualified
// names in a template definition will no longer find members of a
// dependent base.  According to
// http://gcc.gnu.org/gcc-3.4/changes.html, it is strongly recommended
// that such names be made dependent by prefixing them with "this->".

//   void put(int m,int n,A value);
   void increment(int m,int n,A value);
//   A get(int m,int n) const;
   void get_row(int m,A row[]) const;
   std::vector<A> get_row(int m) const;
   void put_row(int m,const std::vector<A>& row);
   void get_column(int n,A column[]) const;
   std::vector<A> get_column(int n) const;
   void put_column(int n,const std::vector<A>& column);
   A* get_ptr(int m,int n) const;

   void generate_frequency_histogram(
      int n_inputs,int nxbins,double xminimum,double xmaximum,double x[],
      int nybins,double yminimum,double ymaximum,double y[],
      A value[]);

   A min_value() const;
   A min_value(int& m_min,int& n_min) const;
   A max_value() const;
   A max_value(int& m_max,int& n_max) const;

// ---------------------------------------------------------------------
// Friend functions:
// ---------------------------------------------------------------------

   template <class B>
   friend Genarray<B> operator+ (const Genarray<B>& X,const Genarray<B>& Y);
   template <class B>
   friend Genarray<B> operator- (const Genarray<B>& X,const Genarray<B>& Y);
   template <class B>
   friend Genarray<B> operator- (const Genarray<B>& X);
   template <class B>
   friend Genarray<B> operator* (A a,const Genarray<B>& X);
   template <class B>
   friend Genarray<B> operator* (const Genarray<B>& X,A a);

  protected:

// Integers mdim and ndim represent the "relevant working" dimensions
// of the genarray object.  

   unsigned int mdim,ndim;

  private: 

};

// typedef Genarray<complex> complexarray;
typedef Genarray<double> genarray;
typedef Genarray<float> genfloatarray;
typedef Genarray<std::string> genstringarray;

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

template <class A> inline void Genarray<A>::set_mdim(int m)
{
   mdim=m;
}

template <class A> inline void Genarray<A>::set_ndim(int n)
{
   ndim=n;
}

template <class A> inline unsigned int Genarray<A>::get_mdim() const
{
   return mdim;
}

template <class A> inline unsigned int Genarray<A>::get_ndim() const
{
   return ndim;
}

// ---------------------------------------------------------------------
// Boolean member function check_indices_validity returns true if
// input indices m and n lie within the allowed range for the current
// Genarray object:

template <class A> inline bool Genarray<A>::check_indices_validity(
   int m,int n) const
{
   bool indices_OK=true;
   if (m < 0 || n < 0 || m >= mdim || n >= ndim || this==NULL)
   {
      std::cout << "Error inside Genarray<A>::check_indices_validity()" 
                << std::endl;
      if (this==NULL)
      {
         std::cout << "this = NULL !!!" << std::endl;
      }
      else
      {
         std::cout << "m = " << m << " n = " << n << std::endl;
         std::cout << "mdim = " << mdim << " ndim = " << ndim << std::endl;
      }
      indices_OK=false;
   }
   return indices_OK;
}

// ---------------------------------------------------------------------
// Member function indices_to_index converts Genarray indices into an
// equivalent one-dimensonal array index value:

template <class A> inline int Genarray<A>::indices_to_index(int m,int n) const
{
   return ndim*m+n;
}

// ---------------------------------------------------------------------
// Member function index_to_indices performs the inverse operation to
// member function indices_to_index.  It returns the Genarray indices
// m and n corresponding to the one-dimensonal array index value:

template <class A> inline void Genarray<A>::index_to_indices(
   int index,int& m,int& n) const
{
   n=index%ndim;
   m=(index-n)/ndim;
}

// ---------------------------------------------------------------------
/*
template <class A> inline void Genarray<A>::put(int m,int n,A value)
{
#ifdef DEBUGFLAG   
   if (check_indices_validity(m,n))
#endif
   {
      this->put(ndim*m+n,value);
   }
}
*/

template <class A> inline void Genarray<A>::increment(int m,int n,A value)
{
#ifdef DEBUGFLAG
   if (check_indices_validity(m,n))
#endif
   {
      this->put(ndim*m+n,get(ndim*m+n)+value);
   }
}

/*
template <class A> inline A Genarray<A>::get(int m,int n) const
{
#ifdef DEBUGFLAG
   if (check_indices_validity(m,n))
#endif
   {
      return this->get(ndim*m+n);
   }
#ifdef DEBUGFLAG
   else
   {
      return NEGATIVEINFINITY;
   }
#endif
}
*/

template <class A> inline A* Genarray<A>::get_ptr(int m,int n) const
{
#ifdef DEBUGFLAG
   if (check_indices_validity(m,n))
#endif      
   {
      return &(this->e[ndim*m+n]);
   }
#ifdef DEBUGFLAG
   else
   {
      return NULL;
   }
#endif
}

template <class A> inline void Genarray<A>::get_row(int m,A row[]) const
{
   for (unsigned int j=0; j<ndim; j++)
   {
      row[j]=get(m,j);
   }
}

template <class A> inline std::vector<A> Genarray<A>::get_row(int m) const
{
   std::vector<A> row;
   for (unsigned int j=0; j<ndim; j++)
   {
      row.push_back(get(m,j));
   }
   return row;
}

template <class A> inline void Genarray<A>::put_row(
   int m,const std::vector<A>& row)
{
   for (int j=0; j<ndim; j++)
   {
      put(m,j,row[j]);
   }
   
}

template <class A> inline void Genarray<A>::get_column(
   int n,A column[]) const
{
   for (unsigned int i=0; i<mdim; i++)
   {
      column[i]=get(i,n);
   }
}

template <class A> inline std::vector<A> Genarray<A>::get_column(int n) const
{
   std::vector<A> column;
   for (unsigned int i=0; i<mdim; i++)
   {
      column.push_back(get(i,n));
   }
   return column;
}

template <class A> inline void Genarray<A>::put_column(
   int n,const std::vector<A>& column)
{
   for (unsigned int i=0; i<mdim; i++)
   {
      put(i,n,column[i]);
   }
}

#include "Genarray.cc"

#endif  // T_math/Genarray.h









