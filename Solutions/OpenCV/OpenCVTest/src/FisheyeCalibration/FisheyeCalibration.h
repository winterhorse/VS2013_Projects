#pragma once

#include <opencv2\opencv.hpp>
#include <fstream>

#include "SearchContours.h"

using namespace std;
using namespace cv;

typedef struct Line_ {		// Line description y = alpha * x + beta
	double alpha;
	double beta;
}Line;

class FisheyeCalibration
{
public:
	FisheyeCalibration();
	~FisheyeCalibration();

	/* 提取角点 */
	bool FindCorners(const Mat& src, vector<Point2f>& corners);

	/* 画出检测到的角点 */
	void DrawCorners(const Mat& src, const vector<Point2f>& corners);

	/* 初始化定标板上角点的三维坐标 假设z=0 */
	void SetObjectPoints(vector<vector<Point3f>>& points);

	/* 鱼眼矫正 */
	void FisheyeCameraCalibration(Matx33d& intrinsicMatrix, Vec4d& distortionCoeffs, vector<Vec3d>& rotationVectors, vector<Vec3d>& translationVectors);

	/* 评价矫正结果 */
	void EvaluateCalibrateRes();

	/* 保存矫正结果到文件 */
	void SaveCalibrateRes(const string& filename);

	/* 读取矫正结果 */
	bool ReadCalibrateRes(const string& filename);

	/* 重映射 */
	Mat ImageRemap(const Mat& image, const double& scale);

	// 运行矫正过程
	void RunCalibration();

	// 鱼眼矫正并保存结果到文件
	bool CalibrateAndSave2File(const int& totalImgNumber, const string& filename);

	// defisheye
	void Defisheye();

	Mat GetPerspectiveImgLeft(Mat& image);

	int GetImagePoints(cv::Mat &undist_img, uint num, std::vector<cv::Point2f> &img_points);

	void RunStitching();

	Mat GetPerspectiveImg(Mat& image);

	// 矫正评价

private:
	const Size boardSize = Size(7, 7);		//定标板上每行、列的角点数
	//const Size boardSize = Size(9, 6);		//定标板上每行、列的角点数
	const int  imageNum = 13;				//输入鱼眼图片的数量
	vector<vector<Point2f>>  cornersSeq;	//保存检测到的所有角点
	vector<Mat>  imageSeq;					//保存可以检测到角点的图片

	Size squareSize = Size(20, 20);
	vector<vector<Point3f>>  objectPoints;	// 保存定标板上角点的三维坐标

	Matx33d intrinsicMatrix;				// 摄像机内参数矩阵
	Matx33d newIntrinsicMatrix;				// 新的摄像机内参数矩阵
	Vec4d distortionCoeffs;					// 摄像机的4个畸变系数：k1,k2,k3,k4
	vector<Vec3d> rotationVectors;          // 每幅图像的旋转向量
	vector<Vec3d> translationVectors;		// 每幅图像的平移向量

	Mat rotationVec;						// 旋转向量
	Mat translationVec;						// 平移向量

	SearchContours contours;

	Line line_left[4];
	Line line_right[4];
	int cnt = 0;

private:
	inline Line GetAlphaBeta(cv::Point2f p1, cv::Point2f p2)
	{
		Line result;
		if (p1.x == p2.x) // If line is x = X, then alpha = INFINITY and beta = X.
		{
			result.alpha = INFINITY;
			result.beta = p1.x;
		}
		else // Otherwise
		{
			result.alpha = (p2.y - p1.y) / (p2.x - p1.x);
			result.beta = (p1.y * p2.x - p2.y * p1.x) / (p2.x - p1.x);
		}
		return result;
	}
	// 判断输入的点是否为有效的数据点，即需要拷贝的点
	inline bool IsValidPoint(const Point2f& p, const Line& line_left, const Line& line_right)
	{
		// 当点在图像内部且满足y <= y_left && y <= y_right才需要拷贝
		double y_left = line_left.alpha*p.x + line_left.beta;
		double y_right = line_right.alpha*p.x + line_right.beta;

		if ((p.y <= y_left) && (p.y <= y_right))
		{
			return true;
		}
		
		return false;
	}

	inline void CutImage(Mat& image, const Line& line_left, const Line& line_right)
	{
		// 沿着特征点剪切	
		for (int row = 0; row < image.rows; row++)
		{
			for (int col = 0; col < image.cols; col++)
			{
				if (IsValidPoint(Point2f(col, row), line_left, line_right))
				{
					image.at<Vec3b>(row, col)[0] = image.at<Vec3b>(row, col)[0];
					image.at<Vec3b>(row, col)[1] = image.at<Vec3b>(row, col)[1];
					image.at<Vec3b>(row, col)[2] = image.at<Vec3b>(row, col)[2];
				}
				else
				{
					image.at<Vec3b>(row, col)[0] = 0;
					image.at<Vec3b>(row, col)[1] = 0;
					image.at<Vec3b>(row, col)[2] = 0;
				}
			}
		}
	}
};

