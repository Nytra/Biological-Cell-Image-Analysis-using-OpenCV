#include <iostream>
#include <SFML\Graphics.hpp>
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

// scans the whole image for clusters and stores them in the coords vector
std::vector<Coord> findClusters(std::vector<int> pixels, double tolerance, int width, int height) {
	std::vector<Coord> coords;
	int v, cx, cy;
	int sum = 0;
	int pixelCount = width * height;
	int count = 0;
	int scanSize = 5;

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

	TIFF* tif = TIFFOpen("test.tif", "r");
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
		imageWidth = width;
		imageHeight = height;
	}

	sf::RenderWindow window(sf::VideoMode(imageWidth, imageHeight), "Application");
	sf::RectangleShape pixel(sf::Vector2f(1, 1));
	sf::RectangleShape cluster(sf::Vector2f(2, 2));
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
			int x = coords[i].x * (imageWidth / windowWidth);
			int y = coords[i].y * (imageHeight / windowHeight);
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