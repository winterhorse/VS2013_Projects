
//Rendering
#include "view.hpp"
#include "display.h"
#include "settings.hpp"
#include "camera.hpp"
#include "grid.hpp"
#include "masks.hpp"
#include "exposure_compensator.hpp"


/**********************************************************************************************************************
* Types
**********************************************************************************************************************/
struct camera_view {
	int camera_index;
	std::vector<int> mesh_index;
};

enum viewStates { fisheye_view = 0, defisheye_view, contours_view, grids_view, result_view };

XMLParameters param;			// Cameras parameters

Display *display;					// Display
View* view;						// View object
std::vector<camera_view> cam_views;	// View indexes
std::vector<Camera*> cameras;		// Cameras

#define CURVI

#ifdef CURVI
std::vector<CurvilinearGrid*> grids;	// Grids
#else
std::vector<RectilinearGrid*> grids;	// Grids
#endif

viewStates view_state = fisheye_view;

int tmpl_width;
int tmpl_height;
int contours_buf;
int grid_buf;

int objectsInit() 
{

	////////////////// Read XML parameters /////////////////////
	if (param.readXML("../Content/settings.xml") == -1) return (-1);

	if (param.setTmpPoint() == -1) return (-1);

	if (param.getTmpMaxVal("template_1.txt", &tmpl_width) == -1)
		return (-1);
	if (param.getTmpMaxVal("template_2.txt", &tmpl_height) == -1)
		return (-1);

	//////////////////////// Display ////////////////////////////
	display = new Display(WINDOW_WIDTH, WINDOW_HEIGHT, "Calibration");

	/////////////////// Create view object //////////////////////
	view = new View;

	if (view->addProgram(s_v_shader, s_f_shader) == -1) return (-1);
	if (view->addProgram(s_v_shader_line, s_f_shader_line) == -1) return (-1);
	if (view->addProgram(s_v_shader_bowl, s_f_shader_bowl) == -1) return (-1);
	if (view->setProgram(0) == -1) return (-1);

	for (int i = 0; i < param.camera_num; i++)
	{
		camera_view cam_view;
		cam_view.camera_index = view->addCamera(i, param.cameras[i].width, param.cameras[i].height);
		if (cam_view.camera_index == -1) return (-1);
		cam_view.mesh_index.push_back(view->addMesh(std::string("../Content/meshes/original/mesh" + std::to_string(i + 1))));
		cam_views.push_back(cam_view);
	}

	for (int i = 0; i < param.camera_num; i++)
	{
		view->runCamera(i);
	}		

	///////////////////// Create camera objects //////////////////
	for (int i = 0; i < param.camera_num; i++) {
		std::string calib_res_txt = param.camera_model + "/calib_results_" + std::to_string(i + 1) + ".txt"; // Camera model
		Creator creator;
		Camera* camera = creator.create(calib_res_txt.c_str(), param.cameras[i].sf, i); // Create Camera object
		if (camera == NULL) {
			std::cout << "Failed to create camera model" << std::endl;
			return (-1);
		}
		cameras.push_back(camera);
	}

	///////////////////// Create grid objects //////////////////
	for (int i = 0; i < param.camera_num; i++) {


#ifdef CURVI
		CurvilinearGrid* grid_element = new CurvilinearGrid(param.grid_angles, param.grid_start_angle, param.grid_nop_z, param.grid_step_x);
		grids.push_back(grid_element);
#else		
		RectilinearGrid* grid_element = new RectilinearGrid(param.grid_angles, param.grid_start_angle, param.grid_nop_z, param.grid_step_x);
		grids.push_back(grid_element);
#endif
	}
}

void printState(int vstate) {
	switch (vstate) {
	case fisheye_view:
		std::cout << "Load fisheye camera view..." << std::endl;
		break;
	case defisheye_view:
		std::cout << "Remove fisheye distortion..." << std::endl;
		break;
	case contours_view:
		std::cout << "Search contours..." << std::endl;
		break;
	case grids_view:
		std::cout << "Prepare meshes..." << std::endl;
		break;
	case result_view:
		std::cout << "Calculate the result view..." << std::endl;
		break;
	default:
		break;
	}
}

