#include "opencv2/opencv.hpp"
#include <iostream>

#include "nxp_logo.h"
#include "left_logo.h"
#include "right_logo.h"
#include "straight_logo.h"

using namespace std;
using namespace cv;

void PutLogo(cv::Mat& input, int offsetY, int offsetX)
{
	uint32_t pLogoPixel = 0;
	for (int i = 0; i < NXP_LOGO_HEIGHT; i++)
	{
		char* row = (char*)input.ptr(offsetY + i);
		for (int j = 0; j < (NXP_LOGO_WIDTH); j++)
		{
			char* pPixel = row + (offsetX + j) * 3;
			if ((nxp_logo[pLogoPixel + 2] != 0xff) ||
				(nxp_logo[pLogoPixel + 1] != 0xff) ||
				(nxp_logo[pLogoPixel] != 0xff))
			{
				pPixel[0] = nxp_logo[pLogoPixel + 2];
				pPixel[1] = nxp_logo[pLogoPixel + 1];
				pPixel[2] = nxp_logo[pLogoPixel];
			}
			pLogoPixel += 3;
		}
	}
}

void DrawLogo(cv::Mat& input, unsigned char* logo, int logo_h, int logo_w, int offsetY, int offsetX)
{
	if ((offsetY + logo_h) > input.rows)
	{
		offsetY = input.rows - logo_h;
	}

	if ((offsetX + logo_w) > input.cols)
	{
		offsetX = input.cols - logo_w;
	}
	
	uint32_t pPixelCnt = 0;

	for (int i = 0; i < logo_h; i++)
	{
		char* row = (char*)input.ptr(offsetY + i);
		for (int j = 0; j < logo_w; j++)
		{
			char* pPixel = row + (offsetX + j) * 3;
			if ((logo[pPixelCnt + 2] != 0x00) ||
				(logo[pPixelCnt + 1] != 0x00) ||
				(logo[pPixelCnt] != 0x00))
			{
				pPixel[0] = logo[pPixelCnt + 0];
				pPixel[1] = logo[pPixelCnt + 1];
				pPixel[2] = logo[pPixelCnt + 2];
			}
			pPixelCnt += 3;
		}
	}
}




void SaveImageData2File(cv::Mat& image, const std::string filename)
{
	string file = "./icon/" + filename;
	ofstream fout(file);

	int cnt = 0;

	if (image.channels() == 3) 
	{
		for (int i = 0; i < image.rows; i++)
		{
			for (int j = 0; j < image.cols; j++)
			{
				char buf[512];

				sprintf(buf, "0x%0.2x,0x%0.2x,0x%0.2x,", image.at<Vec3b>(i, j)[0], image.at<Vec3b>(i, j)[1], image.at<Vec3b>(i, j)[2]);
				cnt += 3;

				fout << buf;

				if (cnt % 30 == 0)
				{
					fout << "\n";

					cnt = 0;
				}				
			}
			
		}
	}

	fout.close();
}

void PutDirection(cv::Mat& input, cv::Mat& icon, int offsetY, int offsetX)
{
	uint32_t pPixelCnt = 0;

	if ((offsetY + icon.rows) > input.rows)
	{
		offsetY = input.rows - icon.rows;
	}

	if ((offsetX + icon.cols) > input.cols)
	{
		offsetX = input.cols - icon.cols;
	}

	char* pData = (char*)icon.ptr();

	for (int i = 0; i < icon.rows; i++)
	{
		char* row = (char*)input.ptr(offsetY + i);		

		for (int j = 0; j < icon.cols; j++)
		{
			char* pPixel = row + (offsetX + j) * 3;
			if ((pData[pPixelCnt + 2] != 0x0) ||
				(pData[pPixelCnt + 1] != 0x0) ||
				(pData[pPixelCnt] != 0x0))
			{
				pPixel[0] = pData[pPixelCnt];
				pPixel[1] = pData[pPixelCnt + 1];
				pPixel[2] = pData[pPixelCnt + 2];
			}
			pPixelCnt += 3;		
		}
	}
}

void DrawNXPLogo(cv::Mat& outImage)
{
	PutLogo(outImage, outImage.rows - 125, outImage.cols - 225);
}

int main(){
	Mat src = imread("./icon/sv.jpg");

	Mat left = imread("./icon/left.png");
	Mat right = imread("./icon/right.png");
	Mat straight = imread("./icon/straight.png");

	std::cout << "left: " << left.rows << " " << left.cols  << " " << left.channels() << std::endl;
	std::cout << "right: " << right.rows << " " << right.cols << " " << right.channels() << std::endl;
	std::cout << "straight: " << straight.rows << " " << straight.cols << " " << straight.channels() << std::endl;
 	

	if (src.empty()){
		cout << "can not load picture" << endl;

		return -1;
	}

	
	//PutDirection(src, left, 100, 248);
	//PutDirection(src, straight, 100, 337);
	//PutDirection(src, right, 100, 427);

	//SaveImageData2File(left, "left.txt");
	//SaveImageData2File(right, "right.txt");
	//SaveImageData2File(straight, "straight.txt");

	//DrawNXPLogo(src);

	DrawLogo(src, left_logo, 128, 128, 100, 247);	
	DrawLogo(src, straight_logo, 128, 128, 100, 337);
	DrawLogo(src, right_logo, 128, 128, 100, 427);

	namedWindow("LearnOpenCV", CV_WINDOW_AUTOSIZE);
	imshow("LearnOpenCV", src);

	waitKey(0);
	return 0;
}