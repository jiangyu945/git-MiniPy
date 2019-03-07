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
    //等待次线程的结束
    worker.quit();
    worker.wait();
    delete ui;
}


/*=======================================Functions====================================*/
void Widget::init()
{
    ui->Bt_SaveImg->setEnabled(false);  //按钮状态初始化
    ui->Bt_CamClose->setEnabled(false);

    capTimer = new QTimer(this);
    showTimer =new QTimer(this);
    index = 0;   //初始化图片保存名下标

    //槽连接
    connect(ui->Bt_CamOpen,SIGNAL(clicked(bool)),this,SLOT(doProcessOpenCam()));  //打开摄像头
    connect(ui->Bt_SaveImg,SIGNAL(clicked(bool)),this,SLOT(doProcessViewImg()));  //预览图片
    connect(ui->Bt_CamClose,SIGNAL(clicked(bool)),this,SLOT(doProcessCloseCam())); //关闭摄像头,退出程序
    connect(capTimer,SIGNAL(timeout()),this,SLOT(doProcessCapture()));    //利用定时器超时控制采集频率
    connect(showTimer,SIGNAL(timeout()),this,SLOT(update()));    //定时刷新显示,update()触发painter刷新画面

    connect(ui->Bt_SWB,SIGNAL(clicked(bool)),this,SLOT(doProcessSelectWB()));   //选择白平衡模式
}

//开启线程
void Widget::startObjthread()
{
    workerObj = new workerThread;    //创建一个object
    workerObj->moveToThread(&worker);  //将对象移动到线程
    connect(this,SIGNAL(SigToReadFrame(QImage)),workerObj,SLOT(doProcessReadFrame(QImage)));  //图像采集

    //线程结束后自动销毁
    connect(&worker,SIGNAL(finished()),workerObj,SLOT(deleteLater()));   //资源回收

    connect(workerObj,SIGNAL(SigToDisplay(QImage)),this,SLOT(doProcessDisplay(QImage)));  //图片存储类型转换

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
        return;
    }

    //设置摄像头参数
    set_cap_para();

    //显示初始白平衡模式
    ui->lineEdit_wbShow->setText("Now WB Mode : Sunny"); //设置文本内容
    ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);  //居中对齐
    //获取摄像头参数
//    get_cap_para();
    //内存映射初始化
    init_mmap();
    //开启视频流
    start_cap();
    //加入camera监听
    epoll_cam();

    ui->Bt_CamOpen->setEnabled(false);
    capTimer->start(1000.000/30);    //定时发送采集请求
}

//白平衡模式设置
void Widget::doProcessSelectWB()
{

    QMessageBox msgbox(QMessageBox::NoIcon, NULL,"Please select white balance mode : ",
                       QMessageBox::Yes | QMessageBox::YesAll | QMessageBox::Save |
                       QMessageBox::Ok | QMessageBox::Open | QMessageBox::No |
                       QMessageBox::Cancel);
    msgbox.setButtonText(QMessageBox::Yes,"Morn1h");      //日出一小时  3500K
    msgbox.setButtonText(QMessageBox::YesAll,"Morn2h");   //日出两小时  4700K
    msgbox.setButtonText(QMessageBox::Save,"Cloudy");    //阴天       7000K
    msgbox.setButtonText(QMessageBox::Ok,"Sunny");       //正午阳光    6000K
    msgbox.setButtonText(QMessageBox::Open,"Tungsten");  //钨丝灯      3000K
    msgbox.setButtonText(QMessageBox::No,"Fluorescent"); //日光灯      6000K
    msgbox.setButtonText(QMessageBox::Cancel,"Sunset");  //日落        2500K
    msgbox.setDefaultButton(QMessageBox::Ok);
    switch(msgbox.exec()){
        case QMessageBox::Yes:
            set_cap_wb(3500);
            ui->lineEdit_wbShow->setText("Now WB Mode : Morn1h"); //设置文本内容
            ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);  //居中对齐
            break;

        case QMessageBox::YesAll:
            set_cap_wb(4700);
            ui->lineEdit_wbShow->setText("Now WB Mode : Morn2h");
            ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);
            break;

        case QMessageBox::Save:
            set_cap_wb(7000);
            ui->lineEdit_wbShow->setText("Now WB Mode : Cloudy");
            ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);
            break;

        case QMessageBox::Ok:
            set_cap_wb(6000);
            ui->lineEdit_wbShow->setText("Now WB Mode : Sunny");
            ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);
            break;

        case QMessageBox::Open:
            set_cap_wb(3000);
            ui->lineEdit_wbShow->setText("Now WB Mode : Tungsten");
            ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);
            break;

        case QMessageBox::No:
            set_cap_wb(6000);
            ui->lineEdit_wbShow->setText("Now WB Mode : Fluorescent");
            ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);
            break;

        case QMessageBox::Cancel:
            set_cap_wb(2500);
            ui->lineEdit_wbShow->setText("Now WB Mode : Sunset");
            ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);
            break;

        default:
            break;
    }

    //获取摄像头参数
    get_cap_para();
}

