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


#include "settings.hpp"

/**************************************************************************************************************
 *
 * @brief  			Read calibration settings from xml file.
 *
 * @param  in		const char* filename - name of xml file with project settings
 *
 * @return 			The function returns 0 if all settings were read successfully. Otherwise -1 has been returned.
 * 					The public properties are set.
 *
 * @remarks			The function reeds settings from filename xml file and write them to public properties. 
 *
 **************************************************************************************************************/
int XMLParameters::readXML(const char* filename)
{
	xmlDocPtr pdoc;
	xmlNodePtr pnode;

	pdoc = xmlReadFile(filename, NULL, 0);

	if (pdoc == NULL) {
		std::cout << "Cannot open the " << filename << " file" << std::endl;
		return (-1);
	}

	pnode = xmlDocGetRootElement(pdoc);

	if (pnode == NULL) {
		std::cout << "The " << filename << " document is empty" << std::endl;
		xmlFreeDoc(pdoc);
		return (-1);
	}
	
	for (pnode = pnode->children; pnode != NULL; pnode = pnode->next) {
		if (pnode->type == XML_ELEMENT_NODE) {
			xmlNodePtr pchildren  = pnode->xmlChildrenNode;
			for (pchildren = pnode->children; pchildren != NULL; pchildren = pchildren->next) {
				if (pchildren->type == XML_ELEMENT_NODE) {
					if (setParam(getParam((char*)pchildren->name), (char*)xmlNodeGetContent(pchildren)) == -1) {
						xmlFreeDoc(pdoc);
						return (-1);
					}
				}
			}
		}
	}
	xmlFreeDoc(pdoc);
	return (0);
}

/**************************************************************************************************************
 *
 * @brief  			Set public property with val value
 *
 * @param	in		int num - property number
 *			in		const char* val - property new value
 *
 * @return 			The function returns 0 if public property was set successfully. Otherwise -1 has been returned.
 *
 * @remarks			The function write val value to the public properties with num number. Property can be
 *					unsigned integer, float, string, bool or CamParam structure value.
 *
 **************************************************************************************************************/
int XMLParameters::setParam(int num, const char* val)
{
	int ret_val = 0;
	switch (num)
	{
		case 0: // char* camera_inputs;
			camera_inputs = std::string(val);
			break;
		case 1: // char* camera_model;
			camera_model = std::string(val);
			break;
		case 2: // char* tmplt;
			tmplt = std::string(val);
			break;
		case 3: // int camera_num;
			camera_num = atoi(val);
			if ((camera_num < 1) || (camera_num > 4)) {
				std::cout << "Camera numbers must be in [1, 4]" << std::endl;
				return (-1);
			}
			break;
		case 4: // int disp_height
			ret_val = readUInt(val, &disp_height);
			break;
		case 5: // int disp_width
			ret_val = readUInt(val, &disp_width);
			break;		
		case 6: // bool show_debug_img;
			readBool(val, &show_debug_img);
			break;			
		case 7: // int grid_angles;
			ret_val = readUInt(val, &grid_angles);
			break;	
		case 8: // int grid_start_angle;
			ret_val = readUInt(val, &grid_start_angle);
			break;		
		case 9: // int grid_nop_z;
			ret_val = readUInt(val, &grid_nop_z);
			break;		
		case 10: // float grid_step_x;
			ret_val = readFloat(val, &grid_step_x);
			break;
		case 11: // float bowl_radius;
			ret_val = readFloat(val, &bowl_radius);
			break;
		case 12: // float smooth_angle;
			ret_val = readFloat(val, &smooth_angle);
			break;
		case 13: // char* keyboard;
			keyboard = std::string(val);
			break;	
		case 14: // char* mouse;
			mouse = std::string(val);
			break;	
		case 15: // char* out_disp;
			out_disp = std::string(val);
			break;	
		case 16: // float model_scale[3];
			ret_val = readFloat(val, &model_scale[0]);
			break;	
		case 17: // float model_scale[3];
			ret_val = readFloat(val, &model_scale[1]);
			break;	
		case 18: // float model_scale[3];
			ret_val = readFloat(val, &model_scale[2]);
			break;	
		case 19: // for test
			ret_val = readUInt(val, &temp_len_ex);
			break;
		case 20: // 
			ret_val = readUInt(val, &temp_len_in);
			break;
		case 21: // 
			ret_val = readUInt(val, &temp_distance_t13);
			break;
		case 22: // 
			ret_val = readUInt(val, &temp_distance_t24);
			break;
		case 100: case 101: case 102: case 103: // CamParam cameras[4];
			ret_val = readCamera(val, &cameras[num - 100]);
			break;		
		default:
			std::cout << "Too much parameters in xml file" << std::endl;
			ret_val = -1;
			break;
	}	
	return (ret_val);
}