int searchContours(int index) {
	cv::Mat img = view->takeFrame(cam_views[index].camera_index);

	std::string ref_points_txt = param.tmplt + "/template_" + std::to_string(index + 1) + ".txt"; // Template points
	std::string chessboard = param.camera_model + "/chessboard_" + std::to_string(index + 1) + "/"; // Chessboard image

	// Set roi in which contours will be searched (in % of image height) and empiric bound for minimal allowed perimeter for contours
	cameras[index]->setRoi(param.cameras[index].roi);
	cameras[index]->setContourMinSize(param.cameras[index].cntr_min_size);
	// Set template size and reference points
	if (cameras[index]->setTemplate(ref_points_txt.c_str(), cv::Size2d(tmpl_width, tmpl_height)) != 0) return (-1);
	// Calculate intrinsic camera parameters using chessboard image
	std::string chessboard_name = std::string("frame" + std::to_string(index + 1) + "_");
	if (cameras[index]->setIntrinsic(chessboard.c_str(), chessboard_name.c_str(), param.cameras[index].chessboard_num, cv::Size(7, 7)) != 0) return (-1);
	//Estimate extrinsic camera parameters using calibrating template
	if (cameras[index]->setExtrinsic(img) != 0) return (-1);

	return (0);
}

int getContours(float** gl_lines) {
	float** contours = (float**)calloc(cameras.size(), sizeof(float*)); // Contour arrays for each camera
	
	int array_num[4] = { 0 }; // Number of array elements for each camera
	int sum_num = 0;
	int index = 0;

	for (uint i = 0; i < cameras.size(); i++) {
		if (searchContours(i) == 0) {
			array_num[i] = cameras[i]->getContours(&contours[i]);
			sum_num += array_num[i];
		}
	}
	*gl_lines = new float[sum_num];
	for (uint i = 0; i < cameras.size(); i++) {
		for (int j = 0; j < array_num[i]; j++) {
			(*gl_lines)[index] = contours[i][j];
			index++;
		}
	}
	for (int i = cameras.size() - 1; i >= 0; i--)
		if (contours[i]) free(contours[i]);
	if (contours) free(contours);

	return sum_num;
}

int getGrids(float** gl_grid) {
	if (cameras.size() == 0) return 0;
	float** grids_data = (float**)calloc(cameras.size(), sizeof(float*)); // Contour arrays for each camera
	if (!grids_data) {
		std::cout << "Cannot allocate memory" << std::endl;
		return(0);
	}

	int array_num[4] = { 0 }; // Number of array elements for each camera

	int sum_num = 0;
	int index = 0;

	int nopz = param.grid_nop_z;
	for (uint i = 0; i < cameras.size(); i++) { 	// Get number of points in z axis
		int tmp = cameras[i]->getBowlHeight(param.bowl_radius * cameras[i]->getBaseRadius(), param.grid_step_x);
		nopz = MIN(nopz, tmp);
	}

	std::vector< std::vector<cv::Point3f> > seam;
	for (uint i = 0; i < grids.size(); i++) {
#ifdef CURVI
		*grids[i] = CurvilinearGrid(param.grid_angles, param.grid_start_angle, nopz, param.grid_step_x);
#else		
		*grids[i] = RectilinearGrid(param.grid_angles, param.grid_start_angle, nopz, param.grid_step_x);
#endif

		grids[i]->createGrid(cameras[i], param.bowl_radius * cameras[i]->getBaseRadius()); // Calculate grid points and save grid to the file
		array_num[i] = grids[i]->getGrid(&grids_data[i]);
		sum_num += array_num[i];


	}

	*gl_grid = new float[sum_num];
	for (uint i = 0; i < cameras.size(); i++) {
		for (int j = 0; j < array_num[i]; j++) {
			(*gl_grid)[index] = grids_data[i][j];
			index++;
		}
	}

	for (int i = cameras.size() - 1; i >= 0; i--)
		if (grids_data[i]) free(grids_data[i]);
	if (grids_data) free(grids_data);

	return sum_num;
}

void saveGrids() {
	std::vector<std::vector<cv::Point3f> > seams;
	std::vector<cv::Point3f> seam_points;

	for (uint i = 0; i < grids.size(); i++) {
		grids[i]->saveGrid(cameras[i]);
		grids[i]->getSeamPoints(seam_points);	// Get grid seams
		seams.push_back(seam_points);
	}

	Masks masks;
	masks.createMasks(cameras, seams, param.smooth_angle); // Calculate masks for blending       
	masks.splitGrids();

	Compensator compensator(cv::Size(param.disp_width, param.disp_height)); // Exposure correction  
	compensator.feed(cameras, seams);
	compensator.save((char*)"./compensator");
}


