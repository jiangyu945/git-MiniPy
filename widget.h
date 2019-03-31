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
#include "opencv_measure.h"

#define WB_VALUE  180.0f     //白平衡矫正值

#define    FPS    30         //帧率
#define  SHOW_WIDTH   390    //屏宽(仅指可显示宽度)
#define  SHOW_HEIGHT  240    //屏高(仅指可显示高度)

//画布起始点
#define  PIX_X   0
#define  PIX_Y   32

//目标索引框
#define  OBJ_WIDTH    220
#define  OBJ_HEIGHT   180
#define  OBJ_X     85
#define  OBJ_Y     62

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
    void doProcessDisplay(QImage);  // 显示图片
    void doProcessViewImg();  // 预览图片
    void doProcessRemoveTfcard();  //移除TF卡

    void doProcessCloseCam(); // 关闭摄像头


protected:
    virtual void paintEvent(QPaintEvent*);

private:
    Ui::Widget *ui;
    QWidget* mpShadeWindow1;  //遮罩窗口1
    QWidget* mpShadeWindow2;  //遮罩窗口2
    QWidget* mpShadeWindow3;  //遮罩窗口3
    QWidget* mpShadeWindow4;  //遮罩窗口4


    QPixmap p_img,sp_img,pp_img;
    QTimer *capTimer;  //采集控制定时器
    QTimer *showTimer; //刷新显示定时器
    int index;      //图片保存名称下标

    QThread worker; //次线程
    workerThread* workerObj; //次线程工作对象

    QTime t;

    void init();    //初始化
    void startObjthread(); //开启多线程
    void showTime();    //日期显示
};

#endif // WIDGET_H
