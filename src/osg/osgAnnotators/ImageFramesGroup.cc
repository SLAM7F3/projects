// ==========================================================================
// IMAGEFRAMESGROUP class member function definitions
// ==========================================================================
// Last modified on 1/17/07; 1/19/07; 2/22/07; 10/13/07; 11/27/07
// ==========================================================================

#include <iomanip>
#include <vector>
#include "osg/osgAnnotators/ImageFrame.h"
#include "osg/osgAnnotators/ImageFramesGroup.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ImageFramesGroup::allocate_member_objects()
{
}		       

void ImageFramesGroup::initialize_member_objects()
{
   GraphicalsGroup_name="ImageFramesGroup";

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<ImageFramesGroup>(
         this, &ImageFramesGroup::update_display));
}		       

ImageFramesGroup::ImageFramesGroup(Pass* PI_ptr,AnimationController* AC_ptr):
   GraphicalsGroup(3,PI_ptr,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

ImageFramesGroup::~ImageFramesGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const ImageFramesGroup& f)
{
   int node_counter=0;
   for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
   {
      ImageFrame* ImageFrame_ptr=f.get_ImageFrame_ptr(n);
      outstream << "ImageFrame node # " << node_counter++ << endl;
      outstream << "ImageFrame = " << *ImageFrame_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// ImageFrame creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_ImageFrame from all other graphical insertion
// and manipulation methods...

ImageFrame* ImageFramesGroup::generate_new_ImageFrame(int ID)
{
//   cout << "inside ImageFramesGroup::generate_new_ImageFrame()" << endl;

   if (ID==-1) ID=get_next_unused_ID();
   ImageFrame* curr_ImageFrame_ptr=new ImageFrame(
      pass_ptr,ID,AnimationController_ptr);
   GraphicalsGroup::insert_Graphical_into_list(curr_ImageFrame_ptr);
//   insert_graphical_PAT_into_OSGsubPAT(curr_ImageFrame_ptr,0);

   get_OSGsubPAT_ptr(0)->addChild(
      curr_ImageFrame_ptr->get_linesegments_group_ptr()->get_OSGgroup_ptr());
   get_OSGsubPAT_ptr(0)->setNodeMask(1);

   curr_ImageFrame_ptr->get_linesegments_group_ptr()->
      set_AnimationController_ptr(AnimationController_ptr);
   return curr_ImageFrame_ptr;
}

// ---------------------------------------------------------------------
void ImageFramesGroup::update_display()
{   
//   cout << "inside IFG::update_display()" << endl;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      get_ImageFrame_ptr(n)->get_linesegments_group_ptr()->
         update_display();
   }
   GraphicalsGroup::update_display();
}
