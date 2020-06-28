//#include "opencv2/opencv.hpp"
//#include <iostream>
//
//using namespace std;
//using namespace cv;
//
//int main(){
//	Mat src = imread("./img/test.jpg");
//
//	if (src.empty()){
//		cout << "can not load picture" << endl;
//
//		return -1;
//	}
//
//	Mat dst, mapx1, mapy1;
//	dst.create(src.size(), src.type());
//	mapx1.create(src.size(), CV_32FC1);
//	mapy1.create(src.size(), CV_32FC1);
//	
//	for (int j = 0; j < src.rows; j++)
//	{
//		for (int i = 0; i < src.cols; i++)
//		{
//			mapx1.at<float>(j, i) = static_cast<float>(src.cols - i);
//			mapy1.at<float>(j, i) = static_cast<float>(src.rows - j);
//		}
//	}
//
//	remap(src, dst, mapx1, mapy1, INTER_LINEAR);
//
//	namedWindow("LearnOpenCV", CV_WINDOW_AUTOSIZE);
//	imshow("LearnOpenCV", dst);
//
//	waitKey(0);
//	return 0;
//}