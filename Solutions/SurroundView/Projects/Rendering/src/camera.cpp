/*
*
* Copyright ?2017 NXP
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice (including the next
* paragraph) shall be included in all copies or substantial portions of the
* Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/


#include "camera.hpp"

/**************************************************************************************************************
 *
 * @brief  			Camera class creator.
 *
 * @param  in 		char *filename - *.txt file with polynomial camera model from Scaramuzza toolbox for Matlab
 * 		   in		float sf - scale factor
 * 		   in 	 	int index - camera index
 *
 * @return 			Functions returns Camera* object if the camera model has been loaded successfully. A return
 * 					value of NULL indicates an error.
 * 					The public properties of Camera object are set: sf, index.
 * 					The roi value is set on default value ?50%.
 *
 * @remarks 		The class creator reads polynomial camera model from filename file. If any problems were
 * 					to occur then creator return NULL. Otherwise it returns the pointer to Camera object.
 *
 **************************************************************************************************************/
Camera* Creator::create(const char *filename, float sf, int index)
{
	Camera* mp = new Camera;
	if((mp->model).loadModel(filename) == -1) { // Read polynomial camera model from filename file
		delete mp;
		return(NULL);
	}
	mp->sf = sf;
	mp->index = index;
	mp->setRoi(50); // Default value is 50%
	
	(mp->model).createLUT(mp->xmap, mp->ymap, sf); // create LUT table
	
	return(mp);
}

/**************************************************************************************************************
 *
 * @brief  			Update LUTs for removing defisheye distortion.
 *
 * @param  in 		float scale - scale factor.
 *
 * @return 			-
 *
 * @remarks 		The function recalculates LUTs for removing defisheye distortion with new value of scale factor.
 *
 **************************************************************************************************************/
void Camera::updateLUT(float scale)
{
	sf = scale;
	model.createLUT(xmap, ymap, sf);
}

/**************************************************************************************************************
 *
 * @brief  			Set template parameters.
 *
 * @param  in 		char *filename - *.txt file with coordinates of reference points
 * 					Size template_size - the size of global template for whole 4 cameras system
 *
 * @return 			Functions returns 0 if the file-status information is obtained. If the file not found
 * 					the function returns -1.
 * 					The private property tmp of Camera object is set.
 *
 * @remarks 		The method sets template parameters (template file name, template size, number of template
 * 					points) if file filename exists.
 * 					The template size is used to normalize reference template point. For front and back cameras
 * 					the tmp.size property (which contain template size) is equal input value of template_size.
 * 					But for left and right cameras the template_size has been rotated.
 *
 **************************************************************************************************************/
int Camera::setTemplate(const char *filename, cv::Size template_size)
{
	struct stat st;
	if(stat(filename, &st) != 0) // Get filename file information and check if statistics are valid
	{
		std::cout << "File " << filename << " not found" << std::endl;
		return(-1);
	}
	sprintf(tmp.filename, "%s", filename);	// Set reference points file name

	int x, y;
	int max_x = 0;
	tmp.pt_count = 0;
	std::ifstream ifs_ref(filename);
	while (ifs_ref >> x >> y)
	{
		if(x > max_x) max_x = x;	// Get width of reference pattern
		tmp.pt_count++; 			// Set number of reference points
	}
	if(max_x != template_size.width)
		tmp.size = cv::Size(max_x, template_size.width);	// Set template size
	else
		tmp.size = cv::Size(max_x, template_size.height);	// Set template size

	ifs_ref.close();
	poster = template_size;
	return(0);
}


/**************************************************************************************************************
 *
 * @brief  		Set camera intrinsic parameters (camera matrix and distortion coefficients).
 *
 * @param  in	char *filepath - path to the folder which contains chessboard images 
 *				char *filename - chessboard *.jpg file name
 *				int img_num - number of calibrating images
 * 				Size patternSize - number of chessboard corners in horizontal and vertical directions
 *
 * @return 		The function returns 0 value if all of the corners are found and they are placed in a certain
 * 				order (row by row, left to right in every row). Otherwise -1 has been returned.
 * 				The private property param and public properties xmap, ymap of Camera object is set.
 *
 * @remarks		The intrinsic camera parameters are calculated for the camera after removing fisheye transformation.
 * 				Therefore the estimation of intrinsic camera parameters has been made after fisheye distortion has been removed.
 *				The function calculates camera matrix K using images of chessboard.
 *				    |fx   0   cx |
 *				K = |0    fy  cy |	(cx, cy) -   principal point at the image center
 *			    	|0    0    0 |	fx, fy - focal lengths in x and y axis
 * 				Distortion coefficients distCoeffs are set to 0 after defisheye transformation.
 *				LUTs are calculated.
 *
 **************************************************************************************************************/
