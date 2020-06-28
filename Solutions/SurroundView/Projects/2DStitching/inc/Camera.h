#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <fstream>
#include <iostream>
#include <string.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "XMLParameters.h"

using namespace std;
using namespace cv;

struct CameraParameters
{ 		/* Intrinsic and extrinsic camera parameters */
	cv::Mat K; 						/* Camera matrix */
	cv::Mat distCoeffs; 			/* Distortion coefficients */
	cv::Mat rvec;					/* Rotation vector */
	cv::Mat tvec;					/* Translation vector */
};

class Camera
{
public:

	cv::Matx33d intrinsicMatrix;				// 摄像机内参数矩阵
	cv::Matx33d newIntrinsicMatrix;				// 新的摄像机内参数矩阵
	cv::Vec4d distortionCoeffs;					// 摄像机的4个畸变系数：k1,k2,k3,k4

	cv::Mat rotationVec;						// 旋转向量
	cv::Mat translationVec;						// 平移向量

public:
	Mat Defisheye(const Mat& image);

	void Init(const string& filename, const XMLParameters& param, int index);
	Mat GetMapx() { return xmap; };
	Mat GetMapy() { return ymap; };
	cv::Mat xmap, ymap;			/* X and Y maps for removing fisheye distortion */

private:
	bool GetCalibrateParam(const string& filename);
	bool GetUndistortMaps(cv::Mat& mapx, cv::Mat& mapy, const double& scale);
	void SetCameraImageSize(const cv::Size size);
	

private:
	CameraParameters cameraParam;

	cv::Size cameraImageSize;	/* The image size (width/height) */

	int index;					/* Camera index */
	
};


#endif