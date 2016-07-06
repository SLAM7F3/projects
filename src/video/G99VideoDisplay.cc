// Note added on 3/4/14: Eliminate get_pixel_coords(), get_uv_coords() 
// and get_pixel_coordinates() in this class in favor of those in
// texture_rectangle_class !!!

// Note: The starting position for unsigned char* data_ptr pixels is
// in the upper left corner corresponding to px=py=0.  So the correct
// counter for *data_ptr is p=py*getWidth()+px and NOT
// p=(getHeight()-1-py)*getWidth()+px.  

// Assumptions:

// Video has Nimages labeled from 0 to Nimages-1

// first_frame_to_display >= 0
// last_frame_to_display <= Nimages-1
// Nframes_to_display=last_frame_to_display-first_frame_to_display+1

// ========================================================================
// G99VideoDisplay provides functionality for displaying G99 movie files.
// ========================================================================
// Last updated on 7/25/10; 12/4/10; 3/6/14; 3/28/14; 4/15/16
// ========================================================================

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osg/Geometry>
#include <osg/TextureRectangle>
#include "math/adv_mathfuncs.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "math/basic_math.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "general/filefuncs.h"
#include "video/G99VideoDisplay.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ostringstream;
using std::pair;
using std::string;
using std::vector;

// ========================================================================
// Initialization, constructor and destructor member functions
// ========================================================================

void G99VideoDisplay::allocate_member_objects()
{
   vertices_refptr = new osg::Vec3Array(4);
   texturecoords_refptr=new osg::Vec3Array(4);
   generate_geometry();
}		       

// ----------------------------------------------------------------
void G99VideoDisplay::initialize_member_objects()
{
//   cout << "inside G99VideoDisplay::initialize_member_objects()" << endl;

   externally_constructed_texture_rectangle_flag=false;
   dynamic_frame_flag=false;
   annotated_pixels_flag=false;
   hide_backface_flag=false;
   RGBA_twoDarray.first=RGBA_twoDarray.second=RGBA_twoDarray.third=
      RGBA_twoDarray.fourth=NULL;
}

// ----------------------------------------------------------------
G99VideoDisplay::G99VideoDisplay(
   string& video_filename,AnimationController* AC_ptr,
   bool hide_backface_flag)
{
//   cout << "inside G99VD constructor #1" << endl;
//   cout << "filename = " << video_filename << endl;
   allocate_member_objects();
   initialize_member_objects();

   this->hide_backface_flag=hide_backface_flag;
   texture_rectangle* tr_ptr=
      generate_texture_rectangle(video_filename,AC_ptr);

   if (tr_ptr->get_VideoType() != texture_rectangle::unknown)
   {
      initialize_geom_vertices(0);
      fill_drawable_geom(0);
   }
}

// ----------------------------------------------------------------
G99VideoDisplay::G99VideoDisplay(texture_rectangle* tr_ptr)
{
//   cout << "inside G99VD constructor #2, texture_rectangle_ptr = " 
//        << tr_ptr << endl;

   allocate_member_objects();
   initialize_member_objects();

   externally_constructed_texture_rectangle_flag=true;
   texture_rectangle_ptrs.push_back(tr_ptr);
   set_default_texture_fracs();

   initialize_geom_vertices(0);
   fill_drawable_geom(0);
   initialize_texture_rectangle(0);
}

// ----------------------------------------------------------------
G99VideoDisplay::~G99VideoDisplay()
{
//   cout << "inside G99VD destructor" << endl;
   if (!externally_constructed_texture_rectangle_flag)
   {
      for (unsigned int t=0; t<get_n_textures(); t++)
      {
         delete texture_rectangle_ptrs[t];
      }
   }

   delete_RGBA_twoDarrays();
}

// ========================================================================
// Texture member functions
// ========================================================================

texture_rectangle* G99VideoDisplay::generate_texture_rectangle(
   string& video_filename,AnimationController* AC_ptr)
{
//   cout << "inside G99VD::generate_texture_rectangle()" << endl;
//   cout << "filename = " << video_filename << endl;

   int t=get_n_textures();

   texture_rectangle* tr_ptr=new texture_rectangle(video_filename,AC_ptr);
   texture_rectangle_ptrs.push_back(tr_ptr);

   if (tr_ptr->get_VideoType() != texture_rectangle::unknown)
   {
      set_default_texture_fracs();
      initialize_texture_rectangle(t);
   }

   return tr_ptr;
}

// ----------------------------------------------------------------
void G99VideoDisplay::set_default_texture_fracs()
{
   lower_left_texture_frac=twovector(0.0 , 0.0);
   lower_right_texture_frac=twovector(1.0 , 0.0);
   upper_right_texture_frac=twovector(1.0 , 1.0);
   upper_left_texture_frac=twovector(0.0 , 1.0);
   reset_texture_coords();
}

// ----------------------------------------------------------------
// Member function reset_texture_coords() uses member twovector
// fractions lower[upper]_left[right]_texture_frac to set the 2D pixel
// values within *texturecords_refptr.

void G99VideoDisplay::reset_texture_coords()
{
//   cout << "inside G99VideoDisplay::reset_texture_coords() #1" << endl;
//   cout << "lower_left_texture_frac = " << lower_left_texture_frac << endl;
//   cout << "upper_left_texture_frac = " << upper_left_texture_frac << endl;
//   cout << "upper_right_texture_frac = " << upper_right_texture_frac << endl;
//   cout << "lower_right_texture_frac = " << lower_right_texture_frac << endl;
//   cout << "getWidth() = " << getWidth()
//        << " getHeight() = " << getHeight() << endl;

   if (!texturecoords_refptr.valid()) return;

// Fill geometry with current texture information:

   vector<twovector> fUV;
   fUV.push_back(lower_left_texture_frac);
   fUV.push_back(upper_left_texture_frac);
   fUV.push_back(upper_right_texture_frac);
   fUV.push_back(lower_right_texture_frac);

   for (int i=0; i<4; i++)
   {
      threevector curr_coords(fUV[i].get(0)*getWidth(),
                              (1-fUV[i].get(1))*getHeight(),1);
      texturecoords_refptr->at(i).set(curr_coords.get(0),curr_coords.get(1),
                                      curr_coords.get(2));

//      cout << "i = " << i
//           << " fUV[i].get(0)*getWidth() = "
//           << fUV[i].get(0)*getWidth() << endl;
//      cout << " (1-fUV[i].get(1))*getHeight() = "
//           << (1-fUV[i].get(1))*getHeight() << endl;
//           << " curr_coords = " << curr_coords << endl;
   } // loop over index i 
}

// ----------------------------------------------------------------
void G99VideoDisplay::set_texture_fracs(
   const twovector& lower_left_frac,
   const twovector& lower_right_frac,
   const twovector& upper_right_frac,
   const twovector& upper_left_frac)
{
//   cout << "inside G99VideoDisplay::set_texture_fracs()" << endl;
   
   lower_left_texture_frac=lower_left_frac;
   lower_right_texture_frac=lower_right_frac;
   upper_right_texture_frac=upper_right_frac;
   upper_left_texture_frac=upper_left_frac;

//   cout << "init lower_left_texture_frac = " << lower_left_texture_frac
//        << endl;
//   cout << "init upper_right_texture_frac = "
//        << upper_right_texture_frac << endl;

   clip_texture_fracs(lower_left_texture_frac);
   clip_texture_fracs(lower_right_texture_frac);
   clip_texture_fracs(upper_left_texture_frac);
   clip_texture_fracs(upper_right_texture_frac);

   reset_texture_coords();
}

// ----------------------------------------------------------------
// Member function initialize_texture_rectangles() takes in the index
// for some entry within STL vector texture_rectangle_ptrs.  It sets
// the TextureAttributeandModes for the StateSet associated with
// geom_refptr and which has texture unit corresponding to the texture
// rectangle's index.  This method also reads and sets the image for
// the texture rectangle.

void G99VideoDisplay::initialize_texture_rectangle(int t)
{
//   cout << "inside G99VD::initialize_texture_rectangle(), t = " << t << endl;
   texture_rectangle* tr_ptr=texture_rectangle_ptrs[t];

   tr_ptr->initialize_general_video();
   osg::StateSet* StateSet_ptr = get_Geometry_ptr(t)->getOrCreateStateSet();

   StateSet_ptr->setTextureAttributeAndModes(
      0, tr_ptr->get_TextureRectangle_ptr(), 
      osg::StateAttribute::ON);

   tr_ptr->set_TextureRectangle_image();
   tr_ptr->read_and_set_image();

   TexMat_refptr = new osg::TexMat;
   StateSet_ptr->setTextureAttributeAndModes(
      0, TexMat_refptr.get(), osg::StateAttribute::ON);

// On 8/20/09, we learned from Ross Anderson that OSG's CullFace class
// can be used to hide the front or back face of a texture by
// attaching it to a StateSet.

   if (hide_backface_flag)
   {
      CullFace_refptr = new osg::CullFace;
      CullFace_refptr->setMode(osg::CullFace::FRONT);
      StateSet_ptr->setAttribute(
         CullFace_refptr.get(), osg::StateAttribute::ON | 
         osg::StateAttribute::OVERRIDE);
      StateSet_ptr->setMode(
         GL_CULL_FACE, osg::StateAttribute::ON | 
         osg::StateAttribute::OVERRIDE);
   }
}

// ----------------------------------------------------------------
// Member function clip_texture_fracs clips the bbox fractions within
// input twovector V to the viable range [0.0 , 1.0].

