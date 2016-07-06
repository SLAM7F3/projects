// ==========================================================================
// LatLong to UTM conversion methods
// ==========================================================================
// Last modified on 8/28/09; 2/5/10; 5/9/11; 4/4/14
// ==========================================================================

// Lat Long - UTM, UTM - Lat Long conversions

// Reference ellipsoids derived from Peter H. Dana's website-
// http://www.utexas.edu/depts/grg/gcraft/notes/datum/elist.html
// Department of Geography, University of Texas at Austin Internet:
// pdana@mail.utexas.edu 3/22/95

// Source Defense Mapping Agency. 1987b. DMA Technical Report:
// Supplement to Department of Defense World Geodetic System 1984
// Technical Report. Part I and II. Washington, DC: Defense Mapping
// Agency

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "astro_geo/Ellipsoid.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "math/threevector.h"

using std::cout;
using std::endl;

namespace latlongfunc
{
   const double FOURTHPI=0.25*PI;
   const double DEG2RAD = PI / 180.0;
   const double RAD2DEG = 180.0 / PI;

   static Ellipsoid ellipsoid[] =
   {                                                 

//  Recall ellipsoid class constructor input parameters = 
//    id, Ellipsoid name, Equatorial Radius (meters), square of eccentricity.

      Ellipsoid( -1, "Placeholder", 0, 0), // placeholder only, To
    // allow array indices to match id numbers

      Ellipsoid( 1, "Airy", 6377563, 0.00667054),
      Ellipsoid( 2, "Australian National", 6378160, 0.006694542),
      Ellipsoid( 3, "Bessel 1841", 6377397, 0.006674372),
      Ellipsoid( 4, "Bessel 1841 (Nambia) ", 6377484, 0.006674372),
      Ellipsoid( 5, "Clarke 1866", 6378206, 0.006768658),
      Ellipsoid( 6, "Clarke 1880", 6378249, 0.006803511),
      Ellipsoid( 7, "Everest", 6377276, 0.006637847),
      Ellipsoid( 8, "Fischer 1960 (Mercury) ", 6378166, 0.006693422),
      Ellipsoid( 9, "Fischer 1968", 6378150, 0.006693422),
      Ellipsoid( 10, "GRS 1967", 6378160, 0.006694605),
      Ellipsoid( 11, "GRS 1980", 6378137, 0.00669438),
      Ellipsoid( 12, "Helmert 1906", 6378200, 0.006693422),
      Ellipsoid( 13, "Hough", 6378270, 0.00672267),
      Ellipsoid( 14, "International", 6378388, 0.00672267),
      Ellipsoid( 15, "Krassovsky", 6378245, 0.006693422),
      Ellipsoid( 16, "Modified Airy", 6377340, 0.00667054),
      Ellipsoid( 17, "Modified Everest", 6377304, 0.006637847),
      Ellipsoid( 18, "Modified Fischer 1960", 6378155, 0.006693422),
      Ellipsoid( 19, "South American 1969", 6378160, 0.006694542),
      Ellipsoid( 20, "WGS 60", 6378165, 0.006693422),
      Ellipsoid( 21, "WGS 66", 6378145, 0.006694542),
      Ellipsoid( 22, "WGS-72", 6378135, 0.006694318),
      Ellipsoid( 23, "WGS-84", 6378137, 0.00669438)
   };

// Use "private" global tensors A, B and C to hold ALIRT/Google
// calibration information for Boston 2005 map:
   
   tensor* A_ptr=NULL;
   genmatrix* B_ptr=NULL;
   twovector* C_ptr=NULL;

   genmatrix* M_ptr=NULL;
   double avg_sealevel_Z=0;

// --------------------------------------------------------------------------
// Method dms_to_decimal_degs converts an angular position measured in
// degrees, minutes and seconds to decimal degrees:
   
   double dms_to_decimal_degs(int degrees,int minutes,double seconds)
      {
         return degrees+minutes/60.0+seconds/3600.0;
      }
   
   double dm_to_decimal_degs(int degrees,double minutes)
      {
         return degrees+minutes/60.0;
      }
   
