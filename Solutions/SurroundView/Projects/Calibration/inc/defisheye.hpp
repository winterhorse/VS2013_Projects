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


#ifndef SRC_DEFISHEYE_HPP_
#define SRC_DEFISHEYE_HPP_

/*******************************************************************************************
 * Includes
 *******************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <opencv2/highgui/highgui.hpp>

/*******************************************************************************************
 * Types
 *******************************************************************************************/
struct camera_model
{
	std::vector<double>	pol;		/* The polynomial coefficients of radial camera model */
	std::vector<double>	invpol;		/* The coefficients of the inverse polynomial */
	cv::Point2d			center;		/* x and y coordinates of the center in pixels */
	cv::Matx22d			affine;		/* | sx  shy | sx, sy - scale factors along the x/y axis
	                               | shx sy  | shx, shy -  shear factors along the x/y axis */
	cv::Size			img_size;	/* The image size (width/height) */
};


/*******************************************************************************************
 * Classes
 *******************************************************************************************/
/* Defisheye class */
class Defisheye {
public:
	camera_model model;
	int loadModel(std::string filename);
	void createLUT(cv::Mat &mapx, cv::Mat &mapy, float sf);
	
	
	void cam2world(cv::Point3d* p3d, cv::Point2d p2d);
	
private:

};
#endif /* SRC_DEFISHEYE_H */
