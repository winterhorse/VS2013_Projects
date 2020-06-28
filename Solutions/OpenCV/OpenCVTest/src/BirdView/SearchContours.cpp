#include "SearchContours.h"

/************************************************************************************\
This is improved variant of chessboard corner detection algorithm that
uses a graph of connected quads. It is based on the code contributed
by Vladimir Vezhnevets and Philip Gruebele.
Here is the copyright notice from the original Vladimir's code:
===============================================================
The algorithms developed and implemented by Vezhnevets Vldimir
aka Dead Moroz (vvp@graphics.cs.msu.ru)
See http://graphics.cs.msu.su/en/research/calibration/opencv.html
for detailed information.
Reliability additions and modifications made by Philip Gruebele.
<a href="mailto:pgruebele@cox.net">pgruebele@cox.net</a>
Some further improvements for detection of partially ocluded boards at non-ideal
lighting conditions have been made by Alex Bovyrin and Kurt Kolonige
\************************************************************************************/

SearchContours::SearchContours()
{
}


SearchContours::~SearchContours()
{
}

int SearchContours::GetImagePoints(cv::Mat &undist_img, uint num, std::vector<cv::Point2f> &img_points)
{
	int cntr_min_size = 200;
	int index = 1;
	double roi = 0.9;

	/**************************************** 1. Contour detections *******************************************/
	cv::Mat undist_img_gray;
	cvtColor(undist_img, undist_img_gray, CV_RGB2GRAY); // Convert to grayscale

	cv::Mat temp;
	undist_img_gray(cv::Rect(0, undist_img_gray.rows * (1 - roi) - 10, undist_img_gray.cols, undist_img_gray.rows * roi)).copyTo(temp); // Get roi

	cv::Ptr<CvMemStorage> storage;
	storage = cvCreateMemStorage(0);
	CvSeq * root = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSeq*), storage);

	int contours_num = GetContours(temp, &root, storage, cntr_min_size); // Get contours


	// If number of contours not equal to CONTOURS_NUM, then complete the calibration process
	if (contours_num < CONTOURS_NUM) {
		sec2vector(&root, img_points, cv::Point2f(0, undist_img.rows * (1 - roi) - 10));
		if (contours_num == 0) {
			std::cout << "Camera " << index << ". No contours were found. Change the calibration image" << std::endl;
			return(-1);
		}
		std::cout << "Camera " << index << ". The number of contours is: " << contours_num << " fewer than 4. Change the calibration image" << std::endl;
		return(-1);
	}
	else if (contours_num > CONTOURS_NUM) {
		sec2vector(&root, img_points, cv::Point2f(0, undist_img.rows * (1 - roi) - 10));
		std::cout << "Camera " << index << ". The number of contours is: " << contours_num << " bigger than 4. Change the calibration image" << std::endl;
		return(-1);
	}
	/**************************************** 2. Contour sorting  ********************************************/
	SortContours(&root); // Sort contours from left to right

	/************************************** 3. Get contours points *******************************************/
	cv::Point2f shift = cv::Point2f(0, undist_img.rows * (1 - roi) - 10);
	GetFeaturePoints(&root, img_points, shift); // Sort contour points clockwise (start from the top left point)

	for (int i = 0; i < (int)img_points.size() - 1; i++) {
		line(undist_img, img_points[i], img_points[i + 1], cvScalar(255, 0, 0), 1, CV_AA, 0); // Draw contours
	}

	if (img_points.size() != num) { // Check points count
		std::cout << "Too few points were found" << std::endl;
		return(-1);
	}
	return(0);
}

