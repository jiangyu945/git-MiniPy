#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QDebug>
#include <QPainter>

#include <QThread>

#include <QMutex>       //锁
#include <QMutexLocker>

#include "v4l2Cap.h"
#include "workerthread.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
signals:
    void SigToReadFrame();

public slots:
    void doProcessOpenCam();  // 打开摄像头
    void doProcessCapture();  // 采集图片
    void doProcessDisplay(unsigned char* buf,unsigned int len);  // 显示图片
    void doProcessSaveImg();  // 保存图片
    void doProcessCloseCam(); // 关闭摄像头

private:
    Ui::Widget *ui;

    QImage img;
    QTimer *timer;  //计时用
    int index;      // 图片下标

    QThread worker; //次线程
    workerThread* workerObj;

    void init();
    void startObjthread();
};

#endif // WIDGET_H
