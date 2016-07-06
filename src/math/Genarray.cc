// ==========================================================================
// Genarray class member function definitions
// ==========================================================================
// Last modified on 3/30/09; 5/25/10; 1/11/13
// =========================================================================

#include "math/mathfuncs.h"

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

template <class A> Genarray<A>::Genarray(int m):
   Tensor<A>(m)
{
   mdim=m;
   ndim=1;

   if (mdim <= 0)
   {
      std::cout << "Error in Genarray constructor #1" << std::endl;
      std::cout << "mdim = " << mdim << " ndim = " << ndim << std::endl;
   }
   
}

template <class A> Genarray<A>::Genarray(int m,int n):
   Tensor<A>(m,n)
{
   mdim=m;
   ndim=n;

   if (mdim <= 0 || ndim <= 0)
   {
      std::cout << "Error in Genarray constructor #2" << std::endl;
      std::cout << "mdim = " << mdim << " ndim = " << ndim << std::endl;
   }
}

// This next constructor takes in a pointer to an already existing
// Genarray object and builds a new object with the same pixel and
// (x,y) dimensions:


template <class A> Genarray<A>::Genarray(Genarray<A> const *g_ptr):
   Tensor<A>(g_ptr->mdim,g_ptr->ndim)
{
   Tensor<A>::docopy(*g_ptr);
   docopy(*g_ptr);
}

// Copy constructor:

template <class A> Genarray<A>::Genarray(const Genarray<A>& g):
   Tensor<A>(g)
{
   docopy(g);
}

template <class A> Genarray<A>::~Genarray()
{
}

// ---------------------------------------------------------------------
template <class A> void Genarray<A>::docopy(const Genarray<A>& g)
{
   mdim=g.mdim;
   ndim=g.ndim;
   Tensor<A>::docopy(g);
}

// Overload = operator:

