// ==========================================================================
// Header file for Panorama class
// ==========================================================================
// Last updated on 3/23/11; 7/31/11; 8/16/11; 8/17/11
// ==========================================================================

#ifndef PANORAMA_H
#define PANORAMA_H

#include <iostream>
#include <string>
#include "osg/osgAnnotators/ArmySymbol.h"
#include "osg/osgGeometry/Geometrical.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "math/threevector.h"

class Panorama : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   Panorama(OBSFRUSTAGROUP* OFG_ptr,osgText::Font* f_ptr,int ID);
   virtual ~Panorama();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Panorama& p);

// Set & get methods:

   void set_posn(const threevector& posn);
   threevector get_posn() const;
   void pushback_OBSFRUSTUM_ID(int id);
   int get_n_OBSFRUSTA() const;
   int get_OBSFRUSTUM_ID(int i) const;
   OBSFRUSTUM* get_OBSFRUSTUM_ptr(int i);
   int get_OBSFRUSTUM_index(int ID);
   void pushback_ArmySymbol_ptr(ArmySymbol* AS_ptr);
   std::vector<ArmySymbol*>& get_ArmySymbol_ptrs();

// Drawing & text methods:

   osg::Geode* generate_drawable_geode();
   std::string generate_ID_label(double delta_z,double text_size);

// Imageplane proximity member functions:

   double find_closest_imageplane_to_world_posn(
      const threevector& XYZ,int& closest_OBSFRUSTUM_ID,
      twovector& closest_UV);

   void azimuthally_spin(double t,int pass_number,double theta);

  protected:

  private:

   threevector posn;
   std::vector<int> OBSFRUSTA_IDs;
   OBSFRUSTAGROUP* OBSFRUSTAGROUP_ptr;	
			// just pointer to pre-existing OBSFRUSTAGROUP
   std::vector<ArmySymbol*> ArmySymbol_ptrs;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Panorama& p);

   void generate_text(int i,osg::Geode* geode_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:


inline void Panorama::set_posn(const threevector& posn)
{
   this->posn=posn;
}

inline threevector Panorama::get_posn() const
{
   return posn;
}

inline int Panorama::get_n_OBSFRUSTA() const
{
   return OBSFRUSTA_IDs.size();
}

inline int Panorama::get_OBSFRUSTUM_ID(int i) const
{
   return OBSFRUSTA_IDs[i];
}

inline void Panorama::pushback_ArmySymbol_ptr(ArmySymbol* AS_ptr)
{
   ArmySymbol_ptrs.push_back(AS_ptr);
}

inline std::vector<ArmySymbol*>& Panorama::get_ArmySymbol_ptrs()
{
   return ArmySymbol_ptrs;
}



#endif // Panorama.h



