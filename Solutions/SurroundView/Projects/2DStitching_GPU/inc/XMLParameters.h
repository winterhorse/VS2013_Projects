#ifndef XML_PARAMETERS_HPP
#define XML_PARAMETERS_HPP

// read parameters from xml file
// mainly include:
// 1. pattern size

#include <iostream>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sstream>

using namespace std;

struct PatternPara
{
	int pattern_width;
	int pattern_height;
	int disp_width;
	int disp_height;
	int shift_width_ex;
	int shift_height_ex;
	int shift_width_in;
	int shift_height_in;
	int car_width;
	int car_height;
	int mask_height;

	int pattern_width_actual;
	int car_width_actual;
	int car_height_actual;
};

struct CameraPara
{
	int width;
	int height;
	int contour_min_size;
	double sf;
};

class XMLParameters 
{
public:
	XMLParameters() {}
	~XMLParameters() {}

	bool ReadXML(const char* filename);
	void PrintParam();

private:
	int GetParam(const char* name);
	int SetParam(int num, const char* val);
	int ReadUInt(const char* src, int* dst);
	int ReadFloat(const char* src, float* dst);
	void SetCarSize();
	void SetExShiftSize();
	int ReadCamera(const char* src, CameraPara* dst);


public:
	string calibrate_res;
	PatternPara pattern;
	CameraPara cameraParam[4];
};

#endif
