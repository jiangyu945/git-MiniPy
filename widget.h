#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMessageBox>
#include <QImage>
#include <QTimer>
#include <QDebug>

#include "v4l2Cap.h"

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
    void SigToDisplay();

public slots:
    void doProcessOpenCam();  // 打开摄像头
    void doProcessCapture();  // 采集图片
    void doProcessDisplay();  // 显示图片
    void doProcessSaveImg();  // 保存图片
    void doProcessCloseCam(); // 关闭摄像头


private:
    Ui::Widget *ui;
    void init();

    QImage img;
    QTimer *timer;  //计时用
    int index;      // 图片下标
};

#endif // WIDGET_H
