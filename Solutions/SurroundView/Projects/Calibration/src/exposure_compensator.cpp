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

#include "exposure_compensator.hpp"
#include "camera.hpp"

#include <stdlib.h>

/**************************************************************************************************************
 *
 * @brief  			Compensator class constructor.
 *
 * @param  in 		Size mask_size	-	size of compensator mask (screen size)
 *
 * @return 			The function create the Compensator object.
 *
 * @remarks 		The function sets cinf.mask property of new Compensator object.
 *
 **************************************************************************************************************/
Compensator::Compensator(cv::Size mask_size)
{
	cinf.mask = cv::Mat(mask_size, CV_8UC1, cv::Scalar(0));
}

/**************************************************************************************************************
 *
 * @brief  			Get coordinates of rectangle reflection across the x-axis.
 *
 * @param  in 		uint index	-	rectangle index
 *
 * @return 			Rect - rectangle reflection across the x-axis.
 *
 * @remarks 		The function reflect rectangle across the x-axis.
 *
 **************************************************************************************************************/
cv::Rect Compensator::getFlipROI(uint index)
{
	if (index < cinf.roi.size())
	{
		double x1 = (cinf.roi[index].x + 1) * cinf.mask.cols / 2;
		double y1 = (-cinf.roi[index].y + 1) * cinf.mask.rows / 2;
		double x2 = (cinf.roi[index].x + cinf.roi[index].width + 1) * cinf.mask.cols / 2;
		double y2 = (-cinf.roi[index].y - cinf.roi[index].height + 1) * cinf.mask.rows / 2;
		return cv::Rect(cv::Point(x1, y1), cv::Point2f(x2, y2));
	}
	else
		return cv::Rect(0, 0, 0, 0);
}	

/**************************************************************************************************************
 *
 * @brief  			Fill the CompensatorInfo vector private property of Compensator object.
 *
 * @param  	in		vector<Camera*> cameras - vector of Camera objects
 *
 * @return 			-
 *
 * @remarks 		The function fills CompensatorInfo property of Compensator. It calculates mask which
 * 					defines 4 overlap regions and circumscribed rectangles of each region.
 *
 **************************************************************************************************************/
#ifdef CAMERA_HPP_EXIST
void Compensator::feed(std::vector<Camera*> &cameras, std::vector< std::vector<cv::Point3f> > &seam_points)
{
	for(uint i = 0; i < cameras.size(); i++)
		if(seam_points[i].size() < 8)
		{
			std::cout << "Exposure compensator was not generated. Seam " << i << " doesn't contain 8 points" << std::endl;
			return;
		}

	cinf.radius = sqrt(pow(seam_points[0][1].x, 2) + pow(seam_points[0][1].y, 2));

	double height = cinf.mask.rows / cinf.radius / 2;
	double x_gain = 2 * cinf.radius;
	double y_gain = 2 * cinf.radius * cinf.mask.rows / cinf.mask.cols;

	for(uint i = 0; i < cameras.size(); i+=2)
	{
		std::vector<cv::Point> p_left, p_right;
		int sign = pow((double)(-1.0), (int)(i / 2));
		int i_next = NEXT(i, cameras.size() - 1); // Index of the previous camera
		int i_prev = PREV(i, cameras.size() - 1); // Index of the next camera

		p_left.push_back(cv::Point2f((sign * seam_points[i][0].x + cinf.radius) * height, (sign * seam_points[i][0].y + cinf.radius) * height));
		p_left.push_back(cv::Point2f((sign * seam_points[i][1].x + cinf.radius) * height, (sign * seam_points[i][1].y + cinf.radius) * height));
		p_left.push_back(cv::Point2f((sign * seam_points[i_prev][6].y + cinf.radius) * height, (-sign * seam_points[i_prev][6].x + cinf.radius) * height));
		p_left.push_back(cv::Point2f((sign * seam_points[i_prev][7].y + cinf.radius) * height, (-sign * seam_points[i_prev][7].x + cinf.radius) * height));

		p_right.push_back(cv::Point2f((-sign * seam_points[i_next][0].y + cinf.radius) * height, (sign * seam_points[i_next][0].x + cinf.radius) * height));
		p_right.push_back(cv::Point2f((-sign * seam_points[i_next][1].y + cinf.radius) * height, (sign * seam_points[i_next][1].x + cinf.radius) * height));
		p_right.push_back(cv::Point2f((sign * seam_points[i][6].x + cinf.radius) * height, (sign * seam_points[i][6].y + cinf.radius) * height));
		p_right.push_back(cv::Point2f((sign * seam_points[i][7].x + cinf.radius) * height, (sign * seam_points[i][7].y + cinf.radius) * height));

		cinf.roi.push_back(Rect2f(cv::Point2f(sign * seam_points[i][1].x / x_gain, -sign * seam_points[i_prev][6].x / y_gain),
								  cv::Point2f(sign * seam_points[i_prev][7].y / x_gain, sign * seam_points[i][0].y / y_gain)));
		cinf.roi.push_back(Rect2f(cv::Point2f(-sign * seam_points[i_next][0].y / x_gain, sign * seam_points[i][7].y / y_gain),
								  cv::Point2f(sign * seam_points[i][6].x / x_gain, sign * seam_points[i_next][1].x / y_gain)));

		fillConvexPoly(cinf.mask, p_left, cv::Scalar(255)); // Draws a filled convex polygon using all seam points
		fillConvexPoly(cinf.mask, p_right, cv::Scalar(255)); // Draws a filled convex polygon using all seam points
	}

//	erode(cinf.mask, cinf.mask, Mat(), Point(-1, -1), 20, 1, 1);
}
#endif

