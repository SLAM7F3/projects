// ==========================================================================
// POWERPOINTSGROUP class member function definitions
// ==========================================================================
// Last modified on 8/24/07; 8/25/07; 11/27/07; 6/15/08
// ==========================================================================

#include <iostream>
#include <string>
#include "messenger/Messenger.h"
#include "osg/osgAnnotators/PowerPointsGroup.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PowerPointsGroup::allocate_member_objects()
{
}		       

void PowerPointsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="PowerPointsGroup";

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<PowerPointsGroup>(
         this, &PowerPointsGroup::update_display));
}		       

PowerPointsGroup::PowerPointsGroup(Pass* PI_ptr,threevector* GO_ptr):
   BoxesGroup(PI_ptr,GO_ptr), AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

PowerPointsGroup::PowerPointsGroup(
   Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,threevector* GO_ptr):
   BoxesGroup(PI_ptr,clock_ptr,EM_ptr,GO_ptr),AnnotatorsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

PowerPointsGroup::~PowerPointsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PowerPointsGroup& A)
{
   int node_counter=0;
   for (unsigned int n=0; n<A.get_n_Graphicals(); n++)
   {
      PowerPoint* PowerPoint_ptr=A.get_PowerPoint_ptr(n);
      outstream << "PowerPoint node # " << node_counter++ << endl;
      outstream << "PowerPoint = " << *PowerPoint_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// PowerPoint creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_PowerPoint from all other graphical insertion
// and manipulation methods...

PowerPoint* PowerPointsGroup::generate_new_PowerPoint(int ID)
{
   if (ID==-1) ID=get_next_unused_ID();
   PowerPoint* curr_PowerPoint_ptr=new PowerPoint(width,length,height,ID);

   string filename;
   cout << "Enter PowerPoint filename:" << endl;
   cin >> filename;
   curr_PowerPoint_ptr->set_filename(filename);
   GraphicalsGroup::insert_Graphical_into_list(curr_PowerPoint_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_PowerPoint_ptr,0);

   return curr_PowerPoint_ptr;
}

// --------------------------------------------------------------------------
void PowerPointsGroup::generate_powerpoint_group(PowerPoint* powerpoint_ptr)
{
//   cout << "inside ASG::generate_powerpoint_geode()" << endl;

   osg::Group* group_ptr=powerpoint_ptr->generate_drawable_group();
   powerpoint_ptr->get_PAT_ptr()->addChild(group_ptr);

// Orient spheresegment so that it points radially inward wrt earth's
// center if Earth's ellipsoid_model_ptr != NULL:

   rotate_zhat_to_rhat(powerpoint_ptr);
}

// --------------------------------------------------------------------------
// Member function destroy_PowerPoint removes the selected PowerPoint
// from the PowerPointslist and the OSG group.  If the PowerPoint is
// successfully destroyed, its number is returned by this method.
// Otherwise, -1 is returned.

int PowerPointsGroup::destroy_PowerPoint()
{   
//   cout << "inside PowerPointsGroup::destroy_PowerPoint()" << endl;
   int destroyed_PowerPoint_number=destroy_Graphical();
   return destroyed_PowerPoint_number;
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

// Member function issue_invocation_message sends the name of a
// selected powerpoint file to a powerpoint messenger queue.

bool PowerPointsGroup::issue_invocation_message()
{   
   cout << "inside PowerPointsGroup::issue_invocation_message()" << endl;

   int selected_Graphical_ID=get_selected_Graphical_ID();
   cout << "selected_Graphical_ID = " << selected_Graphical_ID << endl;
   
   if (selected_Graphical_ID >= 0)
   {
      string text_message=get_PowerPoint_ptr(selected_Graphical_ID)->
         get_filename();
      cout << "text_message = " << text_message << endl;
      if (get_Messenger_ptr() != NULL)
         get_Messenger_ptr()->sendTextMessage(text_message);
      return true;
   }
   else
   {
      return false;
   }
}
