// ==========================================================================
// Header file for ArmySymbol class
// ==========================================================================
// Last updated on 9/14/06; 8/20/09; 3/31/11
// ==========================================================================

#ifndef ArmySymbol_H
#define ArmySymbol_H

#include "osg/osgGeometry/Box.h"

// class osg::Group;

class ArmySymbol : public Box
{

  public:
    
// Initialization, constructor and destructor functions:

   ArmySymbol(double w,double l,double h,int id);
   ArmySymbol(double w,double l,double h,double displacement,int id);
   virtual ~ArmySymbol();
   friend std::ostream& operator<< (
      std::ostream& outstream,const ArmySymbol& s);

// Set & get methods:

   void set_symbol_type(int symboltype);
   int get_symbol_type() const;
   void set_pointing_direction(const threevector& r_hat);
   const threevector& get_pointing_direction() const;


// Drawing methods:

   osg::Group* generate_drawable_group();
   void reset_symbol_image();
   std::string get_image_filename_corresponding_to_symbol_type();

  protected:

  private:

   int symbol_type;
   threevector pointing_direction;
   osg::Group* group_ptr;
   
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ArmySymbol& s);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ArmySymbol::set_symbol_type(int symboltype)
{
   symbol_type=symboltype;
}

inline int ArmySymbol::get_symbol_type() const
{
   return symbol_type;
}

inline void ArmySymbol::set_pointing_direction(const threevector& r_hat)
{
   pointing_direction=r_hat;
}

inline const threevector& ArmySymbol::get_pointing_direction() const
{
   return pointing_direction;
}

#endif // ArmySymbol.h



   
