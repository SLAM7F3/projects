#include "flashviewer.h"

#include <QFileDialog>
#include <QAxWidget>

FlashViewer::FlashViewer(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	ui.filenameEdit->setText("C:\\WINDOWS\\Help\\Tours\\mmTour\\intro.swf");
}

FlashViewer::~FlashViewer()
{

}

void FlashViewer::on_browsePB_clicked()
{
	QString filename = QFileDialog::getOpenFileName(this,tr("Open Movie"),ui.filenameEdit->text(),"*.swf");
	if( !filename.isEmpty() ) ui.filenameEdit->setText(filename);
}

void FlashViewer::on_filenameEdit_textChanged(const QString & filename)
{
	ui.axWidget->dynamicCall("SetMovie", filename);
}