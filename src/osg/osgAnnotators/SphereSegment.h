// ==========================================================================
// Header file for SphereSegment class
// ==========================================================================
// Last updated on 1/7/07; 1/21/07; 2/6/07; 12/2/11
// ==========================================================================

#ifndef SphereSegment_H
#define SphereSegment_H

#include <osgSim/SphereSegment>
#include "osg/osgGeometry/Geometrical.h"

class SphereSegment : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   SphereSegment(int id,double radius,const osg::Vec3& posn,
                 double az_min,double az_max,double el_min,double el_max);
   virtual ~SphereSegment();
   friend std::ostream& operator<< (
      std::ostream& outstream,const SphereSegment& s);

// Set and get methods:

/*
   osgSim::SphereSegment* get_SphereSegment_ptr();
   const osgSim::SphereSegment* get_SphereSegment_ptr() const;
   osg::Group* get_blast_ptr();
   const osg::Group* get_blast_ptr() const;
*/

// Drawing methods:

   osg::Group* generate_drawable_group(
      bool display_spokes_flag,bool include_blast_flag);
   virtual void set_color(const osg::Vec4& color);   

  private:

   osg::Vec3 position;
   osg::ref_ptr<osgSim::SphereSegment> SphereSegment_refptr;
   osg::ref_ptr<osg::Group> group_refptr;

   void initialize_member_objects();
   void allocate_member_objects();
   void docopy(const SphereSegment& s);

   void GenerateBlast();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

/*
inline osgSim::SphereSegment* SphereSegment::get_SphereSegment_ptr()
{
   return SphereSegment_refptr.get();
}

inline const osgSim::SphereSegment* SphereSegment::get_SphereSegment_ptr() 
   const
{
   return SphereSegment_refptr.get();
}

inline osg::Group* SphereSegment::get_blast_ptr()
{
   return blast_refptr.get();
}

inline const osg::Group* SphereSegment::get_blast_ptr() const
{
   return blast_refptr.get();
}
*/

#endif // SphereSegment.h



