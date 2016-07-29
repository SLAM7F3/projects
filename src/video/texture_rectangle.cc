// Bug note added on 3/14/16: In fill_twoDarray_image(), alpha channel
// output appears to be 0 when n_channels = 4 even though we've
// explicitly set m_image[counter++] = unit_char !!

// Notes added on 1/15/13: Member function get_pixel_intensity()
// should be consolidated into get_pixel_intensity_value().  All
// entries in *ptwoDarray_ptr should be integers ranging from 0 to 255

// ========================================================================
// texture_rectangle provides functionality for displaying video files.
// ========================================================================
// Last updated on 5/30/16; 6/20/16; 6/21/16; 6/25/16
// ========================================================================

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <osg/BlendFunc>
#include <osg/Geometry>
#include <osg/ImageStream>
#include <osg/Quat>
#include <osgDB/ReaderWriter>
#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/WriteFile>
#include "math/adv_mathfuncs.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "math/basic_math.h"
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "distance_transform/dtfuncs.h"
#include "image/extremal_region.h"
#include "ffmpeg/FFMPEGVideo.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/plane.h"
#include "math/prob_distribution.h"
#include "image/raster_parser.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "datastructures/Triple.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::pair;
using std::string;
using std::vector;

// ========================================================================
// Initialization, constructor and destructor member functions
// ========================================================================

void texture_rectangle::allocate_member_objects()
{
//   cout << "inside texture_rectangle::allocate_member_objects()" << endl;
//   cout << "this = " << this << endl;
   TextureRectangle_refptr = new osg::TextureRectangle;

   stackSize=16777216; // = sqr(4096)
   stack=new unsigned int[stackSize];
}		       

// ----------------------------------------------------------------
void texture_rectangle::initialize_member_objects()
{
//   cout << "inside texture_rectangle::initialize_member_objects()" << endl;

   video_type=unknown;
   m_image=NULL;
//   cout << "m_image = " << m_image << endl;
   m_color_image=NULL;
//   cout << "m_color_image = " << m_color_image << endl;
   ColorMap_ptr=NULL;

   ptwoDarray_ptr=NULL;
   RtwoDarray_ptr=NULL;
   GtwoDarray_ptr=NULL;
   BtwoDarray_ptr=NULL;
   AtwoDarray_ptr=NULL;

   if (AnimationController_ptr != NULL)
   {
      AnimationController_ptr->set_first_framenumber(0);
      AnimationController_ptr->set_cumulative_framecounter(0);
   }

   first_frame_to_display=last_frame_to_display=0;
   image_size_in_bytes=prev_imagenumber=-1;
   set_panel_number(-1);

   colormap_flag=false;
   colorrange_ptr=NULL;
   m_g99Video=NULL;
   FFMPEGVideo_ptr=NULL;

   allocation_mode=osg::Image::NO_DELETE;

   GLimageDepth=1;
   GLtype=GL_UNSIGNED_BYTE;
   GLinternalTextureFormat=GL_RGB;
   GLpacking=1;

   min_U=0;
   min_V=0;
   max_V=1;
}

// ----------------------------------------------------------------
texture_rectangle::texture_rectangle()
{
//    cout << "inside texture_rectangle constructor#1" << endl;
//    cout << "this = " << this << endl;

   AnimationController_ptr=NULL;
   allocate_member_objects();
   initialize_member_objects();
   image_refptr = new osg::Image;
}

// ----------------------------------------------------------------
texture_rectangle::texture_rectangle(
   int width,int height,int n_images,int n_channels,
   AnimationController* AC_ptr):AnimationController_ptr(AC_ptr)
   {
//   cout << "inside texture_rectangle constructor#2" << endl;

      allocate_member_objects();
      initialize_member_objects();
      image_refptr = new osg::Image;

      setWidth(width);
      setHeight(height);

      Nimages=n_images;
      if (AnimationController_ptr != NULL)
      {
         AnimationController_ptr->set_nframes(n_images);
      }

      set_first_imagenumber(0);
      set_last_frame_to_display(get_last_imagenumber());

      m_Nchannels=n_channels;

// Note added on 3/2/16: We should really issue a call to
// initialize_general_image(width,height) here !!!

      initialize_general_image(width, height);
   }

// ----------------------------------------------------------------
texture_rectangle::texture_rectangle(
   string filename,AnimationController* AC_ptr):
AnimationController_ptr(AC_ptr)
{
//   cout << "inside texture_rectangle constructor#3" << endl;
//    cout << "this = " << this << endl;
//    cout << "filename = " << filename << endl;

   allocate_member_objects();
   initialize_member_objects();
   set_video_filename(filename);
   string suffix=stringfunc::suffix(video_filename);

   if (suffix=="vid")
   {
      video_type=G99Vid;
      image_refptr = new osg::Image;
      initialize_G99_video();
   }
   else if (suffix=="ntf" || suffix=="I21" || suffix=="I22")
   {
      video_type=still_image;
      image_refptr = new osg::Image;
      initialize_ntf_image();
   }
   else if (suffix=="png" || suffix=="PNG" || suffix=="jpg" 
            || suffix=="JPG" || suffix=="jpeg" 
            || suffix=="JPEG" || suffix=="tif" || suffix=="tiff" 
            || suffix=="rgb" || suffix=="ntf")
   {
      if (!import_photo_from_file(video_filename))
      {
         video_type=unknown;
      }

// On 4/13/09, Ross Anderson taught us that we don't need to
// instantiate member char* array m_image, for image_refptr already
// has data loaded into it when it's instantiated via a call to
// osgDB::readImageFile.

      m_image=NULL;
   }
   else if (suffix=="mp4" || suffix=="MP4" || suffix=="mov" 
            || suffix=="mpg")
   {
      video_type=video;
      image_refptr = new osg::Image;
      AnimationController_ptr->setDelay(0);
      FFMPEGVideo_ptr=FFMPEGVideo::fromFile(video_filename);

// As of 4/25/08, we believe Kevin Chen's FFMPEG code effectively
// ignores the final frame in any video stream.  We set Nimages to one
// less than the number of frames returned by *FFMPEGVideo_ptr:

//      Nimages=FFMPEGVideo_ptr->numFrames();
      Nimages=FFMPEGVideo_ptr->numFrames()-1;
      setWidth(FFMPEGVideo_ptr->width());
      setHeight(FFMPEGVideo_ptr->height());

      image_size_in_bytes=FFMPEGVideo_ptr->bytesPerFrame();
      m_Nchannels=image_size_in_bytes/(m_VidWidth*m_VidHeight);

      cout << "Nframes in FFMPEGVideo = " << Nimages << endl;
//      cout << "width = " << m_VidWidth << " height = " << m_VidHeight
//           << endl;
      cout << "Frame period in secs = " 
           << FFMPEGVideo_ptr->framePeriodSeconds() << endl;
      cout << "bytes per frame = " << image_size_in_bytes << endl;
      cout << "n_channels = " << m_Nchannels << endl;

      delete [] m_image;
      m_image = new unsigned char[ image_size_in_bytes ];
   }
   else
   {
      video_type=unknown;
      Nimages=1;
//      cout << 
//         "In texture_rectangle constructor, do not recognize input video file type"
//           << endl;
   }
   
   if (video_type != unknown) initialize_general_video();
}

// ----------------------------------------------------------------
texture_rectangle::~texture_rectangle()
{
//   cout << "inside texture_rectangle destructor" << endl;
//   cout << "this = " << this << endl;
//   cout << "m_image = " << m_image << endl;
   delete [] m_image;
//   cout << "m_color_image = " << m_color_image << endl;
   delete [] m_color_image;
//   cout << "m_g99Video = " << m_g99Video << endl;
   delete m_g99Video;
//   cout << "ColorMap_ptr = " << ColorMap_ptr << endl;
   delete ColorMap_ptr;

   delete ptwoDarray_ptr;
   delete RtwoDarray_ptr;
   delete GtwoDarray_ptr;
   delete BtwoDarray_ptr;
   delete AtwoDarray_ptr;

   m_image=NULL;
   m_color_image=NULL;
   m_g99Video=NULL;
   ColorMap_ptr=NULL;

   ptwoDarray_ptr=NULL;
   RtwoDarray_ptr=NULL;
   GtwoDarray_ptr=NULL;
   BtwoDarray_ptr=NULL;
   AtwoDarray_ptr=NULL;

   delete [] stack;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const texture_rectangle& t)
{
   outstream << "inside texture_rectangle::operator<<" << endl;
   outstream << "video_filename = " << t.video_filename << endl;
   outstream << "min_U = " << t.min_U << " max_U = " << t.max_U 
             << " dU = " << t.dU << endl;
   outstream << "min_V = " << t.min_V << " max_V = " << t.max_V 
             << " dV = " << t.dV << endl;
   if (t.video_type==texture_rectangle::still_image)
   {
      outstream << "video_type = still_image" << endl;
   }
   else if (t.video_type==texture_rectangle::video)
   {
      outstream << "video_type = video" << endl;

   }
   else if (t.video_type==texture_rectangle::G99Vid)
   {
      outstream << "video_type = G99Vid" << endl;
   }
   else
   {
      outstream << "video_type = unknown" << endl;
   }

   outstream << "width = " << t.getWidth() << " height = " << t.getHeight()
             << endl;
   outstream << "first_imagenumber = " << t.get_first_imagenumber()
             << " last_imagenumber = " << t.get_last_imagenumber()
             << " Nimages = " << t.Nimages << endl;
   outstream << "image_size_in_bytes = " << t.image_size_in_bytes << endl;
   return outstream;
}

// ========================================================================
// Set & get member functions
// ========================================================================

// For reasons we completely don't understand, get_video_filename()
// started causing seg faults on 12/1/13.  So for now, we move this
// get method out of texture_rectangle.h and into here:

std::string texture_rectangle::get_video_filename() 
{
   return video_filename;
}

const std::string& texture_rectangle::get_video_filename() const
{
   return video_filename;
}

// ---------------------------------------------------------------------
// Member function get_RGBA_twoDarrays() instantiates four twoDarrays
// and fills their contents with red, green, blue and alpha
// channel values for the current texture_rectangle.  This method
// returns the twoDarrays within an RGBA_array.

RGBA_array texture_rectangle::get_RGBA_twoDarrays(
   bool include_alpha_channel_flag)
{
//   cout << "inside texture_rectangle::get_RGBA_twoDarrays()" << endl;

   if (RtwoDarray_ptr==NULL ||
       (RtwoDarray_ptr->get_mdim() != getWidth() ||
        RtwoDarray_ptr->get_ndim() != getHeight()) )
   {
      delete RtwoDarray_ptr;
      delete GtwoDarray_ptr;
      delete BtwoDarray_ptr;
      delete AtwoDarray_ptr;
   
      RtwoDarray_ptr=new twoDarray(getWidth(),getHeight());
      GtwoDarray_ptr=new twoDarray(getWidth(),getHeight());
      BtwoDarray_ptr=new twoDarray(getWidth(),getHeight());
      if (include_alpha_channel_flag)
      {
         AtwoDarray_ptr=new twoDarray(getWidth(),getHeight());
      }
      else
      {
         AtwoDarray_ptr=NULL;
      }
   }
   
   RGBA_array rgba_array=RGBA_array(
      RtwoDarray_ptr,GtwoDarray_ptr,BtwoDarray_ptr,AtwoDarray_ptr);
   
   int R,G,B,A;
   for (unsigned int px=0; px<getWidth(); px++)
   {
      for (unsigned int py=0; py<getHeight(); py++)
      {
         get_pixel_RGBA_values(px,py,R,G,B,A);
         RtwoDarray_ptr->put(px,py,R/255.0);
         GtwoDarray_ptr->put(px,py,G/255.0);
         BtwoDarray_ptr->put(px,py,B/255.0);
         if (include_alpha_channel_flag) AtwoDarray_ptr->put(px,py,A/255.0);
      } // loop over py index
   } // loop over px index

   return rgba_array;
}

// ---------------------------------------------------------------------
// Member function set_from_RGB_twoDarrays() imports an RGBA_array
// which is assumed to be filled with RGB doubles ranging from 0 to 1.
// After converting to integers ranging from 0 to 255, this method
// copies the color information into each pixel of the current texture
// rectangle.

void texture_rectangle::set_from_RGB_twoDarrays(RGBA_array& rgba_array)
{
//   cout << "inside texture_rectangle::set_from_RGB_twoDarrays()" << endl;

   twoDarray* RtwoDarray_ptr=rgba_array.first;
   twoDarray* GtwoDarray_ptr=rgba_array.second;
   twoDarray* BtwoDarray_ptr=rgba_array.third;
   
   for (unsigned int px=0; px<getWidth(); px++)
   {
      for (unsigned int py=0; py<getHeight(); py++)
      {
         int R=255*RtwoDarray_ptr->get(px,py);
         int G=255*GtwoDarray_ptr->get(px,py);
         int B=255*BtwoDarray_ptr->get(px,py);
         set_pixel_RGB_values(px,py,R,G,B);
      } // loop over py index
   } // loop over px index

}

// ----------------------------------------------------------------
twoDarray* texture_rectangle::get_ptwoDarray_ptr()
{
//   cout << "inside texture_rectangle::get_ptwoDarray_ptr() #1" << endl;
//   cout << "textrect this = " << this << " ptwoDarray_ptr = " << ptwoDarray_ptr
//        << " mdim = " << ptwoDarray_ptr->get_mdim() 
//        << " ndim = " << ptwoDarray_ptr->get_ndim() << endl;
//   cout << "   Width() = " << getWidth() << " Height() = " << getHeight() 
//        << endl;
   return ptwoDarray_ptr;
}

const twoDarray* texture_rectangle::get_ptwoDarray_ptr() const
{
//   cout << "inside texture_rectangle::get_ptwoDarray_ptr() #2" << endl;
//   cout << "this = " << this << " ptwoDarray_ptr = " << ptwoDarray_ptr
//        << " xdim = " << ptwoDarray_ptr->get_xdim() 
//        << " ydim = " << ptwoDarray_ptr->get_ydim() << endl;
   return ptwoDarray_ptr;
}

// ----------------------------------------------------------------
twoDarray* texture_rectangle::instantiate_ptwoDarray_ptr()
{
//   cout << "inside texture_rectangle::instantiate_ptwoDarray_ptr()" << endl;
//   cout << "text_rect this = " << this << endl;

   if (ptwoDarray_ptr == NULL)
   {
      ptwoDarray_ptr=new twoDarray(getWidth(),getHeight());
   }
   else if (ptwoDarray_ptr != NULL)
   {
      if (getWidth() != ptwoDarray_ptr->get_mdim() ||
          getHeight() != ptwoDarray_ptr->get_ndim())
      {
         delete ptwoDarray_ptr;
         ptwoDarray_ptr=new twoDarray(getWidth(),getHeight());
      }
   }
   return ptwoDarray_ptr;
}

// ----------------------------------------------------------------
twoDarray* texture_rectangle::refresh_ptwoDarray_ptr()
{
   int channel_ID=1;	// red channel
   return refresh_ptwoDarray_ptr(channel_ID);
}

twoDarray* texture_rectangle::refresh_ptwoDarray_ptr(int channel_ID)
{
//   cout << "inside texture_rectangle::refresh_ptwoDarray_ptr()" << endl;
//   cout << "text_rect this = " << this << endl;

//   int n_channels=getNchannels();
//   cout << "n_channels=" << n_channels << endl;

   instantiate_ptwoDarray_ptr();

   int R,G,B,A;
   for (unsigned int px=0; px<getWidth(); px++)
   {
      for (unsigned int py=0; py<getHeight(); py++)
      {
         get_pixel_RGBA_values(px,py,R,G,B,A);
         if (channel_ID==1)
         {
            ptwoDarray_ptr->put(px,py,R);
         }
         else if (channel_ID==2)
         {
            ptwoDarray_ptr->put(px,py,G);
         }
         else if (channel_ID==3)
         {
            ptwoDarray_ptr->put(px,py,B);
         }
      } // loop over py index
   } // loop over px index
   return ptwoDarray_ptr;
}

// ----------------------------------------------------------------
void texture_rectangle::reset_ptwoDarray_ptr(twoDarray* qtwoDarray_ptr)
{
//   cout << "inside texture_rectangle::reset_ptwoDarray_ptr()" << endl;
   delete ptwoDarray_ptr;
   ptwoDarray_ptr=qtwoDarray_ptr;
}

// ----------------------------------------------------------------
// Member function export_sub_twoDarray() instantiates a new
// twoDarray *qtwoDarray_ptr.  It then copies the contents of
// *ptwoDarray_ptr within the bounding box specified by the input
// parameters onto *qtwoDarray_ptr.  

twoDarray* texture_rectangle::export_sub_twoDarray(
   unsigned int pu_start,unsigned int pu_stop,
   unsigned int pv_start,unsigned int pv_stop)
{
//   cout << "inside texture_rectangle::export_sub_twoDarray()" << endl;
//   cout << "pu_start = " << pu_start << " pu_stop = " << pu_stop << endl;
//   cout << "pv_start = " << pv_start << " pv_stop = " << pv_stop << endl;

   unsigned int mdim=pu_stop-pu_start+1;
   unsigned int ndim=pv_stop-pv_start+1;
//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;
   twoDarray* qtwoDarray_ptr=new twoDarray(mdim,ndim);

   for (unsigned int pu=pu_start; pu<=pu_stop; pu++)
   {
      for (unsigned int pv=pv_start; pv<=pv_stop; pv++)
      {
         qtwoDarray_ptr->put(
            pu-pu_start,pv-pv_start,ptwoDarray_ptr->get(pu,pv));
      } // loop over pv index
   } // loop over pu index

//   cout << "At end of export_sub_twoDarray()" << endl;
   return qtwoDarray_ptr;
}

// ========================================================================
// Video initialization member functions
// ========================================================================

// Member function import_photo_from_file() takes in the name for some
// JPEG, PNG, TIF, etc image.  It uses an OSG plugin to load the
// contents of the image file into *image_refptr.  This method resets
// various image parameters such as pixel width, height and number of
// images.  If the photo is not successfully read in, this boolean
// method returns false.

bool texture_rectangle::import_photo_from_file(string photo_filename)
{
//   cout << "inside texture_rectangle::import_photo_from_file()" << endl;
//   cout << "photo_filename = " << photo_filename << endl;

   if (!filefunc::fileexist(photo_filename))
   {
      cout << "Inside texture_rectangle::import_photo_from_file()" 
           << endl;
      cout << "Cannot find photo_filename = " << photo_filename 
           << " photo_filename.size() = " << photo_filename.size() << endl;
      return false;
   }

   string suffix=stringfunc::suffix(photo_filename);
//   cout << "suffix = " << suffix << endl;
   if (suffix=="png" || suffix=="jpg" || suffix=="JPG" || suffix=="jpeg" 
       || suffix=="JPEG" || suffix=="tif" || suffix=="tiff" 
       || suffix=="rgb" || suffix=="ntf")
   {
      video_type=still_image;
   }
   else
   {
      return false;
   }
   
// As of 10/4/09, we believe that images read in from external files
// are not being automatically deallocated.  This memory leak
// eventually leads to an out-of-memory error in BUNDLECITIES,
// PROPAGATOR, etc.  We try to force the current image to be
// deallocated before reading in a new image.  But the following lines
// do NOT appear to solve the memory leak problem...

/*
  if (image_refptr.valid())
  {
  cout << "Before resetting image's data to NULL" << endl;
  cout << "image_refptr.get() = " << image_refptr.get() << endl;
  image_refptr->setImage( m_VidWidth, m_VidHeight, GLimageDepth,
  GLinternalTextureFormat,GLformat,GLtype,
  NULL, allocation_mode, GLpacking );
  }
*/

// On 5/5/10, we discovered the painful way that the following call to
// readImageFile() can fail and yield a "Could not find plugin to read
// objects from file" warning.  This occurs even though the input
// photo_filename is valid.  We empirically found that we may need to
// issue several hundred repeated calls to readImageFile() before the
// file is actually imported.  (Maybe this is an indication of some
// race condition going on?).  For now, we live with the cluge of
// calling readImageFile() a large number of times before eventually
// giving up and returning false...
    
   image_refptr=osgDB::readImageFile(photo_filename);
//   cout << "image_refptr.get() = " << image_refptr.get() << endl;


//      cout << "image_refptr->getPixelFormat() = "
//           << image_refptr->getPixelFormat() << endl;
//      cout << "image_refptr->getDataType() = "
//           << image_refptr->getDataType() << endl;
//      cout << "image_refptr->getPixelSizeInBits() = "
//           << image_refptr->getPixelSizeInBits() << endl;
      
   int iter=0;
   const int max_iters=10000;
   while (!image_refptr.valid())
   {
      iter++;
      image_refptr=osgDB::readImageFile(photo_filename);
      if (iter > max_iters)
      {
         cout << "Inside texture_rectangle::import_photo_from_file()"
              << endl;
         cout << "Gave up trying to read in image file = "
              << photo_filename << " after "+stringfunc::number_to_string(
                 max_iters)+" iterations!" << endl;
         return false;
      }
   }
      
// In Oct 09, Ross taught us that we need to explicitly set
// image_refptr to NULL s.t. this class does not keep an image's data
// in memory after it's no longer needed...

/*
//      double horiz_scale_factor=0.2;
double horiz_scale_factor=0.5;
//      double horiz_scale_factor=1.0;
//      double horiz_scale_factor=2.0;
int new_s=horiz_scale_factor*image_refptr->s();
int new_t=horiz_scale_factor*image_refptr->t();
int new_r=image_refptr->r();
cout << "new_s = " << new_s << " new_t = " << new_t 
<< " new_r = " << new_r << endl;

image_refptr->scaleImage(
new_s,new_t,new_r);
//         horiz_scale_factor*image_refptr->s(),
//         horiz_scale_factor*image_refptr->t(),image_refptr->r());
*/

// We need to flip the image vertically after reading in its bytes:

   image_refptr->flipVertical();

   setWidth(image_refptr->s());
   setHeight(image_refptr->t());
   max_U=double(getWidth())/double(getHeight());
//   cout << "max_U = " << max_U << endl;

   int PixelSizeInBits=image_refptr->getPixelSizeInBits();
//   cout << "PixelSizeInBits = " << PixelSizeInBits << endl;
   m_Nchannels=PixelSizeInBits/8;
//   cout << "m_Nchannels = " << m_Nchannels << endl;

// On 3/27/16, we discovered the hard & painful way that some input
// PNG images have 16-bit rather than 8-bit depths.  We cannot
// currently handle 16-bit color channels...

   if(m_Nchannels > 4)
   {
      cout << "  PixelSizeInBits=" << PixelSizeInBits 
           << " m_Nchannels=" << m_Nchannels 
           << " in texture_rectangle::import_photo_from_file()"
           << endl;
      return false;
   }

   Nimages=1;
   image_size_in_bytes=m_VidWidth * m_VidHeight * m_Nchannels;

//   cout << "m_VidWidth = " << m_VidWidth 
//        << " m_VidHeight = " << m_VidHeight << endl;
//   cout << "image_size_in_bytes = " << image_size_in_bytes << endl;

   set_video_filename(photo_filename);

// On 6/1/12, we added the next line in order to ensure that
// set_GL_format() is called even if texture_rectangle object was
// instantiated with trivial constructor:

   initialize_general_video();

//   check_all_pixel_RGB_values();
   return true;
}

// ---------------------------------------------------------------------
// Member function fast_import_photo_from_file() takes in the name for
// some JPEG, PNG, TIF, etc image.  It uses an OSG plugin to load the
// contents of the image file into *image_refptr.  This method resets
// various image parameters such as pixel width, height and number of
// images.  If the photo is not successfully read in, this boolean
// method returns false.