   void decimal_degs_to_dms(
      double decimal_degs,int& degrees,int& minutes,double& secs)
      {

// Perform deg, min, sec decomposition on absolute value of
// decimal_degs.  Return final results multiplied by minus sign if
// input decimal_degs < 0:

         int sgn=1;
         if (decimal_degs < 0)
         {
            sgn=-1;
            decimal_degs *= -1;
         }

         degrees=basic_math::mytruncate(decimal_degs);
         double delta_minutes=60*(decimal_degs-degrees);
         minutes=basic_math::mytruncate(delta_minutes);
         secs=60*(delta_minutes-minutes);
//            cout << "degs = " << degrees << " mins = " << minutes
//                 << " secs = " << secs << endl;
         degrees *= sgn;
         minutes *= sgn;
         secs *= sgn;
      }

   void decimal_degs_to_int_degs_decimal_minutes(
      double decimal_degs,int& degrees,double& minutes)
      {

// Perform deg, min decomposition on absolute value of
// decimal_degs.  Return final results multiplied by minus sign if
// input decimal_degs < 0:

         int sgn=1;
         if (decimal_degs < 0)
         {
            sgn=-1;
            decimal_degs *= -1;
         }

         degrees=basic_math::mytruncate(decimal_degs);
         minutes=60*(decimal_degs-degrees);

         degrees *= sgn;
         minutes *= sgn;
      }

// --------------------------------------------------------------------------
// Method LLtoUTM converts lat/long to UTM coords.  Equations from
// USGS Bulletin 1532.  East Longitudes are positive, West longitudes
// are negative.  North latitudes are positive, South latitudes are
// negative. Lat and Long are in decimal degrees. Written by Chuck
// Gantz- chuck.gantz@globalstar.com

