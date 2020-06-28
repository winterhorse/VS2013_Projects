#pragma once

#include <opencv2\opencv.hpp>
#include <fstream>

using namespace std;
using namespace cv;

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

	// 鱼眼矫正并保存结果到文件
	bool CalibrateAndSave2File(const int& totalImgNumber, const string& fisheyeFileName, const string& caliResFile);

	

	void RunCalibration();

	// defisheye
	void Defisheye();

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
};

