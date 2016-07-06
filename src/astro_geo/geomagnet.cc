// ==========================================================================
// Geomagnet class member function definitions
// ==========================================================================
// Last modified on 7/31/07; 8/1/07
// ==========================================================================

#include <iostream>
#include <math.h>
#include "astro_geo/geomagnet.h"
#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;

#define NaN log(-1.0)

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void geomagnet::allocate_member_objects()
{
}		       

void geomagnet::initialize_member_objects()
{
   maxdeg = 12;
   warn_H = 0;
   warn_H_val = 99999.0;
   warn_H_strong = 0;
   warn_H_strong_val = 99999.0;
   warn_P = 0;

   data_filename=
      "/home/cho/programs/c++/svn/projects/src/astro_geo/WMM.COF";
}

geomagnet::geomagnet(void)
{
   allocate_member_objects();
   initialize_member_objects();
   initialize_params();
}

// Copy constructor:

geomagnet::geomagnet(const geomagnet& g)
{
//   docopy(g);
}

geomagnet::~geomagnet()
{
}

// ---------------------------------------------------------------------
void geomagnet::docopy(const geomagnet& g)
{
}	

// Overload = operator:

geomagnet geomagnet::operator= (const geomagnet& g)
{
   if (this==&g) return *this;
   docopy(g);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,geomagnet& g)
{
   outstream.precision(12);
   outstream << endl;
//   outstream << "UTM Zone = " << g.UTM_zone << endl;
   return outstream;
}

// =======================================================================
void geomagnet::initialize_params()
{
//   cout << "inside geomagnet::initialize_params()" << endl;
   
   char d_str[81],modl[20];
   FILE* wmmtemp = fopen(data_filename.c_str(),"r");
   fgets(d_str, 80, wmmtemp);
   sscanf(d_str,"%f%s",&epochlowlim,modl);
   fclose(wmmtemp);

   const float epochrange = 5.0;	// years
   epochuplim = epochlowlim + epochrange;

//   cout << "epochlowlim = " << epochlowlim << endl;
//   cout << "epochuplim = " << epochuplim << endl;

   int maxdeg = 12;
   E0000(0,&maxdeg,0.0,0.0,0.0,0.0,NULL,NULL,NULL,NULL);
}

// =======================================================================
void geomagnet::input_geolocation_and_time()
{
   double latitude,longitude,altitude,time;
   cout << "Enter latitude in decimal degrees:" << endl;
   cout << "(North latitudes positive, south latitudes negative)" << endl;
   cin >> latitude;

   cout << "Enter longitude in decimal degrees:" << endl;
   cout << "(East longitudes positive, west longitudes negative)" << endl;
   cin >> longitude;

   cout << "Enter altitude in meters above mean sea level (WGS84)" << endl;
   cin >> altitude;

   cout << "Enter time in decimal year ( ";
   cout << epochlowlim << " - " << epochuplim << " ):" << endl;
   cin >> time;

   set_geolocation(longitude,latitude,altitude);
   get_geolocation().set_time(time);
}

