// ==========================================================================
// Header file for PhotoTour class
// ==========================================================================
// Last updated on 2/26/10; 2/28/10; 3/4/11
// ==========================================================================

#ifndef PhotoTour_H
#define PhotoTour_H

#include <vector>
#include "kdtree/kdtree.h"

class OBSFRUSTAGROUP;
class polyline;

class PhotoTour : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   PhotoTour(OBSFRUSTAGROUP* OFG_ptr,int id=-1);
   virtual ~PhotoTour();
   friend std::ostream& operator<< (
      std::ostream& outstream,const PhotoTour& f);

// Set & get methods:

   void set_loop_to_start_flag(bool flag);
   void set_prev_OBSFRUSTUM_ID(int ID);
   int get_starting_OBSFRUSTUM_ID() const;
   std::vector<int>& get_ordered_OBSFRUSTUM_IDs();
   double get_tour_length() const;

// Tour computation methods:

   void construct_camera_path(
      polyline* input_polyline_ptr,
      KDTree::KDTree<2, threevector>* camera_posns_kdtree_ptr);
   void specify_tour_OBSFRUSTUM_IDs(std::vector<int>& OBSFRUSTUM_IDs);
   std::vector<threevector> get_tour_posns();
   std::vector<int> get_tour_photo_IDs() const;
   void conduct_virtual_tour();
   
  protected:

  private:

   bool loop_to_start_flag;
   unsigned int ordered_OBSFRUSTUM_counter;
   int prev_OBSFRUSTUM_ID;
   double tour_length;
   std::vector<int> ordered_OBSFRUSTUM_IDs;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PhotoTour& f);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PhotoTour::set_loop_to_start_flag(bool flag)
{
   loop_to_start_flag=flag;
}

inline void PhotoTour::set_prev_OBSFRUSTUM_ID(int ID)
{
   prev_OBSFRUSTUM_ID=ID;
}

inline int PhotoTour::get_starting_OBSFRUSTUM_ID() const
{
   if (ordered_OBSFRUSTUM_IDs.size() > 0)
   {
      return ordered_OBSFRUSTUM_IDs.front();
   }
   else
   {
      return -1;
   }
}

inline std::vector<int>& PhotoTour::get_ordered_OBSFRUSTUM_IDs() 
{
   return ordered_OBSFRUSTUM_IDs;
}

inline double PhotoTour::get_tour_length() const
{
   return tour_length;
}


#endif // PhotoTour.h