void G99VideoDisplay::clip_texture_fracs(twovector& V)
{
//   cout << "inside G99VD::clip_texture_fracs()" << endl;
   double fx,fy;
   fx=V.get(0);
   fy=V.get(1);

   fx=basic_math::max(0.0 , fx);
   fx=basic_math::min(1.0 , fx);
   fy=basic_math::max(0.0 , fy);
   fy=basic_math::min(1.0 , fy);

   V=twovector(fx,fy);
}

// ----------------------------------------------------------------
void G99VideoDisplay::rescale_image(double scalefactor)
{
//   cout << "inside G99VD::rescale_image()" << endl;

   int s=get_image_ptr()->s()*scalefactor;
   int t=get_image_ptr()->t()*scalefactor;
   int r=get_image_ptr()->r();
   get_image_ptr()->scaleImage(s,t,r);
}

// ========================================================================
// alpha blending member functions
// ========================================================================

// Pre Feb 28 version of G99VideoDisplay::enable_alpha_blending()

void G99VideoDisplay::enable_alpha_blending(
   double alpha,int ndims,int geom_index)
{
//   cout << "inside G99VD::enable_alpha_blending()" << endl;
   
   if (get_texture_rectangle_ptr(geom_index)==NULL) return;

   if (get_texture_rectangle_ptr(geom_index)->get_VideoType()==
       texture_rectangle::unknown) return;

   osg::StateSet* state = get_Geometry_ptr(geom_index)->getOrCreateStateSet();

// Note added on 2/8/07: We need to set the Rendering Hint in order
// for signpost text to shine through the alpha-blended Copley plaza
// image for main program VIEWPOINTS.  But the following statement
// needs to be commented out in order for rectangles selected on top
// of a conventional 2D photo to appear semi-transparent within our
// VIDEO player...

   if (ndims==3)
   {
      state->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
   }
   
   state->setMode(GL_BLEND,osg::StateAttribute::ON);
   state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

/*
  osg::BlendFunc* fn = new osg::BlendFunc();
//      fn->setFunction(osg::BlendFunc::SRC_COLOR, osg::BlendFunc::ONE);
//      fn->setFunction(osg::BlendFunc::SRC_COLOR, osg::BlendFunc::ZERO);
//      fn->setFunction(osg::BlendFunc::ZERO, osg::BlendFunc::DST_COLOR);
//      fn->setFunction(osg::BlendFunc::ONE, osg::BlendFunc::DST_COLOR);

//      fn->setFunction(osg::BlendFunc::SRC_COLOR, osg::BlendFunc::DST_COLOR);
//      fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::DST_ALPHA);
fn->setFunction(osg::BlendFunc::SRC_ALPHA, 
osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
state->setAttributeAndModes(fn, osg::StateAttribute::ON);
*/

//   cout << "alpha = " << alpha << endl;
   set_alpha(alpha);
}

/*

// This post Feb 28 version of G99VideoDisplay::enable_alpha_blending contains
// Ross' hints for turning off alpha-blending in order to help with
// bluegrass program VIDEOCITIES showing alpha-blended cylinders
// appearing on top of an opaque CH video with an unseen ladar cloud
// underneath...

void G99VideoDisplay::enable_alpha_blending(double alpha)
{
// Note rename state as stateset_ptr...

   osg::StateSet* state = geom_refptr->getOrCreateStateSet();

// Note added on 2/8/07: We need to set the Rendering Hint in order
// for signpost text to shine through the alpha-blended Copley plaza
// image for main program VIEWPOINTS.  But the following statement
// needs to be commented out in order for rectangles selected on top
// of a conventional 2D photo to appear semi-transparent within our
// VIDEO player...

// Note added at 5:20 am on Monday, Mar 3, 2008:

// Ross suggested on 2/28/08 that we should turn off transperency if
// alpha==1.  As of 3/3/08, we restore the transperency lines below.
// We'll return to this issue later...

   if (ndims==3)
   {
      state->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
   }
   
   state->setMode(GL_BLEND,osg::StateAttribute::ON);
   state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);


// Helpful hint from Ross on 2/28/08: setRenderingHint draws texture
// last into depth buffer.  Any object which is transparent should
// exist within the TRANSPARENT BIN.

//   state->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

//   state->setMode(GL_DEPTH_WRITEMASK,osg::StateAttribute::OFF);
//   state->setAttributeAndModes(
//      new osg::Depth(osg::Depth::LESS,0.0, 1.0, false),
//      osg::StateAttribute::ON);

//   osg::BlendFunc* fn = new osg::BlendFunc();


//      fn->setFunction(osg::BlendFunc::SRC_COLOR, osg::BlendFunc::ONE);
//      fn->setFunction(osg::BlendFunc::SRC_COLOR, osg::BlendFunc::ZERO);
//      fn->setFunction(osg::BlendFunc::ZERO, osg::BlendFunc::DST_COLOR);
//      fn->setFunction(osg::BlendFunc::ONE, osg::BlendFunc::DST_COLOR);

//      fn->setFunction(osg::BlendFunc::SRC_COLOR, osg::BlendFunc::DST_COLOR);
//      fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::DST_ALPHA);
  

//   fn->setFunction(osg::BlendFunc::SRC_ALPHA, 
//                   osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
//   state->setAttributeAndModes(fn, osg::StateAttribute::ON);

//   cout << "alpha = " << alpha << endl;
   set_alpha(alpha);
}
*/

// ----------------------------------------------------------------
// Member function set_alpha resets the alpha value within the
// geometry's color array.  Calls to this method can be used to
// interactively blend 2D images with 3D world objects.

void G99VideoDisplay::set_alpha(double alpha,int geom_index)
{
//   cout << "inside G99VideoDisplay::set_alpha()" << endl;

   if (get_texture_rectangle_ptr(geom_index)==NULL) return;

   if (get_texture_rectangle_ptr(geom_index)->get_VideoType()==
       texture_rectangle::unknown) return;

   if (get_Geometry_ptr(geom_index) != NULL)
   {
      osg::Vec4Array* colors=dynamic_cast<osg::Vec4Array*>(
         get_Geometry_ptr(geom_index)->getColorArray()); 
      colors->at(0).set(1,1,1,alpha);
      get_Geometry_ptr(geom_index)->setColorArray(colors);
   }
}

double G99VideoDisplay::get_alpha(int geom_index) 
{
   double alpha=-1;
   if (get_Geometry_ptr(geom_index) != NULL)
   {
      osg::Vec4Array* colors=dynamic_cast<osg::Vec4Array*>(
         get_Geometry_ptr(geom_index)->getColorArray()); 
      alpha=colors->at(0).w();
   }
   return alpha;
}

bool G99VideoDisplay::increase_alpha(double delta_alpha)
{
   bool max_alpha_reached_flag=false;
   double alpha=get_alpha()+delta_alpha;
   if (alpha >= 1.0)
   {
      max_alpha_reached_flag=true;
      alpha=1.0;
   }
   set_alpha(alpha);
//   cout << "alpha = " << alpha << endl;
   return max_alpha_reached_flag;
}

bool G99VideoDisplay::decrease_alpha(double delta_alpha)
{
   bool min_alpha_reached_flag=false;
   double alpha=get_alpha()-delta_alpha;
   if (alpha <= 0.0)
   {
      min_alpha_reached_flag=true;
      alpha=0.0;
   }
   set_alpha(alpha);
//   cout << "alpha = " << alpha << endl;
   return min_alpha_reached_flag;
}

// ========================================================================
// Geometry member functions
// ========================================================================

osg::Geometry* G99VideoDisplay::generate_geometry()
{
   osg::ref_ptr<osg::Geometry> geom_refptr = new osg::Geometry;
   geom_refptrs.push_back(geom_refptr);
   geom_refptr->setVertexArray(vertices_refptr.get());
   geom_refptr->setTexCoordArray(0,texturecoords_refptr.get());
   return geom_refptr.get();
}		       

// ----------------------------------------------------------------
// Member function equate_texture_coords_to_vertices() implements Ross
// Anderson's suggestion made on 12/3/09 to explicitly set
// TexCoordArray using vertices_refptr.  This method is called when a
// photo needs to be warped onto an arbitrary 3D OpenGL quad.

void G99VideoDisplay::equate_texture_coords_to_vertices()
{
   get_Geometry_ptr()->setTexCoordArray(0,vertices_refptr.get());
}		       

// ----------------------------------------------------------------
void G99VideoDisplay::initialize_geom_vertices(int geom_index)
{
//   cout << "inside G99VD::initialize_geom_vertices(), geom_index = " 
//        << geom_index 
//        << endl;
//   cout << "width = " << getWidth(geom_index)
//        << " height = " << getHeight(geom_index) << endl;

   float scale=float(getWidth(geom_index))/float(getHeight(geom_index));
//   cout << "scale = " << scale << endl;
   
   const float TINY=1E-4;
   osg::BoundingBox bb(0.0, 0.0, 0.0, 
                       scale, TINY, 1.0); 
//   osg::BoundingBox bb(0.0, 0.0, 0.0, 
//                       scale, 1.0 , TINY); 
//                       1.0, TINY, 1.0/scale); 

   threevector bottom_left(bb.xMin(),bb.yMax(),bb.zMax());
   threevector top_left(bb.xMin(),bb.yMax(),bb.zMin());
   threevector top_right(bb.xMax(),bb.yMax(),bb.zMin());
   threevector bottom_right(bb.xMax(),bb.yMax(),bb.zMax());

   reset_geom_vertices(top_right,top_left,bottom_left,bottom_right);
   
// Notes: 

// * Recall X, Y and Z axes are oriented horizontally right, inward and
// vertically up for OpenGL images.

// * We could alternatively scale the Z axis:
// bb(0,0,0,1,TINY,1.0/scale)
  
// * In order to prevent images from being vertically flipped, we need
// to set bottom and top to zMax and zMin respectively!

//   cout << "scale = " << scale << endl;
//   cout << "xmin = " << bb.xMin() << " xmax = " << bb.xMax() << endl;
//   cout << "ymin = " << bb.yMin() << " ymax = " << bb.yMax() << endl;
//   cout << "zmin = " << bb.zMin() << " zmax = " << bb.zMax() << endl;
//   cout << "height = " << getHeight()
//        << " width = " << getWidth() << endl;
}

