// ==========================================================================
// Some random astronomical functions.  The algorithms below comes
// from "Practical astronomy with your calculator" by Peter
// Duffett-Smith.  George Zogbi lent us this book in May 2001.

// Important notes: 

// January corresponds to month = 1 rather than month = 0 !

// 1st of January corresponds to day = 1 rather than day = 0 !

// ==========================================================================
// Last updated on 8/1/07; 12/30/07; 2/24/08
// ==========================================================================

#include "math/mathfuncs.h"
#include "astro_geo/astrofuncs.h"

using std::cout;
using std::endl;
using std::ostream;

namespace astrofunc
{

// ==========================================================================
// Time methods
// ==========================================================================

// Method hms_to_decimal_degs converts a time measured in hours,
// minutes and seconds to decimal degrees.  This utility was written
// primarily for RA conversion purposes.

   double hms_to_decimal_degs(int hours,int minutes,double seconds)
      {
         return 15.0*(hours+minutes/60.0+seconds/3600.0);
      }
   
   void decimal_degs_to_hms(
      double decimal_degs,int& hours,int& minutes,double& secs)
      {
         double decimal_hours=decimal_degs/15.0;
         hours=basic_math::mytruncate(decimal_hours);
         double delta_minutes=60*(decimal_hours-hours);
         minutes=basic_math::mytruncate(delta_minutes);
         secs=60*(delta_minutes-minutes);
//         cout << "hours = " << hours << " mins = " << minutes
//              << " secs = " << secs << endl;
      }

   void secs_since_midnight_to_hms(
      double secs_since_midnight,int& hours,int& minutes,double& secs)
      {
         double decimal_degs=secs_since_midnight/240.0;
         decimal_degs_to_hms(decimal_degs,hours,minutes,secs);
      }

// --------------------------------------------------------------------------
// Method julian_day calculates the number of days which have
// elapsed since Greenwich mean noon of 1 Jan 4713 BC.  It is
// important to note that the new Julian day begins at 12h 00m GMT
// (UT), a half day out of step with the civil day.  We believe (as of
// 9/02) that one Julian day spans 24 hours precisely.

   double julian_day(int year,int month,double day)
      {
         int y,m;
         if (month==1 || month==2) 
         {
            m=month+12;
            y=year-1;
         }
         else
         {
            m=month;
            y=year;
         }

         int a,b;
         if ((y > 1583) || (y==1582 && m > 10) || 
             (year==1582 && m==10 && day>15))
         {
            a=basic_math::mytruncate(y/100);
            b=2-a+basic_math::mytruncate(a/4);
         }
         else
         {
            b=0;
         }
         int c=basic_math::mytruncate(365.25*y);
         int d=basic_math::mytruncate(30.6001*(m+1));
         double juliandate=b+c+d+day+1720994.5;
   
         return juliandate;
      }

// --------------------------------------------------------------------------
// Method julian_to_calendar_date converts an input Julian date to
// its counterpart in the Gregorian calendar.  This method is the
// inverse to julian_day().

   void julian_to_calendar_date(
      double julian_day,int& year,int& month,double& day)
      {
         int integer_days=basic_math::mytruncate(julian_day+0.5);
         double f=julian_day+0.5-integer_days;

         int b=0;
         if (integer_days > 2299160)
         {
            int a=basic_math::mytruncate((integer_days-1867216.25)/36524.25);
            b=integer_days+1+a-basic_math::mytruncate(a/4);
         }
         else
         {
//            cout << "Possible error inside astrofunc::julian_to_calendar_date()!" 
//                 << endl;

// Note: According to the algorithm supplied on page 11 of "Practical
// astronomy with your calculator", we are supposed to set
// a=integer_days if integer_days > 2299160.  But we believe this must
// be wrong, for parameter a is not used in the remainder of this
// algorithm.  So as of 15 July 2003, we set b=integer_days in this
// part of the conditional...

            b=integer_days;
         }
         int c=b+1524;
         int d=basic_math::mytruncate((c-122.1)/365.25);
         int e=basic_math::mytruncate(365.25*d);
         int g=basic_math::mytruncate((c-e)/30.6001);
         day=c-e+f-basic_math::mytruncate(30.60001*g);

         if (g < 13.5)
         {
            month=g-1;
         }
         else
         {
            month=g-13;
         }
   
         if (month > 2.5)
         {
            year=d-4716;
         }
         else
         {
            year=d-4715;
         }
      }

// --------------------------------------------------------------------------
// Method GMT_corresponding_to_juliandate returns the GMT (=UTC)
// time in hours corresponding to the fractional day part of a precise
// julian date:

   double GMT_corresponding_to_juliandate(double juliandate)
      {
         int year,month;
         double day;
         julian_to_calendar_date(juliandate,year,month,day);
         double frac_day=day-basic_math::mytruncate(day);
         return frac_day*24;
      }

// --------------------------------------------------------------------------
// Method greenwich_sidereal_time returns the Greenwich mean sidereal
// time (GST) in hours corresponding to a precise julian date:

