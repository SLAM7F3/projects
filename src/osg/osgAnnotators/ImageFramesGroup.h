// ==========================================================================
// Header file for IMAGEFRAMESGROUP class
// ==========================================================================
// Last modified on 8/27/06; 8/30/06; 9/19/06; 11/2/06; 10/13/07
// ==========================================================================

#ifndef IMAGEFRAMESGROUP_H
#define IMAGEFRAMESGROUP_H

#include <iostream>
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "osg/osgAnnotators/ImageFrame.h"

class AnimationController;

class ImageFramesGroup : public GraphicalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   ImageFramesGroup(Pass* PI_ptr,AnimationController* AC_ptr);
   virtual ~ImageFramesGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const ImageFramesGroup& f);

// Set & get methods:

   ImageFrame* get_ImageFrame_ptr(int n) const;
   ImageFrame* get_ID_labeled_ImageFrame_ptr(int ID) const;

// ImageFrame creation and manipulation methods:

   ImageFrame* generate_new_ImageFrame(int ID=-1);
   void update_display();

  protected:

  private:

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ImageFramesGroup& f);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline ImageFrame* ImageFramesGroup::get_ImageFrame_ptr(int n) const
{
   return dynamic_cast<ImageFrame*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline ImageFrame* ImageFramesGroup::get_ID_labeled_ImageFrame_ptr(
   int ID) const
{
   return dynamic_cast<ImageFrame*>(get_ID_labeled_Graphical_ptr(ID));
}


#endif // ImageFramesGroup.h



