// ==========================================================================
// Templatized methods
// ==========================================================================
// Last updated on 8/12/07; 9/3/09; 10/25/10; 3/22/14
// ==========================================================================

#include <cstring>

namespace templatefunc
{
   template <class T> inline void swap(T& A, T& B)
      {
//         if (A != B)
         {
            T C=A;
            A=B;
            B=C;
         }
      }

// ==========================================================================

/*

// This first version of Quicksort is deprecated.  Instead, use STL
// sort algorithm to sort STL vectors!

// Syntax:  std::sort(foo.begin(),foo.end())  where foo = STL vector

   template <class T> void Quicksort(std::vector<T>& A,int n)
      {
         if (n > 1)
         {
            std::cout << "inside Quicksort(vector,n)" << std::endl;
            std::cout << "n = " << n << std::endl;

            templatefunc::mysort(A,0,n-1);
            

         }
      }
*/

// Sort first n objects inside vector A into ascending order, and make
// the corresponding rearrangement of vector B:
   
   template <class T,class U> void Quicksort(
      std::vector<T>& A,std::vector<U>& B)
      {
         if (A.size()==B.size())
         {
            templatefunc::mysort(A,B,0,A.size()-1);
         }
         else
         {
            std::cout << "Error in templatefunc::Quicksort()" << std::endl;
            std::cout << "A.size() = " << A.size() << " does not equal ";
            std::cout << "B.size() = " << B.size() << " !" << std::endl;
         }
      }

// Sort first n objects inside vector A into descending order, and
// make the corresponding rearrangement of vector B:

   template <class T,class U> void Quicksort_descending(
      std::vector<T>& A,std::vector<U>& B)
      {
         if (A.size()==B.size())
         {
            std::vector<T> Atmp;
            std::vector<U> Btmp;

            for (unsigned int i=0; i<A.size(); i++)
            {
               Atmp.push_back(A[i]);
               Btmp.push_back(B[i]);
            }
            Quicksort(Atmp,Btmp);
            for (unsigned int i=0; i<A.size(); i++)
            {
               A[i]=Atmp[A.size()-1-i];
               B[i]=Btmp[A.size()-1-i];
            }
         } 
         else
         {
            std::cout << "Error in templatefunc::Quicksort_descending()" 
                      << std::endl;
            std::cout << "A.size() = " << A.size() << " does not equal ";
            std::cout << "B.size() = " << B.size() << " !" << std::endl;
         }
      }

// Sort first n objects inside vector A into ascending order, and make
// corresponding rearrangements of vectors B and C:

   template <class T,class U,class V> void Quicksort(
      std::vector<T>& A,std::vector<U>& B,std::vector<V>& C)
      {
         std::vector<T> Acopy;
         Acopy.reserve(A.size());
         for (unsigned int i=0; i<A.size(); i++)
         {
            Acopy.push_back(A[i]);
         }
         Quicksort(A,B);
         Quicksort(Acopy,C);
      }

   template <class T,class U,class V> void Quicksort_descending(
      std::vector<T>& A,std::vector<U>& B,std::vector<V>& C)
      {
         std::vector<T> Acopy;
         Acopy.reserve(A.size());
         for (unsigned int i=0; i<A.size(); i++)
         {
            Acopy.push_back(A[i]);
         }
         Quicksort_descending(A,B);
         Quicksort_descending(Acopy,C);
      }

   template <class T,class U,class V,class W> 
      void Quicksort(std::vector<T>& A,std::vector<U>& B,
                     std::vector<V>& C,std::vector<W>& D)
      {
         std::vector<T> Acopy;
         Acopy.reserve(A.size());
         for (unsigned int i=0; i<A.size(); i++)
         {
            Acopy.push_back(A[i]);
            
         }
         Quicksort(A,B,C);
         Quicksort(Acopy,D);
      }