/**************************************************************************************************************
 *
 * @brief  			Convert and write input string to the integer variable
 *
 * @param	in		const char* src - property value
 *			out		int* dst - pointer to the integer property
 *
 * @return 			The function returns 0 if src contains uint value. Otherwise -1 has been returned.
 *
 * @remarks			The function converts and writes input string src to the integer variable *dst.
 *
 **************************************************************************************************************/
int XMLParameters::readUInt(const char* src, int* dst)
{
	*dst = atoi(src);
	if (*dst < 0) {
		std::cout << "All parameters must be a positive number" << std::endl;
		return (-1);
	}
	return (0);
}

/**************************************************************************************************************
 *
 * @brief  			Convert and write input string to the float variable
 *
 * @param	in		const char* src - property value
 *			out		float* dst - pointer to the float property
 *
 * @return 			The function returns 0 if src contains nonnegative float value. Otherwise -1 has been returned.
 *
 * @remarks			The function converts and writes input string src to the float variable *dst.
 *
 **************************************************************************************************************/
int XMLParameters::readFloat(const char* src, float* dst)
{
	*dst = atof(src);
	if (*dst < 0.0) {
		std::cout << "All parameters must be a positive number" << std::endl;
		return (-1);
	}
	return (0);
}

/**************************************************************************************************************
 *
 * @brief  			Convert and write input string to the bool variable
 *
 * @param	in		const char* src - property value
 *			out		bool* dst - pointer to the bool property
 *
 * @return 			-
 *
 * @remarks			The function converts and writes input string src to the bool variable *dst.
 *
 **************************************************************************************************************/
void XMLParameters::readBool(const char* src, bool* dst)
{
	int intval = atoi(src);
	*dst = (intval != 0);
}

/**************************************************************************************************************
 *
 * @brief  			Convert and write input string to the CamParam structure
 *
 * @param	in		const char* src - property value
 *			out		CamParam* dst - pointer to the CamParam property
 *
 * @return 			The function returns -1 if there are too few or too much parameters in the input src string. 
 *					Otherwise 0 has been returned.
 *
 * @remarks			The function converts and writes input string src to the CamParam structure *dst.
 *
 **************************************************************************************************************/
int XMLParameters::readCamera(const char* src, CamParam* dst)
{
	float buf[6] = {0};
	std::stringstream str;
	str << src;
	int i = 0;
	while ((str) && (i < 6))	
	{
		str >> buf[i];
		i++;
	}
	if (!str)
	{
		std::cout << "Too much parameters for a camera object" << std::endl;
		return (-1);		
	}
	else if (i < 6)
	{
		std::cout << "Too few parameters for a camera object" << std::endl;
		return (-1);		
	}
	dst->height = buf[0];
	dst->width = buf[1];
	dst->sf = buf[2];
	dst->roi = buf[3];
	dst->cntr_min_size = buf[4];
	dst->chessboard_num = buf[5];

	return (0);
}

/**************************************************************************************************************
 *
 * @brief  			Write all public parameters values
 *
 * @param			-	
 *
 * @return 			-
 *
 * @remarks			The function writes all public parameters values to screen
 *
 **************************************************************************************************************/