bool texture_rectangle::fast_import_photo_from_file(string photo_filename)
{
//   cout << "inside texture_rectangle::fast_import_photo_from_file()" << endl;
//   cout << "photo_filename = " << photo_filename << endl;

   video_type=still_image;
   
// On 5/5/10, we discovered the painful way that the following call to
// readImageFile() can fail and yield a "Could not find plugin to read
// objects from file" warning.  This occurs even though the input
// photo_filename is valid.  We empirically found that we may need to
// issue several hundred repeated calls to readImageFile() before the
// file is actually imported.  (Maybe this is an indication of some
// race condition going on?).  For now, we live with the cluge of
// calling readImageFile() a large number of times before eventually
// giving up and exiting the program...
    
   image_refptr=osgDB::readImageFile(photo_filename);
//   cout << "image_refptr.get() = " << image_refptr.get() << endl;


//      cout << "image_refptr->getPixelFormat() = "
//           << image_refptr->getPixelFormat() << endl;
//      cout << "image_refptr->getDataType() = "
//           << image_refptr->getDataType() << endl;
//      cout << "image_refptr->getPixelSizeInBits() = "
//           << image_refptr->getPixelSizeInBits() << endl;
      
   int iter=0;
   const int max_iters=10000;
   while (!image_refptr.valid())
   {
      iter++;
      image_refptr=osgDB::readImageFile(photo_filename);
      if (iter > max_iters)
      {
         cout << "Inside texture_rectangle::import_photo_from_file()"
              << endl;
         cout << "Gave up trying to read in image file = "
              << photo_filename << " after "+stringfunc::number_to_string(
                 max_iters)+" iterations!" << endl;
         return false;
      }
   }
//   cout << "iter = " << iter << endl;
      
// We need to flip the image vertically after reading in its bytes:

   image_refptr->flipVertical();

   setWidth(image_refptr->s());
   setHeight(image_refptr->t());

   int PixelSizeInBits=image_refptr->getPixelSizeInBits();
//   cout << "PixelSizeInBits = " << PixelSizeInBits << endl;
   m_Nchannels=PixelSizeInBits/8;
//   cout << "m_Nchannels = " << m_Nchannels << endl;

   Nimages=1;
   image_size_in_bytes=m_VidWidth * m_VidHeight * m_Nchannels;

//   cout << "m_VidWidth = " << m_VidWidth 
//        << " m_VidHeight = " << m_VidHeight << endl;
//   cout << "image_size_in_bytes = " << image_size_in_bytes << endl;

   set_video_filename(photo_filename);

   return true;
}

// ----------------------------------------------------------------
void texture_rectangle::initialize_G99_video()
{
//   cout << "inside texture_rectangle::initialize_G99_video()" << endl;
//   cout << "video_filename = " << video_filename << endl;
   
   m_g99Video = new VidFile( video_filename );
   m_g99Video->query_structure_values(); // print out video information

   setWidth(m_g99Video->getWidth());
   setHeight(m_g99Video->getHeight());

// Anye Li taught us around Xmas 2006 that OSG textures cannot be
// larger than 5000 pixels in either their width or height directions.
// So in order to see any video output, we must ensure that the
// texture box does not become too large...

// FAKE FAKE:  for alg testing only,...Sun Aug 12 at 5:30 am

//   m_VidWidth=basic_math::min(4000,int(m_VidWidth));
//   m_VidHeight=basic_math::min(4000,int(m_VidHeight));

   m_Nchannels = m_g99Video->getNumChannels();
   Nimages=m_g99Video->getNumFrames();

   image_size_in_bytes=m_VidWidth * m_VidHeight * m_Nchannels;

   delete [] m_image;
   m_image = new unsigned char[ image_size_in_bytes ];
}

// ----------------------------------------------------------------
void texture_rectangle::initialize_ntf_image()
{
//   cout << "inside texture_rectangle::initialize_ntf_image()" << endl;
//   cout << "video_filename = " << video_filename << endl;

   raster_parser RasterParser;
   RasterParser.open_image_file(video_filename);

   int channel_ID=0;
   RasterParser.fetch_raster_band(channel_ID);

   twoDarray* ztwoDarray_ptr=RasterParser.get_ztwoDarray_ptr();
   RasterParser.read_raster_data(ztwoDarray_ptr);

   cout << "xlo = " << ztwoDarray_ptr->get_xlo() << endl;
   cout << "xhi = " << ztwoDarray_ptr->get_xhi() << endl;
   cout << "ylo = " << ztwoDarray_ptr->get_ylo() << endl;
   cout << "yhi = " << ztwoDarray_ptr->get_yhi() << endl;

   setWidth(RasterParser.get_raster_Xsize());
   setHeight(RasterParser.get_raster_Ysize());
   m_Nchannels=RasterParser.get_n_channels();
   set_GLformat();
   Nimages=1;

   cout << "m_Nchannels = " << m_Nchannels << endl;

   image_size_in_bytes=m_VidWidth * m_VidHeight * m_Nchannels;

   delete [] m_image;
   m_image = new unsigned char[ image_size_in_bytes ];

//   cout << "width = " << m_VidWidth
//        << " height = " << m_VidHeight << endl;

   double z_min,z_max;
   ztwoDarray_ptr->minmax_values(z_min,z_max);
   cout << "z_min = " << z_min << " z_max = " << z_max << endl;
   bool rescale_zvalues_flag=false;
   RasterParser.fill_Z_unsigned_char_array(
      z_min,z_max,ztwoDarray_ptr,m_image,rescale_zvalues_flag);
   
   set_image();
}

// ----------------------------------------------------------------
// Member function initialize_twoDarray_image() takes in twoDarray
// *ptwoDarray_ptr which is assumed to hold probability values ranging
// from 0 to 1.  It sets the parameters for a texture rectangle to
// hold a still image version of the input data.

void texture_rectangle::initialize_twoDarray_image(
   const twoDarray* ptwoDarray_ptr,int n_channels,bool blank_png_flag)
{
//   cout << "inside texture_rectangle::initialize_twoDarray_image(twoDarray*)" 
//        << endl;

//   cout << "xlo = " << ptwoDarray_ptr->get_xlo() << endl;
//   cout << "xhi = " << ptwoDarray_ptr->get_xhi() << endl;
//   cout << "ylo = " << ptwoDarray_ptr->get_ylo() << endl;
//   cout << "yhi = " << ptwoDarray_ptr->get_yhi() << endl;

   video_type=still_image;
   setWidth(ptwoDarray_ptr->get_mdim());
   setHeight(ptwoDarray_ptr->get_ndim());
//   cout << "width = " << m_VidWidth
//        << " height = " << m_VidHeight << endl;

   m_Nchannels=n_channels;
   set_GLformat();
   Nimages=1;

   image_size_in_bytes=m_VidWidth * m_VidHeight * m_Nchannels;

   delete [] m_image;
   m_image = new unsigned char[ image_size_in_bytes ];

   fill_twoDarray_image(ptwoDarray_ptr,n_channels,blank_png_flag);

   if (!image_refptr.valid()) image_refptr = new osg::Image;
   set_image();
}

// ----------------------------------------------------------------
void texture_rectangle::initialize_RGB_twoDarray_image(
   const twoDarray* RtwoDarray_ptr)
{
//   cout << "inside texture_rectangle::initialize_RGB_twoDarray_image()" 
//        << endl;

   initialize_general_image(
      RtwoDarray_ptr->get_mdim(), RtwoDarray_ptr->get_ndim());
}

// ----------------------------------------------------------------
// Member function initialize_general_image() instantiates an unsigned
// byte array whose size is set by input arguments width and height
// along with m_Nchannels.  The byte array is initialized with zeros
// and then assigned as the data content for image_refptr.

// We wrote this method in January 2016 to eliminate our horribly
// inefficient of creating a blank image on disk and then reading that
// image in order just to create an empty image in memory.

void texture_rectangle::initialize_general_image(int width, int height)
{
//   cout << "inside texture_rectangle::initialize_general_image()" 
//        << endl;

   video_type=still_image;
   setWidth(width);
   setHeight(height);

   set_GLformat();
   Nimages=1;
   image_size_in_bytes=m_VidWidth * m_VidHeight * m_Nchannels;
//   cout << "image size (Mbytes) = " << 1E-6*image_size_in_bytes << endl;

   delete [] m_image;
   m_image = new unsigned char[ image_size_in_bytes ];
   for(int i = 0; i < image_size_in_bytes; i++)
   {
      m_image[i] = 0;
   }

   if (!image_refptr.valid()) image_refptr = new osg::Image;
   set_image();
}

// ----------------------------------------------------------------
// Member function fill_twoDarray_image() takes in twoDarray
// *ptwoDarray_ptr which is assumed to hold probability values ranging
// from 0 to 1.  It transfers the contents of *ptwoDarray_ptr to
// member char array m_image.

void texture_rectangle::fill_twoDarray_image(
   const twoDarray* ptwoDarray_ptr,unsigned int n_channels,bool blank_png_flag)
{
//   cout << "inside texture_rectangle::fill_twoDarray_image(twoDarray*)" 
//        << endl;
//   cout << "n_channels = " << n_channels
//        << " blank_png_flag = " << blank_png_flag << endl;

//   cout << "xlo = " << ptwoDarray_ptr->get_xlo() << endl;
//   cout << "xhi = " << ptwoDarray_ptr->get_xhi() << endl;
//   cout << "ylo = " << ptwoDarray_ptr->get_ylo() << endl;
//   cout << "yhi = " << ptwoDarray_ptr->get_yhi() << endl;

   double p_min,p_max;
   ptwoDarray_ptr->minmax_values(p_min,p_max);
//   cout << "p_min = " << p_min << " p_max = " << p_max << endl;

   const unsigned char unit_char=
      stringfunc::ascii_integer_to_unsigned_char(255);
   const unsigned char zero_char=
      stringfunc::ascii_integer_to_unsigned_char(0);

   int neg_p_values=0;
   int p_values_0_2=0;
   int p_values_2_4=0;
   int p_values_4_6=0;
   int p_values_6_8=0;
   int p_values_8_10=0;
//   cout << "*ptwoDarray_ptr = " << *ptwoDarray_ptr << endl;

   int counter=0;
   for (unsigned int py=0; py<ptwoDarray_ptr->get_ndim(); py++)
   {
      for (unsigned int px=0; px<ptwoDarray_ptr->get_mdim(); px++)
      {
         double curr_p=ptwoDarray_ptr->get(px,py);

         if (curr_p < 0)
         {
            neg_p_values++;
         }
         else if (curr_p >=0 && curr_p < 0.2)
         {
            p_values_0_2++;
         }
         else if (curr_p >=0.2 && curr_p < 0.4)
         {
            p_values_2_4++;
         }
         else if (curr_p >=0.4 && curr_p < 0.6)
         {
            p_values_4_6++;
         }
         else if (curr_p >=0.6 && curr_p < 0.8)
         {
            p_values_6_8++;
         }
         else if (curr_p >=0.8)
         {
            p_values_8_10++;
         }

         const double SMALL=0.00001;
         if (curr_p < SMALL)
         {
            for (unsigned int i=0; i<n_channels; i++)
            {
               m_image[counter++]=zero_char;
            }
         }
         else
         {
            unsigned int ui_p=static_cast<unsigned int>(255*curr_p);

// On 11/23/10, we learned the hard and painful way that we need to
// apply a ceiling threshold so that ui_p never exceeds 255!

            if (ui_p > 255) ui_p=255;
            unsigned char p_char=stringfunc::ascii_integer_to_unsigned_char(
               ui_p);

//         cout << "px = " << px << " py = " << py 
//              << " curr_p = " << curr_p 
//              << " ui_p = " << ui_p 
//              << endl;

            for (unsigned int i=0; i<n_channels; i++)
            {
               if (n_channels==4 && i==3)
               {
                  if (blank_png_flag)
                  {
//                     m_image[counter++]=zero_char;
                     m_image[counter++]=unit_char;
                  }
                  else
                  {
                     m_image[counter++]=unit_char;
                  }
               }
               else
               {
                  m_image[counter++]=p_char;
               }
            } // loop over index i labeling color channels

         } // curr_p < 0 conditional
      } // loop over px index
   } // loop over py index

//   cout << "neg_p_values = " << neg_p_values << endl;
//   cout << "p_values_0_0.2 = " << p_values_0_2 << endl;
//   cout << "p_values_0.2_0.4 = " << p_values_2_4 << endl;
//   cout << "p_values_0.4_0.6 = " << p_values_4_6 << endl;
//   cout << "p_values_0.6_0.8 = " << p_values_6_8 << endl;
//   cout << "p_values_0.8_1 = " << p_values_8_10 << endl;
}

// ----------------------------------------------------------------
void texture_rectangle::initialize_general_video()
{
//   cout << "inside texture_rectangle::initialize_general_video()" << endl;

   image_size_in_bytes=m_VidWidth * m_VidHeight * m_Nchannels;
   set_GLformat();

   set_first_imagenumber(0);

   if (AnimationController_ptr != NULL)
   {
      AnimationController_ptr->set_nframes(
         basic_math::max(Nimages,AnimationController_ptr->get_nframes()));
      set_last_frame_to_display(AnimationController_ptr->get_nframes()-1);
   }
   else
   {
      set_last_frame_to_display(0);
   }

   double Ufactor=1;
   double Vfactor=1;
   reset_UV_coords(0,Ufactor*double(m_VidWidth)/double(m_VidHeight),
                   0,Vfactor*1);
}

// ----------------------------------------------------------------
void texture_rectangle::set_GLformat()
{
//   cout << "inside texture_rectangle::set_GLformat(), m_Nchannels = "
//        << m_Nchannels << endl;
   if (m_Nchannels==1)
   {
      GLformat=GL_LUMINANCE;
      GLinternalTextureFormat=GL_LUMINANCE;
   }
   else if (m_Nchannels==2)
   {
      GLformat=GL_LUMINANCE_ALPHA;
      GLinternalTextureFormat=GL_LUMINANCE_ALPHA;
   }
   else if (m_Nchannels==3)
   {
      GLformat=GL_RGB;
      GLinternalTextureFormat=GL_RGB;
   }
   else if (m_Nchannels==4)
   {
      GLformat=GL_RGBA;
      GLinternalTextureFormat=GL_RGBA;
   }
}

// ----------------------------------------------------------------
void texture_rectangle::reset_UV_coords(
   double min_U,double max_U,double min_V,double max_V)
{
//   cout << "inside texture_rectangle::reset_UV_coords()" << endl;
   this->min_U=min_U;
   this->max_U=max_U;
   this->min_V=min_V;
   this->max_V=max_V;
   dU=(max_U-min_U)/m_VidWidth;
   dV=(max_V-min_V)/m_VidHeight;

//   cout << "VidWidth = " << getWidth() << " VidHeight = " << getHeight()
//        << endl;
//   cout << "Width*Height = " << m_VidWidth*m_VidHeight << endl;
//   cout << "min_U = " << min_U << " max_U = " << max_U 
//        << " dU = " << dU << endl;
//   cout << "min_V = " << min_V << " max_V = " << max_V 
//        << " dV = " << dV << endl;
}


// ----------------------------------------------------------------
void texture_rectangle::set_TextureRectangle_image()
{
//   cout << "inside texture_rectangle::set_TextureRectangle_image()" << endl;
   TextureRectangle_refptr->setImage( get_image_ptr() );
}

// ----------------------------------------------------------------
// Member function fill_ptwoDarray_from_single_channel_byte_data()

twoDarray* texture_rectangle::fill_ptwoDarray_from_single_channel_byte_data()
{
//   cout << "inside texture_rectangle::fill_ptwoDarray_from_single_channel_byte_data()" 
//        << endl;
//   cout << "n_channels = " << getNchannels() << endl;
   
   instantiate_ptwoDarray_ptr();

//   cout << "mdim = " << ptwoDarray_ptr->get_mdim()
//        << " ndim = " << ptwoDarray_ptr->get_ndim() 
//        << endl;

   for (unsigned int py=0; py<ptwoDarray_ptr->get_ndim(); py++)
   {
      for (unsigned int px=0; px<ptwoDarray_ptr->get_mdim(); px++)
      {
         int curr_intensity=get_pixel_intensity_value(px,py);
         ptwoDarray_ptr->put(px,py,curr_intensity);

//         if (curr_intensity > 0) 
//            cout << "px = " << px << " py = " << py 
//                 << " intensity = " << curr_intensity << endl;

      } // loop over px
   } // loop over py

   return ptwoDarray_ptr;
}

// ========================================================================
// Stringstream member functions
// ========================================================================

// Member function read_image_from_char_buffer() implements Ross
// Anderson's suggestion to read JPEG, PNG, etc encoded imagery data
// into a string stream.  The imagery data could have been sent by Qt
// across a TCP/IP connection, read into a QDataArray and subsequently
// converted into input char* array *buffer_ptr.  Its size in bytes is
// specified by input parameter image_size.  This method converts the
// char* data into a stringstream and subsequently into osg Image
// format via an osgDB::ReaderWriter object.  

// With lots of help from Ross, we wrote this method in May 2009 for
// purposes of reconstructing jpegs within the Analysis program for
// the Real-Time Persistent Surveillance project.

void texture_rectangle::read_image_from_char_buffer(
   string input_image_suffix,const char* buffer_ptr,int image_size)
{
//   cout << "inside texture_rectangle::read_image_from_char_buffer()" << endl;
//   cout << "image_size = " << image_size << endl;
   
   if (!Registry_refptr.valid()) Registry_refptr=osgDB::Registry::instance(); 

   osgDB::ReaderWriter* ReaderWriter_ptr=Registry_refptr->
      getReaderWriterForExtension(input_image_suffix);

   string buffer_str(buffer_ptr,image_size);
   istringstream is(buffer_str);

   osgDB::ReaderWriter::ReadResult read_result=
      ReaderWriter_ptr->readImage(is);
   image_refptr=read_result.getImage();

// We need to flip the image vertically after reading in its bytes:

   image_refptr->flipVertical();

   setWidth(image_refptr->s());
   setHeight(image_refptr->t());
   m_Nchannels=3;
   Nimages=1;
   image_size_in_bytes=m_VidWidth * m_VidHeight * m_Nchannels;
   reset_UV_coords(0,double(getWidth())/double(getHeight()),0,1);

//   cout << "image_refptr->s = " << image_refptr->s() << endl;
//   cout << "image_refptr->t = " << image_refptr->t() << endl;
//   cout << "getWidth() = " << getWidth() << " getHeight() = "
//        << getHeight() << endl;

   image_refptr->allocateImage(getWidth(),getHeight(),1,
                               GLformat,GLtype);
   image_refptr=read_result.getImage();
}

// ========================================================================
// Image numbering member functions
// ========================================================================

int texture_rectangle::get_Nimages() const
{
   return Nimages;
}

void texture_rectangle::set_first_frame_to_display(int i)
{
//   cout << "inside texture_rectangle::set_first_frame_to_display(), i = "
//        << i << endl;
   
   first_frame_to_display=basic_math::max(0,i);
//   cout << "first_frame_to_display = " << first_frame_to_display
//        << " last_frame_to_display = " << last_frame_to_display << endl;

   if (last_frame_to_display >= 1)
   {
      first_frame_to_display=basic_math::min(
         first_frame_to_display,last_frame_to_display-1);
   }

//   cout << "first_frame_to_display = " << first_frame_to_display << endl;
}

void texture_rectangle::set_last_frame_to_display(int i)
{
//   cout << "inside texture_rectangle::set_last_frame_to_display(), i = " 
//        << i << endl;
//   cout << "first_frame_to_display = " << first_frame_to_display << endl;
   last_frame_to_display=basic_math::max(first_frame_to_display+1,i);

// As of July 2010, we want to enable temporal sequences of video
// frame stills to be played back as a movie.  For such video sequences, 
// last_frame_to_display != 1 even though VideoType = still_image !

//   cout << "get_Nimages() = " << get_Nimages() << endl;
   
   if (get_VideoType() != still_image)
   {
      last_frame_to_display=basic_math::min(
         get_Nimages()-1,last_frame_to_display);
   }
//   cout << "last_frame_to_display = " << last_frame_to_display << endl;
}

int texture_rectangle::get_first_frame_to_display() const
{
   return first_frame_to_display;
}

int texture_rectangle::get_last_frame_to_display() const
{
   return last_frame_to_display;
}

int texture_rectangle::get_imagenumber() const
{ 
//   cout << "inside texture_rectangle::get_imagenumber()" << endl;

   int Nframes_to_display=last_frame_to_display-first_frame_to_display+1;

//   cout << "first_frame_to_display = " << first_frame_to_display
//        << " last_frame_to_display = " << last_frame_to_display << endl;
//   cout << "Nframes_to_display = " << Nframes_to_display << endl;

   if (FFMPEGVideo_ptr != NULL && 
       AnimationController_ptr->get_loop_to_beginning())
   {
      AnimationController_ptr->set_loop_to_beginning(false);
//      bool OK_flag=
      FFMPEGVideo_ptr->setAndDecodeNextFrame(first_frame_to_display);
//      cout << "Looping to beginning, OK_flag = " << OK_flag << endl;
   }

   if (FFMPEGVideo_ptr != NULL && 
       AnimationController_ptr->get_loop_to_end())
   {
      AnimationController_ptr->set_loop_to_end(false);
//      bool OK_flag=
      FFMPEGVideo_ptr->setAndDecodeNextFrame(last_frame_to_display);
//      cout << "Looping to end, OK_flag = " << OK_flag << endl;
   }

   int curr_frame=first_frame_to_display+modulo(
      AnimationController_ptr->get_curr_framenumber(),Nframes_to_display);

//   if (fabs(curr_frame-0) < 3 || fabs(curr_frame-last_frame_to_display) < 3)
//   {
//      cout << " curr_frame = " << curr_frame << endl;
//   }

   if (FFMPEGVideo_ptr != NULL)
   {
      int frame_skip=AnimationController_ptr->get_frame_skip();
      if (frame_skip != FFMPEGVideo_ptr->get_frame_skip())
      {
         FFMPEGVideo_ptr->set_frame_skip(frame_skip);
      }
   }

   return curr_frame;
}

// ========================================================================
// Frame display member functions
// ========================================================================

void texture_rectangle::display_current_frame()
{
//   cout << "inside texture_rectangle::display_curr_frame()" << endl;
//   outputfunc::enter_continue_char();
//   cout << "video_type = " << video_type << endl;

   if (video_type==G99Vid)
   {
      read_and_set_image();
   }
   else if (video_type==still_image)
   {
      int curr_imagenumber=get_imagenumber();
//      cout << "curr_imagenumber = " << curr_imagenumber
//           << " prev_imagenumber = " << prev_imagenumber << endl;
      int delta_imagenumber=curr_imagenumber-prev_imagenumber;
      if (delta_imagenumber != 0)
      {
         read_next_photo();
         prev_imagenumber=curr_imagenumber;
      }
   }
   else if (video_type==video)
   {
      int curr_imagenumber=get_imagenumber();
//      cout << "curr_image = " << curr_imagenumber
//           << " prev_image = " << prev_imagenumber << endl;

      int delta_imagenumber=curr_imagenumber-prev_imagenumber;
      if (delta_imagenumber != 0)
      {
//         if (delta_imagenumber != 1)
//         {
//            cout << "*****************************************************"
//                 << endl;
//            cout << "In texture_rectangle::display_current_frame()"
//                 << endl;
//            cout << "curr_imagenumber = " << curr_imagenumber
//                 << " prev_imagenumber = " << prev_imagenumber << endl;
//            cout << "Delta = " << delta_imagenumber << endl;
//            outputfunc::enter_continue_char();
//         }

         if ( (delta_imagenumber < 1) ||
              (delta_imagenumber > AnimationController_ptr->get_frame_skip()) 
              || (curr_imagenumber != FFMPEGVideo_ptr->getNextFrameIndex()) )
         {
//            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//            cout << "Before call to setAndDecodeNextFrame, nextFrameIndex = "
//                 << FFMPEGVideo_ptr->getNextFrameIndex() << endl;
//            cout << "curr_imagenumber = " << curr_imagenumber << endl;
//            cout << "delta_imagenumber = " << delta_imagenumber
//                 << " frame skip = "
//                 << AnimationController_ptr->get_frame_skip() << endl;
//            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

            if (FFMPEGVideo_ptr->setAndDecodeNextFrame(curr_imagenumber))
            {
               FFMPEGVideo_ptr->getNextFrame(m_image);
               FFMPEGVideo_ptr->setNextFrameIndex(
                  curr_imagenumber+
                  AnimationController_ptr->get_frame_skip());
               prev_imagenumber=curr_imagenumber;
            }
         }
         else
         {
//            cout << "Before call to getNextFrame, nextFrameIndex = "
//                 << FFMPEGVideo_ptr->getNextFrameIndex() << endl;
            if (FFMPEGVideo_ptr->getNextFrame(m_image))
            {
               prev_imagenumber=curr_imagenumber;
            }
         }
      }
      set_image();

   } // video_type==video flag
//   cout << "At end of texture_rectangle::display_curr_frame()" << endl;
}

// ----------------------------------------------------------------
void texture_rectangle::displayFrame(int p_framenum)
{
//   cout << "inside texture_rectangle::displayFrame(), frame = " << p_framenum
//        << endl;
//   outputfunc::enter_continue_char();
   
   if (p_framenum < first_frame_to_display) 
   {
      p_framenum=last_frame_to_display;
   }
   if (p_framenum > last_frame_to_display )
   {
      p_framenum=first_frame_to_display;
   }
   AnimationController_ptr->set_curr_framenumber(p_framenum);
   AnimationController_ptr->set_cumulative_framecounter(
      AnimationController_ptr->get_cumulative_framecounter()+1);

   read_and_set_image();
}

