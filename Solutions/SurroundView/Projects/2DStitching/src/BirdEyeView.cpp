#include "BirdEyeView.h"
#include "SearchContours.h"
#include<algorithm>
#include<math.h>
#include<float.h>

BirdEyeView::BirdEyeView()
{
	targetPoint.resize(CAMERA_NUMBERS);
	sourcePoint.resize(CAMERA_NUMBERS);

	for (int i = 0; i < CAMERA_NUMBERS; i++)
	{
		targetPoint[i].resize(KEY_POINT_NUMBERS);
		sourcePoint[i].resize(KEY_POINT_NUMBERS);
	}
}

BirdEyeView::~BirdEyeView()
{
}

void BirdEyeView::Remap_cpu(const Mat& src, Mat& dst, const Mat& mapx, const Mat& mapy)
{
	dst.create(src.size(), src.type());
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			int src_x = mapx.at<float>(i, j);
			int src_y = mapy.at<float>(i, j);

			if ((src_x >= 0) && (src_x < src.cols) && (src_y >= 0) && (src_y < src.rows))
			{
				dst.at<Vec3b>(i, j) = src.at<Vec3b>(src_y, src_x);
			}
		}
	}
}

bool BirdEyeView::Init(const char* configFile, const XMLParameters& param, Camera* camera)
{
	bool ret = false;

	/* Read source key point from file */
	if (ReadSrcPoint(configFile))
	{
		SetTargetImageSize(param);
		SetTargetPoint();
		SetTargetMask();
		GetTransfromMat(birdtransform);		
		GetDefisheyeAndTransfromMaps(mapx, mapy, camera);		

		ret = true;
	} 
	else
	{
		std::cout << "Please set the source point!" << std::endl;
		ret = false;
	}

	/* Read the car image */
	carImage = cv::imread("./ov10635/car.png");
	if (!carImage.empty())
	{
		if ((param.pattern.car_width - 2 * param.pattern.shift_width_in > 0)
			|| (param.pattern.car_width - 2 * param.pattern.shift_height_in > 0))
		{
			cv::Size newCarSize = cv::Size(param.pattern.car_width - 2 * param.pattern.shift_width_in,
				param.pattern.car_width - 2 * param.pattern.shift_height_in);

			cv::resize(carImage, newCarImage, newCarSize, CV_INTER_CUBIC);
			carImagePos = cv::Rect(param.pattern.shift_width_ex + param.pattern.pattern_width + param.pattern.shift_width_in,
				param.pattern.shift_height_ex + param.pattern.pattern_height + param.pattern.shift_height_in,
				newCarSize.width, newCarSize.height);
		}
		else
		{
			std::cout << "[ERROR] Car or internal shift Size set error!" << std::endl;
		}
	}
	else
	{
		std::cout << "[ERROR] The car image is empty!" << std::endl;
	}

	return ret;
}

void BirdEyeView::GetTransfromMaps(cv::Mat *mapx, cv::Mat *mapy)
{
	for (int i = 0; i < CAMERA_NUMBERS; i++)
	{
		PerspectiveToMaps(birdtransform[i], imageSize, mapx[i], mapy[i]);
	}
}

void BirdEyeView::GetDefisheyeAndTransfromMaps(cv::Mat *mapx, cv::Mat *mapy, Camera* camera)
{
	GetTransfromMaps(mapx, mapy);

	for (int i = 0; i < CAMERA_NUMBERS; i++)
	{		
		MergeMaps(imageSize, camera[i].xmap, camera[i].ymap, mapx[i], mapy[i], mergedMapx[i], mergedMapy[i]);
	}
}

