// ==========================================================================
// Header file for stand-alone astronomical functions 
// ==========================================================================
// Last updated on 8/24/06; 11/27/06; 12/30/07; 9/3/09
// ==========================================================================

#ifndef ASTRO_H
#define ASTRO_H

#include "math/threevector.h"

namespace astrofunc
{

// Time methods:

   double hms_to_decimal_degs(int hours,int minutes,double seconds);
   void decimal_degs_to_hms(
      double decimal_degs,int& hours,int& minutes,double& secs);
   void secs_since_midnight_to_hms(
      double secs_since_midnight,int& hours,int& minutes,double& secs);

   double julian_day(int year,int month,double day);
   void julian_to_calendar_date(
      double julian_day,int& year,int& month,double& day);
   double GMT_corresponding_to_juliandate(double juliandate);
   double greenwich_sidereal_time(double juliandate);
   double greenwich_sidereal_time(double juliandate,double GMT);
   double compute_GST_b_constant(double juliandate);
   double sidereal_time(double reference_time,double currtime);

   double days_elapsed_since_epoch(int year,int month,double day);
   double days_elapsed_since_epoch(double juliandate);
   bool IsLeapYear(int year);

// Coordinate transformation methods:

   void compute_RA_DEC(const threevector& V,double& alpha,double& delta);
   threevector ECI_vector(double& alpha,double& delta);
   void ecliptic_to_equatorial_coords(
      int year,int month,double day,
      double lambda,double beta,double& alpha,double& delta);
   void ecliptic_to_equatorial_coords(
      double juliandate,double lambda,double beta,
      double& alpha,double& delta);
   double mean_ecliptic_obliquity(int year,int month,double day);
   double mean_ecliptic_obliquity(double juliandate);

   void compute_sun_longitude(
      double elapsed_days,double& m,double& lambda_solar);
   void sun_right_ascension_declination(
      int year,int month,double day,double& alpha,double& delta);
   void sun_right_ascension_declination(
      double juliandate,double& alpha,double& delta);
   void sun_RA_DEC(
      double reference_time,double currtime,double& sun_RA,double& sun_DEC);
}

#endif  // astro.h




