#include <QtGui>
#include "replaceprogressdialog.h"

ReplaceProgressDialog::ReplaceProgressDialog( QTextEdit* edit, const QString& search, const QString& replacement, QWidget* parent )
    :QDialog( parent ), _edit( edit ),_search( search ), _replacement( replacement )
{
    setupUi( this );
    replacementText->setText( QString( "Replace %1 with %2?" ).arg( search ).arg( replacement ) );
    connect( yes, SIGNAL( clicked() ), this, SLOT( replaceCurrent() ) );
    connect( skip, SIGNAL( clicked() ), this, SLOT( skipCurrent() ) );
    connect( all, SIGNAL( clicked() ), this, SLOT( replaceAll() ) );
    connect( Ui::ReplaceProgress::close, SIGNAL( clicked() ), this, SLOT( accept() ) );
    findNext();
}

void ReplaceProgressDialog::replaceCurrent()
{
    // TODO
}

void ReplaceProgressDialog::skipCurrent()
{
    // TODO
}

void ReplaceProgressDialog::replaceAll()
{
    // TODO
}

void ReplaceProgressDialog::findNext()
{
    // TODO
}
