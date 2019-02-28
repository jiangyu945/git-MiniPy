#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QPainter>

#include <QThread>
#include <QCloseEvent>

#include <QMutex>       //锁
#include <QMutexLocker>

#include "v4l2Cap.h"
#include "workerthread.h"

#define    FPS    30         //帧率
#define  SHOW_WIDTH   390    //屏宽(仅指可显示宽度)
#define  SHOW_HEIGHT  272    //屏高(仅指可显示高度)

//目标索引框
#define  OBJ_X     120
#define  OBJ_Y     61
#define  OBJ_WIDTH    150
#define  OBJ_HEIGHT   150

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
    void SigToReadFrame(QImage img);

public slots:
    void doProcessOpenCam();  // 打开摄像头
    void doProcessCapture();  // 采集图片
    void doProcessDisplay(QImage img);  // 显示图片
    void doProcessViewImg();  // 预览图片
    void doProcessCloseCam(); // 关闭摄像头

protected:
    virtual void paintEvent(QPaintEvent*);

private:
    Ui::Widget *ui;

    QImage img;
    QPixmap p_img,sp_img,pp_img;
    QTimer *capTimer;  //采集控制定时器
    QTimer *showTimer; //刷新显示定时器
    int index;      //图片保存名称下标

    QThread worker; //次线程
    workerThread* workerObj; //次线程工作对象

    QTime t;

    void init();
    void startObjthread();
};

#endif // WIDGET_H