   template <class T,class U,class V,class W> void Quicksort_descending(
      std::vector<T>& A,std::vector<U>& B,std::vector<V>& C,
      std::vector<W>& D)
      {
         std::vector<T> Acopy;
         Acopy.reserve(A.size());
         for (unsigned int i=0; i<A.size(); i++)
         {
            Acopy.push_back(A[i]);
         }
         Quicksort_descending(A,B,C);
         Quicksort_descending(Acopy,D);
      }

   template <class T,class U,class V,class W,class X> 
      void Quicksort(std::vector<T>& A,std::vector<U>& B,
                     std::vector<V>& C,std::vector<W>& D,std::vector<X>& E)
      {
         std::vector<T> Acopy;
         Acopy.reserve(A.size());
         for (unsigned int i=0; i<A.size(); i++)
         {
            Acopy.push_back(A[i]);
            
         }
         Quicksort(A,B,C,D);
         Quicksort(Acopy,E);
      }

   template <class T,class U,class V,class W,class X> 
      void Quicksort_descending(
         std::vector<T>& A,std::vector<U>& B,std::vector<V>& C,
         std::vector<W>& D,std::vector<X>& E)
      {
         std::vector<T> Acopy;
         Acopy.reserve(A.size());
         for (unsigned int i=0; i<A.size(); i++)
         {
            Acopy.push_back(A[i]);
         }
         Quicksort_descending(A,B,C,D);
         Quicksort_descending(Acopy,E);
      }

   template <class T,class U,class V,class W,class X,class Y> 
      void Quicksort(std::vector<T>& A,std::vector<U>& B,
                     std::vector<V>& C,std::vector<W>& D,std::vector<X>& E,
                     std::vector<Y>& F)
      {
         std::vector<T> Acopy;
         Acopy.reserve(A.size());
         for (unsigned int i=0; i<A.size(); i++)
         {
            Acopy.push_back(A[i]);
            
         }
         Quicksort(A,B,C,D,E);
         Quicksort(Acopy,F);
      }

   template <class T,class U,class V,class W,class X,class Y> 
      void Quicksort_descending(std::vector<T>& A,std::vector<U>& B,
                     std::vector<V>& C,std::vector<W>& D,std::vector<X>& E,
                     std::vector<Y>& F)
      {
         std::vector<T> Acopy;
         Acopy.reserve(A.size());
         for (unsigned int i=0; i<A.size(); i++)
         {
            Acopy.push_back(A[i]);
            
         }
         Quicksort_descending(A,B,C,D,E);
         Quicksort_descending(Acopy,F);
      }

   template <class T,class U,class V,class W,class X,class Y,class Z> 
      void Quicksort(std::vector<T>& A,std::vector<U>& B,
                     std::vector<V>& C,std::vector<W>& D,
                     std::vector<X>& E,std::vector<Y>& F,
                     std::vector<Z>& G)
      {
         std::vector<T> Acopy;
         Acopy.reserve(A.size());
         for (unsigned int i=0; i<A.size(); i++)
         {
            Acopy.push_back(A[i]);
            
         }
         Quicksort(A,B,C,D,E,F);
         Quicksort(Acopy,G);
      }

   template <class S,class T,class U,class V,class W,class X,class Y,class Z> 
      void Quicksort(std::vector<S>& A,std::vector<T>& B,
                     std::vector<U>& C,std::vector<V>& D,
                     std::vector<W>& E,std::vector<X>& F,
                     std::vector<Y>& G,std::vector<Z>& H)
      {
         std::vector<T> Acopy;
         Acopy.reserve(A.size());
         for (unsigned int i=0; i<A.size(); i++)
         {
            Acopy.push_back(A[i]);
         }
         Quicksort(A,B,C,D,E,F,G);
         Quicksort(Acopy,H);
      }

