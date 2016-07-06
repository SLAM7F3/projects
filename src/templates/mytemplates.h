// ==========================================================================
// Header file for template routines
// ==========================================================================
// Last updated on 8/3/06; 8/12/07; 10/25/10; 3/22/14
// ==========================================================================

#ifndef MYTEMPLATES_H
#define MYTEMPLATES_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <new>
#include <vector>

namespace templatefunc
{
   template <class T> void swap(T& A, T& B);
   template <class T,class U> void Quicksort(
      std::vector<T>& A,std::vector<U>& B);
   template <class T,class U> void Quicksort_descending(
      std::vector<T>& A,std::vector<U>& B);

   template <class T,class U,class V> void Quicksort(
      std::vector<T>& A,std::vector<U>& B,std::vector<V>& C);
   template <class T,class U,class V> void Quicksort_descending(
      std::vector<T>& A,std::vector<U>& B,std::vector<V>& C);

   template <class T,class U,class V,class W> void Quicksort(
      std::vector<T>& A,std::vector<U>& B,std::vector<V>& C,
      std::vector<W>& D);
   template <class T,class U,class V,class W> void Quicksort_descending(
      std::vector<T>& A,std::vector<U>& B,std::vector<V>& C,
      std::vector<W>& D);

   template <class T,class U,class V,class W,class X> void Quicksort(
      std::vector<T>& A,std::vector<U>& B,std::vector<V>& C,
      std::vector<W>& D,std::vector<X>& E);
   template <class T,class U,class V,class W,class X> 
      void Quicksort_descending(
         std::vector<T>& A,std::vector<U>& B,std::vector<V>& C,
         std::vector<W>& D,std::vector<X>& E);

   template <class T,class U,class V,class W,class X,class Y> 
      void Quicksort(std::vector<T>& A,std::vector<U>& B,
                     std::vector<V>& C,std::vector<W>& D,
                     std::vector<X>& E,std::vector<Y>& F);
   template <class T,class U,class V,class W,class X,class Y> 
      void Quicksort_descending(std::vector<T>& A,std::vector<U>& B,
                     std::vector<V>& C,std::vector<W>& D,
                     std::vector<X>& E,std::vector<Y>& F);

   template <class T,class U,class V,class W,class X,class Y,class Z> 
      void Quicksort(std::vector<T>& A,std::vector<U>& B,
                     std::vector<V>& C,std::vector<W>& D,
                     std::vector<X>& E,std::vector<Y>& F,
                     std::vector<Z>& G);

   template <class S,class T,class U,class V,class W,class X,class Y,class Z> 
      void Quicksort(std::vector<S>& A,std::vector<T>& B,
                     std::vector<U>& C,std::vector<V>& D,
                     std::vector<W>& E,std::vector<X>& F,
                     std::vector<Y>& G,std::vector<Z>& H);

   template <class R,class S,class T,class U,class V,class W,
      class X,class Y,class Z> 
      void Quicksort(std::vector<R>& A,std::vector<S>& B,
                     std::vector<T>& C,std::vector<U>& D,
                     std::vector<V>& E,std::vector<W>& F,
                     std::vector<X>& G,std::vector<Y>& H,
                     std::vector<Z>& I);

//   template <class T> void mysort(std::vector<T>& A,int L,int R);
   template <class T,class U> void mysort(
      std::vector<T>& A,std::vector<U>& B,int L,int R);

// Array averaging methods:

   template <class T> T average(T A[], const int nbins);
   template <class T> T average(T A[],double w[],int nbins);
   template <class T> T average(int nbins,double x[],T A[]);
   template <class T> T average(int nbins,double x[],T A[],T dA[],T& dA_avg);

   template <class T> void alpha_filter(
      const T& currx,T filtered_x[],double alpha);

// STL methods:

   template <class T> void printVector(const std::vector<T>& v);
} // templatefunc namespace

// Array allocation and clearing methods:
template <class T> void new_clear_array(T*& T_ptr,int nsize);
template <class T> void clear_array(T* T_ptr,int nsize);

// Object nulling methods:
// template <class T> bool is_primitive_type(T& A);
template <class T> void set_to_null(T& A);

// Extrema element location methods:
template <class T> T max_array_value(const int nbins,T A[]);
template <class T> T max_array_value(const int nbins,int& max_bin,T A[]);
template <class T> T max_array_value(
   const int n1,const int n2,int& min_bin,T A[]);
template <class T> T min_array_value(const int nbins,T A[]);
template <class T> T min_array_value(const int nbins,int& min_bin,T A[]);
template <class T> T min_array_value(
   const int n1,const int n2,int& min_bin,T A[]);

// Sorting methods:
template <class T> void reverse_array_order(const int nbins,T A[]);

template <class T> void Quicksort(T A[],int n);
template <class T,class U> void Quicksort(T A[],U B[],int n);
template <class T,class U,class V> void Quicksort(T A[],U B[],V C[],int n);
template <class T,class U,class V,class W> 
  void Quicksort(T A[],U B[],V C[],W D[],int n);
template <class T,class U,class V,class W,class X> 
  void Quicksort(T A[],U B[],V C[],W D[],X E[],int n);
template <class T,class U,class V,class W,class X,class Y> 
  void Quicksort(T A[],U B[],V C[],W D[],X E[],Y F[],int n);
template <class T,class U,class V,class W,class X,class Y,class Z> 
  void Quicksort(T A[],U B[],V C[],W D[],X E[],Y F[],Z G[],int n);
template <class T,class U,class V,class W,class X,class Y,class Z,class Z1> 
  void Quicksort(T A[],U B[],V C[],W D[],X E[],Y F[],Z G[],Z1 H[],int n);

template <class T> void Quicksort_descending(T A[],int n);
template <class T,class U> void Quicksort_descending(T A[],U B[],int n);
template <class T,class U,class V> 
  void Quicksort_descending(T A[],U B[],V C[],int n);
template <class T,class U,class V,class W> 
  void Quicksort_descending(T A[],U B[],V C[],W D[],int n);
template <class T,class U,class V,class W,class X> 
  void Quicksort_descending(T A[],U B[],V C[],W D[],X E[],int n);

template <class T> void mysort(T A[],int L,int R);
template <class T,class U> void mysort(T A[],U B[],int L,int R);

template <class T> void move_element_to_array_end(
   int i,const int nbins,T A[]);

// We learned from Vadim and Peter B. in Mar 02 that the definitions
// for templatized functions need to be included within the same
// header file where these functions are declared.  Vadim reminded us
// that a templatized function cannot be compiled in advance.
// Instead, it is basically a glorified macro.  So we do NOT include
// "mytemplates.cc" into any makefile...

#include "mytemplates.cc"


#endif  // templates/mytemplates.h