/**************************************************************************************************************
*
* @brief  			Search contours in input image
*
* @param  	in		const Mat &img - input image
* 			in/out	CvSeq** root - sequence of contours
* 			in		CvMemStorage *storage - memory storage
*			in		int min_size - empiric bound for minimal allowed perimeter for contour squares
*
* @return 			Functions returns the number of contours which were found.
*
* @remarks 		The function applies adaptive threshold on input image and searches contours in image.
* 					If 4 contours are found then function has been terminated. Otherwise it changes block size
* 					for adaptive threshold and tries again.
*
**************************************************************************************************************/
int SearchContours::GetContours(const cv::Mat &img, CvSeq** root, CvMemStorage *storage, int min_size)
{
	const int min_dilations = 0;
	const int max_dilations = 0;

	cv::Mat temp_threshold_rgb(img.rows, img.cols, CV_8UC3, cv::Scalar(0, 0, 0, 0));

	for (int k = 0; k < 6; k++) {
		int block_size = cvRound(MIN(img.cols, img.rows) * (k % 2 == 0 ? 0.2 : 0.1)) | 1;
		for (int dilations = min_dilations; dilations <= max_dilations; dilations++)
		{
			cv::Mat temp_threshold;
			cv::Mat temp_threshold_rgb(img.rows, img.cols, CV_8UC3, cv::Scalar(0, 0, 0, 0));

			/*********************** Thresholding ***************************/
#if 1
			//Adaptive threshold
			adaptiveThreshold(img, temp_threshold, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, block_size, (k / 2) * 5);
#else
			// Empiric threshold level
			double mean = cvAvg(img).val[0];
			int thresh_level = cvRound(mean - 10);
			thresh_level = MAX(thresh_level, 10);

			threshold(img, temp_threshold, thresh_level, 255, CV_THRESH_BINARY);
#endif

#if SHOW_ALL_CONTOURS
			//			if(debug_mode)
			//				showImg(temp_threshold);
#endif
			/*********************** Dilatation ***************************/
			//cvDilate(temp_threshold, temp_threshold, 0, dilations);

			/*********************** Generate Quads ***********************/
			IplImage copy = temp_threshold;
			IplImage* tmp = &copy;

			// create temporary storage for contours and the sequence of pointers to found quadrangles
			cv::Ptr<CvMemStorage> temp_storage;
			temp_storage = cvCreateChildMemStorage(storage);
			*root = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSeq*), temp_storage);

			// initialize contour retrieving routine
			CvContourScanner scanner = cvStartFindContours(tmp, temp_storage, sizeof(CvContourEx), cv::RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

			// get all the contours one by one
			CvSeq *src_contour = 0;
			while ((src_contour = cvFindNextContour(scanner)) != 0)
			{
				CvSeq *dst_contour = 0;
				CvRect rect = ((CvContour*)src_contour)->rect;

				// reject contours with too small perimeter
				if (rect.width*rect.height >= min_size)
				{
					int approx_level;
					const int min_approx_level = 1, max_approx_level = MAX_CONTOUR_APPROX;
					for (approx_level = min_approx_level; approx_level <= max_approx_level; approx_level++)
					{
						dst_contour = cvApproxPoly(src_contour, sizeof(CvContour), temp_storage, CV_POLY_APPROX_DP, (float)approx_level);

						if (dst_contour->total == 4)
							break;

						// we call this again on its own output, because sometimes
						// cvApproxPoly() does not simplify as much as it should.
						dst_contour = cvApproxPoly(dst_contour, sizeof(CvContour), temp_storage, CV_POLY_APPROX_DP, (float)approx_level);

						if (dst_contour->total == 4)
							break;
					}

					// reject non-quadrangles
					if (dst_contour->total == 4 && cvCheckContourConvexity(dst_contour))
					{
						if (fabs(cvContourArea(dst_contour, CV_WHOLE_SEQ)) > min_size)
						{

							CvPoint pt[4];
							for (int i = 0; i < 4; i++)
								pt[i] = *(CvPoint*)cvGetSeqElem(dst_contour, i);

							if ((pt[0].x > 10) && (pt[0].y > 10) && (pt[1].x > 10) && (pt[1].y > 10) &&
								(pt[2].x > 10) && (pt[2].y > 10) && (pt[3].x > 10) && (pt[3].y > 10)) {
								CvContourEx* parent = (CvContourEx*)(src_contour->v_prev);
								parent->counter++;
								dst_contour->v_prev = (CvSeq*)parent;
								cvSeqPush(*root, &dst_contour);
							}
						}
					}
				}
			}
			// finish contour retrieving
			cvEndFindContours(&scanner);

			// filter found contours
			FilterContours(root);

			/*#if SHOW_ALL_CONTOURS == 1
			if (debug_mode) {
			for (int idx = 0; idx < (*root)->total; idx++)
			{
			CvSeq * contours = *(CvSeq**)cvGetSeqElem(*root, idx);
			// get contour points
			vector<Point> contour_points;
			for (int i = 0; i < 4; i++) {
			contour_points.push_back(*(CvPoint*)cvGetSeqElem(contours, i));
			}
			for (int i = 0; i < 4; i++) line(temp_threshold_rgb, contour_points[i], contour_points[(i + 1) & 3], Scalar(255, 255, 255), 1, CV_AA, 0);
			}
			showImg(temp_threshold_rgb);
			}
			#endif*/
		}
		// if 4 contours are detected, then break
		if ((*root)->total == CONTOURS_NUM)
			break;
	}

	return ((*root)->total);
}


