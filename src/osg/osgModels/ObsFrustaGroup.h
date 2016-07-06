// ==========================================================================
// Header file for OBSFRUSTAGROUP class
// ==========================================================================
// Last modified on 10/19/07; 2/17/08; 10/29/08
// ==========================================================================

#ifndef OBSFRUSTAGROUP_H
#define OBSFRUSTAGROUP_H

#include <iostream>
#include <string>
#include <vector>
#include "osg/osgAnnotators/AnnotatorsGroup.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgModels/ObsFrustum.h"
#include "osg/osg3D/Terrain_Manipulator.h"

class AnimationController;
class PolyhedraGroup;

class ObsFrustaGroup : public GeometricalsGroup, public AnnotatorsGroup
{

  public:

// Initialization, constructor and destructor functions:

   ObsFrustaGroup(
      Pass* PI_ptr,AnimationController* AC_ptr,threevector* GO_ptr=NULL);
   ObsFrustaGroup(
      Pass* PI_ptr,osgGA::CustomManipulator* CM_ptr,
      AnimationController* AC_ptr,threevector* GO_ptr=NULL);

   virtual ~ObsFrustaGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const ObsFrustaGroup& f);

// Set & get methods:

   void set_z_ColorMap_ptr(ColorMap* cmap_ptr);
   ObsFrustum* get_ObsFrustum_ptr(int n) const;
   ObsFrustum* get_ID_labeled_ObsFrustum_ptr(int ID) const;
   MoviesGroup* get_MoviesGroup_ptr();
   osgGA::Terrain_Manipulator* get_Terrain_Manipulator_ptr();
   const osgGA::Terrain_Manipulator* get_Terrain_Manipulator_ptr() const;

// ObsFrustum creation and manipulation methods:

   ObsFrustum* generate_new_ObsFrustum(Movie* Movie_ptr=NULL,int ID=-1);
   ObsFrustum* generate_new_ObsFrustum(
      AnimationController* AC_ptr,int ID=-1);
   ObsFrustum* generate_new_ObsFrustum(
      double az_extent,double el_extent,int ID=-1);
   ObsFrustum* generate_movie_ObsFrustum(
      std::string movie_filename,double alpha=1.0);

// Still imagery ObsFrusta generation methods:

   int generate_still_imagery_frusta(
      PassesGroup& passes_group,bool multicolor_frusta_flag,
      bool initially_mask_all_frusta_flag);
   void extract_still_imagery_info(Pass* videopass_ptr);
   void reset_frustum_colors_based_on_Zcolormap();

   ObsFrustum* generate_virtual_camera_ObsFrustum();
   ObsFrustum* generate_still_image_ObsFrustum(
      int n_frustum,colorfunc::Color frustum_color);

// Animation methods:

   void update_display();
   void hide_nonselected_ObsFrusta();
   void flyto_camera_location(int ID);
   void compute_camera_flyout_posn_and_orientation(
      int ID,threevector& flyout_camera_posn,genmatrix& Rcamera_flyout);

// HAFB video3D methods:

   ObsFrustum* generate_HAFB_movie_ObsFrustum(
      const std::vector<threevector>& aircraft_posn,double z_offset);
   void read_HAFB_frusta_info(
      std::string segments_filename,std::vector<double>& curr_time,
      std::vector<int>& segment_ID,std::vector<int>& pass_number,
      std::vector<threevector>& V1,std::vector<threevector>& V2,
      std::vector<colorfunc::Color>& color);
   void reconstruct_HAFB_corner_dirs(
      const std::vector<threevector>& plane_posn,
      const std::vector<threevector>& corner_posns,
      std::vector<threevector>& UV_corner_dir);

  protected:

  private:

   ColorMap* z_ColorMap_ptr;
   MoviesGroup* MoviesGroup_ptr;
   PolyhedraGroup* PolyhedraGroup_ptr;
   osgGA::Terrain_Manipulator* CM_3D_ptr;

// Member variables for still image ObsFrusta:

   int n_still_images;
   std::vector<std::string> still_movie_filenames;
   std::vector<double> downrange_distances;
   std::vector<genmatrix*> P_ptrs;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ObsFrustaGroup& f);

   void initialize_new_ObsFrustum(
      ObsFrustum* ObsFrustum_ptr,int OSGsubPAT_number=0);
   ObsFrustum* generate_still_image_ObsFrustum(
      double downrange_distance,std::string movie_filename,
      std::vector<fourvector>& row);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ObsFrustaGroup::set_z_ColorMap_ptr(ColorMap* cmap_ptr) 
{
   z_ColorMap_ptr=cmap_ptr;
}

// --------------------------------------------------------------------------
inline ObsFrustum* ObsFrustaGroup::get_ObsFrustum_ptr(int n) const
{
   return dynamic_cast<ObsFrustum*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline ObsFrustum* ObsFrustaGroup::get_ID_labeled_ObsFrustum_ptr(
   int ID) const
{
   return dynamic_cast<ObsFrustum*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline MoviesGroup* ObsFrustaGroup::get_MoviesGroup_ptr() 
{
   return MoviesGroup_ptr;
}

// --------------------------------------------------------------------------
inline osgGA::Terrain_Manipulator* 
ObsFrustaGroup::get_Terrain_Manipulator_ptr()
{
   return CM_3D_ptr;
}

inline const osgGA::Terrain_Manipulator* 
ObsFrustaGroup::get_Terrain_Manipulator_ptr() const
{
   return CM_3D_ptr;
}

#endif // ObsFrustaGroup.h



