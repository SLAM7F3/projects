#ifndef QSPLINEWIDGET_H
#define QSPLINEWIDGET_H

#include <QVector>
#include <QWidget>
	
#include <geometry/OpenNaturalCubicSpline.h>

class QSplineWidget : public QWidget
{
    Q_OBJECT

public:

    QSplineWidget( bool initializeWithStraightLine = true,
		int width = 200, int height = 200, int controlPointRadius = 10,
		int nTransferFunctionSamples = 256, QWidget* parent = 0 );
    virtual ~QSplineWidget();
	
	QVector< float > getTransferFunction();

	// get y(x)
	float evaluateSpline( float x );

signals:

	void transferFunctionChanged( QVector< float > qvTransferFunction );

protected:

	virtual void mouseMoveEvent( QMouseEvent* event );
	virtual void mousePressEvent( QMouseEvent* event );
	virtual void mouseReleaseEvent( QMouseEvent* event );
	virtual void paintEvent( QPaintEvent* event );

private:

	int getIndexOfFirstControlPointGreaterThan( int x, QVector< int > v );

	// returns -1 if missed
	int getIndexOfClickedControlPoint( int x, int y );
	void insertControlPoint( int x, int y );
	void deleteControlPoint( int controlPointIndex );

	void updateSpline();
	
	int m_iControlPointRadius;

	int m_iMouseDownX;
	int m_iMouseDownY;
	int m_iClickedControlPointIndex;

	int m_nControlPoints;
	QVector< int > m_vXControlPoints;
	QVector< int > m_vYControlPoints;

	OpenNaturalCubicSpline m_xSpline;
	OpenNaturalCubicSpline m_ySpline;

	// y[x]	
	int m_nTransferFunctionSamples;
	QVector< float > m_qvTransferFunction;
};

#endif // QSPLINEWIDGET_H
