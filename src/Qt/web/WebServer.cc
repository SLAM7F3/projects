// ==========================================================================
// WEBSERVER class file
// ==========================================================================
// Last updated on 1/18/11; 1/20/11; 2/16/12
// ==========================================================================

#include <iostream>
#include <set>
#include <vector>
#include <QtCore/QtCore>
#include "WebServer.h"

#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void WebServer::allocate_member_objects()
{
   curl_ptr=curl_easy_init();
   server_ptr=new QTcpServer(this);
}		       

void WebServer::initialize_member_objects()
{
   Server_listening_flag=false;
   write_text_content_to_socket_flag=true;
   content_type="";
}

WebServer::WebServer(std::string host_IP, qint16 port, QObject* parent) 
   : QObject( parent )
{
   allocate_member_objects();
   initialize_member_objects();

   host_IP_address=host_IP;
   tomcat_URL_prefix="http://"+host_IP_address+":8080";

   host_address=QHostAddress(host_IP_address.c_str());
   port_number=port;
//   cout << "host IP address = " << host_IP_address << endl;
//   cout << "port number = " << port_number << endl;

// As of 6/26/08, we follow Ross Anderson's and Dave Ceddia's
// suggestion to set host_address to QHostAddress::Any so that Server
// can listen for client calls coming from any IP address and port:

   host_address=QHostAddress::Any;
   
   setup_initial_signal_slot_connections();

   cout << "Server not yet listening" << endl;

   int counter=0;
   while (!Server_listening_flag && counter < 1000)
   {
      Server_listening_flag=server_ptr->listen( host_address, port_number );
      counter++;
   }

   if (Server_listening_flag)
      cout << "Server now listening" << endl;

/*   

   if ( server_ptr->listen( host_address, port_number ) == false ) 
   {
      cout << "Unable to start web server on IP address = " 
           << host_address.toString().toStdString()
           << " port = " << port_number << endl;
//      qDebug() << "Unable to start the web server on IPD address = "
//               << host_IP_address 
//               << " port = " << port_number << ".";
      return;
   }
*/

}

// ---------------------------------------------------------------------
WebServer::~WebServer()
{
   curl_easy_cleanup(curl_ptr);
   server_ptr->close();
}

// ---------------------------------------------------------------------
// These first signal/slot relationships should be established BEFORE
// any network connections are made...

void WebServer::setup_initial_signal_slot_connections()
{
   connect( server_ptr, SIGNAL( newConnection() ), 
            this, SLOT( incomingConnection() ) );
}

// ---------------------------------------------------------------------
void WebServer::incomingConnection()
{
   if ( server_ptr == NULL ) return;
    
// Open a socket for the incoming connection and notify this upon
// available data:

   QTcpSocket* socket = server_ptr->nextPendingConnection();
   connect( socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()) );
   connect( socket, SIGNAL(readyRead()), this, SLOT(readSocket()) );
//   connect( socket, SIGNAL(bytesWritten(n_written_bytes)), this, 
//            SLOT(readSocket()) );
}

