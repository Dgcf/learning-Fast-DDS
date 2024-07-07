#include "simpleplot.h"
namespace Ui {
namespace Plot {
SimplePlot::~SimplePlot() noexcept
{
    MyMessage msgEnd;
    msgEnd.type = 0;
    this->push(msgEnd);
    m_thread.join();
}

SimplePlot::SimplePlot(QWidget *parent)
    : QCustomPlot(parent)
    , m_thread(&SimplePlot::threadFunc, this)
    , nIndexPre_(-1)
    , nIndexNext_(-1)
    , fCurPosX_(0.0)
    , fCurPosY_(0.0)
    , fInitRangeX(10)
    , fInitRangeY(10)
    , pVerticalLine_(nullptr)
    , pHorizontalLine_(nullptr)
{
    pVerticalLine_ = new QCPItemLine(this);
    if (pVerticalLine_) {
        pVerticalLine_->setPen(QPen(QColor(255, 51, 0)));
        pVerticalLine_->setVisible(false);
    }
    pHorizontalLine_ = new QCPItemLine(this);
    if (pHorizontalLine_) {
        pHorizontalLine_->setPen(QPen(QColor(255, 51, 0)));
        pHorizontalLine_->setVisible(false);
    }
    // configure scroll bars:
    // Since scroll bars only support integer values, we'll set a high default range of -500..500 and
    // divide scroll bar position values by 100 to provide a scroll range -5..5 in floating point
    // axis coordinates. if you want to dynami  cally grow the range accessible with the scroll bar,
    // just increase the minimum/maximum values of the scroll bars as needed.
    this->axisRect()->setBackground(QColor(245, 249, 255));

    this->xAxis->grid()->setPen(QPen(QColor(165, 174, 209), 1, Qt::DotLine)); // 网格线(对应刻度)画笔
    this->yAxis->grid()->setPen(QPen(QColor(165, 174, 209), 1, Qt::DotLine));
    this->xAxis->grid()->setSubGridPen(QPen(QColor(165, 174, 209), 1, Qt::DotLine)); // 子网格线(对应子刻度)画笔
    this->yAxis->grid()->setSubGridPen(QPen(QColor(165, 174, 209), 1, Qt::DotLine));
    this->xAxis->grid()->setSubGridVisible(true); // 显示子网格线
    this->yAxis->grid()->setSubGridVisible(true);

    //----------------------------------------------------------------------------------------
    this->yAxis2->setBasePen(QPen(QColor(165, 174, 209), 1));
    this->yAxis2->setTickPen(QPen(QColor(165, 174, 209), 1));
    this->yAxis2->setSubTickPen(QPen(QColor(165, 174, 209), 1));

    this->xAxis2->setBasePen(QPen(QColor(165, 174, 209), 1));
    this->xAxis2->setTickPen(QPen(QColor(165, 174, 209), 1));
    this->xAxis2->setSubTickPen(QPen(QColor(165, 174, 209), 1));

    this->yAxis->setBasePen(QPen(QColor(165, 174, 209), 1));
    this->yAxis->setTickPen(QPen(QColor(165, 174, 209), 1));
    this->yAxis->setSubTickPen(QPen(QColor(165, 174, 209), 1));

    this->xAxis->setBasePen(QPen(QColor(165, 174, 209), 1));
    this->xAxis->setTickPen(QPen(QColor(165, 174, 209), 1));
    this->xAxis->setSubTickPen(QPen(QColor(165, 174, 209), 1));

    //----------------------------------------------------------------------------------------
    this->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    this->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);

    this->xAxis->grid()->setZeroLinePen(QPen(QColor(165, 174, 209))); // x轴0线颜色白色
    this->yAxis->grid()->setZeroLinePen(QPen(QColor(165, 174, 209))); // y轴0线颜色白色

    //-----------------------------------------------------------------------------------------

    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // create connection between axes and scroll bars:
    // connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
    // connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(vertScrollBarChanged(int)));
    // connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
    // connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisChanged(QCPRange)));

    // initialize axis range (and scroll bar positions via signals we just connected):
    this->xAxis->setRange(0, 10, Qt::AlignCenter);
    this->yAxis->setRange(0, 10, Qt::AlignCenter);

    //--------------------------------------------------------------------------------------------
    this->addGraph();
    this->graph()->setPen(QColor(47, 104, 212));
    // pPlot_->graph()->setBrush(QBrush(QColor(0, 0, 255, 20)));

