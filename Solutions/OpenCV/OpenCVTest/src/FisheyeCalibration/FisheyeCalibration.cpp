#include "FisheyeCalibration.h"

FisheyeCalibration::FisheyeCalibration()
{
	cout << "FisheyeCalibration" << endl;
}


FisheyeCalibration::~FisheyeCalibration()
{
	cout << "~FisheyeCalibration" << endl;
}

void MouseAction(int event, int x, int y, int flags, void *para)
{
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		std::cout << "x: " << x << " y: " << y << std::endl;

		break;
	}
}

void FisheyeCalibration::Defisheye()
{
	// 读取内参矩阵和畸变系数
	if (!ReadCalibrateRes("calib_results.txt"))
	{
		cout << "No calibration data!" << endl;
	}

	// 读取需要拼接的前后左右图片
	Mat left = ImageRemap(imread("./ov10635/left.jpg"), 2);

	//Mat left_cut = GetPerspectiveImgLeft(left);

	for (int i = 0; i < 4; i++) 
	{
		string src_name = "./image/src_" + to_string(i + 1) + ".jpg";
		string dst_name = "./image/cali_" + to_string(i + 1) + ".jpg";
		Mat dst_image = ImageRemap(imread(src_name), 2);
		imshow(dst_name, dst_image);
		imwrite(dst_name, dst_image);
	}

	waitKey(0);
}

void FisheyeCalibration::RunStitching()
{
	// 读取内参矩阵和畸变系数
	if (!ReadCalibrateRes("calib_results.txt"))
	{
		cout << "No calibration data!" << endl;
	}

	// 读取需要拼接的前后左右图片
	Mat front = ImageRemap(imread("./fisheye_img/src_4.jpg"), 2.2);
	Mat back = ImageRemap(imread("./fisheye_img/src_2.jpg"), 2.2);
	Mat left = ImageRemap(imread("./fisheye_img/src_3.jpg"), 2.2);
	Mat right = ImageRemap(imread("./fisheye_img/src_1.jpg"), 2.2);

	cnt = 0;
	Mat front_cut = GetPerspectiveImg(front);
	cnt++;
	Mat back_cut = GetPerspectiveImg(back);
	cnt++;
	Mat left_cut = GetPerspectiveImg(left);
	cnt++;
	Mat right_cut = GetPerspectiveImg(right);

	//imshow("front", front);
	//imshow("back", back);
	//imshow("left", left);
	//imshow("right", right);

	Mat mCombine = Mat::zeros(1280, 1280, front.type());

	// 沿着特征点剪切	
	for (int row = 0; row < front_cut.rows; row++) // 列 x
	{
		for (int col = 0; col < front_cut.cols; col++) // 行 y
		{
			if (IsValidPoint(Point2f(col, row), line_left[0], line_right[0]))
			{
				// front
				mCombine.at<Vec3b>(row, col)[0] = front_cut.at<Vec3b>(row, col)[0];
				mCombine.at<Vec3b>(row, col)[1] = front_cut.at<Vec3b>(row, col)[1];
				mCombine.at<Vec3b>(row, col)[2] = front_cut.at<Vec3b>(row, col)[2];
			}

			if (IsValidPoint(Point2f(col, row), line_left[3], line_right[3]))
			{
				// back
				mCombine.at<Vec3b>(mCombine.rows - row - 1, mCombine.rows - col - 1)[0] = back_cut.at<Vec3b>(row, col)[0];
				mCombine.at<Vec3b>(mCombine.rows - row - 1, mCombine.rows - col - 1)[1] = back_cut.at<Vec3b>(row, col)[1];
				mCombine.at<Vec3b>(mCombine.rows - row - 1, mCombine.rows - col - 1)[2] = back_cut.at<Vec3b>(row, col)[2];
			}

			if (IsValidPoint(Point2f(col, row), line_left[2], line_right[2]))
			{
				// left
				mCombine.at<Vec3b>(mCombine.rows - col - 1, row)[0] = left_cut.at<Vec3b>(row, col)[0];
				mCombine.at<Vec3b>(mCombine.rows - col - 1, row)[1] = left_cut.at<Vec3b>(row, col)[1];
				mCombine.at<Vec3b>(mCombine.rows - col - 1, row)[2] = left_cut.at<Vec3b>(row, col)[2];
			}


			if (IsValidPoint(Point2f(col, row), line_left[3], line_right[3]))
			{
				// right
				mCombine.at<Vec3b>(col, mCombine.rows - row - 1)[0] = right_cut.at<Vec3b>(row, col)[0];
				mCombine.at<Vec3b>(col, mCombine.rows - row - 1)[1] = right_cut.at<Vec3b>(row, col)[1];
				mCombine.at<Vec3b>(col, mCombine.rows - row - 1)[2] = right_cut.at<Vec3b>(row, col)[2];
			}


			//if (IsValidPoint(Point2f(col, row), line_left[1], line_right[1]))
			//{
			//	// back
			//	mCombine.at<Vec3b>(col, row)[0] = back_cut.at<Vec3b>(row, col)[0];
			//	mCombine.at<Vec3b>(col, row)[1] = back_cut.at<Vec3b>(row, col)[1];
			//	mCombine.at<Vec3b>(col, row)[2] = back_cut.at<Vec3b>(row, col)[2];
			//}

			//if (IsValidPoint(Point2f(col, row), line_left[1], line_right[1]))
			//{
			//	// right
			//	mCombine.at<Vec3b>(col, row)[0] = right_cut.at<Vec3b>(row, col)[0];
			//	mCombine.at<Vec3b>(col, row)[1] = right_cut.at<Vec3b>(row, col)[1];
			//	mCombine.at<Vec3b>(col, row)[2] = right_cut.at<Vec3b>(row, col)[2];
			//}
		}
	}

	imshow("res", mCombine);
	waitKey(0);

}