void XMLParameters::printParam()
{
	std::cout << "Path to camera calibration static images " << camera_inputs << std::endl;
	std::cout << "Path to camera models " << camera_model << std::endl;
	std::cout << "Path to templates " << tmplt << std::endl;
	std::cout << "Number of cameras " << camera_num << std::endl;
	for (int i = 0; i < 4; i++)
	{
		std::cout << "Camera " << i + 1 << std::endl;
		std::cout << "\tResolution " << cameras[i].height << " x " << cameras[i].width << std::endl;
		std::cout << "\tDefisheye scale factor sf = " << cameras[i].sf << std::endl;
		std::cout << "\tROI in which contours will be searched (% of image height) roi = " << cameras[i].roi << std::endl;
		std::cout << "\tContours min size = " << cameras[i].cntr_min_size << std::endl;
		std::cout << "\tChessboard images number = " << cameras[i].chessboard_num << std::endl;
	}
	std::cout << "Display resolution " << disp_height << " x " << disp_width << std::endl;
	std::cout << "Show debug info " << (bool)(show_debug_img == 1) << std::endl;
	std::cout << "Grig parameters" << std::endl;
	std::cout << "\tAngles number " << grid_angles << std::endl;
	std::cout << "\tStart angle " << grid_start_angle << std::endl;
	std::cout << "\tNumber of grid points in z axis " << grid_nop_z << std::endl;
	std::cout << "\tStep in x axis " << grid_step_x << std::endl;
	std::cout << "\tRadius of 3D bowl " << bowl_radius << std::endl;
	std::cout << "Mask angle of smoothing " << smooth_angle << std::endl;
	std::cout << "Keyboard events " << keyboard << std::endl;
	std::cout << "Mouse events " << mouse << std::endl;
	std::cout << "Display file " << out_disp << std::endl;
	std::cout << "Car model scale (x,y,z) " << model_scale[0] << ", " << model_scale[0] << ", " << model_scale[0] << std::endl;
	std::cout << "Template information\n" 
		<< "\tTemplate external len: " << temp_len_ex << "\n" 
		<< "\tTemplate internal len: " << temp_len_in << "\n"
		<< "\tDistance of T1 and T3: " << temp_distance_t13 << "\n"
		<< "\tDistance of T2 and T4: " << temp_distance_t24 << "\n"
		<< std::endl;
}

/**************************************************************************************************************
 *
 * @brief  			Get template width (maximum value of template vertices x coordinate)
 *
 * @param	in		const char* filename - template file name
 *			out		int* val - pointer to the output value
 *
 * @return 			The function returns -1 if template file was not found. Otherwise 0 has been returned.
 *
 * @remarks			The function calculates maximum value of template vertices x coordinate and write this value 
 *					to the *val.
 *
 **************************************************************************************************************/
