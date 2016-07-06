// ==========================================================================
// Header file for PanoramaPickHandler class
// ==========================================================================
// Last modfied on 2/24/11
// ==========================================================================

#ifndef PANORAMA_PICKHANDLER_H
#define PANORAMA_PICKHANDLER_H

#include "osg/osgGrid/Grid.h"
#include "osg/osgGeometry/GeometricalPickHandler.h"

// class osg::Group;
class ModeController;
class Panorama;
class PanoramasGroup;
class WindowManager;

class PanoramaPickHandler : public GeometricalPickHandler
{

  public: 

   PanoramaPickHandler(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,PanoramasGroup* OFG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr,threevector* GO_ptr);

   virtual ~PanoramaPickHandler();

// Set & get member functions:

   void set_mask_nonselected_OSGsubPATs_flag(bool flag);
   void set_Grid_ptr(Grid* Grid_ptr);
   void set_disallow_Panorama_doubleclicking_flag(bool flag);

// Panorama generation, manipulation and annihilation methods:

  protected:

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   virtual bool release();
      
  private:

   bool mask_nonselected_OSGsubPATs_flag;
   bool disallow_Panorama_doubleclicking_flag;
   int rotate_about_curr_eyept_counter;
   Grid* Grid_ptr;
   PanoramasGroup* PanoramasGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   bool select_Panorama();
   bool select_Panorama(double U,double V);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PanoramaPickHandler::set_mask_nonselected_OSGsubPATs_flag(
   bool flag)
{
   mask_nonselected_OSGsubPATs_flag=flag;
}

inline void PanoramaPickHandler::set_Grid_ptr(Grid* Grid_ptr)
{
   this->Grid_ptr=Grid_ptr;
}

inline void 
PanoramaPickHandler::set_disallow_Panorama_doubleclicking_flag(bool flag)
{
   disallow_Panorama_doubleclicking_flag=flag;
}

#endif // PanoramaPickHandler.h



