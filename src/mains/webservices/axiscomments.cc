// =======================================================================
// Main toy program to play with c++ web services
// =======================================================================
// Last updated on 11/22/06; 12/12/06; 12/13/06
// =======================================================================

#include <iostream>
#include <stdlib.h>
#include <string>
#include "axis/EntityVO.hpp"
#include "axis/SKSWebServicePortType.hpp"


// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;

// Try to find a known entityVO from the SKSCore.

   EntityVO my_entityVO;

   cout << "hello world" << endl;
   
   xsd__string inStr = "Peter's client from laptop!";
   xsd__string outStr;

   SKSWebServicePortType sksWebService=SKSWebServicePortType();
   outStr = sksWebService.echoString(inStr);

   cout << "result is = %d\n" << outStr << endl;
   cout << "Hit enter to continue...";
   cin.get();        
   return 0;



/*

   LONG64 id = 183;
   entityVO.id = &id;

   _ns1__findByPK6 pkInput;
   pkInput.in0 = &entityVO;


   _ns1__findByPK6Response pkOutput;

   if (service.__ns1__findByPK6(&pkInput, &pkOutput) == SOAP_OK)
   {
      cout << "DBTableName   = " 
           << *((ns4__EntityVO) *pkOutput.out).DBTableName << endl;
      cout << "VOName        = " 
           << *((ns4__EntityVO) *pkOutput.out).VOName << endl;
      cout << "dataUrl       = " 
           << *((ns4__EntityVO) *pkOutput.out).dataUrl << endl;
      cout << "dateCreated   = " 
           << *((ns4__EntityVO) *pkOutput.out).dateCreated << endl;
      cout << "dateInstalled = " 
           << *((ns4__EntityVO) *pkOutput.out).dateInstalled << endl;
      cout << "entityType    = " 
           << *((ns4__EntityVO) *pkOutput.out).entityType << endl;
      cout << "geometry      = " 
           << *((ns4__EntityVO) *pkOutput.out).geometry << endl;
      cout << "id            = " 
           << *((ns4__EntityVO) *pkOutput.out).id << endl;
      cout << "identifier    = " 
           << *((ns4__EntityVO) *pkOutput.out).identifier << endl;
      cout << "locationId    = " 
           << *((ns4__EntityVO) *pkOutput.out).locationId << endl;
      cout << "mimetype      = " 
           << *((ns4__EntityVO) *pkOutput.out).mimetype << endl;
      cout << "securityLabel = " 
           << *((ns4__EntityVO) *pkOutput.out).securityLabel << endl;
      cout << "sourceUrl     = " 
           << *((ns4__EntityVO) *pkOutput.out).sourceUrl << endl;         
      cout << "altitude      = " 
           << *((ns4__EntityVO) *pkOutput.out).altitude << endl;
   }
   else
   {
      cout << "AN ERROR OCCURED DURING __ns1__findByPK6()" << endl;
      soap_print_fault(service.soap, stderr);
   }
*/
   return 0;

}
