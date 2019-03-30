#include "widget.h"
#include "ui_widget.h"


QMutex myMutex;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    qRegisterMetaType<QImage>("QImage");

    mpShadeWindow1 = new QWidget(this); //遮罩窗口1
    mpShadeWindow2 = new QWidget(this); //遮罩窗口2
    mpShadeWindow3 = new QWidget(this); //遮罩窗口3
    mpShadeWindow4 = new QWidget(this); //遮罩窗口4

    init();  //初始化
    startObjthread(); //启动多线程
    //显示当前时间,确保网络时间获取成功
    showTime();
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
    //按钮状态初始化
    ui->Bt_CamOpen->setEnabled(true);    //打开
    ui->Bt_SWB->setEnabled(false);       //WB选择
    ui->Bt_ViewImg->setEnabled(false);   //预览
    ui->Bt_CamClose->setEnabled(false);  //关闭
    ui->Bt_remove->setEnabled(false);    //移除TF卡


    capTimer = new QTimer(this);
    showTimer =new QTimer(this);
    index = 0;   //初始化图片保存名下标

    //槽连接
    connect(ui->Bt_CamOpen,SIGNAL(clicked(bool)),this,SLOT(doProcessOpenCam()));  //打开摄像头
    connect(ui->Bt_ViewImg,SIGNAL(clicked(bool)),this,SLOT(doProcessViewImg()));  //预览图片
    connect(ui->Bt_CamClose,SIGNAL(clicked(bool)),this,SLOT(doProcessCloseCam())); //关闭摄像头,退出程序
    connect(capTimer,SIGNAL(timeout()),this,SLOT(doProcessCapture()));    //利用定时器超时控制采集频率
    connect(showTimer,SIGNAL(timeout()),this,SLOT(update()));    //定时刷新显示,update()触发painter刷新画面

    connect(ui->Bt_SWB,SIGNAL(clicked(bool)),this,SLOT(doProcessSelectWB()));   //选择白平衡模式
    connect(ui->Bt_remove,SIGNAL(clicked(bool)),this,SLOT(doProcessRemoveTfcard()));  //移除TF卡
}

//开启线程
void Widget::startObjthread()
{
    workerObj = new workerThread;    //创建一个object
    workerObj->moveToThread(&worker);  //将对象移动到线程
    connect(this,SIGNAL(SigToReadFrame()),workerObj,SLOT(doProcessReadFrame()));  //图像采集

    //线程结束后自动销毁
    connect(&worker,SIGNAL(finished()),workerObj,SLOT(deleteLater()));   //资源回收

    connect(workerObj,SIGNAL(SigToDisplay(QImage)),this,SLOT(doProcessDisplay(QImage)));  //图片存储类型转换

    //启动线程
    worker.start();
}

//显示当前时间
void Widget::showTime()
{
    struct tm *ptr;
    time_t t;
    char nowtime[20];
    time(&t);
    ptr = localtime(&t);
    strftime(nowtime,sizeof(nowtime),"%Y/%m/%d_%H:%M:%S",ptr);
    nowtime[sizeof(nowtime)-1] = '\0';

    ui->lineEdit_wbShow->setText(nowtime); //设置文本内容
    ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);  //居中对齐
}

//Mat转QImage
QImage cvMat2QImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

//QImage转Mat
cv::Mat QImage2cvMat(QImage image)
{
    cv::Mat mat;
    switch(image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    default:
        break;
    }
    return mat;
}


