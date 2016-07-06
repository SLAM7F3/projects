// Note added on Monday, Oct 9, 2006: Should swap order of UTMnorthing
// and UTMeasting in LLtoUTM and UTMtoLL !!!

// ==========================================================================
// Header file for LatLong-UTM conversion methods
// ==========================================================================
// Last modified on 2/5/10; 3/30/10; 5/9/11
// ==========================================================================

#ifndef LATLONGCONV
#define LATLONGCONV

#include <iostream>
#include <math.h>
#include "astro_geo/geopoint.h"

class threevector;

namespace latlongfunc
{
   double dms_to_decimal_degs(int degrees,int minutes,double seconds);
   double dm_to_decimal_degs(int degrees,double minutes);
   void decimal_degs_to_dms(
      double decimal_degs,int& degrees,int& minutes,double& secs);
   void decimal_degs_to_int_degs_decimal_minutes(
      double decimal_degs,int& degrees,double& minutes);

// Note:  The default ellipsoid (#23) corresponds to WGS-84

   void LLtoUTM(
      const double Lat,const double Long,
      int& ZoneNumber,bool& northern_hemisphere_flag,
      double &UTMNorthing,double &UTMEasting,int ReferenceEllipsoid=23);
   void LLtoUTM(
      const double Lat,const double Long,int ZoneNumber_offset,
      int& ZoneNumber,bool& northern_hemisphere_flag,
      double &UTMNorthing,double &UTMEasting,int ReferenceEllipsoid=23);

   int LL_to_UTM_zonenumber(
      const double longitude,const double latitude,int ReferenceEllipsoid=23);
   void LL_to_northing_easting(
      const double longitude,const double latitude,
      const bool specified_northern_hemisphere_flag,
      const int specified_ZoneNumber,
      double& easting,double& northing,int ReferenceEllipsoid=23);
   void UTMtoLL(
      int ZoneNumber,bool northern_hemisphere_flag,
      const double UTMNorthing,const double UTMEasting,
      double& Lat,double& Long,int ReferenceEllipsoid=23);
   
   char UTMLetterDesignator(double Lat);
   void LLtoSwissGrid(const double Lat,const double Long,
                      double &SwissNorthing,double& SwissEasting);
   void SwissGridtoLL(const double SwissNorthing,const double SwissEasting,
                      double& Lat,double& Long);
   double NewtonRaphson(const double initEstimate);
   double CorrRatio(double LatRad, const double C);

   geopoint compute_geopoint(
      bool northern_hemisphere_flag,int UTM_zonenumber,
      const threevector& UTM_coords);
   geopoint compute_geopoint(double latitude,double longitude,
                             double altitude=0);

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // latlongfunc namespace

#endif // LATLONG2UTMFUNCS.h



