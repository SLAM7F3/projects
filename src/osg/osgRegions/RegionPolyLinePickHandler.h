// ==========================================================================
// Header file for REGIONPOLYLINEPICKHANDLER class
// ==========================================================================
// Last modfied on 12/13/08; 12/14/08; 12/15/08; 5/1/09
// ==========================================================================

#ifndef REGION_POLYLINE_PICK_HANDLER_H
#define REGION_POLYLINE_PICK_HANDLER_H

#include "osg/osgGeometry/PolyLinePickHandler.h"

class RegionPolyLinesGroup;

class RegionPolyLinePickHandler : public PolyLinePickHandler
{

  public: 

   RegionPolyLinePickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
      RegionPolyLinesGroup* RPLG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr);

// Set & get member functions:

   void set_process_pick_flag(bool flag);
   void set_label_prefix(std::string prefix);

  protected:

   virtual ~RegionPolyLinePickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool release();

  private:

   bool process_pick_flag;
   std::string label_prefix;
   RegionPolyLinesGroup* RegionPolyLinesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void RegionPolyLinePickHandler::set_process_pick_flag(bool flag)
{
   process_pick_flag=flag;
}

inline void RegionPolyLinePickHandler::set_label_prefix(std::string prefix)
{
   label_prefix=prefix;
}

#endif // RegionPolyLinePickHandler.h
