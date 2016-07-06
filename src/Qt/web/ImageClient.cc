// ==========================================================================
// IMAGECLIENT class file
// ==========================================================================
// Last updated on 6/4/09; 5/6/10; 5/10/10
// ==========================================================================

#include <iostream>
#include <sstream>
// #include <strstream>

// Include qapplication in order to access qApp macro which obviates
// having to pass app pointer from main Qt program...

#include <Qt/qapplication.h>

#include <QtXml/QtXml>
#include <QtNetwork/QtNetwork>
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "Qt/web/ImageClient.h"

using std::cout;
using std::endl;
using std::istrstream;
using std::ofstream;
using std::string;

// ---------------------------------------------------------------------
void ImageClient::allocate_member_objects()
{
   socket_ptr = new QTcpSocket(this);
   instream_ptr=new QDataStream(socket_ptr);
}		       

void ImageClient::initialize_member_objects()
{
   iPhone_beacon_flag=false;
   save_images_to_JPEG_files_flag=false;
   instream_ptr->setVersion(QDataStream::Qt_4_1);
   packet_size=0;
//   set_ready_for_next_image_flag(false);
   Movie_ptr=NULL;

   image_number=0;
   double fps=1;
//   double fps=2;
//   double fps=10;
   set_output_image_subdir("/tmp/recovered_images/");
   
   requestTimer.setInterval( 1000. * ( 1. / fps ) );
   requestTimer.start();
}

ImageClient::ImageClient(string Server_URL,int id,bool iPhone_beacon_flag,
                         QObject* parent)
   : QObject( parent )
{
//   cout << "inside ImageClient constructor()" << endl;
   allocate_member_objects();
   initialize_member_objects();

   ID=id;
   string host_IP=stringfunc::get_hostname_from_URL(Server_URL);
   int Server_portnumber=stringfunc::get_portnumber_from_URL(Server_URL);

   QHostAddress host_address(host_IP.c_str());
//   cout << "host IP = " << host_IP << endl;
//   cout << "port number = " << Server_portnumber << endl;

   connect( socket_ptr, SIGNAL( error( QAbstractSocket::SocketError ) ), 
            this, SLOT( slotError( QAbstractSocket::SocketError ) ) );

   connect( socket_ptr, SIGNAL( hostFound() ), 
            this, SLOT( slotHostFound() ) );

   connect( socket_ptr, SIGNAL( connected() ), 
            this, SLOT( slotConnected() ) );

   connect( socket_ptr, SIGNAL(readyRead()), this, SLOT(readSocket()) );

   if (iPhone_beacon_flag)
   {
      connect( &requestTimer, SIGNAL(timeout()), 
               this, SLOT(broadcast_metadata()) );
   }
   else
   {
      connect( &requestTimer, SIGNAL(timeout()), 
               this, SLOT(request_next_image()) );
   }

   qDebug() << "Connecting to host...";
   socket_ptr->connectToHost(host_address,Server_portnumber);
   socket_ptr->waitForConnected(-1);
}

// ---------------------------------------------------------------------
ImageClient::~ImageClient()
{
}

// ---------------------------------------------------------------------
void ImageClient::slotError( QAbstractSocket::SocketError err )
{
   cout << "inside ImageClient::slotError()" << endl;
   qDebug() << "Error:" << err << socket_ptr->errorString();
}

// ---------------------------------------------------------------------
void ImageClient::slotHostFound()
{
   cout << "inside ImageClient::slotHostFound()" << endl;
//   qDebug("Host found");
}

// ---------------------------------------------------------------------
void ImageClient::slotConnected()
{
   cout << "inside ImageClient::slotConnected()" << endl;
   set_ready_for_next_image_flag(true);
}

// ---------------------------------------------------------------------
void ImageClient::set_output_image_subdir(string subdir)
{
   output_image_subdir=subdir;
   filefunc::dircreate(output_image_subdir);
}

// ---------------------------------------------------------------------
void ImageClient::request_next_image()
{
//   cout << "inside ImageClient::request_next_image()" << endl;
   GET_command="GET_NEXT_IMAGE_FOR_REGION "+stringfunc::number_to_string(ID)
      +" \n";
//   if (get_ready_for_next_image_flag())
   {
      socket_ptr->write( GET_command.c_str() );
//      set_ready_for_next_image_flag(false);
   }
}

