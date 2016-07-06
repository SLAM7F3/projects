// ==========================================================================
// Templatized Tensor class member function definitions
// ==========================================================================
// Last modified on 4/1/09; 5/25/10; 4/4/14; 4/5/14; 4/6/14
// =========================================================================

#include <math.h>
#include "math/basic_math.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"

// ==========================================================================
// Initialization, constructor and destructor methods:
// ==========================================================================

template <class A> void Tensor<A>::allocate_member_objects()
{
   dim.clear();
   Indices.clear();
   if (rank==0)
   {
      dim.push_back(1);
      Indices.reserve(1);
   }
   else
   {
      dim.reserve(rank);
      Indices.reserve(rank);
   }
}

template <class A> Tensor<A>::Tensor(
   unsigned int rnk,const std::vector<unsigned int>& d)
{
   initialize_member_objects(rnk);
   allocate_member_objects();
   dimproduct=1;
   for (unsigned int r=0; r<rank; r++)
   {
      dim.push_back(d[r]);
      dimproduct *= dim[r];
   }
   e=new A[dimproduct];
   clear_values();
}

// Specialized constructor for rank-0 tensors ( = scalars):

template <class A> Tensor<A>::Tensor()
{
   initialize_member_objects(0);
   allocate_member_objects();
   dimproduct=1;
   e=new A[dimproduct];
   clear_values();
}

// Specialized constructor for rank-1 tensors ( = vectors):

template <class A> Tensor<A>::Tensor(int m)
{
   initialize_member_objects(1);
   allocate_member_objects();
   dim.push_back(m);
   dimproduct=dim[0];
   e=new A[dimproduct];
   clear_values();
}

// Specialized constructor for rank-2 tensors ( = matrices):

template <class A> Tensor<A>::Tensor(int m,int n)
{
   initialize_member_objects(2);
   allocate_member_objects();
   dim.push_back(m);
   dim.push_back(n);
   dimproduct=dim[0]*dim[1];
   e=new A[dimproduct];
//   clear_values();
}

// Specialized constructor for rank-3 tensors:

template <class A> Tensor<A>::Tensor(int m,int n,int p)
{
   initialize_member_objects(3);
   allocate_member_objects();
   dim.push_back(m);
   dim.push_back(n);
   dim.push_back(p);
   dimproduct=dim[0]*dim[1]*dim[2];
   e=new A[dimproduct];
   clear_values();
}

// Specialized constructor for rank-4 tensors:

template <class A> Tensor<A>::Tensor(int m,int n,int p,int q)
{
   initialize_member_objects(4);
   allocate_member_objects();
   dim.push_back(m);
   dim.push_back(n);
   dim.push_back(p);
   dim.push_back(q);
   dimproduct=dim[0]*dim[1]*dim[2]*dim[3];
   e=new A[dimproduct];
   clear_values();
}

// Copy constructor:

template <class A> Tensor<A>::Tensor(const Tensor<A>& T)
{
   initialize_member_objects(T.get_rank());
   allocate_member_objects();
   new_clear_array(e,T.get_dimproduct());
   docopy(T);
}

template <class A> Tensor<A>::~Tensor()
{
   delete [] e;
   e=NULL;
}

// ---------------------------------------------------------------------
template <class A> void Tensor<A>::docopy(const Tensor<A>& m)
{
   rank=m.get_rank();
   dimproduct=m.get_dimproduct();
   for (unsigned int r=0; r<rank; r++)
   {
      dim[r]=m.get_dim(r);
      Indices[r]=m.get_Indices(r);
   }
   memcpy(e,m.e,dimproduct*sizeof(e[0]));
}

// Overload = operator:

template <class A> Tensor<A>& Tensor<A>::operator= (const Tensor<A>& T)
{
   if (this==&T) return *this;
   docopy(T);
   return *this;
}

