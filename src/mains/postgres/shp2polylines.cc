// ========================================================================
// Program SHP2POLYLINES reads in MassGIS streets information from a
// shape file.  It uses the OGR library to parse the shape file and
// extract LineString objects which intersect the Boston bounding box.
// This program writes out an ascii file containing vertex information
// which can be used to generate polylines within our OSG viewers to
// display the street locations.

// Important note: This program must be run from a subdirectory
// containing links to .shp, .shx and .dbf files!

// ========================================================================
// Last updated on 12/22/06
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "ogrsf_frmts.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "templates/mytemplates.h"

int main()
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ofstream;
   using std::string;
   using std::vector;
   
   cout.precision(10);

   OGRRegisterAll();

   string shape_filename="EOTMAJROADS_ARC.shp";
//   cout << "Enter input shape filename:" << endl;
//   cin >> shape_filename;

   OGRDataSource* poDS = OGRSFDriverRegistrar::Open( 
      shape_filename.c_str(), FALSE );

   if( poDS == NULL )
   {
      cout << "Open failed" << endl;
      exit( 1 );
   }

   int layercount=poDS->GetLayerCount();
   cout << "Number of layers within input shape file = " 
        << layercount << endl;
   OGRLayer* poLayer = poDS->GetLayer(0);
   poLayer->ResetReading();

// ========================================================================
// Set up transformation from Massachusetts state plane coordinates to
// UTM zone 19 coordinates:

   OGRSpatialReference oSRS,tSRS;

   oSRS.SetProjCS( "Massachusetts state plane coord system" );
   oSRS.SetWellKnownGeogCS( "WGS84" );
   oSRS.SetStatePlane( 2001, TRUE );

   tSRS.SetProjCS( "UTM coord system" );
   tSRS.SetWellKnownGeogCS( "WGS84" );
   tSRS.SetUTM( 19, TRUE );

   vector<string> param_name;
   param_name.push_back("standard_parallel_1");
   param_name.push_back("standard_parallel_2");
   param_name.push_back("false_easting");
   param_name.push_back("false_northing");
   param_name.push_back("central_meridian");
   param_name.push_back("latitude_of_origin");
   for (int i=0; i<param_name.size(); i++)
   {
//      cout << param_name[i] << " = " 
//           << oSRS.GetProjParm(param_name[i].c_str(),-999.99) << endl;
//      cout << param_name[i] << " = " 
//           << tSRS.GetProjParm(param_name[i].c_str(),-999.99) << endl;
   }

   OGRCoordinateTransformation* poCT = OGRCreateCoordinateTransformation( 
      &oSRS,&tSRS );
   if (poCT == NULL)
   {
      cout << "Transformation from state plane to UTM is invalid!" << endl;
      exit(-1);
   } // poCT != NULL conditional

// ========================================================================
// Extract street vertices from database file.  Transform to UTM
// coordinates.  Write each street's vertices to an output text file
// which can be used to generate OSG polylines.

   string polylines_filename="street_polylines.txt";
   ofstream streetstream;
   streetstream.precision(10);
   filefunc::openfile(polylines_filename,streetstream);

   int street_ID=0;
//   int max_streets=3;
   int max_streets=3000000;
   vector<int> street_class_freq(5);
   vector<int> admin_type_freq(5);

   OGRFeature* poFeature;
   while( (poFeature = poLayer->GetNextFeature()) != NULL
          && street_ID < max_streets)
   {
      string banner="Street counter = "+stringfunc::number_to_string(
         street_ID);
      if (street_ID%1000==0) outputfunc::write_big_banner(banner);

      OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
      int street_class,admin_type;
      colorfunc::RGB curr_rgb;
      for( int iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
      {
         OGRFieldDefn* poFieldDefn = poFDefn->GetFieldDefn( iField );
         string field_name(poFieldDefn->GetNameRef());

         if (field_name=="CLASS")
         {
            street_class=poFeature->GetFieldAsInteger( iField );
            street_class_freq[street_class] = 
               street_class_freq[street_class]+1;
         }
         else if (field_name=="ADMIN_TYPE")
         {
            admin_type=poFeature->GetFieldAsInteger( iField );
            admin_type_freq[admin_type] = admin_type_freq[admin_type]+1;
         }
/*         
         cout << "i = " << iField << " field_name = " << field_name << endl;
         if ( poFieldDefn->GetType() == OFTInteger )
         {
            cout << poFeature->GetFieldAsInteger( iField ) << endl;
         }
         else if ( poFieldDefn->GetType() == OFTReal )
         {
            cout << poFeature->GetFieldAsDouble(iField) << endl;
         }
         else
         {
            cout << poFeature->GetFieldAsString(iField) << endl;
         }
*/
      } // loop over iField index labeling feature fields

      OGRGeometry* poGeometry = poFeature->GetGeometryRef();
//      cout << "Geometry name = " 
//           << string(poGeometry->getGeometryName()) << endl;
//      cout << "geom dim = " << poGeometry->getDimension() << endl;
//      cout << "coord dim = " << poGeometry->getCoordinateDimension() << endl;

      if( poGeometry != NULL && 
          poGeometry->getGeometryType()== wkbLineString )
      {
         OGRLineString* poLineString=dynamic_cast<OGRLineString*>(poGeometry);

         for (int p=0; p<poLineString->getNumPoints(); p++)
         {
            double x=poLineString->getX(p);
            double y=poLineString->getY(p);
//            cout << "p = " << p << " x = " << x << " y = " << y ;
            poCT->Transform(1,&x,&y);
//            cout << " xnew = " << x << " ynew = " << y << endl;
//
//            outputfunc::enter_continue_char();
            
            const double t=0;
            const int pass_number=0;
            const double z=10;

/*
//            switch (street_class)
            switch (admin_type)
            {
               case 0:
                  curr_rgb=colorfunc::get_RGB_values(colorfunc::yellow);
                  break;
               case 1:
                  curr_rgb=colorfunc::get_RGB_values(colorfunc::white);
                  break;

               case 2:
                  curr_rgb=colorfunc::get_RGB_values(colorfunc::red);
                  break;

               case 3:
                  curr_rgb=colorfunc::get_RGB_values(colorfunc::cyan);
                  break;

               case 4:
                  curr_rgb=colorfunc::get_RGB_values(colorfunc::purple);
                  break;
            }
*/

            curr_rgb=colorfunc::get_RGB_values(colorfunc::white);
            
// Don't bother to write out street vertices if they lie outside bbox
// surrounding Boston point cloud:

            const double xmin = 324211;
            const double xmax = 336667;
            const double ymin = 4.68985e+06;
            const double ymax = 4.69363e+06;

            if (x > xmin && x < xmax && y > ymin && y < ymax )
            {
               streetstream << t << " "
                            << street_ID << " "
                            << pass_number << " "
                            << x << " "
                            << y << " "
                            << z << " "
                            << curr_rgb.first << " "
                            << curr_rgb.second << " "
                            << curr_rgb.third << endl;
            }
         } // loop over p index labeling vertices for a particular street
      }
      OGRFeature::DestroyFeature( poFeature );

      street_ID++;
   } // while loop

   filefunc::closefile(polylines_filename,streetstream);

   templatefunc::printVector(street_class_freq);
   templatefunc::printVector(admin_type_freq);

   OGRDataSource::DestroyDataSource( poDS );
}
