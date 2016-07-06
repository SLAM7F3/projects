// ==========================================================================
// Program GRIDCOMP generates an alpha-numeric KML grid which can be
// used to grossly but quickly geolocate ground points.
// ==========================================================================
// Last updated on 4/27/11; 4/28/11; 9/22/11
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "astro_geo/geopoint.h"
#include "gearth/kml_parser.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   sysfunc::clearscreen();

   cout << "Program GRIDCOMP generates an alpha-numeric grid which can be used"
        << endl;
   cout << "to grossly but quickly geolocate ground points." << endl;
   cout << endl;

   double gridsize_in_nmi=1;
   cout << "Enter grid cell size in nautical miles (suggested size = 1 nmi):"
        << endl;
   cin >> gridsize_in_nmi;

   double longitude_min,longitude_max,latitude_min,latitude_max;
   cout << "Enter grid's lower longitude value in decimal degrees:" 
        << endl;
   cin >> longitude_min;
   cout << "Enter grid's upper longitude value in decimal degrees:" 
        << endl;
   cin >> longitude_max;
   cout << "Enter grid's lower latitude value in decimal degrees:" 
        << endl;
   cin >> latitude_min;
   cout << "Enter grid's upper latitude value in decimal degrees:" 
        << endl;
   cin >> latitude_max;

// Lat-lon entries need to be entered in the following order:

   vector<double> longitudes,latitudes;

// Lower left corner
// Lower right corner
// Upper right corner
// Upper left corner

   latitudes.push_back(latitude_min);
   latitudes.push_back(latitude_min);
   latitudes.push_back(latitude_max);
   latitudes.push_back(latitude_max);

   longitudes.push_back(longitude_min);
   longitudes.push_back(longitude_max);
   longitudes.push_back(longitude_max);
   longitudes.push_back(longitude_min);

/*
// North Coronado Nation Forest area:

   latitudes.push_back(32.339804);
   latitudes.push_back(32.339804);
   latitudes.push_back(32.485067);
   latitudes.push_back(32.485067);
   
   longitudes.push_back(-110.983733);
   longitudes.push_back(-110.812643);
   longitudes.push_back(-110.812643);
   longitudes.push_back(-110.983733);
*/

/*
// NH practice area:

   latitudes.push_back(latlongfunc::dm_to_decimal_degs(42,52.22));
   latitudes.push_back(latlongfunc::dm_to_decimal_degs(42,48.01));
   latitudes.push_back(latlongfunc::dm_to_decimal_degs(43,15.19));
   latitudes.push_back(latlongfunc::dm_to_decimal_degs(43,19.42));

   longitudes.push_back(-latlongfunc::dm_to_decimal_degs(71,3.64));
   longitudes.push_back(-latlongfunc::dm_to_decimal_degs(70,51.31));
   longitudes.push_back(-latlongfunc::dm_to_decimal_degs(70,33.98));
   longitudes.push_back(-latlongfunc::dm_to_decimal_degs(70,46.37));
*/

/*
// AZ border:

   latitudes.push_back(latlongfunc::dm_to_decimal_degs(31,37.14));
   latitudes.push_back(latlongfunc::dm_to_decimal_degs(31,20.01));
   latitudes.push_back(latlongfunc::dm_to_decimal_degs(31,37.86));
   latitudes.push_back(latlongfunc::dm_to_decimal_degs(31,57.51));
   
   longitudes.push_back(-latlongfunc::dm_to_decimal_degs(112,11.70));
   longitudes.push_back(-latlongfunc::dm_to_decimal_degs(110,56.85));
   longitudes.push_back(-latlongfunc::dm_to_decimal_degs(110,44.93));
   longitudes.push_back(-latlongfunc::dm_to_decimal_degs(111,49.87));
*/


   vector<geopoint> corners;
   for (int c=0; c<4; c++)
   {
      corners.push_back(geopoint(longitudes[c],latitudes[c]));
      cout << "Corner c = " << c << endl;
      cout << corners.back() << endl;
      cout << "=============================================" << endl;
   }
   
   bool northern_hemisphere_flag=corners[0].get_northern_hemisphere_flag();
   int UTM_zonenumber=corners[0].get_UTM_zonenumber();

   threevector grid_origin=corners[0].get_UTM_posn();
   threevector Uvec=(corners[1].get_UTM_posn()-corners[0].get_UTM_posn());
   threevector Vvec=(corners[3].get_UTM_posn()-corners[0].get_UTM_posn());
//   cout << "Uvec = " << Uvec << endl;
//   cout << "Vvec = " << Vvec << endl;
   double U=Uvec.magnitude();
   double V=Vvec.magnitude();
//   cout << "U = " << U << " V = " << V << endl;

   threevector uhat=Uvec.unitvector();
   threevector vhat(-uhat.get(1),uhat.get(0));
   cout << "uhat = " << uhat << endl;
   cout << "vhat = " << vhat << endl;

   const double meters_per_nmi=1852;
   int du=gridsize_in_nmi*meters_per_nmi;	// 1 nmi for Coronado
   int dv=gridsize_in_nmi*meters_per_nmi;	// 1 nmi for Coronado
//   int du=2*meters_per_nmi;	// 2 nmi
//   int dv=2*meters_per_nmi;	// 2 nmi
   int n_ubins=U/du;
   int n_vbins=V/dv;
   if (is_odd(n_ubins)) n_ubins++;
   if (is_odd(n_vbins)) n_vbins++;
   cout << "n_ubins = " << n_ubins << " n_vbins = " << n_vbins << endl;