/**************************************************************************************************************
 *
 * @brief  			Save compensator info
 *
 * @param  	in		char* path - path name
 *
 * @return 			-
 *
 * @remarks 		The function generates grids only for overlap regions which lays on flat bowl bottom for each
 * 					camera and saves the grids into “compensator?folder ?1 grid per camera. Each grid contained
 * 					description of two overlap regions: left and right.
 *					Also the application save the file with texel coordinates of circumscribed rectangle for each
 *					overlap region. The texture mapping application uses this information to copy only overlap
 *					regions from frame buffer when it calculates exposure correction coefficients.
 *
 **************************************************************************************************************/
int Compensator::save(char* path)
{
	double x_gain = 2 * cinf.radius;
	double y_gain = 2 * cinf.radius * cinf.mask.rows / cinf.mask.cols;
	double height = cinf.mask.rows / cinf.radius / 2;

	struct stat st = {0};
	if(stat(path, &st) == -1) { // Check if the 'path' path exists
		//if((mkdir(path, 0700) != 0) && (errno != EEXIST)) // Create 'path' path if doesn't exist
		//system("md test");

		std::cout << "path: " << path << std::endl;

		{
			//std::cout << "mkdir: cannot create directory " << path << std::endl;
			//return(-1);
		}
	}

	for(uint i = 0; i < cinf.roi.size(); i++)
	{
		char file_name[50], file_name_roi[50];
		sprintf(file_name, "./array%d", i + 1);
		sprintf(file_name_roi, "%s/array%d", path, i + 1);

		std::ifstream ifs_ref(file_name);
		if(ifs_ref) // // The file exists, and is open for input
		{
			std::ofstream outC; // Output file
			outC.open(file_name_roi, std::ofstream::out | std::ofstream::trunc); // Any contents that existed in the file before it is open are discarded.

			float vx[3], vy[3], vz[3], tx[3], ty[3];
			while (ifs_ref >> vx[0] >> vy[0] >> vz[0] >> tx[0] >> ty[0] >> vx[1] >> vy[1] >> vz[1] >> tx[1] >> ty[1] >> vx[2] >> vy[2] >> vz[2] >> tx[2] >> ty[2] ) // Read triangle
			{
				if((vz[0] == 0) && (vz[1] == 0) && (vz[2] == 0))
				{
					double x0 = (vx[0] + cinf.radius) * height;
					double y0 = (- vy[0] + cinf.radius) * height;

					double x1 = (vx[1] + cinf.radius) * height;
					double y1 = (- vy[1] + cinf.radius) * height;

					double x2 = (vx[2] + cinf.radius) * height;
					double y2 = (- vy[2] + cinf.radius) * height;


					if((x0 >= 0) && (x0 < cinf.mask.cols) && (y0 >= 0) && (y0 < cinf.mask.rows) &&
					   (x1 >= 0) && (x1 < cinf.mask.cols) && (y1 >= 0) && (y1 < cinf.mask.rows) &&
					   (x2 >= 0) && (x2 < cinf.mask.cols) && (y2 >= 0) && (y2 < cinf.mask.rows))

					{
						uint vertexes_sum = (uint)cinf.mask.at<uchar>(cv::Point(x0, y0)) +
							(uint)cinf.mask.at<uchar>(cv::Point(x1, y1)) +
							(uint)cinf.mask.at<uchar>(cv::Point(x2, y2));

						if(vertexes_sum == 765) // 3 * 255
						{
							outC << vx[0] / x_gain << " " << vy[0] / y_gain << " " << vz[0] << " " << tx[0] << " " << ty[0] << std::endl;
							outC << vx[1] / x_gain << " " << vy[1] / y_gain << " " << vz[1] << " " << tx[1] << " " << ty[1] << std::endl;
							outC << vx[2] / x_gain << " " << vy[2] / y_gain << " " << vz[2] << " " << tx[2] << " " << ty[2] << std::endl;
						}
					}
				}
			}
			outC.close(); // Close file
		}
		else
		{
			std::cout << "Compensator grids have not been saved. File " << file_name << " not found" << std::endl;
			return(-1);
		}
	}

	char file_name[50];
	sprintf(file_name, "%s/compensator", path);
	std::ofstream outC; // Output file
	outC.open(file_name, std::ofstream::out | std::ofstream::trunc); // Any contents that existed in the file before it is open are discarded.
	for(uint i = 0; i < cinf.roi.size(); i++)
	{
		outC << cinf.roi[i].x << " " << cinf.roi[i].y << " ";
		outC << cinf.roi[i].x + cinf.roi[i].width << " " << cinf.roi[i].y + cinf.roi[i].height << std::endl;
	}
	outC.close(); // Close file

	return(0);
}

