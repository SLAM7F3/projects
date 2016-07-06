// ==========================================================================
// IMAGESERVER class file
// ==========================================================================
// Last updated on 5/6/10; 5/10/10; 7/12/10
// ==========================================================================

#include <iostream>
#include <set>
#include <vector>
#include <QtCore/QtCore>
#include "osg/osgGraphicals/AnimationController.h"
#include "Qt/web/ImageServer.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
void ImageServer::allocate_member_objects()
{
   server_ptr=new QTcpServer(this);
}		       

void ImageServer::initialize_member_objects()
{
   Server_listening_flag=iPhone_beacon_flag=false;
   image_number=sensor_ID=0;
   packet_size=0;
   Movie_ptr=NULL;
   iPhone_track_ptr=NULL;
   instream_ptr=NULL;
}

ImageServer::ImageServer(std::string host_IP, qint16 port, QObject* parent) 
   : QObject( parent )
{
   allocate_member_objects();
   initialize_member_objects();

   host_IP_address=host_IP;
   host_address=QHostAddress(host_IP_address.c_str());
   port_number=port;
//   cout << "host IP address = " << host_IP_address << endl;
   cout << "port number = " << port_number << endl;

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
ImageServer::~ImageServer()
{
   server_ptr->close();
}

// ---------------------------------------------------------------------
// These first signal/slot relationships should be established BEFORE
// any network connections are made...

void ImageServer::setup_initial_signal_slot_connections()
{
   connect( server_ptr, SIGNAL( newConnection() ), 
            this, SLOT( incomingConnection() ) );
}

// ---------------------------------------------------------------------
void ImageServer::incomingConnection()
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
// On 5/1/09, Dave Ceddia explained to us that a Server responds to a
// Client request by first establishing a bi-directional socket (which
// can be thought of as a doorway attached to the server; the address
// for this socket doorway is returned by sender() ).  Once the
// readyRead() signal is emitted by the new socket, the server reads
// the client's request in this member function readSocket().  The
// server can then respond to the request by writing information out
// to the socket for transmission back to the client.

void ImageServer::readSocket()
{
   cout << "inside ImageServer::readSocket()" << endl;
   QTcpSocket* socket_ptr = qobject_cast<QTcpSocket *>( sender() );
   if ( socket_ptr == NULL ) return;

//   cout << "socket_ptr = " << socket_ptr << endl;
   cout << "iPhone_beacon_flag = " << iPhone_beacon_flag << endl;
   if (iPhone_beacon_flag)
   {
      if (instream_ptr==NULL) instream_ptr=new QDataStream(socket_ptr);
      extract_iPhone_metadata(socket_ptr);
      return;
   }

// FAKE FAKE:  Thurs, May 6, 2010

// Comment out PLAY check needed for RTPS

// Add following line for bots n dogs alg develop

//   if (Movie_ptr->get_AnimationController_ptr()->getState() != 
//       AnimationController::PLAY) return;

   Movie_ptr->import_next_to_latest_photo();

// Is there a full line to read?

   if ( !socket_ptr->canReadLine() ) return;
    
   QString command = socket_ptr->readLine();
//   cout << "command = " << command.toStdString() << endl;
   if (!command.startsWith("GET_NEXT_IMAGE")) return;

// ----------------------------------------------------------------------
// This next section emulates Bots n Dogs imagery chips being sent
// from a netbook to a ground analysis computer.  It dumps 1000 pixel
// x 1000 pixel chips into image_data_array:

   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      command.toStdString());

//   cout << "Movie filename = " << Movie_ptr->get_video_filename() << endl;
   string curr_image_filename=
      "sensor"+stringfunc::number_to_string(sensor_ID)+"_"+
      Movie_ptr->get_video_filename();
   cout << "Current image filename = " << curr_image_filename << endl;

// Convert image contents into output QByteArray:
   
   texture_rectangle* texture_rectangle_ptr=Movie_ptr->
      get_texture_rectangle_ptr();
   int image_number=texture_rectangle_ptr->get_imagenumber();
   twovector lower_left_corner_fracs(0,0);
   twovector upper_right_corner_fracs(1,1);

   string output_image_suffix="jpg";
   bool draw_central_bbox_flag=false;
   int n_horiz_output_pixels=-1;  // original image size maintained
//   int n_horiz_output_pixels=500;
//   int n_horiz_output_pixels=1000;
   texture_rectangle_ptr->retrieve_curr_subframe_byte_data(
      lower_left_corner_fracs,upper_right_corner_fracs,output_image_suffix,
      draw_central_bbox_flag,n_horiz_output_pixels);
   string output_image_str=texture_rectangle_ptr->get_output_image_string();
//   cout << "output_image_str.size() = " << output_image_str.size() << endl;

// ----------------------------------------------------------------------
/*
// This next section emulates real CH/Massivs imagery chips being sent
// from Alex' communication computer to Peter's analysis computer.  It
// should be replaced by some other code which dumps 1000 pixel x 1000
// pixel chips into image_data_array:

   vector<string> substrings=stringfunc::decompose_string_into_substrings(
      command.toStdString());
   int chip_ID=0;
   if (substrings.size() > 1)
   {
      chip_ID=stringfunc::string_to_number(substrings[1]);
   }
   cout << "chip_ID = " << chip_ID << endl;

// Convert image contents into output QByteArray:
   
   texture_rectangle* texture_rectangle_ptr=Movie_ptr->
      get_texture_rectangle_ptr();
   int image_number=texture_rectangle_ptr->get_imagenumber();
   cout << "image_number = " << image_number << endl;

   twovector lower_left_corner_fracs(0.629,0.629);
   twovector upper_right_corner_fracs(1,1);
   if (chip_ID > 0)
   {
      lower_left_corner_fracs=twovector(0,0.629);
      upper_right_corner_fracs=twovector(0.371,1.0);
   }

   string output_image_suffix="jpg";
   bool draw_central_bbox_flag=false;
//   int n_horiz_output_pixels=500;
   int n_horiz_output_pixels=1000;
   texture_rectangle_ptr->retrieve_curr_subframe_byte_data(
      lower_left_corner_fracs,upper_right_corner_fracs,output_image_suffix,
      draw_central_bbox_flag,n_horiz_output_pixels);
   string output_image_str=texture_rectangle_ptr->get_output_image_string();
//   cout << "output_image_str.size() = " << output_image_str.size() << endl;
*/
// ----------------------------------------------------------------------

   QByteArray image_data_array(
      output_image_str.c_str(),output_image_str.size());
   int image_size=image_data_array.size();
//   cout << "image_data_array.size() = " << image_size << endl;

// Write out packet to socket:

   QByteArray packet;
   QDataStream outstream(&packet,QIODevice::WriteOnly);
   outstream.setVersion( QDataStream::Qt_4_1 );
        
// Magic number indicates start of packet:

   outstream << quint32(0xABCD);

// Packet length placeholder

   outstream << quint32(0);
   
// Header values for iPhone camera pictures:

   quint32     imagenumber(image_number++);
   QString     filename(curr_image_filename.c_str());
//   QString     filename("phony_image_filename.jpg");
   qreal       longitude = -82.872978+image_number;
   qreal       latitude = 24.628477+image_number;

   outstream << imagenumber 
             << sensor_ID
             << filename 
             << longitude
             << latitude;

   cout << "imagenumber = " << imagenumber 
        << " sensor_ID = " << sensor_ID << endl;
   cout << "filename = " << filename.toStdString() << endl;
   cout << "longitude = " << longitude 
        << " latitude = " << latitude 
        << endl;

/*
// Possible header values for Constant Hawk/Massivs 1000 pixel x 1000
// pixel image chips:

   outstream << framenumber
	     << lower_left_corner_longitude
	     << lower_left_corner_latitude
	     << lower_right_corner_longitude
	     << lower_right_corner_latitude
	     << upper_right_corner_longitude
	     << upper_right_corner_latitude
	     << upper_left_corner_longitude
	     << upper_left_corner_latitude
             << endl;
*/

// Image length and data (note that this will write 4-bytes length and
// then data)

/*
   QFile imageFile( 
      "/home/cho/programs/c++/svn/projects/src/mains/aerialEO/Qt/lower_left_corner_1000.jpg");
   imageFile.open( QIODevice::ReadOnly );
   outstream << imageFile.readAll();        
*/

   outstream << image_data_array;

// Seek to beginning of Datastream.  Then overwrite second 4 bytes
// with genuine packet reduced size (which does not count the first 4
// bytes containing the magic number nor the second 4 bytes containing
// the packet length):

   outstream.device()->seek(4);
   quint32 reduced_packet_size=packet.size()-2*sizeof(quint32);

   cout << "--------------------------------------------------" << endl;
   outstream << reduced_packet_size;

   socket_ptr->write(packet);
}

