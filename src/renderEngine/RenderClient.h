
#ifndef RENDER_CLIENT_H
#define RENDER_CLIENT_H

#include <iostream>
#include <sstream>
#include <string>

#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile> 

#include "socket/ClientSocket.h"
#include "socket/ServerSocket.h"
#include "socket/packet_header.h"

class RenderClient
{
  public:

   RenderClient(std::string host="localhost", int port_number=7777);
   bool send_osg_group(osg::Group* group, std::string name, 
                       std::string extension);
   bool send_delete_group(std::string name);
   void close();

  private:
        
   ClientSocket* the_connection;
   std::string host;
   int port_number;
};

#endif