   double greenwich_sidereal_time(double juliandate)
      {
         double GMT=GMT_corresponding_to_juliandate(juliandate);
         return greenwich_sidereal_time(juliandate,GMT);
      }

// --------------------------------------------------------------------------
// This overloaded version of method greenwich_sidereal_time converts
// Greenwich mean time (GMT) = Coordinated Universal Time (UTC)
// specified in hours into Greenwich mean sidereal time (GST) in
// hours.  This method discards the fractional day part within input
// juliandate parameter and takes account of this fractional
// information within the input GMT parameter instead.

   double greenwich_sidereal_time(double juliandate,double GMT)
      {
         const double a=0.0657098;
         const double c=1.002738;

         int year,month;
         double day;
         julian_to_calendar_date(juliandate,year,month,day);
         int days_elapsed=
            basic_math::mytruncate(days_elapsed_since_epoch(year,month,day)-
                                   days_elapsed_since_epoch(year,1,0));
         double b=compute_GST_b_constant(juliandate);
         double T0=a*days_elapsed-b;
         T0 += c*GMT;
         if (T0 > 24)
         {
            T0 -= 24;
         }
         else if (T0 < 0)
         {
            T0 += 24;
         }
         return T0;
      }

// --------------------------------------------------------------------------
// Method compute_GST_b_constant returns the constant b entering
// into the procedure used to calculate Greenwich mean sidereal time:

   double compute_GST_b_constant(double juliandate)
      {
         int year,month;
         double day;

         julian_to_calendar_date(juliandate,year,month,day);
         double reduced_juliandate=julian_day(year,1,0);
         double s=reduced_juliandate-2415020;
         double t=s/36525;
         double r=6.6460656+(2400.051262*t)+(0.00002581*sqr(t));
         double u=r-24*(year-1900);
         double b=24-u;
         return b;
      }

// ---------------------------------------------------------------------
// Method function sidereal_time takes in some time measured in
// seconds since midnight UTC on the pass date specified by pass_date
// (measured in seconds since midnight on 1970-01-01.  It returns the
// corresponding sidereal time.  We wrote this utility primarily for
// synthetic pass generation purposes.

   double sidereal_time(double reference_time,double currtime)
      {
//   const double ref_julian_date=astrofunc::julian_day(1970,1,1); // 1970-01-01 00:00:00
         const double ref_julian_date=2440587.5;  // 1970-01-01 00:00:00

         double days_since_ref=(reference_time+currtime)/(24.0*3600.0);// days
         double curr_julian_date=ref_julian_date+days_since_ref;   // days
         return greenwich_sidereal_time(curr_julian_date);
      }

// --------------------------------------------------------------------------
// Method days_elapsed_since_epoch calculates the number of days
// which have elapsed since some input calendar date:

   double days_elapsed_since_epoch(int year,int month,double day)
      {
//       const double ref_julian_date=astrofunc::julian_day(1980,1,0); 
         const double reference_julian_date=2444238.5; // 1980 January 0.0

         double juliandate=julian_day(year,month,day);   
         double days_elapsed=juliandate-reference_julian_date;
         return days_elapsed;
      }

   double days_elapsed_since_epoch(double juliandate)
      {
         const double reference_julian_date=2444238.5; // 1980 January 0.0

         double days_elapsed=juliandate-reference_julian_date;
         return days_elapsed;
      }

// --------------------------------------------------------------------------
   bool IsLeapYear(int year)
      {
         return ( ((year %4 ==0) && (year % 100 != 0)) || (year % 400 ==0) );
      }

// ==========================================================================
// Coordinate transformation methods
// ==========================================================================

// Method compute_RA_DEC returns the right ascension and declination
// in degrees of input vector V which is assumed to be in ECI
// coordinates.  We wrote this little utility on 1/24/02 primarily to
// compare with XELIAS results.

   void compute_RA_DEC(const threevector& V,double& alpha,double& delta)
      {
         threevector Vhat(V.unitvector());
         delta=asin(Vhat.get(2));
         alpha=atan2(Vhat.get(1)/cos(delta),Vhat.get(0)/cos(delta));
         delta *= 180/PI;
         alpha *= 180/PI;
      }

// --------------------------------------------------------------------------
// Method function ECI_vector returns a direction vector in ECI
// coordinates which corresponds to right ascension and declination
// angles alpha and delta measured in degrees.  This method represents
// the inverse of compute_RA_DEC().

   threevector ECI_vector(double& alpha,double& delta)
      {
         double RA=alpha*PI/180;
         double DEC=delta*PI/180;
         return threevector(cos(DEC)*cos(RA),cos(DEC)*sin(RA),sin(DEC));
      }

// --------------------------------------------------------------------------
// Method ecliptic_to_equatorial_coords converts ecliptic
// longitude lambda and ecliptic latitude beta into right ascension
// alpha and declination delta measured in radians.

   void ecliptic_to_equatorial_coords(
      int year,int month,double day,
      double lambda,double beta,double& alpha,double& delta)
      {
         double eps=mean_ecliptic_obliquity(year,month,day);
         double numer=sin(lambda)*cos(eps)-tan(beta)*sin(eps);
         double denom=cos(lambda);
         alpha=atan2(numer,denom);
         numer=sin(beta)*cos(eps)+cos(beta)*sin(eps)*sin(lambda);
         delta=asin(numer);
      }

