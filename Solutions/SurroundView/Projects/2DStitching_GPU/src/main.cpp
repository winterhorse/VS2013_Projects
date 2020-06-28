#include <stdio.h>

#include "display.hpp"
#include "view.hpp"

enum viewStates { fisheye_view = 0, defisheye_view, contours_view, grids_view, result_view };
viewStates view_state = fisheye_view;

struct camera_view
{
	int camera_index;
	std::vector<int> mesh_index;
};


std::vector<camera_view> cam_views;	// View indexes

View *view;


void switchState(int new_state)
{
	float* data;
	int data_num;

	//printState(new_state);

	switch (new_state)
	{
	case fisheye_view:
		// Fisheye
		for (uint i = 0; i < 4; i++)
		{
			//std::string filename = "../Content/meshes/original/mesh" + std::to_string(i + 1);
			//view->reloadMesh(cam_views[i].mesh_index[0], filename);

			view->loadFishEyeMesh(cam_views[i].mesh_index[0], cv::Point2f(((i & 1) - 1.0), ((~i >> 1) & 1)));
		}
		break;

	case defisheye_view:
		// Defisheye
		for (uint i = 0; i < 4; i++)
			view->changeMesh(view->cameras[i].GetMapx(), view->cameras[i].GetMapy(), 2, cv::Point2f(((i & 1) - 1.0), ((~i >> 1) & 1)), cam_views[i].mesh_index[0]);
		break;

	//case contours_view:
	//	// Contours searching
	//	data_num = getContours(&data);
	//	contours_buf = view->addBuffer(&data[0], data_num / 3);
	//	view->setBufferAsAttr(contours_buf, 1, (char*)"vPosition");
	//	if (data) free(data);
	//	break;

	//case grids_view:
	//	// Mesh generation
	//	data_num = getGrids(&data);
	//	grid_buf = view->addBuffer(&data[0], data_num / 3);
	//	view->setBufferAsAttr(grid_buf, 1, (char*)"vPosition");
	//	if (data) free(data);
	//	break;

	//case result_view:
	//	// Result view
	//	saveGrids();

	//	std::cout << "finished save grid" << std::endl;

	//	for (uint i = 0; i < cam_views.size(); i++)
	//	{
	//		if (cam_views[i].mesh_index.size() == 1)
	//		{
	//			cam_views[i].mesh_index.push_back(view->addMesh(std::string("array" + std::to_string(i + 1) + "1")));
	//			cam_views[i].mesh_index.push_back(view->addMesh(std::string("array" + std::to_string(i + 1) + "2")));
	//		}
	//		else
	//		{
	//			view->reloadMesh(cam_views[i].mesh_index[1], std::string("array" + std::to_string(i + 1) + "1"));
	//			view->reloadMesh(cam_views[i].mesh_index[2], std::string("array" + std::to_string(i + 1) + "2"));
	//		}
	//	}

		break;

	default:
		break;
	}
	std::cout << "Done" << std::endl;
}

int main()
{
	Display display(WINDOW_WIDTH, WINDOW_HEIGHT, "2D Stitching");

	view = new View;
	if (view->addProgram(s_v_shader, s_f_shader) == -1) return (-1);

	for (int i = 0; i < 4; i++)
	{
		camera_view cam_view;
		cam_view.camera_index = i;
		if (cam_view.camera_index == -1) return (-1);
		//cam_view.mesh_index.push_back(view->addMesh(std::string("../Content/meshes/original/mesh" + std::to_string(i + 1))));
		cam_view.mesh_index.push_back(view->createFishEyeMesh(cv::Point2f(((i & 1) - 1.0), ((~i >> 1) & 1))));
		cam_views.push_back(cam_view);
	}

	for (int i = 0; i < 4; i++)
	{
		view->addCamera(i, 1280, 800);
	}
	
	while (!display.isClosed())
	{
		view->cleanView();

		int key = display.getKeyState();
		switch (key)
		{
		case 1:	// next step
			if (view_state < result_view)
			{
				view_state = (viewStates)(view_state + 1);
				switchState(view_state);
			}
			break;

		case 2:	// back 
			if (view_state > fisheye_view)
			{
				view_state = (viewStates)(view_state - 1);
				switchState(view_state);
			}
			break;

		//case 3:	// update
		//	std::cout << "Update parameters" << std::endl;
		//	if (param.readXML("../Content/settings.xml") == -1) break;
		//	updateState(view_state);
		//	break;

		default:
			break;
		}

		if (view_state != result_view)
		{
			if (view->setProgram(0) == 0)
			{
				for (uint i = 0; i < 4; i++)
					view->renderView(cam_views[i].camera_index, cam_views[i].mesh_index[0]);
			}
		}

		display.update();
	}
	
	return 0;
}