// ==========================================================================
// Header file for ARMYSYMBOLSGROUP class
// ==========================================================================
// Last modified on 11/10/06; 1/9/07; 2/7/07
// ==========================================================================

#ifndef ARMYSYMBOLSGROUP_H
#define ARMYSYMBOLSGROUP_H

#include <iostream>
#include "osg/osgAnnotators/AnnotatorsGroup.h"
#include "osg/osgAnnotators/ArmySymbol.h"
#include "osg/osgGeometry/BoxesGroup.h"

class Clock;
class Ellipsoid_model;

class ArmySymbolsGroup : public BoxesGroup, public AnnotatorsGroup
{

  public:

// Initialization, constructor and destructor functions:

   ArmySymbolsGroup(Pass* PI_ptr,threevector* GO_ptr);
   ArmySymbolsGroup(Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,
                    threevector* GO_ptr);
   virtual ~ArmySymbolsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const ArmySymbolsGroup& A);

// Set & get methods:

   ArmySymbol* get_ArmySymbol_ptr(int n) const;
   ArmySymbol* get_ID_labeled_ArmySymbol_ptr(int ID) const;

// ArmySymbol creation and manipulation methods:

   ArmySymbol* generate_new_ArmySymbol(int ID=-1);
   void generate_armysymbol_group(ArmySymbol* armysymbol_ptr);
   int destroy_ArmySymbol();

  protected:

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ArmySymbolsGroup& A);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline ArmySymbol* ArmySymbolsGroup::get_ArmySymbol_ptr(int n) const
{
   return dynamic_cast<ArmySymbol*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline ArmySymbol* ArmySymbolsGroup::get_ID_labeled_ArmySymbol_ptr(int ID) 
   const
{
   return dynamic_cast<ArmySymbol*>(get_ID_labeled_Graphical_ptr(ID));
}



#endif // ArmySymbolsGroup.h



