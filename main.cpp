#include <iostream>
#include <SFML\Graphics.hpp>
#include <string>
#include <Windows.h>
#include "constants.h"
#include "tiffio.h"

struct Coord {
	int x;
	int y;
};

class Cluster {
private:
	std::vector<Coord> _coords;
public:
	Cluster(std::vector<Coord> coords) {
		_coords = coords;
	}

	Cluster() {}
	
	void addCoord(Coord c) {
		_coords.push_back(c);
	}

	std::vector<Coord> getCoords() {
		return _coords;
	}

	void clear() {
		_coords.clear();
	}
};

std::string getexepath() {
	char result[MAX_PATH];
	GetModuleFileName(NULL, result, MAX_PATH);
	std::string::size_type pos = std::string(result).find_last_of("\\/");
	return std::string(result).substr(0, pos);
}

std::string convertLSMToTIFF(std::string filename) {
	std::string oname = filename + ".tif";
	std::string path = getexepath();
	std::string command = path + "\\imgcnv.exe -i " + filename + " -o " + oname + " -t TIFF -display -flip";
	system(command.c_str());
	return oname;
}

std::vector<std::string> get_all_files_names_within_folder(std::string folder)
{

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
			if (std::size_t(name.find(std::string(".lsm")) != std::string::npos)) {
				names.push_back(fd.cFileName);
			}
			
			//}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	return names;
}

// scans the whole image for clusters and stores them in the coords vector
std::vector<Coord> findClusters(std::vector<int> pixels, double tolerance, int width, int height) {
	std::vector<Coord> coords;
	int v, cx, cy;
	int sum = 0;
	int pixelCount = width * height;
	int count = 0;
	int scanSize = 10;

	for (int i = 0; i < pixelCount; i++) {

		if (pixels[i] > 0) {
			count += 1;
			sum += pixels[i];
		}
	}

	int threshold = (sum / count) * tolerance;  // average pixel value multiplied by the inverse of the tolerance

	for (int y = 0; y < height - scanSize; y += scanSize) {

		for (int x = 0; x < width - scanSize; x += scanSize) {
			sum = 0;
			count = 0;

			for (int j = y; j < y + scanSize; j++) {

				for (int k = x; k < x + scanSize; k++) {
					v = pixels[(j * width) + k];
					sum += v;
					count += 1;
				}
			}

			if (count > 0 && sum / count > threshold) {
				Coord coord;
				coord.x = x;
				coord.y = y;
				coords.push_back(coord);
			}
		}
	}

	return coords;
}

