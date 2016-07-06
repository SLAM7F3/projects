// ==========================================================================
// Program OPTSHADOW reads time, target position, orbit normal and
// anti-nadir direction vector information from an input parameter
// file.  It computes the direction vector pointing from a specified
// geoposition to the target within the IGES model's basis.  We wrote
// this little utility to provide inputs into Hyrum's shadowing codes
// for 3D optical draping purposes.
// ==========================================================================
// Last updated on 12/17/07; 12/20/07; 12/21/07; 12/28/07
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "astro_geo/astrofuncs.h"
#include "astro_geo/Clock.h"
#include "astro_geo/Ellipsoid_model.h"
#include "general/filefuncs.h"
#include "templates/mytemplates.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   Ellipsoid_model* Ellipsoid_model_ptr=new Ellipsoid_model();
   Clock* Clock_ptr=new Clock();

   string input_filename="input_params.txt";
   cout << "Enter name of input parameter file:" << endl;
   cin >> input_filename;
   
   if (!filefunc::ReadInfile(input_filename))
   {
      cout << "Could not read in parameter file.  Quitting..." << endl;
      exit(-1);
   }
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
//      cout << filefunc::text_line[i] << endl;
   }
   
   int year=stringfunc::string_to_integer(filefunc::text_line[0]);
   int month=stringfunc::string_to_integer(filefunc::text_line[1]);
   int day=stringfunc::string_to_integer(filefunc::text_line[2]);
   int hour=stringfunc::string_to_integer(filefunc::text_line[3]);
   int minutes=stringfunc::string_to_integer(filefunc::text_line[4]);
   double seconds=stringfunc::string_to_number(filefunc::text_line[5]);

   cout << "year = " << year << " month = " << month
        << " day = " << day << endl;
   cout << "hour = " << hour << " min = " << minutes << " secs = " << seconds
        << endl;

//   Clock_ptr->current_local_time_and_UTC();
//   Clock_ptr->set_local_time(year,month,day,hour,minute,sec);
//   Clock_ptr->set_daylight_savings_flag(false);
   Clock_ptr->set_UTC_time(year,month,day,hour,minutes,seconds);

// Read in target's position in ECI coordinates:

   threevector target_ECI_posn(
      stringfunc::string_to_number(filefunc::text_line[6]),
      stringfunc::string_to_number(filefunc::text_line[7]),
      stringfunc::string_to_number(filefunc::text_line[8]));
   cout << "target ECI position = " << target_ECI_posn << endl;

// Read in orbit normal (=xhat_IGES) and anti-nadir (=yhat_IGES for
// earth-stable motion) direction vectors in ECI coordinates:

   threevector xhat_IGES_ECI(
      stringfunc::string_to_number(filefunc::text_line[9]),
      stringfunc::string_to_number(filefunc::text_line[10]),
      stringfunc::string_to_number(filefunc::text_line[11]));
   threevector yhat_IGES_ECI(
      stringfunc::string_to_number(filefunc::text_line[12]),
      stringfunc::string_to_number(filefunc::text_line[13]),
      stringfunc::string_to_number(filefunc::text_line[14]));
   threevector zhat_IGES_ECI=xhat_IGES_ECI.cross(yhat_IGES_ECI);
   cout << "xhat_IGES_ECI = " << xhat_IGES_ECI << endl;
   cout << "yhat_IGES_ECI = " << yhat_IGES_ECI << endl;
   cout << "zhat_IGES_ECI = " << zhat_IGES_ECI << endl;

/*
   double alpha,delta;
   astrofunc::compute_RA_DEC(xhat_IGES_ECI,alpha,delta);
   cout << "xhat alpha = " << alpha << " delta = " << delta << endl;
   astrofunc::compute_RA_DEC(yhat_IGES_ECI,alpha,delta);
   cout << "yhat alpha = " << alpha << " delta = " << delta << endl;
   astrofunc::compute_RA_DEC(zhat_IGES_ECI,alpha,delta);
   cout << "zhat alpha = " << alpha << " delta = " << delta << endl;
*/

