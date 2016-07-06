#include <QtGui>
#include "replaceprogressdialog.h"

ReplaceProgressDialog::ReplaceProgressDialog( QTextEdit* edit, const QString& search, const QString& replacement, QWidget* parent )
    :QDialog( parent ), _edit( edit ),_search( search ), _replacement( replacement ), _replaceAll( false )
{
    setupUi( this );
    replacementText->setText( QString( "Replace %1 with %2?" ).arg( search ).arg( replacement ) );
    connect( yes, SIGNAL( clicked() ), this, SLOT( replaceCurrent() ) );
    connect( skip, SIGNAL( clicked() ), this, SLOT( skipCurrent() ) );
    connect( all, SIGNAL( clicked() ), this, SLOT( replaceAll() ) );
    connect( Ui::ReplaceProgress::close, SIGNAL( clicked() ), this, SLOT( accept() ) );
    _cursor = QTextCursor( edit->document() );
    findNext();
}

void ReplaceProgressDialog::replaceCurrent()
{
    _cursor.insertText( _replacement );
    findNext();
}

void ReplaceProgressDialog::skipCurrent()
{
    findNext();
}

void ReplaceProgressDialog::replaceAll()
{
    _replaceAll = true;
    do {
        _cursor.insertText( _replacement );
        findNext();
    } while ( !_cursor.isNull() );
}

void ReplaceProgressDialog::findNext()
{
    _cursor = _edit->document()->find( _search, _cursor );
    if ( _cursor.isNull() )
        accept();
    else {
        _cursor.movePosition( QTextCursor::StartOfWord );
        _cursor.movePosition( QTextCursor::EndOfWord, QTextCursor::KeepAnchor );
        _edit->setTextCursor( _cursor );
    }
}
