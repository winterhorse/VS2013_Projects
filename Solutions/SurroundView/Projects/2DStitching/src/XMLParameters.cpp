#include "XMLParameters.h"

void XMLParameters::PrintParam()
{
	std::cout << "Pattern information\n"
		<< "\tThe Chessboard size: (" << pattern.pattern_width << " " << pattern.pattern_height << ")\n"
		<< "\tExternal shift size: (" << pattern.shift_width_ex << " " << pattern.shift_height_ex << ")\n"
		<< "\tInternal shift size: (" << pattern.shift_width_in << " " << pattern.shift_height_in << ")\n"
		<< "\tThe  display   size: (" << pattern.disp_width << " " << pattern.disp_height << ")\n"
		<< "\tThe        car size: (" << pattern.car_width << " " << pattern.car_height << ")\n"
		<< "\tThe mask height is : (" << pattern.mask_height << ")\n"
		<< std::endl;
}

bool XMLParameters::ReadXML(const char* filename)
{	
	xmlDocPtr pdoc;
	xmlNodePtr pnode;

	pdoc = xmlReadFile(filename, NULL, 0);

	if (pdoc == NULL)
	{
		std::cout << "Cannot open the " << filename << " file" << std::endl;
		return (false);
	}

	pnode = xmlDocGetRootElement(pdoc);

	if (pnode == NULL)
	{
		std::cout << "The " << filename << " document is empty" << std::endl;
		xmlFreeDoc(pdoc);
		return (false);
	}

	for (pnode = pnode->children; pnode != NULL; pnode = pnode->next)
	{
		if (pnode->type == XML_ELEMENT_NODE)
		{
			xmlNodePtr pchildren = pnode->xmlChildrenNode;
			for (pchildren = pnode->children; pchildren != NULL; pchildren = pchildren->next)
			{
				if (pchildren->type == XML_ELEMENT_NODE)
				{					
					if (SetParam(GetParam((char*)pchildren->name), (char*)xmlNodeGetContent(pchildren)) == -1)
					{
						xmlFreeDoc(pdoc);
						return (false);
					}
				}
			}
		}
	}

	SetCarSize();
	SetExShiftSize();

	xmlFreeDoc(pdoc);
	return true;
}

int XMLParameters::GetParam(const char* name)
{
	int return_val = -1;
	if (strcmp(name, "calibrate_result") == 0) return_val = 0;
	else if (strcmp(name, "pattern_width") == 0) return_val = 1;
	else if (strcmp(name, "display_width") == 0) return_val = 2;
	else if (strcmp(name, "display_height") == 0) return_val = 3;
	else if (strcmp(name, "shift_width_in") == 0) return_val = 4;
	else if (strcmp(name, "shift_height_in") == 0) return_val = 5;
	else if (strcmp(name, "mask_height") == 0) return_val = 6;

	else if (strcmp(name, "pattern_width_actual") == 0) return_val = 7;
	else if (strcmp(name, "car_width_actual") == 0) return_val = 8;
	else if (strcmp(name, "car_height_actual") == 0) return_val = 9;

	else if (strcmp(name, "camera1") == 0) return_val = 100;
	else if (strcmp(name, "camera2") == 0) return_val = 101;
	else if (strcmp(name, "camera3") == 0) return_val = 102;
	else if (strcmp(name, "camera4") == 0) return_val = 103;
	return (return_val);
}

int XMLParameters::SetParam(int num, const char* val)
{
	int res = 0;

	switch (num)
	{
	case 0:
		calibrate_res = string(val);
		break;
	case 1:
		res = ReadUInt(val, &pattern.pattern_width);
		if (0 == res) 
		{
			pattern.pattern_height = pattern.pattern_width;
		}		
		break;
	case 2:
		res = ReadUInt(val, &pattern.disp_width);
		break;
	case 3:
		res = ReadUInt(val, &pattern.disp_height);
		break;
	case 4:
		res = ReadUInt(val, &pattern.shift_width_in);
		break;
	case 5:
		res = ReadUInt(val, &pattern.shift_height_in);
		break;
	case 6:
		res = ReadUInt(val, &pattern.mask_height);
		break;
	case 7:
		res = ReadUInt(val, &pattern.pattern_width_actual);
		break;
	case 8:
		res = ReadUInt(val, &pattern.car_width_actual);
		break;
	case 9:
		res = ReadUInt(val, &pattern.car_height_actual);
		break;
	case 100: case 101: case 102: case 103: // CamParam cameras[4];
		res = ReadCamera(val, &cameraParam[num - 100]);
		break;
	default:
		break;
	}

	return res;
}

void XMLParameters::SetCarSize()
{
	double scale_w = pattern.car_width_actual / (pattern.pattern_width_actual * 1.0f);
	double scale_h = pattern.car_height_actual / (pattern.pattern_width_actual * 1.0f);

	pattern.car_width = scale_w*pattern.pattern_width;
	pattern.car_height = scale_h*pattern.pattern_height;
}

void XMLParameters::SetExShiftSize()
{
	pattern.shift_width_ex = round((pattern.disp_width - pattern.car_width - pattern.pattern_width * 2) / 2);
	pattern.shift_height_ex = round((pattern.disp_height - pattern.car_height - pattern.pattern_height * 2) / 2);
}

int XMLParameters::ReadUInt(const char* src, int* dst)
{
	*dst = atoi(src);
	if (*dst < 0)
	{
		std::cout << "All parameters must be a positive number" << std::endl;
		return (-1);
	}
	return (0);
}

int XMLParameters::ReadFloat(const char* src, float* dst)
{
	*dst = atof(src);
	if (*dst < 0.0)
	{
		std::cout << "All parameters must be a positive number" << std::endl;
		return (-1);
	}
	return (0);
}

int XMLParameters::ReadCamera(const char* src, CameraPara* dst)
{
	int lines = 4;
	
	float buf[4] = { 0 };
	std::stringstream str;
	str << src;
	int i = 0;
	while ((str) && (i < lines))
	{
		str >> buf[i];
		i++;
	}
	if (!str)
	{
		std::cout << "Too much parameters for a camera object" << std::endl;
		return (-1);
	} else if (i < lines)
	{
		std::cout << "Too few parameters for a camera object" << std::endl;
		return (-1);
	}
	dst->height = buf[0];
	dst->width = buf[1];
	dst->sf = buf[2];
	dst->contour_min_size = buf[3];

	return (0);
}