// =======================================================================
void geomagnet::E0000(
   int IENTRY, int *maxdeg, float alt, float glat, float glon, 
   float time, float *dec, float *dip, float *ti, float *gv)
{
   static int maxord,i,icomp,n,m,j,D1,D2,D3,D4;
   static float c[13][13],cd[13][13],tc[13][13],dp[13][13],snorm[169],
      sp[13],cp[13],fn[13],fm[13],pp[13],k[13][13],pi,dtr,a,b,re,
      a2,b2,c2,a4,b4,c4,epoch,gnm,hnm,dgnm,dhnm,flnmj,otime,oalt,
      olat,olon,dt,rlon,rlat,srlon,srlat,crlon,crlat,srlat2,
      crlat2,q,q1,q2,ct,st,r2,r,d,ca,sa,aor,ar,br,bt,bp,bpp,
      par,temp1,temp2,parp,bx,by,bz,bh;
   static char model[20], c_str[81], c_new[5];
   static float *p = snorm;

   FILE* wmmdat;

   switch(IENTRY){case 0: goto GEOMAG; case 1: goto GEOMG1;}
  
  GEOMAG:

   wmmdat = fopen(data_filename.c_str(),"r");
//   wmmdat = fopen("/home/cho/programs/c++/svn/projects/src/astro_geo/WMM.COF","r");
//   wmmdat = fopen("/home/cho/programs/c++/svn/projects/src/mains/testdir/WMM.COF","r");
  
// INITIALIZE CONSTANTS 

   maxord = *maxdeg;
   sp[0] = 0.0;
   cp[0] = *p = pp[0] = 1.0;
   dp[0][0] = 0.0;
   a = 6378.137;
   b = 6356.7523142;
   re = 6371.2;
   a2 = a*a;
   b2 = b*b;
   c2 = a2-b2;
   a4 = a2*a2;
   b4 = b2*b2;
   c4 = a4 - b4;
  
// READ WORLD MAGNETIC MODEL SPHERICAL HARMONIC COEFFICIENTS 
   c[0][0] = 0.0;
   cd[0][0] = 0.0;
  
   fgets(c_str, 80, wmmdat);
   sscanf(c_str,"%f%s",&epoch,model);

   while (true)
   {
      fgets(c_str, 80, wmmdat);

// CHECK FOR LAST LINE IN FILE 

      for (i=0; i<4 && (c_str[i] != '\0'); i++)
      {
         c_new[i] = c_str[i];
         c_new[i+1] = '\0';
      }
      icomp = strcmp("9999", c_new);
      if (icomp == 0) goto S4;

// END OF FILE NOT ENCOUNTERED, GET VALUES 

      sscanf(c_str,"%d%d%f%f%f%f",&n,&m,&gnm,&hnm,&dgnm,&dhnm);
      if (m <= n)
      {
         c[m][n] = gnm;
         cd[m][n] = dgnm;
         if (m != 0)
         {
            c[n][m-1] = hnm;
            cd[n][m-1] = dhnm;
         }
      }
   } // while true loop

// CONVERT SCHMIDT NORMALIZED GAUSS COEFFICIENTS TO UNNORMALIZED 

  S4:

   *snorm = 1.0;
   for (n=1; n<=maxord; n++)
   {
      *(snorm+n) = *(snorm+n-1)*(float)(2*n-1)/(float)n;
      j = 2;
      for (m=0,D1=1,D2=(n-m+D1)/D1; D2>0; D2--,m+=D1)
      {
         k[m][n] = (float)(((n-1)*(n-1))-(m*m))/(float)((2*n-1)*(2*n-3));
         if (m > 0)
         {
            flnmj = (float)((n-m+1)*j)/(float)(n+m);
            *(snorm+n+m*13) = *(snorm+n+(m-1)*13)*sqrt(flnmj);
            j = 1;
            c[n][m-1] = *(snorm+n+m*13)*c[n][m-1];
            cd[n][m-1] = *(snorm+n+m*13)*cd[n][m-1];
         }
         c[m][n] = *(snorm+n+m*13)*c[m][n];
         cd[m][n] = *(snorm+n+m*13)*cd[m][n];
      }
      fn[n] = (float)(n+1);
      fm[n] = (float)n;
   }
   k[1][1] = 0.0;
  
   otime = oalt = olat = olon = -1000.0;
   fclose(wmmdat);
   return;
  
// *************************************************************************

  GEOMG1:
  
   dt = time - epoch;
   if (otime < 0.0 && (dt < 0.0 || dt > 5.0))
   {      
      printf("\n\n WARNING - TIME EXTENDS BEYOND MODEL 5-YEAR LIFE SPAN");
      printf("\n CONTACT NGDC FOR PRODUCT UPDATES:");
      printf("\n         National Geophysical Data Center");
      printf("\n         NOAA EGC/2");
      printf("\n         325 Broadway");
      printf("\n         Boulder, CO 80303 USA");
      printf("\n         Attn: Susan McLean or Stefan Maus");
      printf("\n         Phone:  (303) 497-6478 or -6522");
      printf("\n         Email:  Susan.McLean@noaa.gov");
      printf("\n         or");
      printf("\n         Stefan.Maus@noaa.gov");
      printf("\n         Web: http://www.ngdc.noaa.gov/seg/WMM/");
      printf("\n\n EPOCH  = %.3lf",epoch);
      printf("\n TIME   = %.3lf",time);
      printf("\n Do you wish to continue? (y or n) ");


      char answer;
      scanf("%c%*[^\n]",&answer);
      getchar();
      if ((answer == 'n') || (answer == 'N'))
      {
         printf("\n Do you wish to enter more point data? (y or n) ");
         scanf("%c%*[^\n]",&answer);
         getchar();
         if ((answer == 'y')||(answer == 'Y')) goto GEOMG1;
         else exit (0);
      }
   }

   pi = 3.14159265359;
   dtr = pi/180.0;
   rlon = glon*dtr;
   rlat = glat*dtr;
   srlon = sin(rlon);
   srlat = sin(rlat);
   crlon = cos(rlon);
   crlat = cos(rlat);
   srlat2 = srlat*srlat;
   crlat2 = crlat*crlat;
   sp[1] = srlon;
   cp[1] = crlon;

// CONVERT FROM GEODETIC COORDS. TO SPHERICAL COORDS. 

   if (alt != oalt || glat != olat)
   {
      q = sqrt(a2-c2*srlat2);
      q1 = alt*q;
      q2 = ((q1+a2)/(q1+b2))*((q1+a2)/(q1+b2));
      ct = srlat/sqrt(q2*crlat2+srlat2);
      st = sqrt(1.0-(ct*ct));
      r2 = (alt*alt)+2.0*q1+(a4-c4*srlat2)/(q*q);
      r = sqrt(r2);
      d = sqrt(a2*crlat2+b2*srlat2);
      ca = (alt+d)/r;
      sa = c2*crlat*srlat/(r*d);
   }
   if (glon != olon)
   {
      for (m=2; m<=maxord; m++)
      {
         sp[m] = sp[1]*cp[m-1]+cp[1]*sp[m-1];
         cp[m] = cp[1]*cp[m-1]-sp[1]*sp[m-1];
      }
   }
   aor = re/r;
   ar = aor*aor;
   br = bt = bp = bpp = 0.0;
   for (n=1; n<=maxord; n++)
   {
      ar = ar*aor;
      for (m=0,D3=1,D4=(n+m+D3)/D3; D4>0; D4--,m+=D3)
      {

//  COMPUTE UNNORMALIZED ASSOCIATED LEGENDRE POLYNOMIALS
//  AND DERIVATIVES VIA RECURSION RELATIONS

         if (alt != oalt || glat != olat)
         {
            if (n == m)
            {
               *(p+n+m*13) = st**(p+n-1+(m-1)*13);
               dp[m][n] = st*dp[m-1][n-1]+ct**(p+n-1+(m-1)*13);
               goto S50;
            }
            if (n == 1 && m == 0)
            {
               *(p+n+m*13) = ct**(p+n-1+m*13);
               dp[m][n] = ct*dp[m][n-1]-st**(p+n-1+m*13);
               goto S50;
            }
            if (n > 1 && n != m)
            {
               if (m > n-2) *(p+n-2+m*13) = 0.0;
               if (m > n-2) dp[m][n-2] = 0.0;
               *(p+n+m*13) = ct**(p+n-1+m*13)-k[m][n]**(p+n-2+m*13);
               dp[m][n] = ct*dp[m][n-1] - st**(p+n-1+m*13)-k[m][n]*dp[m][n-2];
            }
         }
        S50:

//  TIME ADJUST THE GAUSS COEFFICIENTS

         if (time != otime)
         {
            tc[m][n] = c[m][n]+dt*cd[m][n];
            if (m != 0) tc[n][m-1] = c[n][m-1]+dt*cd[n][m-1];
         }

//  ACCUMULATE TERMS OF THE SPHERICAL HARMONIC EXPANSIONS

         par = ar**(p+n+m*13);
         if (m == 0)
         {
            temp1 = tc[m][n]*cp[m];
            temp2 = tc[m][n]*sp[m];
         }
         else
         {
            temp1 = tc[m][n]*cp[m]+tc[n][m-1]*sp[m];
            temp2 = tc[m][n]*sp[m]-tc[n][m-1]*cp[m];
         }
         bt = bt-ar*temp1*dp[m][n];
         bp += (fm[m]*temp2*par);
         br += (fn[n]*temp1*par);

//  SPECIAL CASE:  NORTH/SOUTH GEOGRAPHIC POLES

         if (st == 0.0 && m == 1)
         {
            if (n == 1) pp[n] = pp[n-1];
            else pp[n] = ct*pp[n-1]-k[m][n]*pp[n-2];
            parp = ar*pp[n];
            bpp += (fm[m]*temp2*parp);
         }
      }
   }
   if (st == 0.0) bp = bpp;
   else bp /= st;

//  ROTATE MAGNETIC VECTOR COMPONENTS FROM SPHERICAL TO
//  GEODETIC COORDINATES

   bx = -bt*ca-br*sa;
   by = bp;
   bz = bt*sa-br*ca;

//  COMPUTE DECLINATION (DEC), INCLINATION (DIP) AND
//  TOTAL INTENSITY (TI)

   bh = sqrt((bx*bx)+(by*by));
   *ti = sqrt((bh*bh)+(bz*bz));
   *dec = atan2(by,bx)/dtr;
   *dip = atan2(bz,bh)/dtr;

//  COMPUTE MAGNETIC GRID VARIATION IF THE CURRENT
//  GEODETIC POSITION IS IN THE ARCTIC OR ANTARCTIC
//  (I.E. GLAT > +55 DEGREES OR GLAT < -55 DEGREES)

//  OTHERWISE, SET MAGNETIC GRID VARIATION TO -999.0

   *gv = -999.0;
   if (fabs(glat) >= 55.)
   {
      if (glat > 0.0 && glon >= 0.0) *gv = *dec-glon;
      if (glat > 0.0 && glon < 0.0) *gv = *dec+fabs(glon);
      if (glat < 0.0 && glon >= 0.0) *gv = *dec+glon;
      if (glat < 0.0 && glon < 0.0) *gv = *dec-fabs(glon);
      if (*gv > +180.0) *gv -= 360.0;
      if (*gv < -180.0) *gv += 360.0;
   }
   otime = time;
   oalt = alt;
   olat = glat;
   olon = glon;
   return;
}