   template <class R,class S,class T,class U,class V,class W,class X,class Y,class Z> 
      void Quicksort(std::vector<R>& A,std::vector<S>& B,
                     std::vector<T>& C,std::vector<U>& D,
                     std::vector<V>& E,std::vector<W>& F,
                     std::vector<X>& G,std::vector<Y>& H,
                     std::vector<Z>& I)
      {
         std::vector<T> Acopy;
         Acopy.reserve(A.size());
         for (unsigned int j=0; j<A.size(); j++)
         {
            Acopy.push_back(A[j]);
         }
         Quicksort(A,B,C,D,E,F,G,H);
         Quicksort(Acopy,I);
      }

   template <class T,class U> void mysort(
      std::vector<T>& A,std::vector<U>& B,int L,int R)
      {
         if (A.size() > 0 && A.size()==B.size())
         {
            int i=L;
            int j=R;
            T key=A[(L+R)/2];

            do
            {
               while (A[i] < key) i++;
               while (key < A[j]) j--;
      
               if (i <= j)
               {
                  templatefunc::swap(A[i],A[j]);
                  templatefunc::swap(B[i],B[j]);
                  i++;
                  j--;
               }
            }
            while (i<=j);

            if (L<j) templatefunc::mysort(A,B,L,j);
            if (j+1 < R) templatefunc::mysort(A,B,i,R);
         } //  (A.size() > 0 && A.size()==B.size()) conditional
      }

// ==========================================================================
// Array averaging methods:
// ==========================================================================

   template <class T> T average(T A[],const int nbins)
      {
         T avg;
         set_to_null(avg);
         for (int i=0; i<nbins; i++)
         {
            avg += A[i];
         }
         avg /= double(nbins);
         return avg;
      }

// This overloaded version of method average returns a mean 
// weighted by the values in input array w:

   template <class T> T average(T A[],double w[],int nbins)
      {
         T numer;
         set_to_null(numer);
         double denom=0;
         for (int i=0; i<nbins; i++)
         {
            numer += w[i]*A[i];
            denom += w[i];
         }
         T weighted_avg=numer/denom;

         return weighted_avg;
      }

// This overloaded version of method average computes a mean value
// for a function A given UNEVENLY sampled A(x) values.  It
// essentially implements a poor-man's Simpson's integration of A(x)
// from x[0] to x[Nsize-1].  On 10/10/02, we explicitly checked that
// it is better to use this algorithm to find the true average of
// unevenly distributed linear data than simply divide the sum of the
// function values by the total number of points...

   template <class T> T average(int nbins,double x[],T A[])
      {
         double *deltax;
         new_clear_array(deltax,nbins);
         deltax[0]=0.5*(x[1]-x[0]);
         for (int i=1; i<=nbins-2; i++)
         {
            deltax[i]=0.5*(x[i+1]-x[i-1]);
         }
         deltax[nbins-1]=0.5*(x[nbins-1]-x[nbins-2]);   

         double denom=0;
         for (int i=0; i<nbins; i++)
         {
            denom += deltax[i];
         }

         double w[nbins];
         T weighted_avg;
         set_to_null(weighted_avg);
         for (int i=0; i<nbins; i++)
         {
            w[i]=deltax[i]/denom;
            weighted_avg += w[i]*A[i];
         }
         delete [] deltax;
         return weighted_avg;
      }

   template <class T> T average(int nbins,double x[],T A[],T dA[],T& dA_avg)
      {
         double *deltax;
         new_clear_array(deltax,nbins);
         deltax[0]=0.5*(x[1]-x[0]);
         for (int i=1; i<=nbins-2; i++)
         {
            deltax[i]=0.5*(x[i+1]-x[i-1]);
         }
         deltax[nbins-1]=0.5*(x[nbins-1]-x[nbins-2]);   

         double denom=0;
         for (int i=0; i<nbins; i++)
         {
            denom += deltax[i];
         }

         double w[nbins];
         T weighted_avg;
         T weighted_sqr_error;
         set_to_null(weighted_avg);
         set_to_null(weighted_sqr_error);
         for (int i=0; i<nbins; i++)
         {
            w[i]=deltax[i]/denom;
            weighted_avg += w[i]*A[i];
            weighted_sqr_error += sqr(w[i]*dA[i]);
         }
         delete [] deltax;

         dA_avg=sqrt(weighted_sqr_error);
         return weighted_avg;
      }

// ==========================================================================
// First order alpha filter:

