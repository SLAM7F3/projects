// =========================================================================
// DESCRIPTOR class member function definitions
// =========================================================================
// Last modified on 6/13/13; 8/25/13; 11/5/13; 4/6/14
// =========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "datastructures/descriptor.h"
#include "math/genmatrix.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void descriptor::allocate_member_objects()
{
   d_ptr=new vector<float>;
   d_ptr->reserve(d_dims);
   clear_values();
}

void descriptor::initialize_member_objects()
{
   e_ptr=NULL;
}		 

// ---------------------------------------------------------------------
descriptor::descriptor(int d)
{
   d_dims=d;
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

descriptor::descriptor(const descriptor& descript)
{
   docopy(descript);
}

descriptor::~descriptor()
{
//   cout << "inside descriptor destructor" << endl;

   delete d_ptr;
}

// ---------------------------------------------------------------------
void descriptor::docopy(const descriptor& descript)
{
   d_dims=descript.d_dims;
   for (unsigned int d=0; d<d_dims; d++)
   {
      d_ptr->at(d)=descript.d_ptr->at(d);
   }
   e_ptr=descript.e_ptr;
}

// Overload = operator:

descriptor& descriptor::operator= (const descriptor& descript)
{
   if (this==&descript) return *this;
   docopy(descript);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,descriptor& descript)
{
   outstream << "d_dims = " << descript.get_mdim() << endl;
   for (unsigned int d=0; d<descript.get_mdim(); d++)
   {
      outstream << "d = " << d 
                << " descriptor[d] = " << descript.get(d)
                << endl;
   }
   return outstream;
}

// ==========================================================================

void descriptor::clear_values()
{
   d_ptr->clear();
   for (unsigned int d=0; d<d_dims; d++)
   {
      d_ptr->push_back(0);
   }
}		 

double* descriptor::get_e_ptr()
{
   if (e_ptr==NULL)
   {
      e_ptr=new double(d_dims);
      for (unsigned int d=0; d<d_dims; d++)
      {
         e_ptr[d]=get(d);
      }
   }
   return e_ptr;
}

double descriptor::magnitude() const
{
   double sqrd_mag=0;
   for (unsigned int d=0; d<d_dims; d++)
   {
      sqrd_mag += sqr(get(d));
   }
   return sqrt(sqrd_mag);
}		 

// --------------------------------------------------------------------------
// Member function entropy() computes the entropy for a SIFT
// descriptors as defined by Wei Dong in his 2011 Princeton
// Ph.D. thesis "High-dimensional similarity search for large
// datasets".  See page 144 of Dong_princeton_0181D_10018.pdf

double descriptor::entropy() const
{
   vector<int> i_bins;
   i_bins.reserve(256);
   for (unsigned int i=0; i<256; i++)
   {
      i_bins.push_back(0);
   }

   for (unsigned int k=0; k<d_dims; k++)
   {
      int f_k=get(k);
      i_bins[f_k]=i_bins[f_k]+1;
   } // loop over index k 

   double entropy=0;
   for (unsigned int i=0; i<256; i++)
   {
      if (i_bins[i]==0) continue;
      double p_i=double(i_bins[i])/d_dims;
      entropy -= p_i*log(p_i);
   } // loop over index i 
   entropy /= log(2);
//   cout << "entropy = " << entropy << endl;
   return entropy;
}

/*
double descriptor::entropy() const
{
   double entropy=0;
   for (unsigned int i=0; i<d_dims; i++)
   {
      int k_counter=0;
      for (unsigned int k=0; k<d_dims; k++)
      {
         if (get(k)==i) k_counter++;
      } // loop over index k 
      if (k_counter==0) continue;
      double p_i=double(k_counter)/d_dims;
      entropy -= p_i*log(p_i);
   } // loop over index i 
   entropy /= log(2);
//   cout << "entropy = " << entropy << endl;
   return entropy;
}
*/

// --------------------------------------------------------------------------
double descriptor::sqrd_distance_to_another_descriptor(
   descriptor* element2_ptr)
{
   double sqrd_mag=0;
   for (unsigned int d=0; d<d_dims; d++)
   {
      sqrd_mag += sqr(get(d)-element2_ptr->get(d));
   }
   return sqrd_mag;
}

// --------------------------------------------------------------------------
flann::Matrix<float>* descriptor::outerproduct(const descriptor& Y) const
{
   unsigned int mdim=d_dims;
   unsigned int ndim=Y.get_mdim();

   flann::Matrix<float>* outerproduct_matrix_ptr=
      new flann::Matrix<float>(new float[mdim*ndim],mdim,ndim);

   for (unsigned int i=0; i<mdim; i++)
   {
      for (unsigned int j=0; j<ndim; j++)
      {
         (*outerproduct_matrix_ptr)[i][j]=get(i)*Y.get(j);
      }
   }
   return outerproduct_matrix_ptr;
}

// --------------------------------------------------------------------------
// This overloaded version of member function outerproduct() takes in 
// descriptor Y.  It returns the outerproduct between *this and Y
// within output genmatrix *outerproduct_matrix_ptr.

void descriptor::outerproduct(
   const descriptor& Y, genmatrix& outerproduct_matrix) const
{
   unsigned int mdim=d_dims;
   unsigned int ndim=Y.get_mdim();

   for (unsigned int i=0; i<mdim; i++)
   {
      for (unsigned int j=0; j<ndim; j++)
      {
         outerproduct_matrix.put(i,j,get(i)*Y.get(j));
      }
   }
}

// --------------------------------------------------------------------------
// Overload += operator:

void descriptor::operator+= (const descriptor& X)
{
  for (unsigned int i=0; i<d_dims; i++)
  {
     put(i,get(i)+X.get(i));
  }
}

// Overload -= operator:

void descriptor::operator-= (const descriptor& X)
{
  for (unsigned int i=0; i<d_dims; i++)
  {
     put(i,get(i)-X.get(i));
  }
}

// Overload *= operator:

void descriptor::operator*= (double a)
{
  for (unsigned int i=0; i<d_dims; i++)
  {
     put(i,get(i)*a);
  }
}

// Overload /= operator:

void descriptor::operator/= (double a)
{
  for (unsigned int i=0; i<d_dims; i++)
  {
     put(i,get(i)/a);
  }
}

// ==========================================================================
// Note: Keyword friend should appear in class declaration file and not 
// within class member function definition file.  Friendly functions should
// not be declared as member functions of a class.  So their definition syntax
// is not 

// returntype classname::memberfunctioname(argument list)

// but rather

// returntype friendlyfunctionname(argument list)
// ==========================================================================

// Overload + operator:

descriptor operator+ (const descriptor& X,const descriptor& Y)
{
   if (X.d_dims==Y.d_dims)
   {
      descriptor Z(X.d_dims);
      for (unsigned int i=0; i<X.d_dims; i++)
      {
         Z.put(i,X.get(i)+Y.get(i));
      }
      return Z;
   }
   else
   {
      cout << "Error inside operator+ friend function of descriptor class!"
           << endl;
      cout << "X.d_dims = " << X.d_dims << " does not equal Y.d_dims = " 
           << Y.d_dims << endl;
      exit(-1);
   }
}

// Overload - operator:

descriptor operator- (const descriptor& X,const descriptor& Y)
{
   if (X.d_dims==Y.d_dims)
   {
      descriptor Z(X.d_dims);
      for (unsigned int i=0; i<X.d_dims; i++)
      {
         Z.put(i,X.get(i)-Y.get(i));
      }
      return Z;
   }
   else
   {
      cout << "Error inside operator- friend function of descriptor class!"
           << endl;
      cout << "X.d_dims = " << X.d_dims << " does not equal Y.d_dims = " 
           << Y.d_dims << endl;
      exit(-1);
   }
}

// Overload * operator:

descriptor operator* (double a,const descriptor& X)
{
   descriptor Z(X.d_dims);
   
   for (unsigned int i=0; i<X.d_dims; i++)
   {
      Z.put(i,a*X.get(i));
   }
   return Z;
}

// Overload * operator:

descriptor operator* (const descriptor& X,double a)
{
   return a*X;
}

// Overload / operator:

descriptor operator/ (const descriptor& X,double a)
{
   descriptor Z(X.d_dims);
   
   for (unsigned int i=0; i<X.d_dims; i++)
   {
      Z.put(i,X.get(i)/a);
   }
   return Z;
}

// =========================================================================
// Coates preprocessing (See "Learning feature representations with
// K-means" by Coates and Ng, 2012.)
// =========================================================================

void descriptor::mean_and_std_dev(float& mean, float& std_dev)
{
   mean=0;
   float mean_sqr=0;
   for (unsigned int i=0; i<d_dims; i++)
   {
      mean += d_ptr->at(i);
      mean_sqr += sqr(d_ptr->at(i));
   }
   mean /= d_dims;
   mean_sqr /= d_dims;
   float variance = mean_sqr - sqr(mean);
   std_dev=sqrt(variance);
}

// Member function contrast_normalize() computes and removes the mean
// from each of the descriptor's d_dims components.  It also divides
// the translated component values by (approximately) their
// standard deviation.  Coates suggests setting eps_norm = 10 if the
// descriptor's raw intensity values range over [0,255].
// Equivalently, eps_norm = 10/(255)**2 = 1.537E-4 if the raw
// descriptor components range over [0,1].

void descriptor::contrast_normalize(float eps_norm)
{
   float mu,sigma;
   mean_and_std_dev(mu,sigma);
   float variance=sqr(sigma);

   float denom=sqrt(variance+eps_norm);
   for (unsigned int i=0; i<d_dims; i++)
   {
      float numer=d_ptr->at(i)-mu;
      d_ptr->at(i) = numer/denom;
   }
}

