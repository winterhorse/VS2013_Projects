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

#include "camera_tex.hpp"

using namespace std;
using namespace cv;

/***************************************************************************************
***************************************************************************************/
bool RenderInit()
{
	if(camerasInit() == -1) return(false);	// Camera capturing
	camTexInit();	// Camera textures initialization 
	ecTexInit();	// Exposure correction
	
	glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);  // Enable blending
	glEnable(GL_CULL_FACE);  // Cull back face
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Set framebuffer

	// We have to initialize all utils here, because they need OpenGL context
	if (!modelLoader.Initialize()) cout << "Car model was not initialized" << endl;
	mrt->Initialize();
	fontRenderer->Initialize();
	fontRenderer->SetShader(fontProgram.getHandle());
	return (GL_NO_ERROR == glGetError());
}
/***************************************************************************************
***************************************************************************************/
// Actual rendering here.
void Render()
{
	// Calculate ModelViewProjection matrix
	glm::mat4 mv = glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(px, py, pz)), ry, glm::vec3(1, 0, 0)), rx, glm::vec3(0, 0, 1));
	glm::mat4 mvp = gProjection*mv;
	glm::mat3 mn = glm::mat3(glm::rotate(glm::rotate(glm::mat4(1.0f), ry, glm::vec3(1, 0, 0)), rx, glm::vec3(0, 1, 0)));

	GLuint mrtFBO = 0;
	if (mrt->isEnabled())
	{
		mrtFBO = mrt->getFBO();
		glBindFramebuffer(GL_FRAMEBUFFER, mrtFBO);
		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers); // "1" is the size of DrawBuffers	
	}

	if (1)
	{	
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		
		// Clear background.
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Render camera frames
		int i;

		// Render overlap regions of camera frame with blending
		glUseProgram(renderProgram.getHandle());
		for (int camera = 0; camera < CAMERA_NUM; camera++)
		{
			// Lock the camera frame
			//pthread_mutex_lock(&v4l2_cameras[camera].th_mutex);

			// Get index of the newes camera buffer
			if (v4l2_cameras[camera].fill_buffer_inx == -1) i = 0;
			else  i = v4l2_cameras[camera].fill_buffer_inx;


			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, txtMask[camera]);
			glUniform1i(glGetUniformLocation(renderProgram.getHandle(), "myMask"), 1);
						
			// Set gain value for the camera
			glUniform4f(locGain[0], gain->Gains::gain[camera][0], gain->Gains::gain[camera][1], gain->Gains::gain[camera][2], 1.0);

			// Render overlap regions of camera frame with blending
			glBindVertexArray(VAO[2 * camera]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gTexObj[2 * camera]);
			glUniform1i(glGetUniformLocation(renderProgram.getHandle(), "myTexture"), 0);
			mapFrame(i, camera);
				
			GLint mvpLoc = glGetUniformLocation(renderProgram.getHandle(), "mvp");
			glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp)); 
	
			glDrawArrays(GL_TRIANGLES, 0, vertices[2 * camera]);
			glBindVertexArray(0);
	
			// Release camera frame
			//pthread_mutex_unlock(&v4l2_cameras[camera].th_mutex);
		}

		// Render non-overlap region of camera frame without blending
		glUseProgram(renderProgramWB.getHandle()); 	// Use fragment shader without blending
		glDisable(GL_BLEND); 
		for (int camera = 0; camera < CAMERA_NUM; camera++)
		{
			// Lock the camera frame
			//pthread_mutex_lock(&v4l2_cameras[camera].th_mutex);

			// Get index of the newes camera buffer
			if (v4l2_cameras[camera].fill_buffer_inx == -1) i = 0;
			else  i = v4l2_cameras[camera].fill_buffer_inx;

			// Set gain value for the camera
			glUniform4f(locGain[1], gain->Gains::gain[camera][0], gain->Gains::gain[camera][1], gain->Gains::gain[camera][2], 1.0); // Set gain value for the camera

			// Render non-overlap region of camera frame without blending				
			glBindVertexArray(VAO[2 * camera + 1]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gTexObj[2 * camera+ 1]);
			glUniform1i(glGetUniformLocation(renderProgramWB.getHandle(), "myTexture"), 0);
			mapFrame(i, camera);		
			
			GLint mvpLoc = glGetUniformLocation(renderProgramWB.getHandle(), "mvp");
			glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp)); 
						
			glDrawArrays(GL_TRIANGLES, 0, vertices[2 * camera+ 1]);	// Draw texture
			glBindVertexArray(0);

			// Release camera frame
			//pthread_mutex_unlock(&v4l2_cameras[camera].th_mutex);
		}

		// Render car model
		glm::mat4 carModelMatrix = glm::rotate(glm::rotate(glm::scale(glm::mat4(1.0f), car_scale), glm::radians(CAR_ORIENTATION_X), glm::vec3(1, 0, 0)), 
			glm::radians(CAR_ORIENTATION_Y), glm::vec3(0, 1, 0));
		mvp = gProjection * mv * carModelMatrix;
		mn = glm::mat3(glm::rotate(glm::rotate(glm::rotate(glm::rotate(glm::mat4(1.0f), ry, glm::vec3(1, 0, 0)), rx, glm::vec3(0, 0, 1)),
				glm::radians(CAR_ORIENTATION_X), glm::vec3(1, 0, 0)), glm::radians(CAR_ORIENTATION_Y), glm::vec3(0, 1, 0)));
		
		glUseProgram(carModelProgram.getHandle());
		glEnable(GL_DEPTH_TEST);

		// Set matrices
		glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(mvp)); 
		glUniformMatrix4fv(mvUniform, 1, GL_FALSE, glm::value_ptr(mv)); 
		glUniformMatrix3fv(mnUniform, 1, GL_FALSE, glm::value_ptr(mn)); 

		modelLoader.Draw(carModelProgram.getHandle());

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		fpsValue = report_fps();		
		//expcor++;
	}
	else
	{
		// Try to lock gain mutex
		//if (pthread_mutex_trylock(&gain->Gains::th_mutex) == 0)
		{
			glUseProgram(exposureCorrectionProgram.getHandle());
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);	// Change frame buffer
			
			glEnable(GL_BLEND);
			
			int i;
			// Render camera overlap regions
			for (int camera = 0; camera < CAMERA_NUM; camera++)
			{
				// Lock the camera frame
				//pthread_mutex_lock(&v4l2_cameras[camera].th_mutex);
				
				// Get index of the newes camera buffer
				if (v4l2_cameras[camera].fill_buffer_inx == -1) i = 0;
				else  i = v4l2_cameras[camera].fill_buffer_inx;
				
				// Clear background.
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
					
		   		// Render camera overlap regions
				glBindVertexArray(VAO_EC[camera]);			   
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, gTexObj[2 * camera]);
				glUniform1i(glGetUniformLocation(exposureCorrectionProgram.getHandle(), "myTexture"), 0);
				mapFrame(i, camera);
					
				glDrawArrays(GL_TRIANGLES, 0, vertices_ec[camera]);
				glBindVertexArray(0);

				//char rt[10];
				//sprintf(rt, "PrScr%d.jpg", camera);
				//Mat PrScr(viewport[3], viewport[2], CV_8UC(4));
				//glReadPixels(0, 0, viewport[2], viewport[3], GL_RGBA, GL_UNSIGNED_BYTE, PrScr.data); 
				//imwrite(rt, PrScr);

				glReadPixels(gain->compensator->getFlipROI(camera).x,
					gain->compensator->getFlipROI(camera).y,
					gain->compensator->getFlipROI(camera).width,
					gain->compensator->getFlipROI(camera).height, 
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					gain->Gains::overlap_roi[camera][0].data);
				int next = NEXT(camera, camera_num - 1);
				glReadPixels(gain->compensator->getFlipROI(next).x,
					gain->compensator->getFlipROI(next).y,
					gain->compensator->getFlipROI(next).width,
					gain->compensator->getFlipROI(next).height, 
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					gain->Gains::overlap_roi[camera][1].data);
				
				//char name[10];
				//sprintf(name, "%d.jpg", camera);
				//imwrite(name, overlap_roi[camera][0]);
				//sprintf(name, "%d_.jpg", camera);
				//imwrite(name, overlap_roi[camera][1]);
				

				// Release camera frame
				//pthread_mutex_unlock(&v4l2_cameras[camera].th_mutex);
			}
			
			// Release gain mutex
			//pthread_mutex_unlock(&gain->Gains::th_mutex);	
			// Release gain semaphore
			//sem_post(&gain->Gains::th_semaphore);
			glBindFramebuffer(GL_FRAMEBUFFER, mrtFBO); // Reset framebuffer			
		}
		expcor = 0;
	}

	glDisable(GL_BLEND);
	
	if (mrt->isEnabled())
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, param.disp_width, param.disp_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mrt->RenderSmallQuad(showTexProgram.getHandle());
	}  
		
	stringstream ss;
	ss << std::fixed << std::setprecision(2) << fpsValue;
	string fpsText = "FPS: " + ss.str();
	fontRenderer->RenderText(fpsText.c_str(), 5, 10); 
}
/***************************************************************************************
***************************************************************************************/
void RenderCleanup()
{
	if(fontRenderer != NULL) delete(fontRenderer);
	if(gain!= NULL) delete(gain);
	if(mrt!= NULL) delete(mrt);
	for (uint camera = 0; camera < v4l2_cameras.size(); camera++)
	{
		v4l2_cameras[camera].stopCapturing();	
	}
	for (int camera = 0; camera < VAO_NUM; camera++)
	{
		glDeleteTextures(1, &gTexObj[camera]);
	}
}
/***************************************************************************************
***************************************************************************************/
void sighandler(int signal)
{
	printf("Caught signal %d, setting flaq to quit.\n", signal);
	quit = 1;
}

