// ==========================================================================
// Header file for SIGNPOSTPICKHANDLER class
// ==========================================================================
// Last modfied on 2/7/07; 6/16/07; 9/13/07; 10/8/07; 1/27/08
// ==========================================================================

#ifndef SIGNPOST_PICK_HANDLER_H
#define SIGNPOST_PICK_HANDLER_H

#include "osg/osgGeometry/GeometricalPickHandler.h"

// class osg::Group;
class ModeController;
class SignPost;
class SignPostsGroup;
class WindowManager;

class SignPostPickHandler : public GeometricalPickHandler
{

  public: 

   SignPostPickHandler(
      const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
      SignPostsGroup* SPG_ptr,ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr);
   virtual ~SignPostPickHandler();

   void set_allow_doubleclick_in_manipulate_fused_data_mode(bool flag);

  protected:

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool release();
   virtual float get_max_distance_to_Graphical();
      
  private:

   bool allow_doubleclick_in_manipulate_fused_data_mode;
   SignPostsGroup* SignPostsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   bool instantiate_SignPost(double X,double Y);
   bool select_SignPost();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void SignPostPickHandler::set_allow_doubleclick_in_manipulate_fused_data_mode(bool flag)
{
   allow_doubleclick_in_manipulate_fused_data_mode=flag;
}


#endif // SignPostPickHandler.h



