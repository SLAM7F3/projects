#include "QBilateralSigmaWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <math/MathUtils.h>

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

QBilateralSigmaWidget::QBilateralSigmaWidget( int width, int height, int controlPointRadius,
											 float xMin, float xMax, float yMin, float yMax, QWidget* parent ) :
    
	QWidget( parent ),

	m_iWidth( width ),
	m_iHeight( height ),
	m_iControlPointRadius( controlPointRadius ),
	m_fxMin( xMin ),
	m_fxMax( xMax ),
	m_fyMin( yMin ),
	m_fyMax( yMax ),

	m_iMouseDownX( -1 ),
	m_iMouseDownY( -1 ),
	m_iClickedControlPointIndex( -1 ),

	m_qLinePen( Qt::gray )
{
	// initialize 7 pens
	m_qvPens.append( QPen( Qt::red ) );
	m_qvPens.append( QPen( Qt::green ) );
	m_qvPens.append( QPen( Qt::blue ) );
	m_qvPens.append( QPen( Qt::cyan ) );
	m_qvPens.append( QPen( Qt::yellow ) );
	m_qvPens.append( QPen( Qt::magenta ) );	

	m_qLinePen.setStyle( Qt::DashDotLine );	

	setFixedSize( width, height );
}

// virtual
QBilateralSigmaWidget::~QBilateralSigmaWidget()
{

}

void QBilateralSigmaWidget::appendControlPoint( float fx, float fy )
{
	fx = MathUtils::clampToRangeFloat( fx, m_fxMin, m_fxMax );
	fy = MathUtils::clampToRangeFloat( fy, m_fyMin, m_fyMax );

	int ix = MathUtils::rescaleFloatToInt( fx, m_fxMin, m_fxMax, 0, width() );
	int iy = height() - MathUtils::rescaleFloatToInt( fy, m_fyMin, m_fyMax, 0, height() ) - 1;

	m_qvXControlPoints.append( ix );
	m_qvYControlPoints.append( iy );	

	m_qvFloatXControlPoints.append( fx );
	m_qvFloatYControlPoints.append( fy );
}

void QBilateralSigmaWidget::deleteControlPoint( int controlPointIndex )
{
	m_qvXControlPoints.remove( controlPointIndex );
	m_qvYControlPoints.remove( controlPointIndex );

	m_qvFloatXControlPoints.remove( controlPointIndex );
	m_qvFloatYControlPoints.remove( controlPointIndex );
}


//////////////////////////////////////////////////////////////////////////
// Protected
//////////////////////////////////////////////////////////////////////////

// virtual
void QBilateralSigmaWidget::mouseMoveEvent( QMouseEvent* event )
{
	const QPoint position = event->pos();
	int mouseMoveX = position.x();
	int mouseMoveY = position.y();

	mouseMoveX = MathUtils::clampToRangeInt( mouseMoveX, 0, width() );
	mouseMoveY = MathUtils::clampToRangeInt( mouseMoveY, 0, height() );

	if( m_iClickedControlPointIndex != -1 )
	{
		m_qvXControlPoints[ m_iClickedControlPointIndex ] = mouseMoveX;
		m_qvYControlPoints[ m_iClickedControlPointIndex ] = mouseMoveY;

		// TODO: convertMouseXYToFloatXY or whatever
		float fx = MathUtils::rescaleIntToFloat( mouseMoveX, 0, width(), m_fxMin, m_fxMax );
		float fy = MathUtils::rescaleIntToFloat( height() - mouseMoveY - 1, 0, height(), m_fyMin, m_fyMax );

		m_qvFloatXControlPoints[ m_iClickedControlPointIndex ] = fx;
		m_qvFloatYControlPoints[ m_iClickedControlPointIndex ] = fy;

		update(); // redraw
		emit controlPointsChanged( m_qvFloatXControlPoints, m_qvFloatYControlPoints );
	}
}

// virtual
void QBilateralSigmaWidget::mousePressEvent( QMouseEvent* event )
{
	const QPoint position = event->pos();
	m_iMouseDownX = position.x();
	m_iMouseDownY = position.y();

	m_iClickedControlPointIndex = getIndexOfClickedControlPoint( m_iMouseDownX, m_iMouseDownY );
}

// virtual
void QBilateralSigmaWidget::mouseReleaseEvent( QMouseEvent* event )
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
			float fx = MathUtils::rescaleIntToFloat( mouseUpX, 0, width(), m_fxMin, m_fxMax );
			float fy = MathUtils::rescaleIntToFloat( height() - mouseUpY - 1, 0, height(), m_fyMin, m_fyMax );

			appendControlPoint( fx, fy );
			update(); // redraw
			emit controlPointsChanged( m_qvFloatXControlPoints, m_qvFloatYControlPoints );
		}
		else // right button: delete
		{			
			if( m_iClickedControlPointIndex != -1 )
			{
				deleteControlPoint( m_iClickedControlPointIndex );				
				update(); // redraw
				emit controlPointsChanged( m_qvFloatXControlPoints, m_qvFloatYControlPoints );
			}
		}
	}
}

// virtual
void QBilateralSigmaWidget::paintEvent( QPaintEvent* event )
{
	QPainter painter( this );
	painter.setRenderHint( QPainter::Antialiasing );
	painter.setRenderHint( QPainter::TextAntialiasing );

	QBrush blackBrush( Qt::black );
	painter.fillRect( rect(), blackBrush );

	for( int i = 0; i < m_qvXControlPoints.size(); ++i )
	{
		int x = m_qvXControlPoints[i];
		int y = m_qvYControlPoints[i];
		float fx = m_qvFloatXControlPoints[i];
		float fy = m_qvFloatYControlPoints[i];

		painter.setPen( m_qLinePen );
		painter.drawLine( 0, y, width(), y );
		painter.drawLine( x, 0, x, height() );

		QString textString = QString( "s: %1 r: %2" ).arg( fx, 0, 'f', 2 ).arg( fy, 0, 'f', 2 );
		painter.drawText( x + m_iControlPointRadius + 2, y - 2,
			50, 50,
			Qt::TextWordWrap,
			textString );

		QPen circlePen = m_qvPens.at( i );
		circlePen.setWidth( 3 );
		painter.setPen( circlePen );
		painter.drawEllipse( x - m_iControlPointRadius, y - m_iControlPointRadius, 2 * m_iControlPointRadius, 2 * m_iControlPointRadius );		
	}
}

//////////////////////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////////////////////

int QBilateralSigmaWidget::getIndexOfClickedControlPoint( int x, int y )
{
	for( int i = 0; i < m_qvXControlPoints.size(); ++i )
	{
		int controlPointX = m_qvXControlPoints[i];
		int controlPointY = m_qvYControlPoints[i];

		int dxSquared = ( x - controlPointX ) * ( x - controlPointX );
		int dySquared = ( y - controlPointY ) * ( y - controlPointY );

		if( dxSquared + dySquared < m_iControlPointRadius * m_iControlPointRadius )
		{
			return i;
		}
	}

	return -1;
}