// ----------------------------------------------------------------
void G99VideoDisplay::fill_drawable_geom(int geom_index)
{
//   cout << "inside G99VD::fill_drawable_geom(), geom_index = " << geom_index 
//        << endl;
  
/*
// As of 1/21/08, we're not certain that the following normals info
// needs to be included within this method...

   osg::Vec3Array* normals = new osg::Vec3Array(1); 
   normals->at(0).set(0,-1,0);
   get_Geometry_ptr(geom_index)->setNormalArray(normals); 
   get_Geometry_ptr(geom_index)->setNormalBinding(
      osg::Geometry::BIND_OVERALL); 
*/

/*
// Create a primitive set

    osg::DrawElementsUInt* pPrimitiveSet = 
       new osg::DrawElementsUInt( osg::PrimitiveSet::QUADS, 0 );

    pPrimitiveSet->push_back( 3 );
    pPrimitiveSet->push_back( 2 );
    pPrimitiveSet->push_back( 1 );
    pPrimitiveSet->push_back( 0 );
    get_Geometry_ptr(geom_index)->addPrimitiveSet( pPrimitiveSet );
*/

   get_Geometry_ptr(geom_index)->addPrimitiveSet(
      new osg::DrawArrays(GL_QUADS, 0, 4)); 

   osg::Vec4Array* colors = new osg::Vec4Array(1); 
   get_Geometry_ptr(geom_index)->setColorArray(colors); 
   get_Geometry_ptr(geom_index)->
      setColorBinding(osg::Geometry::BIND_OVERALL); 

  
// Disable display list so our modified tex coordinates show up:

//   get_Geometry_ptr()->setUseDisplayList(false); 
}

// ----------------------------------------------------------------
// Fill geometry with current vertex information:

void G99VideoDisplay::reset_geom_vertices(
   const threevector& top_right,const threevector& top_left,
   const threevector& bottom_left,const threevector& bottom_right)
{
//   cout << "inside G99VideoDisplay::reset_geom_vertices()" << endl;
//   cout << "this = " << this << endl;
//   cout << "top_right = " << top_right << endl;
//   cout << "top_left = " << top_left << endl;
//   cout << "bottom_left = " << bottom_left << endl;
//   cout << "bottom_right = " << bottom_right << endl;

   vertices_refptr->at(0)=osg::Vec3(
      top_left.get(0),top_left.get(1),top_left.get(2));
   vertices_refptr->at(1)=osg::Vec3(
      bottom_left.get(0),bottom_left.get(1),bottom_left.get(2));
   vertices_refptr->at(2)=osg::Vec3(
      bottom_right.get(0),bottom_right.get(1),bottom_right.get(2));
   vertices_refptr->at(3)=osg::Vec3(
      top_right.get(0),top_right.get(1),top_right.get(2));

   dirtyGeomDisplay();
}

// ---------------------------------------------------------------- 
// On 5/14/09, Ross Anderson said that OSG tries to reuse geometries
// that have already been sent to the GPU.  So he told us that we need
// to explicitly call the dirtyDisplayList() method for the geometry
// if its 2D texture coordinates are altered.  If the geometry's 3D
// coordinates have changed, we also need to recompute the bounding
// sphere surrounding the new geometry.  So we need to call
// dirtyBound() for the geometry as well.

void G99VideoDisplay::dirtyGeomDisplay()
{
//   cout << "inside G99VideoDisplay::dirtyGeomDisplay()" << endl;
   
   for (unsigned int t=0; t<get_n_textures(); t++)
   {
      get_Geometry_ptr(t)->dirtyDisplayList();
      get_Geometry_ptr(t)->dirtyBound();
   }
}

// ----------------------------------------------------------------
// Member function compute_corners_COM

threevector G99VideoDisplay::compute_corners_COM()
{
//   cout << "inside G99VideoDisplay::compute_corners_COM()" << endl;

   threevector corners_COM;
   for (int c=0; c<4; c++)
   {
      corners_COM += 0.25*threevector(vertices_refptr->at(c));
   }
   return corners_COM;
}

// ========================================================================
// Video chip member functions
// ========================================================================

// Member function compute_2D_chip takes in 4 corner texture
// fractions.  Their values are clipped to lie within the physical
// range [0 , 1] for both U and V.  The corners' geometry vertices are
// then set based upon the clipped texture fractions.

void G99VideoDisplay::compute_2D_chip(
   const twovector& lower_left_texture_fracs,
   const twovector& lower_right_texture_fracs,
   const twovector& upper_right_texture_fracs,
   const twovector& upper_left_texture_fracs)
{
//   cout << "inside G99VD::compute_2D_chip()" << endl;
//   cout << "lower_left_texture_fracs = " << lower_left_texture_fracs << endl;
//   cout << "upper_right_texture_fracs = " << upper_right_texture_fracs 
//        << endl;

// First clip input texture fractions so that they lie within the
// physical range [0.0 , 1.0] .  Recall that set_texture_fracs() calls
// clip_texture_fracs and then sets member vars
// lower[upper]_left[right]_texture_frac (no s!).  We use those
// internal member vars (with no trailing s!) below:

   set_texture_fracs(
      lower_left_texture_fracs,lower_right_texture_fracs,
      upper_right_texture_fracs,upper_left_texture_fracs);

   threevector bottom_left,top_left,top_right,bottom_right;

   const double TINY=1E-4;
   top_left=threevector(
      lower_left_texture_frac.get(0)*get_maxU(),TINY,
      lower_left_texture_frac.get(1)*get_maxV());
   top_right=threevector(
      lower_right_texture_frac.get(0)*get_maxU(),TINY,
      lower_right_texture_frac.get(1)*get_maxV());
   bottom_right=threevector(
      upper_right_texture_frac.get(0)*get_maxU(),TINY,
      upper_right_texture_frac.get(1)*get_maxV());
   bottom_left=threevector(
      upper_left_texture_frac.get(0)*get_maxU(),TINY,
      upper_left_texture_frac.get(1)*get_maxV());
   
   reset_geom_vertices(top_right,top_left,bottom_left,bottom_right);
}

// ----------------------------------------------------------------
// Member function initialize_3D_chip takes in 4 corner subtexture
// fractions as well as world-space positions for the corners of the
// full video.  This method first clips the subtexture fractions so
// that they lie within the range [0 , 1]. It next computes the
// homography and inverse homography which maps the UV plane into the
// world XY plane.  This method also computes and stores the average
// of the input world corner vertices' Z values.

void G99VideoDisplay::initialize_3D_chip(
   const twovector& bottom_left_texture_fracs,
   const twovector& bottom_right_texture_fracs,
   const twovector& upper_right_texture_fracs,
   const twovector& upper_left_texture_fracs,
   const threevector& video_top_right, 
   const threevector& video_top_left,
   const threevector& video_bottom_left, 
   const threevector& video_bottom_right)
{
//   cout << "inside G99VD::initialize_3D_chip" << endl;
//   cout << "video_bottom_left = " << video_bottom_left << endl;
//   cout << "video_bottom_right = " << video_bottom_right << endl;
//   cout << "video_top_right = " << video_top_right << endl;
//   cout << "video_top_left = " << video_top_left << endl;

//   cout << "bottom_left frac = " << bottom_left_texture_fracs << endl;
//   cout << "bottom_right_fracs = " << bottom_right_texture_fracs << endl;
//   cout << "upper_right_fracs = " << upper_right_texture_fracs << endl;
//   cout << "upper_left_fracs = " << upper_left_texture_fracs << endl;

// First clip input texture fractions so that they lie within the
// physical range [0.0 , 1.0] .  Recall that set_texture_fracs() calls
// clip_texture_fracs and then sets member vars
// bottom[upper]_left[right]_texture_frac (no s!).  We use those
// internal member vars (with no trailing s!) below:

   set_texture_fracs(
      bottom_left_texture_fracs,bottom_right_texture_fracs,
      upper_right_texture_fracs,upper_left_texture_fracs);

   compute_image_to_Z_plane_homographies(
      lower_left_texture_frac,lower_right_texture_frac,
      upper_right_texture_frac,upper_left_texture_frac,
      video_top_right,video_top_left,video_bottom_left,video_bottom_right);

   Zplane_avg=0.25*(video_top_right.get(2)+video_top_left.get(2)+
                    video_bottom_left.get(2)+video_bottom_right.get(2));
//   cout << "Zplane_avg = " << Zplane_avg << endl;
}

// ----------------------------------------------------------------
// Member function compute_image_to_Z_plane_homographies takes in 4
// (U,V) coordinates for the corners of some input video.  It also
// takes in the worldspace threevectors for these same corners which
// we assume all lie within a common Z-plane.  This method computes
// the homography H which maps (X,Y) -> (U,V) as well as its inverse
// Hinv which maps (U,V) -> (X,Y).  

