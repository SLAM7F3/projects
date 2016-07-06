// ==========================================================================
// Header file for genericHUD class 
// ==========================================================================
// Last modified on 2/11/08; 2/12/08; 12/31/11
// ==========================================================================

#ifndef GENERIC_HUD
#define GENERIC_HUD

#include <string>
#include <osg/Geode>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Referenced>
#include <osgText/Text>
#include "color/colorfuncs.h"

class GenericHUD : public osg::Referenced
{
  public:

   // sets up image viewing size and other defaults

   GenericHUD( 
      const int p_minx, const int p_maxx, 
      const int p_miny, const int p_maxy );
   
   void setFont( const std::string & p_fontname );
   void setText( const std::string & p_text , int line_number=0);
   void set_text_color(colorfunc::Color text_color);
   void set_text_backdrop_type(osgText::Text::BackdropType bdt);
   void setCharacterSize(const float p_size,int line_number=0);
   void reset_text_size(double magnification_factor,int line_number);
   void setModeCharacterSize();
   void setPosition(const float p_x, const float p_y, const float p_z,
                    int line_number=0);
   void setPosition(osg::Vec3 vec,int line_number=0);
   
   void setDefaultPosition();
   osg::Projection* getProjection() const;

   osg::Vec3 getPosition();
   void setAlignment(osgText::Text::AlignmentType at);

  protected:

   int m_minx,m_maxx,m_miny,m_maxy;
   osg::ref_ptr<osg::Projection> m_projection_refptr;
   osg::ref_ptr<osg::MatrixTransform> m_matrixTransform_refptr;
   osg::ref_ptr<osg::Geode> m_geode_refptr;

   osg::ref_ptr<osgText::Text> m_text_refptr;
   osg::ref_ptr<osgText::Text> m_text2_refptr;
   osg::ref_ptr<osgText::Text> m_text3_refptr;
   osg::ref_ptr<osgText::Font> m_font_refptr;

   double m_windowScaleFrac;
   float m_layoutCharacterSize;
   float m_margin;

//   virtual ~GenericHUD=0;

  private:

   void allocate_member_objects();
   void initialize_member_objects();
};


// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void GenericHUD::setText( const std::string & p_text , int line_number)
{
   if (line_number==0)
   {
      m_text_refptr->setText( p_text );
   }
   else if (line_number==1)
   {
      m_text2_refptr->setText( p_text );
   }
   else if (line_number==2)
   {
      m_text3_refptr->setText( p_text );
   }
}

inline void GenericHUD::setCharacterSize(const float p_size,int line_number)
{
   m_layoutCharacterSize = p_size;
   if (line_number==0)
   {
      m_text_refptr->setCharacterSize( m_layoutCharacterSize );
   }
   else if (line_number==1)
   {
      m_text2_refptr->setCharacterSize( m_layoutCharacterSize );
   }
   else if (line_number==2)
   {
      m_text3_refptr->setCharacterSize( m_layoutCharacterSize );
   }
}

inline void GenericHUD::setPosition(
   const float p_x, const float p_y, const float p_z,int line_number)
{
   if (line_number==0)
   {
      m_text_refptr->setPosition( osg::Vec3(p_x, p_y, p_z));
   }
   else if (line_number==1)
   {
      m_text2_refptr->setPosition( osg::Vec3(p_x, p_y, p_z));
   }
   else if (line_number==2)
   {
      m_text3_refptr->setPosition( osg::Vec3(p_x, p_y, p_z));
   }
}
   
inline void GenericHUD::setPosition(osg::Vec3 vec,int line_number)
{
   if (line_number==0)
   {
      m_text_refptr->setPosition(vec);
   }
   else if (line_number==1)
   {
      m_text2_refptr->setPosition(vec);
   }
   else if (line_number==2)
   {
      m_text3_refptr->setPosition(vec);
   }
}

inline osg::Vec3 GenericHUD::getPosition()
{
   return m_text_refptr->getPosition();
}

inline void GenericHUD::setAlignment(osgText::Text::AlignmentType at)
{
   m_text_refptr->setAlignment(at);
}

inline void GenericHUD::setModeCharacterSize()
{
   int maxx=m_maxx*2;
   m_windowScaleFrac = (maxx - 512.0) / (512.0);
   if (m_windowScaleFrac < 0) m_windowScaleFrac = 0;
   
   setCharacterSize( float( 20.0*(1+0.5*m_windowScaleFrac) ) );
}

inline void GenericHUD::setDefaultPosition()
{
   setPosition( m_margin, m_maxy - m_margin, 0 , 0 );
   setPosition( m_margin, m_maxy - 2*m_margin, 0 , 1 );
   setPosition( m_margin, m_maxy - 3*m_margin, 0 , 2 );
}

inline osg::Projection* GenericHUD::getProjection() const
{ 
   return m_projection_refptr.get(); 
}


#endif
