// ==========================================================================
// Header file for BUILDING_INFO class
// ==========================================================================
// Last modified on 4/18/05; 4/23/06
// ==========================================================================

#ifndef BUILDING_INFO_H
#define BUILDING_INFO_H

class building_info
{
  public:

   enum Relationship
   {
      less_than,equal_to,greater_than,unknown
   };

   enum Spine_Direction
   {
      parallel,perpendicular,undefined
   };

   enum Gross_Spatial_Direction
   {
      in_front,in_back,on_right,on_left,none
   };

// Initialization, constructor and destructor functions:

   building_info();
   building_info(int id);
   building_info(const building_info& b);
   virtual ~building_info();
   building_info& operator= (const building_info& b);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const building_info& b);

// Set & get member functions:

   void set_building_ID(const int id);
   void set_relative_height(const Relationship rh);
   void set_spine_dir(const Spine_Direction sd);
   void set_tall_tree_posn(const Gross_Spatial_Direction gross_dir);
   void set_small_shrub_posn(const Gross_Spatial_Direction gross_dir);

   int get_building_ID() const;
   Relationship get_relative_height() const;
   Spine_Direction get_spine_dir() const;
   Gross_Spatial_Direction get_tall_tree_posn() const;
   Gross_Spatial_Direction get_small_shrub_posn() const;

  private:

   int building_ID;
   Relationship relative_height;
   Spine_Direction spine_dir;
   Gross_Spatial_Direction tall_tree_posn,small_shrub_posn;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const building_info& b);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void building_info::set_building_ID(const int id)
{
   building_ID=id;
}

inline void building_info::set_relative_height(const Relationship rh)
{
   relative_height=rh;
}

inline void building_info::set_spine_dir(const Spine_Direction sd)
{
   spine_dir=sd;
}

inline void building_info::set_tall_tree_posn(
   const Gross_Spatial_Direction gross_dir)
{
   tall_tree_posn=gross_dir;
}

inline void building_info::set_small_shrub_posn(
   const Gross_Spatial_Direction gross_dir)
{
   small_shrub_posn=gross_dir;
}

inline int building_info::get_building_ID() const
{
   return building_ID;
}

inline building_info::Relationship building_info::get_relative_height() const
{
   return relative_height;
}

inline building_info::Spine_Direction building_info::get_spine_dir() const
{
   return spine_dir;
}

inline building_info::Gross_Spatial_Direction 
building_info::get_tall_tree_posn() const
{
   return tall_tree_posn;
}

inline building_info::Gross_Spatial_Direction 
building_info::get_small_shrub_posn() const
{
   return small_shrub_posn;
}

#endif // building_info.h



