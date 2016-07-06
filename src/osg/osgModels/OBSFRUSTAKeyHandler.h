// ==========================================================================
// OBSFRUSTAKeyHandler header file 
// ==========================================================================
// Last modified on 8/19/09; 10/3/09; 2/1/12
// ==========================================================================

#ifndef OBS_FRUSTAKEYHANDLER_H
#define OBS_FRUSTAKEYHANDLER_H

#include <vector>
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgGraphicals/GraphicalsKeyHandler.h"
#include "osg/osgGeometry/PolyhedraGroup.h"

class ModeController;
class Movie;
class OBSFRUSTAGROUP;

class OBSFRUSTAKeyHandler : public GraphicalsKeyHandler
{
  public:

   OBSFRUSTAKeyHandler(OBSFRUSTAGROUP* OFG_ptr,ModeController* MC_ptr);
   OBSFRUSTAKeyHandler(OBSFRUSTAGROUP* OFG_ptr,ModeController* MC_ptr,
                       osgGA::Terrain_Manipulator* AM_ptr);

   virtual bool handle( 
      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& );

// Set & get member functions:

   void set_z_ground(double z);
   void set_Grid_ptr(Grid* Grid_ptr);
   void set_SignPostsGroup_ptr(SignPostsGroup* SignPostsGroup_ptr);
   void set_photogroup_ptr(photogroup* p_ptr);
   void set_PolyhedraGroup_ptr(PolyhedraGroup* PolyhedraGroup_ptr);

  protected:

   virtual ~OBSFRUSTAKeyHandler();

  private:

   int n_still_images,n_frustum,toggle_counter;
   double z_ground;
   Grid* Grid_ptr;
   SignPostsGroup* SignPostsGroup_ptr;
   photogroup* photogroup_ptr;
   PolyhedraGroup* PolyhedraGroup_ptr;
   osgGA::Terrain_Manipulator* Terrain_Manipulator_ptr;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
}; 

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void OBSFRUSTAKeyHandler::set_z_ground(double z)
{
   z_ground=z;
}

inline void OBSFRUSTAKeyHandler::set_Grid_ptr(Grid* Grid_ptr)
{
   this->Grid_ptr=Grid_ptr;
}

inline void OBSFRUSTAKeyHandler::set_SignPostsGroup_ptr(
   SignPostsGroup* SignPostsGroup_ptr)
{
   this->SignPostsGroup_ptr=SignPostsGroup_ptr;
}

inline void OBSFRUSTAKeyHandler::set_photogroup_ptr(photogroup* p_ptr)
{
   photogroup_ptr=p_ptr;
}

inline void OBSFRUSTAKeyHandler::set_PolyhedraGroup_ptr(
   PolyhedraGroup* PolyhedraGroup_ptr)
{
   this->PolyhedraGroup_ptr=PolyhedraGroup_ptr;
}



#endif // OBSFRUSTAKEYHANDLER.h