void BirdEyeView::MergeMaps(const cv::Size imageSize,
	const cv::Mat& mapxUndistorted,
	const cv::Mat& mapyUndistorted,
	const cv::Mat& mapxUndistortedToBird,
	const cv::Mat& mapyUndistortedToBird,
	cv::Mat& mergedMapx,
	cv::Mat& mergedMapy)
{
	mergedMapx = cv::Mat(imageSize, CV_32FC1);
	mergedMapy = cv::Mat(imageSize, CV_32FC1);

	//std::cout << "mapxUndistorted size(" << mapxUndistorted.rows << "," << mapxUndistorted.cols << ")\n";
	//std::cout << "mapyUndistorted size(" << mapyUndistorted.rows << "," << mapyUndistorted.cols << ")\n";
	//std::cout << "mapxUndistortedToBird size(" << mapxUndistortedToBird.rows << "," << mapxUndistortedToBird.cols << ")\n";

	Mat mapxUndistortedToBirdTemp = mapxUndistortedToBird.clone();
	Mat mapyUndistortedToBirdTemp = mapyUndistortedToBird.clone();

	for (int i = 0; i<mapxUndistortedToBird.rows; i++)
	{
		float *distPtX = mergedMapx.ptr<float>(i);
		float *distPtY = mergedMapy.ptr<float>(i);
		float *srcBirdMapx = mapxUndistortedToBirdTemp.ptr<float>(i);
		float *srcBirdMapy = mapyUndistortedToBirdTemp.ptr<float>(i);

		for (int j = 0; j<mapxUndistortedToBird.cols; j++)
		{
			if ((srcBirdMapx[j] >= 0) && (srcBirdMapx[j] <= mapxUndistorted.cols)
				&& (srcBirdMapy[j] >= 0) && (srcBirdMapy[j] <= mapxUndistorted.rows))
			{
				if (srcBirdMapx[j] == mapxUndistortedToBird.cols)
				{
					srcBirdMapx[j] = mapxUndistortedToBird.cols - 1;
				}

				if (srcBirdMapy[j] == mapxUndistortedToBird.rows)
				{
					srcBirdMapy[j] = mapxUndistortedToBird.rows - 1;
				}

				distPtX[j] = mapxUndistorted.at<float>(srcBirdMapy[j], srcBirdMapx[j]);
				distPtY[j] = mapyUndistorted.at<float>(srcBirdMapy[j], srcBirdMapx[j]);
		
			}
			else
			{
				distPtX[j] = mapxUndistorted.at<float>(i, j);
				distPtY[j] = mapyUndistorted.at<float>(i, j);
			}
		}
	}
}

void BirdEyeView::PerspectiveToMaps(const cv::Mat &perspective_mat, const cv::Size img_size, cv::Mat &map1, cv::Mat &map2)
{
	// invert the matrix because the transformation maps must be
	// bird's view -> original
	cv::Mat inv_perspective(perspective_mat.inv());
	inv_perspective.convertTo(inv_perspective, CV_32FC1);

	// create XY 2D array
	// (((0, 0), (1, 0), (2, 0), ...),
	//  ((0, 1), (1, 1), (2, 1), ...),
	// ...)
	cv::Mat xy(img_size, CV_32FC2);
	float *pxy = (float*)xy.data;
	for (int y = 0; y < img_size.height; y++)
		for (int x = 0; x < img_size.width; x++)
		{
			*pxy++ = x;
			*pxy++ = y;
		}

	// perspective transformation of the points
	cv::Mat xy_transformed;
	cv::perspectiveTransform(xy, xy_transformed, inv_perspective);

	//Prevent errors when float32 to int16
	//float *pmytest = (float*)xy_transformed.data;
	//for (int y = 0; y < xy_transformed.rows; y++)
	//	for (int x = 0; x < xy_transformed.cols; x++)
	//	{
	//		if (abs(*pmytest) > 5000) *pmytest = 5000.00;
	//		pmytest++;
	//		if (abs(*pmytest) > 5000) *pmytest = 5000.00;
	//		pmytest++;
	//	}

	// split x/y to extra maps
	assert(xy_transformed.channels() == 2);
	cv::Mat maps[2]; // map_x, map_y
	cv::split(xy_transformed, maps);
	
	map1 = maps[0];
	map2 = maps[1];
}

void BirdEyeView::GetTransfromView(const Mat& src, Mat& dst, const int& cameraIndex)
{
	cv::remap(src, dst, mapx[cameraIndex], mapy[cameraIndex], INTER_LINEAR);
	//cv::remap(src, dst, mergedMapx[cameraIndex], mergedMapy[cameraIndex], INTER_LINEAR);

	//Remap_cpu(src, dst, mapx[cameraIndex], mapy[cameraIndex]);
	//Remap_cpu(src, dst, mergedMapx[cameraIndex], mergedMapy[cameraIndex]);	
}

