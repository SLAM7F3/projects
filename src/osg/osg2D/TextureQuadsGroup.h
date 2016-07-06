// ==========================================================================
// Header file for TEXTUREQUADSGROUP class
// ==========================================================================
// Last modified on 5/29/07
// ==========================================================================

#ifndef TEXTUREQUADSGROUP_H
#define TEXTUREQUADSGROUP_H

#include <string>
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "osg/osg2D/TextureQuad.h"

class TextureQuadsGroup : public GeometricalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   TextureQuadsGroup(Pass* PI_ptr);
   virtual ~TextureQuadsGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const TextureQuadsGroup& T);

// Set & get methods:

   int get_curr_TextureQuad_ID() const;

// TextureQuad creation methods:

   TextureQuad* generate_new_TextureQuad(int ID=-1);
   void generate_TextureQuad_geode(
      TextureQuad* TextureQuad_ptr,std::string image_filename,
      const std::vector<twovector>& UV_corners);

  protected:

  private:

   int curr_TextureQuad_ID;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const TextureQuadsGroup& T);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline int TextureQuadsGroup::get_curr_TextureQuad_ID() const
{
   return curr_TextureQuad_ID;
}

#endif // TextureQuadsGroup.h