   void ecliptic_to_equatorial_coords(
      double juliandate,double lambda,double beta,double& alpha,double& delta)
      {
         double eps=mean_ecliptic_obliquity(juliandate);
         double numer=sin(lambda)*cos(eps)-tan(beta)*sin(eps);
         double denom=cos(lambda);
         alpha=atan2(numer,denom);
         numer=sin(beta)*cos(eps)+cos(beta)*sin(eps)*sin(lambda);
         delta=asin(numer);
      }


// --------------------------------------------------------------------------
// Method mean_ecliptic_obliquity calculates the mean obliquity of
// the ecliptic in terms of T = number of Julian centuries since epoch
// 1900 January 0.5.  

   double mean_ecliptic_obliquity(int year,int month,double day)
      {
         const double pi=3.14159265358979323846;
         double juliandate=julian_day(year,month,day);
         double T=(juliandate-2415020.0)/36525.0;
         double deltaeps=46.845*T+0.0059*sqr(T)-0.00181*
            mathfunc::real_power(T,3);
         double eps=23.452294-deltaeps/3600; // degs
         eps *= pi/180;	// rads
   
         return eps;
      }

   double mean_ecliptic_obliquity(double juliandate)
      {
         const double pi=3.14159265358979323846;
         double T=(juliandate-2415020.0)/36525.0;
         double deltaeps=46.845*T+0.0059*sqr(T)-0.00181*
            mathfunc::real_power(T,3);
         double eps=23.452294-deltaeps/3600; // degs
         eps *= pi/180;	// rads
   
         return eps;
      }

// --------------------------------------------------------------------------
// Method compute_sun_longitude returns the sun's longitude and angle
// past perigee in radians.

   void compute_sun_longitude(
      double elapsed_days,double& m,double& lambda_solar)
      {
         const double pi=3.14159265358979323846;
         const double eps_g=278.833540*pi/180; 
         // Ecliptic longitude at epoch 1980.0 (rads)
         const double pomega_g=282.596403*pi/180;
         // Ecliptic longitude of perigee (rads)
         const double e=0.016718;	// eccentricity of orbit
   
         double n=(360/365.2422*elapsed_days)*pi/180;	// rads
         n=basic_math::phase_to_canonical_interval(n,0,2*pi);

// Imagine that Sun moves in a circle around the earth at a constant
// speed, rather than along the ellipse which it actually traces.
// Then m = angle through which this fictitious sun has moved since it
// passed through perigee:

         m=n+eps_g-pomega_g;	// rads
         m=basic_math::phase_to_canonical_interval(m,0,2*pi);
   
         double Ec=2*e*sin(m);      	// rads
         lambda_solar=n+Ec+eps_g;	// rads
         lambda_solar=basic_math::phase_to_canonical_interval(
            lambda_solar,0,2*pi); // longitude of Sun

//         m *= 180/PI;
//         lambda_solar *= 180/PI;
      }

// --------------------------------------------------------------------------
// Method sun_right_ascension_declination computes the position of the
// sun in RA-DEC coordinates (measured in radians) as a function of
// calendar date:

   void sun_right_ascension_declination(
      int year,int month,double day,double& alpha,double& delta)
      {
         double elapsed_days=days_elapsed_since_epoch(year,month,day);
         double m,lambda_solar;
         compute_sun_longitude(elapsed_days,m,lambda_solar);
         ecliptic_to_equatorial_coords(
            year,month,day,lambda_solar,0,alpha,delta);
      }

   void sun_right_ascension_declination(
      double juliandate,double& alpha,double& delta)
      {
         double elapsed_days=days_elapsed_since_epoch(juliandate);
         double m,lambda_solar;
         compute_sun_longitude(elapsed_days,m,lambda_solar);
         ecliptic_to_equatorial_coords(juliandate,lambda_solar,0,alpha,delta);
      }

// ---------------------------------------------------------------------
// Method sun_RA_DEC calculates the sun's right ascension and
// declination (in degrees) corresponding to the input currtime
// parameter measured in seconds since midnight on the date specified
// by member variable pass_date.  Note: On 5/30/01, we found that the
// results returned by this member function closely match (but do not
// precisely equal) those returned by XELIAS provided we compute the
// Julian reference day corresponding to 1970-01-01 00:00:00 via
// julian_day(1970,1,1) rather than julian_day(1970,1,0)!

   void sun_RA_DEC(
      double reference_time,double currtime,double& sun_RA,double& sun_DEC)
      {
//   const double ref_julian_date=astrofunc::julian_day(1970,1,1); // 1970-01-01 00:00:00
         const double ref_julian_date=2440587.5;	        // 1970-01-01 00:00:00

         double days_since_ref=(reference_time+currtime)/(24.0*3600.0);
         double curr_julian_date=ref_julian_date+days_since_ref;
         astrofunc::sun_right_ascension_declination(
            curr_julian_date,sun_RA,sun_DEC);
         sun_RA *= 180/PI;
         sun_DEC *= 180/PI;
      }

} // astrofunc namespace


