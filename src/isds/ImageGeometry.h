// ==========================================================================
// Header file for ImageGeometry class
// ==========================================================================
// Last updated on 10/19/06
// ==========================================================================

#ifndef ImageGeometry_H
#define ImageGeometry_H

#include <iostream>
#include <string>

class ImageGeometry 
{

  public:
    
// Initialization, constructor and destructor functions:

   ImageGeometry();
   virtual ~ImageGeometry();
   friend std::ostream& operator<< (
      std::ostream& outstream,const ImageGeometry& i);

// Set & get methods:

   void set_imageID(std::string id);
   void set_wktGeometry(std::string geom);
   void set_pixelWidth(int pw);
   void set_pixelHeight(int ph);

   std::string get_imageID() const;
   std::string get_wktGeometry() const;

  private:

   std::string imageID,wktGeometry;
   int pixelWidth,pixelHeight;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void ImageGeometry::set_imageID(std::string id)
{
   imageID=id;
}

inline void ImageGeometry::set_wktGeometry(std::string geom)
{
   wktGeometry=geom;
}

inline void ImageGeometry::set_pixelWidth(int pw)
{
   pixelWidth=pw;
}

inline void ImageGeometry::set_pixelHeight(int ph)
{
   pixelHeight=ph;
}

inline std::string ImageGeometry::get_imageID() const
{
   return imageID;
}

inline std::string ImageGeometry::get_wktGeometry() const
{
   return wktGeometry;
}


#endif // ImageGeometry.h



