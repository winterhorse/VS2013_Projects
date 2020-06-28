//#include  "birdView.hpp"
//
//int main()
//{
//	Mat v[4];
//
//	for (int i = 0; i < 4; i++)
//	{
//		//char buf[10];
//		//sprintf(buf, "./image/%d.png", i);
//		//std::cout << buf << std::endl;
//		//v[i] = imread(buf);
//
//		string image_name = "./image/cali_" + to_string(i + 1) + ".jpg";
//		v[i] = imread(image_name);
//	}
//
//	BirdView b("config.yml");
//
//	// search the key point and save to file, execute only once
//	//b.searchPoints(v);
//
//	//b.setCarSize(240, 380);
//	b.setCarSize(160, 160);
//	b.setShift(200,200);
//	b.setChessSize(60, 60);
//	b.setMaskHeigth(200);
//	b.setInternalShift(20, 20);
//
//	//b.sourcePointClick(v);
//	//b.searchPoints(v);
//
//	imshow("bird view", b.transformView(v));
//
//	waitKey(0);
//
//	//while (1)
//	//{
//	//	imshow("bird view", b.transformView(v));
//	//	if (waitKey(20) == 27)	break;
//	//}
//}