// =======================================================================
void geomagnet::geomg1(
   float alt, float glat, float glon, float time, float *dec, float *dip, 
   float *ti, float *gv)
{
   E0000(1,NULL,alt,glat,glon,time,dec,dip,ti,gv);
}

// ==========================================================================
// Member function compute_magnetic_field_components takes in an
// altitude (measured in meters), latitude and longitude (measured in
// decimal degrees) and time (measured in decimal years).  

void geomagnet::compute_magnetic_field_components()
{
//   cout << "inside geomagnet::compute_magnetic_field_components()" << endl;

// Convert input altitude from meters to kilometers:

   float dlon=curr_geolocation.get_longitude();
   float dlat=curr_geolocation.get_latitude();
   float alt=curr_geolocation.get_altitude()*0.001;	// kilometers
   float time=curr_geolocation.get_time();

   float dec,dip,ti,gv;
   geomg1(alt,dlat,dlon,time,&dec,&dip,&ti,&gv);
   float time1 = time;
   dec1 = dec;
   float dip1 = dip;
   float ti1 = ti;
   time = time1 + 1.0;
      
   geomg1(alt,dlat,dlon,time,&dec,&dip,&ti,&gv);
   float dec2 = dec;
   float dip2 = dip;
//   float ti2 = ti;

// COMPUTE X, Y, Z, AND H COMPONENTS OF THE MAGNETIC FIELD
      
   const float rTd=0.017453292;
//   float x1=ti1*(cos((dec1*rTd))*cos((dip1*rTd)));
//   float x2=ti2*(cos((dec2*rTd))*cos((dip2*rTd)));
//   float y1=ti1*(cos((dip1*rTd))*sin((dec1*rTd)));
//   float y2=ti2*(cos((dip2*rTd))*sin((dec2*rTd)));
//   float z1=ti1*(sin((dip1*rTd)));
//   float z2=ti2*(sin((dip2*rTd)));
   float h1=ti1*(cos((dip1*rTd)));
//   float h2=ti2*(cos((dip2*rTd)));

// COMPUTE ANNUAL CHANGE FOR TOTAL INTENSITY  

//   float ati = ti2 - ti1;

//  COMPUTE ANNUAL CHANGE FOR DIP & DEC  

   adip = (dip2 - dip1) * 60.;
   adec = (dec2 - dec1) * 60.;

//  COMPUTE ANNUAL CHANGE FOR X, Y, Z, AND H 

//   float ax = x2-x1;
//   float ay = y2-y1;
//   float az = z2-z1;
//   float ah = h2-h1;

   string decd_str;
   if (dec1 < 0.0) 
   { 
      decd_str="(WEST)";
   }
   else 
   { 
      decd_str="(EAST)";
   }

   string dipd_str;
   if (dip1 < 0.0) 
   {
      dipd_str="(UP)  ";
   }
   else 
   {
      dipd_str="(DOWN)";
   }

   // deal with geographic and magnetic poles 
      
   if (h1 < 100.0) // at magnetic poles 
   {
      dec1 = NaN;
      adec = NaN;
      decd_str="(VOID)";
      // while rest is ok 
   }

   if (h1 < 1000.0) 
   {
      warn_H = 0;
      warn_H_strong = 1;
      warn_H_strong_val = h1;
   }
   else if (h1 < 5000.0 && !warn_H_strong) 
   {
      warn_H = 1;
      warn_H_val = h1;
   }
            
   if (90.0-fabs(dlat) <= 0.001) // at geographic poles 
   {
//      x1 = NaN;
//      y1 = NaN;
      dec1 = NaN;
//      ax = NaN;
//      ay = NaN;
      adec = NaN;
      decd_str="(VOID)";
      warn_P = 1;
      warn_H = 0;
      warn_H_strong = 0;
      // while rest is ok 
   }

   // convert D and I to deg and min 

   if (isnan(dec1)) 
   {
      ddeg = dec1; 
   }
   else 
   {
      ddeg=(int)dec1;
   }
      
   dmin=(dec1-(float)ddeg)*60;
   if(ddeg!=0) dmin=fabs(dmin);
   if (isnan(dip1)) 
   {
      ideg = dip1; 
   }
   else 
   {
      ideg=(int)dip1;
   }
      
   imin=(dip1-(float)ideg)*60;
   if(ideg!=0) imin=fabs(imin);
}

