//#include <opencv2\opencv.hpp>
//#include <iostream>
//
//using namespace std;
//using namespace cv;
//using namespace cv::cuda;
//
//int main()
//{	
//	try
//	{
//		cv::cuda::DeviceInfo _deviceInfo;
//
//		bool _isDeviceOK = _deviceInfo.isCompatible();
//		std::cout << "IsGPUDeviceOK : " << _isDeviceOK << std::endl;
//
//		/*-------------------------以下四种验证方式任意选取一种即可-------------------------*/
//		//获取显卡简单信息
//		cuda::printShortCudaDeviceInfo(cuda::getDevice());  //有显卡信息表示GPU模块配置成功
//
//		//获取显卡详细信息
//		cuda::printCudaDeviceInfo(cuda::getDevice());  //有显卡信息表示GPU模块配置成功
//
//		//获取显卡设备数量
//		int Device_Num = cuda::getCudaEnabledDeviceCount();
//		cout << Device_Num << endl;  //返回值大于0表示GPU模块配置成功
//
//		//获取显卡设备状态
//		cuda::DeviceInfo Device_State;
//		bool Device_OK = Device_State.isCompatible();
//		cout << "Device_State: " << Device_OK << endl;  //返回值大于0表示GPU模块配置成功
//	}
//	catch (exception e){
//		std::cout << "程序异常：" << e.what() << std::endl;
//	}
//
//	system("pause");
//	
//	return 0;
//}