int XMLParameters::getTmpMaxVal(const char* filename, int* val)
{
	*val = 0;
	struct stat st;
	std::string ref_points_txt = tmplt + "/" + (std::string)filename;
	if (stat(ref_points_txt.c_str(), &st) != 0)
	{
		std::cout << "File " << ref_points_txt << " not found" << std::endl;
		return (-1);
	}
	int x, y;
	std::ifstream ifs_ref(ref_points_txt.c_str());
	while (ifs_ref >> x >> y)
		if (x > *val) *val = x;
	ifs_ref.close();
	return (0);
}
// write the point to template_1.txt file
int XMLParameters::setTmpPoint() {

	cv::Point tmp;
	std::vector<cv::Point> point_template13, point_template24;

	/*Point1 - Point8*/
	/* point 1*/
	tmp = cv::Point(0, 0); 
	point_template13.push_back(tmp); point_template24.push_back(tmp);

	/* point 2*/
	tmp = cv::Point(temp_len_ex, 0); 
	point_template13.push_back(tmp); point_template24.push_back(tmp); 

	/* point 3*/
	tmp = cv::Point(temp_len_ex, temp_len_ex); 
	point_template13.push_back(tmp); point_template24.push_back(tmp);

	/* point 4*/
	tmp = cv::Point(0, temp_len_ex);
	point_template13.push_back(tmp); point_template24.push_back(tmp);

	/* point 5*/
	tmp = cv::Point((temp_len_ex - temp_len_in) / 2.0, (temp_len_ex - temp_len_in) / 2.0); 
	point_template13.push_back(tmp); point_template24.push_back(tmp);

	/* point 6*/
	tmp = cv::Point((temp_len_ex + temp_len_in) / 2.0, (temp_len_ex - temp_len_in) / 2.0);
	point_template13.push_back(tmp); point_template24.push_back(tmp);

	/* point 7*/
	tmp = cv::Point((temp_len_ex + temp_len_in) / 2.0, (temp_len_ex + temp_len_in) / 2.0);
	point_template13.push_back(tmp); point_template24.push_back(tmp);

	/* point 8*/
	tmp = cv::Point((temp_len_ex - temp_len_in) / 2.0, (temp_len_ex + temp_len_in) / 2.0);
	point_template13.push_back(tmp); point_template24.push_back(tmp);

	/*Point9 - Point16*/
	cv::Point tmp1, tmp2;
	/* point 9*/
	tmp1 = cv::Point(temp_len_ex + temp_distance_t13, 0);
	tmp2 = cv::Point(temp_len_ex + temp_distance_t24, 0);
	point_template13.push_back(tmp1); 	
	point_template24.push_back(tmp2);

	/* point 10*/
	tmp1 = cv::Point(2*temp_len_ex + temp_distance_t13, 0);
	tmp2 = cv::Point(2*temp_len_ex + temp_distance_t24, 0);
	point_template13.push_back(tmp1);
	point_template24.push_back(tmp2);

	/* point 11*/
	tmp1 = cv::Point(2 * temp_len_ex + temp_distance_t13, temp_len_ex);
	tmp2 = cv::Point(2 * temp_len_ex + temp_distance_t24, temp_len_ex);
	point_template13.push_back(tmp1);
	point_template24.push_back(tmp2);

	/* point 12*/
	tmp1 = cv::Point(temp_len_ex + temp_distance_t13, temp_len_ex);
	tmp2 = cv::Point(temp_len_ex + temp_distance_t24, temp_len_ex);
	point_template13.push_back(tmp1);
	point_template24.push_back(tmp2);

	/* point 13*/
	tmp1 = cv::Point(temp_len_ex + temp_distance_t13 + (temp_len_ex - temp_len_in) / 2.0, (temp_len_ex - temp_len_in) / 2.0);
	tmp2 = cv::Point(temp_len_ex + temp_distance_t24 + (temp_len_ex - temp_len_in) / 2.0, (temp_len_ex - temp_len_in) / 2.0);
	point_template13.push_back(tmp1);
	point_template24.push_back(tmp2);

	/* point 14*/
	tmp1 = cv::Point(temp_len_ex + temp_distance_t13 + (temp_len_ex + temp_len_in) / 2.0, (temp_len_ex - temp_len_in) / 2.0);
	tmp2 = cv::Point(temp_len_ex + temp_distance_t24 + (temp_len_ex + temp_len_in) / 2.0, (temp_len_ex - temp_len_in) / 2.0);
	point_template13.push_back(tmp1);
	point_template24.push_back(tmp2);

	/* point 15*/
	tmp1 = cv::Point(temp_len_ex + temp_distance_t13 + (temp_len_ex + temp_len_in) / 2.0, (temp_len_ex + temp_len_in) / 2.0);
	tmp2 = cv::Point(temp_len_ex + temp_distance_t24 + (temp_len_ex + temp_len_in) / 2.0, (temp_len_ex + temp_len_in) / 2.0);
	point_template13.push_back(tmp1);
	point_template24.push_back(tmp2);

	/* point 16*/
	tmp1 = cv::Point(temp_len_ex + temp_distance_t13 + (temp_len_ex - temp_len_in) / 2.0, (temp_len_ex + temp_len_in) / 2.0);
	tmp2 = cv::Point(temp_len_ex + temp_distance_t24 + (temp_len_ex - temp_len_in) / 2.0, (temp_len_ex + temp_len_in) / 2.0);
	point_template13.push_back(tmp1);
	point_template24.push_back(tmp2);

	// Write the template point to file
	for (int cameraIndex = 0; cameraIndex < 4; cameraIndex++)
	{
		char file_name[50];
		sprintf(file_name, "template_%d.txt", cameraIndex + 1);
		std::string ref_points_txt = tmplt + "/" + (std::string)file_name;

		std::ofstream outFile; // Output file
		outFile.open(ref_points_txt.c_str(), std::ofstream::out | std::ofstream::trunc); // Any contents that existed in the file before it is open are discarded.

		if (cameraIndex%2 == 0) // T1 and T3
		{
			for (int i = 0; i < point_template13.size(); i++) 
			{
				outFile << point_template13.at(i).x << " " << point_template13.at(i).y << std::endl;
			}
		}
		else			  // T2 and T4
		{
			for (int i = 0; i < point_template24.size(); i++) 
			{
				outFile << point_template24.at(i).x << " " << point_template24.at(i).y << std::endl;
			}
		}

		outFile.close(); // Close file
	}

	return (0);
}