// =======================================================================
void geomagnet::geomag_introduction(float epochlowlim)
{
   char help;
  
   printf("\n\n Welcome to the World Magnetic Model (WMM) %4.0f C-Program\n\n", epochlowlim);
   printf("            --- Version 2.0, September 2005 ---\n\n");
   printf("\n This program estimates the strength and direction of ");
   printf("\n Earth's main magnetic field for a given point/area.");
   printf("\n Enter h for help and contact information or c to continue.");
   printf ("\n >");
   scanf("%c%*[^\n]",&help);
   getchar();

   if ((help == 'h') || (help == 'H'))
   {
      printf("\n Help information ");
      
      printf("\n The World Magnetic Model (WMM) for %7.2f", epochlowlim);
      printf("\n is a model of Earth's main magnetic field.  The WMM");
      printf("\n is recomputed every five (5) years, in years divisible by ");
      printf("\n five (i.e. 2005, 2010).  See the contact information below");
      printf("\n to obtain more information on the WMM and associated software.");
      printf("\n ");
      printf("\n Input required is the location in geodetic latitude and");
      printf("\n longitude (positive for northern latitudes and eastern ");
      printf("\n longitudes), geodetic altitude in meters, and the date of "); 
      printf("\n interest in years.");
      
      printf("\n\n\n The program computes the estimated magnetic Declination");
      printf("\n (D) which is sometimes called MAGVAR, Inclination (I), Total");
      printf("\n Intensity (F or TI), Horizontal Intensity (H or HI), Vertical");
      printf("\n Intensity (Z), and Grid Variation (GV). Declination and Grid");
      printf("\n Variation are measured in units of degrees and are considered"); 
      printf("\n positive when east or north.  Inclination is measured in units");
      printf("\n of degrees and is considered positive when pointing down (into");
      printf("\n the Earth).  The WMM is reference to the WGS-84 ellipsoid and");
      printf("\n is valid for 5 years after the base epoch.");
      
      printf("\n\n\n It is very important to note that a  degree and  order 12 model,");
      printf("\n such as WMM, describes only the long  wavelength spatial magnetic ");
      printf("\n fluctuations due to  Earth's core.  Not included in the WMM series");
      printf("\n models are intermediate and short wavelength spatial fluctuations ");
      printf("\n that originate in Earth's mantle and crust. Consequently, isolated");
      printf("\n angular errors at various  positions on the surface (primarily over");
      printf("\n land, incontinental margins and  over oceanic seamounts, ridges and");
      printf("\n trenches) of several degrees may be expected.  Also not included in");
      printf("\n the model are temporal fluctuations of magnetospheric and ionospheric");
      printf("\n origin. On the days during and immediately following magnetic storms,");
      printf("\n temporal fluctuations can cause substantial deviations of the geomagnetic");
      printf("\n field  from model  values.  If the required  declination accuracy  is");
      printf("\n more stringent than the WMM  series of models provide, the user is");
      printf("\n advised to request special (regional or local) surveys be performed");
      printf("\n and models prepared. Please make requests of this nature to the");
      printf("\n National Geospatial-Intelligence Agency (NGA) at the address below.");
      
      printf("\n\n\n Contact Information");
      
      printf("\n  Software and Model Support");
      printf("\n	National Geophysical Data Center");
      printf("\n	NOAA EGC/2");
      printf("\n	325 Broadway");
      printf("\n	Boulder, CO 80303 USA");
      printf("\n	Attn: Susan McLean or Stefan Maus");
      printf("\n	Phone:  (303) 497-6478 or -6522");
      printf("\n	Email:  Susan.McLean@noaa.gov or Stefan.Maus@noaa.gov ");
      
   }
}

