// ==========================================================================
// Header file for LOSMODEL class
// ==========================================================================
// Last updated on 9/19/09
// ==========================================================================

#ifndef LOSMODEL_H
#define LOSMODEL_H

#include "osg/osgModels/MODEL.h"

class LOSMODEL : public MODEL
{

  public:
    
// Initialization, constructor and destructor functions:

   LOSMODEL(threevector* GO_ptr,std::string filename,int id);
   virtual ~LOSMODEL();
   friend std::ostream& operator<< (
      std::ostream& outstream,const LOSMODEL& m);

// Set & get methods:

// Model manipulation member functions:

// Observation frusta instantiation and manipulation member functions:

// Dynamic OBSFRUSTA manipulation member functions:

// ActiveMQ broadcast member functions:

  protected:

  private:

   void allocate_member_objects();
   void initialize_member_objects();
//   void docopy(const LOSMODEL& m);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif // LOSMODEL.h



 
