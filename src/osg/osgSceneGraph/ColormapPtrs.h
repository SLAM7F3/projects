// ==========================================================================
// Header file for COLORMAPPTRS class which holds pointers to height
// and probability colormaps which can differ for multiple point
// clouds.  Each point cloud contains its own "colormapptrs" object,
// and pointers to these (multiple) colormapptr objects are stored
// within the scene graph.  Whenever a ColorGeodeVisitor encounters
// the top of a point cloud subtree within the scenegraph, it resets
// the current height and probability colormap pointers.  In this way,
// multiple colormaps with varying thresholds and cyclic fraction
// offsets can be stored within a single scenegraph.
// ==========================================================================
// Last modified on 4/2/07; 4/15/07; 4/22/07; 6/27/07
// ==========================================================================

#ifndef COLORMAPPTRS_H
#define COLORMAPPTRS_H

#include <osg/Referenced>
#include "osg/osgSceneGraph/ColorMap.h"

class ColormapPtrs : public osg::Referenced
{

  public:

   ColormapPtrs();
   ~ColormapPtrs();

// Set & get member functions:

   void set_height_colormap_ptr(ColorMap* CM_ptr);
   ColorMap* get_height_colormap_ptr();
   const ColorMap* get_height_colormap_ptr() const;

   void set_prob_colormap_ptr(ColorMap* CM_ptr);
   ColorMap* get_prob_colormap_ptr();
   const ColorMap* get_prob_colormap_ptr() const;

  private:

   ColorMap* height_colormap_ptr;
   ColorMap* prob_colormap_ptr;

   void initialize_member_objects();
   void allocate_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ColormapPtrs::set_prob_colormap_ptr(ColorMap* CM_ptr)
{
   prob_colormap_ptr=CM_ptr;
}

inline ColorMap* ColormapPtrs::get_prob_colormap_ptr()
{
   return prob_colormap_ptr;
}

inline const ColorMap* ColormapPtrs::get_prob_colormap_ptr() const
{
   return prob_colormap_ptr;
}

#endif // ColormapPtrs.h



