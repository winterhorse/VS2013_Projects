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


#ifndef SRC_LINES_HPP_
#define SRC_LINES_HPP_

/*******************************************************************************************
 * Includes
 *******************************************************************************************/
#include <opencv2/core/core.hpp>


/*******************************************************************************************
 * Types
 *******************************************************************************************/
struct Line {		// Line description y = alpha * x + beta
	double alpha;
	double beta;
};

struct Seam {		// Seam definition
	Line line;		// Seam coefficients y = alpha * x + beta
	cv::Point2f p1;		// Seam start point
	cv::Point2f p2;		// Seam end point
};

/*******************************************************************************************
 * Inline functions
 *******************************************************************************************/

/**************************************************************************************************************
 *
 * @brief  			Calculate line between 2 points
 *
 * @param  	in		Point2f p1 - first point
 * 			in		Point2f p2 - second point
 *
 * @return 			Line - coefficients of line between p1 and p2 points
 *
 * @remarks 		The function calculates line between 2 points.
 * 					If line is x = X, then alpha = INFINITY and beta = X.
 *
 **************************************************************************************************************/
inline Line getAlphaBeta(cv::Point2f p1, cv::Point2f p2)
{
	Line result;
	if(p1.x == p2.x) // If line is x = X, then alpha = INFINITY and beta = X.
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

/**************************************************************************************************************
 *
 * @brief  			Calculate lines intersection point
 *
 * @param  	in		Line line1 - coefficients of first line
 * 			in		Line line2 - coefficients of second line
 *
 * @return 			Point2f - x and y coordinates of intersection point
 *
 * @remarks 		The function calculates lines intersection and return coordinates of intersection point.
 * 					If lines are parallel the INFINITY value is return.
 * 					If alpha coefficient of a line is equal INFINITY, then line is x = beta.
 *
 **************************************************************************************************************/
inline cv::Point2f getIntersection(Line line1, Line line2)
{
	cv::Point2f result;
	if(line1.alpha == line2.alpha) // If lines are parallel
	{
		result.x = INFINITY;
		result.y = INFINITY;
	}
	else
		if(line1.alpha == INFINITY) // If the first line is x = const
		{
			result.x = line1.beta;
			result.y = line2.alpha * result.x + line2.beta;
		}
		else
			if(line2.alpha == INFINITY) // If the second line is x = const
			{
				result.x = line2.beta;
				result.y = line1.alpha * result.x + line1.beta;
			}
			else // Otherwise
			{
				result.x = (line2.beta - line1.beta) / (line1.alpha - line2.alpha);
				result.y = (line1.alpha * line2.beta - line2.alpha * line1.beta) / (line1.alpha - line2.alpha);
			}
	return result;
}


#endif /* SRC_LINES_HPP_ */
