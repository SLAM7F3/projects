// ==========================================================================
// FLIRCLIENT class file
// ==========================================================================
// Last updated on 10/20/11; 10/24/11; 10/26/11
// ==========================================================================

#include <iostream>
#include <sstream>
// #include <strstream>
#include <vector>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include <QtXml/QtXml>
#include <QtNetwork/QtNetwork>

#include "Qt/web/FLIRClient.h"
#include "math/constants.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::istrstream;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void FLIRClient::allocate_member_objects()
{
   socket_ptr = new QTcpSocket(this);
   instream_ptr=new QDataStream(socket_ptr);
}		       

void FLIRClient::initialize_member_objects()
{
   instream_ptr->setVersion(QDataStream::Qt_4_1);
   packet_size=0;
}

FLIRClient::FLIRClient(
   string Server_hostname,int Server_portnumber,QObject* parent)
   : QObject( parent )
{
//   cout << "inside FLIRClient constructor()" << endl;
   allocate_member_objects();
   initialize_member_objects();

   QHostAddress host_address(Server_hostname.c_str());
   cout << "host address = " << Server_hostname.c_str() << endl;
   cout << "port number = " << Server_portnumber << endl;

   connect( socket_ptr, SIGNAL( error( QAbstractSocket::SocketError ) ), 
            this, SLOT( slotError( QAbstractSocket::SocketError ) ) );

   connect( socket_ptr, SIGNAL( hostFound() ), 
            this, SLOT( slotHostFound() ) );

   cout << "Before call to slotconnected" << endl;
   connect( socket_ptr, SIGNAL( connected() ), 
            this, SLOT( slotConnected() ) );

   cout << "Before call to readsocket" << endl;
   connect( socket_ptr, SIGNAL(readyRead()), this, SLOT(readSocket()) );

   qDebug() << "Connecting to host...";
   socket_ptr->connectToHost(host_address,Server_portnumber);
   cout << "Before waitForConnected()" << endl;
   socket_ptr->waitForConnected(-1);
}

// ---------------------------------------------------------------------
FLIRClient::~FLIRClient()
{
}

// ---------------------------------------------------------------------
void FLIRClient::slotError( QAbstractSocket::SocketError err )
{
   cout << "inside FLIRClient::slotError()" << endl;
   qDebug() << "Error:" << err << socket_ptr->errorString();
}

// ---------------------------------------------------------------------
void FLIRClient::slotHostFound()
{
   cout << "inside FLIRClient::slotHostFound()" << endl;
//   qDebug("Host found");
}

// ---------------------------------------------------------------------
void FLIRClient::slotConnected()
{
   cout << "inside FLIRClient::slotConnected()" << endl;
}

// ---------------------------------------------------------------------
void FLIRClient::readSocket()
{
//   cout << "inside FLIRClient::readSocket()" << endl;
//   cout << "packet_size = " << packet_size << endl;

   if (packet_size==0)
   {

// First search for magic number & packet size pair:

      while (true) 
      {
         if (socket_ptr->bytesAvailable() < 2*sizeof(quint32))
         {
            process_Qt_events(8);
         }
         
         quint32 magic_number;
         *instream_ptr >> magic_number;
         cout << "magic_number = " << magic_number << endl;
            
// If we fail to find the magic number, keep looking:
// Note: 0x4A4B4C4D = 1246448717 

         if ( magic_number != 0x4A4B4C4D ) continue;

         *instream_ptr >> packet_size;
         break;
      }
   }
   cout << "packet_size = " << packet_size << endl;

// Note: On 10/26/2011, Jon Johnson realized that Qt 4.6 and higher
// automatically expects the number of bytes to be preprended as a
// Quint 32 at the start of any stream which is translated into a
// QBytearray!  So if the server does not send the packet size as the
// first 4 bytes that are read into a QBytearray, this client will not
// be able to parse it...

// Read in packet_size worth of bytes from socket:

   if (socket_ptr->bytesAvailable() < packet_size) 
   {
      process_Qt_events(packet_size);
   }

// Parse payload bytes:

   QByteArray byteData;
   *instream_ptr >> byteData;
//   cout << "byteData.size() = " << byteData.size() << endl;

   string message_string;
   for (int c=0; c<byteData.size(); c++)
   {
//      cout << "c = " << c << " byteData[c] = " << byteData.at(c) << endl;
      message_string += byteData.at(c);
   }
   decompose_message_string(message_string);

   packet_size=0;

// If we are falling behind by 0.5 MB, skip ahead:

//   cout << "socket_ptr->bytesAvailable() = "
//        << socket_ptr->bytesAvailable() << endl;
    if ( socket_ptr->bytesAvailable() > 512*1024 ) 
    {
       qDebug() << "Dropping " << socket_ptr->bytesAvailable() << " bytes";
       instream_ptr->skipRawData( socket_ptr->bytesAvailable() );
    }

// Process additional frames of data:

   if ( socket_ptr->bytesAvailable() ) readSocket();
}

