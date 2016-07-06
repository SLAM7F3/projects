// ==========================================================================
// Program MAINCHAT
// ==========================================================================
// Last updated on 7/26/07
// ==========================================================================

// This is a good example.  To run it, we need to first fire up the
// server created by this main program.  We then need to fire up a
// client.  Specifically, we can use telnet as a client which talks to
// our server.  Chant "telnet 127.0.0.1 4242".  Then whatever we type
// into the telnet window should be echoed within the TextEdit window
// on the server.

#include <iostream>
#include <QtGui>
#include "MyChat.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   QApplication my_app(argc,argv);

   MyChat* chat_ptr=new MyChat;
   chat_ptr->show();

   return my_app.exec();
}