void G99VideoDisplay::compute_image_to_Z_plane_homographies(
   const twovector& bottom_left_texture_fracs,
   const twovector& bottom_right_texture_fracs,
   const twovector& upper_right_texture_fracs,
   const twovector& upper_left_texture_fracs,
   const threevector& video_top_right, 
   const threevector& video_top_left,
   const threevector& video_bottom_left, 
   const threevector& video_bottom_right)
{
//   cout << "inside G99VD::compute_image_to_Z_plane_homographies" << endl;
//   cout << "video_bottom_left = " << video_bottom_left << endl;
//   cout << "video_bottom_right = " << video_bottom_right << endl;
//   cout << "video_top_right = " << video_top_right << endl;
//   cout << "video_top_left = " << video_top_left << endl;

//   cout << "bottom_left frac = " << bottom_left_texture_fracs << endl;
//   cout << "bottom_right_fracs = " << bottom_right_texture_fracs << endl;
//   cout << "upper_right_fracs = " << upper_right_texture_fracs << endl;
//   cout << "upper_left_fracs = " << upper_left_texture_fracs << endl;

// First clip input texture fractions so that they lie within the
// physical range [0.0 , 1.0] .  Recall that set_texture_fracs() calls
// clip_texture_fracs and then sets member vars
// bottom[upper]_left[right]_texture_frac (no s!).  We use those
// internal member vars (with no trailing s!) below:

   set_texture_fracs(
      bottom_left_texture_fracs,bottom_right_texture_fracs,
      upper_right_texture_fracs,upper_left_texture_fracs);

   vector<twovector> XY,UV;
   XY.push_back(twovector(video_bottom_left));
   XY.push_back(twovector(video_bottom_right));
   XY.push_back(twovector(video_top_right));
   XY.push_back(twovector(video_top_left));
   UV.push_back(lower_left_texture_frac);
   UV.push_back(lower_right_texture_frac);
   UV.push_back(upper_right_texture_frac);
   UV.push_back(upper_left_texture_frac);

   H.parse_homography_inputs(XY,UV);
   H.compute_homography_matrix();
//   H.check_homography_matrix(XY,UV);
   H.compute_homography_inverse();

   genmatrix Hinv(3,3);
   Hinv=*(H.get_Hinv_ptr());

//   cout << "Homography H = " << H << endl;
//   cout << "Hinv = " << Hinv << endl;
//   cout << "H*Hinv = " << *(H.get_H_ptr())*Hinv << endl;
}

// ----------------------------------------------------------------
// Member function compute_3D_chip takes in UV image plane coordinates
// for 4 corner vertices.  It first resets the texture fractions.
// This method next computes the XYZ world-space points corresponding
// to the input image plane coordinates.  Finally, the video's
// geometry vertices are reset based upon the XYZ world-space points.

void G99VideoDisplay::compute_3D_chip(
   const twovector& bottom_left_texture_fracs,
   const twovector& bottom_right_texture_fracs,
   const twovector& upper_right_texture_fracs,
   const twovector& upper_left_texture_fracs)
{
//   cout << "inside G99VD::compute_3D_chip#1" << endl;
//   cout << "video_bottom_left = " << video_bottom_left << endl;
//   cout << "video_bottom_right = " << video_bottom_right << endl;
//   cout << "video_top_right = " << video_top_right << endl;
//   cout << "video_top_left = " << video_top_left << endl;

//   cout << "bottom_left frac = " << bottom_left_texture_fracs << endl;
//   cout << "bottom_right_fracs = " << bottom_right_texture_fracs << endl;
//   cout << "upper_right_fracs = " << upper_right_texture_fracs << endl;
//   cout << "upper_left_fracs = " << upper_left_texture_fracs << endl;

// First clip input texture fractions so that they lie within the
// physical range [0.0 , 1.0] .  Recall that set_texture_fracs() calls
// clip_texture_fracs and then sets member vars
// bottom[upper]_left[right]_texture_frac (no s!).  We use those
// internal member vars (with no trailing s!) below:

   set_texture_fracs(
      bottom_left_texture_fracs,bottom_right_texture_fracs,
      upper_right_texture_fracs,upper_left_texture_fracs);

   twovector BL=H.project_image_plane_to_world_plane(
      lower_left_texture_frac);
   twovector BR=H.project_image_plane_to_world_plane(
      lower_right_texture_frac);
   twovector TR=H.project_image_plane_to_world_plane(
      upper_right_texture_frac);
   twovector TL=H.project_image_plane_to_world_plane(
      upper_left_texture_frac);

   threevector bottom_left(BL,Zplane_avg);
   threevector bottom_right(BR,Zplane_avg);
   threevector top_right(TR,Zplane_avg);
   threevector top_left(TL,Zplane_avg);

// On 12/9/07, we empirically determined that the movie needs to be
// vertically flipped.  So we pass the bottom_right, bottom_left,
// top_left and top_right video corner vertices as arguments to
// reset_geom_vertices():
   
   reset_geom_vertices(bottom_right,bottom_left,top_left,top_right);
}

// ----------------------------------------------------------------
// This overloaded version of compute_3D_chip takes in world-space
// coordinates for the video chip's 4 corners.  It computes the
// corresponding UV image plane coordinates and then calls the
// preceding version of compute_3D_chip().

void G99VideoDisplay::compute_3D_chip(
   const threevector& bottom_left,const threevector& bottom_right,
   const threevector& top_right,const threevector& top_left)
{
//   cout << "inside G99VD::compute_3D_chip#2" << endl;
//   cout << "video_bottom_left = " << video_bottom_left << endl;
//   cout << "video_bottom_right = " << video_bottom_right << endl;
//   cout << "video_top_right = " << video_top_right << endl;
//   cout << "video_top_left = " << video_top_left << endl;

//   cout << "bottom_left frac = " << bottom_left_texture_fracs << endl;
//   cout << "bottom_right_fracs = " << bottom_right_texture_fracs << endl;
//   cout << "upper_right_fracs = " << upper_right_texture_fracs << endl;
//   cout << "upper_left_fracs = " << upper_left_texture_fracs << endl;

//   cout << "H = " << H << endl;
   return;

   twovector BL=twovector(H.project_world_plane_to_image_plane(bottom_left));
   twovector BR=twovector(H.project_world_plane_to_image_plane(bottom_right));
   twovector TR=twovector(H.project_world_plane_to_image_plane(top_right));
   twovector TL=twovector(H.project_world_plane_to_image_plane(top_left));
   
   compute_3D_chip(BL,BR,TR,TL);
}

/*

// ========================================================================
// Subtexture member functions
// ========================================================================

double G99VideoDisplay::f_frac(double U) const
{
   return (U-get_minU())/(get_maxU()-get_minU());
}

double G99VideoDisplay::g_frac(double U) const
{
   return (get_maxU()-U)/(get_maxU()-get_minU());
}

double G99VideoDisplay::s_frac(double V) const
{
   return (V-get_minV())/(get_maxV()-get_minV());
}

double G99VideoDisplay::t_frac(double V) const
{
   return (get_maxV()-V)/(get_maxV()-get_minV());
}

threevector G99VideoDisplay::interpolated_vertex_posn(
   const twovector& texture_fracs,
   const threevector& video_bottom_left,
   const threevector& video_bottom_right,
   const threevector& video_top_left,
   const threevector& video_top_right) const
{
   double U=texture_fracs.get(0);
   double V=texture_fracs.get(1);
   threevector P
      =t_frac(V)*g_frac(U)*video_bottom_left
      +t_frac(V)*f_frac(U)*video_bottom_right
      +s_frac(V)*g_frac(U)*video_top_left
      +s_frac(V)*f_frac(U)*video_top_right;
   return P;
}
*/

// ========================================================================
// Image numbering member functions
// ========================================================================

unsigned int G99VideoDisplay::get_n_textures() const
{
   return texture_rectangle_ptrs.size();
}

int G99VideoDisplay::get_Nimages() const
{
   return get_texture_rectangle_ptr()->get_Nimages();
}

int G99VideoDisplay::get_first_frame_to_display() const
{
   return get_texture_rectangle_ptr()->get_first_frame_to_display();
}

int G99VideoDisplay::get_last_frame_to_display() const
{
   return get_texture_rectangle_ptr()->get_last_frame_to_display();
}

int G99VideoDisplay::get_imagenumber() const
{ 
   return get_texture_rectangle_ptr()->get_imagenumber();
}

// ========================================================================
// Frame display member functions
// ========================================================================

void G99VideoDisplay::display_current_frame()
{
//   cout << "inside G99VD::display_curr_frame()" << endl;

   for (unsigned int t=0; t<get_n_textures(); t++)
   {
//      cout << "t = " << t 
//           << " texture_rect_ptr = " << get_texture_rectangle_ptr(t) << endl;
      get_texture_rectangle_ptr(t)->display_current_frame();
   }
}

// ----------------------------------------------------------------
void G99VideoDisplay::displayFrame(int p_framenum)
{
//   cout << "inside G99VD::displayFrame(), frame = " << p_framenum << endl;
   get_texture_rectangle_ptr()->displayFrame(p_framenum);
}

// ----------------------------------------------------------------
void G99VideoDisplay::draw_annotation()
{
   twoDarray* RtwoDarray_ptr=get_RGBA_twoDarray(0);
   twoDarray* GtwoDarray_ptr=get_RGBA_twoDarray(1);
   twoDarray* BtwoDarray_ptr=get_RGBA_twoDarray(2);

   if (RtwoDarray_ptr != NULL)
   {
      int mdim=RtwoDarray_ptr->get_mdim();
      int ndim=RtwoDarray_ptr->get_ndim();
      
      for (int px=0; px<mdim; px++)
      {
         for (int py=0; py<ndim; py++)
         {
            double curr_r=RtwoDarray_ptr->get(px,py);
            if (curr_r >= 0)
            {
               double curr_g=GtwoDarray_ptr->get(px,py);
               double curr_b=BtwoDarray_ptr->get(px,py);
               set_pixel_RGB_values(px,py,curr_r,curr_g,curr_b);
            }
         } // loop over py index
      } // loop over px index
   } // RtwoDarray_ptr != NULL conditional
}

