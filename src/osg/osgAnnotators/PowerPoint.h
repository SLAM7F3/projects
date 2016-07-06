// ==========================================================================
// Header file for PowerPoint class
// ==========================================================================
// Last updated on 8/24/07
// ==========================================================================

#ifndef PowerPoint_H
#define PowerPoint_H

#include <string>
#include "osg/osgGeometry/Box.h"

// class osg::Group;

class PowerPoint : public Box
{

  public:
    
// Initialization, constructor and destructor functions:

   PowerPoint(double w,double l,double h,int id);
   PowerPoint(double w,double l,double h,double displacement,int id);
   virtual ~PowerPoint();
   friend std::ostream& operator<< (
      std::ostream& outstream,const PowerPoint& PP);

// Set & get methods:

   void set_filename(std::string filename);
   std::string get_filename() const;

// Drawing methods:

   osg::Group* generate_drawable_group();

  protected:

  private:

   std::string filename;
   
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PowerPoint& s);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PowerPoint::set_filename(std::string filename)
{
   this->filename=filename;
}

inline std::string PowerPoint::get_filename() const
{
   return filename;
}


#endif // PowerPoint.h



   
