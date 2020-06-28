#ifndef BIRD_EYE_VIEW_HPP
#define BIRD_EYE_VIEW_HPP

#include <opencv2/opencv.hpp>
#include <iostream>

#include "XMLParameters.h"
#include "Camera.h"

#define CAMERA_NUMBERS		4
#define KEY_POINT_NUMBERS	8

using namespace cv;
using namespace std;

class BirdEyeView
{
public:
	BirdEyeView();
	~BirdEyeView();

	bool SearchKeyPoints(cv::Mat *srcMat);
	bool Init(const char* configFile, const XMLParameters& param, Camera* camera);
	Mat TransformView(Mat* v);
	void GetTransfromView(const Mat& src, Mat& dst, const int& cameraIndex);
	Mat Run2DStitching(const Mat* images);

private:
	bool ReadSrcPoint(const char* srcPointFile = "srcPoint.yml");
	bool SaveSrcPoint(const char* configFile);	
	void SetTargetImageSize(const XMLParameters& param);
	void SetTargetPoint();
	void SetTargetMask();
	void GetTransfromMat(cv::Mat *tansform);
	void GetTransfromMaps(cv::Mat *mapx, cv::Mat *mapy);
	void GetDefisheyeAndTransfromMaps(cv::Mat *mapx, cv::Mat *mapy, Camera* camera);

	void DrawStitchingLine(Mat& image);
	void StitchingSeamFusion(Mat& left, Mat& right, Mat& dst);

	void PerspectiveToMaps(const cv::Mat &perspectiveMat, const cv::Size imgSize, 
		cv::Mat &mapx, cv::Mat &mapy);

	void MergeMaps(const cv::Size imageSize,
		const cv::Mat& mapxUndistorted,
		const cv::Mat& mapyUndistorted,
		const cv::Mat& mapxUndistortedToBird,
		const cv::Mat& mapyUndistortedToBird,
		cv::Mat& mergedMapx,
		cv::Mat& mergedMapy);

	void Remap_cpu(const Mat& src, Mat& dst, const Mat& mapx, const Mat& mapy);

private:
	vector<vector<Point2f> > targetPoint, sourcePoint;

	cv::Size exShift, inShift, chessBord, outImgSize, carSize;
	int maskHeight;
	cv::Rect roi[CAMERA_NUMBERS];
	cv::Rect carImagePos;
	cv::Mat birdtransform[CAMERA_NUMBERS], maskF, maskB;
	cv::Mat mapx[CAMERA_NUMBERS], mapy[CAMERA_NUMBERS];
	cv::Mat mergedMapx[CAMERA_NUMBERS], mergedMapy[CAMERA_NUMBERS];
	cv::Mat carImage, newCarImage;
	cv::Size imageSize = cv::Size(1280, 800);

private:
	typedef struct Line_
	{		// Line description y = alpha * x + beta
		double alpha;
		double beta;
	}Line;

	inline Line GetAlphaBeta(cv::Point2f p1, cv::Point2f p2)
	{
		Line result;
		if (p1.x == p2.x) // If line is x = X, then alpha = INFINITY and beta = X.
		{
			result.alpha = INFINITY;
			result.beta = p1.x;
		} else // Otherwise
		{
			result.alpha = (p2.y - p1.y) / (p2.x - p1.x);
			result.beta = (p1.y * p2.x - p2.y * p1.x) / (p2.x - p1.x);
		}
		return result;
	}

};

#endif
