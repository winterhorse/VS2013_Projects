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

#include "src_v4l2.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
//using namespace cv;

int v4l2Camera::exit_flag = 0; // Exit flag
//pthread_mutex_t v4l2Camera::th_mutex = PTHREAD_MUTEX_INITIALIZER;	// Mutex for camera access sinchronization

/**************************************************************************************************************
 *
 * @brief  			v4l2Camera class constructor.
 *
 * @param  in 		int in_width - input frame width
 *					int in_height - input frame height
 *					int in_pixel_fmt - camera pixel format
 *					int in_mem_type - memory type
 *					const char* in_device - camera device name
 *
 * @return 			The function creates the v4l2Camera object.
 *
 * @remarks 		The function creates v4l2Camera object and initializes object attributes.
 *
 **************************************************************************************************************/
v4l2Camera::v4l2Camera(int in_width, int in_height, int in_pixel_fmt, int in_mem_type, const char* in_device)
{
	width = in_width;
	height = in_height;
	pixel_fmt = in_pixel_fmt;
	mem_type = in_mem_type;
	device = std::string(in_device) + ".jpg";

	fill_buffer_inx = -1;

	//th_arg = (thread_arg){NULL, NULL, NULL, NULL};
}


/**************************************************************************************************************
 *
 * @brief  			Setup camera capturing
 *
 * @param   		-
 *
 * @return 			The function returns 0 if capturing device was set successfully. Otherwise -1 has been returned.
 *
 * @remarks 		The function opens camera devices and sets capturing mode.
 *
 **************************************************************************************************************/
int v4l2Camera::captureSetup()
{
	int buf_len = (int)(height * width * 3);
	buffers[0].start = new unsigned char[buf_len];
	buffers[0].offset = 0;
	
	cv::Mat img = cv::imread(device.c_str(), CV_LOAD_IMAGE_COLOR);
	//cv::Mat img = cv::imread(device.c_str(), -1);
	if (img.empty())
	{
		std::cout << device << " file is not found" << std::endl;
		return (-1);
	}
	//cvtColor(img, img, cv::COLOR_BGR2RGB);
	//cvtColor(img, img, cv::COLOR_BGR2RGB);
	memcpy(buffers[0].start, (const char*)img.data, (int)(height * width * 3));	

	return(0);
}

/**************************************************************************************************************
 *
 * @brief  			v4l2Camera class destructor.
 *
 * @param  in 		-
 *
 * @return 			The function deletes the v4l2Camera object.
 *
 * @remarks 		The function deletes v4l2Camera object and cleans object attributes.
 *
 **************************************************************************************************************/
v4l2Camera::~v4l2Camera()
{

}

/**************************************************************************************************************
 *
 * @brief  			Start capturing
 *
 * @param   		-
 *
 * @return 			The function returns 0 if capturing was run successfully. Otherwise -1 has been returned.
 *
 * @remarks			The function starts camera capturing
 *
 **************************************************************************************************************/
int v4l2Camera::startCapturing()
{
	fill_buffer_inx = 0;
	return(0);
}

/**************************************************************************************************************
 *
 * @brief  			Stop camera capturing
 *
 * @param   		-
 *
 * @return 			-
 *
 * @remarks			The function stop camera capturing, releases all memory and close camera device.
 *
 **************************************************************************************************************/
void v4l2Camera::stopCapturing()
{	
	delete[] buffers[0].start;
}

/**************************************************************************************************************
 *
 * @brief  			Capturing thread
 *
 * @param   in		void* input_args (thread_arg).
 *									Video inputs:	FILE *fp;	// Video file  
 *													int* fill_buffer_inx;	// 1: filled with camera data, 0: not filled
 *													videobuffer* buffers[BUFFER_NUM];	// Pointer to videobuffer structures
 *
 *									Camera inputs:	int* fd;	// Thread id
 *													int* fill_buffer_inx;	// Index of buffer which contain last captured camera frame
 *													v4l2_buffer* capture_buf[BUFFER_NUM];	// Pointer to captured buffer
 *													videobuffer* buffers[BUFFER_NUM];		// Pointer to videobuffer structures
 *
 * @return 			-
 *
 * @remarks 		The function creates thread with camera frame capturing loop. The capturing loop is terminated
 *					when the exit flag exit_flag is set to 1. Access to camera is synchronized by the mutex th_mutex.
 *					
 *					The function isn't used for image inputs.
 *
 **************************************************************************************************************/
void* v4l2Camera::getFrameThread(void* input_args)
{	
	return (void *)0;	
}

/**************************************************************************************************************
 *
 * @brief  			Create thread with camera frame capturing loop
 *
 * @param   		-
 *
 * @return 			The function returns 0 if capturing thread was created successfully.
 *
 * @remarks 		The function creates thread with camera frame capturing loop.
 *					
 *					The function isn't used for image inputs. It always returns 0.
 *
 **************************************************************************************************************/
int v4l2Camera::getFrame()
{
	return(0);
}