Mat BirdEyeView::Run2DStitching(const Mat* images)
{
	Mat m = Mat(outImgSize, CV_8UC3);
	int seq[CAMERA_NUMBERS] = { 0, 2, 1, 3 };
	for (int i = 0; i < CAMERA_NUMBERS; i++)
	{
		if ((images[seq[i]].cols < (roi[seq[i]].x + roi[seq[i]].width)) ||
			(images[seq[i]].rows < (roi[seq[i]].y + roi[seq[i]].height)))
		{
			std::cout << "[ERROR] ROI set error!" << std::endl;
			std::cout << images[seq[i]].cols << " != " << (roi[seq[i]].x) << " + " << (roi[seq[i]].width) << std::endl;
			std::cout << images[seq[i]].rows << " != " << (roi[seq[i]].y) << " + " << (roi[seq[i]].height) << std::endl;
		}

		switch (seq[i])
		{
		case 1: // forward
			images[seq[i]](roi[seq[i]]).copyTo(m(roi[seq[i]]), maskF);
			break;
		case 3: // backward			
			images[seq[i]](roi[seq[i]]).copyTo(m(roi[seq[i]]), maskB);
			break;
		default: // left and right
			images[seq[i]](roi[seq[i]]).copyTo(m(roi[seq[i]]));
			break;
		}
	}

	if (!carImage.empty())
	{
		//newCarImage.copyTo(m(carImagePos));
	}

	return m;
}

Mat BirdEyeView::TransformView(Mat* v) 
{
	Mat b[CAMERA_NUMBERS];
	Mat m = Mat(outImgSize, CV_8UC3);
	int seq[CAMERA_NUMBERS] = { 0, 2, 1, 3 };

	//Mat mapx[CAMERA_NUMBERS], mapy[CAMERA_NUMBERS];

	for (int i = 0; i < CAMERA_NUMBERS; i++)
	{
		if (!v[seq[i]].data)
		{
			continue;
		}
		//warpPerspective(v[seq[i]], b[seq[i]], birdtransform[seq[i]], outImgSize, cv::INTER_LINEAR);

		remap(v[seq[i]], b[seq[i]], mapx[seq[i]], mapy[seq[i]], INTER_LINEAR);

		//remap(v[seq[i]], b[seq[i]], mergedMapx[seq[i]], mergedMapy[seq[i]], INTER_LINEAR);

		//Remap_cpu(v[seq[i]], b[seq[i]], mapx[seq[i]], mapy[seq[i]]);
		//Remap_cpu(v[seq[i]], b[seq[i]], mergedMapx[seq[i]], mergedMapy[seq[i]]);		

		string name = "seq_" + to_string(seq[i]);	

		if ((b[seq[i]].cols != (roi[seq[i]].x + roi[seq[i]].width)) &&
			(b[seq[i]].rows != (roi[seq[i]].y + roi[seq[i]].height)))
		{
			std::cout << "[ERROR] ROI set error!" << std::endl;			
		}

		switch (seq[i])
		{
		case 1: // forward
			b[seq[i]](roi[seq[i]]).copyTo(m(roi[seq[i]]), maskF);
			//imshow(name, m);
			//imshow(name, b[seq[i]](roi[seq[i]]));			
			break;
		case 3: // backward			
			b[seq[i]](roi[seq[i]]).copyTo(m(roi[seq[i]]), maskB);
			//imshow(name, m);
			//imshow(name, b[seq[i]](roi[seq[i]]));
			break;
		default: // left and right
			b[seq[i]](roi[seq[i]]).copyTo(m(roi[seq[i]]));
			//imshow(name, m);
			//imshow(name, b[seq[i]](roi[seq[i]]));
			break;
		}
	}	

	if (!carImage.empty()) 
	{
		//newCarImage.copyTo(m(carImagePos));
	}	

	//StitchingSeamFusion(b[0](roi[0]), b[1](roi[1]), m);
	//StitchingSeamFusion(b[1](roi[1]), b[2](roi[2]), m);
	//DrawStitchingLine(m);

	return m;
}

