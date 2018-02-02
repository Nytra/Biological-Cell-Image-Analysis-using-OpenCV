#include <iostream>
#include <SFML\Graphics.hpp>
#include <string>
#include <Windows.h>
#include <shlobj.h>  
#include "constants.h"
#include "tiffio.h"

// ToDo: Improve cluster scanning, save results to a file (database? csv? xml?)

struct Coord {
	int x;
	int y;
	Coord() {}
	Coord(int _x, int _y) {
		x = _x;
		y = _y;
	}
	
};

class Image {
private:
	uint32 _height;
	uint32 _width;
	std::vector<int> _red; // one-dimensional for no reason whatsoever
	std::vector<int> _green;
	std::vector<int> _blue;
public:
	void setDimensions(uint32 width, uint32 height) {
		_width = width;
		_height = height;
	}
	void setRed(std::vector<int> red) {
		_red = red;
	}
	void setGreen(std::vector<int> green) {
		_green = green;
	}
	void setBlue(std::vector<int> blue) {
		_blue = blue;
	}
	uint32 getHeight() {
		return _height;
	}
	uint32 getWidth() {
		return _width;
	}
	std::vector<int> getRed() {
		return _red;
	}
	std::vector<int> getGreen() {
		return _green;
	}
	std::vector<int> getBlue() {
		return _blue;
	}
};

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
			if (ends_with(name, ".lsm")) {
				names.push_back(newPath + "\\" + fd.cFileName);
			}
			
			//}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	return names;
}

bool checkCoord(std::vector<Coord> coords, int x, int y) {
	for (int i = 0; i < coords.size(); i++) {
		if (coords[i].x == x &&
			coords[i].y == y) {
			return true;
		}
	}
	return false;
}

std::vector<Coord> findPixels(std::vector<int> pixels, int scanSize, double tolerance, int width, int height) {
	std::vector<Coord> coords;
	int sum, count, pixelCount, threshold, value;

	// get average pixel value and multiply it by the tolerance

	sum = 0;
	count = 0;
	pixelCount = width * height;

	for (int i = 0; i < pixelCount; i++) {

		if (pixels[i] > 0) {
			count += 1;
			sum += pixels[i];
		}
	}

	if (count > 0) {
		threshold = (sum / count) * tolerance;  
	}
	else {
		threshold = 0;
	}

	std::cout << "Intensity Threshold: " << threshold << std::endl;

	// begin main scan

	for (int y = 0; y < height - scanSize; y += scanSize) {

		for (int x = 0; x < width - scanSize; x += scanSize) {

			sum = 0;
			count = 0;
			value = 0;

			for (int j = y; j < y + scanSize; j++) {
				for (int k = x; k < x + scanSize; k++) {
					value = pixels[(j * width) + k];
					sum += value;
					count += 1;
				}
			}

			if (count > 0 && sum / count > threshold) {
				coords.push_back(Coord(x, y));
			}
		}
	}

	return coords;
}

Image getImage(std::string path) {
	Image img;
	std::vector<int> red;
	std::vector<int> green;
	std::vector<int> blue;
	TIFF* tif = TIFFOpen(convertLSMToTIFF(path).c_str(), "r");

	if (tif) {
		uint32 width, height;
		size_t npixels;
		uint32* raster;
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		npixels = width * height;
		raster = (uint32*)_TIFFmalloc(npixels * sizeof(uint32));

		if (raster != NULL) {

			if (TIFFReadRGBAImage(tif, width, height, raster, 0)) {

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
		img.setRed(red);
		img.setGreen(green);
		img.setBlue(blue);
		img.setDimensions(width, height);
	}

	return img;
}

std::string BrowseFolder()
{
	char path[MAX_PATH];
	BROWSEINFO bi = { 0 };
	bi.lpszTitle = ("Select Source Directory");
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		// get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);
		return std::string(path);
	}
	else {
		return std::string();
	}

}

int main() {
	std::vector<Coord> coords;
	std::vector<Coord> clusters;
	std::string path;
	std::vector<std::string> names;

	path = BrowseFolder();

	if (path.empty()) {
		return -1;
	}

	std::cout << path << std::endl;
	names = get_all_files_names_within_folder(path);
	std::cout << "Images to Process: " << names.size() << std::endl;

	sf::RenderWindow window(sf::VideoMode(800, 800), "Cell Counter");
	sf::RectangleShape pixel(sf::Vector2f(2, 2));
	window.setFramerateLimit(60);
	pixel.setFillColor(sf::Color::Blue);
	int imageIndex = 0; 
	bool done = false;
	Image img;

	while (window.isOpen()) {
		sf::Event event;

		while (window.pollEvent(event)) {

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		img = getImage(names[imageIndex]); 

		//std::cout << names[imageIndex] << std::endl;

		window.setSize(sf::Vector2u((int)img.getWidth(), (int)img.getHeight()));
		coords = findPixels(img.getBlue(), img.getWidth() / scanSizeDiv, 1, img.getWidth(), img.getHeight());
		std::cout << "Found " << coords.size() << " coords." << std::endl;

		window.clear(sf::Color::White);

		for (int i = 0; i < coords.size(); i++) { // draw the points
			pixel.setPosition(sf::Vector2f(coords[i].x, coords[i].y));
			window.draw(pixel);
		}

		int x, y, radius, count, scanSize, dist, dy, dx;
		scanSize = img.getWidth() / scanSizeDiv;
		radius = 2;
		count = 0;
		for (int i = 0; i < coords.size(); i++) {
			x = coords[i].x;
			y = coords[i].y;
			//count = 0;
			for (int j = 0; j < coords.size(); j++) {
				if (i != j) {
					dy = std::abs(y - coords[j].y);
					dx = std::abs(x - coords[j].x);
					dist = std::sqrt((dx * dx) + (dy * dy)) / scanSize;
					//std::cout << dist << std::endl;
					if (dist == 1 && coords[j].x != 0) {
						count += 1;
						coords[j].x = 0;
					}
				}
			}
			
			// check around this coord in multiples of the scanSize
		}
		if (count > 0) {
			std::cout << count << std::endl;
		}
		//std::cout << "Image " << imageIndex + 1 << " - Found " << clusters.size() << " clusters." << std::endl;

		window.display();

		system("pause"); 

	}

	//system("pause");

	return 0;
}