// ----------------------------------------------------------------
void G99VideoDisplay::set_image()
{
//   cout << "inside G99VD::set_image()" << endl;
   get_texture_rectangle_ptr()->set_image();
}

// ========================================================================
// Colormap member functions
// ========================================================================

// Member function change_color_map

void G99VideoDisplay::change_color_map(int map_number)
{
   get_texture_rectangle_ptr()->change_color_map(map_number);
}

// ========================================================================
// RGBA member functions
// ========================================================================

// Member function convert_to_RGB renormalizes the naive red and blue
// values to compensate for the red and blue filters on the Group 99
// video camera attenuating more light than the green.  

// We learned from Mike Braun on 5/25/05 that he manually altered the
// dynamic ranges on the red and blue channels until he obtained
// reasonably colored imagery.  Mike said that he simply clips all
// renormalized RGB values so that they lie within the range [0,255]:
// But we observed in HAFB video data that Mike's truncation leads to
// bad saturation artifacts in the colored video which are not present
// in the progenitor greyscale video.  

// So this member function first multiplies the red and blue values by
// an error function which ranges from Mike's constant red and blue
// amplification factors down to unity as the naive red and blue
// values go from 0 to 255.  It next converts from RGB to HSV color
// space.  The intensity (V) values are then remapped onto a fixed
// gaussian distribution ("histogram specification") which eliminates
// global, discontinuous illumination jumps from the video and
// enhances contrast.  

// This method should only be called by main program
// mains/video/DEMOSAIC at the time that raw greyscale video is
// converted to RGB video.  The renormalized RGB values should then be
// saved and never need to be recalculated again...

void G99VideoDisplay::convert_to_RGB(
   unsigned char* data_ptr,double intensity_threshold,
   const prob_distribution& p_gaussian,vector<double>& h,
   vector<double>& s,vector<double>& v,
   vector<double>& non_negligible_intensities)
{

// Note: On 10/6/05, we found that the following red and blue
// constants yield reasonable looking color imagery for our favorite
// HAFB video pass.  But these values definitely will need to be
// altered for Lowell and other data sets!

   const double red_lo=0.95*255.0/140.0;
   const double red_hi=1.0;
   const double mu_red=190;
   const double sigma_red=20;

   const double blue_lo=0.95*255.0/174.0;
   const double blue_hi=1.0;
   const double mu_blue=190;
   const double sigma_blue=20;

   const int n_pixels=getHeight()*getWidth();
   double curr_r,curr_g,curr_b,curr_h,curr_s,curr_v;

   for (int n=0; n<n_pixels; n++)
   {
      curr_r=stringfunc::unsigned_char_to_ascii_integer(data_ptr[3*n+0]);
      curr_g=stringfunc::unsigned_char_to_ascii_integer(data_ptr[3*n+1]);
      curr_b=stringfunc::unsigned_char_to_ascii_integer(data_ptr[3*n+2]);

      double arg_red=(curr_r-mu_red)/(SQRT_TWO*sigma_red);
      double red_fudge_factor=red_lo+(red_hi-red_lo)*0.5*(
         1+mathfunc::errorfunc::fast_error_function(arg_red));
      curr_r *= red_fudge_factor;

      double arg_blue=(curr_b-mu_blue)/(SQRT_TWO*sigma_blue);
      double blue_fudge_factor=blue_lo+(blue_hi-blue_lo)*0.5*(
         1+mathfunc::errorfunc::fast_error_function(arg_blue));
      curr_b *= blue_fudge_factor;

      colorfunc::RGB_to_hs(curr_r,curr_g,curr_b,curr_h,curr_s);
      curr_v=basic_math::max(curr_r,curr_g,curr_b)/255.0;
      h.push_back(curr_h);
      s.push_back(curr_s);
      v.push_back(curr_v);
      if (255.0*curr_v > intensity_threshold) 
         non_negligible_intensities.push_back(curr_v);
   } // loop over index n labeling pixels

   const int nbins=75;
   prob_distribution prob_v(non_negligible_intensities,nbins);

   for (int n=0; n<n_pixels; n++)
   {
      double new_v=v[n];
      if (255.0*v[n] > intensity_threshold)
      {
         double x=v[n];
         int b=prob_v.get_bin_number(x);
         double pcum=prob_v.get_pcum(b);
//            new_v=255*pcum;	// Histogram equalization
         double y=p_gaussian.find_x_corresponding_to_pcum(pcum);
         new_v=y;
      }

// Try to enhance hue richness by increasing saturation values.
// Recall s=1 corresponds to zero white admixture, while s=0
// corresponds to pure white:

      double new_s=basic_math::min(1.0,s[n]*1.15);
      
      colorfunc::hsv_to_RGB(h[n],new_s,new_v,curr_r,curr_g,curr_b);
      int R=basic_math::min(255,basic_math::round(255*curr_r));
      int G=basic_math::min(255,basic_math::round(255*curr_g));
      int B=basic_math::min(255,basic_math::round(255*curr_b));
      data_ptr[3*n+0]=stringfunc::ascii_integer_to_unsigned_char(R);
      data_ptr[3*n+1]=stringfunc::ascii_integer_to_unsigned_char(G);
      data_ptr[3*n+2]=stringfunc::ascii_integer_to_unsigned_char(B);
   } // loop over index n labeling pixels
} 

/*
// Member function convert_to_RGB should only be called within main
// program mains/video/DEMOSAIC at the time that raw greyscale video
// is converted into an RGB video.  The renormalized RGB values should
// then be saved and never need to be recalculated again...

void G99VideoDisplay::convert_to_RGB(unsigned char* data_ptr)
{
   if (getNumChannels()==3)
   {

// We learned from Mike Braun on 5/25/05 that he manually altered the
// dynamic ranges on the red and blue channels until he obtained
// reasonably colored imagery.  Braun said that the red and blue
// filters attenuate much more light than does the green.  So it is
// necessary to amplify their values.  Mike also said that he simply
// clips all renormalized RGB values so that they lie within the range
// [0,255]:

      const double red_fudge_factor=255.0/140.0;
      const double green_fudge_factor=1.0;
      const double blue_fudge_factor=255.0/174.0;

      for (int i=0; i<get_texture_rectangle_ptr()->get_image_size_in_bytes(); 
      i += 3)
      {
         int R=basic_math::min(255,basic_math::round(
            red_fudge_factor*
            stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+0])));
         int G=basic_math::min(255,basic_math::round(
            green_fudge_factor*
            stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1])));
         int B=basic_math::min(255,basic_math::round(
            blue_fudge_factor*
            stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+2])));
      
         data_ptr[i+0]=stringfunc::ascii_integer_to_unsigned_char(R);
         data_ptr[i+1]=stringfunc::ascii_integer_to_unsigned_char(G);
         data_ptr[i+2]=stringfunc::ascii_integer_to_unsigned_char(B);
      }
   } // NumChannels conditional
}
*/

// ------------------------------------------------------------------------
// Member function generate_RGBA_twoDarrays dynamically allocates and
// initializes 3 twoDarrays to hold RGB data read from Group 99 RGB
// video files.  The 3 arrays are elements of member Triple
// RGBA_twoDarray.

void G99VideoDisplay::generate_RGBA_twoDarrays()
{
   if (RGBA_twoDarray.first==NULL || RGBA_twoDarray.second==NULL ||
       RGBA_twoDarray.third==NULL || RGBA_twoDarray.fourth==NULL)
   {
      twoDarray* RtwoDarray_ptr=new twoDarray(getWidth(),getHeight());

      RtwoDarray_ptr->set_xlo(0);
      RtwoDarray_ptr->set_xhi(getWidth());
      RtwoDarray_ptr->set_ylo(0);
      RtwoDarray_ptr->set_yhi(getHeight());
      RtwoDarray_ptr->set_deltax(1);
      RtwoDarray_ptr->set_deltay(1);
      twoDarray* GtwoDarray_ptr=new twoDarray(RtwoDarray_ptr);
      twoDarray* BtwoDarray_ptr=new twoDarray(RtwoDarray_ptr);
      twoDarray* AtwoDarray_ptr=new twoDarray(RtwoDarray_ptr);

//   cout << "*RtwoDarray_ptr = " << *RtwoDarray_ptr << endl;
//   cout << "*GtwoDarray_ptr = " << *GtwoDarray_ptr << endl;
//   cout << "*BtwoDarray_ptr = " << *BtwoDarray_ptr << endl;
      RGBA_twoDarray=RGBA_array(
         RtwoDarray_ptr,GtwoDarray_ptr,BtwoDarray_ptr,AtwoDarray_ptr);
   } // 1st, 2nd or 3rd members of RGBA_twoDarray==NULL conditional
}

// ------------------------------------------------------------------------
void G99VideoDisplay::delete_RGBA_twoDarrays()
{
   delete RGBA_twoDarray.first;
   delete RGBA_twoDarray.second;
   delete RGBA_twoDarray.third;
   delete RGBA_twoDarray.fourth;
   RGBA_twoDarray.first=RGBA_twoDarray.second=RGBA_twoDarray.third=
      RGBA_twoDarray.fourth=NULL;
}

// ------------------------------------------------------------------------
// Member function convert_charstar_array_to_RGBA_twoDarrays fills the
// first, second and third members of the RGBA_twoDarray triple with
// RGB values extracted from input unsigned char array *data_ptr.