// ---------------------------------------------------------------------
void WebServer::readSocket()
{
//   cout << "inside WebServer::readSocket()" << endl;
   QTcpSocket*  socket = qobject_cast<QTcpSocket *>( sender() );
   if ( socket == 0 ) return;

   // have we already parsed the header from this socket?

   QHttpRequestHeader requestHeader;
   if ( _pending.contains( socket ) ) 
   {
      requestHeader = _pending[ socket ];
   } 
   else 
   {
      // accumulate header lines (everything up until \r\n)
      QString requestHeaderString;
      while( socket->canReadLine() ) 
      {
         QString line = socket->readLine();
         requestHeaderString += line;
         if ( line == "\r\n" ) break;    // end of header delimiter
      }
      requestHeader = QHttpRequestHeader( requestHeaderString );
   }
    
// Parse request header:

   QString method = requestHeader.method();
//   cout << "method = " << method.toStdString() << endl;
   uint content_length=requestHeader.contentLength();
//   cout << "requestHeader.content_length = " << content_length << endl;

   if (requestHeader.hasContentType())
   {
      content_type=requestHeader.contentType().toStdString();
//      cout << "content_type = " << content_type << endl;
   }
   else
   {
//      cout << "requestHeader has no content type" << endl;
   }
   
// Handle request method:

   if ( method == "GET" || method == "HEAD" ) 
   {
      QUrl url( requestHeader.path() );
        
      // does the resource exist?
      if ( isPathValid( url.path() ) ) 
      {
         // write header
         QHttpResponseHeader responseHeader( 200, "Ok" );

         responseHeader.setContentType( "text/javascript" );
//         responseHeader.setContentType( "text/xml" );
         QByteArray content=get(url,responseHeader);

         if (write_text_content_to_socket_flag)
         {
            if ( method == "GET" ) 
            {
               // write page content

               responseHeader.setContentLength(content.length());
               responseHeader.addValue( "charset", "utf-8" );

               QTextStream os( socket );
               os << responseHeader.toString();
               os << content;
            }
         } // write_text_content_to_socket_flag conditional
      } 
      else 
      {
         // Error 404
         QHttpResponseHeader responseHeader( 404, "Not Found" );
         responseHeader.setContentType( "text/xml" );
         responseHeader.addValue( "charset", "utf-8" );

         QTextStream os( socket );
         os << responseHeader.toString();
      }
   } 
   else if ( method == "POST" ) 
   {
//      cout << "method = POST" << endl;
      QUrl url( requestHeader.path() );
//      qDebug() << "url = " << url;

      // do not begin reading until all data is available
      if ( socket->bytesAvailable() < requestHeader.contentLength() ) 
      {
         _pending[ socket ] = requestHeader;
         return; // return without closing socket
      }
        
      // read attached POST data
      QByteArray postData;
      postData = socket->read( socket->bytesAvailable() );
//      cout << "postData.size() = " << postData.size() << endl;

      // remove socket from pending list, it's done.
      _pending.remove( socket );
        
      // does the resource exist?
      if ( isPathValid( url.path() ) ) 
      {
         // write header
         QHttpResponseHeader responseHeader( 200, "Ok" );
         responseHeader.setContentType( "text/javascript" );
//         responseHeader.setContentType( "text/html" );
//         responseHeader.setContentType( "text/xml" );
         responseHeader.addValue( "charset", "utf-8" );

         QByteArray content=post(url,postData,responseHeader);
         responseHeader.setContentLength(content.length());
         
//         cout << "POST request:" << endl;
//         cout << "content.length() = " 
//              << responseHeader.contentLength() << endl;

         QTextStream os( socket );
         os << responseHeader.toString();
         os << content;
      } 
      else 
      {
         // Error 404
         QHttpResponseHeader responseHeader( 404, "Not Found" );
         responseHeader.setContentType( "text/xml" );
//         responseHeader.setContentType( "text/html" );
         responseHeader.addValue( "charset", "utf-8" );
         QTextStream os( socket );
         os << responseHeader.toString();
      }
   } 
   else if ( method == "OPTIONS" ) 
   {
      // Return supported methods
      QHttpResponseHeader responseHeader( 200, "Ok" );
      responseHeader.addValue( "Public", "OPTIONS, GET, HEAD, POST" );
      QTextStream os( socket );
      os << responseHeader.toString();
   } 
   else 
   {
      // Error 405
      QHttpResponseHeader responseHeader( 405, "Method Not Allowed" );
      QTextStream os( socket );
      os << responseHeader.toString();
   }

   socket->close();
}

// ==========================================================================
// Set & get member functions:
// ==========================================================================

string WebServer::get_server_URL_prefix() const
{
   string URL_prefix="http://"+host_IP_address+":"+
      stringfunc::number_to_string(port_number);
   return URL_prefix;
}

string WebServer::get_tomcat_URL_prefix() const
{
   return tomcat_URL_prefix;
}

// ==========================================================================
// Get & post member functions
// ==========================================================================

bool WebServer::isPathValid( const QString& path )
{
   // no favicons here
   if ( path == "/favicon.ico" ) return false;
    
   return true;
}

// ---------------------------------------------------------------------
QByteArray WebServer::get( 
   const QUrl& url, QHttpResponseHeader& responseHeader)
{
   QDomDocument doc;
   QDomElement html = doc.createElement( "html" );
   return get(doc,html,url,responseHeader);
}

