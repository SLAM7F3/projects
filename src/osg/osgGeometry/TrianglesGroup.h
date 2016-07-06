// ==========================================================================
// Header file for TRIANGLESGROUP class
// ==========================================================================
// Last modified on 11/24/06; 1/9/07; 1/30/07; 1/26/12; 3/22/14
// ==========================================================================

#ifndef TRIANGLESGROUP_H
#define TRIANGLESGROUP_H

#include <iostream>
#include <string>
#include <vector>
#include <osg/Group>
#include <osg/Node>
#include "color/colorfuncs.h"
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "network/Network.h"
#include "osg/osgGeometry/Triangle.h"
#include "image/TwoDarray.h"

class triangles_group;

class TrianglesGroup : public GraphicalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   TrianglesGroup(const int p_ndims,Pass* PI_ptr);
   virtual ~TrianglesGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const TrianglesGroup& f);

// Set & get methods:

   double get_size() const;
   Triangle* get_Triangle_ptr(int n) const;
   Triangle* get_ID_labeled_Triangle_ptr(int ID) const;
   twoDarray* get_zsample_twoDarray_ptr();
   const twoDarray* get_zsample_twoDarray_ptr() const;
   
// Triangle creation and manipulation methods:

   Triangle* generate_new_Triangle(int ID=-1);
   void generate_triangle_geode(Triangle* triangle_ptr);
   bool erase_Triangle();
   bool unerase_Triangle();
//    void generate_triangles(triangles_group* triangles_group_ptr);

// Ascii file I/O methods

   void save_info_to_file();
   void read_info_from_file(
      std::string triangles_filename,std::vector<double>& curr_time,
      std::vector<int>& triangle_ID,std::vector<int>& pass_number,
      std::vector<threevector>& V0,std::vector<int>& V0_ID,
      std::vector<threevector>& V1,std::vector<int>& V1_ID,
      std::vector<threevector>& V2,std::vector<int>& V2_ID,
      std::vector<colorfunc::Color>& color);
   bool reconstruct_triangles_from_file_info();
   bool reconstruct_triangles_from_file_info(std::string input_filename);

// Triangle network construction methods:

   void regenerate_triangles(
      const std::vector<double>& curr_time,
      const std::vector<int>& triangle_ID,
      const std::vector<int>& pass_number,
      const std::vector<threevector>& V0,const std::vector<int>& V0_ID,
      const std::vector<threevector>& V1,const std::vector<int>& V1_ID,
      const std::vector<threevector>& V2,const std::vector<int>& V2_ID,
      const std::vector<colorfunc::Color>& color);
   void generate_triangles_network(const std::vector<int>& triangle_ID);
   void color_triangles();
   void update_display();

// XY lattice and Z coordinate methods:

   void extremal_XY_coords(
      double& min_x,double& min_y,double& max_x,double& max_y);
   void sample_zcoords_on_XYgrid();
   double approx_zcoord(double x,double y);

  protected:

  private:

   double size[4];
   std::vector<threevector> vertices;
   Network<Triangle*>* triangles_network_ptr;
   twoDarray* zsample_twoDarray_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const TrianglesGroup& f);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline double TrianglesGroup::get_size() const
{
   return size[get_ndims()];
}

// --------------------------------------------------------------------------
inline Triangle* TrianglesGroup::get_Triangle_ptr(int n) const
{
   return dynamic_cast<Triangle*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline Triangle* TrianglesGroup::get_ID_labeled_Triangle_ptr(int ID) const
{
   return dynamic_cast<Triangle*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline twoDarray* TrianglesGroup::get_zsample_twoDarray_ptr()
{
   return zsample_twoDarray_ptr;
}

inline const twoDarray* TrianglesGroup::get_zsample_twoDarray_ptr() const
{
   return zsample_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Member function approx_zcoord takes in an (x,y) pair and returns
// the closest z coordinate within *zsample_twoDarray_ptr.

inline double TrianglesGroup::approx_zcoord(double x,double y)
{
   unsigned int px,py;
   if (zsample_twoDarray_ptr->point_to_pixel(x,y,px,py))
   {
      return zsample_twoDarray_ptr->get(px,py);
   }
   else
   {
      std::cout << "Error in TrianglesGroup::approx_zcoord() !!" << std::endl;
      std::cout << "x = " << x << " y = " << y 
                << " lies outside valid region " << std::endl;
      return NEGATIVEINFINITY;
   }
   
}


#endif // TrianglesGroup.h



