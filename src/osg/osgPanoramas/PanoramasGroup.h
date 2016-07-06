// ==========================================================================
// Header file for PANORAMASGROUP class
// ==========================================================================
// Last modified on 8/17/11; 8/24/11; 8/28/11
// ==========================================================================

#ifndef PANORAMASGROUP_H
#define PANORAMASGROUP_H

#include <iostream>
#include <string>
#include "osg/osgGeometry/ArrowsGroup.h"
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "network/Network.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgModels/OBSFRUSTUMPickHandler.h"
#include "osg/osgPanoramas/Panorama.h"
#include "osg/osgPanoramas/PanoramasGroup.h"
#include "osg/osgAnnotators/SignPostsGroup.h"

class ArmySymbolsGroup;
class LineSegmentsGroup;
class Movie;
namespace osgGA
{
   class Terrain_Manipulator;
}

class PanoramasGroup : public GeometricalsGroup
{

  public:

   typedef std::map<int,int> INT_INT_MAP;

// Initialization, constructor and destructor functions:

   PanoramasGroup(
      Pass* PI_ptr,OBSFRUSTAGROUP* OFG_ptr,OBSFRUSTUMPickHandler* OPH_ptr,
      osgGA::Terrain_Manipulator* CM_3D_ptr,
      threevector* GO_ptr=NULL);
   PanoramasGroup(
      Pass* PI_ptr,OBSFRUSTAGROUP* OFG_ptr,OBSFRUSTUMPickHandler* OPH_ptr,
      ArmySymbolsGroup* ArmySymbolsGroup_ptr,
      osgGA::Terrain_Manipulator* CM_3D_ptr,
      osgGA::Terrain_Manipulator* CM_3D_global_ptr,threevector* GO_ptr=NULL);
   virtual ~PanoramasGroup();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const PanoramasGroup& p);

// Set & get member functions:

   void set_GPS_response_flag(bool flag);
   bool get_GPS_response_flag() const;
   void set_selected_Panorama_ID(int id);
   int get_selected_Panorama_ID() const;
   int get_curr_OBSFRUSTUM_ID() const;
   int get_panorama_ID_for_OBSFRUSTUM(int OBSFRUSTUM_ID);
   Panorama* get_Panorama_ptr(int n) const;
   Panorama* get_ID_labeled_Panorama_ptr(int ID) const;
   Panorama* get_selected_Panorama_ptr() const;

   ArrowsGroup* get_ArrowsGroup_ptr();
   Network<Panorama* >* get_Panoramas_network_ptr();
   const Network<Panorama* >* get_Panoramas_network_ptr() const;

   void set_GPS_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr);

// Panorama creation member functions:

   int generate_panoramas(
      int n_OBSFRUSTA_per_panorama,double label_delta_z=1,
      double label_text_size=0.5);
   LineSegment* get_or_create_LineSegment_ptr(unsigned int n);
   
// Panorama network member functions:

   bool add_panorama_network_link(int ID1,int ID2);
   bool draw_panorama_network_link(int ID1,int ID2);
   void associate_jump_OBSFRUSTA(int ID1,int ID2);
   int get_jump_OBSFRUSTUM_ID(int OBSFRUSTUM_ID);
   int OBSFRUSTUM_in_particular_direction(int ID,double r_az);
   ArmySymbol* add_forward_translation_arrows(
      const threevector& V1,const threevector& V2,int next_panorama_ID);
   void Delaunay_triangulate_pano_centers(
      const threevector& bottom_left,const threevector& top_right,
      Movie* movie_ptr);
   void Delaunay_triangulate_pano_centers();

// Panorama jump member functions:

   Panorama* identify_next_panorama(const threevector& p_hat);
   void move_from_curr_to_next_panorama(int pano_ID,const threevector& p_hat);
   void move_from_curr_to_next_panorama(
      Panorama* next_Panorama_ptr,const threevector& p_hat);
   void fly_to_next_panorama(
      Panorama* next_Panorama_ptr,const threevector& p_hat);
   double compute_forward_translation_frac();
   void fade_panoramas(double forward_translation_frac_completed);
   void recenter_virtual_camera_over_wagonwheel(int pano_ID);
   void recenter_virtual_camera_over_wagonwheel(
      const threevector& selected_wagonwheel_posn);

   void load_hires_panels(int panorama_ID);
   void load_thumbnail_panels(int panorama_ID);

