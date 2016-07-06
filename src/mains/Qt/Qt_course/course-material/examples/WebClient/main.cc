#include <string>
#include <QtGui>
#include "WebClient.h"

using std::string;

int main(int argc, char** argv)
{
   QApplication app( argc, argv );

   string host_IP="155.34.162.78";
   int port=4040;
   WebClient* WebClient_ptr = new WebClient(host_IP,port);

   return app.exec();
}


