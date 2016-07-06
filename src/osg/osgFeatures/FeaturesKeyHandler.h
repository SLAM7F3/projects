// ==========================================================================
// FeaturesKeyHandler header file 
// ==========================================================================
// Last modified on 12/21/05; 7/10/06; 1/3/07
// ==========================================================================

#ifndef FEATURESKEYHANDLER_H
#define FEATURESKEYHANDLER_H

#include "osg/osgGraphicals/GraphicalsKeyHandler.h"

class FeaturesGroup;
class ModeController;

class FeaturesKeyHandler : public GraphicalsKeyHandler
{
  public:

   FeaturesKeyHandler(const int p_ndims,FeaturesGroup* FG_ptr,
                      ModeController* MC_ptr);

   FeaturesGroup* const get_FeaturesGroup_ptr() const;

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

  protected:

   virtual ~FeaturesKeyHandler();

  private:

   int ndims;
   FeaturesGroup* FeaturesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline FeaturesGroup* const FeaturesKeyHandler::get_FeaturesGroup_ptr() const
{
   return FeaturesGroup_ptr;
}

#endif 
