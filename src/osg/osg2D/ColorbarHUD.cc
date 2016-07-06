// ==========================================================================
// ColorbarHUD class member function definitions
// ==========================================================================
// Last modified on 6/17/09; 9/28/09; 2/1/12
// ==========================================================================

#include <osgSim/ColorRange>
#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "osg/osg2D/ColorbarHUD.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

void ColorbarHUD::allocate_member_objects()
{
   scalarbar_geode_refptr=new osgSim::ScalarBar;
   title_geode_refptr=new osg::Geode;
   labels_geode_refptr=new osg::Geode;
   colorrange_refptr=new osgSim::ColorRange(0.0,1.0);

   title_transform_refptr=new osg::MatrixTransform;
   scalarbar_transform_refptr=new osg::MatrixTransform;
   labels_transform_refptr=new osg::MatrixTransform;
}

void ColorbarHUD::initialize_member_objects()
{
//   cout << "inside ColorbarHUD::initialize_member_objects()" << endl;
   truncate_value_flag=true;

   numLabels=5;

   scalarbar_geode_refptr->setNumColors(256);
   scalarbar_geode_refptr->setNumLabels(1);
   scalarbar_geode_refptr->setAspectRatio(0.03);
   scalarbar_geode_refptr->setOrientation(osgSim::ScalarBar::HORIZONTAL);
   scalarbar_geode_refptr->setScalarsToColors(colorrange_refptr.get());

   colorbar_index=0;

   title_transform_refptr->addChild(title_geode_refptr.get());
   scalarbar_transform_refptr->addChild(scalarbar_geode_refptr.get());
   labels_transform_refptr->addChild(labels_geode_refptr.get());

   initialize_title_transform();
   initialize_scalarbar_transform();
   initialize_labels_transform();

   getProjection()->addChild(title_transform_refptr.get());
   getProjection()->addChild(scalarbar_transform_refptr.get());
   getProjection()->addChild(labels_transform_refptr.get());
}

