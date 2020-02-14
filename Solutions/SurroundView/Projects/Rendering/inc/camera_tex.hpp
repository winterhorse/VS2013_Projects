/*
*
* Copyright © 2017 NXP
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

#ifndef CAMERA_TEX_HPP_
#define CAMERA_TEX_HPP_

#include "config.hpp"

/*******************************************************************************************
 * Global variables
 *******************************************************************************************/

volatile sig_atomic_t quit = 0;
static int expcor = 0;
static double fpsValue = 0;

//Cameras parameters
XMLParameters param;
int camera_num;						// Cameras number
vector<int> g_in_width;				// Input frame width
vector<int> g_in_height;			// Input frame height
vector<v4l2Camera> v4l2_cameras;	// Camera buffers

// Cameras mapping
GLuint gTexObj[VAO_NUM] = {0};		// Camera textures
GLuint txtMask[CAMERA_NUM] = {0};	// Camera masks textures
vector<cv::Mat> mask;					// Mask images

#ifdef IMX8QM	
#define GL_PIXEL_TYPE GL_RGBA
#define CAM_PIXEL_TYPE V4L2_PIX_FMT_RGB32
#else	
#define GL_PIXEL_TYPE GL_VIV_UYVY
#define CAM_PIXEL_TYPE V4L2_PIX_FMT_UYVY
#endif

GLuint VAO[VAO_NUM];
vector<int> vertices;						   

Programs renderProgram;
Programs renderProgramWB;

// Exposure correction
GLuint VAO_EC[CAMERA_NUM];
vector<int> vertices_ec;

GLint viewport[4];
GLuint fbo, rbo;
Programs exposureCorrectionProgram;
GLint locGain[2];
Gains* gain = NULL;

//Model Loader
glm::mat4 gProjection;	// Initialization is needed
float rx = 0.0f, ry = 0.0f, px = 0.0f, py = 0.0f, pz = -10.0f;
glm::vec3 car_scale = glm::vec3(1.0f, 1.0f, 1.0f);

#define CAR_ORIENTATION_X 90.0f 
#define CAR_ORIENTATION_Y 270.0f
#define CAM_LIMIT_RY_MIN -1.57f
#define CAM_LIMIT_RY_MAX 0.0f
#define CAM_LIMIT_ZOOM_MIN -11.5f
#define CAM_LIMIT_ZOOM_MAX -2.5f

ModelLoader modelLoader;
Programs carModelProgram;
GLuint mvpUniform, mvUniform, mnUniform;

//MRT
MRT* mrt = NULL;	// Initialization is needed
Programs showTexProgram;

//Font Renderer
FontRenderer* fontRenderer = NULL;	// Initialization is needed
Programs fontProgram;

/*******************************************************************************************
 * Global functions
 *******************************************************************************************/
// Get fps rate
extern double report_fps();

extern int programsInit();
extern void programsDestroj();
extern int setParam(XMLParameters* xml_param);
extern void texture2dInit(GLuint* texture);
extern void bufferObjectInit(GLuint* text_vao, GLuint* text_vbo, GLfloat* vert, int num);
extern void vLoad(GLfloat** vert, int* num, string filename);
extern int camerasInit();
extern void camTexInit();
extern void ecTexInit();
extern inline void mapFrame(int buf_index, int camera);
	
#endif /* CAMERA_TEX_HPP_ */