   void LLtoUTM(
      const double Lat,const double Long,
      int& ZoneNumber,bool& northern_hemisphere_flag,
      double &UTMNorthing,double &UTMEasting,int ReferenceEllipsoid)
      {
         //Make sure the longitude is between -180.00 .. 179.9
         // -180.00 .. 179.9;

         double LongTemp = (Long+180)-int((Long+180)/360)*360-180;

         double LatRad = Lat*DEG2RAD;
         double LongRad = LongTemp*DEG2RAD;
         ZoneNumber = int((LongTemp + 180)/6) + 1;
         northern_hemisphere_flag=(Lat >= 0);

         if( Lat >= 56.0 && Lat < 64.0 && LongTemp >= 3.0 && LongTemp < 12.0 )
            ZoneNumber = 32;

         // Special zones for Svalbard
         if( Lat >= 72.0 && Lat < 84.0 )
         {
            if(      LongTemp >= 0.0  && LongTemp <  9.0 ) ZoneNumber = 31;
            else if( LongTemp >= 9.0  && LongTemp < 21.0 ) ZoneNumber = 33;
            else if( LongTemp >= 21.0 && LongTemp < 33.0 ) ZoneNumber = 35;
            else if( LongTemp >= 33.0 && LongTemp < 42.0 ) ZoneNumber = 37;
         }
         double LongOrigin = (ZoneNumber - 1)*6 - 180 + 3;    
			//+3 puts origin in middle of zone
         double LongOriginRad = LongOrigin * DEG2RAD;

         double eccSquared = 
            ellipsoid[ReferenceEllipsoid].get_eccentricitySquared();
         double eccPrimeSquared = (eccSquared)/(1-eccSquared);

         double a = ellipsoid[ReferenceEllipsoid].get_EquatorialRadius();
         double N = a/sqrt(1-eccSquared*sin(LatRad)*sin(LatRad));
         double T = tan(LatRad)*tan(LatRad);
         double C = eccPrimeSquared*cos(LatRad)*cos(LatRad);
         double A = cos(LatRad)*(LongRad-LongOriginRad);

         double M = 
            a*((1   - eccSquared/4      - 3*eccSquared*eccSquared/64    
                - 5*eccSquared*eccSquared*eccSquared/256)*LatRad
               - (3*eccSquared/8   + 3*eccSquared*eccSquared/32    
                  + 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatRad)
               + (15*eccSquared*eccSquared/256 
                  + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatRad)
               - (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatRad));
         
         const double k0 = 0.9996;
         UTMEasting = (double)(k0*N*(A+(1-T+C)*A*A*A/6
                                     + (5-18*T+T*T+72*C-58*eccPrimeSquared)
                                     *A*A*A*A*A/120)
                               + 500000.0);

         UTMNorthing = (double)(k0*(M+N*tan(LatRad)*
                                    (A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
                                     + (61-58*T+T*T+600*C-330*eccPrimeSquared)
                                     *A*A*A*A*A*A/720)));
         if(Lat < 0)
            UTMNorthing += 10000000.0;                
			//10000000 meter offset for southern hemisphere
      }

// --------------------------------------------------------------------------
   void LLtoUTM(
      const double Lat,const double Long,int ZoneNumber_offset,
      int& ZoneNumber,bool& northern_hemisphere_flag,
      double &UTMNorthing,double &UTMEasting,int ReferenceEllipsoid)
      {
         //Make sure the longitude is between -180.00 .. 179.9

         double LongTemp = (Long+180)-int((Long+180)/360)*360-180;

         double LatRad = Lat*DEG2RAD;
         double LongRad = LongTemp*DEG2RAD;
         ZoneNumber = int((LongTemp + 180)/6) + 1;
         northern_hemisphere_flag=(Lat >= 0);

         if( Lat >= 56.0 && Lat < 64.0 && LongTemp >= 3.0 && LongTemp < 12.0 )
            ZoneNumber = 32;

         // Special zones for Svalbard
         if( Lat >= 72.0 && Lat < 84.0 )
         {
            if(      LongTemp >= 0.0  && LongTemp <  9.0 ) ZoneNumber = 31;
            else if( LongTemp >= 9.0  && LongTemp < 21.0 ) ZoneNumber = 33;
            else if( LongTemp >= 21.0 && LongTemp < 33.0 ) ZoneNumber = 35;
            else if( LongTemp >= 33.0 && LongTemp < 42.0 ) ZoneNumber = 37;
         }

// Experiment with offseting true ZoneNumber by some integer:

         ZoneNumber += ZoneNumber_offset;

         double LongOrigin = (ZoneNumber - 1)*6 - 180 + 3;    
			//+3 puts origin in middle of zone
         double LongOriginRad = LongOrigin * DEG2RAD;

         double eccSquared = 
            ellipsoid[ReferenceEllipsoid].get_eccentricitySquared();
         double eccPrimeSquared = (eccSquared)/(1-eccSquared);

         double a = ellipsoid[ReferenceEllipsoid].get_EquatorialRadius();
         double N = a/sqrt(1-eccSquared*sin(LatRad)*sin(LatRad));
         double T = tan(LatRad)*tan(LatRad);
         double C = eccPrimeSquared*cos(LatRad)*cos(LatRad);
         double A = cos(LatRad)*(LongRad-LongOriginRad);

         double M = 
            a*((1   - eccSquared/4      - 3*eccSquared*eccSquared/64    
                - 5*eccSquared*eccSquared*eccSquared/256)*LatRad
               - (3*eccSquared/8   + 3*eccSquared*eccSquared/32    
                  + 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatRad)
               + (15*eccSquared*eccSquared/256 
                  + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatRad)
               - (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatRad));
         
         const double k0 = 0.9996;
         UTMEasting = (double)(k0*N*(A+(1-T+C)*A*A*A/6
                                     + (5-18*T+T*T+72*C-58*eccPrimeSquared)
                                     *A*A*A*A*A/120)
                               + 500000.0);

         UTMNorthing = (double)(k0*(M+N*tan(LatRad)*
                                    (A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
                                     + (61-58*T+T*T+600*C-330*eccPrimeSquared)
                                     *A*A*A*A*A*A/720)));
         if(Lat < 0)
            UTMNorthing += 10000000.0;                
			//10000000 meter offset for southern hemisphere
      }

// --------------------------------------------------------------------------
// Method LL_to_UTM_zonenumber returns the UTM zonenumber
// corresponding to the input (longitude,latitude) pair.

