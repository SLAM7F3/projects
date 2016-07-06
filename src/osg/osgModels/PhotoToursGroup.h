// ==========================================================================
// Header file for PhotoToursGroup class
// ==========================================================================
// Last modified on 2/25/10; 2/28/10; 3/1/10; 3/4/11
// ==========================================================================

#ifndef PHOTOTOURSGROUP_H
#define PHOTOTOURSGROUP_H

#include "osg/osgGeometry/GeometricalsGroup.h"
#include "osg/osgModels/PhotoTour.h"

class OBSFRUSTAGROUP;
class PolyLinesGroup;

class PhotoToursGroup : public GeometricalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   PhotoToursGroup(OBSFRUSTAGROUP* OFG_ptr);
   PhotoToursGroup(OBSFRUSTAGROUP* OFG_ptr,PolyLinesGroup* PLG_ptr);
   virtual ~PhotoToursGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const PhotoToursGroup& f);

// Set & get methods:

   void set_conduct_tours_flag(bool flag);
   PhotoTour* get_PhotoTour_ptr(int n) const;
   KDTree::KDTree<2, threevector>* get_camera_posns_kdtree_ptr();

// PhotoTour creation and manipulation methods:

   PhotoTour* generate_new_PhotoTour(int OSGsubPAT_ID=0,int ID=-1);
   void destroy_all_PhotoTours();
   bool destroy_PhotoTour(int PhotoTour_ID);

// Camera path construction and display methods:

   PhotoTour* generate_specified_tour(const std::vector<int>& tour_photo_IDs);

  protected:

  private:

   bool conduct_tours_flag;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;
   PolyLinesGroup* PolyLinesGroup_ptr;

   KDTree::KDTree<2, threevector>* camera_posns_kdtree_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PhotoToursGroup& f);

   void initialize_new_PhotoTour(PhotoTour* PhotoTour_ptr,int OSGsubPAT_ID=0);
   void generate_camera_posns_kdtree();
   void update_display();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PhotoToursGroup::set_conduct_tours_flag(bool flag)
{
   conduct_tours_flag=flag;
}

inline PhotoTour* PhotoToursGroup::get_PhotoTour_ptr(int n) const
{
   return dynamic_cast<PhotoTour*>(get_Graphical_ptr(n));
}

inline KDTree::KDTree<2, threevector>* 
PhotoToursGroup::get_camera_posns_kdtree_ptr()
{
   return camera_posns_kdtree_ptr;
}

#endif // PHOTOTOURSGROUP.h



