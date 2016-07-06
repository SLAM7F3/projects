// ==========================================================================
// Header file for POLYGONSGROUP class
// ==========================================================================
// Last modified on 2/19/08; 2/21/08; 9/24/09; 9/25/09; 10/4/09
// ==========================================================================

#ifndef POLYGONSGROUP_H
#define POLYGONSGROUP_H

#include <iostream>
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "osg/osgGeometry/Polygon.h"

class AnimationController;
class PolyLine;
class PolyLinesGroup;

namespace osgGeometry
{

   class PolygonsGroup : public GeometricalsGroup
      {

        public:

// Initialization, constructor and destructor functions:

         PolygonsGroup(const int p_ndims,Pass* PI_ptr,
                       AnimationController* AC_ptr=NULL,
                       threevector* GO_ptr=NULL);
         virtual ~PolygonsGroup();

         friend std::ostream& operator<< 
            (std::ostream& outstream,const PolygonsGroup& P);

// Set & get methods:

         double get_size() const;
         Polygon* get_Polygon_ptr(int n) const;
         Polygon* get_ID_labeled_Polygon_ptr(int ID) const;
         void set_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr);
         PolyLinesGroup* get_PolyLinesGroup_ptr();

// Polygon creation and manipulation methods:

         Polygon* generate_new_Polygon(
            const threevector& reference_origin,const polygon& p,int ID=-1);
//         Polygon* generate_new_Polygon(
//            const threevector& reference_origin,const polygon& p,
//            colorfunc::Color permanent_color,int ID=-1);
         Polygon* generate_new_Polygon(PolyLine* PolyLine_ptr,int ID=-1);

         void destroy_all_Polygons();
         bool destroy_Polygon();
         bool destroy_Polygon(int ID);
         bool destroy_Polygon(Polygon* curr_Polygon_ptr);

         bool erase_Polygon();
         bool unerase_Polygon();
         void reset_colors();

// Ascii file I/O methods

         void save_info_to_file();
         void read_info_from_file();

// Video annotation member functions:

         void generate_translucent_Polygon(
            colorfunc::Color poly_color,
            const std::vector<threevector>& vertices,double alpha,
            bool draw_border_flag=true);
         std::vector<threevector> sample_translucent_Polygons(
            int mdim,int ndim,const bounding_box& imageplane_bbox);

        protected:

        private:

         double size[4];
         PolyLinesGroup* PolyLinesGroup_ptr;

         void allocate_member_objects();
         void initialize_member_objects();
         void docopy(const PolygonsGroup& f);

         void update_display();
      };

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

   inline double PolygonsGroup::get_size() const
      {
         return size[get_ndims()];
      }

// --------------------------------------------------------------------------
   inline Polygon* PolygonsGroup::get_Polygon_ptr(int n) const
      {
         return dynamic_cast<Polygon*>(get_Graphical_ptr(n));
      }

// --------------------------------------------------------------------------
   inline Polygon* PolygonsGroup::get_ID_labeled_Polygon_ptr(int ID) const
      {
         return dynamic_cast<Polygon*>(get_ID_labeled_Graphical_ptr(ID));
      }

// --------------------------------------------------------------------------
   inline void PolygonsGroup::set_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr)
      {
         PolyLinesGroup_ptr=PLG_ptr;
      }

   inline PolyLinesGroup* PolygonsGroup::get_PolyLinesGroup_ptr()
      {
         return PolyLinesGroup_ptr;
      }
   

} // osgGeometry namespace


#endif // PolygonsGroup.h