// Panorama annotation member functions:

   void project_world_posn_into_imageplanes(
      const threevector& XYZ,std::vector<int>& OBSFRUSTUM_IDs,
      std::vector<twovector>& UV_projections);
   void project_SignPosts_into_imageplanes(
      SignPostsGroup* SignPostsGroup_ptr,bool blinking_flag=true);
   void recolor_start_stop_panoramas(int start_pano_ID,int stop_pano_ID);
   
// Manipulation member funtions:

   void translate(const threevector& trans);

  protected:

  private:

   bool GPS_response_flag;
   int network_edge_counter,selected_panorama_ID;
   int start_panorama_ID,stop_panorama_ID,prev_panorama_ID;
   int curr_OBSFRUSTUM_ID,start_left_OBSFRUSTUM_ID,start_right_OBSFRUSTUM_ID;
   double edge_width,last_pano_selection_time;
   ArrowsGroup* ArrowsGroup_ptr;
   Arrow* pointing_Arrow_ptr;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;	
			// just pointer to pre-existing OBSFRUSTAGROUP
   OBSFRUSTUMPickHandler* OBSFRUSTUMPickHandler_ptr;
		        // just pointer to pre-existing OBSFRUSTUMPickHandler
   Network<Panorama* >* Panoramas_network_ptr;
   INT_INT_MAP *jump_frusta_map_ptr,*frusta_panoramas_map_ptr;
   LineSegmentsGroup* LineSegmentsGroup_ptr;
   osgGA::Terrain_Manipulator *Terrain_Manipulator_ptr,
      *global_Terrain_Manipulator_ptr;
   ArmySymbolsGroup* ArmySymbolsGroup_ptr;
   SignPostsGroup *SignPostsGroup_ptr,*GPS_SignPostsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PanoramasGroup& C);

   void initialize_new_Panorama(
      Panorama* curr_panorama_ptr,int OSGsubPAT_number=0);
   void insert_panorama_into_network(Panorama* curr_panorama_ptr);

   void update_display();
   void update_selected_panorama_based_on_GPS_posn();
   void check_armysymbols();

// Pointing direction arrow member functions:

   void generate_pointing_direction_arrow(
      Pass* PI_ptr,threevector* GO_ptr);
   void update_pointing_direction_arrow(
      OBSFRUSTUM* selected_OBSFRUSTUM_ptr,double camera_az);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PanoramasGroup::set_GPS_response_flag(bool flag)
{
   GPS_response_flag=flag;
}

inline bool PanoramasGroup::get_GPS_response_flag() const
{
   return GPS_response_flag;
}

inline void PanoramasGroup::set_selected_Panorama_ID(int id)
{
   selected_panorama_ID=id;
}

inline int PanoramasGroup::get_selected_Panorama_ID() const
{
   return selected_panorama_ID;
}

// --------------------------------------------------------------------------
inline ArrowsGroup* PanoramasGroup::get_ArrowsGroup_ptr()
{
   return ArrowsGroup_ptr;
}

inline int PanoramasGroup::get_curr_OBSFRUSTUM_ID() const
{
   return curr_OBSFRUSTUM_ID;
}

// --------------------------------------------------------------------------
inline Panorama* PanoramasGroup::get_Panorama_ptr(int n) const
{
   return dynamic_cast<Panorama*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Panorama* PanoramasGroup::get_ID_labeled_Panorama_ptr(int ID) const
{
   return dynamic_cast<Panorama*>(get_ID_labeled_Graphical_ptr(ID));
}

inline Panorama* PanoramasGroup::get_selected_Panorama_ptr() const
{
   return get_ID_labeled_Panorama_ptr(get_selected_Panorama_ID());
}

// --------------------------------------------------------------------------
inline Network<Panorama* >* PanoramasGroup::get_Panoramas_network_ptr()
{
   return Panoramas_network_ptr;
}

inline const Network<Panorama* >* PanoramasGroup::get_Panoramas_network_ptr() 
   const
{
   return Panoramas_network_ptr;
}

// --------------------------------------------------------------------------
inline void PanoramasGroup::set_GPS_SignPostsGroup_ptr(SignPostsGroup* SPG_ptr)
{
   GPS_SignPostsGroup_ptr=SPG_ptr;
}


#endif // PanoramasGroup.h



