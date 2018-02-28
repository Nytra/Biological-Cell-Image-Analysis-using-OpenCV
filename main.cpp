#include <iostream>
#include <SFML\Graphics.hpp>
#include <string>
#include <Windows.h>
#include <shlobj.h>  
#include <cmath>
#include "opencv2\core.hpp"
#include "opencv2\opencv.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\imgproc.hpp"
#include "constants.h"

// ToDo: Improve cluster scanning, save results to a file (database? csv? xml?)

bool ends_with(std::string const & value, std::string const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string getexepath() {
	char result[MAX_PATH];
	GetModuleFileName(NULL, result, MAX_PATH);
	std::string::size_type pos = std::string(result).find_last_of("\\/");
	return std::string(result).substr(0, pos);
}

std::string convertLSMToTIFF(std::string filename) {
	std::string oname = filename + ".tif";
	std::string path = getexepath();
	std::string command = path + "\\imgcnv.exe -i \"" + filename + "\" -o \"" + oname + "\" -t TIFF -display";
	system(command.c_str());
	return oname;
}

std::vector<std::string> get_all_files_names_within_folder(std::string folder) {

	std::string newPath;
	for (int i = 0; i < folder.length(); i++) {
		if (folder[i] != (char)"\\") {
			newPath += folder[i];
		}
		else {
			newPath += "/";
		}
	}

	std::vector<std::string> names;
	std::string search_path = newPath + "\\*";
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
				names.push_back(newPath + "\\" + fd.cFileName);
			}
			
			//}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	return names;
}

std::string BrowseFolder()
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

int main() {
	std::string path;
	std::vector<std::string> names;

	path = BrowseFolder();

	if (path.empty()) {
		return 1;
	}

	std::cout << path << std::endl;
	names = get_all_files_names_within_folder(path);
	std::cout << "Images to Process: " << names.size() << std::endl;

	cv::Mat img, img_gray;
	std::vector<cv::Vec3f> circles;
	int dilation_size = 2;

	for (std::string filename : names) {
		img = cv::imread(filename, cv::IMREAD_COLOR);
		cv::cvtColor(img, img_gray, CV_BGR2GRAY);
		cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
													cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
													cv::Point(dilation_size, dilation_size));
		cv::dilate(img_gray, img_gray, element);
		//cv::HoughCircles(img_gray, circles, CV_HOUGH_GRADIENT, 1, img_gray.rows / 8, 20, 50, 0, 0);

		std::cout << "Found " << circles.size() << " circles." << std::endl;
		for (int i = 0; i < circles.size(); i++) {
			cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			// circle center
			cv::circle(img, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
			// circle outline
			cv::circle(img, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
		}

		cv::imshow("Image", img);
		cv::waitKey(0);
	}

	return 0;
}