template <class A> Genarray<A>& Genarray<A>::operator= (const Genarray<A>& g)
{
   if (this==&g) return *this;
   Tensor<A>::operator=(g);
   docopy(g);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class A> std::ostream& operator<< 
(std::ostream& outstream,const Genarray<A>& g)
{
   outstream << std::endl;
   for (unsigned int i=0; i<g.mdim; i++)
   {
      for (unsigned int j=0; j<g.ndim; j++)
      {
         outstream << "i = " << i << " j = " << j 
                   << " g(i,j) = " << g.get(i,j) << std::endl;
      }
   }
   outstream << std::endl;

   int i,j;
   for (unsigned int index=0; index<g.get_dimproduct(); index++)
   {
      g.index_to_indices(index,i,j);
      outstream << "Genarray indices = " << i << "\t" << j << "\t";
      outstream << " Genarray value = " << g.get(i,j) << std::endl;
   }
   outstream << "mdim = " << g.mdim << " ndim = " << g.ndim << std::endl;
   return(outstream);
}

// ---------------------------------------------------------------------
// Member function generate_frequency_histogram fills the current
// Genarray object with integrated values based upon the three same
// sized and correlated input arrays x and y and value.

template <class A> void Genarray<A>::generate_frequency_histogram(
   int n_inputs,int nxbins,double xminimum,double xmaximum,double x[],
   int nybins,double yminimum,double ymaximum,double y[],
   A value[]) 
{
   double dx=(xmaximum-xminimum)/(nxbins-1);
   double dy=(ymaximum-yminimum)/(nybins-1);
   
// Clear and then fill x histogram with data values:

   this->clear_values();

   for (int n=0; n<n_inputs; n++)
   {
      int i=basic_math::round((x[n]-xminimum)/dx);
      int j=basic_math::round((y[n]-yminimum)/dy);

      if (i < 0 || i > nxbins || j < 0 || j > nybins)
      {
//         std::cout << "Trouble in Genarray<A>::generate_frequency_histogram()" 
//              << std::endl;
//         std::cout << "While histogram, i = " << i << " j = " << j << std::endl;
//         std::cout << "n = " << n << " x[n] = " << x[n] << " y[n] = " << y[n]
//              << std::endl;
//         std::cout << "xminimum = " << xminimum << " dx = " << dx << std::endl;
//         std::cout << "yminimum = " << yminimum << " dy = " << dy << std::endl;
      }
      else
      {
         increment(i,j,value[n]);
      }
   } // loop over index n
}

// ---------------------------------------------------------------------
// Member functions min_value and max_value return the extremal array
// values as well as their bin coordinates:

template <class A> A Genarray<A>::min_value() const
{
   int m_min,n_min;
   return min_value(m_min,n_min);
}

template <class A> A Genarray<A>::min_value(int& m_min,int& n_min) const
{
   A minvalue=POSITIVEINFINITY;
   A currvalue;
   for (int m=0; m<mdim; m++)
   {
      for (int n=0; n<ndim; n++)
      {
         currvalue=get(m,n);
         if (currvalue < minvalue)
         {
            minvalue=currvalue;
            m_min=m;
            n_min=n;
         }
      }
   }
   return minvalue;
}

template <class A> A Genarray<A>::max_value() const
{
   int m_max,n_max;
   return max_value(m_max,n_max);
}

template <class A> A Genarray<A>::max_value(int& m_max,int& n_max) const
{
   A maxvalue=NEGATIVEINFINITY;
   A currvalue;
   for (int m=0; m<mdim; m++)
   {
      for (int n=0; n<ndim; n++)
      {
         currvalue=get(m,n);
         if (currvalue > maxvalue)
         {
            maxvalue=currvalue;
            m_max=m;
            n_max=n;
         }
      }
   }
   return maxvalue;
}

// ==========================================================================
// Note: Keyword friend should appear in class declaration file and
// not within class member function definition file.  Fristd::endly
// functions should not be declared as member functions of a class.
// So their definition syntax is not

// returntype classname::memberfunctioname(argument list)

// but rather

// returntype fristd::endlyfunctionname(argument list)
// ==========================================================================

// Overload + operator:

template <class B> Genarray<B> operator+ (
   const Genarray<B>& X,const Genarray<B>& Y)
{
   if (X.mdim==Y.mdim && X.ndim==Y.ndim)
   {
      Genarray<B> Z(X.mdim,X.ndim);

      for (int i=0; i<X.mdim; i++)
      {
         for (int j=0; j<X.ndim; j++)
         {
            Z.put(i,j,X.get(i,j)+Y.get(i,j));
         }
      }
      return Z;
   }
   else
   {
      std::cout << "Error inside operator+ in Genarray.cc class!" << std::endl;
      std::cout << "X.mdim = " << X.mdim << " X.ndim = " << X.ndim << std::endl;
      std::cout << "Y.mdim = " << Y.mdim << " Y.ndim = " << Y.ndim << std::endl;
      std::cout << "Cannot add together two Genarrays of different dimensions!"
           << std::endl;
      exit(-1);
   }
}

// Overload - operator:

template <class B> Genarray<B> operator- (
   const Genarray<B>& X,const Genarray<B>& Y)
{
   if (X.mdim==Y.mdim && X.ndim==Y.ndim)
   {
      Genarray<B> Z(X.mdim,X.ndim);

      for (int i=0; i<X.mdim; i++)
      {
         for (int j=0; j<X.ndim; j++)
         {
            Z.put(i,j,X.get(i,j)-Y.get(i,j));
         }
      }
      return Z;
   }
   else
   {
      std::cout << "Error inside operator- in Genarray.cc class!" << std::endl;
      std::cout << "X.mdim = " << X.mdim << " X.ndim = " << X.ndim << std::endl;
      std::cout << "Y.mdim = " << Y.mdim << " Y.ndim = " << Y.ndim << std::endl;
      std::cout << "Cannot subtract two Genarrays of different dimensions!"
           << std::endl;
      exit(-1);
   }
}

// Overload - operator:

template <class B> Genarray<B> operator- (const Genarray<B>& X)
{
   Genarray<B> Y(X.mdim,X.ndim);
	 
   for (int i=0; i<X.mdim; i++)
   {
      for (int j=0; j<X.ndim; j++)
      {
         Y.put(i,j,-X.get(i,j));
      }
   }
   return Y;
}

// Overload * operator for multiplying a Genarray by a scalar

template <class A,class B> Genarray<B> operator* (A a,const Genarray<B>& X)
{
   Genarray<B> Y(X.mdim,X.ndim);

   for (int i=0; i<X.mdim; i++)
   {
      for (int j=0; j<X.ndim; j++)
      {
         Y.put(i,j,a*X.get(i,j));
      }
   }
   return Y;
}

template <class A,class B> Genarray<B> operator* (
   const Genarray<B>& X,A a)
{
   return a*X;
}