void G99VideoDisplay::convert_charstar_array_to_RGBA_twoDarrays(
   unsigned char* data_ptr)
{
   cout << "inside G99VD::convert_charstar_array_to_RGBA_twoDarrays()"
        << endl;

   int i=0;
   for (unsigned int py=0; py<RGBA_twoDarray.first->get_ndim(); py++)
   {
      for (unsigned int px=0; px < RGBA_twoDarray.first->get_mdim(); px++)
      {
         int R,G,B,A;
         R=G=B=A=0;
         if (getNchannels()==1)
         {
            R=G=B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i]);
            A=255;
         }
         else if (getNchannels()==3)
         {
            R=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+0]);
            G=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1]);
            B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+2]);
            A=255;
         }
         else if (getNchannels()==4)
         {
            R=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+0]);
            G=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1]);
            B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+2]);
            A=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+3]);
         }
         RGBA_twoDarray.first->put(px,py,R);
         RGBA_twoDarray.second->put(px,py,G);
         RGBA_twoDarray.third->put(px,py,B);
         RGBA_twoDarray.fourth->put(px,py,A);
//         cout << "px = " << px << " py = " << py
//              << " R = " << RGBA_twoDarray.first->get(px,py)
//              << " G = " << RGBA_twoDarray.second->get(px,py)
//              << " B = " << RGBA_twoDarray.third->get(px,py) << endl;
         i += getNchannels();
      } // loop over px
   } // loop over py
}

// ---------------------------------------------------------------------
// Member function convert_RGBAarrays_to_charstar_array takes in
// integers mdim and ndim which can be less than the pixel widths and
// heights of the twoDarrays stored within member Triple
// RGBA_twoDarray.  This method instantiates an unsigned char* array
// and fills it with just those RGB values lying within 0 <= px < mdim
// and 0 <= py < ndim.  This method can be used to crop unsigned char*
// arrays so that their sizes are perfect powers of two.  This is
// important for rapid video display purposes.

unsigned char* G99VideoDisplay::convert_RGBAarrays_to_charstar_array()
{
   return convert_RGBAarrays_to_charstar_array(getWidth(),getHeight());
}

unsigned char* G99VideoDisplay::convert_RGBAarrays_to_charstar_array(
   int mdim,int ndim)
{
   const int NBYTES_PER_PIXEL=3;
//   const int NBYTES_PER_PIXEL=4;
   unsigned char* data=new unsigned char[NBYTES_PER_PIXEL*mdim*ndim];   
   convert_RGBAarrays_to_charstar_array(
      NBYTES_PER_PIXEL,mdim,ndim,data);
   return data;
}

void G99VideoDisplay::convert_RGBAarrays_to_charstar_array(
   int NBYTES_PER_PIXEL,int mdim,int ndim,unsigned char* data)
{
   colorfunc::RGBA curr_RGBA;
   for (int py=0; py<ndim; py++)
//   for (int py=ndim-1; py >= 0; py--)
   {
      for (int px=0; px<mdim; px++)
      {
         curr_RGBA.first=RGBA_twoDarray.first->get(px,py);
         curr_RGBA.second=RGBA_twoDarray.second->get(px,py);
         curr_RGBA.third=RGBA_twoDarray.third->get(px,py);

         if (NBYTES_PER_PIXEL==4)
         {
            curr_RGBA.fourth=RGBA_twoDarray.fourth->get(px,py);
         }
         
         colorfunc::RGBA_bytes curr_RGBA_bytes=
            colorfunc::RGBA_to_bytes(curr_RGBA,false);

         int p=py*mdim+px;
         int i=NBYTES_PER_PIXEL*p;
         data[i+0]=curr_RGBA_bytes.first;
         data[i+1]=curr_RGBA_bytes.second;
         data[i+2]=curr_RGBA_bytes.third;
         if (NBYTES_PER_PIXEL==4)
         {
            data[i+3]=curr_RGBA_bytes.fourth;
         }
         
      } // loop over px index
   } // loop over py index
}

// ========================================================================
// Pixel value manipulation member functions
// ========================================================================

// Important note: In the 2 methods below, pv = 0 corresponds to the
// TOP (and not BOTTOM) of the image! 

pair<unsigned int,unsigned int> G99VideoDisplay::get_pixel_coords(
   double u,double v)
{
   unsigned int pu,pv;
   get_pixel_coords(u,v,pu,pv);
   return pair<int,int>(pu,pv);
}

pair<double,double> G99VideoDisplay::get_uv_coords(int pu,int pv)
{
   double u,v;
   get_uv_coords(pu,pv,u,v);
   return pair<double,double>(u,v);
}

// ----------------------------------------------------------------
// Member function get_pixel_RBB_values coords takes in continuous
// image coordinates (u,v) for some pixel.  It returns the pixel's RGB
// values in the range 0 to 255:

void G99VideoDisplay::get_pixel_RGB_values(
   unsigned int pu,unsigned int pv,int& R,int& G,int& B)
{
//   cout << "inside G99VD::get_pixel_RGB_values(pu,pv,R,G,B)" << endl;
//   cout << "pu = " << pu << " pv = " << pv << endl;
   R=G=B=-1; // missing data values
   if (pu >= 0 && pu < getWidth() && pv >= 0 && pv < getHeight())
   {
      get_pixel_RGB_values(pu,pv,get_m_image_ptr(),R,G,B);
   }
}

Triple<int,int,int> G99VideoDisplay::get_RGB_values(double u,double v)
{
   int R,G,B;
   R=G=B=0;
   get_RGB_values(u,v,R,G,B);
   return Triple<int,int,int>(R,G,B);
}

// ----------------------------------------------------------------
// Member function set_pixel_RBB_values alters the RGB values for the
// pixel labeled by input coordinates pu and pv.

void G99VideoDisplay::set_pixel_RGB_values(
   unsigned int pu,unsigned int pv,int R,int G,int B)
{
//   cout << "inside G99VD::set_pixel_RGB_values() " << endl;
   
   if (pu >= 0 && pu < getWidth() && pv >= 0 && pv < getHeight())
   {
      set_pixel_RGB_values(pu,pv,get_m_image_ptr(),R,G,B);
//      int rnew,gnew,bnew;
//      get_pixel_RGB_values(pu,pv,rnew,gnew,bnew);
//      cout << "rnew = " << rnew << " gnew = " << gnew << " bnew = " << bnew
 //          << endl;
   }
}

// ----------------------------------------------------------------
void G99VideoDisplay::set_RGB_values(double u,double v,int R,int G,int B)
{
   unsigned int pu,pv;
   get_pixel_coords(u,v,pu,pv);
   set_pixel_RGB_values(pu,pv,R,G,B);
}

// Member function check_pixel_bounds makes sure input byte index p
// lies between 0 and width*height. 

// ----------------------------------------------------------------
void G99VideoDisplay::check_pixel_bounds(unsigned int& p)
{
   if (p <0 || p >= getWidth()*getHeight())
   {
      cout << "Error in G99VideoDisplay::check_pixel_bounds()!" << endl;
      cout << "p = " << p << " is >= getWidth()*getHeight() = "
           << getWidth()*getHeight() << endl;
      p=getWidth()*getHeight()-1;
   }
}

// ------------------------------------------------------------------------
// Member function set_pixel_RGB_values takes in integer pixel
// coordinates (px,py) and integer RGB values.  It then resets the
// appropriate bytes within the input unsigned char* array.

void G99VideoDisplay::set_pixel_RGB_values(
   int px,int py,unsigned char* data_ptr,int R,int G,int B)
{
//   cout << "inside G99VideoDisplay::set_pixel_RGB_values(), R = " << R
//        << " G = " << G << " B = " << B << endl;

// See note at top of this file:

   int p=py*getWidth()+px;

//   check_pixel_bounds(p);
   
   int i=getNchannels()*p;
   if (getNchannels() == 1)
   {
      data_ptr[i+0]=stringfunc::ascii_integer_to_unsigned_char(R);
   }
   else if (getNchannels() == 3 || getNchannels() == 4)
   {
      data_ptr[i+0]=stringfunc::ascii_integer_to_unsigned_char(R);
      data_ptr[i+1]=stringfunc::ascii_integer_to_unsigned_char(G);
      data_ptr[i+2]=stringfunc::ascii_integer_to_unsigned_char(B);
   }
   else
   {
      cout << "Error in G99VideoDisplay::set_pixel_RGB_values()" << endl;
      cout << "getNchannels() = " << getNchannels() << endl;
      exit(-1);
   }
}

/*
void G99VideoDisplay::set_pixel_RGBA_values(
   int px,int py,unsigned char* data_ptr,int R,int G,int B,int A)
{
//   cout << "inside G99VideoDisplay::set_pixel_RGB_values(), R = " << R
//        << " G = " << G << " B = " << B << endl;

// See note at top of this file:

   int p=py*getWidth()+px;

//   check_pixel_bounds(p);
   
   int i=getNchannels()*p;
   data_ptr[i+0]=stringfunc::ascii_integer_to_unsigned_char(R);
   data_ptr[i+1]=stringfunc::ascii_integer_to_unsigned_char(G);
   data_ptr[i+2]=stringfunc::ascii_integer_to_unsigned_char(B);
   data_ptr[i+3]=stringfunc::ascii_integer_to_unsigned_char(A);
}
*/

// ------------------------------------------------------------------------
// Member function get_pixel_RGB_values takes in integer pixel coordinates
// (px,py).  It returns this pixel's RGB values.

