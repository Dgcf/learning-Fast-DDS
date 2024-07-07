#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QScrollBar>
#include "qcustomplot.h"
#include "simpleplot.h"

class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void setupPlot();

protected:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void horzScrollBarChanged(int value);
    void vertScrollBarChanged(int value);
    void xAxisChanged(QCPRange range);
    void yAxisChanged(QCPRange range);

private:
    QScrollBar *pVer_;
    QScrollBar *pHor_;

    QCPItemLine *pVerticalLine_;
    QCPItemLine *pHorizontalLine_;
    Ui::Plot::SimplePlot *pPlot_;

    QVector<double> x_;
    QVector<double> y_;
    QPushButton *pbtn;
    QPushButton *pbtn2;
};
#endif // WIDGET_H
