// ==========================================================================
// Header file for LINESEGMENTSGROUP class
// ==========================================================================
// Last modified on 10/8/09; 10/21/09; 11/16/10; 4/5/14
// ==========================================================================

#ifndef LINESEGMENTSGROUP_H
#define LINESEGMENTSGROUP_H

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osg/Group>
#include <osg/Vec4>
#include "osg/Custom3DManipulator.h"
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "osg/osgGeometry/LineSegment.h"

class AnimationController;
class ViewFrustum;

class LineSegmentsGroup : public GraphicalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   LineSegmentsGroup(
      const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr=NULL);
   LineSegmentsGroup(
      const int p_ndims,Pass* PI_ptr,osgGA::Custom3DManipulator* CM_ptr,
      ViewFrustum* VF_ptr);
   virtual ~LineSegmentsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const LineSegmentsGroup& f);

// Set & get methods:

   double get_size() const;
   LineSegment* get_LineSegment_ptr(int n) const;
   LineSegment* get_ID_labeled_LineSegment_ptr(int ID) const;

// LineSegment creation and manipulation methods:

   LineSegment* generate_new_segment(
      const threevector& V1,const threevector& V2,
      bool draw_arrow_flag=false,int ID=-1);
   void generate_canonical_segments(unsigned int n_segments);
   LineSegment* generate_new_canonical_LineSegment(
      int ID,bool draw_arrow_flag);
   LineSegment* generate_new_canonical_LineSegment(
      int ID,bool draw_endpoint1_flag,bool draw_endpoint2_flag,
      double endpoint_size_prefactor=1);

   void initialize_posn(LineSegment* curr_LineSegment_ptr);
   void edit_LineSegment_label();
   bool erase_LineSegment();
   bool unerase_LineSegment();

   void change_size(double factor);
   void set_width(double width);
   void reset_colors();

   void update_display();

// LineSegment destruction member functions:

   void destroy_all_LineSegments();
   bool destroy_LineSegment(int ID);
   bool destroy_LineSegment(LineSegment* curr_LineSegment_ptr);
   
// Ascii feature file I/O methods:

   void save_info_to_file();
   bool read_info_from_file(
      std::string segments_filename,std::vector<double>& curr_time,
      std::vector<int>& segment_ID,std::vector<int>& pass_number,
      std::vector<threevector>& V1,std::vector<threevector>& V2,
      std::vector<colorfunc::Color>& color);
   bool read_info_from_file(
      std::string segments_filename,std::vector<double>& curr_time,
      std::vector<int>& segment_ID,std::vector<int>& pass_number,
      std::vector<threevector>& V1,std::vector<threevector>& V2,
      std::vector<osg::Vec4>& color);
   bool reconstruct_lines_from_file_info(std::string lines_filename,
                                         bool RGB_flag=false);
   void generate_FOV_segments(
      const std::vector<double>& curr_time,
      const std::vector<int>& pass_number,
      const std::vector<threevector>& V1,const std::vector<threevector>& V2,
      const std::vector<colorfunc::Color>& color,
      const std::vector<threevector>& plane_posn);
   void generate_FOV_segments(
      double rho,const threevector& camera_XYZ,
      const std::vector<threevector>& UV_corner_dir);

// View frustum display methods:

   void draw_FOV_frustum();

  protected:

  private:

   double size[4];
   ViewFrustum* ViewFrustum_ptr;
   osg::ref_ptr<osgGA::Custom3DManipulator> CM_3D_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const LineSegmentsGroup& f);

   void initialize_new_LineSegment(
      LineSegment* LineSegment_ptr,int OSGsubPAT_number=0);
   void regenerate_segment(
      int curr_segment_ID,
      const std::vector<double>& curr_time,const std::vector<int>& segment_ID,
      const std::vector<threevector>& V1,const std::vector<threevector>& V2,
      const std::vector<colorfunc::Color>& color_index);
   void regenerate_segment(
      int curr_segment_ID,
      const std::vector<double>& curr_time,const std::vector<int>& segment_ID,
      const std::vector<threevector>& V1,const std::vector<threevector>& V2,
      const std::vector<osg::Vec4>& color);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline double LineSegmentsGroup::get_size() const
{
   return size[get_ndims()];
}

// --------------------------------------------------------------------------
inline LineSegment* LineSegmentsGroup::get_LineSegment_ptr(int n) const
{
   return dynamic_cast<LineSegment*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline LineSegment* LineSegmentsGroup::get_ID_labeled_LineSegment_ptr(int ID) 
   const
{
   return dynamic_cast<LineSegment*>(get_ID_labeled_Graphical_ptr(ID));
}


#endif // LineSegmentsGroup.h



