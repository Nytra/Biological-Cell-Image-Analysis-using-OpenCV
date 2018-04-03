#include <iostream>
//#include <SFML\Graphics.hpp>
#include <string>
#include <Windows.h>
#include <shlobj.h>  
#include <cmath>
#include "opencv2\core.hpp"
#include "opencv2\opencv.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\imgproc.hpp"
#include "opencv2\imgcodecs.hpp"
#include "opencv2\features2d.hpp"
#include "opencv2\xfeatures2d.hpp"
#include "constants.h"

using namespace cv;
using namespace cv::xfeatures2d;

//int dilation_size, erosion_size;

// ToDo: Improve cluster scanning, save results to a file (database? csv? xml?)

bool ends_with(std::string const & value, std::string const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string get_exe_path() {
	char result[MAX_PATH];
	GetModuleFileName(NULL, result, MAX_PATH);
	std::string::size_type pos = std::string(result).find_last_of("\\/");
	return std::string(result).substr(0, pos);
}

std::string convertLSMToTIFF(std::string filename) {
	std::string oname = filename + ".tif";
	std::string path = get_exe_path();
	std::string command = path + "\\imgcnv.exe -i \"" + filename + "\" -o \"" + oname + "\" -t TIFF -display";
	system(command.c_str());
	return oname;
}

std::vector<std::string> get_all_files_names_within_folder(std::string folder) {

	std::string new_path;
	for (int i = 0; i < folder.length(); i++) {
		if (folder[i] != (char)"\\") {
			new_path += folder[i];
		}
		else {
			new_path += "/";
		}
	}

	std::vector<std::string> names;
	std::string search_path = new_path + "\\*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			//if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			//std::cout << fd.cFileName << std::endl;
			std::string name(fd.cFileName);
			if (ends_with(name, ".lsm")) {
				names.push_back(new_path + "\\" + fd.cFileName);
			}
			
			//}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	return names;
}

std::string browseFolder()
{
	char path[MAX_PATH];
	BROWSEINFO bi = { 0 };
	bi.lpszTitle = ("Select Source Directory");
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		// get the name of the folder and put it in path buffer
		SHGetPathFromIDList(pidl, path);
		return std::string(path);
	}
	else {
		return std::string();
	}

}

void erode(cv::Mat img, int size) {
	cv::Mat element = cv::getStructuringElement(
		cv::MORPH_ELLIPSE,
		cv::Size(2 * size + 1, 2 * size + 1),
		cv::Point(size, size)
	);
	cv::erode(img, img, element);
}

void dilate(cv::Mat img, int size) {
	cv::Mat element = cv::getStructuringElement(
		cv::MORPH_ELLIPSE,
		cv::Size(2 * size + 1, 2 * size + 1),
		cv::Point(size, size)
	);
	cv::dilate(img, img, element);
}

int main() {
	std::string path;
	std::vector<std::string> names;

	//path = browseFolder();
	path = "C:\\Users\\samue\\Desktop\\Data\\PIC to be converted in TIF\\2017.06.02 ReNd28 treatments";

	if (path.empty()) {
		return 1;
	}

	std::cout << path << std::endl;
	names = get_all_files_names_within_folder(path);
	std::cout << "Images to Process: " << names.size() << std::endl;

	cv::Mat img, img_gray;
	std::vector<cv::Vec3f> circles;
	int dilation_size = 5;
	int erosion_size = 6;

	for (std::string filename : names) {
		img_gray = cv::imread(filename, cv::IMREAD_GRAYSCALE);
		if (img_gray.empty()) {
			std::cout << "Error opening image" << std::endl;
			continue;
		}

		int minHessian = 5000; 
		int pixThresh = 30;
		GaussianBlur(img_gray, img_gray, cv::Size(11, 11), 1);
		threshold(img_gray, img_gray, pixThresh, 255, THRESH_BINARY);
		erode(img_gray, 2);
		//dilate(img_gray, 4);

		Ptr<SURF> detector = SURF::create(minHessian);
		std::vector<KeyPoint> keypoints_1;
		detector->detect(img_gray, keypoints_1);
		Mat img_keypoints_1;
		drawKeypoints(img_gray, keypoints_1, img_keypoints_1, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

		std::cout << "Found " << keypoints_1.size() << " keypoints." << std::endl;

		cv::imshow("Image", img_keypoints_1);
		//cv::imshow("Spectrum Magnitude", magI);
		cv::waitKey(0);
	}

	return 0;
}