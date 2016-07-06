// ========================================================================
// Program DATAPARSER is a playground for forming SKS DataServer
// queries as well as parsing XML returned by the SKS DataServer.


//	       dataparser SA6_countries_simple.response.xml 

//	       dataparser SA6_sites_range.response.xml


// ========================================================================
// Last updated on 3/15/08; 3/16/08
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "SKSDataServerInterfacer.h"
#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

int main( int argc, char** argv )
{

   string SKSDataServer_IP="touchy";
   int port=8080;
   SKSDataServerInterfacer sks_interface(SKSDataServer_IP,port);

//   string query1=sks_interface.form_all_SAM_attributes_query();
//   cout << "query = " << query1 << endl;

//   string query2=sks_interface.form_particular_SAM_attributes_query("SA-4");
//   cout << query2 << endl;

   double range=100;	// kms
   string query3=sks_interface.form_filtered_SAM_range_query(range);
   cout << query3 << endl;

//   int IOC=1975;
//   string query4=sks_interface.form_filtered_SAM_IOC_query(IOC);
//   cout << query4 << endl;

//   double range=100;	// kms
//   int IOC=1975;
//   string query5=sks_interface.form_filtered_SAM_range_and_IOC_query(
 //     range,IOC);
//   cout << query5 << endl;

//   string query6=sks_interface.form_countries_owning_SAM_query("SA-10");
//   cout << query6 << endl;

//   string query7=sks_interface.form_SAM_sites_query("SA-4");
//   cout << query7 << endl;

   exit(-1);

   string xml_filename(argv[1]);
   sks_interface.load_returned_XML_into_DOM(xml_filename);

   vector<string> named_values;
   if (sks_interface.extract_named_values_from_attributes(
      "name",named_values))
   {
      cout << "name = name " << endl;
      cout << "values = " << endl;
      templatefunc::printVector(named_values);
   }

   if (sks_interface.extract_named_values_from_attributes(
      "range.max.km",named_values))
   {
      cout << "name = range.max.km " << endl;
      cout << "values = " << endl;
      templatefunc::printVector(named_values);
   }

   vector<string> geometries;
   if (sks_interface.extract_named_geometries_from_coverages(
      "site",geometries))
   {
      cout << "site geometries = " << endl;
//      templatefunc::printVector(geometries);
      vector<threevector> lla=sks_interface.extract_points(geometries);
      templatefunc::printVector(lla);
   }
   
}
