#ifndef OPENCV_MEASURE_H
#define OPENCV_MEASURE_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <vector>

using namespace cv;
using namespace std;

void PaintText(cv::Mat& , char* ,cv::Scalar,int , cv::Point);
cv::Mat fineMinAreaRect(cv::Mat&, cv::Mat&,int);
cv::Mat opencv_measure(cv::Mat&,int);



#endif // OPENCV_MEASURE_H
