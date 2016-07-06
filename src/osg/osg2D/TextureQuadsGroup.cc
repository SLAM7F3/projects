// ==========================================================================
// TEXTUREQUADSSGROUP class member function definitions
// ==========================================================================
// Last modified on 5/29/07; 11/27/07
// ==========================================================================

#include <iostream>
#include "osg/osg2D/TextureQuadsGroup.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void TextureQuadsGroup::allocate_member_objects()
{
}		       

void TextureQuadsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="TextureQuadsGroup";
   curr_TextureQuad_ID=-1;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<TextureQuadsGroup>(
         this, &TextureQuadsGroup::update_display));
}		       

TextureQuadsGroup::TextureQuadsGroup(Pass* PI_ptr):
   GeometricalsGroup(3,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

TextureQuadsGroup::~TextureQuadsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const TextureQuadsGroup& TSG)
{
   return(outstream);
}

// ==========================================================================
// TextureQuad creation methods:
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_TextureQuad from all other graphical
// insertion and manipulation methods...

TextureQuad* TextureQuadsGroup::generate_new_TextureQuad(int ID)
{
   if (ID==-1) ID=get_next_unused_ID();
   curr_TextureQuad_ID=ID;
   TextureQuad* curr_TextureQuad_ptr=new TextureQuad(ID);
   GraphicalsGroup::insert_Graphical_into_list(curr_TextureQuad_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_TextureQuad_ptr,0);
   return curr_TextureQuad_ptr;
}

// --------------------------------------------------------------------------
void TextureQuadsGroup::generate_TextureQuad_geode(
   TextureQuad* TextureQuad_ptr,string image_filename,
   const vector<twovector>& UV_corners)
{
   osg::Geode* geode_ptr=TextureQuad_ptr->generate_drawable_geode(
      image_filename,UV_corners);
   TextureQuad_ptr->get_PAT_ptr()->addChild(geode_ptr);
}