Mat FisheyeCalibration::GetPerspectiveImgLeft(Mat& image)
{
	Mat temp = image.clone();

	std::vector<cv::Point2f> imageMarkPoint;
	imageMarkPoint.clear();

	/* search contour, 16 points on the image */
	if (contours.GetImagePoints(temp, 16, imageMarkPoint) != 0)
	{
		cout << "get image point failed!" << endl;

		return temp;
	}

	/* Get 3D reference points */
	std::vector<cv::Point3f> ref_points, temp_points;
	int x, y;
	string filename = "./template/template_1.txt";
	ref_points.clear();

	std::ifstream ifs_ref(filename);

	while (ifs_ref >> x >> y)
	{
		temp_points.push_back(cv::Point3f(x / 10.0, y / 10.0, 0));
	}

	// 三维坐标点(z=0平面)， 此时ref_points的点映射到图片之后他们的相对位置就固定了。输入的长度就是像素长度。
	float sf_x = temp.size().width / 4700;
	float sf_y = temp.size().height / 4700;

	float offset_x = (temp.size().width / 2.0) - ((temp_points[8].x - temp_points[1].x) / 2.0) - temp_points[1].x;
	//float offset_y = img_p[0].y;
	float offset_y = 405; // TODO

	cout << "offset_x: " << offset_x << endl;

	for (int i = 0; i < temp_points.size(); i++)
	{
		ref_points.push_back(Point3f(temp_points[i].x + offset_x, temp_points[i].y + offset_y, 0));
	}

	// 计算沿着参考点的直线
	line_left[cnt] = GetAlphaBeta(Point2f(ref_points[0].x, ref_points[0].y), Point2f(ref_points[2].x, ref_points[2].y));
	line_right[cnt] = GetAlphaBeta(Point2f(ref_points[9].x, ref_points[9].y), Point2f(ref_points[11].x, ref_points[11].y));

	std::vector<cv::Point2f> image_points;
	std::vector<cv::Point3f> object_points;
	for (uint j = 0; j < imageMarkPoint.size(); j++)
	{
		image_points.push_back(imageMarkPoint[j]);
		object_points.push_back(cv::Point3f(ref_points[j].x, ref_points[j].y, 0.0));
	}

	cv::Mat matImgPoints(image_points);
	cv::Mat matObjPoints(object_points);

	// solvePnP 计算外参 及平移矢量和旋转矢量
	//solvePnP(matObjPoints, matImgPoints, intrinsicMatrix, distortionCoeffs, rotationVec, translationVec);

	Mat homo = findHomography(matImgPoints, matObjPoints, CV_RANSAC);

	//cout << "homo: " << homo << endl;

	Mat mPerspectiveImg;
	warpPerspective(image, mPerspectiveImg, homo, image.size(), cv::INTER_LINEAR);

	//// 沿着特征点剪切	
	//for (int row = 0; row < mPerspectiveImg.rows; row++)
	//{
	//	for (int col = 0; col < mPerspectiveImg.cols; col++)
	//	{
	//		if (IsValidPoint(Point2f(col, row), line_left[cnt], line_right[cnt]))
	//		{
	//			// front
	//			mPerspectiveImg.at<Vec3b>(row, col)[0] = mPerspectiveImg.at<Vec3b>(row, col)[0];
	//			mPerspectiveImg.at<Vec3b>(row, col)[1] = mPerspectiveImg.at<Vec3b>(row, col)[1];
	//			mPerspectiveImg.at<Vec3b>(row, col)[2] = mPerspectiveImg.at<Vec3b>(row, col)[2];

	//		}
	//		else
	//		{
	//			mPerspectiveImg.at<Vec3b>(row, col)[0] = 0;
	//			mPerspectiveImg.at<Vec3b>(row, col)[1] = 0;
	//			mPerspectiveImg.at<Vec3b>(row, col)[2] = 0;
	//		}
	//	}
	//}

	//imshow("res", mPerspectiveImg);
	//waitKey(0);

	return mPerspectiveImg;
}