// ---------------------------------------------------------------------
void FLIRClient::process_Qt_events(int n_bytes_to_read)
{
//   cout << "inside FLIRClient::process_Qt_events(), n_bytes_to_read = "
//        << n_bytes_to_read << endl;
   
   while (socket_ptr->bytesAvailable() < n_bytes_to_read)
   {
      qApp->processEvents();
   }
}

// ---------------------------------------------------------------------
// Method decompose_message_string() takes in a string which is
// assumed to containg comma-separated text substrings.

void FLIRClient::decompose_message_string(string message)
{
   cout << "message_string = " << message << endl;
   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      message,",");

   double azimuth,elevation,FOV,focus_posn,latitude,longitude,altitude;
   double heading,pitch,roll;
   double tgt_latitude,tgt_longitude,tgt_altitude,tgt_slant_range;
   double tgt_GPS_source,tgt_GPS_week_secs,tgt_GPS_week;
   double GPS_week_secs,GPS_week;

   for (int i=0; i<substrings.size(); i++)
   {
      cout << "i = " << i << " substrings[i] = " << substrings[i]
           << endl;
      
      bool is_number_flag=stringfunc::is_number(substrings[i]);
      double curr_value=NEGATIVEINFINITY;
      if (is_number_flag)
      {
         curr_value=stringfunc::string_to_number(substrings[i]);
      }

      if (i==0)
      {
         azimuth=curr_value;
      }
      else if (i==1)
      {
         elevation=curr_value;
      }
      else if (i==2)
      {
         FOV=curr_value;
      }
      else if (i==3)
      {
         focus_posn=curr_value;
      }
      else if (i==4)
      {
         latitude=curr_value;
      }
      else if (i==5)
      {
         longitude=curr_value;
      }
      else if (i==6)
      {
         altitude=curr_value;
      }
      else if (i==7)
      {
         heading=curr_value;
      }
      else if (i==8)
      {
         pitch=curr_value;
      }
      else if (i==9)
      {
         roll=curr_value;
      }
      else if (i==10)
      {
         tgt_latitude=curr_value;
      }
      else if (i==11)
      {
         tgt_longitude=curr_value;
      }
      else if (i==12)
      {
         tgt_altitude=curr_value;
      }
      else if (i==13)
      {
         tgt_slant_range=curr_value;
      }
      else if (i==14)
      {
         tgt_GPS_source=curr_value;
      }
      else if (i==15)
      {
         tgt_GPS_week_secs=curr_value;
      }
      else if (i==16)
      {
         tgt_GPS_week=curr_value;
      }
      else if (i==17)
      {
         GPS_week_secs=curr_value;
      }
      else if (i==18)
      {
         GPS_week=curr_value;
      }

   } // loop over index i labeling substrings

   cout << "az = " << azimuth << " el = " << elevation << " FOV = " << FOV
        << endl;
   cout << "focus_posn = " << focus_posn << " lat = " << latitude
        << " lon = " << longitude << " alt = " << altitude << endl;
   cout << "heading = " << heading << " pitch = " << pitch << " roll = "
        << roll << endl;
   cout << "tgt lat = " << tgt_latitude << " tgt lon = " << tgt_longitude
        << " tgt_alt = " << tgt_altitude << " tgt slant range = "
        << tgt_slant_range << endl;
   cout << "tgt_GPS_source = " << tgt_GPS_source << " tgt_GPS_week_secs = "
        << tgt_GPS_week_secs << " tgt_GPS_week = " << tgt_GPS_week << endl;
   cout << "GPS_week_secs = " << GPS_week_secs << " GPS_week = "
        << GPS_week << endl;
}