void BirdEyeView::StitchingSeamFusion(Mat& left, Mat& right, Mat& dst)
{
	//imshow("left", left);
	//imshow("right", right);
	//imshow("dst", dst);

	Line line_lf = GetAlphaBeta(Point(0, roi[1].height - maskHeight), Point(roi[0].width, roi[1].height));

	double processWidth = roi[0].width;
	double alpha = 1;

	//int rows = roi[0].width;
	//int cols = maskHeight;


	int rows = roi[1].height;
	int cols = roi[0].width;

	Mat LUT = Mat(cv::Size(cols, rows), CV_8UC1);

	int x_offset = 0;
	int y_offset = roi[1].height - maskHeight;

	if (y_offset < 0)
	{
		y_offset = 0;
		x_offset = -(line_lf.beta / line_lf.alpha);
	}

	y_offset = 0;
	x_offset = 0;

	int seamWidth = 50;

	for (int i = y_offset; i < rows; i++) // rows行数对应高度y
	{
		uchar* l = left.ptr<uchar>(i);
		uchar* r = right.ptr<uchar>(i);
		uchar* d = dst.ptr<uchar>(i);

		uchar* lut = LUT.ptr<uchar>(i);

		for (int j = x_offset; j < cols; j++) // cols列数对应宽度x 
		{			
			//如果遇到图像trans中无像素的黑点，则完全拷贝img1中的数据
			if (l[j * 3] == 0 && l[j * 3 + 1] == 0 && l[j * 3 + 2] == 0)
			{
				alpha = 1;
			} 
			else if (r[j * 3] == 0 && r[j * 3 + 1] == 0 && r[j * 3 + 2] == 0)
			{
				alpha = 0;
			}
			else 
			{
				//img1中像素的权重，与当前处理点距重叠区域左边界的距离成正比，实验证明，这种方法确实好  
				//alpha = (processWidth - j) / processWidth;
				alpha = j / processWidth;
			}

			double yy = line_lf.alpha*j + line_lf.beta;

			//cv::circle(LUT, Point2f(j, yy), 1, cv::Scalar(255, 0, 0), -1);	

			lut[j] = alpha * 255;

			//if (i < yy) 
			//{
			//	if ((yy - i) < seamWidth)
			//	{
			//		d[j * 3] = r[j * 3] * alpha + l[j * 3] * (1 - alpha);
			//		d[j * 3 + 1] = r[j * 3 + 1] * alpha + l[j * 3 + 1] * (1 - alpha);
			//		d[j * 3 + 2] = r[j * 3 + 2] * alpha + l[j * 3 + 2] * (1 - alpha);	

			//		lut[j] = alpha * 255;
			//	}
			//	else
			//	{
			//		alpha = 0;

			//		d[j * 3] = r[j * 3] * alpha + l[j * 3] * (1 - alpha);
			//		d[j * 3 + 1] = r[j * 3 + 1] * alpha + l[j * 3 + 1] * (1 - alpha);
			//		d[j * 3 + 2] = r[j * 3 + 2] * alpha + l[j * 3 + 2] * (1 - alpha);
			//	}
			//}
			//else
			//{
			//	if ((i - yy) < seamWidth)
			//	{					
			//		d[j * 3] = r[j * 3] * alpha + l[j * 3] * (1 - alpha);
			//		d[j * 3 + 1] = r[j * 3 + 1] * alpha + l[j * 3 + 1] * (1 - alpha);
			//		d[j * 3 + 2] = r[j * 3 + 2] * alpha + l[j * 3 + 2] * (1 - alpha);

			//		lut[j] = alpha * 255;
			//	}
			//	else
			//	{
			//		alpha = 1;

			//		d[j * 3] = r[j * 3] * alpha + l[j * 3] * (1 - alpha);
			//		d[j * 3 + 1] = r[j * 3 + 1] * alpha + l[j * 3 + 1] * (1 - alpha);
			//		d[j * 3 + 2] = r[j * 3 + 2] * alpha + l[j * 3 + 2] * (1 - alpha);
			//	}
			//}

			d[j * 3] = r[j * 3] * alpha + l[j * 3] * (1 - alpha);
			d[j * 3 + 1] = r[j * 3 + 1] * alpha + l[j * 3 + 1] * (1 - alpha);
			d[j * 3 + 2] = r[j * 3 + 2] * alpha + l[j * 3 + 2] * (1 - alpha);

			//d[j * 3] = 0;
			//d[j * 3 + 1] = 0;
			//d[j * 3 + 2] = 0;
		}

		
	}
	imshow("lut", LUT);

}

