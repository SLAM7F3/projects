// ==========================================================================
// Header file for Designer version of MyChat class
// ==========================================================================
// Last modified on 7/26/07
// ==========================================================================

#ifndef MYCHAT_H
#define MYCHAT_H

#include <iostream>
#include <string>
#include <vector>
#include <QtGui>
#include <QTcpSocket>
#include <QTcpServer>

#include "ui_chatdialog.h"

class MyChat: public QWidget , private Ui::ChatDialog
{

   Q_OBJECT

  public:

// Initialization, constructor and destructor functions:

   MyChat(QWidget* parent_ptr=NULL);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~MyChat();

// Set and get member functions:

   void setup_initial_signal_slot_connections();
   void setup_later_signal_slot_connections();
   void set_tool_tips();
   
  private: 
   
   QHostAddress host_address;
   int port_number;
   QTcpServer* server_ptr;
   std::vector<QTcpSocket*> socket_ptrs;
//   QTcpSocket* socket_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
//   void docopy(const MyChat& p);

   private slots:

     void on_lineEdit_textChanged();
     void slotConnected();
     void readData();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================


#endif  // MyChat.h



