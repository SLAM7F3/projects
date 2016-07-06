// Note added on 1/23/08: VALGRIND indicates there's a clash between
// object pooling and OSG reference counting within this class !!!

// ==========================================================================
// Header file for RegionPolyLine class
// ==========================================================================
// Last updated on 12/13/08; 5/3/09
// ==========================================================================

#ifndef RegionPolyLine_H
#define RegionPolyLine_H

#include "osg/osgGeometry/PolyLine.h"

class RegionPolyLine : public PolyLine
{

  public:
    
// Initialization, constructor and destructor functions:

   RegionPolyLine();
   RegionPolyLine(const int p_ndim,Pass* PI_ptr,threevector* GO_ptr,
               osgText::Font* f_ptr,int n_text_messages,int id);
   RegionPolyLine(const int p_ndim,Pass* PI_ptr,threevector* GO_ptr,
               const threevector& reference_origin,
               const std::vector<threevector>& V,osgText::Font* f_ptr,
               int n_text_messages,int id);
   virtual ~RegionPolyLine();
   friend std::ostream& operator<< (
      std::ostream& outstream,const RegionPolyLine& l);

// Set & get methods:

  protected:

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PolyLine& p);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // RegionPolyLine.h



