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


#ifndef SRC_RENDER_HPP_
#define SRC_RENDER_HPP_



#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

/*******************************************************************************************
 * Includes
 *******************************************************************************************/
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

// String
#include <string>

//OpenGL
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>

//Capturing
//#include "src_v4l2.hpp"

#include "Camera.h"
#include "XMLParameters.h"

#include "opencv2/opencv.hpp"

//Shaders
#include "gl_shaders.hpp"
#include "shaders.hpp"

/**********************************************************************************************************************
 * Macros
 **********************************************************************************************************************/
#ifdef IMX8QM	
#define GL_PIXEL_TYPE GL_RGBA
#define CAM_PIXEL_TYPE V4L2_PIX_FMT_RGB32
#else	
#define GL_PIXEL_TYPE GL_VIV_UYVY
#define CAM_PIXEL_TYPE V4L2_PIX_FMT_UYVY
#endif

/**********************************************************************************************************************
 * Types
 **********************************************************************************************************************/
struct vertices_obj			
{
	GLuint	vao;
	GLuint	vbo;
	GLuint	tex;
	int		num;
};

/*******************************************************************************************
 * Classes
 *******************************************************************************************/
/* Render class */
class View {
public:
	View();
	~View();

	int addProgram(const char* v_shader, const char* f_shader);
	int setProgram(uint index);
	void cleanView();
	
	int addCamera(int index, int width, int height);
	int addMesh(std::string filename);
	
	void renderView(int camera, int mesh);
	int runCamera(int index);
	
	int createMesh(cv::Mat xmap, cv::Mat ymap, std::string filename, int density, cv::Point2f top);
	void reloadMesh(int index, std::string filename);
	int changeMesh(cv::Mat xmap, cv::Mat ymap, int density, cv::Point2f top, int index);
	
	cv::Mat takeFrame(int index);
	
	
	int getVerticesNum(uint num) {if (num < v_obj.size()) return (v_obj[num].num); return (-1);}
	
	
	int addBuffer(GLfloat* buf, int num);
	int setBufferAsAttr(int buf_num, int prog_num, char* atr_name);
	void renderBuffer(int buf_num, int type, int vert_num);
	void updateBuffer(int buf_num, GLfloat* buf, int num);
	//int getProgId(uint index) {if (index < render_prog.size()) return render_prog[index].getHandle();	return (-1);}

public:
	void calcVert(GLfloat** vert, int* num, cv::Point2f top);
	int createFishEyeMesh(cv::Point2f top);
	void loadFishEyeMesh(int index, cv::Point2f top);

public:
	Camera cameras[4];
	
	
private:
	int current_prog;	
	std::vector<Programs> render_prog;
	std::vector<vertices_obj> v_obj;
	//std::vector<v4l2Camera> v4l2_cameras;	// Camera buffers

	XMLParameters parameter;
	
	void vLoad(GLfloat** vert, int* num, std::string filename);
	void bufferObjectInit(GLuint* text_vao, GLuint* text_vbo, GLfloat* vert, int num);
	void texture2dInit(GLuint* texture);	
};

#endif /* SRC_RENDER_HPP_ */