Mat FisheyeCalibration::GetPerspectiveImg(Mat& image)
{
	Mat temp = image.clone();

	std::vector<cv::Point2f> imageMarkPoint;
	imageMarkPoint.clear();

	// search contour
	if (contours.GetImagePoints(temp, 16, imageMarkPoint) != 0)
	{
		cout << "get image point failed!" << endl;

		return temp;
	}

	/*************************************** 2. Get 3D reference points ***************************************/
	std::vector<cv::Point3f> ref_points, temp_points;
	int x, y;
	string filename = "./template/template_1.txt";
	ref_points.clear();

	std::ifstream ifs_ref(filename);

	while (ifs_ref >> x >> y)
	{
		temp_points.push_back(cv::Point3f(x / 10.0, y / 10.0, 0));
	}

	// 三维坐标点(z=0平面)， 此时ref_points的点映射到图片之后他们的相对位置就固定了。输入的长度就是像素长度。
	float sf_x = temp.size().width / 4700;
	float sf_y = temp.size().height / 4700;

	float offset_x = (temp.size().width / 2.0) - ((temp_points[8].x - temp_points[1].x) / 2.0) - temp_points[1].x;
	//float offset_y = img_p[0].y;
	float offset_y = 405; // TODO

	cout << "offset_x: " << offset_x << endl;

	for (int i = 0; i < temp_points.size(); i++)
	{
		ref_points.push_back(Point3f(temp_points[i].x + offset_x, temp_points[i].y + offset_y, 0));
	}

	// 计算沿着参考点的直线
	line_left[cnt] = GetAlphaBeta(Point2f(ref_points[0].x, ref_points[0].y), Point2f(ref_points[2].x, ref_points[2].y));
	line_right[cnt] = GetAlphaBeta(Point2f(ref_points[9].x, ref_points[9].y), Point2f(ref_points[11].x, ref_points[11].y));

	std::vector<cv::Point2f> image_points;
	std::vector<cv::Point3f> object_points;
	for (uint j = 0; j < imageMarkPoint.size(); j++)
	{
		image_points.push_back(imageMarkPoint[j]);
		object_points.push_back(cv::Point3f(ref_points[j].x, ref_points[j].y, 0.0));
	}

	cv::Mat matImgPoints(image_points);
	cv::Mat matObjPoints(object_points);

	// solvePnP 计算外参 及平移矢量和旋转矢量
	//solvePnP(matObjPoints, matImgPoints, intrinsicMatrix, distortionCoeffs, rotationVec, translationVec);

	Mat homo = findHomography(matImgPoints, matObjPoints, CV_RANSAC);

	//cout << "homo: " << homo << endl;

	Mat mPerspectiveImg;
	warpPerspective(image, mPerspectiveImg, homo, image.size(), cv::INTER_LINEAR);

	// 画出标记点线段
	//for (int col = 0; col < mPerspectiveImg.cols; col++)
	//{
	//	cv::circle(mPerspectiveImg, Point2f(col, line_left[cnt].alpha*col + line_left[cnt].beta), 1, cv::Scalar(0, 0, 255), -1);
	//	cv::circle(mPerspectiveImg, Point2f(col, line_right[cnt].alpha*col + line_right[cnt].beta), 1, cv::Scalar(0, 0, 255), -1);
	//}

	// 沿着特征点剪切	
	for (int row = 0; row < mPerspectiveImg.rows; row++)
	{
		for (int col = 0; col < mPerspectiveImg.cols; col++)
		{
			if (IsValidPoint(Point2f(col, row), line_left[cnt], line_right[cnt]))
			{
				// front
				mPerspectiveImg.at<Vec3b>(row, col)[0] = mPerspectiveImg.at<Vec3b>(row, col)[0];
				mPerspectiveImg.at<Vec3b>(row, col)[1] = mPerspectiveImg.at<Vec3b>(row, col)[1];
				mPerspectiveImg.at<Vec3b>(row, col)[2] = mPerspectiveImg.at<Vec3b>(row, col)[2];

			}
			else
			{
				mPerspectiveImg.at<Vec3b>(row, col)[0] = 0;
				mPerspectiveImg.at<Vec3b>(row, col)[1] = 0;
				mPerspectiveImg.at<Vec3b>(row, col)[2] = 0;
			}
		}
	}

	//imshow("res", mPerspectiveImg);
	//waitKey(0);

	return mPerspectiveImg;
}

