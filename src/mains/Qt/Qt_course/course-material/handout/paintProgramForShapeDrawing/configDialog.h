#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QSpinBox;

class ConfigDialog :public QDialog
{
    Q_OBJECT

public:
    ConfigDialog( QWidget* parent );

private slots:
    void slotBrushTypeChanged( int );
    void slotEmitData();

signals:
    void setPen( int lineWidth, Qt::PenStyle penStyle );
    void setBrush( Qt::BrushStyle brushStyle );
    void setBrush( const QPixmap& pixmap );
    void useGradientBrush();

private:
    QLabel* _styleLabel;
    QLabel* _pixmapLabel;
    QSpinBox* _lineWidth;
    QComboBox* _lineStyle;
    QComboBox* _brushType;
    QComboBox* _brushStyle;
    QLineEdit* _pixmap;
    QLabel* _gradientLabel;
    QComboBox* _gradient;
};


#endif /* CONFIGDIALOG_H */

