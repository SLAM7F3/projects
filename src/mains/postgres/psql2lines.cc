// ========================================================================
// Program PSQL2LINES reads in North American border information from
// a shape file supplied to us by Jim Garlick.  It uses the OGR
// library to parse a PostGIS table created from the shape file and to
// extract MultiLineString objects.  This program writes out an ascii
// file containing vertex information which can be used to generate
// polylines within our OSG viewers to display the state borders.

// Important note: This program must be run from a subdirectory
// containing links to .shp, .shx and .dbf files!

// ========================================================================
// Last updated on 1/17/07; 1/18/07
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "ogrsf_frmts.h"
#include "osg/osgGIS/postgis_database.h"
#include "general/filefuncs.h"

int main()
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ofstream;
   using std::string;
   using std::vector;
   
   cout.precision(10);

// Instantiate database object to send data to and retrieve data from
// an external PostGIS database:

   string hostname="fusion1";
   string database_name="babygis";
   string username="cho";
   postgis_database babygis_db(hostname,database_name,username);

   string TableName="eotmajroads";
   babygis_db.read_table(TableName);
   babygis_db.setup_coordinate_transformation("WGS84",2001,19,true);

   const double xmin = 324211;	// meters
   const double xmax = 336667;	// meters
   const double ymin = 4.68985e+06;	// meters
   const double ymax = 4.69363e+06;	// meters
   babygis_db.pushback_gis_bbox(xmin,xmax,ymin,ymax);
   babygis_db.parse_table_contents();
}