bool FisheyeCalibration::CalibrateAndSave2File(const int& totalImgNumber, const string& filename)
{
	
	// 读取图片并提取角点
	for (int i = 0; i < totalImgNumber; i++)
	{
		String imageName = "./fisheye_img/frame1_" + to_string(i) + ".jpg";

		Mat image = imread(imageName);

		if (image.empty()) 
		{
			cout << "Image: " << imageName << "is empty!" << endl;
			continue;
		}

		vector<Point2f> corners;

		if (FindCorners(image, corners)) 
		{
			cout << "Image: " << imageName << " find Chessboard Corners success!" << endl;
			cornersSeq.push_back(corners);
			imageSeq.push_back(image);

			//DrawCorners(image, corners);
		}
	}

	// 设置定标板放在世界坐标系中的坐标
	SetObjectPoints(objectPoints);

	FisheyeCameraCalibration(intrinsicMatrix, distortionCoeffs, rotationVectors, translationVectors);

	EvaluateCalibrateRes();

	SaveCalibrateRes(filename);

	return true;
	
}

void FisheyeCalibration::RunCalibration()
{	
	if (!ReadCalibrateRes("calib_results.txt"))
	{
		// 读取图片并提取角点
		for (int i = 0; i < imageNum; i++)
		{
			String imageName = "./fisheye_img/frame1_" + to_string(i) + ".jpg";

			Mat image = imread(imageName);

			if (image.empty())
			{
				cout << "Image: " << imageName << "is empty!" << endl;
				continue;
			}

			vector<Point2f> corners;

			if (FindCorners(image, corners))
			{
				cout << "Image: " << imageName << " find Chessboard Corners success!" << endl;
				cornersSeq.push_back(corners);
				imageSeq.push_back(image);

				//DrawCorners(image, corners);
			}
		}

		// 设置定标板放在世界坐标系中的坐标
		SetObjectPoints(objectPoints);

		FisheyeCameraCalibration(intrinsicMatrix, distortionCoeffs, rotationVectors, translationVectors);

		EvaluateCalibrateRes();

		SaveCalibrateRes("calib_results.txt");
	}

	Mat res = ImageRemap(imread("./fisheye_img/src_4.jpg"), 2.1);
	//Mat res = ImageRemap(imread("./fisheye_img/test_out.jpg"));
	Mat temp = res.clone();
	

	std::vector<cv::Point2f> img_p;
	img_p.clear();

	// search contour
	if (contours.GetImagePoints(temp, 16, img_p) == 0)
	{
		//cout << "search contour failed!" << endl;

		//for (int j = 0; j < img_p.size(); j++) {
		//	cv::circle(res, img_p[j], 5, cv::Scalar(0, 0, 255), -1);
		//	cout << "Point" << j + 1 << ":(" << img_p[j].x << "," << img_p[j].y << ")" << endl;
		//}

		//int key = 0;
		//imshow("res", res);

		//while (key != 'q')
		//{			
		//	setMouseCallback("res", MouseAction);	
		//	key = waitKey(10);
		//}
	}

	/*************************************** 2. Get 3D reference points ***************************************/
	std::vector<cv::Point3f> ref_points, temp_points;
	int x, y;
	string filename = "./template/template_1.txt";
	ref_points.clear();

	std::ifstream ifs_ref(filename);

	while (ifs_ref >> x >> y)
	{
		temp_points.push_back(cv::Point3f(x / 10.0, y / 10.0, 0));
	}

	// 三维坐标点(z=0平面)， 此时ref_points的点映射到图片之后他们的相对位置就固定了。输入的长度就是像素长度。
	float sf_x = res.size().width / 4700;
	float sf_y = res.size().height / 4700;

	float offset_x = (res.size().width / 2.0) - ((temp_points[8].x - temp_points[1].x) / 2.0) - temp_points[1].x;
	//float offset_y = img_p[0].y;
	float offset_y = 404;

	cout << "offset_x: " << offset_x << endl;

	for (int i = 0; i < temp_points.size(); i++)
	{
		ref_points.push_back(Point3f(temp_points[i].x + offset_x, temp_points[i].y + offset_y, 0));
	}

	// 计算沿着参考点的直线
	Line line_left = GetAlphaBeta(Point2f(ref_points[0].x, ref_points[0].y), Point2f(ref_points[2].x, ref_points[2].y));
	Line line_right = GetAlphaBeta(Point2f(ref_points[9].x, ref_points[9].y), Point2f(ref_points[11].x, ref_points[11].y));


	// Normalization into [-1, 1] according template size
	//for (uint j = 0; j < ref_points.size(); j++) // Normalize 3D coordinates
	//{
	//	ref_points[j].x = (2 * ref_points[j].x - 4700) / 4700;
	//	ref_points[j].y = (2 * ref_points[j].y - 4700) / 4700;
	//}

	/************************************ 3. Get points from distorted image ***********************************/


	/************************ 4. Find an object pose from 3D-2D point correspondences. *************************/
	std::vector<cv::Point2f> image_points;
	std::vector<cv::Point3f> object_points;
	for (uint j = 0; j < img_p.size(); j++)
	{
		image_points.push_back(img_p[j]);
		object_points.push_back(cv::Point3f(ref_points[j].x, ref_points[j].y, 0.0));		
	}

	cv::Mat matImgPoints(image_points);
	cv::Mat matObjPoints(object_points);

	// solvePnP 计算外参 及平移矢量和旋转矢量
	//solvePnP(matObjPoints, matImgPoints, intrinsicMatrix, distortionCoeffs, rotationVec, translationVec);

	Mat homo = findHomography(matImgPoints, matObjPoints, CV_RANSAC);

	cout << "homo: " << homo << endl;

	Mat mPerspectiveImg;
	warpPerspective(res, mPerspectiveImg, homo, res.size(), cv::INTER_LINEAR);

	Mat tempMatFront, tempMatBack, tempMatLeft, tempMatRight;
	tempMatFront = mPerspectiveImg.clone();
	tempMatBack = mPerspectiveImg.clone();
	tempMatLeft = mPerspectiveImg.clone();
	tempMatRight = mPerspectiveImg.clone();	

	// 画出标记点线段
	//for (int col = 0; col < tempMatFront.cols; col++)
	//{
	//	cv::circle(tempMatFront, Point2f(col, line_left.alpha*col + line_left.beta), 1, cv::Scalar(0, 0, 255), -1);
	//	cv::circle(tempMatFront, Point2f(col, line_right.alpha*col + line_right.beta), 1, cv::Scalar(0, 0, 255), -1);
	//}

	Mat mCombine = Mat::zeros(1280, 1280, res.type());

	// 画出标记点线段
	for (int col = 0; col < mCombine.cols; col++)
	{
		cv::circle(mCombine, Point2f(col, line_left.alpha*col + line_left.beta), 1, cv::Scalar(0, 0, 255), -1);
		cv::circle(mCombine, Point2f(col, line_right.alpha*col + line_right.beta), 1, cv::Scalar(0, 0, 255), -1);

		cv::circle(mCombine, Point2f(line_left.alpha*col + line_left.beta, col), 1, cv::Scalar(255, 0, 0), -1);
		cv::circle(mCombine, Point2f(line_right.alpha*col + line_right.beta, col), 1, cv::Scalar(255, 0, 0), -1);
	}


	// 沿着特征点剪切	
	for (int row = 0; row < tempMatFront.rows; row++)
	{
		for (int col = 0; col < tempMatFront.cols; col++)
		{
			if (IsValidPoint(Point2f(col, row), line_left, line_right))
			{
				// front
				mCombine.at<Vec3b>(row, col)[0] = tempMatFront.at<Vec3b>(row, col)[0];
				mCombine.at<Vec3b>(row, col)[1] = tempMatFront.at<Vec3b>(row, col)[1];
				mCombine.at<Vec3b>(row, col)[2] = tempMatFront.at<Vec3b>(row, col)[2];
				
				// left 逆时针90度
				mCombine.at<Vec3b>(col, row)[0] = tempMatFront.at<Vec3b>(row, col)[0];
				mCombine.at<Vec3b>(col, row)[1] = tempMatFront.at<Vec3b>(row, col)[1];
				mCombine.at<Vec3b>(col, row)[2] = tempMatFront.at<Vec3b>(row, col)[2];
			}
			//else
			//{
			//	mCombine.at<Vec3b>(row, col)[0] = 0;
			//	mCombine.at<Vec3b>(row, col)[1] = 0;
			//	mCombine.at<Vec3b>(row, col)[2] = 0;
			//}
		}
	}


	imshow("mCombine", mCombine);

	//CutImage(tempMatFront, line_left, line_right);
	//CutImage(tempMatBack, line_left, line_right);
	//CutImage(tempMatLeft, line_left, line_right);
	//CutImage(tempMatRight, line_left, line_right);

	//imshow("front", tempMatFront);

	//flip(tempMatBack, tempMatBack, -1);
	//imshow("back", tempMatBack);

	//cv::transpose(tempMatLeft, tempMatLeft);
	//flip(tempMatLeft, tempMatLeft, 0);
	//imshow("left", tempMatLeft);

	//cv::transpose(tempMatRight, tempMatRight);
	//flip(tempMatRight, tempMatRight, 1);
	//imshow("right", tempMatRight);

	//Mat mCombine = Mat::zeros(1280, 1280, res.type());

	//cv::Mat mImgRoiInputFront;
	//cv::Mat mImgRoiInputBack;
	//cv::Mat mImgRoiInputLeft;
	//cv::Mat mImgRoiInputRight;

	//mImgRoiInputFront = mCombine(cv::Rect(0, 0, res.size().width, res.size().height));
	//mImgRoiInputLeft = mCombine(cv::Rect(0, 0, res.size().height, res.size().width));
	//tempMatFront.copyTo(mImgRoiInputFront);
	//tempMatLeft.copyTo(mImgRoiInputLeft);

	//imshow("mCombine", mCombine);

	waitKey(0);
}

