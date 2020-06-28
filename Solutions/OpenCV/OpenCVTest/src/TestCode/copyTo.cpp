//#include "opencv2/opencv.hpp"
//#include <iostream>
//
//using namespace std;
//using namespace cv;
//
//int main(){
//	Mat log = imread("./img/nxp.jpg");
//	Mat background = imread("./img/lena.jpg");
//		
//	//imshow("background", background);
//	//imshow("log", log);
//
//	Mat roi = background(Rect(10, 10, log.cols, log.rows));
//	Mat mask(roi.rows, roi.cols, roi.depth(), Scalar(1));
//	//Mat mask = log.clone();
//
//	log.copyTo(roi, mask);
//
//	imshow("background2", background);
//
//	waitKey(0);
//	return 0;
//}