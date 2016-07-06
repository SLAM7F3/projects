// ========================================================================
// GraphQuery class
// ========================================================================
// Last updated on 2/24/08
// ========================================================================

#include <jaula.h>
#include <curl/curl.h>
#include <sstream>
#include <iostream>

#include "track/graphquery.h"
#include "track/observation.h"
#include "track/trackitem.h"
#include "track/tracklist.h"

using namespace JAULA;

using std::cout;
using std::endl;
using std::ios;
using std::list;
using std::string;
using std::stringstream;

// ---------------------------------------------------------------------
size_t _queryCURLCallback( void *data, size_t size, size_t nmemb, 
                           void *graphquery )
{
   GraphQuery *g = static_cast<GraphQuery *>(graphquery);

   return g->handleData(data, size, nmemb);
}

// ---------------------------------------------------------------------
GraphQuery::GraphQuery( string server )
{
   m_server = server;
   // Must end in a '?'
   if( m_server[m_server.size()-1] != '?' ) {
      m_server += "?";
   }
   m_buffer.clear();
}

// ---------------------------------------------------------------------
GraphQuery::~GraphQuery()
{
   // this space intentionally left blank
}

// ---------------------------------------------------------------------
TrackList* GraphQuery::parseJSON( stringstream &str )
{
//    cout << "inside GraphQuery::parseJSON()" << endl;

   try {
      Value_Complex* json = Parser::parseStream(str); 

      // It should be a single element called "trackList"
//       cout << "Has " << json->size() << " elements." << endl;
      if( json->size() != 1 )
         throw InvalidFormat( (char *) "Wrong number of elements in root JSON object.");

      // The root element should be an { object }, not an [ array ]
      // JAULA seems to call it an array though, even when it is actually
      // an object.
      if( json->getType() != Value::TYPE_ARRAY )
         throw InvalidFormat( (char *) "Root element should be of type TYPE_OBJECT.");

      // Convert to a Value_Object now, and try to enumerate the elements
      Value_Object *obj = dynamic_cast<Value_Object *>(json);
      Value* trackList = obj->getData().begin()->second;

      // Inside TrackList should be an array
      if( trackList->getType() != Value::TYPE_ARRAY )
         throw InvalidFormat( (char *) "trackList should be of type TYPE_ARRAY");

      // Enumerate array elements
      Value_Array *trackArray = dynamic_cast<Value_Array *>(trackList);
      Value_Array::dataType d = trackArray->getData();
      Value_Array::dataType::iterator i;
//       cout << "Track List has " << trackArray->size() << " elements." << endl;

      TrackList *tl = new TrackList;
      for( i = d.begin(); i != d.end(); ++i ) {
         // Each internal element should be an { object }
         if( (*i)->getType() != Value::TYPE_ARRAY )
            throw InvalidFormat( (char *) "Track item should be of type TYPE_OBJECT.");

         // Enumerate its elements
//          cout << "track w/ type: " << (*i)->getType() << endl;
         Value_Object *track = dynamic_cast<Value_Object *>(*i);
         Value_Object::dataType propertyMap = track->getData();
         Value_Object::dataType::iterator j;

         string entityId = "";
         string entityType = "";
         string label = "";
         list<Observation *> obl;

         for( j = propertyMap.begin(); j != propertyMap.end(); ++j ) {
            // Each property inside the track should be an { object } with a name
            // and a value.
            if( j->first == string("entityId") ) {
//                cout << "entityId" << endl;
               entityId = dynamic_cast<Value_String *>(j->second)->getData();
            } else if( j->first == "entityType" ) {
//                cout << "entityType" << endl;
               entityType = dynamic_cast<Value_String *>(j->second)->getData();
            } else if( j->first == "label" ) {
//                cout << "label" << endl;
               label = dynamic_cast<Value_String *>(j->second)->getData();
            } else if( j->first == "observationList" ) {
               //cout << "observationList" << endl;
               if( j->second->getType() != Value::TYPE_ARRAY )
                  throw InvalidFormat( (char *) "Observation list should be a TYPE_ARRAY");
          
               // Iterate through the Observations in the [ array ]
               Value_Array *obsArray = dynamic_cast<Value_Array *>(j->second);
               Value_Array::dataType obsList = obsArray->getData();
               Value_Array::dataType::iterator obs;

               for( obs = obsList.begin(); obs != obsList.end(); ++obs ) {
                  // Each Observation should be an { object }
                  if( (*obs)->getType() != Value::TYPE_ARRAY )
                     throw InvalidFormat( (char *) 
                        "Observation should be a TYPE_OBJECT.");

                  //cout << "observation:" << endl;
                  Value_Object::dataType obsProperty = 
                     dynamic_cast<Value_Object *>(*obs)->getData();
                  Value_Object::dataType::iterator k;
                  Observation* ob = new Observation;
                  for (k = obsProperty.begin(); k != obsProperty.end(); ++k) 
                  {
                     if ( k->first == "lat" ) 
                     {
                        ob->set_latitude(dynamic_cast<Value_Number *>(
                              k->second)->getData());
                     } 
                     else if( k->first == "lon" ) 
                     {
                        ob->set_longitude(dynamic_cast<Value_Number *>(
                           k->second)->getData());
                     } 
                     else if( k->first == "time" ) 
                     {
                        ob->set_time( 
                           dynamic_cast<Value_Number_Int *>(
                              k->second)->getData());
                     } 
                     else 
                     { 
                        //cout << "!!unknown observation property!!" << endl;
                     }
                  }
                  obl.push_back(ob);
               }
            } 
            else 
            {
               //cout << "!!unknown track property!! : " << j->first << endl;
            }
         }

         TrackItem *ti = new TrackItem( entityId, entityType, label, obl );
         tl->addItem( ti );
      }

      return tl;
   } catch (Exception ex) {
      cout << "Error. Invalid JSON?" << endl;
   } catch (InvalidFormat e) {
      cout << "InvalidFormat: " << e.message << endl;
   }

   return NULL;
}