   int LL_to_UTM_zonenumber(
      const double longitude,const double latitude,int ReferenceEllipsoid)
      {
         bool northern_hemisphere_flag;
         int ZoneNumber;
         double easting,northing;
         LLtoUTM(
            latitude,longitude,ZoneNumber,northern_hemisphere_flag,
            northing,easting,ReferenceEllipsoid);
         return ZoneNumber;
      }

// --------------------------------------------------------------------------
// Method LL_to_northing_easting returns the (easting,northing) pair
// corresponding to the input (longitude,latitude) within a
// **specified** UTM zone.  We wrote this method in Nov 2007 to
// accomodate earth regions which straddle a UTM zone boundary
// (e.g. Lubbock, TX).

// On 3/30/2010, we learned the painful and hard way that UTM
// northings experience a 1E7 discontinuity across the equator!  This
// discontinuity fouls up LatLongGrid display.  So
// LL_to_northing_easting() now takes in
// specified_northern_hemisphere_flag.  If this boolean equals true,
// the 1E7 discontinuity is subtracted away from the output northing.

   void LL_to_northing_easting(
      const double longitude,const double latitude,
      const bool specified_northern_hemisphere_flag,
      const int specified_ZoneNumber,double& easting,double& northing,
      int ReferenceEllipsoid)
      {
         bool northern_hemisphere_flag;
         int true_ZoneNumber;

         LLtoUTM(
            latitude,longitude,true_ZoneNumber,northern_hemisphere_flag,
            northing,easting,ReferenceEllipsoid);
         int ZoneNumber_offset=specified_ZoneNumber-true_ZoneNumber;
         int ZoneNumber;
         LLtoUTM(
            latitude,longitude,ZoneNumber_offset,
            ZoneNumber,northern_hemisphere_flag,
            northing,easting,ReferenceEllipsoid);

         if (specified_northern_hemisphere_flag &&
             !northern_hemisphere_flag)
         {
            northing -= 10000000;
         }
      }

// --------------------------------------------------------------------------
// Method UTMtoLL converts UTM coords to lat/long.  Equations from
// USGS Bulletin 1532 East Longitudes are positive, West longitudes
// are negative.  North latitudes are positive, South latitudes are
// negative Lat and Long are in decimal degrees.  Written by Chuck
// Gantz- chuck.gantz@globalstar.com


   void UTMtoLL(
      int ZoneNumber,bool northern_hemisphere_flag,
      const double UTMNorthing,const double UTMEasting,
      double& Lat,double& Long,int ReferenceEllipsoid)
      {
//         cout << "inside latlong2utmfunc::UTMtoLL" << endl;
//         cout << "ZoneNumber = " << ZoneNumber << endl;
         
         double x = UTMEasting - 500000.0;                    
		//remove 500,000 meter offset for longitude
         double y = UTMNorthing;

//         int NorthernHemisphere; // 1 for northern hemisphere, 0 for southern
         if(northern_hemisphere_flag)
	 {
//            NorthernHemisphere = 1; // point is in northern hemisphere
	 }
         else
         {
//            NorthernHemisphere = 0; // point is in southern hemisphere
            y -= 10000000.0;                          
	    // remove 10,000,000 meter offset used for southern hemisphere
         }

         double LongOrigin = (ZoneNumber - 1)*6 - 180 + 3;    
		//+3 puts origin in middle of zone
         double eccSquared = ellipsoid[ReferenceEllipsoid].
            get_eccentricitySquared();
         double eccPrimeSquared = (eccSquared)/(1-eccSquared);

         const double k0 = 0.9996;
         double M = y / k0;
         double a = ellipsoid[ReferenceEllipsoid].get_EquatorialRadius();
         double mu = M/(a*(1-eccSquared/4-3*eccSquared*eccSquared/64
                    -5*eccSquared*eccSquared*eccSquared/256));

         double e1 = (1-sqrt(1-eccSquared))/(1+sqrt(1-eccSquared));
         double phi1Rad = mu    + (3*e1/2-27*e1*e1*e1/32)*sin(2*mu)
            + (21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu)
            +(151*e1*e1*e1/96)*sin(6*mu);
//         double phi1 = phi1Rad*RAD2DEG;

         double N1 = a/sqrt(1-eccSquared*sin(phi1Rad)*sin(phi1Rad));
         double T1 = tan(phi1Rad)*tan(phi1Rad);
         double C1 = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
         double R1 = a*(1-eccSquared)/pow(1-eccSquared*sin(phi1Rad)*
                                          sin(phi1Rad),1.5);
         double D = x/(N1*k0);

         Lat = phi1Rad - (N1*tan(phi1Rad)/R1)*
            (D*D/2-(5+3*T1+10*C1-4*C1*C1-9*eccPrimeSquared)*D*D*D*D/24
             +(61+90*T1+298*C1+45*T1*T1-252*eccPrimeSquared-3*C1*C1)
             *D*D*D*D*D*D/720);
         Lat = Lat * RAD2DEG;

         Long = (D-(1+2*T1+C1)*D*D*D/6+(5-2*C1+28*T1-3*C1*C1+
                                        8*eccPrimeSquared+24*T1*T1)
                 *D*D*D*D*D/120)/cos(phi1Rad);
         Long = LongOrigin + Long * RAD2DEG;

//         cout << "Longitude = " << Long
//              << " Latitude = " << Lat << endl;
      }

// --------------------------------------------------------------------------
// Method UTMLetterDesignator determines the correct UTM letter
// designator for the given latitude returns 'Z' if latitude is
// outside the UTM limits of 84N to 80S Written by Chuck Gantz-
// chuck.gantz@globalstar.com