void BirdEyeView::DrawStitchingLine(Mat& image)
{
	// 画出拼接线
	Line line_lf = GetAlphaBeta(Point(0, roi[1].height - maskHeight), Point(roi[0].width, roi[1].height));
	Line line_lb = GetAlphaBeta(Point(0, maskHeight), Point(roi[0].width, 0));

	Line line_rf = GetAlphaBeta(Point(roi[1].width, roi[1].height - maskHeight), Point(roi[1].width - roi[2].width, roi[1].height));
	Line line_rb = GetAlphaBeta(Point(roi[3].width, maskHeight), Point(roi[3].width - roi[2].width, 0));

	int x_offset = 0;
	int y_offset = 0;

	for (int col = 0; col < roi[0].width; col++)
	{
		x_offset = 0;
		y_offset = 0;
		cv::circle(image, Point2f(col + x_offset, line_lf.alpha*(col + x_offset) + line_lf.beta + y_offset), 1, cv::Scalar(255, 0, 0), -1);

		x_offset = roi[1].width - roi[2].width;
		y_offset = 0;
		cv::circle(image, Point2f(col + x_offset, line_rf.alpha*(col + x_offset) + line_rf.beta + y_offset), 1, cv::Scalar(0, 255, 0), -1);

		x_offset = 0;
		y_offset = roi[0].height - roi[3].height;
		cv::circle(image, Point2f(col + x_offset, line_lb.alpha*(col + x_offset) + line_lb.beta + y_offset), 1, cv::Scalar(0, 0, 255), -1);

		x_offset = roi[1].width - roi[2].width;
		y_offset = roi[0].height - roi[3].height;
		cv::circle(image, Point2f(col + x_offset, line_rb.alpha*(col + x_offset) + line_rb.beta + y_offset), 1, cv::Scalar(255, 0, 255), -1);
	}
}

void BirdEyeView::SetTargetImageSize(const XMLParameters& param)
{
	chessBord = cv::Size(param.pattern.pattern_width, param.pattern.pattern_height);
	exShift = cv::Size(param.pattern.shift_width_ex, param.pattern.shift_height_ex);
	inShift = cv::Size(param.pattern.shift_width_in, param.pattern.shift_height_in);
	carSize = cv::Size(param.pattern.car_width, param.pattern.car_height);
	maskHeight = param.pattern.mask_height;

	//outImgSize = cv::Size(exShift.width * 2 + inShift.width * 2  + carSize.width + chessBord.width * 2,
	//	exShift.height * 2 + inShift.height * 2 + carSize.height + chessBord.height * 2);

	outImgSize = cv::Size(param.pattern.disp_width, param.pattern.disp_height);
	//outImgSize = cv::Size(exShift.width * 2 + carSize.width + chessBord.width * 2,
	//	exShift.height * 2 + carSize.height + chessBord.height * 2);

	std::cout << "[INFO] Output image size: ( " << outImgSize.width << " " << outImgSize.height << ")" << std::endl;

	// visible range
	float scale_per_pixel = param.pattern.pattern_width_actual / (param.pattern.pattern_width*1.0f);// cm/pixel
	std::cout << "[INFO] Visible range is : ( " << outImgSize.width*scale_per_pixel / 100.0f << "m, "
		<< outImgSize.height*scale_per_pixel / 100.0f << "m )" << std::endl;
}