// Hardwire Maui sensor's geoposition:

   double sensor_longitude=-156.256819;	// AMOS on Maui
   double sensor_latitude=20.708119;	// AMOS on Maui
   double sensor_altitude=3058;	// meters
   threevector sensor_ECI_posn=Ellipsoid_model_ptr->ConvertLongLatAltToECI(
      sensor_longitude,sensor_latitude,sensor_altitude,*Clock_ptr);
   cout << "Sensor_ECI_posn = " << sensor_ECI_posn << endl;

//   threevector range=target_ECI_posn-sensor_ECI_posn;
//   cout << "range = " << (target_ECI_posn-sensor_ECI_posn).magnitude();
   threevector p_hat_ECI=(target_ECI_posn-sensor_ECI_posn).unitvector();
   threevector p_hat_IGES(
      p_hat_ECI.dot(xhat_IGES_ECI),
      p_hat_ECI.dot(yhat_IGES_ECI),
      p_hat_ECI.dot(zhat_IGES_ECI));

// ============================================================
// Compute optical image plane Uhat and Vhat axes in ECI and IGES
// model coords:

   Ellipsoid_model_ptr->compute_east_north_radial_dirs(
      sensor_latitude,sensor_longitude);
   Ellipsoid_model_ptr->convert_surface_to_ECI_directions(*Clock_ptr);

   threevector east_ECI_hat=Ellipsoid_model_ptr->get_east_ECI_hat();
   threevector north_ECI_hat=Ellipsoid_model_ptr->get_north_ECI_hat();
   threevector radial_ECI_hat=Ellipsoid_model_ptr->get_radial_ECI_hat();

   cout << "east_ECI_hat = " << east_ECI_hat << endl;
   cout << "north_ECI_hat = " << north_ECI_hat << endl;
   cout << "radial_ECI_hat = " << radial_ECI_hat << endl;

   double azimuth=stringfunc::string_to_number(filefunc::text_line[15])
      *PI/180;
   double elevation=stringfunc::string_to_number(filefunc::text_line[16])
      *PI/180;
   
   double cos_az=cos(azimuth);
   double sin_az=sin(azimuth);
   double cos_el=cos(elevation);
   double sin_el=sin(elevation);

   threevector Uhat_ECI=cos_az*east_ECI_hat - sin_az*north_ECI_hat;
   threevector Vhat_ECI=
      -sin_az*sin_el*east_ECI_hat - cos_az*sin_el*north_ECI_hat
      +cos_el*radial_ECI_hat;
   threevector What_ECI=
      -sin_az*cos_el*east_ECI_hat - cos_az*cos_el*north_ECI_hat
      -sin_el*radial_ECI_hat;

   cout << "Uhat_ECI = " << Uhat_ECI << endl;
   cout << "Vhat_ECI = " << Vhat_ECI << endl;
   cout << "What_ECI = " << What_ECI << endl;
   cout << "p_hat_ECI = " << p_hat_ECI << endl;
   
   cout << "Uhat_ECI.Uhat_ECI = " << Uhat_ECI.dot(Uhat_ECI) << endl;
   cout << "Vhat_ECI.Vhat_ECI = " << Vhat_ECI.dot(Vhat_ECI) << endl;
   cout << "What_ECI.What_ECI = " << What_ECI.dot(What_ECI) << endl;
   cout << "Uhat_ECI.Vhat_ECI = " << Uhat_ECI.dot(Vhat_ECI) << endl;
   cout << "Vhat_ECI.What_ECI = " << Vhat_ECI.dot(What_ECI) << endl;
   cout << "What_ECI.Uhat_ECI = " << What_ECI.dot(Uhat_ECI) << endl;

   cout << "Uhat_ECI x Vhat_ECI - What_ECI = " 
        << Uhat_ECI.cross(Vhat_ECI) - What_ECI << endl;

   threevector Uhat_IGES(
      Uhat_ECI.dot(xhat_IGES_ECI),
      Uhat_ECI.dot(yhat_IGES_ECI),
      Uhat_ECI.dot(zhat_IGES_ECI));

   threevector Vhat_IGES(
      Vhat_ECI.dot(xhat_IGES_ECI),
      Vhat_ECI.dot(yhat_IGES_ECI),
      Vhat_ECI.dot(zhat_IGES_ECI));

   threevector What_IGES(
      What_ECI.dot(xhat_IGES_ECI),
      What_ECI.dot(yhat_IGES_ECI),
      What_ECI.dot(zhat_IGES_ECI));

   cout << "# Uhat in IGES model coordinates " << endl;
   cout << Uhat_IGES << endl;
   cout << "# Vhat in IGES model coordinates " << endl;
   cout << Vhat_IGES << endl;
   cout << "# What in IGES model coordinates " << endl;
   cout << What_IGES << endl;
   cout << "p_hat_IGES = " << p_hat_IGES << endl;