// ---------------------------------------------------------------------
void ImageClient::readSocket()
{
//   cout << "inside ImageClient::readSocket()";

   if (packet_size==0)
   {

// Search for magic number/packet length pair      

      while (true) 
      {
         if (socket_ptr->bytesAvailable() < 2*sizeof(quint32))
         {
            process_Qt_events(8);
//            return;
         }
//         set_ready_for_next_image_flag(false);
         
         quint32 magic_number;
         *instream_ptr >> magic_number;
            
// If we fail to find the magic number, keep looking:

         if ( magic_number != 0xABCD ) continue;
        
         *instream_ptr >> packet_size;
         break;
      }
   }
   cout << "packet_size = " << packet_size << endl;

   if (socket_ptr->bytesAvailable() < packet_size) 
   {
      process_Qt_events(packet_size);
//      return;
   }

// Read remainder of header:

   quint32     image_number,sensor_ID;
   QString     filename;
   qreal       longitude,latitude;
    
   *instream_ptr >> image_number 
                 >> sensor_ID
                 >> filename 
                 >> longitude
                 >> latitude;

   cout << "image_number = " << image_number 
        << "sensor_ID = " << sensor_ID << endl;
   cout << "filename = " << filename.toStdString() << endl;
   string suffix=QFileInfo(filename).suffix().toStdString();
   cout << "suffix = " << suffix << endl;
   cout << " longitude = " << longitude 
        << "latitude = " << latitude 
        << endl;

// Read payload:

   QByteArray imageData;
   *instream_ptr >> imageData;
//   cout << "imageData.size() = " << imageData.size() << endl;

   texture_rectangle* texture_rectangle_ptr=Movie_ptr->
      get_texture_rectangle_ptr();
//   cout << "Movie_ptr->getWidth() = " << Movie_ptr->getWidth()
//        << " Movie_ptr->getHeight() = " << Movie_ptr->getHeight() << endl;
   
   texture_rectangle_ptr->read_image_from_char_buffer(
      suffix,imageData.constData(),imageData.size());
   texture_rectangle_ptr->set_TextureRectangle_image();
//   Movie_ptr->reset_texture_coords();

// Save current image to output JPEG file:

   string curr_filename=filename.toStdString();
   if (save_images_to_JPEG_files_flag &&
       curr_filename != last_saved_filename)
   {
      string output_filename=output_image_subdir+curr_filename;
      texture_rectangle_ptr->write_curr_frame(output_filename);
      last_saved_filename=curr_filename;
   }
   
   packet_size=0;

// If we are falling behind by 0.5 MB, skip ahead:

//   cout << "socket_ptr->bytesAvailable() = "
//        << socket_ptr->bytesAvailable() << endl;
    if ( socket_ptr->bytesAvailable() > 512*1024 ) 
    {
       qDebug() << "Dropping " << socket_ptr->bytesAvailable() << " bytes";
       instream_ptr->skipRawData( socket_ptr->bytesAvailable() );
    }

//    set_ready_for_next_image_flag(true);

// Process additional frames of data:

   if ( socket_ptr->bytesAvailable() ) readSocket();
}

// ---------------------------------------------------------------------
void ImageClient::process_Qt_events(int n_bytes_to_read)
{
   while (socket_ptr->bytesAvailable() < n_bytes_to_read)
   {
      qApp->processEvents();
   }
}

// ---------------------------------------------------------------------
// Member function broadcast_metadata() emulates an iPhone client
// (running on an actual iPhone) which periodically broadcasts its GPS
// location.

void ImageClient::broadcast_metadata()
{
//   cout << "inside ImageClient::broadcast_metadata()" << endl;

// Write out packet to socket:

   QByteArray packet;
   QDataStream outstream(&packet,QIODevice::WriteOnly);
   outstream.setVersion( QDataStream::Qt_4_1 );
        
// Magic number indicates start of packet:

   outstream << quint32(0xABCD);

// Packet length placeholder

   outstream << quint32(0);
   
// Packet values

   quint32 iPhone_ID=10000;
   quint32 imagenumber(image_number++);
   qreal longitude = -88.1+0.001*image_number;
   qreal latitude = 43+0*image_number;
   qreal altitude = 10+0*image_number;
   qreal horizontal_uncertainty=20+image_number;
   qreal vertical_uncertainty=30+image_number;

// Simulate iPhone transmitting current time as seconds elapsed since
// some canonical reference time = Midnight on 1 Jan 1970:

   clock.current_local_time_and_UTC();
   qreal elapsed_secs=clock.secs_elapsed_since_reference_date();
//   cout << "elapsed_secs = " << elapsed_secs << endl;

   outstream << iPhone_ID
             << imagenumber 
             << latitude 
             << longitude
             << altitude
             << horizontal_uncertainty
             << vertical_uncertainty
             << elapsed_secs;

//   cout << "iPhone_ID = " << iPhone_ID 
//        << " imagenumber = " << imagenumber << endl;
//   cout << "longitude = " << longitude 
//        << " latitude = " << latitude 
//        << " altitude = " << altitude << endl;
//   cout << "horiz_uncertainty = " << horizontal_uncertainty
//        << " vertical_uncertainty = " << vertical_uncertainty << endl;
//   cout << "elased_secs = " << elapsed_secs << endl;

//   QDateTime mytime;
//   mytime.setTime_t(uint(elapsed_secs));
//   QString curr_time=mytime.toString();
//   cout << "curr_time = " << curr_time.toStdString() << endl;

// Seek to beginning of Datastream.  Then overwrite second 4 bytes
// with genuine packet reduced size (which does not count the first 4
// bytes containing the magic number nor the second 4 bytes containing
// the packet length):

   outstream.device()->seek(4);
   quint32 reduced_packet_size=packet.size()-2*sizeof(quint32);

//   cout << "reduced_packet_size = " << reduced_packet_size << endl;
//   outstream << reduced_packet_size;

   socket_ptr->write( packet );
}
