// ========================================================================
// Ross' UpdateColormapCallback header file
// ========================================================================
// Last updated on 3/22/09; 3/23/09; 12/2/11
// ========================================================================

#ifndef UPDATECOLORMAPCALLBACK_H
#define UPDATECOLORMAPCALLBACK_H

#include <osg/NodeCallback>
#include <osg/NodeVisitor>
#include <model/Metadata.h>
#include "osg/osgSceneGraph/ColorGeodeVisitor.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "osg/osgSceneGraph/ParentVisitor.h"

class UpdateColormapCallback : public osg::NodeCallback
{

  public:
		
   UpdateColormapCallback(ColorMap* cm_ptr);
   virtual ~UpdateColormapCallback();

   void set_ColorGeodeVisitor_ptr(ColorGeodeVisitor* CGV_ptr);

   std::string get_Callback_name() const;
   bool getValue( 
      float& value, const osg::Vec3f& vertex, const model::Metadata* metadata,
      const unsigned int element, const int axis );

   virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

  protected:
   
  private:
   
   std::string Callback_name;
   ColorMap* ColorMap_ptr;
   ColorGeodeVisitor* ColorGeodeVisitor_ptr;
   osg::ref_ptr<ParentVisitor> ParentVisitor_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void UpdateColormapCallback::set_ColorGeodeVisitor_ptr(
   ColorGeodeVisitor* CGV_ptr)
{
   ColorGeodeVisitor_ptr=CGV_ptr;
}

inline std::string UpdateColormapCallback::get_Callback_name() const
{
   return Callback_name;
}

#endif // UpdateColormapCallback.h
