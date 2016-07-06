// ==========================================================================
// GenericHUD class member function definitions
// ==========================================================================
// Last modified on 2/12/08; 9/13/08; 12/31/11
// ==========================================================================

#include <iostream>
#include <osg/StateSet>
#include "color/colorfuncs.h"
#include "osg/GenericHUD.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GenericHUD::allocate_member_objects()
{
}		       

void GenericHUD::initialize_member_objects()
{
}		       

GenericHUD::GenericHUD( 
   const int p_minx, const int p_maxx, const int p_miny, const int p_maxy ):
   m_minx( p_minx ), m_maxx( p_maxx ), m_miny( p_miny ), m_maxy( p_maxy )
{ 
   allocate_member_objects();
   initialize_member_objects();

   m_projection_refptr = new osg::Projection;
   m_projection_refptr->getOrCreateStateSet()->setMode(
      GL_DEPTH_TEST,osg::StateAttribute::OFF);
   m_projection_refptr->getOrCreateStateSet()->setRenderBinDetails(
      523,"RenderBin");
   m_projection_refptr->setMatrix( osg::Matrix::ortho2D( 
      m_minx, m_maxx, m_miny, m_maxy ));

   m_matrixTransform_refptr = new osg::MatrixTransform;
   m_matrixTransform_refptr->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
   m_projection_refptr->addChild( m_matrixTransform_refptr.get() );
  
   m_geode_refptr = new osg::Geode;
   m_matrixTransform_refptr->addChild( m_geode_refptr.get() );

   m_text_refptr = new osgText::Text;
   m_text2_refptr = new osgText::Text;
   m_text3_refptr = new osgText::Text;
   m_geode_refptr->addDrawable( m_text_refptr.get() );
   m_geode_refptr->addDrawable( m_text2_refptr.get() );
   m_geode_refptr->addDrawable( m_text3_refptr.get() );

   m_windowScaleFrac = (m_maxx - 512.0) / (512.0);
   if (m_windowScaleFrac < 0) m_windowScaleFrac = 0;
  
// FAKE FAKE: Cluge added on 7/21/05 to circumvent font problems:

   setFont("fonts/arial_3D.ttf");
   set_text_color(colorfunc::brightpurple);
   
   setCharacterSize( float( 20.0*(1+0.5*m_windowScaleFrac) ) );
   m_margin = float( 25.0*(1+0.5*m_windowScaleFrac ));
  
   setText("");
};

void GenericHUD::setFont( const std::string & p_fontname )
{
   m_font_refptr = osgText::readFontFile( p_fontname.c_str() );
   m_text_refptr->setFont( m_font_refptr.get() );
   m_text2_refptr->setFont( m_font_refptr.get() );
   m_text3_refptr->setFont( m_font_refptr.get() );
}

void GenericHUD::set_text_color(colorfunc::Color text_color)
{
   m_text_refptr->setColor(colorfunc::get_OSG_color(text_color));
   m_text2_refptr->setColor(colorfunc::get_OSG_color(text_color));
   m_text3_refptr->setColor(colorfunc::get_OSG_color(text_color));
}

void GenericHUD::set_text_backdrop_type(osgText::Text::BackdropType bdt)
{
   m_text_refptr->setBackdropType(bdt);
   m_text2_refptr->setBackdropType(bdt);
   m_text3_refptr->setBackdropType(bdt);
}

void GenericHUD::reset_text_size(double magnification_factor,int line_number)
{
   setCharacterSize( float( 20.0*(1+0.5*m_windowScaleFrac) ), line_number );
   if (line_number==0)
   {
      m_text_refptr->setCharacterSize(m_text_refptr->getCharacterHeight()*
                                      magnification_factor);
   }
   else if (line_number==1)
   {
      m_text2_refptr->setCharacterSize(m_text2_refptr->getCharacterHeight()*
                                       magnification_factor);
   }
   else if (line_number==2)
   {
      m_text3_refptr->setCharacterSize(m_text3_refptr->getCharacterHeight()*
                                       magnification_factor);
   }
}
