#include "opencv2/opencv.hpp"
#include <iostream>
#include "BirdEyeView.h"
#include "FisheyeCalibration.h"
#include "Camera.h"

int main()
{
	XMLParameters parameter;
	string name = "./config/config.xml";
	string srcPointFile = "./config/config.yml";

	parameter.ReadXML(name.c_str());
	parameter.PrintParam();

	Camera cameras[4];
	Mat fisheyeMat[4];
	Mat caliImage[4];
	Mat transfromImage[4];
	Mat caliImage_gray[4];

	FisheyeCalibration fisheyeCalibration;
	BirdEyeView birdEyeView;

	for (int i = 0; i < CAMERA_NUMBERS; i++)
	{
		std::string fisheyeFileName = "./ov10635/fisheye/frame" + to_string(i + 1) + "/" + "frame" + to_string(i + 1);
		std::string calibrationRes = "./ov10635/fisheye/calibration_result/calibration_result_" + to_string(i + 1) + ".txt";

		//fisheyeCalibration.CalibrateAndSave2File(12, fisheyeFileName, calibrationRes);

		cameras[i].Init(calibrationRes, parameter, i);	

		std::string srcImage = "./ov10635/srcImage/" + to_string(i + 1) + ".jpg";
		fisheyeMat[i] = imread(srcImage);
	}

	birdEyeView.Init(srcPointFile.c_str(), parameter, cameras);

	//while (true)
	{
		for (int i = 0; i < CAMERA_NUMBERS; i++)
		{
			caliImage[i] = cameras[i].Defisheye(fisheyeMat[i]);
			birdEyeView.GetTransfromView(fisheyeMat[i], transfromImage[i], i);
			//birdEyeView.GetTransfromView(caliImage[i], transfromImage[i], i);
		}

		imshow("bird view", birdEyeView.Run2DStitching(transfromImage));

		//if (birdEyeView.SearchKeyPoints(caliImage))
		//{
			//imshow("bird view", birdEyeView.TransformView(caliImage));
			//imshow("bird view", birdEyeView.TransformView(fisheyeMat));
		//}

		//birdEyeView.TransformView(caliImage);

		//std::cout << "Loop" << std::endl;
	}
	
	waitKey(0);

	return 0;
}