// ==========================================================================
// SignPostsKeyHandler header file 
// ==========================================================================
// Last modified on 9/30/09; 10/1/09; 10/2/09
// ==========================================================================

#ifndef SIGNPOSTSKEYHANDLER_H
#define SIGNPOSTSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"
#include "image/TwoDarray.h"

#include "osg/osgGraphicals/AnimationController.h"

class SignPostsGroup;
class ModeController;

class SignPostsKeyHandler : public GraphicalsKeyHandler
{
  public:

   SignPostsKeyHandler(SignPostsGroup* SPG_ptr,ModeController* MC_ptr);

//   void set_DTED_ztwoDarray_ptr(twoDarray* ztwoDarray_ptr);
   void set_AnimationController_ptr(AnimationController* AC_ptr);
//   void set_PolyhedraGroup_ptr(PolyhedraGroup* PG_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );


  protected:

   virtual ~SignPostsKeyHandler();

  private:

   SignPostsGroup* SignPostsGroup_ptr;
//   twoDarray* DTED_ztwoDarray_ptr;
   AnimationController* AnimationController_ptr;
//   PolyhedraGroup* PolyhedraGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

/*
inline void SignPostsKeyHandler::set_DTED_ztwoDarray_ptr(
   twoDarray* ztwoDarray_ptr)
{
   DTED_ztwoDarray_ptr=ztwoDarray_ptr;
}
*/

inline void SignPostsKeyHandler::set_AnimationController_ptr(
   AnimationController* AC_ptr)
{
   AnimationController_ptr=AC_ptr;
}

/*
inline void SignPostsKeyHandler::set_PolyhedraGroup_ptr(
   PolyhedraGroup* PG_ptr)
{
   PolyhedraGroup_ptr=PG_ptr;
}
*/


#endif 
