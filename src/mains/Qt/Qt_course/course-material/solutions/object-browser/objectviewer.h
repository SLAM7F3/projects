#ifndef OBJECTVIEWER_H
#define OBJECTVIEWER_H
#include <QTreeView>

class ObjectViewer : public QTreeView {
    Q_OBJECT

public:
    ObjectViewer( QWidget* parent = 0 );

protected slots:
    void displayWidget( const QModelIndex& );
};


#endif /* OBJECTVIEWER_H */