   char UTMLetterDesignator(double Lat)
      {
         char LetterDesignator;

         if((84 >= Lat) && (Lat >= 72)) LetterDesignator = 'X';
         else if((72 > Lat) && (Lat >= 64)) LetterDesignator = 'W';
         else if((64 > Lat) && (Lat >= 56)) LetterDesignator = 'V';
         else if((56 > Lat) && (Lat >= 48)) LetterDesignator = 'U';
         else if((48 > Lat) && (Lat >= 40)) LetterDesignator = 'T';
         else if((40 > Lat) && (Lat >= 32)) LetterDesignator = 'S';
         else if((32 > Lat) && (Lat >= 24)) LetterDesignator = 'R';
         else if((24 > Lat) && (Lat >= 16)) LetterDesignator = 'Q';
         else if((16 > Lat) && (Lat >= 8)) LetterDesignator = 'P';
         else if(( 8 > Lat) && (Lat >= 0)) LetterDesignator = 'N';
         else if(( 0 > Lat) && (Lat >= -8)) LetterDesignator = 'M';
         else if((-8> Lat) && (Lat >= -16)) LetterDesignator = 'L';
         else if((-16 > Lat) && (Lat >= -24)) LetterDesignator = 'K';
         else if((-24 > Lat) && (Lat >= -32)) LetterDesignator = 'J';
         else if((-32 > Lat) && (Lat >= -40)) LetterDesignator = 'H';
         else if((-40 > Lat) && (Lat >= -48)) LetterDesignator = 'G';
         else if((-48 > Lat) && (Lat >= -56)) LetterDesignator = 'F';
         else if((-56 > Lat) && (Lat >= -64)) LetterDesignator = 'E';
         else if((-64 > Lat) && (Lat >= -72)) LetterDesignator = 'D';
         else if((-72 > Lat) && (Lat >= -80)) LetterDesignator = 'C';
         else LetterDesignator = 'Z';                  

// This is here as an error flag to show that the Latitude is outside
// the UTM limits

         return LetterDesignator;
      }

// --------------------------------------------------------------------------
// Method LLtoSwissGrid converts lat/long to Swiss Grid coords.
// Equations from "Supplementary PROJ.4 Notes- Swiss Oblique Mercator
// Projection", August 5, 1995, Release 4.3.3, by Gerald I. Evenden
// Lat and Long are in decimal degrees This transformation is, of
// course, only valid in Switzerland Written by Chuck Gantz-
// chuck.gantz@globalstar.com

