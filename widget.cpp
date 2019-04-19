#include "widget.h"
#include "ui_widget.h"

static float whitePixel = 255.0f;  //白色像素值
QMutex myMutex;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    qRegisterMetaType<QImage>("QImage");
    qRegisterMetaType<QPixmap>("QPixmap");

    mpShadeWindow1 = new QWidget(this); //遮罩窗口1
    mpShadeWindow2 = new QWidget(this); //遮罩窗口2
    mpShadeWindow3 = new QWidget(this); //遮罩窗口3
    mpShadeWindow4 = new QWidget(this); //遮罩窗口4

    cwb_flag = false;
    line_flag = false;
    minArea = 50;

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
    ui->Bt_View->setEnabled(false);      //预览
    ui->Bt_RemoveTF->setEnabled(false);    //移除TF卡
    ui->Bt_CamClose->setEnabled(false);  //关闭

    //曝光调节滑动条
    ui->ExposureSlider->setMinimum(0);     //设置滑动条控件的最小值
    ui->ExposureSlider->setMaximum(1000);   //设置滑动条控件的最大值
    ui->ExposureSlider->setSingleStep(50); //设置步长
    ui->ExposureSlider->setValue(300);     //设置滑动条控件的初始值

    //最小轮廓阈值调节滑动条
    ui->Slider_MinArea->setMinimum(0);     //设置滑动条控件的最小值
    ui->Slider_MinArea->setMaximum(1000);   //设置滑动条控件的最大值
    ui->Slider_MinArea->setSingleStep(50); //设置步长
    ui->Slider_MinArea->setValue(50);     //设置滑动条控件的初始值

    capTimer = new QTimer(this);
    showTimer =new QTimer(this);
    index = 0;   //初始化图片保存名下标

    //槽连接
    connect(ui->Bt_CamOpen,SIGNAL(clicked(bool)),this,SLOT(doProcessOpenCam()));  //打开摄像头
    connect(ui->Bt_View,SIGNAL(clicked(bool)),this,SLOT(doProcessViewImg()));  //预览图片
    connect(ui->Bt_RemoveTF,SIGNAL(clicked(bool)),this,SLOT(doProcessRemoveTfcard()));  //移除TF卡
    connect(ui->Bt_CamClose,SIGNAL(clicked(bool)),this,SLOT(doProcessCloseCam())); //关闭摄像头,退出程序
    connect(capTimer,SIGNAL(timeout()),this,SLOT(doProcessCapture()));    //利用定时器超时控制采集频率
    connect(showTimer,SIGNAL(timeout()),this,SLOT(update()));    //定时刷新显示,update()触发painter刷新画面

    connect(ui->Bt_DrawDivLine,SIGNAL(clicked(bool)),this,SLOT(doDrawDivLine()));  //画测量分界线
    connect(ui->Bt_Cwb,SIGNAL(clicked(bool)),this,SLOT(doProcessCalibrateWB()));   //获取自定义白平衡矫正白色点的像素值
    connect(ui->ExposureSlider, SIGNAL(valueChanged(int)), this, SLOT(setExposureValue(int)));   //当拖动滑动条并释放时，发出信号，调节曝光
    connect(ui->Slider_MinArea,SIGNAL(valueChanged(int)),this,SLOT(adjustMinArea(int)));  //调整最小轮廓面积
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
    strftime(nowtime,sizeof(nowtime),"%Y/%m/%d_%H:%M:%S",ptr); //时间格式化
    nowtime[sizeof(nowtime)-1] = '\0';

    ui->lineEdit_TipShow->setText(nowtime); //设置文本内容
    ui->lineEdit_TipShow->setAlignment(Qt::AlignCenter);  //居中对齐

    //判断当前时间是否准确
    if(!(nowtime[2]>49 || nowtime[3]>50)){  //49、50分别为“1”、“2”的ASCII码值
        ui->Bt_CamOpen->setEnabled(false);
    }
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
        //qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
       // qDebug() << "ERROR: Mat could not be converted to QImage.";
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

