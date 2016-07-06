// ==========================================================================
// Header file for CENTERPICKHANDLER class
// ==========================================================================
// Last modfied on 9/19/06; 12/29/06; 8/17/07
// ==========================================================================

#ifndef CENTER_PICK_HANDLER_H
#define CENTER_PICK_HANDLER_H

#include <iostream>
#include <osgGA/GUIEventAdapter>
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/GraphicalPickHandler.h"
#include "passes/PassesGroup.h"

class ModeController;
class WindowManager;

class CenterPickHandler : public GraphicalPickHandler
{

  public: 

   CenterPickHandler(
      const int p_ndims,Pass* PI_ptr,
      osgGA::CustomManipulator* CM_ptr,CentersGroup* CG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,
      threevector* GO_ptr=NULL);

// Set & get methods:

   CentersGroup* get_CentersGroup_ptr();
   const CentersGroup* get_CentersGroup_ptr() const;

   float get_max_distance_to_Graphical();
   void edit_center_location();

  protected:

   virtual ~CenterPickHandler();

// Mouse pick event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool release();

  private:

   CentersGroup* CentersGroup_ptr;
   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline CentersGroup* CenterPickHandler::get_CentersGroup_ptr()
{
   return CentersGroup_ptr;
}

inline const CentersGroup* CenterPickHandler::get_CentersGroup_ptr() 
   const
{
   return CentersGroup_ptr;
}

#endif // CenterPickHandler.h



