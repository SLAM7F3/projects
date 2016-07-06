// ==========================================================================
// Header file for rubbersheet class
// ==========================================================================
// Last modified on 2/6/08; 9/14/08
// ==========================================================================

#ifndef RUBBERSHEET_H
#define RUBBERSHEET_H

#include <vector>
#include "math/threevector.h"
#include "math/twovector.h"

class genmatrix;

class rubbersheet
{

  public:

// Initialization, constructor and destructor functions:

   rubbersheet();
   rubbersheet(const std::vector<twovector>& xyid,
               const std::vector<twovector>& uv);
   rubbersheet(const std::vector<threevector>& xyid,
               const std::vector<twovector>& uv);
   rubbersheet(const rubbersheet& r);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~rubbersheet();
   rubbersheet& operator= (const rubbersheet& r);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const rubbersheet& r);

// Set and get member functions:

   unsigned int get_n_features() const;
   genmatrix* get_M_ptr();
   const genmatrix* get_M_ptr() const;
   genmatrix* get_Minv_ptr();
   const genmatrix* get_Minv_ptr() const;
   twovector& get_trans();
   const twovector& get_trans() const;

   void copy_XYID(const std::vector<threevector>& xyid);
   void copy_UV(const std::vector<twovector>& uv);

// Warping member functions:

   void fit_linear_warp(std::vector<bool>& retained_feature_flag);
   double compare_measured_and_linearly_transformed_UVs(
      int npoints_to_discard,std::vector<bool>& retained_feature_flag);

  private: 

   unsigned int n_features;
   std::vector<twovector> UV;
   std::vector<threevector> XYID;
   genmatrix *M_ptr,*Minv_ptr;
   twovector trans;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const rubbersheet& r);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline unsigned int rubbersheet::get_n_features() const
{
   return n_features;
}

inline genmatrix* rubbersheet::get_M_ptr() 
{
   return M_ptr;
}

inline const genmatrix* rubbersheet::get_M_ptr() const
{
   return M_ptr;
}

inline genmatrix* rubbersheet::get_Minv_ptr() 
{
   return Minv_ptr;
}

inline const genmatrix* rubbersheet::get_Minv_ptr() const
{
   return Minv_ptr;
}

inline twovector& rubbersheet::get_trans()
{
   return trans;
}

inline const twovector& rubbersheet::get_trans() const
{
   return trans;
}

#endif  // rubbersheet.h