    this->graph()->setLineStyle(QCPGraph::lsLine); // 设置图表线段的风格
    this->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    connect(this, &SimplePlot::Draw, this, &SimplePlot::updateGraph);
}

void SimplePlot::push(const MyMessage &msg)
{
    std::unique_lock<std::mutex> lck(mtx_);
    queueTask_.push(msg);
    cv_.notify_all();
}

void SimplePlot::wait(MyMessage &msg)
{
    std::unique_lock<std::mutex> lck(mtx_);
    // while (!queue_.size())
    cv_.wait(lck, [&] { return !queueTask_.empty(); });
    msg = queueTask_.front();
    queueTask_.pop();
}

void SimplePlot::threadFunc()
{
    MyMessage msg;
    while (1) {
        // 等待队列的消息
        this->wait(msg);
        if (msg.type == 0)
            break;
        {
            std::unique_lock<std::mutex> lck(mtx_);
            vecCacheX_.append(msg.fValueX);
            vecCacheY_.append(msg.fValueY);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // QMetaObject::invokeMethod(this, "updateGraph", Qt::QueuedConnection);
        emit this->Draw();
    }
    qDebug() << "thread exited\n" << endl;
};

void SimplePlot::updateGraph()
{
    if (!graph()) {
        return;
    }
    std::unique_lock<std::mutex> lck(mtx_);
    this->graph()->setData(vecCacheX_, vecCacheY_);
    this->replot();
}

void SimplePlot::mouseMoveEvent(QMouseEvent *event)
{
    QCustomPlot::mouseMoveEvent(event);
    // 获取当前鼠标坐标点
    if (graph()->data()->isEmpty()) {
        return;
    }

    QPointF mousePos = event->localPos();

    // 将坐标转换为图表坐标系下的值
    fCurPosX_ = this->xAxis->pixelToCoord(mousePos.x());
    fCurPosY_ = this->yAxis->pixelToCoord(mousePos.y());

    //---------------------------------------------------------------------------------
    if (fCurPosX_ > xAxis->range().lower && fCurPosX_ < xAxis->range().upper && fCurPosY_ > yAxis->range().lower &&
        fCurPosY_ < yAxis->range().upper) {
        for (int i = 0; i < graph()->dataCount(); ++i) {

            if (graph()->dataMainKey(i) - fCurPosX_ < 0) {

                nIndexPre_ = i;

            } else {

                nIndexNext_ = i;
                break;
            }
        }

        if (nIndexNext_ < nIndexPre_) {
            return;
        }
        int nTmp =
            (abs(graph()->dataMainKey(nIndexPre_) - fCurPosX_) < abs(graph()->dataMainKey(nIndexNext_) - fCurPosX_)
                 ? nIndexPre_
                 : nIndexNext_);

        fCurPosX_ = graph()->dataMainKey(nTmp);
        fCurPosY_ = graph()->dataMainValue(nTmp);

        pVerticalLine_->start->setCoords(fCurPosX_, yAxis->range().lower);
        pVerticalLine_->end->setCoords(fCurPosX_, yAxis->range().upper);
        pHorizontalLine_->start->setCoords(this->xAxis->range().lower, fCurPosY_);
        pHorizontalLine_->end->setCoords(this->xAxis->range().upper, fCurPosY_);
        pVerticalLine_->setVisible(true);
        pHorizontalLine_->setVisible(true);
        // QToolTip::hideText();
        QToolTip::showText(event->globalPos(),
                           QString(QStringLiteral("key: %1  --  value: %2"))
                               .arg(graph()->dataMainKey(nTmp))
                               .arg(graph()->dataMainValue(nTmp)),
                           this, rect());
        this->replot();
    }
}

void SimplePlot::wheelEvent(QWheelEvent *event)
{
    QCustomPlot::wheelEvent(event);
    if (graph()->data()->isEmpty()) {
        return;
    }
    // QPoint mousePos = event->pos();

    // // 将坐标转换为图表坐标系下的值
    // fCurPosX_ = this->xAxis->pixelToCoord(mousePos.x());
    // fCurPosY_ = this->yAxis->pixelToCoord(mousePos.y());

    // 设置十字架线的位置
    // pVerticalLine_->start->setCoords(fCurPosX_, this->yAxis->range().lower);
    // pVerticalLine_->end->setCoords(fCurPosX_, this->yAxis->range().upper);
    // pHorizontalLine_->start->setCoords(this->xAxis->range().lower, fCurPosY_);
    // pHorizontalLine_->end->setCoords(this->xAxis->range().upper, fCurPosY_);

    // // 设置十字架线可见
    // pVerticalLine_->setVisible(true);
    // pHorizontalLine_->setVisible(true);

    // 重新绘制图表
    //    this->replot();

    QPointF mousePos = event->pos();

    // 将坐标转换为图表坐标系下的值
    fCurPosX_ = this->xAxis->pixelToCoord(mousePos.x());
    fCurPosY_ = this->yAxis->pixelToCoord(mousePos.y());

    //---------------------------------------------------------------------------------
    if (fCurPosX_ > xAxis->range().lower && fCurPosX_ < xAxis->range().upper && fCurPosY_ > yAxis->range().lower &&
        fCurPosY_ < yAxis->range().upper) {
        for (int i = 0; i < graph()->dataCount(); ++i) {

            if (graph()->dataMainKey(i) - fCurPosX_ < 0) {

                nIndexPre_ = i;

            } else {

                nIndexNext_ = i;
                break;
            }
        }

        if (nIndexNext_ < nIndexPre_) {
            return;
        }
        int nTmp =
            (abs(graph()->dataMainKey(nIndexPre_) - fCurPosX_) < abs(graph()->dataMainKey(nIndexNext_) - fCurPosX_)
                 ? nIndexPre_
                 : nIndexNext_);

        fCurPosX_ = graph()->dataMainKey(nTmp);
        fCurPosY_ = graph()->dataMainValue(nTmp);

        pVerticalLine_->start->setCoords(fCurPosX_, yAxis->range().lower);
        pVerticalLine_->end->setCoords(fCurPosX_, yAxis->range().upper);
        pHorizontalLine_->start->setCoords(this->xAxis->range().lower, fCurPosY_);
        pHorizontalLine_->end->setCoords(this->xAxis->range().upper, fCurPosY_);
        pVerticalLine_->setVisible(true);
        pHorizontalLine_->setVisible(true);
        // QToolTip::hideText();
        QToolTip::showText(
            event->globalPos(),
            QString(QStringLiteral("X: %1 -- Y: %2")).arg(graph()->dataMainKey(nTmp)).arg(graph()->dataMainValue(nTmp)),
            this, rect());
        this->replot();
    }
}

void SimplePlot::leaveEvent(QEvent *e)
{
    QCustomPlot::leaveEvent(e);
    if (graph()->data()->isEmpty()) {
        return;
    }
    pVerticalLine_->setVisible(false);
    pHorizontalLine_->setVisible(false);
    this->replot();
}

void SimplePlot::keyPressEvent(QKeyEvent *event)
{
    QCustomPlot::keyPressEvent(event);
    if (graph()->data()->isEmpty()) {
        return;
    }
    bool bTmp = false;

    switch (event->key()) {
    case Qt::Key_Space: {
        bTmp = true;
        this->xAxis->setRange(-fInitRangeX / 2, fInitRangeX / 2);
        this->yAxis->setRange(-fInitRangeY / 2, fInitRangeY / 2);

        // if (pVerticalLine_->visible() && pHorizontalLine_->visible()) {
        //     // 设置十字架线的位置
        //     pVerticalLine_->start->setCoords(fCurPosX_, this->yAxis->range().lower);
        //     pVerticalLine_->end->setCoords(fCurPosX_, this->yAxis->range().upper);
        //     pHorizontalLine_->start->setCoords(this->xAxis->range().lower, fCurPosY_);
        //     pHorizontalLine_->end->setCoords(this->xAxis->range().upper, fCurPosY_);
        // }

        // this->replot();
        break;
    }
    case Qt::Key_Left: {
        bTmp = true;
        qDebug() << "x::" << this->xAxis->range().size() << endl;
        this->xAxis->moveRange(-1.00);

        break;
    }
    case Qt::Key_Up: {
        bTmp = true;
        qDebug() << "y::" << this->yAxis->range().size() << endl;
        this->yAxis->moveRange(1.00);

        break;
    }
    case Qt::Key_Right: {
        bTmp = true;
        this->xAxis->moveRange(1.00);

        break;
    }
    case Qt::Key_Down: {
        bTmp = true;
        this->yAxis->moveRange(-1.00);
        break;
    }
    default:
        break;
    }
    if (bTmp) {
        pVerticalLine_->setVisible(false);
        pHorizontalLine_->setVisible(false);
        this->replot();
    }

    return;
}
} // namespace Plot
} // namespace Ui
