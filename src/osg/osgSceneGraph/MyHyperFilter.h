// ========================================================================
// Minor variant of Ross' HyperFilter header file
// ========================================================================
// Last updated on 11/27/11; 12/3/11
// ========================================================================

#ifndef __MYHYPERFILTER_H__
#define __MYHYPERFILTER_H__ 1

#include <osg/Array>
#include <osg/Geode>
#include <osg/Node>
#include <osg/NodeCallback>
#include <osg/PrimitiveSet>

#include <config.h>
#include <model/HyperBoundingBox.h>
#include <model/Metadata.h>

namespace model {


/** A MyHyperFilter is used to filter a given scene graph by
 *  temporarily hiding primitives which do not fall inside the
 *  given HyperBoundingBox. The filter uses the higher dimensions
 *  specified by Metadata objects attached to drawables in the
 *  scene. The MyHyperFilter will replace the PrimitiveSet objects
 *  attached in the scene to show only the appropriate data. Therefore,
 *  any pre-existing PrimitiveSet objects will be overwritten.
 */
   class MyHyperFilter : public osg::Referenced
   {
     public:
		
      MyHyperFilter();
		
// Get and set the hyper bounding box that encloses the desired range.

      HyperBoundingBox getHyperBoundingBox() const { return _hbb; }
      void setHyperBoundingBox( const HyperBoundingBox& hbb );
		
// !!! Rather than having a single hbb, there should be multiple
// allowed regions.  Also, it should be either inclusive or exclusive.

      inline unsigned long getCurrentUpdateIndex() const { 
         return _currentUpdateIndex; }
		
// Does the given bounds at least partially intersect the filter
// bounds? 

      bool intersects( const HyperBoundingBox& hbb ) 
      { return _hbb.intersects(hbb); }
		
      /* Is the given bounds entirely contained by the filter bounds? */
      bool contains( const HyperBoundingBox& hbb ) 
      { return _hbb.contains(hbb); }
		
      /* Update the filtering of the given node. */
      void update( osg::Geode& geode, osg::NodePath& nodepath );
		
      /* Filter the given data and set by the current filter parameters. */
      osg::PrimitiveSet* filter( 
         osg::PrimitiveSet* unfiltered, osg::Vec3Array* vertices, 
         model::Metadata* metadata, const osg::Matrix& localToWorld );

      /* Return the cull callback that must be called to update the
       * filter for each Geode. */

      osg::NodeCallback* get_UpdateCallback_ptr();

     protected:
		
      virtual ~MyHyperFilter();
		
      HyperBoundingBox _hbb;
      osg::ref_ptr<osg::NodeCallback> _updateCallback;
      unsigned long _currentUpdateIndex;

      friend class UpdateMyHyperFilterCallback;

     private:

      void allocate_member_objects();
      void initialize_member_objects();
      void IncrementUpdateIndex();
         
   };

} // namespace model

#endif // __MYHYPERFILTER_H__
