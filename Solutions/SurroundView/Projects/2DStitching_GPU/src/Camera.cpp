
#include "Camera.h"

void Camera::Init(const string& filename, const XMLParameters& param, int index)
{
	this->index = index;
	GetCalibrateParam(filename);

	/* Set camera image size */
	SetCameraImageSize(cv::Size(param.cameraParam[index].width, param.cameraParam[index].height));

	GetUndistortMaps(this->xmap, this->ymap, param.cameraParam[index].sf);
}

void Camera::Run()
{
	std::string srcImage = "./ov10635/srcImage/" + to_string(index + 1) + ".jpg";
	fishEyeImage = imread(srcImage);

	//fishEyeImage = Defisheye(fishEyeImage);
}

void Camera::SetCameraImageSize(const cv::Size size)
{
	this->cameraImageSize = size;
}

Mat Camera::Defisheye(const Mat& image)
{
	cv::Mat defish;
	remap(image, defish, xmap, ymap, INTER_LINEAR);

	return defish;
}

bool Camera::GetUndistortMaps(cv::Mat& mapx, cv::Mat& mapy, const double& scale)
{	
	mapx = Mat(cameraImageSize, CV_32FC1);
	mapy = Mat(cameraImageSize, CV_32FC1);
	Mat R = Mat::eye(3, 3, CV_32F);

	Matx33d newK = intrinsicMatrix;
	newK(0, 0) = intrinsicMatrix(0, 0) / scale; // fx
	newK(1, 1) = intrinsicMatrix(1, 1) / scale; // fy

	fisheye::initUndistortRectifyMap(intrinsicMatrix, distortionCoeffs, R, newK, cameraImageSize, CV_32FC1, mapx, mapy);

	return true;
}

bool Camera::GetCalibrateParam(const string& filename)
{
	string file = filename;
	ifstream fin(file);

	if (!fin.good())
	{
		std::cout << "File " << filename << " not found" << std::endl;
		return false;
	}

	string line;
	int lineNum = 0;
	double val;

	while (getline(fin, line))
	{
		if ((line.length() > 1) && (line.at(0) != '#'))
		{
			stringstream str_stream(line);
			int i = 0;

			switch (lineNum++)
			{
			case 0:	// camera intrinsic matrix
				while (str_stream >> val)
				{
					intrinsicMatrix(i / 3, i % 3) = val;
					i++;
				}
				break;
			case 1: // distortion coeffs
				while (str_stream >> val)
				{
					distortionCoeffs(i++) = val;
				}
				break;
			case 2: // camera new intrinsic matrix
				while (str_stream >> val)
				{
					newIntrinsicMatrix(i / 3, i % 3) = val;
					i++;
				}
				break;

			default:
				return false;
				break;
			}
		}
	}

	return (lineNum == 2 ? true : false);
}
