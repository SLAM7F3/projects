#ifndef COMPASSWIDGET2_H
#define COMPASSWIDGET2_H

#include <QtGui>

class QRegion;

class CompassWidget2 : public QWidget
{
	Q_OBJECT

public:
	enum CompassDirection { North, NorthWest, West, SouthWest, South, SouthEast, East, NorthEast,
	                        NumDirections };

	CompassWidget2( CompassDirection d, QWidget* parent = 0 );
	CompassWidget2( QWidget* parent = 0 );
	~CompassWidget2();

	CompassDirection direction() const;

	virtual QSize sizeHint() const { return QSize( 100, 100 ); }

public slots:
	void setDirection( CompassDirection d );

signals:
	void directionChanged( CompassDirection d );
	void directionChanged( const QString& );

protected:
	void paintEvent( QPaintEvent* );
	void mousePressEvent( QMouseEvent* );
	void resizeEvent( QResizeEvent* );
	void keyPressEvent( QKeyEvent* );

private:
	void init();
	void setDirectionAndEmit( CompassDirection d );
	CompassDirection _direction;
	QPolygon _points[ NumDirections ];
	QRegion* _region[ NumDirections ];
};

#endif


