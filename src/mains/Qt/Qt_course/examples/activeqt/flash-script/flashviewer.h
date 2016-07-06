#ifndef FLASHVIEWER_H
#define FLASHVIEWER_H

#include <QtGui/QWidget>
#include "ui_flashviewer.h"

class QAxScript;
class QAxScriptManager;

class FlashViewer : public QWidget
{
    Q_OBJECT

public:
    FlashViewer(QWidget *parent = 0);
    ~FlashViewer();

private:
    Ui::flashviewerClass ui;
	QAxScriptManager* m_scriptmgr;
	QAxScript* m_script;

private slots:
	void on_runPB_clicked();
	void slotFinished();
	void slotError( QAxScript* script, int code, const QString& descr,
							int srcpos, const QString& srctxt);
};

#endif // FLASHVIEWER_H
