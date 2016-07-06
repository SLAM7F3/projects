#ifndef QBILATERALSIGMAWIDGET_H
#define QBILATERALSIGMAWIDGET_H

#include <QPen>
#include <QVector>
#include <QWidget>

class QBilateralSigmaWidget : public QWidget
{
    Q_OBJECT

public:

	QBilateralSigmaWidget( int width, int height, int controlPointRadius,
		float xMin, float xMax, float yMin, float yMax, QWidget* parent = 0 );
	
    virtual ~QBilateralSigmaWidget();

	void appendControlPoint( float fx, float fy );
	void deleteControlPoint( int controlPointIndex );

signals:

	void controlPointsChanged( QVector< float > qvXControlPoints, QVector< float > qvYControlPoints );

protected:

	virtual void mouseMoveEvent( QMouseEvent* event );
	virtual void mousePressEvent( QMouseEvent* event );
	virtual void mouseReleaseEvent( QMouseEvent* event );
	virtual void paintEvent( QPaintEvent* event );

private:

	// returns -1 if missed
	int getIndexOfClickedControlPoint( int x, int y );	

	QVector< int > m_qvXControlPoints;
	QVector< int > m_qvYControlPoints;

	QVector< float > m_qvFloatXControlPoints;
	QVector< float > m_qvFloatYControlPoints;

	QVector< QPen > m_qvPens;
	QPen m_qLinePen;

	int m_iWidth;
	int m_iHeight;
	int m_iControlPointRadius;
	float m_fxMin;
	float m_fxMax;
	float m_fyMin;
	float m_fyMax;

	int m_iMouseDownX;
	int m_iMouseDownY;
	int m_iClickedControlPointIndex;
};

#endif // QBILATERALSIGMAWIDGET_H