//获取灰度值
float Widget::getGrayPixel(QPixmap sp_img)
{
    QImage c_img = sp_img.toImage();
    Mat mat = QImage2cvMat(c_img);        //此处得到的Mat对象格式类型为CV_8UC4
    cvtColor(mat,mat,CV_RGBA2RGB);

    std::vector<Mat> imageRGB;

    //RGB三通道分离
    split(mat, imageRGB);

    //求原始图像的RGB分量的均值
    double R, G, B;
    B = mean(imageRGB[0])[0];
    G = mean(imageRGB[1])[0];
    R = mean(imageRGB[2])[0];

    //取灰度均值
    float grayPix = (B + G + R) / 3;
    if(grayPix > 127.5){
        grayPix = 127.5;
    }
   // printf("grayPix = %.2f\n",grayPix);

    //调整RGB三个通道各自的值
    imageRGB[0] = grayPix;
    imageRGB[1] = grayPix;
    imageRGB[2] = grayPix;

    //RGB三通道图像合并
    merge(imageRGB, mat);

//    //对话框显示效果
//    QMessageBox showbox(QMessageBox::NoIcon, NULL, NULL, QMessageBox::Ok);
//    QImage grayc_img = cvMat2QImage(mat);   //Mat 转 QImage
//    QPixmap grayc_pix = QPixmap::fromImage(grayc_img);

//    QPixmap scl_pix = grayc_pix.scaled(360,162,Qt::KeepAspectRatio);  //适度缩放,优化显示效果
//    showbox.setIconPixmap(scl_pix);    //显示

//    if( showbox.exec()== QMessageBox::Ok )
//    {
//    }
    QString data = QString("白平衡矫正成功！ 灰度值: %1").arg(grayPix); //WB Calibrate succeed!
    ui->lineEdit_TipShow->setText(data);
    return grayPix;
}

//自定义白平衡灰卡矫正
void Widget::doProcessCalibrateWB()
{
    QMutexLocker locker(&myMutex); //加锁
    QPixmap cwb_pix = p_img.copy(OBJ_X*WIDTH/SHOW_WIDTH-70+(OBJ_WIDTH*WIDTH/SHOW_WIDTH/4),OBJ_Y*HEIGHT/SHOW_HEIGHT-300+(OBJ_HEIGHT*HEIGHT/SHOW_HEIGHT/4),            //抓取目标区域图像
               OBJ_WIDTH*WIDTH/SHOW_WIDTH/2,OBJ_HEIGHT*HEIGHT/SHOW_HEIGHT/2);  //涉及图像大小比例转换!
    float grayPixel = getGrayPixel(cwb_pix);  //获取灰色像素值
    whitePixel = 2*grayPixel;

    cwb_flag = true;
}

//白平衡色彩矫正
QImage wb_calibrate(QPixmap sp_img)
{
    QImage c_img = sp_img.toImage();
    Mat mat = QImage2cvMat(c_img);        //此处得到的Mat对象格式类型为CV_8UC4
    cvtColor(mat,mat,CV_RGBA2RGB);        //此步骤特别重要！使CV_8UC4的RGBA转为CV_8UC3的RGB

    /***********************注意：下面图像处理要求Mat格式类型必须为CV_8UC3!!!*********************/
    const float inputMin = 0.0f;
    const float inputMax = 255.0f;
    const float outputMin = 0.0f;
    const float outputMax = whitePixel;

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

    merge(bgr, mat); //合并R、G、B三通道


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
        msg.about(NULL,"错误","打开摄像头失败! 请检查摄像头是否连接正确！");
        return;
    }

    //设置摄像头参数
    set_cap_para();

    //获取摄像头参数
    get_cap_para();
    //内存映射初始化
    init_mmap();
    //开启视频流
    start_cap();
    //加入camera监听
    epoll_cam();

    ui->Bt_CamOpen->setEnabled(false);
    ui->Bt_RemoveTF->setEnabled(true);
    ui->Bt_CamClose->setEnabled(true);

    capTimer->start(1000.000/(2*FPS));    //定时发送采集请求,时间给到显示频率的一半，降低显示延迟
}

//调节曝光时间
void Widget::setExposureValue(int value)
{
    int ex_value = ui->ExposureSlider->value();
    setExposureTime(ex_value);
    QString s_value = QString::number(ex_value, 10);  //int转string,10代表十进制
    QString str = "曝光值:  ";  //Exposure Value
    ui->lineEdit_TipShow->setText(str.append(s_value));
}

//调整最小轮廓面积
void Widget::adjustMinArea(int value)
{
    minArea = ui->Slider_MinArea->value();

    int min_value = ui->Slider_MinArea->value();
    QString s_value = QString::number(min_value, 10);  //int转string,10代表十进制
    QString str = "最小轮廓面积:  ";
    ui->lineEdit_TipShow->setText(str.append(s_value));
}

//采集图片
void Widget::doProcessCapture()
{
    emit SigToReadFrame();
}