/***************************************************************************************
***************************************************************************************/
// Program entry.
int main(int argc, char** argv)
{
	////////////////// Read XML parameters /////////////////////
	if (setParam(&param) == -1) return (-1);

	//////////////////////// Display ////////////////////////////
	Display* mDisplay = new Display(WINDOW_WIDTH, WINDOW_HEIGHT, "Rendering");

	//////////////////////// Shaders ////////////////////////////
	if (programsInit() == -1) 
	{
		delete mDisplay;
		return (-1);	
	}
 
	////////////////// Exposure correction //////////////////////
	gain = new Gains(param.disp_width, param.disp_height);
	gain->updateGains();

	/////////////////////// Rendering ///////////////////////////
	if (RenderInit())
	{		
		while (!mDisplay->isClosed())
		{
			int key = mDisplay->getKeyState();	

			switch (key)
			{
			case 00:
				py += 0.5f;
				break;
			case 11:
				py -= 0.5f;
				break;
			case 22:
				px -= 0.5f;
				break;
			case 33:
				px += 0.5f;
				break;
			case 44:
				pz += 0.5f;
				//pz = glm::clamp(pz, CAM_LIMIT_ZOOM_MIN, CAM_LIMIT_ZOOM_MAX); 
				break;
			case 55:
				pz -= 0.5f;
				//pz = glm::clamp(pz, CAM_LIMIT_ZOOM_MIN, CAM_LIMIT_ZOOM_MAX); 
				break;
			case 66:
				rx += glm::radians(0.5f);				
				break;
			case 77:
				rx -= glm::radians(0.5f);
				break;
			case 88:	
				ry += glm::radians(0.5f);
				break;
			case 99:
				ry -= glm::radians(0.5f);
				break;

			default:
				break;
			}

			// Left mouse
			if (mDisplay->isMouseLeftPressed())
			{
				
				if (mDisplay->offset.x > 0)
				{
					rx += glm::radians(0.3f);
				}
				else
				{
					rx -= glm::radians(0.3f);
				}			
			}

			// right mouse
			if (mDisplay->isMouseRightPressed())
			{
				if (mDisplay->offset.x > 0)
				{
					ry -= glm::radians(0.3f);
				}
				else
				{
					ry += glm::radians(0.3f);
				}
			}

			// middle mouse
			if (mDisplay->isMouseMiddlePressed())
			{
				if (mDisplay->offset.x > 0)
				{
					pz += 0.05f;
				}
				else
				{
					pz -= 0.05f;
				}
			}			

			Render();
			mDisplay->update();
		}
		glFinish();
	}

	/////////////////////// Free memory ///////////////////////////
	delete mDisplay;
	RenderCleanup();
	programsDestroj();
	
	return 0;
}


