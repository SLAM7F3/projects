// ==========================================================================
// Header file for CENTER class
// ==========================================================================
// Last modified on 10/10/05
// ==========================================================================

#ifndef CENTER_H
#define CENTER_H

#include <iostream>
#include "osg/osgGraphicals/Graphical.h"

class Center: public Graphical
{

  public:

// Initialization, constructor and destructor functions:

   Center(const int p_ndims,int id);
   virtual ~Center();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Center& c);

// Set & get member functions:

  protected:

  private:

   void allocate_member_objects();
   void initialize_member_objects();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================


#endif // Center.h
