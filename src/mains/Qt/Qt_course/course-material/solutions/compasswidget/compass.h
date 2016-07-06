#ifndef COMPASSWIDGET_H
#define COMPASSWIDGET_H

#include <QtGui>

class QPushButton;
class QButtonGroup;

class CompassWidget : public QWidget
{
	Q_OBJECT

public:
	enum CompassDirection { North, NorthWest, West, SouthWest, South, SouthEast, East, NorthEast };

	CompassWidget( CompassDirection d, QWidget* parent = 0 );
	CompassWidget( QWidget* parent = 0 );

	CompassDirection direction() const;

public slots:
	void setDirection( CompassDirection d );

signals:
	void directionChanged( CompassDirection d );
	void directionChanged( const QString& );

private slots:
	void slotButtonChecked( QAbstractButton* button );

private:
	void init();
	CompassDirection _direction;
	QPushButton* _northbutton;
	QPushButton* _northwestbutton;
	QPushButton* _northeastbutton;
	QPushButton* _westbutton;
	QPushButton* _eastbutton;
	QPushButton* _southwestbutton;
	QPushButton* _southeastbutton;
	QPushButton* _southbutton;
	QButtonGroup* _buttongroup;
};

#endif


