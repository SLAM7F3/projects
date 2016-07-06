// ==========================================================================
// Program SENDIMAGE acts as a simple iPhone simulator which can
// transmit input photos plus metadata to a VideoServer.  
// ==========================================================================
// Last updated on 5/13/09; 5/14/09; 5/15/09
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <QtCore/QtCore>
#include <QtNetwork/QHttp>
#include "astro_geo/Clock.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   QCoreApplication my_app(argc,argv);

   Clock clock;

//   string image_filename="piers1.jpg";
   string image_filename="copley.jpg";
   cout << "Enter name of image file to send:" << endl;
   cin >> image_filename;

   QFile imageFile( image_filename.c_str() );
   imageFile.open( QIODevice::ReadOnly );
   QByteArray image_data(imageFile.readAll());


   string iPhone_server_IP="127.0.0.1";
   int iPhone_server_portnumber=4042;
   string iPhone_server_url=
      "http://"+iPhone_server_IP+":"+stringfunc::number_to_string(
         iPhone_server_portnumber);
   iPhone_server_url += "/iPhone_photo/";
   QString url(iPhone_server_url.c_str());

   QUrl qurl(url);

   typedef QPair<QString, QString> Pair;
   QList<Pair> items;

   Pair curr_pair;

// Simulate iPhone transmitting photo filename:

//   curr_pair.first=QString("Filename");
//   curr_pair.second=QString(image_filename.c_str());
//   items.append(curr_pair);

// Simulate iPhone transmitting its geoposition as
// Longitude/Latitude/Altitude coordinates:

   curr_pair.first=QString("Longitude");
   curr_pair.second=QString("101.1");
   items.append(curr_pair);
   curr_pair.first=QString("Latitude");
   curr_pair.second=QString("34.5");
   items.append(curr_pair);
   curr_pair.first=QString("Altitude");
   curr_pair.second=QString("56.3");
   items.append(curr_pair);

// Simulate iPhone transmitting lateral and vertical uncertainties
// measured in meters:

   curr_pair.first=QString("HorizontalUncertainty");
   curr_pair.second=QString("20");
   items.append(curr_pair);
   curr_pair.first=QString("VerticalUncertainty");
   curr_pair.second=QString("30");
   items.append(curr_pair);

// Simulate iPhone transmitting current time as seconds elapsed since
// some canonical reference time = Midnight on 1 Jan 1970:

   clock.current_local_time_and_UTC();
   double elapsed_secs=clock.secs_elapsed_since_reference_date();
   cout << "elapsed_secs = " << elapsed_secs << endl;
   curr_pair.first=QString("Time");
   curr_pair.second=QString(
      stringfunc::number_to_string(elapsed_secs).c_str());
   items.append(curr_pair);
   qurl.setQueryItems(items);

   url=qurl.toString();

   QHttp iPhone_client;
   iPhone_client.setHost(
      QString(iPhone_server_IP.c_str()),iPhone_server_portnumber);

   iPhone_client.post(url,image_data);

   return my_app.exec();
}