//图片存储类型转换
void Widget::doProcessDisplay(QImage rd_img)
{
    ui->Bt_View->setEnabled(true);

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

    if(line_flag){
        //画分界线(1/3)
        QPen penMid(QPen(Qt::red,2,Qt::DashDotDotLine)); //设置画笔形式
        mypainter.setPen(penMid);
        mypainter.drawLine(OBJ_X+OBJ_WIDTH/3,OBJ_Y,OBJ_X+OBJ_WIDTH/3,OBJ_Y+OBJ_HEIGHT);
    }


    //开启四个遮罩窗口
    QString str("QWidget{background-color:rgba(0,0,0,0.6);}");  //透明度0-1
    mpShadeWindow1->setStyleSheet(str);
    mpShadeWindow1->setGeometry(PIX_X, PIX_Y, SHOW_WIDTH, (SHOW_HEIGHT-OBJ_HEIGHT)/2);
    //mpShadeWindow->setWindowOpacity(0.6);
    mpShadeWindow1->show();

    mpShadeWindow2->setStyleSheet(str);
    mpShadeWindow2->setGeometry(PIX_X, PIX_Y+SHOW_HEIGHT-(SHOW_HEIGHT-OBJ_HEIGHT)/2, SHOW_WIDTH, (SHOW_HEIGHT-OBJ_HEIGHT)/2);
    mpShadeWindow2->show();

    mpShadeWindow3->setStyleSheet(str);
    mpShadeWindow3->setGeometry(PIX_X, PIX_Y+(SHOW_HEIGHT-OBJ_HEIGHT)/2, (SHOW_WIDTH-OBJ_WIDTH)/2, OBJ_HEIGHT);
    mpShadeWindow3->show();

    mpShadeWindow4->setStyleSheet(str);
    mpShadeWindow4->setGeometry(PIX_X+SHOW_WIDTH-(SHOW_WIDTH-OBJ_WIDTH)/2, PIX_Y+(SHOW_HEIGHT-OBJ_HEIGHT)/2, (SHOW_WIDTH-OBJ_WIDTH)/2, OBJ_HEIGHT);
    mpShadeWindow4->show();
}


//绘制图像索引框
void Widget::doDrawDivLine()
{
    line_flag = !line_flag;
}