/**************************************************************************************************************
 *
 * @brief  			Get parameter number
 *
 * @param	in		const char* name - parameter  name
 *
 * @return 			int - parameter number.
 *
 * @remarks			The function returns parameter number according to the parameter name. If parameter name 
 *					is not defined then -1 is returned.
 *
 **************************************************************************************************************/
int XMLParameters::getParam(const char* name)
{
	int return_val = -1;
	if (strcmp(name, "camera_inputs") == 0) return_val = 0;
	else if (strcmp(name, "camera_model") == 0) return_val = 1;
	else if (strcmp(name, "template") == 0) return_val = 2;
	else if (strcmp(name, "number") == 0) return_val = 3;
	else if (strcmp(name, "height") == 0) return_val = 4;
	else if (strcmp(name, "width") == 0) return_val = 5;
	else if (strcmp(name, "show_debug_img") == 0) return_val = 6;
	else if (strcmp(name, "angles") == 0) return_val = 7;
	else if (strcmp(name, "start_angle") == 0) return_val = 8;
	else if (strcmp(name, "nop_z") == 0) return_val = 9;
	else if (strcmp(name, "step_x") == 0) return_val = 10;
	else if (strcmp(name, "radius") == 0) return_val = 11;
	else if (strcmp(name, "smooth_angle") == 0) return_val = 12;
	else if (strcmp(name, "keyboard") == 0) return_val = 13;
	else if (strcmp(name, "mouse") == 0) return_val = 14;
	else if (strcmp(name, "display") == 0) return_val = 15;
	else if (strcmp(name, "x_scale") == 0) return_val = 16;
	else if (strcmp(name, "y_scale") == 0) return_val = 17;
	else if (strcmp(name, "z_scale") == 0) return_val = 18;

	// for test
	else if (strcmp(name, "temp_len_ex") == 0) return_val = 19;
	else if (strcmp(name, "temp_len_in") == 0) return_val = 20;
	else if (strcmp(name, "temp_distance_t13") == 0) return_val = 21;
	else if (strcmp(name, "temp_distance_t24") == 0) return_val = 22;

	else if (strcmp(name, "camera1") == 0) return_val = 100;
	else if (strcmp(name, "camera2") == 0) return_val = 101;
	else if (strcmp(name, "camera3") == 0) return_val = 102;
	else if (strcmp(name, "camera4") == 0) return_val = 103;
	return (return_val);		
}