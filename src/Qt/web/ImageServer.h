// ========================================================================
// IMAGESERVER header file
// ========================================================================
// Last updated on 5/6/10; 5/10/10; 7/12/10
// ========================================================================

#ifndef IMAGESERVER_H
#define IMAGESERVER_H

#include <string>
#include <QtNetwork/QtNetwork>
#include <QtXml/QtXml>
#include "messenger/Messenger.h"
#include "osg/osg2D/Movie.h"
#include "track/track.h"

class ImageServer : public QObject
{
   Q_OBJECT
    
      public:

// Note: Avoid using port 8080 which is the default for TOMCAT.

   ImageServer(std::string host_IP, qint16 port, QObject* parent = NULL );
   ~ImageServer();

// Set & get member functions:

   void set_sensor_ID(int ID);
   void set_iPhone_beacon_flag(bool flag);
   void set_Movie_ptr(Movie* M_ptr);
   void set_iPhone_track_ptr(track* t_ptr);

// iPhone member functions:

   int get_iPhone_ID() const;
   int get_iPhone_imagenumber() const;
   double get_iPhone_longitude() const;
   double get_iPhone_latitude() const;
   double get_iPhone_altitude() const;
   double get_iPhone_horizontal_uncertainty() const;
   double get_iPhone_vertical_uncertainty() const;
   double get_iPhone_elapsed_secs() const;

   double get_iPhone_direction() const;
   double get_iPhone_speed() const;
   double get_iPhone_yaw() const;
   double get_iPhone_roll() const;
   double get_iPhone_pitch() const;

   double get_iPhone_gravx() const;
   double get_iPhone_gravy() const;
   double get_iPhone_gravz() const;

// ActiveMQ message handling member functions:

   void pushback_Messenger_ptr(Messenger* M_ptr);
   Messenger* get_Messenger_ptr();
   const Messenger* get_Messenger_ptr() const;
   int get_n_Messenger_ptrs() const;
   Messenger* get_Messenger_ptr(int i);
   const Messenger* get_Messenger_ptr(int i) const;

   void rebroadcast_iPhone_metadata();
   void new_rebroadcast_iPhone_metadata();

  protected:

   private slots:
        
   void incomingConnection();
   void readSocket();
        
  private:
        
   bool Server_listening_flag,iPhone_beacon_flag;
   uint32_t packet_size;
   int port_number,image_number,sensor_ID;
   std::string host_IP_address;
   QTcpServer* server_ptr;
   QHostAddress host_address;
   QDataStream* instream_ptr;
   Movie* Movie_ptr;
   track* iPhone_track_ptr;

// iPhone parameters:

   uint32_t iPhone_ID,iPhone_imagenumber;
   double iPhone_longitude,iPhone_latitude,iPhone_altitude;
   double iPhone_horizontal_uncertainty,iPhone_vertical_uncertainty;
   double iPhone_elapsed_secs;

   double iPhone_direction,iPhone_speed;
   double iPhone_yaw,iPhone_pitch,iPhone_roll;
   double iPhone_gravx,iPhone_gravy,iPhone_gravz;

   std::vector<Messenger*> Messenger_ptrs;

   void allocate_member_objects();
   void initialize_member_objects();

   void setup_initial_signal_slot_connections();
   void extract_iPhone_metadata(QTcpSocket* socket_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void ImageServer::set_sensor_ID(int ID)
{
   sensor_ID=ID;
}

inline void ImageServer::set_iPhone_beacon_flag(bool flag)
{
   iPhone_beacon_flag=flag;
}

inline void ImageServer::set_Movie_ptr(Movie* M_ptr)
{
   Movie_ptr=M_ptr;
}

inline void ImageServer::set_iPhone_track_ptr(track* t_ptr)
{
   iPhone_track_ptr=t_ptr;
}

#endif // IMAGESERVER_H