//预览并保存图片
void Widget::doProcessViewImg()
{    
    capTimer->stop();   //停止后台采集
    showTimer->stop();  //停止显示刷新

    //构造带时间戳的图片名
    struct tm *ptr;
    time_t t;
    char outfile[100];
    time(&t);
    ptr = localtime(&t);
    strftime(outfile,sizeof(outfile),"/media/mmcblk1p1/pictures/Greein_%Y%m%d_%H%M%S",ptr);
    strcat(outfile,".jpg");

    //未进行白平衡矫正
    if(!cwb_flag){
        //预览对话框
        QMessageBox noCwbBox(QMessageBox::NoIcon, NULL,"提示：未进行色彩矫正！", QMessageBox::Yes | QMessageBox::No);
        noCwbBox.setButtonText(QMessageBox::Yes,"保存");     //保存
        noCwbBox.setButtonText(QMessageBox::No,"取消");    //取消
        noCwbBox.setDefaultButton(QMessageBox::No);

        myMutex.lock();  //加锁
        sp_img = p_img.copy(OBJ_X*WIDTH/SHOW_WIDTH-70,OBJ_Y*HEIGHT/SHOW_HEIGHT-300,            //抓取目标区域图像
                            OBJ_WIDTH*WIDTH/SHOW_WIDTH,OBJ_HEIGHT*HEIGHT/SHOW_HEIGHT);  //涉及图像大小比例转换!

        pp_img = sp_img.scaled(480,222,Qt::KeepAspectRatio);  //适度缩放,优化显示效果
        noCwbBox.setIconPixmap(pp_img);    //显示

        myMutex.unlock();  //解锁
        //保存myMutex.unlock();  //解锁
        if(noCwbBox.exec() == QMessageBox::Yes)
        {
            //pp_img = sp_img.scaled(WIDTH,HEIGHT,Qt::IgnoreAspectRatio);  //放大为采集分辨率
            bool ret = sp_img.save(outfile);   //保存图片
            if(!ret){
                ui->lineEdit_TipShow->setText("警告！ 保存失败！！！"); //Warning! Save Failed!!!
                ui->lineEdit_TipShow->setAlignment(Qt::AlignCenter);
            }
            else{
                ui->lineEdit_TipShow->setText("图片保存成功");  //Save succeed!
                ui->lineEdit_TipShow->setAlignment(Qt::AlignCenter);
            }
        }
    }

    //已进行白平衡矫正
   else{
        //预览对话框
        QMessageBox msgbox(QMessageBox::NoIcon, NULL, NULL, QMessageBox::Yes | QMessageBox::No |QMessageBox::Ok);
        msgbox.setButtonText(QMessageBox::Yes,"保存");
        msgbox.setButtonText(QMessageBox::Ok,"测量");
        msgbox.setButtonText(QMessageBox::No,"取消");
        msgbox.setDefaultButton(QMessageBox::No);

        myMutex.lock();  //加锁
        sp_img = p_img.copy(OBJ_X*WIDTH/SHOW_WIDTH-70,OBJ_Y*HEIGHT/SHOW_HEIGHT-300,            //抓取目标区域图像
                            OBJ_WIDTH*WIDTH/SHOW_WIDTH,OBJ_HEIGHT*HEIGHT/SHOW_HEIGHT);  //涉及图像大小比例转换!
        //色彩矫正
        QImage cal_img = wb_calibrate(sp_img);
        QPixmap mmp_img = QPixmap::fromImage(cal_img);

        pp_img = mmp_img.scaled(480,222,Qt::KeepAspectRatio);  //适度缩放,优化显示效果

        msgbox.setIconPixmap(pp_img);    //显示

        myMutex.unlock();  //解锁

        //保存
        if(msgbox.exec() == QMessageBox::Yes)
        {
            //pp_img = sp_img.scaled(WIDTH,HEIGHT,Qt::IgnoreAspectRatio);  //放大为采集分辨率
            bool ret = cal_img.save(outfile);   //保存图片
            if(!ret){
                ui->lineEdit_TipShow->setText("警告！ 保存失败！！！");
                ui->lineEdit_TipShow->setAlignment(Qt::AlignCenter);
            }
            else{
                ui->lineEdit_TipShow->setText("保存成功！");
                ui->lineEdit_TipShow->setAlignment(Qt::AlignCenter);
            }
        }

        //尺寸测量
        if(msgbox.exec() == QMessageBox::Ok)
        {
            Mat src = QImage2cvMat(cal_img);        //QImage 转 Mat
            cvtColor(src,src,CV_RGBA2RGB);        //此步骤特别重要！使CV_8UC4的RGBA转为CV_8UC3的RGB

            Mat m_src = opencv_measure(src,minArea);             //尺寸测量
            QImage mat_img = cvMat2QImage(m_src);        //Mat 转 QImage
            QPixmap ms_img = QPixmap::fromImage(mat_img);  //QImage转QPixmap


            //显示
            QMessageBox msg_measure(QMessageBox::NoIcon, NULL, NULL, QMessageBox::Yes | QMessageBox::No );
            msg_measure.setButtonText(QMessageBox::Yes,"保存");    //保存
            msg_measure.setButtonText(QMessageBox::No,"取消");   //取消

            //msg_measure.setGeometry(0,32,340,240);

            QPixmap mm_img = ms_img.scaled(480,222,Qt::KeepAspectRatio);  //适度缩放,优化显示效果 360 222
            msg_measure.setIconPixmap(mm_img);    //将图片显示在对话框中

            //保存
            if(msg_measure.exec() == QMessageBox::Yes)
            {
                //QPixmap ss_img = ms_img.scaled(WIDTH,HEIGHT,Qt::IgnoreAspectRatio);  //放大为采集分辨率
                bool ret = ms_img.save(outfile);   //保存图片
                if(!ret){
                    ui->lineEdit_TipShow->setText("警告！ 保存失败！！！");
                    ui->lineEdit_TipShow->setAlignment(Qt::AlignCenter);
                }
                else{
                    ui->lineEdit_TipShow->setText("图片保存成功！");
                    ui->lineEdit_TipShow->setAlignment(Qt::AlignCenter);
                }
            }

         }
    }


    capTimer->start(1000.000/FPS);  //重启采集定时器
    showTimer->start(1000.000/FPS);  //重启显示刷新
}


//移除TF卡
void Widget::doProcessRemoveTfcard()
{
    int ret = system("umount /media/mmcblk1p1");
    if(ret != -1){
        ui->lineEdit_TipShow->setText("移除TF卡成功！");  //Remove TF Card succeed!
        ui->lineEdit_TipShow->setAlignment(Qt::AlignCenter);
        ui->Bt_RemoveTF->setEnabled(false);
    }

}

//关闭摄像头
void Widget::doProcessCloseCam()
{
    //关闭对话框
    QMessageBox quitMsg;
    quitMsg.setText("关闭提示"); //Exit tip
    quitMsg.setInformativeText("确认退出？");  //Do you really want exit ?
    quitMsg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    quitMsg.setButtonText(QMessageBox::Yes,"是");    //保存
    quitMsg.setButtonText(QMessageBox::No,"否");   //取消
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
            //qApp->quit(); //退出应用程序 (存在问题:执行后程序并未退出!!!)
            break;
        case QMessageBox::No:
            break;
        default:
            break;
    }
}