   void LLtoSwissGrid(const double Lat, const double Long,
                      double &SwissNorthing, double &SwissEasting)
      {
         double a = ellipsoid[3].get_EquatorialRadius();    
		 //Bessel ellipsoid
         double eccSquared = ellipsoid[3].get_eccentricitySquared();
         double ecc = sqrt(eccSquared);
         double LongOrigin = 7.43958333;               //E7d26'22.500"
         double LatOrigin = 46.95240556;               //N46d57'8.660"
         double LatRad = Lat*DEG2RAD;
         double LongRad = Long*DEG2RAD;
         double LatOriginRad = LatOrigin*DEG2RAD;
         double LongOriginRad = LongOrigin*DEG2RAD;
         double c = sqrt(1+((eccSquared * pow(cos(LatOriginRad), 4)) 
                            / (1-eccSquared)));
         double sin_LatOriginRad=sin(LatOriginRad);
         double equivLatOrgRadPrime = asin(sin_LatOriginRad/c);

         //eqn. 1
         double K = log(tan(FOURTHPI + equivLatOrgRadPrime/2))
            -c*(log(tan(FOURTHPI + LatOriginRad/2))- ecc/2 
                * log((1+ecc*sin_LatOriginRad)/(1-ecc*sin_LatOriginRad)));
         //eqn 2
         double LongRadPrime = c*(LongRad - LongOriginRad);
         double sin_LatRad=sin(LatRad);
         double w = c*(log(tan(FOURTHPI + LatRad/2))- ecc/2 
                       * log((1+ecc*sin_LatRad) / (1-ecc*sin_LatRad))) + K;
         //eqn 1
         double LatRadPrime = 2 * (atan(exp(w)) - FOURTHPI);

         //eqn 3
         double cos_LatRadPrime=cos(LatRadPrime);
         double sinLatDoublePrime = cos(equivLatOrgRadPrime) * sin(LatRadPrime)
            - sin(equivLatOrgRadPrime) * cos_LatRadPrime*cos(LongRadPrime);
         double LatRadDoublePrime = asin(sinLatDoublePrime);

         //eqn 4
         double sinLongDoublePrime = cos_LatRadPrime*sin(LongRadPrime)/ 
            cos(LatRadDoublePrime);
         double LongRadDoublePrime = asin(sinLongDoublePrime);

         double R = a*sqrt(1-eccSquared) / (1-eccSquared*sin_LatOriginRad* 
                                            sin_LatOriginRad);

         //eqn 5
         SwissNorthing = R*log(tan(FOURTHPI + LatRadDoublePrime/2)) + 200000.0;
         //eqn 6
         SwissEasting = R*LongRadDoublePrime + 600000.0;
      }

// --------------------------------------------------------------------------
   void SwissGridtoLL(const double SwissNorthing, const double SwissEasting,
                      double& Lat, double& Long)
      {
         double a = ellipsoid[3].get_EquatorialRadius(); 
			    //Bessel ellipsoid
         double eccSquared = ellipsoid[3].get_eccentricitySquared();
//         double ecc = sqrt(eccSquared);

         double LongOrigin = 7.43958333;               //E7d26'22.500"
         double LatOrigin = 46.95240556;               //N46d57'8.660"
         double LatOriginRad = LatOrigin*DEG2RAD;
         double LongOriginRad = LongOrigin*DEG2RAD;
         double sin_LatOriginRad=sin(LatOriginRad);
         double R = a*sqrt(1-eccSquared) / (1-eccSquared*sin_LatOriginRad * 
                                            sin_LatOriginRad);

         //eqn. 7
         double LatRadDoublePrime = 2*(atan(exp((SwissNorthing - 200000.0)/R)) 
                                       - FOURTHPI);
         //eqn. 8 with equation corrected
         double LongRadDoublePrime = (SwissEasting - 600000.0)/R;

         double c = sqrt(1+((eccSquared * pow(cos(LatOriginRad), 4)) / 
                            (1-eccSquared)));
         double equivLatOrgRadPrime = asin(sin_LatOriginRad/c);

         double cos_LatRadDoublePrime=cos(LatRadDoublePrime);
         double sinLatRadPrime = cos(equivLatOrgRadPrime)
            *sin(LatRadDoublePrime)
            + sin(equivLatOrgRadPrime)*cos_LatRadDoublePrime*
            cos(LongRadDoublePrime);
         double LatRadPrime = asin(sinLatRadPrime);
         double sinLongRadPrime = cos_LatRadDoublePrime
            *sin(LongRadDoublePrime)/cos(LatRadPrime);
         double LongRadPrime = asin(sinLongRadPrime);
         Long = (LongRadPrime/c + LongOriginRad) * RAD2DEG;
         Lat = NewtonRaphson(LatRadPrime) * RAD2DEG;
      }

// --------------------------------------------------------------------------
   double NewtonRaphson(const double initEstimate)
      {
         double Estimate = initEstimate;
         double tol = 0.00001;
         double eccSquared = ellipsoid[3].get_eccentricitySquared();
         double ecc = sqrt(eccSquared);
         double LatOrigin = 46.95240556;               //N46d57'8.660"
         double LatOriginRad = LatOrigin*DEG2RAD;

         double c = sqrt(1+((eccSquared * pow(cos(LatOriginRad), 4)) / 
                            (1-eccSquared)));
         double sin_LatOriginRad=sin(LatOriginRad);
         double equivLatOrgRadPrime = asin(sin_LatOriginRad/c);

         //eqn. 1
         double K = log(tan(FOURTHPI + equivLatOrgRadPrime/2))
            -c*(log(tan(FOURTHPI + LatOriginRad/2))
                - ecc/2 * log((1+ecc*sin_LatOriginRad) / 
                              (1-ecc*sin_LatOriginRad)));
         double C = (K - log(tan(FOURTHPI + initEstimate/2)))/c;

         double corr;
         do
         {
            corr = CorrRatio(Estimate, C);
            Estimate = Estimate - corr;
         }
         while (fabs(corr) > tol);
         return Estimate;
      }

// --------------------------------------------------------------------------
   double CorrRatio(double LatRad, const double C)
      {
         double eccSquared = ellipsoid[3].get_eccentricitySquared();
         double ecc = sqrt(eccSquared);
         double sin_LatRad=sin(LatRad);
         double corr = (C + log(tan(FOURTHPI + LatRad/2))
                        - ecc/2 * log((1+ecc*sin_LatRad) 
                                      / (1-ecc*sin_LatRad))) 
            * (((1-eccSquared*sin_LatRad*sin_LatRad) * cos(LatRad)) 
               / (1-eccSquared));
         return corr;
      }

// --------------------------------------------------------------------------
// Member function compute_geopoint takes in threevector UTM_coords as
// well as an input string specifying the UTM zone.  It instantiates a
// geopoint object and fills its UTM, decimal lat/long and deg/sec/min
// lat/long variables.

