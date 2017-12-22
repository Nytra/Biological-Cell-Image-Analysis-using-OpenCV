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

class Image {
private:
	uint32 _height;
	uint32 _width;
	std::vector<int> _red;
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

inline bool ends_with(std::string const & value, std::string const & ending)
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

// scans the whole image for clusters and stores them in the coords vector
std::vector<Coord> findClusters(std::vector<int> pixels, int scanSize, double tolerance, int width, int height) {
	std::vector<Coord> coords;
	int v;
	int sum = 0;
	int pixelCount = width * height;
	int count = 0;

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

int main() {
	std::vector<Coord> coords;
	std::string path;
	std::vector<std::string> names;

	std::cout << "Path to source directory: ";
	std::getline(std::cin, path);
	names = get_all_files_names_within_folder(path);
	std::cout << "Images to Process: " << names.size() << std::endl;

	sf::RenderWindow window(sf::VideoMode(1, 1), "Application");
	sf::RectangleShape pixel(sf::Vector2f(1, 1));
	sf::Clock clock;
	pixel.setFillColor(sf::Color::Blue);
	int imageIndex = 0;
	bool done = false;
	bool wait = false;
	bool b = false;
	sf::Time time;
	Image img;

	while (window.isOpen()) {
		sf::Event event;

		while (window.pollEvent(event)) {

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		if (wait) {
			if (clock.getElapsedTime().asSeconds() > 5) {
				wait = false;
				clock.restart();
			}
		}

		window.clear(sf::Color::White);

		//if (!done) {
		//	if (!b) {
		//		img = getImage(names[imageIndex]);
		//		window.setSize(sf::Vector2u(img.getWidth(), img.getHeight()));
		//		b = true;
		//	}

		//	for (int y = 0; y < img.getHeight(); y++) {
		//		for (int x = 0; x < img.getWidth(); x++) {
		//			int val = img.getBlue()[(y * img.getWidth()) + x];
		//			if (val > 0) {
		//				pixel.setPosition(sf::Vector2f(x, y));
		//				window.draw(pixel);
		//			}
		//		}
		//	}

		coords = findClusters(img.getBlue(), img.getWidth() / 20, tolerance, img.getWidth(), img.getHeight());
		//for (int i = 0; i < coords.size(); i++) {
		//pixel.setPosition(sf::Vector2f(coords[i].x, coords[i].y));
		//		//window.draw(pixel);
		//	//}
		std::cout << "Image " << imageIndex + 1 << " - Found " << coords.size() << " coords." << std::endl;
		//}

		window.display();

		if (imageIndex < names.size() - 1) {
			imageIndex += 1;
		}
		else {
			done = true;
		}

		//if (!wait) {
			//wait = true;
			//clock.restart();
		//}
	}

	system("pause");

	return 0;
}