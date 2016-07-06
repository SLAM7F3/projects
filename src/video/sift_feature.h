// ==========================================================================
// Header file for sift_feature class
// ==========================================================================
// Last modified on 3/17/11
// ==========================================================================

#ifndef SIFT_FEATURE_H
#define SIFT_FEATURE_H

#include <string>
#include <vector>

class sift_feature
{

  public:

   sift_feature(int ID);
   sift_feature(const sift_feature& s);
   ~sift_feature();
   sift_feature& operator= (const sift_feature& s);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const sift_feature& s);

// Set and get member functions:

   int get_ID() const;
   void set_photo_ID(int id);
   int get_photo_ID() const;
   void set_u_v_orientation_scale(double u,double v,double theta,double scale);
   void get_u_v_orientation_scale(
      double& u,double& v,double& orientation,double& scale) const;

   void set_descriptor(const std::vector<double>& values);
   std::string get_descriptor_as_string() const;

  private: 

   int ID,photo_ID;
   double u,v,orientation,scale;
   std::vector<double> descriptor;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const sift_feature& p);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int sift_feature::get_ID() const
{
   return ID;
}

inline void sift_feature::set_photo_ID(int id)
{
   photo_ID=id;
}

inline int sift_feature::get_photo_ID() const
{
   return photo_ID;
}

inline void sift_feature::set_u_v_orientation_scale(
   double u,double v,double theta,double scale)
{
   this->u=u;
   this->v=v;
   this->orientation=theta;
   this->scale=scale;
}

inline void sift_feature::get_u_v_orientation_scale(
   double& u,double& v,double& orientation,double& scale) const
{
   u=this->u;
   v=this->v;
   orientation=this->orientation;
   scale=this->scale;
}



#endif  // sift_feature.h