ColorbarHUD::ColorbarHUD(
   double hue_start,double hue_stop,
   double scalar_value_start,double scalar_value_stop,string title):
   GenericHUD( -640, 640, -512, 512 )
{
   allocate_member_objects();
   initialize_member_objects();

//   cout << "Enter number of labels" << endl;
//   cin >> numLabels;

   this->hue_start.push_back(hue_start);
   this->hue_stop.push_back(hue_stop);
   this->scalar_value_start.push_back(scalar_value_start);
   this->scalar_value_stop.push_back(scalar_value_stop);
   this->title.push_back(title);

   reset_title();
   reset_scalarbar();
   reset_labels();
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void ColorbarHUD::set_colorbar_index(unsigned int i)
{
   if (i >= title.size())
   {
      cout << "Error in ColorbarHUD::set_colorbar_index()!" << endl;
      cout << "i = " << i << endl;
      cout << "n_colorbars = " << title.size() << endl;
   }
   else
   {
      colorbar_index=i;
   }
   reset_title();
   reset_scalarbar();
   reset_labels();
}

void ColorbarHUD::pushback_hue_start(double hue)
{
   hue_start.push_back(hue);
}

void ColorbarHUD::pushback_hue_stop(double hue)
{
   hue_stop.push_back(hue);
}

void ColorbarHUD::pushback_scalar_value_start(double scalar)
{
   scalar_value_start.push_back(scalar);
}

void ColorbarHUD::pushback_scalar_value_stop(double scalar)
{
   scalar_value_stop.push_back(scalar);
}

void ColorbarHUD::pushback_title(string title)
{
   this->title.push_back(title);
}

// --------------------------------------------------------------------------
void ColorbarHUD::set_nodemask(int mask_value)
{
   scalarbar_geode_refptr->setNodeMask(mask_value);
   labels_geode_refptr->setNodeMask(mask_value);
}

// ==========================================================================
// Scalarbar generation functions
// ==========================================================================

void ColorbarHUD::initialize_title_transform()
{
   const double scale_factor=800;
//   osg::Vec3 posn(0,0,0);
   osg::Vec3 posn(-0.5 , 0.75 , 0);

   title_transform_refptr->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
   osg::Matrixd matrix(
      osg::Matrix::translate( posn ) * 
      osg::Matrix::rotate(osg::DegreesToRadians(-90.0),0,0,1) * 
      osg::Matrixd::scale(scale_factor,scale_factor,1) );
   title_transform_refptr->setMatrix(matrix);
}

// --------------------------------------------------------------------------
void ColorbarHUD::initialize_scalarbar_transform()
{
   const double scale_factor=800;
//   osg::Vec3 posn(0,0,0);
   osg::Vec3 posn(-0.5,0.69,0);

   scalarbar_transform_refptr->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
   osg::Matrixd matrix(
      osg::Matrix::translate( posn ) * 
      osg::Matrix::rotate(osg::DegreesToRadians(-90.0),0,0,1) * 
      osg::Matrixd::scale(scale_factor,scale_factor,1) );
   scalarbar_transform_refptr->setMatrix(matrix);
}

// --------------------------------------------------------------------------
void ColorbarHUD::initialize_labels_transform()
{
   labels_transform_refptr->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
   osg::Vec3 posn(0.67,0,0);

   osg::Matrixd labels_matrix(
      osg::Matrix::translate( posn ) * 
      osg::Matrixd::scale(800,800,800) );
   labels_transform_refptr->setMatrix(labels_matrix);
}

// --------------------------------------------------------------------------
// Member function reset_title() implements our own title
// functionality.  Unlike the built-in OSG1.2 Scalarbar title, this
// method allows us to easily control the size of the title text as
// well as its backdrop.

void ColorbarHUD::reset_title()
{
//   cout << "inside ColorbarHUD::reset_title()" << endl;
   float characterSize = 0.05;
   osg::Vec4 text_color(1,1,1,1);

   title_geode_refptr->removeDrawables(
      0,title_geode_refptr->getNumDrawables());

   osgText::Text* text = new osgText::Text;
   text->setFont("fonts/times.ttf");
   text->setColor(text_color);
   text->setCharacterSize(characterSize);

// Increase backdropoffset from its default value of 0.07 in order to
// amplify black border behind text labels.  This helps to make
// colorbar labels stand out against noisy, colored backgrounds:

   text->setBackdropOffset(0.15);

//   cout << "title[colorbar_index] = " << title[colorbar_index] << endl;
//   cout << "title.size = " << title[colorbar_index].size() << endl;
   
   text->setText(title[colorbar_index]);

// delta_x = 0 places the start of the title at the very top of the
// colorbar.  delta_x=1 places the start of the title at the very
// bottom of the colorbar.  So we need to introduce a fudge factor
// which converts from text character spaces to a value in the
// interval [0,1]:

   double fudge_factor=0.425;
//   cout << "Enter fudge factor:" << endl;
//   cin >> fudge_factor;

//   double delta_x=0;
//   double delta_x=1;

   double delta_x=0.5-0.5*fudge_factor*
      title[colorbar_index].size()*characterSize;
   double delta_y=0;

   osg::Vec3 title_posn(delta_x,delta_y,0);
   text->setPosition(title_posn);
   text->setAxisAlignment(osgText::Text::XY_PLANE);
   text->setBackdropType(osgText::Text::OUTLINE);
   
   title_geode_refptr->addDrawable(text);
}

// --------------------------------------------------------------------------
void ColorbarHUD::reset_scalarbar()
{
//   cout << "inside ColorbarHUD::reset_scalarbar()" << endl;
//   cout << "colorbar_index = " << colorbar_index << endl;

   int n_hue_steps=fabs(hue_stop[colorbar_index]-hue_start[colorbar_index])/
      (20.0);
//   cout << "n_hue_steps = " << n_hue_steps << endl;

// If hue_start==hue_stop, we assume the color bar varies in value
// rather than in hue:

   double r,g,b;
   double alpha=1.0;
   vector<osg::Vec4> colors;
   if (n_hue_steps==0)
   {
      int n_value_steps=5;
      double delta_value=1.0/n_value_steps;
      for (int n=0; n<=n_value_steps; n++)
      {
         double s=1.0;
         double v=1.0-n*delta_value;
         colorfunc::hsv_to_RGB(hue_start[colorbar_index],s,v,r,g,b);
//         cout << "v = " << v << " r = " << r << " g = " << g << " b = " << b
//              << endl;
         colors.push_back(osg::Vec4(r,g,b,alpha));
      }
   }
   else
   {
      double delta_hue=(hue_stop[colorbar_index]-hue_start[colorbar_index])/
         (n_hue_steps-1);
      double s=1.0;
      double v=1.0;

      for (int n=0; n<n_hue_steps; n++)
      {
         double h=hue_stop[colorbar_index]-n*delta_hue;
         colorfunc::hsv_to_RGB(h,s,v,r,g,b);
//      cout << "h = " << h << " r = " << r << " g = " << g << " b = " << b
//           << endl;
         colors.push_back(osg::Vec4(r,g,b,alpha));
      }
   }

   colorrange_refptr->setColors(colors);
   scalarbar_geode_refptr->setTitle("");

   osgSim::ScalarBar::TextProperties text_properties;
   text_properties._fontFile="fonts/arial.ttf";
   text_properties._fontResolution.first=200;
   text_properties._fontResolution.second=200;
   text_properties._characterSize=0;
   text_properties._color=osg::Vec4(1,1,1,1);
   scalarbar_geode_refptr->setTextProperties(text_properties);
}

// --------------------------------------------------------------------------
// Member function reset_labels() implements our own horizontally
// oriented labels for the colorbar.  We do not like the built-in
// color bar text label capabilities in OSG1.2.  

void ColorbarHUD::reset_labels()
{
//   cout << "inside ColorbarHUD::reset_labels()" << endl;
   float characterSize = 0.04;
   osg::Vec4 text_color(1,1,1,1);

   labels_geode_refptr->removeDrawables(
      0,labels_geode_refptr->getNumDrawables());

   for (int i=0; i<numLabels; i++)
   {
      osgText::Text* text = new osgText::Text;
      text->setFont("fonts/times.ttf");
      text->setColor(text_color);
      text->setCharacterSize(characterSize);

// Increase backdropoffset from its default value of 0.07 in order to
// amplify black border behind text labels.  This helps to make
// colorbar labels stand out against noisy, colored backgrounds:

      text->setBackdropOffset(0.15);

      double curr_frac=double(i)/double(numLabels-1);
      double scalar_value=scalar_value_start[colorbar_index]+curr_frac*(
         scalar_value_stop[colorbar_index]-
         scalar_value_start[colorbar_index]);
//      cout << "scalar_value = " << scalar_value << endl;
//      cout << "truncate_value_flag = " << truncate_value_flag << endl;
      
      string curr_label;
      if (truncate_value_flag)
      {
         scalar_value=basic_math::mytruncate(scalar_value);
         curr_label=stringfunc::number_to_string(scalar_value);
      }
      else
      {
         curr_label=stringfunc::number_to_string(scalar_value,2);
      }
      text->setText(curr_label);

//      cout << "curr_label = " << curr_label << endl;

      double delta_x=-0.5*curr_label.size()*characterSize;

      double delta_y=1.0/double(numLabels-1);
//      cout << "delta_y = " << delta_y << endl;
      double ychar_offset=0.01;

      osg::Vec3 label_posn;
      if (is_odd(numLabels))
      {
         label_posn=
            osg::Vec3(delta_x,(i-numLabels/2)*delta_y-ychar_offset,0);
      }
      else
      {
         label_posn=
            osg::Vec3(delta_x,(i-0.5*(numLabels-1))*delta_y-ychar_offset,0);
      }
      text->setPosition(label_posn);
      text->setAxisAlignment(osgText::Text::XY_PLANE);
      text->setBackdropType(osgText::Text::OUTLINE);

      labels_geode_refptr->addDrawable(text);
   } // loop over index i labeling colorbar labels
}

