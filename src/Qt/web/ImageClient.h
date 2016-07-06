// ========================================================================
// IMAGECLIENT header file
// ========================================================================
// Last updated on 5/19/09; 6/3/09; 5/6/10
// ========================================================================

#ifndef IMAGECLIENT_H
#define IMAGECLIENT_H

#include <string>
#include <QtNetwork/QtNetwork>
#include <QtCore/QSignalMapper>
#include "astro_geo/Clock.h"
#include "osg/osg2D/Movie.h" 

class ImageClient : public QObject
{
   Q_OBJECT

      public:

   ImageClient(std::string Server_URL,int id=0,
               bool iPhone_beacon_flag=false,QObject* parent=NULL);
   ~ImageClient();

// Set & get member functions

   void set_iPhone_beacon_flag(bool flag);
   void set_save_images_to_JPEG_files_flag(bool flag);
   void set_ready_for_next_image_flag(bool flag);
   bool get_ready_for_next_image_flag() const;
   
   void set_GET_command(std::string cmd);
   void set_Movie_ptr(Movie* M_ptr);
   void set_output_image_subdir(std::string subdir);

  protected:

   private slots:

   void slotError( QAbstractSocket::SocketError );
   void slotHostFound();
   void slotConnected();
   void request_next_image();
   void readSocket();

   void broadcast_metadata();

  private:

   bool iPhone_beacon_flag,ready_for_next_image_flag;
   bool save_images_to_JPEG_files_flag;
   quint32 packet_size;	// packet header data
   int ID,image_size,image_number;
   std::string GET_command,input_image_suffix,output_image_subdir;
   std::string last_saved_filename;
   Clock clock;

   QTimer requestTimer;
   QTcpSocket* socket_ptr;
   QDataStream* instream_ptr;
   Movie* Movie_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   void process_Qt_events(int n_iters);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void ImageClient::set_iPhone_beacon_flag(bool flag)
{
   iPhone_beacon_flag=flag;
}

inline void ImageClient::set_save_images_to_JPEG_files_flag(bool flag)
{
   save_images_to_JPEG_files_flag=flag;
}

inline void ImageClient::set_ready_for_next_image_flag(bool flag)
{
   ready_for_next_image_flag=flag;
}

inline bool ImageClient::get_ready_for_next_image_flag() const
{
   return ready_for_next_image_flag;
}

inline void ImageClient::set_GET_command(std::string cmd)
{
   GET_command=cmd;
}

inline void ImageClient::set_Movie_ptr(Movie* M_ptr)
{
   Movie_ptr=M_ptr;
}


#endif // IMAGECLIENT_H

