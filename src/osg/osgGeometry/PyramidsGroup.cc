// ==========================================================================
// PYRAMIDSGROUP class member function definitions
// ==========================================================================
// Last modified on 12/15/07; 12/17/07; 2/17/08; 4/5/14
// ==========================================================================

#include <iomanip>
#include <string>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/osgGeometry/PyramidsGroup.h"
#include "geometry/pyramid.h"

#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PyramidsGroup::allocate_member_objects()
{
}		       

void PyramidsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="PyramidsGroup";

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<PyramidsGroup>(
         this, &PyramidsGroup::update_display));
}		       

PyramidsGroup::PyramidsGroup(Pass* PI_ptr,threevector* GO_ptr,
                             AnimationController* AC_ptr):
   PolyhedraGroup(PI_ptr,GO_ptr,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

PyramidsGroup::~PyramidsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PyramidsGroup& P)
{
   int node_counter=0;
   for (unsigned int n=0; n<P.get_n_Graphicals(); n++)
   {
      Pyramid* Pyramid_ptr=P.get_Pyramid_ptr(n);
      outstream << "Pyramid node # " << node_counter++ << endl;
      outstream << "Pyramid = " << *Pyramid_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Pyramid creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Pyramid from all other graphical insertion
// and manipulation methods...

Pyramid* PyramidsGroup::generate_new_Pyramid(int ID)
{
//   cout << "inside PyramidsGroup::generate_new_Pyramid()" << endl;
   if (ID==-1) ID=get_next_unused_ID();

   Pyramid* curr_Pyramid_ptr=new Pyramid(
      pass_ptr,get_grid_world_origin(),NULL,
      font_refptr.get(),ID,AnimationController_ptr);

   unsigned int OSGsubPAT_number=0;
   initialize_new_Pyramid(curr_Pyramid_ptr,OSGsubPAT_number);

   return curr_Pyramid_ptr;
}

Pyramid* PyramidsGroup::generate_new_Pyramid(pyramid* p_ptr,int ID)
{
//   cout << "inside PyramidsGroup::generate_new_Pyramid()" << endl;
//   cout << "p_ptr = " << p_ptr << endl;
   if (ID==-1) ID=get_next_unused_ID();
   Pyramid* curr_Pyramid_ptr=new Pyramid(
      pass_ptr,get_grid_world_origin(),p_ptr,font_refptr.get(),
      ID,AnimationController_ptr);

   unsigned int OSGsubPAT_number=0;
   initialize_new_Pyramid(curr_Pyramid_ptr,OSGsubPAT_number);

   osg::Geode* geode_ptr=curr_Pyramid_ptr->generate_drawable_geode();
   curr_Pyramid_ptr->get_PAT_ptr()->addChild(geode_ptr);

   return curr_Pyramid_ptr;
}

// ---------------------------------------------------------------------
void PyramidsGroup::initialize_new_Pyramid(
   Pyramid* Pyramid_ptr,unsigned int OSGsubPAT_number)
{
//   cout << "inside PyramidsGroup::initialize_new_Pyramid()" << endl;
   
   GraphicalsGroup::insert_Graphical_into_list(Pyramid_ptr);

// Note added on 1/17/07: We should someday write a
// destroy_Pyramid() method which should explicitly remove the
// following LineSegmentsGroup->OSGGroup_ptr from curr_Pyramid
// before deleting curr_Pyramid...

   initialize_Graphical(Pyramid_ptr);
   Pyramid_ptr->get_PAT_ptr()->addChild(
      Pyramid_ptr->get_LineSegmentsGroup_ptr()->get_OSGgroup_ptr());
   Pyramid_ptr->get_LineSegmentsGroup_ptr()->
      set_AnimationController_ptr(AnimationController_ptr);

   insert_graphical_PAT_into_OSGsubPAT(Pyramid_ptr,OSGsubPAT_number);
}

// --------------------------------------------------------------------------
void PyramidsGroup::generate_pyramid_geode(Pyramid* Pyramid_ptr)
{
//   cout << "inside PyramidsGroup::generate_pyramid_geode()" << endl;
//   cout << "Pyramid_ptr->get_pyramid_ptr() = "
//        << Pyramid_ptr->get_pyramid_ptr() << endl;
   
   Pyramid_ptr->set_polyhedron_ptr(Pyramid_ptr->get_pyramid_ptr());
   osg::Geode* geode_ptr=Pyramid_ptr->generate_drawable_geode();
   Pyramid_ptr->get_PAT_ptr()->addChild(geode_ptr);
}

// --------------------------------------------------------------------------
// Member function update_display()

void PyramidsGroup::update_display()
{
//   cout << "inside PyramidsGroup::update_display()" << endl;
   GraphicalsGroup::update_display();
}