Mat FisheyeCalibration::ImageRemap(const Mat& image, const double& scale)
{
	Mat imageRemap;
	Size imageSize = image.size();
	Mat mapx = Mat(imageSize, CV_32FC1);
	Mat mapy = Mat(imageSize, CV_32FC1);
	Mat R = Mat::eye(3, 3, CV_32F);

	Matx33d newK = intrinsicMatrix;
	newK(0, 0) = intrinsicMatrix(0, 0) / scale; // fx
	newK(1, 1) = intrinsicMatrix(1, 1) / scale; // fy

	fisheye::initUndistortRectifyMap(intrinsicMatrix, distortionCoeffs, R, newK, imageSize, CV_32FC1, mapx, mapy);
	//fisheye::initUndistortRectifyMap(intrinsicMatrix, distortionCoeffs, R, intrinsicMatrix, imageSize, CV_32FC1, mapx, mapy);	
	
	remap(image, imageRemap, mapx, mapy, INTER_LINEAR);

	return imageRemap;
}

bool FisheyeCalibration::ReadCalibrateRes(const string& filename)
{
	string file = "./calibration_result/" + filename;
	ifstream fin(file);

	if (!fin.good())
	{
		std::cout << "File " << filename << " not found" << std::endl;
		return false;
	}

	string line;
	int lineNum = 0;
	double val;

	while (getline(fin, line))
	{
		if ((line.length() > 1) && (line.at(0) != '#'))
		{
			stringstream str_stream(line);
			int i = 0;

			switch (lineNum++)
			{
			case 0:	// camera intrinsic matrix
				while (str_stream >> val)
				{
					intrinsicMatrix(i / 3, i % 3) = val;
					i++;
				}
				break;
			case 1: // distortion coeffs
				while (str_stream >> val)
				{
					distortionCoeffs(i++) = val;
				}
				break;
			case 2: // camera new intrinsic matrix
				while (str_stream >> val)
				{
					newIntrinsicMatrix(i / 3, i % 3) = val;
					i++;
				}
				break;

			default:
				return false;
				break;
			}
		}
	}

	return (lineNum == 3 ? true : false);
}