/**************************************************************************************************************
 *
 * @brief  			Load compensator info
 *
 * @param  	in		char* path - path name
 *
 * @return 			-
 *
 * @remarks 		The function loads texel coordinates of circumscribed rectangle for each overlap region.
 * 					The texture mapping application uses this information to copy only overlap regions from
 * 					frame buffer when it calculates exposure correction coefficients.
 *
 **************************************************************************************************************/
int Compensator::load(char* path)
{
	char file_name[50];
	sprintf(file_name, "%s/compensator", path);
	std::ifstream ifs_ref(file_name);
	if(ifs_ref) // The file exists, and is open for input
	{
		cinf.roi.clear();
		float p1, p2, p3, p4;
		while (ifs_ref >> p1 >> p2 >> p3 >> p4 ) // Read triangle
		{
			cinf.roi.push_back(Rect2f(cv::Point2f(p1, p2), cv::Point2f(p3, p4)));

			std::vector<cv::Point> p;
			p.push_back(cv::Point2f((p1 + 1) * cinf.mask.cols / 2, (p2 + 1) * cinf.mask.rows / 2));
			p.push_back(cv::Point2f((p3 + 1) * cinf.mask.cols / 2, (p2 + 1) * cinf.mask.rows / 2));
			p.push_back(cv::Point2f((p3 + 1) * cinf.mask.cols / 2, (p4 + 1) * cinf.mask.rows / 2));
			p.push_back(cv::Point2f((p1 + 1) * cinf.mask.cols / 2, (p4 + 1) * cinf.mask.rows / 2));
			fillConvexPoly(cinf.mask, p, cv::Scalar(255)); // Draws a filled convex polygon using all seam points
		}
	}
	else
	{
		std::cout << "Compensator has not been loaded. File " << file_name << " not found" << std::endl;
		return(-1);
	}
	return(0);
}