// ----------------------------------------------------------------
void texture_rectangle::read_and_set_image()
{
//   cout << "inside texture_rectangle::read_and_set_image()" << endl;
//   outputfunc::enter_continue_char();

   if (video_type != G99Vid) return;
//   if (video_type == video || video_type==unknown) return;

// Read in unsigned char* data for the current image only once!

   int curr_imagenumber=get_imagenumber();
   if (curr_imagenumber != prev_imagenumber)
   {
      m_g99Video->read_image( curr_imagenumber, m_image );
      prev_imagenumber=curr_imagenumber;
   }
   
// Fill the osg::Image object with data from the file:

   if (colormap_flag)
   {
      osg::Vec4 RGBA;
      for (unsigned int i=0; i<m_VidHeight*m_VidWidth; i++)
      {
         RGBA=colorrange_ptr->getColor(static_cast<float>(m_image[i]));
//         cout << "i = " << i
//              << " r = " << RGBA.r() 
//              << " g = " << RGBA.g() 
//              << " b = " << RGBA.b()  << endl;

         m_color_image[3*i+0]=static_cast<unsigned char>(RGBA.r());
         m_color_image[3*i+1]=static_cast<unsigned char>(RGBA.g());
         m_color_image[3*i+2]=static_cast<unsigned char>(RGBA.b());
      }
      image_refptr->setImage( m_VidWidth, m_VidHeight, GLimageDepth,
                              GLinternalTextureFormat,GL_RGB,GLtype,
                              m_color_image, allocation_mode, GLpacking );
   }
   else
   {
      set_image();
   } // colormap_flag conditional
}

// ----------------------------------------------------------------
void texture_rectangle::set_image()
{
//   cout << "inside texture_rectangle::set_image()" << endl;

// On 4/13/09, Ross Anderson taught us that we don't need to
// instantiate member char* array m_image, for image_refptr already
// has data loaded into it when it's instantiated via a call to
// osgDB::readImageFile.  In this case, we don't need to execute this
// method either...

//   cout << "&m_image = " << &m_image << endl;
   if (m_image==NULL) 
   {
//      m_image=image_refptr->data();
//      cout << "m_image = NULL" << endl;
      return;
   }

   GLimageDepth=1;
   GLtype=GL_UNSIGNED_BYTE;
   GLpacking=1;

//   cout << "getNchannels() = " << getNchannels() << endl;
/*
   if (getNchannels()==1)
   {
      GLinternalTextureFormat=GL_LUMINANCE;
   }
   else if (getNchannels()==2)
   {
      GLinternalTextureFormat=GL_LUMINANCE_ALPHA;
   }
   else if (getNchannels()==3)
   {
      GLinternalTextureFormat=GL_RGB;
   }
   else if (getNchannels()==4)
   {
      GLinternalTextureFormat=GL_RGBA;
   }
*/

   image_refptr->setImage( m_VidWidth, m_VidHeight, GLimageDepth,
                           GLinternalTextureFormat,GLformat,GLtype,
                           m_image, allocation_mode, GLpacking );
}

// ========================================================================
// Colormap member functions
// ========================================================================

// Member function change_color_map

void texture_rectangle::change_color_map(int map_number)
{
   if (!colormap_flag)
   {
      colormap_flag=true;
      m_color_image = new unsigned char[3*image_size_in_bytes];

      vector<osg::Vec4> colors;
      if (ColorMap_ptr==NULL) ColorMap_ptr=new ColorMap();
      ColorMap_ptr->set_mapnumber(map_number);
      ColorMap_ptr->load(map_number,colors);

/*
  colors.push_back(osg::Vec4(0,0,128,1));
  colors.push_back(osg::Vec4(0,0,255,1));
  colors.push_back(osg::Vec4(0,255,255,1));
  //      colors.push_back(osg::Vec4(0,255,0,1));
  colors.push_back(osg::Vec4(255,255,0,1));
  colors.push_back(osg::Vec4(255,0,0,1));
  colors.push_back(osg::Vec4(128,0,0,1));
*/

      double min_intensity=0;
      double max_intensity=255;
      colorrange_ptr=new osgSim::ColorRange(
         min_intensity,max_intensity,colors);
    
   } // colormap_flag conditional
}

// ========================================================================
// Pixel coordinates member functions
// ========================================================================

// Member function get_pixel_coords takes in continuous image
// coordinates (u,v).  As of 8/8/05, we have empirically observed that
// v lies within the range [0,1], while u lies in [0,1.33846].  Recall
// that the Group 99 video camera has horizontal [vertical] pixel
// extents of 1392 [1040].  So we have empirically found that pv =
// v*1040 while pu = u*1040.  If the horizontal pixel extent on the
// G99 camera had been smaller than the vertical, we assume that we
// would have multiplied u and v by its value to compute pu and pv.
// So this is why we take the min of m_VidWidth and m_VidHeight
// below...

// Important note: In the 2 methods below, pv = 0 corresponds to the
// TOP (and not BOTTOM) of the image! 

pair<int,int> texture_rectangle::get_pixel_coords(double u,double v) const
{
   unsigned int pu,pv;
   get_pixel_coords(u,v,pu,pv);
   return pair<int,int>(pu,pv);
}

pair<double,double> texture_rectangle::get_uv_coords(
   unsigned int pu,unsigned int pv) const
{
   double u,v;
   get_uv_coords(pu,pv,u,v);
   return pair<double,double>(u,v);
}

// ----------------------------------------------------------------
void texture_rectangle::check_pixel_bounds(unsigned int& p) const
{
   if (p <Unsigned_Zero || p >= getWidth()*getHeight())
   {
      cout << "Error in texture_rectangle::check_pixel_bounds()!" << endl;
      cout << "p = " << p << " is >= getWidth()*getHeight() = "
           << getWidth()*getHeight() << endl;
      p=getWidth()*getHeight()-1;
   }
}

// ---------------------------------------------------------------------
// Member function RandomNeighborCoordinates() takes in pixel coords
// (px,py) and returns the coords for some random 8-neighbor.  This
// method performs brute-force checks to ensure the neighbor exists
// within 0 <= px_neighbor < width and 0 <= py_neighbor < height.  We
// wrote this method in Jan 2013 for ViBe background subtraction
// purposes.

void texture_rectangle::RandomNeighborCoordinates(
   unsigned int px,unsigned int py,
   unsigned int& px_neighbor,unsigned int& py_neighbor) const
{
//   cout << "inside texture_rectangle::RandomNeighborCoordinate(), px = " << px << " py = " << py << endl;

   int N;
   if (px==0 and py==0)
   {
      N=3;
      int n=mathfunc::getRandomInteger(N);
      if (n==0)
      {
         px_neighbor=px+1;
         py_neighbor=py;
      }
      else if (n==1)
      {
         px_neighbor=px;
         py_neighbor=py+1;
      }
      else if (n==2)
      {
         px_neighbor=px+1;
         py_neighbor=py+1;
      }
   }
   else if (px==0 && py==getHeight()-1)
   {
      N=3;
      int n=mathfunc::getRandomInteger(N);
      if (n==0)
      {
         px_neighbor=px;
         py_neighbor=py-1;
      }
      else if (n==1)
      {
         px_neighbor=px+1;
         py_neighbor=py-1;
      }
      else if (n==2)
      {
         px_neighbor=px+1;
         py_neighbor=py;
      }
   }
   else if (px==getWidth()-1 && py==0)
   {
      N=3;
      int n=mathfunc::getRandomInteger(N);
      if (n==0)
      {
         px_neighbor=px-1;
         py_neighbor=py;
      }
      else if (n==1)
      {
         px_neighbor=px-1;
         py_neighbor=py+1;
      }
      else if (n==2)
      {
         px_neighbor=px;
         py_neighbor=py+1;
      }
   }
   else if (px==getWidth()-1 && py==getHeight()-1)
   {
      N=3;
      int n=mathfunc::getRandomInteger(N);
      if (n==0)
      {
         px_neighbor=px-1;
         py_neighbor=py-1;
      }
      else if (n==1)
      {
         px_neighbor=px;
         py_neighbor=py-1;
      }
      else if (n==2)
      {
         px_neighbor=px-1;
         py_neighbor=py;
      }
   }
   else if (px==0)
   {
      N=5;
      int n=mathfunc::getRandomInteger(N);
      if (n==0)
      {
         px_neighbor=px-1;
         py_neighbor=py;
      }
      else if (n==1)
      {
         px_neighbor=px+1;
         py_neighbor=py;
      }
      else if (n==2)
      {
         px_neighbor=px-1;
         py_neighbor=py+1;
      }
      else if (n==3)
      {
         px_neighbor=px;
         py_neighbor=py+1;
      }
      else if (n==4)
      {
         px_neighbor=px+1;
         py_neighbor=py+1;
      }
   }
   else if (px==getWidth()-1)
   {
      N=5;
      int n=mathfunc::getRandomInteger(N);
      if (n==0)
      {
         px_neighbor=px-1;
         py_neighbor=py;
      }
      else if (n==1)
      {
         px_neighbor=px-1;
         py_neighbor=py-1;
      }
      else if (n==2)
      {
         px_neighbor=px;
         py_neighbor=py-1;
      }
      else if (n==3)
      {
         px_neighbor=px+1;
         py_neighbor=py-1;
      }
      else if (n==4)
      {
         px_neighbor=px+1;
         py_neighbor=py;
      }
   }
   else if (py==0)
   {
      N=5;
      int n=mathfunc::getRandomInteger(N);
      if (n==0)
      {
         px_neighbor=px;
         py_neighbor=py-1;
      }
      else if (n==1)
      {
         px_neighbor=px+1;
         py_neighbor=py-1;
      }
      else if (n==2)
      {
         px_neighbor=px+1;
         py_neighbor=py;
      }
      else if (n==3)
      {
         px_neighbor=px+1;
         py_neighbor=py+1;
      }
      else if (n==4)
      {
         px_neighbor=px;
         py_neighbor=py+1;
      }
   }
   else if (py==getHeight()-1)
   {
      N=5;
      int n=mathfunc::getRandomInteger(N);
      if (n==0)
      {
         px_neighbor=px-1;
         py_neighbor=py-1;
      }
      else if (n==1)
      {
         px_neighbor=px;
         py_neighbor=py-1;
      }
      else if (n==2)
      {
         px_neighbor=px-1;
         py_neighbor=py;
      }
      else if (n==3)
      {
         px_neighbor=px-1;
         py_neighbor=py+1;
      }
      else if (n==4)
      {
         px_neighbor=px;
         py_neighbor=py+1;
      }
   }
   else
   {
      int N=8;
      int n=mathfunc::getRandomInteger(N);
      if (n==0)
      {
         px_neighbor=px-1;
         py_neighbor=py-1;
      }
      else if (n==1)
      {
         px_neighbor=px;
         py_neighbor=py-1;
      }
      else if (n==2)
      {
         px_neighbor=px+1;
         py_neighbor=py-1;
      }
      else if (n==3)
      {
         px_neighbor=px-1;
         py_neighbor=py;
      }
      else if (n==4)
      {
         px_neighbor=px+1;
         py_neighbor=py;
      }
      else if (n==5)
      {
         px_neighbor=px-1;
         py_neighbor=py+1;
      }
      else if (n==6)
      {
         px_neighbor=px;
         py_neighbor=py+1;
      }
      else if (n==7)
      {
         px_neighbor=px+1;
         py_neighbor=py+1;
      }
   }
}

// ----------------------------------------------------------------
// Member function lexicographical_order() reduces an input (pu,pv)
// pixel coordinate to a single integer.  The output integer obeys the
// property (pu,pv) <= (qu,qv) iff pu < qu OR pu=qu AND pv <= qv.

int texture_rectangle::lexicographical_order(
   unsigned int pu,unsigned int pv) const
{
   return pu*getHeight()+pv;
}

// ========================================================================
// Pixel intensity get & set member functions
// ========================================================================

void texture_rectangle::set_pixel_intensity_value(
   unsigned int pu,unsigned int pv,int value)
{
//   if ( value > 0)
//   {
//      cout << "inside texture_rectangle::set_pixel_intensity_value()" << endl;
//      cout << "pu = " << pu << " pv = " << pv << " value = " << value << endl;
//   }
   
//   cout << "ptwoDarray_ptr = " << ptwoDarray_ptr << endl;
   ptwoDarray_ptr->put(pu,pv,value);
   set_pixel_RGB_values(pu,pv,value,value,value);
}

// Member function get_pixel_intensity() takes in pixel coordinates
// (pu,pv) for an image which is assumed to have been converted from
// RGB to greyscale.  It returns the pixel's greyscale intensity as an
// integer ranging from 0 to 255.

int texture_rectangle::get_pixel_intensity(
   unsigned int pu,unsigned int pv) const
{
//   int R,G,B;
//   get_pixel_RGB_values(pu,pv,R,G,B);
//   return R;

   return ptwoDarray_ptr->get(pu,pv);
}

// ------------------------------------------------------------------------
// Member function get_pixel_intensity_value() takes in integer pixel
// coordinates (px,py) along with a byte array which is assumed to
// correspond to a single-channel greyscale image.  It returns this
// pixel's intensity value.

int texture_rectangle::get_pixel_intensity_value(
   unsigned int pu,unsigned int pv) const
{
//   cout << "inside texture_rectangle::get_pixel_intensity_value(pu,pv)" 
//        << endl;
//   cout << "pu = " << pu << " pv = " << pv 
//        << " W = " << getWidth() << " H = " << getHeight()
//        << endl;
   
   if (pu >= 0 && pu < getWidth() && pv >= 0 && pv < getHeight())
   {
      return get_pixel_intensity_value(pu,pv,image_refptr->data());
   }
   else
   {
      return -1;
   }
}

int texture_rectangle::get_pixel_intensity_value(
   unsigned int px,unsigned int py,const unsigned char* data_ptr) const
{
//   cout << "inside texture_rectangle::get_pixel_intensity_value(px,py,data_ptr)" << endl;
//   cout << "px = " << px << " py = " << py << endl;

//   cout << "m_VidWidth = " << m_VidWidth 
//        << " m_VidHeight = " << m_VidHeight << endl;
//   cout << "data_ptr = " << data_ptr << endl;
   if (data_ptr==NULL) cout << "data_ptr=NULL!" << endl;
   
// See note at top of this file:

   unsigned int p=py*getWidth()+px;
   check_pixel_bounds(p);
   
//   cout << "nchannels = " << getNchannels() << endl;
   int i=getNchannels()*p;

   if (getNchannels() == 1 || getNchannels() == 2)
   {
      return stringfunc::unsigned_char_to_ascii_integer(data_ptr[i]);
   }

/*
  else if (getNchannels()==3)
  {

  }
*/
   else 
   {
      return -1;
   }
}

int texture_rectangle::fast_get_pixel_intensity_value(
   unsigned int px,unsigned int py) const
{
//   cout << "inside texture_rectangle::fast_get_pixel_intensity_value(px,py,data_ptr)" << endl;
//   cout << "px = " << px << " py = " << py << endl;
   return static_cast<int>(image_refptr->data()[py * m_VidWidth + px]);
}

// ------------------------------------------------------------------------
// Member function clear_all_intensities() resets all pixel values to
// zero.

void texture_rectangle::clear_all_intensities()
{
//   cout << "inside texture_rectangle::clear_all_intensities()" << endl;
 
   for (unsigned int pu=0; pu<getWidth(); pu++)
   {
      for (unsigned int pv=0; pv<getHeight(); pv++)
      {
         set_pixel_intensity_value(pu,pv,0);
      }
   }
}

// ------------------------------------------------------------------------
vector<double> texture_rectangle::get_pixel_region_intensity_values(
   unsigned int px_lo,unsigned int px_hi,
   unsigned int py_lo,unsigned int py_hi,const unsigned char* data_ptr)
{
//   cout << "inside texture_rectangle::get_pixel_region_intensity_values()" << endl;

   vector<double> intensity_values;
   intensity_values.reserve( (py_hi-py_lo+1)*(px_hi-px_lo+1) );

   unsigned int n_channels=3;		// We assume m_Nchannels == 3
   double h,s,v;
   for (unsigned int py=py_lo; py <= py_hi; py++)
   {
      for (unsigned int px=px_lo; px <= px_hi; px++)
      {
         unsigned int i=n_channels*(py*m_VidWidth+px);
         double R=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+0]);
         double G=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1]);
         double B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+2]);
         double r=R/255.0;
         double g=G/255.0;
         double b=B/255.0;
         colorfunc::RGB_to_hsv(r,g,b,h,s,v);
         intensity_values.push_back(v);
      } // loop over py
   } // loop over px

   return intensity_values;
}

// ------------------------------------------------------------------------
void texture_rectangle::get_pixel_region_intensity_moments(
   int px_lo,int px_hi,int py_lo,int py_hi,
   double& mu_intensity,double& sigma_intensity)
{
   vector<double> intensity_values=get_pixel_region_intensity_values(
      px_lo,px_hi,py_lo,py_hi,image_refptr->data());
   mathfunc::mean_and_std_dev(intensity_values,mu_intensity,sigma_intensity);
}

// ========================================================================
// RGB get & set member functions
// ========================================================================

// Member function get_pixel_RGB_values coords takes in continuous
// image coordinates (u,v) for some pixel.  If the input pixel's
// coordinates are valid, it returns the pixel's RGB values in the
// range 0 to 255.  Otherwise, this method returns sentinel negative
// values for R, G and B:

void texture_rectangle::get_pixel_RGB_values(
   unsigned int pu,unsigned int pv,int& R,int& G,int& B) const
{
//   cout << "inside texture_rectangle::get_pixel_RGB_values(pu,pv,R,G,B)" 
//        << endl;
//   cout << "pu = " << pu << " pv = " << pv << endl;
//   cout << "m_VidWidth = " << m_VidWidth 
//        << " m_VidHeight = " << m_VidHeight << endl;

   R=G=B=-1; // missing data values
   if (pu >= 0 && pu < getWidth() && pv >= 0 && pv < getHeight())
   {
      get_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);
   }
}

// ---------------------------------------------------------------------
// Member function fast_get_pixel_RGB_values coords takes in (pu,pv)
// coordinates for some pixel.  The number of color channels is
// assumed to equal 3.  It returns the pixel's RGB values
// in the range 0 to 255 as fast as possible without performing any
// safety checks.

void texture_rectangle::fast_get_pixel_RGB_values(
   unsigned int pu,unsigned int pv,int& R,int& G,int& B) const
{
   int i=3*(pv*m_VidWidth+pu);
   R=image_refptr->data()[i];
   G=image_refptr->data()[i+1];
   B=image_refptr->data()[i+2];
}

// ---------------------------------------------------------------------
void texture_rectangle::get_pixel_row_RGB_values(
   unsigned int pu,unsigned int pv,unsigned int n_pixels_in_rows,
   int* R, int* G, int* B) const
{
   int i=3*(pv*m_VidWidth+pu);
   for (unsigned int n=0; n<n_pixels_in_rows; n++)
   {
      R[n]=image_refptr->data()[i];
      G[n]=image_refptr->data()[i+1];
      B[n]=image_refptr->data()[i+2];
      i += 3;
   }
}

// ----------------------------------------------------------------
Triple<int,int,int> texture_rectangle::get_RGB_values(double u,double v) const
{
   int R,G,B;
   get_RGB_values(u,v,R,G,B);
   return Triple<int,int,int>(R,G,B);
}

void texture_rectangle::get_RGB_values(
   double u,double v,int& R,int& G,int& B) const
{
//   cout << "inside texture_rectangle::get_RGB_values()" << endl;
//   cout << "u = " << u << " v = " << v << endl;
//   cout << "minU = " << min_U << " maxU = " << max_U
//        << " minV = " << min_V << " maxV = " << max_V << endl;

//   R=G=B=-1; // missing data values

   if (u >= min_U && u <= max_U && v >= min_V && v <= max_V)
   {
      std::pair<int,int> p=get_pixel_coords(u,v);
      unsigned int pu=p.first;
      unsigned int pv=p.second;
//      cout << "pu = " << pu << " pv = " << pv << endl;
//      get_pixel_RGB_values(pu,pv,get_m_image_ptr(),R,G,B);
      get_pixel_RGB_values(pu,pv,R,G,B);
//      cout << "R = " << R << " G = " << G << " B = " << B << endl;
   }
}

// ----------------------------------------------------------------
bool texture_rectangle::get_pixel_hsv_values(
   unsigned int pu,unsigned int pv,double& h,double& s,double& v) const
{
   int R,G,B;
   get_pixel_RGB_values(pu,pv,R,G,B);
   double r=R/255.0;
   double g=G/255.0;
   double b=B/255.0;
   colorfunc::RGB_to_hsv(r,g,b,h,s,v);

// Recall that hue=NEGATIVEINFINITY whenever s==0.  In this case, this
// boolean method returns false as an indication that hue is
// ill-defined:

   if (h < 0 || h > 360)
   {
      return false;
   }
   return true;
}

bool texture_rectangle::get_pixel_hsva_values(
   unsigned int pu,unsigned int pv,double& h,double& s,double& v, double& a) 
const
{
   int R,G,B,A;
   get_pixel_RGBA_values(pu,pv,R,G,B,A);
   double r=R/255.0;
   double g=G/255.0;
   double b=B/255.0;
   colorfunc::RGB_to_hsv(r,g,b,h,s,v);
   a=A/255.0;

// Recall that hue=NEGATIVEINFINITY whenever s==0.  In this case, this
// boolean method returns false as an indication that hue is
// ill-defined:

   if (h < 0 || h > 360)
   {
      return false;
   }
   return true;
}

void texture_rectangle::get_hsv_values(
   double U,double V,double& h,double& s,double& v) const
{
   unsigned int pu,pv;
   get_pixel_coords(U,V,pu,pv);
   get_pixel_hsv_values(pu,pv,h,s,v);
}

void texture_rectangle::get_pixel_RGBhsv_values(
   unsigned int pu,unsigned int pv,
   int& R,int& G,int& B,double& h,double& s,double& v) const
{
   get_pixel_RGB_values(pu,pv,R,G,B);
   double r=R/255.0;
   double g=G/255.0;
   double b=B/255.0;
   colorfunc::RGB_to_hsv(r,g,b,h,s,v);
}

// ----------------------------------------------------------------
// Member function set_pixel_RGB_values alters the RGB values for the
// pixel labeled by input coordinates pu and pv.

void texture_rectangle::set_pixel_RGB_values(
   unsigned int pu,unsigned int pv,int R,int G,int B)
{
//   cout << "inside texture_rectangle::set_pixel_RGB_values() " << endl;
   
   if (pu >= 0 && pu < getWidth() && pv >= 0 && pv < getHeight())
   {
      set_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);
//      int rnew,gnew,bnew;
//      get_pixel_RGB_values(pu,pv,rnew,gnew,bnew);
//      cout << "rnew = " << rnew << " gnew = " << gnew << " bnew = " << bnew
//           << endl;
   }
   image_refptr->dirty();
}

// ----------------------------------------------------------------
// Member function set_pixel_hue alters the hue for the
// pixel labeled by input coordinates pu and pv.

void texture_rectangle::set_pixel_hue(
   unsigned int pu,unsigned int pv,double hue)
{
//   cout << "inside texture_rectangle::set_pixel_hue() " << endl;
   
   if (pu >= 0 && pu < getWidth() && pv >= 0 && pv < getHeight())
   {
      double h,s,v;
      get_pixel_hsv_values(pu,pv,h,s,v);

      double r,g,b;
      colorfunc::hsv_to_RGB(hue,s,v,r,g,b);
      int R=255*r;
      int G=255*g;
      int B=255*b;
      set_pixel_RGB_values(pu,pv,R,G,B);
   }
}

// ----------------------------------------------------------------
// Member function set_pixel_hsv_values alters the hsv values for the
// pixel labeled by input coordinates pu and pv.

void texture_rectangle::set_pixel_hsv_values(
   unsigned int pu,unsigned int pv,double h,double s,double v)
{
//   cout << "inside texture_rectangle::set_pixel_hsv_values() " << endl;
   
   if (pu >= 0 && pu < getWidth() && pv >= 0 && pv < getHeight())
   {
      double r,g,b;
      colorfunc::hsv_to_RGB(h,s,v,r,g,b);
      int R=255*r;
      int G=255*g;
      int B=255*b;
      set_pixel_RGB_values(pu,pv,R,G,B);
   }
}

// ----------------------------------------------------------------
void texture_rectangle::set_pixel_hsva_values(
   unsigned int pu,unsigned int pv,double h,double s,double v,double a)
{
//   cout << "inside texture_rectangle::set_pixel_hsva_values() " << endl;
   
   if (pu >= 0 && pu < getWidth() && pv >= 0 && pv < getHeight())
   {
      double r,g,b;
      colorfunc::hsv_to_RGB(h,s,v,r,g,b);
      int R=255*r;
      int G=255*g;
      int B=255*b;
      int A=255*a;
      set_pixel_RGBA_values(pu,pv,R,G,B,A);
   }
}

