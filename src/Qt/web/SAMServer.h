// ========================================================================
// SAMSERVER header file
// ========================================================================
// Last updated on 3/20/08; 3/21/08; 3/23/08; 3/24/08; 4/28/08
// ========================================================================

#ifndef __SAMSERVER_H__
#define __SAMSERVER_H__

#include <set>
#include <QtXml/QtXml>
#include <QtNetwork/QHttp>
#include "Qt/web/WebServer.h"

class SAM;
class SAMs_group;
class SKSDataServerInterfacer;
class WebClient;

class SAMServer : public WebServer
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   SAMServer(SAMs_group* SG_ptr,std::string host_IP_address, qint16 port,
             QObject* parent=NULL);
   SAMServer(SAMs_group* SG_ptr,std::string host_IP_address, qint16 port,
             WebClient* WebClient_ptr,QObject* parent=NULL);
   ~SAMServer();

// Set & get member functions:

   void set_HTMLServer_URL(std::string URL);

// SKS DataServer communication member functions:
    
   void establish_SKSDataServer_connection(std::string URL);
   std::string query_SKS_DataServer(std::string curr_query);

   protected slots:
        
  protected:

// HTTP processing member functions:

   virtual QString get( const QUrl& url );
   virtual QString get(
      QDomDocument& doc,QDomElement& response,const QUrl& url,
      std::string& URL_path);
   virtual QString post(const QUrl& url,const QByteArray& postData);

// SAM specific query member functions:

   void process_SAM_system_queries(
      const std::vector<std::pair<std::string,std::string> >& KeyValue,
      QDomDocument& doc,QDomElement& response);
   void process_country_queries(
      const std::vector<std::pair<std::string,std::string> >& KeyValue,
      QDomDocument& doc,QDomElement& response);
   void process_SAM_specs_queries(
      const std::vector<std::pair<std::string,std::string> >& KeyValue,
      QDomDocument& doc,QDomElement& response);
   void process_SAM_site_queries(
      const std::vector<std::pair<std::string,std::string> >& KeyValue,
      QDomDocument& doc,QDomElement& response);
   void process_flight_path_queries(
      std::string XML_string,QDomDocument& doc,QDomElement& response);

   std::vector<std::string> collate_country_owners(std::string SAM_name);
   std::vector<std::string> collate_country_owners(
      const std::vector<std::string>& SAM_names);
   void retrieve_SAM_sites(std::string SAM_name);

// XML/KML output member functions:

   void generate_XML_and_KML_query_output(
      std::string query,QDomDocument& doc,QDomElement& response);

   std::vector<std::string> output_SAM_list(
      QDomDocument& doc,QDomElement& response);
   void check_for_unknown_values(std::vector<std::string>& values);
   void output_SAM_list(
      const std::vector<SAM*>& representative_SAM_ptrs,
      QDomDocument& doc,QDomElement& response);

   void output_country_list(
      const std::vector<std::string>& country_owner_names,
      QDomDocument& doc,QDomElement& response);
   void output_site_list(
      const std::vector<SAM*> matching_SAMs_ptrs,
      QDomDocument& doc,QDomElement& response);

   void generate_SAM_owner_country_output(
      const std::vector<std::string>& SAM_names,
      QDomDocument& doc,QDomElement& response);
   void generate_SAM_sites_output(
      const std::vector<std::string>& SAM_names,
      QDomDocument& doc,QDomElement& response);
   
  private:

   std::string SKSDataServer_URL,HTMLServer_URL;
   SAMs_group* SAMs_group_ptr;
   SKSDataServerInterfacer* SKS_interface_ptr;
   QHttp http_client;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void SAMServer::set_HTMLServer_URL(std::string URL)
{
   HTMLServer_URL=URL;
}

#endif // __SAMSERVER_H__