QByteArray WebServer::get(
   QDomDocument& doc,QDomElement& html,const QUrl& url,
   QHttpResponseHeader& responseHeader)
{
    Q_UNUSED(responseHeader);

   doc.appendChild( html );
    
// Display URL path:

   QDomElement path = doc.createElement( "h1" );
   html.appendChild( path );
   path.appendChild( doc.createTextNode( url.path() ) );
    
// Display key/value items attached to URL:

   typedef QPair<QString, QString> Pair;
   QList<Pair> items = url.queryItems();
   foreach( Pair item, items ) 
      {
         string key=item.first.toStdString();
         string value=item.second.toStdString();
//         cout << "key = " << key << " value = " << value << endl;
         
         QDomElement keyValueText = doc.createElement( "h2" );
         html.appendChild( keyValueText );
        
         keyValueText.appendChild( doc.createTextNode( 
            item.first + " = " + item.second ) );
      }
   
   return doc.toByteArray();
}

// ---------------------------------------------------------------------
// Member function extract_KeyValue_pairs() parses key & value
// information passed as part of the request header within the input
// URL string.

void WebServer::extract_KeyValue_pairs(const QUrl& url)
{
//   cout << "inside WebServer::extract_KeyValue_pairs()" << endl;

// Display key/value items attached to URL:

   KeyValue.clear();
   Key.clear();
   Value.clear();

   typedef QPair<QString, QString> Pair;
   QList<Pair> items = url.queryItems();
//   cout << "items.size() = " << items.size() << endl;

   int* outlength_ptr=new int;
   
   foreach( Pair item, items ) 
      {
         string key=item.first.toStdString();

//         string value=item.second.toStdString();	// Ubuntu 8.04

// ----------------------------------------------------------------------
// This next section of code replaces the above value line for Ubuntu 10.4!

         string encoded_value=item.second.toStdString();
//         cout << "encoded_value = " << encoded_value << endl;

// After lots of trial and error in June 2010 and with lots of help
// from Ross Anderson and Dave Ceddia, we finally figured out how to 
// to convert encoded URL values into conventional STL strings where
// '+' is replaced with ' ' and percent characters are returned to their
// unencoded forms.  See http://bugreports.qt.nokia.com/browse/QTBUG-10146 

         QString decoded_value=
            QUrl::fromPercentEncoding(
               url.encodedQueryItemValue(QByteArray(key.c_str())).
               replace('+',' '));
         string value=decoded_value.toStdString();
//         cout << "key = " << key << " value = " << value << endl;

// As of 6/29/10, we still cannot get QUrl::fromPercentEncoding() to
// successfully decode %23FF00FF as #FF00FF (magenta coloring).  So we
// resort to performing the following libCurl call:

         char* decoded_charstr=
            curl_easy_unescape(curl_ptr,value.c_str(),0,outlength_ptr);
         string decoded_str(decoded_charstr);
         value=decoded_str;

//         cout << "Decoded string = " << decoded_str << endl;
//         cout << "*outlength_ptr = " << *outlength_ptr << endl;
         curl_free(decoded_charstr);
// ----------------------------------------------------------------------

         pair<string,string> P(key,value);
         KeyValue.push_back(P);
         Key.push_back(key);
         Value.push_back(value);
      }

   n_keys=Key.size();
   for (int k=0; k<n_keys; k++)
   {
      string key=Key[k];
      string value=Value[k];
//      cout << "k = " << k 
//           << " key = " << key
//           << " value = " << value << endl;
   }
   
   delete outlength_ptr;
}

// ---------------------------------------------------------------------
QByteArray WebServer::post( const QUrl& url, const QByteArray& postData,
                            QHttpResponseHeader& responseHeader)
{
   cout << "inside WebServer::post()" << endl;

   QDomDocument doc;
   QDomElement html = doc.createElement( "html" );
   get(doc,html,url,responseHeader);

   QDomElement post = doc.createElement( "h2" );
   html.appendChild( post );
   string post_str="Post: "+string(postData.constData());
   
   post.appendChild( doc.createTextNode( QString(post_str.c_str() ) ) );
    
   return doc.toByteArray();
}