// ----------------------------------------------------------------
void texture_rectangle::set_RGB_values(double u,double v,int R,int G,int B)
{
   unsigned int pu,pv;
   get_pixel_coords(u,v,pu,pv);
   set_pixel_RGB_values(pu,pv,R,G,B);
}

// Member function check_pixel_bounds makes sure input byte index p
// lies between 0 and width*height. 

// ------------------------------------------------------------------------
// Member function set_pixel_RGB_values takes in integer pixel
// coordinates (px,py) and integer RGB values.  It then resets the
// appropriate bytes within the input unsigned char* array.

void texture_rectangle::set_pixel_RGB_values(
   unsigned int px,unsigned int py,unsigned char* data_ptr,int R,int G,int B)
{
//   cout << "inside texture_rectangle::set_pixel_RGB_values(), R = " << R
//        << " G = " << G << " B = " << B << endl;
//   cout << "data_ptr = " << data_ptr << endl;
//   cout << "Nchannels = " << getNchannels() << endl;

   int n_channels = getNchannels();
   if(n_channels == 4)
   {
      int A = 255;
      set_pixel_RGBA_values(px,py,data_ptr,R,G,B,A);
      return;
   }
   
// See note at top of this file:

   unsigned int p=py*getWidth()+px;
//   check_pixel_bounds(p);

   int i=n_channels * p;
   if (n_channels == 1)
   {
      data_ptr[i+0]=stringfunc::ascii_integer_to_unsigned_char(R);
   }
   else if (n_channels == 2)
   {
      int A = 255;
      data_ptr[i+0]=stringfunc::ascii_integer_to_unsigned_char(R);
      data_ptr[i+1]=stringfunc::ascii_integer_to_unsigned_char(A);
   }
   else if (n_channels == 3)
   {
      data_ptr[i+0]=stringfunc::ascii_integer_to_unsigned_char(R);
      data_ptr[i+1]=stringfunc::ascii_integer_to_unsigned_char(G);
      data_ptr[i+2]=stringfunc::ascii_integer_to_unsigned_char(B);
   }
   else
   {
      cout << "Error in texture_rectangle::set_pixel_RGB_values()" << endl;
      cout << "getNchannels() = " << n_channels << endl;
      exit(-1);
   }
}

void texture_rectangle::set_pixel_RGBA_values(
   unsigned int px,unsigned int py,unsigned char* data_ptr,
   int R,int G,int B,int A)
{
//   cout << "inside texture_rectangle ::set_pixel_RGBA_values(), R = " << R
//        << " G = " << G << " B = " << B << " A = " << A << endl;
//   cout << "px = " << px << " py = " << py << endl;
//   cout << "&m_image = " << &m_image << endl;

// See note at top of this file:

   unsigned int p=py*getWidth()+px;
//   check_pixel_bounds(p);
   
   int n_channels = getNchannels();
   if(n_channels != 4)
   {
      cout << "Error in texture_rectangle::set_pixel_RGBA_values()"
           << " n_channels = " << n_channels << endl;
      exit(-1);
   }
   
   int i=n_channels * p;
   data_ptr[i+0]=stringfunc::ascii_integer_to_unsigned_char(R);
   data_ptr[i+1]=stringfunc::ascii_integer_to_unsigned_char(G);
   data_ptr[i+2]=stringfunc::ascii_integer_to_unsigned_char(B);
   data_ptr[i+3]=stringfunc::ascii_integer_to_unsigned_char(A);
}

// ------------------------------------------------------------------------
// Member function get_pixel_RGB_values() takes in integer pixel coordinates
// (px,py) along with a byte array which is assumed to correspond to a
// 3-channel image.  It returns this pixel's RGB values.

void texture_rectangle::get_pixel_RGB_values(
   unsigned int px,unsigned int py,const unsigned char* data_ptr,
   int& R,int& G,int& B) const
{
//   cout << "inside texture_rectangle::get_pixel_RGB_values(px,py,data_ptr,R,G,B)" << endl;
//   cout << "px = " << px << " py = " << py << endl;

//   cout << "m_VidWidth = " << m_VidWidth 
//        << " m_VidHeight = " << m_VidHeight << endl;
//   cout << "data_ptr = " << data_ptr << endl;
   if (data_ptr==NULL) cout << "data_ptr=NULL!" << endl;
   
// See note at top of this file:

   unsigned int p=py*getWidth()+px;
   check_pixel_bounds(p);
   
   int n_channels = getNchannels();
//   cout << "nchannels = " << getNchannels() << endl;
   int i=n_channels * p;

   if (n_channels == 1 || n_channels == 2)
   {
      R=G=B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i]);
   }
   else if (n_channels == 3)
   {
      R=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+0]);
      G=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1]);
      B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+2]);
   }
}

// ------------------------------------------------------------------------
// Member function get_pixel_RGBA_values() takes in integer pixel coordinates
// (px,py) along with a byte array which is assumed to correspond to a
// 4-channel image.  It returns this pixel's RGBA values.

void texture_rectangle::get_pixel_RGBA_values(
   unsigned int px,unsigned int py,const unsigned char* data_ptr,
   int& R,int& G,int& B,int& A) const
{
//   cout << "inside texture_rectangle::get_pixel_RGBA_values()" << endl;

// See note at top of this file:

   unsigned int p=py*getWidth()+px;
//   check_pixel_bounds(p);
   
   int n_channels = getNchannels();
   int i = n_channels * p;

   if (n_channels == 1)
   {
      R=G=B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i]);
      A=255;
   }
   else if (n_channels == 2)
   {
      R=G=B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i]);
      A=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1]);
   }
   else if (n_channels == 3)
   {
      R=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+0]);
      G=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1]);
      B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+2]);
      A=255;
   }
   else if (n_channels == 4)
   {
      R=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+0]);
      G=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1]);
      B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+2]);
      A=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+3]);
//      cout << "R = " << R << " G = " << G << " B = " <<  B
//           << " A = " << A << endl;
   }
}

// ------------------------------------------------------------------------
Triple<int,int,int> texture_rectangle::get_pixel_RGB_values(
   unsigned int px,unsigned int py,unsigned char* data_ptr) const
{
   int R,G,B;
   get_pixel_RGB_values(px,py,data_ptr,R,G,B);
   return Triple<int,int,int>(R,G,B);
}

Quadruple<int,int,int,int> texture_rectangle::get_pixel_RGBA_values(
   unsigned int px,unsigned int py,unsigned char* data_ptr) const
{
   int R,G,B,A;
   get_pixel_RGBA_values(px,py,data_ptr,R,G,B,A);
   return Quadruple<int,int,int,int>(R,G,B,A);
}

// ----------------------------------------------------------------
void texture_rectangle::set_pixel_RGBA_values(
   unsigned int pu,unsigned int pv,int R,int G,int B,int A)
{
//   cout << "inside texture_rectangle::set_pixel_RGBA_values() " << endl;
   
   if (pu >= 0 && pu < getWidth() && pv >= 0 && pv < getHeight())
   {
      set_pixel_RGBA_values(pu,pv,image_refptr->data(),R,G,B,A);
//      int rnew,gnew,bnew,anew;
//      get_pixel_RGBA_values(pu,pv,rnew,gnew,bnew,anew);
//      cout << "rnew = " << rnew << " gnew = " << gnew << " bnew = " << bnew
//           << endl;
   }
}

void texture_rectangle::get_pixel_RGBA_values(
   unsigned int pu,unsigned int pv,int& R,int& G,int& B,int& A) const
{
//   cout << "inside texture_rectangle::get_pixel_RGBA_values(pu,pv,R,G,B,A)" << endl;
//   cout << "pu = " << pu << " pv = " << pv << endl;
//   cout << "m_VidWidth = " << m_VidWidth 
//        << " m_VidHeight = " << m_VidHeight << endl;
   
   R=G=B=A=-1; // missing data values
   if (pu >= 0 && pu < getWidth() && pv >= 0 && pv < getHeight())
   {
      get_pixel_RGBA_values(pu,pv,image_refptr->data(),R,G,B,A);
   }
}

// ------------------------------------------------------------------------
// Member function get_all_RGB_values() fills STL vectors with RGB
// values as efficiently as possible.

void texture_rectangle::get_all_RGB_values(
   vector<double>& red_values,vector<double>& green_values,
   vector<double>& blue_values)
{
//   cout << "inside texture_rectangle::get_all_RGB_values()" << endl;

   get_all_RGB_values(
      image_refptr->data(),red_values,green_values,blue_values);
}

void texture_rectangle::get_all_RGB_values(
   const unsigned char* data_ptr,
   vector<double>& red_values,vector<double>& green_values,
   vector<double>& blue_values)
{
//   cout << "inside texture_rectangle::get_all_RGB_values()" << endl;

   if (getNchannels() != 3)
   {
      cout << "Error in texture_rectangle::get_all_RGB_values()" << endl;
      cout << "n_channels = " << getNchannels() << endl;
      exit(-1);
   }

   if (data_ptr==NULL)
   {
      cout << "Error in texture_rectangle::get_all_RGB_values()" << endl;
      cout << "data_ptr=NULL!" << endl;
      return;
   }

   unsigned int n_pixels=getWidth()*getHeight();
//   cout << "n_pixels = " << n_pixels << endl;

   red_values.reserve(n_pixels);
   green_values.reserve(n_pixels);
   blue_values.reserve(n_pixels);
   
   for (unsigned int p=0; p<n_pixels; p++)
   {
      red_values.push_back(static_cast<double>(data_ptr[3*p+0]));
      green_values.push_back(static_cast<double>(data_ptr[3*p+1]));
      blue_values.push_back(static_cast<double>(data_ptr[3*p+2]));
   } // loop over index p labeling pixels
}

// ---------------------------------------------------------------------
// Member function copy_RGB_values() transfers the contents of
// texture_rectangle_ptr->get_m_image_ref_ptr() to
// this->get_m_image_ref_ptr() as efficiently as possible.

void texture_rectangle::copy_RGB_values(
   texture_rectangle* texture_rectangle_ptr)
{
//   cout << "inside texture_rectangle::copy_RGB_values()" << endl;

   unsigned char* this_data_ptr=get_m_image_ptr();
   unsigned char* tr_data_ptr=texture_rectangle_ptr->get_m_image_ptr();

   int n_pixels=getWidth()*getHeight();
   memcpy(this_data_ptr,tr_data_ptr,3*n_pixels);
}

// ========================================================================
// Pixel region RGB member functions
// ========================================================================

// Member function get_pixel_region_RGB_values() takes in center
// pixel coordinates (px_c,py_c), the size of a square bbox side measured
// in pixels, and a byte array which is assumed to correspond to a
// 3-channel image.  It returns the pixel region's red, green and blue
// values within output STL vectors R, G and B.

void texture_rectangle::get_pixel_region_RGB_values(
   unsigned int px_c,unsigned int py_c,int n_size)
{
   get_pixel_region_RGB_values(px_c,py_c,n_size,image_refptr->data());
}

void texture_rectangle::get_pixel_region_RGB_values(
   unsigned int px_c,unsigned int py_c,int n_size,
   const unsigned char* data_ptr)
{
//   cout << "inside texture_rectangle::get_pixel_region_RGB_values()" << endl;

   unsigned int px_lo=basic_math::max(Unsigned_Zero,px_c-n_size/2);
   unsigned int px_hi=basic_math::min(m_VidWidth-1,px_c+n_size/2);
   unsigned int py_lo=basic_math::max(Unsigned_Zero,py_c-n_size/2);
   unsigned int py_hi=basic_math::min(m_VidHeight-1,py_c+n_size/2);
   get_pixel_region_RGB_values(px_lo,px_hi,py_lo,py_hi,data_ptr);
}

void texture_rectangle::get_pixel_region_RGB_values(
   unsigned int px_lo,unsigned int px_hi,
   unsigned int py_lo,unsigned int py_hi,
   const unsigned char* data_ptr)
{
//   cout << "inside texture_rectangle::get_pixel_region_RGB_values()" << endl;

   region_R_values.clear();
   region_G_values.clear();
   region_B_values.clear();

   unsigned int n_channels=3;		// We assume m_Nchannels == 3

   for (unsigned int py=py_lo; py <= py_hi; py++)
   {
      for (unsigned int px=px_lo; px <= px_hi; px++)
      {
         unsigned int i=n_channels*(py*m_VidWidth+px);
         region_R_values.push_back(
            stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+0]));
         region_G_values.push_back(
            stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1]));
         region_B_values.push_back(
            stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+2]));
      } // loop over py
   } // loop over px
}

// ------------------------------------------------------------------------
// Member function get_pixel_region_RGB_moments() checks if the mean
// and standard deviations {median and quartile widths} for RGB color
// distributions extracted from pixels within a bbox around (px,py)
// were previously calculated.  If not, it computes these RGB moments
// and stores the results within the input twoDarrays.

void texture_rectangle::get_pixel_region_RGB_moments(
   int px,int py,int n_size,
   twoDarray* mu_R_twoDarray_ptr,
   twoDarray* mu_G_twoDarray_ptr,
   twoDarray* mu_B_twoDarray_ptr,
   twoDarray* sigma_R_twoDarray_ptr,
   twoDarray* sigma_G_twoDarray_ptr,
   twoDarray* sigma_B_twoDarray_ptr,
   double& mu_R,double& mu_G,double& mu_B,
   double& sigma_R,double& sigma_G,double& sigma_B) 
{
   double prev_mu_R=mu_R_twoDarray_ptr->get(px,py);
   if (prev_mu_R > 0.5*NEGATIVEINFINITY)
   {
      mu_R=prev_mu_R;
      mu_G=mu_G_twoDarray_ptr->get(px,py);
      mu_B=mu_B_twoDarray_ptr->get(px,py);
      sigma_R=sigma_R_twoDarray_ptr->get(px,py);
      sigma_G=sigma_G_twoDarray_ptr->get(px,py);
      sigma_B=sigma_B_twoDarray_ptr->get(px,py);
      return;
   }
   
   get_pixel_region_RGB_values(px,py,n_size);
   double n_pixels_frac=region_R_values.size()/double(sqr(n_size));
   if (n_pixels_frac < 0.25)
   {
      cout << "inside texture_rectangle::get_pixel_region_RGB_moments()" 
           << endl;
      cout << "px = " << px << " py = " << py << endl;
      cout << "region_R_values.size() = " << region_R_values.size()
           << endl;
      cout << "width = " << getWidth() 
           << " height = " << getHeight() << endl;
      cout << "n_size = " << n_size << endl;
      cout << "n_pixels_frac = " << n_pixels_frac << endl;
      outputfunc::enter_continue_char();
   }

// As of 3/6/14, we see no obvious advantage to working with median
// and quartile widths rather than means and standard deviations.  And
// the latter are faster to compute than the former...

   mathfunc::mean_and_std_dev(region_R_values,mu_R,sigma_R);
   mathfunc::mean_and_std_dev(region_G_values,mu_G,sigma_G);
   mathfunc::mean_and_std_dev(region_B_values,mu_B,sigma_B);
   
//   mathfunc::median_value_and_quartile_width(region_R_values,mu_R,sigma_R);
//   mathfunc::median_value_and_quartile_width(region_G_values,mu_G,sigma_G);
//   mathfunc::median_value_and_quartile_width(region_B_values,mu_B,sigma_B);

   mu_R_twoDarray_ptr->put(px,py,mu_R);
   mu_G_twoDarray_ptr->put(px,py,mu_G);
   mu_B_twoDarray_ptr->put(px,py,mu_B);
   sigma_R_twoDarray_ptr->put(px,py,sigma_R);
   sigma_G_twoDarray_ptr->put(px,py,sigma_G);
   sigma_B_twoDarray_ptr->put(px,py,sigma_B);
}

// This overloaded version of get_pixel_region_RGB_moments()
// retrieves RGB values within the bbox specified by px_lo, px_hi,
// py_lo and py_hi.  It then computes and returns the mean and
// standard deviations for the pixels' RGB values.

void texture_rectangle::get_pixel_region_RGB_moments(
   int px_lo,int px_hi,int py_lo,int py_hi,
   double& mu_R,double& mu_G,double& mu_B,
   double& sigma_R,double& sigma_G,double& sigma_B) 
{
   get_pixel_region_RGB_values(px_lo,px_hi,py_lo,py_hi,image_refptr->data());
   
// As of 3/6/14, we see no obvious advantage to working with median
// and quartile widths rather than means and standard deviations.  And
// the latter are faster to compute than the former...

//   mathfunc::mean_and_std_dev(region_R_values,mu_R,sigma_R);
//   mathfunc::mean_and_std_dev(region_G_values,mu_G,sigma_G);
//   mathfunc::mean_and_std_dev(region_B_values,mu_B,sigma_B);
   
   mathfunc::median_value_and_quartile_width(region_R_values,mu_R,sigma_R);
   mathfunc::median_value_and_quartile_width(region_G_values,mu_G,sigma_G);
   mathfunc::median_value_and_quartile_width(region_B_values,mu_B,sigma_B);
}


// ========================================================================
// Drawing member functions
// ========================================================================

void texture_rectangle::fill_circle(
   int pu,int pv,double radius,colorfunc::Color c)
{
   int R,G,B;
   R=G=B=0;
   colorfunc::RGB rgb=colorfunc::get_RGB_values(c);
   colorfunc::rgb_to_RGB(rgb.first,rgb.second,rgb.third,R,G,B);

   for (int px=pu-radius; px<=pu+radius; px++)
   {
      for (int py=pv-radius; py<=pv+radius; py++)
      {
         double rsq=sqr(px-pu)+sqr(py-pv);
         if (rsq > radius*radius) continue;
         set_pixel_RGB_values(px,py,R,G,B);
      } // loop over py index
   } // loop over px index
}

void texture_rectangle::draw_pixel_bbox(
   const bounding_box& bbox,colorfunc::Color c)
{
   unsigned int px_min=bbox.get_xmin();
   unsigned int px_max=bbox.get_xmax();
   unsigned int py_min=bbox.get_ymin();
   unsigned int py_max=bbox.get_ymax();
   
   draw_pixel_bbox(px_min,px_max,py_min,py_max,c);
}

void texture_rectangle::draw_pixel_bbox(
   const bounding_box& bbox,int R, int G, int B)
{
   unsigned int px_min=bbox.get_xmin();
   unsigned int px_max=bbox.get_xmax();
   unsigned int py_min=bbox.get_ymin();
   unsigned int py_max=bbox.get_ymax();
   
   draw_pixel_bbox(px_min,px_max,py_min,py_max,R,G,B);
}

void texture_rectangle::draw_pixel_bbox(
   unsigned int px_min,unsigned int px_max,
   unsigned int py_min,unsigned int py_max,
   colorfunc::Color c)
{
   int R,G,B;
   R=G=B=0;
   colorfunc::RGB rgb=colorfunc::get_RGB_values(c);
   colorfunc::rgb_to_RGB(rgb.first,rgb.second,rgb.third,R,G,B);
   draw_pixel_bbox(px_min,px_max,py_min,py_max,R,G,B);
}

void texture_rectangle::draw_pixel_bbox(
   unsigned int px_min,unsigned int px_max,
   unsigned int py_min,unsigned int py_max,
   int R,int G,int B)
{
   for (unsigned int px=px_min; px<=px_max; px++)
   {
      set_pixel_RGB_values(px,py_min,R,G,B);
      set_pixel_RGB_values(px,py_max,R,G,B);
   }
   
   for (unsigned int py=py_min; py<=py_max; py++)
   {
      set_pixel_RGB_values(px_min,py,R,G,B);
      set_pixel_RGB_values(px_max,py,R,G,B);
   }
}

void texture_rectangle::fill_pixel_bbox(
   const bounding_box& bbox,int R, int G, int B)
{
   unsigned int px_min=bbox.get_xmin();
   unsigned int px_max=bbox.get_xmax();
   unsigned int py_min=bbox.get_ymin();
   unsigned int py_max=bbox.get_ymax();
   
   fill_pixel_bbox(px_min,px_max,py_min,py_max,R,G,B);
}

void texture_rectangle::fill_pixel_bbox(
   unsigned int px_min,unsigned int px_max,
   unsigned int py_min,unsigned int py_max,
   int R,int G,int B)
{
   for(unsigned int py = py_min; py <= py_max; py++)
   {
      if(py < 0 || py >= getHeight()) continue;
      for (unsigned int px=px_min; px<=px_max; px++)
      {
         if(px < 0 || px >= getWidth()) continue;
         set_pixel_RGB_values(px,py,R,G,B);
      }
   }
}

// ========================================================================
// RGB color to greyscale conversion member functions
// ========================================================================

void texture_rectangle::reset_all_RGB_values(int R,int G,int B)
{
   for (unsigned int pu=0; pu<getWidth(); pu++)
   {
      for (unsigned int pv=0; pv<getHeight(); pv++)
      {
         set_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);
      }
   }
}

void texture_rectangle::reset_all_RGBA_values(int R, int G, int B, int A)
{
   if(getNchannels() != 4)
   {
      cout << "Error in texture_rectangle::reset_all_RGBA_values()"
           << endl;
      cout << "n_channels = " << getNchannels() << endl;
      exit(-1);
   }
   
   for (unsigned int pu=0; pu<getWidth(); pu++)
   {
      for (unsigned int pv=0; pv<getHeight(); pv++)
      {
         set_pixel_RGBA_values(pu,pv,image_refptr->data(),R,G,B,A);
      }
   }
}

// Member function clear_all_RGB_values() resets all pixel values to
// zero.

void texture_rectangle::clear_all_RGB_values()
{
   reset_all_RGB_values(0,0,0);
}

void texture_rectangle::clear_all_RGBA_values()
{
   reset_all_RGBA_values(0,0,0,0);
}

// ----------------------------------------------------------------
// Member function convert_color_image_to_greyscale loops over all
// pixels within the current texture rectangle.  For each pixel, this
// method first computes the h,s,v color coordinates corresponding to
// the pixel's RGB coordinates.  It then resets the pixel's color to
// just the greyscale value corresponding to v.  We wrote this method
// in Jan 2009 in order to transform colored 3D panoramic mosaics into
// black-and-white backdrops for dynamic, colored 3D videos.

void texture_rectangle::convert_color_image_to_greyscale()
{
   convert_color_image_to_h_s_or_v(2);
}

void texture_rectangle::convert_color_image_to_h_s_or_v(int color_channel)
{
//   cout << "inside texture_rectangle::convert_color_image_to_h_s_or_v()" 
//	  << endl;

   instantiate_ptwoDarray_ptr();

   unsigned int n_channels=getNchannels();
//   cout << "n_channels = " << n_channels << endl;
   
   int R,G,B,A;
   R=G=B=A=0;
   double h,s,v;
   for (unsigned int pu=0; pu<getWidth(); pu++)
   {
      for (unsigned int pv=0; pv<getHeight(); pv++)
      {
         if (n_channels==4)
         {
            get_pixel_RGBA_values(pu,pv,image_refptr->data(),R,G,B,A);
         }
         else
         {
            get_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);
            A=-255;
         }

         double r=R/255.0;
         double g=G/255.0;
         double b=B/255.0;
         double a=A/255.0;
//         cout << "r = " << r << " g = " << g << " b = " << b
//              << " a = " << a << endl;

         if (n_channels==4 && nearly_equal(a,0))
         {
            R=G=B=A=0;
//            R=G=B=A=255;
         }
         else
         {
            colorfunc::RGB_to_hsv(r,g,b,h,s,v);
            if (color_channel==0)
            {
               R=G=B=255*h/360.0;
            }
            else if (color_channel==1)
            {
               R=G=B=255*s;
            }
            else if (color_channel==2)
            {
               R=G=B=255*v;
            }
         }
         if (n_channels==4)
         {
            set_pixel_RGBA_values(pu,pv,image_refptr->data(),R,G,B,A);
         }
         else
         {
            set_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);
         }
         ptwoDarray_ptr->put(pu,pv,R);
      }
   }

   image_refptr->dirty();
}

// ----------------------------------------------------------------
void texture_rectangle::convert_color_image_to_single_color_channel(
   int c,bool generate_greyscale_image_flag)
{
//   cout << "inside texture_rectangle::convert_color_image_to_single_color_channel()" 
//	  << endl;

   instantiate_ptwoDarray_ptr();
 
   int R,G,B;
   R=G=B=0;
   for (unsigned int pu=0; pu<getWidth(); pu++)
   {
      for (unsigned int pv=0; pv<getHeight(); pv++)
      {
         get_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);
         if (c==1)
         {
            G=B=0;
            if (generate_greyscale_image_flag) 
            {
               G=R;
               B=R;
            }
         }
         else if (c==2)
         {
            R=B=0;
            if (generate_greyscale_image_flag) 
            {
               R=G;
               B=G;
            }
         }
         else if (c==3)
         {
            R=G=0;
            if (generate_greyscale_image_flag) 
            {
               R=B;
               G=B;
            }
         }
         set_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);
         ptwoDarray_ptr->put(pu,pv,R);
      }
   }
   image_refptr->dirty();
}

