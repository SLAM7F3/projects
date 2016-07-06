#include "QSplineWidget.h"

#include <QMouseEvent>
#include <QPainter>

#include <math/MathUtils.h>

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

QSplineWidget::QSplineWidget( bool initializeWithStraightLine,
							 int width, int height, int controlPointRadius,
							 int nTransferFunctionSamples, QWidget* parent ) :
    
	QWidget( parent ),
	m_nControlPoints( 0 ),
	
	m_iControlPointRadius( controlPointRadius ),

	m_iMouseDownX( -1 ),
	m_iMouseDownY( -1 ),
	m_iClickedControlPointIndex( -1 ),

	m_nTransferFunctionSamples( nTransferFunctionSamples )
{
	// TODO: assert width > 25 or something

	m_qvTransferFunction = QVector< float >( nTransferFunctionSamples );	
	for( int i = 0; i < nTransferFunctionSamples; ++i )
	{
		m_qvTransferFunction[ i ] = i / static_cast< float >( nTransferFunctionSamples - 1 );
	}

	if( initializeWithStraightLine )
	{
		insertControlPoint( 0, height - 1 );
		insertControlPoint( width / 3, height - height / 3 - 1 );
		insertControlPoint( 2 * width / 3, height - 2 * height / 3 - 1 );
		insertControlPoint( width - 1, 0 );
	}

	setFixedSize( width, height );

	updateSpline();
	update();

	for( int i = 0; i < nTransferFunctionSamples; ++i )
	{
		printf( "TF[ %d ] = %f\n", i, m_qvTransferFunction[ i ] );
	}
}

QSplineWidget::~QSplineWidget()
{

}

QVector< float > QSplineWidget::getTransferFunction()
{
	return m_qvTransferFunction;
}

//////////////////////////////////////////////////////////////////////////
// Protected
//////////////////////////////////////////////////////////////////////////

// virtual
void QSplineWidget::mouseMoveEvent( QMouseEvent* event )
{	
	const QPoint position = event->pos();
	int mouseMoveX = position.x();
	int mouseMoveY = position.y();
	
	mouseMoveX = MathUtils::clampToRangeInt( mouseMoveX, 0, width() );
	mouseMoveY = MathUtils::clampToRangeInt( mouseMoveY, 0, height() );

	if( m_iClickedControlPointIndex != -1 )
	{
		// clamp x movement to the two neighboring x values
		int prevIndex = m_iClickedControlPointIndex - 1;
		int nextIndex = m_iClickedControlPointIndex + 1;

		if( prevIndex != -1 )
		{
			mouseMoveX = MathUtils::clampToRangeInt( mouseMoveX, m_vXControlPoints[ prevIndex ] + 2 * m_iControlPointRadius + 1, width() );
		}

		if( nextIndex != m_nControlPoints )
		{
			mouseMoveX = MathUtils::clampToRangeInt( mouseMoveX, 0, m_vXControlPoints[ nextIndex ] - 2 * m_iControlPointRadius - 1 );
		}

		m_vXControlPoints[ m_iClickedControlPointIndex ] = mouseMoveX;
		m_vYControlPoints[ m_iClickedControlPointIndex ] = mouseMoveY;

		updateSpline();
		update(); // redraw
	}
}

// virtual
void QSplineWidget::mousePressEvent( QMouseEvent* event )
{	
	const QPoint position = event->pos();
	m_iMouseDownX = position.x();
	m_iMouseDownY = position.y();

	m_iClickedControlPointIndex = getIndexOfClickedControlPoint( m_iMouseDownX, m_iMouseDownY );
}

// virtual
void QSplineWidget::mouseReleaseEvent( QMouseEvent* event )
{	
	Qt::MouseButton button = event->button();
	const QPoint position = event->pos();
	int mouseUpX = position.x();
	int mouseUpY = position.y();

	if( ( m_iMouseDownX == mouseUpX ) && ( m_iMouseDownY == mouseUpY ) )
	{
		// left button: add
		if( button == Qt::LeftButton )
		{
			insertControlPoint( mouseUpX, mouseUpY );
			update(); // redraw
		}
		else // right button: delete
		{			
			if( m_iClickedControlPointIndex != -1 )
			{
				deleteControlPoint( m_iClickedControlPointIndex );				
				update(); // redraw
			}
		}
	}
}

