#pragma once

#include <opencv2\opencv.hpp>
#include <fstream>
using namespace std;
using namespace cv;

/*******************************************************************************************
* Macros
*******************************************************************************************/
#define MAX_CONTOUR_APPROX  7
#define CONTOURS_NUM 4

#define MIN4(a,b,c,d)  (((a <= b) & (a <= c) & (a <= d)) ? (a) : \
						(((b <= c) & (b <= d)) ? (b) : \
						(((c <= d)) ? (c) : (d))))

#define MAX4(a,b,c,d)  (((a >= b) & (a >= c) & (a >= d)) ? (a) : \
						(((b >= c) & (b >= d)) ? (b) : \
						(((c >= d)) ? (c) : (d))))

/*******************************************************************************************
* Types
*******************************************************************************************/
struct CvContourEx
{
	CV_CONTOUR_FIELDS()
		int counter;
};

class SearchContours
{
public:
	SearchContours();
	~SearchContours();
	int GetContours(const cv::Mat &img, CvSeq** root, CvMemStorage *storage, int min_size);
	void FilterContours(CvSeq** root);
	void SortContours(CvSeq** root);
	void GetFeaturePoints(CvSeq** root, std::vector<cv::Point2f> &feature_points, cv::Point2f shift);
	void sec2vector(CvSeq** root, std::vector<cv::Point2f> &feature_points, cv::Point2f shift);

	int GetImagePoints(cv::Mat &undist_img, uint num, std::vector<cv::Point2f> &img_points, int min_size);
};

