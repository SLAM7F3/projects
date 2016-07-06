#include <iostream>
#include <stdlib.h>
#include <string>

#include "isds/soapSKSWebServiceHttpBindingProxy.h" // get proxy
#include "isds/SKSWebServiceHttpBinding.nsmap" // get namespace bindings 

using namespace std;

int main(int argc, char *argv[])
{
   struct soap *soap = soap_new(); 
        
   //Simply say hello to the webservice.
   _ns1__echoString input = _ns1__echoString();
   std::string in = "THIS IS CPP CLIENT!";
   input.in0 = &in;
   _ns1__echoStringResponse output;
   SKSWebServiceHttpBinding service;
   if (service.__ns1__echoString(&input, &output) == SOAP_OK)
   {
      std::cout << *output.out << std::endl;
   }
   else
   {
      std::cout << "AN ERROR OCCURED DURING echoString()" << std::endl;
      soap_print_fault(service.soap, stderr);
   }
   std::cout << "Hit enter to continue...";
   std::cin.get();        

        
   //Try to find a known entityVO from the SKSCore.
   ns4__EntityVO entityVO = ns4__EntityVO();
   LONG64 id = 183;
   entityVO.id = &id;
   _ns1__findByPK6 pkInput = _ns1__findByPK6();
   pkInput.in0 = &entityVO;
   _ns1__findByPK6Response pkOutput;
   if (service.__ns1__findByPK6(&pkInput, &pkOutput) == SOAP_OK)
   {
      std::cout << "DBTableName   = " << *((ns4__EntityVO)* pkOutput.out).DBTableName << std::endl;

      std::cout << "VOName        = " << *((ns4__EntityVO) *pkOutput.out).VOName << std::endl;
      std::cout << "dataUrl       = " << *((ns4__EntityVO) *pkOutput.out).dataUrl << std::endl;
      std::cout << "dateCreated   = " << *((ns4__EntityVO) *pkOutput.out).dateCreated << std::endl;
      std::cout << "dateInstalled = " << *((ns4__EntityVO) *pkOutput.out).dateInstalled << std::endl;
      std::cout << "entityType    = " << *((ns4__EntityVO) *pkOutput.out).entityType << std::endl;
      std::cout << "geometry      = " << *((ns4__EntityVO) *pkOutput.out).geometry << std::endl;
      std::cout << "id            = " << *((ns4__EntityVO) *pkOutput.out).id << std::endl;
      std::cout << "identifier    = " << *((ns4__EntityVO) *pkOutput.out).identifier << std::endl;
      std::cout << "locationId    = " << *((ns4__EntityVO) *pkOutput.out).locationId << std::endl;
      std::cout << "mimetype      = " << *((ns4__EntityVO) *pkOutput.out).mimetype << std::endl;
      std::cout << "securityLabel = " << *((ns4__EntityVO) *pkOutput.out).securityLabel << std::endl;
      std::cout << "sourceUrl     = " << *((ns4__EntityVO) *pkOutput.out).sourceUrl << std::endl;                
   }
   else
   {
      std::cout << "AN ERROR OCCURED DURING __ns1__findByPK6()" << std::endl;
      soap_print_fault(service.soap, stderr);
   }
   std::cout << "Hit enter to continue...";
   std::cin.get();        
   return 0;
}