/***************************************************************************************
***************************************************************************************/
//read parameters from xml
int setParam(XMLParameters* xml_param)
{
	if (xml_param->readXML("../Content/settings.xml") == -1)
	{
		cout << "Error of parameters reading" << endl;
		return (-1);
	}
	
	camera_num = xml_param->camera_num;
	if(camera_num > CAMERA_NUM) camera_num = CAMERA_NUM;
	
	for (int i = 0; i < camera_num; i++)
	{
		g_in_width.push_back(xml_param->cameras[i].width);
		g_in_height.push_back(xml_param->cameras[i].height);
	}
	
	gProjection = glm::perspective(45.0f, (float)xml_param->disp_width / (float)xml_param->disp_height, 0.1f, 100.0f);
	car_scale = glm::vec3(xml_param->model_scale[0], xml_param->model_scale[0], xml_param->model_scale[0]);
	fontRenderer = new FontRenderer(xml_param->disp_width, xml_param->disp_height, "../Content/font.png");
	mrt = new MRT(xml_param->disp_width, xml_param->disp_height);
	
	return(0);
}

/***************************************************************************************
***************************************************************************************/
// load and compiler vertex/fragment shaders.
int programsInit()
{
	// Overlap regions
	if (renderProgram.loadShaders(s_v_shader_glm, s_f_shader_b_ec) == -1) // Overlap regions
	{
		cout << "Render program was not loaded" << endl;
		return (-1);
	}
	
	// Exposure correction
	if (exposureCorrectionProgram.loadShaders(s_v_shader, s_f_shader) == -1) // Ecposure correction
	{
		cout << "Exposure correction program was not loaded" << endl;
		return (-1);
	}

	
	// Non-overlap regions
	if (renderProgramWB.loadShaders(s_v_shader_glm, s_f_shader_ec) == -1) // Non-overlap regions
	{
		cout << "Render program was not loaded" << endl;
		return (-1);
	}
	
	// Car Model
	if (carModelProgram.loadShaders(s_v_shader_model, s_f_shader_model) == -1) // Car image
	{
		cout << "Car rendering program was not loaded" << endl;
		return (-1);
	}
	mvpUniform = glGetUniformLocation(carModelProgram.getHandle(), "mvp");
	mvUniform = glGetUniformLocation(carModelProgram.getHandle(), "mv");
	mnUniform = glGetUniformLocation(carModelProgram.getHandle(), "mn");
	
	
	// Show tex
	if (showTexProgram.loadShaders(s_v_shader_tex, s_f_shader_tex) == -1) // Show tex
	{
		cout << "Show text program was not loaded" << endl;
		return (-1);
	}
	
	// Font
	if (fontProgram.loadShaders(s_v_shader_font, s_f_shader_font) == -1) // Font renderer
	{
		cout << "Fonf program was not loaded" << endl;
		return (-1);
	}
	return (0);
}