int Camera::setIntrinsic(const char *filepath, const char *filename, int img_num, cv::Size patternSize)
{
	/***************************************** 1.Load chessboard image ****************************************/
	std::vector<std::vector<cv::Point3f> > object_points;
	std::vector<std::vector<cv::Point2f> > image_points;
	
	// Define chessboard corners in 3D spase
	std::vector<cv::Point3f> obj;		// Chessboard corners in 3D spase
	for (int j = 0; j < patternSize.width * patternSize.height; j++)
		obj.push_back(cv::Point3f(j / patternSize.width, j % patternSize.height, 0.0f));
	
	for (int i = 0; i < img_num; i++)
	{		
		//char img_name[strlen(filepath) + strlen(filename) + 7]; // c++11
		char img_name[256];

		sprintf(img_name, "%s%s%d.jpg", filepath, filename, i);		

		cv::Mat chessboard_img = cv::imread(img_name, CV_LOAD_IMAGE_COLOR);
		if (chessboard_img.empty()) // Check if chessboard had been loaded
		{
			std::cout << "The " << img_name << " image not found" << std::endl;
			return (-1);
		}

		/************** 2. Calculate maps for fisheye undistortion using Scarramuza calibrating data **************/
		remap(chessboard_img, chessboard_img, xmap, ymap, cv::INTER_LINEAR); //remove fisheye distortion

		/******************************* 3. Calculate camera intrinsic parameters **********************************/
		// Convert the image into a grayscale image
		cv::Mat chessboard_gray;
		cvtColor(chessboard_img, chessboard_gray, CV_BGR2GRAY);

		//Find chessboard corners
		std::vector<cv::Point2f> corners;	// Chessboard corners in 2D camera frame
		if (findChessboardCorners(chessboard_img, patternSize, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS))
		{
			// Store points results into the lists
			image_points.push_back(corners);
			object_points.push_back(obj);
		}		
	}
		
	// Calculate intrinsic parameters
	if (object_points.size() > 0)
	{
		param.K = initCameraMatrix2D(object_points, image_points, cv::Size(1280, 800), 0);
		param.distCoeffs = cv::Mat(4, 1, CV_32F, cv::Scalar(0));
		std::cout << "K: \n" << param.K << std::endl;
	}
	else
	{
		std::cout << "Problem with corner detection" << std::endl;
		return (-1);
	}
	return(0);
}


/**************************************************************************************************************
 *
 * @brief  			Set camera extrinsic parameters (rotation and translation vectors).
 *
 * @param  in 		const Mat &img - captured calibrating frame from camera
 *
 * @return 			Functions returns 0 if camera extrinsic parameters have been set. A return value of ?
 * 					indicates some problems.
 *					The private properties param and radius of Camera object are set.
 *
 * @remarks 		The function calculate extrinsic camera parameters finding an object pose from 3D-2D point
 * 					correspondences.
 * 					To estimate extrinsic parameters a special template with patterns of knowing size must be used.
 * 					The application identifies all pattern corners in the captured camera images and establishes
 * 					a correspondence with the real world distance of these corners. Using these correspondences
 * 					the extrinsic camera parameters (rotation and translation vectors) have been estimated.
 *					The estimation of extrinsic parameters has been made after fisheye distortion has been removed.
 *					It is necessary to set templates parameters and intrinsic camera parameters before the extrinsic
 *					camera parameters will be calculated.
 *
 *					Important: calibration patterns must be visible on the captured frame.
 **************************************************************************************************************/
