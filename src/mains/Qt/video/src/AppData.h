#ifndef APP_DATA_H
#define APP_DATA_H

#include <GL/glew.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <QObject>
#include <QPair>
#include <QSize>
#include <QVector>

#include <common/BasicTypes.h>
#include <GL/GLInitializable.h>
#include <time/QGameLoop.h>

#define DISPLAY_FRAME_TIME 1

class GLFramebufferObject;
class KernelGraph;
class OutputWidget;
class OutputKernelPort;

class AppData : public QGameLoop, GLInitializable
{
	Q_OBJECT

public:

	static AppData* getInstance();

	void initializeGL();

	void setCgPathPrefix( QString str );
	QString getCgPathPrefix();

	void setTextureNumBits( int nBits );
	int getTextureNumBits();
	
	void setOutputWidthHeight( int width, int height );
	QSize getOutputWidthHeight();

	int getTextureNumComponents();

	KernelGraph* getGraph();
	void setGraph( KernelGraph* pGraph );

	void setAutoDirtyKernelsAndPorts( QVector< QPair< QString, QString > > autoDirtyKernelsAndPorts );

	OutputWidget* getOutputWidget();
	void setOutputWidget( OutputWidget* pWidget );

	void setInputIsReady( bool b );

	int getNumObservedPorts(); // TODO: by type, etc, by name to have multiple ports, etc
	void setObservedPort( OutputKernelPort* pPort, int outputIndex, int arrayIndex = 0 );

	// HACK
	void updateAndDraw();

	virtual ~AppData();

public slots:
	
	void handleSaveFrameClicked();	
	void handleDesiredFPSChanged( int period );		

protected:

	virtual void updateState();
	virtual void draw();

private:

	// =========================================================================
	// Methods
	// =========================================================================

	AppData();
	void initializeCg();

	// ---- Application State ----
	bool isInputReady();	

	// =========================================================================
	// Fields
	// =========================================================================

	static AppData* s_pSingleton;

	GLFramebufferObject* m_pFBO;

	// ---- Computation Graph ----
	KernelGraph* m_pGraph;	

	// ---- Output Widget ----
	QVector< QPair< QString, QString > > m_qvAutoDirtyKernelsAndPorts;
	QVector< OutputKernelPort* > m_qvObservedPorts;
	QVector< int > m_qvObservedPortsArrayIndex;
	OutputWidget* m_pOutputWidget;
	
	// ---- Application State ----
	QString m_qsCgPathPrefix;
	int m_nBits;
	int m_iOutputWidth;
	int m_iOutputHeight;
	bool m_bIsInputReady;
};

#endif