// virtual
void QSplineWidget::paintEvent( QPaintEvent* event )
{
	QPainter painter( this );
	painter.setRenderHint( QPainter::Antialiasing );

	QBrush blackBrush( Qt::black );
	painter.fillRect( rect(), blackBrush );

	if( m_nControlPoints > 3 )
	{
		QPen greenPen( Qt::green );
		greenPen.setWidth( 3 );
		painter.setPen( greenPen );		

		int nSamples = width(); // TODO: m_nDrawingSamples or something

		for( int i = 0; i < nSamples - 1; ++i )
		{
			float t0 = static_cast< float >( i ) / nSamples;
			float t1 = static_cast< float >( i + 1 ) / nSamples;

			float x0 = width() * m_xSpline( t0 );
			float y0 = height() * ( 1.0f - m_ySpline( t0 ) );
			float x1 = width() * m_xSpline( t1 );
			float y1 = height() * ( 1.0f - m_ySpline( t1 ) );

			painter.drawLine( QPointF( x0, y0 ), QPointF( x1, y1 ) );
		}
	}

	// draw control points
	QBrush redBrush( Qt::red );
	painter.setBrush( redBrush );
	painter.setPen( QPen() );
	for( int i = 0; i < m_nControlPoints; ++i )
	{
		int x = m_vXControlPoints[i];
		int y = m_vYControlPoints[i];

		painter.drawEllipse( x - m_iControlPointRadius, y - m_iControlPointRadius, 2 * m_iControlPointRadius, 2 * m_iControlPointRadius );
	}

	// DEBUG: draw transfer function
	QPen bluePen( Qt::blue );
	bluePen.setWidth( 3 );
	painter.setPen( bluePen );

	float delta = 1.0f / ( m_nTransferFunctionSamples - 1 );
	for( int i = 0; i < m_nTransferFunctionSamples - 1; ++i )
	{
		float x0 = width() * ( i * delta );
		float y0 = height() * ( 1.0f - m_qvTransferFunction.at( i ) );

		float x1 = width() * ( ( i + 1 ) * delta );
		float y1 = height() * ( 1.0f - m_qvTransferFunction.at( i + 1 ) );

		painter.drawLine( QPointF( x0, y0 ), QPointF( x1, y1 ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

int QSplineWidget::getIndexOfFirstControlPointGreaterThan( int x, QVector< int > v )
{
	for( int i = 0; i < v.size(); ++i )
	{
		if( x < v[i] )
		{
			return i;
		}
	}

	// greater than all of them
	return v.size();
}

int QSplineWidget::getIndexOfClickedControlPoint( int x, int y )
{
	for( int i = 0; i < m_nControlPoints; ++i )
	{
		int controlPointX = m_vXControlPoints[i];
		int controlPointY = m_vYControlPoints[i];

		int dxSquared = ( x - controlPointX ) * ( x - controlPointX );
		int dySquared = ( y - controlPointY ) * ( y - controlPointY );

		if( dxSquared + dySquared < m_iControlPointRadius * m_iControlPointRadius )
		{
			return i;
		}
	}

	return -1;
}

void QSplineWidget::insertControlPoint( int x, int y )
{
	int index = getIndexOfFirstControlPointGreaterThan( x, m_vXControlPoints );

	m_vXControlPoints.insert( index, x );	
	m_vYControlPoints.insert( index, y );
	++m_nControlPoints;

	updateSpline();
}

void QSplineWidget::deleteControlPoint( int controlPointIndex )
{
	m_vXControlPoints.remove( controlPointIndex );
	m_vYControlPoints.remove( controlPointIndex );
	--m_nControlPoints;

	updateSpline();
}

void QSplineWidget::updateSpline()
{
	if( m_nControlPoints > 3 )
	{
		QVector< float > xFloatControlPoints;
		QVector< float > yFloatControlPoints;

		float fx;
		float fy;

		for( int i = 0; i < m_nControlPoints; ++i )
		{
			fx = m_vXControlPoints[i] * 1.0f / width();
			fy = 1.0f - m_vYControlPoints[i] * 1.0f / height();

			xFloatControlPoints.append( fx );
			yFloatControlPoints.append( fy );
		}

		m_xSpline.setPoints( xFloatControlPoints );
		m_ySpline.setPoints( yFloatControlPoints );

		// update transfer function		
		float tPrevious;

		float delta = 1.0f / ( m_nTransferFunctionSamples - 1 );
		for( int i = 0; i < m_nTransferFunctionSamples; ++i )
		{
			float t;
			float x = i * delta;
			if( x < xFloatControlPoints[0] )
			{
				t = 0;
			}
			else if( x > xFloatControlPoints[ m_nControlPoints - 1 ] )
			{
				t = 1;
			}
			else
			{
				t = m_xSpline.inverse( x, tPrevious, 0.001f, 20 );
			}

			m_qvTransferFunction[i] = m_ySpline( t );
			tPrevious = t;
		}

		emit transferFunctionChanged( m_qvTransferFunction );
	}
}