// ----------------------------------------------------------------
// Member function convert_color_image_to_luminosity() forms and
// returns a particular linear combination of the R,G,B values
// corresponding to "luminosity" for each pixel 

void texture_rectangle::convert_color_image_to_luminosity()
{
//   cout << "inside texture_rectangle::convert_color_image_to_luminosity()" 
//	  << endl;

   instantiate_ptwoDarray_ptr();

   unsigned int n_channels=getNchannels();
//   cout << "n_channels = " << n_channels << endl;
   
   int R,G,B,A;
   R=G=B=A=0;
   for (unsigned int pu=0; pu<getWidth(); pu++)
   {
      for (unsigned int pv=0; pv<getHeight(); pv++)
      {
         if (n_channels==4)
         {
            get_pixel_RGBA_values(pu,pv,image_refptr->data(),R,G,B,A);
         }
         else
         {
            get_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);
            A=-255;
         }

         if (n_channels==4 && A==0)
         {
            R=G=B=A=0;
         }
         else
         {
            double Luminosity=colorfunc::RGB_to_luminosity(R,G,B);
            if (Luminosity > 255) Luminosity=255;
            R=G=B=Luminosity;
         }

         if (n_channels==4)
         {
            set_pixel_RGBA_values(pu,pv,image_refptr->data(),R,G,B,A);
         }
         else
         {
            set_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);
         }
         ptwoDarray_ptr->put(pu,pv,R);
      }
   }

   image_refptr->dirty();
}


// ========================================================================
// Greyscale to RGB color conversion member functions
// ========================================================================

// Member function convert_grey_values_to_hues() reads in 
// RGBAvalues which are currently stored within the
// texture_rectangle's pixels.  This method assumes n_channels=3 and
// that initially R=G=B for every pixel.  If R=G=B=0, the pixel's
// color is left unchanged at black.  Otherwise, this method resets
// the pixels' hues equal to their intensity values corresponding to
// their (R,G,B) coordinates spread across the interval
// [hue_min,hue_max].  The pixels' saturations are set to unity.  The
// pixels' values are rescaled from [0,1] to [0.5,1].

// We wrote this method in Feb 2012 in order to render plume masks
// much easier to see than in their original grey-scale form.

void texture_rectangle::convert_grey_values_to_hues(
   double hue_min,double hue_max)
{
//   cout << "inside texture_rectangle::convert_grey_values_to_hues()" 
//	  << endl;

   int R,G,B;
   double h,s,v;
   R=G=B=0;
   for (unsigned int pu=0; pu<getWidth(); pu++)
   {
      for (unsigned int pv=0; pv<getHeight(); pv++)
      {
         get_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);

         if (R < 1 && G < 1 && B < 1) continue;
         
//	 cout << "R = " << R << " G = " << G << " B = " << B << endl;

         double r=R/255.0;
         double g=G/255.0;
         double b=B/255.0;
         colorfunc::RGB_to_hsv(r,g,b,h,s,v);
         double new_hue=hue_min+(hue_max-hue_min)*v;
         s=1;	// Recall s = 0 corresponds to grey scale!
         v=0.5+0.5*v;

         colorfunc::HSV hsv(new_hue,s,v);
         colorfunc::RGB rgb=colorfunc::hsv_to_RGB(hsv);

         R=255*rgb.first;
         G=255*rgb.second;
         B=255*rgb.third;
         set_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);
      }
   }
   image_refptr->dirty();
}

// ----------------------------------------------------------------
// Member function convert_greyscale_image_to_hue_colored() reads in
// RGBA values which are currently stored within the
// texture_rectangle's pixels.  This method assumes n_channels=4 and
// that initially R=G=B for every pixel.  It resets the pixels' hues
// equal to their intensity values corresponding to their (R,G,B)
// coordinates spread across the interval [hue_min,hue_max].  The
// pixels' saturations and values are both set to unity.  

void texture_rectangle::convert_greyscale_image_to_hue_colored(
   double output_alpha)
{
//   cout << "inside texture_rectangle::convert_greyscale_image_to_hue_colored()" 
//        << endl;

   double hue_min=0;	// red
   double hue_max=120;	// green
   convert_greyscale_image_to_hue_colored(hue_min,hue_max,output_alpha);
}

void texture_rectangle::convert_greyscale_image_to_hue_colored(
   double hue_min,double hue_max,double output_alpha)
{
//   cout << "inside texture_rectangle::convert_greyscale_image_to_hue_colored()" 
//        << endl;
//   cout << "hue_min = " << hue_min << " hue_max = " << hue_max << endl;

//   int neg_p_values=0;
//   int p_values_0_2=0;
//   int p_values_2_4=0;
//   int p_values_4_6=0;
//   int p_values_6_8=0;
//   int p_values_8_10=0;

   int R,G,B,A;
   R=G=B=A=0;
   double h,s,v;
   for (unsigned int pu=0; pu<getWidth(); pu++)
   {
      for (unsigned int pv=0; pv<getHeight(); pv++)
      {
         get_pixel_RGBA_values(pu,pv,image_refptr->data(),R,G,B,A);
         bool black_pixel_flag=false;
         if (R==0 && G==0 && B==0 && A==0) black_pixel_flag=true;

//         cout << "pu = " << pu << " pv = " << pv
//              << " R = " << R << " G = " << G
//              << " B = " << B << " A = " << A << endl;

         double r=R/255.0;
         double g=G/255.0;
         double b=B/255.0;
         double a=A/255.0;

/*
  if (R==0) neg_p_values++;
  if (r > 0 && r < 0.2)
  {
  p_values_0_2++;
  }
  else if (r >= 0.2 && r < 0.4)
  {
  p_values_2_4++;
  }
  else if (r >= 0.4 && r < 0.6)
  {
  p_values_4_6++;
  }
  else if (r >= 0.6 && r < 0.8)
  {
  p_values_6_8++;
  }
  else if (r >= 0.8)
  {
  p_values_8_10++;
  }
*/

         colorfunc::RGB_to_hsv(r,g,b,h,s,v);
         double new_hue=hue_min+(hue_max-hue_min)*v;
         s=1;	// Recall s = 0 corresponds to grey scale!
         v=1;

         colorfunc::HSV hsv(new_hue,s,v);
         colorfunc::RGB rgb=colorfunc::hsv_to_RGB(hsv);
         R=255*rgb.first;
         G=255*rgb.second;
         B=255*rgb.third;

         const double SMALL_POS=0.001;
         if (a < SMALL_POS)
         {
            A=0;
         }
         else
         {
            A=255*output_alpha;
         }

// As of 10/20/10, we want to keep the color for no-data pixels as black:

         if (black_pixel_flag)
         {
            R=G=B=A=0;
//            R=G=B=0;
//            A=255;
         }
         
//         cout << "pu = " << pu << " pv = " << pv
//              << " new h = " << new_hue
//              << " R = " << R << " G = " << G
//              << " B = " << B << " A = " << A << endl;

         set_pixel_RGBA_values(pu,pv,image_refptr->data(),R,G,B,A);
      }
   }
   image_refptr->dirty();

/*
  cout << "neg_p_values = " << neg_p_values << endl;
  cout << "p_values_0_0.2 = " << p_values_0_2 << endl;
  cout << "p_values_0.2_0.4 = " << p_values_2_4 << endl;
  cout << "p_values_0.4_0.6 = " << p_values_4_6 << endl;
  cout << "p_values_0.6_0.8 = " << p_values_6_8 << endl;
  cout << "p_values_0.8_1 = " << p_values_8_10 << endl;
*/
}

// ----------------------------------------------------------------
void texture_rectangle::convert_greyscale_image_to_hue_value_colored(
   double hue,double output_alpha)
{
//   cout << "inside texture_rectangle::convert_greyscale_image_to_hue_value_colored()" 
//        << endl;

   int R,G,B,A;
   R=G=B=A=0;
   double h,s,v;
   for (unsigned int pu=0; pu<getWidth(); pu++)
   {
      for (unsigned int pv=0; pv<getHeight(); pv++)
      {
         get_pixel_RGBA_values(pu,pv,image_refptr->data(),R,G,B,A);
         double r=R/255.0;
         double g=G/255.0;
         double b=B/255.0;
         double a=A/255.0;

         colorfunc::RGB_to_hsv(r,g,b,h,s,v);

//         cout << "new_hue = " << new_hue 
//              << " sat = " << s 
//              << " v = " << v << endl;


         s=1;	// Recall s = 0 corresponds to grey scale!
//         v=1;


// On 11/2/09, we experimented with displaying the LOST threat map as
// red against grey rather than red against black.  Both Jennifer
// Drexler and I thought that the red vs black map looks better.  

//         s=v;
//         double v_hi=1.0;
//         double v_lo=0.25;
//         v=v_lo+v*(v_hi-v_lo);

         colorfunc::HSV hsv(hue,s,v);
         colorfunc::RGB rgb=colorfunc::hsv_to_RGB(hsv);
         R=255*rgb.first;
         G=255*rgb.second;
         B=255*rgb.third;

         const double SMALL_POS=0.001;
         if (a < SMALL_POS)
         {
            A=0;
         }
         else
         {
            A=255*output_alpha;
         }

//         cout << "pu = " << pu << " pv = " << pv
//              << " new h = " << new_hue
//              << " R = " << R << " G = " << G
//              << " B = " << B << endl;

         set_pixel_RGBA_values(pu,pv,image_refptr->data(),R,G,B,A);
      }
   }
   image_refptr->dirty();
}

// ----------------------------------------------------------------
// Member function convert_single_twoDarray_to_three_channels() takes
// in twoDarray *qtwoDarray_ptr which we assume is filled with
// intensity values ranging from 0 to 255.  We further assume *this
// has already been initialized to have 3 color channels [e.g. via
// call to constructor #2 followed by call to
// generate_blank_image_file()].  This method sets the R,G,B values
// for each pixel within the mdim x ndim texture rectangle equal to
// the corresponding intensity within *qtwoDarray_ptr.

// Note: On 8/7/12, we empirically found that we needed to add some
// small random fluctuations to at least one of the color channels in
// order to force the exported JPG version of an input 8-byte
// greyscale image to really have 3 RGB channels rather than a single
// grey channel.

void texture_rectangle::convert_single_twoDarray_to_three_channels(
   const twoDarray* qtwoDarray_ptr,bool randomize_blue_values_flag)
{
   cout << "inside texture_rectangle::convert_single_twoDarray_to_three_channels()" << endl;

   unsigned int mdim=qtwoDarray_ptr->get_mdim();
   unsigned int ndim=qtwoDarray_ptr->get_ndim();
   cout << "mdim = " << mdim << " ndim = " << ndim << endl;
   for (unsigned int pu=0; pu<mdim; pu++)
   {
      for (unsigned int pv=0; pv<ndim; pv++)
      {
         double curr_q=qtwoDarray_ptr->get(pu,pv);

         int R,G,B;
         if (curr_q < 1) 
         {
            R=G=B=0;
         }
         else
         {
            R=curr_q;
            G=curr_q;
            B=curr_q;
            if (randomize_blue_values_flag)
            {
               B += 3*nrfunc::ran1();
               B=basic_math::max(0,B);
               B=basic_math::min(255,B);
            }
         }
         set_pixel_RGB_values(pu,pv,image_refptr->data(),R,G,B);

//         cout << "pu = " << pu << " pv = " << pv 
//              << " q = " << curr_q 
//              << " R = " << R
//              << " G = " << G
//              << " B = " << B
//              << endl;
      }
   }

   image_refptr->dirty();
}

// ----------------------------------------------------------------
// Member function minutely_perturb_RGB_values() adds small random
// fluctuations to R=G=B color values in order to force them to be
// unequal.  We wrote this method in Aug 31 to avoid black & white
// images from turning into 8-bit greyscale rather than 24-bit
// RGB JPGs.

void texture_rectangle::minutely_perturb_RGB_values()
{
//   cout << "inside texture_rectangle::minutely_perturb_RGB_values()" << endl;

   int R,G,B;
   for (unsigned int pu=0; pu<getWidth(); pu++)
   {
      for (unsigned int pv=0; pv<getHeight(); pv++)
      {
         get_pixel_RGB_values(pu,pv,R,G,B);

         R += 3*nrfunc::ran1();
         R=basic_math::max(0,R);
         R=basic_math::min(255,R);

         G += 3*nrfunc::ran1();
         G=basic_math::max(0,G);
         G=basic_math::min(255,G);

         B += 3*nrfunc::ran1();
         B=basic_math::max(0,B);
         B=basic_math::min(255,B);

         set_pixel_RGB_values(pu,pv,R,G,B);
      }
   }

   image_refptr->dirty();
}

// ---------------------------------------------------------------------
// Member function generate_RGB_from_grey_texture_rectangle() works
// with the current texture rectangle which is assumed to be filled
// with 8-bit greyscale values.  It instantiates a new texture
// rectangle which holds three bytes for each pixel.  Greyscale
// intensity values are copied from the 8-bit to the new 24-bit
// texture rectangle.

texture_rectangle* 
texture_rectangle::generate_RGB_from_grey_texture_rectangle()
{
//   cout << "inside texture_rectangle::generate_RGB_from_grey_texture_rectangle()" << endl;

   unsigned int width=getWidth();
   unsigned int height=getHeight();

   texture_rectangle* RGB_texture_rectangle_ptr=new texture_rectangle(
      width,height,1,3,NULL);
   string blank_filename="blank.jpg";
   RGB_texture_rectangle_ptr->generate_blank_image_file(
      width,height,blank_filename,0.5);

   int R,G,B;
   bool randomize_blue_values_flag=true;
   for (unsigned int pv=0; pv<height; pv++)
   {
      for (unsigned int pu=0; pu<width; pu++)
      {
         get_pixel_RGB_values(pu,pv,R,G,B);
         R = G = B = 255 * get_pixel_intensity(pu, pv);

         if (randomize_blue_values_flag)
         {
            B += 3*nrfunc::ran1();
            B=basic_math::max(0,B);
            B=basic_math::min(255,B);
         }

         if(R > 1){
            cout << "pu = " << pu << " pv = " << pv
                 << " R = " << R << " G = " << G << " B = " << B << endl;
         }
         
         RGB_texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);
      }
   }

   return RGB_texture_rectangle_ptr;
}

// ========================================================================
// Texture content manipulation member functions
// ========================================================================

void texture_rectangle::modify_hsv_in_region(
   unsigned int px_lo, unsigned int px_hi,
   unsigned int py_lo, unsigned int py_hi,
   double hue,double min_s,double min_v)
{
   double h,s,v;
   for (unsigned int pv=py_lo; pv<=py_hi; pv++)
   {
      for (unsigned int pu=px_lo; pu<=px_hi; pu++)
      {
         get_pixel_hsv_values(pu,pv,h,s,v);
         h=hue;
         s=basic_math::max(s,min_s);
         v=basic_math::max(v,min_v);
         set_pixel_hsv_values(pu,pv,h,s,v);
      }
   }
}

// ----------------------------------------------------------------
// Member function globally_perturb_hsv() adjusts every pixel's HSV
// color coordinates by specified input offsets.

void texture_rectangle::globally_perturb_hsv(
   double delta_h, double delta_s, double delta_v)
{
   globally_perturb_hsv(0, getWidth()-1, 0, getHeight()-1, 
                        delta_h, delta_s, delta_v);
}

void texture_rectangle::globally_perturb_hsv(
   int px_lo, int px_hi,
   int py_lo, int py_hi,
   double delta_h, double delta_s, double delta_v)
{
   double h, s, v;
   for(int py = py_lo; py <= py_hi; py++)
   {
      if (py < 0) continue;
      if (py >= int(getHeight()) - 1) continue;
      for(int px = px_lo; px <= px_hi; px++)
      {
         if(px < 0) continue;
         if (px >= int(getWidth()) - 1) continue;         

         get_pixel_hsv_values(px, py, h, s, v);
         h += delta_h;
         s += delta_s;
         v += delta_v;

         h = basic_math::phase_to_canonical_interval(h, 0, 360);
         s = basic_math::max(0.0, s);
         s = basic_math::min(1.0, s);
         v = basic_math::max(0.0, v);
         v = basic_math::min(1.0, v);
         set_pixel_hsv_values(px, py, h, s, v);
      } // loop over px index
   } // loop over py index
}

// ----------------------------------------------------------------
// Member function add_gaussian_noise() fluctuates RGB values for
// every pixel within a 3 or 4 channel texture rectangle.

void texture_rectangle::add_gaussian_noise(double sigma)
{
   add_gaussian_noise(0 , getWidth() - 1, 0, getHeight() - 1, sigma);
}

void texture_rectangle::add_gaussian_noise(
   int pu_lo, int pu_hi,
   int pv_lo, int pv_hi, double sigma)
{
//      cout << "inside texture_rectangle::add_gaussian_noise()" << endl;

   int n_channels = getNchannels();
   for (int pv = pv_lo; pv <= pv_hi; pv++)
   {
      if (pv < 0) continue;
      if (pv >= int(getHeight()) - 1) continue;
      for (int pu = pu_lo; pu <= pu_hi; pu++)
      {
         if (pu < 0) continue;
         if (pu >= int(getWidth()) - 1) continue;

         int R,G,B,A;
         if(n_channels == 3)
         {
            get_pixel_RGB_values(pu,pv,R,G,B);
         }
         else if (n_channels == 4)
         {
            get_pixel_RGBA_values(pu,pv,R,G,B,A);
         }

         int Rnew=colorfunc::fluctuate_value(R, sigma);
         int Gnew=colorfunc::fluctuate_value(G, sigma);
         int Bnew=colorfunc::fluctuate_value(B, sigma);

         if(n_channels == 3)
         {
            set_pixel_RGB_values(pu,pv,Rnew,Gnew,Bnew);
         }
         else if (n_channels == 4)
         {
            int Anew = A;
            set_pixel_RGBA_values(pu,pv,Rnew,Gnew,Bnew,Anew);
         }
      } // loop over pu
   } // loop over pv
}

// ========================================================================
// Multiple texture overlay member functions
// ========================================================================

bool texture_rectangle::overlay(string ontop_img_filename, int qx, int qy)
{
   texture_rectangle* ontop_tr_ptr = new texture_rectangle(
      ontop_img_filename, NULL);
   int ontop_n_channels = ontop_tr_ptr->getNchannels();

   int R,G,B,A;
   for(int py = qy; py < qy + int(ontop_tr_ptr->getHeight()); py++)
   {
      if(py < 0 || py >= int(getHeight()) ) continue;
      for(int px = qx; px < qx + int(ontop_tr_ptr->getWidth()); px++)
      {
         if(px < 0 || px >= int(getWidth()) ) continue;
         if(ontop_n_channels == 4)
         {
            ontop_tr_ptr->get_pixel_RGBA_values(px-qx,py-qy,R,G,B,A);
            if(A==0) continue;
         }
         else if (ontop_n_channels == 3)
         {
            ontop_tr_ptr->get_pixel_RGB_values(px-qx,py-qy,R,G,B);
         }
         else
         {
            cout << "Error in texture_rectangle::superpose()" << endl;
            cout << "ontop_n_channels = " << ontop_n_channels << endl;
            cout << "ontop_tr_ptr->get_video_filename() = "
                 << ontop_tr_ptr->get_video_filename() << endl;
            return false;
         }
         if(R < 0 || G < 0 || B < 0) continue;
         
         set_pixel_RGB_values(px,py,R,G,B);
      } // loop over px
   } // loop over py 
   
   delete ontop_tr_ptr;
   return true;
}

// ----------------------------------------------------------------
// Member function overlay_layer() takes in texture rectangle
// *overlay_tr_ptr whose contents are to be superposed on top of the
// current texture rectangle object.  The upper left corner of
// *overlay_tr_ptr is placed at pixel coordinates (pu_start, pv_start)
// for the current texture rectangle object.  Any portion of
// *overlay_tr_ptr which lies outside of the current texture rectangle
// object is ignored.

// pu,pv = pixel coordinates within current texture rectangle
// qu,qv = pixel coordinates within *overlay_tr_ptr

void texture_rectangle::overlay_layer(
   int pu_start, int pv_start, texture_rectangle* overlay_tr_ptr,
   bool generate_mask_flag)
{
   for(unsigned int qv = 0; qv < overlay_tr_ptr->getHeight(); qv++)
   {
      for(unsigned int qu = 0; qu < overlay_tr_ptr->getWidth(); qu++)
      {
         int R, G, B;
         overlay_tr_ptr->get_pixel_RGB_values(qu, qv, R, G, B);
         if(generate_mask_flag)
         {
            if(R > 0 || G > 0 || B > 0)
            {
               R = G = B = 1;
//               R = G = B = 255;
            }
         }

         int pu = pu_start + qu;
         int pv = pv_start + qv;
         set_pixel_RGB_values(pu,pv,R,G,B);

      } // loop over qu
   } // loop over qv
}

// ========================================================================
// Video file output member functions
// ========================================================================

// Member function write_dotVidfile instatiates a VidFile and copies
// byte data from the input STL char* vector to it.

void texture_rectangle::write_dotVidfile(
   string output_filename,const vector<unsigned char*>& charstar_vector)
{
//   cout << "inside texture_rectangle::write_dotVidfile()" << endl;
//   cout << "getWidth() = " << getWidth() 
//        << " getHeight() = " << getHeight()
//        << " get_Nimages = " << get_Nimages()
//        << " getNchannels = " << getNchannels() << endl;
   
   VidFile vid_out;
   vid_out.New_8U(
      output_filename,getWidth(),getHeight(),get_Nimages(),getNchannels());

   for (unsigned int i=0; i<charstar_vector.size(); i++)
   {
      vid_out.WriteFrame(charstar_vector[i],getWidth()*getNchannels());
   }
}

// ----------------------------------------------------------------
// Member function write_curr_frame uses OSG's built-in file writer
// plugins to output the current contents of *get_image_ptr() as a png
// file.  It chips out the region defined by the input lower left and
// upper right corner fractions.  If input parameter
// n_horiz_output_pixels != -1, this method uses ImageMagick's CONVERT
// to resample the output image so that its horizontal size matches
// the specified input value.

void texture_rectangle::write_curr_frame(
   string output_filename,int n_horiz_output_pixels)
{
   twovector lower_left_corner_fracs(0,0);
   twovector upper_right_corner_fracs(1,1);
   write_curr_frame(lower_left_corner_fracs,upper_right_corner_fracs,
                    output_filename,n_horiz_output_pixels);
}

void texture_rectangle::write_curr_frame(
   const twovector& lower_left_corner_fracs,
   const twovector& upper_right_corner_fracs,
   string output_filename,int n_horiz_output_pixels)
{
//   outputfunc::write_banner("Writing current subframe to output file:");
//     cout << "inside texture_rectangle::write_curr_frame() #2" << endl;
//     cout << "output_filename = " << output_filename << endl;

   int px_start=lower_left_corner_fracs.get(0)*getWidth();
   int px_stop=upper_right_corner_fracs.get(0)*getWidth();
   int py_start=lower_left_corner_fracs.get(1)*getHeight();
   int py_stop=upper_right_corner_fracs.get(1)*getHeight();
   write_curr_frame(
      px_start,px_stop,py_start,py_stop,
      output_filename,n_horiz_output_pixels);
}

