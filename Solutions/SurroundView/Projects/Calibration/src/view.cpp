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
#include "view.hpp"

//using namespace std;
//using namespace cv;

/***************************************************************************************
***************************************************************************************/
View::View()
{
	current_prog = 0;
	glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA); 
	glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);  
	glDisable(GL_DEPTH_TEST);	
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/***************************************************************************************
***************************************************************************************/
View::~View()
{
	for (int i = v4l2_cameras.size() - 1; i >= 0; i--)
		v4l2_cameras[i].stopCapturing();	
	for (int i = v_obj.size() - 1; i >= 0; i--)
	{
		glDeleteTextures(1, &v_obj[i].tex);
		glDeleteBuffers(1, &v_obj[i].vbo);
	}
	for (int i = render_prog.size() - 1; i >= 0; i--)	
		render_prog[i].destroyShaders();

}


/***************************************************************************************
***************************************************************************************/
int View::addProgram(const char* v_shader, const char* f_shader)
{
	// load and compiler vertex/fragment shaders.
	Programs new_prog;
	if (new_prog.loadShaders(v_shader, f_shader) == -1) // Non-overlap regions
	{
		std::cout << "Render program was not loaded" << std::endl;
		return (-1);
	}
	render_prog.push_back(new_prog);
	return (render_prog.size() - 1);
}


/***************************************************************************************
***************************************************************************************/
int View::setProgram(uint index)
{
	if (index >= render_prog.size())
	{
		std::cout << "A program with index " << index << " doesn't exist" << std::endl;
		return (-1);		
	}
	current_prog = index;
	glUseProgram(render_prog[index].getHandle());
	return(0);
}


/***************************************************************************************
***************************************************************************************/
void View::cleanView()
{
	// Clear background.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


/***************************************************************************************
***************************************************************************************/
int View::addCamera(int index, int width, int height)
{
#ifdef CAMERAS	
	string dev_name = "/dev/video" + to_string(index);		
#else	
	std::string dev_name = "../Content/camera_inputs/src_" + std::to_string(index + 1);
#endif	
	
	v4l2Camera v4l2_camera(width, height, 0, 0, dev_name.c_str());
	v4l2_cameras.push_back(v4l2_camera);
	
	int current_index = v4l2_cameras.size() - 1;
	
	if (v4l2_cameras[current_index].captureSetup() == -1) 
	{
		std::cout << "v4l_capture_setup failed camera " << index << std::endl;
		return (-1);
	}

	return (current_index);
}



/***************************************************************************************
***************************************************************************************/
int View::runCamera(int index)
{
#ifdef IMX8QM	
	if (v4l2_cameras[index].startCapturing() == -1) return (-1);
	if (v4l2_cameras[index].getFrame() == -1) return (-1);
#else
	if (v4l2_cameras[index].getFrame() == -1) return (-1);
	if (v4l2_cameras[index].startCapturing() == -1) return (-1);
#endif
	return 0;
}


/***************************************************************************************
***************************************************************************************/
int View::addMesh(std::string filename)
{
	///////////////////////////////// Load vertices arrays ///////////////////////////////
	vertices_obj vo_tmp;
	GLfloat* vert;
	vLoad(&vert, &vo_tmp.num, filename);
		
	//////////////////////// Camera textures initialization /////////////////////////////
	glGenVertexArrays(1, &vo_tmp.vao);
	glGenBuffers(1, &vo_tmp.vbo);

	bufferObjectInit(&vo_tmp.vao, &vo_tmp.vbo, vert, vo_tmp.num);
	texture2dInit(&vo_tmp.tex);
	
	v_obj.push_back(vo_tmp); 
	
	if(vert) free(vert);
	
	return (v_obj.size() - 1);
}


/***************************************************************************************
***************************************************************************************/
void View::renderView(int camera, int mesh)
{
	// Render camera frames
	int i;

	// Lock the camera frame
	//pthread_mutex_lock(&v4l2_cameras[camera].th_mutex);

	// Get index of the newes camera buffer
	if (v4l2_cameras[camera].fill_buffer_inx == -1) i = 0;
	else  i = v4l2_cameras[camera].fill_buffer_inx;

	glBindVertexArray(v_obj[mesh].vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, v_obj[mesh].tex);
	glUniform1i(glGetUniformLocation(render_prog[current_prog].getHandle(), "myTexture"), 0);
	
#ifdef CAMERAS
	glTexDirectVIVMap(GL_TEXTURE_2D, v4l2_cameras[camera].getWidth(), v4l2_cameras[camera].getHeight(), GL_PIXEL_TYPE, (GLvoid **)& v4l2_cameras[camera].buffers[i].start, (const GLuint *)(&v4l2_cameras[camera].buffers[i].offset));
	glTexDirectInvalidateVIV(GL_TEXTURE_2D);
#else		
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, v4l2_cameras[camera].getWidth(), v4l2_cameras[camera].getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, v4l2_cameras[camera].buffers[i].start);
#endif	
	
	glDrawArrays(GL_TRIANGLES, 0, v_obj[mesh].num);
	glBindVertexArray(0);
	glFinish();
	
	// Release camera frame
	//pthread_mutex_unlock(&v4l2_cameras[camera].th_mutex);
}

