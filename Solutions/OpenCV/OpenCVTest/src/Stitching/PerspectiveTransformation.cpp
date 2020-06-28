//#include "opencv2/opencv.hpp"
//#include <iostream>
//
//using namespace std;
//using namespace cv;
//
///*
//OpenCV中透视变换的又分为两种:
//1. 稀疏透视变换 2. 密集透视变换
//我们经常提到的对图像的透视变换都是指密集透视变换，而稀疏透视变换在OpenCV的特征点匹配之后的特征对象区域标识中经常用到。
//一般情况下密集透视变换warpPerspective函数常与函数getPerspectiveTransform一起使用实现对图像的透视校正
//而稀疏透视变换perspectiveTransform经常与findhomography一起使用。
//
//*/
//
//vector<Point2f> srcTri(4);
//vector<Point2f> dstTri(4);
//int clickTimes = 0;  //在图像上单击次数
//Mat src, dst;
//Mat imageWarp;
//
//void onMouse(int event, int x, int y, int flags, void *utsc);
//
//int main()
//{
//	src = imread("./img/cali_1.jpg");
//	namedWindow("src", CV_WINDOW_AUTOSIZE);
//	imshow("src", src);
//
//	setMouseCallback("src", onMouse);
//
//	waitKey();
//
//	return 0;
//
//	
//	//Mat src = imread("./img/cali_1.jpg");
//	//if (src.empty()){
//	//	cout << "can not load picture" << endl;	
//	//	return -1;
//	//}
//	//
//	////namedWindow("src", CV_WINDOW_AUTOSIZE);
//	////imshow("src", src);
//
//	//vector<Point2f>src_coners(4);
//	//src_coners[0] = Point2f(0, 0);
//	//src_coners[1] = Point2f(0, src.rows);
//	//src_coners[2] = Point2f(src.cols, 0);
//	//src_coners[3] = Point2f(src.cols, src.rows);
//	//circle(src, src_coners[0], 3, Scalar(0, 0, 255), 3, 8);
//	//circle(src, src_coners[1], 3, Scalar(0, 0, 255), 3, 8);
//	//circle(src, src_coners[2], 3, Scalar(0, 0, 255), 3, 8);
//	//circle(src, src_coners[3], 3, Scalar(0, 0, 255), 3, 8);
//
//	//imshow("src", src);
//
//	//vector<Point2f>dst_coners(4);
//	//dst_coners[0] = Point2f(0, 0);
//	//dst_coners[1] = Point2f(0, src.rows);
//	//dst_coners[2] = Point2f(src.cols, 0);
//	//dst_coners[3] = Point2f(src.cols, src.rows);
//
//	//Mat warpMatrix = getPerspectiveTransform(src_coners, dst_coners);
//	//Mat dst;
//	//warpPerspective(src, dst, warpMatrix, dst.size(), INTER_LINEAR, BORDER_CONSTANT);
//
//	//imshow("dst", dst);
//
//	//waitKey();
//	//return 0;
//}
//
//
//void onMouse(int event, int x, int y, int flags, void *utsc)
//{
//	if (event == CV_EVENT_LBUTTONUP)   //响应鼠标左键事件
//	{
//		circle(src, Point(x, y), 2, Scalar(0, 0, 255), 2);  //标记选中点
//		imshow("wait ", src);
//		srcTri[clickTimes].x = x;
//		srcTri[clickTimes].y = y;
//		clickTimes++;
//
//		cout << "x: " << x << " y: " << y << endl;
//	}
//	if (clickTimes == 4)
//	{
//		dstTri[0].x = 0;
//		dstTri[0].y = 0;
//
//		dstTri[1].x = 282;
//		dstTri[1].y = 0;
//
//		dstTri[2].x = 282;
//		dstTri[2].y = 438;
//
//		dstTri[3].x = 0;
//		dstTri[3].y = 438;
//
//		//Mat H = findHomography(srcTri, dstTri, RANSAC);//计算透视矩阵
//		//warpPerspective(src, dst, H, Size(282, 438));//图像透视变换
//
//		Mat warpMatrix = getPerspectiveTransform(srcTri, dstTri);
//		//warpPerspective(src, dst, warpMatrix, dst.size(), INTER_LINEAR, BORDER_CONSTANT);
//		warpPerspective(src, dst, warpMatrix, Size(282, 438));//图像透视变换
//
//		imshow("output", dst);
//	}
//}