int Camera::setExtrinsic(const cv::Mat &img)
{
	int x, y;

	/******************************************* 1. Defisheye *************************************************/
	cv::Mat und_img;
	remap(img, und_img, xmap, ymap, cv::INTER_LINEAR); //Remap

	/*************************************** 2. Get 3D reference points ***************************************/
	tmp.ref_points.clear();
	std::ifstream ifs_ref((char*)tmp.filename);
	while (ifs_ref >> x >> y)
		tmp.ref_points.push_back(cv::Point3f(x, y, 0));

	// Normalization into [-1, 1] according template size
	for(uint j = 0; j < tmp.ref_points.size(); j ++) // Normalize 3D coordinates
	{
		tmp.ref_points[j].x = (2 * tmp.ref_points[j].x - tmp.size.width) / poster.width;
		tmp.ref_points[j].y = (2 * tmp.ref_points[j].y - tmp.size.height) / poster.width;
	}

	/************************************ 3. Get points from distorted image ***********************************/
	img_p.clear();
	if (getImagePoints(und_img, tmp.pt_count, img_p) != 0)
    	return(-1);
	
	/************************ 4. Find an object pose from 3D-2D point correspondences. *************************/
	std::vector<cv::Point2f> image_points;
	std::vector<cv::Point3f> object_points;
	for (uint j = 0; j < img_p.size(); j++) {
		image_points.push_back(img_p[j]);
		object_points.push_back(cv::Point3f(tmp.ref_points[j].x, tmp.ref_points[j].y, 0.0));
	}

	cv::Mat matImgPoints(image_points);
	cv::Mat matObjPoints(object_points);
	solvePnP(matObjPoints, matImgPoints, param.K, param.distCoeffs, param.rvec, param.tvec);


#if 0
	/********************************* 5. Calculate camera world position. *************************************
	 * Translation and rotation vectors from solvePnP are telling where is the object in camera's coordinates.
	 * To determine world coordinates of a camera we need to get an inverse transform.
	 ***********************************************************************************************************/
	Mat R;
	Rodrigues(param.rvec, R);
	Mat cameraRotationVector;
	Rodrigues(R.t(), cameraRotationVector);
	Mat cameraTranslationVector = -R.t() * param.tvec;
	cout << "Camera translation  " <<  cameraTranslationVector << endl;
	cout << "Camera rotation  " <<  cameraRotationVector << endl;
#endif

	radius = sqrt((double)pow(tmp.ref_points[0].y, 2) + (double)pow(tmp.ref_points[0].x, 2));

	
	return(0);
}


/**************************************************************************************************************
 *
 * @brief  			Get maximum number of grid rows in z axis.
 *
 * @param  in 		double radius - radius of flat circle bottom of bowl. The radius must be defined relative
 * 									to template width. The template width (in pixels) is considered as 1.0.
 * 		   in		double step_x -	step in x axis which is used to define grid points in z axis.
 * 									Step in z axis: step_z[i] = (i * step_x)^2, i = 1, 2, ... - number of point.
 *
 * @return 			Functions returns the maximum number of grid rows in z axis which can be rendered for defined
 * 					radius and step_x. It means that if we add one more grid row in z axis, then vertexes of this
 * 					row will not belong to the input camera frame. It will be outside of the camera FOV.
 *
 * @remarks 		The output is calculated for the input values of radius and step.
 *
 **************************************************************************************************************/
int Camera::getBowlHeight(double radius, double step_x)
{
	if (param.rvec.empty() || param.tvec.empty() || param.K.empty())
		return (0);	
	
	int num = 1;
	bool next_point = true;

	// Get mask for defisheye transformation
	cv::Mat mask(xmap.rows, xmap.cols, CV_8U, cv::Scalar(255));

	remap(mask, mask, xmap, ymap, cv::INTER_LINEAR);	

	// Get 3D points projection into 2D image for bowl side with x = 0 (z = (y - radius)^2)
	while((next_point) && (num < 100))
	{
		std::vector<cv::Point3f> p3d;
		std::vector<cv::Point2f> p2d;

		double new_point = num * step_x;
		p3d.push_back(cv::Point3f(0, -radius - new_point, -new_point * new_point)); // Get num point for 3D template
		projectPoints(p3d, param.rvec, param.tvec, param.K, param.distCoeffs, p2d); // Project the point into 2D image

		if((p2d[0].y >= 0) && (p2d[0].y < mask.rows) && (p2d[0].x >= 0) && (p2d[0].x < mask.cols))
		{
			if((mask.data != NULL) && (mask.at<uchar>((round)(p2d[0].y), (round)(p2d[0].x)) != 255))
			{
				next_point = false;
			}
		}
		else next_point = false;
		num++;
	}
	return(MAX(0, (num - 2)));
}

/**************************************************************************************************************
 *
 * @brief  			Search pattern points in captured image from the camera.
 *
 * @param  in		Mat &undist_img - defisheye captured image from the camera
 * 		   in		uint num - expected number of reference points
 * 		   out 		vector<Point2f> &img_points - vector of image points
 *
 * @return 			Functions returns 0 if points have been found successfully. A return value of ? indicates
 * 					an error.
 *
 * @remarks			The function searches contours in the bottom half of the input image. If 4 quadrangles have been
 * 					found successfully then contours have been sorted from left to right. Corners of each contour
 * 					have been sorted from top-left clockwise.
 * 					Sorted corners have been written to the img_points vector.
 *
 **************************************************************************************************************/