void switchState(int new_state) {
	float* data;
	int data_num;

	printState(new_state);

	switch (new_state) {
	case fisheye_view:
		// Fisheye
		for (uint i = 0; i < cameras.size(); i++) {
			std::string filename = "../Content/meshes/original/mesh" + std::to_string(i + 1);
			view->reloadMesh(cam_views[i].mesh_index[0], filename);
		}
		break;

	case defisheye_view:
		// Defisheye
		for (uint i = 0; i < cameras.size(); i++)
			view->changeMesh(cameras[i]->xmap, cameras[i]->ymap, 10, cv::Point2f(((i & 1) - 1.0), ((~i >> 1) & 1)), cam_views[i].mesh_index[0]);
		break;

	case contours_view:
		// Contours searching
		data_num = getContours(&data);
		contours_buf = view->addBuffer(&data[0], data_num / 3);
		view->setBufferAsAttr(contours_buf, 1, (char*)"vPosition");
		if (data) free(data);
		break;

	case grids_view:
		// Mesh generation
		data_num = getGrids(&data);
		grid_buf = view->addBuffer(&data[0], data_num / 3);
		view->setBufferAsAttr(grid_buf, 1, (char*)"vPosition");
		if (data) free(data);
		break;

	case result_view:
		// Result view
		saveGrids();

		std::cout << "finished save grid" << std::endl;

		for (uint i = 0; i < cam_views.size(); i++) {
			if (cam_views[i].mesh_index.size() == 1) {
				cam_views[i].mesh_index.push_back(view->addMesh(std::string("array" + std::to_string(i + 1) + "1")));
				cam_views[i].mesh_index.push_back(view->addMesh(std::string("array" + std::to_string(i + 1) + "2")));
			}
			else {
				view->reloadMesh(cam_views[i].mesh_index[1], std::string("array" + std::to_string(i + 1) + "1"));
				view->reloadMesh(cam_views[i].mesh_index[2], std::string("array" + std::to_string(i + 1) + "2"));
			}
		}

		break;

	default:
		break;
	}
	std::cout << "Done" << std::endl;
}

void updateState(int new_state) {
	float* data;
	int data_num;

	printState(new_state);

	switch (new_state) {

	case defisheye_view:
		// Defisheye
		for (uint i = 0; i < cameras.size(); i++) {
			cameras[i]->updateLUT(param.cameras[i].sf);
			view->changeMesh(cameras[i]->xmap, cameras[i]->ymap, 10, cv::Point2f(((i & 1) - 1.0), ((~i >> 1) & 1)), cam_views[i].mesh_index[0]);
		}
		break;

	case contours_view:
		// Contours searching
		data_num = getContours(&data);
		view->updateBuffer(contours_buf, &data[0], data_num / 3);
		if (data) free(data);
		break;

	case grids_view:
		// Mesh generation
		data_num = getGrids(&data);
		view->updateBuffer(grid_buf, &data[0], data_num / 3);
		if (data) free(data);
		break;

	case result_view:
		for (uint i = 0; i < cam_views.size(); i++) {
			view->reloadMesh(cam_views[i].mesh_index[1], std::string("array" + std::to_string(i + 1) + "1"));
			view->reloadMesh(cam_views[i].mesh_index[2], std::string("array" + std::to_string(i + 1) + "2"));
		}
		break;

	default:
		break;
	}
	std::cout << "Done" << std::endl;
}

int main() 
{
	if (objectsInit() == -1) 
	{
		std::cout << "objectsInit failed!" << std::endl;

		return -1;
	}

	std::cout << "objectsInit success!" << std::endl;

	param.printParam();


	while (!display->isClosed()) 
	{

		view->cleanView();

		int key = display->getKeyState();

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

			case 3:	// update
				std::cout << "Update parameters" << std::endl;
				if (param.readXML("../Content/settings.xml") == -1) break;
				updateState(view_state);
				break;

			default:
				break;
		}

		if (view_state != result_view) {
			if (view->setProgram(0) == 0) {
				for (uint i = 0; i < cam_views.size(); i++)
					view->renderView(cam_views[i].camera_index, cam_views[i].mesh_index[0]);
			}
		}

		if (view_state == contours_view) {
			if (view->setProgram(1) == 0)
				view->renderBuffer(contours_buf, 0, view->getVerticesNum(contours_buf));
		}

		if (view_state == grids_view) {
			if (view->setProgram(1) == 0)
				view->renderBuffer(grid_buf, 1, view->getVerticesNum(grid_buf));
		}

		if (view_state == result_view) {
			if (view->setProgram(2) == 0) {
				for (uint i = 0; i < cam_views.size(); i++) {
					view->renderView(cam_views[i].camera_index, cam_views[i].mesh_index[1]);
					view->renderView(cam_views[i].camera_index, cam_views[i].mesh_index[2]);
				}
			}
		}


		display->update();
		glFinish();
	}

	return 0;
}