/*
// Approximately double Coronado National Forest North grid size:

   int n_extra_rows=6;
   grid_origin -= n_extra_rows*du*uhat;
   grid_origin -= n_extra_rows*dv*vhat;
*/

/*
// Move grid origin deeper into Mexican territory:

   int n_extra_rows=10;
   grid_origin -= n_extra_rows*dv*vhat;
   n_vbins += n_extra_rows;
*/

   vector<geopoint> start_vertices,stop_vertices;

// Open text file for writing out starting and stopping gridline
// endpoints in decimal degrees for importing into FalconView via Excel:
   
   string gridlines_filename="grid_endpoints.txt";
   ofstream outstream;
   outstream.precision(9);
   filefunc::openfile(gridlines_filename,outstream);
   
// Compute start/stop points for grid lines parallel to vhat:

//   n_ubins=24;	        // Coronado National Forest North
//   n_vbins=24;		// Coronado National Forest North
//   n_ubins=5;	        // NH practice grid
//   n_vbins=15;	// NH practice grid
//   n_ubins=40;	// AZ border grid
//   n_vbins=26;	// AZ border grid
   for (int i=0; i<n_ubins+1; i++)
   {
      threevector start_point=grid_origin+i*du*uhat;
      threevector stop_point=grid_origin+i*du*uhat+n_vbins*dv*vhat;

      geopoint start_vertex(
         northern_hemisphere_flag,UTM_zonenumber,
         start_point.get(0),start_point.get(1));

      geopoint stop_vertex(
         northern_hemisphere_flag,UTM_zonenumber,
         stop_point.get(0),stop_point.get(1));
      
//      cout << "start = " << start_vertex 
//           << " stop = " << stop_vertex << endl;
      start_vertices.push_back(start_vertex);
      stop_vertices.push_back(stop_vertex);

      outstream << "N" << start_vertex.get_latitude() << "w"
                << fabs(start_vertex.get_longitude()) << endl;
      outstream << "N" << stop_vertex.get_latitude() << "w"
                << fabs(stop_vertex.get_longitude()) << endl;
   } // loop over index i labeling ubins

   for (int i=0; i<n_vbins+1; i++)
   {
      threevector start_point=grid_origin+i*dv*vhat;
      threevector stop_point=grid_origin+i*dv*vhat+n_ubins*du*uhat;

      geopoint start_vertex(
         northern_hemisphere_flag,UTM_zonenumber,
         start_point.get(0),start_point.get(1));

      geopoint stop_vertex(
         northern_hemisphere_flag,UTM_zonenumber,
         stop_point.get(0),stop_point.get(1));
      
//      cout << "start = " << start_vertex 
//           << " stop = " << stop_vertex << endl;
      start_vertices.push_back(start_vertex);
      stop_vertices.push_back(stop_vertex);

      outstream << "N" << start_vertex.get_latitude() << "w"
                << fabs(start_vertex.get_longitude()) << endl;
      outstream << "N" << stop_vertex.get_latitude() << "w"
                << fabs(stop_vertex.get_longitude()) << endl;
   } // loop over index i labeling ubins

   filefunc::closefile(gridlines_filename,outstream);

   string banner="Grid info for FalconView (via Excel) written to ";
   banner += gridlines_filename;
   outputfunc::write_big_banner(banner);

// Generate KML version of alpha-numeric grid which FLIR operators
// onboard Twin Otter can use:

   kml_parser KmlParser;
   string output_kml_filename="grid.kml";
   filefunc::deletefile(output_kml_filename);

   vector<string> pushpin_name,pushpin_label;
   vector<double> pushpin_lon,pushpin_lat;

   int Achar_int=stringfunc::char_to_ascii_integer('A');
   for (int i=0; i<n_ubins; i++)
   {
      for (int j=0; j<n_vbins; j++)
      {
         char curr_char=stringfunc::ascii_integer_to_char(Achar_int+j);
         string letter_str=stringfunc::char_to_string(curr_char);
         if (j >= 26)
         {
            int k=j-26;
            curr_char=stringfunc::ascii_integer_to_char(Achar_int+k);
            letter_str=stringfunc::char_to_string(curr_char);
            letter_str += letter_str;
         }

         string name=letter_str+" "+stringfunc::number_to_string(i);
         pushpin_name.push_back(name);
         threevector cell_center=
            grid_origin+(i+0.5)*du*uhat+(j+0.5)*dv*vhat;
         geopoint center_vertex(
            northern_hemisphere_flag,UTM_zonenumber,
            cell_center.get(0),cell_center.get(1));
         double lon=center_vertex.get_longitude();
         double lat=center_vertex.get_latitude();
         pushpin_lon.push_back(lon);
         pushpin_lat.push_back(lat);
         string label="Grid cell center: \n lon = "
            +stringfunc::number_to_string(lon,7)
            +"\n lat = "+stringfunc::number_to_string(lat,7);
         pushpin_label.push_back(label);
      } // loop over j index
   } // loop over i index

   KmlParser.generate_gridlines_kml_file(
      n_ubins,n_vbins,start_vertices,stop_vertices,
      pushpin_name,pushpin_label,pushpin_lon,pushpin_lat,
      output_kml_filename);

   banner="KML version of grid written to "+output_kml_filename;
   outputfunc::write_big_banner(banner);

}