//白平衡矫正
QImage wb_calibrate(QPixmap sp_img)
{
    QImage c_img = sp_img.toImage();
    Mat mat = QImage2cvMat(c_img);        //此处得到的Mat对象格式类型为CV_8UC4
    cvtColor(mat,mat,CV_RGBA2RGB);        //此步骤特别重要！使CV_8UC4的RGBA转为CV_8UC3的RGB
   // Mat wb_src(mat.size(),mat.type());

    /***********************注意：下面图像处理要求Mat格式类型必须为CV_8UC3!!!*********************/
    const float inputMin = 0.0f;
    const float inputMax = 255.0f;
    const float outputMin = 0.0f;
    const float outputMax = WB_VALUE;

    std::vector<Mat> bgr;
    split(mat, bgr);        //分离B、G、R三通道

    // 在rgb三通道上分别计算直方图
    // 将1%的最大值和最小值设置为255和0
    // 其余值映射到(0, 255), 这样使得每个通道的值在rgb中分布较均匀, 以实现简单的颜色平衡
    /********************* Simple white balance *********************/
    float s1 = 1.0f;//最低值百分比：取1%
    float s2 = 1.0f;//最高值百分比：取1%

    int depth = 2;// depth of histogram tree
    int bins = 16;// number of bins at each histogram level
    int total = bgr[0].cols * bgr[0].rows;
    int nElements = int(pow((float)bins, (float)depth));// number of elements in histogram tree

    for (size_t k = 0; k < bgr.size(); ++k)
    {
        std::vector<int> hist(nElements, 0);
        uchar *pImg = bgr[k].data;
        // histogram filling
        for (int i = 0; i < total; i++)
        {
            int pos = 0;
            float minValue = inputMin - 0.5f;
            float maxValue = inputMax + 0.5f;
            float interval = float(maxValue - minValue) / bins;

            uchar val = pImg[i];
            for (int j = 0; j < depth; ++j)
            {
                int currentBin = int((val - minValue + 1e-4f) / interval);
                ++hist[pos + currentBin];

                pos = (pos + currentBin)*bins;
                minValue = minValue + currentBin*interval;
                interval /= bins;
            }
        }

        int p1 = 0, p2 = bins - 1;
        int n1 = 0, n2 = total;
        float minValue = inputMin - 0.5f;
        float maxValue = inputMax + 0.5f;
        float interval = float(maxValue - minValue) / bins;

        // searching for s1 and s2
        for (int j = 0; j < depth; ++j)
        {
            while (n1 + hist[p1] < s1 * total / 100.0f)
            {
                n1 += hist[p1++];
                minValue += interval;
            }
            p1 *= bins;

            while (n2 - hist[p2] > (100.0f - s2) * total / 100.0f)
            {
                n2 -= hist[p2--];
                maxValue -= interval;
            }
            p2 = p2*bins - 1;

            interval /= bins;
        }

        bgr[k] = (outputMax - outputMin) * (bgr[k] - minValue) / (maxValue - minValue) + outputMin;
    }

    cv::merge(bgr, mat); //合并R、G、B三通道


    QImage cc_img = cvMat2QImage(mat);   //Mat 转 QImage
    return  cc_img;
}
/**************************************************************************************************************************/


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
    get_cap_para();
    //内存映射初始化
    init_mmap();
    //开启视频流
    start_cap();
    //加入camera监听
    epoll_cam();

    ui->Bt_CamOpen->setEnabled(false);
    ui->Bt_SWB->setEnabled(false);
    ui->Bt_remove->setEnabled(true);
    ui->Bt_CamClose->setEnabled(true);

    capTimer->start(1000.000/30);    //定时发送采集请求
}

//白平衡模式设置
void Widget::doProcessSelectWB()
{

    QMessageBox msgbox(QMessageBox::NoIcon, NULL,"Please select white balance mode : ",
                       QMessageBox::Yes | QMessageBox::YesAll | QMessageBox::Save |
                       QMessageBox::Ok | QMessageBox::Open | QMessageBox::No |
                       QMessageBox::Cancel);
    msgbox.setButtonText(QMessageBox::Yes,"Mor1h");      //日出一小时  3500K
    msgbox.setButtonText(QMessageBox::YesAll,"Mor2h");   //日出两小时  4700K
    msgbox.setButtonText(QMessageBox::Save,"Cloudy");    //阴天       7000K
    msgbox.setButtonText(QMessageBox::Ok,"Sunny");       //正午阳光    6000K
    msgbox.setButtonText(QMessageBox::Open,"Tung");      //钨丝灯      3000K
    msgbox.setButtonText(QMessageBox::No,"Flus");        //日光灯      6000K
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
    emit SigToReadFrame();
}

//图片存储类型转换
void Widget::doProcessDisplay(QImage rd_img)
{
    ui->Bt_ViewImg->setEnabled(true);

    p_img = QPixmap::fromImage(rd_img);  //QImage转化为QPixmap

    showTimer->start(1000.000/FPS); //开启显示定时器
}

 //QPainter绘制图像
void Widget::paintEvent(QPaintEvent *)
{
    QPainter mypainter(this);

    QMutexLocker locker(&myMutex); //加锁,主要目的是锁住全局的p_img,防止绘制期间被篡改
    //绘制图形
    mypainter.drawPixmap(PIX_X,PIX_Y,SHOW_WIDTH,SHOW_HEIGHT,p_img);


    //画目标索引框
    QPen pen(QPen(Qt::green,2,Qt::DashDotDotLine)); //设置画笔形式
    mypainter.setPen(pen);
    mypainter.drawRect(OBJ_X,OBJ_Y,OBJ_WIDTH,OBJ_HEIGHT);

    //画分界线(1/3)
    QPen penMid(QPen(Qt::red,2,Qt::DashDotDotLine)); //设置画笔形式
    mypainter.setPen(penMid);
    mypainter.drawLine(OBJ_X+OBJ_WIDTH/3,OBJ_Y,OBJ_X+OBJ_WIDTH/3,OBJ_Y+OBJ_HEIGHT);

    //开启四个遮罩窗口
    QString str("QWidget{background-color:rgba(0,0,0,0.7);}");  //透明度0-1
    mpShadeWindow1->setStyleSheet(str);
    mpShadeWindow1->setGeometry(0, 28, 390, OBJ_Y-28);
    //mpShadeWindow->setWindowOpacity(0.6);
    mpShadeWindow1->show();

    mpShadeWindow2->setStyleSheet(str);
    mpShadeWindow2->setGeometry(0, OBJ_Y, OBJ_X, OBJ_HEIGHT);
    mpShadeWindow2->show();

    mpShadeWindow3->setStyleSheet(str);
    mpShadeWindow3->setGeometry(0, OBJ_Y+OBJ_HEIGHT, 390, OBJ_Y-28);
    mpShadeWindow3->show();

    mpShadeWindow4->setStyleSheet(str);
    mpShadeWindow4->setGeometry(OBJ_X+OBJ_WIDTH, OBJ_Y, OBJ_X, OBJ_HEIGHT);
    mpShadeWindow4->show();
}