// ==========================================================================
// iPhone member functions
// ==========================================================================

int ImageServer::get_iPhone_ID() const
{
   return int(iPhone_ID);
}

int ImageServer::get_iPhone_imagenumber() const
{
   return int(iPhone_imagenumber);
}

double ImageServer::get_iPhone_longitude() const
{
   return double(iPhone_longitude);
}

double ImageServer::get_iPhone_latitude() const
{
   return double(iPhone_latitude);
}

double ImageServer::get_iPhone_altitude() const
{
   return double(iPhone_altitude);
}

double ImageServer::get_iPhone_horizontal_uncertainty() const
{
   return double(iPhone_horizontal_uncertainty);
}

double ImageServer::get_iPhone_vertical_uncertainty() const
{
   return double(iPhone_vertical_uncertainty);
}

double ImageServer::get_iPhone_elapsed_secs() const
{
   return double(iPhone_elapsed_secs);
}

double ImageServer::get_iPhone_direction() const
{
   return iPhone_direction;
}

double ImageServer::get_iPhone_speed() const
{
   return iPhone_speed;
}

double ImageServer::get_iPhone_yaw() const
{
   return iPhone_yaw;
}

double ImageServer::get_iPhone_roll() const
{
   return iPhone_roll;
}

double ImageServer::get_iPhone_pitch() const
{
   return iPhone_pitch;
}

