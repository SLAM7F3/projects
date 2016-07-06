// ==========================================================================
// GeometricalsKeyHandler header file 
// ==========================================================================
// Last modified on 5/22/11
// ==========================================================================

#ifndef GEOMETRICALSKEYHANDLER_H
#define GEOMETRICALSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class ModeController;
class GeometricalsGroup;

class GeometricalsKeyHandler : public GraphicalsKeyHandler
{

  public:

   GeometricalsKeyHandler(GeometricalsGroup* GG_ptr,ModeController* MC_ptr);

// Set and get methods:

   void set_Allow_Insertion_flag(bool flag);
   void set_Allow_Manipulation_flag(bool flag);

  protected:

   bool Allow_Insertion_flag,Allow_Manipulation_flag;
   virtual ~GeometricalsKeyHandler();

  private:

   GeometricalsGroup* GeometricalsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void GeometricalsKeyHandler::set_Allow_Insertion_flag(bool flag)
{
   Allow_Insertion_flag=flag;
}

inline void GeometricalsKeyHandler::set_Allow_Manipulation_flag(bool flag)
{
   Allow_Manipulation_flag=flag;
}



#endif 