void BirdEyeView::SetTargetPoint()
{
	/* set targetPoint */
	/*left*/
	targetPoint[0][0] = (Point2f(exShift.width + chessBord.width, exShift.height));
	targetPoint[0][1] = (Point2f(exShift.width + chessBord.width, exShift.height + chessBord.height));
	targetPoint[0][2] = (Point2f(exShift.width + chessBord.width, outImgSize.height - exShift.height - chessBord.height));
	targetPoint[0][3] = (Point2f(exShift.width + chessBord.width, outImgSize.height - exShift.height));
	targetPoint[0][4] = (Point2f(exShift.width, outImgSize.height - exShift.height));
	targetPoint[0][5] = (Point2f(exShift.width, outImgSize.height - exShift.height - chessBord.height));
	targetPoint[0][6] = (Point2f(exShift.width, exShift.height + chessBord.height));
	targetPoint[0][7] = (Point2f(exShift.width, exShift.height));

	/*forward*/
	targetPoint[1][0] = (Point2f(outImgSize.width - exShift.width, exShift.height + chessBord.height));
	targetPoint[1][1] = (Point2f(outImgSize.width - exShift.width - chessBord.width, exShift.height + chessBord.height));
	targetPoint[1][2] = (Point2f(exShift.width + chessBord.width, exShift.height + chessBord.height));
	targetPoint[1][3] = (Point2f(exShift.width, exShift.height + chessBord.height));
	targetPoint[1][4] = (Point2f(exShift.width, exShift.height));
	targetPoint[1][5] = (Point2f(exShift.width + chessBord.width, exShift.height));
	targetPoint[1][6] = (Point2f(outImgSize.width - exShift.width - chessBord.width, exShift.height));
	targetPoint[1][7] = (Point2f(outImgSize.width - exShift.width, exShift.height));

	/*right*/
	targetPoint[2][0] = (Point2f(outImgSize.width - exShift.width - chessBord.width, outImgSize.height - exShift.height));
	targetPoint[2][1] = (Point2f(outImgSize.width - exShift.width - chessBord.width, outImgSize.height - exShift.height - chessBord.height));
	targetPoint[2][2] = (Point2f(outImgSize.width - exShift.width - chessBord.width, exShift.height + chessBord.width));
	targetPoint[2][3] = (Point2f(outImgSize.width - exShift.width - chessBord.width, exShift.height));

	targetPoint[2][4] = (Point2f(outImgSize.width - exShift.width, exShift.height));
	targetPoint[2][5] = (Point2f(outImgSize.width - exShift.width, exShift.height + chessBord.width));
	targetPoint[2][6] = (Point2f(outImgSize.width - exShift.width, outImgSize.height - exShift.height - chessBord.height));
	targetPoint[2][7] = (Point2f(outImgSize.width - exShift.width, outImgSize.height - exShift.height));

	/*backward*/
	targetPoint[3][0] = (Point2f(exShift.width, outImgSize.height - exShift.height - chessBord.height));
	targetPoint[3][1] = (Point2f(exShift.width + chessBord.width, outImgSize.height - exShift.height - chessBord.height));
	targetPoint[3][2] = (Point2f(outImgSize.width - exShift.width - chessBord.width, outImgSize.height - exShift.height - chessBord.height));
	targetPoint[3][3] = (Point2f(outImgSize.width - exShift.width, outImgSize.height - exShift.height - chessBord.height));
	targetPoint[3][4] = (Point2f(outImgSize.width - exShift.width, outImgSize.height - exShift.height));
	targetPoint[3][5] = (Point2f(outImgSize.width - exShift.width - chessBord.width, outImgSize.height - exShift.height));
	targetPoint[3][6] = (Point2f(exShift.width + chessBord.width, outImgSize.height - exShift.height));
	targetPoint[3][7] = (Point2f(exShift.width, outImgSize.height - exShift.height));
}

