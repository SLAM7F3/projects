// ==========================================================================
// CentersKeyHandler header file 
// ==========================================================================
// Last modified on 7/10/06; 1/3/07
// ==========================================================================

#ifndef CENTERSKEYHANDLER_H
#define CENTERSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class CenterPickHandler;
class ModeController;

class CentersKeyHandler : public GraphicalsKeyHandler
{
  public:

   CentersKeyHandler(const int p_ndims,CenterPickHandler* CPH_ptr,
                     ModeController* MC_ptr);


   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   ~CentersKeyHandler();

  private:

   int ndims;
   CenterPickHandler* CenterPickHandler_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif 
