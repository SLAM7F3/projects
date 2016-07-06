// ==========================================================================
// Header file for ClassificationHUD class which displays an
// Classification message
// ==========================================================================
// Last modified on 5/2/08; 9/8/08; 10/4/08; 10/6/08
// ==========================================================================

#ifndef ClassificationHUD_H
#define ClassificationHUD_H

#include "osg/GenericHUD.h"
#include "passes/PassesGroup.h"

class ClassificationHUD : public GenericHUD
{
  public:

   ClassificationHUD(PassesGroup::ClassificationType classification=
           PassesGroup::unclassified);
   void showHUD();

// Set & get member functions:

  protected:

  private:

   PassesGroup::ClassificationType classification;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:


#endif 