// Compute Sun's directoin vector in IGES basis:

   double sun_RA=255.228;
   double sun_DEC=-22.7462;
   threevector sun_ECI=astrofunc::ECI_vector(sun_RA,sun_DEC);
   cout << "sun_ECI = " << sun_ECI << endl;

   threevector sun_IGES(
      sun_ECI.dot(xhat_IGES_ECI),
      sun_ECI.dot(yhat_IGES_ECI),
      sun_ECI.dot(zhat_IGES_ECI));

   cout << "# sun_hat in IGES model coordinates " << endl;
   cout << sun_IGES << endl;

// Compute xhat_IGES, yhat_IGES and zhat_IGES in Uhat, Vhat, What basis:

   threevector xhat_UVW(
      xhat_IGES_ECI.dot(Uhat_ECI),
      xhat_IGES_ECI.dot(Vhat_ECI),
      xhat_IGES_ECI.dot(What_ECI));

   threevector yhat_UVW(
      yhat_IGES_ECI.dot(Uhat_ECI),
      yhat_IGES_ECI.dot(Vhat_ECI),
      yhat_IGES_ECI.dot(What_ECI));

   threevector zhat_UVW(
      zhat_IGES_ECI.dot(Uhat_ECI),
      zhat_IGES_ECI.dot(Vhat_ECI),
      zhat_IGES_ECI.dot(What_ECI));

   cout << "xhat_UVW.xhat_UVW = " << xhat_UVW.dot(xhat_UVW) << endl;
   cout << "yhat_UVW.yhat_UVW = " << yhat_UVW.dot(yhat_UVW) << endl;
   cout << "zhat_UVW.zhat_UVW = " << zhat_UVW.dot(zhat_UVW) << endl << endl;

   cout << "xhat_UVW.yhat_UVW = " << xhat_UVW.dot(yhat_UVW) << endl;
   cout << "yhat_UVW.zhat_UVW = " << yhat_UVW.dot(zhat_UVW) << endl;
   cout << "zhat_UVW.xhat_UVW = " << zhat_UVW.dot(xhat_UVW) << endl << endl;

   cout << "input_filename = " << input_filename << endl << endl;
   cout << "xhat_UVW = " 
        << xhat_UVW.get(0) << " , " 
        << xhat_UVW.get(1) << " , " 
        << xhat_UVW.get(2) << endl;
   cout << "yhat_UVW = " 
        << yhat_UVW.get(0) << " , " 
        << yhat_UVW.get(1) << " , " 
        << yhat_UVW.get(2) << endl;
   cout << "zhat_UVW = " 
        << zhat_UVW.get(0) << " , " 
        << zhat_UVW.get(1) << " , " 
        << zhat_UVW.get(2) << endl;

   cout << endl;

   delete Clock_ptr;
   delete Ellipsoid_model_ptr;
}
