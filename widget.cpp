#include "widget.h"
#include "ui_widget.h"

QMutex myMutex;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    qRegisterMetaType<QImage>("QImage");

    init();  //初始化
    startObjthread(); //启动多线程
}

Widget::~Widget()
{
    delete ui;
}


/*=======================================Functions====================================*/
void Widget::init()
{
    ui->Bt_SaveImg->setEnabled(false);  //按钮状态初始化
    ui->Bt_CamClose->setEnabled(false);

    timer = new QTimer(this);
    t_show =new QTimer(this);
    index = 0;   //初始化图片保存名下标

//    //设置默认显示图片
//    QImage Initimg("./img/fire.jpg");
//    //图片自适应窗口标签大小
//    Initimg.scaled(ui->label_show->size(),Qt::IgnoreAspectRatio);
//    ui->label_show->setScaledContents(true);
//    ui->label_show->setPixmap(QPixmap::fromImage(Initimg));

    //槽连接
    connect(ui->Bt_CamOpen,SIGNAL(clicked(bool)),this,SLOT(doProcessOpenCam()));  //打开摄像头
    connect(ui->Bt_SaveImg,SIGNAL(clicked(bool)),this,SLOT(doProcessSaveImg()));  //保存图片
    connect(ui->Bt_CamClose,SIGNAL(clicked(bool)),this,SLOT(doProcessCloseCam())); //关闭摄像头
    connect(timer,SIGNAL(timeout()),this,SLOT(doProcessCapture()));    //利用定时器超时控制采集频率
    connect(t_show,SIGNAL(timeout()),this,SLOT(repaint()));    //定时刷新

//    connect(this,SIGNAL(SigToDisplay()),this,SLOT(doProcessDisplay()));  //发送信号,进行显示
}

//开启线程
void Widget::startObjthread()
{
    workerObj = new workerThread; //创建一个object
    workerObj->moveToThread(&worker);            //将对象移动到线程
    connect(this,SIGNAL(SigToReadFrame(QImage)),workerObj,SLOT(doProcessReadFrame(QImage)));  //图像采集

    //线程结束后自动销毁
    connect(&worker,SIGNAL(finished()),workerObj,SLOT(deleteLater()));    //资源回收

    connect(workerObj,SIGNAL(SigToDisplay(QImage)),this,SLOT(doProcessDisplay(QImage)));  //图像显示

    //启动线程
    worker.start();
}


/*========================================SLOT=========================================*/
//打开摄像头
void Widget::doProcessOpenCam(){

    //打开摄像头
    int ret = open_cam();
    if(ret == -1){
        QMessageBox msg;
        msg.about(NULL,"Error","Open Camera Failed!");
    }

    //设置帧格式
    set_cap_frame();
    //获取帧率
    get_fps();
    //内存映射初始化
    init_mmap();
    //开启视频流
    start_cap();

    //加入camera监听
    epoll_cam();

    ui->Bt_CamOpen->setEnabled(false);
    timer->start(1000.000/30);    //开启定时器,超时发出信号
}

//采集图片
void Widget::doProcessCapture()
{
    emit SigToReadFrame(img);
}

//图像显示
void Widget::doProcessDisplay(QImage img)
{
//    t.start();
    ui->Bt_SaveImg->setEnabled(true);
    ui->Bt_CamClose->setEnabled(true);

    p_img = QPixmap::fromImage(img);  //QImage转化为QPixmap

    t_show->start(1000.000/FPS); //开启显示定时器

}

 //QPainter绘制图像
void Widget::paintEvent(QPaintEvent *)
{
    QPainter mypainter(this);

    t.start();

    //加锁
    myMutex.lock();

    mypainter.drawPixmap(0,0,SHOW_WIDTH,SHOW_HEIGHT,p_img);

    //解锁
    myMutex.unlock();
    qDebug("绘制图片耗时: %d ms",t.elapsed());  //打印耗时
}

//保存图片
void Widget::doProcessSaveImg()
{
    index++;
    char outfile[50];
    sprintf(outfile, "./cap_test%d.jpg", index); //利用sprintf格式化图片名

    //消息对话框---图片保存
    QMessageBox msg;
    msg.setText("Picture has been captured!");
    msg.setInformativeText("Do you want to save it?");
    msg.setStandardButtons(QMessageBox::Cancel|QMessageBox::Save );
    msg.setDefaultButton(QMessageBox::Save);
    int ret = msg.exec();

    switch (ret) {
        case QMessageBox::Cancel:
            // Cancel was clicked
            break;
        case QMessageBox::Save:
            // Save was clicked
        //加锁
        myMutex.lock();
        p_img.save(outfile);
        //解锁
        myMutex.unlock();

            break;
        default:
            // should never be reached
            break;
      }

    ui->Bt_SaveImg->setEnabled(false);
}

//关闭摄像头
void Widget::doProcessCloseCam()
{
    ui->Bt_CamClose->setEnabled(false);

    timer->stop();   // 停止读取数据定时器
    t_show->stop();  //关闭显示定时器
    stop_cap();
    close_cam();

//    ui->label_show->setText("Camera has Closed!");
//    ui->label_show->setAlignment(Qt::AlignCenter);
    ui->Bt_CamOpen->setEnabled(true);

}