   geopoint compute_geopoint(
      bool northern_hemisphere_flag,int UTM_zonenumber,
      const threevector& UTM_coords)
      {
         geopoint curr_geopoint(northern_hemisphere_flag,UTM_zonenumber,
                                UTM_coords.get(0),UTM_coords.get(1));
         curr_geopoint.set_altitude(UTM_coords.get(2));

         double latitude,longitude;
         latlongfunc::UTMtoLL(
            UTM_zonenumber,northern_hemisphere_flag,
            curr_geopoint.get_UTM_northing(),
            curr_geopoint.get_UTM_easting(),latitude,longitude);
         curr_geopoint.set_longitude(longitude);
         curr_geopoint.set_latitude(latitude);
         return curr_geopoint;
      }

   geopoint compute_geopoint(double latitude,double longitude,double altitude)
      {
         geopoint curr_geopoint(longitude,latitude,altitude);

         bool northern_hemisphere_flag;
         int UTM_zonenumber;
         double UTMNorthing,UTMEasting;
         latlongfunc::LLtoUTM(
            latitude,longitude,UTM_zonenumber,northern_hemisphere_flag,
            UTMNorthing,UTMEasting);

         curr_geopoint.set_northern_hemisphere_flag(northern_hemisphere_flag);
         curr_geopoint.set_UTM_zonenumber(UTM_zonenumber);
         curr_geopoint.set_UTM_easting(UTMEasting);
         curr_geopoint.set_UTM_northing(UTMNorthing);
         return curr_geopoint;
      }


} // latlong namespace