void FisheyeCalibration::SaveCalibrateRes(const string& filename)
{
	// calib_results_1.txt
	string file = "./calibration_result/" + filename;
	ofstream fout(file);

	fout << "#camera intrinsic matrix" << endl;
	fout << intrinsicMatrix(0, 0) << ' ' << intrinsicMatrix(0, 1) << ' ' << intrinsicMatrix(0, 2) << ' '
		<< intrinsicMatrix(1, 0) << ' ' << intrinsicMatrix(1, 1) << ' ' << intrinsicMatrix(1, 2) << ' '
		<< intrinsicMatrix(2, 0) << ' ' << intrinsicMatrix(2, 1) << ' ' << intrinsicMatrix(2, 2) << endl << endl;

	fout << "#distortion coeffs" << endl;
	fout << distortionCoeffs(0) << ' ' << distortionCoeffs(1) << ' ' << distortionCoeffs(2) << ' ' << distortionCoeffs(3) << endl << endl;

	fout << "#camera new intrinsic matrix" << endl;

	fout << newIntrinsicMatrix(0, 0) << ' ' << newIntrinsicMatrix(0, 1) << ' ' << newIntrinsicMatrix(0, 2) << ' '
		<< newIntrinsicMatrix(1, 0) << ' ' << newIntrinsicMatrix(1, 1) << ' ' << newIntrinsicMatrix(1, 2) << ' '
		<< newIntrinsicMatrix(2, 0) << ' ' << newIntrinsicMatrix(2, 1) << ' ' << newIntrinsicMatrix(2, 2) << endl;
}

