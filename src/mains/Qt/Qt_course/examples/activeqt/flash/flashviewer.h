#ifndef FLASHVIEWER_H
#define FLASHVIEWER_H

#include <QtGui/QWidget>
#include "ui_flashviewer.h"

class FlashViewer : public QWidget
{
    Q_OBJECT

public:
    FlashViewer(QWidget *parent = 0);
    ~FlashViewer();

private:
    Ui::flashviewerClass ui;

private slots:
	void on_filenameEdit_textChanged(const QString &);
	void on_browsePB_clicked();
};

#endif // FLASHVIEWER_H