// ==========================================================================
// Member function display_magnetic_field_components

void geomagnet::display_magnetic_field_components()
{
   if (curr_geolocation.get_latitude() < 0)
   {
      cout << "latitude = " << -curr_geolocation.get_latitude() 
           << "S" << endl;
   }
   else
   {
      cout << "latitude = " << curr_geolocation.get_latitude() 
           << "N" << endl;
   }

   if (curr_geolocation.get_longitude() < 0)
   {
      cout << "longitude = " << -curr_geolocation.get_longitude() 
           << "W" << endl;
   }
   else
   {
      cout << "longitude = " << curr_geolocation.get_longitude() 
           << "E" << endl;
   }
   cout << "Altitude = " << curr_geolocation.get_altitude() 
        << " meters amsl (WGS84)" << endl;
//   cout << "Date = " << time << endl;


   if (isnan(dec1))
   {
      cout << "D = NaN  dD = NaN" << endl;
   }
   else
   {
      cout << endl;
      cout << "D = " << ddeg << " Deg " << dmin << " Min " << decd_str
           << " dD = " << adec << " Min/yr" << endl;
   }
      
   cout << "I = " << ideg << " Deg " << imin << " Min " << dipd_str
        << " dI = " << adip << " Min/yr" << endl;

   if (warn_H)
   {
      cout << "Warning: The horizontal field strength at this location is only " << endl;
      cout << warn_H_val << "nT" << endl;
      cout << "Compass readings have large uncertainties in areas where H is "
           << endl;
      cout << "smaller than 5000 nT" << endl;
   } 
   if (warn_H_strong)
   {
      cout << "Warning: The horizontal field strength at this location is only " << endl;
      cout << warn_H_strong_val << "nT" << endl;
      cout << "Compass readings have VERY LARGE uncertainties in areas where H is" 
           << endl;
      cout << " smaller than 1000 nT" << endl;
   }
   if (warn_P)
   {
      cout << "Location is at geographic pole where X, Y and Decl are undefined"
           << endl;
   } 
}

// ==========================================================================
// Member function get_delta_yaw

double geomagnet::get_delta_yaw() const
{
   return latlongfunc::dms_to_decimal_degs(ddeg,dmin,0);
}
