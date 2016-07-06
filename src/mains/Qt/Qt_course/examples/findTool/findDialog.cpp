#include <QtGui>
#include "findDialog.h"

FindDialog::FindDialog( QWidget* parent ) :QDialog( parent )
{
    setupUi( this );
    _process = new QProcess( this );
    connect( _process, SIGNAL( finished(int) ), this, SLOT( endOfProcess() ) );
    connect( _process, SIGNAL( readyReadStandardOutput() ), this, SLOT( readStdout() ) );
    connect( _process, SIGNAL( readyReadStandardError() ), this, SLOT( readStderr() ) );
    connect( abortButton, SIGNAL( clicked() ), this, SLOT( slotEndProcess() ) );
}

void FindDialog::on_editButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory( this, QString(),rootDir->text() );
    if ( !dir.isNull() ) {
        rootDir->setText( dir );
    }
}

void FindDialog::on_goButton_clicked()
{
    QStringList arguments;

    if ( !rootDir->text().isEmpty() )
        arguments << rootDir->text();
    else
        arguments << ".";

    arguments << "-name" << filePattern->text();

    qDebug() << arguments;
    // Now run the process
    _process->start( "find", arguments );

    abortButton->setEnabled( true );
    goButton->setEnabled( false );
    results->clear();
}

void FindDialog::endOfProcess()
{
    abortButton->setEnabled( false );
    goButton->setEnabled( true );
}

void FindDialog::readStdout()
{
    _process->setReadChannel( QProcess::StandardOutput );
    while ( _process->canReadLine() ) {
        QString line =_process->readLine();
        line.chop(1); // remove \n
        results->append( line );
    }
}

void FindDialog::readStderr()
{
    _process->setReadChannel( QProcess::StandardError );
    while ( _process->canReadLine() ) {
        QString line =_process->readLine();
        line.chop(1); // remove \n
        results->append( QString("<font color=\"red\">%1</font>").arg(line) );
    }
}

void FindDialog::slotEndProcess()
{
    _process->terminate();
}

void FindDialog::on_quitButton_clicked()
{
    slotEndProcess();
    qApp->quit();
}