void FisheyeCalibration::EvaluateCalibrateRes()
{
	cout << "开始评价定标结果………………" << endl;
	double total_err = 0.0;                  /* 所有图像的平均误差的总和 */
	double err = 0.0;                        /* 每幅图像的平均误差 */
	vector<Point2f>  image_points2;          /****   保存重新计算得到的投影点    ****/

	vector<int>  point_counts;

	for (int i = 0; i< imageSeq.size(); i++)
	{
		point_counts.push_back(boardSize.width*boardSize.height);
	}

	cout << "每幅图像的定标误差：" << endl;

	for (int i = 0; i<imageSeq.size(); i++)
	{
		vector<Point3f> tempPointSet = objectPoints[i];
		/****    通过得到的摄像机内外参数，对空间的三维点进行重新投影计算，得到新的投影点     ****/
		fisheye::projectPoints(tempPointSet, image_points2, rotationVectors[i], translationVectors[i], intrinsicMatrix, distortionCoeffs);
		
		/* 计算新的投影点和旧的投影点之间的误差*/
		vector<Point2f> tempImagePoint = cornersSeq[i];
		Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
		Mat image_points2Mat = Mat(1, image_points2.size(), CV_32FC2);
		for (size_t i = 0; i != tempImagePoint.size(); i++)
		{
			image_points2Mat.at<Vec2f>(0, i) = Vec2f(image_points2[i].x, image_points2[i].y);
			tempImagePointMat.at<Vec2f>(0, i) = Vec2f(tempImagePoint[i].x, tempImagePoint[i].y);
		}
		err = norm(image_points2Mat, tempImagePointMat, NORM_L2);
		total_err += err /= point_counts[i];
		cout << "第" << i + 1 << "幅图像的平均误差：" << err << "像素" << endl;		
	}
	cout << "总体平均误差：" << total_err / imageSeq.size() << "像素" << endl;
	cout << "评价完成！" << endl;
}