double ImageServer::get_iPhone_gravx() const
{
   return iPhone_gravx;
}

double ImageServer::get_iPhone_gravy() const
{
   return iPhone_gravy;
}

double ImageServer::get_iPhone_gravz() const
{
   return iPhone_gravz;
}

// ---------------------------------------------------------------------
// Member function extract_iPhone_metadata() pulls out iPhone ID,
// imagenumber, geolocation and time from the latest received photo.

void ImageServer::extract_iPhone_metadata(QTcpSocket* socket_ptr)
{
   cout << "inside ImageServer::extract_iPhone_metadata()" << endl;

   if (packet_size==0)
   {

// Search for magic number/packet length pair      

      while (true) 
      {
         if (socket_ptr->bytesAvailable() < 2*sizeof(quint32))
         {
            return;
         }
         
         uint32_t magic_number;
         *instream_ptr >> magic_number;
//         cout << "magic_number = " << magic_number << endl;

// If we fail to find the magic number, keep looking:

         if ( magic_number != 0xABCD ) continue;
	         // Note: 0xABCD (hex) = 43981 (integer)
        
         *instream_ptr >> packet_size;
         break;
      }
   }
//   cout << "packet_size = " << packet_size << endl;

   if (socket_ptr->bytesAvailable() < packet_size) 
   {
      return;
   }

// Read remainder of metadata packet:

   *instream_ptr >> iPhone_ID
                 >> iPhone_imagenumber 

                 >> iPhone_latitude
                 >> iPhone_longitude
                 >> iPhone_altitude
                 >> iPhone_horizontal_uncertainty
                 >> iPhone_vertical_uncertainty
                 >> iPhone_elapsed_secs

                 >> iPhone_direction
                 >> iPhone_speed

                 >> iPhone_yaw
                 >> iPhone_pitch
                 >> iPhone_roll

                 >> iPhone_gravx
                 >> iPhone_gravy
                 >> iPhone_gravz;

/*
   *instream_ptr >> iPhone_ID
                 >> iPhone_imagenumber 
                 >> iPhone_latitude
                 >> iPhone_longitude
                 >> iPhone_altitude
                 >> iPhone_horizontal_uncertainty
                 >> iPhone_vertical_uncertainty
                 >> iPhone_elapsed_secs;
*/

   QDateTime mytime;
   mytime.setTime_t(uint(iPhone_elapsed_secs));
   QString curr_time=mytime.toString();

   cout << "ID = " << get_iPhone_ID() << endl;
   cout << "imagenumber = " << get_iPhone_imagenumber() << endl;
   cout << "elapsed_secs = " << get_iPhone_elapsed_secs() << endl;
   cout << "longitude = " << get_iPhone_longitude()
        << " latitude = " << get_iPhone_latitude() 
        << " altitude = " << get_iPhone_altitude() << endl;
   cout << "horiz uncertainty = " << get_iPhone_horizontal_uncertainty()
        << " vert uncertainty = " << get_iPhone_vertical_uncertainty() 
        << endl;

   cout << "direction = " << get_iPhone_direction()
        << " speed = " << get_iPhone_speed() << endl;
   cout << " yaw = " << get_iPhone_yaw()
        << " roll = " << get_iPhone_roll()
        << " pitch = " << get_iPhone_pitch() << endl;
   cout << "gravx = " << get_iPhone_gravx()
        << " gravy = " << get_iPhone_gravy()
        << " gravz = " << get_iPhone_gravz() << endl;
   cout << endl;

//   rebroadcast_iPhone_metadata();
   new_rebroadcast_iPhone_metadata();

/*
   cout << "curr_time = " << curr_time.toStdString() << endl;
*/

   packet_size=0;

// Process additional frames of data:

   if ( socket_ptr->bytesAvailable() ) readSocket();

}