   template <class T> void alpha_filter(
      const T& currx,T filtered_x[],double alpha)
      {

// First move previous time's filtered x value from 0th to 1st
// position in filtered array:

         filtered_x[1]=filtered_x[0];

// Filtered estimate for position at present time:
         filtered_x[0]=alpha*currx+(1-alpha)*(filtered_x[1]);
      }

// ==========================================================================
// STL methods:
// ==========================================================================

// Next method copied from Deitel & Deitel, chapter 20, page 987.

   template <class T> void printVector(const std::vector<T>& v)
      {
         typename std::vector<T>::const_iterator p1;
         for (p1=v.begin(); p1 != v.end(); p1++)
         {
            std::cout << *p1 << std::endl;
         }
      }

} // templatefunc namespace

// ==========================================================================
// Array allocation and clearing method:
// ==========================================================================

template <class T> void new_clear_array(T*& T_ptr,int nsize)
{
   T_ptr=new T[nsize];
   int number_of_bytes=nsize*sizeof(T);
   memset(T_ptr,0,number_of_bytes);
}

template <class T> void clear_array(T* T_ptr,int nsize)
{
   int number_of_bytes=nsize*sizeof(T);
   memset(T_ptr,0,number_of_bytes);
}

// ==========================================================================
// Object nulling methods:
// ==========================================================================

// Templatized method is_primitive_type works in conjunction with
// the "specialized" is_primitive_type methods sitting inside
// genfuncs.cc.  Basically, we want ints, floats and doubles to be
// identified as primitives.  All other objects such as vectors and
// matrices are not considered to be primitives.  This little set of
// methods is useful for nulling purposes.  

/*
template <class T> inline bool is_primitive_type(T& A)
{
   return false;
}
*/

// Method set_to_null zeros out the contents of input object A

template <class T> void set_to_null(T& A)
{
   A = 0 * A;
/*
   T B;
   if (is_primitive_type(B))
   {
      A=0;
   }
   else
   {
      A=B-B;  // This is overkill.  Void constructor for non-primitives
	      // always initialize objects to zero...
   }
*/
}

// ==========================================================================
// Extrema element location methods:
// ==========================================================================

// Method max_array_value returns the maximum element within input
// array A:

template <class T> T max_array_value(const int nbins,T A[])
{
   int max_bin;
   return max_array_value(0,nbins-1,max_bin,A);
}

template <class T> T max_array_value(const int nbins,int& max_bin,T A[])
{
   return max_array_value(0,nbins-1,max_bin,A);
}

template <class T> T max_array_value(
   const int n1,const int n2,int& max_bin,T A[])
{
   T maxvalue=A[n1];
   max_bin=n1;
   
   for (int i=n1; i<=n2; i++)
   {
      if (A[i] > maxvalue)
      {
         maxvalue=A[i];
         max_bin=i;
      }
   }
   return maxvalue;
}

// Method min_array_value returns the minimum element within input
// array A:

template <class T> T min_array_value(const int nbins,T A[])
{
   int min_bin;
   return min_array_value(0,nbins-1,min_bin,A);
}

template <class T> T min_array_value(const int nbins,int& min_bin,T A[])
{
   return min_array_value(0,nbins-1,min_bin,A);
}

template <class T> T min_array_value(
   const int n1,const int n2,int& min_bin,T A[])
{
   T minvalue=A[n1];
   min_bin=n1;

   for (int i=n1; i<=n2; i++)
   {
      if (A[i] < minvalue)
      {
         minvalue=A[i];
         min_bin=i;
      }
   }
   return minvalue;
}