void FisheyeCalibration::FisheyeCameraCalibration(Matx33d& intrinsicMatrix, Vec4d& distortionCoeffs, vector<Vec3d>& rotationVectors, vector<Vec3d>& translationVectors)
{
	Size imageSize = imageSeq[0].size();

	int flags = 0;
	flags |= cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC;
	flags |= cv::fisheye::CALIB_CHECK_COND;
	flags |= cv::fisheye::CALIB_FIX_SKEW;

	/* 角点在标定板内的坐标， 角点在图像中的坐标， 图片大小， 内参矩阵， 四个畸变参数， 输出的旋转向量， 输出的平移向量， 迭代次数 误差*/
	double totalError = fisheye::calibrate(objectPoints, cornersSeq, imageSize, intrinsicMatrix, distortionCoeffs, rotationVectors, translationVectors, flags, TermCriteria(3, 20, 1e-6));
	cout << "Total error is: " << totalError << endl;
	//fisheye::estimateNewCameraMatrixForUndistortRectify(intrinsicMatrix, distortionCoeffs, imageSize, Matx33d::eye(), newIntrinsicMatrix, 0, imageSize, 1);
}

void FisheyeCalibration::SetObjectPoints(vector<vector<Point3f>>& points)
{
	if (cornersSeq.empty())
	{
		return;
	}

	for (int i = 0; i < cornersSeq.size(); i++)
	{
		vector<Point3f> tempPointSet;
		for (int i = 0; i < boardSize.height; i++)
		{
			for (int j = 0; j < boardSize.width; j++)
			{
				/* 假设定标板放在世界坐标系中z=0的平面上 */
				Point3f tempPoint;
				tempPoint.x = i*squareSize.width;
				tempPoint.y = j*squareSize.height;
				tempPoint.z = 0;
				tempPointSet.push_back(tempPoint);
			}
		}

		points.push_back(tempPointSet);
	}
}

void FisheyeCalibration::DrawCorners(const Mat& src, const vector<Point2f>& corners)
{
	/* 绘制检测到的角点并保存 */
	Mat imageTemp = src.clone();
	for (int j = 0; j < corners.size(); j++)
	{
		circle(imageTemp, corners[j], 10, Scalar(0, 0, 255), 2, 8, 0);
	}

	imshow("corners", imageTemp);	

	int key = 0;
	while (key != 'q')
	{
		key = waitKey(10);
	}
}

bool FisheyeCalibration::FindCorners(const Mat& src, vector<Point2f>& corners)
{
	if (src.empty())
	{
		cout << "image is empty!" << endl;

		return false;
	}

	// TODO: 对图像的边缘进行扩展 适当增大可以不损失原图像信息

	// 转化成灰度图像
	Mat imageGray;
	cvtColor(src, imageGray, CV_RGB2GRAY);

	// 提取角点
	bool patternfound = findChessboardCorners(src, boardSize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
	if (patternfound)
	{
		/* 亚像素精确化,对检测到的整数坐标角点精确化，精确化后的点存在corners中， 最小二乘迭代100次，误差在0.001*/
		cornerSubPix(imageGray, corners, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 100, 0.001));

		return true;
	}
	else
	{
		cout << "find Chessboard Corners failed!" << endl;

		return false;
	}
}