int main() {
	sf::Font font;
	uint32 imageHeight;
	uint32 imageWidth;

	std::vector<Coord> coords;

	std::vector<int> red;
	std::vector<int> green;
	std::vector<int> blue;

	std::string path;
	std::vector<std::string> names;

	std::cout << "Path to source directory: ";
	std::getline(std::cin, path);
	names = get_all_files_names_within_folder(path);
	for (int i = 0; i < names.size(); i++) {
		std::cout << names[i] << std::endl;
	}
	system("pause");

	std::string iname = "test.lsm";

	TIFF* tif = TIFFOpen(convertLSMToTIFF(iname).c_str(), "r");
	if (tif) {
		//uint32 width, height;
		size_t npixels;
		uint32* raster;
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &imageWidth);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imageHeight);
		npixels = imageWidth * imageHeight;
		raster = (uint32*)_TIFFmalloc(npixels * sizeof(uint32));
		if (raster != NULL) {
			if (TIFFReadRGBAImage(tif, imageWidth, imageHeight, raster, 0)) {
				for (int i = 0; i < npixels; i++) {
					int r = TIFFGetR(raster[i]);
					int g = TIFFGetG(raster[i]);
					int b = TIFFGetB(raster[i]);
					red.push_back(r);
					green.push_back(g);
					blue.push_back(b);
				}
			}
			_TIFFfree(raster);
		}
		TIFFClose(tif);
	}

	sf::RenderWindow window(sf::VideoMode(imageWidth, imageHeight), "Application");
	sf::RectangleShape pixel(sf::Vector2f(1, 1));
	sf::RectangleShape cluster(sf::Vector2f(10, 10));
	cluster.setFillColor(sf::Color::Red);
	window.setFramerateLimit(30);
	bool processed = false;

	while (window.isOpen()) {

		sf::Event event;
		while (window.pollEvent(event)) {

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		window.clear(sf::Color::White);

		for (int y = 0; y < imageHeight; y++) {
			for (int x = 0; x < imageWidth; x++) {
				int value = blue[(y * imageWidth) + x];
				if (value > 0) {
					pixel.setFillColor(sf::Color(0, 0, value));
					pixel.setPosition(x, y);
					window.draw(pixel);
				}
			}
		}
		
		if (!processed) {
			processed = true;
			coords = findClusters(blue, tolerance, imageWidth, imageHeight);
			std::cout << "Found " << coords.size() << " coords." << std::endl;
		}

		for (int i = 0; i < coords.size(); i++) {
			int x = coords[i].x;
			int y = coords[i].y;
			cluster.setPosition(sf::Vector2f(x, y));
			window.draw(cluster);
		}

		//	// start cluster logic

		//	std::vector<Coord> checked;
		//	Coord c;
		//	int count = 0;

		//	// use a while loop to make a recursive path finder which keeps going until there is nowhere else to go
		//	for (int i = 0; i < coords.size(); i++) {

		//		break;

		//		for (int j = 0; j < coords.size(); j++) {

		//			if (i == j) {
		//				continue;
		//			}

		//			bool found = false;
		//			for (int k = 0; k < checked.size(); k++) {
		//				if (checked[k].x == coords[i].x || checked[k].y == coords[i].y) {
		//					found = true;
		//				}
		//				if (checked[k].x == coords[j].x || checked[k].y == coords[j].y) {
		//					found = true;
		//				}
		//			}

		//			if (found) {
		//				continue;
		//			}

		//			// right left
		//			if (coords[i].x == coords[j].x + scanSize && coords[i].y == coords[j].y) { // right
		//				count += 1;
		//				c.x = coords[j].x + scanSize;
		//				c.y = coords[j].y;
		//				checked.push_back(c);
		//				continue;
		//			}
		//			if (coords[i].x == coords[j].x - scanSize && coords[i].y == coords[j].y) { // left
		//				count += 1;
		//				c.x = coords[j].x - scanSize;
		//				c.y = coords[j].y;
		//				checked.push_back(c);
		//				continue;
		//			}

		//			// up down
		//			if (coords[i].x == coords[j].x && coords[i].y == coords[j].y + scanSize) { // down
		//				count += 1;
		//				c.x = coords[j].x;
		//				c.y = coords[j].y + scanSize;
		//				checked.push_back(c);
		//				continue;
		//			}
		//			if (coords[i].x == coords[j].x && coords[i].y == coords[j].y - scanSize) { // up
		//				count += 1;
		//				c.x = coords[j].x;
		//				c.y = coords[j].y - scanSize;
		//				checked.push_back(c);
		//				continue;
		//			}

		//			// diagonals
		//			if (coords[i].x == coords[j].x + scanSize && coords[i].y == coords[j].y + scanSize) { // bottom right
		//				count += 1;
		//				c.x = coords[j].x + scanSize;
		//				c.y = coords[j].y + scanSize;
		//				checked.push_back(c);
		//				continue;
		//			}
		//			if (coords[i].x == coords[j].x + scanSize && coords[i].y == coords[j].y - scanSize) { // top right
		//				count += 1;
		//				c.x = coords[j].x + scanSize;
		//				c.y = coords[j].y - scanSize;
		//				checked.push_back(c);
		//				continue;
		//			}
		//			if (coords[i].x == coords[j].x - scanSize && coords[i].y == coords[j].y + scanSize) { // bottom left
		//				count += 1;
		//				c.x = coords[j].x - scanSize;
		//				c.y = coords[j].y + scanSize;
		//				checked.push_back(c);
		//				continue;
		//			}
		//			if (coords[i].x == coords[j].x - scanSize && coords[i].y == coords[j].y - scanSize) { // top left
		//				count += 1;
		//				c.x = coords[j].x - scanSize;
		//				c.y = coords[j].y - scanSize;
		//				checked.push_back(c);
		//				continue;
		//			}
		//		}
		//	}

		//	std::cout << "Found " << count << " clusters." << std::endl;

		//	
		//}

		//// end cluster logic

		

		//text.setString(std::to_string(clusters[i].value));
		//text.setPosition(x, y);
		//window.draw(text);


		window.display();
		
	}

	return 0;
}