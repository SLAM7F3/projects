#ifndef SORTEDTABLEVIEW_H
#define SORTEDTABLEVIEW_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QTableView;
class QGridLayout;
class QAbstractItemModel;
class QSortFilterProxyModel;

class SortedTableView : public QWidget
{
    Q_OBJECT

public:
    explicit SortedTableView ( QWidget *parent = 0 );

    void setModel ( QAbstractItemModel* model );

protected slots:
    void setFilterColumn( int );

private:
    QSortFilterProxyModel *_filter;
    QGridLayout *_layout;
    QTableView *_tableView;
    QLabel *_label;
    QLineEdit *_lineEdit;
};


#endif
