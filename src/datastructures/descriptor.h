// ==========================================================================
// Header file for DESCRIPTOR class
// ==========================================================================
// Last modified on 6/13/13; 8/25/13; 11/5/13; 4/6/14
// ==========================================================================

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <vector>
#include <flann/flann.hpp>

class genmatrix;

class descriptor
{

  public:

   descriptor(int d);
   descriptor(const descriptor& descript);
   ~descriptor();

   descriptor& operator= (const descriptor& descript);
   friend std::ostream& operator<< 
      (std::ostream& outstream,descriptor& descript);

// Set and get member functions:

   unsigned int get_mdim() const;
   float get(int i) const;
   void put(int i,float value);

   void clear_values();
   double* get_e_ptr();
   double magnitude() const;
   double sqrd_distance_to_another_descriptor(descriptor* element2_ptr);
   double entropy() const;


   flann::Matrix<float>* outerproduct(const descriptor& Y) const;
   void outerproduct(
      const descriptor& Y, genmatrix& outerproduct_matrix) const;

   void operator+= (const descriptor& X);
   void operator-= (const descriptor& X);
   void operator*= (double a);
   void operator/= (double a);

// ---------------------------------------------------------------------
// Friend functions:
// ---------------------------------------------------------------------

   friend descriptor operator+ (const descriptor& X,const descriptor& Y);
   friend descriptor operator- (const descriptor& X,const descriptor& Y);
   friend descriptor operator* (double a,const descriptor& X);
   friend descriptor operator* (const descriptor& X,double a);
   friend descriptor operator/ (const descriptor& X,double a);

// Coates preprocessing:

   void mean_and_std_dev(float& mean, float& std_dev);
   void contrast_normalize(float eps_norm);

  private: 

   unsigned int d_dims;
   std::vector<float>* d_ptr;
   double* e_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const descriptor& descript);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline unsigned int descriptor::get_mdim() const
{
   return d_dims;
}

inline void descriptor::put(int i,float value) 
{
   (*d_ptr)[i]=value;
}

inline float descriptor::get(int i) const
{
//   cout << "inside get(), i = " << i << endl;
   return d_ptr->at(i);
}


#endif  // descriptor.h
