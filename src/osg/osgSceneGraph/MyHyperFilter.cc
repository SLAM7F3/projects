// ========================================================================
// Minor variant of Ross' HyperFilter class
// ========================================================================
// Last updated on 11/27/11; 12/3/11; 12/29/11
// ========================================================================

#include <iostream>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/PrimitiveSet>

#include <model/Metadata.h>
#include <model/HyperExtentsVisitor.h>
#include "general/outputfuncs.h"
#include "osg/osgSceneGraph//MyHyperFilter.h"
#include "osg/osgSceneGraph/MyNodeInfo.h"
#include "osg/osgSceneGraph//UpdateMyHyperFilterCallback.h"

using namespace model;
using namespace osg;

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MyHyperFilter::allocate_member_objects()
{
   _updateCallback = new UpdateMyHyperFilterCallback(this);
}

void MyHyperFilter::initialize_member_objects()
{
//   _currentUpdateIndex=0;
   _currentUpdateIndex=1;
}

MyHyperFilter::MyHyperFilter() :
_hbb( HyperBoundingBox::Constraint )
{
//   cout << "inside MyHyperFilter constructor" << endl;

   allocate_member_objects();
   initialize_member_objects();
}

MyHyperFilter::~MyHyperFilter()
{
}

void MyHyperFilter::setHyperBoundingBox( const HyperBoundingBox& hbb )
{
//   cout << "inside MyHyperFilter::setHyperBoundingBox()" << endl;

   if ( ! hbb.valid() ) return;
	
   _hbb = hbb;
   IncrementUpdateIndex();
}

void MyHyperFilter::IncrementUpdateIndex()
{
//   cout << "inside MyHyperFilter::IncrementUpdateIndex()" << endl;

   // Nodes will update next time they are drawn
   _currentUpdateIndex++;
//   cout << "update index = " << _currentUpdateIndex << endl;
}

osg::NodeCallback* MyHyperFilter::get_UpdateCallback_ptr() 
{ 
//   cout << "inside MyHyperFilter::get_UpdateCallback_ptr()" << endl;
   return _updateCallback.get(); 
}

// ------------------------------------------------------------------------
void MyHyperFilter::update( osg::Geode& geode, osg::NodePath& nodepath )
{
//   cout << "inside MyHyperFilter::update()" << endl;
   
   MyNodeInfo* info = getOrCreateMyInfoForNode( geode );

   if ( info->filterNeedsUpdate( getCurrentUpdateIndex() ) == false ) 
      return;

   osg::Matrixd	localToWorld;

   // Iterate through all drawables to find Geometries.
   Geode::DrawableList::const_iterator	drawableIter = 
      geode.getDrawableList().begin();
   while( drawableIter != geode.getDrawableList().end() ) 
   {
      if ( Geometry* geometry = const_cast<Geometry*>( 
         drawableIter->get()->asGeometry() ) ) 
      {
         if ( geometry->getVertexArray()->getType() == 
         osg::Array::Vec3ArrayType )
         {
            // find a matrix to transform local to world coordinates
            localToWorld = osg::computeLocalToWorld( nodepath );
				
            PrimitiveSet* originalSet = info->getUnfilteredPrimitiveSet();
//            cout << "originalSet = " << originalSet << endl;
            if ( originalSet == NULL ) 
            {
               // This is the first time we have filtered this node,
	       // save the original
               // PrimitiveSet (assume there is only one).
               if ( geometry->getNumPrimitiveSets() == 1 ) 
               {
                  originalSet = geometry->getPrimitiveSet(0);
                  info->setUnfilteredPrimitiveSet(originalSet);
						
                  // Now cache the full hyperbounds of the data
                  HyperExtentsVisitor	hev;
                  geode.accept(hev);
						
                  HyperBoundingBox	worldExtent = hev.getExtent();
                  worldExtent.transform( localToWorld );
						
                  info->setUnfilteredBounds( worldExtent );

 // setup the initial bound so it doesn't get tighter as we filter it

                  geode.setInitialBound( geode.getBound() );
               } 
               else 
               {
                  // Can't filter a set with <> 1 PrimitiveSet
                  drawableIter++;
                  return;
               }
            }
				
// Don't attempt to compare datasets that arn't dimensionally equal

            if ( _hbb.dim() != info->getUnfilteredBounds().dim() ) 
            {
               drawableIter++;
               return;
            }

            PrimitiveSet* primitiveSet = NULL;
				
            if ( contains( info->getUnfilteredBounds() ) ) 
            {

// If all the data is entirely within the hyper bounding box, just
// replace the current set with the original.

               primitiveSet = originalSet;
            } 
            else if ( !intersects( info->getUnfilteredBounds() ) ) 
            {

// If none of the data is within the hyper bounding box, then remove
// the entire primitive set.

               primitiveSet = NULL;
            } 
            else 
            {
               Vec3Array* vertices = 
                  static_cast<osg::Vec3Array*>( geometry->getVertexArray() );
               Metadata* metadata = getMetadataForGeometry( *geometry );
					
// Partial intersection. Filter the original primitive set into a new set.
               
               primitiveSet = filter( 
                  originalSet, vertices, metadata, localToWorld );
            }
				
            Geometry::PrimitiveSetList	primitiveSetList;
            if ( primitiveSet ) primitiveSetList.push_back( primitiveSet );
            geometry->setPrimitiveSetList( primitiveSetList );
				
            geometry->dirtyDisplayList();

         }
      }
      ++drawableIter;
   }
   
}

// ------------------------------------------------------------------------
osg::PrimitiveSet* MyHyperFilter::filter( 
   osg::PrimitiveSet* unfiltered, osg::Vec3Array* vertices, 
   model::Metadata* metadata, const osg::Matrix& localToWorld )
{
//   cout << "inside MyHyperFilter::filter()" << endl;
   
   if ( !unfiltered || !vertices ) return unfiltered;
	
   unsigned int indices, stride;
	
   // how many vertices per primitive?
   indices = unfiltered->getNumIndices();
   stride = indices / unfiltered->getNumPrimitives();
	
   // check for a sane value
   if ( stride < 1 || stride > 20 ) return unfiltered;
		
   // make a new indexing primitive set of the same type as the original
   DrawElementsUShort*	primitiveSet = new DrawElementsUShort( 
      unfiltered->getMode() );
	
   // filter the primitive set
   unsigned int metadata_dims = 0;
   if ( metadata ) metadata_dims = metadata->dims();
	
   bool	keepPrimitive;
   unsigned int	index;
   std::vector<float> point( 3 + metadata_dims );
   osg::Vec3f vertex;
	
   for (unsigned int i = 0; i < indices; i += stride ) 
   {
      keepPrimitive = true;
		
      for (unsigned int i_prim = i; i_prim < i+stride; i_prim++ ) 
      {
         index = unfiltered->index(i_prim);
         vertex = (*vertices)[index] * localToWorld;
			
         // stuff the point vector
         point[0] = vertex[0];
         point[1] = vertex[1];
         point[2] = vertex[2];
			
         for (unsigned int mddi = 0; mddi < metadata_dims; mddi++ )
            point[3+mddi] = metadata->get( index, mddi );
			
         // does this index fall outside the filter bounds?
         if ( !_hbb.contains( point ) )
            keepPrimitive = false;
      }
		
      if ( keepPrimitive ) 
      {
         for( unsigned int i_prim = i; i_prim < i+stride; i_prim++ ) 
         {
            primitiveSet->push_back( unfiltered->index(i_prim) );
         }
      }
   }

   return primitiveSet;
}