int Camera::getImagePoints(cv::Mat &undist_img, uint num, std::vector<cv::Point2f> &img_points)
{
	/**************************************** 1. Contour detections *******************************************/
	cv::Mat undist_img_gray;
	cvtColor(undist_img, undist_img_gray, CV_RGB2GRAY); // Convert to grayscale

	cv::Mat temp;
	undist_img_gray(cv::Rect(0, undist_img_gray.rows * (1 - roi) - 10, undist_img_gray.cols, undist_img_gray.rows * roi)).copyTo(temp); // Get roi

	cv::Ptr<CvMemStorage> storage;
	storage = cvCreateMemStorage(0);
	CvSeq * root = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvSeq*), storage );

	int contours_num =  GetContours(temp, &root, storage, cntr_min_size); // Get contours


	// If number of contours not equal to CONTOURS_NUM, then complete the calibration process
	if(contours_num < CONTOURS_NUM) {
		sec2vector(&root, img_points, cv::Point2f(0, undist_img.rows * (1 - roi) - 10));
		if(contours_num == 0) {
			std::cout << "Camera " << index << ". No contours were found. Change the calibration image" << std::endl;
			return(-1);
		}
		std::cout << "Camera " << index << ". The number of contours is: " << contours_num  << " fewer than 4. Change the calibration image" << std::endl;
		return(-1);
	}
	else if(contours_num > CONTOURS_NUM) {
		sec2vector(&root, img_points, cv::Point2f(0, undist_img.rows * (1 - roi) - 10));
		std::cout << "Camera " << index << ". The number of contours is: " << contours_num << " bigger than 4. Change the calibration image" << std::endl;
		return(-1);
	}
	/**************************************** 2. Contour sorting  ********************************************/
	SortContours(&root); // Sort contours from left to right

	/************************************** 3. Get contours points *******************************************/
	cv::Point2f shift = cv::Point2f(0, undist_img.rows * (1 - roi) - 10);
	GetFeaturePoints(&root, img_points, shift); // Sort contour points clockwise (start from the top left point)

	for(int i = 0; i < (int)img_points.size() - 1; i++) {
		line(undist_img, img_points[i], img_points[i+1], cvScalar(255,0,0), 1, CV_AA, 0); // Draw contours
	}

	if (img_points.size() != num) { // Check points count
		std::cout << "Too few points were found" << std::endl;
		return(-1);
	}
	return(0);
}
		
/**************************************************************************************************************
 *
 * @brief  			Convert vector of points into array of lines
 *
 * @param  out 		float** lines - array of lines
 *
 * @return 			Functions returns number of array elements.
 *
 * @remarks			The function converts vector of contours points into array of lines. Input data consist of  
 *					contours points in 2D space (x,y coordinates). Output array consist of lines in 3D space.
 *					Each line is described as 6 floats - 2 3D coordinates of its edges. If a point is a part of 
 *					two or more lines, its coordinates will be added to output array more times. The output 
 *					coordinates are normalized.
 *
 **************************************************************************************************************/	
int Camera::getContours(float** lines)
{
	cv::Point2f top = cv::Point2f(((index & 1) - 1.0), ((~index >> 1) & 1));
		
	float x_norm = 1.0 / xmap.cols;
	float y_norm = 1.0 / xmap.rows;
		
	(*lines) = (float*)malloc(3 * 2 * img_p.size() * sizeof(float));
	if ((*lines) == NULL) {
		std::cout << "Memory allocation did not complete successfully" << std::endl;
		return(0);
	}

	for (uint i = 0; i < img_p.size(); i++)
	{
		int next_i = 4 * (int)(i / 4) + ((i + 1) % 4);
		
		(*lines)[6 * i] = img_p[i].x * x_norm + top.x;
		(*lines)[6 * i + 1] = top.y - img_p[i].y * y_norm;
		(*lines)[6 * i + 2] = 0;
		
		(*lines)[6 * i + 3] = img_p[next_i].x * x_norm + top.x;
		(*lines)[6 * i + 4] = top.y - img_p[next_i].y * y_norm;
		(*lines)[6 * i + 5] = 0;
	}

	return (6 * img_p.size());
}
