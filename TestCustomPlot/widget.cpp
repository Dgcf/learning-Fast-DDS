#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , pVer_(nullptr)
    , pHor_(nullptr)
    , pVerticalLine_(nullptr)
    , pHorizontalLine_(nullptr)
    , pPlot_(nullptr)
    , pbtn(nullptr)
    , pbtn2(nullptr)
{
    // ui->horizontalScrollBar->setRange(-500, 500);
    // ui->verticalScrollBar->setRange(-500, 500);

    pVer_ = new QScrollBar(Qt::Vertical, this);
    pHor_ = new QScrollBar(Qt::Horizontal, this);

    pHor_->setRange(-500, 500);
    pVer_->setRange(-500, 500);

    pPlot_ = new Ui::Plot::SimplePlot(this);
    // setCentralWidget(pPlot_);
    if (pPlot_) {
        setupPlot();

        connect(pHor_, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
        connect(pVer_, SIGNAL(valueChanged(int)), this, SLOT(vertScrollBarChanged(int)));
        connect(pPlot_->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
        connect(pPlot_->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisChanged(QCPRange)));
    }

    pPlot_->axisRect()->setupFullAxesBox(true);
    double fMax = 0;
    // x_.reserve(200);
    // y_.reserve(200);

    static int a = 0;
    pbtn = new QPushButton("Push", this);
    pbtn->resize(60, 20);

    pbtn2 = new QPushButton("Clear", this);
    pbtn2->resize(60, 20);

    for (int i = 0; i < 200; ++i) {
        x_.append((i / 199.0 - 0.5) * 40);
        y_.append((qExp(-x_[i] * x_[i] * 0.25) * qSin(x_[i] * 5) * 5));
    }

    connect(pbtn2, &QPushButton::clicked, this, [&]() {
        pPlot_->ClearCache();
        pPlot_->clearPlottables();
        pPlot_->clearGraphs();
        pPlot_->replot();
        pPlot_->addGraph();
        pPlot_->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    });

    connect(pbtn, &QPushButton::clicked, this, [&]() {
        if (a > 48) {
            return;
        }
        for (int i = 0; i < 200; ++i) {
            MyMessage msg;
            msg.type = 1;
            msg.fValueX = x_[i];
            msg.fValueY = y_[i];
            pPlot_->push(msg);
        }

        // MyMessage msg;
        // msg.type = 1;
        // msg.fValueX = x_[a];
        // msg.fValueY = y_[a];
        // ++a;
        // pPlot_->push(msg);
    });
}

Widget::~Widget() {}

void Widget::setupPlot()
{
    // QCustomPlot
    //  The following plot setup is mostly taken from the plot demos:
    // 1
    // QVector<double> x(500), y0(500), y1(500);
    // for (int i = 0; i < 500; ++i) {
    //     x[i] = (i / 499.0 - 0.5) * 10;
    //     y0[i] = qExp(-x[i] * x[i] * 0.25) * qSin(x[i] * 5) * 5;
    //     y1[i] = qExp(-x[i] * x[i] * 0.25) * 5;
    // }
    // pPlot_->graph(0)->setData(x, y0);
    // pPlot_->replot();

    // 2

    // std::swap(x, y);

    // pPlot_->graph(0)->setData(x, y);
    //  pPlot_->xAxis->setRange(0, x.count(), Qt::AlignCenter);
    //  pPlot_->yAxis->setRange(0, fMax * 2, Qt::AlignCenter);

    // pPlot_->replot();
}

void Widget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    pPlot_->move(0, 0);
    pPlot_->resize(width() * 3 / 4, height() * 3 / 4);

    pHor_->move(0, height() * 3 / 4);
    pHor_->setFixedHeight(20);
    pHor_->setFixedWidth(pPlot_->width());

    pVer_->move(width() * 3 / 4, 0);
    pVer_->setFixedHeight(pPlot_->height());
    pVer_->setFixedWidth(20);

    pbtn->move(width() * 3 / 4 + 30, 5);
    pbtn2->move(width() * 3 / 4 + 30, 35);
}

void Widget::horzScrollBarChanged(int value)
{
    if (qAbs(pPlot_->xAxis->range().center() - value / 100.0)
        > 0.01) // if user is dragging plot, we don't want to replot twice
    {
        pPlot_->xAxis->setRange(value / 100.0, pPlot_->xAxis->range().size(), Qt::AlignCenter);
        pPlot_->replot();
    }
}

void Widget::vertScrollBarChanged(int value)
{
    if (qAbs(pPlot_->yAxis->range().center() + value / 100.0)
        > 0.01) // if user is dragging plot, we don't want to replot twice
    {
        pPlot_->yAxis->setRange(-value / 100.0, pPlot_->yAxis->range().size(), Qt::AlignCenter);
        pPlot_->replot();
    }
}

void Widget::xAxisChanged(QCPRange range)
{
    pHor_->setValue(qRound(range.center() * 100.0));  // adjust position of scroll bar slider
    pHor_->setPageStep(qRound(range.size() * 100.0)); // adjust size of scroll bar slider
}

void Widget::yAxisChanged(QCPRange range)
{
    pVer_->setValue(qRound(-range.center() * 100.0)); // adjust position of scroll bar slider
    pVer_->setPageStep(qRound(range.size() * 100.0)); // adjust size of scroll bar slider
}
