// ==========================================================================
// Header file for geomagnet class
// ==========================================================================
// Last modified on 7/31/07; 8/1/07
// ==========================================================================

#ifndef GEOMAGNET_H
#define GEOMAGNET_H

#include <string>
#include "astro_geo/geopoint.h"

class geomagnet
{
   
  public:

// Initialization, constructor and destructor functions:

   geomagnet(void);
   geomagnet(const geomagnet& g);
   ~geomagnet();
   geomagnet operator= (const geomagnet& g);
   friend std::ostream& operator<< 
      (std::ostream& outstream,geomagnet& g);

// Set and get methods:

   void set_geolocation(double longitude,double latitude,double altitude);
   void set_geolocation(const geopoint& geolocation);
   geopoint& get_geolocation();
   const geopoint& get_geolocation() const;
   double get_delta_yaw() const;

   void input_geolocation_and_time();

   void E0000(
      int IENTRY, int *maxdeg, float alt, float glat, float glon, 
      float time, float *dec, float *dip, float *ti, float *gv);
   void geomg1(
      float alt, float glat, float glon, float time, float *dec, float *dip, 
      float *ti, float *gv);
   void geomag_introduction(float epochlowlim);
   void compute_magnetic_field_components();
   void display_magnetic_field_components();

  private: 

   int maxdeg,warn_H,warn_H_strong,warn_P;
   float warn_H_val,warn_H_strong_val;
   float epochlowlim,epochuplim;
   float dec1,adec,adip;
   float ddeg,ideg,dmin,imin;
   std::string decd_str,dipd_str;
   std::string data_filename;
   geopoint curr_geolocation;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const geomagnet& g);

   void initialize_params();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void geomagnet::set_geolocation(
   double longitude,double latitude,double altitude)
{
   curr_geolocation=geopoint(longitude,latitude,altitude);
}

inline void geomagnet::set_geolocation(const geopoint& geolocation)
{
   curr_geolocation=geopoint(geolocation.get_longitude(),
                             geolocation.get_latitude(),
                             geolocation.get_altitude());
}

inline geopoint& geomagnet::get_geolocation()
{
   return curr_geolocation;
}

inline const geopoint& geomagnet::get_geolocation() const
{
   return curr_geolocation;
}

#endif  // geomagnet.h