/***************************************************************************************
***************************************************************************************/
void programsDestroj()
{
	renderProgram.destroyShaders();
	exposureCorrectionProgram.destroyShaders();
	renderProgramWB.destroyShaders();
	carModelProgram.destroyShaders();
	showTexProgram.destroyShaders();
	fontProgram.destroyShaders();
}	
	
/***************************************************************************************
***************************************************************************************/
double report_fps(void)
{
	//static bool first_call = true;
	//static unsigned int fps_count = 0;
	//static struct timespec t_start = { 0, 0 };
	//static struct timespec t_end = { 0, 0 };
	//struct timespec dt;
	//static double fpsValue = 0;
	//
	//if (first_call) {
	//	if (clock_gettime(CLOCK_REALTIME, &t_start) != 0) {
	//		cout << "ERROR: clock_gettime(): error " << errno << ": " << strerror(errno) << endl;
	//		return fpsValue;
	//	}
	//	first_call = false;
	//}

	//if (fps_count >= 100) {
	//	if (clock_gettime(CLOCK_REALTIME, &t_end) != 0) {
	//		cout << "ERROR: clock_gettime(): error " << errno << ": " << strerror(errno) << endl;
	//		return fpsValue;
	//	}
	//	Timespec_Sub(&dt, &t_end, &t_start);
	//	fpsValue = (double)fps_count / (double)Timespec_Double(&dt); 
	//	memcpy(&t_start, &t_end, sizeof(timespec));
	//	fps_count = 0;
	//}
	//++fps_count;

	static double fpsValue = 0;

	return fpsValue;
}

