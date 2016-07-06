// ========================================================================
// Brad Boven's JIGSAWGRID class
// ========================================================================
// Last updated on 3/27/06; 6/13/06; 9/11/06
// ========================================================================

#include <iostream>
#include <osg/Geode>
#include <osg/Geometry>
#include "osg/osgGrid/JigsawGrid.h"

using std::cout;
using std::endl;
using std::string;

// -------------------------------------------------------------------------
JigsawGrid::JigsawGrid(): 
   Grid2d()
{
}

JigsawGrid::~JigsawGrid()
{
}

// -------------------------------------------------------------------------
void JigsawGrid::update_grid()
{
   reset_grid();

   if (delta_x < 1) delta_x = 1;
   if (delta_y < 1) delta_y = 1;

   const double frac_increase=0;
   set_world_origin_and_middle();

   reset_axes_lines();

   int number_y_crosslines=basic_math::round(xsize/delta_x)+1;
   int number_x_crosslines=basic_math::round(ysize/delta_y)+1;

/*
   n_vertices = 0;
   int num_x_lines = 0;
   int num_y_lines = 0;
   osg::Vec3 xlinesize(xsize,0.0f,0.0f);
   osg::Vec3 ylinesize(0.0f,ysize,0.0f);

   //Set up the X-Axis Lines
   for (int i=0; i <= (int)ysize; i=i+delta_y)
   {
      osg::Vec3 xdir(x_origin,y_origin+i,zplane);
      (*coords).push_back(xdir);
      (*coords).push_back(xdir+xlinesize);
      n_vertices=n_vertices+2;
      num_y_lines++;
   }

   //Set up the Y-Axis Lines
   for (int i=0; i <= (int)xsize; i=i+delta_x)
   {
      osg::Vec3 ydir(x_origin+i,y_origin,zplane);
      (*coords).push_back(ydir);
      (*coords).push_back(ydir+ylinesize);
      n_vertices=n_vertices+2;
      num_x_lines++;
   }

   geom->addPrimitiveSet(
      new osg::DrawArrays(osg::PrimitiveSet::LINES,0,n_vertices));
*/

   cout << "Axis label size was: "<< axis_char_label_size << endl;
   cout << "Tick label size was: "<< tick_char_label_size << endl;
   float axis_label_character_size=axis_char_label_size;
   float tick_label_character_size=tick_char_label_size;

   double x_offset_from_x_middle = 60;
   double x_offset_from_x_axis = 40;
   double y_offset_from_y_middle = 60;
   double y_offset_from_y_axis = 40;
   double x_tick_offset = 10;
   double y_tick_offset = 15;

   if (xsize < 200)
   {
      x_offset_from_x_middle = (x_offset_from_x_middle*.05);
      x_offset_from_x_axis = (x_offset_from_x_axis*.05);
   }
   if (ysize < 200)
   {
      y_offset_from_y_axis = (y_offset_from_y_axis*.05);
      y_offset_from_y_middle = (y_offset_from_y_middle*.05);
   }
   if (xsize < 200 || ysize < 200)
   {
      x_tick_offset = (x_tick_offset*.05);
      y_tick_offset = (y_tick_offset*.05);
   }
    
   osg::Vec3 posx(
      xmiddle-x_offset_from_x_middle,y_origin-x_offset_from_x_axis, zplane);
   osg::Vec3 posy(
      x_origin-y_offset_from_y_axis, ymiddle-y_offset_from_y_middle, zplane);

   x_axis_text->setFont("fonts/times.ttf");
   x_axis_text->setCharacterSize(axis_label_character_size);
   //x_axis_text->setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
   x_axis_text->setPosition(posx);
   x_axis_text->setAxisAlignment(osgText::Text::XY_PLANE);
   x_axis_text->setColor(colors->at(0));
   x_axis_text->setText(x_axis_label);

   y_axis_text->setFont("fonts/times.ttf");
   y_axis_text->setCharacterSize(axis_label_character_size);
   //y_axis_text->setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
   y_axis_text->setPosition(posy);
   y_axis_text->setAxisAlignment(osgText::Text::XY_PLANE);
   y_axis_text->setColor(colors->at(0));
   y_axis_text->setText(y_axis_label);
   osg::Quat r1;
   r1.makeRotate(-osg::inDegrees(-90.0f),0.0f,0.0f,1.0f);
   y_axis_text->setRotation(r1);

   string tick_string;
   int tick_int;
   osg::Vec3 tick_pos;
   for (int i = -4; i < ((num_x_lines/2)+1); i++)
   {
      std::ostringstream tick_stream;
      osgText::Text* x_tick_text = new osgText::Text;
      tick_int = i*delta_x;
      tick_stream << tick_int;
      tick_string = tick_stream.str();

      tick_pos = osg::Vec3(
         x_origin+((i+4)*delta_x), y_origin-x_tick_offset, zplane);

      x_tick_text->setFont("fonts/times.ttf");
      x_tick_text->setCharacterSize(tick_label_character_size);
      x_tick_text->setCharacterSizeMode(
         osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
      x_tick_text->setPosition(tick_pos);
      x_tick_text->setAxisAlignment(osgText::Text::SCREEN);
      x_tick_text->setColor(colors->at(0));
      x_tick_text->setText(tick_string);

      x_axis_tick_texts.push_back(x_tick_text);
      geode->addDrawable(x_tick_text);
   }

   for (int i = 0; i < num_y_lines; i++)
   {
      std::ostringstream tick_stream;
      osgText::Text* y_tick_text = new osgText::Text;
      tick_int = (i+18)*delta_y;
      tick_stream << tick_int;
      tick_string = tick_stream.str();

      tick_pos = osg::Vec3(x_origin-y_tick_offset, y_origin+(i*delta_y), 
                           zplane );

      y_tick_text->setFont("fonts/times.ttf");
      y_tick_text->setCharacterSize(tick_label_character_size);
      y_tick_text->setCharacterSizeMode(
         osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
      y_tick_text->setPosition(tick_pos);
      y_tick_text->setAxisAlignment(osgText::Text::SCREEN);
      y_tick_text->setColor(colors->at(0));
      y_tick_text->setText(tick_string);

      y_axis_tick_texts.push_back(y_tick_text);
      geode->addDrawable(y_tick_text);
   }
}

