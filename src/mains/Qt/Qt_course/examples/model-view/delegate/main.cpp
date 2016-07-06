#include <QtGui>

class CountryDelegate :public QItemDelegate
{
    Q_OBJECT
public:
    virtual QWidget* createEditor ( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        if ( index.column() == 1 )
            return QItemDelegate::createEditor(parent, option, index );

        QComboBox* editor = new QComboBox( parent );
        editor->installEventFilter( const_cast<CountryDelegate*>(this) );
        return editor;
    }
    virtual void setEditorData( QWidget* editor, const QModelIndex& index ) const
    {
        if ( index.column() == 1 )
            QItemDelegate::setEditorData( editor, index );
        else {
            QComboBox* combo = static_cast<QComboBox*>( editor );
            combo->addItems( countries() );
            int idx = CountryDelegate::countries().indexOf( index.data( Qt::DisplayRole ).toString() );
            combo->setCurrentIndex( idx );
        }
    }
    virtual void setModelData( QWidget * editor, QAbstractItemModel* model, const QModelIndex& index ) const
    {
        if ( index.column() == 1 )
            QItemDelegate::setModelData( editor, model, index );
        else {
            QComboBox* combo = static_cast<QComboBox*>( editor );
            model->setData( index, combo->currentText() );
        }
    }

    virtual void updateEditorGeometry(QWidget *editor,
                                      const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        if ( index.column() == 1 )
            QItemDelegate::updateEditorGeometry( editor, option, index );
        else {
            // Just a silly example, don't allow the editor to get a smaller height than its sizehint.
            int hCell = option.rect.height();
            int hEditor = editor->sizeHint().height();
            int h = qMax( hCell, hEditor );
            QSize size = option.rect.size();
            size.setHeight( h );
            editor->setGeometry( QRect( option.rect.topLeft() - QPoint(0,(h-hCell)/2), size ) );
        }
    }

    virtual bool eventFilter( QObject* obj, QEvent* event )
    {
        if ( event->type() == QEvent::KeyRelease && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Return ) {
            emit commitData( static_cast<QWidget*>(obj) );
            emit closeEditor( static_cast<QWidget*>(obj), EditNextItem );
        }
        return false;
    }


    static QStringList countries()
    {
        QStringList countries;
        countries << "Denmark" << "Sweeden" << "Norway" << "USA" << "Germany"
                  << "Poland" << "Iceland" << "Holland" << "Great Britain" << "Ireland" << "Scotland";
        return countries;
    }
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QList<int> populations;
    populations << 5432 << 9001 << 4593 << 295734 << 82431 << 38635 << 296 << 16407 << 60441 << 4015 << 5062;

    QStandardItemModel* model = new QStandardItemModel( 11, 2 );
    for ( int row = 0; row < 11; ++row ) {
        model->setData( model->index( row, 0, QModelIndex() ), CountryDelegate::countries()[row] );
        model->setData( model->index( row, 1, QModelIndex() ), populations[row] );
        model->setData( model->index( row, 1, QModelIndex() ), Qt::AlignRight, Qt::TextAlignmentRole );
    }
    model->setHeaderData( 0, Qt::Horizontal, "Country" );
    model->setHeaderData( 1, Qt::Horizontal, "Population" );

    QListView* list = new QListView;
    list->setWindowTitle( "QListView" );
    list->setModel( model );
    list->setItemDelegate( new CountryDelegate() );
    list->show();

    QTableView* table = new QTableView;
    table->setWindowTitle( "QTableView" );
    table->setModel( model );
    table->setItemDelegate( new CountryDelegate() );
    table->show();

    QTreeView* tree = new QTreeView;
    tree->setWindowTitle( "QTreeView" );
    tree->setModel( model );
    tree->setItemDelegate( new CountryDelegate() );
    tree->show();

    return app.exec();
}

#include "main.moc"
