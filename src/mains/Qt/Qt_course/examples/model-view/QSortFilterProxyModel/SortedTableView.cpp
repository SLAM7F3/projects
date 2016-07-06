#include <QtGui>

#include "SortedTableView.h"


SortedTableView::SortedTableView( QWidget *parent )
    : QWidget ( parent )
    , _filter ( new QSortFilterProxyModel() )
{
    _layout = new QGridLayout(this);

    _label = new QLabel( this);
    _layout->addWidget(_label, 0, 0);

    _lineEdit = new QLineEdit(this);
    _layout->addWidget(_lineEdit, 0, 1);
    _label->setText( tr( "Filter:" ) );
    _label->setBuddy(_lineEdit);

    _tableView = new QTableView(this);
    _layout->addWidget(_tableView, 1, 0, 1, 2);

    _tableView->setSortingEnabled( true );
    //_filter->setSortCaseSensitivity( Qt::CaseInsensitive );

    _tableView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    connect ( _tableView->horizontalHeader(), SIGNAL( sectionClicked( int ) ),
              SLOT( setFilterColumn( int ) ) );
    _filter->setFilterCaseSensitivity ( Qt::CaseInsensitive );
    connect ( _lineEdit, SIGNAL( textChanged(QString) ),
              _filter, SLOT( setFilterWildcard (QString) ) );

    setFilterColumn( 0 );
}

SortedTableView::~SortedTableView()
{
    delete _filter;
}

void SortedTableView::setModel( QAbstractItemModel *model )
{
    _filter->setSourceModel( model );
    _tableView->setModel( _filter );
    _tableView->resizeColumnsToContents();
}

void SortedTableView::setFilterColumn( int column )
{
    _filter->setFilterKeyColumn( column );
    _lineEdit->setFocus();
    _lineEdit->clear();
}
