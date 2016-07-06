// ==========================================================================
// Header file for GraphQuery class
// ==========================================================================
// Last modified on 2/24/08
// ==========================================================================

#ifndef _GRAPHQUERY_H
#define _GRAPHQUERY_H

#include <string>
#include "track/tracklist.h"

// Exception thrown when bad JSON is downloaded
class InvalidFormat
{
  public:
   InvalidFormat(char *msg) { message = msg; }
   char *message;
};

class GraphQuery
{
  public:
   // Create a GraphQuery that will query a particular server
   GraphQuery( std::string server );

   ~GraphQuery();

   // Return all tracks that fall inside a given time/space bounding box

   TrackList* bboxQuery( double xmin, double ymin, 
                         double xmax, double ymax,
                         long long t0, long long t1,
                         std::string type );

   // Return an EntityList object populated with the JSON result from
   // the query Returns 0 on success, -1 on failure

   int query( double &latitude, double &longitude, std::string q );
   TrackList* rawQuery( std::string q );

   // Parse the JSON query result. Return an EntityList with the contents.
   TrackList* parseJSON( std::stringstream &str );

   // Called when data is received from libcurl
   size_t handleData( void *data, size_t size, size_t nmemb );

  private:
   std::string m_server;
   std::string m_buffer;
};

size_t _queryCURLCallback( void *data, size_t size, size_t nmemb,
                           void *graphquery );

#endif