void BirdEyeView::SetTargetMask()
{
	/* Set Mask */
	// 左ROI
	roi[0] = Rect(0, 0, exShift.width + chessBord.width + inShift.width, outImgSize.height);

	// 前ROI
	roi[1] = Rect(0, 0, outImgSize.width, exShift.height + chessBord.height + inShift.height);

	// 右ROI
	roi[2] = Rect(outImgSize.width - exShift.width - chessBord.width - inShift.width, 0, exShift.width + chessBord.width + inShift.width, outImgSize.height);

	// 后ROI
	roi[3] = Rect(0, outImgSize.height - exShift.height - chessBord.height - inShift.height, outImgSize.width, exShift.height + chessBord.height + inShift.height);

	// 前摄像头
	maskF = Mat(roi[1].size(), CV_8UC1, Scalar(1));

	// 后摄像头
	maskB = Mat(roi[1].size(), CV_8UC1, Scalar(1));

	/*make mask, need roi */
	vector<vector<Point> > maskVec;
	/*forward*/
	maskVec.push_back(vector<Point>());
	maskVec[0].push_back(Point(0, roi[1].height));
	maskVec[0].push_back(Point(0, roi[1].height - maskHeight));
	maskVec[0].push_back(Point(roi[0].width, roi[1].height));

	maskVec.push_back(vector<Point>());
	maskVec[1].push_back(Point(roi[1].width, roi[1].height));
	maskVec[1].push_back(Point(roi[1].width, roi[1].height - maskHeight));
	maskVec[1].push_back(Point(roi[1].width - roi[2].width, roi[1].height));

	/*backward*/
	maskVec.push_back(vector<Point>());
	maskVec[2].push_back(Point(0, 0));
	maskVec[2].push_back(Point(0, maskHeight));
	maskVec[2].push_back(Point(roi[0].width, 0));

	maskVec.push_back(vector<Point>());
	maskVec[3].push_back(Point(outImgSize.width, 0));
	maskVec[3].push_back(Point(outImgSize.width, maskHeight));
	maskVec[3].push_back(Point(outImgSize.width - roi[2].width, 0));
	/*draw  mask*/
	// 绘制轮廓
	drawContours(maskF, maskVec, 0, Scalar(0), CV_FILLED);
	drawContours(maskF, maskVec, 1, Scalar(0), CV_FILLED);
	drawContours(maskB, maskVec, 2, Scalar(0), CV_FILLED);
	drawContours(maskB, maskVec, 3, Scalar(0), CV_FILLED);
}

void BirdEyeView::GetTransfromMat(cv::Mat *tansform) 
{
	for (size_t i = 0; i < CAMERA_NUMBERS; i++)
	{
		tansform[i] = findHomography(sourcePoint[i], targetPoint[i]);
	}
}

bool BirdEyeView::SearchKeyPoints(cv::Mat *srcMat)
{
	SearchContours contours;

	std::vector<cv::Point2f> imageMarkPoint[CAMERA_NUMBERS];

	for (int i = 0; i < CAMERA_NUMBERS; i++)
	{
		Mat tempImage = srcMat[i].clone();
		/* search contour, 16 points on the image */
		if (contours.GetImagePoints(tempImage, 16, imageMarkPoint[i], 50) != 0)
		{
			cout << "Get image point failed!" << endl;

			return false;
		}
	}

	int index[] = { 10, 11, 2, 3, 0, 1, 8, 9 };

	for (int i = 0; i < CAMERA_NUMBERS; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			sourcePoint[i][j] = imageMarkPoint[i][index[j]];
		}
	}

	SaveSrcPoint("./config/config.yml");/*save source's points*/

	return true;
}

bool BirdEyeView::ReadSrcPoint(const char* srcPointFile)
{
	cv::FileStorage fs(srcPointFile, cv::FileStorage::READ);
	if (fs.isOpened())
	{
		for (int i = 0; i < CAMERA_NUMBERS; i++)
		{
			for (int k = 0; k < KEY_POINT_NUMBERS; k++)
			{
				char buf[20];
				sprintf(buf, "sourcePoint%d%d", i, k);
				fs[buf] >> sourcePoint[i][k];
			}
		}		

		std::cout << "[INFO] Config file read sucessfully!" << std::endl;

		return true;
	} 
	else 
	{
		std::cout << "[INFO] There is not a config file in folder\n";

		return false;
	}		
}

bool BirdEyeView::SaveSrcPoint(const char* configFile)
{
	bool res = false;

	for (int i = 0; i < CAMERA_NUMBERS; i++)
	{
		if (sourcePoint[i].empty())
		{
			std::cout << "[ERROR] sourcePoint has not been comfired all\n" << std::endl;
			res = false;
		}
	}
	FileStorage fs(configFile, FileStorage::WRITE);
	if (fs.isOpened())
	{
		for (int i = 0; i < CAMERA_NUMBERS; i++)
		{
			for (int k = 0; k < KEY_POINT_NUMBERS; k++)
			{
				char buf[20];
				sprintf(buf, "sourcePoint%d%d", i, k);
				write(fs, buf, sourcePoint[i][k]);
			}
		}
		fs.release();
		std::cout << "[INFO] param save complete! \n";

		res = true;
	}

	return res;
}
