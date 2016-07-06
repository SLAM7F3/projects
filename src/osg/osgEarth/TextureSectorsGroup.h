// ==========================================================================
// Header file for TEXTURESECTORSGROUP class
// ==========================================================================
// Last modified on 8/22/08; 4/1/09; 5/7/09
// ==========================================================================

#ifndef TEXTURESECTORSGROUP_H
#define TEXTURESECTORSGROUP_H

#include <string>
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "osg/osg2D/MoviesGroup.h"
#include "video/texture_rectangle.h"
#include "osg/osgEarth/TextureSector.h"

class Movie;

class TextureSectorsGroup : public GeometricalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   TextureSectorsGroup(Pass* PI_ptr,MoviesGroup* MG_ptr=NULL);
   virtual ~TextureSectorsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const TextureSectorsGroup& T);

// Set & get methods:

   int get_curr_TextureSector_ID() const;
   TextureSector* get_TextureSector_ptr(int n) const;
   TextureSector* get_ID_labeled_TextureSector_ptr(int ID) const;
   MoviesGroup* get_MoviesGroup_ptr();
   const MoviesGroup* get_MoviesGroup_ptr() const;

// TextureSector creation methods:

   TextureSector* generate_new_TextureSector(
      Earth* Earth_ptr=NULL,int ID=-1);

   void generate_TextureSector_geode(
      TextureSector* TextureSector_ptr,std::string image_filename);
   void generate_TextureSector_geode(
      const std::vector<threevector>& video_corner_vertices,
      TextureSector* TextureSector_ptr,std::string texture_filename);
   Movie* generate_TextureSector_video_chip(
      TextureSector* TextureSector_ptr,double alpha=0);
   void generate_TextureSector_video_chip(
      Movie* Movie_ptr,TextureSector* TextureSector_ptr);
   texture_rectangle* generate_TextureSector_texture_rectangle(
      TextureSector* TextureSector_ptr,std::string texture_filename);
   Movie* generate_TextureSector_Movie(
      TextureSector* TextureSector_ptr,double alpha=0);
   void destroy_TextureSector_Movie(
      TextureSector* TextureSector_ptr,Movie* Movie_ptr);

// Animation member functions:

   void update_display();
//   void update_TextureSector_video_chips(TextureSector* TextureSector_ptr);

  protected:

  private:

   int curr_TextureSector_ID;
   MoviesGroup* MoviesGroup_ptr;
   std::vector<TextureSector*> regions;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const TextureSectorsGroup& T);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline int TextureSectorsGroup::get_curr_TextureSector_ID() const
{
   return curr_TextureSector_ID;
}

// --------------------------------------------------------------------------
inline TextureSector* TextureSectorsGroup::get_TextureSector_ptr(int n) const
{
   return dynamic_cast<TextureSector*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline TextureSector* TextureSectorsGroup::get_ID_labeled_TextureSector_ptr(int ID) 
   const
{
   return dynamic_cast<TextureSector*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline MoviesGroup* TextureSectorsGroup::get_MoviesGroup_ptr() 
{
   return MoviesGroup_ptr;
}

inline const MoviesGroup* TextureSectorsGroup::get_MoviesGroup_ptr() const
{
   return MoviesGroup_ptr;
}

#endif // TextureSectorsGroup.h