void texture_rectangle::write_curr_frame(
   unsigned int px_start,unsigned int px_stop,
   unsigned int py_start,unsigned int py_stop,
   string output_filename,int n_horiz_output_pixels)
{
//   outputfunc::write_banner("Writing current subframe to output file:");
//   cout << "inside texture_rectangle::write_curr_frame() #3" << endl;
//   cout << "output_filename = " << output_filename << endl;

   px_start=basic_math::max(Unsigned_Zero,px_start);
   px_stop=basic_math::min(getWidth()-1,px_stop);
   py_start=basic_math::max(Unsigned_Zero,py_start);
   py_stop=basic_math::min(getHeight()-1,py_stop);

// Flip image vertically prior to writing it to output file.
// Afterwards, flip it again:

   get_image_ptr()->flipVertical();

   osg::Image* subimage_ptr = new osg::Image;
   
   unsigned int width=basic_math::min(getWidth(),px_stop-px_start+1);
   unsigned int height=basic_math::min(getHeight(),py_stop-py_start+1);
 
//   cout << "px_start = " << px_start << " px_stop = " << px_stop << endl;
//   cout << "py_start = " << py_start << " py_stop = " << py_stop << endl;
//   cout << "width = " << width << " height = " << height << endl;
//   cout << "getHeight() = " << getHeight() << endl;
//   cout << "getWidth() = " << getWidth() << endl;
//   cout << "getNchannels() = " << getNchannels() << endl;
   
   int subimage_size_in_bytes=width*height*getNchannels();
   unsigned char* m_subimage = new unsigned char[ subimage_size_in_bytes ];
//   cout << "subimage_size_in_bytes = " << subimage_size_in_bytes << endl;

// Copy image data already read into image's data array into scratch
// data array m_subimage:

   unsigned int p=py_start*getWidth()+px_start;
//   cout << "p = " << p << endl;
   unsigned int i=0;
   for (unsigned int h=0; h<height; h++)
   {
      for (unsigned int w=0; w<width; w++)
      {
         unsigned int pixel_offset=p+h*getWidth()+w;
         for (unsigned int c=0; c<getNchannels(); c++)
         {
            int byte_offset=pixel_offset*getNchannels()+c;
            m_subimage[i]=*(get_image_ptr()->data()+byte_offset);
            i++;
         } // loop over c index
      } // loop over w index
   } // loop over h index

   subimage_ptr->setImage( width,height,GLimageDepth,
                           GLinternalTextureFormat,GLformat,GLtype,
                           m_subimage, allocation_mode, GLpacking );

   osgDB::writeImageFile(*subimage_ptr,output_filename);
   get_image_ptr()->flipVertical();

// If n_horiz_output_pixels > 0, resample image file using
// gdal_translate to force number of horizontal output pixels to agree
// with specified input number while maintaining the image's aspect
// ratio.  On 6/10/08, we empirically found that ImageMagick's CONVERT
// program does a much better of upsampling small chips than does
// GDAL's gdal_translate...

//   cout << "n_horiz_output_pixels = " << n_horiz_output_pixels << endl;
   if (n_horiz_output_pixels > 0)
   {
//      sleep(3);
   
      double aspect_ratio=
         double(fabs(py_stop-py_start))/double(fabs(px_stop-px_start));
      int resampled_width=n_horiz_output_pixels;
      int resampled_height=aspect_ratio*resampled_width;

//      string resampled_filename="resampled_"+output_filename;
      string resampled_filename="./resampled.png";
      string subsample_command=
         "convert -resize "
         +stringfunc::number_to_string(resampled_width)+"x"
         +stringfunc::number_to_string(resampled_height)+" "
         +output_filename+" "+resampled_filename;
//      string subsample_command=
//         "gdal_translate -outsize "
//         +stringfunc::number_to_string(resampled_width)+" "
//         +stringfunc::number_to_string(resampled_height)+" "
//         +output_filename+" "+resampled_filename;
      sysfunc::unix_command(subsample_command);

      string mv_command="mv "+resampled_filename+" "+output_filename;
      sysfunc::unix_command(mv_command);
   } // n_horiz_output_pixels > 0 conditional

//   string banner="Finished writing subframe to "+output_filename;
//   outputfunc::write_banner(banner);

   delete [] m_subimage;
}

// ----------------------------------------------------------------
// Member function write_curr_subframe() is a minor variant of the
// preceding write_curr_frame() method.  It handles px_stop >= width
// and/or py_stop >= height by simply setting such pixels lying
// outside the original image equal to outside_frame_value within the
// exported subframe.  If horiz_flipped_flag == true, a left-right
// parity flip is performed before the texture rectangle's subimage is
// exported.

void texture_rectangle::write_curr_subframe(
   int px_start, int px_stop,
   int py_start, int py_stop,
   string output_filename, bool horiz_flipped_flag)
{
   unsigned int outside_frame_value = 128;
   int px_offset = 0;
   int py_offset = 0;
   write_curr_subframe(px_start, px_stop, py_start, py_stop,
                       px_offset, py_offset, output_filename, 
                       outside_frame_value, horiz_flipped_flag);
}

void texture_rectangle::write_curr_subframe(
   int px_start, int px_stop,
   int py_start, int py_stop,
   int px_offset, int py_offset,
   string output_filename, unsigned int outside_frame_value,
   bool horiz_flipped_flag)
{
//   cout << "inside tr::write_curr_subframe()" << endl;
//   cout << "px_start = " << px_start << " px_stop = " << px_stop
//        << " px_offset = " << px_offset << endl;
//   cout << "py_start = " << py_start << " py_stop = " << py_stop
//        << " py_offset = " << py_offset << endl;
   
// Flip image vertically prior to writing it to output file.
// Afterwards, flip it again:

   get_image_ptr()->flipVertical();

   osg::Image* subimage_ptr = new osg::Image;
   
   unsigned int fullframe_width=px_stop-px_start+1;
   unsigned int fullframe_height=py_stop-py_start+1;
   unsigned int subimage_size_in_bytes=
      fullframe_width*fullframe_height*getNchannels();
   unsigned char* m_subimage = new unsigned char[ subimage_size_in_bytes ];

   unsigned int p=py_start*getWidth()+px_start;
   unsigned int istart=(py_offset*fullframe_width+px_offset)*getNchannels();

// Initialize all entries in m_subimage to outside_frame_value:

   memset(m_subimage, outside_frame_value, subimage_size_in_bytes);
   unsigned int i = istart;

// Next copy image data already read into image's data array into
// scratch data array m_subimage:

   for (unsigned int h=0; h<fullframe_height && i<subimage_size_in_bytes; h++)
   {
      unsigned int py = py_start + h;

      for (unsigned int w=0; w<fullframe_width && i<subimage_size_in_bytes; 
           w++)
      {
         unsigned int w2 = w;
         if(horiz_flipped_flag)
         {
            w2 = fullframe_width - 1 - w;
         }
         unsigned int px = px_start + w2;

         bool pixel_outside_image = false;
         if(py < 0 || py >= getHeight()) pixel_outside_image = true;
         if(px < 0 || px >= getWidth()) pixel_outside_image = true;

         for (unsigned int c=0; c<getNchannels() && i<subimage_size_in_bytes; 
              c++)
         {
            if(!pixel_outside_image)
            {
               unsigned int pixel_offset=p+h*getWidth()+w2;
               int byte_offset=pixel_offset*getNchannels()+c;
               m_subimage[i]=*(get_image_ptr()->data()+byte_offset);
            }
            i++;
         } // loop over c index
      } // loop over w index
   } // loop over h index

   subimage_ptr->setImage( fullframe_width,fullframe_height,GLimageDepth,
                           GLinternalTextureFormat,GLformat,GLtype,
                           m_subimage, allocation_mode, GLpacking );

   osgDB::writeImageFile(*subimage_ptr,output_filename);
   get_image_ptr()->flipVertical();

   delete [] m_subimage;
}

// ----------------------------------------------------------------
// Member function retrieve_curr_subframe_byte_data takes in the
// horizontal and vertical fractions of the current video frame which
// define a subimage.  It extracts the contents of this subframe to
// string member output_image_string in the form of an image whose
// type is specified by the output_image_suffix input argument.  The
// byte contents of output_image_string can later be returned to an
// http client such as a web browser for display.

void texture_rectangle::retrieve_curr_subframe_byte_data(
   const twovector& lower_left_corner_fracs,
   const twovector& upper_right_corner_fracs,
   string output_image_suffix,bool draw_central_bbox_flag,
   int n_horiz_output_pixels)
{
   outputfunc::write_banner("Retrieving current subframe byte data:");
//   cout << "inside texture_rectangle::retrieve_curr_subframe_byte_data()"
//        << endl;
//   cout << "lower_left_corner_fracs = " << lower_left_corner_fracs << endl;
//   cout << "upper_right_corner_fracs = " << upper_right_corner_fracs << endl;

   AnimationController_ptr->get_time_corresponding_to_curr_frame();
//   cout << "Current frame number = " << AnimationController_ptr->
//      get_true_framenumber() << endl;
//   cout << "World time = " << AnimationController_ptr->get_world_time_string()
//        << endl;

   unsigned int px_start=lower_left_corner_fracs.get(0)*getWidth();
   unsigned int px_stop=upper_right_corner_fracs.get(0)*getWidth();
   unsigned int width=basic_math::min(getWidth(),px_stop-px_start+1);
   
   unsigned int py_start=lower_left_corner_fracs.get(1)*getHeight();
   unsigned int py_stop=upper_right_corner_fracs.get(1)*getHeight();
   unsigned int height=basic_math::min(getHeight(),py_stop-py_start+1);

//   cout << "px_start = " << px_start << " px_stop = " << px_stop << endl;
//   cout << "py_start = " << py_start << " py_stop = " << py_stop << endl;
//   cout << "width = " << width << " height = " << height << endl;
//   cout << "getWidth() = " << getWidth()
//        << " getHeight() = " << getHeight() << endl;

   int subimage_size_in_bytes=width*height*getNchannels();
   unsigned char* m_subimage = new unsigned char[ subimage_size_in_bytes ];

// Flip entire image vertically prior to extract subframe byte data.
// At very end of this method, flip it again:

   get_image_ptr()->flipVertical();

// Copy image data already read into image's data array into scratch
// data array m_subimage:

   unsigned int p=py_start*getWidth()+px_start;
//   cout << "p = " << p << endl;
   unsigned int i=0;
   for (unsigned int h=0; h<height; h++)
   {
      for (unsigned int w=0; w<width; w++)
      {
         unsigned int pixel_offset=p+h*getWidth()+w;
         for (unsigned int c=0; c<getNchannels(); c++)
         {
            int byte_offset=pixel_offset*getNchannels()+c;
            m_subimage[i]=*(get_image_ptr()->data()+byte_offset);
            i++;
         } // loop over c index
      } // loop over w index
   } // loop over h index

// For ground truth vehicle tracking purposes, draw relatively small
// colored bounding box at center of m_subimage:

   if (draw_central_bbox_flag)
   {
      double frac_size=0.075;
      generate_central_bbox(
         height,width,frac_size,colorfunc::green,m_subimage);
   } // draw_central_bbox_flag conditional
   
   osg::Image* subimage_ptr = new osg::Image;
   subimage_ptr->setImage( width,height,GLimageDepth,
                           GLinternalTextureFormat,GLformat,GLtype,
                           m_subimage, allocation_mode, GLpacking );

// Resample image file using OSG's scaleImage command to force number
// of horizontal output pixels to agree with specified input number
// while maintaining the image's aspect ratio:

//   cout << "n_horiz_output_pixels = " << n_horiz_output_pixels << endl;
   if (n_horiz_output_pixels > 0)
   {
      int resampled_width=n_horiz_output_pixels;
      int resampled_height=
         (upper_right_corner_fracs.get(1)-lower_left_corner_fracs.get(1))/
         (upper_right_corner_fracs.get(0)-lower_left_corner_fracs.get(0))*
         resampled_width;
//      cout << "resampled_width = " << resampled_width
//           << " resampled_height = " << resampled_height << endl;
      subimage_ptr->scaleImage(resampled_width,resampled_height,GLimageDepth);
   }

// Write output subframe byte data first to string stream and then to
// string member output_image_string.  VideoServer::get() can
// subsequently rewrite output_image_string as a QByteArray which can
// be returned as a response to an http get request:

//   cout << "output_image_suffix = " << output_image_suffix << endl;

   if (!Registry_refptr.valid()) Registry_refptr=osgDB::Registry::instance(); 
   osgDB::ReaderWriter* ReaderWriter_ptr=Registry_refptr->
      getReaderWriterForExtension(output_image_suffix);
   ostringstream output_stream;

   osgDB::ReaderWriter::WriteResult WR=
      ReaderWriter_ptr->writeImage(*subimage_ptr,output_stream);
//   cout << "WR.status() = " << WR.status() << endl;
//   cout << "WR.success() = " << WR.success() << endl;
//   cout << "WR.error() = " << WR.error() << endl;
//   cout << "WR.notHandled = " << WR.notHandled() << endl;
//   cout << "WR.message() = " << WR.message() << endl;

   output_image_string=output_stream.str();
//   cout << "output_image_string.size() = " << output_image_string.size()
//        << endl;

   delete [] m_subimage;

// Recall that the image has been flipped once.  We need to flip it
// again before exiting this method:

   get_image_ptr()->flipVertical();
}

// ---------------------------------------------------------------------
// Member function generate_blank_image_file() takes in the horizontal
// and vertical size in pixels, the name of the output blank filename
// and a grey level (ranging from 0 to 1).  It uses the current
// texture rectangle to output a PNG or JPG file to disk.  We wrote
// this little utility in Jan 2011 for virtual OBSFRUSTUM display
// within the LADAR thin client.

void texture_rectangle::generate_blank_image_file(
   unsigned int mdim,unsigned int ndim,string blank_filename,double grey_level)
{
//   cout << "inside texture_rectangle::generate_blank_image_file()" << endl;
//   cout << "mdim = " << mdim << " ndim = " << ndim << " blank_filename = "
//        << blank_filename << endl;
   
   setWidth(mdim);
   setHeight(ndim);

   Nimages=1;
   set_first_imagenumber(0);
   set_last_frame_to_display(0);

   m_Nchannels=3;
   bool blank_png_flag=false;

   string suffix=stringfunc::suffix(blank_filename);
   if (suffix=="png" || suffix=="PNG")
   {
      m_Nchannels=4;
      blank_png_flag=true;
   }

   twoDarray* ztwoDarray_ptr=new twoDarray(mdim,ndim);
//   cout << "ztwoDarray_ptr = " << ztwoDarray_ptr << endl;
   for (unsigned int px=0; px<mdim; px++)
   {
      for (unsigned int py=0; py<ndim; py++)
      {
         ztwoDarray_ptr->put(px,py,grey_level);
      }
   }
   initialize_twoDarray_image(ztwoDarray_ptr,m_Nchannels,blank_png_flag);   

   write_curr_frame(blank_filename);
   delete ztwoDarray_ptr;
}

// ----------------------------------------------------------------
// Member function generate_central_bbox takes in the height and width
// for a video chip measured in pixels.  It also takes in the bounding
// box's fractional size, the bbox color and the unsigned char* array
// which holds the video chip data.  This method draws a single pixel
// wide box surrounding the video chip's center location.

void texture_rectangle::generate_central_bbox(
   double chip_height,double chip_width,double frac_size,
   colorfunc::Color bbox_color,unsigned char* m_subimage)
{
//   cout << "inside texture_rectangle::generate_central_bbox()" << endl;
  
   unsigned int h_top=(0.5-0.5*frac_size)*chip_height;
   unsigned int h_bottom=(0.5+0.5*frac_size)*chip_height;
   unsigned int w_left=(0.5-0.5*frac_size)*chip_width;
   unsigned int w_right=(0.5+0.5*frac_size)*chip_width;

   colorfunc::RGB_bytes bbox_color_bytes=
      colorfunc::RGB_to_bytes(colorfunc::get_RGB_values(bbox_color));

   for (unsigned int h=h_top; h<=h_bottom; h++)
   {
      color_subimage_array(m_subimage,h*chip_width+w_left,bbox_color_bytes);
      color_subimage_array(m_subimage,h*chip_width+w_right,bbox_color_bytes);
   } // loop over index h labeling chip_height variable

   for (unsigned int w=w_left; w<=w_right; w++)
   {
      color_subimage_array(m_subimage,h_top*chip_width+w,bbox_color_bytes);
      color_subimage_array(m_subimage,h_bottom*chip_width+w,bbox_color_bytes);
   } // loop over index h labeling chip_height variable
}

// ----------------------------------------------------------------
// Member function color_subimage_array colors the 3 RGB entries
// within char* array m_subimage located at pixel_offset from its
// start.

void texture_rectangle::color_subimage_array(
   unsigned char* m_subimage,int pixel_offset,
   colorfunc::RGB_bytes bbox_color)
{
   m_subimage[pixel_offset*getNchannels()+0]=bbox_color.first;
   m_subimage[pixel_offset*getNchannels()+1]=bbox_color.second;
   m_subimage[pixel_offset*getNchannels()+2]=bbox_color.third;
}

// ========================================================================
// Photograph manipulation member functions
// ========================================================================

// Member function read_next_photo() retrieves the filename of the
// next image to be displayed.  If the next photo's width or height
// differs from those of the maximal width and height in the entire
// sequence, this method centers the former within the latter.
// Either the original or modified next image is then displayed.

bool texture_rectangle::read_next_photo()
{
//   cout << "inside texture_rectangle::read_next_photo()" << endl;
//   cout << "true framenumber = " 
//        << AnimationController_ptr->get_true_framenumber() << endl;
   
   string next_photo_filename=AnimationController_ptr->
      get_next_ordered_image_filename();
//   cout << "next_photo_filename = " << next_photo_filename << endl;
   if (next_photo_filename.size()==0) return false;

   unsigned int next_width,next_height;
   imagefunc::get_image_width_height(
      next_photo_filename,next_width,next_height);
//   cout << "next_width = " << next_width
//        << " next_height = " << next_height << endl;

   string resized_photo_filename="";
   if (next_width != getWidth() || next_height != getHeight())
   {
      texture_rectangle* subtexture_rectangle_ptr=
         new texture_rectangle(next_photo_filename,NULL);
      
      int R=20;
      int G=20;
      int B=20;
      reset_all_RGB_values(R,G,B);

      unsigned int border_x = 0.5 * (getWidth() - next_width);
      unsigned int border_y = 0.5 * (getHeight() - next_height);
      resized_photo_filename="/tmp/resized.jpg";

      for (unsigned int py=0; py<next_height; py++)
      {
         for (unsigned int px=0; px<next_width; px++)
         {
            subtexture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);
            set_pixel_RGB_values(border_x+px,border_y+py,R,G,B);
         } // loop over px index
      } // loop over py index
      delete subtexture_rectangle_ptr;
   }

   cout << "Displayed image filename = " 
        << filefunc::getbasename(next_photo_filename) << endl;
   cout << "   xdim = " << next_width << " ydim = " << next_height << endl;
//   cout << "   curr_framenumber = " 
//        << AnimationController_ptr->get_curr_framenumber() << endl;

   if (panel_number >= 0)
   {
      modify_next_photo_filename(next_photo_filename);
   }

   if (resized_photo_filename.size() > 0)
   {
      filefunc::deletefile(resized_photo_filename);
   }
   else
   {
      reset_texture_content(next_photo_filename);
   }
 
   return true;
}

// ---------------------------------------------------------------------
// Member function modify_next_photo_filename() takes in a template
// for the next photo filename which is assumed to be of the form
// XXXX-YYPPP-NNNN.png.  Here PPP represents the "panel number" for a
// photo, while NNN represents its frame number.  (The panel number
// labels different panels of a panoramic image from a D7 video
// camera.)  This method replaces PPP within the input filename with
// the panel number assigned to the current texture_rectangle object.  

// We wrote this specialized method in Nov 2010 in order to play D7
// imagery as a video within 5 OBSFRUSTA in program HALFWHEEL.

