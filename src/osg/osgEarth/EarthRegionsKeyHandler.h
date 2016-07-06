// ==========================================================================
// EarthRegionsKeyHandler header file 
// ==========================================================================
// Last modified on 5/20/08
// ==========================================================================

#ifndef EARTHREGIONSKEYHANDLER_H
#define EARTHREGIONSKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class EarthRegionsGroup;
class ModeController;
class MoviesGroup;

class EarthRegionsKeyHandler : public GraphicalsKeyHandler
{
  public:

   EarthRegionsKeyHandler(EarthRegionsGroup* ERG_ptr,ModeController* MC_ptr,
                          MoviesGroup* MG_ptr=NULL);

   EarthRegionsGroup* const get_EarthRegionsGroup_ptr();

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~EarthRegionsKeyHandler();

  private:

   EarthRegionsGroup* EarthRegionsGroup_ptr;
   MoviesGroup* MoviesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

#endif 