/**************************************************************************************************************
*
* @brief  			Filter found contours.
*
* @param  	in/out	CvSeq** root - sequence of contours
*
* @return 			-
*
* @remarks 		The function checks all contours from the input sequence and removes contours
* 					which are not located inside another sequence contour or which do not contain
* 					another sequence contour.
*
**************************************************************************************************************/
void SearchContours::FilterContours(CvSeq** root)
{
	int idx = (*root)->total - 1; // index of the last sequence contour
	int contours_num = (*root)->total; // number of contours
	// for each sequence contour
	while (idx >= 0)
	{
		CvSeq * src_contour = *(CvSeq**)cvGetSeqElem(*root, idx); // get idx contour
		// get contour points
		std::vector<cv::Point2f> contour_1;
		for (int i = 0; i < src_contour->total; i++) {
			contour_1.push_back(*(CvPoint*)cvGetSeqElem(src_contour, i));
		}

		bool test = 0;
		for (int j = 0; j < contours_num; j++)
		{
			CvSeq * contour = *(CvSeq**)cvGetSeqElem(*root, j); // get j contour
			// get contour points
			std::vector<cv::Point2f> contour_2;
			for (int i = 0; i < contour->total; i++) {
				contour_2.push_back(*(CvPoint*)cvGetSeqElem(contour, i));
			}

			bool is_inside = 1; // idx contour is inside another contour
			bool is_outside = 1; // another contour is inside the idx contour
			for (int i = 0; i < contour->total; i++) {
				is_inside = is_inside & (pointPolygonTest(contour_1, contour_2[i], false) > 0); // check all points of contour
				is_outside = is_outside & (pointPolygonTest(contour_2, contour_1[i], false) > 0); // check all points of contour
			}
			if (is_inside || is_outside) // if idx contour is inside j contour or j contour is inside idx contour
			{
				test = 1; // don't remove contour
				break;
			}
		}
		if (!test)
		{
			cvSeqRemove(*root, idx); // remove contour
			contours_num--; // decrease contours number
		}
		idx--; // next contour
	}
}


/**************************************************************************************************************
*
* @brief  			Sort contours from left to right
*
* @param  	in		CvSeq** root - sequence of contours
*
* @return 			-
*
* @remarks 		The function searches min value of contour points in X axis and then sorts all contours
* 					according to this value from left to right.
*
**************************************************************************************************************/
void SearchContours::SortContours(CvSeq** root)
{
	CvSeq * src_contour[CONTOURS_NUM];
	CvPoint pt[CONTOURS_NUM][4];
	int min_x[CONTOURS_NUM];
	int sort_seq[CONTOURS_NUM];

	// Search min value of contour points in X axis
	for (int idx = 0; idx < CONTOURS_NUM; idx++)
	{
		src_contour[idx] = *(CvSeq**)cvGetSeqElem(*root, idx);

		for (int i = 0; i < 4; i++) {
			pt[idx][i] = *(CvPoint*)cvGetSeqElem(src_contour[idx], i);
		}

		min_x[idx] = MIN4(pt[idx][0].x, pt[idx][1].x, pt[idx][2].x, pt[idx][3].x);
		sort_seq[idx] = idx;
	}

	// Sort contours from left to right
	for (int j = 0; j < CONTOURS_NUM; j++)
		for (int i = j + 1; i < CONTOURS_NUM; i++)
		{
			if (min_x[sort_seq[i]] < min_x[sort_seq[j]]){
				int tmp = sort_seq[j];
				sort_seq[j] = sort_seq[i];
				sort_seq[i] = tmp;
			}
		}

	// Push sorted contours to the input vector
	cvClearSeq(*root);
	for (int idx = 0; idx < CONTOURS_NUM; idx++)
	{
		cvSeqPush(*root, &src_contour[sort_seq[idx]]);
	}
}


