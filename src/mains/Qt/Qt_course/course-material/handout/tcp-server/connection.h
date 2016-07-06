#ifndef CONNECTION_H
#define CONNECTION_H

#include <QWidget>
class QLineEdit;
class QTextEdit;
class QTcpSocket;

class ConnectionWindow : public QWidget
{
    Q_OBJECT
public:
    ConnectionWindow( QTcpSocket* socket, QWidget* parent );

private slots:
    void slotRead();
    void slotSendLine();

private:
    QTcpSocket* _socket;
    QTextEdit *_in;
    QLineEdit *_out;
};

#endif /* CONNECTION_H */

