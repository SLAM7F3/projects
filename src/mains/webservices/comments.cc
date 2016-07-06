// =======================================================================
// Main toy program to play with c++ web services
// =======================================================================
// Last updated on 11/22/06; 12/12/06
// =======================================================================

#include <iostream>
#include <stdlib.h>
#include <string>
#include "isds/soapSKSWebServiceHttpBindingProxy.h" // get proxy
#include "isds/SKSWebServiceHttpBinding.nsmap" // get namespace bindings 

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;

   struct soap *soap = soap_new(); 
        
   //Simply say hello to the webservice.
   _ns1__echoString input = _ns1__echoString();
   string in = "THIS IS CPP CLIENT!";
   input.in0 = &in;
   _ns1__echoStringResponse output;
   SKSWebServiceHttpBinding service;
   if (service.__ns1__echoString(&input, &output) == SOAP_OK)
   {
      cout << *output.out << endl;
   }
   else
   {
      cout << "AN ERROR OCCURED DURING echoString()" << endl;
      soap_print_fault(service.soap, stderr);
   }
        
// Try to find a known entityVO from the SKSCore.

   ns4__EntityVO entityVO;
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
   return 0;
}