// ==========================================================================
// Sorting and swapping methods:
// ==========================================================================

// Reverse order of array so that first element becomes last, second
// element becomes second to last, etc...

template <class T> void reverse_array_order(const int nbins,T A[])
{
   T B[nbins];
   for (int i=0; i<nbins; i++)
   {
      B[i]=A[nbins-1-i];
   }
   for (int i=0; i<nbins; i++)
   {
      A[i]=B[i];
   }
}

// Sort first n objects inside input array A into ascending order.  We
// believe that this routine came from Paul Bamberg's C++ course which
// we audited during summer 1998:

template <class T> void Quicksort(T A[],int n)
{
   if (n > 1)
   {
      mysort(A,0,n-1);
   }
}

// Sort first n objects inside array A into ascending order, and make
// the corresponding rearrangement of array B:
   
template <class T,class U> void Quicksort(T A[],U B[],int n)
{
   if (n > 1)
   {
      mysort(A,B,0,n-1);
   }
}

// Sort first n objects inside array A into ascending order, and make
// corresponding rearrangements of array B and array C:

template <class T,class U,class V> void Quicksort(T A[],U B[],V C[],int n)
{
   if (n > 1)
   {
      T *Acopy=new T[n];
      for (int i=0; i<n; i++)
      {
         Acopy[i]=A[i];
      }
      Quicksort(A,B,n);
      Quicksort(Acopy,C,n);
      delete [] Acopy;
   }
}

template <class T,class U,class V,class W> 
  void Quicksort(T A[],U B[],V C[],W D[],int n)
{
   if (n > 1)
   {
      T *Acopy=new T[n];
      for (int i=0; i<n; i++)
      {
         Acopy[i]=A[i];
      }
      Quicksort(A,B,C,n);
      Quicksort(Acopy,D,n);
      delete [] Acopy;
   }
}

template <class T,class U,class V,class W,class X> 
  void Quicksort(T A[],U B[],V C[],W D[],X E[],int n)
{
   if (n > 1)
   {
      T *Acopy=new T[n];
      for (int i=0; i<n; i++)
      {
         Acopy[i]=A[i];
      }
      Quicksort(A,B,C,D,n);
      Quicksort(Acopy,E,n);
      delete [] Acopy;
   }
}

template <class T,class U,class V,class W,class X,class Y> 
  void Quicksort(T A[],U B[],V C[],W D[],X E[],Y F[],int n)
{
   if (n > 1)
   {
      T *Acopy=new T[n];
      for (int i=0; i<n; i++)
      {
         Acopy[i]=A[i];
      }
      Quicksort(A,B,C,D,E,n);
      Quicksort(Acopy,F,n);
      delete [] Acopy;
   }
}

template <class T,class U,class V,class W,class X,class Y,class Z> 
  void Quicksort(T A[],U B[],V C[],W D[],X E[],Y F[],Z G[],int n)
{
   if (n > 1)
   {
      T *Acopy=new T[n];
      for (int i=0; i<n; i++)
      {
         Acopy[i]=A[i];
      }
      Quicksort(A,B,C,D,E,F,n);
      Quicksort(Acopy,G,n);
      delete [] Acopy;
   }
}

template <class T,class U,class V,class W,class X,class Y,class Z,class Z1> 
  void Quicksort(T A[],U B[],V C[],W D[],X E[],Y F[],Z G[],Z1 H[],int n)
{
   if (n > 1)
   {
      T *Acopy=new T[n];
      for (int i=0; i<n; i++)
      {
         Acopy[i]=A[i];
      }
      Quicksort(A,B,C,D,E,F,G,n);
      Quicksort(Acopy,H,n);
      delete [] Acopy;
   }
}

// Sort first n objects inside input array A into descending order. 

