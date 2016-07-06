#ifndef LISTENER_H
#define LISTENER_H

#include <QTextEdit>
#include <QTcpSocket>

class Listener :public QTextEdit
{
   Q_OBJECT

      public:
   Listener( QWidget* parent );

   private slots:
      void readData();
   void slotError( QAbstractSocket::SocketError );
   void slotConnected();
   void slotHostFound();

  private:
   QTcpSocket* _socket;
};


#endif /* LISTENER_H */