// ---------------------------------------------------------------------
TrackList* GraphQuery::bboxQuery( double xmin, double ymin, 
                                  double xmax, double ymax,
                                  long long t0, long long t1,
                                  string type )
{

// http://127.0.0.1:8080/SKSDataServer/getObservations?mode=all&bbox=-101.97,33.48,-101.91,33.53&t0=1190908800&t1=1190912400&types=vehicle&labelfield=id_bg&

   stringstream url;

   // Build a query and send it to rawQuery
   url << "mode=" << "all" << "&";
   url << "format="  << "json" << "&";
   url << "bbox=" << xmin << "," << ymin << "," << xmax << "," << ymax << "&";
   url << "t0=" << t0 << "&";
   url << "t1=" << t1 << "&";
   url << "types=" << type << "&";

   return rawQuery( m_server + url.str() );
}

// ---------------------------------------------------------------------
TrackList* GraphQuery::rawQuery( string q )
{
//    cout << "inside GraphQuery::rawQuery, string q = " << endl;
//    cout << q << endl;

   //char *clean_query;
   //stringstream url;

   // Try to set up curl...
   CURL* curl = curl_easy_init();
   if( !curl ) 
      return NULL;

   // Clean that query URL by escaping non-URL characters
   /*
     clean_query = curl_easy_escape(curl, q.c_str(), q.length());
     if( !clean_query ) {
     curl_easy_cleanup(curl);
     return NULL;
     }
   */

   // Build a query url
   //url << m_server;
   //url << "format="  << "json" << "&";
   //url << "bbox=" << "5" << "," << "5" << "," << "10" << "," << "10" << "&";
   //url << "q="       << clean_query << "&";

   //string finalUrl( url.str() );
   //string finalUrl( q );

   // Initialize the buffer (it will contain the HTTP response)
   m_buffer.clear();

   // Try to get the web page

//   std::cout << q << std::endl;

   curl_easy_setopt( curl, CURLOPT_URL, q.c_str() );
   curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1 );
   curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, _queryCURLCallback );
   curl_easy_setopt( curl, CURLOPT_WRITEDATA, this );
   curl_easy_perform( curl );

   // Clean up
   curl_easy_cleanup(curl);
  
   // Create a stringstream to parse the JSON

   stringstream str(m_buffer, ios::in);
//   cout << " m_buffer = " << endl;
//   cout << m_buffer << endl;

   TrackList* tl = parseJSON(str);
   return tl;
}

// ---------------------------------------------------------------------
size_t GraphQuery::handleData( void *data, size_t size, size_t nmemb )
{
   m_buffer.append((const char *)data, size * nmemb);

   return size * nmemb;
}