void G99VideoDisplay::get_pixel_RGB_values(
   int px,int py,unsigned char* data_ptr,int& R,int& G,int& B)
{
//   cout << "inside G99VD::get_pixel_RGB_values(), px = " << px << " py = "
//        << py << endl;

// See note at top of this file:

   unsigned int p=py*getWidth()+px;
   check_pixel_bounds(p);
   
   int i=getNchannels()*p;
   if (getNchannels()==1)
   {
      R=G=B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i]);
   }
   else if (getNchannels()==3)
   {
      R=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+0]);
      G=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1]);
      B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+2]);
   }

//   cout << "R = " << R << " G = " << G << " B = " << B << endl;
}

void G99VideoDisplay::get_pixel_RGBA_values(
   int px,int py,unsigned char* data_ptr,int& R,int& G,int& B,int& A)
{

// See note at top of this file:

   unsigned int p=py*getWidth()+px;

//   check_pixel_bounds(p);
   
   int i=getNchannels()*p;
   if (getNchannels()==1)
   {
      R=G=B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i]);
      A=255;
   }
   else if (getNchannels()==4)
   {
      R=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+0]);
      G=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1]);
      B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+2]);
      A=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+3]);
   }
}

// ------------------------------------------------------------------------
Triple<int,int,int> G99VideoDisplay::get_pixel_RGB_values(
   int px,int py,unsigned char* data_ptr)
{
   int R,G,B;
   R=G=B=0;
   get_pixel_RGB_values(px,py,data_ptr,R,G,B);
   return Triple<int,int,int>(R,G,B);
}

Quadruple<int,int,int,int> G99VideoDisplay::get_pixel_RGBA_values(
   int px,int py,unsigned char* data_ptr)
{
   int R,G,B,A;
   R=G=B=A=0;
   get_pixel_RGBA_values(px,py,data_ptr,R,G,B,A);
   return Quadruple<int,int,int,int>(R,G,B,A);
}

// ========================================================================
// Intensity regularization functions:
// ========================================================================

// Member function compute_median_greyscale_image_intensity loops over
// every image within the current video which we assume is greyscale.
// For each image, it computes an probability distribution for all
// intensities greater than the input threshold.  It then calculates
// the median and quartile width for each of these intensity
// distributions.  This member function returns the median over all
// the images of their median and quartile width intensities.

pair<double,double> G99VideoDisplay::compute_median_greyscale_image_intensity(
   double intensity_threshold)
{
   outputfunc::write_banner("Computing median greyscale image intensities:");

   vector<double> intensities;
   intensities.reserve(getWidth()*getHeight());
   median_intensities.reserve(get_Nimages());
   quartile_widths.reserve(get_Nimages());

   for (int n=get_first_imagenumber(); n <= get_last_imagenumber(); n++)
   {
      intensities.clear();
      get_VidFile_ptr()->read_image(n, get_m_image_ptr() );

// Load intensities greater than input threshold from current image
// into STL vector:

      for (unsigned int i=0; i<getWidth()*getHeight(); i++)
      {
         int grey_value=stringfunc::unsigned_char_to_ascii_integer(
            get_m_image_ptr()[i]);
         if (grey_value > intensity_threshold)
         {
            intensities.push_back(grey_value);
         }
      }
      prob_distribution prob(intensities,50);

      median_intensities.push_back(prob.median());
      quartile_widths.push_back(prob.quartile_width());

      cout << "imagenumber = " << n 
           << " median intensity = " << median_intensities.back()
           << " quartile width = " << quartile_widths.back() << endl;
   } // loop over index n labeling image number

   prob_distribution prob_median_intensities(median_intensities,30);
   prob_distribution prob_quartile_widths(quartile_widths,30);

   double median_intensity=prob_median_intensities.median();
   double quartile_width=prob_quartile_widths.median();
   cout << "Median intensity for all images = " << median_intensity << endl;
   cout << " Quartile width for all images = " << quartile_width << endl;
   return pair<double,double>(median_intensity,quartile_width);
}

// ----------------------------------------------------------------
// Member function regularize_greyscale_image_intensities loops over
// every image within the current video which we assume is greyscale.
// For each image, it 

void G99VideoDisplay::regularize_greyscale_image_intensities(
   string input_greyscale_video_filename,double intensity_threshold,
   const std::pair<double,double>& p)
{
   outputfunc::write_banner("Regularizing greyscale image intensities:");

// Initialize output .vid file to hold renormalized grey-scale images:

   VidFile vid_out;
   unsigned int dot_posn=input_greyscale_video_filename.rfind(".vid");
   string output_filename=input_greyscale_video_filename.substr(0,dot_posn)
      +"_renorm.vid";
   vid_out.New_8U(
      output_filename.c_str(),getWidth(),getHeight(),get_Nimages(),1);

   unsigned char* renormalized_image=new unsigned char[
      get_texture_rectangle_ptr()->get_image_size_in_bytes()];

   for (int n=get_first_imagenumber(); n<=get_last_imagenumber(); n++)
   {
      cout << n << " " << flush;
      get_VidFile_ptr()->read_image(n, get_m_image_ptr() );
      double alpha=p.second/quartile_widths[n];
      double beta=p.first-alpha*median_intensities[n];

      for (unsigned int i=0; i<getHeight()*getWidth(); i++)
      {
         int old_grey_value=
            stringfunc::unsigned_char_to_ascii_integer(get_m_image_ptr()[i]);
         int new_grey_value=old_grey_value;
         if (old_grey_value > intensity_threshold)
         {
            new_grey_value=basic_math::round(alpha*old_grey_value+beta);
            new_grey_value=basic_math::min(255,new_grey_value);
            new_grey_value=basic_math::max(0,new_grey_value);
         }
         renormalized_image[i]=
            stringfunc::ascii_integer_to_unsigned_char(new_grey_value);
      } // loop over index i labeling unsigned char byte
      vid_out.WriteFrame(renormalized_image, getWidth());
   } // loop over index n labeling image numbers
   cout << endl;

   delete [] renormalized_image;
}

// ----------------------------------------------------------------
// Member function equalize_greyscale_intensity_histograms implements
// "intensity histogram specification" for each image in the G99 video
// sequence.  It eliminates discrete, global illumination changes.
// The renormalization performed by this method also increases
// intensity contrast.  We found on 9/21/05 that pure "histogram
// equalization" which yields a flat intensity histogram acts as a
// high pass filter which too strongly amplifies noise and leads to
// saturated-looking imagery.  So this method sets the final intensity
// distribution to a gaussian centered about a mean 50% intensity
// level.  

// On 9/21/05, we saw significantly better performance for KLT
// tracking based upon images whose intensity histograms had all been
// set equal to a common gaussian than for images whose 1st and 2nd
// moments were simply equated.  

void G99VideoDisplay::equalize_greyscale_intensity_histograms(
   string input_greyscale_video_filename,double intensity_threshold)
{
   vector<double> intensity_thresholds;
   for (int i=get_first_imagenumber(); i<=get_last_imagenumber(); i++)
   {
      intensity_thresholds.push_back(intensity_threshold);
   }
   equalize_greyscale_intensity_histograms(input_greyscale_video_filename,
                                           intensity_thresholds);
}

void G99VideoDisplay::equalize_greyscale_intensity_histograms(
   string input_greyscale_video_filename,vector<double> intensity_thresholds)
{
   outputfunc::write_banner("Equalizing greyscale intensity histograms:");

// Initialize output .vid file to hold renormalized grey-scale images:

   VidFile vid_out;
   unsigned int dot_posn=input_greyscale_video_filename.rfind(".vid");
   string output_filename=input_greyscale_video_filename.substr(0,dot_posn)
      +"_renorm.vid";
   vid_out.New_8U(
      output_filename.c_str(),getWidth(),getHeight(),get_Nimages(),1);

// Generate final, desired gaussian intensity distribution:

   const int nbins=200;
   double mu=0.5;
   double sigma=0.2;
   prob_distribution p_gaussian=advmath::generate_gaussian_density(
      nbins,mu,sigma);

   unsigned char* renormalized_image=
      new unsigned char[
         get_texture_rectangle_ptr()->get_image_size_in_bytes()];

   vector<double> intensities;
   intensities.reserve(getWidth()*getHeight());

   const double normalization=1.0/255.0;
   for (int n=get_first_imagenumber(); n<=get_last_imagenumber(); n++)
   {
      cout << n << " " << flush;
      intensities.clear();
      get_VidFile_ptr()->read_image(n, get_m_image_ptr() );

// Load intensities greater than input threshold from current image
// into STL vector:

      for (unsigned int i=0; i<getHeight()*getWidth(); i++)
      {
         int grey_value=stringfunc::unsigned_char_to_ascii_integer(
            get_m_image_ptr()[i]);
         if (grey_value > intensity_thresholds[n])
         {
            intensities.push_back(normalization*grey_value);
         }
      }
      prob_distribution prob(intensities,nbins);

// To perform "intensity histogram equalization", we reset each
// normalized intensity value 0 <= x <= 1 to Pcum(x).  To perform
// "intensity histogram specification" onto the desired gaussian
// distribution, we perform an inverse histogram equalization and map
// Pcum(x) onto the y value for which Pcum(x) = Pgaussian(y):

      for (unsigned int i=0; i<getHeight()*getWidth(); i++)
      {
         int old_grey_value=
            stringfunc::unsigned_char_to_ascii_integer(get_m_image_ptr()[i]);
         int new_grey_value=old_grey_value;
         if (old_grey_value > intensity_thresholds[n])
         {
            double x=normalization*old_grey_value;
            int n=prob.get_bin_number(x);
            double pcum=prob.get_pcum(n);
//            new_grey_value=255*pcum;	// Histogram equalization
            double y=p_gaussian.find_x_corresponding_to_pcum(pcum);
            new_grey_value=static_cast<int>(255*y);
         }
         renormalized_image[i]=
            stringfunc::ascii_integer_to_unsigned_char(new_grey_value);
      } // loop over index i labeling unsigned char byte
      vid_out.WriteFrame(renormalized_image, getWidth());
   } // loop over index n labeling image numbers

   cout << endl;

   delete [] renormalized_image;
}

