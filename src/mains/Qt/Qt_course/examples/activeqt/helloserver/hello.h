#ifndef HELLO_H
#define HELLO_H

#include <QWidget>

class QTimer;

class Hello : public QWidget
{
    Q_OBJECT
	Q_CLASSINFO("ClassID", "{500F47E0-A9F1-4c50-9642-6E3EB464662F}")
	Q_CLASSINFO("InterfaceID", "{6D8B07B1-B4B8-4be8-B90C-98A0559C920C}")
	Q_CLASSINFO("EventsID", "{AE627982-160E-4be6-9BF9-D559CE056D58}")
	Q_PROPERTY( QString text READ text WRITE setText )
public:
    Hello( QWidget *parent=0 );

	QString text() const;
public slots:
	void setText( const QString& );

	void start();
	void stop();
signals:
    void clicked();
protected:
    void mouseReleaseEvent( QMouseEvent * );
    void paintEvent( QPaintEvent * );
private slots:
    void animate();
private:
    QString _text;
    int     b;
	QTimer* _timer;
};

#endif