template <class A> bool Tensor<A>::operator== (const Tensor<A>& T) const
{
   bool tensors_equal=true;

   if (T.get_rank() != rank)
   {
      tensors_equal=false;
   }
   else
   {
      for (unsigned int r=0; r<rank; r++)
      {
         if (T.get_dim(r) != dim[r])
         {
            tensors_equal=false;
         }
      } // loop over index r labeling tensor rank
   } // T.get_rank() != rank conditional
   
   if (tensors_equal)
   {
      for (unsigned int i=0; i<dimproduct; i++)
      {
         if (T.get(i) != get(i))
         {
            tensors_equal=false;
         }
      }
   }
   return tensors_equal;
}

template <class A> bool Tensor<A>::operator!= (const Tensor<A>& T) const
{
   return !(*this==T);
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class A> std::ostream& operator<< 
(std::ostream& outstream,Tensor<A>& T)
{
   outputfunc::newline();
   outstream << "Tensor rank = " << T.get_rank() 
             << " dimproduct = " << T.get_dimproduct() << std::endl;
   for (unsigned int i=0; i<T.get_dimproduct(); i++)
   {
      outstream << "Tensor indices = ";
      T.index_to_indices(i);
      for (unsigned int r=0; r<T.get_rank(); r++)
      {
         outstream << T.get_Indices(r) << " ";
      }
      outstream << " Tensor value = " << T.get(i) << std::endl;
   }

   return(outstream);
}

// ==========================================================================
// Boolean member function check_index_validity returns true if the
// input index lies within the allowed range for the current tensor
// object:

template <class A> bool Tensor<A>::check_index_validity(unsigned int index) 
const
{
   bool index_OK=true;
   if (this==NULL)
   {
      std::cout << "Error inside Tensor<A>::check_index_validity()" 
                << std::endl;
      std::cout << "this = NULL !!!" << std::endl;
      index_OK=false;
   }
   if (index < 0 || index >= dimproduct)
   {
      std::cout << "Error inside Tensor<A>::check_index_validity()" 
                << std::endl;
      std::cout << "index = " << index << " dimproduct = " << dimproduct 
                << std::endl;
      index_OK=false;
   }
   return index_OK;
}

// ---------------------------------------------------------------------
// Member function indices_to_index converts multidimensional tensor
// indices contained within the first rank elements of input array
// entry into an equivalent one-dimensonal array index value:

template <class A> unsigned int Tensor<A>::indices_to_index(
   const std::vector<unsigned int>& entry) const
{
   unsigned int index=Unsigned_Zero;
   int dimprod=1;

// First check that we're not trying to access an element which lies
// out tensor's dimensioned size:

   for (unsigned int r=0; r<rank; r++)
   {
      if (entry[r] > dim[r]-1)
      {
         std::cout << "Error in Tensor<A>::indices_to_index() !" << std::endl;
         std::cout << "r = " << r
              << " entry[r] = " << entry[r] << " exceeds "
              << " dim[r]-1 = " << dim[r]-1 << std::endl;
         exit(-1);
      }
   }

   for (unsigned int r=0; r<rank; r++)
   {
      if (r>0) 
      {
         dimprod *= dim[rank-r];
      }
      index += entry[rank-1-r]*dimprod;
   }

   if (check_index_validity(index))
   {
      return index;
   }
   else
   {
      return NEGATIVEINFINITY;
   }
}

// ---------------------------------------------------------------------
// Member function index_to_indices performs the inverse operation to
// member function indices_to_index.  It returns the multidimensional
// tensor indices contained within the first rank elements of input
// array entry corresponding to the one-dimensonal array index value:

template <class A> void Tensor<A>::index_to_indices(unsigned int index) 
{
   if (check_index_validity(index))
   {
      for (unsigned int r=0; r<rank; r++)
      {
         Indices[rank-1-r]=index%dim[rank-1-r];
         index -= Indices[rank-1-r];
         index /= dim[rank-1-r];
      }
   }
}

// ---------------------------------------------------------------------
// Member function initialize_values sets all entries within the
// current tensor object equal to input parameter value:

template <class A> void Tensor<A>::initialize_values(A value)
{
   for (unsigned int i=0; i<dimproduct; i++)
   {
      e[i]=value;
   }
}

// ---------------------------------------------------------------------
// Member function minmax_values returns the minimum and maximum
// valued components of the current tensor object:

template <class A> void Tensor<A>::minmax_values(
   A& minvalue,A& maxvalue) const
{
   minmax_values(minvalue,2*NEGATIVEINFINITY,maxvalue,2*POSITIVEINFINITY);
}

// This overloaded version of minmax_values takes in floor and ceiling
// values which restrict the search for extremal tensor values:

template <class A> void Tensor<A>::minmax_values(
   A& minvalue,A floor_value,A& maxvalue,A ceiling_value) const
{
   minvalue=POSITIVEINFINITY;
   maxvalue=NEGATIVEINFINITY;
   for (unsigned int i=0; i<dimproduct; i++)
   {
      if (e[i] < minvalue && e[i] > floor_value) minvalue=e[i];
      if (e[i] > maxvalue && e[i] < ceiling_value) maxvalue=e[i];
   }
}

template <class A> A Tensor<A>::minimum_value() const
{
   A minvalue=POSITIVEINFINITY;
   for (unsigned int i=0; i<dimproduct; i++)
   {
      if (e[i] < minvalue) minvalue=e[i];
   }
   return minvalue;
}

template <class A> A Tensor<A>::maximum_value() const
{
   A maxvalue=NEGATIVEINFINITY;
   for (unsigned int i=0; i<dimproduct; i++)
   {
      if (e[i] > maxvalue) maxvalue=e[i];
   }
   return maxvalue;
}

// ---------------------------------------------------------------------
// Member function sum_values integrates the current rank r tensor
// over all its dimensions.

template <class A> A Tensor<A>::sum_values() const
{
   A sum=0;
   for (unsigned int i=0; i<dimproduct; i++) sum += e[i];
   return sum;
}

// ---------------------------------------------------------------------
template <class A> bool Tensor<A>::is_finite() const
{
   for (unsigned int i=0; i<dimproduct; i++)
   {
      if (isfinite(get(i))==0) return false;
   }
   return true;
}

// ---------------------------------------------------------------------
// Boolean member function compare_rank_and_dims returns true if input
// tensor T's rank and dimensions match those of the current tensor
// object:

template <class A> bool Tensor<A>::compare_rank_and_dims(
   const Tensor<A>& T) const
{
   bool tensors_match=true;

   if (T.get_rank() != rank)
   {
      tensors_match=false;
   }
   else
   {
      for (unsigned int r=0; r<rank; r++)
      {
         if (T.get_dim(r) != dim[r])
         {
            tensors_match=false;
         }
      }
   }
   return tensors_match;
}

// ---------------------------------------------------------------------
// Member function nearly_equal represents a minor variant of
// operator==.  But it's probably more robust for tensors of floats or
// doubles.

template <class A> bool Tensor<A>::nearly_equal(
   const Tensor<A>& T,double TINY) const
{
   bool tensors_nearly_equal=true;

   if (T.get_rank() != rank)
   {
      tensors_nearly_equal=false;
   }
   else
   {
      for (unsigned int r=0; r<rank; r++)
      {
         if (T.get_dim(r) != dim[r])
         {
            tensors_nearly_equal=false;
         }
      } // loop over index r labeling tensor rank
   } // T.get_rank() != rank conditional
   
   if (tensors_nearly_equal)
   {
      for (unsigned int i=0; i<dimproduct; i++)
      {
         if (!::nearly_equal(T.get(i),get(i),TINY))
         {
            tensors_nearly_equal=false;
         }
      }
   }
   return tensors_nearly_equal;
}

// ==========================================================================
// Tensor manipulation methods
// ==========================================================================

// Member function outerproduct returns a new tensor containing the
// outer product of the current tensor with input tensor T.

template <class A> Tensor<A> Tensor<A>::outerproduct(const Tensor<A>& T) const
{
   unsigned int total_rank=rank+T.get_rank();

   std::vector<unsigned int> d(total_rank);
   d.clear();
   
   for (unsigned int r=0; r<rank; r++)
   {
      d.push_back(dim[r]);
   }
   for (unsigned int r=rank; r<total_rank; r++)
   {
      d.push_back(T.get_dim(r-rank));
   }
   Tensor O(total_rank,d);

   std::vector<unsigned int> curr_indices(rank);
   std::vector<unsigned int> T_indices(total_rank);   
   for (unsigned int i=0; i<O.get_dimproduct(); i++)
   {
      O.index_to_indices(i);

      curr_indices.clear();
      for (unsigned int r=0; r<O.rank; r++)
      {
         curr_indices.push_back(O.get_Indices(r));
      }

      T_indices.clear();
      for (unsigned int r=rank; r<total_rank; r++)
      {
         T_indices.push_back(O.get_Indices(r));
      }
      
      O.put(i,get(curr_indices) * T.get(T_indices));
   }

   return O;
}

// ---------------------------------------------------------------------
template <class A> Tensor<A> Tensor<A>::outerproduct(
   const Tensor<A>& S,const Tensor<A>& T) const
{
   Tensor<A> C=outerproduct(S);
   return C.outerproduct(T);
}

// ---------------------------------------------------------------------
template <class A> Tensor<A> Tensor<A>::outerproduct(
   const Tensor<A>& R,const Tensor<A>& S,const Tensor<A>& T) const
{
   Tensor<A> C=outerproduct(R,S);
   return C.outerproduct(T);
}

// ---------------------------------------------------------------------
// Member function contract computes and returns the rank r-2
// contraction of the current rank r Tensor object on indices labeled
// by input integers i and j.

template <class A> Tensor<A> Tensor<A>::contract(unsigned int i,unsigned int j)
{
   if (i < 0 || i >= rank || j < 0 || j >= rank)
   {
      std::cout << "Error in Tensor::contract()" << std::endl;
      std::cout << "i = " << i << " j = " << j << " rank = " << rank 
                << std::endl;
      exit(-1);
   }
   else
   {
      if (i > j) templatefunc::swap(i,j);

      std::vector<unsigned int> reduced_d(rank);
      reduced_d.clear();
      
      for (unsigned int r=0; r<rank; r++)
      {
         if (r != i && r != j) reduced_d.push_back(dim[r]);
      } // loop over index r 
      
      Tensor contraction(rank-2,reduced_d);
      std::vector<unsigned int> contracted_indices(rank);
      
      for (unsigned int index=0; index<dimproduct; index++)
      {
         contracted_indices.clear();
         index_to_indices(index);
         if (get_Indices(i)==get_Indices(j))
         {
            for (unsigned int rlo=0; rlo<i; rlo++)
            {
               contracted_indices.push_back(get_Indices(rlo));
            }
            for (unsigned int rmed=i+1; rmed<j; rmed++)
            {
               contracted_indices.push_back(get_Indices(rmed));
            }
            for (unsigned int rhi=j+1; rhi<rank; rhi++)
            {
               contracted_indices.push_back(get_Indices(rhi));
            }
            contraction.increment(contracted_indices,get(index));
         } 

//         std::cout << "Original tensor indices:" << std::endl;
//         templatefunc::printVector(indices);
//         std::cout << "Contracted tensor indices:" << std::endl;
//         templatefunc::printVector(contracted_indices);
      } // loop over index iterator
//      std::cout << "contraction = " << contraction << std::endl;
      return contraction;
   }
}

// ---------------------------------------------------------------------
// Member function contract_adjacent_pairs returns a rank r-4
// contraction of the current rank r Tensor.  The index labeled by
// input integer i is contracted with the index labeled by i+2, and
// the index labeled by index i+1 is contracted with the index labeled
// by i+3.  This special utility method facilitates evaluating tensor
// products like A_{i,j,k} * B^{j,k,l}

template <class A> Tensor<A> Tensor<A>::contract_adjacent_pairs(unsigned int i)
{
   Tensor<A> C1(contract(i,i+2));
   return C1.contract(i,i+1);
}

// ---------------------------------------------------------------------
// Overload += operator:

template <class A> void Tensor<A>::operator+= (const Tensor<A>& T)
{
   for (unsigned int i=0; i<dimproduct; i++) e[i] += T.e[i];
}

// Overload += operator:

template <class A> void Tensor<A>::operator-= (const Tensor<A>& T)
{
   for (unsigned int i=0; i<dimproduct; i++) e[i] -= T.e[i];
}

// Overload *= operator:

template <class A> void Tensor<A>::operator*= (A a)
{
   for (unsigned int i=0; i<dimproduct; i++) e[i] *= a;
}

// Overload /= operator:

template <class A> void Tensor<A>::operator/= (A a)
{
   for (unsigned int i=0; i<dimproduct; i++) e[i] /= a;
}

// ==========================================================================
// Friend functions
// ==========================================================================

// Overload + operator:

template <class A> Tensor<A> operator+ (
   const Tensor<A>& X,const Tensor<A>& Y)
{
   if (X.compare_rank_and_dims(Y))
   {
      Tensor<A> Z(X.get_rank(),X.dim);

      for (unsigned int i=0; i<Z.get_dimproduct(); i++)
      {
         Z.put(i,X.get(i)+Y.get(i));
//         std::cout << "i = " << i << " X.e[i] = " <<  X.e[i]
//                   << " Y.e[i] = " << Y.e[i] << " Z.e[i] = " << Z.e[i]
//                   << std::endl;
      }
//      std::cout << "Before returning tensor sum" << std::endl;
      return Z;
   }
   else
   {
      std::cout << "Error inside tensor operator+ !" << std::endl;
      std::cout << "Input tensors do not have same rank &/or dimensions" 
                << std::endl;
      exit(-1);
   }
   std::cout << "At end of operator+" << std::endl;
}

// Overload - operator:

template <class A> Tensor<A> operator- (
   const Tensor<A>& X,const Tensor<A>& Y)
{
   if (X.compare_rank_and_dims(Y))
   {
      Tensor<A> Z(X.get_rank(),X.dim);

      for (unsigned int i=0; i<Z.get_dimproduct(); i++)
      {
         Z.put(i,X.get(i)-Y.get(i));
      }
      return Z;
   }
   else
   {
      std::cout << "Error inside Tensor operator- !" << std::endl;
      std::cout << "Input Tensors do not have same rank &/or dimensions" 
                << std::endl;
      std::cout << "X.rank = " << X.get_rank() 
                << " Y.rank = " << Y.get_rank() << std::endl;

      for (int r=0; r<basic_math::min(X.get_rank(),Y.get_rank()); r++)
      {
         std::cout << "r = " << r << " X.dim[r] = " << X.get_dim(r)
                   << " Y.dim[r] = " << Y.get_dim(r) << std::endl;
      }
      exit(-1);
   }
}

// Overload - operator:

template <class A> Tensor<A> operator- (const Tensor<A>& X)
{
   Tensor<A> Z(X.get_rank(),X.dim);
   for (unsigned int i=0; i<Z.get_dimproduct(); i++)
   {
      Z.put(i,-X.get(i));
   }
   return Z;
}

// Overload * operator for multiplying a Tensor by a scalar:

/*
template <class A,class B>
Tensor<A> operator* (B b,const Tensor<A>& X)
{
   Tensor<A> Z(X.get_rank(),X.dim);
   for (unsigned int i=0; i<Z.get_dimproduct(); i++)
   {
      Z.put(i,b*X.get(i));
   }
   return Z;
}

template <class A,class B>
Tensor<A> operator* (const Tensor<A>& X,B b)
{
   return b*X;
}
*/