template <class T> void Quicksort_descending(T A[],int n)
{
   if (n > 1)
   {
      T *Atmp=new T[n];

      for (int i=0; i<n; i++)
      {
         Atmp[i]=A[i];
      }
      mysort(Atmp,0,n-1);
      for (int i=0; i<n; i++)
      {
         A[i]=Atmp[n-1-i];
      }
      delete [] Atmp;
   } // n > 1 conditional
}

// Sort first n objects inside array A into descending order, and make
// the corresponding rearrangement of array B:

template <class T,class U> void Quicksort_descending(T A[],U B[],int n)
{
   if (n > 1)
   {
      T *Atmp=new T[n];
      U *Btmp=new U[n];

      for (int i=0; i<n; i++)
      {
         Atmp[i]=A[i];
         Btmp[i]=B[i];
      }
      mysort(Atmp,Btmp,0,n-1);
      for (int i=0; i<n; i++)
      {
         A[i]=Atmp[n-1-i];
         B[i]=Btmp[n-1-i];
      }

      delete [] Atmp;
      delete [] Btmp;
   } // n > 1 conditional
}

// Sort first n objects inside array A into descending order, and make
// corresponding rearrangements of array B and array C:

template <class T,class U,class V> 
  void Quicksort_descending(T A[],U B[],V C[],int n)
{
   if (n > 1)
   {
      T *Acopy=new T[n];
      for (int i=0; i<n; i++)
      {
         Acopy[i]=A[i];
      }
      Quicksort_descending(A,B,n);
      Quicksort_descending(Acopy,C,n);
      delete [] Acopy;
   }
}

template <class T,class U,class V,class W> 
  void Quicksort_descending(T A[],U B[],V C[],W D[],int n)
{
   if (n > 1)
   {
      T *Acopy=new T[n];
      for (int i=0; i<n; i++)
      {
         Acopy[i]=A[i];
      }
      Quicksort_descending(A,B,C,n);
      Quicksort_descending(Acopy,D,n);
      delete [] Acopy;
   }
}

template <class T,class U,class V,class W,class X> 
  void Quicksort_descending(T A[],U B[],V C[],W D[],X E[],int n)
{
   if (n > 1)
   {
      T *Acopy=new T[n];
      for (int i=0; i<n; i++)
      {
         Acopy[i]=A[i];
      }
      Quicksort_descending(A,B,C,D,n);
      Quicksort_descending(Acopy,E,n);
      delete [] Acopy;
   }
}

// ==========================================================================
template <class T> void mysort(T A[],int L,int R)
{
   int i=L;
   int j=R;
   T key=A[(L+R)/2];

   do
   {
      while (A[i] < key) i++;
      while (key < A[j]) j--;
      
      if (i <= j)
      {
         templatefunc::swap(A[i],A[j]);
         i++;
         j--;
      }
   }
   while (i<=j);
   
   if (L<j) mysort(A,L,j);
   if (j+1 < R) mysort(A,i,R);

}
template <class T,class U> void mysort(T A[],U B[],int L,int R)
{
   int i=L;
   int j=R;
   T key=A[(L+R)/2];

   do
   {
      while (A[i] < key) i++;
      while (key < A[j]) j--;
      
      if (i <= j)
      {
         templatefunc::swap(A[i],A[j]);
         templatefunc::swap(B[i],B[j]);
         i++;
         j--;
      }
   }
   while (i<=j);
   
   if (L<j) mysort(A,B,L,j);
   if (j+1 < R) mysort(A,B,i,R);
}

template <class T> void move_element_to_array_end(
   int posn,const int nbins,T A[])
{
   T *Acopy=new T[nbins];
   for (int n=0; n<posn; n++)
   {
      Acopy[n]=A[n];
   }
   Acopy[nbins-1]=A[posn];
   for (int n=posn; n<nbins-1; n++)
   {
      Acopy[n]=A[n+1];
   }
   for (int n=0; n<nbins; n++)
   {
      A[n]=Acopy[n];
   }
   delete Acopy;
}