/**************************************************************************************************************
*
* @brief  			Generate vector of contour points in proper order.
*
* @param  	in		CvSeq** root - sequence of contours
* 			out		vector<Point2f> * feature_points
* 			in		Point2f shift
*
* @return 			-
*
* @remarks 		The function sorts contours corners from top-left clockwise, applies shift on corners
* 					coordinates and push the corners to the feature_points array in proper order.
*
**************************************************************************************************************/
void SearchContours::GetFeaturePoints(CvSeq** root, std::vector<cv::Point2f> &feature_points, cv::Point2f shift)
{
	// Sort contour points clockwise (start from the top left point)
	for (int idx = 0; idx < CONTOURS_NUM; idx++)
	{
		CvSeq * src_contour = *(CvSeq**)cvGetSeqElem(*root, idx);
		CvPoint pt[4];

		for (int i = 0; i < 4; i++) {
			pt[i] = *(CvPoint*)cvGetSeqElem(src_contour, i);
		}

		// Calculate Y coordinate of the contour center point
		float ym;
		if (pt[0].x == pt[2].x) {
			ym = (pt[0].y + pt[2].y) / 2;
		}
		else if (pt[1].x == pt[3].x) {
			ym = (pt[1].y + pt[3].y) / 2;
		}
		else {
			float a1 = (float)(pt[2].y - pt[0].y) / (float)(pt[2].x - pt[0].x);
			float b1 = (float)(pt[0].y * pt[2].x - pt[2].y * pt[0].x) / (float)(pt[2].x - pt[0].x);

			float a2 = (float)(pt[3].y - pt[1].y) / (float)(pt[3].x - pt[1].x);
			float b2 = (float)(pt[1].y * pt[3].x - pt[3].y * pt[1].x) / (float)(pt[3].x - pt[1].x);

			ym = (a1 * b2 - a2 * b1) / (a1 - a2);
		}

		// Sort contours corners from top-left clockwise
		CvPoint pt_[4] = { 0 };
		for (int i = 0; i < 4; i++)
		{
			// Contours found by findContours function has direction. Objects are counter-clockwise, and holes are clockwise
			if (CV_IS_SEQ_HOLE(src_contour)) { // Holes
				if ((pt[i].y <= ym) && (pt[(i + 1) & 3].y <= ym)){
					pt_[0] = pt[i];
					pt_[1] = pt[(i + 1) & 3];
					pt_[2] = pt[(i + 2) & 3];
					pt_[3] = pt[(i + 3) & 3];
					break;
				}
			}
			else { // Objects
				if ((pt[i].y <= ym) && (pt[(i + 3) & 3].y <= ym)){
					pt_[0] = pt[i];
					pt_[1] = pt[(i + 3) & 3];
					pt_[2] = pt[(i + 2) & 3];
					pt_[3] = pt[(i + 1) & 3];
					break;
				}
			}
		}
		for (int i = 0; i < 4; i++)
			feature_points.push_back(cv::Point2f(pt_[i].x + shift.x, pt_[i].y + shift.y));
	}
}


/**************************************************************************************************************
*
* @brief  			Convert contours sequence into the vector of points.
*
* @param  	in		CvSeq** root - sequence of contours
* 			out		vector<Point2f> * feature_points
* 			in		Point2f shift
*
* @return 			-
*
* @remarks 		The function convert sequence of contours into points array. The shift is applied on corners
* 					coordinates.
*
**************************************************************************************************************/
void SearchContours::sec2vector(CvSeq** root, std::vector<cv::Point2f> &feature_points, cv::Point2f shift)
{
	for (int idx = 0; idx < (*root)->total; idx++)
	{
		CvSeq * src_contour = *(CvSeq**)cvGetSeqElem(*root, idx);
		for (int i = 0; i < (*root)->elem_size; i++) {
			CvPoint pt = *(CvPoint*)cvGetSeqElem(src_contour, i);
			feature_points.push_back(cv::Point2f(pt.x + shift.x, pt.y + shift.y));
		}
	}
}
