#include "opencv_measure.h"

#define  PHY_LEN   114.0      //参照物较长边边长的物理长度


/*
 *  在图片上绘制文本信息
 *  @param  img   目标图片
 *  @param  text  文本信息
 *  @param  origin 文本绘制位置
 *  @return 无
 */
void PaintText(Mat& img, char* text, Point origin)
{
    int fontface = CV_FONT_HERSHEY_SIMPLEX;   //字体
    double fontscale = 1.0;                   //尺寸因子，值越大文字越大
    Scalar textcolor = Scalar(0, 0, 255);    //文本颜色
    int thickness = 4;  //线宽
    int linetype = 8;   //线型

                        //int baseline;

                        //获取文本框的长宽
                        //Size text_size = getTextSize(text, fontface, fontscale, thickness, &baseline);

    putText(img, text, origin, fontface, fontscale, textcolor, thickness, linetype);
}


/*
 *  寻找最小轮廓，绘制矩形及尺寸标注
 *  @param  threshold_output   二值图像
 *  @param  src  原始图片
 *  @param  minArea   最小轮廓阈值
 *  @return Mat  返回值，Mat类型图像数据
 */
Mat fineMinAreaRect(Mat &threshold_output, Mat &src,int minArea)
{
    bool flag = false;   //查找比例尺标志
    double  pp = 0.0;    //比例因子：每像素代表的实际物理尺寸

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    //寻找轮廓(只查找最外层轮廓)，输入必须为二值图像
    findContours(threshold_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0)); //CV_CHAIN_APPROX_SIMPLE

    //第一轮筛选：移除过短的轮廓
    unsigned int cmin = 100; //最小轮廓长度
    unsigned int cmax = 5000; //最大轮廓长度
    vector<vector<Point>>::iterator itc = contours.begin();
    while (itc != contours.end())
    {
        if (itc->size() < cmin || itc->size() > cmax)
            itc = contours.erase(itc);
        else
            ++itc;
    }

    //第二轮筛选：剔除面积小于指定阈值的轮廓
   //剔除小面积轮廓
    itc = contours.begin();
    for (; itc != contours.end();)
    {
        double g_dConArea = fabs(contourArea(*itc));
        if (g_dConArea < minArea)
        {
            itc = contours.erase(itc);
        }
        else
        {
            ++itc;
        }
    }

    //对每个找到的轮廓创建可倾斜的边界框
    vector<RotatedRect> minRect(contours.size());
    for (unsigned int i = 0; i < contours.size(); i++)
    {
        minRect[i] = minAreaRect(Mat(contours[i]));
    }

    //绘出轮廓及其可倾斜的边界框
    //Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3);

    //先遍历寻找参照物，计算出比例因子
    for (unsigned int i = 0; i < contours.size(); i++)
    {
        Point2f rect_points[4];
        minRect[i].points(rect_points);

        //寻找比例尺，四顶点均位于左1/3图
        if (!flag && rect_points[0].x < src.cols / 3 && rect_points[1].x < src.cols / 3 && rect_points[2].x < src.cols / 3
            && rect_points[3].x < src.cols / 3)
        {
            flag = true;
            double length1 = sqrt(pow((rect_points[1].x - rect_points[0].x), 2) + pow((rect_points[1].y - rect_points[0].y), 2));
            double length2 = sqrt(pow((rect_points[2].x - rect_points[1].x), 2) + pow((rect_points[2].y - rect_points[1].y), 2));
            if (length1 <= length2) {
                pp = PHY_LEN / length2;
                break;
            }
            else {
                pp = PHY_LEN / length1;
                break;
            }
        }
    }

    double length;          //矩形边长
    Point paint_point;      //中点位置
    char text[20] = { 0 };  //文本字符串数组

    //再次遍历，绘制外接矩形
    for (unsigned int i = 0; i< contours.size(); i++)
    {
        Scalar color = Scalar(255, 255, 255);  //白色轮廓线
        //绘制轮廓
        drawContours(src, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point());
        Point2f rect_points[4];
        minRect[i].points(rect_points);

        //for (int j = 0; j < 4; j++) {
        //	line(drawing, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 255, 0), 2, 8);  //绿色矩形框
        //}

        //分别绘制矩形的四条边
        //第一条边
        line(src, rect_points[0], rect_points[1], Scalar(0, 255, 0), 2, 8);  //绿色边长
        length = pp* (sqrt(pow((rect_points[1].x - rect_points[0].x), 2) + pow((rect_points[1].y - rect_points[0].y), 2)));  //计算长度
        sprintf(text, "%.3lfmm", length);   //转为字符串

        paint_point.x = (rect_points[0].x + rect_points[1].x) / 2;    //寻找中点
        paint_point.y = (rect_points[0].y + rect_points[1].y) / 2;
        PaintText(src, text, paint_point);   //尺寸标注

        //第二条边
        line(src, rect_points[1], rect_points[2], Scalar(255, 0, 0), 2, 8);  //蓝色边长
        length = pp* (sqrt(pow((rect_points[2].x - rect_points[1].x), 2) + pow((rect_points[2].y - rect_points[1].y), 2)));
        sprintf(text, "%.3lfmm", length);
        paint_point.x = (rect_points[1].x + rect_points[2].x) / 2;
        paint_point.y = (rect_points[1].y + rect_points[2].y) / 2;
        PaintText(src, text, paint_point);

        //第三、四条边
        line(src, rect_points[2], rect_points[3], Scalar(0, 255, 0), 2, 8);
        line(src, rect_points[3], rect_points[0], Scalar(255, 0, 0), 2, 8);

    }


    return src;
    //结果显示
    //imshow("测量结果", src);
}

/*
 *  尺寸测量
 *  @param  origin   输入图像
 *  @param  minArea   最小轮廓阈值
 *  @return Mat  返回值，Mat类型图像数据
 */
Mat opencv_measure(Mat& origin,int minArea)
{
    Mat  src,m_src,src_gray, edge;

    if(origin.empty())
    {
        std::cout << "Original image is empty!" << std::endl;
    }

    src = origin.clone(); //深拷贝

    GaussianBlur(src, m_src, Size(3, 3), BORDER_DEFAULT);  //高斯模糊
                                                         //blur(src, src, Size(3, 3)); //均值滤波
                                                         //medianBlur(src, src, 3);   //中值滤波
    //经调试发现：此处使用双边滤波会导致系统崩溃，原因未知！！！
    //bilateralFilter(src, m_src, 25, 25 * 2, 25 / 2);     //保留边缘效果最好,双边滤波不支持in-place操作
                                                         //imshow("滤波效果", src);
    //（图像卷积运算）滤波器。（很重要，可增强对比度，否则部分背光区域边缘轮廓无法找到）
    Mat kernel = (Mat_<int>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    filter2D(m_src, m_src, -1, kernel, Point(-1, -1), 0);

    //形态学闭运算
    //定义核
    //Mat element = getStructuringElement(MORPH_RECT, Size(7, 7));
    //morphologyEx(edge, edge, MORPH_CLOSE, element);
    cvtColor(m_src, src_gray, CV_BGR2GRAY);  //转为灰度图像

    Canny(src_gray, edge, 150, 450, 3);  //边缘检测,得到二值图像
    //imshow("边缘检测", edge);

    return fineMinAreaRect(edge, origin, minArea);  //寻找最小外接矩形
}