// ==========================================================================
// ActiveMQ message handling member functions
// ==========================================================================

void ImageServer::pushback_Messenger_ptr(Messenger* M_ptr)
{
   Messenger_ptrs.push_back(M_ptr);
}

Messenger* ImageServer::get_Messenger_ptr()
{
   return get_Messenger_ptr(Messenger_ptrs.size()-1);
}

const Messenger* ImageServer::get_Messenger_ptr() const
{
   return get_Messenger_ptr(Messenger_ptrs.size()-1);
}

int ImageServer::get_n_Messenger_ptrs() const
{
   return Messenger_ptrs.size();
}

Messenger* ImageServer::get_Messenger_ptr(int i)
{
   if (i >= 0 && i < Messenger_ptrs.size())
   {
      return Messenger_ptrs[i];
   }
   else
   {
      return NULL;
   }
}

const Messenger* ImageServer::get_Messenger_ptr(int i) const
{
   if (i >= 0 && i < Messenger_ptrs.size())
   {
      return Messenger_ptrs[i];
   }
   else
   {
      return NULL;
   }
}

// ---------------------------------------------------------------------
// Member function rebroadcast_iPhone_metadata() publishes iPhone
// timing and position information to the ActiveMQ messenger of the
// ImageServer class.

// On 6/4/09, Ross recommended that this ImageServer be responsible
// for broadcasting iPhone metadata at regular, periodic intervals.
// Input from the iPhone may very well not come in smoothly over time.
// So the ImageServer should perform some buffering and transmit an
// indication of data staleness within the output ActiveMQ messages.

void ImageServer::rebroadcast_iPhone_metadata()
{
//   cout << "inside ImageServer::rebroadcast_iPhone_metadata()" << endl;
   
   string command="UPDATE_IPHONE_METADATA";
   string key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   key="iPhone_ID";
   value=stringfunc::number_to_string(get_iPhone_ID());
   properties.push_back(property(key,value));

   key="imagenumber";
   value=stringfunc::number_to_string(get_iPhone_imagenumber());
   properties.push_back(property(key,value));

   key="elapsed_secs";
   value=stringfunc::number_to_string(get_iPhone_elapsed_secs());
   properties.push_back(property(key,value));

   key="longitude";
   value=stringfunc::number_to_string(get_iPhone_longitude());
   properties.push_back(property(key,value));

   key="latitude";
   value=stringfunc::number_to_string(get_iPhone_latitude());
   properties.push_back(property(key,value));

   key="altitude";
   value=stringfunc::number_to_string(get_iPhone_altitude());
   properties.push_back(property(key,value));

   key="horiz_uncertainty";
   value=stringfunc::number_to_string(get_iPhone_horizontal_uncertainty());
   properties.push_back(property(key,value));

   key="vert_uncertainty";
   value=stringfunc::number_to_string(get_iPhone_vertical_uncertainty());
   properties.push_back(property(key,value));

   get_Messenger_ptr()->sendTextMessage(command,properties);
}




void ImageServer::new_rebroadcast_iPhone_metadata()
{
   cout << "inside ImageServer::new_rebroadcast_iPhone_metadata()" << endl;

   if (iPhone_track_ptr==NULL) return;

   double curr_time=get_iPhone_elapsed_secs();

// As of May 2010, Jennifer Drexler's blue force tracker thin client
// expects to receive longitude,latitude rather than easting,northing
// geocoordinates:  

   threevector curr_lla_posn(
      get_iPhone_longitude(),
      get_iPhone_latitude(),get_iPhone_altitude());
   threevector velocity=Zero_vector;

   cout << "curr_time = " << curr_time
        << " curr_lla = " << curr_lla_posn << endl;

   iPhone_track_ptr->set_posn_velocity(curr_time,curr_lla_posn,velocity);
   iPhone_track_ptr->broadcast_statevector(curr_time,get_Messenger_ptr());
}