//采集图片
void Widget::doProcessCapture()
{
    emit SigToReadFrame(img);
}

//图片存储类型转换
void Widget::doProcessDisplay(QImage img)
{
    ui->Bt_SaveImg->setEnabled(true);
    ui->Bt_CamClose->setEnabled(true);

    p_img = QPixmap::fromImage(img);  //QImage转化为QPixmap

    showTimer->start(1000.000/FPS); //开启显示定时器
}

 //QPainter绘制图像
void Widget::paintEvent(QPaintEvent *)
{
    QPainter mypainter(this);

    QMutexLocker locker(&myMutex); //加锁,主要目的是锁住全局的p_img,防止绘制期间被篡改
    //绘制图形
    mypainter.drawPixmap(PIX_X,PIX_Y,SHOW_WIDTH,SHOW_HEIGHT,p_img);
    //绘制图像拍摄指引框
    QPen pen(QPen(Qt::green,2,Qt::DashDotDotLine)); //设置画笔形式
    mypainter.setPen(pen);
    mypainter.drawRect(OBJ_X,OBJ_Y,OBJ_WIDTH,OBJ_HEIGHT);//画目标索引框
}

//预览并保存图片
void Widget::doProcessViewImg()
{
    //格式化图片保存名称
//    index++;
//    char outfile[50];
//    sprintf(outfile, "/app/cap_test%d.jpg", index);

    //图片名加入时间戳
    struct tm *ptr;
    time_t t;
    char outfile[51];
    time(&t);
    ptr = localtime(&t);
    strftime(outfile,sizeof(outfile),"/app/Greein_%Y%m%d_%H%M%S",ptr);
    strcat(outfile,".jpg");

    //预览对话框
    QMessageBox msgbox(QMessageBox::NoIcon, NULL, NULL, QMessageBox::Yes | QMessageBox::No);
    msgbox.setButtonText(QMessageBox::Yes,"Save");
    msgbox.setButtonText(QMessageBox::No,"Cancel");

    myMutex.lock();  //加锁
    sp_img = p_img.copy(OBJ_X*WIDTH/SHOW_WIDTH,OBJ_Y*HEIGHT/SHOW_HEIGHT,            //抓取目标区域图像
                        OBJ_WIDTH*WIDTH/SHOW_WIDTH,OBJ_HEIGHT*HEIGHT/SHOW_HEIGHT);  //涉及图像大小比例转换!
    myMutex.unlock();  //解锁
    pp_img = sp_img.scaled(360,222,Qt::IgnoreAspectRatio);  //适度缩放,优化显示效果
    msgbox.setIconPixmap(pp_img);    //将图片显示在对话框中

    //判断用户操作
    if(msgbox.exec() == QMessageBox::Yes)
    {
        pp_img = sp_img.scaled(WIDTH,HEIGHT,Qt::IgnoreAspectRatio);  //放大为采集分辨率
        pp_img.save(outfile);   //保存图片
    }
}

//关闭摄像头
void Widget::doProcessCloseCam()
{
    //关闭对话框
    QMessageBox quitMsg;
    quitMsg.setText("Exit tip");
    quitMsg.setInformativeText("Do you really want exit ?");
    quitMsg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    quitMsg.setDefaultButton(QMessageBox::Yes);
    int ret = quitMsg.exec();
    switch(ret){
        case QMessageBox::Yes:
            //处理关闭事宜
            capTimer->stop();   // 停止读取数据定时器
            showTimer->stop();  //关闭显示定时器
            stop_cap();   //失能视频流输出
            close_cam();  //关闭摄像头

            ui->Bt_CamClose->setEnabled(false);
            ui->Bt_CamOpen->setEnabled(true);
//            qApp->quit(); //退出应用程序 (存在问题:执行后程序并未退出!!!)
            break;
        case QMessageBox::No:
            break;
        default:
            break;
    }
}