/***************************************************************************************
***************************************************************************************/
// 2D texture init
void texture2dInit(GLuint* texture)
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
void bufferObjectInit(GLuint* text_vao, GLuint* text_vbo, GLfloat* vert, int num)
{
	// rectangle
	glBindBuffer(GL_ARRAY_BUFFER, *text_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 5 * num, &vert[0], GL_STATIC_DRAW);
	glBindVertexArray(*text_vao);
	glBindBuffer(GL_ARRAY_BUFFER, *text_vbo);
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
void vLoad(GLfloat** vert, int* num, string filename)
{
	ifstream input(filename.c_str());
	*num = count(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>(), '\n'); // Get line number from the array file
	input.clear();
	input.seekg(0, ios::beg); // Returning to the beginning of fstream

	*vert = NULL; 
	*vert = (GLfloat*)malloc((*num) * 5 * sizeof(GLfloat));	
	if (*vert == NULL) 	{
		cout << "Memory allocation did not complete successfully" << endl; 
	} 
	for (int k = 0; k < (*num) * 5; k++)
	{
		input >> (*vert)[k];
	}
	input.close();
}

/***************************************************************************************
***************************************************************************************/
int camerasInit()
{
	for (int i = 0; i < CAMERA_NUM; i++)
	{
#if defined(CAMERAS)
		string dev_name = "/dev/video" + to_string(i);
#else
		string dev_name = "../Content/camera_inputs/src_" + to_string(i + 1);		
#endif		
		v4l2Camera v4l2_camera(g_in_width[i], g_in_height[i], 0, 0, dev_name.c_str());
		v4l2_cameras.push_back(v4l2_camera);
		if (v4l2_cameras[i].captureSetup() == -1)
		{ 
			cout << "v4l_capture_setup failed camera " << i << endl;
			return(-1);
		}
	}


#ifdef IMX8QM	
	for (int i = 0; i < CAMERA_NUM; i++) // Start capturing
		if (v4l2_cameras[i].startCapturing() == -1) return(-1);

	for (int i = 0; i < CAMERA_NUM; i++) // Get frames from cameras
		if (v4l2_cameras[i].getFrame() == -1) return(-1);
#else	
	for (int i = 0; i < CAMERA_NUM; i++) // Get frames from cameras
		if (v4l2_cameras[i].getFrame() == -1) return(-1);
	
	for (int i = 0; i < CAMERA_NUM; i++) // Start capturing
		if (v4l2_cameras[i].startCapturing() == -1) return(-1);
#endif

	return(0);

}

/***************************************************************************************
***************************************************************************************/
void camTexInit()
{
	///////////////////////////////// Load vertices arrays ///////////////////////////////
	GLfloat* vVertices[VAO_NUM];
	for (int j = 0; j < VAO_NUM; j++)
	{
		int vrt;
		string array = "./array" + to_string((int)(j / 2) + 1) + to_string(j % 2 + 1);
		vLoad(&vVertices[j], &vrt, array);
		vertices.push_back(vrt);
	}	
	
	//////////////////////// Camera textures initialization /////////////////////////////

	GLuint VBO[VAO_NUM];
	glGenVertexArrays(VAO_NUM, VAO);
	glGenBuffers(VAO_NUM, VBO);

	for (int j = 0; j < VAO_NUM; j++)
	{	
		bufferObjectInit(&VAO[j], &VBO[j], vVertices[j], vertices[j]);
		texture2dInit(&gTexObj[j]);
	}

	for (int j = 0; j < camera_num; j++)
	{			
		// j camera mask init
		texture2dInit(&txtMask[j]);
		string mask_name = "./mask" + to_string(j) + ".jpg";
		Mat camera_mask = imread(mask_name, CV_LOAD_IMAGE_GRAYSCALE);
		mask.push_back(camera_mask);
		
		glBindTexture(GL_TEXTURE_2D, txtMask[j]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mask[j].cols, mask[j].rows, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, (uchar*)mask[j].data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

/***************************************************************************************
***************************************************************************************/
void ecTexInit()
{
	////////////////////////// Exposure correction //////////////////////////////
	// Screenshots
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, viewport[2], viewport[3]);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

	///////////////////////////////// Load vertices arrays ///////////////////////////////
	GLfloat* vVertices_ec[4];
	for (int j = 0; j < camera_num; j++)
	{
		int vrt;
		string array = "./compensator/array" + to_string(j + 1);
		vLoad(&vVertices_ec[j], &vrt, array);
		vertices_ec.push_back(vrt);
	}	
	
	//////////////////////// Camera textures initialization /////////////////////////////
	GLuint VBO_EC[4];
	glGenVertexArrays(4, VAO_EC);
	glGenBuffers(4, VBO_EC);
	for (int j = 0; j < 4; j++) {	
		bufferObjectInit(&VAO_EC[j], &VBO_EC[j], vVertices_ec[j], vertices_ec[j]);
	}
	
	locGain[0] = glGetUniformLocation(renderProgram.getHandle(), "myGain");
	locGain[1] = glGetUniformLocation(renderProgramWB.getHandle(), "myGain");
}


/***************************************************************************************
***************************************************************************************/
inline void mapFrame(int buf_index, int camera) 
{
#if defined(CAMERAS)
	glTexDirectVIVMap(GL_TEXTURE_2D, g_in_width[camera], g_in_height[camera], GL_PIXEL_TYPE, (GLvoid **)& v4l2_cameras[camera].buffers[buf_index].start, (const GLuint *)(&v4l2_cameras[camera].buffers[buf_index].offset));
	glTexDirectInvalidateVIV(GL_TEXTURE_2D);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_in_width[camera], g_in_height[camera], 0, GL_RGB, GL_UNSIGNED_BYTE, v4l2_cameras[camera].buffers[buf_index].start);
#endif		
}
