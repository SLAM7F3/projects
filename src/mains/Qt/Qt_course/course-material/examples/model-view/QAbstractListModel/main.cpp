#include <QtGui>

class StringListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    StringListModel(const QStringList& list, QObject *parent = 0)
        : QAbstractListModel(parent), _list(list) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED( parent );
        return _list.count();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (index.row() < 0 || index.row() >= _list.size())
            return QVariant();

        if (role == Qt::DisplayRole || role == Qt::EditRole)
            return _list.at(index.row());
        else
            return QVariant();
    }

    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal) {
            Q_ASSERT( section == 0 );
            return QString("Country");
        }
        else
            return QString("Country %1").arg(section);
    }

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        if (!index.isValid())
            return QAbstractItemModel::flags(index);

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool setData(const QModelIndex &index,
                 const QVariant &value, int role)
    {
        if (index.isValid() && role == Qt::EditRole) {
            _list.replace(index.row(), value.toString());
            emit dataChanged(index, index);
            return true;
        }
        return false;
    }

    bool insertRows(int position, int rows, const QModelIndex& /*parent*/ )
    {
        beginInsertRows(QModelIndex(), position, position+rows-1);

        for (int row = 0; row < rows; ++row) {
            _list.insert(position, "");
        }

        endInsertRows();
        return true;
    }

    bool removeRows(int position, int rows, const QModelIndex& /*parent*/)
    {
        beginRemoveRows(QModelIndex(), position, position+rows-1);

        for (int row = 0; row < rows; ++row) {
            _list.removeAt(position);
        }

        endRemoveRows();
        return true;
    }

private:
    QStringList _list;
};

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    QStringList countries;
    countries << "Denmark" << "Norway" << "Sweden" << "USA" << "Poland";
    StringListModel* model = new StringListModel(countries);

    QListView* list = new QListView;
    list->setModel( model );
    list->setWindowTitle( "QListView" );
    list->show();

    QTreeView* tree = new QTreeView;
    tree->setModel( model );
    tree->setWindowTitle( "QTreeView" );
    tree->show();

    QTableView* table = new QTableView;
    table->setModel( model );
    table->setWindowTitle( "QTableView" );

    table->show();

    return app.exec();
}

#include "main.moc"
