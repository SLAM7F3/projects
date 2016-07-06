// ==========================================================================
// Header file for GEOMETRICALPICKHANDLER class
// ==========================================================================
// Last modfied on 1/21/07; 6/16/07; 8/17/07; 6/30/08
// ==========================================================================

#ifndef GEOMETRICAL_PICK_HANDLER_H
#define GEOMETRICAL_PICK_HANDLER_H

#include "osg/osgGraphicals/GraphicalPickHandler.h"

class ModeController;
class WindowManager;

class GeometricalPickHandler : public GraphicalPickHandler
{

  public: 

   GeometricalPickHandler(
      const int p_ndims,Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
      GraphicalsGroup* GG_ptr,ModeController* MC_ptr,
      WindowManager* WCC_ptr,threevector* GO_ptr=NULL);

// Set and get methods:

   void set_Allow_Insertion_flag(bool flag);
   void set_Allow_Manipulation_flag(bool flag);

// Geometrical generation, manipulation and annihilation methods:

   virtual float get_max_distance_to_Graphical();

  protected:

   bool Allow_Insertion_flag,Allow_Manipulation_flag;

   virtual ~GeometricalPickHandler();

// Mouse event handling methods:

   virtual bool doubleclick(const osgGA::GUIEventAdapter& ea);
   
  private:

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void GeometricalPickHandler::set_Allow_Insertion_flag(bool flag)
{
   Allow_Insertion_flag=flag;
}

inline void GeometricalPickHandler::set_Allow_Manipulation_flag(bool flag)
{
   Allow_Manipulation_flag=flag;
}


#endif // GeometricalPickHandler.h



