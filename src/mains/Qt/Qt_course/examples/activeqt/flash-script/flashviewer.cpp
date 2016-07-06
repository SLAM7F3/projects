#include <QtGui>
#include <QAxWidget>
#include <QAxScriptManager>

#include "flashviewer.h"

FlashViewer::FlashViewer(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	ui.scriptEdit->setPlainText("axWidget.movie = \"C:\\WINDOWS\\Help\\Tours\\mmTour\\intro.swf\"\naxWidget.Rewind()\naxwidget.Play()");
	m_scriptmgr = new QAxScriptManager(this);
	connect(m_scriptmgr,SIGNAL(error(QAxScript*,int,const QString&,int,const QString&)),
		this, SLOT(slotError(QAxScript*,int,const QString&,int,const QString&)));
	qDebug() << "Adding object " << ui.axWidget->objectName();
	m_scriptmgr->addObject(static_cast<QAxBase*>(ui.axWidget));
}

FlashViewer::~FlashViewer()
{
}

void FlashViewer::on_runPB_clicked()
{
	ui.statusLabel->setText(tr("Running"));
	ui.runPB->setEnabled(false);
	QString code = QString::fromAscii("Sub myfunc()\n  ")+ui.scriptEdit->toPlainText()+QString::fromAscii("\nEnd Sub");
	m_script = m_scriptmgr->load( code, "myscript", "VBScript");
	connect(m_script,SIGNAL(finished()),this,SLOT(slotFinished()));
	m_script->call("myfunc()");
}

void FlashViewer::slotFinished()
{
	m_script->deleteLater();
	m_script = 0;
	ui.runPB->setEnabled(true);
	ui.statusLabel->setText(tr("Done"));
}

void FlashViewer::slotError( QAxScript*, int code, const QString& descr,
							int srcpos, const QString& srctxt)
{
	ui.statusLabel->setText(tr("Error %1 in script line %2: %3").arg(code).arg(srcpos).arg(descr));
	m_script->deleteLater();
	m_script = 0;
	ui.runPB->setEnabled(true);
}