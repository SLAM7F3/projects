// ========================================================================
// MyNodeInfo header file.  Minor variant of Ross Anderson's NodeInfo
// class which includes osg::Marix _transform member.
// ========================================================================
// Last updated on 7/9/06; 8/20/06; 3/21/09
// ========================================================================

#ifndef __MYNODEINFO_H__
#define __MYNODEINFO_H__

#include <osg/Referenced>
#include <osg/Node>
#include <osg/PrimitiveSet>

#include <config.h>
#include <model/HyperBoundingBox.h>

namespace model {

   class MyNodeInfo;

/** Return the info class for the node. Create a default one if needed. */

   MyNodeInfo* getOrCreateMyInfoForNode( osg::Node& node );

/** MyNodeInfo is an UserData object that is attached, at runtime, to
 *  each Geode in the data set. Because viewer features, such
 *  as coloring the data, require that some information in the
 *  graph be modified, we can save state here, such as whether
 *  or not the newest colormap has been updated for this node yet.
 */
   class MyNodeInfo : public osg::Referenced
      {
	public:
		
         MyNodeInfo() :
            _colormapUpdateIndex(0),
            _filterUpdateIndex(0)
            {}
		
         inline unsigned long getColormapUpdateIndex() const { 
            return _colormapUpdateIndex; }
         inline void setColormapUpdateIndex( unsigned long i ) { 
            _colormapUpdateIndex = i; }
		
         inline unsigned long getFilterUpdateIndex() const { 
            return _filterUpdateIndex; }
         inline void setFilterUpdateIndex( unsigned long i ) { 
            _filterUpdateIndex = i; }
		
         inline const HyperBoundingBox& getUnfilteredBounds() const { 
            return _unfilteredBounds; }
         inline void setUnfilteredBounds( HyperBoundingBox hbb ) { 
            _unfilteredBounds = hbb; }

         inline const osg::PrimitiveSet* getUnfilteredPrimitiveSet() const { 
            return _unfilteredPrimitiveSet.get(); }
         inline osg::PrimitiveSet* getUnfilteredPrimitiveSet() { 
            return _unfilteredPrimitiveSet.get(); }
         inline void setUnfilteredPrimitiveSet( osg::PrimitiveSet* set ) { 
            _unfilteredPrimitiveSet = set; }
		
// Return true if the update index is less than given, and update the
// index. 

         bool colormapNeedsUpdate( unsigned long i );
         bool filterNeedsUpdate( unsigned long i );
		
         void set_transform(const osg::Matrix& transform)
            {
               _transform=transform;
            }
         
         osg::Matrix& get_transform()
            {
               return _transform;
            }
         
         const osg::Matrix& get_transform() const
            {
               return _transform;
            }

         osg::Matrix* get_transform_ptr()
            {
               return &_transform;
            }

	protected:
		
         virtual ~MyNodeInfo() {};
		
         unsigned long _colormapUpdateIndex;
         unsigned long _filterUpdateIndex;
		
         HyperBoundingBox _unfilteredBounds;
         osg::ref_ptr<osg::PrimitiveSet> _unfilteredPrimitiveSet;

         osg::Matrix _transform;
      };

} // namespace model

#endif // __MYNODEINFO_H__