//预览并保存图片
void Widget::doProcessViewImg()
{
    //停止后台采集
    capTimer->stop();

    //构造带时间戳的图片名
    struct tm *ptr;
    time_t t;
    char outfile[100];
    time(&t);
    ptr = localtime(&t);
    strftime(outfile,sizeof(outfile),"/media/mmcblk1p1/pictures/Greein_%Y%m%d_%H%M%S",ptr);
    strcat(outfile,".jpg");

    //预览对话框
    QMessageBox msgbox(QMessageBox::NoIcon, NULL, NULL, QMessageBox::Yes | QMessageBox::No |QMessageBox::Ok);
    msgbox.setButtonText(QMessageBox::Yes,"Save");     //保存
    msgbox.setButtonText(QMessageBox::Ok,"Measure");   //测量
    msgbox.setButtonText(QMessageBox::No,"Cancel");    //取消
    msgbox.setDefaultButton(QMessageBox::No);

    myMutex.lock();  //加锁
    sp_img = p_img.copy(OBJ_X*WIDTH/SHOW_WIDTH,OBJ_Y*HEIGHT/SHOW_HEIGHT,            //抓取目标区域图像
                        OBJ_WIDTH*WIDTH/SHOW_WIDTH,OBJ_HEIGHT*HEIGHT/SHOW_HEIGHT);  //涉及图像大小比例转换!

    //色彩矫正
    QImage cal_img = wb_calibrate(sp_img);

    QPixmap mmp_img = QPixmap::fromImage(cal_img);

    pp_img = mmp_img.scaled(360,222,Qt::IgnoreAspectRatio);  //适度缩放,优化显示效果

    msgbox.setIconPixmap(pp_img);    //显示

    myMutex.unlock();  //解锁

    //保存
    if(msgbox.exec() == QMessageBox::Yes)
    {
        //pp_img = sp_img.scaled(WIDTH,HEIGHT,Qt::IgnoreAspectRatio);  //放大为采集分辨率
        bool ret = cal_img.save(outfile);   //保存图片
        if(!ret){
            ui->lineEdit_wbShow->setText("Warning! Save Failed!!!");
            ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);
        }
        else{
            ui->lineEdit_wbShow->setText("Save succeed!");
            ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);
        }
    }

    //尺寸测量
    if(msgbox.exec() == QMessageBox::Ok)
    {
        Mat src = QImage2cvMat(cal_img);        //QImage 转 Mat
        cvtColor(src,src,CV_RGBA2RGB);        //此步骤特别重要！使CV_8UC4的RGBA转为CV_8UC3的RGB
        Mat m_src = opencv_measure(src);             //尺寸测量
        QImage mat_img = cvMat2QImage(m_src);        //Mat 转 QImage
        QPixmap ms_img = QPixmap::fromImage(mat_img);  //QImage转QPixmap


        //显示
        QMessageBox msg_measure(QMessageBox::NoIcon, NULL, NULL, QMessageBox::Yes | QMessageBox::No );
        msg_measure.setButtonText(QMessageBox::Yes,"Save");    //保存
        msg_measure.setButtonText(QMessageBox::Ok,"Cancel");   //取消

        //msg_measure.setGeometry(0,32,340,240);

        QPixmap mm_img = ms_img.scaled(360,222,Qt::IgnoreAspectRatio);  //适度缩放,优化显示效果 360 222
        msg_measure.setIconPixmap(mm_img);    //将图片显示在对话框中

        //保存
        if(msg_measure.exec() == QMessageBox::Yes)
        {
            //QPixmap ss_img = ms_img.scaled(WIDTH,HEIGHT,Qt::IgnoreAspectRatio);  //放大为采集分辨率
            bool ret = ms_img.save(outfile);   //保存图片
            if(!ret){
                ui->lineEdit_wbShow->setText("Warning! Save Failed!!!");
                ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);
            }
            else{
                ui->lineEdit_wbShow->setText("Save succeed!");
                ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);
            }
        }

     }



    //重启采集定时器
    capTimer->start(1000.000/30);
}

//移除TF卡
void Widget::doProcessRemoveTfcard()
{
    int ret = system("umount /media/mmcblk1p1");
    if(ret != -1){
        ui->lineEdit_wbShow->setText("Remove TF Card succeed!");
        ui->lineEdit_wbShow->setAlignment(Qt::AlignCenter);
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

            //ui->Bt_CamOpen->setEnabled(true);
//            qApp->quit(); //退出应用程序 (存在问题:执行后程序并未退出!!!)
            break;
        case QMessageBox::No:
            break;
        default:
            break;
    }
}
