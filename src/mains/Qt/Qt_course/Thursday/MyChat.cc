// ==========================================================================
// MyChat class member function definitions
// ==========================================================================
// Last modified on 7/26/07
// ==========================================================================

#include "MyChat.h"
#include <QtDebug>

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MyChat::allocate_member_objects()
{
   server_ptr=new QTcpServer(this);
}		       

void MyChat::initialize_member_objects()
{
   host_address=QHostAddress("127.0.0.1");
   port_number=4242;
//   socket_ptr=NULL;
}

MyChat::MyChat(QWidget* parent_ptr):
   QWidget(parent_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   setupUi(this);
   setup_initial_signal_slot_connections();

   server_ptr->listen(host_address , port_number);
}

MyChat::~MyChat()
{
}

// ---------------------------------------------------------------------
// These first signal/slot relationships should be established BEFORE
// any network connections are made...

void MyChat::setup_initial_signal_slot_connections()
{
   QObject::connect(lineEdit, SIGNAL(textEdited(const Qstring&)),
                    this, SLOT(on_lineEdit_textChanged()));
   QObject::connect( server_ptr, SIGNAL( newConnection() ), 
                     this, SLOT( slotConnected() ) );
}

// ---------------------------------------------------------------------
void MyChat::on_lineEdit_textChanged()
{
   QString mystring(lineEdit->text());
   textEdit->setText(mystring);
}

// ---------------------------------------------------------------------
void MyChat::slotConnected()
{
   qDebug( "inside MyChat::slotConnected()" );
   socket_ptrs.push_back(server_ptr->nextPendingConnection());

   setup_later_signal_slot_connections();
}

// ---------------------------------------------------------------------
void MyChat::setup_later_signal_slot_connections()
{
   QObject::connect( socket_ptrs.back(), SIGNAL(readyRead()), 
                     this, SLOT(readData()) );
}

// ---------------------------------------------------------------------
void MyChat::readData()
{
   qDebug("Ready to read data");
   QTextStream stream( socket_ptrs.back() );
   QString txt = stream.readAll();
   textEdit->append( txt );
}