// ----------------------------------------------------------------
// Member function equalize_RGB_intensity_histograms reads in a
// demosaiced Group 99 RGB .vid file.  For each image in the file, it
// converts every pixel's RGB values to HSV.  It then forms a
// distribution for all intensity values lying above the input
// threshold.  This method remaps every image's intensity distribution
// onto a gaussian with some fixed mean and standard deviation.  The
// new V plus original H and S values are subsequently converted back
// to new RGB values and written to an output file with a
// "_renorm.vid" suffix.

void G99VideoDisplay::equalize_RGB_intensity_histograms(
   string input_RGB_video_filename,double intensity_threshold)
{
   outputfunc::write_banner("Equalizing RGB intensity histograms:");
//   int start_imagenumber=10;
//   int stop_imagenumber=13;
//   cout << "Enter starting imagenumber:" << endl;
//   cin >> start_imagenumber;
//   cout << "Enter stopping imagenumber:" << endl;
//   cin >> stop_imagenumber;

// Initialize output .vid file to hold renormalized grey-scale images:

   VidFile vid_out;
   unsigned int dot_posn=input_RGB_video_filename.rfind(".vid");
   string output_filename=input_RGB_video_filename.substr(0,dot_posn)
      +"_renorm.vid";
   vid_out.New_8U(
      output_filename.c_str(),getWidth(),getHeight(),get_Nimages(),3);

   const int nbins=200;
   double mu=0.5;
   double sigma=0.2;
   prob_distribution p_gaussian=advmath::generate_gaussian_density(
      nbins,mu,sigma);

   unsigned char* renormalized_image=
      new unsigned char[get_texture_rectangle_ptr()->
                        get_image_size_in_bytes()];

   const int n_pixels=getWidth()*getHeight();
   vector<double> h,s,v,non_negligible_intensities;
   h.reserve(n_pixels);
   s.reserve(n_pixels);
   v.reserve(n_pixels);
   non_negligible_intensities.reserve(n_pixels);

   for (int i=get_first_imagenumber(); i<=get_last_imagenumber(); i++)
   {
      cout << i << " " << flush;
      h.clear();
      s.clear();
      v.clear();
      non_negligible_intensities.clear();
      get_VidFile_ptr()->read_image(i, get_m_image_ptr() );

// Load RGB values and convert them to HSV.  Save intensities greater
// than input threshold within STL vector non_negligible_intensites:

      double curr_r,curr_g,curr_b,curr_h,curr_s,curr_v;
      for (int n=0; n<n_pixels; n++)
      {
         curr_r=static_cast<double>(
            stringfunc::unsigned_char_to_ascii_integer(
               get_m_image_ptr()[3*n+0]))/255.0;
         curr_g=static_cast<double>(
            stringfunc::unsigned_char_to_ascii_integer(
               get_m_image_ptr()[3*n+1]))/255.0;
         curr_b=static_cast<double>(
            stringfunc::unsigned_char_to_ascii_integer(
               get_m_image_ptr()[3*n+2]))/255.0;
         colorfunc::RGB_to_hsv(curr_r,curr_g,curr_b,curr_h,curr_s,curr_v);
         h.push_back(curr_h);
         s.push_back(curr_s);
         v.push_back(curr_v);
//         cout << "r = " << curr_r << " g = " << curr_g << " b = " << curr_b
//              << " h = " << curr_h << " s = " << curr_s << " v = " << curr_v
//              << endl;
         if (255*curr_v > intensity_threshold) 
            non_negligible_intensities.push_back(curr_v);
      } // loop over index n labeling pixels
      prob_distribution prob_v(non_negligible_intensities,nbins);

// To perform "intensity histogram equalization", we reset each
// normalized intensity value 0 <= x <= 1 to Pcum(x).  To perform
// "intensity histogram specification" onto a desired gaussian
// distribution, we perform an inverse histogram equalization and map
// Pcum(x) onto the y value for which Pcum(x) = Pgaussian(y):

      for (int n=0; n<n_pixels; n++)
      {
//         double old_s=s[n];
//         double new_s=basic_math::max(0.1,old_s);
         
         double old_v=v[n];
         double new_v=old_v;
         if (255*old_v > intensity_threshold)
         {
            double x=old_v;
            int b=prob_v.get_bin_number(x);
            double pcum=prob_v.get_pcum(b);
//            new_v=255*pcum;	// Histogram equalization
            new_v=p_gaussian.find_x_corresponding_to_pcum(pcum);
//            new_v=basic_math::min(0.75,new_v);
         }
//         colorfunc::hsv_to_RGB(h[n],new_s,old_v,curr_r,curr_g,curr_b);
         colorfunc::hsv_to_RGB(h[n],s[n],new_v,curr_r,curr_g,curr_b);
//         colorfunc::hsv_to_RGB(h[n],new_s,new_v,curr_r,curr_g,curr_b);
//         colorfunc::hsv_to_RGB(h[n],s[n],new_v,curr_r,curr_g,curr_b);
//         cout << "h = " << h[n] << " s = " << s[n] << " v = " << old_v
//              << " r = " << curr_r << " g = " << curr_g << " b = " << curr_b
//              << endl;
         renormalized_image[3*n+0]=stringfunc::ascii_integer_to_unsigned_char(
            static_cast<int>(curr_r*255));
         renormalized_image[3*n+1]=stringfunc::ascii_integer_to_unsigned_char(
            static_cast<int>(curr_g*255));
         renormalized_image[3*n+2]=stringfunc::ascii_integer_to_unsigned_char(
            static_cast<int>(curr_b*255));
      } // loop over index n labeling pixels
      vid_out.WriteFrame(renormalized_image, getWidth()*3);
   } // loop over index i labeling image numbers
   cout << endl;

   delete [] renormalized_image;
}

// ========================================================================
// Homography member functions
// ========================================================================

// Member function planar_orthorectify takes in the known (or
// estimated) physical world-space dimensions for some planar object.
// It also takes in homography H which we assume orthorectifies the
// view of this planar object.  Looping over every pixel within the
// orthorectified output image, this method computes associated planar
// world coordinates, and transforms the world coordinates to image
// plane coordinates.  This method then colors the output
// orthorectified pixel according to its image plane pixel's RGB
// values.  The resulting orthorectified image is returned within
// *transformed_data_ptr.

void G99VideoDisplay::planar_orthorectify(
   homography& H_in,double xmin,double xmax,double ymin,double ymax,
   unsigned char* transformed_data_ptr)
{
   int n=0;

   for (unsigned int pv_world=0; pv_world < getHeight(); pv_world++)
   {
      for (unsigned int pu_world=0; pu_world < getWidth(); pu_world++)
      {
         double u_world,v_world;
         get_uv_coords(pu_world,pv_world,u_world,v_world);
         double frac_u_world=(u_world-get_minU())/(get_maxU()-get_minU());
         double frac_v_world=(v_world-get_minV())/(get_maxV()-get_minV());
         
         double x_world=xmin+frac_u_world*(xmax-xmin);
         double y_world=ymin+frac_v_world*(ymax-ymin);

         double u_image,v_image;
         H_in.project_world_plane_to_image_plane(
            x_world,y_world,u_image,v_image);

         int R,G,B;
         R=G=B=0;
         get_RGB_values(u_image,v_image,R,G,B);

         transformed_data_ptr[3*n+0]=
            stringfunc::ascii_integer_to_unsigned_char(R);
         transformed_data_ptr[3*n+1]=
            stringfunc::ascii_integer_to_unsigned_char(G);
         transformed_data_ptr[3*n+2]=
            stringfunc::ascii_integer_to_unsigned_char(B);

         n++;
         
      } // loop over px index
   } // loop over py index
}

// ----------------------------------------------------------------
// Member function SetupTextureMatrix() comes from Noah Snavely's
// codes.  Noah sent us this snippet in Nov 2009 which he uses to warp
// photos onto 3-space quadrilaterals via homographies.

// This function takes care of loading the right texture matrix onto
// the texture matrix stack, given a camera 'cam' and texture
// coordinate boundaries 'bounds' (often [0,1] x [0,1])

void G99VideoDisplay::SetupTextureMatrix()
//   const CameraInfo &cam, const ParameterBound &bounds)
{

// Create the texture matrix 

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

//    double x_min = bounds.m_min_x;
//    double x_max = bounds.m_max_x;
//    double y_min = bounds.m_min_y;
//    double y_max = bounds.m_max_y;

    double x_min=0;
    double x_max=1;
    double y_min=0;
    double y_max=1;
    
    double x_length = x_max - x_min;
    double y_length = y_max - y_min;

    glTranslated(x_min + 0.5 * x_length, y_min + 0.5 * y_length, 0.0);
    glScaled(0.5 * x_length, 0.5 * y_length, 1.0);

//     Perspective projection 
//    double fov = RAD2DEG(2.0 * atan(0.5 * cam.m_height / cam.m_focal));
//    double aspect = ((double) cam.m_width) / ((double) cam.m_height);

//    double fov=30;
//    double aspect=4.0/3.0;
//    gluPerspective(fov, aspect, 1.0, 100.0);
  
    /* Setup the rigid transform */
//    double camera[16];

//    cam.GetRigid4x4(camera);
    
//    double cameraT[16];
//    matrix_transpose(4, 4, camera, cameraT);

//    glMultMatrixd(cameraT);
    
    glMatrixMode(GL_MODELVIEW);
}