/***************************************************************************************
***************************************************************************************/
// 2D texture init
void View::texture2dInit(GLuint* texture)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/***************************************************************************************
***************************************************************************************/
void View::bufferObjectInit(GLuint* text_vao, GLuint* text_vbo, GLfloat* vert, int num)
{
	// rectangle
	glBindBuffer(GL_ARRAY_BUFFER, *text_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 5 * num, &vert[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(*text_vao);
	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

/***************************************************************************************
***************************************************************************************/
// Load vertices arrays
void View::vLoad(GLfloat** vert, int* num, std::string filename)
{
	std::ifstream input(filename.c_str());
	*num = count(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>(), '\n'); // Get line number from the array file
	input.clear();
	input.seekg(0, std::ios::beg); // Returning to the beginning of fstream

	*vert = NULL; 
	*vert = (GLfloat*)malloc((*num) * 5 * sizeof(GLfloat));	
	if (*vert == NULL) {
		std::cout << "Memory allocation did not complete successfully" << std::endl;
	} 
	for (int k = 0; k < (*num) * 5; k++)
	{
		input >> (*vert)[k];
	}
	input.close();
}

/***************************************************************************************
***************************************************************************************/
int View::createMesh(cv::Mat xmap, cv::Mat ymap, std::string filename, int density, cv::Point2f top)
{
	if ((xmap.rows == 0) || (xmap.cols == 0) || (ymap.rows == 0) || (ymap.cols == 0))
	{
		std::cout << "Mesh was not generated. LUTs are empty" << std::endl;
		return (-1);	
	}		
	
	int rows = xmap.rows / density;
	int cols = xmap.cols / density;
	
	float x_norm = 1.0 / xmap.cols;
	float y_norm = 1.0 / xmap.rows;
	
	std::ofstream outC; // Output vertices/texels file
	outC.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc); // Any contents that existed in the file before it is open are discarded.
	for (int row = 1; row < rows; row++)
	{
		for (int col = 1; col < cols; col++)
		{
		    /****************************************** Get triangles *********************************************
		     *   							  v3 _  v2
		     *   Triangles orientation: 		| /|		1 triangle (v4-v1-v2)
		     *   								|/_|		2 triangle (v4-v2-v3)
		     *   							  v4   v1
		     *******************************************************************************************************/
			// Vertices
			cv::Point2f v1 = cv::Point2f(col * density, row * density);
			cv::Point2f v2 = cv::Point2f(col * density, (row - 1) * density);
			cv::Point2f v3 = cv::Point2f((col - 1) * density, (row - 1) * density);
			cv::Point2f v4 = cv::Point2f((col - 1) * density, row * density);

			// Texels
			cv::Point2f p1 = cv::Point2f(xmap.at<float>(v1), ymap.at<float>(v1));
			cv::Point2f p2 = cv::Point2f(xmap.at<float>(v2), ymap.at<float>(v2));
			cv::Point2f p3 = cv::Point2f(xmap.at<float>(v3), ymap.at<float>(v3));
			cv::Point2f p4 = cv::Point2f(xmap.at<float>(v4), ymap.at<float>(v4));

			if ((p2.x > 0) && (p2.y > 0) && (p2.x < xmap.cols) && (p2.y < xmap.rows) &&	// Check if p2 belongs to the input frame
			   (p4.x > 0) && (p4.y > 0) && (p4.x < xmap.cols) && (p4.y < xmap.rows))		// Check if p4 belongs to the input frame
			{
				// Save triangle points to the output file
				/*******************************************************************************************************
				 *   							  		v2
				 *   1 triangle (v4-v1-v2): 		  /|
				 *   								 /_|
				 *   							  v4   v1
				 *******************************************************************************************************/
				if ((p1.x > 0) && (p1.y > 0) && (p1.x < xmap.cols) && (p1.y < xmap.rows))	// Check if p1 belongs to the input frame
				{
					outC << (top.x + v1.x * x_norm)  << " " << (top.y - v1.y * y_norm) << " " << 0 << " " << p1.x * x_norm << " " << p1.y * y_norm << std::endl;
					outC << (top.x + v2.x * x_norm)  << " " << (top.y - v2.y * y_norm) << " " << 0 << " " << p2.x * x_norm << " " << p2.y * y_norm << std::endl;
					outC << (top.x + v4.x * x_norm)  << " " << (top.y - v4.y * y_norm) << " " << 0 << " " << p4.x * x_norm << " " << p4.y * y_norm << std::endl;
				}

				/*******************************************************************************************************
				 *   							  v3 _	v2
				 *   2 triangle (v4-v2-v3): 		| /
				 *   								|/
				 *   							  v4
				 *******************************************************************************************************/
				if ((p3.x > 0) && (p3.y > 0) && (p3.x < xmap.cols) && (p3.y < xmap.rows))	// Check if p3 belongs to the input frame)
				{
					outC << (top.x + v4.x * x_norm) << " " << (top.y - v4.y * y_norm) << " " << 0 << " " << p4.x * x_norm << " " << p4.y * y_norm << std::endl;
					outC << (top.x + v2.x * x_norm) << " " << (top.y - v2.y * y_norm) << " " << 0 << " " << p2.x * x_norm << " " << p2.y * y_norm << std::endl;
					outC << (top.x + v3.x * x_norm) << " " << (top.y - v3.y * y_norm) << " " << 0 << " " << p3.x * x_norm << " " << p3.y * y_norm << std::endl;
				}
			}
		}
	}
	outC.close(); // Close file	
	return(0);
}

/***************************************************************************************
***************************************************************************************/

void View::reloadMesh(int index, std::string filename)
{
	GLfloat* vert;
	vLoad(&vert, &v_obj[index].num, filename);	
	glBindBuffer(GL_ARRAY_BUFFER, v_obj[index].vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 5 * v_obj[index].num, &vert[0], GL_DYNAMIC_DRAW);
	if(vert) free(vert);
}



/***************************************************************************************
***************************************************************************************/
int View::changeMesh(cv::Mat xmap, cv::Mat ymap, int density, cv::Point2f top, int index)
{
	if ((xmap.rows == 0) || (xmap.cols == 0) || (ymap.rows == 0) || (ymap.cols == 0))
	{
		std::cout << "Mesh was not generated. LUTs are empty" << std::endl;
		return (-1);	
	}		
	
	int rows = xmap.rows / density;
	int cols = xmap.cols / density;
	
	GLfloat* vert = NULL;
	v_obj[index].num = 6 * rows * cols;
	vert = (GLfloat*)calloc((int)(v_obj[index].num * 5), sizeof(GLfloat));	

	if (vert == NULL) {
		std::cout << "Memory allocation did not complete successfully" << std::endl;
		return(-1);
	} 
	
	float x_norm = 1.0 / xmap.cols;
	float y_norm = 1.0 / xmap.rows;
	
	
	int k =  0;
	for (int row = 1; row < rows; row++)
		for (int col = 1; col < cols; col++)
		{
		    /****************************************** Get triangles *********************************************
		     *   							  v3 _  v2
		     *   Triangles orientation: 		| /|		1 triangle (v4-v1-v2)
		     *   								|/_|		2 triangle (v4-v2-v3)
		     *   							  v4   v1
		     *******************************************************************************************************/
			// Vertices
			cv::Point2f v1 = cv::Point2f(col * density, row * density);
			cv::Point2f v2 = cv::Point2f(col * density, (row - 1) * density);
			cv::Point2f v3 = cv::Point2f((col - 1) * density, (row - 1) * density);
			cv::Point2f v4 = cv::Point2f((col - 1) * density, row * density);

		    // Texels
			cv::Point2f p1 = cv::Point2f(xmap.at<float>(v1), ymap.at<float>(v1));
			cv::Point2f p2 = cv::Point2f(xmap.at<float>(v2), ymap.at<float>(v2));
			cv::Point2f p3 = cv::Point2f(xmap.at<float>(v3), ymap.at<float>(v3));
			cv::Point2f p4 = cv::Point2f(xmap.at<float>(v4), ymap.at<float>(v4));

			if ((p2.x > 0) && (p2.y > 0) && (p2.x < xmap.cols) && (p2.y < xmap.rows) &&	// Check if p2 belongs to the input frame
			   (p4.x > 0) && (p4.y > 0) && (p4.x < xmap.cols) && (p4.y < xmap.rows))		// Check if p4 belongs to the input frame
			{
				// Save triangle points to the output file
				/*******************************************************************************************************
				 *   							  		v2
				 *   1 triangle (v4-v1-v2): 		  /|
				 *   								 /_|
				 *   							  v4   v1
				 *******************************************************************************************************/
				if ((p1.x >= 0) && (p1.y >= 0) && (p1.x < xmap.cols) && (p1.y < xmap.rows))	// Check if p1 belongs to the input frame
				{
					vert[k] = v1.x * x_norm + top.x;
					vert[k + 1] = (top.y - v1.y * y_norm);
					vert[k + 2] = 0;
					vert[k + 3] = p1.x * x_norm;
					vert[k + 4] = p1.y * y_norm;
					
					vert[k + 5] = v2.x * x_norm + top.x;
					vert[k + 6] = (top.y - v2.y * y_norm);
					vert[k + 7] = 0;
					vert[k + 8] = p2.x * x_norm;
					vert[k + 9] = p2.y * y_norm;

					vert[k + 10] = v4.x * x_norm + top.x;
					vert[k + 11] = (top.y - v4.y * y_norm);
					vert[k + 12] = 0;
					vert[k + 13] = p4.x * x_norm;
					vert[k + 14] = p4.y * y_norm;
					
					k += 15;
				}

				/*******************************************************************************************************
				 *   							  v3 _	v2
				 *   2 triangle (v4-v2-v3): 		| /
				 *   								|/
				 *   							  v4
				 *******************************************************************************************************/
				if ((p3.x > 0) && (p3.y > 0) && (p3.x < xmap.cols) && (p3.y < xmap.rows))	// Check if p3 belongs to the input frame)
				{
					vert[k] = v4.x * x_norm + top.x;
					vert[k + 1] = (top.y - v4.y * y_norm);
					vert[k + 2] = 0;
					vert[k + 3] = p4.x * x_norm;
					vert[k + 4] = p4.y * y_norm;
					
					vert[k + 5] = v2.x * x_norm + top.x;
					vert[k + 6] = (top.y - v2.y * y_norm);
					vert[k + 7] = 0;
					vert[k + 8] = p2.x * x_norm;
					vert[k + 9] = p2.y * y_norm;

					vert[k + 10] = v3.x * x_norm + top.x;
					vert[k + 11] = (top.y - v3.y * y_norm);
					vert[k + 12] = 0;
					vert[k + 13] = p3.x * x_norm;
					vert[k + 14] = p3.y * y_norm;
					
					k += 15;
				}
			}
	}
	

	glBindBuffer(GL_ARRAY_BUFFER, v_obj[index].vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 5 * v_obj[index].num, &vert[0], GL_DYNAMIC_DRAW);
	if(vert) free(vert);
	return (0);
}

/***************************************************************************************
***************************************************************************************/
cv::Mat View::takeFrame(int index)
{
	cv::Mat out;

	// Lock the camera frame
	//pthread_mutex_lock(&v4l2_cameras[index].th_mutex);



#ifndef CAMERAS	
	out = cv::Mat(v4l2_cameras[index].getHeight(), v4l2_cameras[index].getWidth(), CV_8UC3, (char*)v4l2_cameras[index].buffers[v4l2_cameras[index].fill_buffer_inx].start);


#elif defined (IMX8QM)
	Mat rgba(v4l2_cameras[index].getHeight(), v4l2_cameras[index].getWidth(), CV_8UC4, (char*)v4l2_cameras[index].buffers[v4l2_cameras[index].fill_buffer_inx].start);
	cvtColor(rgba, out, CV_RGBA2RGB);
#else
	Mat yuv(v4l2_cameras[index].getHeight(), v4l2_cameras[index].getWidth(), CV_8UC2, (char*)v4l2_cameras[index].buffers[v4l2_cameras[index].fill_buffer_inx].start);
	cvtColor(yuv, out, CV_YUV2BGR_Y422);
#endif

	//std::string name = std::string("camera_" + std::to_string(index) + ".jpg");
	//cv::imwrite(name, out);

	glFinish();
	// Release camera frame
	//pthread_mutex_unlock(&v4l2_cameras[index].th_mutex);
	
	return out;
}


/***************************************************************************************
***************************************************************************************/
int View::addBuffer(GLfloat* buf, int num)
{
	///////////////////////////////// Load vertices arrays ///////////////////////////////
	vertices_obj vo_tmp;
	vo_tmp.num = num;
		
	//////////////////////// Camera textures initialization /////////////////////////////
	glGenVertexArrays(1, &vo_tmp.vao);
	glGenBuffers(1, &vo_tmp.vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vo_tmp.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * num, &buf[0], GL_DYNAMIC_DRAW);
	
	v_obj.push_back(vo_tmp); 

	//std::cout << "size : " << v_obj.size() << std::endl;
	
	return (v_obj.size() - 1);
}

/***************************************************************************************
***************************************************************************************/
int View::setBufferAsAttr(int buf_num, int prog_num, char* atr_name)
{
	if ((buf_num < 0) || (buf_num >= (int)v_obj.size()) || (prog_num < 0) || (prog_num >= (int)render_prog.size()))
		return (-1);
	glBindBuffer(GL_ARRAY_BUFFER, v_obj[buf_num].vbo);
	glBindVertexArray(v_obj[buf_num].vao);
	GLint position_attribute = glGetAttribLocation(render_prog[prog_num].getHandle(), atr_name);
	glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_attribute);	
	return (0);
}


/***************************************************************************************
***************************************************************************************/
void View::renderBuffer(int buf_num, int type, int vert_num)
{
	glBindVertexArray(v_obj[buf_num].vao);
	switch (type)
	{
	case 0:
		glLineWidth(2.0);
		glDrawArrays(GL_LINES, 0, vert_num);
		glBindVertexArray(0);
		break;
	case 1:
		//glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, vert_num);
		//glEndTransformFeedback();
		glBindVertexArray(0);
		break;	
	default:
		break;
	}
}

/***************************************************************************************
***************************************************************************************/
void View::updateBuffer(int buf_num, GLfloat* buf, int num)
{
	v_obj[buf_num].num = num;
	glBindBuffer(GL_ARRAY_BUFFER, v_obj[buf_num].vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * num, &buf[0], GL_DYNAMIC_DRAW);
}