void texture_rectangle::modify_next_photo_filename(string& next_photo_filename)
{
//   cout << "inside texture_rectangle::modify_next_photo_filename()" << endl;
//   cout << "Original next_photo_filename = " << next_photo_filename << endl;
//   cout << "panel_number = " << panel_number << endl;

   string dirname=filefunc::getdirname(next_photo_filename);
   string basename=filefunc::getbasename(next_photo_filename);

   string separator_chars=".-_";
   vector<string> substrings,separator_substrings;
   stringfunc::Tokenize(
      basename,substrings,separator_substrings,separator_chars);

   unsigned int panel_substring_index=substrings.size()-3;
   string panel_substring=substrings[panel_substring_index];
//   cout << "panel_substring = " << panel_substring << endl;

   separator_chars="0123456789";
   vector<string> panel_subsubstrings=
      stringfunc::decompose_string_into_substrings(
         panel_substring,separator_chars);
   for (unsigned int i=0; i<panel_subsubstrings.size(); i++)
   {
//      cout << "i = " << i << " subsubstring[i] = " << panel_subsubstrings[i] 
//           << endl;
   }

   panel_substring=panel_subsubstrings[0]+
      stringfunc::number_to_string(panel_number);
//   cout << "panel_substring = " << panel_substring << endl;
   
   string new_basename="";
   for (unsigned int i=0; i<panel_substring_index; i++)
   {
      new_basename += substrings[i];
      new_basename += separator_substrings[i];
   }
   new_basename += panel_substring;
   new_basename += separator_substrings[panel_substring_index];
   for (unsigned int i=panel_substring_index+1; i<separator_substrings.size();
        i++)
   {
      new_basename += substrings[i];
      new_basename += separator_substrings[i];
   }
   new_basename += substrings.back();
   next_photo_filename=dirname+new_basename;

//   cout << "Modified next_photo_filename = " << next_photo_filename << endl;
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
bool texture_rectangle::reset_texture_content(string photo_filename)
{
//   cout << "inside texture_rectangle::reset_texture_content()" << endl;
//   cout << "photo_filename = " << photo_filename << endl;

   if (!import_photo_from_file(photo_filename)) return false;
   initialize_general_video();
   set_TextureRectangle_image();

   return true;
}

// ------------------------------------------------------------------------
// Member function check_all_pixel_RGB_values()

void texture_rectangle::check_all_pixel_RGB_values()
{
   cout << "inside texture_rectangle::check_all_pixel_RGB_values()" << endl;

//   cout << "m_VidWidth = " << m_VidWidth 
//        << " m_VidHeight = " << m_VidHeight << endl;
   cout << "getNchannels() = " << getNchannels() << endl;
   cout << "image_size_in_bytes = " << image_size_in_bytes << endl;
   cout << "image_refptr->getTotalSizeInBytes() = "
        << image_refptr->getTotalSizeInBytes() << endl;

//   unsigned char* data_ptr=image_refptr->data();
   for (unsigned int py=0; py<getHeight(); py++)
   {
      for (unsigned int px=0; px<getWidth(); px++)
      {
         unsigned int p=py*getWidth()+px;
         check_pixel_bounds(p);

//         int i=getNchannels()*p;
//         int R=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+0]);
//         int G=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+1]);
//         cout << "G = " << G << endl;
//         int B=stringfunc::unsigned_char_to_ascii_integer(data_ptr[i+2]);
//         cout << "B = " << B << endl;


/*
  int R,G,B;
  R=G=B=0;
  get_pixel_RGB_values(px,py,image_refptr->data(),R,G,B);

  if (R > 253 && G < 3 && B < 3)
  {
  cout << "px = " << px
  << " py = " << py
  << " R = " << R << " G = " << G << " B = " << B 
  << endl;
  }
*/
         
      }
   }

   cout << "Before writing out new image:" << endl;
   string output_filename="new_image.png";
   write_curr_frame(output_filename);
}

// ---------------------------------------------------------------------
// Member function compute_image_entropy() calculates S = -sum_i p_i
// ln p_i where i ranges over the 256 intensity values for a greyscale
// image.  It returns a normalized values for S.  S=0 implies
// intensity is constant across the entire image.  S=1 implies image
// is "busy", "noisy", "informative".

double texture_rectangle::compute_image_entropy(
   bool filter_intensities_flag,int color_channel_ID)
{
//   cout << "inside texture_rectangle::compute_image_entropy()" << endl;
   return compute_image_entropy(
      0,getWidth(),0,getHeight(),filter_intensities_flag,color_channel_ID);
}

// ---------------------------------------------------------------------
double texture_rectangle::compute_image_entropy(
   unsigned int pu_start,unsigned int pu_stop,
   unsigned int pv_start,unsigned int pv_stop,
   bool filter_intensities_flag,int color_channel_ID)
{
//   cout << endl;
//   cout << "inside texture_rectangle::compute_image_entropy()" << endl;
//   cout << "pu_start = " << pu_start << " pu_stop = " << pu_stop << endl;
//   cout << "pv_start = " << pv_start << " pv_stop = " << pv_stop << endl;
//   cout << "filter_intensities_flag = " << filter_intensities_flag
//        << " greyscale_flag = " << greyscale_flag << endl;

   if(pu_stop < pu_start) return 0;
   if(pv_stop < pv_start) return 0;

   int curr_R,curr_G,curr_B;
   double h,s,v;
   vector<int> intensity,filtered_intensity;

   for (unsigned int pu=pu_start; pu<pu_stop; pu++)
   {
      for (unsigned int pv=pv_start; pv<pv_stop; pv++)
      {
         get_pixel_RGB_values(pu,pv,curr_R,curr_G,curr_B);

// Recall get_pixel_RGB_values() returns R=G=B=-1 to indicate missing
// data!

         if (curr_R < 0 || curr_G < 0 || curr_B < 0) continue;

         double r=curr_R/255.0;
         double g=curr_G/255.0;
         double b=curr_B/255.0;
         colorfunc::RGB_to_hsv(r,g,b,h,s,v);

         double curr_intensity = 0;
         if (color_channel_ID==0)
         {
            curr_intensity=v;
         }
         else if (color_channel_ID==1)
         {
            curr_intensity=s;
         }
	 else if (color_channel_ID==2){
            curr_intensity=r;
	 }
	 else if (color_channel_ID==3){
            curr_intensity=g;
	 }
	 else if (color_channel_ID==4){
            curr_intensity=b;
	 }
         intensity.push_back(255*curr_intensity);
         filtered_intensity.push_back(255*curr_intensity);
      }
   }

   int n_output_bins=256;
   double xlo=0;
   double dx=1;
   if (filter_intensities_flag)
   {
      prob_distribution prob(intensity,n_output_bins,xlo,dx);

// As of 6/24/11, we experiment with rejecting the lowest and highest
// 5% greyscale values from the intensity distribution.  We then
// compute image entropy for the "filtered" intensity distribution:

      double cumprob_lo=0.05;
      double cumprob_hi=0.95;
      double intensity_lo=prob.find_x_corresponding_to_pcum(cumprob_lo);
      double intensity_hi=prob.find_x_corresponding_to_pcum(cumprob_hi);
//      cout << "intensity_lo = " << intensity_lo
//           << " intensity_hi = " << intensity_hi << endl;

      filtered_intensity.clear();
      for (unsigned int i=0; i<intensity.size(); i++)
      {
         double curr_intensity=intensity[i];
         if (curr_intensity < intensity_lo || curr_intensity > intensity_hi)
            continue;
         filtered_intensity.push_back(curr_intensity);
      }
   }
   
//   cout << "intensity.size() = " << intensity.size()
//        << " filtered_intensity.size() = " << filtered_intensity.size()
//        << endl;

   prob_distribution filtered_prob(filtered_intensity,n_output_bins,xlo,dx);

//   cout << "prob = " << prob << endl;
//   double entropy=prob.entropy();
//   entropy /= log(256);

   const double ln_256=5.54517744448;
   double filtered_entropy=filtered_prob.entropy()/ln_256;

//   cout << " filtered_entropy = " << filtered_entropy << endl;
//   outputfunc::enter_continue_char();

   return filtered_entropy;
}

// ---------------------------------------------------------------------

void texture_rectangle::compute_RGB_image_entropies(
   unsigned int pu_start,unsigned int pu_stop,
   unsigned int pv_start,unsigned int pv_stop,
   double& R_entropy,double& G_entropy,double& B_entropy)
{
//   cout << endl;
//   cout << "inside texture_rectangle::compute_RGB_image_entropies()" << endl;
//   cout << "pu_start = " << pu_start << " pu_stop = " << pu_stop << endl;
//   cout << "pv_start = " << pv_start << " pv_stop = " << pv_stop << endl;
//   cout << "width = " << getWidth() << " height = " << getHeight()
//        << endl;

   int curr_R,curr_G,curr_B;
   vector<double> R,G,B;
   for (unsigned int pu=pu_start; pu<pu_stop; pu++)
   {
      for (unsigned int pv=pv_start; pv<pv_stop; pv++)
      {
         get_pixel_RGB_values(pu,pv,curr_R,curr_G,curr_B);
	 R.push_back(curr_R);
	 G.push_back(curr_G);
	 B.push_back(curr_B);
      }
   }

   int n_output_bins=256;
   double xlo=0;
   double dx=1;
   prob_distribution R_prob(R,n_output_bins,xlo,dx);
   prob_distribution G_prob(G,n_output_bins,xlo,dx);
   prob_distribution B_prob(B,n_output_bins,xlo,dx);

   const double ln_256=5.54517744448;
   R_entropy=R_prob.entropy()/ln_256;
   G_entropy=G_prob.entropy()/ln_256;
   B_entropy=B_prob.entropy()/ln_256;

   cout << "R_entropy = " << R_entropy << endl;
   cout << "G_entropy = " << G_entropy << endl;
   cout << "B_entropy = " << B_entropy << endl;
}

// ---------------------------------------------------------------------
// Member function RGB_entropy_integral_images() computes RGB integral
// entropy images from the current RGB image.  

void texture_rectangle::RGB_entropy_integral_images(
   twoDarray* SrtwoDarray_ptr, twoDarray* SgtwoDarray_ptr, 
   twoDarray* SbtwoDarray_ptr)
{
//   cout << "inside texture_rectangle::RGB_entropy_integral_images()" << endl;

   int xdim = getWidth();
   int ydim = getHeight();
   int R, G, B;
   double r, g, b;
   double sr, sg, sb;

   for(int py = 0; py < ydim; py++)
   {
      for(int px = 0; px < xdim; px++)
      {
         fast_get_pixel_RGB_values(px, py, R, G, B);
         if(R <= 0) 
         {
            sr = 0;
         }
         else
         {
            r = R/255.0;
            sr = -r * log(r);
         }

         if(G <= 0) 
         {
            sg = 0;
         }
         else
         {
            g = G/255.0;
            sg = -g * log(g);
         }

         if(B <= 0) 
         {
            sb = 0;
         }
         else
         {
            b = B/255.0;
            sb = -b * log(b);
         }

         if(px >= 1)
         {
            sr += SrtwoDarray_ptr->get(px-1, py);
            sg += SgtwoDarray_ptr->get(px-1, py);
            sb += SbtwoDarray_ptr->get(px-1, py);
         }
         if(py >= 1)
         {
            sr += SrtwoDarray_ptr->get(px, py-1);
            sg += SgtwoDarray_ptr->get(px, py-1);
            sb += SbtwoDarray_ptr->get(px, py-1);
         }
         if(px >= 1 && py >= 1)
         {
            sr -= SrtwoDarray_ptr->get(px-1, py-1);
            sg -= SgtwoDarray_ptr->get(px-1, py-1);
            sb -= SbtwoDarray_ptr->get(px-1, py-1);
         }

         
//         if(sr < 0) cout << "px = " << px << " py = " << py 
//                         << " sr = " << sr << endl;
//         if(sg < 0) cout << "px = " << px << " py = " << py 
//                         << " sg = " << sg << endl;
//         if(sb < 0) cout << "px = " << px << " py = " << py 
//                         << " sb = " << sb << endl;
         
         SrtwoDarray_ptr->put(px,py,sr);
         SgtwoDarray_ptr->put(px,py,sg);
         SbtwoDarray_ptr->put(px,py,sb);

      } // loop over px
   } // loop over py
}

// ---------------------------------------------------------------------
// Member function bbox_RGB_entropies() returns the R, G, B color
// channel entropies for pixels within the bbox specified by
// horizontal and vertical limits.

void texture_rectangle::bbox_RGB_entropies(
   int px_lo, int px_hi, int py_lo, int py_hi,
   twoDarray* SrtwoDarray_ptr, twoDarray* SgtwoDarray_ptr, 
   twoDarray* SbtwoDarray_ptr, double& Sr, double& Sg, double& Sb)
{
   Sr = SrtwoDarray_ptr->get(px_hi, py_hi) + 
      SrtwoDarray_ptr->get(px_lo, py_lo) - 
      SrtwoDarray_ptr->get(px_hi, py_lo) - 
      SrtwoDarray_ptr->get(px_lo, py_hi);

   Sg = SgtwoDarray_ptr->get(px_hi, py_hi) + 
      SgtwoDarray_ptr->get(px_lo, py_lo) - 
      SgtwoDarray_ptr->get(px_hi, py_lo) - 
      SgtwoDarray_ptr->get(px_lo, py_hi);

   Sb = SbtwoDarray_ptr->get(px_hi, py_hi) + 
      SbtwoDarray_ptr->get(px_lo, py_lo) - 
      SbtwoDarray_ptr->get(px_hi, py_lo) - 
      SbtwoDarray_ptr->get(px_lo, py_hi);

   if(Sr < 0) Sr = 0;
   if(Sg < 0) Sg = 0;
   if(Sb < 0) Sb = 0;
}

// ========================================================================
// Lode's computer graphics tutorial methods
// Adapted from lodev.org/cgtutor/floodfill.html
// ========================================================================

// Stack functions

bool texture_rectangle::pop(unsigned int& px,unsigned int& py)
{
   if (stackPointer > 0)
   {
      unsigned int p = stack[stackPointer];
      px = p / getHeight();
      py = p % getHeight();
      stackPointer--;
      return 1;
   }    
   else
   {
      return 0;
   }   
}   
 
bool texture_rectangle::push(unsigned int px,unsigned int py)
{
   if (stackPointer < stackSize - 1)
   {
      stackPointer++;
      stack[stackPointer] = getHeight() * px + py;
      return 1;
   }    
   else
   {
      return 0;
   }   
}    

void texture_rectangle::emptyStack()
{
   unsigned int px, py;
   while (pop(px, py));
}

// ---------------------------------------------------------------------
// Member function floodfill() takes in some initial RGB values and
// pixel coords px,py.  It also takes in classifier RGB values within 
// flood_R, flood_G and flood_B.  This method recursively searches for
// neighboring pixels whose RGB values are locally close to their
// nearest 4-neighbors and also within global_threshold of the
// starting init_R, init_G and init_B values.  It returns all flooded
// pixel coordinates within STL vector filled_pixels and their
// original RGB values within STL vector encountered_RGBs.

void texture_rectangle::floodFill(
   unsigned int px,unsigned int py,int flood_R,int flood_G,int flood_B,
   int init_R,int init_G,int init_B,
   double local_threshold,double global_threshold,
   vector<pair<int,int> >& filled_pixels,
   vector<threevector>& encountered_RGBs)
{
//   cout << "inside texture_rectangle::floodFill() #1" << endl;
   encountered_RGBs.clear();

//   check_all_pixel_RGB_values();
//   outputfunc::enter_continue_char();

   if ( (init_R==flood_R) && (init_G==flood_G) && (init_B==flood_B)) return;
   // avoid infinite loop

   emptyStack();
      
   if (!push(px,py)) return; 

   int R,G,B;
   int prev_R,prev_G,prev_B;
   while (pop(px,py))
   {
      get_pixel_RGB_values(px,py,prev_R,prev_G,prev_B);
//      cout << "px = " << px << " py = " << py
//           << " prev_R = " << prev_R
//           << " prev_G = " << prev_G
//           << " prev_B = " << prev_B
//           << endl;

      if (prev_R==flood_R && prev_G==flood_G && prev_B==flood_B)
      {
      }
      else
      {
         encountered_RGBs.push_back(threevector(prev_R,prev_G,prev_B));
      }

      pair<int,int> P(px,py);
      filled_pixels.push_back(P);

      set_pixel_RGB_values(px,py,flood_R,flood_G,flood_B);

      if (px+1 < getWidth())
      {
         get_pixel_RGB_values(px+1,py,R,G,B);
         if (R != flood_R  || G != flood_G  ||  B != flood_B)
         {
            if (colorfunc::color_match(
                   R,G,B,init_R,init_G,init_B,global_threshold) &&
                colorfunc::color_match(R,G,B,prev_R,prev_G,prev_B,
                                       local_threshold))
            {          
               if (!push(px+1,py)) return;           
            }    
         }
      }

      if (px-1 >= 0)
      {
         get_pixel_RGB_values(px-1,py,R,G,B);
         if (R != flood_R  || G != flood_G  ||  B != flood_B)
         {
            if (colorfunc::color_match(
                   R,G,B,init_R,init_G,init_B,global_threshold) &&
                colorfunc::color_match(R,G,B,prev_R,prev_G,prev_B,
                                       local_threshold))
            {          
               if (!push(px-1,py)) return;           
            }    
         }
      }
      
      if (py+1 < getHeight())
      {
         get_pixel_RGB_values(px,py+1,R,G,B);
         if (R != flood_R  || G != flood_G  ||  B != flood_B)
         {
            if (colorfunc::color_match(
                   R,G,B,init_R,init_G,init_B,global_threshold) &&
                colorfunc::color_match(R,G,B,prev_R,prev_G,prev_B,
                                       local_threshold))
            {
               if (!push(px,py+1)) return;           
            }    
         }
      }
      
      if (py-1 >= 0)
      {
         get_pixel_RGB_values(px,py-1,R,G,B);
         if (R != flood_R  || G != flood_G  ||  B != flood_B)
         {
            if (colorfunc::color_match(
                   R,G,B,init_R,init_G,init_B,global_threshold) &&
                colorfunc::color_match(R,G,B,prev_R,prev_G,prev_B,
                                       local_threshold))
            {
               if (!push(px,py-1)) return;           
            }    
         }
      }
      
   } // while loop
}

// ---------------------------------------------------------------------
// This overloaded version of floodfill() takes in pixel coords px,py
// for some starting seed location along with an un-segmented image
// within *filtered_texture_rectangle_ptr.  It retrieves or computes
// 1st and 2nd RGB color-space moments within a pixel patch
// surrounding (px,py). This method then recursively searches for
// neighboring pixel patches whose 1st and 2nd RGB moments are locally
// close to their nearest 4-neighbors and also globally reasonably
// close to the starting pixel patch's moments.  It returns all
// flooded pixel coordinates within *segmentation_mask_twoDarray_ptr.

void texture_rectangle::floodFill(
   texture_rectangle* filtered_texture_rectangle_ptr,
   twoDarray* segmentation_mask_twoDarray_ptr,
   twoDarray* mu_R_twoDarray_ptr,
   twoDarray* mu_G_twoDarray_ptr,
   twoDarray* mu_B_twoDarray_ptr,
   twoDarray* sigma_R_twoDarray_ptr,
   twoDarray* sigma_G_twoDarray_ptr,
   twoDarray* sigma_B_twoDarray_ptr,
   unsigned int px,unsigned int py,int flood_R,int flood_G,int flood_B,
   int bbox_size,double local_mu_threshold,double global_mu_threshold,
   double local_sigma_threshold,double global_sigma_threshold)
{
//   cout << "inside texture_rectangle::floodFill() #2" << endl;

   emptyStack();
   if (!push(px,py)) return; 

// Compute 1st & 2nd RGB moments within patch surrounding starting
// pixel's location:

   double init_mu_R,init_mu_G,init_mu_B;
   double init_sigma_R,init_sigma_G,init_sigma_B;
   get_pixel_region_RGB_moments(
      px,py,bbox_size,
      mu_R_twoDarray_ptr,mu_G_twoDarray_ptr,mu_B_twoDarray_ptr,
      sigma_R_twoDarray_ptr,sigma_G_twoDarray_ptr,sigma_B_twoDarray_ptr,
      init_mu_R,init_mu_G,init_mu_B,
      init_sigma_R,init_sigma_G,init_sigma_B);

   int d_px=1;
//   int d_px=2;
   int d_py=d_px;
   int R,G,B;
   double mu_R,mu_G,mu_B,sigma_R,sigma_G,sigma_B;
   double prev_mu_R,prev_mu_G,prev_mu_B;
   double prev_sigma_R,prev_sigma_G,prev_sigma_B;

   while (pop(px,py))
   {
      filtered_texture_rectangle_ptr->get_pixel_region_RGB_moments(
         px,py,bbox_size,
         mu_R_twoDarray_ptr,mu_G_twoDarray_ptr,mu_B_twoDarray_ptr,
         sigma_R_twoDarray_ptr,sigma_G_twoDarray_ptr,sigma_B_twoDarray_ptr,
         prev_mu_R,prev_mu_G,prev_mu_B,
         prev_sigma_R,prev_sigma_G,prev_sigma_B);

      set_pixel_RGB_values(px,py,flood_R,flood_G,flood_B);
      segmentation_mask_twoDarray_ptr->put(px,py,1);

      if (px+d_px < getWidth())
      {
         get_pixel_RGB_values(px+d_px,py,R,G,B);
         if (R != flood_R  || G != flood_G || B != flood_B)
         {
            filtered_texture_rectangle_ptr->get_pixel_region_RGB_moments(
               px+d_px,py,bbox_size,
               mu_R_twoDarray_ptr,mu_G_twoDarray_ptr,mu_B_twoDarray_ptr,
               sigma_R_twoDarray_ptr,sigma_G_twoDarray_ptr,
               sigma_B_twoDarray_ptr,
               mu_R,mu_G,mu_B,sigma_R,sigma_G,sigma_B);

            if (colorfunc::color_match(
                   mu_R,mu_G,mu_B,init_mu_R,init_mu_G,init_mu_B,
                   global_mu_threshold) &&
                colorfunc::color_match(
                   mu_R,mu_G,mu_B,prev_mu_R,prev_mu_G,prev_mu_B,
                   local_mu_threshold) &&
                colorfunc::color_match(
                   sigma_R,sigma_G,sigma_B,init_sigma_R,
                   init_sigma_G,init_sigma_B,global_sigma_threshold) &&
                colorfunc::color_match(
                   sigma_R,sigma_G,sigma_B,prev_sigma_R,
                   prev_sigma_G,prev_sigma_B,local_sigma_threshold))
            {          
               if (!push(px+d_px,py)) return;
            }    
         }
      }

      if (px-d_px >= 0)
      {
         get_pixel_RGB_values(px-d_px,py,R,G,B);
         if (R != flood_R  || G != flood_G || B != flood_B)
         {
            filtered_texture_rectangle_ptr->get_pixel_region_RGB_moments(
               px-d_px,py,bbox_size,
               mu_R_twoDarray_ptr,mu_G_twoDarray_ptr,mu_B_twoDarray_ptr,
               sigma_R_twoDarray_ptr,sigma_G_twoDarray_ptr,
               sigma_B_twoDarray_ptr,
               mu_R,mu_G,mu_B,sigma_R,sigma_G,sigma_B);

            if (colorfunc::color_match(
                   mu_R,mu_G,mu_B,init_mu_R,init_mu_G,init_mu_B,
                   global_mu_threshold) &&
                colorfunc::color_match(
                   mu_R,mu_G,mu_B,prev_mu_R,prev_mu_G,prev_mu_B,
                   local_mu_threshold) &&
                colorfunc::color_match(
                   sigma_R,sigma_G,sigma_B,init_sigma_R,
                   init_sigma_G,init_sigma_B,global_sigma_threshold) &&
                colorfunc::color_match(
                   sigma_R,sigma_G,sigma_B,prev_sigma_R,
                   prev_sigma_G,prev_sigma_B,local_sigma_threshold))

            {          
               if (!push(px-d_px,py)) return;           
            }
         }    
      }
      
      if (py+d_py < getHeight())
      {
         get_pixel_RGB_values(px,py+d_py,R,G,B);
         if (R != flood_R  || G != flood_G || B != flood_B)
         {
            filtered_texture_rectangle_ptr->get_pixel_region_RGB_moments(
               px,py+d_py,bbox_size,
               mu_R_twoDarray_ptr,mu_G_twoDarray_ptr,mu_B_twoDarray_ptr,
               sigma_R_twoDarray_ptr,sigma_G_twoDarray_ptr,
               sigma_B_twoDarray_ptr,
               mu_R,mu_G,mu_B,sigma_R,sigma_G,sigma_B);

            if (colorfunc::color_match(
                   mu_R,mu_G,mu_B,init_mu_R,init_mu_G,init_mu_B,
                   global_mu_threshold) &&
                colorfunc::color_match(
                   mu_R,mu_G,mu_B,prev_mu_R,prev_mu_G,prev_mu_B,
                   local_mu_threshold) &&
                colorfunc::color_match(
                   sigma_R,sigma_G,sigma_B,init_sigma_R,
                   init_sigma_G,init_sigma_B,global_sigma_threshold) &&
                colorfunc::color_match(
                   sigma_R,sigma_G,sigma_B,prev_sigma_R,
                   prev_sigma_G,prev_sigma_B,local_sigma_threshold))

            {
               if (!push(px,py+d_py)) return;           
            }    
         }
      }
      
      if (py-d_py >= 0)
      {
         get_pixel_RGB_values(px,py-d_py,R,G,B);
         if (R != flood_R  || G != flood_G || B != flood_B)
         {
            filtered_texture_rectangle_ptr->get_pixel_region_RGB_moments(
               px,py-d_py,bbox_size,
               mu_R_twoDarray_ptr,mu_G_twoDarray_ptr,mu_B_twoDarray_ptr,
               sigma_R_twoDarray_ptr,sigma_G_twoDarray_ptr,
               sigma_B_twoDarray_ptr,
               mu_R,mu_G,mu_B,sigma_R,sigma_G,sigma_B);

            if (colorfunc::color_match(
                   mu_R,mu_G,mu_B,init_mu_R,init_mu_G,init_mu_B,
                   global_mu_threshold) &&
                colorfunc::color_match(
                   mu_R,mu_G,mu_B,prev_mu_R,prev_mu_G,prev_mu_B,
                   local_mu_threshold) &&
                colorfunc::color_match(
                   sigma_R,sigma_G,sigma_B,init_sigma_R,
                   init_sigma_G,init_sigma_B,global_sigma_threshold) &&
                colorfunc::color_match(
                   sigma_R,sigma_G,sigma_B,prev_sigma_R,
                   prev_sigma_G,prev_sigma_B,local_sigma_threshold))
            {
               if (!push(px,py-d_py)) return;           
            }    
         }
      }
      
   } // while loop
}

// ---------------------------------------------------------------------
// Member function find_interior_median_RGBs() takes in extremal
// region *extremal_region_ptr.  After decoding its run-length encoded
// pixels, this method computes and returns the median R, G and B
// values for all pixels lying inside the region's interior.

threevector texture_rectangle::find_interior_median_RGBs(
   const extremal_region* extremal_region_ptr)
   
{
//   cout << "inside texture_rectangle::find_interior_median_RGBs()" << endl;

   unsigned int px_start,px_stop,py_start,py_stop;
   int R,G,B;
   vector<double> interior_R,interior_G,interior_B;

// Look for pixels located at horizontal ends of pixel "runs" which
// belong to extremal region's perimeter:

   for (unsigned int i=0; i<extremal_region_ptr->get_RLE_pixel_IDs().size(); 
        i += 2)
   {
      unsigned int start_pixel_ID=
         extremal_region_ptr->get_RLE_pixel_IDs().at(i);
      unsigned int stop_pixel_ID=
         extremal_region_ptr->get_RLE_pixel_IDs().at(i+1);

      graphicsfunc::get_pixel_px_py(
         start_pixel_ID,getWidth(),px_start,py_start);
      if (px_start > 0)
      {
         get_pixel_RGB_values(px_start-1,py_start,R,G,B);
         interior_R.push_back(R);
         interior_G.push_back(G);
         interior_B.push_back(B);
      }

      graphicsfunc::get_pixel_px_py(
         stop_pixel_ID,getWidth(),px_stop,py_stop);
      if (px_start < getWidth()-1)
      {
         get_pixel_RGB_values(px_stop+1,py_stop,R,G,B);
         interior_R.push_back(R);
         interior_G.push_back(G);
         interior_B.push_back(B);
      }
   }

   double median_R=mathfunc::median_value(interior_R);
   double median_G=mathfunc::median_value(interior_G);
   double median_B=mathfunc::median_value(interior_B);

   return threevector(median_R,median_G,median_B);
}

// ---------------------------------------------------------------------
// Member function find_perimeter_seeds() takes in horizontal pixel
// run information for a particular extremal region as well as its
// connected component binary mask.  It searches for
// perimeter pixels that lie precisely outside the connected
// component.  This method computes the median RGB for all perimeter
// pixels.  All perimeter pixels whose RGB values are reasonably close
// to the median are then returned within STL vector perim_seeds.

double texture_rectangle::find_perimeter_seeds(
   extremal_region* extremal_region_ptr,
   const twoDarray* cc_twoDarray_ptr,vector<twovector>& perim_seeds,
   threevector& median_perim_RGB)
{
//   cout << "inside texture_rectangle::find_perimeter_seeds()" << endl;

   unsigned int px_start,px_stop,py_start,py_stop;
   int R,G,B;
   vector<double> perim_R,perim_G,perim_B;
   vector<twovector> perim_pixels;

   unsigned int min_px,min_py,max_px,max_py;
   extremal_region_ptr->get_bbox(min_px,min_py,max_px,max_py);

//   twoDarray* perim_twoDarray_ptr=new twoDarray(getWidth(),getHeight());
//   perim_twoDarray_ptr->clear_values();

// Look for pixels located at horizontal ends of pixel "runs" which
// belong to extremal region's perimeter:

   for (unsigned int i=0; i<extremal_region_ptr->get_RLE_pixel_IDs().size(); 
        i += 2)
   {
      int start_pixel_ID=extremal_region_ptr->get_RLE_pixel_IDs().at(i);
      int stop_pixel_ID=extremal_region_ptr->get_RLE_pixel_IDs().at(i+1);

      graphicsfunc::get_pixel_px_py(
         start_pixel_ID,getWidth(),px_start,py_start);
      if (px_start > 1)
      {
         perim_pixels.push_back(twovector(px_start-2,py_start));         
         get_pixel_RGB_values(px_start-2,py_start,R,G,B);
         perim_R.push_back(R);
         perim_G.push_back(G);
         perim_B.push_back(B);
//         perim_twoDarray_ptr->put(px_start-1,py_start,255);
      }

      graphicsfunc::get_pixel_px_py(
         stop_pixel_ID,getWidth(),px_stop,py_stop);
      if (px_stop < getWidth()-2)
      {
         perim_pixels.push_back(twovector(px_stop+2,py_stop));         
         get_pixel_RGB_values(px_stop+2,py_stop,R,G,B);
         perim_R.push_back(R);
         perim_G.push_back(G);
         perim_B.push_back(B);
//         perim_twoDarray_ptr->put(px_stop+1,py_start,255);
      }
   }

// We also need to search in the vertical directions for extremal
// region perimeter pixels:

   for (unsigned int px=min_px; px<max_px; px++)
   {
      for (unsigned int py=0; py<max_py; py++)
      {
         int pixel_value=cc_twoDarray_ptr->get(px,py);
         if (pixel_value > 128)
         {
            if (py > 1)
            {
               perim_pixels.push_back(twovector(px,py-2));         
               get_pixel_RGB_values(px,py-2,R,G,B);
               perim_R.push_back(R);
               perim_G.push_back(G);
               perim_B.push_back(B);
//               perim_twoDarray_ptr->put(px,py-1,255);
            }
            break;
         }
      } // loop over py starting from 0

      for (unsigned int py=getHeight()-1; py>=min_py; py--)
      {
         int pixel_value=cc_twoDarray_ptr->get(px,py);
         if (pixel_value > 128)
         {
            if (py < getHeight()-3)
            {
               perim_pixels.push_back(twovector(px,py+2));         
               get_pixel_RGB_values(px,py+2,R,G,B);
               perim_R.push_back(R);
               perim_G.push_back(G);
               perim_B.push_back(B);
//               perim_twoDarray_ptr->put(px,py+1,255);
            }
            break;
         }
      } // loop over py starting ndim-1
   } // loop over px

/*
  initialize_twoDarray_image(perim_twoDarray_ptr,3);
  string perim_filename="perim.jpg";
  write_curr_frame(perim_filename);
  delete perim_twoDarray_ptr;
  outputfunc::enter_continue_char();
*/

   double median_R=mathfunc::median_value(perim_R);
   double median_G=mathfunc::median_value(perim_G);
   double median_B=mathfunc::median_value(perim_B);
   median_perim_RGB=threevector(median_R,median_G,median_B);
   
//   cout << "Median perimeter RGB: " 
//        << median_R << " , " << median_G << " , " << median_B << endl;

// Loop over all perimeter pixels.  Retain those whose color is
// reasonably close to averaged exterior RGB values:

   for (unsigned int i=0; i<perim_R.size(); i++)
   {
//      double threshold=10;
//      double threshold=15;
      double threshold=20;
//      double threshold=25;

//      cout << "i = " << i 
//           << " R = " << perim_RGBs[i].get(0)
//           << " G = " << perim_RGBs[i].get(1)
//           << " B = " << perim_RGBs[i].get(2)
//           << endl;
      if (colorfunc::color_match(
             perim_R[i],perim_G[i],perim_B[i],
             median_R,median_G,median_B,threshold))
      {
         double px=perim_pixels[i].get(0);
         double py=perim_pixels[i].get(1);
         perim_seeds.push_back(twovector(px,py));
      }
   }

   double perim_color_frac=double(perim_seeds.size())/
      double(perim_R.size());
//   cout << "perim_color_frac = " << perim_color_frac << endl;
//   outputfunc::enter_continue_char();

   return perim_color_frac;
}

// ---------------------------------------------------------------------
// Member floodfill_color_region_bbox() takes in pixel coordinates for some
// seed location.  If the seed's color already matches
// flood_R,flood_G,flood_B, this method returns.  Otherwise, it flood-fills
// all pixels around the seed whose RGB values are globally close to
// the seed's and whose differential RGB changes are relatively small.
// The input bounding box *symbol_bbox_ptr is updated by each
// returned flood-filled pixel's image location.

void texture_rectangle::floodfill_color_region_bbox(
   unsigned int seed_pu,unsigned int seed_pv,bounding_box* symbol_bbox_ptr)
{
//   cout << "inside texture_rectangle::floodfill_color_region_bbox()" << endl;
//   cout << "seed_pu = " << seed_pu << " seed_pv = " << seed_pv << endl;
   
   const int flood_R=255;
   const int flood_G=0;
   const int flood_B=255;

   int curr_R,curr_G,curr_B;
   get_pixel_RGB_values(seed_pu,seed_pv,curr_R,curr_G,curr_B);
   if (curr_R==flood_R && curr_G==flood_G && curr_B==flood_B) 
   {
//      cout << "Seed position previously analyzed" << endl;
      return;
   }
   
   const double local_threshold=15;
   const double global_threshold=30;

//   const double local_threshold=20;
//   const double global_threshold=40;

//   const double local_threshold=25;
//   const double global_threshold=50;

   vector<pair<int,int> > filled_pixels;
   vector<threevector> encountered_RGBs;
   floodFill(
      seed_pu,seed_pv,flood_R,flood_G,flood_B,curr_R,curr_G,curr_B,
      local_threshold,global_threshold,
      filled_pixels,encountered_RGBs);
//   cout << "filled_pixels.size() = " << filled_pixels.size() 
//        << " encountered_RGBs.size() = " << encountered_RGBs.size() 
//        << endl;

// Wrap bounding box around flooded region:

//   double initial_bbox_area=symbol_bbox_ptr->get_area();
   for (unsigned int i=0; i<filled_pixels.size(); i++)
   {
      double px=filled_pixels[i].first;
      double py=filled_pixels[i].second;
      symbol_bbox_ptr->update_bounds(px,py);
   }
//   double final_bbox_area=symbol_bbox_ptr->get_area();
//   cout << "symbol_bbox = " << *symbol_bbox_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function floodfill_black_pixels() 

void texture_rectangle::floodfill_black_pixels(
   int flood_R,int flood_G,int flood_B,
   double max_seed_v_threshold,double black_v,double darkgrey_v)
{
   cout << "inside texture_rectangle::floodfill_black_pixels()" << endl;
   vector<Triple<int,int,int> > black_pixel_seeds;

   double h,s,v;
   for (unsigned int pu=1; pu<getWidth()-1; pu++)
   {
      for (unsigned int pv=1; pv<getHeight()-1; pv++)
      {
         get_pixel_hsv_values(pu,pv,h,s,v);
         if (v > max_seed_v_threshold) continue;
         
         Triple<int,int,int> seed_black_pixel(pu,pv,-1);
         black_pixel_seeds.push_back(seed_black_pixel);

      } // loop over pv
   } // loop over pu
   cout << "black_pixel_seeds.size() = " << black_pixel_seeds.size() << endl;

   int R,G,B;
   vector<pair<int,int> > filled_pixels;
   vector<threevector> encountered_RGBs;

   for (unsigned int j=0; j<black_pixel_seeds.size(); j++)
   {
      if (j%1000==0) cout << j << " " << flush;
      int pu=black_pixel_seeds[j].first;
      int pv=black_pixel_seeds[j].second;
      get_pixel_RGB_values(pu,pv,R,G,B);
      if (R==flood_R && G==flood_G && B==flood_B) continue;

//      const double black_v=0.12;
//      const double darkgrey_v=0.22;
      floodDarkGrey(
         pu,pv,flood_R,flood_G,flood_B,R,G,B,
         black_v,darkgrey_v,
         filled_pixels,encountered_RGBs);
   } // loop over index j labeling black pixel seeds
   cout << endl;

   cout << "filled_pixels.size() = " << filled_pixels.size() << endl;
}

// ---------------------------------------------------------------------
// Member function floodDarkGrey() takes in some initial RGB values and
// pixel coords px,py.  

void texture_rectangle::floodDarkGrey(
   unsigned int px,unsigned int py,
   int flood_R,int flood_G,int flood_B,
   int init_R,int init_G,int init_B,
   double black_v,double darkgrey_v,
   vector<pair<int,int> >& filled_pixels,
   vector<threevector>& encountered_RGBs)
{
//   cout << "inside floodDarkGrey, px = " << px << " py = " << py << endl;
   
   encountered_RGBs.clear();

   if ( (init_R==flood_R) && (init_G==flood_G) && (init_B==flood_B)) return;
   // avoid infinite loop

   emptyStack();
      
   if (!push(px,py)) return; 

   int R,G,B,prev_R,prev_G,prev_B;
   double h,s,v,prev_h,prev_s,prev_v;
   while (pop(px,py))
   {
      get_pixel_RGBhsv_values(px,py,prev_R,prev_G,prev_B,prev_h,prev_s,prev_v);
//      cout << "px = " << px << " py = " << py
//           << " prev_R = " << prev_R
//           << " prev_G = " << prev_G
//           << " prev_B = " << prev_B
//           << endl;

      if (prev_R==flood_R && prev_G==flood_G && prev_B==flood_B)
      {
      }
      else
      {
         encountered_RGBs.push_back(threevector(prev_R,prev_G,prev_B));
      }

//      cout << "prev_h = " << prev_h
//           << " prev_s = " << prev_s
//           << " prev_v = " << prev_v
//           << endl;

      double darkgrey_delta_h=180;
      if (prev_s > 0.05)
      {
         darkgrey_delta_h=10+250*(darkgrey_v-prev_v)+5.0/prev_s;
      }

      double darkgrey_delta_s=0.1+2.5*(darkgrey_v-prev_v);

      pair<int,int> P(px,py);
      filled_pixels.push_back(P);
      set_pixel_RGB_values(px,py,flood_R,flood_G,flood_B);


      if (px-1 >= 0 && py-1 >= 0)
      {
         get_pixel_RGBhsv_values(px-1,py-1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         bool new_pixel_OK=false;
         if (v < black_v) new_pixel_OK=true;
         if (v < darkgrey_v && fabs(s-prev_s) < darkgrey_delta_s  &&
             fabs(h-prev_h) < darkgrey_delta_h) new_pixel_OK=true;
         if (new_pixel_OK)
         {
            if (!push(px-1,py-1)) return;           
         }
      } // px-1 >= 0 && py-1 >= 0 conditional


      if (py-1 >= 0)
      {
         get_pixel_RGBhsv_values(px,py-1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         bool new_pixel_OK=false;
         if (v < black_v) new_pixel_OK=true;
         if (v < darkgrey_v && fabs(s-prev_s) < darkgrey_delta_s  &&
             fabs(h-prev_h) < darkgrey_delta_h) new_pixel_OK=true;
         if (new_pixel_OK)
         {
            if (!push(px,py-1)) return;           
         }
      } // py-1 >= 0 conditional


      if (px+1 < getWidth() && py-1 >= 0)
      {
         get_pixel_RGBhsv_values(px+1,py-1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         bool new_pixel_OK=false;
         if (v < black_v) new_pixel_OK=true;
         if (v < darkgrey_v && fabs(s-prev_s) < darkgrey_delta_s  &&
             fabs(h-prev_h) < darkgrey_delta_h) new_pixel_OK=true;
         if (new_pixel_OK)
         {
            if (!push(px+1,py-1)) return;           
         }
      } // px+1 < width && py-1 >= 0 conditional


      if (px-1 >= 0)
      {
         get_pixel_RGBhsv_values(px-1,py,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         bool new_pixel_OK=false;
         if (v < black_v) new_pixel_OK=true;
         if (v < darkgrey_v && fabs(s-prev_s) < darkgrey_delta_s  &&
             fabs(h-prev_h) < darkgrey_delta_h) new_pixel_OK=true;
         if (new_pixel_OK)
         {
            if (!push(px-1,py)) return;           
         }
      } // px-1 >= 0 conditional


      if (px+1 < getWidth())
      {
         get_pixel_RGBhsv_values(px+1,py,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         bool new_pixel_OK=false;
         if (v < black_v) new_pixel_OK=true;
         if (v < darkgrey_v && fabs(s-prev_s) < darkgrey_delta_s  &&
             fabs(h-prev_h) < darkgrey_delta_h) new_pixel_OK=true;
         if (new_pixel_OK)
         {
            if (!push(px+1,py)) return;           
         }
      } // px+1 < width conditional


      if (px-1 >= 0 && py+1 < getHeight())
      {
         get_pixel_RGBhsv_values(px-1,py+1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         bool new_pixel_OK=false;
         if (v < black_v) new_pixel_OK=true;
         if (v < darkgrey_v && fabs(s-prev_s) < darkgrey_delta_s  &&
             fabs(h-prev_h) < darkgrey_delta_h) new_pixel_OK=true;
         if (new_pixel_OK)
         {
            if (!push(px-1,py+1)) return;           
         }
      } // px-1 >= 0 and py+1 < height conditional

      
      if (py+1 < getHeight())
      {
         get_pixel_RGBhsv_values(px,py+1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         bool new_pixel_OK=false;
         if (v < black_v) new_pixel_OK=true;
         if (v < darkgrey_v && fabs(s-prev_s) < darkgrey_delta_s  &&
             fabs(h-prev_h) < darkgrey_delta_h) new_pixel_OK=true;
         if (new_pixel_OK)
         {
            if (!push(px,py+1)) return;           
         }
      } // py+1 < height conditional

      if (px+1 < getWidth() && py+1 < getHeight())
      {
         get_pixel_RGBhsv_values(px+1,py+1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         bool new_pixel_OK=false;
         if (v < black_v) new_pixel_OK=true;
         if (v < darkgrey_v && fabs(s-prev_s) < darkgrey_delta_s  &&
             fabs(h-prev_h) < darkgrey_delta_h) new_pixel_OK=true;
         if (new_pixel_OK)
         {
            if (!push(px+1,py+1)) return;           
         }
      } // px+1 < width && py+1 < height conditional



   } // while loop
}


// ---------------------------------------------------------------------
// Member function floodfill_white_pixels() 

void texture_rectangle::floodfill_white_pixels(
   int flood_R,int flood_G,int flood_B)
{
   cout << "inside texture_rectangle::floodfill_white_pixels()" << endl;
   vector<Triple<int,int,int> > white_pixel_seeds;

   double h,s,v;
   double s1,s2,s3,s4,s5,s6,s7,s8;
   double v1,v2,v3,v4,v5,v6,v7,v8;
   for (unsigned int pu=1; pu<getWidth()-1; pu++)
   {
      for (unsigned int pv=1; pv<getHeight()-1; pv++)
      {
         get_pixel_hsv_values(pu,pv,h,s,v);
         if (v < 0.4) continue;
         if (s > 0.4) continue;

         get_pixel_hsv_values(pu-1,pv-1,h,s1,v1);
         get_pixel_hsv_values(pu,pv-1,h,s2,v2);
         get_pixel_hsv_values(pu+1,pv-1,h,s3,v3);

         get_pixel_hsv_values(pu-1,pv,h,s4,v4);
         get_pixel_hsv_values(pu+1,pv,h,s5,v5);

         get_pixel_hsv_values(pu-1,pv+1,h,s6,v6);
         get_pixel_hsv_values(pu,pv+1,h,s7,v7);
         get_pixel_hsv_values(pu+1,pv+1,h,s8,v8);

         int n_dark_grey_neighbors=0;
         if (v1 < 0.3) n_dark_grey_neighbors++;
         if (v2 < 0.3) n_dark_grey_neighbors++;
         if (v3 < 0.3) n_dark_grey_neighbors++;
         if (v4 < 0.3) n_dark_grey_neighbors++;
         if (v5 < 0.3) n_dark_grey_neighbors++;
         if (v6 < 0.3) n_dark_grey_neighbors++;
         if (v7 < 0.3) n_dark_grey_neighbors++;
         if (v8 < 0.3) n_dark_grey_neighbors++;

         if (n_dark_grey_neighbors >= 2)
         {
            Triple<int,int,int> seed_white_pixel(pu,pv,n_dark_grey_neighbors);
            white_pixel_seeds.push_back(seed_white_pixel);
//            set_pixel_RGB_values(pu,pv,255,0,255);
         }
      } // loop over pv
   } // loop over pu
   cout << "white_pixel_seeds.size() = " << white_pixel_seeds.size() << endl;

   int R,G,B;
   vector<pair<int,int> > filled_pixels;
   vector<threevector> encountered_RGBs;

   for (unsigned int j=0; j<white_pixel_seeds.size(); j++)
   {
      if (j%1000==0) cout << j << " " << flush;
      int pu=white_pixel_seeds[j].first;
      int pv=white_pixel_seeds[j].second;
      get_pixel_RGB_values(pu,pv,R,G,B);
      if (R==flood_R && G==flood_G && B==flood_B) continue;

      floodLightGrey(
         pu,pv,flood_R,flood_G,flood_B,R,G,B,
         filled_pixels,encountered_RGBs);
   } // loop over index j labeling white pixel seeds
   cout << endl;

   cout << "filled_pixels.size() = " << filled_pixels.size() << endl;
}


// ---------------------------------------------------------------------
// Member function floodLightGrey() takes in some initial RGB values and
// pixel coords px,py.  

void texture_rectangle::floodLightGrey(
   unsigned int px,unsigned int py,
   int flood_R,int flood_G,int flood_B,
   int init_R,int init_G,int init_B,
   vector<pair<int,int> >& filled_pixels,
   vector<threevector>& encountered_RGBs)
{
//   cout << "inside floodLightGrey, px = " << px << " py = " << py << endl;
   
   encountered_RGBs.clear();

   if ( (init_R==flood_R) && (init_G==flood_G) && (init_B==flood_B)) return;
   // avoid infinite loop

   emptyStack();
      
   if (!push(px,py)) return; 

   int R,G,B,prev_R,prev_G,prev_B;
   double h,s,v,prev_h,prev_s,prev_v;
   while (pop(px,py))
   {
      get_pixel_RGBhsv_values(px,py,prev_R,prev_G,prev_B,prev_h,prev_s,prev_v);
//      cout << "px = " << px << " py = " << py
//           << " prev_R = " << prev_R
//           << " prev_G = " << prev_G
//           << " prev_B = " << prev_B
//           << endl;

      if (prev_R==flood_R && prev_G==flood_G && prev_B==flood_B)
      {
      }
      else
      {
         encountered_RGBs.push_back(threevector(prev_R,prev_G,prev_B));
      }

//      cout << "prev_h = " << prev_h
//           << " prev_s = " << prev_s
//           << " prev_v = " << prev_v
//           << endl;

      double lightgrey_delta_h=180;
      if (prev_s > 0.0277)
      {
         lightgrey_delta_h=5.0/prev_s;
      }

      double lightgrey_v=0.35;
      double lightgrey_delta_s=0.2;
      double lightgrey_delta_v=0.2+0.3*(v-lightgrey_v);

      pair<int,int> P(px,py);
      filled_pixels.push_back(P);
      set_pixel_RGB_values(px,py,flood_R,flood_G,flood_B);

      if (px-1 >= 0 && py-1 >= 0)
      {
         get_pixel_RGBhsv_values(px-1,py-1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         double lightgrey_s=0.45-0.075*v;
         if (s < lightgrey_s && v > lightgrey_v && 
             fabs(h-prev_h) < lightgrey_delta_h &&
             fabs(s-prev_s) < lightgrey_delta_s  &&
             fabs(v-prev_v) < lightgrey_delta_v) 
         {
            if (!push(px-1,py-1)) return;           
         }
      } // px-1 >= 0 && py-1 >= 0 conditional

      if (py-1 >= 0)
      {
         get_pixel_RGBhsv_values(px,py-1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         double lightgrey_s=0.45-0.075*v;
         if (s < lightgrey_s && v > lightgrey_v && 
             fabs(h-prev_h) < lightgrey_delta_h &&
             fabs(s-prev_s) < lightgrey_delta_s  &&
             fabs(v-prev_v) < lightgrey_delta_v) 
         {
            if (!push(px,py-1)) return;           
         }
      } // py-1 >= 0 conditional

      if (px+1 < getWidth() && py-1 >= 0)
      {
         get_pixel_RGBhsv_values(px+1,py-1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         double lightgrey_s=0.45-0.075*v;
         if (s < lightgrey_s && v > lightgrey_v && 
             fabs(h-prev_h) < lightgrey_delta_h &&
             fabs(s-prev_s) < lightgrey_delta_s  &&
             fabs(v-prev_v) < lightgrey_delta_v) 
         {
            if (!push(px+1,py-1)) return;           
         }
      } // px+1 < width && py-1 >= 0 conditional

      if (px-1 >= 0)
      {
         get_pixel_RGBhsv_values(px-1,py,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         double lightgrey_s=0.45-0.075*v;
         if (s < lightgrey_s && v > lightgrey_v && 
             fabs(h-prev_h) < lightgrey_delta_h &&
             fabs(s-prev_s) < lightgrey_delta_s  &&
             fabs(v-prev_v) < lightgrey_delta_v) 
         {
            if (!push(px-1,py)) return;           
         }
      } // px-1 >= 0 conditional

      if (px+1 < getWidth())
      {
         get_pixel_RGBhsv_values(px+1,py,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         double lightgrey_s=0.45-0.075*v;
         if (s < lightgrey_s && v > lightgrey_v && 
             fabs(h-prev_h) < lightgrey_delta_h &&
             fabs(s-prev_s) < lightgrey_delta_s  &&
             fabs(v-prev_v) < lightgrey_delta_v) 
         {
            if (!push(px+1,py)) return;           
         }
      } // px+1 < width conditional

      if (px-1 >= 0 && py+1 < getHeight())
      {
         get_pixel_RGBhsv_values(px-1,py+1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         double lightgrey_s=0.45-0.075*v;
         if (s < lightgrey_s && v > lightgrey_v && 
             fabs(h-prev_h) < lightgrey_delta_h &&
             fabs(s-prev_s) < lightgrey_delta_s  &&
             fabs(v-prev_v) < lightgrey_delta_v) 
         {
            if (!push(px-1,py+1)) return;           
         }
      } // px-1 >= 0 and py+1 < height conditional
      
      if (py+1 < getHeight())
      {
         get_pixel_RGBhsv_values(px,py+1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         double lightgrey_s=0.45-0.075*v;
         if (s < lightgrey_s && v > lightgrey_v && 
             fabs(h-prev_h) < lightgrey_delta_h &&
             fabs(s-prev_s) < lightgrey_delta_s  &&
             fabs(v-prev_v) < lightgrey_delta_v) 
         {
            if (!push(px,py+1)) return;           
         }
      } // py+1 < height conditional

      if (px+1 < getWidth() && py+1 < getHeight())
      {
         get_pixel_RGBhsv_values(px+1,py+1,R,G,B,h,s,v);
         h=basic_math::phase_to_canonical_interval(h,prev_h-180,prev_h+180);

         double lightgrey_s=0.45-0.075*v;
         if (s < lightgrey_s && v > lightgrey_v && 
             fabs(h-prev_h) < lightgrey_delta_h &&
             fabs(s-prev_s) < lightgrey_delta_s  &&
             fabs(v-prev_v) < lightgrey_delta_v) 
         {
            if (!push(px+1,py+1)) return;           
         }
      } // px+1 < width && py+1 < height conditional


   } // while loop
}





// ========================================================================
// Integral image methods
// ========================================================================

/***** p_integral_imd ****************************************************************
 * Description: Computes the integral image of im.
 *
 * inputs: int_im : resulting integral image, allocated beforhand
 *                  size needs to be +1 in each dimension compared to im
 *         im : input image
 *
 * Output: none
 *
 ******************************************************************************************/

/*
  void p_integral_imd(p_imaged *int_im, p_imagef *im)
  {
  int x, y;
  float *X;
  const int xs = im->x_size;
  const int ys = im->y_size;

  double *A, *B, *C, *D, *tmp;
  tmp = int_im->mem;
  for(x = 0;  x < int_im->x_size; x++){
  *tmp = 0;
  ++tmp;
  }
  tmp = int_im->mem;
  for(y = 0; y < int_im->y_size; y++){
  *tmp = 0;
  tmp += int_im->pitch;
  }
  for(y = 0; y < ys; y++){
  A = int_im->mem + y * int_im->pitch;
  B = A + 1;
  C = A + int_im->pitch;
  D = C + 1;
  X = im->mem + y * im->pitch;
  for(x = 0; x < xs; x++){
  *D = (*X + *B + *C) - *A;
  A = B;
  C = D;
  ++B;
  ++D;
  ++X;
  }
  }
  }
*/


/***** p_integral_imd_rec ****************************************************************
 * Description: Returns the sum of the rectangle given by upper left and lower right coordinates/
 *
 * inputs: int_im : Integral image
 *         ulx, uly : upper left  x and y coordinate (with respect to original image)
 *         lrx, lry : lower right x and y coordinate (with respect to original image)
 *
 * Output: Sum of the values inside the original rectangle
 *
 ******************************************************************************************/


/*
  float p_integral_imd_rec(p_imaged *int_im, int ulx, int uly, int lrx, int lry){
  return P_PIX(int_im, lrx + 1, lry + 1) + P_PIX(int_im, ulx, uly) - P_PIX(int_im, lrx + 1, uly) - P_PIX(int_im, ulx, lry +1);
  }

*/


