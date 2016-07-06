// ========================================================================
// Program LANDMARKS reads in MassGIS landmark information from a
// shape file.  It uses the OGR library to parse the shape file and
// extract Point objects which intersect the Boston bounding box.
// This program writes out a new shape file containing just those
// landmarks which have names associated with them.

// Important note: This program must be run from a subdirectory
// containing links to .shp, .shx and .dbf files!

// ========================================================================
// Last updated on 12/26/06
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "ogrsf_frmts.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "datastructures/Triple.h"
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

//   string input_shape_filename="landmax1.shp";
   string input_shape_filename="SCHOOLS_PT.shp";
   cout << "Enter input shape filename:" << endl;
   cin >> input_shape_filename;
   
   OGRDataSource* poDS = OGRSFDriverRegistrar::Open( 
      input_shape_filename.c_str(), FALSE );
   if ( poDS == NULL )
   {
      cout << "Open failed" << endl;
      exit( 1 );
   }

   int layercount=poDS->GetLayerCount();
   cout << "Number of layers within input shape file = " << layercount 
        << endl;
   OGRLayer* poLayer = poDS->GetLayer(0);
   poLayer->ResetReading();

// ========================================================================
// Set up transformation from Massachusetts state plane coordinates to
// UTM zone 19 coordinates:

   OGRSpatialReference oSRS,tSRS,lSRS;

   oSRS.SetProjCS( "Massachusetts state plane coord system" );
   oSRS.SetWellKnownGeogCS( "WGS84" );
   oSRS.SetStatePlane( 2001, TRUE );

   tSRS.SetProjCS( "UTM coord system" );
   tSRS.SetWellKnownGeogCS( "WGS84" );
   tSRS.SetUTM( 19, TRUE );

   lSRS.SetWellKnownGeogCS( "WGS84" );

//   vector<string> param_name;
//   param_name.push_back("standard_parallel_1");
//   param_name.push_back("standard_parallel_2");
//   param_name.push_back("false_easting");
//   param_name.push_back("false_northing");
//   param_name.push_back("central_meridian");
//   param_name.push_back("latitude_of_origin");
//   for (int i=0; i<param_name.size(); i++)
//   {
//      cout << param_name[i] << " = " 
//           << oSRS.GetProjParm(param_name[i].c_str(),-999.99) << endl;
//      cout << param_name[i] << " = " 
//           << tSRS.GetProjParm(param_name[i].c_str(),-999.99) << endl;
//   }

   OGRCoordinateTransformation* poCT = OGRCreateCoordinateTransformation( 
//      &oSRS,&tSRS );
      &oSRS,&lSRS );
   if (poCT == NULL)
   {
      cout << "Transformation from state plane to UTM is invalid!" << endl;
      exit(-1);
   } // poCT != NULL conditional

// ========================================================================
// Extract landmark locations from database file.  Transform to UTM
// coordinates. 

//   string points_filename="landmark_points.txt";
//   ofstream landmarkstream;
//   landmarkstream.precision(10);

   vector<Triple<string,double,double> > landmarks;

   OGRFeature* poFeature;
   while( (poFeature = poLayer->GetNextFeature()) != NULL)
   {
      OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
      string landmark_name;
      for( int iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
      {
         OGRFieldDefn* poFieldDefn = poFDefn->GetFieldDefn( iField );
         string field_name(poFieldDefn->GetNameRef());

//         if (field_name=="LANAME")
         if (field_name=="NAME")
//         if (field_name=="COLLEGE")
         {
            landmark_name=poFeature->GetFieldAsString( iField );
         }
      } // loop over iField index labeling feature fields

      if (landmark_name.size() > 0)
      {
         OGRGeometry* poGeometry = poFeature->GetGeometryRef();
         if ( poGeometry != NULL && poGeometry->getGeometryType()== wkbPoint )
         {
            OGRPoint* poPoint=dynamic_cast<OGRPoint*>(poGeometry);

            double x=poPoint->getX();
            double y=poPoint->getY();
//            cout << " x = " << x << " y = " << y ;
            poCT->Transform(1,&x,&y);
//            cout << landmark_name << " xnew = " << x << " ynew = " << y 
//                 << endl;

// Don't bother to write out landmark vertices if they lie outside bbox
// surrounding Boston point cloud:

//            const double xmin = 324211;	// UTM easting
//            const double xmax = 336667;	// UTM easting
//            const double ymin = 4.68985e+06;  // UTM northing
//            const double ymax = 4.69363e+06;  // UTM northing

            const double xmin = -71.13;	// degs
            const double xmax = -70.99; // degs
            const double ymin = 42.345; // degs
            const double ymax = 42.375; // degs

            if (x > xmin && x < xmax && y > ymin && y < ymax )
            {
               cout << "landmark_name = " << landmark_name
                    << " longitude = " << x << " latitude = " << y << endl;
//                    << " East = " << x << " North = " << y << endl;
               Triple<string,double,double> t(landmark_name,x,y);
               landmarks.push_back(t);
            }
            
         } // geometry = Point conditional
         OGRFeature::DestroyFeature( poFeature );
      } // landmark_name.size() > 0 conditional
   } // while loop

//   filefunc::closefile(points_filename,landmarkstream);

//   templatefunc::printVector(landmark_class_freq);
//   templatefunc::printVector(admin_type_freq);

   OGRDataSource::DestroyDataSource( poDS );

// ========================================================================
// Write reduced landmark locations to output shape file

   string driver_name="ESRI Shapefile";
   OGRSFDriver* poDriver = OGRSFDriverRegistrar::GetRegistrar()->
      GetDriverByName(driver_name.c_str());
   if ( poDriver == NULL )
   {
      cout << "Driver not available" << endl;
      exit( 1 );
   }

   string output_subdir="./boston_landmarks";
   OGRDataSource* poDS_new = poDriver->CreateDataSource( 
      output_subdir.c_str(), NULL );
   if ( poDS_new == NULL )
   {
      cout << "Creation of output shape files failed" << endl;
      exit( 1 );
   }

   OGRLayer* poLayer_new = poDS_new->CreateLayer( 
      "boston_landmark", NULL, wkbPoint, NULL );
   if ( poLayer_new == NULL )
   {
      cout << "Layer creation failed" << endl;
      exit( 1 );
   }

   OGRFieldDefn oField( "Name", OFTString );
   oField.SetWidth(32);
   if ( poLayer_new->CreateField( &oField ) != OGRERR_NONE )
   {
      cout << "Name field creation failed" << endl;
      exit( 1 );
   }

   for (int i=0; i<landmarks.size(); i++)
   {
      Triple<string,double,double> curr_triple=landmarks[i];
      OGRFeature* poFeature = new OGRFeature( poLayer_new->GetLayerDefn() );

      poFeature->SetField( 0, curr_triple.first.c_str() );
      OGRPoint pt;

      pt.setX( curr_triple.second );
      pt.setY( curr_triple.third );
      poFeature->SetGeometry( &pt ); 
      
      if ( poLayer_new->CreateFeature( poFeature ) != OGRERR_NONE )
      {
         cout << "Failed to create feature in shapefile" << endl;
         exit( 1 );
      }

      OGRFeature::DestroyFeature( poFeature );
   } // loop over index i labeling reduced landmarks lying within
     //  pointcloud region

   OGRDataSource::DestroyDataSource( poDS_new );
}
