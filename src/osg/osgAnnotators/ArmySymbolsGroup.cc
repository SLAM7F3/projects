// ==========================================================================
// ARMYSYMBOLSGROUP class member function definitions
// ==========================================================================
// Last modified on 1/4/07; 1/26/07; 2/7/07; 11/27/07
// ==========================================================================

#include <iostream>
#include "osg/osgAnnotators/ArmySymbol.h"
#include "osg/osgAnnotators/ArmySymbolsGroup.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ArmySymbolsGroup::allocate_member_objects()
{
}		       

void ArmySymbolsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="ArmySymbolsGroup";
   
   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<ArmySymbolsGroup>(
         this, &ArmySymbolsGroup::update_display));
}		       

ArmySymbolsGroup::ArmySymbolsGroup(Pass* PI_ptr,threevector* GO_ptr):
   BoxesGroup(PI_ptr,GO_ptr), AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

ArmySymbolsGroup::ArmySymbolsGroup(
   Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,threevector* GO_ptr):
   BoxesGroup(PI_ptr,clock_ptr,EM_ptr,GO_ptr),AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

ArmySymbolsGroup::~ArmySymbolsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const ArmySymbolsGroup& A)
{
   int node_counter=0;
   for (unsigned int n=0; n<A.get_n_Graphicals(); n++)
   {
      ArmySymbol* ArmySymbol_ptr=A.get_ArmySymbol_ptr(n);
      outstream << "ArmySymbol node # " << node_counter++ << endl;
      outstream << "ArmySymbol = " << *ArmySymbol_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// ArmySymbol creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_ArmySymbol from all other graphical insertion
// and manipulation methods...

ArmySymbol* ArmySymbolsGroup::generate_new_ArmySymbol(int ID)
{
   if (ID==-1) ID=get_next_unused_ID();
   ArmySymbol* curr_ArmySymbol_ptr=new ArmySymbol(width,length,height,ID);
   GraphicalsGroup::insert_Graphical_into_list(curr_ArmySymbol_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_ArmySymbol_ptr,0);
   return curr_ArmySymbol_ptr;
}

// --------------------------------------------------------------------------
void ArmySymbolsGroup::generate_armysymbol_group(ArmySymbol* armysymbol_ptr)
{
//   cout << "inside ASG::generate_armysymbol_geode()" << endl;

   osg::Group* group_ptr=armysymbol_ptr->generate_drawable_group();
   armysymbol_ptr->get_PAT_ptr()->addChild(group_ptr);

// Orient spheresegment so that it points radially inward wrt earth's
// center if Earth's ellipsoid_model_ptr != NULL:

   rotate_zhat_to_rhat(armysymbol_ptr);
}

// --------------------------------------------------------------------------
// Member function destroy_ArmySymbol removes the selected ArmySymbol
// from the ArmySymbolslist and the OSG group.  If the ArmySymbol is
// successfully destroyed, its number is returned by this method.
// Otherwise, -1 is returned.

int ArmySymbolsGroup::destroy_ArmySymbol()
{   
//   cout << "inside ArmySymbolsGroup::destroy_ArmySymbol()" << endl;
   int destroyed_ArmySymbol_number=destroy_Graphical();
   return destroyed_ArmySymbol_number;
}
